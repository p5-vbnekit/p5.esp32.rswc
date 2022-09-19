#include <memory>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <unordered_map>

#include <p5/rswc/implementation_/platform/class.hpp>


namespace p5::rswc::implementation_::platform {

    struct Class::Private_ final {
        ::std::unordered_map<::std::type_index, ::std::shared_ptr<Module>> map;
    };

    Class & Class::instance() noexcept(false) { static Class instance_; return instance_; }

    ::std::shared_ptr<Module> Class::get_module_(::std::type_index const &key) noexcept(true) {
        auto const &map_ = private_->map;
        auto const iterator_ = map_.find(key);
        if (map_.end() == iterator_) return {};
        return iterator_->second;
    }

    ::std::shared_ptr<Module> & Class::acquire_(::std::type_index const &key) noexcept(false) {
        auto &map_ = private_->map;
        auto const emplace_result_ = map_.emplace(key, nullptr);
        if (! emplace_result_.second) throw ::std::invalid_argument{"key assigned already"};
        return emplace_result_.first->second;
    }

    void Class::release_(::std::type_index const &key, ::std::shared_ptr<Module> &pointer) noexcept(false) {
        auto &map_ = private_->map;
        auto const iterator_ = map_.find(key);
        if (map_.end() == iterator_) throw ::std::invalid_argument{"key not found"};
        if (&pointer != &(iterator_->second)) throw ::std::invalid_argument{"invalid pointer"};
        map_.erase(iterator_);
    }

    Class::Class() noexcept(false): private_{new ::std::decay_t<decltype(*private_)>} {}

    Class::~Class() = default;

} // namespace p5::rswc::implementation_::platform
