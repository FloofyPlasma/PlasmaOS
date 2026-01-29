#include "thread.h"

#include <stddef.h>

extern void context_switch(uint64_t *old_rsp, uint64_t new_rsp);

#define MAX_THREADS 2
#define STACK_SIZE 4096

static Thread *current = NULL;
static Thread *thread_list = NULL;

static uint8_t stacks[MAX_THREADS][STACK_SIZE];
static Thread threads[MAX_THREADS];
static int thread_count = 0;

void thread_init()
{
  current = NULL;
  thread_list = NULL;
  thread_count = 0;
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
  t->next = NULL;

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

void thread_yield()
{
  Thread *prev = current;
  current = current->next;
  context_switch(&prev->rsp, current->rsp);
}

void scheduler_start()
{
  if (!thread_list)
  {
    return;
  }

  current = thread_list;
  uint64_t dummy = 0;
  context_switch(&dummy, current->rsp);
}
