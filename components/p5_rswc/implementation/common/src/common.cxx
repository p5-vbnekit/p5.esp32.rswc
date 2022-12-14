#include <cstdlib>

#include <string>
#include <utility>
#include <typeinfo>
#include <exception>
#include <functional>
#include <string_view>

#include <cxxabi.h>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common.hpp>


namespace p5::rswc::implementation_::common {
namespace exception_handling {

    void walk(::std::exception const &exception, ::std::function<void(::std::exception_ptr const &)> const &delegate) noexcept(false) {
        walk(::std::make_exception_ptr(exception), delegate);
    }

    void walk(::std::exception_ptr const &exception, ::std::function<void(::std::exception_ptr const &)> const &delegate) noexcept(false) {
        delegate(exception);
        if (! static_cast<bool>(exception)) return;
        try { ::std::rethrow_exception(exception); }
        catch(::std::exception const &exception) {
            try { ::std::rethrow_if_nested(exception); }
            catch(...) { walk(::std::current_exception(), delegate); }
        }
        catch(...) {}
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

} // namespace exception_handling

namespace utils {

    ::std::string demangle(::std::type_info const &info) noexcept(false) {
        int status;
        auto &&text_ = ::std::string{};
        auto const * const name_ = info.name();
        try {
            if (! static_cast<bool>(name_)) throw ::std::invalid_argument{"invalid type_info (null name)"};
            auto * const pointer_ = ::abi::__cxa_demangle(name_, 0, 0, &status);
            if (! static_cast<bool>(pointer_)) throw ::std::runtime_error{"null pointer returned"};
            try {
                text_ = pointer_;
                if (text_.empty()) throw ::std::runtime_error{"empty name returned"};
            }
            catch(...) { ::free(pointer_); throw; }
            ::free(pointer_);
        }
        catch(...) { ::std::throw_with_nested(::std::runtime_error{::fmt::format("failed to demangle: {}", name_)}); }
        return ::std::move(text_);
    }

} // namespace utils
} // namespace p5::rswc::implementation_::common
