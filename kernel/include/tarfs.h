#pragma once

#include <stdint.h>
#include <vfs.h>

#ifndef __ASSEMBLER__
typedef struct
{
  unsigned char name[100];
  unsigned char mode[8];
  unsigned char uid[8];
  unsigned char gid[8];
  unsigned char size[12];
  unsigned char mtime[12];
  unsigned char checksum[8];
  unsigned char type[1];
  unsigned char linkname[100];
  unsigned char tar_indicator[6];
  unsigned char tar_version[2];
  unsigned char owner[32];
  unsigned char group[32];
  unsigned char device_major[8];
  unsigned char device_minor[8];
  unsigned char prefix[155];
}__attribute__((packed)) tar_header_t;

typedef struct
{
  char *name;
  tar_header_t *tar;
  INODE buffer;
  uint32_t users;
} tarfs_entry_t;

#define TAR_TYPE_FILE 0
#define TAR_TYPE_LINK '1'
#define TAR_TYPE_SYMLINK '2'
#define TAR_TYPE_CHARDEV '3'
#define TAR_TYPE_BLOCKDEV '4'
#define TAR_TYPE_DIR '5'
#define TAR_TYPE_FIFO '6'
#define TAR_TYPE_CONT '7'

INODE tarfs_init(tar_header_t *tar);

#endif
