#pragma once

#include <utility>
#include <string>
#include <string_view>


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

    template <class Level> inline static auto routine(Level &&level, ::std::string const &message) noexcept(false) {
        return log_::routine(::std::forward<Level>(level), message.c_str());
    }

    template <class Level> inline static auto routine(Level &&level, ::std::string_view const &message) noexcept(false) {
        return log_::routine(::std::forward<Level>(level), ::std::string{message});
    }

} // namespace log_

    using LogLevel = log_::Level;

    template <class Level, class Message> inline static auto log(Level &&level, Message &&message) noexcept(false) {
        return log_::routine(::std::forward<Level>(level), ::std::forward<Message>(message));
    }

    template <LogLevel level = log_::Level::Info, class Message> inline static auto log(Message &&message) noexcept(false) {
        return log_::routine(level, ::std::forward<Message>(message));
    }

} // namespace p5::rswc::implementation_
