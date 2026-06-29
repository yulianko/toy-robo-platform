#include "HttpServer.h"

#include <esp_check.h>
#include <esp_log.h>
#include <http_parser.h>

#include "HttpUtils.h"

static constexpr const char* TAG = "HttpServer";

esp_err_t HttpServer::start(const Config& config, IHttpPage** pages, uint8_t pageCount) {
    if (pages == nullptr || pageCount == 0) {
        ESP_LOGW(TAG, "No pages provided to HTTP server");
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t totalHandlers = 0;
    for (uint8_t i = 0; i < pageCount; ++i) {
        if (pages[i] != nullptr) {
            totalHandlers += pages[i]->handlerCount();
        } else {
            ESP_LOGW(TAG, "Page pointer is undefined");
        }
    }

    httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();
    httpdConfig.server_port = config.port;
    httpdConfig.max_open_sockets = config.maxSockets;
    httpdConfig.stack_size = config.stackSize;
    httpdConfig.max_uri_handlers = totalHandlers;

    ESP_RETURN_ON_ERROR(httpd_start(&_server, &httpdConfig), TAG, "Failed to start HTTP server");

    for (uint8_t i = 0; i < pageCount; ++i) {
        if (pages[i] == nullptr) {
            continue;
        }

        const char* pageRoute = pages[i]->route();
        uint8_t count = pages[i]->handlerCount();
        for (uint8_t j = 0; j < count; ++j) {
            httpd_uri_t h = pages[i]->handlers()[j];

            char* buf = _uriBufs[_uriBufCount++];

            if (!HttpUtils::joinUri(buf, MAX_URI_LEN, pageRoute, h.uri)) {
                ESP_LOGE(TAG, "URI too long or invalid, skipping: %s + %s", pageRoute, h.uri);
                continue;
            }

            h.uri = buf;

            esp_err_t err = httpd_register_uri_handler(_server, &h);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Registered endpoint: %s [%s]", h.uri, http_method_str((http_method)h.method));
            } else {
                ESP_LOGE(TAG,
                         "Failed to register endpoint: %s [%s]. %s",
                         h.uri,
                         http_method_str((http_method)h.method),
                         esp_err_to_name(err));
            }
        }
    }

    ESP_LOGI(TAG, "Started on port %d (%d handlers)", config.port, totalHandlers);
    return ESP_OK;
}

esp_err_t HttpServer::stop() {
    if (_server) {
        ESP_RETURN_ON_ERROR(httpd_stop(_server), TAG, "Failed to stop");
    }

    _server = nullptr;
    return ESP_OK;
}
