#include <tarfs.h>
#include <vfs.h>
#include <k_debug.h>
#include <trees.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

vfs_driver_t tarfs_driver;

void tartree_add_node(tree_t *tree, tar_header_t *tar, char *path)
{
  char *p = strdup(path);
  char *s = p;
  tree_node_t *node = tree->root;

  p = strchr(p, '/');

  for(p = strtok(p, "/"); p; p= strtok(NULL, "/"))
  {
    int found = 0;
    list_t *l;
    for_each_in_list(&node->children, l)
    {
      tree_node_t *tn = list_entry(l, tree_node_t, siblings);
      tarfs_entry_t *entry = tn->item;
      if(!strcmp(entry->name, p))
      {
        found = 1;
        node = tn;
        break;
      }
    }
    if(!found)
    {
      tree_node_t *new = malloc(sizeof(tree_node_t));
      init_tree_node(new);
      tarfs_entry_t *n = new->item = calloc(1, sizeof(tarfs_entry_t));
      n->name = strdup(p);
      n->tar = tar;

      tree_make_child(node, new);
      node = new;
    }
  }
  free(s);
}

tree_t *build_tar_tree(tar_header_t *tar, INODE node)
{
  tree_t *tree = malloc(sizeof(tree_t));

  tree_node_t *root = tree->root = malloc(sizeof(tree_node_t));
  init_tree_node(root);
  tarfs_entry_t *n = calloc(1, sizeof(tarfs_entry_t));

  char *s = strdup((const char *)tar->name);
  n->name = strdup(strtok(s, "/"));
  free(s);
  n->tar = tar;
  root->item = n;
  n->buffer = node;
  n->users = 1;

  while(tar->name[0])
  {
    tartree_add_node(tree, tar, (char *)&tar->name);
    uint32_t size;
    sscanf((char *)&tar->size, "%o", &size);
    tar = (tar_header_t *)((uint32_t)tar + size + 512);
    if((uint32_t)tar % 512)
      tar = (tar_header_t *)((uint32_t)tar + 512 - ((uint32_t)tar%512));
  }

  return tree;
}

INODE tar_get_inode(tree_node_t *tn)
{
  tarfs_entry_t *entry = tn->item;
  if(entry->buffer)
  {
    entry->users++;
    return entry->buffer;
  }

  INODE node = entry->buffer = calloc(1, sizeof(vfs_node_t));
  strcpy(node->name, entry->name);
  node->d = &tarfs_driver;
  node->data = (void *)tn;
  sscanf((char *)&entry->tar->size, "%o", &node->length);
  if(entry->tar->type[0] == TAR_TYPE_DIR)
  {
    node->type = FS_DIRECTORY;
  } else {
    node->type = FS_FILE;
  }
  entry->users = 1;

  return node;
}


void flush_tar(INODE node)
{
  tree_node_t *tn = node->data;
  tarfs_entry_t *entry = tn->item;
  if(entry->buffer != node)
  {
    debug("[error] Free bad tars inode %x %x %s\n", node, entry->buffer, node->name);
  }
  entry->users--;
  if(entry->users == 0)
  {
    free(entry->buffer);
    entry->buffer = 0;
  }
}

uint32_t read_tar(INODE node, void *buffer, uint32_t size, uint32_t offset)
{
  tree_node_t *tn = (tree_node_t *)node->data;
  tarfs_entry_t *te = (tarfs_entry_t *)tn->item;
  tar_header_t *tar = te->tar;

  uint32_t tsz;
  sscanf((char *)&tar->size, "%o", &tsz);
  if(offset > tsz) return 0;

  if((size + offset) > tsz)
    size = tsz - offset;

  offset = offset + (uint32_t)tar + 512;

  memcpy(buffer, (void *)offset, size);
  if(size == tsz - offset)
  {
    ((char *)buffer)[size] = EOF;
  }
  return size;
}

uint32_t write_tar(INODE node, void *buffer, uint32_t size, uint32_t offset)
{
  (void)node;
  (void)buffer;
  (void)size;
  (void)offset;
  return 0;
}

int32_t open_tar(INODE node, uint32_t flags)
{
  (void)node;
  (void)flags;
  return 0;
}

int32_t close_tar(INODE node)
{
  (void)node;
  return 0;
}

dirent_t *tar_readdir(INODE node, uint32_t index)
{
  tree_node_t *tn = (tree_node_t *)node->data;
  dirent_t *de = calloc(1, sizeof(dirent_t));

  if(index == 0)
  {
    // Special case for .
    strcpy(de->name, ".");
    de->ino = tar_get_inode(tn);
    return de;
  }
  if(index == 1)
  {
    // Special case for ..
    strcpy(de->name, "..");
    de->ino = tar_get_inode(tn->parent);
    return de;
  }
  index--;

  list_t *l = 0;
  for_each_in_list(&tn->children, l)
  {
    index--;
    if(!index) break;
  }
  if(index)
  {
    // Reached end of directory
    free(de);
    return 0;
  }

  tree_node_t *cn = list_entry(l, tree_node_t, siblings);
  tarfs_entry_t *entry = cn->item;

  strcpy(de->name, entry->name);
  de->ino = tar_get_inode(cn);
  return de;
}

INODE tar_finddir(INODE dir, const char *name)
{
  tree_node_t *tn = (tree_node_t *)dir->data;
  list_t *l;
  tarfs_entry_t *entry = 0;
  tree_node_t *cn = 0;

  // Special cases for . and ..
  if(!strcmp(name, "."))
    return tar_get_inode(tn);
  if(!strcmp(name, ".."))
    return tar_get_inode(tn->parent);

  for_each_in_list(&tn->children, l)
  {
    cn = list_entry(l, tree_node_t, siblings);
    entry = cn->item;
    if(!strcmp(entry->name, name))
    {
      return tar_get_inode(cn);
    }
  }

  return 0;
}



vfs_driver_t tarfs_driver =
{
  open_tar,
  close_tar,
  read_tar,
  write_tar,
  0,
  0,
  0,
  0,
  0,
  tar_readdir,
  tar_finddir,
  flush_tar
};

INODE tarfs_init(tar_header_t *tar)
{
  vfs_node_t *node = calloc(1,sizeof(vfs_node_t));
  strcpy(node->name, "tarfs");
  node->d = &tarfs_driver;
  node->type = FS_DIRECTORY;

  tree_t *tar_tree = build_tar_tree(tar, node);
  node->data = tar_tree->root;
  free(tar_tree);

  return node;
}
