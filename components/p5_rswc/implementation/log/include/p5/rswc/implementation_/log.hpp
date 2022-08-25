#pragma once

#include <string>
#include <utility>


namespace p5::rswc::implementation_ {
namespace log_ {

    enum class Level {
        Error, // ESP_LOG_ERROR: Critical errors, software module can not recover on its own
        Warning, // ESP_LOG_WARN: Error conditions from which recovery measures have been taken
        Info, // ESP_LOG_INFO: Information messages which describe normal flow of events
        Debug, // ESP_LOG_DEBUG: Extra information which is not necessary for normal use (values, pointers, sizes, etc).
        Verbose // ESP_LOG_VERBOSE: Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
    };

    void routine(Level, char const *) noexcept(false);

    inline static auto routine(Level level, ::std::string const &message) noexcept(false) {
        return routine(level, message.c_str());
    }

} // namespace log_

    using LogLevel = log_::Level;

    template <class levelT, class messageT> inline static auto log(levelT &&level, messageT &&message) noexcept(false) {
        return log_::routine(::std::forward<levelT>(level), ::std::forward<messageT>(message));
    }

    template <LogLevel level = log_::Level::Info, class messageT> inline static auto log(messageT &&message) noexcept(false) {
        return log_::routine(level, ::std::forward<messageT>(message));
    }

} // namespace p5::rswc::implementation_
