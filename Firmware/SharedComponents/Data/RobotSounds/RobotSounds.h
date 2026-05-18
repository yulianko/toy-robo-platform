#pragma once

#include "SoundAnimation.h"

namespace RobotSounds {

static const SoundAnimation curiosity() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{800, 2200}, 200},
        {EffectFreqSlide{2200, 2000}, 80},
        {EffectFreqSlide{2000, 2300}, 60},
        {EffectFreqSlide{2300, 2000}, 60},
        {EffectFreqSlide{2000, 2300}, 60},
        {EffectFreqSlide{2300, 1800}, 120},
        {EffectSilence{}, 80},
    };
    static const SoundAnimation sound = SoundAnimation("curiosity", steps, 7, 1);
    return sound;
}

static const SoundAnimation surprise() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1000, 3800}, 80},
        {EffectTone{3800}, 30},
        {EffectFreqSlide{3800, 3200}, 40},
        {EffectFreqSlide{3200, 3800}, 40},
        {EffectFreqSlide{3800, 3200}, 40},
        {EffectFreqSlide{3200, 3800}, 40},
        {EffectFreqSlide{3800, 1200}, 250},
        {EffectSilence{}, 60},
    };
    static const SoundAnimation sound = SoundAnimation("surprise", steps, 8, 1);
    return sound;
}

static const SoundAnimation agreement() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1500, 2800}, 120},
        {EffectSilence{}, 60},
        {EffectFreqSlide{1800, 3200}, 140},
        {EffectFreqSlide{3200, 3000}, 60},
        {EffectSilence{}, 50},
    };
    static const SoundAnimation sound = SoundAnimation("agreement", steps, 5, 1);
    return sound;
}

static const SoundAnimation disagreement() {
    static const SoundAnimation::Step steps[] = {
        {EffectFreqSlide{1200, 600}, 200},
        {EffectSilence{}, 60},
        {EffectFreqSlide{900, 400}, 180},
        {EffectFreqSlide{400, 600}, 80},
        {EffectFreqSlide{600, 400}, 80},
        {EffectSilence{}, 80},
    };
    static const SoundAnimation sound = SoundAnimation("disagreement", steps, 6, 1);
    return sound;
}

}  // namespace RobotSounds
