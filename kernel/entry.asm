; Plasma Kernel Entry Point

section .text.entry
bits 64

global _start
extern kernel_main

_start:
  cld

  lea rsp, [rel stack_top]

  xor rbp, rbp

  call kernel_main

  ; Kernel shouldn't return, but just incase
.hang:
  cli
  hlt
  jmp .hang

section .bss
align 16
stack_bottom:
  resb 16384
stack_top:

