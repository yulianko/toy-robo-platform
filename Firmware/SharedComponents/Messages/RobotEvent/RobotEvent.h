#pragma once

#include <cstdint>

struct RobotEvent {
    enum class Type : uint8_t {
        // System button events
        SYS_BUTTON_SHORT_PRESSED,
        SYS_BUTTON_LONG_PRESSED,

        // Push button events
        PUSH_BUTTON_SHORT_PRESSED,
        PUSH_BUTTON_LONG_PRESSED,

        // Indicator events
        INDICATORS_ANIMATION_DONE,
    };

    Type type;

    RobotEvent() = default;

    RobotEvent(Type eventType);
};
