#include <robotics/random/random.hpp>

namespace robotics::system::Random {
static uint32_t rand_value = 0x12345678;

void impl::Entropy(uint32_t value) {
  rand_value = (value + rand_value) * 0x31415926;
}

static uint32_t ReadAndScramble() {
  auto return_value = rand_value;

  // xor-shift 32bit
  rand_value ^= rand_value << 13;
  rand_value ^= rand_value >> 17;
  rand_value ^= rand_value << 5;

  return return_value;
}

void Init() {}

uint8_t GetByte() {
  auto value = ReadAndScramble();

  uint8_t ret = 0;
  ret ^= (value >> 24) & 0xff;
  ret ^= (value >> 16) & 0xff;
  ret ^= (value >> 8) & 0xff;
  ret ^= (value)&0xff;
  return ret;
}
}  // namespace robotics::system::Random