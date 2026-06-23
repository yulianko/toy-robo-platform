#include "BaseTask.h"

#include <esp_log.h>

static const char* TAG = "BaseTask";

void BaseTask::start(UBaseType_t priority) {
    if (_taskHandle != nullptr) {
        ESP_LOGW(TAG, "Task %s already running", getTaskName());
        return;
    }

    if (!isInitialized()) {
        ESP_LOGE(TAG, "Task %s not initialized - call init() first", getTaskName());
        return;
    }

    BaseType_t result = xTaskCreate(
        taskWrapper,
        getTaskName(),
        getStackSize(),
        this,
        priority,
        &_taskHandle
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task %s", getTaskName());
        _taskHandle = nullptr;
        return;
    }

    ESP_LOGI(TAG, "Started task %s with priority %d", getTaskName(), priority);
}

void BaseTask::taskWrapper(void* arg) {
    BaseTask* task = static_cast<BaseTask*>(arg);
    ESP_LOGI(TAG, "Task %s started", task->getTaskName());
    
    task->run();
    
    ESP_LOGI(TAG, "Task %s ended", task->getTaskName());
    task->_taskHandle = nullptr;  // Clear handle on natural exit
    vTaskDelete(nullptr);
}