#pragma once
#include <thread.h>

#ifndef __ASSEMBLER__

typedef struct
{
  struct thread_struct *prev;
  struct thread_struct  *next;
} thread_list_t;

void scheduler_insert(thread_t *th);
void scheduler_remove(thread_t *th);
thread_t *scheduler_next();
void scheduler_init();

void scheduler_list();

#endif
