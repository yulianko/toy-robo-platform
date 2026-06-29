#pragma once
#include <esp_http_server.h>

#include "IHttpPage.h"

class HttpServer {
  public:
    struct Config {
        uint16_t port = 80;
        uint8_t maxSockets = 4;
        size_t stackSize = 6144;
    };

    esp_err_t start(const Config& config, IHttpPage** pages, uint8_t pageCount);
    esp_err_t stop();

  private:
    static constexpr uint8_t MAX_HANDLERS = 16;
    static constexpr uint8_t MAX_URI_LEN = 64;

    httpd_handle_t _server = nullptr;

    char _uriBufs[MAX_HANDLERS][MAX_URI_LEN];
    uint8_t _uriBufCount = 0;
};
