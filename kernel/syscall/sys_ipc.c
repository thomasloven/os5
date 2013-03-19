#include <syscall.h>
#include <k_syscall.h>
#include <stdint.h>
#include <process.h>

KDEF_SYSCALL(kill, r)
{
  process_stack stack = init_pstack();
  process_t *p = get_process(stack[0]);

  signal_process(p, stack[1]);

  schedule();
  return r;
}
