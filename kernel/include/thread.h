#pragma once

#include <common.h>
#include <arch.h>
#include <pmm.h>
#include <lists.h>
#include <idt.h>
#include <process.h>

#ifndef __ASSEMBLER__

typedef struct thread_struct
{
	registers_t r;
	uint32_t tid;
	uint32_t state;
	list_t tasks;
	registers_t *kernel_thread;
	uint32_t interrupt_level;
	struct process_struct *proc;
	list_t proc_threads;
} thread_t;

#define TH_STATE_RUNNING	0x1
#define TH_STATE_WAITING	0x2
#define TH_STATE_FINISHED	0x3

// Changing this will require chaning kvalloc and all calls to it and
//	current_thread_info()
#define MAX_THREAD_STACK_SIZE PAGE_SIZE
#define MIN_THREAD_STACK_SIZE (sizeof(uint32_t) * 100)

#define THREAD_STACK_SIZE (MAX_THREAD_STACK_SIZE - sizeof(thread_t) \
	+ sizeof(registers_t))
#define THREAD_STACK_SPACE (THREAD_STACK_SIZE - sizeof(registers_t))


typedef struct thread_info_st
{
	uint8_t stackspace[THREAD_STACK_SPACE];
	thread_t tcb;
} thread_info_t;

thread_info_t *current_thread_info();

#define current ((thread_t *)(&current_thread_info()->tcb))
#define stack_from_thinfo(info) ((uintptr_t)&info->tcb.tid)
#define tcb_from_thinfo(info) ((thread_t *)(info->tcb))
#define thinfo_from_tcb(tcb) \
	((thread_info_t *)((uintptr_t)(tcb)-THREAD_STACK_SPACE))
#define stack_from_tcb(tcb) (&(tcb)->tid)

thread_t *alloc_thread();
thread_t *new_thread(uint8_t user, uint32_t userstack);
void free_thread(thread_t *th);
void bind_thread(thread_t *th, struct process_struct *p);
void unbind_thread(thread_t *th);
thread_t *add_thread(void *func, uint8_t user);
registers_t *switch_kernel_thread(registers_t *r);
thread_t *threads_init(void *func);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define schedule() __asm__ volatile("int $" TOSTRING(INT_SCHEDULE))

#endif
