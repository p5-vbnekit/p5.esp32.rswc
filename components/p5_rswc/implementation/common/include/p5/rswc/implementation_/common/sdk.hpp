#pragma once

#include <string_view>
#include <type_traits>


namespace p5::rswc::implementation_::common::sdk {

    using ErrorCode = int;

namespace private_ {

    ::std::string_view error_name(ErrorCode) noexcept(true);
    ErrorCode check_or_throw(ErrorCode) noexcept(false);

} // namespace private_

    template <class T> inline static auto error_name(T &&error_code) noexcept(true) {
        static_assert(::std::is_same_v<::std::decay_t<T>, ErrorCode>);
        return private_::error_name(::std::forward<T>(error_code));
    }

    template <class T> inline static auto check_or_throw(T &&error_code) noexcept(false) {
        static_assert(::std::is_same_v<::std::decay_t<T>, ErrorCode>);
        return private_::check_or_throw(::std::forward<T>(error_code));
    }

} // namespace p5::rswc::implementation_::common::sdk
