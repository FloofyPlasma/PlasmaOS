#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

typedef struct
{
  uint16_t limit_low; // Lower 16 bits of limit
  uint16_t base_low; // Lower 16 bits of base
  uint8_t base_mid; // Next 8 bits of base
  uint8_t access; // Access flags
  uint8_t granularity; // Granularity and limit high
  uint8_t base_high; // High 8 bits of base
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
  uint32_t reserved0;
  uint64_t rsp0; // Stack pointer for ring 0
  uint64_t rsp1; // Stack pointer for ring 1
  uint64_t rsp2; // Stack pointer for ring 2
  uint64_t reserved1;
  uint64_t ist1; // Interrupt stack table entries
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap_base;
} __attribute__((packed)) tss_t;

typedef struct
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

#define GDT_NULL 0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE 3
#define GDT_USER_DATA 4
#define GDT_TSS 5

#define KERNEL_CS (GDT_KERNEL_CODE << 3)
#define KERNEL_DS (GDT_KERNEL_DATA << 3)
#define USER_CS ((GDT_USER_CODE << 3) | 3)
#define USER_DS ((GDT_USER_DATA << 3) | 3)
#define TSS_SEG (GDT_TSS << 3)

#define GDT_ACCESS_PRESENT (1 << 7)
#define GDT_ACCESS_RING0 (0 << 5)
#define GDT_ACCESS_RING3 (3 << 5)
#define GDT_ACCESS_SEGMENT (1 << 4)
#define GDT_ACCESS_EXECUTABLE (1 << 3)
#define GDT_ACCESS_RW (1 << 1)

#define GDT_GRANULARITY_4K (1 << 7)
#define GDT_GRANULARITY_64BIT (1 << 5)

void gdt_init(void);
void gdt_set_kernel_stack(uint64_t stack);

extern void gdt_load(gdt_ptr_t *gdt_ptr);
extern void tss_load(uint16_t tss_selector);

#endif
