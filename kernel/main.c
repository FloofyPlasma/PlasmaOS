#include <stddef.h>

#include "gdt.h"
#include "idt.h"
#include "kernel_limine.h"
#include "pit.h"
#include "serial.h"
#include "thread.h"

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

  serial_print("\nInitializing threading subsystem...\n");
  thread_init();

  serial_print("Creating IPC test port...\n");
  test_port = port_create();

  if (!test_port)
  {
    serial_print("ERROR: Failed to create port!\n");
    hcf();
  }

  serial_print("Port created successfully\n\n");

  thread_create(thread_receiver);
  thread_create(thread_sender);

  serial_print("Starting scheduler...\n\n");
  scheduler_start();

  // Should never reach here
  serial_print("\nERROR: Scheduler returned!\n");
  hcf();
}
