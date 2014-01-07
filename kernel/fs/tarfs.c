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
      tarfs_entry_t *n = new->item = malloc(sizeof(tarfs_entry_t));
      n->name = strdup(p);
      n->tar = tar;

      tree_make_child(node, new);
      node = new;
    }
  }
  free(s);
}

tree_t *build_tar_tree(tar_header_t *tar)
{
  tree_t *tree = malloc(sizeof(tree_t));

  tree_node_t *root = tree->root = malloc(sizeof(tree_node_t));
  init_tree_node(root);
  tarfs_entry_t *n = malloc(sizeof(tarfs_entry_t));

  char *s = strdup((const char *)tar->name);
  n->name = strdup(strtok(s, "/"));
  free(s);
  n->tar = tar;
  root->item = n;

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

INODE tar_finddir(INODE dir, const char *name);
dirent_t *tar_readdir(INODE node, uint32_t index)
{
  tree_node_t *tn = (tree_node_t *)node->data;
  if(index == 0)
  {
    dirent_t *de = calloc(1, sizeof(dirent_t));
    strcpy(de->name, ".");
    de->ino = node;
    return de;
  }
  if(index == 1)
  {
    dirent_t *de = calloc(1, sizeof(dirent_t));
    strcpy(de->name, "..");
    de->ino = tar_finddir(node, "..");
    return de;
  }
  index -= 1;
  list_t *l = tn->children.next;
  for_each_in_list(&tn->children, l)
  {
    index--;
    if(!index)
      break;
  }
  if(index)
  {
    return 0;
  }

  tree_node_t *cn = list_entry(l, tree_node_t, siblings);
  tarfs_entry_t *entry = cn->item;

  dirent_t *de = calloc(1, sizeof(dirent_t));
  strcpy(de->name, entry->name);
  de->ino = tar_finddir(node, entry->name);
  if(!de->ino)
  {
    debug(" Couldn't find %s\n", entry->name);
    return 0;
  }
  return de;
}

INODE tar_finddir(INODE dir, const char *name)
{
  tree_node_t *tn = (tree_node_t *)dir->data;
  list_t *l;
  tarfs_entry_t *entry = 0;
  tree_node_t *cn = 0;
  if(!strcmp(name, "."))
    return dir;
  if(!strcmp(name, ".."))
  {
    cn = tn->parent;
    entry = cn->item;
  } else {
    for_each_in_list(&tn->children, l)
    {
      cn = list_entry(l, tree_node_t, siblings);
      entry = cn->item;
      if(!strcmp(entry->name, name))
      {
        break;
      }
    }
    if(l == &tn->children)
      return 0;
  }
  if(entry)
  {
    INODE node = calloc(1, sizeof(vfs_node_t));
    strcpy(node->name, entry->name);
    node->d = &tarfs_driver;
    node->data = (void *)cn;
    sscanf((char *)&entry->tar->size, "%o", &node->length);
    if(entry->tar->type[0] == TAR_TYPE_DIR)
    {
      node->type = FS_DIRECTORY;
    }
    else
      node->type = FS_FILE;
    return node;
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
  tar_finddir
};

INODE tarfs_init(tar_header_t *tar)
{
  vfs_node_t *node = calloc(1,sizeof(vfs_node_t));
  strcpy(node->name, "tarfs");
  node->d = &tarfs_driver;
  node->type = FS_DIRECTORY;

  tree_t *tar_tree = build_tar_tree(tar);
  node->data = tar_tree->root;
  free(tar_tree);

  return node;
}
