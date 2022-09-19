#pragma once

#include "interface.fwd.hpp"


namespace p5::rswc::implementation_::platform::module {

    struct Interface {
        virtual ~Interface();

    protected:
        Interface() noexcept(true);

    private:
        Interface(Interface &&) = delete;
        Interface(Interface const &) = delete;
        Interface & operator = (Interface &&) = delete;
        Interface & operator = (Interface const &) = delete;
    };

} // namespace p5::rswc::implementation_::module
