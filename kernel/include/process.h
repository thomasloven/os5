#pragma once

#include <thread.h>
#include <elf.h>

#define PROC_THREAD_NUM 256

#ifndef __ASSEMBLER__


typedef struct process_struct
{
	uint32_t pid;
	uint32_t state;
	struct process_struct *parent;
	struct process_struct *child;
	struct process_struct *older_sibling;
	struct process_struct *younger_sibling;
	struct thread_struct  *threads[PROC_THREAD_NUM];
	elf_t elf;
}process_t;

#endif

