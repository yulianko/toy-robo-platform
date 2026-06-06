#pragma once

#include "DRV8833Module.h"

/**
 * @brief Differential drive. Knows robot geometry and translates movement commands into per-motor speeds.
 *
 * Motor layout (top view):
 *
 *        [front]
 *    A ──┤   ├── B
 *        [back]
 *
 * Config::invertA / invertB corrects for mirrored motor mounting.
 */
class Drivetrain {
  public:
    struct Config {
        bool invertA = false;
        bool invertB = false;
    };

    Drivetrain(DRV8833Module& module, const Config& cfg);

    esp_err_t forward(float speed);
    esp_err_t backward(float speed);
    esp_err_t turnLeft(float speed);
    esp_err_t turnRight(float speed);
    esp_err_t pivotLeft(float speed);
    esp_err_t pivotRight(float speed);
    esp_err_t brake();
    esp_err_t coast();

  private:
    esp_err_t apply(float speedA, float speedB);

    DRV8833Module& _module;
    const Config _cfg;
};
