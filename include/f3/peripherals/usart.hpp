#pragma once

#include <cstddef>
#include <cstdint>

#include <concepts>
#include <cstdlib>
#include <utility>

#include "f3/peripherals/rcc.hpp"
#include "stm32f303x8.h"

#include <f3/ram_vector.hpp>

namespace stm32f3 {
template <typename T>
concept USARTHandler = requires(T t) {
  { t.HandleRx(std::declval<char>()) } -> std::same_as<void>;
};

template <int kPeripheralId, USARTHandler Handlers>
class USART {
  static constexpr uintptr_t usart = kPeripheralId == 1   ? USART1_BASE
                                     : kPeripheralId == 2 ? USART2_BASE
                                     : kPeripheralId == 3 ? USART3_BASE
                                                          : USART1_BASE;

  static constexpr auto const usart_clk_en =
      kPeripheralId == 1   ? RCC_APB2ENR_USART1EN
      : kPeripheralId == 2 ? RCC_APB1ENR_USART2EN
      : kPeripheralId == 3 ? RCC_APB1ENR_USART3EN
                           : RCC_APB2ENR_USART1EN;

  static constexpr auto usart_clk_reg_number =  //
      kPeripheralId == 1   ? 2
      : kPeripheralId == 2 ? 1
      : kPeripheralId == 3 ? 1
                           : -1;

  static constexpr auto IRQn =  //
      kPeripheralId == 1   ? USART1_IRQn
      : kPeripheralId == 2 ? USART2_IRQn
      : kPeripheralId == 3 ? USART3_IRQn
                           : UsageFault_IRQn;
  static_assert(IRQn != UsageFault_IRQn, "Invalid peripheral id");

  static void Rx_IRQHandler() {
    if (Instance()->ISR & USART_ISR_RXNE) {
      Handlers::HandleRx(Instance()->RDR);
    }
  }

  static auto RCCClockRegister() {
    if (usart_clk_reg_number == 1) {
      return &RCC->APB1ENR;
    } else if (usart_clk_reg_number == 2) {
      return &RCC->APB2ENR;
    }
  }

 public:
  static USART_TypeDef* Instance() {
    return reinterpret_cast<USART_TypeDef*>(usart);
  }

  static void ConfigureSwap() { Instance()->CR2 |= USART_CR2_SWAP; }

  template <int kBaudrate, rcc::RCCConfigLike kRcc, bool force_init = false>
  static void Configure() {
    static constexpr auto kBusClock =
        kPeripheralId == 1   ? kRcc::GetAPB2Clock()
        : kPeripheralId == 2 ? kRcc::GetAPB1Clock()
        : kPeripheralId == 3 ? kRcc::GetAPB1Clock()
                             : kRcc::GetAPB2Clock();

    constexpr float real_baudrate = kBusClock / (int)(kBusClock / kBaudrate);
    constexpr float error_rate = (real_baudrate - kBaudrate) / kBaudrate;
    if (!force_init) {
      static_assert(-0.05 <= error_rate, "Too small error rate");
      static_assert(error_rate <= 0.05, "Too big error rate");
    }

    constexpr int kDiv = kBusClock / kBaudrate;
    static_assert(kDiv < 0x10000, "Too slow baudrate");

    *RCCClockRegister() |= usart_clk_en;
    Instance()->BRR = kDiv;
  }

  static void Start() {
    Instance()->CR1 |= USART_CR1_TE | USART_CR1_RE;
    Instance()->CR1 |= USART_CR1_UE;
  }

  /// @brief Enable half-duplex transfer
  static void EnableHD() {
    static_assert(kPeripheralId == 1, "LIN is supported only on USART1");

    Instance()->CR2 &= ~USART_CR2_LINEN;
    Instance()->CR2 &= ~USART_CR2_CLKEN;

    Instance()->CR3 &= ~USART_CR3_SCEN;
    Instance()->CR3 &= ~USART_CR3_IREN;

    Instance()->CR3 |= USART_CR3_HDSEL;
  }

  static void DisableHD() {
    static_assert(kPeripheralId == 1, "LIN is supported only on USART1");

    Instance()->CR3 &= ~USART_CR3_HDSEL;
  }

  static void DeInit() {
    Instance()->CR1 &= ~USART_CR1_UE;  // disable Driver
  }

  static void EnableRxInterrupt() {
    Instance()->CR1 |= USART_CR1_RXNEIE;

    ram_vector::ram_vector[16 + IRQn] = &USART::Rx_IRQHandler;

    NVIC_EnableIRQ(IRQn);
  }

  static void Write(uint8_t data) {
    while (!(Instance()->ISR & USART_ISR_TXE))
      ;
    Instance()->TDR = data;

    while (!(Instance()->ISR & USART_ISR_TC))
      ;
  }

  template <typename T>
  requires(sizeof(T) == 1) static void Write(T* data, size_t len) {
    while (len) {
      Write(*data++);
      len--;
    }
  }
};
}  // namespace stm32f3