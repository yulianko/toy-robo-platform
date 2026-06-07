#pragma once

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static void logTaskInfo(const TaskStatus_t& t);

static void heartbeatTask(void* /*arg*/);

void startHeartbeatTask();
