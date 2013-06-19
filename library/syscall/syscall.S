#include <syscall.h>

[global syscall_errno]
syscall_errno:
  db 0x0, 0x0, 0x0, 0x0
[global syscall_temp]
syscall_temp:
  db 0x0, 0x0, 0x0, 0x0

[global _syscall_fork]
DEF_SYSCALL(fork, 0x1)

[global _syscall_printf]
DEF_SYSCALL(printf, SYSCALL_PRINTF)

[global _syscall_getpid]
DEF_SYSCALL(getpid, SYSCALL_GETPID)

[global _syscall_exit]
DEF_SYSCALL(exit, SYSCALL_EXIT)

[global _syscall_unimpl]
DEF_SYSCALL(unimpl, SYSCALL_UNIMPL)

[global _syscall_wait]
DEF_SYSCALL(wait, SYSCALL_WAIT)

[global _syscall_sbrk]
DEF_SYSCALL(sbrk, SYSCALL_SBRK)


[global _syscall_open]
DEF_SYSCALL(open, SYSCALL_OPEN)

[global _syscall_write]
DEF_SYSCALL(write, SYSCALL_WRITE)
