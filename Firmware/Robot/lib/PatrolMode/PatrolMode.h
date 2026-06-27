#pragma once

#include "IMode.h"
#include "RobotEvent.h"

class PatrolMode : public IMode {
  public:
    const char* name() const override {
        return "PatrolMode";
    }

    void onEnter(RobotContext& ctx) override;
    void onExit() override;
    void onEvent(const RobotEvent& event) override;

  private:
    enum class Step : uint8_t {
        MoveForward,
        PivotRight,
    };

    enum class State : uint8_t {
        Patrolling,
        Evading,
    };

    void startForward();
    void startPivot();
    void onDistanceChanged(RobotEvent::DistanceData::Range range, uint16_t cm);

    static const char* stepToString(Step step);

    RobotContext* _ctx = nullptr;
    Step _step = Step::MoveForward;
    State _state = State::Patrolling;

    static constexpr float PATROL_SPEED = 0.7f;
    static constexpr uint32_t FORWARD_MS = 2000;
    static constexpr uint32_t PIVOT_MS = 500;
};
