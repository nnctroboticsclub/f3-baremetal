#include <f3/ram_vector.hpp>

namespace stm32f3::ram_vector {
std::array<HandlerType, 0x200 / 4> ram_vector
    __attribute__((section(".ram_vector")));

void DefaultHandler() {
  auto vect_active = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;
  printf("DefaultHandler (active: %ld)", vect_active);
  while (true) {
    asm("nop");
  }
}

extern "C" void InitVector() {
  for (int i = 0; i < 0x200 / 4; i++) {
    ram_vector[i] = DefaultHandler;
  }

#ifdef __EMULATION__
  *(uintptr_t*)0xABCD0000 = (uintptr_t)ram_vector.data();
#else
  SCB->VTOR = (uint32_t)ram_vector.data();

#endif
}
}  // namespace stm32f3::ram_vector