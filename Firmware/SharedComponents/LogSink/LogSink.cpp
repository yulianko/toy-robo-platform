#include "LogSink.h"

#include <atomic>
#include <cstdio>
#include <cstring>

#include "esp_log.h"
#include "esp_timer.h"

// ---- Internal state -------------------------------------------------------------
static QueueHandle_t s_queue = nullptr;
static const char* const* s_filters = nullptr;
static std::atomic<bool> s_active = false;

static bool isFiltered(const char* tag) {
    if (!s_filters) {
        return false;
    }

    for (size_t i = 0; s_filters[i] != nullptr; i++) {
        if (strncmp(tag, s_filters[i], strlen(s_filters[i])) == 0) {
            return true;
        }
    }

    return false;
}

static int sinkFn(const char* format, va_list args) {
    va_list uartArgs;
    va_copy(uartArgs, args);
    int ret = vprintf(format, uartArgs);
    va_end(uartArgs);

    if (!s_active.load(std::memory_order_relaxed)) {
        return ret;
    }

    static char staticMsgBuffer[256];

    va_list msgArgs;
    va_copy(msgArgs, args);
    vsnprintf(staticMsgBuffer, sizeof(staticMsgBuffer), format, msgArgs);
    va_end(msgArgs);

    char tag[16] = {};
    LogSink::parseTag(staticMsgBuffer, tag, sizeof(tag));

    if (isFiltered(tag)) {
        return ret;
    }

    LogEntry entry{};
    entry.level = LogSink::parseLevel(staticMsgBuffer);
    entry.timestampMs = static_cast<uint32_t>(esp_timer_get_time() / 1'000);
    strncpy(entry.tag, tag, sizeof(entry.tag) - 1);
    strncpy(entry.message, staticMsgBuffer, sizeof(entry.message) - 1);

    xQueueSend(s_queue, &entry, 0);

    return ret;
}

// ---- Public API -------------------------------------------------------------
namespace LogSink {

void install(QueueHandle_t queue, const char* const* filters) {
    s_queue = queue;
    s_filters = filters;
    s_active.store(true, std::memory_order_relaxed);
    esp_log_set_vprintf(sinkFn);
}

void uninstall() {
    s_active.store(false, std::memory_order_relaxed);
    esp_log_set_vprintf(vprintf);  // restore default
    s_queue = nullptr;
    s_filters = nullptr;
}

bool isInstalled() {
    return s_active.load(std::memory_order_relaxed);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parsing helpers
// ESP-IDF log format (color codes optional):
//   "\033[0;32mI (1234) TAG: message\033[0m\n"
//   After stripping escape: "I (1234) TAG: message\n"
// ─────────────────────────────────────────────────────────────────────────────

LogEntry::Level parseLevel(const char* format) {
    const char* p = format;

    // Skip ANSI escape prefix "\033[x;xxm" if present
    if (*p == '\033') {
        while (*p && *p != 'm') p++;
        if (*p) p++;
    }

    switch (*p) {
        case 'D':
            return LogEntry::Level::Debug;
        case 'I':
            return LogEntry::Level::Info;
        case 'W':
            return LogEntry::Level::Warn;
        case 'E':
            return LogEntry::Level::Error;
        default:
            return LogEntry::Level::None;
    }
}

void parseTag(const char* format, char* tagOut, size_t tagLen) {
    if (!tagOut || tagLen == 0) return;
    tagOut[0] = '\0';

    // Find closing ')' of timestamp "(1234)"
    const char* p = strchr(format, ')');
    if (!p) return;

    p++;                    // skip ')'
    while (*p == ' ') p++;  // skip space(s)

    // Copy until ':' (end of tag), ANSI escape, or end of string
    size_t i = 0;
    while (*p && *p != ':' && *p != '\033' && i < tagLen - 1) {
        tagOut[i++] = *p++;
    }
    tagOut[i] = '\0';

    // Trim trailing spaces (rare but possible)
    while (i > 0 && tagOut[i - 1] == ' ') {
        tagOut[--i] = '\0';
    }
}

}  // namespace LogSink
