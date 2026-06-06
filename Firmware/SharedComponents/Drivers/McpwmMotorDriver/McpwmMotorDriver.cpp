#include "McpwmMotorDriver.h"

#include <algorithm>
#include <cmath>

#include "esp_check.h"
#include "esp_log.h"

static const char* TAG = "McpwmMotorDriver";

// ---- Initialization ----

McpwmMotorDriver::McpwmMotorDriver(mcpwm_oper_handle_t oper, const Config& cfg, uint32_t periodTicks)
    : _cfg(cfg), _oper(oper), _periodTicks(periodTicks) {
}

esp_err_t McpwmMotorDriver::init() {
    mcpwm_comparator_config_t cmpCfg{};
    cmpCfg.flags.update_cmp_on_tez = true;

    // Comparators
    ESP_RETURN_ON_ERROR(
        mcpwm_new_comparator(_oper, &cmpCfg, &_cmpIn1), TAG, "new comparator IN1 (GPIO %d)", _cfg.pinIn1);

    ESP_RETURN_ON_ERROR(
        mcpwm_new_comparator(_oper, &cmpCfg, &_cmpIn2), TAG, "new comparator IN2 (GPIO %d)", _cfg.pinIn2);

    // Generators
    auto initGenerator = [&](gpio_num_t pin, mcpwm_cmpr_handle_t cmp, mcpwm_gen_handle_t& gen) -> esp_err_t {
        mcpwm_generator_config_t genCfg{};
        genCfg.gen_gpio_num = pin;

        ESP_RETURN_ON_ERROR(mcpwm_new_generator(_oper, &genCfg, &gen), TAG, "new generator GPIO %d", pin);

        ESP_RETURN_ON_ERROR(
            mcpwm_generator_set_action_on_timer_event(
                gen,
                MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)),
            TAG,
            "gen action on zero GPIO %d",
            pin);

        ESP_RETURN_ON_ERROR(
            mcpwm_generator_set_action_on_compare_event(
                gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmp, MCPWM_GEN_ACTION_LOW)),
            TAG,
            "gen action on compare GPIO %d",
            pin);

        return ESP_OK;
    };

    ESP_RETURN_ON_ERROR(initGenerator(_cfg.pinIn1, _cmpIn1, _genIn1), TAG, "gen IN1");
    ESP_RETURN_ON_ERROR(initGenerator(_cfg.pinIn2, _cmpIn2, _genIn2), TAG, "gen IN2");

    // Default state
    ESP_RETURN_ON_ERROR(setSpeed(0.0f), TAG, "initial coast");

    ESP_LOGI(TAG, "init OK - IN1=GPIO%d  IN2=GPIO%d", _cfg.pinIn1, _cfg.pinIn2);
    return ESP_OK;
}

// ---- Control methods ----

esp_err_t McpwmMotorDriver::setSpeed(float speed) {
    speed = std::clamp(speed, -1.0f, 1.0f);

    if (std::fabs(speed) < 0.0001f) {
        return applyCompare(0, 0);
    }

    const uint32_t active = static_cast<uint32_t>(std::fabs(speed) * static_cast<float>(_periodTicks));
    if (speed > 0.0f) {
        return applyCompare(active, 0);
    } else {
        return applyCompare(0, active);
    }
}

esp_err_t McpwmMotorDriver::setHigh() {
    return applyCompare(_periodTicks, _periodTicks);
}

// ---- Private methods ----

esp_err_t McpwmMotorDriver::applyCompare(uint32_t cmpIn1, uint32_t cmpIn2) {
    ESP_RETURN_ON_ERROR(mcpwm_comparator_set_compare_value(_cmpIn1, cmpIn1), TAG, "set compare IN1");
    ESP_RETURN_ON_ERROR(mcpwm_comparator_set_compare_value(_cmpIn2, cmpIn2), TAG, "set compare IN2");
    return ESP_OK;
}
