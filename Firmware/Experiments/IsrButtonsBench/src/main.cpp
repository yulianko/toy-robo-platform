#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "IsrButton.h"

static const char* TAG = "main";

static constexpr gpio_num_t BTN1_GPIO = GPIO_NUM_48;
static constexpr gpio_num_t BTN2_GPIO = GPIO_NUM_47;

static QueueHandle_t button1Queue;
static QueueHandle_t button2Queue;

static IsrButton btn1({BTN1_GPIO, "btn1", 150'000, 2'000'000});
static IsrButton btn2({BTN2_GPIO, "btn2", 150'000, 1'000'000});

static void button1Task(void* arg) {
    IsrButton::ButtonEvent ev;
    while (true) {
        if (xQueueReceive(button1Queue, &ev, portMAX_DELAY)) {
            const char* actionStr = (ev.action == IsrButton::Action::CLICK) ? "CLICK" : "LONG_PRESS";
            ESP_LOGI(TAG, "Button 1 %s: %s", ev.subject, actionStr);
        }
    }
}

static void button2Task(void* arg) {
    IsrButton::ButtonEvent ev;
    while (true) {
        if (xQueueReceive(button2Queue, &ev, portMAX_DELAY)) {
            const char* actionStr = (ev.action == IsrButton::Action::CLICK) ? "CLICK" : "LONG_PRESS";
            ESP_LOGI(TAG, "Button 2 %s: %s", ev.subject, actionStr);
        }
    }
}

extern "C" void app_main() {
    button1Queue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    button2Queue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));

    btn1.init(button1Queue);
    btn2.init(button2Queue);

    xTaskCreate(button1Task, "btn1_task", 2048, nullptr, 10, nullptr);
    xTaskCreate(button2Task, "btn2_task", 2048, nullptr, 10, nullptr);
}
