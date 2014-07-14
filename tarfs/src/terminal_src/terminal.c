#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <syscalls.h>
#include <ctype.h>


int ansi_state = 0;
char ansi_buffer[256];
char *ansi_bufp;
uint32_t ansi_args[16];
uint32_t *ansi_argp;
int ansi_escape_char(char c)
{
  // 0 - Not part of a command
  // 1 - Argument
  // 2 - Argument separator
  // 3 - Function
  //
  // h, l, p not supported

  if(( c >= '0' && c <= '9'))
    return 1;
  if( c == ';' )
    return 2;
  if( c == 'H' || c == 'f' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 's' || c == 'u' || c == 'J' || c == 'K' || c == 'm' )
    return 3;
  return 0;
}
int count_saved_args()
{
  int i = 0;
  while(ansi_argp != &ansi_args[i] && i < 16)i++;
  return i;
}
int check_for_escape(struct terminal *t, char c)
{
  int i;
  switch(ansi_state)
  {
    case 0: // Haven't started
      if(c == '\033')
      {
        ansi_state = 1;
        return 1;
      }
      return 0;
    case 1: // Got \033, waiting for [
      if(c == '[')
      {
        ansi_state = 2;
        memset(ansi_buffer, 0, 256);
        ansi_bufp = ansi_buffer;
        *ansi_bufp++ = '[';

        memset(ansi_args, 0, 16*sizeof(uint32_t));
        ansi_argp = ansi_args;
        return 1;
      }
      return 0;
    case 2: // In a possible ansi command sequence
      // Save the characters to buffer
      strncat(ansi_bufp, &c, 1);

      switch(ansi_escape_char(c))
      {
        case 0: // Not part of an ansi command, give up
          ansi_state = 0;
          goto bail_print;
        case 1: // A digit. Wait for next
          return 1;
        case 2: // A semi colon. Save the previous data as an argument
          *ansi_argp++ = atoi(ansi_bufp);
          ansi_bufp += strlen(ansi_bufp);
          return 1;
        case 3: // End of an ansi command

          // Save data as argument if present
          if(strlen(ansi_bufp)>1)
          {
            *ansi_argp++ = atoi(ansi_bufp);
            ansi_bufp += strlen(ansi_bufp);
          }

          switch(c)
          {
            case 'H':
            case 'f':
              // Set position
              if(count_saved_args() != 2)
                goto bail_print;
              t->csr_row = ansi_args[0];
              t->csr_col = ansi_args[1];
              break;
            case 'A':
              // Up
              if(count_saved_args() > 1)
                goto bail_print;
              t->csr_row -= ansi_args[0]+1;
              if(t->csr_row < 0)
                t->csr_row = 0;
              break;
            case 'B':
              // Down
              if(count_saved_args() > 1)
                goto bail_print;
              t->csr_row += ansi_args[0]+1;
              if(t->csr_row >= t->rows)
                t->csr_row = t->rows;
              break;
            case 'C':
              // Forward
              if(count_saved_args() > 1)
                goto bail_print;
              t->csr_col += ansi_args[0]+1;
              if(t->csr_col >= t->cols)
                t->csr_col = t->cols;
              break;
            case 'D':
              // Backward
              if(count_saved_args() > 1)
                goto bail_print;
              t->csr_col -= ansi_args[0]+1;
              if(t->csr_col < 0)
                t->csr_col = 0;
              break;
            case 's':
              // Save position
              break;
            case 'u':
              // Restore position
              break;
            case 'J':
              // Erase display
              if(count_saved_args() != 1)
                goto bail_print;
              if(ansi_args[0] != 2)
                goto bail_print;
              memset(t->buffer, 0, 2*t->rows*t->cols);
              t->csr_row = 0;
              t->csr_col = 0;
              break;
            case 'K':
              // Erase line
              if(count_saved_args())
                goto bail_print;
              memset(&t->buffer[t->csr_row*t->cols + t->csr_col], 0, \
                  t->cols - t->csr_col);
              break;
            case 'm':
              i = 0;
              while(i < count_saved_args())
              {
                int arg = ansi_args[i];
                if(arg == 0)
                {
                  t->current_style = 0;
                  t->fg_color = 7;
                  t->bg_color = 0;
                }
                if(arg == 1)
                  t->current_style |= 1;
                if(arg == 7)
                  t->current_style |= 2;
                if(arg >= 30 && arg <= 37)
                {
                  t->fg_color = arg-30;
                }
                if(arg >= 40 && arg <= 47)
                {
                  t->bg_color = arg-40;
                }
                i++;
              }


          }
          ansi_state = 0;
          return 1;
      }
  }

bail_print:
  // What we got wasn't an ansi escape sequence
  // Give up and print everything
  i = 0;
  if(ansi_buffer[0])
    terminal_puts(t, ansi_buffer);
  while(ansi_buffer[i])
  {
    if(isprint((int)ansi_buffer[i]))
      terminal_puts(t, ansi_buffer);
    i++;
  }
  return 1;
}


void copy_data(struct terminal *t)
{
  if(!t)
    return;
  if(!t->buffer)
    return;

  uint32_t s1 = &t->buffer[t->buffer_size] - t->buf_ptr;
  int32_t s2 = t->cols*t->rows - s1;
  if(s1 > t->cols*t->rows)
    s1 = t->cols*t->rows;
  if(s2 < 0)
    s2 = 0;
  printf("S1: %x, S2: %x b: %x\n", s1, s2, t->buf_ptr);

  memcpy(vidmem, t->buf_ptr, s1*sizeof(uint16_t));
  memcpy(&vidmem[s1], t->buffer, s2*sizeof(uint16_t));
}

void copybuffer(struct terminal *t)
{
  if(!t)
    return;
  if(!t->buffer)
    return;

  copy_data(t);

  unsigned short position = t->csr_row*80 + t->csr_col;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(position & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)(position >> 8) & 0xF);
}


uint8_t get_style(uint8_t fg, uint8_t bg, uint8_t style)
{
  uint8_t stl;
  if(style & 0x2)
    stl = (fg << 4) + (bg & 0x0F);
  else
    stl = (bg << 4) + (fg & 0x0F);
  if(style & 0x1)
    stl |= 0x8;
  return stl;
}

void terminal_puts(struct terminal *t, char *c)
{
  if (!t)
    return;
  if(!t->buffer)
    return;
  while(*c)
  {
    if(check_for_escape(t, *c))
    {
      c++;
      continue;
    }
    switch(*c)
    {
      case '\n':
        t->csr_col = 0;
        t->csr_row++;
        break;

      default:
        if(isprint((int)*c))
        {
          // Write character
          uint8_t stl = get_style(t->fg_color, t->bg_color, t->current_style);
          // HÄR ÄR FELET!
          uint32_t pos = t->csr_row*t->cols + t->csr_col;
          pos += t->buf_ptr - t->buffer;
          pos %= t->buffer_size;
          t->buffer[pos] = (stl << 8) | *c;
          /* t->buf_ptr[t->csr_row*t->cols + t->csr_col] = (stl << 8) | *c; */

          // Advance pointer
          t->csr_col ++;
          
        }
    }
    while(t->csr_col > t->cols)
    {
      t->csr_col = 0;
      t->csr_row++;
    }

    while(t->csr_row >= t->rows)
    {
      // Scroll down one line
      t->buf_ptr = &t->buf_ptr[t->cols];

      uint32_t m = (t->rows-1)*t->cols;
      uint32_t pos = (m + (t->buf_ptr-t->buffer)) % t->buffer_size;
      memset(&t->buffer[pos], 0, t->cols*sizeof(uint16_t));
      
      t->csr_row--;
    }

    if(((uint32_t)t->buf_ptr -(uint32_t)t->buffer) >= t->buffer_size)
      t->buf_ptr = t->buffer;
    c++;
  }
}

void terminal_output_handler(uint32_t a)
{
  struct terminal *t = vterm[a];
  while(1)
  {
    int cnt = read(t->write_fd[0], t->output_buffer, 256);
    t->output_buffer[cnt] = '\0';
    terminal_puts(t, t->output_buffer);
    if(active_vterm == a)
      copybuffer(t);
  }
}

void terminal_init(int num, uint32_t rows, uint32_t cols, char **argv)
{
  struct terminal *t = vterm[num] = calloc(1, sizeof(struct terminal));
  t->rows = rows;
  t->cols = cols;
  t->fg_color=7;
  t->buffer_size = 2*rows*cols;
  t->buffer = calloc(t->buffer_size, sizeof(uint16_t));
  t->buf_ptr = t->buffer;
  t->output_buffer = calloc(256, 1);

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
  t->pid = fork();
  if(!t->pid)
  {
    dup2(t->read_fd[0], 0);
    dup2(t->write_fd[1], 1);
    dup2(t->write_fd[1], 2);
    close(t->read_fd[0]);
    close(t->read_fd[1]);
    close(t->write_fd[0]);
    close(t->write_fd[1]);
    execvp(argv[0], &argv[0]);
    exit(1);
  }
}

void screen_init()
{
  vidmem = (uint16_t *)_syscall_vidmem();
}
