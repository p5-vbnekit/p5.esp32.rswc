#pragma once

#include <p5/rswc/implementation_/common/coro.hpp>


namespace p5::rswc::implementation_::main_task {

    common::coro::Task<void> make() noexcept(false);

    void post_test_event() noexcept(false);

} // namespace p5::rswc::implementation_::main_task
