#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "DistanceProxy.h"
#include "IndicatorsProxy.h"
#include "MotionProxy.h"
#include "WiFiProxy.h"

struct RobotContext {
    IndicatorsProxy indicators;
    DistanceProxy distance;
    MotionProxy motion;
    WiFiProxy wifiProxy;

    RobotContext(QueueHandle_t indicatorCommandQueue,
                 DistanceTask& distanceTask,
                 QueueHandle_t motionCommnadQueue,
                 WiFiProxy& wifi)
        : indicators(indicatorCommandQueue), distance(&distanceTask), motion(motionCommnadQueue), wifiProxy(wifi) {
    }
};
