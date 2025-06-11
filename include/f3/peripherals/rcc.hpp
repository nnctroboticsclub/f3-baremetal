#pragma once

#include <stdint.h>
#include <concepts>
#include <utility>

#include <stm32f3xx.h>

namespace stm32f3::rcc {
//* Clock Origin
struct ClockOrigin {
  uint32_t HSI;
  uint32_t HSE;
};

//* Clock Source
struct ClockSource {
  uint32_t HSI;
  uint32_t HSE;
  uint32_t PLL;
};

//* PLL Source Config
template <typename T>
concept PLLSourceConfig = requires {
  { T::ApplyConfig() } -> std::same_as<void>;
  { T::Frequency(std::declval<ClockOrigin>()) } -> std::same_as<uint32_t>;
};

template <int kPrediv>
struct PLLSource_HSE {
  static_assert(1 <= kPrediv && kPrediv <= 16, "Invalid HSE Pre-Divider");

  static void ApplyConfig() {
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
    RCC->CFGR2 |= (kPrediv - 1) << RCC_CFGR2_PREDIV_Pos;
  }

  [[nodiscard]] constexpr static uint32_t Frequency(ClockOrigin const origin) {
    return origin.HSE / kPrediv;
  }
};
static_assert(PLLSourceConfig<PLLSource_HSE<1>>);

struct PLLSource_HSI_D2 {
  static void ApplyConfig() { RCC->CFGR &= ~RCC_CFGR_PLLSRC; }

  [[nodiscard]] constexpr static uint32_t Frequency(ClockOrigin const origin) {
    return origin.HSI / 2;
  }
};
static_assert(PLLSourceConfig<PLLSource_HSI_D2>);

//* PLL Config
template <PLLSourceConfig Source, int kMul>
struct PLLConfig {
  static_assert(2 <= kMul && kMul <= 16, "Invalid PLL Multiplier");
  static void ApplyConfig() {
    Source::ApplyConfig();

    RCC->CFGR |= (kMul - 2) << RCC_CFGR_PLLMUL_Pos;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0)
      ;
  }
  [[nodiscard]] constexpr static uint32_t Frequency(ClockOrigin const origin) {
    return Source::Frequency(origin) * kMul;
  }
};

template <typename T>
static constexpr bool PLLConfigLike_v = false;

template <PLLSourceConfig Source, int kMul>
static constexpr bool PLLConfigLike_v<PLLConfig<Source, kMul>> = true;

template <typename T>
concept PLLConfigLike = requires {
  { T::ApplyConfig() } -> std::same_as<void>;
  { T::Frequency(std::declval<ClockOrigin>()) } -> std::same_as<uint32_t>;
}
&&PLLConfigLike_v<T>;

//* System Clock
enum class SystemClockSource {
  kHSI = 0,
  kHSE = 1,
  kPLL = 2,
};

template <SystemClockSource Source>
struct SystemClockConfig {
  static void ApplyConfig() {
    using enum SystemClockSource;
    auto source_code = static_cast<uint32_t>(Source);

    RCC->CFGR &= ~RCC_CFGR_SW;

    RCC->CFGR |= source_code << RCC_CFGR_SW_Pos;

    while ((RCC->CFGR & RCC_CFGR_SWS) != (source_code << RCC_CFGR_SWS_Pos))
      ;
  }

  [[nodiscard]] constexpr static uint32_t Frequency(ClockSource const sources) {
    using enum SystemClockSource;
    switch (Source) {
      case kHSI:
        return sources.HSI;
      case kHSE:
        return sources.HSE;
      case kPLL:
        return sources.PLL;
    }
  }
};

template <typename T>
static constexpr bool SystemClockConfig_v = false;

template <SystemClockSource Source>
static constexpr bool SystemClockConfig_v<SystemClockConfig<Source>> = true;

template <typename T>
concept SystemClockConfigLike = requires {
  { T::ApplyConfig() } -> std::same_as<void>;
}
&&SystemClockConfig_v<T>;

//* Bus Clock
enum class AHBPrescaler {
  kDiv1 = 0b0111,  // 0xxx
  kDiv2 = 0b1000,
  kDiv4 = 0b1001,
  kDiv8 = 0b1010,
  kDiv16 = 0b1011,
  kDiv64 = 0b1100,
  kDiv128 = 0b1101,
  kDiv256 = 0b1110,
  kDiv512 = 0b1111,
};

enum class APB1Prescaler {
  kDiv1 = 0b011,  // 0xx
  kDiv2 = 0b100,
  kDiv4 = 0b101,
  kDiv8 = 0b110,
  kDiv16 = 0b111,
};

enum class APB2Prescaler {
  kDiv1 = 0b011,  // 0xx
  kDiv2 = 0b100,
  kDiv4 = 0b101,
  kDiv8 = 0b110,
  kDiv16 = 0b111,
};

template <AHBPrescaler AHB, APB1Prescaler APB1, APB2Prescaler APB2>
struct BusClockConfig {
  static void ApplyConfig() {
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= static_cast<uint32_t>(AHB) << RCC_CFGR_HPRE_Pos;

    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= static_cast<uint32_t>(APB1) << RCC_CFGR_PPRE1_Pos;

    RCC->CFGR &= ~RCC_CFGR_PPRE2;
    RCC->CFGR |= static_cast<uint32_t>(APB2) << RCC_CFGR_PPRE2_Pos;
  }

  [[nodiscard]] constexpr static float AHB_Ratio() {  // ratio to SYSCLK
    auto code = static_cast<uint32_t>(AHB);
    return 1.0f / (float)(1 << (code - 0b0111));
  }

  [[nodiscard]] constexpr static float APB1_Ratio() {  // ratio to AHB Clock
    auto code = static_cast<uint32_t>(APB1);
    return AHB_Ratio() * 1 / (float)(1 << (code - 0b011));
  }

  [[nodiscard]] constexpr static float APB2_Ratio() {  // ratio to AHB Clock
    auto code = static_cast<uint32_t>(APB2);
    return AHB_Ratio() * 1 / (float)(1 << (code - 0b011));
  }
};

template <typename T>
static constexpr bool BusClockConfig_v = false;

template <AHBPrescaler AHB, APB1Prescaler APB1, APB2Prescaler APB2>
static constexpr bool BusClockConfig_v<BusClockConfig<AHB, APB1, APB2>> = true;

template <typename T>
concept BusClockConfigLike = requires {
  { T::ApplyConfig() } -> std::same_as<void>;
  { T::AHB_Ratio() } -> std::same_as<float>;
  { T::APB1_Ratio() } -> std::same_as<float>;
  { T::APB2_Ratio() } -> std::same_as<float>;
}
&&BusClockConfig_v<T>;

template <ClockOrigin kClockOrigin, PLLConfigLike PLL,
          SystemClockConfigLike SystemClock, BusClockConfigLike BusClock>
struct RCCConfig {
  static inline void ApplyConfig() {  // Flash Configuration
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    PLL::ApplyConfig();
    SystemClock::ApplyConfig();
    BusClock::ApplyConfig();
  }

  constexpr static auto GetClockSource() {
    ClockSource sources{.HSI = kClockOrigin.HSI,
                        .HSE = kClockOrigin.HSE,
                        .PLL = PLL::Frequency(kClockOrigin)};

    return sources;
  }

  static constexpr uint32_t GetSystemClock() {
    return SystemClock::Frequency(GetClockSource());
  }

  static constexpr uint32_t GetAHBClock() {
    return GetSystemClock() * BusClock::AHB_Ratio();
  }

  static constexpr uint32_t GetAPB1Clock() {
    return GetAHBClock() * BusClock::APB1_Ratio();
  }

  static constexpr uint32_t GetAPB2Clock() {
    return GetAHBClock() * BusClock::APB2_Ratio();
  }
};

template <typename T>
concept RCCConfigLike = requires {
  { T::ApplyConfig() } -> std::same_as<void>;
  { T::GetClockSource() } -> std::same_as<ClockSource>;
  { T::GetSystemClock() } -> std::same_as<uint32_t>;
  { T::GetAHBClock() } -> std::same_as<uint32_t>;
  { T::GetAPB1Clock() } -> std::same_as<uint32_t>;
  { T::GetAPB2Clock() } -> std::same_as<uint32_t>;
};

using DefaultConfig =
    RCCConfig<ClockOrigin{.HSI = 8000000, .HSE = 8000000},
              PLLConfig<PLLSource_HSI_D2, 9>,
              SystemClockConfig<SystemClockSource::kPLL>,
              BusClockConfig<AHBPrescaler::kDiv1, APB1Prescaler::kDiv1,
                             APB2Prescaler::kDiv1>>;

}  // namespace stm32f3::rcc