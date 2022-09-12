#include <utility>
#include <exception>
#include <functional>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/common/coro.hpp>


namespace p5::rswc::implementation_::logged_action {
namespace private_ {

    struct Helper final {
        inline auto throw_exception() noexcept(false) {
            success_ = false;
            auto &&exception_ = ::std::runtime_error{::fmt::format("action failed: {}", name_)};
            if (::std::current_exception()) ::std::throw_with_nested(::std::forward<decltype(exception_)>(exception_));
            throw ::std::forward<decltype(exception_)>(exception_);
        }

        template <class Name> requires(! ::std::is_base_of_v<Helper, ::std::decay_t<Name>>)
        inline explicit Helper(Name &&name) noexcept(true): name_{::std::forward<Name>(name)} {
            try { log<LogLevel::Verbose>(::fmt::format("attempt to perform an action: {}", name)); } catch (...) {}
        }

        inline ~Helper() noexcept(true) {
            try { if (success_) log<LogLevel::Info>(::fmt::format("action completed successfully: {}", name_)); } catch(...) {}
        }

    private:
        bool success_ = true;
        ::std::string_view const name_;

        Helper() = delete;
        Helper(Helper &&) = delete;
        Helper(Helper const &) = delete;
        Helper & operator = (Helper &&) = delete;
        Helper & operator = (Helper const &) = delete;
    };

} // namespace private_

    template <class Name, class Action> inline static decltype(auto) execute(
        Name &&name, Action &&action
    ) noexcept(false) {
        auto helper_ = private_::Helper{::std::forward<Name>(name)};
        try { return ::std::invoke(::std::forward<Action>(action)); }
        catch(...) { helper_.throw_exception(); }
    }

    template <class Name, class Action> inline static common::coro::Task<
        ::std::decay_t<decltype(::std::declval<Action>().operator co_await ().await_resume())>
    > wrap_awaitable(Name &&name, Action &&action) noexcept(false) {
        auto helper_ = private_::Helper{::std::forward<Name>(name)};
        try { co_return co_await ::std::forward<Action>(action); }
        catch(...) { helper_.throw_exception(); }
    }

} // namespace p5::rswc::implementation_::logged_action
