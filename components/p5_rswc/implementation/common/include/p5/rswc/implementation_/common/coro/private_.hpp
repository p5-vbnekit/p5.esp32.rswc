#pragma once

#include <utility>
#include <coroutine>
#include <exception>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <string_view>


namespace p5::rswc::implementation_::common::coro::private_ {

    using Handler = ::std::function<void(void)>;
    using ExceptionHandler = ::std::function<void(::std::exception_ptr const &, ::std::string_view const &)>;

    void handle_exception(::std::string_view const & = {}) noexcept(true);
    void handle_exception(::std::exception_ptr const &, ::std::string_view const & = {}) noexcept(true);

    void notify_handler(Handler const &, ::std::string_view const & = {}) noexcept(true);
    template <class Handler, class Location> static auto wrap_handler(Handler &&, Location &&) noexcept(false);

    template <class Handler, class Location> static auto wrap_handler(Handler &&, Location &&) noexcept(false);

    void register_coroutine(::std::coroutine_handle<> const &) noexcept(true);
    void unregister_coroutine(::std::coroutine_handle<> const &) noexcept(true);
    Handler make_coroutine_resumer(::std::coroutine_handle<> const &) noexcept(false);


    template <class Handler, class Location> inline static auto wrap_handler(
        Handler &&handler, Location &&location
    ) noexcept(false) {
        return [
            handler_ = [&handler] () {
                auto &&handler_ = [&handler] () -> decltype(auto) {
                    using Result_ = private_::Handler;
                    if constexpr (::std::is_same_v<Result_, ::std::decay_t<Handler>>) return ::std::forward<Handler>(handler);
                    return Result_{::std::forward<Handler>(handler)};
                } ();
                if (! static_cast<bool>(handler_)) throw ::std::invalid_argument{"empty handler"};
                return ::std::forward<decltype(handler_)>(handler_);
            } (),
            location_ = ::std::forward<Location>(location)
        ] () { notify_handler(
            ::std::forward<decltype(handler_)>(handler_),
            ::std::forward<decltype(location_)>(location_)
        ); };
    }

    void set_exception_handler(ExceptionHandler &&) noexcept(false);
    void set_exception_handler(ExceptionHandler const &) noexcept(false);

} // p5::rswc::implementation_::common::coro::private_
