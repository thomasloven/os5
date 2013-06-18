#include <stdint.h>
#include <syscall.h>
#include <stdlib.h>
int main();
void _init_signal();

void _init()
{
  _init_signal();
  exit(main());
  for(;;);
}
