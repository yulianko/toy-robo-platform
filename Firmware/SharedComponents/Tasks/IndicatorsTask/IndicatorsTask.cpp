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

void IndicatorsTask::init(RgbPlayer& rgbPlayer,
                          SoundPlayer& soundPlayer,
                          QueueHandle_t commandQueue,
                          QueueHandle_t robotEventQueue) {
    _rgbPlayer = &rgbPlayer;
    _soundPlayer = &soundPlayer;
    _commandQueue = commandQueue;
    _robotEventQueue = robotEventQueue;
    _initialized = true;

    ESP_LOGI(TAG, "Initialized with RGB and Sound players");
}

void IndicatorsTask::run() {
    assert(_rgbPlayer && _soundPlayer && _commandQueue && _robotEventQueue);
    ESP_LOGI(TAG, "Ready to receive commands");

    while (true) {
        const uint32_t now = nowMs();

        IndicatorCommand command;
        if (xQueueReceive(_commandQueue, &command, 0) == pdTRUE) {
            handleCommand(command, now);
        }

        ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->tick(now), TAG, "rgbPlayer tick failed");
        ESP_RETURN_VOID_ON_ERROR(_soundPlayer->tick(now), TAG, "soundPlayer tick failed");

        checkDone();

        vTaskDelay(pdMS_TO_TICKS(TICK_INTERVAL_MS));
    }
}

void IndicatorsTask::handleCommand(const IndicatorCommand& command, uint32_t now) {
    ESP_LOGI(TAG, "Received command type: %d", static_cast<int>(command.type));

    switch (command.type) {
        case IndicatorCommand::Type::StartRgb:
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->stop(), TAG, "rgbPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->start(command.rgbAnimation, now), TAG, "rgbPlayer start failed");
            _shouldNotifyDone = true;
            ESP_LOGI(TAG, "Started RGB animation: %s", command.rgbAnimation.name);
            break;

        case IndicatorCommand::Type::StartSound:
            ESP_RETURN_VOID_ON_ERROR(_soundPlayer->stop(), TAG, "soundPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_soundPlayer->start(command.soundAnimation, now), TAG, "soundPlayer start failed");
            _shouldNotifyDone = true;
            ESP_LOGI(TAG, "Started sound animation: %s", command.soundAnimation.name);
            break;

        case IndicatorCommand::Type::StartBoth:
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->stop(), TAG, "rgbPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_soundPlayer->stop(), TAG, "soundPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->start(command.rgbAnimation, now), TAG, "rgbPlayer start failed");
            ESP_RETURN_VOID_ON_ERROR(_soundPlayer->start(command.soundAnimation, now), TAG, "soundPlayer start failed");
            _shouldNotifyDone = true;
            ESP_LOGI(TAG,
                     "Started both animations - RGB: %s, Sound: %s",
                     command.rgbAnimation.name,
                     command.soundAnimation.name);
            break;

        case IndicatorCommand::Type::Stop:
            ESP_RETURN_VOID_ON_ERROR(_rgbPlayer->stop(), TAG, "rgbPlayer stop failed");
            ESP_RETURN_VOID_ON_ERROR(_soundPlayer->stop(), TAG, "soundPlayer stop failed");
            _shouldNotifyDone = false;
            ESP_LOGI(TAG, "Stopped all animations");
            break;
    }
}

void IndicatorsTask::checkDone() {
    if (_shouldNotifyDone && _rgbPlayer->isDone() && _soundPlayer->isDone()) {
        _shouldNotifyDone = false;

        RobotEvent event(RobotEvent::Type::INDICATORS_ANIMATION_DONE);
        if (xQueueSend(_robotEventQueue, &event, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to send INDICATORS_ANIMATION_DONE event");
        } else {
            ESP_LOGI(TAG, "All animations completed, sent INDICATORS_ANIMATION_DONE event");
        }
    }
}
