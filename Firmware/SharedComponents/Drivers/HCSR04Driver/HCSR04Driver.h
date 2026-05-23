#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "esp_err.h"

class IDistanceDriver {
  public:
    virtual esp_err_t measure(uint16_t& outCm) = 0;
};

class HCSR04Driver : public IDistanceDriver {
  public:
    struct Config {
        gpio_num_t pinTrigger;
        gpio_num_t pinEcho;
        uint16_t maxDistanceCm = 400;
    };

    static constexpr double SOUND_SPEED_CM_PER_US = 343.0 * 100 / 1'000'000;

    explicit HCSR04Driver(const Config& cfg);

    esp_err_t init();
    esp_err_t measure(uint16_t& outCm) override;

  private:
    const Config _cfg;
    const uint32_t _timeoutUs;
};
