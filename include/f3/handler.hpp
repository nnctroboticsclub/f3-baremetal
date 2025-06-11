#pragma once

#include <concepts>

namespace stm32f3 {
template <typename T>
concept Handler = requires {
  { T::Handle() } -> std::same_as<void>;
};
}  // namespace stm32f3