#pragma once

#include <cstdint>

#include "RgbAnimation.h"
#include "RgbLedDriver.h"

class RgbPlayer {
  public:
    explicit RgbPlayer(IRgbLedDriver& driver);

    esp_err_t start(const RgbEffect& effect, uint32_t nowMs, uint32_t durationMs = 0);
    esp_err_t start(const RgbAnimation& animation, uint32_t nowMs);

    esp_err_t tick(uint32_t nowMs);
    esp_err_t stop();

    bool isDone() const;

  private:
    esp_err_t startCurrentStep(uint32_t nowMs);

    IRgbLedDriver& _driver;

    RgbAnimation _animation;
    size_t _stepIndex = 0;
    uint32_t _loopCount = 0;
    uint32_t _stepStartMs = 0;
    bool _isDone = true;
};
