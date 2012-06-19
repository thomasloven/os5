#pragma once
#include <stdint.h>

#ifndef __ASSEMBLER__

#define memcopy(dst, src, len) uint8_t *dst2 = dst; \
	uint8_t *src2 = src; \
	uint32_t len2 = len; \
	for(;len2;len2--) *dst2++ = *src2++

#define memset(dst, val, len) uint8_t *dst2 = dst; \
	uint32_t len2 = len; \
	for(;len2;len2--) *dst2++ = val

#endif
