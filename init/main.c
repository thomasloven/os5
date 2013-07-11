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
    while(strcmp(line, "exit\n"))
    {
      fgets(line, 128, stdin);
      fputs(line, stdout);
      if(!strcmp(line, "open\n"))
      {
        printf("%s", line);
        char *line2 = malloc(128);
        FILE *tarfile = fopen("/tarfs/hello.txt", "r");
        while(fgets(line2, 128, tarfile) != NULL)
          printf("Read from file: %s", line2);
        fclose(tarfile);
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
