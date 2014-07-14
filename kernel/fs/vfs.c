#include <vfs.h>
#include <trees.h>
#include <string.h>
#include <lists.h>
#include <k_debug.h>

#include <stdlib.h>
#include <errno.h>

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
  vfs_root = calloc(1, sizeof(vfs_node_t));
  vfs_root->parent = vfs_root;
  strcpy(vfs_root->name, "/");
}

void vfs_free(INODE ino)
{
  if(in_vfs_tree(ino))
    return;
  if(ino->users > 0)
    return;
  if(ino->d && ino->d->flush)
    ino->d->flush(ino);
  else
    free(ino);
}

int32_t vfs_open(INODE ino, uint32_t mode)
{
  ino->users++;
  if(ino->d->open)
    return ino->d->open(ino, mode);
  errno = EACCES;
  return -1;
}
int32_t vfs_close(INODE ino)
{
  ino->users--;
  if(ino->d->close)
    return ino->d->close(ino);
  errno = EBADF;
  return -1;
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
int32_t vfs_link(INODE ino, INODE parent, const char *name)
{
  if(ino->d->link)
    return ino->d->link(ino, parent, name);
  errno = EACCES;
  return -1;
}
int32_t vfs_unlink(INODE ino, const char *name)
{
  if(ino->d->unlink)
    return ino->d->unlink(ino, name);
  errno = EACCES;
  return -1;
}
int32_t vfs_stat(INODE ino, struct stat *st)
{
  if(ino->d->stat)
    return ino->d->stat(ino, st);
  errno = EBADF;
  return -1;
}
int32_t vfs_isatty(INODE ino)
{
  if(ino->d->isatty)
    return ino->d->isatty(ino);
  return 0;
}
int32_t vfs_mkdir(INODE ino, const char *name)
{
  if(ino->d->mkdir)
    return ino->d->mkdir(ino, name);
  errno = EACCES;
  return -1;
}
dirent_t *vfs_readdir(INODE ino, uint32_t num)
{
  if(!((ino->type & FS_TYPE_MASK) == FS_DIRECTORY))
    return 0;
  if(in_vfs_tree(ino))
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
  dirent_t *de = 0;
  if(ino->d->readdir)
    de = ino->d->readdir(ino, num);

  if(de && in_vfs_tree(ino))
  {
    // Replace inode with the one from the vfs tree if it exists.
    INODE n = ino->child;
    while(n)
    {
      if(!strcmp(de->name, n->name))
      {
        vfs_free(de->ino);
        de->ino = n;
        break;
      }
      n = n->older;
    }
  }
  return de;
}
INODE vfs_finddir(INODE ino, const char *name)
{
  if(!((ino->type & FS_TYPE_MASK) == FS_DIRECTORY))
    return 0;
  if(in_vfs_tree(ino))
  {
    if(!strcmp(name, "."))
    {
      return ino;
    } else if(!strcmp(name, "..")) {
      return ino->parent;
    }

    // Search through the mount tree first
    INODE n = ino->child;
    while(n)
    {
      if(!strcmp(name, n->name))
        return n;
      n = n->older;
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
      vfs_free(de->ino);
      free(de);
      num++;
    }
    INODE ret = de->ino;
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
      current = current->older;
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
  if(root && in_vfs_tree(root))
  {
    debug("[error] Tried to mount node already in mount tree: %s->%s\n", root->name, path);
  }
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
    {
      vfs_free(current);
      return 0;
    }

    if(root)
    {
    // Add node to mount tree if the goal is to mount something
      next->parent = current;
      next->older = current->child;
      current->child = next;
    }

    vfs_free(current);
    current = next;
  }
  free(npath);

  if(root)
  {
    // Replace node in mount tree
    root->parent = current->parent;
    current->parent = 0;
    if(root->parent->child == current) root->parent->child = root;
    root->older = current->older;
    if(root->older) root->older->younger = current;
    root->younger = current->younger;
    if(root->younger) root->younger->older = current;
    // Replace name
    strcpy(root->name, current->name);
    root->type |= FS_MOUNT;
    if(current == vfs_root)
      vfs_root = root;

    vfs_free(current);
    vfs_free(root);
    current = root;
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
    // TODO: Needs rewriting
    free(npath);
    return 0;
  } else {
    // Remove node from mount tree
    if(ino->parent->child == ino) ino->parent->child = ino->older;
    if(ino->younger) ino->younger->older = ino->older;
    if(ino->older) ino->older->younger = ino->younger;
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

char *canonicalize_path(const char *path, const char *prefix)
{
  // Tokenize path and push every piece onto a stack
  // Push everything onto another stack, remove all . and pop one item
  // for ..
  // Pop second stack into new path

  typedef struct pth_stack{
    char *name;
    struct pth_stack *prev;
   } stack_item;

  int length = strlen(path) + 1;
  if(prefix && path[0] != '/')
    length += strlen(prefix) + 1;
  char *pth = calloc(1, length);
  if(prefix && path[0] != '/')
  {
    strcat(pth, prefix);
    strcat(pth, "/");
  }
  strcat(pth, path);

  stack_item *i = 0, *j = 0, *k = 0;
  char *p = pth;
  for(p = strtok(p, "/"); p; p = strtok(NULL, "/"))
  {
    if(!strcmp(p, "."))
      continue;
    if(!strcmp(p, ".."))
    {
      // Pop
      i = j;
      j = j->prev;
      free(i->name);
      free(i);
      continue;
    }
    i = j;
    j = calloc(1, sizeof(stack_item));
    j->name = strdup(p);
    j->prev = i;
  }
  free(pth);

  // Turn stack around
  while(j)
  {
    i = k;
    k = calloc(1, sizeof(stack_item));
    k->name = j->name;
    k->prev = i;
    i = j;
    j = j->prev;
    free(i);
  }

  char *ret = calloc(1, strlen(path)+1);
  if(!k)
    strcat(ret, "/");
  while(k)
  {
    strcat(ret, "/");
    strcat(ret, k->name);
    i = k;
    k = k->prev;
    free(i->name);
    free(i);
  }
 return ret;
}
