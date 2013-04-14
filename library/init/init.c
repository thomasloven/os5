#include <stdint.h>
#include <syscall.h>
#include <signals.h>
int main();

void _init()
{
  init_signals();
  _syscall_exit(main());
  for(;;);
}
