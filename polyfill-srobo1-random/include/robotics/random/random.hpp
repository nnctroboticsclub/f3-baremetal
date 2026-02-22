#pragma once

#include <cmath>
#include <cstdint>

namespace robotics::system {
namespace Random {

namespace impl {
void Entropy(uint32_t);
}

void Init();
uint8_t GetByte();
};  // namespace Random
}  // namespace robotics::system
