#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "DistanceProxy.h"
#include "IndicatorsProxy.h"

struct RobotContext {
    IndicatorsProxy indicators;
    DistanceProxy distance;

    RobotContext(QueueHandle_t indicatorCommandQueue, DistanceTask& distanceTask)
        : indicators(indicatorCommandQueue), distance(&distanceTask) {
    }
};
