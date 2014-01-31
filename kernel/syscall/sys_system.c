#include <k_syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>

#include <sys/times.h>
#include <errno.h>

#undef errno
extern int errno;


clock_t times(struct tms *buf)
{
  (void)buf;
  errno = 0;
  return -1;
}
KDEF_SYSCALL(times, r)
{
  process_stack stack = init_pstack();
  r->eax = times((struct tms *)stack[0]);
  r->ebx = errno;
  return r;
}


KDEF_SYSCALL(vidmem, r)
{
  return r;
}
