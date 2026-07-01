#pragma once

#include "IHttpPage.h"

class LogBufferPage : public IHttpPage {
  public:
    LogBufferPage(const char* route) : IHttpPage(route) {
    }

    void init();

    const httpd_uri_t* handlers() const override {
        return _handlers;
    }
    uint8_t handlerCount() const override {
        return HANDLER_COUNT;
    }

  private:
    static constexpr uint8_t HANDLER_COUNT = 1;
    httpd_uri_t _handlers[HANDLER_COUNT] = {};

    static esp_err_t handleDump(httpd_req_t* req);
};
