#include "pit.h"

#include "serial.h"

static volatile uint64_t pit_ticks = 0;
static uint32_t pit_frequency = 0;

static inline void outb(uint16_t port, uint8_t value) { __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port)); }

static inline uint8_t inb(uint16_t port)
{
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

static void pic_init()
{
  outb(PIC1_COMMAND, PIC_INIT);
  outb(PIC2_COMMAND, PIC_INIT);

  outb(PIC1_DATA, 32);
  outb(PIC2_DATA, 40);

  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);

  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);

  outb(PIC1_DATA, 0xFC);
  outb(PIC2_DATA, 0xFF);
}

void pit_init(uint32_t frequency)
{
  pic_init();

  uint32_t divisor = PIT_BASE_FREQ / frequency;

  if (divisor > 65535)
  {
    divisor = 65535;
  } else if (divisor < 1)
  {
    divisor = 1;
  }

  pit_frequency = PIT_BASE_FREQ / divisor;
  pit_ticks = 0;

  outb(PIT_COMMAND, PIT_CMD_CHANNEL0 | PIT_CMD_LSB_MSB | PIT_CMD_MODE2 | PIT_CMD_BINARY);
  outb(PIT_CHANNEL0, (uint8_t) (divisor & 0xFF));
  outb(PIT_CHANNEL0, (uint8_t) ((divisor >> 8) & 0xFF));

  __asm__ volatile("sti");
}

void pit_tick() { pit_ticks++; }

uint64_t pit_get_ticks() { return pit_ticks; }

void pit_sleep(uint32_t milliseconds)
{
  uint64_t target = pit_ticks + (milliseconds * pit_frequency / 1000);

  while (pit_ticks < target)
  {
    __asm__ volatile("hlt");
  }
}
