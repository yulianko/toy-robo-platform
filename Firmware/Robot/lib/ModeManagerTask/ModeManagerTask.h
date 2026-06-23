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
    const char* getTaskName() const override { return "ModeManagerTask"; }
    bool isInitialized() const override { return _ctx && _queue && _modeCount > 0; }
    void run() override;

  private:
    enum State : uint8_t { CHOOSING, RUNNING };

    ModeManagerTask() = default;
    ModeManagerTask(const ModeManagerTask&) = delete;
    ModeManagerTask& operator=(const ModeManagerTask&) = delete;

    void handleChoosing(const RobotEvent& event);
    void handleRunning(const RobotEvent& event);
    void activateMode(size_t index);
    void returnToMenu();
    void showCurrentMode();

    IMode* _modes[MAX_MODES]{};
    size_t _modeCount = 0;
    size_t _selectedIdx = 0;
    size_t _activeIdx = 0;
    State _state = State::CHOOSING;

    RobotContext* _ctx = nullptr;
    QueueHandle_t _queue = nullptr;
};
