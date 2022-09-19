#include <utility>
#include <exception>
#include <stdexcept>

#include <fmt/format.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/platform/logged_action.hpp>


namespace p5::rswc::implementation_::platform::logged_action {
namespace private_ {

    void Helper::throw_exception() noexcept(false) {
        success_ = false;
        auto &&exception_ = ::std::runtime_error{::fmt::format("action failed: {}", name_)};
        if (::std::current_exception()) ::std::throw_with_nested(::std::forward<decltype(exception_)>(exception_));
        throw ::std::forward<decltype(exception_)>(exception_);
    }

    void Helper::handle_ctor_() noexcept(true) {
        try { log<LogLevel::Verbose>(::fmt::format("attempt to perform an action: {}", name_)); } catch (...) {}
    }

    void Helper::handle_dtor_() noexcept(true) {
        try { if (success_) log<LogLevel::Info>(::fmt::format("action completed successfully: {}", name_)); } catch(...) {}
    }

} // namespace private_
} // namespace p5::rswc::implementation_::platform::logged_action
