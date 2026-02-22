  .syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb


.equ  BootRAM,        0xF1E0F85F

    .section	.text.Reset_Handler
	.weak	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack
	bl StartUp

.size	Reset_Handler, .-Reset_Handler