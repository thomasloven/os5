#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <syscalls.h>

int main(int argc, char **argv)
{
  (void)argc;
  _syscall_pdbg();
  execvp(argv[1], &argv[1]);
  return 1;
}
