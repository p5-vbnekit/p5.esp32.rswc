#include <cstdint>

#include <string>
#include <utility>
#include <charconv>
#include <stdexcept>
#include <algorithm>
#include <string_view>
#include <type_traits>
#include <system_error>
#include <forward_list>
#include <unordered_map>

#include <fmt/format.h>

#include <cJSON.h>

#include <p5/rswc/implementation_/romfs.hpp>


namespace p5::rswc::implementation_::romfs {

    struct Private_ final {

        inline static auto const & instance() noexcept(false) {
            static auto const instance_ = Private_{};
            return instance_;
        }

        inline auto const & map() const noexcept(true) { return map_; }

    private:
        ::std::forward_list<::std::string> keys_;
        ::std::unordered_map<::std::string_view, ::std::string_view> map_;

        Private_() noexcept(false);
        Private_(Private_ &&) = delete;
        Private_(Private_ const &) = delete;
    };

    inline Private_::Private_() noexcept(false) {
        auto const image_view_ = [] () {
            extern char const end_[] asm("p5_rswc_romfs_image_layout_end");
            extern char const begin_[] asm("p5_rswc_romfs_image_layout_begin");
            if (! (end_ >= begin_)) throw ::std::runtime_error{"invalid image references"};
            return ::std::string_view{begin_, static_cast<::std::size_t>(::std::distance(begin_, end_))};
        } ();

        auto const image_header_size_ = [begin_ = image_view_.begin(), end_ = image_view_.end()] () {
            auto const zero_ = ::std::find(begin_, end_, 0);
            if (! (zero_ < end_)) throw ::std::runtime_error{"failed to parse image header, null-terminated c-string expected"};
            auto const distance_ = 1 + ::std::distance(begin_, zero_);
            if (! (6 < distance_)) throw ::std::runtime_error{"failed to parse image header, json array expected"};
            return static_cast<::std::size_t>(distance_);
        } ();

        auto const body_size_ = image_view_.size() - image_header_size_;
        auto const image_body_ = image_view_.data() + image_header_size_;

        auto * const json_header_ = [pointer_ = image_view_.data(), size_ = image_header_size_] () {
            char const *last_ = nullptr;
            auto * const parsed_ = ::cJSON_ParseWithLengthOpts(pointer_, size_, &last_, true);
            if (! static_cast<bool>(parsed_)) {
                auto const * const error_ = ::cJSON_GetErrorPtr();
                if (static_cast<bool>(error_)) throw ::std::runtime_error{::fmt::format(
                    "failed to parse image header, json error, offset={}", ::std::distance(pointer_, error_)
                )};
                throw ::std::runtime_error{"failed to parse image header, unknown json error"};
            }
            try {
                if (! ::cJSON_IsArray(parsed_)) throw ::std::runtime_error{"failed to parse image header, json array expected"};
                if (last_ != (pointer_ + size_ - 1)) throw ::std::runtime_error{::fmt::format(
                    "failed to parse image header, unexpected data, offset={}", ::std::distance(pointer_, last_)
                )};
            }
            catch(...) { ::cJSON_free(parsed_); throw; }
            return parsed_;
        } ();

        try {
            ::std::size_t index_ = 0;
            ::std::size_t offset_ = 0;
            for(auto *item_ = json_header_->child; static_cast<bool>(item_); item_ = item_->next) {
                if (0 == (index_ % 2)) {
                    if (! ::cJSON_IsString(item_)) throw ::std::runtime_error{::fmt::format(
                        "failed to parse image header item #{}, json string expected", index_
                    )};
                    auto const &&value_ = ::std::string_view{item_->valuestring};
                    if (value_.empty()) throw ::std::runtime_error{::fmt::format(
                        "failed to parse image header item #{}, non-empty json string expected", index_
                    )};
                    keys_.emplace_front(::std::move(value_));
                }
                else {
                    if (! ::cJSON_IsNumber(item_)) throw ::std::runtime_error{::fmt::format(
                        "failed to parse image header item #{}, json number expected", index_
                    )};
                    auto const raw_ = [item_] () mutable {
                        auto &&collector_ = ::std::string{};
                        auto * const pointer_ = ::cJSON_Print(item_);
                        try { collector_ = pointer_; }
                        catch(...) { cJSON_free(pointer_); }
                        return ::std::move(collector_);
                    } ();
                    ::std::size_t size_;
                    try {
                        if (raw_.empty()) throw ::std::runtime_error{"non-empty json number expected"};
                        auto const [pointer_, error_] = ::std::from_chars(raw_.data(), raw_.data() + raw_.size(), size_);
                        if (::std::errc{} != error_) {
                            try { throw ::std::system_error{::std::make_error_code(error_)}; }
                            catch(...) { ::std::throw_with_nested(::std::runtime_error{"positive json number expected"}); }
                        }
                        auto const left_ = body_size_ - offset_;
                        if (! (left_ >= size_)) throw ::std::runtime_error{::fmt::format(
                            "invalid size ({}), image limit ({}) exceeded", size_, left_
                        )};
                    }
                    catch(...) {
                        ::std::throw_with_nested(::std::runtime_error{::fmt::format(
                            "failed to parse image header item #{}", index_
                        )});
                    }
                    auto const &&key_ = ::std::string_view{keys_.front()};
                    if (! map_.emplace(key_, ::std::string_view{image_body_ + offset_, size_}).second) throw ::std::runtime_error{
                        ::fmt::format("failed to parse image header, got a duplicate key: {}", ::std::move(key_))
                    };
                    offset_ += size_;
                }
                index_++;
            }
            if (0 != (index_ % 2)) throw ::std::runtime_error{"failed to parse image header, even-size json array expected"};
            if (offset_ != body_size_) throw ::std::runtime_error{"failed to parse image, body size does not match header metadata"};
        }

        catch(...) {
            ::cJSON_Delete(json_header_);
            throw;
        }

        ::cJSON_Delete(json_header_);
    }

    Class::Class() noexcept(false) { Private_::instance(); }

    ::std::string_view const & Class::operator () (::std::string_view const &key) const noexcept(false) {
        if (key.empty()) throw ::std::invalid_argument{"non-empty key expected"};
        auto const &map_ = Private_::instance().map();
        auto const iterator_ = map_.find(key);
        if (iterator_ == map_.end()) throw ::std::runtime_error{::fmt::format("key not found: {}", key)};
        return iterator_->second;
    }

} // namespace p5::rswc::implementation_
