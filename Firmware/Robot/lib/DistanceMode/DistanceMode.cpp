#include "DistanceMode.h"

#include <esp_log.h>

#include "RobotAnimations.h"
#include "RobotEvent.h"
#include "RobotSounds.h"

static const char* TAG = "DistanceMode";

void DistanceMode::onEnter(RobotContext& ctx) {
    _ctx = &ctx;
    ctx.distance.setProfile(DistanceScanProfile::normal());
    ctx.indicators.start(RobotAnimations::exploring(), RobotSounds::exploring());
    ESP_LOGI(TAG, "entered");
}

void DistanceMode::onExit() {
    _ctx->indicators.stop();
    _ctx->distance.setProfile(DistanceScanProfile::idle());
    _ctx = nullptr;
    ESP_LOGI(TAG, "exited");
}

void DistanceMode::onEvent(const RobotEvent& event) {
    if (event.type != RobotEvent::Type::DISTANCE_RANGE_CHANGED) {
        return;
    }

    const RobotEvent::DistanceData::Range range = event.distance.range;
    const uint16_t cm = event.distance.cm;

    updateIndicators(range);
    ESP_LOGI(TAG, "%s - %lu cm", RobotEvent::rangeToString(range), cm);
}

void DistanceMode::updateIndicators(RobotEvent::DistanceData::Range range) {
    switch (range) {
        case RobotEvent::DistanceData::Range::Critical:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::red(), 300, 0.5f), RobotSounds::beep(3));
            break;
        case RobotEvent::DistanceData::Range::Close:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::red(), 600, 0.3f), RobotSounds::beep(2));
            break;
        case RobotEvent::DistanceData::Range::Near:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::orange(), 1000, 0.1f), RobotSounds::beep(1));
            break;
        case RobotEvent::DistanceData::Range::Far:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::yellow(), 1500, 0.05f));
            break;
        case RobotEvent::DistanceData::Range::Clear:
        case RobotEvent::DistanceData::Range::Unknown:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::green(), 1800, 0.1f));
            break;
    }
}
