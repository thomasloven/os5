#include <k_syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>
#include <procmm.h>
#include <vmm.h>

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
  new_area(current->proc, 0xB8000, 0xB9000, MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_NOSHARE, MM_TYPE_DATA);
  pmm_free_page(vmm_page_get(0xB8000));
  vmm_page_set(0xB8000, vmm_page_val(0xB8000, PAGE_PRESENT | PAGE_WRITE | PAGE_USER));

  r->eax = 0xB8000;

  return r;
}
