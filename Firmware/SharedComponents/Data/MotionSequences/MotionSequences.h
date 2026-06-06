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

inline MotionSequence patrol(float speed) {
    const MotionStep steps[] = {
        {MotionVerb::Forward, speed, 2000},
        {MotionVerb::PivotRight, speed, 500},
    };
    return MotionSequence("patrol", steps, 2, MotionSequence::LOOP_INFINITE);
}

}  // namespace MotionSequences
