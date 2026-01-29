#include "idt.h"

#include "pit.h"
#include "serial.h"

#include <stddef.h>

#define IDT_ENTRIES 256
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_pointer;

static const char *exception_messages[] = { "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
  "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available", "Double Fault",
  "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present", "Stack-Segment Fault",
  "General Protection Fault", "Page Fault", "Reserved", "x87 FPU Error", "Alignment Check", "Machine Check",
  "SIMD Floating-Point Exception", "Virtualization Exception", "Control Protection Exception", "Reserved", "Reserved",
  "Reserved", "Reserved", "Reserved", "Reserved", "Hypervisor Injection Exception", "VMM Communication Exception",
  "Security Exception", "Reserved" };

extern void idt_load(idt_ptr_t *idt_ptr);

void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t type_attr)
{
  idt[num].offset_low = handler & 0xFFFF;
  idt[num].selector = selector;
  idt[num].ist = 0;
  idt[num].type_attr = type_attr;
  idt[num].offset_mid = (handler >> 16) & 0xFFFF;
  idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
  idt[num].reserved = 0;
}

void idt_init()
{
  for (int i = 0; i < IDT_ENTRIES; i++)
  {
    idt[i].offset_low = 0;
    idt[i].selector = 0;
    idt[i].ist = 0;
    idt[i].type_attr = 0;
    idt[i].offset_mid = 0;
    idt[i].offset_high = 0;
    idt[i].reserved = 0;
  }

  idt_set_gate(0, (uint64_t) isr_stub_0, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(1, (uint64_t) isr_stub_1, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(2, (uint64_t) isr_stub_2, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(3, (uint64_t) isr_stub_3, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(4, (uint64_t) isr_stub_4, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(5, (uint64_t) isr_stub_5, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(6, (uint64_t) isr_stub_6, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(7, (uint64_t) isr_stub_7, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(8, (uint64_t) isr_stub_8, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(9, (uint64_t) isr_stub_9, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(10, (uint64_t) isr_stub_10, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(11, (uint64_t) isr_stub_11, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(12, (uint64_t) isr_stub_12, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(13, (uint64_t) isr_stub_13, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(14, (uint64_t) isr_stub_14, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(15, (uint64_t) isr_stub_15, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(16, (uint64_t) isr_stub_16, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(17, (uint64_t) isr_stub_17, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(18, (uint64_t) isr_stub_18, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(19, (uint64_t) isr_stub_19, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(20, (uint64_t) isr_stub_20, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(21, (uint64_t) isr_stub_21, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(22, (uint64_t) isr_stub_22, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(23, (uint64_t) isr_stub_23, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(24, (uint64_t) isr_stub_24, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(25, (uint64_t) isr_stub_25, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(26, (uint64_t) isr_stub_26, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(27, (uint64_t) isr_stub_27, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(28, (uint64_t) isr_stub_28, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(29, (uint64_t) isr_stub_29, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(30, (uint64_t) isr_stub_30, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(31, (uint64_t) isr_stub_31, 0x08, IDT_TYPE_INTERRUPT_GATE);

  idt_set_gate(32, (uint64_t) isr_stub_32, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(33, (uint64_t) isr_stub_33, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(34, (uint64_t) isr_stub_34, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(35, (uint64_t) isr_stub_35, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(36, (uint64_t) isr_stub_36, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(37, (uint64_t) isr_stub_37, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(38, (uint64_t) isr_stub_38, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(39, (uint64_t) isr_stub_39, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(40, (uint64_t) isr_stub_40, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(41, (uint64_t) isr_stub_41, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(42, (uint64_t) isr_stub_42, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(43, (uint64_t) isr_stub_43, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(44, (uint64_t) isr_stub_44, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(45, (uint64_t) isr_stub_45, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(46, (uint64_t) isr_stub_46, 0x08, IDT_TYPE_INTERRUPT_GATE);
  idt_set_gate(47, (uint64_t) isr_stub_47, 0x08, IDT_TYPE_INTERRUPT_GATE);

  idt_pointer.limit = sizeof(idt) - 1;
  idt_pointer.base = (uint64_t) &idt;

  idt_load(&idt_pointer);
}

void exception_handler(registers_t *regs)
{
  serial_print("\n=== CPU EXCEPTION ===\n");
  serial_print("Exception: ");

  if (regs->int_no < 32)
  {
    serial_print(exception_messages[regs->int_no]);
  } else
  {
    serial_print("Unknown Exception");
  }

  serial_print("\nInterrupt Number: ");
  serial_print_dec(regs->int_no);
  serial_print("\nError Code: ");
  serial_print_hex(regs->error_code);

  serial_print("\n\nRegisters:\n");
  serial_print("RAX=");
  serial_print_hex(regs->rax);
  serial_print(" RBX=");
  serial_print_hex(regs->rbx);
  serial_print(" RCX=");
  serial_print_hex(regs->rcx);
  serial_print("\nRDX=");
  serial_print_hex(regs->rdx);
  serial_print(" RSI=");
  serial_print_hex(regs->rsi);
  serial_print(" RDI=");
  serial_print_hex(regs->rdi);
  serial_print("\nRBP=");
  serial_print_hex(regs->rbp);
  serial_print(" RSP=");
  serial_print_hex(regs->rsp);
  serial_print("\nRIP=");
  serial_print_hex(regs->frame.rip);
  serial_print(" RFLAGS=");
  serial_print_hex(regs->frame.rflags);
  serial_print("\nCS=");
  serial_print_hex(regs->frame.cs);
  serial_print(" SS=");
  serial_print_hex(regs->frame.ss);

  if (regs->int_no == EXCEPTION_PAGE_FAULT)
  {
    uint64_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    serial_print("\nFaulting Address (CR2): ");
    serial_print_hex(cr2);
  }
  serial_print("\n\nSystem Halted.\n");

  for (;;)
  {
    __asm__ volatile("cli; hlt");
  }
}

void irq_handler(registers_t *regs)
{
  if (regs->int_no >= 40)
  {
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t) 0x20), "Nd"((uint16_t) 0xA0));
  }
  __asm__ volatile("outb %0, %1" : : "a"((uint8_t) 0x20), "Nd"((uint16_t) 0x20));
  switch (regs->int_no)
  {
    case IRQ_TIMER:
    {
      pit_tick();
      break;
    }

    case IRQ_KEYBOARD:
    {
      // TODO:
      break;
    }

    default:
    {
      break;
    }
  }
}
