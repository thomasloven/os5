#include "terminal.h"
#include <stdint.h>
#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "keyboard.h"

void terminal_output_handler(uint32_t a)
{
  while(1)
  {
    char c = fgetc(vterm[a]->write_pipe[0]);
    terminal_putch(vterm[a], c);
    if(active_vterm == a)
      copybuffer(vterm[a]);
  }
}

struct terminal *new_terminal(uint32_t rows, uint32_t cols, char **argv)
{
  struct terminal *t = calloc(1, sizeof(struct terminal));
  t->rows = rows;
  t->cols = cols;
  t->current_style = 0x7;
  t->buffer = calloc(rows*cols, sizeof(uint16_t));

  // Setup reading side (keyboard)
  pipe(t->read_fd);
  t->read_pipe[0] = fdopen(t->read_fd[0], "r");
  t->read_pipe[1] = fdopen(t->read_fd[1], "w");

  // Setup writing side (screen)
  pipe(t->write_fd);
  t->write_pipe[0] = fdopen(t->write_fd[0], "r");
  t->write_pipe[1] = fdopen(t->write_fd[1], "w");

  // Start running program
  if(!fork())
  {
    dup2(t->read_fd[0], 0);
    dup2(t->write_fd[1], 1);
    dup2(t->write_fd[1], 1);
    execvp(argv[0], &argv[0]);
    exit(1);
  }

  return t;
}

int main(int argc, char **argv)
{
  (void)argc;
  _syscall_pdbg();
  vidmem = (uint16_t *)_syscall_vidmem();

  // Start with a single vterm
  active_vterm = 0;
  vterm[0] = new_terminal(25,80, &argv[1]);
  // Keep a thread to take care of new output
  void *stack = calloc(1,1280);
  _syscall_thread((void *)((uint32_t)stack + 1280), (void *)&terminal_output_handler, 0);

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
