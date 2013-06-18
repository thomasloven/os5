#pragma once


#ifndef __ASSEMBLER__
#include <stdint.h>

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, int32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, int32_t, uint8_t *);
typedef void (*open_type_t)(struct fs_node *, uint32_t);
typedef void (*close_type_t)(struct fs_node *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *);

typedef struct fs_node
{
  char name[256];
  void *device;
  uint32_t flags;
  uint32_t inode;
  uint32_t length;

  read_type_t read;
  write_type_t write;
  open_type_t open;
  close_type_t close;
  readdir_type_t readdir;
  finddir_type_t finddir;

  struct fs_node *ptr;
} fs_node_t;

struct dirent {
  uint32_t ino;
  char name[256];
};

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void vfs_open(fs_node_t *node, uint32_t flags);
void vfs_close(fs_node_t *node);
struct dirent *vfs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *vfs_finddir(fs_node_t *node, char *name);

void vfs_init();

#endif
