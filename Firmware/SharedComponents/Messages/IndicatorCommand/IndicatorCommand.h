#pragma once

#include <cstdint>

#include "RgbAnimation.h"
#include "SoundAnimation.h"

struct IndicatorCommand {
    enum class Type : uint8_t { StartRgb, StartSound, StartBoth, Stop };

    Type type = Type::Stop;
    RgbAnimation rgbAnimation;
    SoundAnimation soundAnimation;

    static IndicatorCommand startRgb(const RgbAnimation& anim) {
        IndicatorCommand cmd;
        cmd.type = Type::StartRgb;
        cmd.rgbAnimation = anim;
        return cmd;
    }

    static IndicatorCommand startSound(const SoundAnimation& anim) {
        IndicatorCommand cmd;
        cmd.type = Type::StartSound;
        cmd.soundAnimation = anim;
        return cmd;
    }

    static IndicatorCommand startBoth(const RgbAnimation& rgbAnim, const SoundAnimation& soundAnim) {
        IndicatorCommand cmd;
        cmd.type = Type::StartBoth;
        cmd.rgbAnimation = rgbAnim;
        cmd.soundAnimation = soundAnim;
        return cmd;
    }

    static IndicatorCommand stop() {
        IndicatorCommand cmd;
        cmd.type = Type::Stop;
        return cmd;
    }

    // Legacy compatibility - maps to StartRgb
    static IndicatorCommand start(const RgbAnimation& anim) {
        return startRgb(anim);
    }
};
