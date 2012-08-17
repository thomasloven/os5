#pragma once

typedef uint32_t (*read_type_t)(struct fs_node_struct  *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct fs_node_struct *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t)(struct fs_node_struct *);
typedef void (*close_type_t)(struct fs_node_struct *);
typedef struct dirent_struct *(*readdir_type_t)(struct fs_node_struct *, uint32_t);
typedef struct fs_node_struct *(finddir_type_t)(struct fs_node_struct *, char *);

typedef struct dirent_struct
{
	char name[128];
	uint32_t ino;
} dirent_t;

typedef struct fs_node_struct
{
	char name[128];
	uint32_t flags;
	uint32_t ino;
	uint32_t device;
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
} fs_node_t;

#define FS_FILE 0x1
#define FS_DIRECTORY 0x2
#define FS_CHARDEVICE 0x3
#define FS_BLOCKDEVICe 0x4
#define FS_PIPE 0x5
#define FS_SYMLINK 0x6
#define FS_MOUNTPOINT 0x8

