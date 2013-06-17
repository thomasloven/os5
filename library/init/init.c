#include <stdint.h>
#include <syscall.h>
#include <stdlib.h>
int main();

void _init()
{
  exit(main());
  for(;;);
}
