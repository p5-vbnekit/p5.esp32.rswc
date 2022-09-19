#pragma once

#include "platform.fwd.hpp"
#include "platform/class.hpp"
#include "platform/module.hpp"
#include "platform/logged_action.hpp"


namespace p5::rswc::implementation_::platform {

    inline static auto & instance() noexcept(false) { return Class::instance(); }

} // namespace p5::rswc::implementation_::platform
