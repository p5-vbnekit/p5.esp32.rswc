#include <cstdint>

#include <memory>
#include <string>
#include <iostream>
#include <coroutine>
#include <exception>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common/exception_handling.hpp>
#include <p5/rswc/implementation_/common/coro.hpp>


namespace p5::rswc::implementation_::common::coro {
namespace task::private_ {

    ::std::string_view short_module_name() noexcept(true) { return "coro.task"; }

} // namespace task::private_

namespace future::private_ {

    ::std::string_view short_module_name() noexcept(true) { return "coro.future"; }

} // namespace future::private_

namespace private_ {

    struct Global_ final {
        ExceptionHandler exception_handler;
        ::std::unordered_map<void const *, ::std::shared_ptr<::std::coroutine_handle<>>> coroutines;
        inline static auto & instance() noexcept(true) { static Global_ instance_; return instance_; }
    };

    inline static auto default_exception_handler_(::std::exception_ptr const &exception, ::std::string_view const &location) noexcept(true) {
        try {
            if (exception) try { ::std::rethrow_exception(exception); } catch(...) {
                if (location.empty()) ::std::cerr << "exception caught" << ::std::endl << ::std::flush;
                else ::std::cerr << "exception caught in \"" << location << "\"" << ::std::endl << ::std::flush;
                exception_handling::walk(exception, [] (auto &&exception) { ::std::cerr << exception_handling::details(
                    ::std::forward<decltype(exception)>(exception)
                ) << ::std::endl << ::std::flush; });
            }
        }
        catch (...) { ::std::terminate(); }
    }

    void set_exception_handler(ExceptionHandler &&handler) noexcept(false) {
        Global_::instance().exception_handler = ::std::move(handler);
    }

    void set_exception_handler(ExceptionHandler const &handler) noexcept(false) {
        Global_::instance().exception_handler = handler;
    }

    void handle_exception(::std::string_view const &location) noexcept(true) {
        return handle_exception(::std::current_exception(), location);
    }

    void handle_exception(::std::exception_ptr const &exception, ::std::string_view const &location) noexcept(true) {
        try {
            if (! exception) return;
            auto const &global_ = Global_::instance();
            if (global_.exception_handler) global_.exception_handler(exception, location);
            else default_exception_handler_(exception, location);
        }
        catch (...) { ::std::terminate(); }
    }

    void notify_handler(Handler const &handler, ::std::string_view const &from) noexcept(true) {
        try { handler(); } catch (...) {
            ::std::string buffer_;
            ::std::string_view location_;
            try { location_ = buffer_ = ::fmt::format("{}.handler notification [address = {}]", from, static_cast<void const *>(&handler)); }
            catch (...) { location_ = "handler notification [failed to render address]"; }
            handle_exception(location_);
        }
    }

    void register_coroutine(::std::coroutine_handle<> const &coroutine) noexcept(true) {
        try {
            if (! coroutine) throw ::std::invalid_argument{"empty coroutine handle"};
            auto &coroutines_storage_ = Global_::instance().coroutines;
            auto const [iterator_, status_] = coroutines_storage_.insert({coroutine.address(), {}});
            if (! status_) throw ::std::invalid_argument{"coroutine already registered"};
            try { iterator_->second = ::std::make_shared<::std::decay_t<decltype(coroutine)>>(coroutine); }
            catch (...) { coroutines_storage_.erase(iterator_); throw; }
        }

        catch (...) {
            ::std::string buffer_;
            ::std::string_view location_;
            try { location_ = buffer_ = ::fmt::format("coro::_private::register_coroutine [address = {}]", coroutine.address()); }
            catch (...) { location_ = "coro::_private::register_coroutine [failed to render address]"; }
            handle_exception(location_);
        }
    }

    void unregister_coroutine(::std::coroutine_handle<> const &coroutine) noexcept(true) {
        try {
            if (! coroutine) throw ::std::invalid_argument{"empty coroutine handle"};
            if (0 == Global_::instance().coroutines.erase(coroutine.address())) throw ::std::invalid_argument{"coroutine not registered"};
        }

        catch (...) {
            ::std::string buffer_;
            ::std::string_view location_;
            try { location_ = buffer_ = ::fmt::format("coro::_private::unregister_coroutine [address = {}]", coroutine.address()); }
            catch (...) { location_ = "coro::_private::unregister_coroutine [failed to render address]"; }
            handle_exception(location_);
        }
    }

    Handler make_coroutine_resumer(::std::coroutine_handle<> const &coroutine) noexcept(false) {
        if (! coroutine) throw ::std::invalid_argument{"empty coroutine handle"};
        auto const &coroutines_storage_ = Global_::instance().coroutines;
        auto const iterator_ = coroutines_storage_.find(coroutine.address());
        if (coroutines_storage_.end() == iterator_) return [coroutine] () { coroutine.resume(); };
        auto const &pointer_ = iterator_->second;
        if (! pointer_) throw ::std::invalid_argument{"coroutine handle is destroyed"};
        return [weak_ = ::std::weak_ptr<::std::decay_t<decltype(*pointer_)>>(pointer_), pointer_ = coroutine.address()] () {
            if (auto const shared_ = weak_.lock()) {
                shared_->resume();
                return;
            }
            ::std::string buffer_;
            ::std::string_view view_;
            try { view_ = buffer_ = ::fmt::format("attepmt to resume unregistered coroutine [address = {}]", pointer_); }
            catch (...) { view_ = "attepmt to resume unregistered coroutine [failed to render address]"; }
            try { throw ::std::invalid_argument{view_.data()}; }
            catch (...) { handle_exception("coro::_private coroutine resumer"); }
        };
    }

} // namespace private_
} // p5::rswc::implementation_::common::coro
