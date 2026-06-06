#pragma once
#include <cstdint>

enum class MotionVerb : uint8_t {
    Forward,
    Backward,
    TurnLeft,
    TurnRight,
    PivotLeft,
    PivotRight,
    Brake,
    Coast,
};
