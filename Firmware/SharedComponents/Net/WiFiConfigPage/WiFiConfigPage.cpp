#include "WiFiConfigPage.h"

#include "HttpUtils.h"
#include "WiFiConfigPageContent.h"
#include "WiFiProxy.h"

static const char* TAG = "WiFiConfigPage";

void WiFiConfigPage::init(WiFiProxy* wifiProxy) {
    _wifiProxy = wifiProxy;

    _handlers[0].uri = "style.css";
    _handlers[0].method = HTTP_GET;
    _handlers[0].handler = handleCss;
    _handlers[0].user_ctx = this;

    _handlers[1].uri = "";
    _handlers[1].method = HTTP_GET;
    _handlers[1].handler = handlePageConnections;
    _handlers[1].user_ctx = this;

    _handlers[2].uri = "api/scan";
    _handlers[2].method = HTTP_GET;
    _handlers[2].handler = handleApiScanGet;
    _handlers[2].user_ctx = this;

    _handlers[3].uri = "api/scan";
    _handlers[3].method = HTTP_POST;
    _handlers[3].handler = handleApiScanPost;
    _handlers[3].user_ctx = this;

    _handlers[4].uri = "api/connect";
    _handlers[4].method = HTTP_POST;
    _handlers[4].handler = handleApiConnect;
    _handlers[4].user_ctx = this;
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
    WiFiConfigPage* page = static_cast<WiFiConfigPage*>(req->user_ctx);
    char ssid[WiFiProxy::SSID_STR_LEN] = {};
    char ip[WiFiProxy::IP_ADDR_STR_LEN] = {};

    httpd_resp_set_type(req, "text/html");

    if (page && page->_wifiProxy->isConnected(ssid, sizeof(ssid), ip, sizeof(ip))) {
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
    WiFiConfigPage* page = static_cast<WiFiConfigPage*>(req->user_ctx);
    ScanResultSet results;
    char buf[2048];

    if (!page || !page->_wifiProxy->getScanResults(results)) {
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
    WiFiConfigPage* page = static_cast<WiFiConfigPage*>(req->user_ctx);
    if (!page) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No WiFi proxy");
        return ESP_FAIL;
    }

    page->_wifiProxy->postScan();
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Scan started");

    return ESP_OK;
}

esp_err_t WiFiConfigPage::handleApiConnect(httpd_req_t* req) {
    WiFiConfigPage* page = static_cast<WiFiConfigPage*>(req->user_ctx);
    if (!page) {
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
    HttpUtils::urlDecode(rawSsid, msg.ssid, sizeof(msg.ssid));
    HttpUtils::urlDecode(rawPassword, msg.password, sizeof(msg.password));

    if (strlen(msg.ssid) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing SSID");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connect request. SSID: '%s'", msg.ssid);

    if (!page->_wifiProxy->postConnect(msg.ssid, msg.password)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Manager queue full");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Connecting... check serial output for result.");

    return ESP_OK;
}
