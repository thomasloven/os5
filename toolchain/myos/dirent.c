#include <syscalls.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


DIR *opendir(const char *dirname)
{
  int fd = open(dirname, O_RDONLY, 0);
  if(fd == -1)
    return 0;

  DIR *dir = (DIR *)calloc(1, sizeof(DIR));
  dir->fd = fd;
  dir->cur_entry = -1;
  return dir;
}
int closedir(DIR *dirp)
{
  if(dirp->fd)
    return close(dirp->fd);
  return -1;
}
struct dirent *readdir(DIR *dirp)
{
  static struct dirent de;
  dirp->cur_entry++;
  int ret = _syscall_readdir(dirp->fd, dirp->cur_entry, &de);
  if(ret)
  {
    memset(&de, 0, sizeof(struct dirent));
    return 0;
  }
  return &de;
}
int dirfd(DIR *dirp)
{
  return dirp->fd;
}
void rewinddir(DIR *dirp)
{
  dirp->cur_entry = -1;
}
void seekdir(DIR *dirp, int pos)
{
  dirp->cur_entry = pos;
}
int telldir(DIR *dirp)
{
  return dirp->cur_entry;
}
