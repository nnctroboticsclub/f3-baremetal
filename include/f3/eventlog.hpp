#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <stm32f3xx.h>

extern "C" void TIM6_DAC1_IRQHandler();

template <size_t kDepth>
class EventLog {
  struct Entry {
    // Event
    uint32_t timestamp;
    char message[0x30];

    // Link
    Entry* next;
  };

  // Circular Buffer
  Entry kEventLogPool[kDepth];
  Entry* kEventLogHead;

 public:
  uint32_t tick = 0;

 public:
  EventLog() {
    for (int i = 0; i < kDepth - 1; i++) {
      auto log = &kEventLogPool[i];
      log->timestamp = 0;
      memset(log->message, 0, sizeof(log->message));
      log->next = &kEventLogPool[i + 1];
    }
    kEventLogPool[kDepth - 1].next = &kEventLogPool[0];

    kEventLogHead = &kEventLogPool[0];
  }

  void LogRaw(const char* line) {
    auto log = kEventLogHead;
    kEventLogHead = kEventLogHead->next;

    log->timestamp = tick;
    for (int i = 0; i < sizeof(log->message) - 1; i++) {
      log->message[i] = line[i];
      if (line[i] == 0)
        break;
    }
  }
  void Log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    static char buffer[0x30];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    LogRaw(buffer);

    va_end(args);
  }

  //* Iterator
  class Iterator {
    Entry* current;
    Entry* base;

   public:
    Iterator(Entry* head, Entry* base) : current(head), base(base) {}

    bool HasNext() { return current->timestamp != 0; }

    Iterator& operator++() {
      current = current->next;
      if (current == base)
        current = nullptr;

      return *this;
    }

    bool operator!=(const Iterator& other) {  //
      return current != other.current;
    }

    Entry* operator*() { return current; }
  };

  Iterator begin() { return Iterator(kEventLogHead, kEventLogHead); }
  Iterator end() { return Iterator(nullptr, kEventLogHead); }
};
