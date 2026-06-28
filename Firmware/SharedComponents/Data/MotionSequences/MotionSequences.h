#pragma once
#include "MotionSequence.h"
#include "MotionVerb.h"

namespace MotionSequences {

inline MotionSequence forward(float speed) {
    const MotionStep steps[] = {{MotionVerb::Forward, speed, 0}};
    return MotionSequence("forward", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence backward(float speed) {
    const MotionStep steps[] = {{MotionVerb::Backward, speed, 0}};
    return MotionSequence("backward", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence pivotLeft(float speed) {
    const MotionStep steps[] = {{MotionVerb::PivotLeft, speed, 0}};
    return MotionSequence("pivot_left", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence pivotRight(float speed) {
    const MotionStep steps[] = {{MotionVerb::PivotRight, speed, 0}};
    return MotionSequence("pivot_right", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence turnLeft(float speed) {
    const MotionStep steps[] = {{MotionVerb::TurnLeft, speed, 0}};
    return MotionSequence("turn_left", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence turnRight(float speed) {
    const MotionStep steps[] = {{MotionVerb::TurnRight, speed, 0}};
    return MotionSequence("turn_right", steps, 1, MotionSequence::LOOP_ONCE);
}

inline MotionSequence forwardFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::Forward, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("forward_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence backwardFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::Backward, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("backward_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence turnLeftFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::TurnLeft, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("turn_left_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence turnRightFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::TurnRight, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("turn_right_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence pivotLeftFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::PivotLeft, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("pivot_left_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence pivotRightFor(float speed, uint32_t ms) {
    const MotionStep steps[] = {
        {MotionVerb::PivotRight, speed, ms},
        {MotionVerb::Coast, 0.0f, 0},
    };
    return MotionSequence("pivot_right_timed", steps, 2, MotionSequence::LOOP_ONCE);
}

inline MotionSequence patrol(float speed) {
    const MotionStep steps[] = {
        {MotionVerb::Forward, speed, 2000},
        {MotionVerb::PivotRight, speed, 500},
    };
    return MotionSequence("patrol", steps, 2, MotionSequence::LOOP_INFINITE);
}

inline MotionSequence evadeLeft(float speed, uint32_t backMs = 600, uint32_t pivotMs = 700) {
    const MotionStep steps[] = {
        {MotionVerb::Backward, speed, backMs},
        {MotionVerb::PivotLeft, speed, pivotMs},
        {MotionVerb::Brake, 0.0f, 0},
    };
    return MotionSequence("evade_left", steps, 3);
}

inline MotionSequence evadeRight(float speed, uint32_t backMs = 600, uint32_t pivotMs = 700) {
    const MotionStep steps[] = {
        {MotionVerb::Backward, speed, backMs},
        {MotionVerb::PivotRight, speed, pivotMs},
        {MotionVerb::Brake, 0.0f, 0},
    };
    return MotionSequence("evade_right", steps, 3);
}
}  // namespace MotionSequences
