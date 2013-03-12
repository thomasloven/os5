#include <syscall.h>

[global _syscall_fork]
DEF_SYSCALL(fork, 0x1)

[global _syscall_printf]
DEF_SYSCALL(printf, SYSCALL_PRINTF)

[global _syscall_getpid]
DEF_SYSCALL(getpid, SYSCALL_GETPID)
