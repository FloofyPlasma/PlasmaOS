#include "vmm.h"

#include "pmm.h"
#include "serial.h"

#include <stddef.h>

#include "kernel_limine.h"

// This should be set during kernel init from Limine's HHDM request
extern uint64_t hhdm_offset;

static address_space_t kernel_address_space;

static inline uint64_t pml4_index(uint64_t vaddr) { return (vaddr >> 39) & 0x1FF; }
static inline uint64_t pdpt_index(uint64_t vaddr) { return (vaddr >> 30) & 0x1FF; }
static inline uint64_t pd_index(uint64_t vaddr) { return (vaddr >> 21) & 0x1FF; }
static inline uint64_t pt_index(uint64_t vaddr) { return (vaddr >> 12) & 0x1FF; }

static inline uint64_t entry_to_phys(uint64_t entry) { return entry & 0x000FFFFFFFFFF000ULL; }

// Convert physical address to virtual address using Limine's HHDM
static inline void *phys_to_virt(uint64_t phys) { return (void *) (phys + hhdm_offset); }

static inline uint64_t virt_to_phys(void *virt)
{
  uint64_t addr = (uint64_t) virt;

  // Check if it's in the HHDM region
  if (addr >= hhdm_offset && addr < hhdm_offset + 0x100000000000ULL)
  {
    return addr - hhdm_offset;
  }

  // Check if it's in the kernel higher half
  if (addr >= 0xFFFFFFFF80000000ULL)
  {
    return addr - 0xFFFFFFFF80000000ULL;
  }

  // Otherwise assume it's already physical
  return addr;
}

static page_table_t *get_or_create_table(uint64_t *entry, uint64_t flags)
{
  if (*entry & PAGE_PRESENT)
  {
    uint64_t phys = entry_to_phys(*entry);
    return (page_table_t *) phys_to_virt(phys);
  }

  // pmm_alloc_page returns a physical address
  void *table_phys = pmm_alloc_page();
  if (!table_phys)
  {
    return NULL;
  }

  uint64_t phys = (uint64_t) table_phys;
  *entry = phys | flags;

  // Return virtual address so caller can write to it
  return (page_table_t *) phys_to_virt(phys);
}

void vmm_init()
{
  serial_print("VMM: Initializing virtual memory manager...\n");

  struct limine_executable_address_request kernel_address_request = limine_get_executable_address_request();
  if (!kernel_address_request.response)
  {
    serial_print("VMM: Error: No kernel address info from bootloader!\n");
    return;
  }

  uint64_t kernel_phys_base = kernel_address_request.response->physical_base;
  uint64_t kernel_virt_base = kernel_address_request.response->virtual_base;

  serial_print("VMM: Kernel physical base: ");
  serial_print_hex(kernel_phys_base);
  serial_print("\n");
  serial_print("VMM: Kernel virtual base: ");
  serial_print_hex(kernel_virt_base);
  serial_print("\n");

  // Get current CR3 (Limine's page tables)
  uint64_t current_cr3;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
  current_cr3 &= ~0xFFF; // Clear flags

  uint64_t current_rsp;
  __asm__ volatile("mov %%rsp, %0" : "=r"(current_rsp));

  kernel_address_space.cr3_value = current_cr3;
  kernel_address_space.pml4 = (page_table_t *) phys_to_virt(current_cr3);

  // Limine has already set up all necessary mappings:
  // - Kernel code and data at 0xFFFFFFFF80000000+
  // - Stack
  // - Framebuffer
  // - HHDM at hhdm_offset
  // - Identity mappings for low memory

  // We can add custom mappings here in the future if needed

  serial_print("VMM: Virtual memory manager initialized\n");
  serial_print("VMM: Page tables ready for use\n");
}

int vmm_map_page(address_space_t *as, uint64_t virt, uint64_t phys, uint64_t flags)
{
  if (!as || !as->pml4)
  {
    return -1;
  }

  uint64_t pml4_i = pml4_index(virt);
  uint64_t pdpt_i = pdpt_index(virt);
  uint64_t pd_i = pd_index(virt);
  uint64_t pt_i = pt_index(virt);

  page_table_t *pdpt = get_or_create_table(&as->pml4->entries[pml4_i], PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER));
  if (!pdpt)
  {
    return -1;
  }

  if (flags & PAGE_HUGE && pd_i == 0 && pt_i == 0)
  {
    pdpt->entries[pdpt_i] = phys | flags;
    return 0;
  }

  page_table_t *pd = get_or_create_table(&pdpt->entries[pdpt_i], PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER));
  if (!pd)
  {
    return -1;
  }

  if (flags & PAGE_HUGE)
  {
    pd->entries[pd_i] = phys | flags;
    return 0;
  }

  page_table_t *pt = get_or_create_table(&pd->entries[pd_i], PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER));
  if (!pt)
  {
    return -1;
  }

  pt->entries[pt_i] = phys | flags;

  __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return 0;
}

void vmm_unmap_page(address_space_t *as, uint64_t virt)
{
  if (!as || !as->pml4)
  {
    return;
  }

  uint64_t pml4_i = pml4_index(virt);
  uint64_t pdpt_i = pdpt_index(virt);
  uint64_t pd_i = pd_index(virt);
  uint64_t pt_i = pt_index(virt);

  if (!(as->pml4->entries[pml4_i] & PAGE_PRESENT))
  {
    return;
  }

  uint64_t pdpt_phys = entry_to_phys(as->pml4->entries[pml4_i]);
  page_table_t *pdpt = (page_table_t *) phys_to_virt(pdpt_phys);

  if (!(pdpt->entries[pdpt_i] & PAGE_PRESENT))
  {
    return;
  }

  if (pdpt->entries[pdpt_i] & PAGE_HUGE)
  {
    pdpt->entries[pdpt_i] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    return;
  }

  uint64_t pd_phys = entry_to_phys(pdpt->entries[pdpt_i]);
  page_table_t *pd = (page_table_t *) phys_to_virt(pd_phys);

  if (!(pd->entries[pd_i] & PAGE_PRESENT))
  {
    return;
  }

  if (pd->entries[pd_i] & PAGE_HUGE)
  {
    pd->entries[pd_i] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    return;
  }

  uint64_t pt_phys = entry_to_phys(pd->entries[pd_i]);
  page_table_t *pt = (page_table_t *) phys_to_virt(pt_phys);

  pt->entries[pt_i] = 0;

  __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_switch_address_space(address_space_t *as)
{
  if (!as)
  {
    return;
  }

  __asm__ volatile("mov %0, %%cr3" : : "r"(as->cr3_value) : "memory");
}

address_space_t *vmm_get_kernel_address_space() { return &kernel_address_space; }

address_space_t *vmm_create_address_space()
{
  // This allocates physical memory, not virtual!
  address_space_t *as_phys = pmm_alloc_page();
  if (!as_phys)
  {
    return NULL;
  }

  address_space_t *as = (address_space_t *) phys_to_virt((uint64_t) as_phys);

  void *pml4_phys = pmm_alloc_page();
  if (!pml4_phys)
  {
    pmm_free_page(as_phys);
    return NULL;
  }

  as->pml4 = (page_table_t *) phys_to_virt((uint64_t) pml4_phys);
  as->cr3_value = (uint64_t) pml4_phys;

  // Copy kernel mappings (upper half)
  for (int i = 256; i < 512; i++)
  {
    as->pml4->entries[i] = kernel_address_space.pml4->entries[i];
  }

  return as;
}

void vmm_destroy_address_space(address_space_t *as)
{
  if (!as || as == &kernel_address_space)
  {
    return;
  }
  // TODO: Walk and free user page tables, for now just free the PML4

  if (as->pml4)
  {
    uint64_t pml4_phys = virt_to_phys(as->pml4);
    pmm_free_page((void *) pml4_phys);
  }

  uint64_t as_phys = virt_to_phys(as);
  pmm_free_page((void *) as_phys);
}
