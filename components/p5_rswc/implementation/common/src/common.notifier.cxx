#include <cstdint>

#include <list>
#include <limits>
#include <utility>
#include <iterator>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <functional>
#include <type_traits>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common/notifier.hpp>


namespace p5::rswc::implementation_::common::notifier {
namespace private_ {

    template <class Lock> inline auto make_lock_guard_(Lock &lock) noexcept(false) {
        struct Result_ final {
            inline Result_(Lock &lock) noexcept(false): lock(lock) { lock.acquire(); }
            inline ~Result_() noexcept(true) { try { lock.release(); } catch (...) { ::std::terminate(); } }

        private:
            Lock &lock;
        };

        return Result_{lock};
    }

    struct Storage final: ::std::enable_shared_from_this<Storage> {
        auto move(subscription::Class *, subscription::Class *) noexcept(true);
        auto remove(subscription::Class *key) noexcept(true);

        auto clear() noexcept(true);
        auto notify() noexcept(false);
        template <class Handler> inline auto & subscribe(Handler &&) noexcept(false);

    private:
        struct Item_ final {
            Handler handler;
            subscription::Class root;
            subscription::Class *pointer = &root;

            inline explicit Item_(Handler &&handler): handler{::std::move(handler)} {}

        private:
            Item_() = delete;
            Item_(Item_ &&) = delete;
            Item_(Item_ const &) = delete;
            Item_ & operator = (Item_ &&) = delete;
            Item_ & operator = (Item_ const &) = delete;
        };

        struct NotifyLock_ final {
            inline constexpr auto acquire() noexcept(false) {
                if (state_) throw ::std::runtime_error{"failed to acquire notify lock"};
                state_ = true;
            }

            inline constexpr auto release() noexcept(true) {
                if (! state_) try { throw ::std::logic_error{"failed to release notify lock"}; } catch (...) { ::std::terminate(); }
                state_ = false;
            }

            constexpr NotifyLock_() noexcept(true) = default;

        private:
            bool state_ = false;

            NotifyLock_(NotifyLock_ &&) = delete;
            NotifyLock_(NotifyLock_ const &) = delete;
            NotifyLock_ & operator = (NotifyLock_ &&) = delete;
            NotifyLock_ & operator = (NotifyLock_ const &) = delete;
        };

        using List = ::std::list<Item_>;

        struct RemoveLock_ final {
            List collector;

            inline constexpr auto acquire() noexcept(true) {
                if (! (::std::numeric_limits<::std::decay_t<decltype(counter_)>>::max() > counter_)) {
                    try { throw ::std::logic_error{"remove lock counter overflow"}; } catch (...) { ::std::terminate(); }
                }
                counter_++;
            }

            inline constexpr auto release() noexcept(true) {
                if (! (0 < counter_)) try { throw ::std::logic_error{"failed to release remove lock"}; } catch (...) { ::std::terminate(); }
                if (1 == counter_) { try { collector.clear(); } catch (...) { ::std::terminate(); } }
                counter_--;
            }

            inline constexpr operator bool () noexcept(true) { return 0 < counter_; }

            RemoveLock_() noexcept(true) = default;

        private:
            ::std::size_t counter_ = 0;

            RemoveLock_(RemoveLock_ &&) = delete;
            RemoveLock_(RemoveLock_ const &) = delete;
            RemoveLock_ & operator = (RemoveLock_ &&) = delete;
            RemoveLock_ & operator = (RemoveLock_ const &) = delete;
        };

        ::std::list<Item_> items_;
        NotifyLock_ notify_lock_;
        RemoveLock_ remove_lock_;
    };

    inline auto Storage::move(subscription::Class *o, subscription::Class *n) noexcept(true) {
        try {
            if (! o) throw ::std::invalid_argument{"empty old key"};
            if (! n) throw ::std::invalid_argument{"empty new key"};
            if (! o->storage_.expired()) throw ::std::invalid_argument{"old key storage reference is not expired"};
            if constexpr (true) {
                auto const * const pointer_ = n->storage_.lock().get();
                if (this != pointer_) throw ::std::invalid_argument{::fmt::format(
                    "new key storage reference is another: {} != {}",
                    static_cast<void const *>(pointer_),
                    static_cast<void const *>(this)
                )};
            }
            auto const iterator_ = ::std::find_if(items_.begin(), items_.end(), [key = o] (auto const &item) { return key == item.pointer; });
            if (items_.end() == iterator_) throw ::std::logic_error{::fmt::format("old key not found: {}", static_cast<void const *>(o))};
            iterator_->pointer = n;
        }
        catch (...) { ::std::terminate(); }
    }

    inline auto Storage::remove(subscription::Class *key) noexcept(true) {
        try {
            if (! key) throw ::std::invalid_argument{"empty key"};
            if (! key->storage_.expired()) throw ::std::invalid_argument{"key storage reference is not expired"};
            auto const iterator_ = ::std::find_if(items_.cbegin(), items_.cend(), [key] (auto const &item) { return key == item.pointer; });
            if (items_.end() == iterator_) throw ::std::invalid_argument{::fmt::format("key not found: {}", static_cast<void const *>(key))};
            [[maybe_unused]] auto const remove_lock_guard_ = make_lock_guard_(remove_lock_);
            remove_lock_.collector.splice(remove_lock_.collector.end(), ::std::move(items_), iterator_);
        }
        catch (...) { ::std::terminate(); }
    }

    inline auto Storage::clear() noexcept(true) {
        [[maybe_unused]] auto const remove_lock_guard_ = make_lock_guard_(remove_lock_);
        try {
            for (auto const &item: items_) if (item.pointer) item.pointer->storage_.reset();
            remove_lock_.collector.splice(remove_lock_.collector.end(), ::std::move(items_));
        }
        catch (...) { ::std::terminate(); }
    }

    inline auto Storage::notify() noexcept(false) {
        [[maybe_unused]] auto const notify_lock_guard_ = make_lock_guard_(notify_lock_);
        [[maybe_unused]] auto const remove_lock_guard_ = make_lock_guard_(remove_lock_);
        try {
            auto handlers_ = ::std::list<Handler const *>{};
            for (auto const &item: items_) if (item.pointer) handlers_.push_back(&item.handler);
            for (auto const &handler: handlers_) ::std::invoke(*handler);
        }
        catch (...) { ::std::terminate(); }
    }

    template <class Handler> inline auto & Storage::subscribe(Handler &&handler) noexcept(false) {
        [[maybe_unused]] auto const remove_lock_guard_ = make_lock_guard_(remove_lock_);
        auto &subscription_ = items_.emplace_back(::std::forward<Handler>(handler)).root;
        subscription_.storage_ = shared_from_this();
        return subscription_;
    }

} // namespace private_

namespace subscription {

    void Class::cancel() noexcept(false) {
        if (storage_.expired()) throw ::std::runtime_error{"subscription expired"};
        auto const shared_ = storage_.lock();
        storage_.reset();
        shared_->remove(this);
    }

    bool Class::expired() const noexcept(true) { return storage_.expired(); }

    Class::Class(Class &&other) noexcept(true):
        storage_{::std::move(other.storage_)}
    {
        if (storage_.expired()) return;
        auto const shared_ = storage_.lock();
        shared_->move(&other, this);
    }

    Class & Class::operator = (Class &&other) noexcept(true) {
        if (! (storage_ = ::std::move(other.storage_)).expired()) {
            auto const shared_ = storage_.lock();
            shared_->move(&other, this);
        }
        return *this;
    }

    Class::~Class() noexcept(true) {
        if (storage_.expired()) return;
        auto const shared_ = storage_.lock();
        storage_.reset();
        shared_->remove(this);
    }

} // namespace subscription

    void Class::clear() noexcept(false) {
        if (! storage_) throw ::std::runtime_error{"notifier expired"};
        auto const shared_ = storage_;
        shared_->clear();
    }

    void Class::notify() const noexcept(false) {
        if (! storage_) throw ::std::runtime_error{"notifier expired"};
        auto const shared_ = storage_;
        shared_->notify();
    }

    bool Class::expired() const noexcept(true) { return ! storage_; }

    subscription::Class & Class::subscribe_(Handler &&handler) noexcept(false) {
        if (! storage_) throw ::std::runtime_error{"notifier expired"};
        auto const shared_ = storage_;
        return shared_->subscribe(::std::move(handler));
    }

    Class::Class() noexcept(false): storage_{new ::std::decay_t<decltype(*storage_)>} {}

    Class::Class(Class &&other) noexcept(true) = default;
    Class & Class::operator = (Class &&other) noexcept(true) = default;

    Class::~Class() noexcept(true) {
        if (auto const shared_ = ::std::move(storage_)) shared_->clear();
    };

} // namespace p5::rswc::implementation_::common::notifier
