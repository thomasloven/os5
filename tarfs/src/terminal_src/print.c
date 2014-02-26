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
