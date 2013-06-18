#include <vfs.h>

fs_node_t *root_node;

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
  if(node->read)
    return node->read(node, offset, size, buffer);
  else
    return 0;
}

uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
  if(node->write)
    return node->write(node, offset, size, buffer);
  else
    return 0;
}

void vfs_open(fs_node_t *node, uint32_t flags)
{
  if(node->open)
    node->open(node, flags);
}

void vfs_close(fs_node_t *node)
{
  if(node->close)
    node->close(node);
}

struct dirent *vfs_readdir(fs_node_t *node, uint32_t index)
{
  if(node->readdir)
    return node->readdir(node, index);
  else
    return (struct dirent *)0;
}

fs_node_t *vfs_finddir(fs_node_t *node, char *name)
{
  if(node->finddir)
    return node->finddir(node, name);
  else
    return (fs_node_t *)0;
}


void vfs_init();
