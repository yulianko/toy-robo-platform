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
    Drivetrain drivetrain(driver, {.invertA = false, .invertB = true});

    MotionPlayer player(drivetrain, [] { ESP_LOGI(TAG, "sequence done"); });

    // Demo
    ESP_LOGD(TAG, "Start demo, debug level test");

    // timed sequence - forward 2s, backward 1s, stop
    const MotionStep steps[] = {
        {MotionVerb::Forward, 0.7f, 2000},
        {MotionVerb::Backward, 0.7f, 1000},
        {MotionVerb::Brake, 0.0f, 0},
    };
    MotionSequence seq("test", steps, 3);
    ESP_ERROR_CHECK(player.play(seq));

    while (player.isPlaying()) {
        player.tick();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // named sequence
    ESP_ERROR_CHECK(player.play(MotionSequences::patrol(0.7f)));
    while (player.isPlaying()) {
        player.tick();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    ESP_ERROR_CHECK(player.brake());

    vTaskDelete(nullptr);
}
