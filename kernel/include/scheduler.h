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

void scheduler_sleep(thread_t *th, process_t *on);
void scheduler_wake(process_t *p, uint32_t value);
void scheduler_wake_thread(thread_t *th, uint32_t value);
void scheduler_sleep_thread(thread_t *th, thread_t *on);

#endif
