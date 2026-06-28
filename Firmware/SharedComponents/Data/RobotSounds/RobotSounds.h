#pragma once

#include "SoundAnimation.h"

namespace RobotSounds {

// Universal beep
static const SoundAnimation beep(size_t count = 1) {
    static SoundAnimation::Step steps[16];
    size_t stepCount = 0;

    for (size_t i = 0; i < count && stepCount < 14; ++i) {
        steps[stepCount++] = {EffectTone{1200}, 80};
        if (i < count - 1) {
            steps[stepCount++] = {EffectSilence{}, 60};
        }
    }

    if (stepCount < 16) {
        steps[stepCount++] = {EffectSilence{}, 100};
    }

    static SoundAnimation sound = SoundAnimation("beep", steps, stepCount, 1);
    sound = SoundAnimation("beep", steps, stepCount, 1);
    return sound;
}

// Optimistic and neutral
static const SoundAnimation exploring() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1000, 1800}, 160},
        {EffectFreqSlide{1800, 1300}, 140},
        {EffectSilence{}, 100},
    };
    static const SoundAnimation sound = SoundAnimation("exploring", steps, 3, 1);
    return sound;
}

static const SoundAnimation curiosity() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{900, 2400}, 180},
        {EffectFreqSlide{2400, 2100}, 60},
        {EffectFreqSlide{2100, 2500}, 50},
        {EffectFreqSlide{2500, 2100}, 50},
        {EffectFreqSlide{2100, 2600}, 50},
        {EffectFreqSlide{2600, 1900}, 100},
        {EffectSilence{}, 80},
    };
    static const SoundAnimation sound = SoundAnimation("curiosity", steps, 7, 1);
    return sound;
}

// Very sharp upward start
static const SoundAnimation surprise() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1200, 4000}, 60},
        {EffectTone{4000}, 40},
        {EffectFreqSlide{4000, 3400}, 30},
        {EffectFreqSlide{3400, 4000}, 30},
        {EffectFreqSlide{4000, 1500}, 200},
        {EffectSilence{}, 60},
    };
    static const SoundAnimation sound = SoundAnimation("surprise", steps, 6, 1);
    return sound;
}

// Two ascending tones
static const SoundAnimation agreement() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1600, 2200}, 80},
        {EffectSilence{}, 40},
        {EffectFreqSlide{2000, 3000}, 120},  // Second sound higher than the first
        {EffectSilence{}, 50},
    };
    static const SoundAnimation sound = SoundAnimation("agreement", steps, 4, 1);
    return sound;
}

// Two heavy descending tones imitating disappointed
static const SoundAnimation disagreement() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1100, 600}, 140},
        {EffectSilence{}, 80},
        {EffectFreqSlide{800, 400}, 160},  // Second sound lower than the first
        {EffectSilence{}, 80},
    };
    static const SoundAnimation sound = SoundAnimation("disagreement", steps, 4, 1);
    return sound;
}

// Warning siren
static const SoundAnimation danger() {
    static const SoundAnimation::Step steps[] = {
        {EffectTone{2800}, 150},  // High piercing tone
        {EffectTone{1800}, 150},  // Lower tone
        {EffectTone{2800}, 150},
        {EffectTone{1800}, 150},
    };
    static const SoundAnimation sound = SoundAnimation("danger", steps, 4, 1);
    return sound;
}

// Sharp tone jumps upward
static const SoundAnimation menuEntry() {
    static const SoundAnimation::Step steps[] = {
        {EffectTone{1000}, 60},  // First light flash
        {EffectSilence{}, 40},
        {EffectTone{1500}, 60},  // Second flash
        {EffectSilence{}, 40},
        {EffectTone{2200}, 180},  // Final menu fixation (long tone)
        {EffectSilence{}, 60},
    };
    static const SoundAnimation sound = SoundAnimation("menu_entry", steps, 6, 1);
    return sound;
}

}  // namespace RobotSounds
