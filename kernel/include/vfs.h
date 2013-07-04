#pragma once
#include <synch.h>
#include <lists.h>

#ifndef __ASSEMBLER__
#include <stdint.h>


struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, char *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, char *);
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

typedef struct vfs_entry
{
  char *name;
  fs_node_t *node;
} vfs_entry_t;

typedef struct vfs_pipe
{
  char *buffer;
  uint32_t size;
  uint32_t read_pos;
  uint32_t write_pos;
  uint32_t users;
  semaphore_t semaphore;
  list_head_t waiting;

} vfs_pipe_t;

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
void vfs_open(fs_node_t *node, uint32_t flags);
void vfs_close(fs_node_t *node);
struct dirent *vfs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *vfs_finddir(fs_node_t *node, char *name);

void vfs_init();
void vfs_mount(char *path, fs_node_t *mount_root);
fs_node_t *vfs_find_node(const char *path);

void vfs_print_tree();

fs_node_t *debug_dev_init();
fs_node_t *new_pipe(uint32_t size);

#endif
