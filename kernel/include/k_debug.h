#pragma once
#include <stdint.h>

#define VIDMEM	0xC00B8000

#define SCRN_W	80
#define SCRN_H	25

#define VGA_STYLE(text,bckg) text | bckg

#define BLCK	0x0
#define LGRY	0x7

#ifndef __ASSEMBLER__

#define debug kdbg_printf

void kdbg_init();
void kdbg_scroll();
void kdbg_putch(uint8_t c, uint8_t style);
void kdbg_setpos(uint32_t pos);
void kdbg_printf(char *str, ...);

int kdbg_num2str(uint32_t num, uint32_t base, char *buf);
void kdbg_setclr(uint32_t style);

#ifndef NDEBUG

#define debug kdbg_printf
#define assert(n) ({if(!(n)){ \
	debug("\n WARNING! \n Assertion failed (%s)", #n); \
	debug(": %s line %d", __FILE__, __LINE__- 2); \
	for(;;);}})

#else

	#define debug(...) 
	#define assert(n) 

#endif

#endif

