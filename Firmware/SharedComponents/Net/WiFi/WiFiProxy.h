#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "WiFiManager.h"

class WiFiProxy {
  public:
    static constexpr size_t SSID_STR_LEN = WiFiManager::SSID_STR_LEN;
    static constexpr size_t IP_ADDR_STR_LEN = WiFiManager::IP_ADDR_STR_LEN;

    WiFiProxy(WiFiManager* manager, QueueHandle_t commandQueue);

    void postScan();
    bool postConnect(const char* ssid, const char* password);
    bool postReset();

    bool getScanResults(ScanResultSet& out);
    bool isConnected(char* ssidOut = nullptr, size_t ssidLen = 0, char* ipOut = nullptr, size_t ipLen = 0);

  private:
    WiFiManager* _manager = nullptr;
    QueueHandle_t _commandQueue = nullptr;
};
