#include <vfs.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>
#include <sys/stat.h>



uint32_t debug_write(INODE node, void *buffer, uint32_t size, uint32_t offset)
{
  char *buf = calloc(size+1, 1);
  memcpy(buf, buffer, size);
  kdbg_puts(buf);
  free(buf);
  return size;
}

uint32_t debug_stat(INODE node, struct stat *st)
{
  memset(st, 0, sizeof(struct stat));
  st->st_mode = S_IFCHR;
  return 0;
}

uint32_t debug_isatty(INODE node)
{
  return 1;
}

vfs_driver_t debug_driver =
{
  0,
  0,
  0,
  debug_write,
  0,
  0,
  debug_stat,
  debug_isatty,
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
