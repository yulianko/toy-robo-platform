#pragma once

#include "DistanceScanProfile.h"
#include "IMode.h"

class ConfigMode : public IMode {
  public:
    const char* name() const override {
        return "ConfigMode";
    }

    void onEnter(RobotContext& ctx) override;
    void onExit() override;
    void onEvent(const RobotEvent& event) override;

  private:
    RobotContext* _ctx = nullptr;
};
