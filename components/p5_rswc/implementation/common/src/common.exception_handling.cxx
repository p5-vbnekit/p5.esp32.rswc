#include <string>
#include <exception>
#include <string_view>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common/utils.hpp>
#include <p5/rswc/implementation_/common/exception_handling.hpp>


namespace p5::rswc::implementation_::common::exception_handling {

    void walk(::std::exception const &exception, ::std::function<void(::std::exception_ptr const &)> const &delegate) noexcept(false) {
        walk(::std::make_exception_ptr(exception), delegate);
    }

    void walk(::std::exception_ptr const &exception, ::std::function<void(::std::exception_ptr const &)> const &delegate) noexcept(false) {
        if (! static_cast<bool>(exception)) return;
        try { ::std::rethrow_exception(exception); }
        catch(::std::exception const &exception) {
            try { ::std::rethrow_if_nested(exception); }
            catch(...) { walk(::std::current_exception(), delegate); }
        }
        catch(...) {}
        delegate(exception);
    }

    ::std::string details(::std::exception const &exception) noexcept(false) {
        auto const &&type_ = utils::demangle(typeid(exception));
        auto const &&what_ = ::std::string_view{exception.what()};
        if (what_.empty()) return ::std::move(type_);
        return ::fmt::format("{}: {}", ::std::move(type_), ::std::move(what_));
    }

    ::std::string details(::std::exception_ptr const &exception) noexcept(false) {
        if (! exception) return "empty exception pointer";
        try { ::std::rethrow_exception(exception); }
        catch(::std::exception const &exception_) { return details(exception_); }
        catch(...) {}
        return "unknown exception";
    }

} // namespace p5::rswc::implementation_::common
