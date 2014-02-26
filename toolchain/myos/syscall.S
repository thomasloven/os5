#include <syscalls.h>


#ifndef KERNEL_MODE

.align 4
.global syscall_errno
syscall_errno:
  .int 0x00000000
.global syscall_temp
syscall_temp:
  .int 0x00000000

.global _syscall_exit
DEF_SYSCALL(exit, SYSCALL_EXIT)

.global _syscall_close
DEF_SYSCALL(close, SYSCALL_CLOSE)

.global _syscall_execve
DEF_SYSCALL(execve, SYSCALL_EXECVE)

.global _syscall_fork
DEF_SYSCALL(fork, SYSCALL_FORK)

.global _syscall_fstat
DEF_SYSCALL(fstat, SYSCALL_FSTAT)

.global _syscall_getpid
DEF_SYSCALL(getpid, SYSCALL_GETPID)

.global _syscall_isatty
DEF_SYSCALL(isatty, SYSCALL_ISATTY)

.global _syscall_kill
DEF_SYSCALL(kill, SYSCALL_KILL)

.global _syscall_link
DEF_SYSCALL(link, SYSCALL_LINK)

.global _syscall_lseek
DEF_SYSCALL(lseek, SYSCALL_LSEEK)

.global _syscall_open
DEF_SYSCALL(open, SYSCALL_OPEN)

.global _syscall_read
DEF_SYSCALL(read, SYSCALL_READ)

.global _syscall_sbrk
DEF_SYSCALL(sbrk, SYSCALL_SBRK)

.global _syscall_stat
DEF_SYSCALL(stat, SYSCALL_STAT)

.global _syscall_times
DEF_SYSCALL(times, SYSCALL_TIMES)

.global _syscall_unlink
DEF_SYSCALL(unlink, SYSCALL_UNLINK)

.global _syscall_wait
DEF_SYSCALL(wait, SYSCALL_WAIT)

.global _syscall_write
DEF_SYSCALL(write, SYSCALL_WRITE)

.global _syscall_waitpid
DEF_SYSCALL(waitpid, SYSCALL_WAITPID)

.global _syscall_yield
DEF_SYSCALL(yield, SYSCALL_YIELD)

.global _syscall_signal
DEF_SYSCALL(signal, SYSCALL_SIGNAL)

.global _syscall_pdbg
DEF_SYSCALL(pdbg, SYSCALL_PDBG)

.global _syscall_readdir
DEF_SYSCALL(readdir, SYSCALL_READDIR)

.global _syscall_dup
DEF_SYSCALL(dup, SYSCALL_DUP)

.global _syscall_dup2
DEF_SYSCALL(dup2, SYSCALL_DUP2)

.global _syscall_pipe
DEF_SYSCALL(pipe, SYSCALL_PIPE)

.global _syscall_thread
DEF_SYSCALL(thread, SYSCALL_THREAD)


.global _syscall_vidmem
DEF_SYSCALL(vidmem, SYSCALL_VIDMEM)

.global _syscall_printf
DEF_SYSCALL(printf, SYSCALL_PRINTF)

#endif
