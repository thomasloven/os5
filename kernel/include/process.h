#pragma once

#include <thread.h>
#include <elf.h>
#include <lists.h>

#define PROC_THREAD_NUM 256

#ifndef __ASSEMBLER__

struct thread_struct;

typedef struct process_struct
{
	uint32_t pid;
	uint32_t state;
	struct process_struct *parent;
	struct process_struct *child;
	struct process_struct *older_sibling;
	struct process_struct *younger_sibling;
	list_head_t threads;
	elf_t elf;
	uintptr_t pd;
	uint32_t exit_status;
} process_t;

#define PROC_STATE_RUNNING 0x1
#define PROC_STATE_WAITING 0x2
#define PROC_STATE_FINISHED 0x3

process_t *process_init(void *func);
void free_process(process_t *p);
void kill_process();
process_t *fork_process(process_t *parent, registers_t *r);
void switch_process(process_t *p);

#endif

