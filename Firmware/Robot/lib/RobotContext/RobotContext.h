#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "IndicatorsProxy.h"

struct RobotContext {
    IndicatorsProxy indicators;

    RobotContext(QueueHandle_t indicatorCommandQueue) : indicators(indicatorCommandQueue) {
    }
};
