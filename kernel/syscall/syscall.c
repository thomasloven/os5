#include <k_syscall.h>
#include <syscall.h>
#include <stdint.h>
#include <arch.h>
#include <idt.h>
#include <lists.h>
#include <memory.h>

#include <k_debug.h>

syscall_t syscall_handlers[NUM_SYSCALLS];

void syscall_init()
{
  memset(syscall_handlers, 0 , NUM_SYSCALLS*sizeof(syscall_t *));
  register_int_handler(INT_SYSCALL, syscall_handler);

  KREG_SYSCALL(printf, SYSCALL_PRINTF);
  KREG_SYSCALL(fork, SYSCALL_FORK);
  KREG_SYSCALL(getpid, SYSCALL_GETPID);
  KREG_SYSCALL(exit, SYSCALL_EXIT);
  KREG_SYSCALL(wait, SYSCALL_WAIT);
  KREG_SYSCALL(sbrk, SYSCALL_SBRK);
  KREG_SYSCALL(yield, SYSCALL_YIELD);

  KREG_SYSCALL(open, SYSCALL_OPEN);
  KREG_SYSCALL(write, SYSCALL_WRITE);
}

registers_t *syscall_handler(registers_t *r)
{

  if(syscall_handlers[r->eax])
    r = syscall_handlers[r->eax](r);
  else
    r->ebx = ERROR_NOSYSCALL;

  return r;
}

syscall_t register_syscall(uint32_t num, syscall_t handler)
{
  syscall_t ret = syscall_handlers[num];

  syscall_handlers[num] = handler;

  return ret;
}
