#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "IndicatorCommand.h"
#include "RgbAnimation.h"
#include "SoundAnimation.h"

class IndicatorsProxy {
  public:
    explicit IndicatorsProxy(QueueHandle_t commandQueue);

    void start(const RgbAnimation& animation);
    void start(const SoundAnimation& animation);
    void start(const RgbAnimation& rgbAnimation, const SoundAnimation& soundAnimation);
    void stop();

  private:
    QueueHandle_t _commandQueue;
};
