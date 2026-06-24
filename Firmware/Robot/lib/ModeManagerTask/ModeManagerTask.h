#pragma once

#include <cstddef>

#include "BaseTask.h"
#include "IMode.h"
#include "RobotContext.h"
#include "RobotEvent.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class ModeManagerTask : public BaseTask {
  public:
    static constexpr size_t MAX_MODES = 8;
    static constexpr size_t QUEUE_LENGTH = 16;

    static ModeManagerTask& instance();

    void init(RobotContext& ctx, QueueHandle_t robotEventQueue);
    void addMode(IMode* mode);

  protected:
    const char* getTaskName() const override {
        return "ModeManagerTask";
    }
    bool isInitialized() const override {
        return _ctx && _queue && _modeCount > 0;
    }
    void run() override;

  private:
    static constexpr RgbColor MENU_COLORS[] = {
        RgbColor::yellow(),
        RgbColor::magenta(),
        RgbColor::cyan(),
        RgbColor::orange(),
        RgbColor::red(),
        RgbColor::green(),
        RgbColor::blue(),
    };
    static constexpr size_t MENU_COLORS_COUNT = sizeof(MENU_COLORS) / sizeof(MENU_COLORS[0]);

    enum State : uint8_t { ENTERING_MENU, CHOOSING, RUNNING };

    ModeManagerTask() = default;
    ModeManagerTask(const ModeManagerTask&) = delete;
    ModeManagerTask& operator=(const ModeManagerTask&) = delete;

    void handleEnteringMenu(const RobotEvent& event);
    void handleChoosing(const RobotEvent& event);
    void handleRunning(const RobotEvent& event);
    void activateMode(size_t index);
    void returnToMenu();
    void showCurrentMode();
    void startMenuEntryAnimation();

    IMode* _modes[MAX_MODES]{};
    size_t _modeCount = 0;
    size_t _selectedIdx = 0;
    size_t _activeIdx = 0;
    State _state = State::ENTERING_MENU;

    RobotContext* _ctx = nullptr;
    QueueHandle_t _queue = nullptr;
};
