#pragma once
#include <esp_http_server.h>

class IHttpPage {
  public:
    virtual ~IHttpPage() = default;
    virtual void registerHandlers(httpd_handle_t server, void* ctx) = 0;
    virtual uint8_t handlerCount() const = 0;
};
