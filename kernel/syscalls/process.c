#include <k_syscalls.h>
#include <syscalls.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>

KDEF_SYSCALL(fork, r)
{
	fork_process(current->proc, r);
	return r;
}

KDEF_SYSCALL(getpid, r)
{
	r->eax = current->proc->pid;
	return r;
}
