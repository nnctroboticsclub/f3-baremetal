```cpp
// g_pfnVectors cannot be const because it is used in assembly code
__attribute__((section(".isr_vector"))) uint32_t* g_pfnVectors[] = {
    (uint32_t*)0x20002000,        // Stack Pointer
    (uint32_t*)&Reset_Handler,    // Reset
    (uint32_t*)&Default_Handler,  // NMI
    (uint32_t*)&Default_Handler,  // HardFault
    (uint32_t*)&Default_Handler,  // MemManage
    (uint32_t*)&Default_Handler,  // BusFault
    (uint32_t*)&Default_Handler,  // UsageFault
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // SVCall
    (uint32_t*)&Default_Handler,  // PendSV
    (uint32_t*)&Default_Handler,  // SysTick
    (uint32_t*)&Default_Handler,  // WWDG
    (uint32_t*)&Default_Handler,  // PVD
    (uint32_t*)&Default_Handler,  // TAMP_STAMP
    (uint32_t*)&Default_Handler,  // RTC_WKUP
    (uint32_t*)&Default_Handler,  // FLASH
    (uint32_t*)&Default_Handler,  // RCC
    (uint32_t*)&Default_Handler,  // EXTI0
    (uint32_t*)&Default_Handler,  // EXTI1
    (uint32_t*)&Default_Handler,  // EXTI2_TS
    (uint32_t*)&Default_Handler,  // EXTI3
    (uint32_t*)&Default_Handler,  // EXTI4
    (uint32_t*)&Default_Handler,  // DMA1_CH1
    (uint32_t*)&Default_Handler,  // DMA1_CH2
    (uint32_t*)&Default_Handler,  // DMA1_CH3
    (uint32_t*)&Default_Handler,  // DMA1_CH4
    (uint32_t*)&Default_Handler,  // DMA1_CH5
    (uint32_t*)&Default_Handler,  // DMA1_CH6
    (uint32_t*)&Default_Handler,  // DMA1_CH7
    (uint32_t*)&Default_Handler,  // ADC1_2
    (uint32_t*)&Default_Handler,  // CAN_TX
    (uint32_t*)&Default_Handler,  // CAN_RX0
    (uint32_t*)&Default_Handler,  // CAN_RX1
    (uint32_t*)&Default_Handler,  // CAN_SCE
    (uint32_t*)&Default_Handler,  // EXTI9_5
    (uint32_t*)&Default_Handler,  // TIM1_BRK_TIM15
    (uint32_t*)&Default_Handler,  // TIM1_UP_TIM16
    (uint32_t*)&Default_Handler,  // TIM1_TRG_COM
    (uint32_t*)&Default_Handler,  // TIM1_CC
    (uint32_t*)&Default_Handler,  // TIM2
    (uint32_t*)&Default_Handler,  // TIM3
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // I2C1_EV
    (uint32_t*)&Default_Handler,  // I2C1_ER
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // SPI1
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // USART1
    (uint32_t*)&Default_Handler,  // USART2
    (uint32_t*)&Default_Handler,  // USART3
    (uint32_t*)&Default_Handler,  // EXTI15_10
    (uint32_t*)&Default_Handler,  // RTC_Alarm
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // TIM6_DAC1
    (uint32_t*)&Default_Handler,  // TIM7_DAC2
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // COMP2
    (uint32_t*)&Default_Handler,  // COMP4_6
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    nullptr,                      // Reserved
    (uint32_t*)&Default_Handler,  // FPU
};

// Cortex Processor have 11 exception handlers
// STM32F303x8 have 82 interrupt handlers
static_assert(std::size(g_pfnVectors) == 11 + 82, "isr_vector size must be 98");
```