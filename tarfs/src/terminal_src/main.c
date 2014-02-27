#include "terminal.h"
#include <stdint.h>
#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "keyboard.h"


int main(int argc, char **argv)
{
  (void)argc;
  _syscall_pdbg();

  screen_init();
  keyboard_init();

  // Start with a single vterm
  active_vterm = 0;
  terminal_init(0, 25, 80, &argv[1]);

  while(1)
  {
    char c;
    c = fgetc(stdin);
    if((c = keyboard_decode(c)))
    {
      terminal_putch(vterm[active_vterm], c);
      copybuffer(vterm[active_vterm]);
      write(vterm[active_vterm]->read_fd[1], &c, 1);
    }
  }

  for(;;);
}
