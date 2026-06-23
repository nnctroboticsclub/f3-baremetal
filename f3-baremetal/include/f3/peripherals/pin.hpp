#pragma once

#include <cstdint>

#include <stm32f303x8.h>

namespace stm32f3::pin {
using PinId = int;

enum class PinPullMode : uint8_t {
  kNoPull = 0b00,
  kPullUp = 0b01,
  kPullDown = 0b10,
};

enum class PinSpeed : uint8_t {
  kSpeed0 = 0,
  kSpeed1 = 1,
  kSpeed2 = 2,
  kSpeed3 = 3,
};

enum class GPIOMode : uint8_t {
  kInput = 0b00,
  kOutput = 0b01,
  kAF = 0b10,
  kAnalog = 0b11,
};

template <int kPinNum>
struct PinInfo {
  static constexpr int kPortId = kPinNum / 16;
  static constexpr int kPinId = kPinNum % 16;

  static constexpr uintptr_t kGPIOPort = kPortId == 0   ? GPIOA_BASE
                                         : kPortId == 1 ? GPIOB_BASE
                                         : kPortId == 2 ? GPIOC_BASE
                                         : kPortId == 3 ? GPIOD_BASE
                                         : kPortId == 5 ? GPIOF_BASE
                                                        : GPIOA_BASE;
  static constexpr uint32_t kClockEn = kPortId == 0   ? RCC_AHBENR_GPIOAEN
                                       : kPortId == 1 ? RCC_AHBENR_GPIOBEN
                                       : kPortId == 2 ? RCC_AHBENR_GPIOCEN
                                       : kPortId == 3 ? RCC_AHBENR_GPIODEN
                                       : kPortId == 5 ? RCC_AHBENR_GPIOFEN
                                                      : RCC_AHBENR_GPIOAEN;
  static constexpr int kModeRPos = kPinId * 2;   // Reg: MODER
  static constexpr int kPUPDPos = kPinId * 2;    // Reg: PUPDR
  static constexpr int kOspeedPos = kPinId * 2;  // Reg: OSPEEDR
  static constexpr int kOtypePos = kPinId;       // Reg: OTYPER
  static constexpr int kAFPos =
      kPinId < 8 ? kPinId * 4 : (kPinId - 8) * 4;  // Reg: AFR
  static constexpr int kDatPos = kPinId;

  static GPIO_TypeDef* Port() {
    return reinterpret_cast<GPIO_TypeDef*>(kGPIOPort);
  }

  static volatile uint32_t* AFRegister() {
    return &Port()->AFR[kPinId < 8 ? 0 : 1];
  }
};

template <PinId kPinId>
struct PinIn;

template <PinId kPinId>
struct PinOut;

template <PinId kPinId>
struct PinInit;

template <PinId kPinId>
struct PinAF;

template <PinId kPinId>
struct PinAnalog;

template <PinId kPinId>
struct PinUninit {
  template <PinPullMode kPullMode, PinSpeed kSpeed>
  PinInit<kPinId> Init() {
    using PinInfo = PinInfo<kPinId>;
    RCC->AHBENR |= PinInfo::kClockEn;

    PinInfo::Port()->PUPDR &= ~(0b11 << PinInfo::kPUPDPos);
    PinInfo::Port()->PUPDR |= static_cast<int>(kPullMode) << PinInfo::kPUPDPos;

    PinInfo::Port()->OSPEEDR &= ~(0b11 << PinInfo::kOspeedPos);
    PinInfo::Port()->OSPEEDR |= static_cast<int>(kSpeed) << PinInfo::kOspeedPos;

    return PinInit<kPinId>{};
  }
};

template <PinId kPinId>
struct PinInit {
  PinIn<kPinId> InitAsInput() {
    using PinInfo = PinInfo<kPinId>;
    PinInfo::Port()->MODER &= ~(0b11 << PinInfo::kModeRPos);
    PinInfo::Port()->MODER |= static_cast<int>(GPIOMode::kInput)
                              << PinInfo::kModeRPos;
    return PinIn<kPinId>{};
  }

  template <bool kOpenDrain>
  PinOut<kPinId> InitAsOutput() {
    using PinInfo = PinInfo<kPinId>;
    PinInfo::Port()->MODER =
        (PinInfo::Port()->MODER & ~(0b11 << PinInfo::kModeRPos)) |
        (static_cast<int>(GPIOMode::kOutput) << PinInfo::kModeRPos);
    PinInfo::Port()->OTYPER =
        (PinInfo::Port()->OTYPER & (1 << PinInfo::kOtypePos)) |
        kOpenDrain << PinInfo::kOtypePos;
    return PinOut<kPinId>{};
  }

  template <int kAFNumber>
  PinAF<kPinId> InitAsAF() {
    using PinInfo = PinInfo<kPinId>;
    *PinInfo::AFRegister() &= ~(kAFNumber << PinInfo::kAFPos);
    *PinInfo::AFRegister() |= kAFNumber << PinInfo::kAFPos;
    PinInfo::Port()->MODER &= ~(0b11 << PinInfo::kModeRPos);
    PinInfo::Port()->MODER |= static_cast<int>(GPIOMode::kAF)
                              << PinInfo::kModeRPos;
    return PinAF<kPinId>{};
  }

  template <int kAFNumber>
  PinAnalog<kPinId> InitAsAnalog() {
    using PinInfo = PinInfo<kPinId>;
    PinInfo::Port()->MODER &= ~(0b11 << PinInfo::kModeRPos);
    PinInfo::Port()->MODER |= static_cast<int>(GPIOMode::kAnalog)
                              << PinInfo::kModeRPos;
    return PinAnalog<kPinId>{};
  }
};

template <PinId kPinId>
struct PinIn {
  bool Read() {
    using PinInfo = PinInfo<kPinId>;
    return (PinInfo::Port()->IDR & (1 << PinInfo::kDatPos)) != 0;
  }
};

template <PinId kPinId>
struct PinOut {
  bool Read() {
    using PinInfo = PinInfo<kPinId>;
    return (PinInfo::Port()->IDR & (1 << PinInfo::kDatPos)) != 0;
  }
  void Write(bool value) {
    using PinInfo = PinInfo<kPinId>;
    PinInfo::Port()->ODR = (PinInfo::Port()->ODR & ~(1 << PinInfo::kDatPos)) |
                           (value << PinInfo::kDatPos);
  }
  void Toggle() {
    using PinInfo = PinInfo<kPinId>;
    PinInfo::Port()->ODR ^= 1 << PinInfo::kDatPos;
  }
};

template <PinId kPinId>
PinUninit<kPinId> NewPin() {
  return PinUninit<kPinId>{};
}
template <int kPortId, int kPinIdx, int kPinId = 16 * kPortId + kPinIdx>
PinUninit<kPinId> NewPin() {
  return PinUninit<kPinId>{};
}

}  // namespace stm32f3::pin

namespace stm32f3 {
using pin::NewPin;
using pin::PinPullMode;
using pin::PinSpeed;
}  // namespace stm32f3
