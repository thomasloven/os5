#include <tarfs.h>
#include <vfs.h>
#include <k_debug.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t read_tar(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  return 0;
}

uint32_t write_tar(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  return 0;
}

void open_tar(fs_node_t *node, uint32_t flags)
{
}

void close_tar(fs_node_t *node)
{
}

struct dirent *tar_readdir(fs_node_t *node, uint32_t index)
{
  return 0;
}

fs_node_t *tar_finddir(fs_node_t *node, char *name)
{
  debug("In tar finddir");
  return 0;
}

fs_node_t *tarfs_init(tar_header_t *tar)
{
  fs_node_t *node = malloc(sizeof(fs_node_t));
  memset(node, 0, sizeof(fs_node_t));
  strcpy(node->name, "tarfs");
  node->write = &write_tar;
  node->read = &read_tar;
  node->open = &open_tar;
  node->close = &close_tar;
  node->readdir = &tar_readdir;
  node->finddir = &tar_finddir;

  node->device = (void *)tar;

  return node;
}
