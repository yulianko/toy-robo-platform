#include "WiFiCredentialsStorage.h"

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include <cstring>

static constexpr const char* TAG = "WifiCredentialsStorage";

WifiCredentialsStorage::WifiCredentialsStorage(const char* ns) : _ns(ns) {
}

bool WifiCredentialsStorage::load(WifiCredentials& out) const {
    nvs_handle_t handle;
    if (nvs_open(_ns, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }

    size_t ssidLen = sizeof(out.ssid);
    size_t passLen = sizeof(out.password);

    bool ok = (nvs_get_str(handle, _KEY_SSID, out.ssid, &ssidLen) == ESP_OK) &&
              (nvs_get_str(handle, _KEY_PASS, out.password, &passLen) == ESP_OK) && (std::strlen(out.ssid) > 0);

    nvs_close(handle);

    if (ok) {
        ESP_LOGI(TAG, "Loaded credentials. SSID: '%s'", out.ssid);
    } else {
        ESP_LOGI(TAG, "No valid credentials found in NVS");
    }
    return ok;
}

bool WifiCredentialsStorage::save(const char* ssid, const char* password) {
    nvs_handle_t handle;
    if (nvs_open(_ns, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = (nvs_set_str(handle, _KEY_SSID, ssid) == ESP_OK) &&
              (nvs_set_str(handle, _KEY_PASS, password) == ESP_OK) && (nvs_commit(handle) == ESP_OK);

    nvs_close(handle);

    if (ok) {
        ESP_LOGI(TAG, "Credentials saved. SSID: '%s'", ssid);
    } else {
        ESP_LOGE(TAG, "Failed to save credentials to NVS");
    }
    return ok;
}

bool WifiCredentialsStorage::clear() {
    nvs_handle_t handle;
    if (nvs_open(_ns, NVS_READWRITE, &handle) != ESP_OK) return false;

    bool ok = (nvs_erase_all(handle) == ESP_OK) && (nvs_commit(handle) == ESP_OK);
    nvs_close(handle);

    if (ok) {
        ESP_LOGW(TAG, "Credentials erased from storage.");
    }
    return ok;
}
