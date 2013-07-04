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
  fopen("/dev/debug", "w");
  fopen("/dev/debug", "w");
  fopen("/dev/debug", "w");

  printf("This is the init process.\n");

  uint16_t pid = fork();
  if(pid)
  {
    printf("Inint parent. Pid:%x", getpid());
    fflush(stdout);
    uint16_t retval = _syscall_waitpid(pid);
    printf("Child (pid:%x) returned with %x", pid, retval);
    fflush(stdout);
    for(;;);

  } else {

    printf("  I am the child! I have pid %x\n", getpid());

    fflush(stdout);
    pid = fork();
    if(pid)
    {
      printf("A\n");
      FILE *fp = fopen("/tmp/pipe", "r");
      char line[128];
      printf("Opened pipe, reading...\n");
      fgets(line, sizeof(line), fp);
      fputs(line, stdout);
      fflush(stdout);
      printf("Returned %x!\n", _syscall_waitpid(pid));
    } else {
      printf("B\n");
      printf(" Childs Child.\n");
      FILE *fp2 = fopen("/tmp/pipe", "w");
      printf("B opened pipe, writing...\n");
      fputs("Hello, pipe.\n", fp2);
      _syscall_yield();
      /*printf("\n Childs child!");*/
      /*for(;;);*/
      return 0;
    }

    printf("after fork");

  }

  return 0x123;
}
