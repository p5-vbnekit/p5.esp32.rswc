#pragma once

#include "coro/task.hpp"
#include "coro/future.hpp"
#include "coro/private_.hpp"


namespace p5::rswc::implementation_::common::coro {

    template <class Handler> inline static auto set_exception_handler(Handler &&handler) noexcept(false) {
        return private_::set_exception_handler(::std::forward<Handler>(handler));
    }

} // namespace p5::rswc::implementation_::common::coro
