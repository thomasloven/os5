#include <k_syscall.h>
#include <syscall.h>
#include <k_debug.h>
#include <procmm.h>

KDEF_SYSCALL(sbrk, r)
{
  process_stack stack = init_pstack();
  uint32_t size = stack[0];
  process_t *p = current->proc;
  
  mem_area_t *area = find_including(p, p->mm.data_end);
  if(area)
  {
    if(area->end > (p->mm.data_end + size))
    {
      // The existing areas are enough
    } else {
      // Extend the area
      new_area(p, area->end, p->mm.data_end + size, MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_CANSHARE, MM_TYPE_DATA);
    }
  } else {
    // Create a new area
      new_area(p, p->mm.data_end, p->mm.data_end + size, MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_CANSHARE, MM_TYPE_DATA);
  }

  r->eax = p->mm.data_end;
  p->mm.data_end = p->mm.data_end + size;
  
  return r;
}
