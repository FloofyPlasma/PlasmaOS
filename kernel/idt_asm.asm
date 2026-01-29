section .text
bits 64

extern exception_handler
extern irq_handler

global idt_load
idt_load:
    lidt [rdi]
    ret

%macro ISR_NO_ERROR 1
global isr_stub_%1
isr_stub_%1:
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERROR 1
global isr_stub_%1
isr_stub_%1:
    push %1
    jmp isr_common
%endmacro


ISR_NO_ERROR 0
ISR_NO_ERROR 1
ISR_NO_ERROR 2
ISR_NO_ERROR 3
ISR_NO_ERROR 4
ISR_NO_ERROR 5
ISR_NO_ERROR 6
ISR_NO_ERROR 7
ISR_ERROR    8
ISR_NO_ERROR 9
ISR_ERROR    10
ISR_ERROR    11
ISR_ERROR    12
ISR_ERROR    13
ISR_ERROR    14
ISR_NO_ERROR 15
ISR_NO_ERROR 16
ISR_ERROR    17
ISR_NO_ERROR 18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_ERROR    21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_ERROR    29
ISR_ERROR    30
ISR_NO_ERROR 31

ISR_NO_ERROR 32
ISR_NO_ERROR 33
ISR_NO_ERROR 34
ISR_NO_ERROR 35
ISR_NO_ERROR 36
ISR_NO_ERROR 37
ISR_NO_ERROR 38
ISR_NO_ERROR 39
ISR_NO_ERROR 40
ISR_NO_ERROR 41
ISR_NO_ERROR 42
ISR_NO_ERROR 43
ISR_NO_ERROR 44
ISR_NO_ERROR 45
ISR_NO_ERROR 46
ISR_NO_ERROR 47

isr_common:
    push rax
    push rcx
    push rdx
    push rbx
    push rsp
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp

    mov rax, [rsp + 16 * 8]
    cmp rax, 32
    jl .exception

    call irq_handler
    jmp .done

.exception:
    call exception_handler

.done:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    add rsp, 8
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16

    iretq