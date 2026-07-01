#pragma once

#include "LogEntry.h"

namespace LogEntryUtils {

inline const char* levelChar(LogEntry::Level level) {
    switch (level) {
        case LogEntry::Level::Debug:
            return "D";
        case LogEntry::Level::Info:
            return "I";
        case LogEntry::Level::Warn:
            return "W";
        case LogEntry::Level::Error:
            return "E";
        default:
            return "?";
    }
}

}  // namespace LogEntryUtils
