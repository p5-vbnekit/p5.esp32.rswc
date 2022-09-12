#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include <fmt/format.h>

#include <esp_event.h>

#include <p5/rswc/implementation_/common/sdk.hpp>

#include "logged_action.hpp"

#include "event_loop.hpp"


namespace p5::rswc::implementation_::event_loop {
namespace private_ {

    inline auto & shared() noexcept(true) {
        static ::std::optional<::std::shared_ptr<Class>> instance_;
        return instance_;
    }

} // namespace private_

    void initialize() noexcept(false) {
        auto &shared_ = private_::shared();
        if (shared_) throw ::std::logic_error{"initialized already"};
        shared_.emplace(new ::std::decay_t<decltype(**shared_)>);
        try { logged_action::execute("SDK ::esp_event_loop_create_default", [] () {
            common::sdk::check_or_throw(::esp_event_loop_create_default());
        }); } catch (...) { shared_.reset(); throw; }
    }

    void deinitialize() noexcept(false) {
        auto &shared_ = private_::shared();
        if (! shared_) throw ::std::logic_error{"not initialized"};
        if (! *shared_) throw ::std::runtime_error{"broken state"};
        if (1 != shared_->use_count()) throw ::std::runtime_error{::fmt::format("still in use: {}", shared_->use_count())};
        shared_.value().reset();
        logged_action::execute("SDK ::esp_event_loop_delete_default", [] () { common::sdk::check_or_throw(::esp_event_loop_delete_default()); });
        shared_.reset();
    }

    ::std::shared_ptr<Class> instance() noexcept(false) { return private_::shared().value_or(nullptr); }

} // namespace p5::rswc::implementation_::event_loop
