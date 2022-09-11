#include <memory>
#include <string>
#include <utility>
#include <exception>
#include <stdexcept>
#include <string_view>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_system.h>

#include <fmt/format.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/dirty.hpp>
#include <p5/rswc/implementation_/common.hpp>

#include "main_task.hpp"

#include <p5/rswc/implementation.hpp>


namespace p5::rswc {
namespace implementation_ {

    inline static auto invoke() noexcept(false) {
        static auto task_ = main_task::make();

        task_.subscribe([] () {
            try {
                log<LogLevel::Info>("main task finished");

                try { task_.result(); }
                catch (...) { ::std::throw_with_nested(::std::runtime_error{"main task failed"}); }

                for (auto counter_ = 3; 0 < counter_; counter_--) {
                    log<LogLevel::Verbose>(::fmt::format("Restarting in {} seconds...", counter_));
                    ::vTaskDelay(1000 / portTICK_PERIOD_MS);
                }

                log<LogLevel::Debug>("Restarting now ...");
                ::esp_restart();
            }
            
            catch (...) { ::std::terminate(); }
        });

        task_.start();

        main_task::post_test_event();
    }

    inline static auto terminate_handler() noexcept(true) {
        try {
            if (auto &&exception_ = ::std::current_exception()) {
                log<LogLevel::Error>("terminating on error");
                common::exception_handling::walk(::std::forward<decltype(exception_)>(exception_), [] (auto &&exception) {
                    log<LogLevel::Error>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                });
            }

            else log<LogLevel::Error>("terminating on unknown error");

            log<LogLevel::Verbose>("Aborting in 6 seconds...");
            ::vTaskDelay(3000 / portTICK_PERIOD_MS);
            log<LogLevel::Debug>("Aborting now...");
        } catch (...) {}

        ESP_ERROR_CHECK(ESP_FAIL);
        ::std::abort();
    }

} // namespace implementation_

    void implementation() noexcept(true) {
        static auto protector_ = false;
        if (protector_) try { throw ::std::logic_error{::fmt::format("repeated call of {}", __PRETTY_FUNCTION__)}; }
        catch (...) { implementation_::terminate_handler(); }
        protector_ = true;
        ::std::set_terminate(implementation_::terminate_handler);
        try { implementation_::invoke(); }
        catch(...) { ::std::terminate(); }
    }

} // namespace p5::rswc
