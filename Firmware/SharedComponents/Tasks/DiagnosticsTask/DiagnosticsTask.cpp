#include "DiagnosticsTask.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "LogEntryUtils.h"
#include "StringUtils.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static constexpr const char* TAG = "Diag";

static constexpr const char* kDefaultFilters[] = {TAG, nullptr};

// ---- Initialization ------------------------------------------------
DiagnosticsTask& DiagnosticsTask::instance() {
    static DiagnosticsTask inst;
    return inst;
}

DiagnosticsTask::DiagnosticsTask() {
    _queue = xQueueCreate(QUEUE_LENGTH, sizeof(LogEntry));
}

void DiagnosticsTask::start(bool installSink) {
    if (installSink) {
        LogSink::install(_queue, kDefaultFilters);
    }

    xTaskCreate(taskFn, "DiagTask", TASK_STACK, this, TASK_PRIO, nullptr);
}

void DiagnosticsTask::taskFn(void* arg) {
    static_cast<DiagnosticsTask*>(arg)->run();
}

void DiagnosticsTask::run() {
    ESP_LOGI(TAG, "started - log sink active: %s", LogSink::isInstalled() ? "yes" : "no");

    ESP_ERROR_CHECK(esp_task_wdt_add(nullptr));

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(200));

        esp_task_wdt_reset();
        drainQueue();

        const uint32_t nowMs = static_cast<uint32_t>(esp_timer_get_time() / 1'000);

        _uptimeS = nowMs / 1'000;

        if (nowMs - _lastHeartbeatMs >= HEARTBEAT_MS) {
            _lastHeartbeatMs = nowMs;
            heartbeat();
        }

        if (nowMs - _lastTaskListMs >= TASK_LIST_MS) {
            _lastTaskListMs = nowMs;
            dumpTasks();
        }
    }
}

void DiagnosticsTask::dumpBufferToString(char* outBuffer, size_t maxLen) {
    if (!outBuffer || maxLen == 0) {
        ESP_LOGW(TAG, "dumpBufferToString: invalid output buffer");
        return;
    }

    ESP_LOGI(TAG, "log event buffer dumping (%zu entries)", _logEventBuffer.count());

    outBuffer[0] = '\0';

    struct Ctx {
        char* buf;
        size_t written;
        size_t maxLen;
    } ctx{outBuffer, 0, maxLen};

    dumpBufferToSink(
        [](const char* data, size_t len, void* rawCtx) -> bool {
            auto* c = static_cast<Ctx*>(rawCtx);
            if (len >= c->maxLen - c->written) {
                ESP_LOGW(TAG, "log event buffer full at %zu bytes", c->written);
                return false;
            }
            std::memcpy(c->buf + c->written, data, len);
            c->written += len;
            c->buf[c->written] = '\0';
            return true;
        },
        &ctx);

    ESP_LOGI(TAG, "log event buffer dump complete (%zu bytes written)", ctx.written);
}

bool DiagnosticsTask::dumpBufferToSink(DumpSinkFn sink, void* ctx) {
    if (sink == nullptr) {
        return false;
    }

    char lineBuf[512];

    auto emit = [&](const char* fmt, auto&&... args) -> bool {
        int res = std::snprintf(lineBuf, sizeof(lineBuf), fmt, args...);
        if (res < 0) {
            return false;
        }
        size_t len = static_cast<size_t>(res);
        if (len >= sizeof(lineBuf)) {
            len = sizeof(lineBuf) - 1;  // snprintf truncated; report what actually fit
        }
        return sink(lineBuf, len, ctx);
    };

    if (!emit("Begin [%zu]\n", _logEventBuffer.count())) {
        return false;
    }

    bool ok = true;
    _logEventBuffer.forEach([&](const LogEntry& e, size_t /*idx*/) -> bool {
        char msg[128];
        strutil::copyWithTrailingNewline(e.message, msg, sizeof(msg));

        // msg already ends with '\n', so no trailing \n in the format string.
        ok = emit("[%lu ms][%s][%s] %s",
                  static_cast<unsigned long>(e.timestampMs),
                  LogEntryUtils::levelChar(e.level),
                  e.tag,
                  msg);
        return ok;
    });

    if (!ok) {
        return false;
    }

    return emit("End\n");
}

void DiagnosticsTask::dumpTasks() {
    char statsBuffer[1024];

    ESP_LOGI(TAG, "Task list:");
    std::printf("Task Name\tStatus\tPrio\tStack\tTaskNum\tCore\n");
    vTaskList(statsBuffer);
    std::printf("%s", statsBuffer);

    ESP_LOGI(TAG, "CPU usage:");
    std::printf("\n--- CPU Usage (Run Time Stats) ---\n");
    std::printf("Task Name\tAbs Time\t%% Time\n");
    vTaskGetRunTimeStats(statsBuffer);
    std::printf("%s", statsBuffer);
}

void DiagnosticsTask::drainQueue() {
    LogEntry entry;
    while (xQueueReceive(_queue, &entry, 0) == pdTRUE) {
        _logEventBuffer.push(entry);
    }
}

void DiagnosticsTask::heartbeat() {
    ESP_LOGI(TAG,
             "heartbeat | uptime %lus | heap %lu B | ring %zu/%d",
             static_cast<unsigned long>(_uptimeS),
             static_cast<unsigned long>(esp_get_free_heap_size()),
             _logEventBuffer.count(),
             static_cast<int>(BUFFER_SIZE));
}
