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
#define FS_TYPE_MASK 0x7

struct vfs_node_st;
typedef struct vfs_node_st * INODE;

typedef struct dirent_st {
  INODE ino;
  char name[256];
} dirent_t;

struct vfs_fstat_st
{
  uint32_t size;
  uint32_t mode;
};

typedef struct vfs_driver_st
{
  int32_t (*open)(INODE, uint32_t);
  int32_t (*close)(INODE);
  uint32_t (*read)(INODE, void *, uint32_t, uint32_t);
  uint32_t (*write)(INODE, void *, uint32_t, uint32_t);
  int32_t (*link)(INODE, INODE, const char *);
  int32_t (*unlink)(INODE, const char *);
  int32_t (*stat)(INODE, struct stat *st);
  int32_t (*isatty)(INODE);
  int32_t (*mkdir)(INODE, const char *);
  dirent_t *(*readdir)(INODE, uint32_t);
  INODE (*finddir)(INODE, const char *);
} vfs_driver_t;

#define VFS_NAME_SZ 256

typedef struct vfs_node_st
{
  char name[VFS_NAME_SZ];
  INODE parent;
  INODE child;
  INODE older, younger;
  vfs_driver_t *d;
  uint32_t type;
  uint32_t length;
  uint32_t users;

  // Driver data
  void *data;
  uint32_t flags;
} vfs_node_t;

#define in_vfs_tree(node) ((node)->parent != 0)

typedef struct vfs_pipe
{
  char *buffer;
  uint32_t size;
  uint32_t read_pos;
  uint32_t write_pos;
  uint32_t readers;
  uint32_t writers;
  semaphore_t semaphore;
  list_head_t waiting;
} vfs_pipe_t;

#define PIPE_READ 0x1
#define PIPE_WRITE 0x2

int32_t vfs_open(INODE ino, uint32_t flags);
int32_t vfs_close(INODE ino);
uint32_t vfs_read(INODE ino, void *ptr, uint32_t length, uint32_t offset);
uint32_t vfs_write(INODE ino, void *ptr, uint32_t length, uint32_t offset);
int32_t vfs_link(INODE ino, INODE parent, const char *name);
int32_t vfs_unlink(INODE ino, const char *name);
int32_t vfs_stat(INODE ino, struct stat *st);
int32_t vfs_isatty(INODE ino);
int32_t vfs_mkdir(INODE ino, const char *name);
dirent_t *vfs_readdir(INODE ino, uint32_t num);
INODE vfs_finddir(INODE ino, const char *name);

void vfs_free(INODE ino);
void vfs_init();
INODE vfs_umount(const char *path);
INODE vfs_namei(const char *path);
INODE vfs_mount(const char *path, INODE root);

char *canonicalize_path(const char *path, const char *prefix);

void vfs_print_tree();

// Defined in fs/debug_dev.c
INODE debug_dev_init();
uint32_t new_pipe(uint32_t size, INODE *nodes);

#endif
