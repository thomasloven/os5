#include <stdint.h>
#include <syscall.h>

int _start()
{
  int a;
  a = 5;

  _syscall_printf("Hello from, %s.", "init");

  _syscall_printf("\nI'm going to fork now.");
  if(_syscall_fork())
  {

    _syscall_printf("\n I am the parent! I have pid %x", _syscall_getpid());

  } else {

    _syscall_printf("\n  I am the child! I have pid %x", _syscall_getpid());

  }

  for(;;);
  return 0;
}
