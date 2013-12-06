#include <vfs.h>
#include <trees.h>
#include <string.h>
#include <lists.h>
#include <k_debug.h>

#include <stdlib.h>

vfs_node_t *vfs_root;


void vfs_print_tree_rec(vfs_node_t *node, int level)
{
  if(node)
  {
    int i;
    for(i=0; i < level; i++)
      debug(".");
    debug("%s\n", node->name);
    node = node->child;
    while(node)
    {
      vfs_print_tree_rec(node, level+1);
      node = node->older;
    }
  }
}

void vfs_print_tree()
{
  debug("[info]VFS tree start\n");
  vfs_print_tree_rec(vfs_root, 0);
  debug("[info]VFS tree end\n");
}


void vfs_init()
{
  debug("[info] vfs_init2\n");
  vfs_root = calloc(1, sizeof(vfs_node_t));
  strcpy(vfs_root->name, "/");
}

uint32_t vfs_open(INODE ino, uint32_t mode)
{
  if(ino->d->open)
    return ino->d->open(ino, mode);
  return 0;
}
uint32_t vfs_close(INODE ino)
{
  if(ino->d->close)
    return ino->d->close(ino);
  return 0;
}
uint32_t vfs_read(INODE ino, void *ptr, uint32_t length, uint32_t offset)
{
  if(ino->d->read)
    return ino->d->read(ino, ptr, length, offset);
  return 0;
}
uint32_t vfs_write(INODE ino, void *ptr, uint32_t length, uint32_t offset)
{
  if(ino->d->write)
    return ino->d->write(ino, ptr, length, offset);
  return 0;
}
uint32_t vfs_fstat(INODE ino, struct stat *st)
{
  if(ino->d->fstat)
    return ino->d->fstat(ino, st);
  return 0;
}
dirent_t *vfs_readdir(INODE ino, uint32_t num)
{
  if(ino->d->readdir)
    return ino->d->readdir(ino, num);
  return 0;
}
INODE vfs_finddir(INODE ino, const char *name)
{
  if(ino->d->finddir)
    return ino->d->finddir(ino, name);
  return 0;
}

INODE vfs_find_root(char **path)
{
  // Find closest point in mount tree
  INODE current = vfs_root;
  INODE mount = current;
  char *name;
  while((name = strsep(path, "/")))
  {
    current = current->child;
    while(current)
    {
      if(!strcmp(current->name, name))
      {
        mount = current;
        break;
      }
      current = current->younger;
    }
    if(!current)
    {
      *path = *path - 1;
      *path[0] = '/';
      *path = name;
      break;
    }
  }

  return (INODE)mount;
}

INODE vfs_namei_mount(const char *path, INODE root)
{
  char *npath = strdup(path);
  char *pth = &npath[1];
  // Find closest point in mount tree
  INODE current = vfs_find_root(&pth);
  char *name;
  while(current && (name = strsep(&pth, "/")))
  {
    // Go through the path
    INODE next = vfs_finddir(current, name);
    if(!next)
      return 0;

    // Add node to mount tree if the goal is to mount something
    if(root)
    {
      // But only if this node is a directory
      if(!next || next->type != FS_DIRECTORY)
        return 0;
      next->parent = current;
      next->older = current->child;
      current->child = next;
    }
    current = next;
  }
  free(npath);

  if(root)
  {
    // Do the actual mounting
    // TODO current->mount();
    root->parent = current->parent;
    root->child = current->child;
    root->older = current->older;
    root->younger = current->younger;
    strcpy(root->name, current->name);
    memcpy(current, root, sizeof(vfs_node_t));
    current->type = FS_MOUNT;
  }
  return current;
}

INODE vfs_namei(const char *path)
{
  return vfs_namei_mount(path, 0);
}

INODE vfs_mount(const char *path, INODE root)
{
  return vfs_namei_mount(path, root);
}
