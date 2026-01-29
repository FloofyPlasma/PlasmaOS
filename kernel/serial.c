#include "serial.h"

#include <stdint.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t value) { __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port)); }

static inline uint8_t inb(uint16_t port)
{
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));

  return value;
}

void serial_init(void)
{
  outb(COM1 + 1, 0x00); // Disable interrupts
  outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
  outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  outb(COM1 + 1, 0x00); //                  (hi byte)
  outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
  outb(COM1 + 4, 0x1E); // Set in loopback mode, test the serial chip
  outb(COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial
                        // returns same byte)

  // Check if serial is faulty (i.e: not same byte as sent)
  if (inb(COM1 + 0) != 0xAE)
  {
    return;
  }

  // If serial is not faulty set it in normal operation mode
  // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(COM1 + 4, 0x0F);
}

static int serial_is_transmit_empty(void) { return inb(COM1 + 5) & 0x20; }

void serial_putc(char c)
{
  while (!serial_is_transmit_empty())
  {
  }
  outb(COM1, c);
}

void serial_print(const char *str)
{
  while (*str)
  {
    serial_putc(*str++);
  }
}

void serial_print_hex(uint64_t value)
{
  const char hex_chars[] = "0123456789ABCFEF";
  char buffer[17];
  buffer[16] = '\0';

  for (int i = 15; i >= 0; i--)
  {
    buffer[i] = hex_chars[value & 0xF];
    value >>= 4;
  }

  serial_print("0x");
  serial_print(buffer);
}

void serial_print_dec(uint64_t value)
{
  if (value == 0)
  {
    serial_putc('0');
    return;
  }

  char buffer[21];
  int i = 20;
  buffer[i] = '\0';

  while (value > 0 && i > 0)
  {
    buffer[--i] = '0' + (value % 10);
    value /= 10;
  }

  serial_print(&buffer[i]);
}
