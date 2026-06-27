#include "MotionPlayer.h"

#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "MotionPlayer";

static uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

MotionPlayer::MotionPlayer(Drivetrain& drivetrain) : _drivetrain(drivetrain) {
}

esp_err_t MotionPlayer::play(const MotionSequence& sequence) {
    if (!sequence.valid()) {
        ESP_LOGW(TAG, "play: empty sequence ignored");
        return ESP_ERR_INVALID_ARG;
    }

    _sequence = sequence;
    _stepIndex = 0;
    _loopCount = 0;
    _playing = true;
    _stepStartMs = nowMs();

    ESP_LOGI(TAG, "play: '%s'  steps=%u loops=%u", _sequence.name(), _sequence.count(), _sequence.loops());

    return applyStep(_sequence.steps()[0]);
}

esp_err_t MotionPlayer::stop() {
    _playing = false;
    ESP_LOGD(TAG, "stop (coast)");
    return _drivetrain.coast();
}

esp_err_t MotionPlayer::brake() {
    _playing = false;
    ESP_LOGD(TAG, "brake");
    return _drivetrain.brake();
}

bool MotionPlayer::isPlaying() const {
    return _playing;
}

void MotionPlayer::tick() {
    if (!_playing) {
        return;
    }

    const MotionStep& step = _sequence.steps()[_stepIndex];

    // durationMs == 0 means hold indefinitely
    if (step.verb != MotionVerb::Brake && step.verb != MotionVerb::Coast && step.durationMs == 0) {
        return;
    }

    const uint32_t elapsed = nowMs() - _stepStartMs;
    if (elapsed >= step.durationMs) {
        esp_err_t err = advance();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "advance failed: %s", esp_err_to_name(err));
            _playing = false;
        }
    }
}

esp_err_t MotionPlayer::applyStep(const MotionStep& step) {
    ESP_LOGD(TAG,
             "step[%u] verb=%u speed=%.2f dur=%lums",
             _stepIndex,
             static_cast<uint8_t>(step.verb),
             step.speed,
             step.durationMs);

    switch (step.verb) {
        case MotionVerb::Forward:
            return _drivetrain.forward(step.speed);
        case MotionVerb::Backward:
            return _drivetrain.backward(step.speed);
        case MotionVerb::TurnLeft:
            return _drivetrain.turnLeft(step.speed);
        case MotionVerb::TurnRight:
            return _drivetrain.turnRight(step.speed);
        case MotionVerb::PivotLeft:
            return _drivetrain.pivotLeft(step.speed);
        case MotionVerb::PivotRight:
            return _drivetrain.pivotRight(step.speed);
        case MotionVerb::Brake:
            return _drivetrain.brake();
        case MotionVerb::Coast:
            return _drivetrain.coast();
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t MotionPlayer::advance() {
    const uint8_t nextStep = _stepIndex + 1;

    if (nextStep < _sequence.count()) {
        ESP_LOGD(TAG, "Next step. step %d, loop %u", nextStep, _loopCount);
        _stepIndex = nextStep;
        _stepStartMs = nowMs();
        return applyStep(_sequence.steps()[_stepIndex]);
    }

    const bool infinite = (_sequence.loops() == MotionSequence::LOOP_INFINITE);
    _loopCount++;

    if (infinite || _loopCount < _sequence.loops()) {
        _stepIndex = 0;
        _stepStartMs = nowMs();
        ESP_LOGD(TAG, "loop %u", _loopCount);
        return applyStep(_sequence.steps()[0]);
    }

    // Sequence complete
    _playing = false;
    ESP_LOGI(TAG, "'%s' complete", _sequence.name());

    const MotionVerb lastVerb = _sequence.steps()[_sequence.count() - 1].verb;
    if (lastVerb != MotionVerb::Brake && lastVerb != MotionVerb::Coast) {
        return _drivetrain.coast();
    }

    return ESP_OK;
}
