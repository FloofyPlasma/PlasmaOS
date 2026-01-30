section .text
bits 64

; void jump_to_usermode(uint64_t entry, uint64_t user_stack)
; rdi = entry point
; rsi = user stack pointer
global jump_to_usermode
jump_to_usermode:
    mov r10, rdi
    mov r11, rsi

    cli

    mov ax, 0x23 ; USER_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23 ; USER_DS

    push r11

    pushfq
    pop rax
    or rax, 0x200
    push rax

    push 0x1B ; USER_CS

    push r10

    iretq

    ud2