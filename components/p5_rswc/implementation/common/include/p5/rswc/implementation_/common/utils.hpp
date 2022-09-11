#pragma once

#include <string>
#include <utility>
#include <typeinfo>
#include <exception>
#include <functional>
#include <string_view>
#include <forward_list>


namespace p5::rswc::implementation_::common::utils {

    ::std::string demangle(::std::type_info const &) noexcept(false);

    template <class Action, class ExceptionHandler> inline static
    auto with_finally(Action &&action, ExceptionHandler &&exception_handler) noexcept(false) {
        return ::std::invoke(::std::forward<Action>(action), [helper_ = [&exception_handler] () {
            struct Helper_ final {
                using Actions = ::std::forward_list<::std::function<void(void)>>;
                using Handler = ::std::function<void(::std::exception_ptr const &)>;

                Actions actions;
                Handler handler;

                inline ~Helper_() {
                    auto const actions_ = ::std::move(actions);
                    for (auto const &action: actions_) { try { action(); } catch (...) {
                        if (! static_cast<bool>(handler)) continue;
                        try { handler(::std::current_exception()); } catch (...) { ::std::terminate(); }
                    } }
                }

                inline Helper_(Handler &&handler) noexcept(true): handler{::std::move(handler)} {}
                inline Helper_(Handler const &handler)  noexcept(true): handler{handler} {}

                Helper_(Helper_ &&) = default;
                Helper_ & operator = (Helper_ &&) = default;

            private:
                Helper_() = delete;
                Helper_(Helper_ const &) = delete;
                Helper_ & operator = (Helper_ const &) = delete;
            };
            return Helper_{::std::forward<ExceptionHandler>(exception_handler)};
        } ()] (auto &&final_action) mutable {
            helper_.actions.push_front(::std::forward<decltype(final_action)>(final_action));
        });
    }

namespace trimmer {

    inline constexpr auto const default_spaces = ::std::string_view{" \t\n\f\r\v"};

    inline constexpr static auto const left(
        ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
    ) noexcept(true) {
        if (source.empty() || spaces.empty()) return source;
        auto const offset_ = source.find_first_not_of(spaces);
        if (offset_ == source.npos) return ::std::string_view{source.data(), 0};
        return ::std::string_view{source.begin() + offset_, source.end()};
    }

    inline constexpr static auto const right(
        ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
    ) noexcept(true) {
        if (source.empty() || spaces.empty()) return source;
        auto const offset_ = source.find_last_not_of(spaces);
        if (offset_ == source.npos) return ::std::string_view{source.data(), 0};
        return ::std::string_view{source.begin(), 1 + offset_};
    }

    inline constexpr static auto const both(
        ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
    ) noexcept(true) {
        return right(left(source), spaces);
    }

} // namespace trimmer
} // namespace p5::rswc::implementation_::common::utils
