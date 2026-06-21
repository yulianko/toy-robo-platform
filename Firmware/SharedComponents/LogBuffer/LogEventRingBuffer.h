#pragma once

#include <cstddef>
#include <cstring>

#include "LogEntry.h"

template <size_t N>
class LogEventRingBuffer {
  public:
    void push(const LogEntry& entry) {
        _buf[_head] = entry;
        _head = (_head + 1) % N;
        if (_count < N) {
            _count++;
        }
    }

    template <typename Fn>
    void forEach(Fn fn) const {
        if (_count == 0) {
            return;
        }

        size_t start = (_count < N) ? 0 : _head;  // oldest slot
        for (size_t i = 0; i < _count; i++) {
            if (!fn(_buf[(start + i) % N], i)) {
                return;  // stopped early by caller
            }
        }

        return;
    }

    size_t count() const {
        return _count;
    }
    void clear() {
        _head = 0;
        _count = 0;
    }

  private:
    LogEntry _buf[N]{};
    size_t _head = 0;
    size_t _count = 0;
};
