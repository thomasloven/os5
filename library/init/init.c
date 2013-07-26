#include <stdint.h>
#include <syscall.h>
#include <stdlib.h>
int main();
void _init_signal();

void _init(int argc, char **argv)
{
  _init_signal();
  exit(main(argc, argv));
  for(;;);
}
