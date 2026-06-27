#pragma once
#include <cstdint>

struct DistanceScanProfile {
    uint32_t intervalMs = 100;  // measurement period
    uint8_t confirmCount = 3;   // consecutive readings before accepting range change
    bool postRaw = false;       // true = post every reading (radar mode)
    bool frontOfQueue = false;  // true = critical events jump the queue

    static constexpr DistanceScanProfile idle() {
        return {.intervalMs = 0, .confirmCount = 0, .postRaw = false};
    }
    static constexpr DistanceScanProfile normal() {
        return {.intervalMs = 100, .confirmCount = 3, .postRaw = false};
    }
    static constexpr DistanceScanProfile obstacle() {
        return {.intervalMs = 50, .confirmCount = 2, .postRaw = false, .frontOfQueue = true};
    }
};
