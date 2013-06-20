#include <stdint.h>
#include <syscall.h>
/*#include <heap.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  int a;
  a = 5;

  _syscall_printf("\nI'm going to fork now.");
  uint32_t pid = fork();
  if(pid)
  {
    _syscall_printf("\n I am the parent! I have pid %x", getpid());
    uint32_t retval = _syscall_wait(pid);
    _syscall_printf("\n Process %x returned %x", pid, retval);
    for(;;);

  } else {

    fopen("/dev/debug", "w");
    fopen("/dev/debug", "w");
    fopen("/dev/debug", "w");
    printf("  I am the child! I have pid %x\n", getpid());

    printf("Hello printf!!");
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
