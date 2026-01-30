#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <stdint.h>

#define SYS_SEND 1
#define SYS_RECV 2
#define SYS_THREAD_EXIT 3
#define SYS_THREAD_YIELD 4
#define SYS_PORT_CREATE 5
#define SYS_PORT_DESTROY 6
#define SYS_MAP_MEMORY 7
#define SYS_UNMAP_MEMORY 8
#define SYS_DEBUG_PRINT 9

static inline uint64_t syscall0(uint64_t num)
{
  int64_t ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num) : "memory");
  return ret;
}


static inline int64_t syscall1(uint64_t num, uint64_t arg1)
{
  int64_t ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num), "D"(arg1) : "memory");
  return ret;
}

static inline int64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2)
{
  int64_t ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2) : "memory");
  return ret;
}

static inline int64_t syscall6(
    uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
{
  int64_t ret;
  register uint64_t r10 __asm__("r10") = arg4;
  register uint64_t r8 __asm__("r8") = arg5;
  register uint64_t r9 __asm__("r9") = arg6;
  __asm__ volatile("int $0x80"
      : "=a"(ret)
      : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
      : "memory");
  return ret;
}

#ifdef USERSPACE
static inline int user_send(uint32_t port_id, uint32_t msg_id, uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3)
{
  return syscall6(SYS_SEND, port_id, msg_id, d0, d1, d2, d3);
}

static inline int user_recv(uint32_t port_id, void *msg_out) { return syscall2(SYS_RECV, port_id, (uint64_t) msg_out); }

static inline void user_exit(int code)
{
  syscall1(SYS_THREAD_EXIT, code);
  __builtin_unreachable();
}

static inline void user_yield(void) { syscall0(SYS_THREAD_YIELD); }

static inline int user_port_create(void) { return syscall0(SYS_PORT_CREATE); }

static inline void user_debug_print(const char *str) { syscall1(SYS_DEBUG_PRINT, (uint64_t) str); }
#endif

void syscall_init(void);
int64_t syscall_handler(
    uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

#endif
