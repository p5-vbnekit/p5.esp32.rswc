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

#include "private_.hpp"


namespace p5::rswc::implementation_::common::coro {
namespace task {
namespace private_ {
namespace promise {
namespace context {

    template <class T> struct Class final {
        bool started = false;
        Notifier notifier;
        ::std::optional<T> result;
        ::std::exception_ptr exception;
    };

} // namespace context

    template <class T> using Context = context::Class<T>;

namespace skeleton {

    template <class T> struct Class {
        Context<T> context_;

        template <class Result> inline auto return_value(Result &&value) noexcept(false) {
            auto &context_ = this->context_;
            if (context_.result) throw ::std::runtime_error{"result already assigned"};
            if (context_.exception) throw ::std::runtime_error{"exception already assigned"};
            context_.result = ::std::forward<Result>(value);
        }
    };

    template <> struct Class<void> {
        Context<bool> context_;

        inline auto return_void() noexcept(false) {
            auto &context_ = this->context_;
            if (context_.result) throw ::std::runtime_error{"result already assigned"};
            if (context_.exception) throw ::std::runtime_error{"exception already assigned"};
            context_.result = true;
        }
    };

} // namespace skeleton

    template <class T> using Skeleton = skeleton::Class<T>;

} // namespace promise

    ::std::string_view short_module_name() noexcept(true);

} // namespace private_

    template <class T = void> struct Class final {
        static_assert(::std::is_same_v<T, ::std::decay_t<T>>);

        struct promise_type final: private_::promise::Skeleton<T> {
            inline auto get_return_object() noexcept(false) { return Class{
                ::std::coroutine_handle<::std::decay_t<decltype(*this)>>::from_promise(*this)
            }; }

            inline auto unhandled_exception() noexcept(true) {
                this->context_.exception = ::std::current_exception();
            }

            inline constexpr static auto initial_suspend() noexcept(true) { return ::std::suspend_always{}; }

            inline constexpr static auto final_suspend() noexcept(true) {
                using Coroutine_ = ::std::coroutine_handle<promise_type>;

                struct Awaitable_ final {
                    inline constexpr static auto await_ready() noexcept(true) { return false; }
                    inline constexpr static auto await_resume() noexcept(true) {}

                    inline static auto await_suspend(Coroutine_ const &coroutine) noexcept(true) {
                        auto const notifier_ = ::std::move(coroutine.promise().context_.notifier);
                        try { notifier_.notify(); } catch (...) { ::std::terminate(); }
                    }

                    constexpr Awaitable_() noexcept(true) = default;

                private:
                    Awaitable_(Awaitable_ &&) = delete;
                    Awaitable_(Awaitable_ const &) = delete;
                    Awaitable_ & operator = (Awaitable_ &&) = delete;
                    Awaitable_ & operator = (Awaitable_ const &) = delete;
                };

                return Awaitable_{};
            }
        };

        inline constexpr auto const valid() const { return static_cast<bool>(coroutine_); }

        inline auto done() const noexcept(true) { return coroutine_ && coroutine_.done(); }
        inline auto started() const noexcept(true) { return coroutine_ && coroutine_.promise().context_.started; }

        inline auto result() const noexcept(false) requires(::std::is_same_v<T, void>) { return result_(); }
        inline decltype(auto) result() & noexcept(false) requires(! ::std::is_same_v<T, void>) { return result_(); }
        inline decltype(auto) result() && noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::move(result_()); }
        inline decltype(auto) result() const & noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::as_const(result_()); }
        inline decltype(auto) result() const && noexcept(false) requires(! ::std::is_same_v<T, void>) { return ::std::move(::std::as_const(result_())); }

        inline auto exception() const noexcept(true) {
            if (! coroutine_) return ::std::decay_t<decltype(coroutine_.promise().context_.exception)>{};
            return coroutine_.promise().context_.exception;
        }

        inline auto start() noexcept(false) { return start_(); }

        inline auto wait() const noexcept(false) { return wait_(); }

        template <class Handler> inline auto & subscribe(Handler &&handler) const noexcept(false) {
            return subscribe_(::std::forward<Handler>(handler));
        }

        inline operator bool () const noexcept(true) { return done(); }
        
        inline auto operator co_await () noexcept(false) requires(::std::is_same_v<T, void>) {
            return co_await_<::std::is_const_v<::std::remove_reference_t<decltype(*this)>>>();
        }

        inline decltype(auto) operator co_await () & noexcept(false) requires(! ::std::is_same_v<T, void>) { return co_await_<false, false>(); }
        inline decltype(auto) operator co_await () && noexcept(false) requires(! ::std::is_same_v<T, void>) { return co_await_<false, true>(); }
        inline decltype(auto) operator co_await () const & noexcept(false) requires(! ::std::is_same_v<T, void>) { return co_await_<true, false>(); }
        inline decltype(auto) operator co_await () const && noexcept(false) requires(! ::std::is_same_v<T, void>) { return co_await_<true, true>(); }

        inline Class(Class &&other) noexcept(true): coroutine_{::std::exchange(other.coroutine_, nullptr)} {}

        Class & operator = (Class &&other) noexcept(true) {
            auto &&coroutine_ = ::std::exchange(this->coroutine_, ::std::exchange(other.coroutine_, nullptr));
            if (coroutine_) {
                coro::private_::unregister_coroutine(coroutine_);
                coroutine_.destroy();
            }
        }

        inline ~Class() {
            if (coroutine_) {
                coro::private_::unregister_coroutine(coroutine_);
                coroutine_.destroy();
            }
        }

    private:
        ::std::coroutine_handle<promise_type> coroutine_;

        inline auto & context_() const noexcept(false) {
            if (! coroutine_) throw ::std::runtime_error{"no coroutine assigned"};
            return coroutine_.promise().context_;
        }

        inline auto result_() const noexcept(false) requires(::std::is_same_v<T, void>) {
            auto &context_ = this->context_();
            if (! coroutine_.done()) throw ::std::runtime_error{"task is not finished"};
            if (context_.exception) ::std::rethrow_exception(context_.exception);
            if (! context_.result.has_value()) throw ::std::runtime_error{"result or exception is not assigned"};
            if (! *(context_.result)) throw ::std::runtime_error{"internal result is not true"};
        }

        inline auto & result_() const noexcept(false) requires(! ::std::is_same_v<T, void>) {
            auto &context_ = this->context_();
            if (! coroutine_.done()) throw ::std::runtime_error{"task is not finished"};
            if (context_.exception) ::std::rethrow_exception(context_.exception);
            if (! context_.result.has_value()) throw ::std::runtime_error{"result or exception is not assigned"};
            return *(context_.result);
        }

        inline auto start_() noexcept(false) {
            auto &context_ = this->context_();
            if (context_.started) throw ::std::runtime_error{"task already started"};
            if (coroutine_.done()) throw ::std::logic_error{"task coroutine finished unexpectedly"};
            context_.started = true;
            coroutine_.resume();
        }

        inline auto wait_() const noexcept(false) {
            if (! coroutine_) throw ::std::runtime_error{"no coroutine assigned"};

            return [] (auto &&coroutine) -> Class<void> {
                using Coroutine_ = ::std::decay_t<decltype(coroutine)>;

                struct Awaitable_ final {
                    inline auto await_ready() const noexcept(true) { return coroutine_.done(); }

                    inline auto await_resume() noexcept(false) {
                        if (! await_ready()) throw ::std::logic_error{"not finished"};
                        if (subscription_) subscription_.cancel();
                    }

                    inline auto await_suspend(::std::coroutine_handle<> const &coroutine) noexcept(false) {
                        if (! coroutine) throw ::std::invalid_argument{"empty coroutine"};
                        if (await_ready()) throw ::std::logic_error{"finished already"};
                        if (suspended_) throw ::std::logic_error{"suspended already"};
                        subscription_ = ::std::move(coroutine_.promise().context_.notifier.subscribe(
                            coro::private_::make_coroutine_resumer(coroutine))
                        );
                        suspended_ = true;
                    }

                    inline explicit Awaitable_(Coroutine_ const &coroutine) noexcept(true): coroutine_{coroutine} {}

                private:
                    Awaitable_() = delete;
                    Awaitable_(Awaitable_ &&) = delete;
                    Awaitable_(Awaitable_ const &) = delete;
                    Awaitable_ & operator = (Awaitable_ &&) = delete;
                    Awaitable_ & operator = (Awaitable_ const &) = delete;

                    Coroutine_ coroutine_;
                    bool suspended_ = false;
                    notifier::Subscription subscription_;
                };

                co_return co_await Awaitable_{::std::forward<decltype(coroutine)>(coroutine)};
            } (coroutine_);
        }

        template <class Handler> inline auto & subscribe_(Handler &&handler) const noexcept(false) {
            auto &context_ = this->context_();
            if (coroutine_.done()) {
                if (! context_.started) throw ::std::runtime_error{"task is finished, but not started"};
                ::std::invoke(coro::private_::wrap_handler(::std::forward<Handler>(handler), private_::short_module_name()));
                thread_local static auto dummy_ = notifier::Subscription{};
                return dummy_;
            }
            return context_.notifier.subscribe(coro::private_::wrap_handler(
                ::std::forward<Handler>(handler), private_::short_module_name()
            ));
        }

        template <bool const_mode, bool rvalue_mode = false>
        inline auto co_await_() const noexcept(false) {
            if constexpr (::std::is_same_v<T, void>) static_assert(! rvalue_mode);
            if (! coroutine_) throw ::std::runtime_error{"no coroutine assigned"};
            using Coroutine_ = ::std::decay_t<decltype(coroutine_)>;

            struct Awaitable_ final {
                inline constexpr auto await_ready() const noexcept(true) { return coroutine_.done(); }

                inline decltype(auto) await_resume() noexcept(false) {
                    if (! await_ready()) throw ::std::logic_error{"not finished"};
                    if (subscription_) subscription_.cancel();
                    auto &context_ = coroutine_.promise().context_;
                    auto &value_ = [&context_] () -> decltype(auto) {
                        if (context_.exception) ::std::rethrow_exception(context_.exception);
                        if (! context_.result.has_value()) ::std::logic_error{"result or exception is not assigned"};
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
                    auto &context_ = coroutine_.promise().context_;
                    subscription_ = ::std::move(context_.notifier.subscribe(coro::private_::make_coroutine_resumer(coroutine)));
                    suspended_ = true;
                    using Result_ = ::std::decay_t<decltype(coroutine)>;
                    if constexpr (! const_mode) {
                        if (! context_.started) {
                            context_.started = true;
                            return static_cast<Result_>(coroutine_);
                        }
                    }
                    return static_cast<Result_>(::std::noop_coroutine());
                }

                inline constexpr explicit Awaitable_(Coroutine_ const &coroutine) noexcept(true): coroutine_{coroutine} {}

            private:
                Coroutine_ coroutine_;
                bool suspended_ = false;
                notifier::Subscription subscription_;

                Awaitable_() = delete;
                Awaitable_(Awaitable_ &&) = delete;
                Awaitable_(Awaitable_ const &) = delete;
                Awaitable_ & operator = (Awaitable_ &&) = delete;
                Awaitable_ & operator = (Awaitable_ const &) = delete;
            };

            return Awaitable_{coroutine_};
        }

        Class() = delete;
        Class(Class const &) = delete;
        Class & operator = (Class const &) = delete;

        template <class Coroutine> inline explicit Class(Coroutine &&coroutine) noexcept(true)
        requires(! ::std::is_base_of_v<Class, ::std::decay_t<Coroutine>>):
            coroutine_{::std::forward<Coroutine>(coroutine)}
        {
            if (coroutine_) coro::private_::register_coroutine(coroutine_);
        }
    };

} // namespace task

    template <class T = void> using Task = task::Class<T>;

} // p5::rswc::implementation_::common::coro
