#pragma once

#include <stm32f303x8.h>

namespace stm32f3 {
template <uint16_t kPolynomial>
class CRC16 {
 public:
  CRC16() {
    CRC->INIT = 0;
    CRC->CR |= ~CRC_CR_REV_OUT_Msk;
    CRC->CR &= ~CRC_CR_REV_IN_Msk;
    CRC->CR |= 0b01U << CRC_CR_REV_IN_Pos;
    CRC->CR &= ~CRC_CR_POLYSIZE_Msk;
    CRC->CR |= 0b01U << CRC_CR_POLYSIZE_Pos;  // 16bit
    CRC->POL = kPolynomial;
    CRC->CR |= CRC_CR_RESET_Msk;
  }

  auto operator<<(uint8_t val) -> CRC16& {
    CRC->DR = val;
    return *this;
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] auto Value() const -> uint16_t { return CRC->DR; }
};
}  // namespace stm32f3