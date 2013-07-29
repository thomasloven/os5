#include <stdint.h>
#include <syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;

int main()
{
  setenv("HOME","/",1);
  setenv("PATH","/usr/sbin:/bin",1);

  while(1)
  {
    int pid = fork();
    if(pid)
    {
      _syscall_waitpid(pid);
    } else {
      execve("/bin/sh", 0, environ);
    }
  }

}
