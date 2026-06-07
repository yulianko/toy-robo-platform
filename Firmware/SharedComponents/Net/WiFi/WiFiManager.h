#pragma once
#include <esp_check.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "BaseTask.h"
#include "WiFiCredentialsStorage.h"
#include "WiFiDriver.h"

static constexpr int MAX_SCAN_RESULTS = 20;  // скільки просимо у драйвера

static constexpr EventBits_t BIT_STA_CONNECTED = BIT0;
static constexpr EventBits_t BIT_STA_DISCONNECTED = BIT1;

enum class WmMsgType : uint8_t { CONNECT, SCAN, RESET_CONFIG };

struct WmMsg {
    WmMsgType type;
    char ssid[33] = {};
    char password[65] = {};
};

struct ScanResult {
    char ssid[33];
    int8_t rssi;
    bool open;
    uint8_t channel;
};

struct ScanResultSet {
    ScanResult networks[20];
    int count = 0;
    bool ready = false;
};

class WiFiManager : public BaseTask {
  public:
    struct Config {
        int scanRssiThreshold = -85;   // dBm
        int scanMaxDisplay = 10;       // how many to present to user
        int connectTimeoutMs = 15000;  // ms
        WiFiDriver::Config driver{};
    };

    static const size_t IP_ADDR_STR_LEN = 16;
    static const size_t SSID_STR_LEN = 33;

    static WiFiManager& instance() {
        static WiFiManager instance;
        return instance;
    }

    void init(const Config& config, QueueHandle_t commandQueue);

    bool post(const WmMsg& msg, TickType_t wait = pdMS_TO_TICKS(100));
    bool getScanResults(ScanResultSet& out);
    bool isConnected(char* ssidOut = nullptr, size_t ssidLen = 0, char* ipOut = nullptr, size_t ipLen = 0);

    static void eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);

  protected:
    const char* getTaskName() const override {
        return "WiFi_Mngr";
    }

    uint32_t getStackSize() const override {
        return 8192;
    }

    bool isInitialized() const override {
        return _initialized;
    }

    void run() override;

  private:
    WiFiManager() = default;

    void doScan();
    void doConnect(const char* ssid, const char* password);

    WifiCredentialsStorage _storage;

    WiFiDriver _driver;
    char _connectedIp[IP_ADDR_STR_LEN] = {};
    char _connectedSsid[SSID_STR_LEN] = {};

    Config _config;
    bool _initialized = false;

    QueueHandle_t _commandsQueue = nullptr;
    EventGroupHandle_t _connectionEventGroup = nullptr;
    SemaphoreHandle_t _scanMutex = nullptr;

    wifi_ap_record_t rawScanResults[MAX_SCAN_RESULTS];
    ScanResultSet scanResults;
};
