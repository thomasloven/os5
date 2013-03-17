#include <stdint.h>
#include <syscall.h>
int main();

void _init()
{
  _syscall_exit(main());
  for(;;);
}
