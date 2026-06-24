#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "IndicatorCommand.h"
#include "RgbAnimation.h"

class IndicatorsProxy {
  public:
    explicit IndicatorsProxy(QueueHandle_t commandQueue);

    void start(const RgbAnimation& animation);
    void stop();

  private:
    QueueHandle_t _commandQueue;
};
