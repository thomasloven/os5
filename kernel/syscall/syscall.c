#include <k_syscall.h>
#include <stdint.h>
#include <arch.h>
#include <idt.h>
#include <lists.h>

#include <k_debug.h>
#include <string.h>
#include <syscalls.h>

syscall_t syscall_handlers[NUM_SYSCALLS];

void syscall_init()
{
  memset(syscall_handlers, 0 , NUM_SYSCALLS*sizeof(syscall_t *));
  register_int_handler(INT_SYSCALL, syscall_handler);

  KREG_SYSCALL(exit, SYSCALL_EXIT);
  KREG_SYSCALL(close, SYSCALL_CLOSE);
  KREG_SYSCALL(execve, SYSCALL_EXECVE);
  KREG_SYSCALL(fork, SYSCALL_FORK);
  KREG_SYSCALL(fstat, SYSCALL_FSTAT);
  KREG_SYSCALL(getpid, SYSCALL_GETPID);
  KREG_SYSCALL(isatty, SYSCALL_ISATTY);
  KREG_SYSCALL(kill, SYSCALL_KILL);
  KREG_SYSCALL(link, SYSCALL_LINK);
  KREG_SYSCALL(lseek, SYSCALL_LSEEK);
  KREG_SYSCALL(open, SYSCALL_OPEN);
  KREG_SYSCALL(read, SYSCALL_READ);
  KREG_SYSCALL(sbrk, SYSCALL_SBRK);
  KREG_SYSCALL(stat, SYSCALL_STAT);
  KREG_SYSCALL(times, SYSCALL_TIMES);
  KREG_SYSCALL(unlink, SYSCALL_UNLINK);
  KREG_SYSCALL(wait, SYSCALL_WAIT);
  KREG_SYSCALL(write, SYSCALL_WRITE);
  KREG_SYSCALL(waitpid, SYSCALL_WAITPID);
  KREG_SYSCALL(yield, SYSCALL_YIELD);
  KREG_SYSCALL(signal, SYSCALL_SIGNAL);
  KREG_SYSCALL(readdir, SYSCALL_READDIR);
  KREG_SYSCALL(dup, SYSCALL_DUP);
  KREG_SYSCALL(dup2, SYSCALL_DUP2);
  KREG_SYSCALL(pipe, SYSCALL_PIPE);
  KREG_SYSCALL(thread_fork, SYSCALL_THREAD);

  KREG_SYSCALL(printf, SYSCALL_PRINTF);
  KREG_SYSCALL(process_debug, SYSCALL_PDBG);
  KREG_SYSCALL(vidmem, SYSCALL_VIDMEM);

}

registers_t *syscall_handler(registers_t *r)
{

  if(syscall_handlers[r->eax])
    r = syscall_handlers[r->eax](r);
  else
    r->edx = ERROR_NOSYSCALL;

  return r;
}

syscall_t register_syscall(uint32_t num, syscall_t handler)
{
  syscall_t ret = syscall_handlers[num];

  syscall_handlers[num] = handler;

  return ret;
}
