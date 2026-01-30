#include "pmm.h"

#include "kernel_limine.h"
#include "serial.h"

#include <limine.h>
#include <stddef.h>

extern uint64_t hhdm_offset;

#define MAX_PAGES 1048576 // For now up to 4GB ram

static uint64_t page_stack[MAX_PAGES];
static uint64_t page_stack_top = 0;
static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t kernel_end = 0;

static inline uint64_t align_up(uint64_t addr, uint64_t align) { return (addr + align - 1) & ~(align - 1); }

static inline uint64_t align_down(uint64_t addr, uint64_t align) { return addr & ~(align - 1); }

void pmm_init()
{
  serial_print("PMM: Initializing physical memory manager...\n");

  struct limine_memmap_request request = limine_get_memmap_request();
  if (!request.response)
  {
    serial_print("PMM: Error: No memory map from bootloader!\n");
    return;
  }

  struct limine_memmap_response *memmap = request.response;

  serial_print("PMM: Found ");
  serial_print_dec(memmap->entry_count);
  serial_print(" memory regions\n");

  page_stack_top = 0;
  total_pages = 0;
  free_pages = 0;

  uint64_t total_usable = 0;
  for (uint64_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE)
    {
      total_usable += entry->length;
    }
  }

  serial_print("PMM: Total usable memory: ");
  serial_print_dec(total_usable / 1024 / 1024);
  serial_print(" MB\n");

  for (uint64_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type != LIMINE_MEMMAP_USABLE)
    {
      continue;
    }

    uint64_t base = align_up(entry->base, PAGE_SIZE);
    uint64_t top = align_down(entry->base + entry->length, PAGE_SIZE);

    if (base >= top)
    {
      continue;
    }

    for (uint64_t addr = base; addr < top; addr += PAGE_SIZE)
    {
      if (addr < 0x10000)
      {
        continue;
      }

      if (page_stack_top >= MAX_PAGES)
      {
        serial_print("PMM: Warning: Page stack full!\n");
        break;
      }

      page_stack[page_stack_top++] = addr;
      total_pages++;
      free_pages++;
    }
  }

  serial_print("PMM: Initialized with ");
  serial_print_dec(total_pages);
  serial_print(" pages (\n");
  serial_print_dec((total_pages * PAGE_SIZE) / 1024 / 1024);
  serial_print(" MB)\n");
}

void *pmm_alloc_page()
{
  if (page_stack_top == 0)
  {
    serial_print("PMM: Error: Out of memory!\n");
    return NULL;
  }

  free_pages--;
  uint64_t page = page_stack[--page_stack_top];

  uint8_t *ptr = (uint8_t *) (page + hhdm_offset);
  for (int i = 0; i < PAGE_SIZE; i++)
  {
    ptr[i] = 0;
  }

  return (void *) page;
}

void pmm_free_page(void *page)
{
  if (!page)
  {
    return;
  }

  if (page_stack_top >= MAX_PAGES)
  {
    serial_print("PMM: Error: Page stack overflow on free!\n");
    return;
  }

  page_stack[page_stack_top++] = (uint64_t) page;
  free_pages++;
}

uint64_t pmm_get_total_memory() { return total_pages * PAGE_SIZE; }

uint64_t pmm_get_free_memory() { return free_pages * PAGE_SIZE; }

uint64_t pmm_get_used_memory() { return (total_pages - free_pages) * PAGE_SIZE; }
