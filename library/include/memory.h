#pragma once
#include <stdint.h>

#ifndef __ASSEMBLER__

#define memcopy(dst, src, len) ({\
	uint8_t *dst_ = (uint8_t *)(dst); \
	uint8_t *src_ = (uint8_t *)(src); \
	uint32_t len_ = (len); \
	for(;len_;len_--) *dst_++ = *src_++ ;})

#define memset(dst, val, len) ({\
	uint8_t *dst_ = (uint8_t *)(dst); \
	uint32_t len_ = (len); \
	for(;len_;len_--) *dst_++ = val ;})

#define PUSH(stack, type, item) \
	stack -= sizeof(type); \
	*((type *) stack) = item;

#define POP(stack, type) ({ \
	(type *)tmp = (type *)stack; \
	stack += sizeof(type); \
	tmp[0]; })

#endif
