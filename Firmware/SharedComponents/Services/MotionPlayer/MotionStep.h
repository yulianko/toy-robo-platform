#pragma once
#include <cstdint>

#include "MotionVerb.h"

struct MotionStep {
    MotionVerb verb;
    float speed;
    uint32_t durationMs;  // 0 = hold indefinitely
};
