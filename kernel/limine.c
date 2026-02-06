#include "kernel_limine.h"

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#include "serial.h"

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[3]
    = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_requests_start[4]
    = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request
    = { .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0, .response = NULL };

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request
    = { .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0, .response = NULL };

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request
    = { .id = LIMINE_HHDM_REQUEST_ID, .revision = 0, .response = NULL };

__attribute__((used,
    section(".limine_requests"))) static volatile struct limine_executable_address_request executable_address_request
    = { .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 0, .response = NULL };

__attribute__((used, section(".limine_requests"))) volatile struct limine_module_request module_request
    = { .id = LIMINE_MODULE_REQUEST_ID, .revision = 0, .response = NULL };

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_requests_end[2]
    = LIMINE_REQUESTS_END_MARKER;

uint64_t hhdm_offset = 0;

void limine_parse_info(void)
{
  if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0)
  {
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    serial_print("  Framebuffer: ");
    serial_print_dec(fb->width);
    serial_print("x");
    serial_print_dec(fb->height);
    serial_print("x");
    serial_print_dec(fb->bpp);
    serial_print(" @ ");
    serial_print_hex((uint64_t) fb->address);
    serial_print("\n");
  } else
  {
    serial_print("  Framebuffer: Not available\n");
  }

  if (memmap_request.response)
  {
    serial_print("  Memory map: ");
    serial_print_dec(memmap_request.response->entry_count);
    serial_print(" entries\n");

    for (uint64_t i = 0; i < memmap_request.response->entry_count && i < 5; i++)
    {
      struct limine_memmap_entry *entry = memmap_request.response->entries[i];
      serial_print("    [");
      serial_print_dec(i);
      serial_print("] Base: ");
      serial_print_hex(entry->base);
      serial_print(", Length: ");
      serial_print_hex(entry->length);
      serial_print(", Type: ");
      serial_print_dec(entry->type);
      serial_print("\n");
    }
    if (memmap_request.response->entry_count > 5)
    {
      serial_print("    ... (");
      serial_print_dec(memmap_request.response->entry_count - 5);
      serial_print(" more entries)\n");
    }
  } else
  {
    serial_print("  Memory map: Not available\n");
  }

  if (hhdm_request.response)
  {
    hhdm_offset = hhdm_request.response->offset;
    serial_print("  HHDM offset: ");
    serial_print_hex(hhdm_request.response->offset);
    serial_print("\n");
  } else
  {
    serial_print("  HHDM: Not available\n");
  }

  if (executable_address_request.response)
  {
    serial_print("  Executable physical base: ");
    serial_print_hex(executable_address_request.response->physical_base);
    serial_print("\n");
    serial_print("  Executable virtual base: ");
    serial_print_hex(executable_address_request.response->virtual_base);
    serial_print("\n");
  } else
  {
    serial_print("  Executable address: Not available\n");
  }
}

struct limine_memmap_request limine_get_memmap_request() { return memmap_request; }
struct limine_executable_address_request limine_get_executable_address_request() { return executable_address_request; }
