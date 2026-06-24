#include "ModeManagerTask.h"

#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "RgbColor.h"

static const char* TAG = "ModeManagerTask";

ModeManagerTask& ModeManagerTask::instance() {
    static ModeManagerTask inst;
    return inst;
}

void ModeManagerTask::init(RobotContext& ctx, QueueHandle_t queue) {
    _ctx = &ctx;
    _queue = queue;

    ESP_LOGI(TAG, "Initialized");
}

void ModeManagerTask::addMode(IMode* mode) {
    if (_modeCount >= MAX_MODES) {
        ESP_LOGE(TAG, "MAX_MODES (%d) reached, ignoring '%s'", MAX_MODES, mode->name());
        return;
    }

    _modes[_modeCount++] = mode;
    ESP_LOGI(TAG, "Registered mode[%d]: %s", _modeCount - 1, mode->name());
}

void ModeManagerTask::run() {
    startMenuEntryAnimation();
    ESP_LOGI(TAG, "Starting in ENTERING_MENU state");

    RobotEvent event;

    while (true) {
        if (xQueueReceive(_queue, &event, portMAX_DELAY)) {
            switch (_state) {
                case State::ENTERING_MENU:
                    handleEnteringMenu(event);
                    break;
                case State::CHOOSING:
                    handleChoosing(event);
                    break;
                case State::RUNNING:
                    handleRunning(event);
                    break;
            }
        }
    }
}

void ModeManagerTask::handleEnteringMenu(const RobotEvent& event) {
    switch (event.type) {
        case RobotEvent::Type::SYS_BUTTON_SHORT_PRESSED:
        case RobotEvent::Type::INDICATORS_ANIMATION_DONE:
            _state = State::CHOOSING;
            ESP_LOGI(TAG, "Entry animation completed, entering CHOOSING state");
            showCurrentMode();
            break;

        case RobotEvent::Type::SYS_BUTTON_LONG_PRESSED:
            ESP_LOGW(TAG, "[ENTERING_MENU] Sys button long press - esp_restart()");
            esp_restart();
            break;

        default:
            // Ignore all other input during entry animation
            break;
    }
}

void ModeManagerTask::handleChoosing(const RobotEvent& event) {
    switch (event.type) {
        case RobotEvent::Type::SYS_BUTTON_SHORT_PRESSED:
            _selectedIdx = (_selectedIdx + 1) % _modeCount;
            showCurrentMode();
            break;

        case RobotEvent::Type::PUSH_BUTTON_SHORT_PRESSED:
        case RobotEvent::Type::PUSH_BUTTON_LONG_PRESSED:
            activateMode(_selectedIdx);
            break;

        case RobotEvent::Type::SYS_BUTTON_LONG_PRESSED:
            ESP_LOGW(TAG, "[CHOOSING] Sys button long press - esp_restart()");
            esp_restart();
            break;

        default:
            break;
    }
}

void ModeManagerTask::handleRunning(const RobotEvent& event) {
    switch (event.type) {
        case RobotEvent::Type::SYS_BUTTON_SHORT_PRESSED:
            ESP_LOGI(TAG, "[RUNNING] Sys button - returning to menu");
            returnToMenu();
            break;

        case RobotEvent::Type::SYS_BUTTON_LONG_PRESSED:
            ESP_LOGW(TAG, "[RUNNING] Sys button long press - esp_restart()");
            esp_restart();
            break;

        default:
            _modes[_activeIdx]->onEvent(event);
            break;
    }
}

void ModeManagerTask::activateMode(size_t index) {
    if (_modeCount == 0) {
        ESP_LOGW(TAG, "No modes registered");
        return;
    }

    _activeIdx = index;
    _state = State::RUNNING;
    ESP_LOGI(TAG, "[RUNNING] '%s'", _modes[_activeIdx]->name());
    _modes[_activeIdx]->onEnter(*_ctx);
}

void ModeManagerTask::returnToMenu() {
    if (_state != State::RUNNING) {
        return;
    }

    _modes[_activeIdx]->onExit();
    ESP_LOGI(TAG, "Exited '%s', playing menu entry animation", _modes[_activeIdx]->name());

    _state = State::ENTERING_MENU;
    startMenuEntryAnimation();
}

void ModeManagerTask::showCurrentMode() {
    if (_modeCount == 0) {
        ESP_LOGW(TAG, "No modes registered");
        return;
    }

    const RgbColor color = MENU_COLORS[_selectedIdx % MENU_COLORS_COUNT];

    static RgbAnimation::Step step;
    static RgbAnimation animation("menu", &step, 1, 0);

    step = {EffectSolid{color}, 0};
    animation = RgbAnimation(_modes[_selectedIdx]->name(), &step, 1, 0);

    ESP_LOGI(TAG,
             "[MENU] %d/%d: %s (R=%d G=%d B=%d)",
             _selectedIdx + 1,
             _modeCount,
             _modes[_selectedIdx]->name(),
             color.r,
             color.g,
             color.b);

    _ctx->indicators.start(animation);
}

void ModeManagerTask::startMenuEntryAnimation() {
    static RgbAnimation::Step step;
    static RgbAnimation animation("menu_entry", &step, 1, 1);

    step = {EffectPulse{RgbColor::white(), 10800, 0.1f}, 10800}, animation = RgbAnimation("menu_entry", &step, 1, 1);

    ESP_LOGI(TAG, "Playing menu entry animation (white flash 600ms)");
    _ctx->indicators.start(animation);
}
