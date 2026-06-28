#pragma once

#include "IMode.h"
#include "esp_log.h"

class PrintMode : public IMode {
  public:
    const char* name() const override {
        return "Print";
    }

    void onEnter(RobotContext& ctx) override {
        _ctx = &ctx;
        ESP_LOGI("PrintMode", "--- ENTERED PrintMode ---");
    }

    void onExit() override {
        ESP_LOGI("PrintMode", "--- EXITED PrintMode ---");
        _ctx = nullptr;
    }

    void onEvent(const RobotEvent& event) override {
        const char* eventName = RobotEvent::typeToString(event.type);
        ESP_LOGI("PrintMode", "Event: %s", eventName);
    }

  private:
    RobotContext* _ctx = nullptr;
};
