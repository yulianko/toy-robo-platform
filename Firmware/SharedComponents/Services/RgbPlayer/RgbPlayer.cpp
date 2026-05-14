#include "RgbPlayer.h"

#include "esp_log.h"

static const RgbAnimation EMPTY_ANIMATION("", nullptr, 0);
static constexpr char TAG[] = "RgbPlayer";

RgbPlayer::RgbPlayer(IRgbLedDriver& driver) : _driver(driver), _animation(EMPTY_ANIMATION) {
}

esp_err_t RgbPlayer::start(const RgbEffect& effect, uint32_t nowMs, uint32_t durationMs) {
    ESP_LOGI(TAG, "Starting effect for %lu ms", durationMs);

    RgbAnimation::Step step{effect, durationMs};
    _animation = RgbAnimation("", &step, 1, 1);
    _stepIndex = 0;
    _loopCount = 0;
    _isDone = false;
    return startCurrentStep(nowMs);
}

esp_err_t RgbPlayer::start(const RgbAnimation& animation, uint32_t nowMs) {
    ESP_LOGI(TAG, "Starting animation '%s'", animation.name);

    _animation = animation;
    _stepIndex = 0;
    _loopCount = 0;
    _isDone = _animation.isEmpty();

    if (_isDone) {
        return ESP_OK;
    }

    return startCurrentStep(nowMs);
}

esp_err_t RgbPlayer::tick(uint32_t nowMs) {
    if (_isDone) {
        return ESP_OK;
    }

    const RgbAnimation::Step& step = _animation.steps[_stepIndex];
    uint32_t elapsed = nowMs - _stepStartMs;
    bool stepExpired = step.durationMs > 0 && elapsed >= step.durationMs;

    if (!stepExpired) {
        return applyEffect(step.effect, _driver, elapsed);
    }

    _stepIndex++;
    if (_stepIndex < _animation.stepsCount) {
        return startCurrentStep(nowMs);
    }

    _stepIndex = 0;
    if (_animation.loops == 0) {
        return startCurrentStep(nowMs);
    }

    _loopCount++;
    if (_loopCount < _animation.loops) {
        return startCurrentStep(nowMs);
    }

    ESP_LOGI(TAG, "Finished animation '%s'", _animation.name);
    _isDone = true;
    return _driver.off();
}

esp_err_t RgbPlayer::stop() {
    ESP_LOGI(TAG, "Stopped animation '%s'", _animation.name);
    _isDone = true;
    return _driver.off();
}

bool RgbPlayer::isDone() const {
    return _isDone;
}

esp_err_t RgbPlayer::startCurrentStep(uint32_t nowMs) {
    _stepStartMs = nowMs;
    const RgbAnimation::Step& step = _animation.steps[_stepIndex];
    ESP_LOGI(TAG,
             "Playing animation '%s' loop %d/%d%s step %d/%d duration %lu%s",
             _animation.name,
             _loopCount + 1,
             _animation.loops,
             _animation.loops == 0 ? "(infinite)" : "",
             _stepIndex + 1,
             _animation.stepsCount,
             step.durationMs,
             step.durationMs == 0 ? "(infinite)" : "ms");
    return applyEffect(step.effect, _driver, 0);
}
