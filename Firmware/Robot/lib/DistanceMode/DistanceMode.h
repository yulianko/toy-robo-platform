#pragma once

#include "DistanceScanProfile.h"
#include "IMode.h"

class DistanceMode : public IMode {
  public:
    const char* name() const override {
        return "DistanceMode";
    }

    void onEnter(RobotContext& ctx) override;
    void onExit() override;
    void onEvent(const RobotEvent& event) override;

  private:
    void updateIndicators(RobotEvent::DistanceData::Range range);
    static const char* rangeToString(RobotEvent::DistanceData::Range r);

    RobotContext* _ctx = nullptr;
};
