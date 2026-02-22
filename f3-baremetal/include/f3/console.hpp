#pragma once

#include <cstdio>
#include "f3/peripherals/rcc.hpp"
#include "peripherals/gpio.hpp"
#include "peripherals/usart.hpp"

namespace stm32f3::console {
struct Handler {
  static void HandleRx(char) {}
};

using SerialPeripheral = USART<2, Handler>;

template <rcc::RCCConfigLike kRcc, int kBaudRate = (int)2e6>
static inline void Init() {
  GPIO<0, 2>::InitAsAF<7>();
  GPIO<0, 15>::InitAsAF<7>();

  SerialPeripheral::Configure<(int)kBaudRate, kRcc, true>();
  SerialPeripheral::Start();
}

}  // namespace stm32f3::console