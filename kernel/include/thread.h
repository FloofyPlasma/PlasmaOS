#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include <stdint.h>

typedef struct Thread
{
  uint64_t rsp;
  struct Thread *next;
} Thread;

void thread_init(void);
void thread_create(void (*entry)(void));
void thread_yield(void);

void scheduler_start(void);

#endif
