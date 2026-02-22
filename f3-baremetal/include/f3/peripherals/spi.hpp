#pragma once

#include <stm32f3xx_hal.h>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <optional>
#include "Nano/no_mutex_lifo.hpp"
#include "f3/ram_vector.hpp"
#include "stm32f303x8.h"
#include "stm32f3xx_hal_rcc_ex.h"
#include "stm32f3xx_hal_spi.h"

namespace stm32::spi {
enum class SPIMode {
  kMaster,
  kSlave,
};

struct Transfer {
  uint8_t* tx_buffer;
  uint8_t* rx_buffer;
  size_t size;

  std::function<void(Transfer&)> callback = [](auto) {
    // printf("Transfer completed\n");
  };
};

Nano::collection::NoMutexLIFO<Transfer, 4> transfer_queue = {};
std::optional<Transfer> ongoing_transfer = std::nullopt;

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
  // printf("[C]\n");
  auto trans = *ongoing_transfer;
  ongoing_transfer = std::nullopt;
  // printf("[C-D]\n");
  trans.callback(trans);
  // printf("[C-d]\n");

  if (transfer_queue.Empty()) {
    // printf("[C-Eoq]\n");
    return;
  }

  // printf("[C-NE]\n");
  ongoing_transfer = transfer_queue.Pop();
  auto ret = HAL_SPI_Transmit_IT(hspi, ongoing_transfer->tx_buffer,
                                 ongoing_transfer->size);
  if (ret != HAL_OK) {
    // printf("[C-Err]\n");
    ongoing_transfer = std::nullopt;
    return;
  }
  // printf("[C-Fin]\n");
}

template <SPI_HandleTypeDef* hspi>
class ITProcessor {
  static auto DoTransfer(Transfer const& trans) {
    ongoing_transfer = trans;

    // printf("{DT %p -> %p}\n", trans.tx_buffer, trans.rx_buffer);
    auto ret = HAL_SPI_Transmit_IT(hspi, ongoing_transfer->tx_buffer,
                                   ongoing_transfer->size);
    // printf("{DT-D ==> %d}\n", ret);
    if (ret != HAL_OK) {
      return -1;
    }

    return 0;
  }

 public:
  ITProcessor() = default;

  void Init() {
    // printf("[I]\n");
    stm32f3::ram_vector::ram_vector[SPI1_IRQn + 16] = []() {
      HAL_SPI_IRQHandler(hspi);
    };
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
  }

  static int RequestTransfer(Transfer trans) {
    if (ongoing_transfer == std::nullopt) {
      // printf("{RTT}\n");
      return DoTransfer(trans);
    }

    // printf("{RTF}\n");
    if (transfer_queue.Full()) {
      return -1;
    }

    // printf("{RTQ}\n");
    transfer_queue.Push(trans);
    return 0;
  }
  static int RequestTransfer(uint8_t const* tx_buffer, uint8_t* rx_buffer,
                             size_t size) {
    struct Transfer transfer;
    transfer.tx_buffer = const_cast<uint8_t*>(tx_buffer);
    transfer.rx_buffer = rx_buffer;
    transfer.size = size;

    return RequestTransfer(transfer);
  }
};

class SPIBus {
  static inline SPI_HandleTypeDef hspi;

  static inline size_t transfering_bytes;
  static inline uint8_t* tx_buffer;
  static inline uint8_t* rx_buffer;

  static inline ITProcessor<&hspi> it{};

 public:
  static int Init(SPIMode mode) {
    __HAL_RCC_SPI1_CLK_ENABLE();

    hspi.Instance = SPI1;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi.Init.Direction = SPI_DIRECTION_1LINE;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7;
    hspi.Init.CRCLength = SPI_CRC_LENGTH_8BIT;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi.Init.Mode =
        mode == SPIMode::kMaster ? SPI_MODE_MASTER : SPI_MODE_SLAVE;

    if (HAL_SPI_Init(&hspi) != HAL_OK) {
      return -1;
    }

    return 0;
  }

  static int RequestTransfer(uint8_t const* tx_buffer, uint8_t* rx_buffer,
                             size_t size) {
    return it.RequestTransfer(tx_buffer, rx_buffer, size);
  }

  static int RequestTransfer(Transfer trans) {
    return it.RequestTransfer(trans);
  }

  static void EnableInterrupt() { it.Init(); }
};
}  // namespace stm32::spi