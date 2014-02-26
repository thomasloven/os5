#include "terminal.h"
#include <stdint.h>
#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>

void prints(char *str)
{
  while(*str)
  {
    printch(vterm[active_vterm], *str);
    str++;
  }
  copybuffer(vterm[active_vterm]);
}

void myfunction(int argument)
{
  int i;
  for(i = 0; i < argument; i++)
  {
    prints("Thread\n");
    _syscall_yield();
  }
}

int main()
{
  _syscall_pdbg();
  vidmem = (uint16_t *)_syscall_vidmem();
  active_vterm = 0;
  vterm[0] = maketerm(25,80);
  prints("Hello, world!");
  void *stack = malloc(0x1000);
  _syscall_thread((void *)((uint32_t)stack + 0x1000), (void *)&myfunction, 3);
  int i;
  for(i = 0; i < 5; i++)
  {
    prints("Parent\n");
    _syscall_yield();
  }

  for(;;);
  while(1)
  {
    char c;
    c = fgetc(stdin);
    printch(vterm[active_vterm], c);
    copybuffer(vterm[active_vterm]);
  }
  for(;;);
}
