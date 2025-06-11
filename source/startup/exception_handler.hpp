#pragma once

#include <cstdint>
#include <cstdio>

#include <f3/ram_vector.hpp>
#include <optional>

#include <stm32f3xx.h>

namespace arm::exception_handler {
struct FaultStatus {
  std::optional<uint32_t> bus_fault_address;
  std::optional<uint32_t> mem_fault_address;

  void Show() {
    printf("\x1b[1;31m===== Exception =====\x1b[0K\x1b[m\n");
    if (bus_fault_address) {
      printf("BusFault address: %08x\n", *bus_fault_address);
    }
    if (mem_fault_address) {
      printf("MemFault address: %08x\n", *mem_fault_address);
    }
  }
};

__attribute__((always_inline)) inline void DumpFaultStatus() {
  auto mfar = SCB->MMFAR;
  auto bfar = SCB->BFAR;

  auto mfar_valid = SCB->CFSR & SCB_CFSR_MMARVALID_Msk;
  auto bfar_valid = SCB->CFSR & SCB_CFSR_BFARVALID_Msk;

  auto fault_status = FaultStatus{
      .bus_fault_address =
          bfar_valid ? std::optional<uint32_t>(bfar) : std::nullopt,
      .mem_fault_address =
          mfar_valid ? std::optional<uint32_t>(mfar) : std::nullopt,
  };
  fault_status.Show();
}

bool DiagnoseBUsFault() {
  auto bfsr = SCB->CFSR;
  if (!(bfsr & SCB_CFSR_BUSFAULTSR_Msk)) {
    return false;
  }

  printf("BusFault: ");
  if (bfsr & SCB_CFSR_LSPERR_Msk) {
    printf("Bus fault on floating-point lazy state preservation");
  } else if (bfsr & SCB_CFSR_STKERR_Msk) {
    printf("Bus fault on stacking for exception entry");
  } else if (bfsr & SCB_CFSR_UNSTKERR_Msk) {
    printf("Bus fault on unstacking for a return from exception");
  } else if (bfsr & SCB_CFSR_IMPRECISERR_Msk) {
    printf("Imprecise data bus error");
  } else if (bfsr & SCB_CFSR_PRECISERR_Msk) {
    printf("Precise data bus error");
  } else if (bfsr & SCB_CFSR_IBUSERR_Msk) {
    printf("Instruction bus error");
  }

  printf("\x1b[0K\n");

  return true;
}
bool DiagnoseMemManage() {
  auto mmfsr = SCB->CFSR;
  if (!(mmfsr & SCB_CFSR_USGFAULTSR_Msk)) {
    return false;
  }

  printf("MemoryManagementFault: ");
  if (mmfsr & SCB_CFSR_MLSPERR_Msk) {
    printf("floating-point lazy state preservation");
  } else if (mmfsr & SCB_CFSR_MSTKERR_Msk) {
    printf("stacking for exception entry");
  } else if (mmfsr & SCB_CFSR_MUNSTKERR_Msk) {
    printf("unstacking for a return from exception");
  } else if (mmfsr & SCB_CFSR_DACCVIOL_Msk) {
    printf("Data access violation");
  } else if (mmfsr & SCB_CFSR_IACCVIOL_Msk) {
    printf("Instruction access violation");
  } else {
    printf("Unknown reason (%08x)", mmfsr);
  }

  printf("\x1b[0K\n");

  return true;
}
bool DiagnoseUsageFault() {
  auto ufsr = SCB->CFSR;
  if (!(ufsr & SCB_CFSR_USGFAULTSR_Msk)) {
    return false;
  }

  printf("UsageFault: ");
  if (ufsr & SCB_CFSR_DIVBYZERO_Msk) {
    printf("Divide by zero");
  } else if (ufsr & SCB_CFSR_UNALIGNED_Msk) {
    printf("Unaligned access");
  } else if (ufsr & SCB_CFSR_NOCP_Msk) {
    printf("No coprocessor");
  } else if (ufsr & SCB_CFSR_INVPC_Msk) {
    printf("Invalid PC load");
  } else if (ufsr & SCB_CFSR_INVSTATE_Msk) {
    printf("Invalid state");
  } else if (ufsr & SCB_CFSR_UNDEFINSTR_Msk) {
    printf("Undefined instruction");
  } else {
    printf("Unknown reason (%08x)", ufsr);
  }

  printf("\x1b[0K\n");

  return true;
}

bool DiagnoseHardFault() {
  auto hfsr = SCB->HFSR;
  printf("HardFault (%08x)\x1b[0K\n", hfsr);
  if (hfsr & SCB_HFSR_VECTTBL_Msk) {
    printf("Vector Table HardFault\x1b[0K\n");
    return true;
  }
  if (hfsr & SCB_HFSR_DEBUGEVT_Msk) {
    printf("Debug Event HardFault\x1b[0K\n");
    return true;
  }
  if (hfsr & SCB_HFSR_FORCED_Msk) {
    auto cfsr = SCB->CFSR;
    printf("Forced HardFault (cfsr: %08x)\x1b[0K\n", cfsr);

    if (DiagnoseBUsFault())
      return true;

    if (DiagnoseMemManage())
      return true;

    if (DiagnoseUsageFault())
      return true;

    printf("Unknown reason (%08x)", cfsr);
  }

  return false;
}

[[noreturn]] void HardFault_Handler() {
  DumpFaultStatus();
  DiagnoseHardFault();

  while (true)
    ;
}

[[noreturn]] void BusFault_Handler() {
  DumpFaultStatus();
  DiagnoseBUsFault();

  while (true)
    ;
}

[[noreturn]] void MemManage_Handler() {
  DumpFaultStatus();
  DiagnoseMemManage();

  while (true)
    ;
}

[[noreturn]] void UsageFault_Handler() {
  DumpFaultStatus();
  DiagnoseUsageFault();

  while (true)
    ;
}

void SetupExceptionHandler() {
  stm32f3::ram_vector::ram_vector[16 + HardFault_IRQn] = HardFault_Handler;
  stm32f3::ram_vector::ram_vector[16 + MemoryManagement_IRQn] =
      MemManage_Handler;
  stm32f3::ram_vector::ram_vector[16 + BusFault_IRQn] = BusFault_Handler;
  stm32f3::ram_vector::ram_vector[16 + UsageFault_IRQn] = UsageFault_Handler;

  // Enable fault handlers
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                SCB_SHCSR_USGFAULTENA_Msk;

  // priority
  NVIC_SetPriority(MemoryManagement_IRQn, 0);
  NVIC_SetPriority(BusFault_IRQn, 0);
  NVIC_SetPriority(UsageFault_IRQn, 0);
}

};  // namespace arm::exception_handler

namespace arm {
using arm::exception_handler::SetupExceptionHandler;
}