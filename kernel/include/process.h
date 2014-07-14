#pragma once

#include <stdint.h>
#include <thread.h>
#include <lists.h>
#include <synch.h>
#include <vfs.h>

#include <signal.h>

#ifndef __ASSEMBLER__

#define NUM_FILEDES 256
#define NUM_SIGNALS 32

#define PROC_FLAG_DEBUG 0x1

struct thread_struct;


typedef struct process_mem_struct
{
  uintptr_t code_start;
  uintptr_t code_end;
  uintptr_t code_entry;
  uintptr_t arg_start;
  uintptr_t arg_end;
  uintptr_t env_start;
  uintptr_t env_end;
  uintptr_t data_end;
  uintptr_t stack_limit;
  list_head_t mem;
} process_mem_t;

typedef struct
{
  INODE ino;
  uint32_t offset;
  uint32_t flags;
  uint32_t users;
} file_desc_t;

#define fd_get(fd) { (fd)->users++; }
#define fd_put(fd) { (fd)->users--; if(!(fd)->users)free(fd); }

typedef struct process_struct
{
  uint32_t pid;
  uint32_t state;
  uint32_t flags;
  struct process_struct *parent;
  struct process_struct *child;
  struct process_struct *older_sibling;
  struct process_struct *younger_sibling;

  char *cmdline;

  list_head_t threads;
  list_head_t proc_list;
  uintptr_t pd;
  semaphore_t pd_sem;

  list_head_t waiting;
  uint32_t exit_code;

  process_mem_t mm;

  file_desc_t *fd[NUM_FILEDES];

  sig_t signal_handler[NUM_SIGNALS];
  uint8_t signal_blocked[NUM_SIGNALS];
  list_head_t signal_queue;
} process_t;

typedef struct
{
  uint32_t sig;
  uint32_t sender;
  list_head_t queue;
} signal_t;

#define PROC_STATE_RUNNING 0x1
#define PROC_STATE_WAITING 0x2
#define PROC_STATE_FINISHED 0x3

void switch_process(process_t *proc);
process_t *process_init(void (*func)(void));


process_t *get_process(uint32_t pid);
process_t *alloc_process();
process_t *fork_process();
void free_process(process_t *p);
void exit_process(process_t *proc, uint32_t exit_code);

#endif
