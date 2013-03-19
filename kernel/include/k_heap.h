#pragma once
#include <heap.h>
#include <stdint.h>

#ifndef __ASSEMBLER__

void *kmalloc(uint32_t size);
void *kcalloc(uint32_t size);
void *kvalloc(uint32_t size);
void kfree(void *a);


#endif

