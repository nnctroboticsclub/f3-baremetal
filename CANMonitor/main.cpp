#include <cstdio>

#include <SEGGER_RTT.h>
#include <stm32f303x8.h>

#include <f3/peripherals/pin.hpp>
#include <f3/peripherals/rcc.hpp>

#include "segger-io.hpp"
#include "sys.hpp"

using namespace stm32f3::rcc;

template class System<
    RCCConfig<ClockOrigin{.HSI = 8000000, .HSE = 8000000},
              PLLConfig<PLLSource_HSI_D2, 10>,
              SystemClockConfig<SystemClockSource::kPLL>,
              BusClockConfig<AHBPrescaler::kDiv1, APB1Prescaler::kDiv1,
                             APB2Prescaler::kDiv1>>,
    SeggerRTTConsole>;

int main() {
  SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;  // Disable D-Cache
  SCB->CPACR |= 0x00F00000;                      // Enable FPU

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  return 0;
}