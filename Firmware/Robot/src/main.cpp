#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// SharedComponents
#include "IsrButton.h"
#include "PushButtonTask.h"
#include "RobotEvent.h"
#include "SysButtonTask.h"

// Robot components
#include "ModeManagerTask.h"
#include "RobotContext.h"
#include "TestModes.h"

static const char* TAG = "main";

// Pin definitions
static constexpr gpio_num_t SYS_BTN_GPIO = GPIO_NUM_48;
static constexpr gpio_num_t PUSH_BTN_GPIO = GPIO_NUM_47;

// Hardware objects
static IsrButton sysButton(
    {.pin = SYS_BTN_GPIO, .subject = "sys_btn", .debounceUs = 150'000, .longPressUs = 2'000'000});

static IsrButton pushButton(
    {.pin = PUSH_BTN_GPIO, .subject = "push_btn", .debounceUs = 150'000, .longPressUs = 1'000'000});

// FreeRTOS queues
static QueueHandle_t robotEventQueue;

static QueueHandle_t sysButtonQueue;
static QueueHandle_t pushButtonQueue;

// Mode instances
static PrintMode printMode;
static CounterMode counterMode;

// Robot context
static RobotContext robotContext;

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting ESP32 Robot - Mode Manager Phase");

    // ---- 1. Create FreeRTOS queues ----
    robotEventQueue = xQueueCreate(ModeManagerTask::QUEUE_LENGTH, sizeof(RobotEvent));
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

    // ---- 3. Initialize tasks (DI) ----
    SysButtonTask::instance().init(sysButton, sysButtonQueue, robotEventQueue);
    PushButtonTask::instance().init(pushButton, pushButtonQueue, robotEventQueue);
    ModeManagerTask::instance().init(robotContext, robotEventQueue);

    // ---- 4. Register modes with ModeManager ----
    ModeManagerTask::instance().addMode(&printMode);
    ModeManagerTask::instance().addMode(&counterMode);

    // ---- 5. Start ModeManager ----
    ModeManagerTask::instance().start(5);

    // ---- 6. Start hardware tasks ----
    SysButtonTask::instance().start(6);
    PushButtonTask::instance().start(6);

    ESP_LOGI(TAG, "All systems started successfully");
    ESP_LOGI(TAG, "Use SYS button to cycle modes, PUSH button to select/interact");
    ESP_LOGI(TAG, "Long press SYS button for emergency restart");

    vTaskDelete(nullptr);
}
