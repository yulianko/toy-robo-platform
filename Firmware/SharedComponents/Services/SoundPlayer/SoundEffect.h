#pragma once

#include <SoundDriver.h>
#include <esp_err.h>

#include <cstdint>
#include <variant>

struct EffectSilence {
    esp_err_t apply(ISoundDriver& driver, uint32_t /*elapsedMs*/, uint32_t /*stepDurationMs*/) const {
        return driver.off();
    }
};

struct EffectTone {
    uint32_t freq;

    esp_err_t apply(ISoundDriver& driver, uint32_t /*elapsedMs*/, uint32_t /*stepDurationMs*/) const {
        return driver.setFrequency(freq);
    }
};

struct EffectFreqSlide {
    uint32_t freqFrom;
    uint32_t freqTo;

    esp_err_t apply(ISoundDriver& driver, uint32_t elapsedMs, uint32_t stepDurationMs) const {
        if (stepDurationMs == 0) {
            return driver.setFrequency(freqTo);
        }

        float progress = static_cast<float>(elapsedMs) / static_cast<float>(stepDurationMs);
        if (progress > 1.0f) progress = 1.0f;

        float freq =
            static_cast<float>(freqFrom) + (static_cast<float>(freqTo) - static_cast<float>(freqFrom)) * progress;
        return driver.setFrequency(static_cast<uint32_t>(freq));
    }
};

using SoundEffect = std::variant<EffectSilence, EffectTone, EffectFreqSlide>;

inline esp_err_t applyEffect(const SoundEffect& effect,
                             ISoundDriver& driver,
                             uint32_t elapsedMs,
                             uint32_t totalDurationMs) {
    return std::visit([&](const auto& e) { return e.apply(driver, elapsedMs, totalDurationMs); }, effect);
}
