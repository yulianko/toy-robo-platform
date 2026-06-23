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
        const char* eventName = eventTypeToString(event.type);
        ESP_LOGI("PrintMode", "Event: %s", eventName);
    }

  private:
    RobotContext* _ctx = nullptr;

    const char* eventTypeToString(RobotEvent::Type type) {
        switch (type) {
            case RobotEvent::Type::SYS_BUTTON_SHORT_PRESSED:
                return "SYS_BTN_SHORT";
            case RobotEvent::Type::SYS_BUTTON_LONG_PRESSED:
                return "SYS_BTN_LONG";
            case RobotEvent::Type::PUSH_BUTTON_SHORT_PRESSED:
                return "PUSH_BTN_SHORT";
            case RobotEvent::Type::PUSH_BUTTON_LONG_PRESSED:
                return "PUSH_BTN_LONG";
            default:
                return "UNKNOWN";
        }
    }
};

class CounterMode : public IMode {
  public:
    const char* name() const override {
        return "Counter";
    }

    void onEnter(RobotContext& ctx) override {
        _ctx = &ctx;
        _pushCount = 0;
        _totalEvents = 0;
        ESP_LOGI("CounterMode", "--- ENTERED CounterMode ---");
        ESP_LOGI("CounterMode", "Push button to increment counter, long press to show stats");
    }

    void onExit() override {
        ESP_LOGI("CounterMode", "--- EXITED CounterMode ---");
        ESP_LOGI("CounterMode", "Final stats: Push=%d, Total=%d", _pushCount, _totalEvents);
        _ctx = nullptr;
    }

    void onEvent(const RobotEvent& event) override {
        _totalEvents++;

        switch (event.type) {
            case RobotEvent::Type::PUSH_BUTTON_SHORT_PRESSED:
                _pushCount++;
                ESP_LOGI("CounterMode", "Push count: %d", _pushCount);
                break;

            case RobotEvent::Type::PUSH_BUTTON_LONG_PRESSED:
                ESP_LOGI("CounterMode", "=== STATS ===");
                ESP_LOGI("CounterMode", "Push events: %d", _pushCount);
                ESP_LOGI("CounterMode", "Total events: %d", _totalEvents);
                break;

            default:
                ESP_LOGI("CounterMode", "Other event received (total: %d)", _totalEvents);
                break;
        }
    }

  private:
    RobotContext* _ctx = nullptr;
    int _pushCount = 0;
    int _totalEvents = 0;
};
