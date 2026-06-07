#include "WiFiDriver.h"

#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include <cstring>

static constexpr const char* TAG = "WiFiDriver";

esp_err_t WiFiDriver::init(const Config& config) {
    _config = config;
    apNetif = esp_netif_create_default_wifi_ap();
    staNetif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t initconfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&initconfig), TAG, "Failed to initialize WiFi driver");

    ESP_RETURN_ON_ERROR(switchToApSta(), TAG, "Failed to switch to APSTA mode");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start WiFi driver");
    return ESP_OK;
}

esp_err_t WiFiDriver::startAp() {
    ESP_RETURN_ON_ERROR(esp_netif_dhcps_stop(apNetif), TAG, "Failed to stop DHCP server");

    esp_netif_ip_info_t ipInfo = {};
    ipInfo.ip.addr = esp_ip4addr_aton(_config.apIp);
    ipInfo.gw.addr = esp_ip4addr_aton(_config.apGateway);
    ipInfo.netmask.addr = esp_ip4addr_aton(_config.apNetmask);
    ESP_RETURN_ON_ERROR(esp_netif_set_ip_info(apNetif, &ipInfo), TAG, "Failed to set IP info");
    ESP_RETURN_ON_ERROR(esp_netif_dhcps_start(apNetif), TAG, "Failed to start DHCP server");

    wifi_config_t wifiConfig = {};
    memcpy(wifiConfig.ap.ssid, _config.apSsid, strlen(_config.apSsid));
    memcpy(wifiConfig.ap.password, _config.apPassword, strlen(_config.apPassword));
    wifiConfig.ap.ssid_len = static_cast<uint8_t>(strlen(_config.apSsid));
    wifiConfig.ap.channel = _config.apChannel;
    wifiConfig.ap.max_connection = _config.apMaxConn;
    wifiConfig.ap.authmode = strlen(_config.apPassword) == 0 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
    ESP_LOGI(TAG, "Starting AP with SSID '%s' IP: %s", _config.apSsid, _config.apIp);
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig), TAG, "Failed to set AP config");
    return ESP_OK;
}

esp_err_t WiFiDriver::switchToApSta() {
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_APSTA), TAG, "Failed to set WiFi mode");
    ESP_RETURN_ON_ERROR(startAp(), TAG, "Failed to start AP");
    ESP_LOGI(TAG, "Mode: APSTA. portal at http://%s", _config.apIp);
    return ESP_OK;
}

esp_err_t WiFiDriver::stopAp() {
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set WiFi mode");
    ESP_LOGI(TAG, "Mode: STA. AP stopped");
    return ESP_OK;
}

esp_err_t WiFiDriver::connect(const char* ssid, const char* password) {
    wifi_config_t wifiConfig = {};
    memcpy(wifiConfig.sta.ssid, ssid, strlen(ssid));
    memcpy(wifiConfig.sta.password, password, strlen(password));
    wifiConfig.sta.threshold.authmode = strlen(password) == 0 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig), TAG, "Failed to set STA config");
    ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "Failed to connect to WiFi");
    return ESP_OK;
}

esp_err_t WiFiDriver::disconnect() {
    ESP_RETURN_ON_ERROR(esp_wifi_disconnect(), TAG, "Failed to disconnect from WiFi");
    return ESP_OK;
}

esp_err_t WiFiDriver::scan(wifi_ap_record_t* records, uint16_t* found) {
    wifi_scan_config_t scanConfig = {
        .ssid = nullptr,
        .bssid = nullptr,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {.active = {.min = 100, .max = 300}},
    };

    uint16_t available = 0;
    ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_num(&available), TAG, "Failed to get AP count");
    ESP_LOGI(TAG, "Available SSID %d", available);
    ESP_RETURN_ON_ERROR(esp_wifi_scan_start(&scanConfig, true), TAG, "Failed to start WiFi scan");
    ESP_RETURN_ON_ERROR(esp_wifi_scan_get_ap_records(found, records), TAG, "Failed to get scanned AP records");
    return ESP_OK;
}
