#include <stdint.h>
#include "syscalls.h"
#include <stdlib.h>
int main();

extern char **environ;

char *__env[1] = { 0 };

void _init(uint32_t *args)
{
  /* _init_signal(); */

  int argc;
  char **argv;
  if(args)
  {
    argc = args[0];
    argv = (char **)args[1];
    environ = (char **)args[2];
  } else {
    argc = 0;
    argv = 0;
    environ = __env;
  }

  exit(main(argc, argv));
  for(;;);
}
