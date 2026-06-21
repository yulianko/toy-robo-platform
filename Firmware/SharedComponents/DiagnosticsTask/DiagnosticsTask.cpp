#include "DiagnosticsTask.h"

#include <cstdio>

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

        if (nowMs - _uptimeS * 1'000 >= 1'000) {
            _uptimeS++;
        }

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

// --- Public methods ------------------------------------------------
void DiagnosticsTask::dumpBufferToString(char* outBuffer, size_t maxLen) {
    if (!outBuffer || maxLen == 0) {
        ESP_LOGW(TAG, "dumpBufferToString: invalid output buffer");
        return;
    }

    ESP_LOGI(TAG, "log event buffer dumping (%zu entries)", _logEventBuffer.count());

    size_t written = 0;
    outBuffer[0] = '\0';

    written += snprintf(outBuffer + written, maxLen - written, "Begin [%zu]\n", _logEventBuffer.count());

    _logEventBuffer.forEach([this, &outBuffer, &written, maxLen](const LogEntry& e, size_t idx) -> bool {
        if (maxLen - written < 128) {
            if (maxLen - written > 16) {
                written += snprintf(outBuffer + written, maxLen - written, "Full [%zu]\n", idx);
            }

            ESP_LOGW(TAG, "log event buffer full (%zu entries)", idx);
            return false;
        }

        const char* lvl = "?";
        switch (e.level) {
            case LogEntry::Level::Debug:
                lvl = "D";
                break;
            case LogEntry::Level::Info:
                lvl = "I";
                break;
            case LogEntry::Level::Warn:
                lvl = "W";
                break;
            case LogEntry::Level::Error:
                lvl = "E";
                break;
            default:
                break;
        }

        int res = snprintf(outBuffer + written,
                           maxLen - written,
                           "[%lu ms][%s][%s] %s",
                           static_cast<unsigned long>(e.timestampMs),
                           lvl,
                           e.tag,
                           e.message);

        if (res > 0) {
            written += res;
        }

        return true;
    });

    if (maxLen - written > 5) {
        written += snprintf(outBuffer + written, maxLen - written, "End\n");
    }

    ESP_LOGI(TAG, "log event buffer dump complete (%zu bytes written)", written);
}

static void appendTaskInfo(char* buffer, size_t& written, size_t maxLen, const TaskStatus_t& t, uint32_t totalRunTime) {
    if (written >= maxLen) {
        return;
    }

    const uint32_t freeBytes = static_cast<uint32_t>(t.usStackHighWaterMark) * sizeof(StackType_t);
    unsigned runPct = 0;
    if (totalRunTime != 0) {
        runPct = static_cast<unsigned>((t.ulRunTimeCounter * 100UL) / totalRunTime);
    }

    const char stateChar = (t.eCurrentState <= eDeleted) ? "XRBSD"[t.eCurrentState] : '?';

    int res = snprintf(buffer + written,
                       maxLen - written,
                       "%-16s | %c | %2u | %6lu B | %3u%%\n",
                       t.pcTaskName,
                       stateChar,
                       static_cast<unsigned>(t.uxCurrentPriority),
                       static_cast<unsigned long>(freeBytes),
                       runPct);

    if (res > 0) {
        written += static_cast<size_t>(res);
        if (written >= maxLen) {
            written = maxLen - 1;
        }
    }
}

void DiagnosticsTask::dumpTasks() {
    static constexpr size_t DUMP_BUFFER_SIZE = 2048;
    char dumpBuffer[DUMP_BUFFER_SIZE];
    size_t written = 0;

    UBaseType_t count = uxTaskGetNumberOfTasks();
    TaskStatus_t* tasks = static_cast<TaskStatus_t*>(pvPortMalloc(count * sizeof(TaskStatus_t)));
    if (!tasks) {
        ESP_LOGE(TAG, "dumpTasks: failed to allocate %u task entries", (unsigned)count);
        return;
    }

    uint32_t totalRunTime = 0;
    UBaseType_t retrieved = uxTaskGetSystemState(tasks, count, &totalRunTime);

    int res = 0;
    if (totalRunTime == 0) {
        res = snprintf(
            dumpBuffer + written, DUMP_BUFFER_SIZE - written, "CPU stats unavailable - runtime counters are zero\n");
    }

    if (res > 0) {
        written += static_cast<size_t>(res);
        if (written >= DUMP_BUFFER_SIZE) {
            written = DUMP_BUFFER_SIZE - 1;
        }
    }

    res = snprintf(dumpBuffer + written,
                   DUMP_BUFFER_SIZE - written,
                   "%u tasks\n"
                   "Name             | S | Pr | Stack   | CPU\n"
                   "-----------------+---+----+---------+-----\n",
                   (unsigned)retrieved);
    if (res > 0) {
        written += static_cast<size_t>(res);
        if (written >= DUMP_BUFFER_SIZE) {
            written = DUMP_BUFFER_SIZE - 1;
        }
    }

    for (UBaseType_t i = 0; i < retrieved; ++i) {
        appendTaskInfo(dumpBuffer, written, DUMP_BUFFER_SIZE, tasks[i], totalRunTime);
    }

    if (written < DUMP_BUFFER_SIZE) {
        dumpBuffer[written] = '\0';
    } else {
        dumpBuffer[DUMP_BUFFER_SIZE - 1] = '\0';
    }

    vPortFree(tasks);

    ESP_LOGI(TAG, "%s", dumpBuffer);
}

// --- Private methods ------------------------------------------------

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
