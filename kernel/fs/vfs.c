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
  vfs_root->parent = vfs_root;
  strcpy(vfs_root->name, "/");
}

void vfs_free(INODE ino)
{
  if(!ino->parent)
    free(ino);
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
uint32_t vfs_link(INODE ino, INODE parent, const char *name)
{
  if(ino->d->link)
    return ino->d->link(ino, parent, name);
  return 0;
}
uint32_t vfs_unlink(INODE ino, const char *name)
{
  if(ino->d->unlink)
    return ino->d->unlink(ino, name);
  return 0;
}
uint32_t vfs_stat(INODE ino, struct stat *st)
{
  if(ino->d->stat)
    return ino->d->stat(ino, st);
  return 0;
}
uint32_t vfs_isatty(INODE ino)
{
  if(ino->d->isatty)
    return ino->d->isatty(ino);
  return 0;
}
uint32_t vfs_mkdir(INODE ino, const char *name)
{
  if(ino->d->mkdir)
    return ino->d->mkdir(ino, name);
  return 0;
}
dirent_t *vfs_readdir(INODE ino, uint32_t num)
{
  if(ino->type & FS_MOUNT)
  {
    if(num == 0)
    {
      dirent_t *ret = calloc(1, sizeof(dirent_t));
      ret->ino = ino;
      strcpy(ret->name, ".");
      return ret;
    } else if(num == 1) {
      dirent_t *ret = calloc(1, sizeof(dirent_t));
      ret->ino = ino->parent;
      strcpy(ret->name, "..");
      return ret;
    }
  }
  if(ino->d->readdir)
    return ino->d->readdir(ino, num);
  return 0;
}
INODE vfs_finddir(INODE ino, const char *name)
{
  if(ino->type & FS_MOUNT)
  {
    if(!strcmp(name, "."))
    {
      return ino;
    } else if(!strcmp(name, "..")) {
      return ino->parent;
    }
  }
  if(ino->d->finddir)
    return ino->d->finddir(ino, name);
  if(ino->d->readdir)
  {
    // Backup solution
    int num = 0;
    dirent_t *de;
    while(1)
    {
      de = vfs_readdir(ino, num);
      if(!de)
        return 0;
      if(!strcmp(name, de->name))
        break;
      free(de->name);
      free(de);
      num++;
    }
    INODE ret = de->ino;
    free(de->name);
    free(de);
    return ret;
  }
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
      if(*path)
      {
        *path = *path - 1;
        *path[0] = '/';
      }
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

    if(root)
    {
    // Add node to mount tree if the goal is to mount something
      if(!next)
      {
        // Add if it doesn't exist and is the last part of the path
        if(pth)
          return 0;
        next = calloc(1, sizeof(vfs_node_t));
        strcpy(next->name, name);
        next->type = FS_DIRECTORY;
      }
      next->parent = current;
      next->older = current->child;
      current->child = next;
    }
    if(!next)
      return 0;

    vfs_free(current);
    current = next;
  }
  free(npath);

  if(root)
  {
    // Replace node in mount tree
    root->parent = current->parent;
    if(root->parent->child == current)
      root->parent->child = root;
    root->older = current->older;
    if(root->older)
      root->older->younger = current;
    root->younger = current->younger;
    if(root->younger)
      root->younger->older = current;
    strcpy(root->name, current->name);
    /* root->type |= FS_MOUNT; */
    if(current == vfs_root)
      vfs_root = root;

    free(current);
  }
  return current;
}

INODE vfs_umount(const char *path)
{
  char *npath = strdup(path);
  char *pth = &npath[1];
  INODE ino = vfs_find_root(&pth);
  if(!ino || pth)
  {
    free(npath);
    return 0;
  }
  if(ino->child)
  {
    free(npath);
    return 0;
  } else {
    // Remove node from mount tree
    if(ino->parent->child == ino)
      ino->parent->child = ino->older;
    if(ino->younger)
      ino->younger->older = ino->older;
    if(ino->older)
      ino->older->younger = ino->younger;
    free(npath);
    return ino;
  }
}

INODE vfs_namei(const char *path)
{
  return vfs_namei_mount(path, 0);
}

INODE vfs_mount(const char *path, INODE root)
{
  debug("[info] VFS mounting %s to %s\n", root->name, path);
  return vfs_namei_mount(path, root);
}
