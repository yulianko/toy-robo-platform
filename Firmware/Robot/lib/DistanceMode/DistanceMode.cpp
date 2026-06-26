#include "DistanceMode.h"

#include <esp_log.h>

#include "RobotEvent.h"
#include "RobotMenuAnimations.h"
#include "RobotMenuSounds.h"
#include "RobotSounds.h"

static const char* TAG = "DistanceMode";

void DistanceMode::onEnter(RobotContext& ctx) {
    _ctx = &ctx;
    ctx.distance.setProfile(DistanceScanProfile::normal());
    ctx.indicators.start(RobotMenuAnimations::pulse(RgbColor::white(), 1800, 0.1f));
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
    ESP_LOGI(TAG, "%s - %lu cm", rangeToString(range), cm);
}

void DistanceMode::updateIndicators(RobotEvent::DistanceData::Range range) {
    switch (range) {
        case RobotEvent::DistanceData::Range::Critical:
            _ctx->indicators.start(RobotMenuAnimations::pulse(RgbColor::red(), 300, 0.5f), RobotSounds::disagreement());
            break;
        case RobotEvent::DistanceData::Range::Close:
            _ctx->indicators.start(RobotMenuAnimations::pulse(RgbColor::red(), 600, 0.3f), RobotSounds::surprise());
            break;
        case RobotEvent::DistanceData::Range::Near:
            _ctx->indicators.start(RobotMenuAnimations::pulse(RgbColor::yellow(), 1000, 0.1f));
            break;
        case RobotEvent::DistanceData::Range::Far:
            _ctx->indicators.start(RobotMenuAnimations::pulse(RgbColor::yellow(), 1500, 0.05f));
            break;
        case RobotEvent::DistanceData::Range::Clear:
        case RobotEvent::DistanceData::Range::Unknown:
            _ctx->indicators.start(RobotMenuAnimations::pulse(RgbColor::green(), 1800, 0.1f));
            break;
    }
}

const char* DistanceMode::rangeToString(RobotEvent::DistanceData::Range r) {
    switch (r) {
        case RobotEvent::DistanceData::Range::Critical:
            return "CRITICAL";
        case RobotEvent::DistanceData::Range::Close:
            return "CLOSE";
        case RobotEvent::DistanceData::Range::Near:
            return "NEAR";
        case RobotEvent::DistanceData::Range::Far:
            return "FAR";
        case RobotEvent::DistanceData::Range::Clear:
            return "CLEAR";
        case RobotEvent::DistanceData::Range::Unknown:
            return "UNKNOWN";
    }
    return "?";
}
