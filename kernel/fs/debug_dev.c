#include <vfs.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>
#include <sys/stat.h>



uint32_t write_debug(INODE node, void *buffer, uint32_t size, uint32_t offset)
{
  // print everything to screen
  ((char *)buffer)[size] = '\0';
  kdbg_puts(buffer);
  return size;
}

uint32_t stat_debug(INODE node, struct stat *st)
{
  st->st_dev = 0;
  st->st_ino = 1;
  st->st_mode = S_IFCHR;
  st->st_nlink = 0;
  st->st_uid = 0;
  st->st_gid = 0;
  st->st_rdev = 0;
  st->st_size = 0;
  st->st_atime = 0;
  st->st_mtime = 0;
  st->st_ctime = 0;
  return 0;
}

uint32_t isatty_debug(INODE node)
{
  if(!node)
    return 0;
  return 1;
}

vfs_driver_t debug_driver =
{
  0,
  0,
  0,
  write_debug,
  0,
  0,
  stat_debug,
  isatty_debug,
  0,
  0,
  0
};

INODE debug_dev_init()
{
  INODE node = calloc(1, sizeof(vfs_node_t));
  strcpy(node->name, "debug");
  node->d = &debug_driver;
  node->type = FS_CHARDEV;
  return node;
}
