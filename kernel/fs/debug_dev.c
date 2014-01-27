#include <vfs.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>
#include <sys/stat.h>


int32_t debug_open(INODE ino, uint32_t mode)
{
  (void)ino;
  (void)mode;
  return 0;
}
int32_t debug_close(INODE ino)
{
  (void)ino;
  return 0;
}

uint32_t debug_write(INODE node, void *buffer, uint32_t size, uint32_t offset)
{
  (void)node;
  (void)offset;
  char *buf = calloc(size+1, 1);
  memcpy(buf, buffer, size);
  kdbg_puts(buf);
  free(buf);
  return size;
}

int32_t debug_stat(INODE node, struct stat *st)
{
  (void)node;
  memset(st, 0, sizeof(struct stat));
  st->st_mode = S_IFCHR;
  return 0;
}

int32_t debug_isatty(INODE node)
{
  (void)node;
  return 1;
}

vfs_driver_t debug_driver =
{
  debug_open,
  debug_close,
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
