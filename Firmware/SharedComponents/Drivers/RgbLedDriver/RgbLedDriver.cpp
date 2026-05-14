#include "RgbLedDriver.h"

#include <esp_check.h>
#include <esp_log.h>

#include "driver/ledc.h"

static const char* TAG = "RgbLedDriver";

RgbLedDriver::RgbLedDriver(const Config& cfg) : _cfg(cfg) {
}

esp_err_t RgbLedDriver::init() {
    ledc_timer_config_t timerCfg = {
        .speed_mode = _cfg.speedMode,
        .duty_resolution = _cfg.DUTY_RESOLUTION,
        .timer_num = _cfg.timer,
        .freq_hz = _cfg.freqHz,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_RETURN_ON_ERROR(ledc_timer_config(&timerCfg), TAG, "ledc_timer_config failed");

    const gpio_num_t pins[3] = {_cfg.pinR, _cfg.pinG, _cfg.pinB};
    const ledc_channel_t chans[3] = {_cfg.chanR, _cfg.chanG, _cfg.chanB};

    for (int i = 0; i < 3; ++i) {
        ledc_channel_config_t chCfg = {
            .gpio_num = pins[i],
            .speed_mode = _cfg.speedMode,
            .channel = chans[i],
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = _cfg.timer,
            .duty = 0,
            .hpoint = 0,
        };

        ESP_RETURN_ON_ERROR(ledc_channel_config(&chCfg), TAG, "ledc_channel_config failed for pin %d", pins[i]);
    }

    ESP_LOGI(TAG,
             "init: R=%d G=%d B=%d, freq=%lu Hz, timer=%d, speed_mode=%d",
             _cfg.pinR,
             _cfg.pinG,
             _cfg.pinB,
             _cfg.freqHz,
             _cfg.timer,
             _cfg.speedMode);

    return ESP_OK;
}

esp_err_t RgbLedDriver::setColor(uint8_t r, uint8_t g, uint8_t b) {
    ESP_RETURN_ON_ERROR(setChannel(_cfg.chanR, r), TAG, "Failed to set channel R");
    ESP_RETURN_ON_ERROR(setChannel(_cfg.chanG, g), TAG, "Failed to set channel G");
    ESP_RETURN_ON_ERROR(setChannel(_cfg.chanB, b), TAG, "Failed to set channel B");

    return ESP_OK;
}

esp_err_t RgbLedDriver::off() {
    return setColor(0, 0, 0);
}

esp_err_t RgbLedDriver::setChannel(ledc_channel_t ch, uint8_t value) {
    uint32_t duty = _cfg.MAX_RESOLUTION_DUTY - value;
    ESP_RETURN_ON_ERROR(ledc_set_duty(_cfg.speedMode, ch, duty), TAG, "Failed to set duty for channel %d", ch);
    ESP_RETURN_ON_ERROR(ledc_update_duty(_cfg.speedMode, ch), TAG, "Failed to update duty for channel %d", ch);

    return ESP_OK;
}
