#include "DRV8833Module.h"
#include "Drivetrain.h"
#include "MotionPlayer.h"
#include "MotionSequences.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

static const char* TAG = "main";

static constexpr gpio_num_t PIN_AIN1 = GPIO_NUM_11;
static constexpr gpio_num_t PIN_AIN2 = GPIO_NUM_12;
static constexpr gpio_num_t PIN_BIN1 = GPIO_NUM_14;
static constexpr gpio_num_t PIN_BIN2 = GPIO_NUM_13;
static constexpr gpio_num_t PIN_STBY = GPIO_NUM_8;

#define MCPWM_GROUP 0

static void runSequence(MotionPlayer& player, const MotionSequence& sequence) {
    ESP_LOGI(TAG, "Running sequence '%s'", sequence.name());
    ESP_ERROR_CHECK(player.play(sequence));
    while (player.isPlaying()) {
        player.tick();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelay(pdMS_TO_TICKS(300));
}

extern "C" void app_main() {
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    DRV8833Module driver({
        .pinAIn1 = PIN_AIN1,
        .pinAIn2 = PIN_AIN2,
        .pinBIn1 = PIN_BIN1,
        .pinBIn2 = PIN_BIN2,
        .pinStby = PIN_STBY,
        .mcpwmGroupId = MCPWM_GROUP,
        .pwmFreqHz = 20'000,
    });
    ESP_ERROR_CHECK(driver.init());

    // invertB = true - right motor mounted mirrored, correct without rewiring
    Drivetrain drivetrain(driver, {.invertA = true, .invertB = true});
    MotionPlayer player(drivetrain);

    ESP_LOGI(TAG, "Movement experiment started");

    const float speed = 0.65f;

    runSequence(player, MotionSequences::forwardFor(speed, 1500));
    runSequence(player, MotionSequences::backwardFor(speed, 1500));

    runSequence(player, MotionSequences::turnLeftFor(speed, 1200));
    runSequence(player, MotionSequences::turnRightFor(speed, 1200));
    runSequence(player, MotionSequences::pivotLeftFor(speed, 1200));
    runSequence(player, MotionSequences::pivotRightFor(speed, 1200));

    ESP_ERROR_CHECK(player.brake());
    ESP_LOGI(TAG, "Movement experiment complete");

    vTaskDelete(nullptr);
}
