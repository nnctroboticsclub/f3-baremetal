#pragma once

#include <stm32f3xx.h>
#include <array>
#include <cstdio>

namespace stm32f3::ram_vector {
using HandlerType = void (*)(void);
static_assert(sizeof(HandlerType) == sizeof(void*));

extern std::array<HandlerType, 0x200 / 4> ram_vector;
static_assert(sizeof(ram_vector) == sizeof(void*) * 0x200 / 4);

[[noreturn]] void DefaultHandler();
extern "C" void InitVector();

}  // namespace stm32f3::ram_vector