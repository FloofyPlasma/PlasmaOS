section .text
bits 64

; void jump_to_usermode(uint64_t entry, uint64_t user_stack)
; rdi = entry point
; rsi = user stack pointer
global jump_to_usermode
jump_to_usermode:
    mov r10, rdi    ; Save entry point
    mov r11, rsi    ; Save user stack

    cli

    mov ax, 0x23    ; USER_DS selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Build iretq frame (must be 64-bit pushes)
    ; Stack layout (top to bottom): SS, RSP, RFLAGS, CS, RIP
    
    mov rax, 0x23   ; USER_DS
    push rax        ; SS
    
    push r11        ; RSP (user stack)
    
    pushfq          ; Get current RFLAGS
    pop rax
    or rax, 0x200   ; Set IF (enable interrupts)
    push rax        ; RFLAGS
    
    mov rax, 0x1B   ; USER_CS
    push rax        ; CS
    
    push r10        ; RIP (entry point)
    
    iretq

    ud2

global usermode_trampoline
usermode_trampoline:
    mov rdi, r12
    mov rsi, r13
    call jump_to_usermode
    ud2