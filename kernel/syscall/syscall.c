#include <k_syscall.h>
#include <stdint.h>
#include <arch.h>
#include <process.h>
#include <lists.h>
#include <syscall.h>

#include <k_debug.h>

registers_t *syscall_handler(registers_t *r)
{
  uint32_t *stack = (uintptr_t *)(r->useresp);
  stack++;
  if(r->eax == SYSCALL_FORK)
  {
    debug("FORKING!");
    process_t *child = fork_process();
    thread_t *cth = list_entry(child->threads.next, thread_t, process_threads);
    cth->r.eax = 0;
    r->eax = child->pid;
    return r;
  } else if(r->eax == SYSCALL_PUTCH) {
    debug("PUTCH:");
    debug((char *)stack[0]);
    debug((char *)stack[1]);
    debug("<<");
    return r;
  } else {
    debug("OTHER");
  }
  return r;
}

