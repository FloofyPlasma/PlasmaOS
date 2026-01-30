section .text
bits 64

extern syscall_handler

; Syscall entry point from userspace
; Arguments in: rax=syscall_num, rdi=arg1, rsi=arg2, rdx=arg3, r10=arg4, r8=arg5, r9=arg6
; Result in: rax
global syscall_entry_asm
syscall_entry_asm:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov rcx, r10

    push rax
    push rdi
    push rsi
    push rdx
    push rcx
    push r8
    push r9

    ; Setup args for syscall_handler(num, arg1, arg2, arg3, arg4, arg5, arg6)
    mov rdi, rax
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    mov rax, [rsp]
    mov [rsp], rdi
    mov rdi, rax

    call syscall_handler

    add rsp, 8

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    iretq