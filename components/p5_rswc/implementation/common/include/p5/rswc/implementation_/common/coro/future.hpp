#pragma once

#include <utility>
#include <optional>
#include <coroutine>
#include <exception>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <type_traits>

#include <p5/rswc/implementation_/common/notifier.hpp>

#include "task.hpp"
#include "private_.hpp"


namespace p5::rswc::implementation_::common::coro {
namespace future {
namespace private_ {

    ::std::string_view short_module_name() noexcept(true);

} // namespace private_

    template <class T> struct Class final {
        static_assert(::std::is_same_v<T, ::std::decay_t<T>>);

        inline constexpr auto ready() const noexcept(true) { return context_.exception || context_.result.has_value(); }

        inline constexpr auto result() const noexcept(false) requires(::std::is_same_v<T, void>) { return result_(); }
        inline constexpr decltype(auto) result() & noexcept(false) requires(! ::std::is_same_v<T, void>) { return result_(); }
        inline constexpr decltype(auto) result() && noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::move(result_()); }
        inline constexpr decltype(auto) result() const & noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::as_const(result_()); }
        inline constexpr decltype(auto) result() const && noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::move(::std::as_const(result_())); }

        inline constexpr auto exception() const noexcept(true) { return context_.exception; }

        template <class Result> inline constexpr auto set_result() noexcept(false) requires(::std::is_same_v<T, void>) {
            if (context_.exception) throw ::std::runtime_error{"exception already assigned"};
            if (context_.result.has_value()) throw ::std::runtime_error{"result already assigned"};
            context_.result = true;
            auto notifier_ = ::std::move(context_.notifier);
            notifier_.notify();
        }

        template <class Result> inline constexpr auto set_result(Result &&result) noexcept(false) requires(! ::std::is_same_v<T, void>) {
            if (context_.exception) throw ::std::runtime_error{"exception already assigned"};
            if (context_.result.has_value()) throw ::std::runtime_error{"result already assigned"};
            context_.result = ::std::make_optional<T>(::std::forward<Result>(result));
            auto notifier_ = ::std::move(context_.notifier);
            notifier_.notify();
        }

        template <class Exception> inline constexpr auto set_exception(Exception &&exception) noexcept(false) {
            if (context_.exception) throw ::std::runtime_error{"exception already assigned"};
            if (context_.result.has_value()) throw ::std::runtime_error{"result already assigned"};
            if constexpr (::std::is_base_of_v<::std::exception_ptr, ::std::decay_t<Exception>>) context_.exception = ::std::forward<Exception>(exception);
            else {
                static_assert(::std::is_base_of_v<::std::exception, ::std::decay_t<Exception>>);
                context_.exception = ::std::make_exception_ptr(::std::forward<Exception>(exception));
            }
            auto notifier_ = ::std::move(context_.notifier);
            notifier_.notify();
        }

        inline constexpr auto wait() const noexcept(true) { return wait_(); }
        template <class Handler> inline auto & subscribe(Handler &&handler) const noexcept(false) {
            return subscribe_(::std::forward<Handler>(handler));
        }

        inline constexpr operator bool () const noexcept(true) { return ready(); }

        inline constexpr auto operator co_await () noexcept(true) requires(::std::is_same_v<T, void>) { return co_await_(); }
        inline constexpr decltype(auto) operator co_await () & noexcept(true) requires(! ::std::is_same_v<T, void>) { return co_await_<false, false>(); }
        inline constexpr decltype(auto) operator co_await () && noexcept(true) requires(! ::std::is_same_v<T, void>) { return co_await_<false, true>(); }
        inline constexpr decltype(auto) operator co_await () const & noexcept(true) requires(! ::std::is_same_v<T, void>) { return co_await_<true, false>(); }
        inline constexpr decltype(auto) operator co_await () const && noexcept(true) requires(! ::std::is_same_v<T, void>) { return co_await_<true, true>(); }

        constexpr Class() = default;
        constexpr Class(Class &&) = default;
        constexpr Class & operator = (Class &&) = default;

    private:
        struct Context_ final {
            Notifier notifier;
            ::std::exception_ptr exception;
            ::std::optional<::std::conditional_t<::std::is_same_v<T, void>, bool, T>> result;
        } context_;

        inline constexpr auto result_() const noexcept(false) requires(::std::is_same_v<T, void>) {
            if (context_.exception) ::std::rethrow_exception(context_.exception);
            if (! context_.result.has_value()) throw ::std::runtime_error{"future is not ready"};
            if (! *(context_.result)) throw ::std::runtime_error{"internal result is not true"};
        }

        inline constexpr auto & result_() const noexcept(false) requires(! ::std::is_same_v<T, void>) {
            if (context_.exception) ::std::rethrow_exception(context_.exception);
            auto &result_ = const_cast<::std::decay_t<decltype(context_)> &>(context_).result;
            if (! result_.has_value()) throw ::std::runtime_error{"future is not ready"};
            return *(result_);
        }

        inline constexpr auto wait_() const noexcept(true) {
            return [] (auto &&context) -> Task<void> {
                struct Awaitable_ final {
                    inline constexpr auto await_ready() const noexcept(true) { return context_.exception || context_.result.has_value(); }

                    inline auto await_resume() noexcept(false) {
                        if (! await_ready()) throw ::std::logic_error{"not finished"};
                        if (subscription_) subscription_.cancel();
                    }

                    inline auto await_suspend(::std::coroutine_handle<> const &coroutine) noexcept(false) {
                        if (! coroutine) throw ::std::invalid_argument{"empty coroutine"};
                        if (await_ready()) throw ::std::logic_error{"finished already"};
                        if (suspended_) throw ::std::logic_error{"suspended already"};
                        subscription_ = ::std::move(context_.notifier.subscribe(
                            coro::private_::make_coroutine_resumer(coroutine))
                        );
                        suspended_ = true;
                    }

                    inline constexpr explicit Awaitable_(Context_ &context) noexcept(true): context_{context} {}

                private:
                    Awaitable_() = delete;
                    Awaitable_(Awaitable_ &&) = delete;
                    Awaitable_(Awaitable_ const &) = delete;
                    Awaitable_ & operator = (Awaitable_ &&) = delete;
                    Awaitable_ & operator = (Awaitable_ const &) = delete;

                    Context_ &context_;
                    bool suspended_ = false;
                    notifier::Subscription subscription_;
                };
                co_return co_await Awaitable_{::std::forward<decltype(context)>(context)};
            } (const_cast<::std::decay_t<decltype(*this)> *>(this)->context_);
        }

        template <class Handler> inline auto & subscribe_(Handler &&handler) const noexcept(false) {
            auto &context_ = const_cast<::std::decay_t<decltype(*this)> &>(*this).context_;
            if (context_.exception || context_.result) {
                ::std::invoke(coro::private_::wrap_handler(::std::forward<Handler>(handler), private_::short_module_name()));
                thread_local static auto dummy_ = notifier::Subscription{};
                return dummy_;
            }
            return context_.notifier.subscribe(coro::private_::wrap_handler(
                ::std::forward<Handler>(handler), private_::short_module_name()
            ));
        }

        template <bool const_mode = true, bool rvalue_mode = false>
        inline constexpr auto co_await_() const noexcept(true) {
            if constexpr (::std::is_same_v<T, void>) static_assert(const_mode && (! rvalue_mode));

            struct Awaitable_ final {
                inline constexpr auto await_ready() const noexcept(true) {
                    return context_.exception || context_.result.has_value();
                }

                inline constexpr decltype(auto) await_resume() noexcept(false) {
                    if (! await_ready()) throw ::std::logic_error{"not finished"};
                    if (subscription_) subscription_.cancel();
                    auto &value_ = [&context_ = this->context_] () -> decltype(auto) {
                        if (context_.exception) ::std::rethrow_exception(context_.exception);
                        if (! context_.result.has_value()) throw ::std::logic_error{"result or exception is not assigned"};
                        if constexpr (const_mode) return ::std::as_const(*(context_.result)); else return *(context_.result);
                    } ();
                    if constexpr (::std::is_same_v<T, void>) {
                        if (! value_) throw ::std::logic_error{"internal result is not true"};
                    }
                    else if constexpr (rvalue_mode) return ::std::move(value_);
                    else return value_;
                }

                inline auto await_suspend(::std::coroutine_handle<> const &coroutine) noexcept(false) {
                    if (! coroutine) throw ::std::invalid_argument{"empty coroutine"};
                    if (await_ready()) throw ::std::logic_error{"finished already"};
                    if (suspended_) throw ::std::logic_error{"suspended already"};
                    subscription_ = ::std::move(context_.notifier.subscribe(
                        coro::private_::make_coroutine_resumer(coroutine))
                    );
                    suspended_ = true;
                }

                inline constexpr explicit Awaitable_(Context_ &context) noexcept(true): context_{context} {}

            private:
                Awaitable_() = delete;
                Awaitable_(Awaitable_ &&) = delete;
                Awaitable_(Awaitable_ const &) = delete;
                Awaitable_ & operator = (Awaitable_ &&) = delete;
                Awaitable_ & operator = (Awaitable_ const &) = delete;

                Context_ &context_;
                bool suspended_ = false;
                notifier::Subscription subscription_;
            };

            return Awaitable_{const_cast<::std::decay_t<decltype(*this)> *>(this)->context_};
        }

        Class(Class const &) noexcept(true) = delete;
        Class & operator = (Class const &) noexcept(true) = delete;
    };

} // namespace future

    template <class T = void> using Future = future::Class<T>;

} // p5::rswc::implementation_::common::coro
