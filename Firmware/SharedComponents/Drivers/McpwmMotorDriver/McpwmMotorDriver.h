#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "esp_err.h"

/**
 * @brief Single H-bridge channel driven by one MCPWM operator.
 * Sign-magnitude PWM logic:
 *  speed > 0  │  IN1 = PWM      │  IN2 = 0
 *  speed < 0  │  IN1 = 0        │  IN2 = PWM
 *  speed = 0  │  IN1 = 0        │  IN2 = 0
 *  setHigh()  │  IN1 = 100%     │  IN2 = 100%
 */
class McpwmMotorDriver {
  public:
    struct Config {
        gpio_num_t pinIn1;
        gpio_num_t pinIn2;
    };

    explicit McpwmMotorDriver(mcpwm_oper_handle_t oper, const Config& cfg, uint32_t periodTicks);
    McpwmMotorDriver(const McpwmMotorDriver&) = delete;
    McpwmMotorDriver& operator=(const McpwmMotorDriver&) = delete;

    esp_err_t init();

    esp_err_t setSpeed(float speed);
    esp_err_t setHigh();

  private:
    esp_err_t applyCompare(uint32_t cmpIn1, uint32_t cmpIn2);

    const Config _cfg;
    const mcpwm_oper_handle_t _oper;
    const uint32_t _periodTicks;

    mcpwm_cmpr_handle_t _cmpIn1{nullptr};
    mcpwm_cmpr_handle_t _cmpIn2{nullptr};
    mcpwm_gen_handle_t _genIn1{nullptr};
    mcpwm_gen_handle_t _genIn2{nullptr};
};
