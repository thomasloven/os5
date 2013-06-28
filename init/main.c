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
    uint16_t retval = _syscall_wait(pid);
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
      printf("Returned %x!\n", _syscall_wait(pid));
    } else {
      printf("B\n");
      printf(" Childs Child.\n");
      _syscall_yield();
      /*printf("\n Childs child!");*/
      /*for(;;);*/
      return 0;
    }

    printf("after fork");

  }

  return 0x123;
}
