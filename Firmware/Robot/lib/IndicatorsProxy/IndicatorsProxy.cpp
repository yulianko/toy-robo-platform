#include "IndicatorsProxy.h"

#include <esp_log.h>

static const char* TAG = "IndicatorsProxy";

IndicatorsProxy::IndicatorsProxy(QueueHandle_t commandQueue) : _commandQueue(commandQueue) {
}

void IndicatorsProxy::start(const RgbAnimation& animation) {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return;
    }

    IndicatorCommand cmd = IndicatorCommand::startRgb(animation);
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send RGB start command");
    }
}

void IndicatorsProxy::start(const SoundAnimation& animation) {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return;
    }

    IndicatorCommand cmd = IndicatorCommand::startSound(animation);
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send sound start command");
    }
}

void IndicatorsProxy::start(const RgbAnimation& rgbAnimation, const SoundAnimation& soundAnimation) {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return;
    }

    IndicatorCommand cmd = IndicatorCommand::startBoth(rgbAnimation, soundAnimation);
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send combined start command");
    }
}

void IndicatorsProxy::stop() {
    if (!_commandQueue) {
        ESP_LOGW(TAG, "Command queue not set");
        return;
    }

    IndicatorCommand cmd = IndicatorCommand::stop();
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send stop command");
    }
}
