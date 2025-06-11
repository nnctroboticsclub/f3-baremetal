#pragma once

#include <cstdint>

#include <stm32f3xx.h>

namespace stm32f3 {
enum class GPIOMode {
  kInput = 0b00,
  kOutput = 0b01,
  kAF = 0b10,
  kAnalog = 0b11,
};

enum class GPIO_PullMode {
  kNoPull = 0b00,
  kPullUp = 0b01,
  kPullDown = 0b10,
};
template <int kPortId, int kPinId>
class GPIO {
  static constexpr uintptr_t gpio_port = kPortId == 0   ? GPIOA_BASE
                                         : kPortId == 1 ? GPIOB_BASE
                                         : kPortId == 2 ? GPIOC_BASE
                                         : kPortId == 3 ? GPIOD_BASE
                                         : kPortId == 5 ? GPIOF_BASE
                                                        : GPIOA_BASE;

  static constexpr int clk_en = kPortId == 0   ? RCC_AHBENR_GPIOAEN
                                : kPortId == 1 ? RCC_AHBENR_GPIOBEN
                                : kPortId == 2 ? RCC_AHBENR_GPIOCEN
                                : kPortId == 3 ? RCC_AHBENR_GPIODEN
                                : kPortId == 5 ? RCC_AHBENR_GPIOFEN
                                               : RCC_AHBENR_GPIOAEN;
  static constexpr int moder_pos = kPinId * 2;   // Reg: MODER
  static constexpr int pupd_pos = kPinId * 2;    // Reg: PUPDR
  static constexpr int ospeed_pos = kPinId * 2;  // Reg: OSPEEDR
  static constexpr int af_pos =
      kPinId < 8 ? kPinId * 4 : (kPinId - 8) * 4;  // Reg: AFR
  static constexpr int otype_pos = kPinId;         // Reg: OTYPER

 public:
  static GPIO_TypeDef* Port() {
    return reinterpret_cast<GPIO_TypeDef*>(gpio_port);
  }

  static volatile uint32_t* AFRegister() {
    return &Port()->AFR[kPinId < 8 ? 0 : 1];
  }

  static inline void WriteGPIO(bool value) {
    Port()->ODR = (Port()->ODR & ~(1 << kPinId)) | (value << kPinId);
  }

  static inline bool ReadGPIO() { return (Port()->IDR & (1 << kPinId)) != 0; }

  static inline void ToggleGPIO() { Port()->ODR ^= 1 << kPinId; }

  static void Deinit() {
    InitEx<GPIOMode::kAnalog, GPIO_PullMode::kNoPull, 0, 0>();
    RCC->AHBENR &= ~clk_en;
  }

  template <GPIO_PullMode pupd>
  static void InitPullMode() {
    Port()->PUPDR &= ~(0b11 << pupd_pos);
    Port()->PUPDR |= static_cast<int>(pupd) << pupd_pos;
  }

  template <GPIOMode mode, GPIO_PullMode pupd, int kSpeed, int kFunction>
  static inline void InitEx() {
    RCC->AHBENR |= clk_en;

    Port()->MODER &= ~(0b11 << moder_pos);
    Port()->MODER |= static_cast<int>(mode) << moder_pos;

    *AFRegister() &= ~(kFunction << af_pos);
    *AFRegister() |= kFunction << af_pos;

    InitPullMode<pupd>();

    Port()->OSPEEDR &= ~(0b11 << ospeed_pos);
    Port()->OSPEEDR |= kSpeed << ospeed_pos;
  }

  static inline void InitAsGPIO() {
    InitEx<GPIOMode::kOutput, GPIO_PullMode::kNoPull, 0b11, 0>();
  }

  static inline void InitAsGPIOIn() {
    InitEx<GPIOMode::kInput, GPIO_PullMode::kNoPull, 0b11, 0>();
  }

  template <int kAF>
  static inline void InitAsAF() {
    InitEx<GPIOMode::kAF, GPIO_PullMode::kPullUp, 0b11, kAF>();
  }

  static void Output_OpenDrain() { Port()->OTYPER |= 1 << otype_pos; }
};

template <typename T>
concept GPIOLike = requires {
  T::Port();
  T::AFRegister();
  T::template InitAsAF<0>();
};
}  // namespace stm32f3