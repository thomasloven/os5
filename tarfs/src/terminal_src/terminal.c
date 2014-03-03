#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <syscalls.h>
#include <ctype.h>

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

int ansi_state = 0;
char ansi_buffer[256];
char *ansi_bufp;
uint32_t ansi_args[16];
uint32_t *ansi_argp;
void terminal_putc(struct terminal *t, char c);

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
  while(ansi_buffer[i])
  {
    if(isprint((int)ansi_buffer[i]))
      terminal_putc(t, ansi_buffer[i]);
    i++;
  }
  return 1;
}

void terminal_putc(struct terminal *t, char c)
{
  char stl = 0;
  if(t->current_style & 0x2) // reverse
    stl = (t->fg_color<<4) + (t->bg_color & 0xF);
  else
    stl = (t->bg_color<<4) + (t->fg_color & 0xF);
  if(t->current_style & 0x1) // Bright
    stl |= 0x8;

  t->buffer[t->csr_row*t->cols + t->csr_col] = (stl << 8) | c;
  t->csr_col++;

  if(t->csr_col > t->cols)
  {
    t->csr_col = 0;
    if(++t->csr_row > t->rows)
    {
      // TODO Scroll buffer
      t->csr_row = 0;
    }
  }
  
}

void terminal_putch(struct terminal *t, char c)
{
  if (!t)
    return;
  if(!t->buffer)
    return;
  if(check_for_escape(t, c))
    return;
  switch(c)
  {
    case '\n':
      t->csr_row++;
      t->csr_col = 0;
      break;

    default:
      if(isprint((int)c))
        terminal_putc(t, c);
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
  t->fg_color=7;
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
  t->pid = fork();
  if(!t->pid)
  {
    dup2(t->read_fd[0], 0);
    dup2(t->write_fd[1], 1);
    dup2(t->write_fd[1], 2);
    execvp(argv[0], &argv[0]);
    exit(1);
  }
}

void screen_init()
{
  vidmem = (uint16_t *)_syscall_vidmem();
}
