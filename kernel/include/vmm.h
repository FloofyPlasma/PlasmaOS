#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include <stdint.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_WRITETHROUGH (1ULL << 3)
#define PAGE_NO_CACHE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NO_EXECUTE (1ULL << 63)

typedef struct
{
  uint64_t entries[512];
} __attribute__((packed)) page_table_t;

typedef struct
{
  page_table_t *pml4;
  uint64_t cr3_value;
} address_space_t;

void vmm_init(void);
address_space_t *vmm_create_address_space(void);
void vmm_destroy_address_space(address_space_t *as);
int vmm_map_page(address_space_t *as, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap_page(address_space_t *as, uint64_t virt);
void vmm_switch_address_space(address_space_t *as);
address_space_t *vmm_get_kernel_address_space(void);

#endif
