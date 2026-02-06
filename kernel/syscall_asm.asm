section .text
bits 64

extern syscall_handler

global syscall_entry_asm
syscall_entry_asm:
    ; CPU has already pushed: SS, RSP, RFLAGS, CS, RIP
    ; Save callee-saved registers
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; syscall_handler(num, arg1, arg2, arg3, arg4, arg5, arg6)
    ; Current state: rax=num, rdi=arg1, rsi=arg2, rdx=arg3, r10=arg4, r8=arg5, r9=arg6
    ; C calling convention needs: rdi=num, rsi=arg1, rdx=arg2, rcx=arg3, r8=arg4, r9=arg5, stack=arg6

    ; We need arg6 on stack for C calling convention
    push r9             ; arg6

    ; Shift arguments
    mov r9, r8          ; arg5: r8 -> r9
    mov r8, r10         ; arg4: r10 -> r8
    mov rcx, rdx        ; arg3: rdx -> rcx
    mov rdx, rsi        ; arg2: rsi -> rdx
    mov rsi, rdi        ; arg1: rdi -> rsi
    mov rdi, rax        ; num: rax -> rdi

    call syscall_handler

    ; Clean up arg6 from stack
    add rsp, 8

    ; Restore callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    ; Return to userspace
    ; The iretq will pop: RIP, CS, RFLAGS, RSP, SS
    iretq