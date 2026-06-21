#pragma once

#include <cstdint>
#include <cstring>

struct LogEntry {
    enum class Level : uint8_t { Debug, Info, Warn, Error, None };

    Level level = Level::None;
    uint32_t timestampMs = 0;
    char tag[16] = {};
    char message[80] = {};
};
