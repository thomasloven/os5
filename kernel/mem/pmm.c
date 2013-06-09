#include <common.h>
#include <stdint.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <ctype.h>
#include <synch.h>
#include <k_debug.h>

uint8_t pmm_running;
uintptr_t pmm_pos;

uintptr_t *pmm_page_stack_max = (uintptr_t *)PMM_PAGE_STACK;
uintptr_t *pmm_page_stack_ptr = (uintptr_t *)PMM_PAGE_STACK;

semaphore_t pmm_sem;

uintptr_t pmm_alloc_page()
{
  if(pmm_running == FALSE)
  {
    pmm_pos += PAGE_SIZE;
    return pmm_pos - PAGE_SIZE;
  }

  if ( pmm_page_stack_ptr == (uintptr_t *)PMM_PAGE_STACK)
  {
    panic("Out of memory!");
  }

  spin_lock(&pmm_sem);
    pmm_page_stack_ptr--;

    if(pmm_page_stack_ptr < (pmm_page_stack_max - PMM_STACK_ENTRIES_PER_PAGE))
    {
      pmm_page_stack_max = pmm_page_stack_max - PMM_STACK_ENTRIES_PER_PAGE;
      uintptr_t ret = vmm_page_get((uintptr_t)pmm_page_stack_max) & PAGE_MASK;
      
      vmm_page_set(ret, 0);
    spin_unlock(&pmm_sem);
      return ret;
    }
  spin_unlock(&pmm_sem);
  return *pmm_page_stack_ptr;
}

void pmm_free_page(uintptr_t page)
{
  page &= PAGE_MASK;

  if(page < pmm_pos + PAGE_SIZE) // Two pages are required for initial setup
    return;

  spin_lock(&pmm_sem);
    if(pmm_page_stack_ptr >= pmm_page_stack_max)
    {
      vmm_page_set((uintptr_t)pmm_page_stack_max, vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
      pmm_page_stack_max = pmm_page_stack_max + PMM_STACK_ENTRIES_PER_PAGE;
    } else {
      *pmm_page_stack_ptr++ = page;
    }
    pmm_running = TRUE;
  spin_unlock(&pmm_sem);
}

void pmm_init(mboot_info_t *mboot)
{
  pmm_pos = (mboot->mem_upper + PAGE_SIZE) & PAGE_MASK;
  pmm_running = FALSE;

  mboot_mmap_entry_t *me = (mboot_mmap_entry_t *)(assert_higher(mboot->mmap_addr));
  uintptr_t mmap_end = assert_higher(mboot->mmap_addr) + mboot->mmap_length;
  while ((uintptr_t)me < mmap_end)
  {
    if(me->type == MBOOT_MEM_FLAG_FREE)
    {
      uint32_t j;
      for(j = me->base_addr_lower; j <= (me->base_addr_lower + me->length_lower); j += PAGE_SIZE)
      {
        pmm_free_page(j);
      }
    }
    me = (mboot_mmap_entry_t *)((uint32_t)me + me->size + sizeof(uint32_t));
  }
}
