#pragma once

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "MotionCommand.h"
#include "MotionSequence.h"

class MotionProxy {
  public:
    explicit MotionProxy(QueueHandle_t commandQueue) : _queue(commandQueue) {
    }

    void play(const MotionSequence& sequence) {
        send(MotionCommand::play(sequence));
    }

    void stop() {
        send(MotionCommand::stop());
    }

    void brake() {
        send(MotionCommand::brake());
    }

  private:
    void send(const MotionCommand& cmd) {
        if (!_queue) {
            ESP_LOGE("MotionProxy", "queue not initialized");
            return;
        }

        if (xQueueSend(_queue, &cmd, 0) != pdTRUE) {
            ESP_LOGW("MotionProxy", "motionQueue full - command dropped");
        }
    }

    QueueHandle_t _queue = nullptr;
};
