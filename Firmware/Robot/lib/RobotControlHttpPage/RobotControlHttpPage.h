#pragma once

#include "IHttpPage.h"
#include "RobotContext.h"

class RobotControlHttpPage : public IHttpPage {
  public:
    RobotControlHttpPage(const char* route) : IHttpPage(route) {
    }

    void init(RobotContext* robotContext);

    const httpd_uri_t* handlers() const override {
        return _handlers;
    }
    uint8_t handlerCount() const override {
        return HANDLER_COUNT;
    }

  private:
    static constexpr uint8_t HANDLER_COUNT = 5;
    httpd_uri_t _handlers[HANDLER_COUNT] = {};

    RobotContext* _robotContext;

    static esp_err_t handlePage(httpd_req_t* req);
    static esp_err_t handleCss(httpd_req_t* req);
    static esp_err_t handleApiMove(httpd_req_t* req);
    static esp_err_t handleApiStop(httpd_req_t* req);
    static esp_err_t handleApiIndicator(httpd_req_t* req);
};
