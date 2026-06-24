#pragma once

#include "SoundAnimation.h"
#include "SoundEffect.h"

namespace RobotMenuSounds {

// Entry sound when entering menu
static const SoundAnimation menuEntry() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{600, 1200}, 120},
        {EffectTone{1200}, 80},
        {EffectFreqSlide{1200, 900}, 100},
        {EffectSilence{}, 60},
    };
    static const SoundAnimation sound = SoundAnimation("menu_entry", steps, 4, 1);
    return sound;
}

// Single beep for mode navigation
static const SoundAnimation modeBeep() {
    static const SoundAnimation::Step steps[] = {
        {EffectTone{800}, 80},
        {EffectSilence{}, 40},
    };
    static const SoundAnimation sound = SoundAnimation("mode_beep", steps, 2, 1);
    return sound;
}

// Confirmation sound when entering a mode
static const SoundAnimation modeConfirm() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{800, 1600}, 120},
        {EffectTone{1600}, 100},
        {EffectFreqSlide{1600, 1200}, 80},
        {EffectSilence{}, 60},
    };
    static const SoundAnimation sound = SoundAnimation("mode_confirm", steps, 4, 1);
    return sound;
}

// Generate beeps based on mode index (1-based)
static const SoundAnimation modeIndexBeeps(size_t count) {
    static SoundAnimation::Step steps[16];  // Max 8 modes * 2 steps each
    size_t stepCount = 0;

    for (size_t i = 0; i < count && stepCount < 14; ++i) {
        steps[stepCount++] = {EffectTone{1000}, 120};
        if (i < count - 1) {  // No silence after last beep
            steps[stepCount++] = {EffectSilence{}, 80};
        }
    }

    if (stepCount < 16) {
        steps[stepCount++] = {EffectSilence{}, 100};  // Final silence
    }

    static SoundAnimation sound = SoundAnimation("mode_index", steps, stepCount, 1);
    sound = SoundAnimation("mode_index", steps, stepCount, 1);
    return sound;
}

}  // namespace RobotMenuSounds
