#include "kernel_limine.h"
#include "serial.h"
#include "thread.h"

static void hcf(void)
{
  for (;;)
  {
    __asm__ volatile("cli; hlt");
  }
}

void thread_a()
{
  for (;;)
  {
    serial_print("Thread A\n");
    thread_yield();
  }
}

void thread_b()
{
  for (;;)
  {
    serial_print("Thread B\n");
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

  serial_print("\nKernel initialized successfully!\n");
  serial_print("Attempting to run threads...\n");

  thread_init();

  thread_create(thread_a);
  thread_create(thread_b);

  scheduler_start();

  hcf();
}
