#include <cstdint>

#include <utility>
#include <exception>
#include <stdexcept>

#include <fmt/format.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_mac.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_random.h>

#include <p5/rswc/implementation_/platform.hpp>
#include <p5/rswc/implementation_/romfs.hpp>
#include <p5/rswc/implementation_/log.hpp>
#include <p5/rswc/implementation_/common.hpp>
#include <p5/rswc/implementation_/platform/modules/event_loop.hpp>

#include "nvs_flash_keys.hpp"

#include "main_task.hpp"


namespace p5::rswc::implementation_::main_task {
namespace private_::sdk {

    enum class Events: ::std::int32_t { Test };

    inline constexpr static ::esp_event_base_t event_base() noexcept(true) {
        return "p5.rswc.main_task";
    }

} // namespace private_::sdk

    common::coro::Task<void> make() noexcept(false) {
        return [] () -> common::coro::Task<void> {
            auto finally_ = common::utils::with_finally(
                [] (auto &&finally) { return ::std::forward<decltype(finally)>(finally); },
                [] (auto &&exception) { common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                    log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                }); }
            );

            finally_([] () { log<LogLevel::Info>(::fmt::format(
                "main task [thread = {}]: about to finish",
                static_cast<void const *>(::xTaskGetCurrentTaskHandle())
            )); });

            if constexpr (true) {
                auto event_loop_ = platform::instance().get_module<platform::modules::EventLoop>();
                if (! event_loop_) throw ::std::logic_error{"event loop pointer is null"};

                log<LogLevel::Info>(::fmt::format(
                    "main task [thread = {}] started, event_loop = [{}, {}]",
                    static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                    static_cast<void const *>(event_loop_.get()),
                    event_loop_.use_count()
                ));
            }

            if constexpr (true) {
                static auto future_ = common::coro::Future<::std::string>{};

                platform::logged_action::execute("initializing event handler", [] () { common::sdk::check_or_throw(::esp_event_handler_register(
                    private_::sdk::event_base(), static_cast<::std::int32_t>(private_::sdk::Events::Test), [] (
                        void *handler_arguments, char const *event_base, ::std::int32_t event_id, void *event_data
                    ) {
                        try {
                            auto &&message_ = ::fmt::format(
                                "{}, {}, {}, {}",
                                ::std::forward<decltype(handler_arguments)>(handler_arguments),
                                ::std::forward<decltype(event_base)>(event_base),
                                ::std::forward<decltype(event_id)>(event_id),
                                ::std::forward<decltype(event_data)>(event_data)
                            );

                            log<LogLevel::Info>(::fmt::format(
                                "main task [thread = {}]: event handler - {}",
                                static_cast<void const *>(::xTaskGetCurrentTaskHandle()), message_
                            ));

                            if (! future_) future_.set_result(::std::forward<decltype(message_)>(message_));
                        }
                        catch (...) { ::std::terminate(); }
                    },
                    nullptr
                )); });

                log<LogLevel::Info>(::fmt::format(
                    "main task [thread = {}]: waiting for future",
                    static_cast<void const *>(::xTaskGetCurrentTaskHandle())
                ));

                co_await future_;

                log<LogLevel::Info>(::fmt::format(
                    "main task [thread = {}]: future received = {}",
                    static_cast<void const *>(::xTaskGetCurrentTaskHandle()),
                    future_.result()
                ));
            }

            platform::logged_action::execute("initializing p5_rswc::romfs module", [] () { romfs::initialize(); });
            finally_([] () { platform::logged_action::execute("deinitializing p5_rswc::romfs module", [] () { romfs::deinitialize(); }); });

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

            auto const nvs_handle_ = platform::logged_action::execute("initializing non-volatile storage", [&finally_] () {
                if constexpr (true) {
                    auto abort_on_exception_ = true;
                    try { platform::logged_action::execute("SDK ::nvs_flash_init", [&abort_on_exception_] () {
                        auto const error_code_ = ::nvs_flash_init();
                        switch (error_code_) {
                        default: break;
                        case ESP_ERR_NVS_NO_FREE_PAGES:
                        case ESP_ERR_NVS_NEW_VERSION_FOUND:
                            abort_on_exception_ = false;
                            break;
                        }
                        common::sdk::check_or_throw(error_code_);
                    }); }
                    catch (...) {
                        if (abort_on_exception_) throw;
                        common::exception_handling::walk(::std::current_exception(), [] (auto &&exception) {
                            log<LogLevel::Warning>(common::exception_handling::details(::std::forward<decltype(exception)>(exception)));
                        });
                        platform::logged_action::execute("SDK ::nvs_flash_erase", [] () { common::sdk::check_or_throw(::nvs_flash_erase()); });
                        platform::logged_action::execute("SDK ::nvs_flash_init", [] () { common::sdk::check_or_throw(::nvs_flash_init()); });
                    }
                }
                finally_([] () { platform::logged_action::execute(
                    "SDK ::nvs_flash_deinit", [] () { common::sdk::check_or_throw(::nvs_flash_deinit()); }
                ); });
                auto const handle_ = platform::logged_action::execute("SDK ::nvs_open", [] () {
                    auto handle_ = static_cast<::nvs_handle_t>(0);
                    common::sdk::check_or_throw(::nvs_open("p5.rswc", ::NVS_READWRITE, &handle_));
                    if (0 == handle_) throw ::std::runtime_error{"zero handle returned"};
                    return handle_;
                });
                finally_([handle_] () { platform::logged_action::execute("SDK ::nvs_close", [&handle_] () { ::nvs_close(handle_); }); });
                return handle_;
            });

            if constexpr (true) {
                auto const nvs_keys_ = nvs_flash_keys::make();

                if constexpr (true) {
                    auto const thread_ = static_cast<void const *>(::xTaskGetCurrentTaskHandle());
                    for (auto const &key_: {
                        "wifi.access_point.ssid",
                        "wifi.access_point.password",
                        "wifi.station.ssid",
                        "wifi.station.password"
                    }) log<LogLevel::Info>(::fmt::format(
                        "main task [thread = {}]: nvs_flash_keys.{} = {}",
                        thread_, key_, nvs_keys_[key_]
                    ));
                }

                auto const wifi_ap_ssid_ = platform::logged_action::execute("generating wifi access point ssid", [&nvs_handle_, &nvs_keys_] () {
                    constexpr static auto const max_size_ = 32;
                    auto const * const nvs_key_ = nvs_keys_["wifi.access_point.ssid"];
                    auto &&text_ = [&nvs_handle_, &nvs_key_] () -> ::std::string {
                        auto abort_on_exception_ = false;
                        try { return platform::logged_action::execute("SDK ::nvs_get_str (wifi access point ssid)", [&nvs_handle_, &nvs_key_, &abort_on_exception_] () -> ::std::string {
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
                        try { text_ = platform::logged_action::execute("generating wifi access point ssid suffix via SDK ::esp_read_mac", [] () {
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
                            text_ = platform::logged_action::execute("generating wifi access point ssid suffix via SDK ::esp_random", [] () {
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
                                constexpr auto const config_ = common::utils::trimmer::both(::std::string_view{CONFIG_P5_RWSC_DEFAULT_WIFI_SSID_PREFIX});
                                if (config_.empty() || (max_prefix_size_ >= config_.size())) return config_;
                                return common::utils::trimmer::right(::std::string_view{config_.data(), max_prefix_size_});
                            } ();
                            if constexpr (! prefix_.empty()) text_ = ::fmt::format("{}{}{}", prefix_, separator_, ::std::forward<decltype(text_)>(text_));
                        }
                        platform::logged_action::execute("writing new generated wifi access point ssid into nvs_flash", [&nvs_handle_, &nvs_key_, &text_] () {
                            platform::logged_action::execute("SDK ::nvs_set_str (wifi access point ssid)", [&nvs_handle_, &nvs_key_, &text_] () {
                                common::sdk::check_or_throw(::nvs_set_str(nvs_handle_, nvs_key_, text_.c_str()));
                            });
                            platform::logged_action::execute("checking saved wifi access point ssid in nvs_flash", [
                                &nvs_handle_, &nvs_key_, source_ = ::std::string_view{text_}
                            ] () {
                                auto const buffer_ = platform::logged_action::execute("SDK ::nvs_get_str (wifi access point ssid)", [&nvs_handle_, &nvs_key_, &source_] () {
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
            }

            co_return;
        } ();
    }

    void post_test_event() noexcept(false) {
        auto const event_loop_ = platform::instance().get_module<platform::modules::EventLoop>();
        if (! event_loop_) throw ::std::logic_error{"event loop is not initialized"};
        common::sdk::check_or_throw(::esp_event_post(
            private_::sdk::event_base(), static_cast<::std::int32_t>(private_::sdk::Events::Test),
            nullptr, 0, portMAX_DELAY
        ));
    }

} // namespace p5::rswc::implementation_::main_task
