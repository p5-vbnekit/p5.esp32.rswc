#pragma once

#include <string_view>

#include <p5/rswc/implementation_/platform/module/interface.hpp>

#include "romfs.fwd.hpp"


namespace p5::rswc::implementation_::platform::modules::romfs {

    struct Interface: module::Interface {
        virtual ::std::string_view const & get(::std::string_view const &) const noexcept(false) = 0;

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

} // namespace p5::rswc::implementation_::platform::modules::romfs
