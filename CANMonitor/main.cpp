#include <cstddef>
#include <cstdio>

#include <SEGGER_RTT.h>
#include <stm32f303x8.h>

#include <f3/peripherals/pin.hpp>
#include <f3/peripherals/rcc.hpp>

class SeggerRTTConsole {
 public:
  static size_t Write(const char* ptr, int len) {
    return SEGGER_RTT_Write(0, ptr, len);
  }

  static size_t Read(char* ptr, size_t len) {
    return SEGGER_RTT_Read(0, ptr, len);
  }
};

void InitRCCImpl();
size_t WriteImpl(int file, const char* ptr, int len);
size_t ReadImpl(int file, char* ptr, size_t len);

extern "C" void InitRCC() {
  InitRCCImpl();
}
extern "C" size_t _write(int file, const char* ptr, int len) {
  return WriteImpl(file, ptr, len);
}
extern "C" size_t _read(int file, char* ptr, size_t len) {
  return ReadImpl(file, ptr, len);
}

using namespace stm32f3::rcc;

template <typename Dummy>
class System {
  using AppRCC =
      RCCConfig<ClockOrigin{.HSI = 8000000, .HSE = 8000000},
                PLLConfig<PLLSource_HSI_D2, 10>,
                SystemClockConfig<SystemClockSource::kPLL>,
                BusClockConfig<AHBPrescaler::kDiv1, APB1Prescaler::kDiv1,
                               APB2Prescaler::kDiv1>>;
  using Console = SeggerRTTConsole;

  static_assert(AppRCC::GetAPB1Clock() == 40e6);
  static_assert(AppRCC::GetAPB2Clock() == 40e6);
  static_assert(AppRCC::GetAHBClock() == 40e6);

 public:
  friend void InitRCCImpl() { AppRCC::ApplyConfig(); }
  friend size_t WriteImpl(int file, const char* ptr, int len) {
    (void)file;
    return Console::Write(ptr, len);
  }
  friend size_t ReadImpl(int file, char* ptr, size_t len) {
    (void)file;
    return Console::Read(ptr, len);
  }
};

template class System<void>;

int main() {
  using stm32f3::NewPin;
  using stm32f3::PinPullMode;
  using stm32f3::PinSpeed;

  SEGGER_RTT_Init();

  SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;  // Disable D-Cache
  SCB->CPACR |= 0x00F00000;                      // Enable FPU

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  auto pin = NewPin<1, 3>()
                 .Init<PinPullMode::kNoPull, PinSpeed::kSpeed0>()
                 .InitAsOutput<false>();
  pin.Write(true);
  while (true) {}

  return 0;
}