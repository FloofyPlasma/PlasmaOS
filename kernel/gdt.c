#include "gdt.h"

#include <stddef.h>

#define GDT_ENTRIES 7
static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdt_pointer;
static tss_t tss;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint32_t granularity)
{
  gdt[index].limit_low = limit & 0xFFFF;
  gdt[index].base_low = base & 0xFFFF;
  gdt[index].base_mid = (base >> 16) & 0xFF;
  gdt[index].access = access;
  gdt[index].granularity = (granularity & 0xF0) | ((limit >> 16) & 0x0F);
  gdt[index].base_high = (base >> 24) & 0xFF;
}

static void gdt_set_tss(int index, uint64_t base, uint32_t limit)
{
  gdt[index].limit_low = limit & 0xFFFF;
  gdt[index].base_low = base & 0xFFFF;
  gdt[index].base_mid = (base >> 16) & 0xFF;
  gdt[index].access = 0x89;
  gdt[index].granularity = 0x00;
  gdt[index].base_high = (base >> 24) & 0xFF;
  gdt[index + 1].limit_low = (base >> 32) & 0xFFFF;
  gdt[index + 1].base_low = (base >> 48) & 0xFFFF;
  gdt[index + 1].base_mid = 0;
  gdt[index + 1].access = 0;
  gdt[index + 1].granularity = 0;
  gdt[index + 1].base_high = 0;
}

void gdt_init()
{
  for (int i = 0; i < GDT_ENTRIES; i++)
  {
    gdt[i].limit_low = 0;
    gdt[i].base_low = 0;
    gdt[i].base_mid = 0;
    gdt[i].access = 0;
    gdt[i].granularity = 0;
    gdt[i].base_high = 0;
  }

  gdt_set_entry(GDT_NULL, 0, 0, 0, 0);

  gdt_set_entry(GDT_KERNEL_CODE, 0, 0xFFFFF,
      GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SEGMENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
      GDT_GRANULARITY_4K | GDT_GRANULARITY_64BIT);

  gdt_set_entry(GDT_KERNEL_DATA, 0, 0xFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SEGMENT | GDT_ACCESS_RW,
      GDT_GRANULARITY_4K | GDT_GRANULARITY_64BIT);

  gdt_set_entry(GDT_USER_CODE, 0, 0xFFFFF,
      GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SEGMENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
      GDT_GRANULARITY_4K | GDT_GRANULARITY_64BIT);

  gdt_set_entry(GDT_USER_DATA, 0, 0xFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SEGMENT | GDT_ACCESS_RW,
      GDT_GRANULARITY_4K | GDT_GRANULARITY_64BIT);

  for (int i = 0; i < sizeof(tss_t); i++)
  {
    ((uint8_t *) &tss)[i] = 0;
  }

  tss.rsp0 = 0;
  tss.iomap_base = sizeof(tss_t);
  gdt_set_tss(GDT_TSS, (uint64_t) &tss, sizeof(tss_t) - 1);

  gdt_pointer.limit = sizeof(gdt) - 1;
  gdt_pointer.base = (uint64_t) &gdt;

  gdt_load(&gdt_pointer);
  tss_load(TSS_SEG);
}

void gdt_set_kernel_stack(uint64_t stack) { tss.rsp0 = stack; }
