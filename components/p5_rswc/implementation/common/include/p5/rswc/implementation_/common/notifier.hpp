#pragma once

#include <memory>
#include <utility>
#include <stdexcept>
#include <type_traits>

#include "notifier/fwd.hpp"


namespace p5::rswc::implementation_::common::notifier {
namespace private_ {

    struct Storage;

} // namespace private_

   struct Class final {
        void clear() noexcept(false);
        void notify() const noexcept(false);
        bool expired() const noexcept(true);

        template <class Handler> auto & subscribe(Handler &&) noexcept(false);

        operator bool () const noexcept(true);
        auto operator () () const noexcept(false);

        Class() noexcept(false);

        Class(Class &&other) noexcept(true);
        Class & operator = (Class &&other) noexcept(true);

        ~Class() noexcept(true);

    private:
        ::std::shared_ptr<private_::Storage> storage_;

        subscription::Class & subscribe_(Handler &&) noexcept(false);

        Class(Class const &) = delete;
        Class & operator = (Class const &) = delete;
    };

    template <class ... T> static auto make(T && ... arguments) noexcept(false);

namespace subscription {

    struct Class final {
        void cancel() noexcept(false);
        bool expired() const noexcept(true);

        operator bool () const noexcept(true);

        Class() noexcept(true) = default;
        Class(Class &&other) noexcept(true);
        Class & operator = (Class &&other) noexcept(true);

        ~Class() noexcept(true);

    private:
        friend private_::Storage;
        ::std::weak_ptr<private_::Storage> storage_;

        Class(Class const &) = delete;
        Class & operator = (Class const &) = delete;
    };

} // namespace subscription

    template <class Handler> auto & Class::subscribe(Handler &&handler) noexcept(false) {
        if constexpr (::std::is_same_v<notifier::Handler, ::std::decay_t<Handler>>) {
            if (! handler) throw ::std::runtime_error{"empty handler"};
            return subscribe_(::std::forward<Handler>(handler));
        }
        else return subscribe_(notifier::Handler{::std::forward<Handler>(handler)});
    }

    inline Class::operator bool () const noexcept(true) { return ! expired(); }

    inline auto Class::operator () () const noexcept(false) { return notify(); }

    template <class ... T> inline static auto make(T && ... arguments) noexcept(false) {
        return Class{::std::forward<T>(arguments) ...};
    }

namespace subscription {

    inline Class::operator bool () const noexcept(true) { return ! expired(); }

} // namespace subscription
} // p5::rswc::implementation_::common::notifier
