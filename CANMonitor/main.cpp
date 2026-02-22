#include "can.hpp"
#include "can_debug.hpp"
#include "can_debug_seq.hpp"
#include "event_log.hpp"
#include "rcc.hpp"

using App = CanDebug;
// using App = CANMonitor::CANDebug_Seq;

int main() {
  stm32::InitRCC();
  CANMonitor::InitConsole();
  CANMonitor::InitCAN();

  CANMonitor::kEventLog.Log("Is RCC Initialized?: %d",
                            CANMonitor::rcc_initialized);

  App app;
  app.Main();
}