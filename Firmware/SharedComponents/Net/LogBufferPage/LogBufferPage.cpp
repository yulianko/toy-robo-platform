#include "LogBufferPage.h"

#include <esp_http_server.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

#include <climits>
#include <cstdio>
#include <cstdlib>

#include "DiagnosticsTask.h"
#include "HttpUtils.h"

static constexpr const char* TAG = "LogBufferPage";
static constexpr size_t LOG_LINE_BUFFER_SIZE = 512;

static bool sendChunk(const char* data, size_t len, void* ctx) {
    if (ctx == nullptr) {
        return false;
    }

    httpd_req_t* req = static_cast<httpd_req_t*>(ctx);
    esp_err_t err = httpd_resp_send_chunk(req, data, len);
    return err == ESP_OK;
}

void LogBufferPage::init() {
    _handlers[0].uri = "";
    _handlers[0].method = HTTP_GET;
    _handlers[0].handler = handleDump;
    _handlers[0].user_ctx = this;
}

esp_err_t LogBufferPage::handleDump(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

    int refreshSeconds = 0;
    if (HttpUtils::getQueryInt(req, "refresh", refreshSeconds, 1, INT_MAX)) {
        char refreshHeader[16] = {};
        std::snprintf(refreshHeader, sizeof(refreshHeader), "%d", refreshSeconds);
        httpd_resp_set_hdr(req, "Refresh", refreshHeader);
    }

    bool ok = DiagnosticsTask::instance().dumpBufferToSink(sendChunk, req);
    if (!ok) {
        ESP_LOGE(TAG, "Failed to send log dump chunk");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Log dump failed");
        return ESP_FAIL;
    }

    if (httpd_resp_send_chunk(req, nullptr, 0) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to finalize log dump");
        return ESP_FAIL;
    }
    return ESP_OK;
}
