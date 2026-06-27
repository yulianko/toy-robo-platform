#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "DistanceProxy.h"
#include "IndicatorsProxy.h"
#include "MotionProxy.h"

struct RobotContext {
    IndicatorsProxy indicators;
    DistanceProxy distance;
    MotionProxy motion;

    RobotContext(QueueHandle_t indicatorCommandQueue, DistanceTask& distanceTask, QueueHandle_t motionCommnadQueue)
        : indicators(indicatorCommandQueue), distance(&distanceTask), motion(motionCommnadQueue) {
    }
};
