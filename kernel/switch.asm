global context_switch

context_switch:
  ; Save old context
  push rbp
  push rbx
  push r12
  push r13
  push r14
  push r15

  ; rdi = &old_rsp;
  ; rsi = new_rsp;
 
  mov [rdi], rsp ; save old
  mov rsp, rsi ; load new

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  pop rbp

  ret
