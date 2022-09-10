#pragma once

#include <string>
#include <utility>
#include <typeinfo>
#include <exception>
#include <functional>
#include <forward_list>


namespace p5::rswc::implementation_::common::utils {

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

} // namespace p5::rswc::implementation_::common::utils
