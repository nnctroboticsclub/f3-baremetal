#pragma once

#include <concepts>

#include <stm32f3xx_hal.h>

namespace stm32f3 {
template <typename T>
concept DMALike = requires {
  { T::ChannelBase(1) } -> std::convertible_to<DMA_Channel_TypeDef*>;
};

template <int kDMA>
class DMA {
  static_assert(kDMA == 1, "Invalid DMA number");

 public:
  static void Init() { __HAL_RCC_DMA1_CLK_ENABLE(); }

  constexpr static DMA_Channel_TypeDef* ChannelBase(int channel) {
    return (DMA_Channel_TypeDef*)(DMA1_Channel1_BASE + (channel - 1) * 0x14);
  }
};

static_assert(DMALike<DMA<1>>);
}  // namespace stm32f3