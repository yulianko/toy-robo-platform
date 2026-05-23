#include "DistanceSensor.h"

DistanceSensor::DistanceSensor(IDistanceDriver& driver, Config cfg) : _driver(driver), _cfg(cfg) {
}

esp_err_t DistanceSensor::measure() {
    uint16_t distanceCm = 0;
    esp_err_t err = _driver.measure(distanceCm);

    if (err == ESP_OK) {
        if (_cfg.onMeasure) {
            _cfg.onMeasure(distanceCm);
        }
    } else {
        if (_cfg.onError) {
            _cfg.onError(err);
        }
    }

    return err;
}

esp_err_t DistanceSensor::measureDirect(uint16_t& outCm) {
    return _driver.measure(outCm);
}
