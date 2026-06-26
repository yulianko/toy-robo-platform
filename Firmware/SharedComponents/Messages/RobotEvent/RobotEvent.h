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

        // Distance
        DISTANCE_RANGE_CHANGED
    };

    struct DistanceData {
        enum class Range : int8_t { Unknown = -1, Clear, Far, Near, Close, Critical };
        uint16_t cm;
        Range range;
        uint8_t padding32;
    };

    Type type;

    union {
        uint32_t value{0};
        DistanceData distance;
    };

    RobotEvent() = default;

    RobotEvent(Type eventType) : type(eventType) {
    }

    RobotEvent(Type eventType, uint32_t eventValue) : type(eventType), value(eventValue) {
    }

    RobotEvent(Type eventType, DistanceData eventDistance) : type(eventType), distance(eventDistance) {
    }
};
