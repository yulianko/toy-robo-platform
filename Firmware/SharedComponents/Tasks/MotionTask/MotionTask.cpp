#include "MotionTask.h"

#include <esp_log.h>

#include "RobotEvent.h"

static const char* TAG = "MotionTask";

void MotionTask::init(MotionPlayer& player, QueueHandle_t motionQueue, QueueHandle_t robotEventQueue) {
    _motionQueue = motionQueue;
    _robotEventQueue = robotEventQueue;
    _player = &player;

    ESP_LOGI(TAG, "initialized");
}

void MotionTask::run() {
    ESP_LOGI(TAG, "started");

    MotionCommand cmd;
    bool wasPlaying = false;

    while (true) {
        const TickType_t wait = _player->isPlaying() ? 0 : pdMS_TO_TICKS(TICK_MS);

        if (xQueueReceive(_motionQueue, &cmd, wait) == pdTRUE) {
            handleCommand(cmd);
        }

        _player->tick();

        const bool nowPlaying = _player->isPlaying();
        if (wasPlaying && !nowPlaying) {
            onPlayerDone();
        }

        wasPlaying = nowPlaying;
        if (nowPlaying) {
            vTaskDelay(pdMS_TO_TICKS(TICK_MS));
        }
    }
}

void MotionTask::handleCommand(const MotionCommand& cmd) {
    esp_err_t err = ESP_OK;

    switch (cmd.type) {
        case MotionCommand::Type::Play:
            err = _player->play(cmd.sequence);
            break;
        case MotionCommand::Type::Stop:
            err = _player->stop();
            break;
        case MotionCommand::Type::Brake:
            err = _player->brake();
            break;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "command failed: %s", esp_err_to_name(err));
    }
}

void MotionTask::onPlayerDone() {
    RobotEvent event(RobotEvent::Type::MOTION_DONE, 0);
    if (xQueueSend(_robotEventQueue, &event, 0) != pdTRUE) {
        ESP_LOGW(TAG, "robotEventQueue full - MOTION_DONE dropped");
    }
}
