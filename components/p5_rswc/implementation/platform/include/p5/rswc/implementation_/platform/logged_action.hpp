#include <utility>
#include <exception>
#include <functional>
#include <string_view>
#include <type_traits>

#include <p5/rswc/implementation_/common/coro.hpp>


namespace p5::rswc::implementation_::platform::logged_action {
namespace private_ {

    struct Helper final {
        void throw_exception() noexcept(false);

        template <class Name> requires(! ::std::is_base_of_v<Helper, ::std::decay_t<Name>>)
        inline explicit Helper(Name &&name) noexcept(true): name_{::std::forward<Name>(name)} { handle_ctor_(); }
        inline ~Helper() noexcept(true) { handle_dtor_(); }

    private:
        bool success_ = true;
        ::std::string_view const name_;

        void handle_ctor_() noexcept(true);
        void handle_dtor_() noexcept(true);

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
        catch(...) { helper_.throw_exception(); throw; }
    }

    template <class Name, class Action> inline static common::coro::Task<
        ::std::decay_t<decltype(::std::declval<Action>().operator co_await ().await_resume())>
    > wrap_awaitable(Name &&name, Action &&action) noexcept(false) {
        auto helper_ = private_::Helper{::std::forward<Name>(name)};
        try { co_return co_await ::std::forward<Action>(action); }
        catch(...) { helper_.throw_exception(); throw; }
    }

} // namespace p5::rswc::implementation_::platform::logged_action
