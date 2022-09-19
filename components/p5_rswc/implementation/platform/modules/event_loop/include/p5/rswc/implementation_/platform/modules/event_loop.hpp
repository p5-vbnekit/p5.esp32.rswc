#pragma once

#include <p5/rswc/implementation_/platform/module/interface.hpp>

#include "event_loop.fwd.hpp"


namespace p5::rswc::implementation_::platform::modules::event_loop {

    struct Interface: module::Interface {
        virtual ~Interface() override;

    protected:
        Interface() noexcept(true);

    private:
        Interface(Interface &&) = delete;
        Interface(Interface const &) = delete;
        Interface & operator = (Interface &&) = delete;
        Interface & operator = (Interface const &) = delete;
    };

    void initialize() noexcept(false);
    void deinitialize() noexcept(false);

} // namespace p5::rswc::implementation_::platform::modules::event_loop
