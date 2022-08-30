#pragma once


namespace p5::rswc::implementation_ {
namespace dirty_ {

    void routine() noexcept(false);

} // namespace dirty_

    inline static auto dirty() noexcept(false) { return dirty_::routine(); }

} // namespace p5::rswc::implementation_
