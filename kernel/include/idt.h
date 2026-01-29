#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

typedef struct
{
  uint16_t offset_low; // Lower 16 bits of handler address
  uint16_t selector; // Kernel code segment selector
  uint8_t ist; // Interupt stack table offset
  uint8_t type_attr; // Type and attributes
  uint16_t offset_mid; // Middle 16 bits of handler address
  uint32_t offset_high; // Upper 32 bits of handler address
  uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) idt_ptr_t;

#define IDT_TYPE_INTERRUPT_GATE 0x8E // Present, Ring 0, Interrupt gate
#define IDT_TYPE_TRAP_GATE 0x8F // Present, Ring 0, Trap gate
#define IDT_TYPE_USER_INTERRUPT 0xEE // Present, Ring 3, Interrupt gate

#define EXCEPTION_DIV_BY_ZERO 0
#define EXCEPTION_DEBUG 1
#define EXCEPTION_NMI 2
#define EXCEPTION_BREAKPOINT 3
#define EXCEPTION_OVERFLOW 4
#define EXCEPTION_BOUND_RANGE 5
#define EXCEPTION_INVALID_OPCODE 6
#define EXCEPTION_DEVICE_NA 7
#define EXCEPTION_DOUBLE_FAULT 8
#define EXCEPTION_INVALID_TSS 10
#define EXCEPTION_SEGMENT_NP 11
#define EXCEPTION_STACK_FAULT 12
#define EXCEPTION_GP_FAULT 13
#define EXCEPTION_PAGE_FAULT 14
#define EXCEPTION_FPU 16
#define EXCEPTION_ALIGNMENT 17
#define EXCEPTION_MACHINE_CHECK 18
#define EXCEPTION_SIMD 19

// Hardware interrupt numbers (PIC remapped to 32-47)
#define IRQ_TIMER 32
#define IRQ_KEYBOARD 33

typedef struct
{
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute__((packed)) interrupt_frame_t;

typedef struct
{
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
  uint64_t int_no, error_code;
  interrupt_frame_t frame;
} __attribute__((packed)) registers_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t type_attr);

void exception_handler(registers_t *regs);
void irq_handler(registers_t *regs);

// Assembly ISR stubs (defined in idt_asm.asm)

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);

extern void isr_stub_32(void);
extern void isr_stub_33(void);
extern void isr_stub_34(void);
extern void isr_stub_35(void);
extern void isr_stub_36(void);
extern void isr_stub_37(void);
extern void isr_stub_38(void);
extern void isr_stub_39(void);
extern void isr_stub_40(void);
extern void isr_stub_41(void);
extern void isr_stub_42(void);
extern void isr_stub_43(void);
extern void isr_stub_44(void);
extern void isr_stub_45(void);
extern void isr_stub_46(void);
extern void isr_stub_47(void);

#endif
