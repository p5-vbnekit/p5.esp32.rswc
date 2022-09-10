#include <cstdlib>

#include <utility>
#include <stdexcept>
#include <string_view>

#include <esp_err.h>

#include <fmt/format.h>

#include <p5/rswc/implementation_/common/sdk.hpp>


namespace p5::rswc::implementation_::common::sdk {
namespace private_ {

    ::std::string_view error_name(ErrorCode error_code) noexcept(true) {
        auto &&text_ = ::std::string_view{::esp_err_to_name(error_code)};
        if (text_.empty()) return "UNKNOWN ERROR";
        return ::std::forward<decltype(text_)>(text_);
    }

    ErrorCode check_or_throw(ErrorCode error_code) noexcept(false) {
        if (ESP_OK != error_code) {
            auto &&name_ = ::std::string_view{::esp_err_to_name(error_code)};
            if (name_.empty()) throw ::std::runtime_error{::fmt::format("error code = {:#x}", error_code)};
            throw ::std::runtime_error{::fmt::format("{}, error code = {:#x}", ::std::forward<decltype(name_)>(name_), error_code)};
        }
        return error_code;
    }

} // namespace private_
} // namespace p5::rswc::implementation_::common::sdk
