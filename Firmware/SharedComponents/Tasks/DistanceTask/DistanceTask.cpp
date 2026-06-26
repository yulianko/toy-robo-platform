#include "DistanceTask.h"

#include <esp_log.h>

#include "RobotEvent.h"

static const char* TAG = "DistanceTask";

void DistanceTask::init(IDistanceDriver& driver, QueueHandle_t robotEventQueue) {
    _driver = &driver;
    _robotEventQueue = robotEventQueue;
    ESP_LOGI(TAG, "initialized");
}

void DistanceTask::setProfile(const DistanceScanProfile& profile) {
    portENTER_CRITICAL(&_mux);
    _staged = profile;
    portEXIT_CRITICAL(&_mux);
    _profileDirty.store(true, std::memory_order_release);
}

void DistanceTask::run() {
    ESP_LOGI(TAG, "started");

    while (true) {
        // Pick up a new profile if one was staged
        if (_profileDirty.exchange(false, std::memory_order_acquire)) {
            portENTER_CRITICAL(&_mux);
            _profile = _staged;
            portEXIT_CRITICAL(&_mux);
            resetHysteresis();
            ESP_LOGI(TAG,
                     "profile updated: interval=%lums confirms=%u raw=%d",
                     _profile.intervalMs,
                     _profile.confirmCount,
                     _profile.postRaw);
        }

        uint16_t cm = 0;
        esp_err_t err = _driver->measure(cm);
        RobotEvent::DistanceData::Range measured =
            (err == ESP_OK) ? classifyDistance(cm) : RobotEvent::DistanceData::Range::Clear;

        if (_profile.postRaw) {
            postEvent(measured, (err == ESP_OK ? cm : 0), _profile.frontOfQueue);
        } else {
            if (measured == _pending) {
                if (++_confirmCnt >= _profile.confirmCount && measured != _confirmed) {
                    _confirmed = measured;
                    _confirmCnt = 0;
                    postEvent(measured, (err == ESP_OK ? cm : 0), _profile.frontOfQueue);
                }
            } else {
                _pending = measured;
                _confirmCnt = 1;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(_profile.intervalMs));
    }
}

void DistanceTask::resetHysteresis() {
    _confirmed = RobotEvent::DistanceData::Range::Unknown;
    _pending = RobotEvent::DistanceData::Range::Unknown;
    _confirmCnt = 0;
}

RobotEvent::DistanceData::Range DistanceTask::classifyDistance(uint16_t cm) {
    if (cm <= 20) return RobotEvent::DistanceData::Range::Critical;
    if (cm <= 50) return RobotEvent::DistanceData::Range::Close;
    if (cm <= 100) return RobotEvent::DistanceData::Range::Near;
    if (cm <= 200) return RobotEvent::DistanceData::Range::Far;
    return RobotEvent::DistanceData::Range::Clear;
}

void DistanceTask::postEvent(RobotEvent::DistanceData::Range range, uint16_t cm, bool frontOfQueue) {
    RobotEvent::DistanceData data{.cm = cm, .range = range};
    RobotEvent event(RobotEvent::Type::DISTANCE_RANGE_CHANGED, data);

    BaseType_t sent =
        frontOfQueue ? xQueueSendToFront(_robotEventQueue, &event, 0) : xQueueSend(_robotEventQueue, &event, 0);

    if (sent != pdTRUE) {
        ESP_LOGW(TAG, "event queue full - dropped range=%d cm=%lu", static_cast<int>(range), cm);
    }
}
