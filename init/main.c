#include <stdint.h>
#include <syscall.h>
/*#include <heap.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{

  printf("This is the init process.\n");

  uint16_t pid = fork();
  if(pid)
  {
    printf("Init parent. Pid:%x", getpid());
    fflush(stdout);
    uint16_t retval = _syscall_waitpid(pid);
    printf("Child (pid:%x) returned with %x", pid, retval);
    fflush(stdout);
    for(;;);

  } else {

    printf("  I am the child! I have pid %x\n", getpid());


    char line[128];

    printf("Reading from keyboard\nStop with 'exit'\n");
    fflush(stdout);
    while(strcmp(line, "exit"))
    {
      printf("\n$ ");
      fflush(stdout);
      fgets(line, 128, stdin);
      /* fputs(line, stdout); */
      int i;
      int len = (int)strlen(line);
      for(i = 0; i < len; i++)
      {
        if(line[i] == ' ')
          line[i] = '\0';
        if(line[i] == '\n')
          line[i] = '\0';
      }
      if(!strcmp(line, "open"))
      {
        char *fname = (char *)((uintptr_t)line + strlen(line) + 1);
        char *line2 = malloc(128);
        FILE *tarfile = fopen(fname, "r");
        if(tarfile)
        {
        while(fgets(line2, 128, tarfile) != NULL)
          printf("%s", line2);
        fclose(tarfile);
        } else {
          printf("No such file");
        }
      }
    }
    printf("Finished reading from keyboard\n");
    pid = fork();
    if(pid)
    {
      printf("A\n");
      printf("Returned %x!\n", _syscall_waitpid(pid));
    } else {
      printf("B\n");
      printf(" Childs Child.\n");
      return 0;
    }

    printf("after fork");

  }

  return 0x123;
}
