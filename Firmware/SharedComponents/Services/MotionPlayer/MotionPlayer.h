#pragma once
#include <cstdint>

#include "Drivetrain.h"
#include "MotionSequence.h"

class MotionPlayer {
  public:
    explicit MotionPlayer(Drivetrain& drivetrain);

    esp_err_t play(const MotionSequence& sequence);
    esp_err_t stop();
    esp_err_t brake();
    void tick();
    bool isPlaying() const;

  private:
    esp_err_t applyStep(const MotionStep& step);
    esp_err_t advance();

    Drivetrain& _drivetrain;

    MotionSequence _sequence;
    bool _playing = false;
    uint8_t _stepIndex = 0;
    uint8_t _loopCount = 0;
    uint32_t _stepStartMs = 0;
};
