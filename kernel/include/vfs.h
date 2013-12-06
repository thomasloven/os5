#pragma once
#include <synch.h>
#include <lists.h>
#include <sys/stat.h>

#ifndef __ASSEMBLER__
#include <stdint.h>

#define VFS_FLAG_ISATTY 0x100000
#define VFS_FLAG_PIPE 0x110000

#define FS_FILE 0x1
#define FS_DIRECTORY 0x2
#define FS_CHARDEV 0x3
#define FS_BLOCKDEV 0x4
#define FS_PIPE 0x5
#define FS_SYMLINK 0x6
#define FS_MOUNT 0x8

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, char *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, char *);
typedef void (*open_type_t)(struct fs_node *, uint32_t);
typedef void (*close_type_t)(struct fs_node *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *);

struct vfs_node_st;
typedef struct vfs_node_st * INODE;

typedef struct dirent_st {
  uint32_t ino;
  char name[256];
} dirent_t;

struct vfs_fstat_st
{
  uint32_t size;
  uint32_t mode;
};

typedef struct vfs_driver_st
{
  uint32_t (*open)(INODE, uint32_t);
  uint32_t (*close)(INODE);
  uint32_t (*read)(INODE, void *, uint32_t, uint32_t);
  uint32_t (*write)(INODE, void *, uint32_t, uint32_t);
  uint32_t (*fstat)(INODE, struct stat *st);
  dirent_t *(*readdir)(INODE, uint32_t);
  INODE (*finddir)(INODE, const char *);
} vfs_driver_t;

#define VFS_NAME_SZ 256

typedef struct vfs_node_st
{
  char name[VFS_NAME_SZ];
  void *parent;
  void *child;
  void *older, *younger;
  vfs_driver_t *d;
  uint32_t type;
  void *data;
  uint32_t length;
} vfs_node_t;


typedef struct fs_node
{
  char name[256];
  uint32_t flags;
  uint32_t length;
  uint32_t inode;

  read_type_t read;
  write_type_t write;
  open_type_t open;
  close_type_t close;
  readdir_type_t readdir;
  finddir_type_t finddir;

  struct fs_node *ptr;
  void *data;
} fs_node_t;


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

uint32_t vfs_read(INODE node, void *buffer, uint32_t offset, uint32_t size);
uint32_t vfs_write(INODE node, void *buffer, uint32_t offset, uint32_t size);
uint32_t vfs_open(INODE node, uint32_t flags);
uint32_t vfs_close(INODE node);
dirent_t *vfs_readdir(INODE node, uint32_t index);
INODE vfs_finddir(INODE node, const char *name);

void vfs_init();
INODE vfs_namei(const char *path);
INODE vfs_mount(const char *path, INODE root);
fs_node_t *vfs_find(const char *path);

void vfs_print_tree();

// Defined in fs/debug_dev.c
fs_node_t *debug_dev_init();
fs_node_t *new_pipe(uint32_t size);

#endif
