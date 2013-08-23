#include <vfs.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>
#include <sys/stat.h>

uint32_t write_debug(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  // print everything to screen
  kdbg_puts(buffer);
  return size;
}

fs_node_t *debug_dev_init()
{
  fs_node_t *node = malloc(sizeof(fs_node_t));
  memset(node, 0, sizeof(fs_node_t));
  strcpy(node->name, "debug");
  node->read = 0;
  node->write = &write_debug;
  node->open = 0;
  node->close = 0;
  node->readdir = 0;
  node->finddir = 0;
  node->flags = FS_CHARDEV;
  return node;
}
