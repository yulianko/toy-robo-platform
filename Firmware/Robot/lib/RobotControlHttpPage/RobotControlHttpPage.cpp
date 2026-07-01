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

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Extract speed parameter from JSON payload, default to 0.65
    float speed = 0.65f;
    const char* speedPtr = strstr(buf, "\"speed\":");
    if (speedPtr) {
        sscanf(speedPtr, "\"speed\":%f", &speed);
        // Clamp speed to valid range [0, 1]
        if (speed < 0.0f) speed = 0.0f;
        if (speed > 1.0f) speed = 1.0f;
    }

    // Crude JSON or query parsing matching incoming payload {dir: '...'}
    if (strstr(buf, "\"dir\":\"forward\"")) {
        page->_robotContext->motion.play(MotionSequences::forwardFor(speed, 1000));
        ESP_LOGI(TAG, "Moving Forward (speed: %.2f)", speed);
    } else if (strstr(buf, "\"dir\":\"left\"")) {
        page->_robotContext->motion.play(MotionSequences::turnLeftFor(speed, 1000));
        ESP_LOGI(TAG, "Turning Left (speed: %.2f)", speed);
    } else if (strstr(buf, "\"dir\":\"right\"")) {
        page->_robotContext->motion.play(MotionSequences::turnRightFor(speed, 1000));
        ESP_LOGI(TAG, "Turning Right (speed: %.2f)", speed);
    } else if (strstr(buf, "\"dir\":\"backward\"")) {
        page->_robotContext->motion.play(MotionSequences::backwardFor(speed, 1000));
        ESP_LOGI(TAG, "Moving Backward (speed: %.2f)", speed);
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

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
        buf[ret] = '\0';
        if (strstr(buf, "\"action\":\"play\"")) {
            // Extract animation type from JSON payload
            int animValue = -1;
            const char* animPtr = strstr(buf, "\"animation\":");
            if (animPtr) {
                sscanf(animPtr, "\"animation\":%d", &animValue);

                AnimationType anim = static_cast<AnimationType>(animValue);
                switch (anim) {
                    case AnimationType::Exploring:
                        page->_robotContext->indicators.start(RobotAnimations::exploring(), RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Exploring");
                        break;
                    case AnimationType::Curiosity:
                        page->_robotContext->indicators.start(RobotAnimations::curiosity(), RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Curiosity");
                        break;
                    case AnimationType::Surprise:
                        page->_robotContext->indicators.start(RobotAnimations::surprise(), RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Surprise");
                        break;
                    case AnimationType::Agreement:
                        page->_robotContext->indicators.start(RobotAnimations::agreement(), RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Agreement");
                        break;
                    case AnimationType::Disagreement:
                        page->_robotContext->indicators.start(RobotAnimations::disagreement(),
                                                              RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Disagreement");
                        break;
                    case AnimationType::Danger:
                        page->_robotContext->indicators.start(RobotAnimations::danger(), RobotSounds::curiosity());
                        ESP_LOGI(TAG, "Indicator: Danger");
                        break;
                    default:
                        ESP_LOGW(TAG, "Unknown animation type: %d", animValue);
                        break;
                }
            }
        } else if (strstr(buf, "\"action\":\"stop\"")) {
            page->_robotContext->indicators.stop();
            ESP_LOGI(TAG, "Indicator Stopped");
        } else if (strstr(buf, "\"action\":\"alert\"")) {
            // Legacy/compatibility: alert maps to curiosity
            page->_robotContext->indicators.start(RobotAnimations::curiosity(), RobotSounds::curiosity());
            ESP_LOGI(TAG, "Indicator Alert Triggered");
        }
    }

    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}
