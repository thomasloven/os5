#pragma once
#include <stdint.h>

#define NUM_TERM 8

struct terminal
{
  uint32_t rows;
  uint32_t cols;
  uint32_t csr_row;
  uint32_t csr_col;
  uint16_t *buffer;
  uint8_t current_style;
};

uint16_t *vidmem;
struct terminal *vterm[NUM_TERM];
uint32_t active_vterm;
void printch(struct terminal *t, char c);
void copybuffer(struct terminal *t);
struct terminal *maketerm(uint32_t rows, uint32_t cols);

