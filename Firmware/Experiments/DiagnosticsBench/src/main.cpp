#include "DiagnosticsTask.h"
#include "LogSink.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---- Fake tasks --------------------------------------------------------------

static void fakeModeManagerTask(void*) {
    static const char* TAG = "ModeManagerTask";
    static const char* modes[] = {"ManualMode", "PatrolMode", "CalibrationMode"};
    int modeIdx = 0, eventCount = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(300));
        eventCount++;

        ESP_LOGI(TAG, "event #%d in mode %s", eventCount, modes[modeIdx]);

        if (eventCount % 7 == 0) {
            ESP_LOGW(TAG, "onEvent in mode %s, test warning message", modes[modeIdx]);
        }
        if (eventCount % 10 == 0) {
            modeIdx = (modeIdx + 1) % 3;
            ESP_LOGI(TAG, "activating mode %s", modes[modeIdx]);
        }
    }
}

static void fakeMotionTask(void*) {
    static const char* TAG = "MotionTask";
    int tick = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(500));
        tick++;

        if (tick % 5 == 0) {
            ESP_LOGI(TAG, "tick %d - Forward", tick);
        }

        if (tick % 13 == 0) {
            ESP_LOGE(TAG, "tick %d - test error message", tick);
        }
    }
}

static void fakeTouchTask(void*) {
    static const char* TAG = "TouchTask";
    static const char* btns[] = {"Red", "Yellow", "Blue"};
    int idx = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(2300));
        ESP_LOGI(TAG, "button #%d %s - ShortClick", idx, btns[idx]);
        idx = (idx + 1) % 3;

        vTaskDelay(pdMS_TO_TICKS(800));
        ESP_LOGD(TAG, "debug message for button #%d %s", idx, btns[idx]);
    }
}

static void fakeDistanceTask(void*) {
    static const char* TAG = "DistanceTask";
    int n = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
        n++;

        if (n % 5 == 0) {
            ESP_LOGI(TAG, "distance: %d cm", 20 + (n % 40));
        }
        if (n % 17 == 0) {
            ESP_LOGW(TAG, "warning message #%d", n);
        }
    }
}

static void fakeDumpTriggerTask(void*) {
    static const char* TAG = "DumpTrigger";

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(15000));
        ESP_LOGI(TAG, "requesting ring buffer dump...");
        static char logDumpBuf[400];
        DiagnosticsTask::instance().dumpBufferToString(logDumpBuf, sizeof(logDumpBuf));
        printf("%s", logDumpBuf);
    }
}

// --- main -------------------------------------------------------------------------

extern "C" void app_main() {
    DiagnosticsTask::instance().start();

    // Small delay to ensure DiagnosticsTask is up and running before other tasks start logging
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(fakeModeManagerTask, "FakeModeManager", 4096, nullptr, 5, nullptr);
    xTaskCreate(fakeMotionTask, "FakeMotion", 4096, nullptr, 3, nullptr);
    xTaskCreate(fakeTouchTask, "FakeTouch", 4096, nullptr, 3, nullptr);
    xTaskCreate(fakeDistanceTask, "FakeDistance", 4096, nullptr, 3, nullptr);
    xTaskCreate(fakeDumpTriggerTask, "FakeDump", 4096, nullptr, 2, nullptr);

    ESP_LOGI("main", "bench running");
    vTaskDelete(nullptr);
}
