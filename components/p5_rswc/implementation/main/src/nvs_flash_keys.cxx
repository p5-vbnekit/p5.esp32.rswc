#include <cstdint>

#include <array>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <initializer_list>

#include <fmt/format.h>

#include "nvs_flash_keys.hpp"


namespace p5::rswc::implementation_::nvs_flash_keys {
namespace private_ {

    inline constexpr static auto array() noexcept(true) {
        return [] (auto && ... arguments) {
            return ::std::array<::std::string_view, sizeof ... (arguments)>{::std::forward<decltype(arguments)>(arguments) ...};
        } (
            "wifi.access_point.ssid",
            "wifi.access_point.password",
            "wifi.station.ssid",
            "wifi.station.password"
        );
    }

    inline static auto make_map() noexcept(false) {
        auto index_ = static_cast<::std::size_t>(0);
        auto &&instance_ = private_::Map{};
        for (auto const &key_: array()) {
            if (key_.empty()) throw ::std::logic_error{"empty key in initializer"};
            if (! (::std::numeric_limits<::std::decay_t<decltype(index_)>>::max() > ++index_)) throw ::std::logic_error{"index overflow"};
            auto const result_ = instance_.emplace(key_, ::fmt::format("{:x}", index_));
            if (! result_.second) throw ::std::logic_error{::fmt::format("key \"{}\" is dupplicated", key_)};
        }
        return ::std::forward<decltype(instance_)>(instance_);
    }

} // namespace private_

    char const * Class::get(char const *key) const noexcept(false) {
        if (! key) throw ::std::invalid_argument{"empty key"};
        auto const iterator_ = map_.find(key);
        if (iterator_ == map_.end()) throw ::std::invalid_argument{"key not found"};
        return iterator_->second.c_str();
    }

    Class::Class() noexcept(false): map_{private_::make_map()} {}

} // p5::rswc::implementation_::nvs_flash_keys
