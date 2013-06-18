#include <stdint.h>
#include <syscall.h>
/*#include <heap.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int a;
  a = 5;

  _syscall_printf("\nI'm going to fork now.");
  uint32_t pid = fork();
  if(pid)
  {

    int fd = open("/dev/debug", "w");
    _syscall_printf("\n Opened file %d", fd);

    _syscall_printf("\n I am the parent! I have pid %x", getpid());
    uint32_t retval = _syscall_wait(pid);
    _syscall_printf("\n Process %x returned %x", pid, retval);
    for(;;);

  } else {

    _syscall_printf("\n  I am the child! I have pid %x", getpid());

    _syscall_printf("\n  errno before: %x", syscall_errno);
    _syscall_unimpl();
    _syscall_printf("\n  errno after: %x", syscall_errno);

    char *str = malloc(sizeof(char)*8);
    _syscall_printf("\n Allocated string at %x", str);
    str[0] = 'a';
    str[1] = 'b';
    str[2] = 'c';
    str[3] = 0;
    _syscall_printf("\n %s", str);

  }

  return 0x123;
}
