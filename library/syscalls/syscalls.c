#include <syscalls.h>
#include <stdint.h>

DEF_SYSCALL1(putch, SYSCALL_PUTCH, char)

DEF_SYSCALL0(fork, SYSCALL_FORK)
DEF_SYSCALL0(getpid, SYSCALL_GETPID)
DEF_SYSCALL3(execv, SYSCALL_EXECV, uintptr_t, char **, char **)
