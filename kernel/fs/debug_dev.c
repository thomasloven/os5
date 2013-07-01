#include <vfs.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>

uint32_t read_debug(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  return 0;
}

uint32_t write_debug(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  debug("%s",buffer);
  return size;
}

void open_debug(fs_node_t *node, uint32_t flags)
{
  return;
}

void close_debug(fs_node_t *node)
{
  return;
}

fs_node_t *debug_dev_init()
{
  fs_node_t *node = malloc(sizeof(fs_node_t));
  memset(node, 0, sizeof(fs_node_t));
  strcpy(node->name, "debug");
  node->read = &read_debug;
  node->write = &write_debug;
  node->open = &open_debug;
  node->close = &close_debug;
  node->readdir = 0;
  node->finddir = 0;
  return node;
}
