#pragma once

#include <cstdint>
#include "cmsis_gcc.h"

namespace stm32f3::bootloader {

struct BL_VecT {
  uint32_t msp;
  void (*reset_handler)();
};

inline void Launch() {
  constexpr uint32_t kBootloader = 0x0800f800;

  auto* vec = reinterpret_cast<BL_VecT*>(kBootloader);

  __set_MSP(vec->msp);
  vec->reset_handler();

  asm volatile("bkpt 0");  // Should never reach here
}
}  // namespace stm32f3::bootloader