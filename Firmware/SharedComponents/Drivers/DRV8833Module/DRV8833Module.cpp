#include "DRV8833Module.h"

#include "esp_check.h"
#include "esp_log.h"

static const char* TAG = "DRV8833Module";

static constexpr uint32_t MCPWM_RESOLUTION_HZ = 1'000'000;

// ---- Initialization ----

DRV8833Module::DRV8833Module(const Config& cfg) : _cfg(cfg) {
}

esp_err_t DRV8833Module::init() {
    const uint32_t periodTicks = MCPWM_RESOLUTION_HZ / _cfg.pwmFreqHz;

    // STBY pin setup (optional)
    if (_cfg.pinStby != GPIO_NUM_NC) {
        gpio_config_t io{
            .pin_bit_mask = 1ULL << _cfg.pinStby,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_RETURN_ON_ERROR(gpio_config(&io), TAG, "GPIO config STBY");
        ESP_RETURN_ON_ERROR(setStby(false), TAG, "STBY low");
    }

    // Timer (shared resource for both channels)
    mcpwm_timer_config_t timerCfg{
        .group_id = _cfg.mcpwmGroupId,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MCPWM_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = periodTicks,
    };
    ESP_RETURN_ON_ERROR(mcpwm_new_timer(&timerCfg, &_timer),
                        TAG,
                        "new timer (group %d, freq %lu Hz)",
                        _cfg.mcpwmGroupId,
                        static_cast<unsigned long>(_cfg.pwmFreqHz));

    // Operator A
    mcpwm_operator_config_t operCfg{.group_id = _cfg.mcpwmGroupId};

    ESP_RETURN_ON_ERROR(mcpwm_new_operator(&operCfg, &_operA), TAG, "new operator A");
    ESP_RETURN_ON_ERROR(mcpwm_operator_connect_timer(_operA, _timer), TAG, "connect operator A → timer");

    // Operator B
    ESP_RETURN_ON_ERROR(mcpwm_new_operator(&operCfg, &_operB), TAG, "new operator B");
    ESP_RETURN_ON_ERROR(mcpwm_operator_connect_timer(_operB, _timer), TAG, "connect operator B → timer");

    // Motor drivers (emplace and init after handles are valid)
    if (_cfg.pinAIn1 != GPIO_NUM_NC && _cfg.pinAIn2 != GPIO_NUM_NC) {
        _motorA.emplace(_operA, McpwmMotorDriver::Config{_cfg.pinAIn1, _cfg.pinAIn2}, periodTicks);
        ESP_RETURN_ON_ERROR(_motorA->init(), TAG, "motor A init");
    } else {
        ESP_LOGW(TAG, "Motor A pins not configured, skipping initialization");
    }

    if (_cfg.pinBIn1 != GPIO_NUM_NC && _cfg.pinBIn2 != GPIO_NUM_NC) {
        _motorB.emplace(_operB, McpwmMotorDriver::Config{_cfg.pinBIn1, _cfg.pinBIn2}, periodTicks);
        ESP_RETURN_ON_ERROR(_motorB->init(), TAG, "motor B init");
    } else {
        ESP_LOGW(TAG, "Motor B pins not configured, skipping initialization");
    }

    // Start timer
    ESP_RETURN_ON_ERROR(mcpwm_timer_enable(_timer), TAG, "enable timer");
    ESP_RETURN_ON_ERROR(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP), TAG, "start timer");

    // Wake up the driver
    ESP_RETURN_ON_ERROR(resume(), TAG, "resume");

    ESP_LOGI(TAG,
             "init OK - A(IN1=%d IN2=%d)  B(IN1=%d IN2=%d)  STBY=%d",
             _cfg.pinAIn1,
             _cfg.pinAIn2,
             _cfg.pinBIn1,
             _cfg.pinBIn2,
             _cfg.pinStby);
    return ESP_OK;
}

// ---- Control methods ----

esp_err_t DRV8833Module::suspend() {
    // Default state
    ESP_RETURN_ON_ERROR(coastA(), TAG, "coastA");
    ESP_RETURN_ON_ERROR(coastB(), TAG, "coastB");

    ESP_RETURN_ON_ERROR(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_STOP_EMPTY), TAG, "stop timer");
    ESP_RETURN_ON_ERROR(setStby(false), TAG, "STBY low");
    ESP_LOGD(TAG, "suspended");
    return ESP_OK;
}

esp_err_t DRV8833Module::resume() {
    ESP_RETURN_ON_ERROR(setStby(true), TAG, "STBY high");
    ESP_RETURN_ON_ERROR(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP), TAG, "restart timer");
    ESP_LOGD(TAG, "resumed");
    return ESP_OK;
}

esp_err_t DRV8833Module::brakeA() {
    if (_motorA.has_value()) {
        ESP_RETURN_ON_ERROR(_motorA->setHigh(), TAG, "brake motor A");
    }

    return ESP_OK;
}

esp_err_t DRV8833Module::brakeB() {
    if (_motorB.has_value()) {
        ESP_RETURN_ON_ERROR(_motorB->setHigh(), TAG, "brake motor B");
    }

    return ESP_OK;
}

esp_err_t DRV8833Module::coastA() {
    if (_motorA.has_value()) {
        ESP_RETURN_ON_ERROR(_motorA->setSpeed(0.0f), TAG, "coast motor A");
    }

    return ESP_OK;
}

esp_err_t DRV8833Module::coastB() {
    if (_motorB.has_value()) {
        ESP_RETURN_ON_ERROR(_motorB->setSpeed(0.0f), TAG, "coast motor B");
    }

    return ESP_OK;
}

esp_err_t DRV8833Module::setSpeedA(float speedA) {
    if (_motorA.has_value()) {
        ESP_RETURN_ON_ERROR(_motorA->setSpeed(speedA), TAG, "set speed motor A");
    }

    return ESP_OK;
}

esp_err_t DRV8833Module::setSpeedB(float speedB) {
    if (_motorB.has_value()) {
        ESP_RETURN_ON_ERROR(_motorB->setSpeed(speedB), TAG, "set speed motor B");
    }

    return ESP_OK;
}

// ---- Private methods ----

esp_err_t DRV8833Module::setStby(bool active) {
    if (_cfg.pinStby == GPIO_NUM_NC) {
        return ESP_OK;
    }

    return gpio_set_level(_cfg.pinStby, active ? 1 : 0);
}
