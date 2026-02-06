#include "ata.h"
#include "serial.h"

#include <stddef.h>
#include <stdint.h>

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CONTROL 0x3F6

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA_LOW 0x03
#define ATA_REG_LBA_MID 0x04
#define ATA_REG_LBA_HIGH 0x05
#define ATA_REG_DRIVE_SELECT 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_STATUS_ERR (1 << 0) // Error
#define ATA_STATUS_DRQ (1 << 3) // Data request ready
#define ATA_STATUS_SRV (1 << 4) // Overlapped mode service request
#define ATA_STATUS_DF (1 << 5) // Drive fault
#define ATA_STATUS_RDY (1 << 6) // Ready
#define ATA_STATUS_BSY (1 << 7) // Busy

static uint16_t ata_base = ATA_PRIMARY_IO;
static uint16_t ata_control = ATA_PRIMARY_CONTROL;

static inline void outb(uint16_t port, uint8_t value) { __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port)); }

static inline uint8_t inb(uint16_t port)
{
  uint8_t value;
  __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

static inline void inw_rep(uint16_t port, void *buffer, uint32_t count)
{
  __asm__ volatile("rep insw" : "+D"(buffer), "+c"(count) : "d"(port) : "memory");
}

static inline void outw_rep(uint16_t port, const void *buffer, uint32_t count)
{
  __asm__ volatile("rep outsw" : "+S"(buffer), "+c"(count) : "d"(port) : "memory");
}

static int ata_wait_bsy()
{
  uint8_t status;
  for (int i = 0; i < 1000000; i++)
  {
    status = inb(ata_base + ATA_REG_STATUS);
    if (!(status & ATA_STATUS_BSY))
    {
      return 0;
    }
  }
  serial_print("ATA: Timeout waiting for BSY to clear\n");
  return -1;
}

static int ata_wait_drq()
{
  uint8_t status;
  for (int i = 0; i < 1000000; i++)
  {
    status = inb(ata_base + ATA_REG_STATUS);
    if (status & ATA_STATUS_DRQ)
    {
      return 0;
    }
    if (status & ATA_STATUS_ERR)
    {
      serial_print("ATA: Error bit set\n");
      return -1;
    }
  }
  serial_print("ATA: Timeout waiting for DRQ\n");
  return -1;
}

void ata_init()
{
  serial_print("ATA: Initializing primary bus...\n");

  outb(ata_base + ATA_REG_COMMAND, 0xA0);

  for (int i = 0; i < 4; i++)
  {
    inb(ata_control);
  }

  outb(ata_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

  if (ata_wait_bsy() != 0)
  {
    serial_print("ATA: No drive detected or timeout\n");
    return;
  }

  uint8_t status = inb(ata_base + ATA_REG_STATUS);
  if (status == 0)
  {
    serial_print("ATA: No drive present\n");
    return;
  }

  if (ata_wait_drq() != 0)
  {
    serial_print("ATA: Drive not ready\n");
    return;
  }

  uint16_t identify_data[256];
  inw_rep(ata_base + ATA_REG_DATA, identify_data, 256);

  serial_print("ATA: Primary master drive detected\n");
  serial_print("ATA: Initialziation complete\n");
}

int ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, void *buffer)
{
  if (drive > 1)
  {
    serial_print("ATA: Invalid drive number\n");
    return -1;
  }

  if (count == 0)
  {
    serial_print("ATA: Invalid sector count (0)\n");
    return -1;
  }

  if (ata_wait_bsy() != 0)
  {
    return -1;
  }
  uint8_t drive_select = (drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F);
  outb(ata_base + ATA_REG_DRIVE_SELECT, drive_select);

  outb(ata_base + ATA_REG_SECTOR_COUNT, count);

  outb(ata_base + ATA_REG_LBA_LOW, lba & 0xFF);
  outb(ata_base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(ata_base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);

  outb(ata_base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);
  uint16_t *buf = (uint16_t *) buffer;
  for (int i = 0; i < count; i++)
  {
    if (ata_wait_drq() != 0)
    {
      serial_print("ATA: Failed to read sector ");
      serial_print_dec(i);
      serial_print("\n");
      return -1;
    }

    inw_rep(ata_base + ATA_REG_DATA, buf, 256);
    buf += 256;
  }

  return 0;
}


int ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)
{
  if (drive > 1)
  {
    return -1;
  }

  if (count == 0)
  {
    return -1;
  }

  if (ata_wait_bsy() != 0)
  {
    return -1;
  }

  uint8_t drive_select = (drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F);
  outb(ata_base + ATA_REG_DRIVE_SELECT, drive_select);

  outb(ata_base + ATA_REG_SECTOR_COUNT, count);
  outb(ata_base + ATA_REG_LBA_LOW, lba & 0xFF);
  outb(ata_base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(ata_base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);

  outb(ata_base + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

  const uint16_t *buf = (const uint16_t *) buffer;
  for (int i = 0; i < count; i++)
  {
    if (ata_wait_drq() != 0)
    {
      return -1;
    }

    outw_rep(ata_base + ATA_REG_DATA, buf, 256);
    buf += 256;
  }

  return 0;
}
