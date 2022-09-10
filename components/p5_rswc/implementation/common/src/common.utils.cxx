#include <cstdlib>

#include <string>
#include <utility>
#include <typeinfo>
#include <exception>
#include <stdexcept>

#include <cxxabi.h>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common/utils.hpp>


namespace p5::rswc::implementation_::common::utils {

    ::std::string demangle(::std::type_info const &info) noexcept(false) {
        int status;
        auto &&text_ = ::std::string{};
        auto const * const name_ = info.name();
        try {
            if (! static_cast<bool>(name_)) throw ::std::invalid_argument{"invalid type_info (null name)"};
            auto * const pointer_ = ::abi::__cxa_demangle(name_, 0, 0, &status);
            if (! static_cast<bool>(pointer_)) throw ::std::runtime_error{"null pointer returned"};
            try {
                text_ = pointer_;
                if (text_.empty()) throw ::std::runtime_error{"empty name returned"};
            }
            catch(...) { ::free(pointer_); throw; }
            ::free(pointer_);
        }
        catch(...) { ::std::throw_with_nested(::std::runtime_error{::fmt::format("failed to demangle: {}", name_)}); }
        return ::std::move(text_);
    }

} // namespace p5::rswc::implementation_::common::utils
