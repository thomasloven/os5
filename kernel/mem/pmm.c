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
  // Hand out the physical address of a page of free memory.

  // If the PMM has not been completely setup (e.g. during setup of the
  // PMM), just allocate pages from right after where the kernel is
  // loaded. This should need to be done, like, twice or so.
  // No big waste.
  if(pmm_running == FALSE)
  {
    pmm_pos += PAGE_SIZE;
    return pmm_pos - PAGE_SIZE;
  }

  if ( pmm_page_stack_ptr == (uintptr_t *)PMM_PAGE_STACK)
  {
    // At bottom of page stack
    panic("Out of memory!");
  }

  spin_lock(&pmm_sem);
    uintptr_t ret;

    if(pmm_page_stack_ptr < (pmm_page_stack_max - PMM_STACK_ENTRIES_PER_PAGE))
    {
      // If a page of the stack area is freed, hand out that page
      // instead.
      pmm_page_stack_max = pmm_page_stack_max - PMM_STACK_ENTRIES_PER_PAGE;
      ret = vmm_page_get((uintptr_t)pmm_page_stack_max) & PAGE_MASK;
      vmm_page_set((uintptr_t)pmm_page_stack_max, 0);
    } else {
      // Otherwise, return the page at the top of the stack.
      pmm_page_stack_ptr--;
      ret = *pmm_page_stack_ptr;
    }

  spin_unlock(&pmm_sem);
  return ret;
}

void pmm_free_page(uintptr_t page)
{
  // Returns a physical page to the free page stack.
  // Does not check if the page is already on the stack.

  page &= PAGE_MASK;

  if(page < pmm_pos + PAGE_SIZE) // Two pages are required for initial setup
    return;

  spin_lock(&pmm_sem);
    // If a new page is needed to fit the stack, use the one we just
    // got.
    if(pmm_page_stack_ptr >= pmm_page_stack_max)
    {
      vmm_page_set((uintptr_t)pmm_page_stack_max, vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
      pmm_page_stack_max = pmm_page_stack_max + PMM_STACK_ENTRIES_PER_PAGE;
    } else {
      // Otherwise put the new page on top of the stack.
      *pmm_page_stack_ptr = page;
      pmm_page_stack_ptr++;
    }
    
    // As soon as there is a page on the stack, the PMM is ready to go.
    pmm_running = TRUE;
  spin_unlock(&pmm_sem);
}

void pmm_init(mboot_info_t *mboot)
{
  // Setup physical memory manager.
  // Takes a multiboot memory map as argument.

  mboot_mod_t *mods = (mboot_mod_t *)(assert_higher(mboot->mods_addr));
  pmm_pos = (mboot->mem_upper + PAGE_SIZE) & PAGE_MASK;
  if(pmm_pos < (mods[mboot->mods_count-1].mod_end))
    pmm_pos = (mods[mboot->mods_count-1].mod_end + PAGE_SIZE) & PAGE_MASK;
  pmm_running = FALSE;

  // Fill physical page stack with free pages from memory map.
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
