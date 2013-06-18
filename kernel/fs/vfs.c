#include <vfs.h>
#include <trees.h>
#include <string.h>
#include <k_heap.h>
#include <lists.h>
#include <k_debug.h>

tree_t vfs_tree;
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

char *strdup(const char *s)
{
  char *d = kmalloc(strlen(s) + 1);
  if(d != 0)
    strcpy(d,s);
  return d;
}

void vfs_init()
{
  vfs_tree.size = 0;
  tree_node_t *root_tn = vfs_tree.root = kmalloc(sizeof(tree_node_t));

  init_tree_node(root_tn);
  vfs_entry_t *root = root_tn->item = kmalloc(sizeof(vfs_entry_t));

  root->name = strdup("[root]");
  root->node = 0;

  tree_node_t *child_tn = kmalloc(sizeof(tree_node_t));
  init_tree_node(child_tn);
  vfs_entry_t *child = child_tn->item = kmalloc(sizeof(vfs_entry_t));

  child->name = strdup("child_node");
  child->node = 0;

  tree_make_child(root_tn, child_tn);
  


}

void vfs_print_tree_rec(tree_node_t *node, int level)
{
  int i;
  debug("\n");
  for(i=0; i < level; i++)
    debug(" ");
  vfs_entry_t *entry = node->item;
  debug("%s", entry->name);
  list_t *l;
  for_each_in_list(&node->children, l)
  {
    vfs_print_tree_rec(list_entry(l, tree_node_t, siblings), level+1);
  }
}

void vfs_print_tree()
{
  vfs_print_tree_rec(vfs_tree.root, 0);
}
