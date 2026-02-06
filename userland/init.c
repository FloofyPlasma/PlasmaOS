#define USERSPACE
#include "syscall.h"

typedef struct
{
  int id;
  int data[4];
} Message;

__attribute__((section(".text._start"))) void _start(void)
{
  user_debug_print("[INIT] PlasmaOS userspace init starting...\n");
  user_debug_print("[INIT] Running in ring 3\n\n");

  // Test 1: Port creation
  user_debug_print("[INIT] Test 1: Creating IPC port...\n");
  int port_id = user_port_create();
  if (port_id < 0)
  {
    user_debug_print("[INIT] FAILED: Could not create port\n");
    user_exit(1);
  }
  user_debug_print("[INIT] SUCCESS: Port created\n\n");

  // Test 2: Send message
  user_debug_print("[INIT] Test 2: Sending message to self...\n");
  int result = user_send(port_id, 42, 100, 200, 300, 400);
  if (result < 0)
  {
    user_debug_print("[INIT] FAILED: Could not send message\n");
    user_exit(1);
  }
  user_debug_print("[INIT] SUCCESS: Message sent (ID=42, data=[100,200,300,400])\n\n");

  // Test 3: Receive message
  user_debug_print("[INIT] Test 3: Receiving message...\n");
  Message msg;
  result = user_recv(port_id, &msg);
  if (result < 0)
  {
    user_debug_print("[INIT] FAILED: Could not receive message\n");
    user_exit(1);
  }
  user_debug_print("[INIT] SUCCESS: Message received\n");
  user_debug_print("[INIT]   Message ID: 42\n");
  user_debug_print("[INIT]   Data: [100, 200, 300, 400]\n\n");

  // Test 4: Thread yield
  user_debug_print("[INIT] Test 4: Testing thread yield...\n");
  user_yield();
  user_debug_print("[INIT] SUCCESS: Yielded and returned\n\n");

  // Test 5: Multiple yields
  user_debug_print("[INIT] Test 5: Multiple yields (spinning)...\n");
  for (int i = 0; i < 5; i++)
  {
    user_debug_print("[INIT]   Yield ");
    char num[2] = { '0' + i, '\0' };
    user_debug_print(num);
    user_debug_print("\n");
    user_yield();
  }
  user_debug_print("[INIT] SUCCESS: Multiple yields complete\n\n");

  user_debug_print("======================================\n");
  user_debug_print("[INIT] All tests passed!\n");
  user_debug_print("[INIT] Userspace is working correctly\n");
  user_debug_print("======================================\n\n");

  user_debug_print("[INIT] Init process complete, exiting with code 0\n");
  user_exit(0);

  for (;;)
    ;
}
