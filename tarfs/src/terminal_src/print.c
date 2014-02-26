#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include <stdlib.h>

struct terminal *maketerm(uint32_t rows, uint32_t cols)
{
  struct terminal *t = calloc(1, sizeof(struct terminal));
  t->rows = rows;
  t->cols = cols;
  t->current_style = 0x7;
  t->buffer = calloc(rows*cols, sizeof(uint16_t));
  return t;
}

void copybuffer(struct terminal *t)
{
  if(!t)
    return;
  if(!t->buffer)
    return;
  memcpy(vidmem, t->buffer, sizeof(uint16_t)*t->rows*t->cols);
}

void putch(struct terminal *t, char c, uint8_t style)
{
  if(!t)
    return;
  if(!t->buffer)
    return;

  t->buffer[t->csr_row*t->cols + t->csr_col] = (style << 8) | c;
}

void printch(struct terminal *t, char c)
{
  if(!t)
    return;
  putch(t, c, t->current_style);
  if(++t->csr_col > t->cols)
  {
    t->csr_col = 0;
    if(++t->csr_row > t->rows)
    {
      // Do scrolling
      t->csr_row = 0;
    }
  }
}
