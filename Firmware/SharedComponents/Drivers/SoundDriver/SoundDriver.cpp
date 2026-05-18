#include "SoundDriver.h"

#include "driver/ledc.h"

SoundLedcDriver::SoundLedcDriver(Config cfg) : _cfg(cfg) {
}

esp_err_t SoundLedcDriver::init() {
    ledc_timer_config_t t_cfg = {.speed_mode = _cfg.mode,
                                 .duty_resolution = LEDC_TIMER_10_BIT,
                                 .timer_num = _cfg.timer,
                                 .freq_hz = 1000,  // Початкова
                                 .clk_cfg = LEDC_AUTO_CLK};
    esp_err_t err = ledc_timer_config(&t_cfg);
    if (err != ESP_OK) {
        return err;
    }

    ledc_channel_config_t c_cfg = {.gpio_num = _cfg.pin,
                                   .speed_mode = _cfg.mode,
                                   .channel = _cfg.channel,
                                   .intr_type = LEDC_INTR_DISABLE,
                                   .timer_sel = _cfg.timer,
                                   .duty = 0,  // Починаємо з тиші
                                   .hpoint = 0};
    return ledc_channel_config(&c_cfg);
}

esp_err_t SoundLedcDriver::setFrequency(uint32_t freq) {
    if (freq == 0) {
        return off();
    }

    esp_err_t err = ledc_set_freq(_cfg.mode, _cfg.timer, freq);
    if (err != ESP_OK) {
        return err;
    }

    // Встановлюємо duty 50% (512 для 10 біт), щоб був звук
    err = ledc_set_duty(_cfg.mode, _cfg.channel, 512);
    if (err != ESP_OK) {
        return err;
    }

    return ledc_update_duty(_cfg.mode, _cfg.channel);
}

esp_err_t SoundLedcDriver::off() {
    esp_err_t err = ledc_set_duty(_cfg.mode, _cfg.channel, 0);
    if (err != ESP_OK) {
        return err;
    }

    return ledc_update_duty(_cfg.mode, _cfg.channel);
}
