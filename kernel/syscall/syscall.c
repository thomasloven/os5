#include <k_syscall.h>
#include <stdint.h>
#include <arch.h>
#include <process.h>
#include <lists.h>

#include <k_debug.h>

registers_t *syscall_handler(registers_t *r)
{
  process_t *child = fork_process();
  thread_t *cth = list_entry(child->threads.next, thread_t, process_threads);
  cth->r.eax = 0;
  r->eax = child->pid;
  return r;
}
