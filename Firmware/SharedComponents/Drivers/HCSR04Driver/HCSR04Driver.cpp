#include "HCSR04Driver.h"

#include <esp_check.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

static const char* TAG = "HCSR04Driver";

HCSR04Driver::HCSR04Driver(const Config& cfg)
    : _cfg(cfg), _timeoutUs(static_cast<uint32_t>((2.0 * cfg.maxDistanceCm) / SOUND_SPEED_CM_PER_US * 1.2)) {
}

esp_err_t HCSR04Driver::init() {
    ESP_LOGI(TAG,
             "init: Trigger Pin: %d, Echo Pin: %d, Max Distance: %d cm, Timeout: %d us",
             _cfg.pinTrigger,
             _cfg.pinEcho,
             _cfg.maxDistanceCm,
             _timeoutUs);

    gpio_config_t pinConfig = {};
    pinConfig.intr_type = GPIO_INTR_DISABLE;
    pinConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pinConfig.pull_up_en = GPIO_PULLUP_DISABLE;

    // Configure Trigger Pin (Output)
    pinConfig.mode = GPIO_MODE_OUTPUT;
    pinConfig.pin_bit_mask = (1ULL << _cfg.pinTrigger);
    ESP_RETURN_ON_ERROR(gpio_config(&pinConfig), TAG, "gpio_config failed for trigger pin %d", _cfg.pinTrigger);

    // Configure Echo Pin (Input)
    pinConfig.mode = GPIO_MODE_INPUT;
    pinConfig.pin_bit_mask = (1ULL << _cfg.pinEcho);
    ESP_RETURN_ON_ERROR(gpio_config(&pinConfig), TAG, "gpio_config failed for echo pin %d", _cfg.pinEcho);

    // Ensure trigger starts LOW
    ESP_RETURN_ON_ERROR(
        gpio_set_level(_cfg.pinTrigger, 0), TAG, "gpio_set_level failed for trigger pin %d", _cfg.pinTrigger);

    ESP_LOGI(TAG, "HCSR04Driver initialized successfully");
    return ESP_OK;
}

esp_err_t HCSR04Driver::measure(uint16_t& outCm) {
    ESP_RETURN_ON_ERROR(
        gpio_set_level(_cfg.pinTrigger, 1), TAG, "gpio_set_level failed for trigger pin %d", _cfg.pinTrigger);
    ets_delay_us(10);
    ESP_RETURN_ON_ERROR(
        gpio_set_level(_cfg.pinTrigger, 0), TAG, "gpio_set_level failed for trigger pin %d", _cfg.pinTrigger);

    int64_t startWaitUs = esp_timer_get_time();
    while (gpio_get_level(_cfg.pinEcho) == 0) {
        if ((esp_timer_get_time() - startWaitUs) > 10'000) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t echoStartUs = esp_timer_get_time();
    while (gpio_get_level(_cfg.pinEcho) == 1) {
        if ((esp_timer_get_time() - echoStartUs) > _timeoutUs) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t echoEndUs = esp_timer_get_time();

    uint32_t durationUs = static_cast<uint32_t>(echoEndUs - echoStartUs);
    double distanceCm = (static_cast<double>(durationUs) * SOUND_SPEED_CM_PER_US) / 2.0;
    uint32_t calculatedDistance = static_cast<uint32_t>(distanceCm + 0.5);

    if (calculatedDistance > _cfg.maxDistanceCm) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    outCm = static_cast<uint16_t>(calculatedDistance);
    return ESP_OK;
}
