#pragma once

#include <common.h>
#include <arch.h>
#include <pmm.h>
#include <lists.h>
#include <idt.h>

#ifndef __ASSEMBLER__

typedef struct thread_struct
{
	registers_t r;
	uint32_t tid;
	uint32_t state;
	list_t tasks;
	registers_t *kernel_thread;
} thread_t;

// Changing this will require chaning kvalloc and all calls to it and current_thread_info()
#define MAX_THREAD_STACK_SIZE PAGE_SIZE
#define MIN_THREAD_STACK_SIZE (sizeof(uint32_t) * 100)

#define THREAD_STACK_SIZE (MAX_THREAD_STACK_SIZE - sizeof(thread_t) + sizeof(registers_t))
#define THREAD_STACK_SPACE (THREAD_STACK_SIZE - sizeof(registers_t))


typedef union
{
	struct
	{
		uint8_t stack[THREAD_STACK_SIZE];
		uint8_t stack_bottom;
	};
	struct
	{
		uint8_t stackspace[THREAD_STACK_SPACE];
		thread_t tcb;
	};
} thread_info_t;

thread_info_t *current_thread_info();

#define current ((thread_t *)(&current_thread_info()->tcb))
#define stack_from_thinfo(info) ((uint32_t)&info->stack_bottom)
#define tcb_from_thinfo(info) ((thread_t *)(info->tcb))
#define thinfo_from_tcb(tcb) ((thread_info_t *)((uint32_t)(tcb)-THREAD_STACK_SPACE))
#define stack_from_tcb(tcb) (&tcb->tid)

thread_t *new_thread(void *func, uint8_t user);
registers_t *switch_kernel_thread(registers_t *r);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define schedule() asm volatile("int $" TOSTRING(INT_SCHEDULE))

#endif
