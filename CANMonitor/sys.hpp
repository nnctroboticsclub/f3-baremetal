
#pragma once

#include <cstddef>
#include "f3/peripherals/rcc.hpp"

void InitRCCImpl();
size_t WriteImpl(int file, const char* ptr, int len);
size_t ReadImpl(int file, char* ptr, size_t len);

extern "C" void InitRCC() {
  InitRCCImpl();
}
extern "C" size_t _write(int file, const char* ptr, int len) {
  return WriteImpl(file, ptr, len);
}
extern "C" size_t _read(int file, char* ptr, size_t len) {
  return ReadImpl(file, ptr, len);
}

template <stm32f3::rcc::RCCConfigLike kRCCConfig, typename Console>
class System {
 public:
  void Init() { Console::Init(); }
  friend void InitRCCImpl() { kRCCConfig::ApplyConfig(); }
  friend size_t WriteImpl(int file, const char* ptr, int len) {
    (void)file;
    return Console::Write(ptr, len);
  }
  friend size_t ReadImpl(int file, char* ptr, size_t len) {
    (void)file;
    return Console::Read(ptr, len);
  }
};