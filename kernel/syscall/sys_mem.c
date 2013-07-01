#include <k_syscall.h>
#include <syscall.h>
#include <k_debug.h>
#include <procmm.h>
#include <vmm.h>

#include <errno.h>

#undef errno
extern int errno;

uintptr_t kmem_top = KERNEL_HEAP_START;
uintptr_t kmem_ptr = KERNEL_HEAP_START;

void *sbrk(int incr)
{
  // sbrk as called from the kernel

    if(kmem_ptr + incr >= KERNEL_HEAP_END)
    {
      errno = ENOMEM;
      return 0;
    }
    while(kmem_top < kmem_ptr + incr)
    {
      vmm_page_set(kmem_top, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE));
      kmem_top += PAGE_SIZE;
    }
    kmem_ptr = kmem_ptr + incr;
    errno = SYSCALL_OK;
    return (void *)(kmem_ptr-incr);
}

KDEF_SYSCALL(sbrk, r)
{
  // sbrk as called by user processes

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

  // Return start of new area
  r->eax = p->mm.data_end;
  p->mm.data_end = p->mm.data_end + size;
  
  r->ebx = SYSCALL_OK;
  return r;
}
