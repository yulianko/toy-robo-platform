#include "ConfigMode.h"

#include <esp_log.h>

#include "DistanceMode.h"
#include "RobotAnimations.h"
#include "RobotEvent.h"
#include "RobotSounds.h"

static const char* TAG = "DistanceMode";

void ConfigMode::onEnter(RobotContext& ctx) {
    _ctx = &ctx;
    ctx.indicators.start(RobotAnimations::pulse(RgbColor::white()), RobotSounds::curiosity());
    ESP_LOGI(TAG, "entered");
}

void ConfigMode::onExit() {
    _ctx->indicators.stop();
    _ctx = nullptr;
    ESP_LOGI(TAG, "exited");
}

void ConfigMode::onEvent(const RobotEvent& event) {
    if (event.type != RobotEvent::Type::PUSH_BUTTON_LONG_PRESSED) {
        return;
    }

    _ctx->wifiProxy.postReset();
    _ctx->indicators.start(RobotAnimations::agreement(), RobotSounds::agreement());

    ESP_LOGI(TAG, "Reset WiFi");
}
