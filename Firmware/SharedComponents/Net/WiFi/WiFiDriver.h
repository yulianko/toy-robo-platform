#pragma once
#include <esp_check.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include <cstdint>

class WiFiDriver {
  public:
    struct Config {
        const char* apSsid = "MyNet";
        const char* apPassword = "_1234567";
        int apChannel = 1;
        int apMaxConn = 4;
        const char* apIp = "192.168.4.1";
        const char* apGateway = "192.168.4.1";
        const char* apNetmask = "255.255.255.0";
    };

    WiFiDriver() = default;
    esp_err_t init(const Config& config);

    esp_err_t startAp();
    esp_err_t stopAp();
    esp_err_t switchToApSta();

    esp_err_t connect(const char* ssid, const char* password);
    esp_err_t disconnect();
    esp_err_t scan(wifi_ap_record_t* records, uint16_t* found);

  private:
    Config _config{};
    esp_netif_t* apNetif = nullptr;
    esp_netif_t* staNetif = nullptr;
};
