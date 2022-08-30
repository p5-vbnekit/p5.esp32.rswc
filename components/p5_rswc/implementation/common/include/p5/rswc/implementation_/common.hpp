#pragma once

#include <string>
#include <utility>
#include <typeinfo>
#include <exception>
#include <functional>
#include <string_view>
#include <type_traits>
#include <forward_list>


namespace p5::rswc::implementation_::common {
namespace sdk {

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

} // namespace sdk

namespace exception_handling {

    void walk(::std::exception const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);
    void walk(::std::exception_ptr const &, ::std::function<void(::std::exception_ptr const &)> const &) noexcept(false);

    ::std::string details(::std::exception const &) noexcept(false);
    ::std::string details(::std::exception_ptr const &) noexcept(false);

} // namespace exception_handling

namespace utils {

    template <class ... T> inline constexpr static auto unused(T && ...) noexcept(true) {}

    ::std::string demangle(::std::type_info const &) noexcept(false);

    template <class Action, class ExceptionHandler> inline static
    auto with_finally(Action &&action, ExceptionHandler &&exception_handler) noexcept(false) {
        return ::std::forward<Action>(action)([helper_ = [&exception_handler] () {
            struct Helper_ final {
                ::std::forward_list<::std::function<void(void)>> actions = {};
                ::std::function<void(::std::exception_ptr const &)> exception_handler;
                inline ~Helper_() { for (auto const &action: ::std::move(actions)) { try { action(); } catch (...) {
                    if (! static_cast<bool>(exception_handler)) continue;
                    try { exception_handler(::std::current_exception()); } catch (...) {}
                } } }
            };
            return Helper_{.exception_handler = ::std::forward<ExceptionHandler>(exception_handler)};
        } ()] (auto &&final_action) mutable {
            helper_.actions.push_front(::std::forward<decltype(final_action)>(final_action));
        });
    }

} // namespace utils
} // namespace p5::rswc::implementation_::common
