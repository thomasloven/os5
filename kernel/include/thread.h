#pragma once

#include <arch.h>
#include <pmm.h>



typedef struct thread_struct
{
	registers_t r;
	uint32_t tid;
	uint32_t state;
} thread_t;

// Chaning this will require chaning kvalloc and all calls to it and current_thread_info()
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

#define current (current_thread_info()->tcb)

