#include "WiFiProxy.h"

#include <esp_log.h>
#include <cstring>

static const char* TAG = "WiFiProxy";

WiFiProxy::WiFiProxy(WiFiManager* manager, QueueHandle_t commandQueue)
    : _manager(manager), _commandQueue(commandQueue) {
}

void WiFiProxy::postScan() {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return;
    }

    WmMsg msg;
    msg.type = WmMsgType::SCAN;
    if (xQueueSend(_commandQueue, &msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send SCAN command");
    }
}

bool WiFiProxy::postConnect(const char* ssid, const char* password) {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return false;
    }

    WmMsg msg;
    msg.type = WmMsgType::CONNECT;
    strncpy(msg.ssid, ssid ? ssid : "", sizeof(msg.ssid) - 1);
    msg.ssid[sizeof(msg.ssid) - 1] = '\0';
    strncpy(msg.password, password ? password : "", sizeof(msg.password) - 1);
    msg.password[sizeof(msg.password) - 1] = '\0';

    if (xQueueSend(_commandQueue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send CONNECT command");
        return false;
    }

    return true;
}

bool WiFiProxy::postReset() {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return false;
    }

    WmMsg msg;
    msg.type = WmMsgType::RESET_CONFIG;
    if (xQueueSend(_commandQueue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send RESET_CONFIG command");
        return false;
    }

    return true;
}

bool WiFiProxy::getScanResults(ScanResultSet& out) {
    if (!_manager) return false;
    return _manager->getScanResults(out);
}

bool WiFiProxy::isConnected(char* ssidOut, size_t ssidLen, char* ipOut, size_t ipLen) {
    if (!_manager) return false;
    return _manager->isConnected(ssidOut, ssidLen, ipOut, ipLen);
}
