#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void copybuffer(struct terminal *t)
{
  if(!t)
    return;
  if(!t->buffer)
    return;
  memcpy(vidmem, t->buffer, sizeof(uint16_t)*t->rows*t->cols);
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
