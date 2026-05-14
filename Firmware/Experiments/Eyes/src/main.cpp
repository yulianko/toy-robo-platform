#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "RgbLedDriver.h"
#include "RgbPlayer.h"
#include "RobotAnimations.h"

static constexpr gpio_num_t PIN_R = GPIO_NUM_17;
static constexpr gpio_num_t PIN_G = GPIO_NUM_16;
static constexpr gpio_num_t PIN_B = GPIO_NUM_15;

static constexpr uint32_t TIMEOUT_MS = 60000;

uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
};

extern "C" void app_main(void) {
    RgbLedDriver::Config driverCfg{.pinR = PIN_R, .pinG = PIN_G, .pinB = PIN_B};
    static RgbLedDriver driver(driverCfg);
    driver.init();

    static RgbPlayer player(driver);

    // RgbAnimation animation = RobotEmotions::magenta();
    // RgbAnimation animation = RobotEmotions::curiosity();
    // RgbAnimation animation = RobotEmotions::surprise();
    // RgbAnimation animation = RobotEmotions::agreement();
    // RgbAnimation animation = RobotEmotions::disagreement();

    while (true) {
        ESP_LOGI("Main", "Loop start");
        player.start(RobotEmotions::curiosity(), nowMs());
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        player.start(RobotEmotions::agreement(), nowMs());
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    ESP_LOGI("Main", "Done");
}
