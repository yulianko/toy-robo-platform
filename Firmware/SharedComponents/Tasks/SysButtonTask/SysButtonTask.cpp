#include "SysButtonTask.h"

#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "SysButtonTask";

SysButtonTask& SysButtonTask::instance() {
    static SysButtonTask inst;
    return inst;
}

SysButtonTask::SysButtonTask() {
    // Empty constructor - initialization happens in init()
}

void SysButtonTask::init(IsrButton& button, QueueHandle_t buttonQueue, QueueHandle_t robotEventQueue) {
    _button = &button;  // Reference, not owner
    _buttonQueue = buttonQueue;
    _robotEventQueue = robotEventQueue;

    ESP_LOGI(TAG, "Initialized");
}

void SysButtonTask::start(UBaseType_t priority) {
    if (_taskHandle != nullptr) {
        ESP_LOGW(TAG, "Task already started");
        return;
    }

    if (!_button || !_buttonQueue || !_robotEventQueue) {
        ESP_LOGE(TAG, "Task not initialized - call init() first with all dependencies");
        return;
    }

    // Create task - button and queue are already initialized by main
    xTaskCreate(taskFn, "SysButtonTask", TASK_STACK, this, priority, &_taskHandle);
    ESP_LOGI(TAG, "Started");
}

void SysButtonTask::stop() {
    if (_taskHandle == nullptr) {
        return;
    }

    vTaskDelete(_taskHandle);
    _taskHandle = nullptr;

    // Don't delete _buttonQueue - it's owned by main, not us
    ESP_LOGI(TAG, "Stopped");
}

void SysButtonTask::taskFn(void* arg) {
    static_cast<SysButtonTask*>(arg)->run();
}

void SysButtonTask::run() {
    ESP_LOGI(TAG, "Listening for system button events");
    IsrButton::ButtonEvent btnEvent;

    while (true) {  // Infinite loop - task ends via vTaskDelete()
        if (xQueueReceive(_buttonQueue, &btnEvent, portMAX_DELAY) == pdTRUE) {
            RobotEvent robotEvent;

            // Convert IsrButton event to RobotEvent
            if (btnEvent.action == IsrButton::Action::SHORT_PRESSED) {
                robotEvent = RobotEvent(RobotEvent::Type::SYS_BUTTON_SHORT_PRESSED);
                ESP_LOGI(TAG, "System button short pressed");
            } else {
                robotEvent = RobotEvent(RobotEvent::Type::SYS_BUTTON_LONG_PRESSED);
                ESP_LOGI(TAG, "System button long pressed");
            }

            // Send to front of queue for priority handling
            if (xQueueSendToFront(_robotEventQueue, &robotEvent, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Failed to send robot event");
            }
        }
    }
}
