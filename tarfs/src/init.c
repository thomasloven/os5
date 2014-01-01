#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;
void _syscall_waitpid(uint32_t pid);

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
      char *command[] = {"/bin/sh"};
      execve(command[0], command, environ);
    }
  }

}
