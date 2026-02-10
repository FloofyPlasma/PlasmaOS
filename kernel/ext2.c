#include "ext2.h"
#include "ata.h"
#include "pmm.h"
#include "serial.h"

#include <stddef.h>

static ext2_superblock_t sb;
static ext2_group_desc_t *group_desc = NULL;
static uint32_t block_size;
static int ext2_ready = 0;

static int ext2_strlen(const char *str)
{
  int len = 0;
  while (str[len])
  {
    len++;
  }
  return len;
}

static int ext2_strncmp(const char *a, const char *b, int n)
{
  for (int i = 0; i < n; i++)
  {
    if (a[i] != b[i])
    {
      return a[i] - b[i];
    }
    if (a[i] == '\0')
    {
      return 0;
    }
  }

  return 0;
}

static int ext2_strcmp(const char *a, const char *b)
{
  while (*a && (*a == *b))
  {
    a++;
    b++;
  }

  return *(unsigned char *) a - *(unsigned char *) b;
}


static int ext2_read_block(uint32_t block_num, void *buffer)
{
  uint32_t sector = (block_num * block_size) / 512;
  uint32_t count = block_size / 512;
  return ata_read_sectors(0, sector, count, buffer);
}

static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode)
{
  if (inode_num == 0)
  {
    serial_print("ext2: Invalid inode 0\n");
    return -1;
  }

  uint32_t inode_index = inode_num - 1;

  uint32_t group = inode_index / sb.s_inodes_per_group;
  uint32_t local_index = inode_index % sb.s_inodes_per_group;

  uint32_t inode_table = group_desc[group].bg_inode_table;

  uint32_t inode_size = (sb.s_rev_level == 0) ? 128 : sb.s_inode_size;
  uint32_t offset = local_index * inode_size;

  uint32_t block = inode_table + (offset / block_size);
  uint32_t block_offset = offset % block_size;
  void *block_buffer = pmm_alloc_page();
  if (!block_buffer)
  {
    serial_print("ext2: Failed to allocate buffer\n");
    return 1;
  }

  if (ext2_read_block(block, block_buffer) != 0)
  {
    serial_print("ext2: Failed to read inode block\n");
    pmm_free_page(block_buffer);
    return -1;
  }

  uint8_t *src = (uint8_t *) block_buffer + block_offset;
  uint8_t *dst = (uint8_t *) inode;
  for (uint32_t i = 0; i < sizeof(ext2_inode_t); i++)
  {
    dst[i] = src[i];
  }

  pmm_free_page(block_buffer);
  return 0;
}

static int ext2_read_inode_data(ext2_inode_t *inode, void *buffer, uint32_t max_size)
{
  uint32_t size = inode->i_size;
  if (size > max_size)
  {
    serial_print("ext2: Warning - truncating file read\n");
    size = max_size;
  }

  uint8_t *buf = (uint8_t *) buffer;
  uint32_t remaining = size;
  uint32_t offset = 0;

  for (int i = 0; i < 12 && remaining > 0; i++)
  {
    if (inode->i_block[i] == 0)
    {
      break;
    }

    void *block_buffer = pmm_alloc_page();
    if (!block_buffer)
    {
      return -1;
    }

    if (ext2_read_block(inode->i_block[i], block_buffer) != 0)
    {
      pmm_free_page(block_buffer);
      return -1;
    }

    uint32_t to_copy = remaining > block_size ? block_size : remaining;
    uint8_t *src = (uint8_t *) block_buffer;
    for (uint32_t j = 0; j < to_copy; j++)
    {
      buf[offset++] = src[j];
    }

    remaining -= to_copy;
    pmm_free_page(block_buffer);
  }

  return size - remaining;
}

int ext2_init()
{
  serial_print("ext2: Initializing ext2 filesystem...\n");

  uint8_t sb_buffer[1024];
  if (ata_read_sectors(0, 2, 2, sb_buffer) != 0)
  {
    serial_print("ext2: Failed to read superblock\n");
    return -1;
  }

  uint8_t *src = sb_buffer;
  uint8_t *dst = (uint8_t *) &sb;
  for (uint32_t i = 0; i < sizeof(ext2_superblock_t); i++)
  {
    dst[i] = src[i];
  }
  if (sb.s_magic != EXT2_MAGIC)
  {
    serial_print("ext2: Invalid magic number: ");
    serial_print_hex(sb.s_magic);
    serial_print("\n");
    return -1;
  }

  block_size = 1024 << sb.s_log_block_size;

  serial_print("ext2: Found valid ext2 filesystem\n");
  serial_print("  Block size: ");
  serial_print_dec(block_size);
  serial_print("\n  Total blocks: ");
  serial_print_dec(sb.s_blocks_count);
  serial_print("\n  Total inodes: ");
  serial_print_dec(sb.s_inodes_count);
  serial_print("\n  Inodes per group: ");
  serial_print_dec(sb.s_inodes_per_group);
  serial_print("\n");

  uint32_t num_groups = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

  uint32_t bgdt_block = (block_size == 1024) ? 2 : 1;
  group_desc = pmm_alloc_page();
  if (!group_desc)
  {
    serial_print("ext2: Failed to allocate memory for group descriptors\n");
    return -1;
  }

  if (ext2_read_block(bgdt_block, group_desc) != 0)
  {
    serial_print("ext2: Failed to read block group descriptor table\n");
    pmm_free_page(group_desc);
    group_desc = NULL;
    return -1;
  }

  ext2_ready = 1;
  serial_print("ext2: Initialization complete\n");
  return 0;
}

void ext2_list_root(void)
{
  if (!ext2_ready)
  {
    serial_print("ext2: Filesystem not initialized\n");
    return;
  }

  serial_print("\nFiles in root directory:\n");

  ext2_inode_t root_inode;
  if (ext2_read_inode(EXT2_ROOT_INO, &root_inode) != 0)
  {
    serial_print("ext2: Failed to read root inode\n");
    return;
  }

  void *dir_data = pmm_alloc_page();
  if (!dir_data)
  {
    serial_print("ext2: Failed to allocate buffer\n");
    return;
  }

  int bytes_read = ext2_read_inode_data(&root_inode, dir_data, 4096);
  if (bytes_read < 0)
  {
    serial_print("ext2: Failed to read directory data\n");
    pmm_free_page(dir_data);
    return;
  }

  uint32_t offset = 0;
  while (offset < (uint32_t) bytes_read)
  {
    ext2_dirent_t *entry = (ext2_dirent_t *) ((uint8_t *) dir_data + offset);

    if (entry->inode == 0)
    {
      offset += entry->rec_len;
      continue;
    }

    serial_print("  ");
    char name_buf[256];
    for (int i = 0; i < entry->name_len && i < 255; i++)
    {
      name_buf[i] = entry->name[i];
    }
    name_buf[entry->name_len] = '\0';
    serial_print(name_buf);

    if (entry->file_type == EXT2_FT_DIR)
    {
      serial_print(" [DIR]");
    } else if (entry->file_type == EXT2_FT_REG_FILE)
    {
      serial_print(" [FILE]");
    }
    serial_print(" (inode ");
    serial_print_dec(entry->inode);
    serial_print(")\n");

    offset += entry->rec_len;
  }

  serial_print("\n");
  pmm_free_page(dir_data);
}


int ext2_read_file(const char *path, void *buffer, uint32_t max_size)
{
  if (!ext2_ready)
  {
    serial_print("ext2: Filesystem not initialized\n");
    return -1;
  }

  const char *filename = path;
  if (path[0] == '/')
  {
    filename = path + 1;
  }

  for (int i = 0; filename[i]; i++)
  {
    if (filename[i] == '/')
    {
      serial_print("ext2: Subdirectories not supported yet\n");
      return -1;
    }
  }

  ext2_inode_t root_inode;
  if (ext2_read_inode(EXT2_ROOT_INO, &root_inode) != 0)
  {
    return -1;
  }

  void *dir_data = pmm_alloc_page();
  if (!dir_data)
  {
    return -1;
  }

  int bytes_read = ext2_read_inode_data(&root_inode, dir_data, 4096);
  if (bytes_read < 0)
  {
    pmm_free_page(dir_data);
    return -1;
  }

  uint32_t target_inode = 0;
  uint32_t offset = 0;
  while (offset < (uint32_t) bytes_read)
  {
    ext2_dirent_t *entry = (ext2_dirent_t *) ((uint8_t *) dir_data + offset);

    if (entry->inode != 0)
    {
      if (ext2_strncmp(entry->name, filename, entry->name_len) == 0 && ext2_strlen(filename) == entry->name_len)
      {
        target_inode = entry->inode;
        break;
      }
    }

    offset += entry->rec_len;
  }

  pmm_free_page(dir_data);

  if (target_inode == 0)
  {
    serial_print("ext2: File not found: ");
    serial_print(filename);
    serial_print("\n");
    return -1;
  }

  ext2_inode_t file_inode;
  if (ext2_read_inode(target_inode, &file_inode) != 0)
  {
    return -1;
  }

  if ((file_inode.i_mode & 0xF000) != EXT2_S_IFREG)
  {
    serial_print("ext2: Not a regular file\n");
    return -1;
  }

  return ext2_read_inode_data(&file_inode, buffer, max_size);
}
