#include "f3/console.hpp"
#include "rcc.hpp"

namespace CANMonitor {

void InitConsole() {
  stm32f3::console::Init<CANMonitor::BaremetalRCC, 921600>();
}

extern "C" int _write(int, char* ptr, int len) {
  stm32f3::console::SerialPeripheral::Write(ptr, len);
  return len;
}

}  // namespace CANMonitor