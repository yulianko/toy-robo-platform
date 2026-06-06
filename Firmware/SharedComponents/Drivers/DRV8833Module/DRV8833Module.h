#pragma once

#include <cstdint>
#include <optional>

#include "McpwmMotorDriver.h"
#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "esp_err.h"

/**
 * @brief Owns the DRV8833 dual-channel module hardware.
 * MCPWM resource tree:
 *   Timer  (one, shared):
 *     ├── Operator 0  ->  McpwmMotorDriver (channel A)
 *     └── Operator 1  ->  McpwmMotorDriver (channel B)
 */
class DRV8833Module {
  public:
    struct Config {
        // Motor A pins
        gpio_num_t pinAIn1 = GPIO_NUM_NC;
        gpio_num_t pinAIn2 = GPIO_NUM_NC;

        // Motor B pins
        gpio_num_t pinBIn1 = GPIO_NUM_NC;
        gpio_num_t pinBIn2 = GPIO_NUM_NC;

        // STBY - pass GPIO_NUM_NC if hardwired to VCC
        gpio_num_t pinStby = GPIO_NUM_NC;

        int mcpwmGroupId = 0;
        uint32_t pwmFreqHz = 20'000;
    };

    explicit DRV8833Module(const Config& cfg);
    DRV8833Module(const DRV8833Module&) = delete;
    DRV8833Module& operator=(const DRV8833Module&) = delete;

    esp_err_t init();

    esp_err_t suspend();
    esp_err_t resume();

    esp_err_t brakeA();
    esp_err_t brakeB();
    esp_err_t coastA();
    esp_err_t coastB();
    esp_err_t setSpeedA(float speedA);
    esp_err_t setSpeedB(float speedB);

  private:
    esp_err_t setStby(bool active);

    const Config _cfg;

    mcpwm_timer_handle_t _timer{nullptr};
    mcpwm_oper_handle_t _operA{nullptr};
    mcpwm_oper_handle_t _operB{nullptr};

    std::optional<McpwmMotorDriver> _motorA;
    std::optional<McpwmMotorDriver> _motorB;
};
