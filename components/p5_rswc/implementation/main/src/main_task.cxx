#include <thread>
#include <sstream>
#include <utility>

#include <fmt/format.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>

#include <p5/rswc/implementation_/common.hpp>
#include <p5/rswc/implementation_/log.hpp>

#include "event_loop.hpp"
#include "main_task.hpp"


namespace p5::rswc::implementation_::main_task {
namespace private_ {
namespace sdk {

    enum class Events: ::std::int32_t { Test };

    inline constexpr static ::esp_event_base_t event_base() noexcept(true) {
        return "p5.rswc.main_task";
    }

} // namespace sdk


    template <class Name, class Delegate> inline static auto action(
        Name &&name, Delegate &&delegate
    ) noexcept(false) {
        struct SuccessNotifier_ final {
            bool enabled = false;
            ::std::string_view const name;
            inline ~SuccessNotifier_() noexcept(true) {
                try { if (enabled) log<LogLevel::Info>(::fmt::format("action completed successfully: {}", ::std::move(name))); }
                catch(...) { ::std::terminate(); }
            }
        } success_notifier_ {.name = name};
        log<LogLevel::Verbose>(::fmt::format("attempt to perform an action: {}", name));
        try {
            success_notifier_.enabled = true;
            return ::std::forward<Delegate>(delegate)();
        }
        catch(...) {
            success_notifier_.enabled = false;
            ::std::throw_with_nested(::std::runtime_error{::fmt::format(
                "action failed: {}", ::std::forward<Name>(name)
            )});
        }
    }

    template <class T> inline static auto thread_id_to_string(T &&thread_id)
    noexcept(false) requires(::std::is_convertible_v<T, ::std::thread::id>)
    {
        ::std::ostringstream stream;
        stream << ::std::forward<T>(thread_id);
        return ::std::move(stream.str());
    }

    inline static auto thread_id_to_string() noexcept(false) { return thread_id_to_string(::std::this_thread::get_id()); }

} // namespace private_

    common::coro::Task<void> make() noexcept(false) {
        return common::utils::with_finally(
            [] (auto finally) {
                log<LogLevel::Info>(::fmt::format(
                    "creating main task, thread = {}",
                    static_cast<void const *>(::xTaskGetCurrentTaskHandle())
                ));

                private_::action("initializing event loop", [] () { event_loop::initialize(); });
                finally([] () { private_::action("deinitializing event loop", [] () { event_loop::deinitialize(); }); });

                return [] (auto finally) -> common::coro::Task<void> {
                    auto finally_ = ::std::move(finally);

                    // if constexpr (true) {
                    //     auto event_loop_ = event_loop::instance();
                    //     if (! event_loop_) throw ::std::logic_error{"event loop pointer is null"};

                    //     log<LogLevel::Info>(::fmt::format(
                    //         "main task [thread = {}] started, event_loop = {}",
                    //         static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                    //         static_cast<void const *>(event_loop_.get())
                    //     ));
                    // }

                    static auto future_ = common::coro::Future<::std::string>{};

                    private_::action("initializing event handler", [] () { common::sdk::check_or_throw(::esp_event_handler_register(
                        private_::sdk::event_base(), static_cast<::std::int32_t>(private_::sdk::Events::Test), [] (
                            void *handler_arguments, char const *event_base, ::std::int32_t event_id, void *event_data
                        ) {
                            try {
                                auto &&message_ = ::fmt::format(
                                    "{}, {}, {}, {}",
                                    ::std::forward<decltype(handler_arguments)>(handler_arguments),
                                    ::std::forward<decltype(event_base)>(event_base),
                                    ::std::forward<decltype(event_id)>(event_id),
                                    ::std::forward<decltype(event_data)>(event_data)
                                );

                                log<LogLevel::Info>(::fmt::format(
                                    "main task [thread = {}]: event handler - {}",
                                    static_cast<void const *>(::xTaskGetCurrentTaskHandle()), message_
                                ));

                                if (! future_) future_.set_result(::std::forward<decltype(message_)>(message_));
                            }
                            catch (...) { ::std::terminate(); }
                        },
                        nullptr
                    )); });

                    log<LogLevel::Info>(::fmt::format(
                        "main task [thread = {}]: waiting for future",
                        static_cast<void const *>(::xTaskGetCurrentTaskHandle())
                    ));

                    log<LogLevel::Info>(::fmt::format(
                        "main task [thread = {}]: future ready = {}",
                        static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                        co_await future_
                    ));

                    co_return;
                } (::std::move(finally));
            },
            [] (auto &&exception) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
            }); }
        );
    }

    void post_test_event() noexcept(false) {
        auto event_loop_ = event_loop::instance();
        if (! event_loop_) throw ::std::logic_error{"event loop is not initialized"};
        common::sdk::check_or_throw(::esp_event_post(
            private_::sdk::event_base(), static_cast<::std::int32_t>(private_::sdk::Events::Test),
            nullptr, 0, portMAX_DELAY
        ));
    }

} // namespace p5::rswc::implementation_::main_task
