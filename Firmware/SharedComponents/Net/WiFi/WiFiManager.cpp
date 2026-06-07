#include "WiFiManager.h"

#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

static const char* TAG = "WiFiManager";

void WiFiManager::init(const Config& config, QueueHandle_t commandQueue) {
    _config = config;
    _commandsQueue = commandQueue;
    _connectionEventGroup = xEventGroupCreate();
    _scanMutex = xSemaphoreCreateMutex();

    _driver.init(config.driver);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiManager::eventHandler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiManager::eventHandler, this));

    if (_commandsQueue) {
        WmMsg firstMsg;
        WifiCredentials creds;
        if (_storage.load(creds)) {
            ESP_LOGI(TAG, "Saved credentials found. Connecting to '%s'", creds.ssid);
            firstMsg.type = WmMsgType::CONNECT;
            memcpy(firstMsg.ssid, creds.ssid, sizeof(creds.ssid));
            memcpy(firstMsg.password, creds.password, sizeof(creds.password));
        } else {
            ESP_LOGI(TAG, "No saved credentials found. Starting config portal.");
            firstMsg.type = WmMsgType::SCAN;
        }
        xQueueSend(_commandsQueue, &firstMsg, 0);
    }

    _initialized = true;
    ESP_LOGI(TAG, "Initialized");
}

bool WiFiManager::post(const WmMsg& msg, TickType_t wait) {
    if (!_initialized || _commandsQueue == nullptr) {
        ESP_LOGW(TAG, "Unable to post WiFi command, task not initialized or queue missing");
        return false;
    }

    if (xQueueSend(_commandsQueue, &msg, wait) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to enqueue WiFi command");
        return false;
    }

    return true;
}

bool WiFiManager::getScanResults(ScanResultSet& out) {
    if (_scanMutex == nullptr) {
        ESP_LOGW(TAG, "Scan results mutex not initialized");
        return false;
    }

    if (xSemaphoreTake(_scanMutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to lock scan results mutex");
        return false;
    }

    out = scanResults;
    xSemaphoreGive(_scanMutex);
    return true;
}

bool WiFiManager::isConnected(char* ssidOut, size_t ssidLen, char* ipOut, size_t ipLen) {
    if (_connectionEventGroup == nullptr) {
        return false;
    }

    bool connected = (xEventGroupGetBits(_connectionEventGroup) & BIT_STA_CONNECTED) != 0;
    if (connected && ssidOut && ssidLen > 0) {
        strncpy(ssidOut, _connectedSsid, ssidLen - 1);
        ssidOut[ssidLen - 1] = '\0';
    }

    if (connected && ipOut && ipLen > 0) {
        strncpy(ipOut, _connectedIp, ipLen - 1);
        ipOut[ipLen - 1] = '\0';
    }

    return connected;
}

void WiFiManager::eventHandler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    WiFiManager* self = static_cast<WiFiManager*>(arg);

    if (base == WIFI_EVENT) {
        switch (id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "AP started");
                break;
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* e = static_cast<wifi_event_ap_staconnected_t*>(data);
                ESP_LOGI(TAG, "Client joined AP. MAC: " MACSTR, MAC2STR(e->mac));
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* e = static_cast<wifi_event_ap_stadisconnected_t*>(data);
                ESP_LOGI(TAG, "Client left AP. MAC: " MACSTR, MAC2STR(e->mac));
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* e = static_cast<wifi_event_sta_disconnected_t*>(data);
                ESP_LOGW(TAG, "STA disconnected. Reason: %d", e->reason);
                xEventGroupSetBits(self->_connectionEventGroup, BIT_STA_DISCONNECTED);
                break;
            }
            default:
                break;
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* e = static_cast<ip_event_got_ip_t*>(data);
        std::snprintf(self->_connectedIp, sizeof(self->_connectedIp), IPSTR, IP2STR(&e->ip_info.ip));
        ESP_LOGI(TAG, "STA got IP: %s", self->_connectedIp);
        xEventGroupSetBits(self->_connectionEventGroup, BIT_STA_CONNECTED);
    }
}

void WiFiManager::doScan() {
    ESP_LOGI(TAG, "Scanning...");
    ESP_LOGI(TAG, "Stack HWM: %d bytes free", uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t));

    uint16_t found = MAX_SCAN_RESULTS;
    if (_driver.scan(rawScanResults, &found) != ESP_OK) {
        ESP_LOGE(TAG, "Scan failed");
        return;
    }

    ESP_LOGI(TAG, "After internal scan");

    std::sort(rawScanResults, rawScanResults + found, [](const wifi_ap_record_t& a, const wifi_ap_record_t& b) {
        return a.rssi > b.rssi;
    });

    int kept = 0;
    if (xSemaphoreTake(_scanMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        for (int i = 0; i < static_cast<int>(found) && kept < _config.scanMaxDisplay; ++i) {
            if (rawScanResults[i].rssi < _config.scanRssiThreshold) {
                break;
            }
            memcpy(scanResults.networks[kept].ssid, rawScanResults[i].ssid, sizeof(scanResults.networks[kept].ssid));
            scanResults.networks[kept].rssi = rawScanResults[i].rssi;
            scanResults.networks[kept].open = (rawScanResults[i].authmode == WIFI_AUTH_OPEN);
            scanResults.networks[kept].channel = rawScanResults[i].primary;
            ++kept;
        }
        scanResults.count = kept;
        scanResults.ready = true;
        xSemaphoreGive(_scanMutex);
    } else {
        ESP_LOGW(TAG, "Failed to lock scan results mutex");
    }

    ESP_LOGI(TAG,
             "Scan complete. %d found, %d shown (threshold %d dBm, max %d)",
             found,
             kept,
             _config.scanRssiThreshold,
             _config.scanMaxDisplay);
}

void WiFiManager::doConnect(const char* ssid, const char* password) {
    ESP_LOGI(TAG, "Connecting to SSID: '%s'", ssid);

    xEventGroupClearBits(_connectionEventGroup, BIT_STA_CONNECTED | BIT_STA_DISCONNECTED);

    ESP_ERROR_CHECK(_driver.connect(ssid, password));

    EventBits_t bits = xEventGroupWaitBits(_connectionEventGroup,
                                           BIT_STA_CONNECTED | BIT_STA_DISCONNECTED,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(_config.connectTimeoutMs));

    if (bits & BIT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Connected to '%s'. Saving credentials", ssid);
        strncpy(_connectedSsid, ssid, sizeof(_connectedSsid) - 1);
        _connectedSsid[sizeof(_connectedSsid) - 1] = '\0';
        _storage.save(ssid, password);
        _driver.stopAp();
    } else if (bits & BIT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Connection to '%s' failed. Wrong password or AP unreachable", ssid);
    } else {
        ESP_LOGW(TAG, "Connection to '%s' timed out after %dms", ssid, _config.connectTimeoutMs);
        ESP_ERROR_CHECK(_driver.disconnect());
    }
}

void WiFiManager::run() {
    WmMsg msg;
    ESP_LOGI(TAG, "WiFi manager task running");
    while (true) {
        if (xQueueReceive(_commandsQueue, &msg, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        switch (msg.type) {
            case WmMsgType::SCAN:
                doScan();
                break;
            case WmMsgType::CONNECT:
                doConnect(msg.ssid, msg.password);
                break;
            case WmMsgType::RESET_CONFIG:
                ESP_LOGI(TAG, "Resetting WiFi configuration");
                _storage.clear();
                ESP_ERROR_CHECK(_driver.disconnect());
                _connectedSsid[0] = '\0';
                _connectedIp[0] = '\0';
                xEventGroupClearBits(_connectionEventGroup, BIT_STA_CONNECTED);
                xEventGroupSetBits(_connectionEventGroup, BIT_STA_DISCONNECTED);
                ESP_ERROR_CHECK(_driver.switchToApSta());
                break;
        }
    }
}
