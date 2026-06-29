#include "RobotControlHttpPage.h"

#include <esp_log.h>
#include <string.h>

#include "MotionSequences.h"
#include "RobotAnimations.h"
#include "RobotContext.h"
#include "RobotControlHttpPageContent.h"
#include "RobotSounds.h"

static const char* TAG = "RobotControlHttpPage";

void RobotControlHttpPage::init(RobotContext* robotContext) {
    _robotContext = robotContext;

    _handlers[0].uri = "";
    _handlers[0].method = HTTP_GET;
    _handlers[0].handler = handlePage;
    _handlers[0].user_ctx = this;

    _handlers[1].uri = "style.css";
    _handlers[1].method = HTTP_GET;
    _handlers[1].handler = handleCss;
    _handlers[1].user_ctx = this;

    _handlers[2].uri = "api/command/move";
    _handlers[2].method = HTTP_POST;
    _handlers[2].handler = handleApiMove;
    _handlers[2].user_ctx = this;

    _handlers[3].uri = "api/command/stop";
    _handlers[3].method = HTTP_POST;
    _handlers[3].handler = handleApiStop;
    _handlers[3].user_ctx = this;

    _handlers[4].uri = "api/command/indicator";
    _handlers[4].method = HTTP_POST;
    _handlers[4].handler = handleApiIndicator;
    _handlers[4].user_ctx = this;
}

esp_err_t RobotControlHttpPage::handlePage(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, RobotControlHttpPageContent::HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t RobotControlHttpPage::handleCss(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=86400");
    httpd_resp_send(req, RobotControlHttpPageContent::CSS, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t RobotControlHttpPage::handleApiMove(httpd_req_t* req) {
    RobotControlHttpPage* page = static_cast<RobotControlHttpPage*>(req->user_ctx);
    if (!page) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Context missing");
        return ESP_FAIL;
    }

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Crude JSON or query parsing matching incoming payload {dir: '...'}
    if (strstr(buf, "\"dir\":\"forward\"")) {
        page->_robotContext->motion.play(MotionSequences::forwardFor(0.65, 1000));
        ESP_LOGI(TAG, "Moving Forward");
    } else if (strstr(buf, "\"dir\":\"left\"")) {
        page->_robotContext->motion.play(MotionSequences::turnLeftFor(0.65, 1000));
        ESP_LOGI(TAG, "Turning Left");
    } else if (strstr(buf, "\"dir\":\"right\"")) {
        page->_robotContext->motion.play(MotionSequences::turnRightFor(0.65, 1000));
        ESP_LOGI(TAG, "Turning Right");
    } else if (strstr(buf, "\"dir\":\"backward\"")) {
        page->_robotContext->motion.play(MotionSequences::backwardFor(0.65, 1000));
        ESP_LOGI(TAG, "Moving Backward");
    }

    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}

esp_err_t RobotControlHttpPage::handleApiStop(httpd_req_t* req) {
    RobotControlHttpPage* page = static_cast<RobotControlHttpPage*>(req->user_ctx);
    if (page) {
        page->_robotContext->motion.stop();
        ESP_LOGI(TAG, "Stopping robot motion");
    }
    httpd_resp_sendstr(req, "{\"status\":\"stopped\"}");
    return ESP_OK;
}

esp_err_t RobotControlHttpPage::handleApiIndicator(httpd_req_t* req) {
    RobotControlHttpPage* page = static_cast<RobotControlHttpPage*>(req->user_ctx);
    if (!page) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Context missing");
        return ESP_FAIL;
    }

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
        buf[ret] = '\0';
        if (strstr(buf, "\"action\":\"alert\"")) {
            page->_robotContext->indicators.start(RobotAnimations::curiosity(), RobotSounds::curiosity());
            ESP_LOGI(TAG, "Indicator Alert Triggered");
        } else if (strstr(buf, "\"action\":\"stop\"")) {
            page->_robotContext->indicators.stop();
            ESP_LOGI(TAG, "Indicator Stopped");
        }
    }

    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}
