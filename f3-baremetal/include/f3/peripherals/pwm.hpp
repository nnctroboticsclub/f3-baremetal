#pragma once

#include <stm32f3xx_hal.h>
#include <cstdio>
#include <type_traits>
#include "stm32f3xx_hal_tim.h"
#include "stm32f3xx_hal_tim_ex.h"

namespace stm32 {
template <int P>
struct HW_Table : std::false_type {};

template <>
struct HW_Table<1> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM1_CLK_ENABLE(); }
  static inline auto Instance() { return TIM1; }
};
template <>
struct HW_Table<2> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM2_CLK_ENABLE(); }
  static inline auto Instance() { return TIM2; }
};
template <>
struct HW_Table<3> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM3_CLK_ENABLE(); }
  static inline auto Instance() { return TIM3; }
};
template <>
struct HW_Table<6> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM6_CLK_ENABLE(); }
  static inline auto Instance() { return TIM6; }
};
template <>
struct HW_Table<7> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM7_CLK_ENABLE(); }
  static inline auto Instance() { return TIM7; }
};
template <>
struct HW_Table<15> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM15_CLK_ENABLE(); }
  static inline auto Instance() { return TIM15; }
};
template <>
struct HW_Table<16> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM16_CLK_ENABLE(); }
  static inline auto Instance() { return TIM16; }
};
template <>
struct HW_Table<17> : std::true_type {
  static inline auto EnClock() { __HAL_RCC_TIM17_CLK_ENABLE(); }
  static inline auto Instance() { return TIM17; }
};

consteval int ChannelToMask(int ch) {
  if (ch == 1)
    return TIM_CHANNEL_1;
  if (ch == 2)
    return TIM_CHANNEL_2;
  if (ch == 3)
    return TIM_CHANNEL_3;
  if (ch == 4)
    return TIM_CHANNEL_4;

  return *reinterpret_cast<int*>(0);
}

template <int kTimerPeripheral, int kCh, bool kNegativePort = false>
class PWM {
  static const int kChannel = ChannelToMask(kCh);

  using HW = HW_Table<kTimerPeripheral>;
  static_assert(HW::value, "Invalid timer peripheral");

  static inline TIM_HandleTypeDef htim;
  static inline int period = 1000;

 public:
  static int Init() {
    HW::EnClock();

    htim.Instance = HW::Instance();
    htim.Init.Prescaler = 0;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = period;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
      return -1;
    }

    TIM_ClockConfigTypeDef sClockSourceConfig = {
        .ClockSource = TIM_CLOCKSOURCE_INTERNAL,
        .ClockPolarity = TIM_CLOCKPOLARITY_RISING,
        .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
        .ClockFilter = 0,
    };
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK) {
      return -2;
    }

    if (HAL_TIM_PWM_Init(&htim) != HAL_OK) {
      return -3;
    }

    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
        .MasterOutputTrigger2 = TIM_TRGO2_RESET,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE,
    };
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) !=
        HAL_OK) {
      return -4;
    }

    TIM_OC_InitTypeDef sConfigOC = {
        .OCMode = TIM_OCMODE_PWM2,
        .Pulse = 0,
        .OCPolarity = TIM_OCPOLARITY_HIGH,
        .OCNPolarity = TIM_OCNPOLARITY_HIGH,
        .OCFastMode = TIM_OCFAST_DISABLE,
        .OCIdleState = TIM_OCIDLESTATE_RESET,
        .OCNIdleState = TIM_OCNIDLESTATE_SET,
    };
    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, kChannel) != HAL_OK) {
      return -5;
    }

    if (HAL_TIM_PWM_Start(&htim, kChannel) != HAL_OK) {
      return -7;
    }
    if (kNegativePort && HAL_TIMEx_PWMN_Start(&htim, kChannel) != HAL_OK) {
      return -7;
    }

    return 0;
  }

  static void Write(float duty) {
    if (duty < 0) {
      duty = 0;
    }
    if (duty > 1) {
      duty = 1;
    }
    const int width = (1 - duty) * period;

    __HAL_TIM_SET_COMPARE(&htim, kChannel, width);
  }

  static void Period(int const count) {
    if (count < 1 || count > 0xFFFF) {
      return;
    }

    period = count;
    htim.Init.Period = period;
  }
};
}  // namespace stm32