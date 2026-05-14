#pragma once

#include <cmath>
#include <cstdint>
#include <variant>

#include "RgbColor.h"
#include "RgbLedDriver.h"
#include "esp_log.h"

struct EffectOff {
    esp_err_t apply(IRgbLedDriver& driver, uint32_t /*elapsedMs*/) const {
        return driver.off();
    }
};

struct EffectSolid {
    RgbColor color;

    esp_err_t apply(IRgbLedDriver& driver, uint32_t /*elapsedMs*/) const {
        return driver.setColor(color.r, color.g, color.b);
    }
};

struct EffectBlink {
    RgbColor color;
    uint32_t periodMs = 1000;
    float offRatio = 1.0f / 3.0f;

    esp_err_t apply(IRgbLedDriver& driver, uint32_t elapsedMs) const {
        if (periodMs == 0) {
            return ESP_ERR_INVALID_ARG;
        }

        uint32_t phase = elapsedMs % periodMs;
        uint32_t offStart = static_cast<uint32_t>(periodMs * (1.0f - offRatio) / 2.0f);
        uint32_t offEnd = static_cast<uint32_t>(periodMs * (1.0f + offRatio) / 2.0f);

        if (phase >= offStart && phase < offEnd) {
            return driver.off();
        }

        return driver.setColor(color.r, color.g, color.b);
    }
};

struct EffectPulse {
    RgbColor color;
    uint32_t periodMs = 1000;
    float minBrightness = 0.0f;

    esp_err_t apply(IRgbLedDriver& driver, uint32_t elapsedMs) const {
        if (periodMs == 0) {
            return ESP_ERR_INVALID_ARG;
        }

        float t = static_cast<float>(elapsedMs % periodMs) / static_cast<float>(periodMs);
        float cosVal = std::cos(2.0f * M_PI * t);
        float brightness = minBrightness + (1.0f - minBrightness) * (cosVal + 1.0f) / 2.0f;

        RgbColor c = color.scaled(brightness);
        return driver.setColor(c.r, c.g, c.b);
    }
};

using RgbEffect = std::variant<EffectOff, EffectSolid, EffectBlink, EffectPulse>;

inline esp_err_t applyEffect(const RgbEffect& effect, IRgbLedDriver& driver, uint32_t elapsedMs) {
    return std::visit([&](const auto& e) { return e.apply(driver, elapsedMs); }, effect);
}
