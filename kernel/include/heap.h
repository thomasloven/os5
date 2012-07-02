#pragma once
#include <stdint.h>

typedef struct heap_header_struct
{
	struct heap_header_struct *prev, *next;
	uint32_t allocated : 1;
	uint32_t size : 31;
} chunk_t;

void *kmalloc(uint32_t size);
void kfree(void *a);

#define chunk_head(a) ((chunk_t *)((uint32_t)a-sizeof(chunk_t)))
#define chunk_data(c) ((void *)((uint32_t)c + sizeof(chunk_t)))

