#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "RobotSounds.h"
#include "SoundDriver.h"
#include "SoundPlayer.h"

static constexpr gpio_num_t PIN = GPIO_NUM_4;

static constexpr uint32_t TIMEOUT_MS = 60000;

uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
};

extern "C" void app_main(void) {
    SoundLedcDriver::Config driverCfg{.pin = PIN};
    static SoundLedcDriver driver(driverCfg);
    driver.init();

    static SoundPlayer player(driver);

    for (size_t i = 0; i < 1; i++) {
        ESP_LOGI("Main", "Loop start");
        uint32_t startMs = nowMs();
        player.start(RobotSounds::curiosity(), startMs);
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    for (size_t i = 0; i < 1; i++) {
        ESP_LOGI("Main", "Loop start");
        uint32_t startMs = nowMs();
        player.start(RobotSounds::surprise(), startMs);
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    for (size_t i = 0; i < 1; i++) {
        ESP_LOGI("Main", "Loop start");
        uint32_t startMs = nowMs();
        player.start(RobotSounds::agreement(), startMs);
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    for (size_t i = 0; i < 1; i++) {
        ESP_LOGI("Main", "Loop start");
        uint32_t startMs = nowMs();
        player.start(RobotSounds::disagreement(), startMs);
        while (!player.isDone()) {
            uint32_t now = nowMs();
            player.tick(now);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    ESP_LOGI("Main", "Done");
}
