#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

void pmm_init(void);
void *pmm_alloc_page(void);
void pmm_free_page(void *page);
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_free_memory(void);
uint64_t pmm_get_used_memory(void);

#endif
