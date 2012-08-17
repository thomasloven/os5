#include <k_syscalls.h>
#include <syscalls.h>
#include <k_debug.h>

KDEF_SYSCALL(putch, r)
{
	debug("%c",r->ebx);

	return r;
}

KDEF_SYSCALL(printf, r)
{
	debug((char *)r->ebx);

	return r;
}
