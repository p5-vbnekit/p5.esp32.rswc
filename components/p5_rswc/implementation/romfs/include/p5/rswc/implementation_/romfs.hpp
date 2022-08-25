#pragma once

#include <string_view>


namespace p5::rswc::implementation_ {
namespace romfs {

    struct Class final {
        Class() noexcept(false);

        ::std::string_view const & operator () (::std::string_view const &key) const noexcept(false);
    };

} // namespace romfs

    using RomFs = romfs::Class;

} // namespace p5::rswc::implementation_
