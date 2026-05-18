#pragma once
#include <cstdint>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

class ISoundDriver {
  public:
    virtual ~ISoundDriver() = default;

    virtual esp_err_t setFrequency(uint32_t freq) = 0;
    virtual esp_err_t off() = 0;
};

class SoundLedcDriver : public ISoundDriver {
  public:
    struct Config {
        gpio_num_t pin;
        ledc_timer_t timer = LEDC_TIMER_1;
        ledc_channel_t channel = LEDC_CHANNEL_3;
        ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    };

    SoundLedcDriver(Config cfg);

    esp_err_t init();
    esp_err_t setFrequency(uint32_t freq) override;
    esp_err_t off() override;

  private:
    Config _cfg;
};
