#pragma once

#include <string>
#include <exception>
#include <functional>


namespace p5::rswc::implementation_::common::exception_handling {

    void walk(::std::exception const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);
    void walk(::std::exception_ptr const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);

    ::std::string details(::std::exception const &) noexcept(false);
    ::std::string details(::std::exception_ptr const &) noexcept(false);

} // namespace p5::rswc::implementation_::common::exception_handling
