#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char **argv)
{
  DIR *dirp;
  if(argc > 1)
    dirp = opendir(argv[1]);
  else
    dirp = opendir("/");
  if(!dirp)
  {
    printf("ls: %s : No such directory\n", argv[1]);
    return 1;
  }
  rewinddir(dirp);
  struct dirent *de;
  printf("ls:\n");
  while((de = readdir(dirp)))
    printf("%s\n", de->name);
  closedir(dirp);
  return 0;
}
