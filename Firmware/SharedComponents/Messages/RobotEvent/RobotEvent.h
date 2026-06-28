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
        DISTANCE_RANGE_CHANGED,

        // Motion
        MOTION_DONE,
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

    static const char* typeToString(Type type) {
        switch (type) {
            case Type::SYS_BUTTON_SHORT_PRESSED:
                return "SYS_BUTTON_SHORT_PRESSED";
            case Type::SYS_BUTTON_LONG_PRESSED:
                return "SYS_BUTTON_LONG_PRESSED";
            case Type::PUSH_BUTTON_SHORT_PRESSED:
                return "PUSH_BUTTON_SHORT_PRESSED";
            case Type::PUSH_BUTTON_LONG_PRESSED:
                return "PUSH_BUTTON_LONG_PRESSED";
            case Type::INDICATORS_ANIMATION_DONE:
                return "INDICATORS_ANIMATION_DONE";
            case Type::DISTANCE_RANGE_CHANGED:
                return "DISTANCE_RANGE_CHANGED";
            case Type::MOTION_DONE:
                return "MOTION_DONE";
        }
        return "UNKNOWN";
    }

    static const char* rangeToString(DistanceData::Range range) {
        switch (range) {
            case DistanceData::Range::Critical:
                return "CRITICAL";
            case DistanceData::Range::Close:
                return "CLOSE";
            case DistanceData::Range::Near:
                return "NEAR";
            case DistanceData::Range::Far:
                return "FAR";
            case DistanceData::Range::Clear:
                return "CLEAR";
            case DistanceData::Range::Unknown:
                return "UNKNOWN";
        }
        return "UNKNOWN";
    }
};
