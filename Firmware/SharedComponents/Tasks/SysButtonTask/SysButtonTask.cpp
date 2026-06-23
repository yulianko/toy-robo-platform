#include "SysButtonTask.h"

#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "SysButtonTask";

SysButtonTask& SysButtonTask::instance() {
    static SysButtonTask inst;
    return inst;
}

void SysButtonTask::init(IsrButton& button, QueueHandle_t buttonQueue, QueueHandle_t robotEventQueue) {
    _button = &button;  // Reference, not owner
    _buttonQueue = buttonQueue;
    _robotEventQueue = robotEventQueue;

    ESP_LOGI(TAG, "Initialized");
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
