#pragma once

#include <f3/peripherals/gpio.hpp>
#include <f3/peripherals/rcc.hpp>
#include <f3/peripherals/usart.hpp>

#include <Nano/queue.hpp>

namespace stm32f3::details::Console {
template <typename T>
concept ConsoleConfig = requires {
  stm32f3::rcc::RCCConfigLike<typename T::RCCConfig>;

  stm32f3::GPIOLike<typename T::ConsoleTx>;
  stm32f3::GPIOLike<typename T::ConsoleRx>;
  {T::kConsoleBaudrate}->std::convertible_to<uint32_t>;
  {T::kConsoleUARTAltFn}->std::convertible_to<uint32_t>;
  {T::kConsoleUARTId}->std::convertible_to<uint32_t>;
  {T::kConsoleRxBufSize}->std::convertible_to<size_t>;
};

auto write(int /*fd*/, char* ptr, int len) -> int;
auto read(int /*fd*/, char* ptr, int len) -> int;

// NOLINTNEXTLINE
extern "C" auto _write(int fd, char* ptr, int len) -> int {
  return write(fd, ptr, len);
}

// NOLINTNEXTLINE
extern "C" auto _read(int fd, char* ptr, int len) -> int {
  return read(fd, ptr, len);
}

template <size_t kRxBufSize>
struct Handler {
  static void HandleRx(char received_char) { rx_buffer.Push(received_char); }

  // NOLINTNEXTLINE
  static inline Nano::collection::Queue<char, kRxBufSize> rx_buffer{};
};

template <>
struct Handler<0> {
  static void HandleRx(char /*received_char*/) {}
};

template <ConsoleConfig Config>
struct Console {

  using HandlerT = Handler<Config::kConsoleRxBufSize>;

  using UART = stm32f3::USART<Config::kConsoleUARTId, HandlerT>;

  static void Init() {
    Config::ConsoleTx::template InitAsAF<Config::kConsoleUARTAltFn>();
    Config::ConsoleRx::template InitAsAF<Config::kConsoleUARTAltFn>();

    UART::template Configure<Config::kConsoleBaudrate,
                             typename Config::RCCConfig>();
    // UART::EnableRxInterrupt();
    UART::Start();
  }

  friend auto write(int /*fd*/, char* ptr, int len) -> int {
    for (int i = 0; i < len; ++i) {
      UART::Write(ptr[i]);
    }
    return len;
  }

  friend auto read(int /*fd*/, char* ptr, int len) -> int {
    if constexpr (Config::kConsoleRxBufSize == 0) {
      return 0;
    } else {
      for (int i = 0; i < len; ++i) {
        while (HandlerT::rx_buffer.Empty()) {}
        ptr[i] = HandlerT::rx_buffer.Pop();
      }
      return len;
    }
  }
};
}  // namespace stm32f3::details::Console

namespace stm32f3 {
template <typename T>
concept ConsoleConfig = details::Console::ConsoleConfig<T>;

template <ConsoleConfig Config>
using Console = details::Console::Console<Config>;
}  // namespace stm32f3
