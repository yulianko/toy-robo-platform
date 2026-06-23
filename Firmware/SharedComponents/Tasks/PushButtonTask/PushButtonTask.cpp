#include "PushButtonTask.h"

#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "PushButtonTask";

PushButtonTask& PushButtonTask::instance() {
    static PushButtonTask inst;
    return inst;
}

void PushButtonTask::init(IsrButton& button, QueueHandle_t buttonQueue, QueueHandle_t robotEventQueue) {
    _button = &button;  // Reference, not owner
    _buttonQueue = buttonQueue;
    _robotEventQueue = robotEventQueue;

    ESP_LOGI(TAG, "Initialized");
}

void PushButtonTask::run() {
    ESP_LOGI(TAG, "Listening for push button events");
    IsrButton::ButtonEvent btnEvent;

    while (true) {  // Infinite loop - task ends via vTaskDelete()
        if (xQueueReceive(_buttonQueue, &btnEvent, portMAX_DELAY) == pdTRUE) {
            RobotEvent robotEvent;

            // Convert IsrButton event to RobotEvent
            if (btnEvent.action == IsrButton::Action::SHORT_PRESSED) {
                robotEvent = RobotEvent(RobotEvent::Type::PUSH_BUTTON_SHORT_PRESSED);
                ESP_LOGI(TAG, "Push button short pressed");
            } else {
                robotEvent = RobotEvent(RobotEvent::Type::PUSH_BUTTON_LONG_PRESSED);
                ESP_LOGI(TAG, "Push button long pressed");
            }

            // Send to back of queue (normal priority)
            if (xQueueSend(_robotEventQueue, &robotEvent, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Failed to send robot event");
            }
        }
    }
}
