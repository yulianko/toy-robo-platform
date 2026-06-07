#include "WiFiConfigPage.h"

#include "HttpUtils.h"
#include "WiFiConfigPageContent.h"
#include "WiFiProxy.h"

static const char* TAG = "WiFiConfigPage";

void WiFiConfigPage::registerHandlers(httpd_handle_t server, void* ctx) {
    _uriCss.uri = "/style.css";
    _uriCss.method = HTTP_GET;
    _uriCss.handler = handleCss;
    _uriCss.user_ctx = ctx;

    _uriFavicon.uri = "/favicon.ico";
    _uriFavicon.method = HTTP_GET;
    _uriFavicon.handler = handleFavicon;
    _uriFavicon.user_ctx = ctx;

    _uriPageRoot.uri = "/";
    _uriPageRoot.method = HTTP_GET;
    _uriPageRoot.handler = handlePageConnections;
    _uriPageRoot.user_ctx = ctx;

    _uriApiScanGet.uri = "/api/scan";
    _uriApiScanGet.method = HTTP_GET;
    _uriApiScanGet.handler = handleApiScanGet;
    _uriApiScanGet.user_ctx = ctx;

    _uriApiScanPost.uri = "/api/scan";
    _uriApiScanPost.method = HTTP_POST;
    _uriApiScanPost.handler = handleApiScanPost;
    _uriApiScanPost.user_ctx = ctx;

    _uriApiConnect.uri = "/api/connect";
    _uriApiConnect.method = HTTP_POST;
    _uriApiConnect.handler = handleApiConnect;
    _uriApiConnect.user_ctx = ctx;

    httpd_register_uri_handler(server, &_uriCss);
    httpd_register_uri_handler(server, &_uriFavicon);
    httpd_register_uri_handler(server, &_uriPageRoot);
    httpd_register_uri_handler(server, &_uriApiScanGet);
    httpd_register_uri_handler(server, &_uriApiScanPost);
    httpd_register_uri_handler(server, &_uriApiConnect);
}

esp_err_t WiFiConfigPage::handleCss(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=86400");
    httpd_resp_send(req, WiFiConfigPageContent::CSS, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WiFiConfigPage::handleFavicon(httpd_req_t* req) {
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=86400");
    httpd_resp_send(req, "x", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WiFiConfigPage::handlePageConnections(httpd_req_t* req) {
    WiFiProxy* wifiProxy = static_cast<WiFiProxy*>(req->user_ctx);
    char ssid[WiFiProxy::SSID_STR_LEN] = {};
    char ip[WiFiProxy::IP_ADDR_STR_LEN] = {};

    httpd_resp_set_type(req, "text/html");

    if (wifiProxy && wifiProxy->isConnected(ssid, sizeof(ssid), ip, sizeof(ip))) {
        char statusPage[sizeof(WiFiConfigPageContent::STATUS_PAGE_TEMPLATE) + WiFiProxy::SSID_STR_LEN +
                        WiFiProxy::IP_ADDR_STR_LEN];
        snprintf(statusPage, sizeof(statusPage), WiFiConfigPageContent::STATUS_PAGE_TEMPLATE, ssid, ip);
        httpd_resp_send(req, statusPage, HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_send(req, WiFiConfigPageContent::CONFIG_PAGE, HTTPD_RESP_USE_STRLEN);
    }

    return ESP_OK;
}

esp_err_t WiFiConfigPage::handleApiScanGet(httpd_req_t* req) {
    WiFiProxy* wifiProxy = static_cast<WiFiProxy*>(req->user_ctx);
    ScanResultSet results;
    char buf[2048];

    if (!wifiProxy || !wifiProxy->getScanResults(results)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scan mutex timeout");
        return ESP_FAIL;
    }

    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "{\"ready\":%s,\"networks\":[", results.ready ? "true" : "false");
    for (int i = 0; i < results.count && pos < (int)(sizeof(buf)) - 64; ++i) {
        const ScanResult& n = results.networks[i];

        char escapedSsid[67] = {};
        int ePos = 0;
        for (int c = 0; n.ssid[c] != '\0' && ePos < 65; ++c) {
            if (n.ssid[c] == '"' || n.ssid[c] == '\\') escapedSsid[ePos++] = '\\';
            escapedSsid[ePos++] = n.ssid[c];
        }

        pos += snprintf(buf + pos,
                        sizeof(buf) - pos,
                        "%s{\"ssid\":\"%s\",\"rssi\":%d,\"open\":%s,\"channel\":%d}",
                        (i > 0 ? "," : ""),
                        escapedSsid,
                        n.rssi,
                        n.open ? "true" : "false",
                        n.channel);
    }

    pos += snprintf(buf + pos, sizeof(buf) - pos, "]}");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buf, pos);

    return ESP_OK;
}

esp_err_t WiFiConfigPage::handleApiScanPost(httpd_req_t* req) {
    WiFiProxy* wifiProxy = static_cast<WiFiProxy*>(req->user_ctx);
    if (!wifiProxy) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No WiFi proxy");
        return ESP_FAIL;
    }

    wifiProxy->postScan();
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Scan started");

    return ESP_OK;
}

esp_err_t WiFiConfigPage::handleApiConnect(httpd_req_t* req) {
    WiFiProxy* wifiProxy = static_cast<WiFiProxy*>(req->user_ctx);
    if (!wifiProxy) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No WiFi proxy");
        return ESP_FAIL;
    }

    char body[256] = {};
    int received = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }

    body[received] = '\0';

    WmMsg msg;
    msg.type = WmMsgType::CONNECT;
    char rawSsid[sizeof(msg.ssid)] = {};
    char rawPassword[sizeof(msg.password)] = {};
    httpd_query_key_value(body, "ssid", rawSsid, sizeof(rawSsid));
    httpd_query_key_value(body, "password", rawPassword, sizeof(rawPassword));
    urlDecode(rawSsid, msg.ssid, sizeof(msg.ssid));
    urlDecode(rawPassword, msg.password, sizeof(msg.password));

    if (strlen(msg.ssid) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing SSID");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connect request. SSID: '%s'", msg.ssid);

    if (!wifiProxy->postConnect(msg.ssid, msg.password)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Manager queue full");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Connecting... check serial output for result.");

    return ESP_OK;
}
