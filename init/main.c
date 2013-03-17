#include <stdint.h>
#include <syscall.h>

int main()
{
  int a;
  a = 5;

  _syscall_printf("Hello from, %s.", "init");

  _syscall_printf("\nI'm going to fork now.");
  uint32_t pid = _syscall_fork();
  if(pid)
  {

    _syscall_printf("\n I am the parent! I have pid %x", _syscall_getpid());
    uint32_t retval = _syscall_waitpid(pid);
    _syscall_printf("\n Process %x returned %x", pid, retval);
    for(;;);

  } else {

    _syscall_printf("\n  I am the child! I have pid %x", _syscall_getpid());

    _syscall_printf("\n  errno before: %x", syscall_errno);
    _syscall_unimpl();
    _syscall_printf("\n  errno after: %x", syscall_errno);

  }

  return 0x123;
}
