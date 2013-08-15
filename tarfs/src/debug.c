#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int execvp(const char *file, char *const argv[])
{
  int i = 0;
  int addpath = 1;
  while(file[i])
  {
    if(file[i] == '/')
    {
      addpath = 0;
      break;
    }
    i++;
  }

  if(addpath)
  {
    char *path = strdup(getenv("PATH"));
    if(!path) path = "/usr/sbin:/bin";
    char *brk;
    char *p = strtok_r(path, ":", &brk);
    while(p)
    {
      char *fullpath = malloc(strlen(p) + strlen(file) + 1);
      strcpy(fullpath, p);
      strcat(fullpath, "/");
      strcat(fullpath, file);

      execve(fullpath, argv, environ);

      p = strtok_r(NULL, ":", &brk);
    }
  }

  return -1;

}

int main(int argc, char **argv)
{
  _syscall_pdbg();
  execvp(argv[1], &argv[2]);
  return 1;
}
