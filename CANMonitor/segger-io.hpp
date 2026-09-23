#pragma once

#include <SEGGER_RTT.h>

class SeggerRTTConsole {
 public:
  static void Init() { SEGGER_RTT_Init(); }

  static size_t Write(const char* ptr, int len) {
    return SEGGER_RTT_Write(0, ptr, len);
  }

  static size_t Read(char* ptr, size_t len) {
    return SEGGER_RTT_Read(0, ptr, len);
  }
};
