#pragma once

#include <stdint.h>
#include <thread.h>
#include <lists.h>

#ifndef __ASSEMBLER__

struct thread_struct;


typedef struct process_mem_struct
{
  uintptr_t code_start;
  uintptr_t code_end;
  uintptr_t arg_start;
  uintptr_t arg_end;
  uintptr_t env_start;
  uintptr_t env_end;
  uintptr_t stack_limit;
  list_head_t mem;
} process_mem_t;

typedef struct process_struct
{
  uint32_t pid;
  uint32_t state;
  struct process_struct *parent;
  struct process_struct *child;
  struct process_struct *older_sibling;
  struct process_struct *younger_sibling;

  list_head_t threads;
  list_head_t proc_list;
  uintptr_t pd;

  process_mem_t mm;
} process_t;

#define PROC_STATE_RUNNING 0x1
#define PROC_STATE_WAITING 0x2
#define PROC_STATE_FINISHED 0x3

void switch_process(process_t *proc);
process_t *process_init(void (*func)(void));


process_t *alloc_process();
#endif
