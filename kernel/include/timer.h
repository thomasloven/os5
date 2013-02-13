#pragma once

#include <idt.h>

#define PIT_CMD_PORT  0x43
#define PIT_CH0_DATA_PORT 0x40

#define PIT_USE_CH0 0x00
#define PIT_ACCESS_LOHI 0x30
#define PIT_MODE_3  0x06

#define PIT_DIVISOR_CONST 1193180

#ifndef __ASSEMBLER__

void timer_init(uint32_t freq);
registers_t *timer_tick(registers_t *r);

#endif

