#pragma once

#include <cstddef>

#include "LogEntry.h"
#include "LogEventRingBuffer.h"
#include "LogSink.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

class DiagnosticsTask {
  public:
    static constexpr size_t TASK_STACK = 4096;
    static constexpr UBaseType_t TASK_PRIO = 2;

    static constexpr size_t QUEUE_LENGTH = 32;
    static constexpr size_t BUFFER_SIZE = 128;

    static constexpr uint32_t HEARTBEAT_MS = 1'000;

    static constexpr uint32_t TASK_LIST_MS = 30'000;

    static DiagnosticsTask& instance();

    DiagnosticsTask(const DiagnosticsTask&) = delete;
    DiagnosticsTask& operator=(const DiagnosticsTask&) = delete;

    void start(bool installSink = true);

    void dumpBufferToString(char* outBuffer, size_t maxLen);

  private:
    DiagnosticsTask();

    static void taskFn(void* arg);
    void run();

    void drainQueue();
    void heartbeat();
    void dumpTasks();

    QueueHandle_t _queue = nullptr;
    LogEventRingBuffer<BUFFER_SIZE> _logEventBuffer;
    uint32_t _lastHeartbeatMs = 0;
    uint32_t _uptimeS = 0;
    uint32_t _lastTaskListMs = 0;
};
