#pragma once

#include "RgbAnimation.h"
#include "RgbColor.h"
#include "RgbEffect.h"

namespace RobotMenuAnimations {

// Entry animation - white flash for menu entry
static const RgbAnimation menuEntry() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{RgbColor::white()}, 600},
    };
    static const RgbAnimation animation = RgbAnimation("menu_entry", steps, 1, 1);
    return animation;
}

// Double blink animation for mode confirmation
static const RgbAnimation modeConfirm(const RgbColor& color) {
    static RgbAnimation::Step steps[5];  // Static storage for the animation

    steps[0] = {EffectSolid{color}, 150};
    steps[1] = {EffectOff{}, 100};
    steps[2] = {EffectSolid{color}, 150};
    steps[3] = {EffectOff{}, 100};
    steps[4] = {EffectSolid{color}, 200};

    static RgbAnimation animation = RgbAnimation("mode_confirm", steps, 5, 1);
    animation = RgbAnimation("mode_confirm", steps, 5, 1);
    return animation;
}

// Pulse animation for menu mode display
static const RgbAnimation modePulse(const RgbColor& color) {
    static RgbAnimation::Step steps[1];

    steps[0] = {EffectPulse{color, 2000, 0.1f}, 0};  // Continuous pulse

    static RgbAnimation animation = RgbAnimation("mode_pulse", steps, 1, 0);
    animation = RgbAnimation("mode_pulse", steps, 1, 0);
    return animation;
}

static const RgbAnimation pulse(const RgbColor& color, uint32_t period, float minBrightness) {
    static RgbAnimation::Step steps[1];

    steps[0] = {EffectPulse{color, period, minBrightness}, 0};  // Continuous pulse

    static RgbAnimation animation = RgbAnimation("pulse", steps, 1, 0);
    animation = RgbAnimation("pulse", steps, 1, 0);
    return animation;
}

}  // namespace RobotMenuAnimations
