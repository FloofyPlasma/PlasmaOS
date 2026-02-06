#include <stddef.h>

#include "gdt.h"
#include "idt.h"
#include "kernel_limine.h"
#include "pit.h"
#include "pmm.h"
#include "serial.h"
#include "syscall.h"
#include "thread.h"
#include "vmm.h"

static void hcf(void)
{
  for (;;)
  {
    __asm__ volatile("cli; hlt");
  }
}

static Port *test_port = NULL;

void thread_receiver()
{
  serial_print("Reciever: Starting and waiting for messages...\n");

  for (int i = 0; i < 5; i++)
  {
    Message msg;
    serial_print("Receiver: Calling recv() [will block if no message]...\n");

    int result = recv(test_port, &msg);

    if (result == 0)
    {
      serial_print("Receiver: Got message! ID=");
      serial_print_dec(msg.id);
      serial_print(" Data=[");
      for (int j = 0; j <= 3; j++)
      {
        serial_print_dec(msg.data[j]);
        serial_print(", ");
      }
      serial_print("]\n");
    } else
    {
      serial_print("Reciever: recv() failed with error code\n");
    }
  }

  serial_print("Receiver: Finished receiving 5 messages, exiting.\n");

  for (;;)
  {
    thread_yield();
  }
}

void thread_sender()
{
  serial_print("Sender: Starting, will send 5 messages...\n");

  for (int i = 0; i < 2; i++)
  {
    thread_yield();
  }

  for (int i = 1; i <= 5; i++)
  {
    serial_print("Sender: Sending message...\n");
    char buf[8];
    buf[0] = '0' + i;
    buf[1] = '\0';
    serial_print(buf);
    serial_print("...\n");

    int result = send(test_port, i, i * 10, i * 20, i * 30, i * 40);

    if (result == 0)
    {
      serial_print("Sender: Message sent successfully\n");
    } else
    {
      serial_print("Sender: send() failed\n");
    }
    for (int j = 0; j < 3; j++)
    {
      thread_yield();
    }
  }

  serial_print("Sender: Finished sending all messages, exiting.\n");

  for (;;)
  {
    thread_yield();
  }
}

void kernel_main(void)
{
  serial_init();

  serial_print("PlasmaOS Kernel v0.1.0\n");
  serial_print("======================\n\n");

  serial_print("Parsing Limine bootloader info...\n");
  limine_parse_info();

  serial_print("\nInitializing CPU infrastructure...\n");
  serial_print("  - Setting up GDT (Global Descriptor Table)...\n");
  gdt_init();
  serial_print("  - GDT loaded successfully\n");

  serial_print("  - Setting up IDT (Interrupt Descriptor Table)...\n");
  idt_init();
  serial_print("  - IDT loaded successfully\n");

  serial_print("  - Initializing PIT (Programmable Interval Timer) at 100Hz...\n");
  pit_init(100);
  serial_print("  - PIT initialized, interrupts enabled\n");

  serial_print("\nCPU infrastructure complete\n\n");

  serial_print("Initializing memory management...\n");
  serial_print("  - Initilizing PMM (Physical Memory Manager)...\n");
  pmm_init();

  serial_print("  - Initializing VMM (Virtual Memory Manager)...\n");
  vmm_init();

  serial_print("\nMemory Management Complete!\n");
  serial_print("  * Free memory: ");
  serial_print_dec(pmm_get_free_memory() / 1024 / 1024);
  serial_print(" MB\n\n");

  serial_print("Testing memory allocation...\n");
  void *page1 = pmm_alloc_page();
  void *page2 = pmm_alloc_page();
  void *page3 = pmm_alloc_page();

  serial_print("  Allocated pages: ");
  serial_print_hex((uint64_t) page1);
  serial_print(", ");
  serial_print_hex((uint64_t) page2);
  serial_print(", ");
  serial_print_hex((uint64_t) page3);
  serial_print("\n");

  pmm_free_page(page2);
  serial_print("  Freed middle page\n");

  void *page4 = pmm_alloc_page();
  serial_print("  Reallocated: ");
  serial_print_hex((uint64_t) page4);
  serial_print("\n");
  serial_print("Memory allocation test passed!\n\n");

  serial_print("\nInitializing threading subsystem...\n");
  thread_init();

  serial_print("Initializing syscall interface...\n");
  syscall_init();
  serial_print("Syscalls initialized\n\n");

  serial_print("Creating IPC test port...\n");
  test_port = port_create();

  if (!test_port)
  {
    serial_print("ERROR: Failed to create port!\n");
    hcf();
  }

  serial_print("Port created successfully\n\n");

  // Optional: uncomment to test kernel-mode IPC
  // thread_create(thread_receiver);
  // thread_create(thread_sender);

  serial_print("Loading userspace init program...\n");

  // Get the init module from Limine
  if (module_request.response == NULL || module_request.response->module_count == 0)
  {
    serial_print("ERROR: No userspace modules loaded!\n");
    serial_print("Make sure init.bin is included in the ISO\n");
    hcf();
  }

  struct limine_file *init_module = module_request.response->modules[0];
  serial_print("  Found module: ");
  serial_print(init_module->path);
  serial_print("\n  Size: ");
  serial_print_dec(init_module->size);
  serial_print(" bytes\n");

  uint64_t user_code_virt = 0x0000000000400000ULL;
  size_t pages_needed = (init_module->size + 0xFFF) / 0x1000;

  serial_print("  Allocating and mapping ");
  serial_print_dec(pages_needed);
  serial_print(" pages for user code...\n");

  address_space_t *kernel_as = vmm_get_kernel_address_space();

  for (size_t i = 0; i < pages_needed; i++)
  {
    void *phys = pmm_alloc_page();
    if (!phys)
    {
      serial_print("ERROR: Failed to allocate page for user code!\n");
      hcf();
    }

    int map_result = vmm_map_page(
        kernel_as, user_code_virt + (i * 0x1000), (uint64_t) phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    if (map_result != 0)
    {
      serial_print("ERROR: Failed to map user code page!\n");
      hcf();
    }
  }

  for (size_t i = 0; i < pages_needed; i++)
  {
    uint8_t *src = (uint8_t *) init_module->address + (i * 0x1000);
    uint8_t *dst = (uint8_t *) (user_code_virt + (i * 0x1000));

    for (size_t j = 0; j < 0x1000 && (i * 0x1000 + j) < init_module->size; j++)
    {
      dst[j] = src[j];
    }
  }

  serial_print("  User code mapped successfully at ");
  serial_print_hex(user_code_virt);
  serial_print("\n");

  serial_print("  Allocating user stack...\n");
  void *user_stack_phys = pmm_alloc_page();
  if (!user_stack_phys)
  {
    serial_print("ERROR: Failed to allocate user stack!\n");
    hcf();
  }

  uint64_t user_stack_virt = 0x00007FFFFFFFE000ULL;

  serial_print("  Mapping user stack to virtual address: ");
  serial_print_hex(user_stack_virt);
  serial_print("\n");

  int map_result
      = vmm_map_page(kernel_as, user_stack_virt, (uint64_t) user_stack_phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

  if (map_result != 0)
  {
    serial_print("ERROR: Failed to map user stack! Error: ");
    serial_print_dec(map_result);
    serial_print("\n");
    hcf();
  }

  void *user_stack_top = (void *) (user_stack_virt + 0x1000);

  serial_print("  User stack mapped successfully\n");

  serial_print("Creating userspace thread...\n");
  serial_print("  Entry point: ");
  serial_print_hex(user_code_virt);
  serial_print("\n");
  serial_print("  Stack top: ");
  serial_print_hex((uint64_t) user_stack_top);
  serial_print("\n");

  thread_create_user((void *) user_code_virt, user_stack_top);
  serial_print("Userspace thread created\n\n");

  serial_print("Starting scheduler...\n\n");
  scheduler_start();

  // Should never reach here
  serial_print("\nERROR: Scheduler returned!\n");
  hcf();
}
