#include "terminal.h"
#include <stdint.h>
#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "keyboard.h"
#include <signal.h>

int kill(int pid, int sig);
char **command;

void switch_vterm(int num)
{
  if(num <0 || num > 4)
    return;
  if(!vterm[num])
    terminal_init(num, 25, 80, command);
  active_vterm = num;
  copybuffer(vterm[num]);
}

int main(int argc, char **argv)
{
  (void)argc;

  screen_init();
  keyboard_init();

  command = &argv[1];

  // Start with a single vterm
  switch_vterm(0);

  while(1)
  {
    char c, d;
    c = fgetc(stdin);
    d = keyboard_decode(c);
    if(kbd_ctrl)
    {
      switch(d)
      {
        case '1':
        case '2':
        case '3':
        case '4':
          switch_vterm(d-'1');
          continue;
          break;
        case 'c':
          kill(vterm[active_vterm]->pid, 9);
        default:
          ;
      }
    }
    if(d)
    {
      terminal_putch(vterm[active_vterm], d);
      copybuffer(vterm[active_vterm]);
      write(vterm[active_vterm]->read_fd[1], &d, 1);
    }
  }

  for(;;);
}
