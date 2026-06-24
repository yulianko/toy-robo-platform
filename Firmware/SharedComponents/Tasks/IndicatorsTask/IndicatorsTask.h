#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "BaseTask.h"
#include "IndicatorCommand.h"
#include "RobotEvent.h"
#include "RgbPlayer.h"
#include "SoundPlayer.h"

class IndicatorsTask : public BaseTask {
  public:
    static IndicatorsTask& instance() {
        static IndicatorsTask instance;
        return instance;
    }

    void init(RgbPlayer& rgbPlayer, SoundPlayer& soundPlayer, QueueHandle_t commandQueue, QueueHandle_t robotEventQueue);

  protected:
    const char* getTaskName() const override { return "IndicatorsTask"; }
    uint32_t getStackSize() const override { return 4096; }
    bool isInitialized() const override { return _initialized; }
    void run() override;

  private:
    IndicatorsTask() = default;
    
    void handleCommand(const IndicatorCommand& command, uint32_t now);
    void checkDone();
    
    RgbPlayer* _rgbPlayer = nullptr;
    SoundPlayer* _soundPlayer = nullptr;
    QueueHandle_t _commandQueue = nullptr;
    QueueHandle_t _robotEventQueue = nullptr;
    bool _initialized = false;
    bool _shouldNotifyDone = false;
};