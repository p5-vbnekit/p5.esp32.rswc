#include <utility>
#include <exception>

#include <fmt/core.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_system.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/romfs.hpp>
#include <p5/rswc/implementation_/common.hpp>

#include <p5/rswc/implementation.hpp>


namespace p5::rswc {
namespace implementation_ {

    inline static auto routine() noexcept(false) {
        RomFs const romfs_;
        log(::fmt::format("hello, world! {} = {}", 42, 42));
        log(::fmt::format("romfs.image/ssl/public.pem: {}", romfs_("ssl/public.pem")));
        log(::fmt::format("romfs.image/ssl/private.pem: {}", romfs_("ssl/private.pem")));
    }

} // namespace implementation_

    void implementation() noexcept(true) {
        using namespace implementation_;

        try {
            try { routine(); }
            catch(...) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) { log<LogLevel::Error>(
                ::fmt::format("caught unhandled {}", common::exception_handling::details(::std::forward<decltype(exception)>(exception)))
            ); }); }
        }

        catch(...) {
            ESP_ERROR_CHECK(ESP_FAIL);
            ::std::terminate();
        }

        for (auto counter_ = 10; 0 < counter_; counter_--) {
            log<LogLevel::Verbose>(::fmt::format("Restarting in {} seconds...", counter_));
            ::vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        log<LogLevel::Debug>("Restarting now ...");
        ::esp_restart();
    }

} // namespace p5::rswc
