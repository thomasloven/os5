#pragma once
#include <stdint.h>
#include <stdio.h>

#define NUM_TERM 8

struct terminal
{
  uint32_t rows;
  uint32_t cols;
  uint32_t csr_row;
  uint32_t csr_col;
  uint16_t *buffer;
  uint8_t current_style;
  int read_fd[2];
  FILE *read_pipe[2];
  int write_fd[2];
  FILE *write_pipe[2];
};

uint16_t *vidmem;
struct terminal *vterm[NUM_TERM];
uint32_t active_vterm;

void copybuffer(struct terminal *t);
struct terminal *new_terminal(uint32_t rows, uint32_t cols, char **argv);
void terminal_putch(struct terminal *t, char c);
void terminal_init(int num, uint32_t rows, uint32_t cols, char **argv);
void screen_init();
