#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class BaseTask {
  public:
    virtual ~BaseTask() = default;

    void start(UBaseType_t priority);

  protected:
    BaseTask() = default;
    BaseTask(const BaseTask&) = delete;
    BaseTask& operator=(const BaseTask&) = delete;

    virtual const char* getTaskName() const = 0;
    virtual uint32_t getStackSize() const { return 4096; }
    virtual bool isInitialized() const = 0;
    virtual void run() = 0;

  private:
    static void taskWrapper(void* arg);

    TaskHandle_t _taskHandle = nullptr;
};