#pragma once

#include <cstdint>
#include <cstring>

#include "RgbEffect.h"

struct RgbAnimation {
    static constexpr size_t MAX_STEPS = 8;

    struct Step {
        RgbEffect effect;
        uint32_t durationMs;
    };

    RgbAnimation() : name(""), stepsCount(0), loops(0) {
    }

    RgbAnimation(const char* name, const Step* s, size_t count, uint32_t loops = 1)
        : name(name), steps{}, stepsCount(count > MAX_STEPS ? MAX_STEPS : count), loops(loops) {
        for (size_t i = 0; i < stepsCount; ++i) {
            steps[i] = s[i];
        }
    }

    const char* name;
    Step steps[MAX_STEPS];
    size_t stepsCount;
    uint32_t loops;

    bool isEmpty() const {
        return stepsCount == 0;
    }
};
