#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include <stdint.h>

typedef enum
{
  THREAD_RUNNING,
  THREAD_BLOCKED,
} ThreadState;

typedef struct Thread Thread;
typedef struct Port Port;
typedef struct Message Message;

#define MESSAGE_DATA_SIZE 4

typedef struct Message
{
  uint32_t id;
  uint32_t data[MESSAGE_DATA_SIZE];
  struct Message *next;
} Message;

#define MAX_MESSAGE_QUEUE 16

typedef struct Port
{
  Message *queue_head;
  Message *queue_tail;
  int message_count;
  Thread *blocked_thread;
} Port;

typedef struct Thread
{
  uint64_t rsp;
  ThreadState state;
  Port *waiting_on_port;
  Thread *next;
} Thread;

void thread_init(void);
void thread_create(void (*entry)(void));
void thread_yield(void);
void scheduler_start(void);

Port *port_create(void);
void port_destroy(Port *port);
int send(Port *port, uint32_t id, uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3);
int recv(Port *port, Message *msg_out);

Thread *thread_current(void);

#endif
