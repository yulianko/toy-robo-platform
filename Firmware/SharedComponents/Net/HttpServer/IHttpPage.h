#pragma once
#include <esp_http_server.h>

class IHttpPage {
  public:
    IHttpPage(const char* route) : ROUTE(route) {
    }

    virtual ~IHttpPage() = default;

    virtual const httpd_uri_t* handlers() const = 0;
    virtual uint8_t handlerCount() const = 0;

    const char* route() const {
        return ROUTE;
    }

  private:
    const char* ROUTE;
};
