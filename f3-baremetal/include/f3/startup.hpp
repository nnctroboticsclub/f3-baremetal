#pragma once

namespace stm32::startup {

bool ShouldStartBootloader();
void SetBootloaderFlag();
void ClearBootloaderFlag();

}  // namespace stm32::startup