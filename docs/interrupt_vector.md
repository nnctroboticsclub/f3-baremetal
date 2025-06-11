| Vector | 0x000                     | 0x004                    | 0x008                         | 0x00C                    |
| ------ | ------------------------- | ------------------------ | ----------------------------- | ------------------------ |
| 0x000  | _estack                   | Reset_Handler            | NMI_Handler                   | HardFault_Handler        |
| 0x010  | MemManage_Handler         | BusFault_Handler         | UsageFault_Handler            | 0                        |
| 0x020  | 0                         | 0                        | 0                             | SVC_Handler              |
| 0x030  | DebugMon_Handler          | 0                        | PendSV_Handler                | SysTick_Handler          |

| 0x040  | WWDG_IRQHandler           | PVD_IRQHandler           | TAMP_STAMP_IRQHandler         | RTC_WKUP_IRQHandler      |
| 0x050  | FLASH_IRQHandler          | RCC_IRQHandler           | EXTI0_IRQHandler              | EXTI1_IRQHandler         |
| 0x060  | EXTI2_TSC_IRQHandler      | EXTI3_IRQHandler         | EXTI4_IRQHandler              | DMA1_Channel1_IRQHandler |
| 0x070  | DMA1_Channel2_IRQHandler  | DMA1_Channel3_IRQHandler | DMA1_Channel4_IRQHandler      | DMA1_Channel5_IRQHandler |

| 0x080  | DMA1_Channel6_IRQHandler  | DMA1_Channel7_IRQHandler | ADC1_2_IRQHandler             | CAN_TX_IRQHandler        |
| 0x090  | CAN_RX0_IRQHandler        | CAN_RX1_IRQHandler       | CAN_SCE_IRQHandler            | EXTI9_5_IRQHandler       |
| 0x0A0  | TIM1_BRK_TIM15_IRQHandler | TIM1_UP_TIM16_IRQHandler | TIM1_TRG_COM_TIM17_IRQHandler | TIM1_CC_IRQHandler       |
| 0x0B0  | TIM2_IRQHandler           | TIM3_IRQHandler          | 0                             | I2C1_EV_IRQHandler       |

| 0x0C0  | I2C1_ER_IRQHandler        | 0                        | 0                             | SPI1_IRQHandler          |
| 0x0D0  | 0                         | USART1_IRQHandler        | USART2_IRQHandler             | USART3_IRQHandler        |
| 0x0E0  | EXTI15_10_IRQHandler      | RTC_Alarm_IRQHandler     | 0                             | 0                        |
| 0x0F0  | 0                         | 0                        | 0                             | 0                        |
| 0x100  | 0                         | 0                        | 0                             | 0                        |

| 0x110  | 0                         | 0                        | TIM6_DAC1_IRQHandler          | TIM7_DAC2_IRQHandler     |
| 0x120  | 0                         | 0                        | 0                             | 0                        |
| 0x130  | 0                         | 0                        | 0                             | 0                        |
| 0x140  | COMP2_IRQHandler          | COMP4_6_IRQHandler       | 0                             | 0                        |
| 0x150  | 0                         | 0                        | 0                             | 0                        |
| 0x160  | 0                         | 0                        | 0                             | 0                        |
| 0x170  | 0                         | 0                        | 0                             | 0                        |
| 0x180  | 0                         | FPU_IRQHandler           | -                             | -                        |