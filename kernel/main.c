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


// ============================================================================
// USERSPACE TEST PROGRAM
// ============================================================================

// Userspace test program entry point
void user_program_entry(void)
{
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Hello from ring 3!\n");

  // Create a port
  int64_t port_id = syscall0(SYS_PORT_CREATE);
  if (port_id < 0)
  {
    syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] ERROR: Failed to create port!\n");
    syscall1(SYS_THREAD_EXIT, 1);
  }

  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Created port successfully\n");

  // Send a message to ourselves
  int64_t result = syscall6(SYS_SEND, port_id, 42, 100, 200, 300, 400);
  if (result < 0)
  {
    syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] ERROR: Failed to send message!\n");
    syscall1(SYS_THREAD_EXIT, 1);
  }

  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Sent message successfully\n");

  // Receive it back
  Message msg;
  result = syscall2(SYS_RECV, port_id, (uint64_t) &msg);
  if (result < 0)
  {
    syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] ERROR: Failed to receive message!\n");
    syscall1(SYS_THREAD_EXIT, 1);
  }

  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Received message successfully!\n");
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Message ID: ");
  // TODO: Print the actual message ID (need number printing)
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[42]\n");

  // Test yielding
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Testing yield...\n");
  syscall0(SYS_THREAD_YIELD);
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Back from yield!\n");

  // Exit cleanly
  syscall1(SYS_DEBUG_PRINT, (uint64_t) "[USERSPACE] Test complete, exiting.\n");
  syscall1(SYS_THREAD_EXIT, 0);

  // Should never reach here
  for (;;)
    ;
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

  // thread_create(thread_receiver);
  // thread_create(thread_sender);
  serial_print("Creating userspace test thread...\n");

  // Allocate user stack from PMM and map it with PAGE_USER flag
  void *user_stack_phys = pmm_alloc_page();
  if (!user_stack_phys)
  {
    serial_print("ERROR: Failed to allocate user stack!\n");
    hcf();
  }

  // Map user stack at a high user address (below kernel space)
  uint64_t user_stack_virt = 0x00007FFFFFFFE000ULL;
  address_space_t *kernel_as = vmm_get_kernel_address_space();

  serial_print("  Mapping to virtual address: ");
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

  serial_print("  User stack mapped successfully\n");

  void *user_stack_top = (void *) (user_stack_virt + 0x1000);

  serial_print("  Entry point: ");
  serial_print_hex((uint64_t) user_program_entry);
  serial_print("\n");
  serial_print("  Stack top: ");
  serial_print_hex((uint64_t) user_stack_top);
  serial_print("\n");

  thread_create_user(user_program_entry, user_stack_top);
  serial_print("Userspace thread created\n\n");

  serial_print("Starting scheduler...\n\n");
  scheduler_start();

  // Should never reach here
  serial_print("\nERROR: Scheduler returned!\n");
  hcf();
}
