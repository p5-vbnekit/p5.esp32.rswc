#include <cstdint>

#include <array>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <type_traits>

#include <esp_err.h>
#include <esp_mac.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <esp_random.h>
#include <esp_http_server.h>

#include <freertos/FreeRTOS.h>

#include <fmt/format.h>

#include <sdkconfig.h>

#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/romfs.hpp>
#include <p5/rswc/implementation_/common.hpp>

#include <p5/rswc/implementation_/dirty.hpp>


namespace p5::rswc::implementation_::dirty_ {
namespace private_ {

    template <class Name, class Delegate> inline static auto action(
        Name &&name, Delegate &&delegate
    ) noexcept(false) {
        struct SuccessNotifier_ final {
            bool enabled = false;
            ::std::string_view const name;
            inline ~SuccessNotifier_() noexcept(true) {
                try { if (enabled) log<LogLevel::Info>(::fmt::format("action completed successfully: {}", ::std::move(name))); }
                catch(...) {}
            }
        } success_notifier_ {.name = name};
        log<LogLevel::Verbose>(::fmt::format("attempt to perform an action: {}", name));
        try {
            success_notifier_.enabled = true;
            return ::std::forward<Delegate>(delegate)();
        }
        catch(...) {
            success_notifier_.enabled = false;
            ::std::throw_with_nested(::std::runtime_error{::fmt::format(
                "action failed: {}", ::std::forward<Name>(name)
            )});
        }
    }

namespace trimmer {

        inline constexpr auto const default_spaces = ::std::string_view{" \t\n\f\r\v"};

        inline constexpr static auto const left(
            ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
        ) noexcept(true) {
            if (source.empty() || spaces.empty()) return source;
            auto const offset_ = source.find_first_not_of(spaces);
            if (offset_ == source.npos) return ::std::string_view{source.data(), 0};
            return ::std::string_view{source.begin() + offset_, source.end()};
        }

        inline constexpr static auto const right(
            ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
        ) noexcept(true) {
            if (source.empty() || spaces.empty()) return source;
            auto const offset_ = source.find_last_not_of(spaces);
            if (offset_ == source.npos) return ::std::string_view{source.data(), 0};
            return ::std::string_view{source.begin(), 1 + offset_};
        }

        inline constexpr static auto const both(
            ::std::string_view const &source, ::std::string_view const &spaces = default_spaces
        ) noexcept(true) {
            return right(left(source), spaces);
        }

} // namespace trimmer
} // namespace private_

    void routine() noexcept(false) {
        auto &&exceptions_ = static_cast<::std::size_t>(0);
        common::utils::with_finally(
            [] (auto &&finally) {
                private_::action("initializing p5_rswc::romfs module", [] () { romfs::initialize(); });
                finally([] () { private_::action(
                    "deinitializing p5_rswc::romfs module", [] () { romfs::deinitialize(); }
                ); });

                if constexpr (true) {
                    constexpr static auto const * const public_certificate_path_ = "ssl/public.pem";
                    log<LogLevel::Info>(::fmt::format(
                        "public certificate \"{}\" size = {}", public_certificate_path_, romfs::get(public_certificate_path_).size()
                    ));

                    constexpr static auto const * const private_certificate_path_ = "ssl/private.pem";
                    log<LogLevel::Info>(::fmt::format(
                        "private certificate \"{}\" size = {}", private_certificate_path_, romfs::get(private_certificate_path_).size()
                    ));
                }

                [[maybe_unused]] static ::httpd_handle_t http_server_handle_ = NULL;

                auto const &&nvs_handle_ = private_::action("initializing non-volatile storage", [&finally] () {
                    if constexpr (true) {
                        auto abort_on_exception_ = true;
                        try { private_::action("SDK ::nvs_flash_init", [&abort_on_exception_] () {
                            auto const &&error_code_ = ::nvs_flash_init();
                            switch (error_code_) {
                            default: break;
                            case ESP_ERR_NVS_NO_FREE_PAGES:
                            case ESP_ERR_NVS_NEW_VERSION_FOUND:
                                abort_on_exception_ = false;
                                break;
                            }
                            common::sdk::check_or_throw(::std::forward<decltype(error_code_)>(error_code_));
                        }); }
                        catch (...) {
                            if (abort_on_exception_) throw;
                            common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                                log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                            });
                            private_::action("SDK ::nvs_flash_erase", [] () { common::sdk::check_or_throw(::nvs_flash_erase()); });
                            private_::action("SDK ::nvs_flash_init", [] () { common::sdk::check_or_throw(::nvs_flash_init()); });
                        }
                    }
                    finally([] () { private_::action(
                        "SDK ::nvs_flash_deinit", [] () { common::sdk::check_or_throw(::nvs_flash_deinit()); }
                    ); });
                    auto const handle_ = private_::action("SDK ::nvs_open", [] () {
                        auto &&handle_ = static_cast<::nvs_handle_t>(0);
                        common::sdk::check_or_throw(::nvs_open("p5.rswc", ::NVS_READWRITE, &handle_));
                        if (0 == handle_) throw ::std::runtime_error{"zero handle returned"};
                        return ::std::forward<decltype(handle_)>(handle_);
                    });
                    finally([handle_] () { private_::action("SDK ::nvs_close", [&handle_] () { ::nvs_close(handle_); }); });
                    return handle_;
                });

                auto const &&wifi_ap_ssid_ = private_::action("generating wifi access point ssid", [&nvs_handle_] () {
                    constexpr static auto const max_size_ = 32;
                    constexpr static auto const * const nvs_key_ = "ssid";
                    auto &&text_ = [&nvs_handle_] () -> ::std::string {
                        auto abort_on_exception_ = false;
                        try { return private_::action("SDK ::nvs_get_str (wifi access point ssid)", [&nvs_handle_, &abort_on_exception_] () -> ::std::string {
                            auto &&raw_size_ = static_cast<::std::size_t>(0);
                            common::sdk::check_or_throw(::nvs_get_str(nvs_handle_, nvs_key_, nullptr, &raw_size_));
                            if (! (1 < raw_size_)) throw ::std::runtime_error{"invalid size returned"};
                            auto const size_ = raw_size_ - 1;
                            if (! (max_size_ >= (size_))) throw ::std::runtime_error{"invalid size returned"};
                            auto &&buffer_ = ::std::string(size_, 0);
                            common::sdk::check_or_throw(::nvs_get_str(nvs_handle_, nvs_key_, buffer_.data(), &raw_size_));
                            if (0 != buffer_.data()[size_]) {
                                abort_on_exception_ = true;
                                throw ::std::runtime_error{"buffer overflow"};
                            }
                            if (raw_size_ != (1 + size_)) throw ::std::runtime_error{"invalid size returned"};
                            if (size_ != ::std::string_view{buffer_.c_str()}.size()) throw ::std::runtime_error{"invalid text returned"};
                            return ::std::move(buffer_);
                        }); }
                        catch (...) {
                            if (abort_on_exception_) throw;
                            common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                                log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                            });
                        }
                        return {};
                    } ();
                    if (text_.empty()) {
                        constexpr static auto const suffix_size_ = 12;
                        try { text_ = private_::action("generating wifi access point ssid suffix via SDK ::esp_read_mac", [] () {
                            ::std::array<::std::uint8_t, suffix_size_ / 2> bytes_;
                            auto const &&error_code_ = ::esp_read_mac(bytes_.data(), ::ESP_MAC_WIFI_STA);
                            if (ESP_OK != error_code_) common::sdk::check_or_throw(::std::forward<decltype(error_code_)>(error_code_));
                            auto const &&text_ = ::fmt::format("{:02x}", ::fmt::join(::std::forward<decltype(bytes_)>(bytes_), ""));
                            if (text_.empty() || (suffix_size_ != text_.size())) throw ::std::runtime_error("invalid text returned");
                            if (text_.npos == text_.find_first_not_of("0")) throw ::std::runtime_error("zero mac returned");
                            return ::std::forward<decltype(text_)>(text_);
                        }); }
                        catch (...) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                            log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                        }); }
                        if (text_.empty()) {
                            text_ = private_::action("generating wifi access point ssid suffix via SDK ::esp_random", [] () {
                                #pragma pack(push, 1)
                                union Union_ {
                                    ::std::array<::std::uint8_t, suffix_size_ / 2> bytes;
                                    ::std::array<::std::uint32_t, 2> dwords = {::esp_random(), ::esp_random()};
                                };
                                #pragma pack(pop)
                                auto const &&text_ = ::fmt::format("{:02x}", ::fmt::join(Union_{}.bytes, ""));
                                if (text_.empty() || (suffix_size_ != text_.size())) throw ::std::runtime_error("invalid text returned");
                                return ::std::forward<decltype(text_)>(text_);
                            });
                        }
                        if constexpr (true) {
                            constexpr static auto const separator_ = ::std::string_view{" - "};
                            constexpr static auto const prefix_ = [] () {
                                constexpr auto const separator_size_ = separator_.size();
                                static_assert(max_size_ > separator_size_ + suffix_size_);
                                constexpr auto const max_prefix_size_ = max_size_ - (separator_size_ + suffix_size_);
                                constexpr auto const config_ = private_::trimmer::both(::std::string_view{CONFIG_P5_RWSC_DEFAULT_WIFI_SSID_PREFIX});
                                if (config_.empty() || (max_prefix_size_ >= config_.size())) return config_;
                                return private_::trimmer::right(::std::string_view{config_.data(), max_prefix_size_});
                            } ();
                            if constexpr (! prefix_.empty()) text_ = ::fmt::format("{}{}{}", prefix_, separator_, ::std::forward<decltype(text_)>(text_));
                        }
                        private_::action("writing new generated wifi access point ssid into nvs_flash", [&nvs_handle_, &text_] () {
                            private_::action("SDK ::nvs_set_str (wifi access point ssid)", [&nvs_handle_, &text_] () {
                                common::sdk::check_or_throw(::nvs_set_str(nvs_handle_, nvs_key_, text_.c_str()));
                            });
                            private_::action("checking saved wifi access point ssid in nvs_flash", [
                                &nvs_handle_, source_ = ::std::string_view{text_}
                            ] () {
                                auto const buffer_ = private_::action("SDK ::nvs_get_str (wifi access point ssid)", [&nvs_handle_, &source_] () {
                                    auto size_ = 1 + source_.size();
                                    auto &&buffer_ = ::std::vector<char>(size_);
                                    common::sdk::check_or_throw(::nvs_get_str(nvs_handle_, nvs_key_, buffer_.data(), &size_));
                                    buffer_.back() = 0;
                                    return ::std::forward<decltype(buffer_)>(buffer_);
                                });
                                if (source_ != ::std::string_view{buffer_.data()}) throw ::std::runtime_error{"saved value mismatch"};
                            });
                        });
                    }
                    return ::std::forward<decltype(text_)>(text_);
                });

                log<LogLevel::Info>(::fmt::format("wifi access point ssid: {}", wifi_ap_ssid_, wifi_ap_ssid_));

                ::std::array<common::coro::Future<::std::string_view>, 2> futures_;
                auto task_ = [&futures_] () -> common::coro::Task<::std::string_view> {
                    for (auto &future_: futures_) log<LogLevel::Info>(::fmt::format("[task]: {}", co_await future_));
                    co_return "ok";
                } ();

                task_.start();
                task_.subscribe([&task_] () { log<LogLevel::Info>(::fmt::format("task finished: {}", task_.result())); });

                futures_[1].set_result("bye, world!");
                futures_[0].set_result("hello, world!");

                log<LogLevel::Info>(::fmt::format("task result: {}", task_.result()));

                private_::action("SDK ::esp_netif_init", [] () { common::sdk::check_or_throw(::esp_netif_init()); });
                finally([] () { private_::action(
                    "SDK ::esp_netif_deinit", [] () { common::sdk::check_or_throw(::esp_netif_deinit()); }
                ); });

                private_::action("SDK ::esp_event_loop_create_default", [] () { common::sdk::check_or_throw(::esp_event_loop_create_default()); });
                finally([] () { private_::action(
                    "SDK ::esp_event_loop_delete_default", [] () { common::sdk::check_or_throw(::esp_event_loop_delete_default()); }
                ); });

#if 0
#ifdef CONFIG_P5_RWSC_ENABLE_WIFI
                if constexpr (static_cast<bool>(CONFIG_P5_RWSC_ENABLE_WIFI)) {
                    common::sdk::check_or_throw(::esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
                    common::sdk::check_or_throw(::esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
                }
#endif // CONFIG_P5_RWSC_ENABLE_WIFI
#ifdef CONFIG_P5_RWSC_ENABLE_ETHERNET
                if constexpr (static_cast<bool>(CONFIG_P5_RWSC_ENABLE_WIFI)) {
                    common::sdk::check_or_throw(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
                    common::sdk::check_or_throw(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
                }
#endif // CONFIG_P5_RWSC_ENABLE_ETHERNET

        /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
        * Read "Establishing Wi-Fi or Ethernet Connection" section in
        * examples/protocols/README.md for more information about this function.
        */
                common::sdk::check_or_throw(example_connect());
#endif
            },
            [&exceptions_] (auto &&exception) {
                exceptions_++;
                common::exception_handling::walk(::std::forward<decltype(exception)>(exception), [] (auto &&exception) {
                    log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                });
            }
        );
        if (0 < exceptions_) throw ::std::runtime_error{::fmt::format(
            "caught {} exceptions in finally state", ::std::forward<decltype(exceptions_)>(exceptions_)
        )};
    }

} // namespace p5::rswc::implementation_::dirty_
