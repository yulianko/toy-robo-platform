#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "HeartbeatTask.h"
#include "HttpServer.h"
#include "IsrButton.h"
#include "WiFiConfigPage.h"
#include "WiFiProxy.h"
#include "WifiManager.h"

struct ResetButtonTaskContext {
    QueueHandle_t buttonQueue;
    WiFiProxy* wifiProxy;
};

static void resetButtonTask(void* arg) {
    ResetButtonTaskContext* ctx = static_cast<ResetButtonTaskContext*>(arg);
    IsrButton::ButtonEvent event;
    while (true) {
        if (xQueueReceive(ctx->buttonQueue, &event, portMAX_DELAY) == pdTRUE) {
            if (event.action == IsrButton::Action::LONG_PRESSED) {
                ESP_LOGI("ResetButton", "Long press detected. Resetting WiFi credentials.");
                ctx->wifiProxy->postReset();
            } else {
                ESP_LOGI("ResetButton", "Short press ignored.");
            }
        }
    }
}

extern "C" void app_main(void) {
    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // default event loop and TCP/IP stack
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    static QueueHandle_t wifiCommandQueue = xQueueCreate(4, sizeof(WmMsg));
    static QueueHandle_t buttonQueue = xQueueCreate(8, sizeof(IsrButton::ButtonEvent));

    static IsrButton resetButton({GPIO_NUM_48, "reset", 150000, 3000000});
    static WiFiProxy wifiProxy(&WiFiManager::instance(), wifiCommandQueue);
    static HttpServer httpServer;
    static ResetButtonTaskContext resetTaskCtx{buttonQueue, &wifiProxy};

    // Init components (DI)
    resetButton.init(buttonQueue);

    WiFiManager::Config wmConfig = {
        .scanRssiThreshold = -85,   // dBm
        .scanMaxDisplay = 10,       // how many to present to user
        .connectTimeoutMs = 15000,  // ms
        .driver =
            {
                .apSsid = "MyNet",
                .apPassword = "_1234567",
                .apChannel = 1,
                .apMaxConn = 4,
            },
    };

    WiFiManager::instance().init(wmConfig, wifiCommandQueue);
    WiFiManager::instance().start(5);

    xTaskCreate(resetButtonTask, "reset_btn", 3072, &resetTaskCtx, 5, nullptr);

    static WiFiConfigPage wifiPage;
    IHttpPage* pages[] = {&wifiPage};

    HttpServer::Config httpServerConfig = {.port = 80, .maxSockets = 4, .stackSize = 6144};
    ESP_ERROR_CHECK(httpServer.start(httpServerConfig, pages, 1, &wifiProxy));

    // Heartbeat task for diagnostics
    startHeartbeatTask();

    // Main should not spin — match robot style
    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelete(nullptr);
}
