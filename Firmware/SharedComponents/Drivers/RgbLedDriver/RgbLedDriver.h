#pragma once

#include "driver/ledc.h"

class IRgbLedDriver {
  public:
    virtual ~IRgbLedDriver() = default;

    virtual esp_err_t setColor(uint8_t r, uint8_t g, uint8_t b) = 0;

    virtual esp_err_t off() = 0;
};

class RgbLedDriver : public IRgbLedDriver {
  public:
    struct Config {
        const ledc_timer_bit_t DUTY_RESOLUTION = LEDC_TIMER_8_BIT;
        const uint16_t MAX_RESOLUTION_DUTY = 255;

        gpio_num_t pinR;
        gpio_num_t pinG;
        gpio_num_t pinB;

        ledc_timer_t timer = LEDC_TIMER_0;
        ledc_channel_t chanR = LEDC_CHANNEL_0;
        ledc_channel_t chanG = LEDC_CHANNEL_1;
        ledc_channel_t chanB = LEDC_CHANNEL_2;
        uint32_t freqHz = 5000;
        ledc_mode_t speedMode = LEDC_LOW_SPEED_MODE;
    };

    RgbLedDriver(const Config& cfg);

    esp_err_t init();

    esp_err_t setColor(uint8_t r, uint8_t g, uint8_t b) override;

    esp_err_t off() override;

  private:
    Config _cfg;

    esp_err_t setChannel(ledc_channel_t ch, uint8_t value);
};
