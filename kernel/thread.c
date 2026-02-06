#include "thread.h"
#include "gdt.h"

#include <stddef.h>

extern void context_switch(uint64_t *old_rsp, uint64_t new_rsp);
extern void jump_to_usermode(uint64_t entry, uint64_t user_stack);

#define MAX_THREADS 8
#define STACK_SIZE 4096
#define MAX_PORTS 16
#define MAX_MESSAGES 64

static Thread *current = NULL;
static Thread *thread_list = NULL;
static uint8_t stacks[MAX_THREADS][STACK_SIZE];
static Thread threads[MAX_THREADS];
static int thread_count = 0;

static Port ports[MAX_PORTS];
static int port_count = 0;

static Message message_pool[MAX_MESSAGES];
static int message_pool_next = 0;

void thread_init()
{
  current = NULL;
  thread_list = NULL;
  thread_count = 0;
  port_count = 0;
  message_pool_next = 0;

  for (int i = 0; i < MAX_MESSAGES; i++)
  {
    message_pool[i].next = NULL;
  }

  for (int i = 0; i < MAX_PORTS; i++)
  {
    ports[i].queue_head = NULL;
    ports[i].queue_tail = NULL;
    ports[i].message_count = 0;
    ports[i].blocked_thread = NULL;
  }
}

void thread_create(void (*entry)(void))
{
  if (thread_count >= MAX_THREADS)
  {
    return;
  }

  Thread *t = &threads[thread_count];
  uint8_t *stack = stacks[thread_count];
  uint64_t *sp = (uint64_t *) (stack + STACK_SIZE);

  // Align stack to 16 bytes
  sp = (uint64_t *) ((uint64_t) sp & ~0xF);

  // Setup initial stack frame
  *(--sp) = (uint64_t) entry; // Return address for 'ret' instruction
  *(--sp) = 0; // rbp
  *(--sp) = 0; // rbx
  *(--sp) = 0; // r12
  *(--sp) = 0; // r13
  *(--sp) = 0; // r14
  *(--sp) = 0; // r15

  t->rsp = (uint64_t) sp;
  t->kernel_rsp = (uint64_t) sp;
  t->user_rsp = 0;
  t->is_user_mode = 0;
  t->state = THREAD_RUNNING;
  t->next = NULL;
  t->waiting_on_port = NULL;

  if (!thread_list)
  {
    thread_list = t;
    t->next = t;
    current = t;
  } else
  {
    Thread *it = thread_list;
    while (it->next != thread_list)
    {
      it = it->next;
    }
    it->next = t;
    t->next = thread_list;
  }

  thread_count++;
}

extern void usermode_trampoline(void);


void thread_create_user(void (*entry)(void), void *user_stack)
{
  if (thread_count >= MAX_THREADS)
  {
    return;
  }

  Thread *t = &threads[thread_count];
  uint8_t *kernel_stack = stacks[thread_count];
  uint64_t *sp = (uint64_t *) (kernel_stack + STACK_SIZE);

  sp = (uint64_t *) ((uint64_t) sp & ~0xF);

  gdt_set_kernel_stack((uint64_t) sp);

  // Setup initial stack frame
  // When context_switch returns, it will jump to usermode_trampoline
  // The trampoline will load r12 and r13 into rdi/rsi and call jump_to_usermode

  *(--sp) = (uint64_t) usermode_trampoline; // return address
  *(--sp) = 0; // rbp
  *(--sp) = 0; // rbx
  *(--sp) = (uint64_t) entry; // r12 - will be loaded into rdi
  *(--sp) = (uint64_t) user_stack; // r13 - will be loaded into rsi
  *(--sp) = 0; // r14
  *(--sp) = 0; // r15

  t->rsp = (uint64_t) sp;
  t->kernel_rsp = (uint64_t) (kernel_stack + STACK_SIZE);
  t->user_rsp = (uint64_t) user_stack;
  t->is_user_mode = 1;
  t->state = THREAD_RUNNING;
  t->next = NULL;
  t->waiting_on_port = NULL;

  if (!thread_list)
  {
    thread_list = t;
    t->next = t;
    current = t;
  } else
  {
    Thread *it = thread_list;
    while (it->next != thread_list)
    {
      it = it->next;
    }
    it->next = t;
    t->next = thread_list;
  }

  thread_count++;
}

static Thread *find_next_runnable(Thread *start)
{
  Thread *candidate = start->next;

  while (candidate != start)
  {
    if (candidate->state == THREAD_RUNNING)
    {
      return candidate;
    }
    candidate = candidate->next;
  }

  if (start->state == THREAD_RUNNING)
  {
    return start;
  }

  return NULL;
}

void thread_yield()
{
  if (!current)
  {
    return;
  }

  Thread *prev = current;
  Thread *next = find_next_runnable(current);

  if (!next)
  {
    return;
  }

  current = next;

  // IMPORTANT: If switching to a user thread, update TSS.rsp0
  if (next->is_user_mode)
  {
    gdt_set_kernel_stack(next->kernel_rsp);
  }

  if (prev != next)
  {
    context_switch(&prev->rsp, current->rsp);
  }
}

void scheduler_start()
{
  if (!thread_list)
  {
    return;
  }

  current = thread_list;

  Thread *runnable = find_next_runnable(current);
  if (!runnable)
  {
    return;
  }

  current = runnable;

  // Set up TSS if this is a user thread
  if (current->is_user_mode)
  {
    gdt_set_kernel_stack(current->kernel_rsp);
  }

  uint64_t dummy = 0;
  context_switch(&dummy, current->rsp);
}

Thread *thread_current() { return current; }

static Message *message_alloc(void)
{
  if (message_pool_next >= MAX_MESSAGES)
  {
    return NULL;
  }

  Message *msg = &message_pool[message_pool_next++];
  msg->next = NULL;
  return msg;
}

static void message_free(Message *msg)
{
  // TODO: Return to a free list
  (void) msg;
}

Port *port_create()
{
  if (port_count >= MAX_PORTS)
  {
    return NULL;
  }

  Port *port = &ports[port_count++];
  port->queue_head = NULL;
  port->queue_tail = NULL;
  port->message_count = 0;
  port->blocked_thread = NULL;

  return port;
}

void port_destroy(Port *port)
{
  if (!port)
  {
    return;
  }

  Message *msg = port->queue_head;
  while (msg)
  {
    Message *next = msg->next;
    message_free(msg);
    msg = next;
  }

  if (port->blocked_thread)
  {
    port->blocked_thread->state = THREAD_RUNNING;
    port->blocked_thread->waiting_on_port = NULL;
  }

  port->queue_head = NULL;
  port->queue_tail = NULL;
  port->message_count = 0;
  port->blocked_thread = NULL;
}

// Port ID management for syscall interface
uint32_t port_to_id(Port *port)
{
  if (!port)
  {
    return 0;
  }

  // Calculate index in ports array
  // Add 1 so that 0 can be an invalid ID
  uint32_t index = (uint32_t) (port - ports);
  if (index >= MAX_PORTS)
  {
    return 0;
  }

  return index + 1;
}

Port *port_from_id(uint32_t id)
{
  // ID 0 is invalid
  if (id == 0 || id > MAX_PORTS)
  {
    return NULL;
  }

  // Convert back to index (subtract 1)
  return &ports[id - 1];
}

int send(Port *port, uint32_t id, uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3)
{
  if (!port)
  {
    return -1; // Invalid port
  }

  Message *msg = message_alloc();
  if (!msg)
  {
    return -2; // Out of messages
  }

  msg->id = id;
  msg->data[0] = d0;
  msg->data[1] = d1;
  msg->data[2] = d2;
  msg->data[3] = d3;
  msg->next = NULL;

  // Check if a thread is blocked waiting on this port
  if (port->blocked_thread)
  {
    Thread *blocked = port->blocked_thread;

    blocked->state = THREAD_RUNNING;
    blocked->waiting_on_port = NULL;
    port->blocked_thread = NULL;
  }

  if (port->queue_tail)
  {
    port->queue_tail->next = msg;
    port->queue_tail = msg;
  } else
  {
    port->queue_head = msg;
    port->queue_tail = msg;
  }

  port->message_count++;

  return 0; // Success
}

int recv(Port *port, Message *msg_out)
{
  if (!port || !msg_out)
  {
    return -1; // Invalid parameters
  }

  if (port->queue_head)
  {
    Message *msg = port->queue_head;
    port->queue_head = msg->next;

    if (!port->queue_head)
    {
      port->queue_tail = NULL;
    }

    port->message_count--;

    msg_out->id = msg->id;
    msg_out->data[0] = msg->data[0];
    msg_out->data[1] = msg->data[1];
    msg_out->data[2] = msg->data[2];
    msg_out->data[3] = msg->data[3];

    message_free(msg);

    return 0; // Success
  }

  if (!current)
  {
    return -2; // No current thread?
  }

  if (port->blocked_thread)
  {
    return -3; // Another thread already blocked
  }

  current->state = THREAD_BLOCKED;
  current->waiting_on_port = port;
  port->blocked_thread = current;

  thread_yield();

  // When we resume a message should be available, try to recieve again
  if (port->queue_head)
  {
    Message *msg = port->queue_head;
    port->queue_head = msg->next;

    if (!port->queue_head)
    {
      port->queue_tail = NULL;
    }

    port->message_count--;

    msg_out->id = msg->id;
    msg_out->data[0] = msg->data[0];
    msg_out->data[1] = msg->data[1];
    msg_out->data[2] = msg->data[2];
    msg_out->data[3] = msg->data[3];

    message_free(msg);

    return 0; // Success
  }

  // We shouldn't reach here...
  return -4; // Woke up without a message??
}
