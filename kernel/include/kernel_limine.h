#ifndef KERNEL_KERNEL_LIMINE_H
#define KERNEL_KERNEL_LIMINE_H

#include <limine.h>

extern uint64_t hhdm_offset;
extern volatile struct limine_module_request module_request;

void limine_parse_info(void);
struct limine_memmap_request limine_get_memmap_request(void);
struct limine_executable_address_request limine_get_executable_address_request(void);

#endif
