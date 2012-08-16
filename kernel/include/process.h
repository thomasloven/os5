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
}process_t;


process_t *process_init(struct thread_struct *th);
process_t *fork_process(struct thread_struct *th);
void switch_process(process_t *p);

#endif

