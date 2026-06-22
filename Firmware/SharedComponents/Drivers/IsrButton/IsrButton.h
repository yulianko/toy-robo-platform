#pragma once

#include <driver/gpio.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/timers.h>

class IsrButton {
  public:
    enum class Action : uint8_t {
        SHORT_PRESSED,
        LONG_PRESSED,
    };

    struct ButtonEvent {
        char subject[16] = "button_event";
        Action action;
    };

    struct Config {
        gpio_num_t pin;
        char subject[16] = "button_event";
        int64_t debounceUs = 150'000;
        int64_t longPressUs = 1'000'000;
    };

    IsrButton(const Config& config);
    esp_err_t init(QueueHandle_t eventQueue);

  private:
    static void IRAM_ATTR isrHandler(void* arg);

    static void debounceTimerCb(TimerHandle_t timer);

    Config _config{};
    QueueHandle_t _queue{nullptr};

    TimerHandle_t _debounceTimer{nullptr};

    volatile int64_t _pressStartUs{0};
    volatile bool _pressed{false};
};
