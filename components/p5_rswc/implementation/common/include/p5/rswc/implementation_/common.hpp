#pragma once

#include <string>
#include <typeinfo>
#include <exception>
#include <functional>


namespace p5::rswc::implementation_::common {
namespace exception_handling {

    void walk(::std::exception const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);
    void walk(::std::exception_ptr const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);

    ::std::string details(::std::exception const &) noexcept(false);
    ::std::string details(::std::exception_ptr const &) noexcept(false);

} // namespace exception_handling

namespace utils {

    ::std::string demangle(::std::type_info const &) noexcept(false);

    template <class ... T> inline constexpr static auto unused(T && ...) noexcept(true) {}

} // namespace utils
} // namespace p5::rswc::implementation_::common
