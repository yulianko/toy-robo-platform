#include "HttpServer.h"

#include <esp_check.h>
#include <esp_log.h>

static constexpr const char* TAG = "HttpServer";

esp_err_t HttpServer::start(const Config& config, IHttpPage** pages, uint8_t pageCount, void* ctx) {
    uint16_t totalHandlers = 0;
    for (uint8_t i = 0; i < pageCount; ++i) {
        totalHandlers += pages[i]->handlerCount();
    }

    httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();
    httpdConfig.server_port = config.port;
    httpdConfig.max_open_sockets = config.maxSockets;
    httpdConfig.stack_size = config.stackSize;
    httpdConfig.max_uri_handlers = totalHandlers;

    ESP_RETURN_ON_ERROR(httpd_start(&_server, &httpdConfig), TAG, "Failed to start");

    for (uint8_t i = 0; i < pageCount; ++i) {
        pages[i]->registerHandlers(_server, ctx);
    }

    ESP_LOGI(TAG, "Started on port %d with %d handlers", config.port, totalHandlers);
    return ESP_OK;
}

esp_err_t HttpServer::stop() {
    if (_server) {
        ESP_RETURN_ON_ERROR(httpd_stop(_server), TAG, "Failed to stop");
    }

    _server = nullptr;
    return ESP_OK;
}
