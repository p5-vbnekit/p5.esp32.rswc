#pragma once

#include <string>
#include <utility>
#include <string_view>
#include <unordered_map>

#include <p5/rswc/implementation_/common/coro.hpp>


namespace p5::rswc::implementation_ {
namespace nvs_flash_keys {
namespace private_ {

    using Map = ::std::unordered_map<::std::string_view, ::std::string>;

} // namespace private_

    struct Class final {
        char const * get(char const *) const noexcept(false);

        Class() noexcept(false);

        template <class T> auto operator [] (T &&name) const noexcept(false) { return get(::std::forward<T>(name)); }

        Class(Class &&) = default;
        Class(Class const &) = default;

        Class & operator = (Class &&) = default;
        Class & operator = (Class const &) = default;

    private:
        private_::Map map_;
    };

    template <class ... T> inline static auto make(T && ... arguments) noexcept(false) {
        return Class{::std::forward<T>(arguments) ...};
    }

} // namespace nvs_flash_keys

    using NfsFlashKeys = nvs_flash_keys::Class;

} // p5::rswc::implementation_
