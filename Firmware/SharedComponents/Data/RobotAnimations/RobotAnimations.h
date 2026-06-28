#pragma once

#include "RgbAnimation.h"
#include "RgbColor.h"
namespace RobotAnimations {

// Generic pulse
static const RgbAnimation pulse(const RgbColor& color, uint32_t periodMs = 2000, float minBrightness = 0.1f) {
    static RgbAnimation::Step steps[1];
    steps[0] = {EffectPulse{color, periodMs, minBrightness}, 0};  // Continuous pulse
    static RgbAnimation animation = RgbAnimation("pulse", steps, 1, 0);
    animation = RgbAnimation("pulse", steps, 1, 0);
    return animation;
}

// Cyan, a jerky "looking around" rhythm
static const RgbAnimation exploring() {
    static const RgbAnimation::Step steps[] = {
        {EffectPulse{{0, 255, 200}, /*periodMs=*/800, /*minBrightness=*/0.2f}, /*durationMs=*/800},
        {EffectOff{}, /*durationMs=*/300},  // Pause, as if the robot stopped to think
        {EffectPulse{{0, 255, 200}, /*periodMs=*/800, /*minBrightness=*/0.2f}, /*durationMs=*/800},
    };
    static const RgbAnimation animation = RgbAnimation("exploring", steps, 3, /*loops=*/2);
    return animation;
}

// Deep blue, the pulsation very soft and continuous
static const RgbAnimation curiosity() {
    static const RgbAnimation::Step steps[] = {
        {EffectPulse{{0, 100, 255}, /*periodMs=*/2000, /*minBrightness=*/0.05f}, /*durationMs=*/2000},
    };
    static const RgbAnimation animation = RgbAnimation("curiosity", steps, 1, /*loops=*/4);
    return animation;
}

// Super fast bright flash
static const RgbAnimation surprise() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{{255, 255, 255}}, /*durationMs=*/60},
        {EffectPulse{{150, 150, 255}, /*periodMs=*/300, /*minBrightness=*/0.0f}, /*durationMs=*/300},
        {EffectOff{}, /*durationMs=*/100},
        {EffectSolid{{255, 255, 255}}, /*durationMs=*/60},
        {EffectPulse{{150, 150, 255}, /*periodMs=*/300, /*minBrightness=*/0.0f}, /*durationMs=*/300},
    };
    static const RgbAnimation animation = RgbAnimation("surprise", steps, 5, /*loops=*/1);
    return animation;
}

// Clear green flash with confirmation
static const RgbAnimation agreement() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{{0, 255, 0}}, /*durationMs=*/150},
        {EffectOff{}, /*durationMs=*/100},
        {EffectSolid{{0, 255, 0}}, /*durationMs=*/150},
    };
    static const RgbAnimation animation = RgbAnimation("agreement", steps, 3, /*loops=*/1);
    return animation;
}

// Fast orange blinking to avoid confusion with Danger
static const RgbAnimation disagreement() {
    static const RgbAnimation::Step steps[] = {
        {EffectBlink{{255, 100, 0}, /*periodMs=*/150, /*offRatio=*/0.5f},
         /*durationMs=*/450},
        {EffectOff{}, /*durationMs=*/150},
        {EffectBlink{{255, 100, 0}, /*periodMs=*/150, /*offRatio=*/0.5f}, /*durationMs=*/450},
    };
    static const RgbAnimation animation = RgbAnimation("disagreement", steps, 3, /*loops=*/1);
    return animation;
}

// Maximum aggressive pure red without pauses, hard strobe
static const RgbAnimation danger() {
    static const RgbAnimation::Step steps[] = {
        {EffectBlink{{255, 0, 0}, /*periodMs=*/100, /*offRatio=*/0.5f}, /*durationMs=*/1000},
    };
    static const RgbAnimation animation = RgbAnimation("danger", steps, 1, /*loops=*/3);
    return animation;
}

// Unique, triple accelerating blinking that transitions to stable light
static const RgbAnimation menuEntry() {
    static const RgbAnimation::Step steps[] = {
        {EffectSolid{{180, 0, 255}}, /*durationMs=*/100},
        {EffectOff{}, /*durationMs=*/100},
        {EffectSolid{{180, 0, 255}}, /*durationMs=*/150},
        {EffectOff{}, /*durationMs=*/80},
        {EffectSolid{{180, 0, 255}}, /*durationMs=*/300},
    };
    static const RgbAnimation animation = RgbAnimation("menu_entry", steps, 5, /*loops=*/1);
    return animation;
}

}  // namespace RobotAnimations
