#pragma once
#include <cstdint>

#include "MotionStep.h"

class MotionSequence {
  public:
    static constexpr uint8_t LOOP_INFINITE = 0;
    static constexpr uint8_t LOOP_ONCE = 1;

    MotionSequence() = default;

    MotionSequence(const char* name, const MotionStep* steps, uint8_t count, uint8_t loops = LOOP_ONCE)
        : _name(name), _loops(loops) {
        _count = count < MAX_STEPS ? count : MAX_STEPS;
        for (uint8_t i = 0; i < _count; ++i) {
            _steps[i] = steps[i];
        }
    }

    const char* name() const {
        return _name;
    }

    const MotionStep* steps() const {
        return _steps;
    }

    uint8_t count() const {
        return _count;
    }

    uint8_t loops() const {
        return _loops;
    }

    bool valid() const {
        return _count > 0;
    }

    static constexpr uint8_t MAX_STEPS = 16;

  private:
    const char* _name = nullptr;
    MotionStep _steps[MAX_STEPS]{};
    uint8_t _count = 0;
    uint8_t _loops = LOOP_ONCE;
};
