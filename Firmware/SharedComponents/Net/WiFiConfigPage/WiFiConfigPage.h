#include "IHttpPage.h"

class WiFiConfigPage : public IHttpPage {
  public:
    void registerHandlers(httpd_handle_t server, void* ctx) override;
    uint8_t handlerCount() const override {
        return 6;
    }

  private:
    httpd_uri_t _uriCss{};
    httpd_uri_t _uriFavicon{};
    httpd_uri_t _uriPageRoot{};
    httpd_uri_t _uriApiScanGet{};
    httpd_uri_t _uriApiScanPost{};
    httpd_uri_t _uriApiConnect{};

    static esp_err_t handleCss(httpd_req_t* req);
    static esp_err_t handleFavicon(httpd_req_t* req);
    static esp_err_t handlePageConnections(httpd_req_t* req);
    static esp_err_t handleApiScanGet(httpd_req_t* req);
    static esp_err_t handleApiScanPost(httpd_req_t* req);
    static esp_err_t handleApiConnect(httpd_req_t* req);
};
