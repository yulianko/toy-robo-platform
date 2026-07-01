#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

// SharedComponents
#include "DRV8833Module.h"
#include "DistanceTask.h"
#include "HCSR04Driver.h"
#include "DiagnosticsTask.h"
#include "HttpServer.h"
#include "IndicatorCommand.h"
#include "IndicatorsTask.h"
#include "IsrButton.h"
#include "MotionCommand.h"
#include "MotionTask.h"
#include "PushButtonTask.h"
#include "RgbLedDriver.h"
#include "RgbPlayer.h"
#include "RobotEvent.h"
#include "SoundDriver.h"
#include "SoundPlayer.h"
#include "SysButtonTask.h"
#include "WiFiConfigPage.h"
#include "WiFiManager.h"
#include "WiFiProxy.h"

// Robot components
#include "ConfigMode.h"
#include "DistanceMode.h"
#include "ModeManagerTask.h"
#include "PatrolMode.h"
#include "RobotContext.h"
#include "RobotControlHttpPage.h"
#include "SelfTestMode.h"
#include "TestModes.h"

static const char* TAG = "main";

// Pin definitions

// Buttons pins
static constexpr gpio_num_t SYS_BTN_GPIO = GPIO_NUM_48;
static constexpr gpio_num_t PUSH_BTN_GPIO = GPIO_NUM_47;

// RGB LED pins
static constexpr gpio_num_t PIN_R = GPIO_NUM_17;
static constexpr gpio_num_t PIN_G = GPIO_NUM_16;
static constexpr gpio_num_t PIN_B = GPIO_NUM_15;

// Buzzer pin
static constexpr gpio_num_t BUZZER_PIN = GPIO_NUM_4;

// Distance pins
static constexpr gpio_num_t TRIGGER_PIN = GPIO_NUM_9;
static constexpr gpio_num_t ECHO_PIN = GPIO_NUM_10;

// Motion
static constexpr gpio_num_t PIN_M_AIN1 = GPIO_NUM_11;
static constexpr gpio_num_t PIN_M_AIN2 = GPIO_NUM_12;
static constexpr gpio_num_t PIN_M_BIN1 = GPIO_NUM_14;
static constexpr gpio_num_t PIN_M_BIN2 = GPIO_NUM_13;
static constexpr gpio_num_t PIN_M_STBY = GPIO_NUM_8;

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

// Distance Sensor
static HCSR04Driver::Config distanceDriverCfg{.pinTrigger = TRIGGER_PIN, .pinEcho = ECHO_PIN};
static HCSR04Driver distanceDriver(distanceDriverCfg);

// Motion
DRV8833Module motionDriver({
    .pinAIn1 = PIN_M_AIN1,
    .pinAIn2 = PIN_M_AIN2,
    .pinBIn1 = PIN_M_BIN1,
    .pinBIn2 = PIN_M_BIN2,
    .pinStby = PIN_M_STBY,
    .mcpwmGroupId = 0,
    .pwmFreqHz = 20'000,
});
static Drivetrain drivetrain(motionDriver, {.invertA = true, .invertB = true});
static MotionPlayer motionPlayer(drivetrain);

// Network
static HttpServer httpServer;

// FreeRTOS queues
static QueueHandle_t robotEventQueue;
static QueueHandle_t sysButtonQueue;
static QueueHandle_t pushButtonQueue;
static QueueHandle_t indicatorCommandQueue;
static QueueHandle_t motionCommandQueue;
static QueueHandle_t wifiCommandQueue;

// Mode instances
static PrintMode printMode;
static SelfTestMode selfTestMode;
static DistanceMode distanceMode;
static PatrolMode patrolMode;
static ConfigMode configMode;

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting ESP32 Robot - Mode Manager Phase");

    // ---- Create FreeRTOS queues ----
    robotEventQueue = xQueueCreate(ModeManagerTask::QUEUE_LENGTH, sizeof(RobotEvent));
    sysButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    pushButtonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));
    indicatorCommandQueue = xQueueCreate(8, sizeof(IndicatorCommand));
    motionCommandQueue = xQueueCreate(8, sizeof(MotionCommand));
    wifiCommandQueue = xQueueCreate(4, sizeof(WmMsg));

    if (!robotEventQueue || !sysButtonQueue || !pushButtonQueue || !indicatorCommandQueue) {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }

    // ---- Initialize hardware drivers ----
    ESP_RETURN_VOID_ON_ERROR(sysButton.init(sysButtonQueue), TAG, "Failed to init sys button");
    ESP_RETURN_VOID_ON_ERROR(pushButton.init(pushButtonQueue), TAG, "Failed to init push button");
    ESP_RETURN_VOID_ON_ERROR(rgbDriver.init(), TAG, "Failed to init RGB driver");
    ESP_RETURN_VOID_ON_ERROR(soundDriver.init(), TAG, "Failed to init sound driver");
    ESP_RETURN_VOID_ON_ERROR(distanceDriver.init(), TAG, "Failed to init distance driver");
    ESP_RETURN_VOID_ON_ERROR(motionDriver.init(), TAG, "Failed to init motion driver");

    // ---- Initialize Net ----
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_VOID_ON_ERROR(nvs_flash_erase(), TAG, "Failed to erase nvs flash");
        ret = nvs_flash_init();
    }
    ESP_RETURN_VOID_ON_ERROR(ret, TAG, "Failed to init nvs flash");

    // default event loop and TCP/IP stack
    ESP_RETURN_VOID_ON_ERROR(esp_event_loop_create_default(), TAG, "Failed to create event loop");
    ESP_RETURN_VOID_ON_ERROR(esp_netif_init(), TAG, "Failed to init netif");

    // ---- Initialize tasks (DI) ----
    SysButtonTask::instance().init(sysButton, sysButtonQueue, robotEventQueue);
    PushButtonTask::instance().init(pushButton, pushButtonQueue, robotEventQueue);
    IndicatorsTask::instance().init(rgbPlayer, soundPlayer, indicatorCommandQueue, robotEventQueue);
    DistanceTask::instance().init(distanceDriver, robotEventQueue);
    MotionTask::instance().init(motionPlayer, motionCommandQueue, robotEventQueue);
    WiFiManager::Config wmConfig = {
        .scanRssiThreshold = -85,   // dBm
        .scanMaxDisplay = 10,       // how many to present to user
        .connectTimeoutMs = 15000,  // ms
        .driver =
            {
                .apSsid = "MyNet",
                .apPassword = "_1234567",
                .apChannel = 1,
                .apMaxConn = 4,
            },
    };

    WiFiManager::instance().init(wmConfig, wifiCommandQueue);

    // ---- Initialize ModeManagerTask ----
    static WiFiProxy wifiProxy(&WiFiManager::instance(), wifiCommandQueue);
    static RobotContext robotContext(indicatorCommandQueue, DistanceTask::instance(), motionCommandQueue, wifiProxy);
    ModeManagerTask::instance().init(robotContext, robotEventQueue);

    ModeManagerTask::instance().addMode(&printMode);
    ModeManagerTask::instance().addMode(&selfTestMode);
    ModeManagerTask::instance().addMode(&distanceMode);
    ModeManagerTask::instance().addMode(&patrolMode);
    ModeManagerTask::instance().addMode(&configMode);

    // ---- Start actuator tasks first (lower priority numbers run first) ----
    IndicatorsTask::instance().start(3);  // Priority 3 - start first
    MotionTask::instance().start(3);      // Priority 3 - same as other actuators tasks
    DistanceTask::instance().start(3);    // Priority 3 - same as other sensor tasks
    WiFiManager::instance().start(5);     // Priority 5 - network

    static WiFiConfigPage wifiPage("");
    static RobotControlHttpPage controlPage("control");
    wifiPage.init(&wifiProxy);
    controlPage.init(&robotContext);

    IHttpPage* pages[] = {&wifiPage, &controlPage};

    HttpServer::Config httpServerConfig = {.port = 80, .maxSockets = 4, .stackSize = 6144};
    httpServer.start(httpServerConfig, pages, 2);

    // ---- Start ModeManager after actuators are ready ----
    ModeManagerTask::instance().start(5);  // Priority 5 - start after actuators

    // ---- Start hardware tasks ----
    SysButtonTask::instance().start(6);  // Priority 6 - highest priority
    PushButtonTask::instance().start(6);

    // ---- Start diagnostic task ----
    DiagnosticsTask::instance().start();

    ESP_LOGI(TAG, "All systems started successfully");
    ESP_LOGI(TAG, "Use SYS button to cycle modes, PUSH button to select/interact");
    ESP_LOGI(TAG, "Long press SYS button for emergency restart");

    vTaskDelete(nullptr);
}
