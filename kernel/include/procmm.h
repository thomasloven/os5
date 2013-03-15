#pragma once
#include <stdint.h>
#include <lists.h>
#include <process.h>

#ifndef __ASSEMBLER__


typedef struct mem_area_struct
{
  uintptr_t start;
  uintptr_t end;
  uint32_t flags;
  uint32_t type;
  list_t mem;
  list_t copies;
  process_t *owner;
} mem_area_t;

#define MM_FLAG_READ (1<<0)
#define MM_FLAG_WRITE (1<<1)
#define MM_FLAG_SHARED (1<<2)
#define MM_FLAG_CANSHARE (1<<3)
#define MM_FLAG_COW (1<<4)
#define MM_FLAG_GROWSDOWN (1<<5)
#define MM_FLAG_AUTOGROW (1<<6)
#define MM_FLAG_ADDONUSE (1<<7)

#define MM_TYPE_CODE 1
#define MM_TYPE_ARG 4
#define MM_TYPE_ENV 5
#define MM_TYPE_STACK 6

void init_procmm(process_t *p);

mem_area_t *new_area(process_t *p, uintptr_t start, uintptr_t end, uint32_t flags, uint32_t type);

mem_area_t *split_area(mem_area_t *ma, uintptr_t start, uintptr_t end);
mem_area_t *glue_area(mem_area_t *ma);

mem_area_t *find_including(process_t *p, uintptr_t addr);
mem_area_t *find_above(process_t *p, uintptr_t addr);

void share_area(process_t *copy, mem_area_t *ma);

void print_areas(process_t *p);

uint32_t procmm_handle_page_fault(uintptr_t address, uint32_t flags);

void procmm_fork(process_t *parent, process_t *child);
void procmm_exit(process_t *proc);
#endif
