#include <DistanceSensor.h>
#include <HCSR04Driver.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static constexpr gpio_num_t TRIGGER_PIN = GPIO_NUM_9;
static constexpr gpio_num_t ECHO_PIN = GPIO_NUM_10;

static const char* TAG = "Sensor";

extern "C" void app_main() {
    HCSR04Driver driver({
        .pinTrigger = TRIGGER_PIN,
        .pinEcho = ECHO_PIN,
    });

    driver.init();
    DistanceSensor sensor(driver,
                          {
                              .onMeasure = [](uint16_t cm) { ESP_LOGI(TAG, "Measured: %lu cm", cm); },
                              .onError = [](esp_err_t err) { ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err)); },
                          });

    ESP_LOGI(TAG, "Starting measure...");
    sensor.measure();
    ESP_LOGI(TAG, "Finished measure...");

    ESP_LOGI(TAG, "Starting loop...");
    while (true) {
        uint16_t cm = 0;
        if (sensor.measureDirect(cm) == ESP_OK) {
            ESP_LOGI(TAG, "%lu cm", cm);
        } else {
            ESP_LOGW(TAG, "Failed to measure");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
