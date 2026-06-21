#pragma once

#include <cstdarg>
#include <cstddef>

#include "LogEntry.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace LogSink {

void install(QueueHandle_t queue, const char* const* filters);

void uninstall();

bool isInstalled();

LogEntry::Level parseLevel(const char* format);

void parseTag(const char* format, char* tagOut, size_t tagLen);

}  // namespace LogSink
