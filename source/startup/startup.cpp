#include <stm32f3xx_hal.h>

#include <f3/peripherals/rcc.hpp>
#include <f3/ram_vector.hpp>

#include "exception_handler.hpp"
#include "stack_clear.hpp"

//* Boundary symbols
extern "C" uint32_t _sidata, _sdata, _edata;
extern "C" uint32_t _sbss, _ebss;
extern "C" char _estack;

extern "C" int main();
extern "C" void __libc_init_array();
extern "C" void Reset_Handler();

extern "C" __attribute__((weak))
uint32_t SystemCoreClock;  // for HAL compatibility
uint32_t SystemCoreClock;

namespace stm32 {
extern "C" void InitRCC();

std::array<stm32f3::ram_vector::HandlerType, 0x200 / 4> flash_vector
    __attribute__((section(".isr_vector"))) = {
        reinterpret_cast<stm32f3::ram_vector::HandlerType>(&_estack),
        Reset_Handler, 0};
}  // namespace stm32

namespace stm32::startup {
static char kBootloaderFlag[16] __attribute__((section(".noinit")));
constexpr char kBootloaderFlagMagic[] = "BOOTLOADER_FLAG";

bool ShouldStartBootloader() {
  for (size_t i = 0; i < sizeof(kBootloaderFlagMagic); ++i) {
    if (kBootloaderFlag[i] != kBootloaderFlagMagic[i]) {
      return false;
    }
  }
  return true;
}

void SetBootloaderFlag() {
  for (size_t i = 0; i < sizeof(kBootloaderFlagMagic); ++i) {
    kBootloaderFlag[i] = kBootloaderFlagMagic[i];
  }
}

void ClearBootloaderFlag() {
  for (char& i : kBootloaderFlag) {
    i = 0;
  }
}

[[noreturn]] static void StartApp() {
  // Copy the .data section to SRAM
  uint32_t const* pSrc = &_sidata;
  for (uint32_t* pDest = &_sdata; pDest < &_edata;) {
    *pDest++ = *pSrc++;
  }

  // Zero fill the .bss section
  for (uint32_t* pDest = &_sbss; pDest < &_ebss;) {
    *pDest++ = 0;
  }

  //* Initialize MCU core features
  stm32f3::ram_vector::InitVector();
  stm32::InitRCC();

  arm::SetupExceptionHandler();
  arm::ClearStack();

  //* Call static constructors
  __libc_init_array();

  //* Initialize HAL
  HAL_Init();
  stm32f3::ram_vector::ram_vector[16 + SysTick_IRQn] = []() {
    HAL_IncTick();
  };

  main();
}

struct BL_VecT {
  uint32_t msp;
  void (*reset_handler)();
};

[[noreturn]] inline static void StartBootloader() {
  constexpr uint32_t kSystemMemory = 0x1FFFd800;

  auto* vec = reinterpret_cast<BL_VecT*>(kSystemMemory);

  __set_MSP(vec->msp);
  vec->reset_handler();

  __ASM volatile("bkpt 0");  // Should never reach here
}

extern "C" [[noreturn]] void StartUp() {
  if (ShouldStartBootloader()) {
    ClearBootloaderFlag();
    StartBootloader();
  } else {
    StartApp();
  }
}
}  // namespace stm32::startup