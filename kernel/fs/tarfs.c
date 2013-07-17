#include <tarfs.h>
#include <vfs.h>
#include <k_debug.h>
#include <trees.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>


void tartree_add_node(tree_t *tree, tar_header_t *tar, char *path)
{
  char *p = strdup(path);
  char *i = p;

  uint32_t path_length = strlen(path);
  uint32_t path_levels = 0;

  // Tokenize path
  while(i < p + path_length)
  {
    if(*i == '/')
    {
      *i = '\0';
      path_levels++;
    }
    i++;
  }
  p[path_length] = '\0';
  // Directory entries end with /
  if(path[path_length-1] == '/')
  {
    path_levels--;
  }
  i = p + strlen(p) + 1;

  tree_node_t *node = tree->root;
  uint32_t current_level = 0;
  while(current_level < path_levels)
  {
    int found = 0;
    list_t *l;
    for_each_in_list(&node->children, l)
    {
      tree_node_t *tn = list_entry(l, tree_node_t, siblings);
      tarfs_entry_t *entry = tn->item;
      if(!strcmp(entry->name, i))
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
      tarfs_entry_t *n = new->item = malloc(sizeof(tarfs_entry_t));
      n->name = strdup(i);
      n->tar = tar;

      tree_make_child(node, new);
      node = new;
    }

    current_level++;
    i += strlen(i) + 1;
  }
}

size_t tar_size(unsigned char *size)
{
  // Returns the size of a file in a tar
  // (octal string)
  size_t ret = 0;
  int i;
  for(i = 0; i < 12 && size[i] >= '0'; i++)
  {
    ret = ret*8;
    ret = ret + ((uint32_t)size[i] - (uint32_t)'0');
  }
  return ret;
}

tree_t *build_tar_tree(tar_header_t *tar)
{
  tree_t *tree = malloc(sizeof(tree_t));

  char *s = strdup((const char *)tar->name);
  size_t i;
  for(i = 0; i < strlen(s); i++)
  {
    if(s[i] == '/')
    {
      s[i] = '\0';
      break;
    }
  }

  tree_node_t *root = tree->root = malloc(sizeof(tree_node_t));
  init_tree_node(root);
  tarfs_entry_t *n = malloc(sizeof(tarfs_entry_t));
  n->name = strdup(s);
  n->tar = tar;
  root->item = n;

  uint32_t offset = 0;

  while(tar->name[0])
  {
    tartree_add_node(tree, tar, (char *)tar->name);
    uint32_t size = tar_size(tar->size);
    uint32_t add = size + 512;
   if(size != 0) add = add + (512 - (size%512));
    offset = offset + add;
    tar = (tar_header_t *)((uint32_t)tar + add);
  }

  return tree;

}


uint32_t read_tar(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  tree_node_t *tn = (tree_node_t *)node->device;
  tarfs_entry_t *te = (tarfs_entry_t *)tn->item;
  tar_header_t *tar = te->tar;

  if(offset > tar_size(tar->size)) return 0;

  if((size + offset) > tar_size(tar->size))
    size = tar_size(tar->size) - offset;

  offset = offset + (uint32_t)tar + 512;

  memcpy(buffer, (void *)offset, size);
  if(size == tar_size(tar->size) - offset)
  {
    buffer[size] = EOF;
  }
  return size;
}

uint32_t write_tar(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  return 0;
}

void open_tar(fs_node_t *node, uint32_t flags)
{
  return;
}

void close_tar(fs_node_t *node)
{
  free(node);
  return;
}

struct dirent *tar_readdir(fs_node_t *node, uint32_t index)
{
  return 0;
}

fs_node_t *tar_finddir(fs_node_t *node, char *name)
{
  tree_node_t *tn = (tree_node_t *)node->device;
  list_t *l;
  for_each_in_list(&tn->children, l)
  {
    tree_node_t *cn = list_entry(l, tree_node_t, siblings);
    tarfs_entry_t *entry = cn->item;
    if(!strcmp(entry->name, name))
    {
      fs_node_t *node = malloc(sizeof(fs_node_t));
      memset(node, 0, sizeof(fs_node_t));
      strcpy(node->name, entry->name);
      node->write = &write_tar;
      node->read = &read_tar;
      node->open = &open_tar;
      node->close = &close_tar;
      node->readdir = &tar_readdir;
      node->finddir = &tar_finddir;
      node->device = (void *)cn;
      node->length = tar_size(entry->tar->size);
      node->mode = S_IFREG;
      return node;
    }
  }
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
  node->mode = S_IFDIR;

  tree_t *tar_tree = build_tar_tree(tar);
  node->device = (void *)tar_tree->root;

  return node;
}
