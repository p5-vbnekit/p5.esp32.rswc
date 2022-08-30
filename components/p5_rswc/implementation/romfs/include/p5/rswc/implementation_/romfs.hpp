#pragma once

#include <string_view>


namespace p5::rswc::implementation_::romfs {

    void initialize() noexcept(false);
    void deinitialize() noexcept(false);

    ::std::string_view const & get(::std::string_view const &) noexcept(false);

} // namespace p5::rswc::implementation_::romfs
