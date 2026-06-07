#include "HeartbeatTask.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static constexpr const char* HB_TAG = "Task_Heartbeat";
static constexpr uint32_t HB_INTERVAL_MS = 30000;
// Max tasks to report in snapshot
static constexpr UBaseType_t HB_MAX_TASKS = 16;

static void logTaskInfo(const TaskStatus_t& t) {
    const uint32_t freeBytes = t.usStackHighWaterMark * sizeof(StackType_t);
    const char* warning = (freeBytes < 400) ? " ⚠ LOW STACK" : "";

    ESP_LOGI(HB_TAG,
             "  %-16s  state: %c  prio: %2lu  stack free: %5lu B%s",
             t.pcTaskName,
             "XRBSD"[t.eCurrentState],
             (unsigned long)t.uxCurrentPriority,
             (unsigned long)freeBytes,
             warning);
}

static void heartbeatTask(void* /*arg*/) {
    static TaskStatus_t taskList[HB_MAX_TASKS];

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(HB_INTERVAL_MS));

        const int64_t uptimeUs = esp_timer_get_time();
        const uint32_t uptimeSec = static_cast<uint32_t>(uptimeUs / 1'000'000);

        // Heap internal DRAM
        const size_t heapFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        const size_t heapMinFree = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        const size_t heapTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

        // PSRAM
        const size_t psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        const size_t psramTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

        // Task snapshot
        UBaseType_t taskCount = uxTaskGetSystemState(taskList, HB_MAX_TASKS, nullptr);

        ESP_LOGI(HB_TAG, "─── System status ──────────────────────────────");
        ESP_LOGI(HB_TAG,
                 "  Uptime:      %luh %02lum %02lus",
                 (unsigned long)(uptimeSec / 3600),
                 (unsigned long)((uptimeSec % 3600) / 60),
                 (unsigned long)(uptimeSec % 60));
        ESP_LOGI(HB_TAG, "  Heap free:   %zu B  (min ever: %zu B  total: %zu B)", heapFree, heapMinFree, heapTotal);

        if (psramTotal > 0) {
            ESP_LOGI(HB_TAG, "  PSRAM free:  %zu B  (total: %zu B)", psramFree, psramTotal);
        }

        ESP_LOGI(HB_TAG, "  Tasks (%lu):", (unsigned long)taskCount);
        for (UBaseType_t i = 0; i < taskCount; ++i) {
            logTaskInfo(taskList[i]);
        }

        ESP_LOGI(HB_TAG, "────────────────────────────────────────────────");
    }
}

void startHeartbeatTask() {
    xTaskCreate(heartbeatTask, "heartbeat", 3072, nullptr, 1, nullptr);
    ESP_LOGI(HB_TAG, "Heartbeat task started — interval: %lus", (unsigned long)(HB_INTERVAL_MS / 1000));
}
