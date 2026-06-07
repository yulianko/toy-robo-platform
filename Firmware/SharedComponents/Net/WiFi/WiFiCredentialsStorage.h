#pragma once

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include <cstring>

struct WifiCredentials {
    static constexpr size_t SSID_MAX_LEN = 33;
    static constexpr size_t PASS_MAX_LEN = 65;

    char ssid[SSID_MAX_LEN] = {};
    char password[PASS_MAX_LEN] = {};
};

class WifiCredentialsStorage {
  public:
    WifiCredentialsStorage(const char* ns = "wifi_creds");

    bool load(WifiCredentials& out) const;
    bool save(const char* ssid, const char* password);
    bool clear();

  private:
    const char* _ns;
    static constexpr const char* _TAG = "WifiStorage";
    static constexpr const char* _KEY_SSID = "ssid";
    static constexpr const char* _KEY_PASS = "password";
};
