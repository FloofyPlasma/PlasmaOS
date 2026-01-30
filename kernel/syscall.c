#include "syscall.h"

#include "idt.h"
#include "serial.h"
#include "thread.h"

#include <stddef.h>

#define SYSCALL_VECTOR 0x80

extern void syscall_entry_asm(void);

void syscall_init()
{
  idt_set_gate(SYSCALL_VECTOR, (uint64_t) syscall_entry_asm, 0x08, IDT_TYPE_USER_INTERRUPT);
  serial_print("Syscalls: Registered interrupt 0x80 for syscalls\n");
}

int64_t syscall_handler(
    uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
{
  switch (num)
  {
    case SYS_SEND:
    {
      // arg1 = port_id (index into port table)
      // arg2 = message_id
      // arg3-arg6 = data[0-3]
      Port *port = port_from_id((uint32_t) arg1);
      if (!port)
        return -1;
      return send(port, (uint32_t) arg2, (uint32_t) arg3, (uint32_t) arg4, (uint32_t) arg5, (uint32_t) arg6);
    }

    case SYS_RECV:
    {
      // arg1 = port_id
      // arg2 = pointer to Message structure
      Port *port = port_from_id((uint32_t) arg1);
      if (!port)
        return -1;

      // TODO: Validate user pointer is in userspace memory
      Message *msg_out = (Message *) arg2;
      return recv(port, msg_out);
    }

    case SYS_THREAD_EXIT:
    {
      // arg1 = exit code
      serial_print("Thread exiting with code: ");
      serial_print_hex(arg1);
      serial_print("\n");

      // Mark thread as dead and yield
      Thread *current = thread_current();
      if (current)
      {
        current->state = THREAD_DEAD;
        thread_yield();
      }
      return 0;
    }

    case SYS_THREAD_YIELD:
    {
      thread_yield();
      return 0;
    }

    case SYS_PORT_CREATE:
    {
      Port *port = port_create();
      if (!port)
        return -1;
      return port_to_id(port);
    }

    case SYS_PORT_DESTROY:
    {
      Port *port = port_from_id((uint32_t) arg1);
      if (!port)
        return -1;
      port_destroy(port);
      return 0;
    }

    case SYS_DEBUG_PRINT:
    {
      // TODO: Validate string is in userspace memory
      const char *str = (const char *) arg1;
      serial_print("[USER] ");
      serial_print(str);
      return 0;
    }

    default:
    {
      serial_print("Unknown syscall: ");
      serial_print_hex(num);
      serial_print("\n");
      return -1;
    }
  }
}
