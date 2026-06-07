#include "IsrButton.h"

#include <esp_check.h>
#include <esp_log.h>

static const char* TAG = "IsrButton";

IsrButton::IsrButton(const Config& config) : _config(config) {
}

esp_err_t IsrButton::init(QueueHandle_t eventQueue) {
    _queue = eventQueue;
    gpio_config_t gpio = {
        .pin_bit_mask = (1ULL << _config.pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        // ANYEDGE so we catch both press (falling) and release (rising)
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    ESP_RETURN_ON_ERROR(gpio_config(&gpio), TAG, "gpio_config failed");

    // Install ISR service once
    // ESP_ERR_INVALID_STATE which means "already installed"
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "gpio_install_isr_service failed: %d", err);
        return err;
    }

    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(_config.pin, isrHandler, this), TAG, "gpio_isr_handler_add failed");

    // Debounce timer
    TickType_t debounceTicks = pdMS_TO_TICKS(_config.debounceUs / 1000);
    if (debounceTicks == 0) {
        debounceTicks = 1;
    }

    _debounceTimer = xTimerCreate("btn_debounce",
                                  debounceTicks,
                                  pdFALSE,  // one-shot
                                  this,
                                  debounceTimerCb);
    if (!_debounceTimer) {
        ESP_LOGE(TAG, "Failed to create debounce timer");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG,
             "init: GPIO %d, debounce %lld µs, long-press %lld µs",
             _config.pin,
             _config.debounceUs,
             _config.longPressUs);

    return ESP_OK;
}

void IsrButton::deinit() {
    gpio_isr_handler_remove(_config.pin);
    if (_debounceTimer) {
        xTimerDelete(_debounceTimer, 0);
        _debounceTimer = nullptr;
    }
}

// ---- Private methods ----

void IRAM_ATTR IsrButton::isrHandler(void* arg) {
    IsrButton* self = static_cast<IsrButton*>(arg);
    BaseType_t woken = pdFALSE;
    // Sets this to pdTRUE if resetting the timer unblocked a higher-priority task
    xTimerResetFromISR(self->_debounceTimer, &woken);
    // if woken is true, this triggers an immediate context switch to that higher-priority task right as the ISR exits,
    // instead of returning to whatever was interrupted.
    // If false, execution just resumes normally.
    // Without this, the timer daemon would only get CPU time on the next scheduler tick,
    portYIELD_FROM_ISR(woken);
}

void IsrButton::debounceTimerCb(TimerHandle_t timer) {
    IsrButton* self = static_cast<IsrButton*>(pvTimerGetTimerID(timer));
    bool nowPressed = (gpio_get_level(self->_config.pin) == 0);

    if (nowPressed && !self->_pressed) {
        self->_pressed = true;
        self->_pressStartUs = esp_timer_get_time();
    } else {
        if (!nowPressed && self->_pressed) {
            self->_pressed = false;
            int64_t duration = esp_timer_get_time() - self->_pressStartUs;

            ButtonEvent ev{};
            strcpy(ev.subject, self->_config.subject);
            ev.action = duration >= self->_config.longPressUs ? Action::LONG_PRESS : Action::CLICK;

            xQueueSend(self->_queue, &ev, 0);
        }
    }
}
