#pragma once

#include "dma_channel.hpp"

namespace stm32f3 {
template <int peripheral_id, int kChannels, stm32f3::DMAChannelLike DMACh>
class ADC {
  static constexpr auto instance_addr = peripheral_id == 1   ? ADC1_BASE
                                        : peripheral_id == 2 ? ADC2_BASE
                                                             : -1;
  static_assert(instance_addr != -1, "Invalid peripheral_id");

  static auto Instance() -> ADC_TypeDef* {
    return reinterpret_cast<ADC_TypeDef*>(instance_addr);
  }

  static inline ADC_HandleTypeDef hadc;

  static inline uint16_t dma_buffer_[kChannels];

 public:
  static int Init() {
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
      return -2;
    }
    __HAL_RCC_ADC12_CLK_ENABLE();

    hadc.Instance = Instance();
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode = ENABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DMAContinuousRequests = ENABLE;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    if (HAL_ADC_Init(&hadc) != HAL_OK) {
      return -1;
    }

    ADC_MultiModeTypeDef multimode = {0};
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc, &multimode) != HAL_OK) {
      return -2;
    }

    return 0;
  }

  static int InitChannel(int rank, int channel) {
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = rank;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
      return -1;
    }

    return 0;
  }

  static int Start() {
    HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);

    hadc.DMA_Handle = DMACh::GetHandle();
    hadc.DMA_Handle->Parent = &hadc;

    DMACh::GetHandle()->Instance->CCR &= ~(DMA_IT_TC | DMA_IT_HT);

    auto result = HAL_ADC_Start_DMA(&hadc, (uint32_t*)dma_buffer_, kChannels);

    if (result != HAL_OK) {
      return -1;
    }

    return 0;
  }

  static auto GetValue(size_t index) -> uint32_t { return dma_buffer_[index]; }
};

template <typename T>
concept ADCLike = requires {
  T::Init();
  T::InitChannel(0, 0);
  T::Start();
  T::GetValue(0);
};

}  // namespace stm32f3