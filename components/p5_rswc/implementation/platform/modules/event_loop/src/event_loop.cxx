#include <memory>

#include <esp_event.h>

#include <p5/rswc/implementation_/common/sdk.hpp>
#include <p5/rswc/implementation_/platform.hpp>

#include <p5/rswc/implementation_/platform/modules/event_loop.hpp>


namespace p5::rswc::implementation_::platform::modules::event_loop {
namespace private_ {

    struct Class final: Interface {
        ::std::shared_ptr<module::Interface> &reference;

        template <class T> inline explicit Class(T &reference) noexcept(false)
        requires(! ::std::is_base_of_v<Class, ::std::decay_t<T>>):
            reference{reference}
        {
            logged_action::execute("SDK ::esp_event_loop_create_default", [] () {
                common::sdk::check_or_throw(::esp_event_loop_create_default());
            });
        }
    };

} // namespace private_

    Interface::Interface() noexcept(true) = default;
    Interface::~Interface() = default;

    void initialize() noexcept(false) {
        auto &platform_ = platform::instance();
        auto &reference_ = platform_.acquire<Interface>();
        try { reference_ = ::std::make_shared<private_::Class>(reference_); }
        catch (...) { platform_.release<Interface>(reference_); throw; }
    }

    void deinitialize() noexcept(false) {
        auto &platform_ = platform::instance();
        auto &reference_ = [&platform_] () -> decltype(auto) {
            auto const base_ = platform_.get_module<Interface>();
            if (! base_) throw ::std::logic_error{"module not initialized"};
            auto const derived_ = ::std::dynamic_pointer_cast<private_::Class>(base_);
            if (! derived_) throw ::std::logic_error{"incompatible module instance"};
            return derived_->reference;
        } ();
        reference_.reset();
        logged_action::execute("SDK ::esp_event_loop_delete_default", [] () {
            common::sdk::check_or_throw(::esp_event_loop_delete_default());
        });
        platform_.release<Interface>(reference_);
    }

} // namespace p5::rswc::implementation_::platform::modules::event_loop
