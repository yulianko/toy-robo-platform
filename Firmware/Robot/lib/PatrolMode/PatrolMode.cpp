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
    if (event.type == RobotEvent::Type::MOTION_DONE) {
        if (_state == State::Evading) {
            _state = State::Patrolling;
            _step = Step::MoveForward;
            ESP_LOGI(TAG, "evasion complete - resuming patrol");
            startForward();
            return;
        }

        switch (_step) {
            case Step::MoveForward:
                startPivot();
                break;
            case Step::PivotRight:
                startForward();
                break;
        }
        return;
    }

    if (event.type == RobotEvent::Type::DISTANCE_RANGE_CHANGED) {
        onDistanceChanged(event.distance.range, event.distance.cm);
        return;
    }
}

void PatrolMode::startForward() {
    _step = Step::MoveForward;
    ESP_LOGI(TAG, "step: %s", stepToString(_step));

    const MotionStep steps[] = {
        {MotionVerb::Forward, PATROL_SPEED, FORWARD_MS},
    };
    _ctx->motion.play(MotionSequence("patrol_forward", steps, 1));
    _ctx->indicators.start(RobotEmotions::curiosity());
}

void PatrolMode::startPivot() {
    _step = Step::PivotRight;
    ESP_LOGI(TAG, "step: %s", stepToString(_step));

    const MotionStep steps[] = {
        {MotionVerb::PivotRight, PATROL_SPEED, PIVOT_MS},
    };
    _ctx->motion.play(MotionSequence("patrol_pivot", steps, 1));
    _ctx->indicators.start(RobotEmotions::agreement(), RobotSounds::agreement());
}

void PatrolMode::onDistanceChanged(RobotEvent::DistanceData::Range range, uint16_t cm) {
    if (_state == State::Evading) {
        return;
    }

    switch (range) {
        case RobotEvent::DistanceData::Range::Critical:
        case RobotEvent::DistanceData::Range::Close: {
            ESP_LOGI(TAG, "obstacle at %u cm - evading", cm);
            _state = State::Evading;

            // Reset hysteresis so the task fires again after pivot,
            // even if the new obstacle is at the same range
            _ctx->distance.resetConfirmed();

            // Interrupt current motion immediately
            _ctx->indicators.start(RobotEmotions::surprise(), RobotSounds::surprise());

            // Back up, pivot away, then MOTION_DONE resumes patrol
            const MotionStep steps[] = {
                {MotionVerb::Backward, PATROL_SPEED, 600},
                {MotionVerb::PivotRight, PATROL_SPEED, 700},
                {MotionVerb::Brake, 0.0f, 0},
            };
            _ctx->motion.play(MotionSequence("evade", steps, 3));
            break;
        }

        case RobotEvent::DistanceData::Range::Near:
            // Slow warning - visual only, don't interrupt motion yet
            _ctx->indicators.start(RobotEmotions::curiosity());
            break;

        case RobotEvent::DistanceData::Range::Far:
        case RobotEvent::DistanceData::Range::Clear:
        case RobotEvent::DistanceData::Range::Unknown:
            if (_step == Step::MoveForward) {
                _ctx->indicators.start(RobotEmotions::curiosity());
            }
            break;
    }
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
