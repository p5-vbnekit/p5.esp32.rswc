#include <cstdint>

#include <utility>
#include <exception>
#include <stdexcept>

#include <fmt/format.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/common.hpp>

#include "event_loop.hpp"
#include "logged_action.hpp"

#include "main_task.hpp"


namespace p5::rswc::implementation_::main_task {
namespace private_::sdk {

    enum class Events: ::std::int32_t { Test };

    inline constexpr static ::esp_event_base_t event_base() noexcept(true) {
        return "p5.rswc.main_task";
    }

} // namespace private_::sdk

    common::coro::Task<void> make() noexcept(false) {
        return [] () -> common::coro::Task<void> {
            [[maybe_unused]] auto finally_ = common::utils::with_finally(
                [] (auto &&finally) { return ::std::forward<decltype(finally)>(finally); },
                [] (auto &&exception) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                    log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                }); }
            );

            finally_([] () { log<LogLevel::Info>(::fmt::format(
                "main task [thread = {}]: about to finish",
                static_cast<void const *>(::xTaskGetCurrentTaskHandle())
            )); });

            if constexpr (true) {
                auto event_loop_ = event_loop::instance();
                if (! event_loop_) throw ::std::logic_error{"event loop pointer is null"};

                log<LogLevel::Info>(::fmt::format(
                    "main task [thread = {}] started, event_loop = [{}, {}]",
                    static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                    static_cast<void const *>(event_loop_.get()),
                    event_loop_.use_count()
                ));
            }

            static auto future_ = common::coro::Future<::std::string>{};

            logged_action::execute("initializing event handler", [] () { common::sdk::check_or_throw(::esp_event_handler_register(
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

            co_await future_;

            log<LogLevel::Info>(::fmt::format(
                "main task [thread = {}]: future received = {}",
                static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                future_.result()
            ));

            co_return;
        } ();
    }

    void post_test_event() noexcept(false) {
        auto const event_loop_ = event_loop::instance();
        if (! event_loop_) throw ::std::logic_error{"event loop is not initialized"};
        common::sdk::check_or_throw(::esp_event_post(
            private_::sdk::event_base(), static_cast<::std::int32_t>(private_::sdk::Events::Test),
            nullptr, 0, portMAX_DELAY
        ));
    }

} // namespace p5::rswc::implementation_::main_task
