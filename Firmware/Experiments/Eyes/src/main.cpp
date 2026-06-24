#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "IndicatorCommand.h"
#include "IndicatorsTask.h"
#include "RgbLedDriver.h"
#include "RgbPlayer.h"
#include "RobotAnimations.h"
#include "RobotEvent.h"

static const char* TAG = "main";

static constexpr gpio_num_t PIN_R = GPIO_NUM_17;
static constexpr gpio_num_t PIN_G = GPIO_NUM_16;
static constexpr gpio_num_t PIN_B = GPIO_NUM_15;

// ---- Hardware Objects (wiring diagram) ----
static RgbLedDriver::Config driverCfg{.pinR = PIN_R, .pinG = PIN_G, .pinB = PIN_B};
static RgbLedDriver driver(driverCfg);
static RgbPlayer player(driver);

// ---- FreeRTOS Queues ----
static QueueHandle_t robotEventQueue;
static QueueHandle_t indicatorCommandQueue;

uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

const char* robotEventTypeToString(RobotEvent::Type type) {
    switch (type) {
        case RobotEvent::Type::INDICATORS_ANIMATION_DONE:
            return "INDICATORS_ANIMATION_DONE";
        default:
            return "UNKNOWN";
    }
}

static void robotEventTask(void* arg) {
    RobotEvent event;
    uint32_t animationIndex = 0;

    // Start first animation immediately
    IndicatorCommand startCmd = IndicatorCommand::start(RobotEmotions::curiosity());
    xQueueSend(indicatorCommandQueue, &startCmd, portMAX_DELAY);
    ESP_LOGI(TAG, "Started first animation: curiosity");

    while (true) {
        if (xQueueReceive(robotEventQueue, &event, portMAX_DELAY)) {
            ESP_LOGI(TAG, "RobotEvent: %s", robotEventTypeToString(event.type));

            if (event.type == RobotEvent::Type::INDICATORS_ANIMATION_DONE) {
                // Cycle through animations
                animationIndex = (animationIndex + 1) % 4;

                IndicatorCommand nextCmd;
                switch (animationIndex) {
                    case 0:
                        nextCmd = IndicatorCommand::start(RobotEmotions::curiosity());
                        ESP_LOGI(TAG, "Starting curiosity animation");
                        break;
                    case 1:
                        nextCmd = IndicatorCommand::start(RobotEmotions::surprise());
                        ESP_LOGI(TAG, "Starting surprise animation");
                        break;
                    case 2:
                        nextCmd = IndicatorCommand::start(RobotEmotions::agreement());
                        ESP_LOGI(TAG, "Starting agreement animation");
                        break;
                    case 3:
                        nextCmd = IndicatorCommand::start(RobotEmotions::disagreement());
                        ESP_LOGI(TAG, "Starting disagreement animation");
                        break;

                    default:
                        break;
                }

                xQueueSend(indicatorCommandQueue, &nextCmd, portMAX_DELAY);
            }
        }
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting Eyes Experiment with IndicatorsTask");

    // ---- 1. Create FreeRTOS queues ----
    robotEventQueue = xQueueCreate(16, sizeof(RobotEvent));
    indicatorCommandQueue = xQueueCreate(8, sizeof(IndicatorCommand));
    if (!robotEventQueue || !indicatorCommandQueue) {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }

    // ---- 2. Initialize hardware drivers ----
    esp_err_t err = driver.init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init RGB driver: %s", esp_err_to_name(err));
        return;
    }

    // ---- 3. Initialize and start tasks ----
    IndicatorsTask::instance().init(player, indicatorCommandQueue, robotEventQueue);
    IndicatorsTask::instance().start(3);  // Priority 3 as per architecture

    // ---- 4. Start robot event processing task ----
    xTaskCreate(robotEventTask, "RobotEventTask", 4096, nullptr, 5, nullptr);

    ESP_LOGI(TAG, "All tasks started successfully");
}
