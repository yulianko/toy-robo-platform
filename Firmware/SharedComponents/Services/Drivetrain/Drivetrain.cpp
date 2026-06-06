#include "Drivetrain.h"

#include <algorithm>

#include "esp_log.h"

static const char* TAG = "Drivetrain";

Drivetrain::Drivetrain(DRV8833Module& module, const Config& cfg) : _module(module), _cfg(cfg) {
}

esp_err_t Drivetrain::forward(float speed) {
    ESP_LOGD(TAG, "forward %.2f", speed);
    return apply(speed, speed);
}

esp_err_t Drivetrain::backward(float speed) {
    ESP_LOGD(TAG, "backward %.2f", speed);
    return apply(-speed, -speed);
}

esp_err_t Drivetrain::turnLeft(float speed) {
    ESP_LOGD(TAG, "turnLeft %.2f", speed);
    return apply(0.0f, speed);
}

esp_err_t Drivetrain::turnRight(float speed) {
    ESP_LOGD(TAG, "turnRight %.2f", speed);
    return apply(speed, 0.0f);
}

esp_err_t Drivetrain::pivotLeft(float speed) {
    ESP_LOGD(TAG, "pivotLeft %.2f", speed);
    return apply(-speed, speed);
}

esp_err_t Drivetrain::pivotRight(float speed) {
    ESP_LOGD(TAG, "pivotRight %.2f", speed);
    return apply(speed, -speed);
}

esp_err_t Drivetrain::brake() {
    ESP_LOGD(TAG, "brake");
    esp_err_t ret = _module.brakeA();
    ret |= _module.brakeB();
    return ret;
}

esp_err_t Drivetrain::coast() {
    ESP_LOGD(TAG, "coast");
    esp_err_t ret = _module.coastA();
    ret |= _module.coastB();
    return ret;
}

esp_err_t Drivetrain::apply(float speedA, float speedB) {
    if (_cfg.invertA) speedA = -speedA;
    if (_cfg.invertB) speedB = -speedB;

    esp_err_t ret = _module.setSpeedA(speedA);
    ret |= _module.setSpeedB(speedB);
    return ret;
}
