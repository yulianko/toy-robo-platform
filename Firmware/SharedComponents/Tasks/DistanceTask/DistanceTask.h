#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/queue.h>

#include <atomic>

#include "BaseTask.h"
#include "DistanceScanProfile.h"
#include "HCSR04Driver.h"
#include "RobotEvent.h"

class DistanceTask : public BaseTask {
  public:
    static DistanceTask& instance() {
        static DistanceTask inst;
        return inst;
    }

    void init(IDistanceDriver& driver, QueueHandle_t robotEventQueue);
    void setProfile(const DistanceScanProfile& profile);
    void resetConfirmed();

  protected:
    const char* getTaskName() const override {
        return "DistanceTask";
    }
    uint32_t getStackSize() const override {
        return 3072;
    }
    bool isInitialized() const override {
        return _driver && _robotEventQueue;
    }
    void run() override;

  private:
    static RobotEvent::DistanceData::Range classifyDistance(uint16_t cm);
    void postEvent(RobotEvent::DistanceData::Range range, uint16_t cm, bool frontOfQueue);
    void resetHysteresis();

    IDistanceDriver* _driver = nullptr;
    QueueHandle_t _robotEventQueue = nullptr;

    portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
    DistanceScanProfile _staged = DistanceScanProfile::idle();
    std::atomic<bool> _profileDirty{false};

    DistanceScanProfile _profile = DistanceScanProfile::idle();
    RobotEvent::DistanceData::Range _confirmed = RobotEvent::DistanceData::Range::Unknown;
    RobotEvent::DistanceData::Range _pending = RobotEvent::DistanceData::Range::Unknown;
    uint8_t _confirmCnt = 0;
};
