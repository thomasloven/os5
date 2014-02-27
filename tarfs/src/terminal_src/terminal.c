#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <syscalls.h>

void copybuffer(struct terminal *t)
{
  if(!t)
    return;
  if(!t->buffer)
    return;
  memcpy(vidmem, t->buffer, sizeof(uint16_t)*t->rows*t->cols);

  unsigned short position = t->csr_row*80 + t->csr_col;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(position & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)(position >> 8) & 0xF);
}

void terminal_putch(struct terminal *t, char c)
{
  if (!t)
    return;
  if(!t->buffer)
    return;
  switch(c)
  {
    case '\n':
      t->csr_row++;
      t->csr_col = 0;
      break;
    default:
      t->buffer[t->csr_row*t->cols + t->csr_col] = (t->current_style << 8) | c;
      t->csr_col++;
  }
  if(t->csr_col > t->cols)
  {
    t->csr_col = 0;
    if(++t->csr_row > t->rows)
    {
      t->csr_row = 0;
    }
  }
  
}

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

void terminal_init(int num, uint32_t rows, uint32_t cols, char **argv)
{
  struct terminal *t = vterm[num] = calloc(1, sizeof(struct terminal));
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
  //
  // Keep a thread to take care of new output
  void *stack = calloc(1,1280);
  _syscall_thread((void *)((uint32_t)stack + 1280), (void *)&terminal_output_handler, num);

  // Start running program
  if(!fork())
  {
    dup2(t->read_fd[0], 0);
    dup2(t->write_fd[1], 1);
    dup2(t->write_fd[1], 1);
    execvp(argv[0], &argv[0]);
    exit(1);
  }
}

void screen_init()
{
  vidmem = (uint16_t *)_syscall_vidmem();
}
