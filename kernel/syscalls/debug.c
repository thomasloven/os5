#include <k_syscalls.h>
#include <syscalls.h>
#include <k_debug.h>

KDEF_SYSCALL(putch, r)
{
	debug("%c",r->ebx);

	return r;
}
