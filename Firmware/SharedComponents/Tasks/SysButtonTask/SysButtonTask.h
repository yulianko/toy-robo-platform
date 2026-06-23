#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "BaseTask.h"
#include "IsrButton.h"
#include "RobotEvent.h"

class SysButtonTask : public BaseTask {
  public:
    static SysButtonTask& instance();

    void init(IsrButton& button, QueueHandle_t buttonQueue, QueueHandle_t robotEventQueue);

  protected:
    const char* getTaskName() const override { return "SysButtonTask"; }
    bool isInitialized() const override { return _button && _buttonQueue && _robotEventQueue; }
    void run() override;

  private:
    SysButtonTask() = default;
    SysButtonTask(const SysButtonTask&) = delete;
    SysButtonTask& operator=(const SysButtonTask&) = delete;

    IsrButton* _button = nullptr;  // Reference, not owner
    QueueHandle_t _buttonQueue = nullptr;
    QueueHandle_t _robotEventQueue = nullptr;
};
