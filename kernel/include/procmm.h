#pragma once
#include <stdint.h>
#include <lists.h>
#include <process.h>

#ifndef __ASSEMBLER__

typedef struct process_mem_struct
{
	struct process_struct *p;
	uintptr_t code_start;
	uintptr_t code_end;
	uintptr_t arg_start;
	uintptr_t arg_end;
	uintptr_t env_start;
	uintptr_t env_end;
	list_head_t mem;
} process_mem_t;

typedef struct mem_area_struct
{
	uintptr_t start;
	uintptr_t end;
	uint32_t flags;
	uint32_t users;
	list_t mem;
	uint32_t maxsize;
} mem_area_t;

#define MM_FLAG_READ (1<<0)
#define MM_FLAG_WRITE (1<<1)
#define MM_FLAG_SHARED (1<<2)
#define MM_FLAG_CANSHARE (1<<3)
#define MM_FLAG_COW (1<<4)
#define MM_FLAG_GROWSDOWN (1<<5)
#define MM_FLAG_AUTOGROW (1<<6)

process_mem_t *new_mem(process_t *p);

mem_area_t *new_area(uint32_t size, uint32_t flags);
void free_area(mem_area_t *area);
mem_area_t *procmm_find_above(uintptr_t address);
mem_area_t *procmm_find_below(uintptr_t address);
mem_area_t *procmm_find_containint(uintptr_t address);

uint32_t grow_area(mem_area_t *area, uint32_t size);

mem_area_t *split_area(mem_area_t *area, uintptr_t address);

#endif
