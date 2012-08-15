#include <k_syscalls.h>
#include <arch.h>
#include <idt.h>
#include <k_debug.h>

syscall_t syscall_handlers[NUM_SYSCALLS];

registers_t *syscall_handler(registers_t *r)
{
	if(syscall_handlers[r->eax])
		r = syscall_handlers[r->eax](r);

	return r;
}

syscall_t register_syscall(uint32_t num, syscall_t handler)
{
	syscall_t old = syscall_handlers[num];
	syscall_handlers[num] = handler;
	return old;
}

void syscalls_init()
{
	
	int i;
	for (i=0; i < NUM_SYSCALLS; i++)
	{
		syscall_handlers[i] = 0;
	}

	register_int_handler(INT_SYSCALL, &syscall_handler);
}


