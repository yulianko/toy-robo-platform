#pragma once

#include "IMode.h"
#include "MotionSequence.h"
#include "RgbAnimation.h"
#include "RobotContext.h"
#include "RobotEvent.h"
#include "SoundAnimation.h"

class SelfTestMode : public IMode {
  public:
    const char* name() const {
        return "SelfTest";
    }

    void onEnter(RobotContext& ctx) override;
    void onExit() override;
    void onEvent(const RobotEvent& event) override;

  private:
    enum class State { INIT, ANIMATIONS, MOTIONS, INTERACTIVE };

    struct AnimationStepDefinition {
        const RgbAnimation (*animation)();
        const SoundAnimation (*sound)();
    };

    struct MotionStepDefinition {
        RgbColor color;
        int pulseMs;
        MotionSequence (*sequence)();
    };

    static const AnimationStepDefinition ANIMATION_STEPS[];
    static const MotionStepDefinition MOTION_STEPS[];
    static const size_t ANIMATION_STEPS_COUNT;
    static const size_t MOTION_STEPS_COUNT;

    RobotContext* _ctx = nullptr;
    State _state = State::INIT;
    int _stepIndex = 0;

    void enterState(State next);
    void advanceStep();
    void handleDistanceIndicator(RobotEvent::DistanceData::Range range);
};
