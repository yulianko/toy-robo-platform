#include "IHttpPage.h"
#include "WiFiProxy.h"

class WiFiConfigPage : public IHttpPage {
  public:
    WiFiConfigPage(const char* route) : IHttpPage(route) {
    }

    void init(WiFiProxy* wifiProxy);

    const httpd_uri_t* handlers() const override {
        return _handlers;
    }
    uint8_t handlerCount() const override {
        return HANDLER_COUNT;
    }

  private:
    static constexpr uint8_t HANDLER_COUNT = 5;
    httpd_uri_t _handlers[HANDLER_COUNT] = {};

    WiFiProxy* _wifiProxy;

    static esp_err_t handleCss(httpd_req_t* req);
    static esp_err_t handleFavicon(httpd_req_t* req);
    static esp_err_t handlePageConnections(httpd_req_t* req);
    static esp_err_t handleApiScanGet(httpd_req_t* req);
    static esp_err_t handleApiScanPost(httpd_req_t* req);
    static esp_err_t handleApiConnect(httpd_req_t* req);
};
