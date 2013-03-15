#include <stdint.h>
#include <syscall.h>
int main();

void _init()
{
  uint32_t exit_status = main();
  uint32_t pid = _syscall_getpid();
  _syscall_printf("\n Process %x ended with code %x", pid, exit_status);
  _syscall_exit(exit_status);
  for(;;);
}
