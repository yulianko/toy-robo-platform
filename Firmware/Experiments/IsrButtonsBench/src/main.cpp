#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "IsrButton.h"
#include "PushButtonTask.h"
#include "RobotEvent.h"
#include "SysButtonTask.h"

static const char* TAG = "main";

static constexpr gpio_num_t SYS_BTN_GPIO = GPIO_NUM_0;  // GPIO_NUM_48;
static constexpr gpio_num_t PUSH_BTN_GPIO = GPIO_NUM_47;

// ---- Hardware Objects (wiring diagram) ----
static IsrButton sysButton(
    {.pin = SYS_BTN_GPIO, .subject = "sys_btn", .debounceUs = 150'000, .longPressUs = 2'000'000});

static IsrButton pushButton(
    {.pin = PUSH_BTN_GPIO, .subject = "push_btn", .debounceUs = 150'000, .longPressUs = 1'000'000});

// ---- FreeRTOS Queues ----
static QueueHandle_t robotEventQueue;

static QueueHandle_t sysButtonQueue;
static QueueHandle_t pushButtonQueue;

static void robotEventTask(void* arg) {
    RobotEvent event;
    while (true) {
        if (xQueueReceive(robotEventQueue, &event, portMAX_DELAY)) {
            ESP_LOGI(TAG, "RobotEvent: %s", RobotEvent::typeToString(event.type));

            if (event.type == RobotEvent::Type::SYS_BUTTON_LONG_PRESSED) {
                ESP_LOGW(TAG, "Executing important system command!");
            }
        }
    }
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting ISR Buttons Bench with RobotEvent system");

    // ---- 1. Create FreeRTOS queues ----
    robotEventQueue = xQueueCreate(16, sizeof(RobotEvent));
    sysButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    pushButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    if (!robotEventQueue || !sysButtonQueue || !pushButtonQueue) {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }

    // ---- 2. Initialize hardware drivers ----
    esp_err_t err = sysButton.init(sysButtonQueue);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init sys button: %s", esp_err_to_name(err));
        return;
    }

    err = pushButton.init(pushButtonQueue);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init push button: %s", esp_err_to_name(err));
        return;
    }

    // ---- 3. Initialize and start tasks (dependency injection) ----
    SysButtonTask::instance().init(sysButton, sysButtonQueue, robotEventQueue);
    PushButtonTask::instance().init(pushButton, pushButtonQueue, robotEventQueue);

    SysButtonTask::instance().start(6);
    PushButtonTask::instance().start(6);

    // ---- 4. Start robot event processing task ----
    xTaskCreate(robotEventTask, "RobotEventTask", 3072, nullptr, 5, nullptr);

    ESP_LOGI(TAG, "All tasks started successfully");
}
