#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND 0x43

#define PIT_CMD_BINARY 0x00
#define PIT_CMD_BCD 0x01
#define PIT_CMD_MODE0 0x00
#define PIT_CMD_MODE1 0x02
#define PIT_CMD_MODE2 0x04
#define PIT_CMD_MODE3 0x06
#define PIT_CMD_MODE4 0x08
#define PIT_CMD_MODE5 0x0A
#define PIT_CMD_LATCH 0x00
#define PIT_CMD_LSB 0x10
#define PIT_CMD_MSB 0x20
#define PIT_CMD_LSB_MSB 0x30
#define PIT_CMD_CHANNEL0 0x00
#define PIT_CMD_CHANNEL1 0x40
#define PIT_CMD_CHANNEL2 0x80

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20
#define PIC_INIT 0x11

#define PIT_BASE_FREQ 1193182

void pit_init(uint32_t frequency);
uint64_t pit_get_ticks(void);
void pit_sleep(uint32_t milliseconds);

void pit_tick(void);

#endif
