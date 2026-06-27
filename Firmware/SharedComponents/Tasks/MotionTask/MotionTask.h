#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "BaseTask.h"
#include "Drivetrain.h"
#include "MotionCommand.h"
#include "MotionPlayer.h"

class MotionTask : public BaseTask {
  public:
    static MotionTask& instance() {
        static MotionTask inst;
        return inst;
    }

    void init(MotionPlayer& player, QueueHandle_t motionQueue, QueueHandle_t robotEventQueue);

  protected:
    const char* getTaskName() const override {
        return "MotionTask";
    }

    uint32_t getStackSize() const override {
        return 3072;
    }

    bool isInitialized() const override {
        return _player && _motionQueue && _robotEventQueue;
    }

    void run() override;

  private:
    static constexpr uint32_t TICK_MS = 20;

    void onPlayerDone();
    void handleCommand(const MotionCommand& cmd);

    MotionPlayer* _player = nullptr;
    QueueHandle_t _motionQueue = nullptr;
    QueueHandle_t _robotEventQueue = nullptr;
};
