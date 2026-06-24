#pragma once

#include <cstdint>

#include "RgbAnimation.h"

struct IndicatorCommand {
    enum class Type : uint8_t { Start, Stop };

    Type type = Type::Stop;
    RgbAnimation animation;

    static IndicatorCommand start(const RgbAnimation& anim) {
        IndicatorCommand cmd;
        cmd.type = Type::Start;
        cmd.animation = anim;
        return cmd;
    }

    static IndicatorCommand stop() {
        IndicatorCommand cmd;
        cmd.type = Type::Stop;
        return cmd;
    }
};
