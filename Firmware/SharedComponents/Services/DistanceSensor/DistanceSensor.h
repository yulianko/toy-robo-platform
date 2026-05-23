#pragma once

#include <esp_err.h>

#include "HCSR04Driver.h"

class DistanceSensor {
  public:
    typedef void (*OnMeasure)(uint16_t distanceCm);
    typedef void (*OnError)(esp_err_t err);

    struct Config {
        OnMeasure onMeasure;
        OnError onError = nullptr;
    };

    DistanceSensor(IDistanceDriver& driver, Config cfg);

    esp_err_t measure();
    esp_err_t measureDirect(uint16_t& outCm);

  private:
    IDistanceDriver& _driver;
    Config _cfg;
};
