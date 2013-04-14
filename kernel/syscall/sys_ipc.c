#include <syscall.h>
#include <k_syscall.h>
#include <stdint.h>
#include <process.h>
#include <k_debug.h>

KDEF_SYSCALL(kill, r)
{
  process_stack stack = init_pstack();
  process_t *p = get_process(stack[0]);

  signal_process(p, stack[1]);

  schedule();
  return r;
}

KDEF_SYSCALL(signal, r)
{
  debug("signal");
  return r;
}

KDEF_SYSCALL(sighndl, r)
{
  debug("sighndl");
  return r;
}
