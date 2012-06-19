#pragma once
#include <stdint.h>

#ifndef __ASSEMBLER__

typedef uint8_t *va_list;
#define va_start(AP, LASTARG) \
	(AP = ((va_list)&(LASTARG)+VA_SIZE(LASTARG)))
#define va_arg(AP, TYPE) \
	(AP += __va_rounded_size(TYPE), \
	*((TYPE *)(AP-__va_rounded_size(TYPE))))
#define __va_rounded_size(TYPE) \
	(((sizeof(TYPE)+sizeof(uint32_t)-1)/sizeof(uint32_t))*sizeof(uint32_t))
#define VA_SIZE(TYPE) \
	((sizeof(TYPE) + sizeof(STACKITEM)-1) & ~(sizeof(STACKITEM)-1))
#define STACKITEM int
#define va_end(AP)

#endif
