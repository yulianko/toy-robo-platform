#include "IndicatorsTask.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_timer.h>

#include <cassert>

static const char* TAG = "IndicatorsTask";
static constexpr TickType_t TICK_INTERVAL_MS = 20;

static uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

void IndicatorsTask::init(RgbPlayer& rgbPlayer, QueueHandle_t commandQueue, QueueHandle_t robotEventQueue) {
    _rgbPlayer = &rgbPlayer;
    _commandQueue = commandQueue;
    _robotEventQueue = robotEventQueue;
    _initialized = true;

    ESP_LOGI(TAG, "Initialized");
}

void IndicatorsTask::run() {
    assert(_rgbPlayer && _commandQueue && _robotEventQueue);
    ESP_LOGI(TAG, "Ready to receive commands");

    while (true) {
        const uint32_t now = nowMs();

        IndicatorCommand command;
        if (xQueueReceive(_commandQueue, &command, 0) == pdTRUE) {
            handleCommand(command, now);
        }

        _rgbPlayer->tick(now);

        checkDone();

        vTaskDelay(pdMS_TO_TICKS(TICK_INTERVAL_MS));
    }
}

void IndicatorsTask::handleCommand(const IndicatorCommand& command, uint32_t now) {
    ESP_LOGI(TAG, "Received command type: %d", static_cast<int>(command.type));

    switch (command.type) {
        case IndicatorCommand::Type::Start:
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->stop(), TAG, "rgbPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->start(command.animation, now), TAG, "rgbPlayer start failed");
            _shouldNotifyDone = true;
            ESP_LOGI(TAG, "Started animation: %s", command.animation.name);

            break;

        case IndicatorCommand::Type::Stop:
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->stop(), TAG, "rgbPlayer stop failed");
            _shouldNotifyDone = false;
            ESP_LOGI(TAG, "Stopped animation");
            break;
    }
}

void IndicatorsTask::checkDone() {
    if (_shouldNotifyDone && _rgbPlayer->isDone()) {
        _shouldNotifyDone = false;

        RobotEvent event(RobotEvent::Type::INDICATORS_ANIMATION_DONE);
        if (xQueueSend(_robotEventQueue, &event, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to send INDICATORS_ANIMATION_DONE event");
        } else {
            ESP_LOGI(TAG, "Animation completed, sent INDICATORS_ANIMATION_DONE event");
        }
    }
}
