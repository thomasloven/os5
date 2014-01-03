#pragma once
#include <stdint.h>

typedef struct dirent {
  uint32_t ino;
  char name[256];
} dirent;

typedef struct DIR {
  int fd;
  int cur_entry;
} DIR;

DIR *opendir(const char *dirname);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
int dirfd(DIR *dirp);
void rewinddir(DIR *dirp);
void seekdir(DIR *dirp, int pos);
int telldir(DIR *dirp);
