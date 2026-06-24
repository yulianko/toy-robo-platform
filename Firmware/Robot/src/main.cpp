#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// SharedComponents
#include "IndicatorCommand.h"
#include "IndicatorsTask.h"
#include "IsrButton.h"
#include "PushButtonTask.h"
#include "RgbLedDriver.h"
#include "RgbPlayer.h"
#include "RobotEvent.h"
#include "SoundDriver.h"
#include "SoundPlayer.h"
#include "SysButtonTask.h"

// Robot components
#include "ModeManagerTask.h"
#include "RobotContext.h"
#include "TestModes.h"

static const char* TAG = "main";

// Pin definitions

// Buttons pins
static constexpr gpio_num_t SYS_BTN_GPIO = GPIO_NUM_0;
static constexpr gpio_num_t PUSH_BTN_GPIO = GPIO_NUM_47;

// RGB LED pins
static constexpr gpio_num_t PIN_R = GPIO_NUM_17;
static constexpr gpio_num_t PIN_G = GPIO_NUM_16;
static constexpr gpio_num_t PIN_B = GPIO_NUM_15;

// Buzzer pin
static constexpr gpio_num_t BUZZER_PIN = GPIO_NUM_4;

// Hardware objects

// Buttons
static IsrButton sysButton(
    {.pin = SYS_BTN_GPIO, .subject = "sys_btn", .debounceUs = 150'000, .longPressUs = 2'000'000});

static IsrButton pushButton(
    {.pin = PUSH_BTN_GPIO, .subject = "push_btn", .debounceUs = 150'000, .longPressUs = 1'000'000});

// RGB LED
static RgbLedDriver::Config driverCfg{.pinR = PIN_R, .pinG = PIN_G, .pinB = PIN_B};
static RgbLedDriver rgbDriver(driverCfg);
static RgbPlayer rgbPlayer(rgbDriver);

// Sound/Buzzer
static SoundLedcDriver::Config soundDriverCfg{.pin = BUZZER_PIN};
static SoundLedcDriver soundDriver(soundDriverCfg);
static SoundPlayer soundPlayer(soundDriver);

// FreeRTOS queues
static QueueHandle_t robotEventQueue;
static QueueHandle_t sysButtonQueue;
static QueueHandle_t pushButtonQueue;
static QueueHandle_t indicatorCommandQueue;

// Mode instances
static PrintMode printMode;
static CounterMode counterMode;

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting ESP32 Robot - Mode Manager Phase");

    // ---- 1. Create FreeRTOS queues ----
    robotEventQueue = xQueueCreate(ModeManagerTask::QUEUE_LENGTH, sizeof(RobotEvent));
    sysButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    pushButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    indicatorCommandQueue = xQueueCreate(8, sizeof(IndicatorCommand));

    if (!robotEventQueue || !sysButtonQueue || !pushButtonQueue || !indicatorCommandQueue) {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }

    // ---- 2. Initialize robot context with queues ----
    static RobotContext robotContext(indicatorCommandQueue);

    // ---- 3. Initialize hardware drivers ----
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

    err = rgbDriver.init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init RGB driver: %s", esp_err_to_name(err));
        return;
    }

    err = soundDriver.init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init sound driver: %s", esp_err_to_name(err));
        return;
    }

    // ---- 4. Initialize tasks (DI) ----
    SysButtonTask::instance().init(sysButton, sysButtonQueue, robotEventQueue);
    PushButtonTask::instance().init(pushButton, pushButtonQueue, robotEventQueue);
    IndicatorsTask::instance().init(rgbPlayer, soundPlayer, indicatorCommandQueue, robotEventQueue);
    ModeManagerTask::instance().init(robotContext, robotEventQueue);

    // ---- 5. Register modes with ModeManager ----
    ModeManagerTask::instance().addMode(&printMode);
    ModeManagerTask::instance().addMode(&counterMode);

    // ---- 6. Start actuator tasks first (lower priority numbers run first) ----
    IndicatorsTask::instance().start(3);  // Priority 3 - start first

    // ---- 7. Start ModeManager after actuators are ready ----
    ModeManagerTask::instance().start(5);  // Priority 5 - start after actuators

    // ---- 8. Start hardware tasks ----
    SysButtonTask::instance().start(6);  // Priority 6 - highest priority
    PushButtonTask::instance().start(6);

    ESP_LOGI(TAG, "All systems started successfully");
    ESP_LOGI(TAG, "Use SYS button to cycle modes, PUSH button to select/interact");
    ESP_LOGI(TAG, "Long press SYS button for emergency restart");

    vTaskDelete(nullptr);
}
