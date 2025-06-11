#pragma once

#include <cstdint>

#include <f3/ram_vector.hpp>
#include "rcc.hpp"
#include "stm32f303x8.h"
#include "stm32f3xx_hal_cortex.h"

namespace stm32f3::basic_timer {
struct BasicTimerConfiguration {
  uint32_t auto_reload_value;
  uint32_t prescaler;
};

consteval BasicTimerConfiguration FindBasicTimerConfig(uint32_t period_us,
                                                       uint32_t clock_freq) {
  //* Symnopsis:
  //   f_clk: Clock Frequency
  //
  //   f_cnt: Count Frequency [/s]
  //   T_cnt: Count Period [s]
  //
  //   T_ovf: Overflow Period [s]
  //   C_arr: Auto Reload Value

  //* Timer Behavior:
  //         ^ Counter
  //         |
  // C_arr - + - - - -*
  //         |       -|
  //         |      - |
  //         |     -  |
  //         |    -   |
  //         |   -    |
  //         |  -     |
  //         | -      |
  //         |-       |
  //       --+--------*---> Time
  //        O| |      |
  //         | |      +- T_ovf
  //         | +- T_cnt

  //* Parameters
  // f_cnt = f_clk / (prescaler + 1)
  // T_cnt = 1 / f_clk
  //       = (prescaler + 1) / f_clk

  // T_ovf = (C_arr + 1) * T_cnt
  //       = (C_arr + 1) * (prescaler + 1) / f_clk

  //                      T_ovf (us) = (C_arr + 1) * (prescaler + 1) / f_clk
  //         T_ovf [us] * f_clk [Hz] = (C_arr + 1) * (prescaler + 1)
  //  1e-6 * T_ovf [ s] * f_clk [Hz] = (C_arr + 1) * (prescaler + 1)

  uint32_t rhs = (uint32_t)(1.0f * period_us * clock_freq * 1e-6);

  if (rhs <= 0xFFFF) {
    return {.auto_reload_value = rhs, .prescaler = 1};
  } else {
    uint32_t prescaler = rhs / 0xFFFF;
    return {.auto_reload_value = rhs / (prescaler + 1),
            .prescaler = prescaler + 1};
  }
}

template <typename T>
concept TimerHandler = requires {
  { T::OnTick() } -> std::same_as<void>;
};

template <int peripheral_id>
class BasicTimer {
  static constexpr auto kIRQn =  //
      peripheral_id == 6   ? TIM6_DAC1_IRQn
      : peripheral_id == 7 ? TIM7_IRQn
                           : UsageFault_IRQn;
  static_assert(kIRQn != UsageFault_IRQn, "Invalid peripheral_id");

  static constexpr auto kClockEnFlag =  //
      peripheral_id == 6   ? RCC_APB1ENR_TIM6EN
      : peripheral_id == 7 ? RCC_APB1ENR_TIM7EN
                           : 0;
  static_assert(kClockEnFlag != 0, "Invalid peripheral_id");

  static TIM_TypeDef* Instance() {
    switch (peripheral_id) {
      case 6:
        return TIM6;
      case 7:
        return TIM7;
      default:
        return nullptr;
    }
  }

 public:
  template <int period_us, rcc::RCCConfigLike RCCConfig>
  static constexpr auto CalculateConfig() {
    return FindBasicTimerConfig(period_us, RCCConfig::GetAPB1Clock());
  }

  template <int period_us, rcc::RCCConfigLike RCCConfig, TimerHandler Handler>
  static void Init() {
    constexpr auto config = CalculateConfig<period_us, RCCConfig>();

    InitEx<Handler>(config.prescaler, config.auto_reload_value);
  }

  template <TimerHandler Handler>
  static void InitEx(int prescaler, int auto_reload_value) {
    NVIC_DisableIRQ(kIRQn);
    RCC->APB1ENR &= ~kClockEnFlag;
    Instance()->CR1 &= ~TIM_CR1_CEN;

    RCC->APB1ENR |= kClockEnFlag;

    Instance()->SR = TIM_CR1_ARPE | TIM_CR1_URS;
    Instance()->DIER |= TIM_DIER_UIE;
    Instance()->PSC = prescaler - 1;
    Instance()->ARR = auto_reload_value - 1;

    stm32f3::ram_vector::ram_vector[16 + kIRQn] = []() {
      Instance()->SR &= ~TIM_SR_UIF;
      Handler::OnTick();
    };

    NVIC_EnableIRQ(kIRQn);
    NVIC_SetPriority(kIRQn, 0);

    Instance()->CR1 |= TIM_CR1_CEN;
  }
};

}  // namespace stm32f3::basic_timer