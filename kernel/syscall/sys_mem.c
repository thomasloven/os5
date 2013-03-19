#include <k_syscall.h>
#include <syscall.h>
#include <k_debug.h>
#include <procmm.h>

KDEF_SYSCALL(sbrk, r)
{
  debug("sbrk called");
  process_stack stack = init_pstack();
  uint32_t size = stack[0];
  process_t *p = current->proc;
  debug("\n Extending from %x", p->mm.data_end);
  
  new_area(p, p->mm.data_end, p->mm.data_end + size, MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_CANSHARE, MM_TYPE_DATA);

  r->eax = p->mm.data_end;
  p->mm.data_end = p->mm.data_end + size;
  print_areas(p);
  debug(" to %x", p->mm.data_end);
  
  return r;
}
