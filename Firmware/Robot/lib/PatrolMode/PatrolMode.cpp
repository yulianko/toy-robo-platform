#include "PatrolMode.h"

#include <esp_log.h>

#include "DistanceScanProfile.h"
#include "MotionSequences.h"
#include "RobotAnimations.h"
#include "RobotEvent.h"
#include "RobotSounds.h"

static const char* TAG = "PatrolMode";

void PatrolMode::onEnter(RobotContext& ctx) {
    _ctx = &ctx;
    _step = Step::MoveForward;
    _state = State::Patrolling;

    ctx.distance.setProfile(DistanceScanProfile::obstacle());
    ctx.indicators.start(RobotAnimations::exploring(), RobotSounds::exploring());

    ESP_LOGI(TAG, "entered");
    startForward();
}

void PatrolMode::onExit() {
    _ctx->motion.brake();
    _ctx->indicators.stop();
    _ctx->distance.setProfile(DistanceScanProfile::idle());
    _ctx = nullptr;
    ESP_LOGI(TAG, "exited");
}

void PatrolMode::onEvent(const RobotEvent& event) {
    switch (event.type) {
        case RobotEvent::Type::MOTION_DONE:
            onPreviousMovementDone();
            break;
        case RobotEvent::Type::DISTANCE_RANGE_CHANGED:
            onDistanceChanged(event.distance.range, event.distance.cm);
            break;
        default:
            break;
    }
}

void PatrolMode::onPreviousMovementDone() {
    if (_state == State::Evading) {
        _state = State::Patrolling;
        _step = Step::MoveForward;
        ESP_LOGI(TAG, "evasion complete - resuming patrol");
        startForward();
        return;
    }

    switch (_step) {
        case Step::MoveForward:
            startPivotRight();
            break;
        case Step::PivotRight:
            startForward();
            break;
    }
}

void PatrolMode::onDistanceChanged(RobotEvent::DistanceData::Range range, uint16_t cm) {
    if (_state == State::Evading) {
        return;
    }

    switch (range) {
        case RobotEvent::DistanceData::Range::Critical: {
            ESP_LOGI(TAG, "obstacle at %u cm - evading", cm);
            _state = State::Evading;

            // Reset hysteresis so the task fires again after pivot,
            // even if the new obstacle is at the same range
            _ctx->distance.resetConfirmed();

            // Interrupt current motion immediately
            _ctx->indicators.start(RobotAnimations::danger(), RobotSounds::danger());

            // Back up, pivot away, then MOTION_DONE resumes patrol
            _ctx->motion.play(MotionSequences::evadeRight(PATROL_SPEED, 600, 700));
            break;
        }

        case RobotEvent::DistanceData::Range::Close:
        case RobotEvent::DistanceData::Range::Near:
            // Slow warning - visual only, don't interrupt motion yet
            _ctx->indicators.start(RobotAnimations::curiosity(), RobotSounds::curiosity());
            break;

        case RobotEvent::DistanceData::Range::Far:
            if (_step == Step::MoveForward) {
                _ctx->indicators.start(RobotAnimations::surprise());
            }
            break;

        case RobotEvent::DistanceData::Range::Clear:
        case RobotEvent::DistanceData::Range::Unknown:
        default:
            break;
    }
}

void PatrolMode::startForward() {
    _step = Step::MoveForward;
    ESP_LOGI(TAG, "step: %s", stepToString(_step));

    _ctx->motion.play(MotionSequences::forwardFor(PATROL_SPEED, FORWARD_MS));
    _ctx->indicators.start(RobotAnimations::exploring());
}

void PatrolMode::startPivotRight() {
    _step = Step::PivotRight;
    ESP_LOGI(TAG, "step: %s", stepToString(_step));

    _ctx->motion.play(MotionSequences::pivotRightFor(PATROL_SPEED, PIVOT_MS));
    _ctx->indicators.start(RobotAnimations::agreement(), RobotSounds::agreement());
}

const char* PatrolMode::stepToString(Step step) {
    switch (step) {
        case Step::MoveForward:
            return "MoveForward";
        case Step::PivotRight:
            return "PivotRight";
    }
    return "?";
}
