#include <vfs.h>
#include <trees.h>
#include <string.h>
#include <lists.h>
#include <k_debug.h>

#include <stdlib.h>

tree_t vfs_tree;

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  if(node->read)
    return node->read(node, offset, size, buffer);
  else
    return 0;
}

uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
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
  if(node->flags & FS_DIRECTORY && node->readdir)
    return node->readdir(node, index);
  else
    return (struct dirent *)0;
}

fs_node_t *vfs_finddir(fs_node_t *node, char *name)
{
  if(node->flags & FS_DIRECTORY && node->finddir)
    return node->finddir(node, name);
  else
    return (fs_node_t *)0;
}

void vfs_init()
{
  init_tree(vfs_tree);
  tree_node_t *root_tn = vfs_tree.root = malloc(sizeof(tree_node_t));

  init_tree_node(root_tn);
  vfs_entry_t *root = root_tn->item = malloc(sizeof(vfs_entry_t));

  root->name = strdup("[root]");
  root->node = 0;
}


fs_node_t *vfs_traverse(const char *path, fs_node_t *mountroot)
{
  tree_node_t *treenode = vfs_tree.root;
  vfs_entry_t *vfsentry = treenode->item;
  fs_node_t *node = vfsentry->node;
  fs_node_t *nextnode;

  char *name, *brk;
  char *npath = strdup(path);
  // For each part of the path...
  for(name = strtok_r(&npath[1], "/", &brk); \
      name; \
      name = strtok_r(0, "/", &brk))
  {
    int found = 0, foundmount = 0;

    // First search the file system
    if(node && node->flags & FS_DIRECTORY)
    {
      // Search file itself
      nextnode = node->finddir(node, name);
      if(nextnode)
      {
        found = 1;
      }
    }
    if(!found)
      nextnode = 0;

    // Then search mount tree
    if(treenode)
    {
      list_t *l;
      for_each_in_list(&treenode->children, l)
      {
        tree_node_t *tn = list_entry(l, tree_node_t, siblings);
        vfs_entry_t *ve = tn->item;
        if(!strcmp(ve->name, name))
        {
          // Found the next node in the mount tree
          foundmount = 1;
          treenode = tn;
          // Mounts override other file system
          if(ve->node)
            nextnode = ve->node;
          break;
        }
      }
    }

    // If we want to mount something, we keep building the tree as we go
    // along
    if(mountroot && !foundmount)
    {
      // Create a new tree node and vfs item
      tree_node_t *tn = malloc(sizeof(tree_node_t));
      init_tree_node(tn);
      vfs_entry_t *ve = tn->item = malloc(sizeof(vfs_entry_t));
      ve->name = strdup(name);
      nextnode = ve->node = 0;

      tree_make_child(treenode, tn);
      treenode = tn;
    }
    node = nextnode;
    nextnode = 0;
    // If the next step was not found, and we don't want to mount
    // anything: exit
    if(!mountroot && !node)
    {
      break;
    }
  }
  // The entire path has been traversed

  /* if(!node) */
  /* { */
    // If the last node was not found
    if(mountroot)
    {
      debug("[info] Mounting %x to %s\n", mountroot, path);
      // If we want to mount it
      vfs_entry_t *ve = treenode->item;
      ve->node = mountroot;
      node = mountroot;
    }
  /* } */

  free(npath);
  return node;
}

void vfs_mount(const char *path, fs_node_t *mountroot)
{
  vfs_traverse(path, mountroot);
}

fs_node_t *vfs_find(const char *path)
{
  fs_node_t *ret = vfs_traverse(path, 0);
  return ret;
}


void vfs_print_tree_rec(tree_node_t *node, int level)
{
  int i;
  debug("\n");
  for(i=0; i < level; i++)
    debug(" ");
  vfs_entry_t *entry = node->item;
  debug("%s", entry->name);
  if(entry->node)
  {
    fs_node_t *fn = entry->node;
    if(fn->name)
      debug("->%s", fn->name);
  }
  list_t *l;
  for_each_in_list(&node->children, l)
  {
    vfs_print_tree_rec(list_entry(l, tree_node_t, siblings), level+1);
  }
}

void vfs_print_tree()
{
  debug("[info]VFS tree start\n");
  vfs_print_tree_rec(vfs_tree.root, 0);
  debug("[info]VFS tree end\n");
}
