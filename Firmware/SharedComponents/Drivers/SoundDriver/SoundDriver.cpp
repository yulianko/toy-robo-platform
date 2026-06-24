#include "SoundDriver.h"

#include <esp_check.h>
#include <esp_log.h>

#include "driver/ledc.h"

static const char* TAG = "SoundDriver";

SoundLedcDriver::SoundLedcDriver(Config cfg) : _cfg(cfg) {
}

esp_err_t SoundLedcDriver::init() {
    ledc_timer_config_t t_cfg = {.speed_mode = _cfg.mode,
                                 .duty_resolution = LEDC_TIMER_10_BIT,
                                 .timer_num = _cfg.timer,
                                 .freq_hz = 1000,
                                 .clk_cfg = LEDC_AUTO_CLK};
    ESP_RETURN_ON_ERROR(ledc_timer_config(&t_cfg), TAG, "ledc_timer_config failed");

    ledc_channel_config_t cfg = {.gpio_num = _cfg.pin,
                                 .speed_mode = _cfg.mode,
                                 .channel = _cfg.channel,
                                 .intr_type = LEDC_INTR_DISABLE,
                                 .timer_sel = _cfg.timer,
                                 .duty = 0,
                                 .hpoint = 0};

    ESP_RETURN_ON_ERROR(ledc_channel_config(&cfg), TAG, "ledc_channel_config failed for pin %d", cfg.gpio_num);

    ESP_LOGI(TAG, "init: pin=%d , chan=%d, timer=%d, speed_mode=%d", _cfg.pin, _cfg.channel, _cfg.timer, _cfg.mode);

    return ESP_OK;
}

esp_err_t SoundLedcDriver::setFrequency(uint32_t freq) {
    if (freq == 0) {
        return off();
    }

    ESP_RETURN_ON_ERROR(ledc_set_freq(_cfg.mode, _cfg.timer, freq), TAG, "Failed to set freq for timer %d", _cfg.timer);

    ESP_RETURN_ON_ERROR(
        ledc_set_duty(_cfg.mode, _cfg.channel, 512), TAG, "Failed to set duty for channel %d", _cfg.channel);

    ESP_RETURN_ON_ERROR(
        ledc_update_duty(_cfg.mode, _cfg.channel), TAG, "Failed to update duty for channel %d", _cfg.channel);

    return ESP_OK;
}

esp_err_t SoundLedcDriver::off() {
    ESP_RETURN_ON_ERROR(
        ledc_set_duty(_cfg.mode, _cfg.channel, 0), TAG, "Failed to set duty for channel %d", _cfg.channel);

    ESP_RETURN_ON_ERROR(
        ledc_update_duty(_cfg.mode, _cfg.channel), TAG, "Failed to update duty for channel %d", _cfg.channel);

    return ESP_OK;
}
