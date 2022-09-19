#pragma once

#include <memory>
#include <utility>
#include <typeindex>
#include <functional>
#include <type_traits>

#include "module.fwd.hpp"
#include "class.fwd.hpp"


namespace p5::rswc::implementation_ {
namespace platform {

    struct Class final {
        template <class> auto get_module() noexcept(true);
        template <class> auto get_module() const noexcept(true);

        template <class> auto & acquire() noexcept(false);
        template <class> auto release(::std::shared_ptr<Module> &) noexcept(false);

        static Class & instance() noexcept(false);

    private:
        struct Private_;
        ::std::unique_ptr<Private_> private_;

        ::std::shared_ptr<Module> get_module_(::std::type_index const &) noexcept(true);

        ::std::shared_ptr<Module> & acquire_(::std::type_index const &) noexcept(false);
        void release_(::std::type_index const &, ::std::shared_ptr<Module> &) noexcept(false);

        template <class> constexpr static auto make_key_() noexcept(true);

        Class() noexcept(false); ~Class();

        Class(Class &&) = delete;
        Class(Class const &) = delete;
        Class & operator = (Class &&) = delete;
        Class & operator = (Class const &) = delete;
    };


    template <class Interface> inline constexpr auto Class::make_key_() noexcept(true) {
        static_assert(! ::std::is_reference_v<Interface>);
        using Interface_ = ::std::decay_t<Interface>;
        static_assert(! ::std::is_pointer_v<Interface_>);
        return ::std::type_index{typeid(Interface_ const *)};
    }

    template <class Interface> inline auto Class::get_module() noexcept(true) {
        return ::std::reinterpret_pointer_cast<Interface>(get_module_(make_key_<Interface>()));
    }

    template <class Interface> inline auto Class::get_module() const noexcept(true) {
        auto &&pointer_ = const_cast<::std::decay_t<decltype(*this)> &>(*this).get_module<Interface>();
        if constexpr (::std::is_const_v<Interface>) return ::std::forward<decltype(pointer_)>(pointer_);
        else return ::std::static_pointer_cast<Interface const>(::std::forward<decltype(pointer_)>(pointer_));
    }

    template <class Interface> auto & Class::acquire() noexcept(false) {
        static_assert(::std::is_same_v<Interface, ::std::decay_t<Interface>>);
        return acquire_(make_key_<Interface>());
    }

    template <class Interface> auto Class::release(::std::shared_ptr<Module> &key) noexcept(false) {
        static_assert(::std::is_same_v<Interface, ::std::decay_t<Interface>>);
        return release_(make_key_<Interface>(), key);
    }

} // namespace platform

    using Platform = platform::Class;

} // namespace p5::rswc::implementation_
