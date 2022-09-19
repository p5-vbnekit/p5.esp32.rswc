#include <mutex>
#include <memory>
#include <thread>
#include <utility>
#include <exception>
#include <stdexcept>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_system.h>

#include <fmt/format.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/dirty.hpp>
#include <p5/rswc/implementation_/common.hpp>
#include <p5/rswc/implementation_/platform/logged_action.hpp>
#include <p5/rswc/implementation_/platform/modules/event_loop.hpp>

#include "main_task.hpp"

#include <p5/rswc/implementation.hpp>


namespace p5::rswc {
namespace implementation_ {

    inline static auto invoke() noexcept(false) {
        if constexpr (true) {
            static auto protector_ = false;
            if (protector_) throw ::std::logic_error{::fmt::format("repeated call of {}", __PRETTY_FUNCTION__)};
            protector_ = true;
        }

        auto task_ = [] () {
            auto &&task_ = [] () -> common::coro::Task<bool> {
                platform::logged_action::execute("initializing event loop", platform::modules::event_loop::initialize);
                auto result_ = false;
                try { co_await platform::logged_action::wrap_awaitable("main task", main_task::make()); result_ = true; }
                catch (...) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                    log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                }); }
                co_return result_;
            } ();
            return ::std::make_shared<::std::decay_t<decltype(task_)>>(::std::forward<decltype(task_)>(task_));
        } ();

        task_->subscribe([task_] () {
            try {
                auto mutex_ = ::std::make_shared<::std::mutex>();
                [[maybe_unused]] auto const lock_ = ::std::unique_lock<::std::decay_t<decltype(*mutex_)>>{*mutex_};
                ::std::thread{[mutex_, task_] () {
                    try {
                        ::std::unique_lock<::std::decay_t<decltype(*mutex_)>>{*mutex_};
                        platform::logged_action::execute("deinitializing event loop", platform::modules::event_loop::deinitialize);
                        if (! task_->result()) throw ::std::runtime_error{"main task failed"};
                        if constexpr (true) {
                            constexpr static auto const max_ = 3;
                            for (auto counter_ = max_; 0 < counter_; counter_--) {
                                log(
                                    max_ > counter_ ? LogLevel::Debug : LogLevel::Info,
                                    ::fmt::format("restarting in {} seconds...", counter_)
                                );
                                ::vTaskDelay(1000 / portTICK_PERIOD_MS);
                            }
                        }
                        log<LogLevel::Info>("restarting now...");
                        ::esp_restart();
                    }
                    catch (...) { ::std::terminate(); }
                }}.detach();
            }
            catch (...) { ::std::terminate(); }
        });

        task_->start();

        main_task::post_test_event();
    }

    inline static auto make_terminate_handler() noexcept(true) {
        static auto flag_ = false;

        return [] () {
            try {
                if (auto &&exception_ = ::std::current_exception()) {
                    log<LogLevel::Error>(::fmt::format(
                        "terminate caused with error, thread = {}", static_cast<void const *>(::xTaskGetCurrentTaskHandle())
                    ));
                    common::exception_handling::walk(::std::forward<decltype(exception_)>(exception_), [] (auto &&exception) {
                        log<LogLevel::Error>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                    });
                }

                else log<LogLevel::Error>(::fmt::format(
                    "terminate caused with error, thread = {}", static_cast<void const *>(::xTaskGetCurrentTaskHandle())
                ));

                if (flag_) return;
                flag_ = true;

                if constexpr (true) {
                    constexpr static auto const max_ = 6;
                    for (auto counter_ = max_; 0 < counter_; counter_--) {
                        log(
                            max_ > counter_ ? LogLevel::Debug : LogLevel::Info,
                            ::fmt::format("aborting in {} seconds...", counter_)
                        );
                        ::vTaskDelay(1000 / portTICK_PERIOD_MS);
                    }
                }

                log<LogLevel::Info>("aborting now...");
            }

            catch (...) {}

            ESP_ERROR_CHECK(ESP_FAIL);
            ::std::abort();
        };
    }

} // namespace implementation_

    void implementation() noexcept(true) {
        ::std::set_terminate(implementation_::make_terminate_handler());
        try { implementation_::invoke(); } catch(...) { ::std::terminate(); }
    }

} // namespace p5::rswc
