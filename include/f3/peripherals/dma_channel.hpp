#pragma once

#include <concepts>

#include <stm32f3xx_hal.h>

#include "dma.hpp"

namespace stm32f3 {

template <typename T>
concept DMAChannelLike = requires {
  T::Init();
  { T::GetHandle() } -> std::convertible_to<DMA_HandleTypeDef*>;
};

template <typename DMA_, int kChannel>
class DMAChannel {
  static inline DMA_HandleTypeDef hdma;

 public:
  static void Init() {
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma.Instance = (DMA_Channel_TypeDef*)DMA_::ChannelBase(kChannel);
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode = DMA_CIRCULAR;
    hdma.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma);
  }

  static DMA_HandleTypeDef* GetHandle() { return &hdma; }
};

static_assert(DMAChannelLike<DMAChannel<DMA<1>, 1>>);

}  // namespace stm32f3