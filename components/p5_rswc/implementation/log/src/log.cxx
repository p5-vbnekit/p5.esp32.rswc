#include <stdexcept>

#include <esp_log.h>

#include <p5/rswc/implementation_/log.hpp>


namespace p5::rswc::implementation_::log_ {

    inline constexpr static auto const to_idf_(Level source) noexcept(true) {
        switch (source) {
            default: break;
            case Level::Error: return ::ESP_LOG_ERROR;
            case Level::Warning: return ::ESP_LOG_WARN;
            case Level::Info: return ::ESP_LOG_INFO;
            case Level::Debug: return ::ESP_LOG_DEBUG;
            case Level::Verbose: return ::ESP_LOG_VERBOSE;
        }
        return ::ESP_LOG_NONE;
    }

    void routine(Level level, char const *message) noexcept(false) {
        if (! static_cast<bool>(message)) throw ::std::invalid_argument{"empty message pointer"};
        auto const idf_level_ = to_idf_(level);
        if (::ESP_LOG_NONE == idf_level_) throw ::std::invalid_argument{"unknown level"};
        ESP_LOG_LEVEL(idf_level_, "p5_rswc", "%s", message);
    }

} // namespace p5::rswc::implementation_::log_
