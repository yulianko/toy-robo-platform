#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "IsrButton.h"
#include "RobotEvent.h"

class PushButtonTask {
  public:
    static PushButtonTask& instance();

    void init(IsrButton& button, QueueHandle_t buttonQueue, QueueHandle_t robotEventQueue);
    void start(UBaseType_t priority);
    void stop();

  private:
    static constexpr uint32_t TASK_STACK = 4096;

    PushButtonTask();
    PushButtonTask(const PushButtonTask&) = delete;
    PushButtonTask& operator=(const PushButtonTask&) = delete;

    static void taskFn(void* arg);
    void run();

    IsrButton* _button = nullptr;  // Reference, not owner
    QueueHandle_t _buttonQueue = nullptr;
    QueueHandle_t _robotEventQueue = nullptr;
    TaskHandle_t _taskHandle = nullptr;
};
