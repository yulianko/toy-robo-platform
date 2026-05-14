#pragma once

#include "RgbAnimation.h"

namespace RobotEmotions {

static const RgbAnimation blue() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{RgbColor::blue()}, /*durationMs=*/0},
    };
    static const RgbAnimation animation = RgbAnimation("blue", steps, 1, /*loops=*/0);
    return animation;
}

static const RgbAnimation curiosity() {
    static const RgbAnimation::Step steps[] = {
        {EffectPulse{{0, 180, 255}, /*periodMs=*/1800, /*minBrightness=*/0.1f},
         /*durationMs=*/1800},  // Slow blue pulse
    };

    static const RgbAnimation animation = RgbAnimation("curiosity", steps, 1, /*loops=*/6);
    return animation;
}

static const RgbAnimation surprise() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{{255, 255, 255}}, /*durationMs=*/80},  // Bright white flash
        {EffectPulse{{200, 200, 255}, /*periodMs=*/400, /*minBrightness=*/0.0f}, /*durationMs=*/400},  // Quick fade out
        {EffectOff{}, /*durationMs=*/150},                                                             // Pause
        {EffectSolid{{255, 255, 255}}, /*durationMs=*/80},  // Bright white flash
        {EffectPulse{{200, 200, 255}, /*periodMs=*/400, /*minBrightness=*/0.0f}, /*durationMs=*/400},  // Quick fade out
        {EffectOff{}, /*durationMs=*/150},                                                             // Pause
    };
    static const RgbAnimation animation = RgbAnimation("surprise", steps, 6, /*loops=*/1);
    return animation;
}

static const RgbAnimation agreement() {
    static const RgbAnimation::Step steps[] = {
        {EffectPulse{{0, 220, 80}, /*periodMs=*/500, /*minBrightness=*/0.0f}, /*durationMs=*/500},  // Slow green pulse
        {EffectOff{}, /*durationMs=*/150},                                                          // Pause
        {EffectPulse{{0, 220, 80}, /*periodMs=*/500, /*minBrightness=*/0.0f}, /*durationMs=*/500},  // Slow green pulse
        {EffectOff{}, /*durationMs=*/1},                                                            // Pause
    };
    static const RgbAnimation animation = RgbAnimation("agreement", steps, 4, /*loops=*/1);
    return animation;
}

static const RgbAnimation disagreement() {
    static const RgbAnimation::Step steps[] = {
        {EffectBlink{{255, 30, 0}, /*periodMs=*/200, /*offRatio=*/0.4f}, /*durationMs=*/600},  // Fast red blink
        {EffectOff{}, /*durationMs=*/100},                                                     // Short pause
        {EffectBlink{{255, 30, 0}, /*periodMs=*/200, /*offRatio=*/0.4f}, /*durationMs=*/600},  // Fast red blink
        {EffectOff{}, /*durationMs=*/1},                                                       // Pause
    };
    static const RgbAnimation animation = RgbAnimation("disagreement", steps, 4, /*loops=*/1);
    return animation;
}

}  // namespace RobotEmotions
