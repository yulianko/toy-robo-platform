#pragma once

#include <cstdint>

struct RgbColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    RgbColor scaled(float brightness) const {
        return {
            static_cast<uint8_t>(r * brightness),
            static_cast<uint8_t>(g * brightness),
            static_cast<uint8_t>(b * brightness),
        };
    }

    static constexpr RgbColor red() {
        return {255, 0, 0};
    }
    static constexpr RgbColor green() {
        return {0, 255, 0};
    }
    static constexpr RgbColor blue() {
        return {0, 0, 255};
    }
    static constexpr RgbColor white() {
        return {255, 255, 255};
    }
    static constexpr RgbColor yellow() {
        return {255, 200, 0};
    }
    static constexpr RgbColor cyan() {
        return {0, 255, 200};
    }
    static constexpr RgbColor magenta() {
        return {255, 0, 200};
    }
    static constexpr RgbColor orange() {
        return {255, 80, 0};
    }
};
