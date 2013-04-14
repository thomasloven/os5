#include <signals.h>
#include <stdint.h>
#include <syscall.h>

signal_handler_t signal_handlers[NUM_SIGNALS];

void _on_event(uint32_t signum)
{
  signal_handlers[signum](signum);
  _syscall_done();
}

void init_signals()
{
  _syscall_sighndl(&_signal_event);
  uint32_t i;
  for(i = 0; i < NUM_SIGNALS; i++)
  {
    signal_handlers[i] = (signal_handler_t)SIG_DFL;
    _syscall_signal(i, (void *)SIG_DFL);
  }
}

void raise(uint32_t signum)
{
  _syscall_kill(_syscall_getpid(), signum);
}

uintptr_t *signal(uint32_t signum, uintptr_t *handler)
{
  uintptr_t *ret = (uintptr_t*)signal_handlers[signum];

  signal_handlers[signum] = (signal_handler_t)handler;

  _syscall_signal(signum, handler);

  return ret;
}
