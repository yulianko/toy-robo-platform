#pragma once

#include <cstdint>

#include "SoundAnimation.h"
#include "SoundDriver.h"
#include "SoundEffect.h"

class SoundPlayer {
  public:
    explicit SoundPlayer(ISoundDriver& driver);

    esp_err_t start(const SoundEffect& effect, uint32_t nowMs, uint32_t durationMs = 0);
    esp_err_t start(const SoundAnimation& animation, uint32_t nowMs);

    esp_err_t tick(uint32_t nowMs);
    esp_err_t stop();

    bool isDone() const;

  private:
    esp_err_t startCurrentStep(uint32_t nowMs);

    ISoundDriver& _driver;

    SoundAnimation _animation;
    size_t _stepIndex = 0;
    uint32_t _loopCount = 0;
    uint32_t _stepStartMs = 0;
    bool _isDone = true;
};
