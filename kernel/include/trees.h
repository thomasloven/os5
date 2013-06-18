#pragma once
#ifndef __ASSEMBLER__

#include <stdint.h>
#include <lists.h>

typedef struct tree_node
{
  void *item;
  list_head_t children;
  struct tree_node *parent;
  list_t siblings;
} tree_node_t;

typedef struct
{
  uint32_t size;
  tree_node_t *root;
} tree_t;

#define init_tree(tree) \
  tree.size=0; \
  tree.root=(tree_node_t *)0;

#endif
