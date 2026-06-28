#include "SelfTestMode.h"

#include <esp_log.h>

#include "MotionSequences.h"
#include "RobotAnimations.h"
#include "RobotSounds.h"

static constexpr const char* TAG = "SelfTestMode";

const SelfTestMode::AnimationStepDefinition SelfTestMode::ANIMATION_STEPS[] = {
    {RobotAnimations::exploring, RobotSounds::exploring},
    {RobotAnimations::curiosity, RobotSounds::curiosity},
    {RobotAnimations::surprise, RobotSounds::surprise},
    {RobotAnimations::agreement, RobotSounds::agreement},
    {RobotAnimations::disagreement, RobotSounds::disagreement},
    {RobotAnimations::danger, RobotSounds::danger},
};

const SelfTestMode::MotionStepDefinition SelfTestMode::MOTION_STEPS[] = {
    {RgbColor::blue(), 200, [] { return MotionSequences::forwardFor(0.65f, 1000); }},
    {RgbColor::green(), 200, [] { return MotionSequences::backwardFor(0.65f, 1000); }},
    {RgbColor::cyan(), 200, [] { return MotionSequences::pivotRightFor(0.65f, 1000); }},
    {RgbColor::orange(), 200, [] { return MotionSequences::pivotLeftFor(0.65f, 1000); }},
    {RgbColor::magenta(), 200, [] { return MotionSequences::turnRightFor(0.65f, 1000); }},
    {RgbColor::yellow(), 200, [] { return MotionSequences::turnLeftFor(0.65f, 1000); }},
};

const size_t SelfTestMode::ANIMATION_STEPS_COUNT = sizeof(ANIMATION_STEPS) / sizeof(ANIMATION_STEPS[0]);
const size_t SelfTestMode::MOTION_STEPS_COUNT = sizeof(MOTION_STEPS) / sizeof(MOTION_STEPS[0]);

void SelfTestMode::onEnter(RobotContext& ctx) {
    _ctx = &ctx;
    _ctx->distance.setProfile(DistanceScanProfile::idle());
    _ctx->motion.brake();
    ESP_LOGI(TAG, "Entered SelfTest Mode");
    enterState(State::ANIMATIONS);
}

void SelfTestMode::onExit() {
    _ctx->indicators.stop();
    _ctx->distance.setProfile(DistanceScanProfile::idle());
    _ctx->motion.brake();
    _ctx = nullptr;
    ESP_LOGI(TAG, "Exited SelfTest Mode");
}

void SelfTestMode::onEvent(const RobotEvent& event) {
    switch (event.type) {
        case RobotEvent::Type::INDICATORS_ANIMATION_DONE:
            if (_state == State::ANIMATIONS) {
                advanceStep();
            }
            return;
        case RobotEvent::Type::MOTION_DONE:
            if (_state == State::MOTIONS) {
                advanceStep();
            }
            return;
        case RobotEvent::Type::DISTANCE_RANGE_CHANGED:
            if (_state == State::INTERACTIVE) {
                handleDistanceIndicator(event.distance.range);
            }
            return;
        default:
            return;
    }
}

void SelfTestMode::enterState(State next) {
    _state = next;
    _stepIndex = 0;

    switch (_state) {
        case State::ANIMATIONS:
            ESP_LOGI(TAG, "State: Animations (%d steps)", ANIMATION_STEPS_COUNT);
            advanceStep();
            return;

        case State::MOTIONS:
            ESP_LOGI(TAG, "State: Motions (%d steps)", MOTION_STEPS_COUNT);
            advanceStep();
            return;

        case State::INTERACTIVE:
            ESP_LOGI(TAG, "State: Interactive");
            _ctx->motion.brake();
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::white()), RobotSounds::beep());

            _ctx->distance.setProfile(DistanceScanProfile::normal());
            return;

        default:
            return;
    }
}

void SelfTestMode::advanceStep() {
    switch (_state) {
        case State::ANIMATIONS: {
            if (_stepIndex >= static_cast<int>(ANIMATION_STEPS_COUNT)) {
                enterState(State::MOTIONS);
                break;
            }

            const SelfTestMode::AnimationStepDefinition& def = ANIMATION_STEPS[_stepIndex++];
            ESP_LOGI(TAG, "[Anim %d/%d]", _stepIndex, ANIMATION_STEPS_COUNT);
            _ctx->indicators.start(def.animation(), def.sound());
            break;
        }

        case State::MOTIONS: {
            if (_stepIndex >= static_cast<int>(MOTION_STEPS_COUNT)) {
                enterState(State::INTERACTIVE);
                break;
            }

            const SelfTestMode::MotionStepDefinition& def = MOTION_STEPS[_stepIndex++];
            ESP_LOGI(TAG, "[Motion %d/%d]", _stepIndex, MOTION_STEPS_COUNT);
            _ctx->indicators.start(RobotAnimations::pulse(def.color, def.pulseMs));
            _ctx->motion.play(def.sequence());
            break;
        }

        default: {
            break;
        }
    }
}

void SelfTestMode::handleDistanceIndicator(RobotEvent::DistanceData::Range range) {
    switch (range) {
        case RobotEvent::DistanceData::Range::Critical:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::red(), 200), RobotSounds::beep(1));
            break;
        case RobotEvent::DistanceData::Range::Near:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::orange(), 500));
            break;
        case RobotEvent::DistanceData::Range::Clear:
            _ctx->indicators.start(RobotAnimations::pulse(RgbColor::green(), 1000));
            break;
        default:
            break;
    }
}
