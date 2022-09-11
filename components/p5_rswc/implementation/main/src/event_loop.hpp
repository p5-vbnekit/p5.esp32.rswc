#pragma once

#include <memory>


namespace p5::rswc::implementation_ {
namespace event_loop {

    struct Class final {};

    void initialize() noexcept(false);
    void deinitialize() noexcept(false);

    ::std::shared_ptr<Class> instance() noexcept(false);

} // namespace event_loop

    using EventLoop = event_loop::Class;

} // namespace p5::rswc::implementation_
