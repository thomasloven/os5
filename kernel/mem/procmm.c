#include <stdint.h>
#include <procmm.h>
#include <process.h>
#include <lists.h>
#include <pmm.h>
#include <vmm.h>
#include <synch.h>
#include <k_debug.h>
#include <string.h>

#include <stdlib.h>

semaphore_t procmm_sem;

void init_procmm(process_t *p)
{
  // Init memory areas for a process
  memset(&p->mm, 0, sizeof(process_mem_t));
  init_list(p->mm.mem);
}

mem_area_t *alloc_area(uintptr_t start, uintptr_t end, 
  uintptr_t flags, uintptr_t type, process_t *owner)
{
  // Helper function. Allocate a new memory area and prepare it
  mem_area_t *ma = calloc(1, sizeof(mem_area_t));
  ma->start = start;
  ma->end = end;
  ma->flags = flags;
  ma->type = type;
  init_list(ma->mem);
  init_list(ma->copies);
  ma->owner = owner;
  return ma;
}

mem_area_t *new_area(process_t *p, uintptr_t start,
  uintptr_t end, uint32_t flags, uint32_t type)
{
  // Add memory area to process and add its memory to memory space

  start &= PAGE_MASK;
  if((end & PAGE_FLAG_MASK))
    end += PAGE_SIZE;
  end &= PAGE_MASK;

  // Check that it doesn't collide with allready allocated space
  uintptr_t i = start;
  while(i <= end)
  {
    mem_area_t *a;
    if((a = find_including(p, i)))
    {
      debug("[error]AREA COLLISION! %x %x %x\n", a->start, a->end, start);
      for(;;);
      return 0;
    }
    i += PAGE_SIZE;
  }

  mem_area_t *area = alloc_area(start, end, flags, type, p);

  uint32_t vmm_flags = PAGE_PRESENT | PAGE_USER;
  if((flags & MM_FLAG_WRITE) && !(flags & MM_FLAG_COW)) // Write enabled
    vmm_flags |= PAGE_WRITE;

  // Add to memory space
  if(!(flags & MM_FLAG_ADDONUSE)  && !(flags & MM_FLAG_COW))
  {
    spin_lock(&p->pd_sem);
      page_dir_t old = vmm_exdir_set(p->pd);
      i = start;
      while (i < end)
      {
        vmm_expage_set(i, vmm_page_val(pmm_alloc_page(), vmm_flags));
        i += PAGE_SIZE;
      }
      vmm_exdir_set(old);
    spin_unlock(&p->pd_sem);
  }

  // Sorted insertion into list
  spin_lock(&procmm_sem);
    list_t *l = p->mm.mem.next;
    while(l != &p->mm.mem)
    {
      mem_area_t *this = list_entry(l, mem_area_t, mem);
      if(this->start > end)
        break;
      l = l->next;
    }
    list_insert_to_left(*l, area->mem);
  spin_unlock(&procmm_sem);

  return glue_area(area);
}

void free_area(mem_area_t *ma)
{
  // Remove area from all lists and free it
  if(!ma)
    return;

  remove_from_list(ma->mem);
  remove_from_list(ma->copies);
  free(ma);
}

mem_area_t *split_area(mem_area_t *ma, uintptr_t start, uintptr_t end)
{
  // Splits an area out from an other
  start &= PAGE_MASK;
  end &= PAGE_MASK;

  if(!ma)
    return ma;

  if((ma->start == start) && (ma->end == end)) // No need to split
    return ma;

  mem_area_t *left = 0;
  mem_area_t *right = 0;

  spin_lock(&procmm_sem);
    if(start > ma->start) // Split off a part to the left
    {
      left = alloc_area(ma->start, start, ma->flags, ma->type, ma->owner);
      list_insert_to_left(ma->mem, left->mem);
    }

    if(end < ma->end) // Split off a part to the right
    {
      right = alloc_area(end, ma->end, ma->flags, ma->type, ma->owner);
      list_insert_to_right(ma->mem, right->mem);
    }

    // Reshape old area
    ma->start = start;
    ma->end = end;


    // Make the same splits with every copy of the memory area
    list_t *copies_list;
    for_each_in_list(&ma->copies, copies_list)
    {

      mem_area_t *mid = list_entry(copies_list, mem_area_t, copies);
      if(left != 0) // Split off a part to the left
      {
        mem_area_t *l = alloc_area(mid->start, start,\
          mid->flags, mid->type, mid->owner);
        list_insert_to_left(mid->mem, l->mem);
        // Add to list of copies
        append_to_list(left->copies, l->copies);
      }
      if(right != 0) // Split off a part to the right
      {
        mem_area_t *r = alloc_area(end, mid->end, \
          mid->flags, mid->type, mid->owner);
        list_insert_to_right(mid->mem, r->mem);
        // Add to list of copies
        append_to_list(right->copies, r->copies);
      }

        // Reshape old area
        mid->start = start;
        mid->end = end;
    }
  spin_unlock(&procmm_sem);

  return ma;
}

mem_area_t *glue_area(mem_area_t *ma)
{
  // If possible, join areas together
  
  if(!ma)
    return ma;

  // An area can only be glued if it has no copies
  if(!(list_empty(ma->copies)))
    return ma;

  if(ma->flags & MM_FLAG_COW)
    return ma;

  spin_lock(&procmm_sem);
    list_t *area_list;
    for_each_in_list(&ma->owner->mm.mem, area_list)
    {
      mem_area_t *area = list_entry(area_list, mem_area_t, mem);
      if(area != ma && area->end == ma->start) // Glue to the left
      {
        if((area->flags == ma->flags) && (area->type == ma->type))
        {
          // Reshape the left area and remove the current
          area->end = ma->end;
          free_area(ma);
          ma = area;
          break;
        }
      }
    }
    for_each_in_list(&ma->owner->mm.mem, area_list)
    {
      mem_area_t *area = list_entry(area_list, mem_area_t, mem);
      if(area != ma && area->start == ma->end) // Glue to the right
      {
        if((area->flags == ma->flags) && (area->type == ma->type))
        {
          // Reshape curent area and free right one
          ma->end = area->end;
          free_area(area);
          break;
        }
      }
    }
  spin_unlock(&procmm_sem);
  // Return glued area
  return ma;
}

mem_area_t *find_including(process_t *p, uintptr_t addr)
{
  // Find an area of process p which contains the address addr
  // If none exists, return 0
  list_t *area_list;
  for_each_in_list(&p->mm.mem, area_list)
  {
    mem_area_t *area = list_entry(area_list, mem_area_t, mem);
    if((area->start <= addr) && (area->end > addr))
      return area;
  }

  return 0;
}

mem_area_t *find_above(process_t *p, uintptr_t addr)
{
  // Find area containing or nearest above address addr
  mem_area_t *area = find_including(p, addr);
  if(area)
    return area;
  list_t *area_list = p->mm.mem.prev;
  while(area_list != p->mm.mem.next)
  {
    area = list_entry(area_list->prev, mem_area_t, mem);
    if(area->end < addr)
      return list_entry(area_list, mem_area_t, mem);
    area_list = area_list->prev;
  }
  return 0;
}

void share_area(process_t *copy, mem_area_t *ma)
{
  spin_lock(&procmm_sem);
  ma->flags |= MM_FLAG_SHARED;
  if((ma->flags & MM_FLAG_WRITE) || (ma->flags & MM_FLAG_COW))
  {
    ma->flags &= ~(MM_FLAG_WRITE);
    ma->flags |= MM_FLAG_COW;

    spin_lock(&ma->owner->pd_sem);
      page_dir_t old = vmm_exdir_set(ma->owner->pd);
      uintptr_t i = ma->start;
      while (i < ma->end)
      {
        uintptr_t pt_val = vmm_expage_get(i);
        pt_val &= ~(PAGE_WRITE);
         vmm_expage_set(i, pt_val);
        i += PAGE_SIZE;
      }
      vmm_exdir_set(old);
    spin_unlock(&ma->owner->pd_sem);

  }
  spin_unlock(&procmm_sem);
  mem_area_t *new = new_area(copy, ma->start, ma->end, ma->flags, ma->type);
  append_to_list(ma->copies, new->copies);
}


void print_areas(process_t *p)
{
  list_t *area_list;
  mem_area_t *area;
  debug("[info]Memory areas start\n");
  for_each_in_list(&p->mm.mem, area_list)
  {
    area = list_entry(area_list, mem_area_t, mem);
    debug("0x%x-0x%x Owner: %x", area->start, area->end, area->owner->pid);
    debug("Flags: %c%c%c%c%c%c%c%c", \
        (area->flags & MM_FLAG_READ)?'R':'-', \
        (area->flags & MM_FLAG_WRITE)?'W':'-', \
        (area->flags & MM_FLAG_SHARED)?'S':'-', \
        (area->flags & MM_FLAG_NOSHARE)?'N':'-', \
        (area->flags & MM_FLAG_COW)?'O':'-', \
        (area->flags & MM_FLAG_GROWSDOWN)?'D':'-', \
        (area->flags & MM_FLAG_AUTOGROW)?'A':'-', \
        (area->flags & MM_FLAG_ADDONUSE)?'a':'-');
    debug("\n");
  }
  debug("[info]Memory areas end\n");
}

uint32_t procmm_handle_page_fault(uintptr_t address, uint32_t flags)
{
  mem_area_t *inside = find_including(current->proc, address);

  if(inside)
  {
    if(inside->flags & MM_FLAG_COW)
    {
      if((flags & PF_PRESENT) && (flags & PF_WRITE))
      {
        // Split out page from area and copy the page
        inside = split_area(inside, (address & PAGE_MASK), (address & PAGE_MASK) + PAGE_SIZE);
        uintptr_t pval = vmm_page_get(address);
        if(list_empty(inside->copies))
        {
          // This is the only copy
          // Unset CopyOnWrite flag
          inside->flags = inside->flags & ~MM_FLAG_COW;
          inside->flags = inside->flags | MM_FLAG_WRITE;
          // Enable write on area
          vmm_page_set(address & PAGE_MASK, pval | PAGE_WRITE);
          return 0;
        } else {
          // Copy page
          vmm_page_set(VMM_TEMP1, pval);
          vmm_page_set(address & PAGE_MASK, vmm_page_val(pmm_alloc_page(), (pval & PAGE_FLAG_MASK) | PAGE_WRITE));
          memcpy((void *)(address & PAGE_MASK), (const void *)VMM_TEMP1, PAGE_SIZE);
          vmm_page_set(VMM_TEMP1, 0);

          // Unset CopyOnWrite flag
          inside->flags = inside->flags & ~MM_FLAG_COW;
          inside->flags = inside->flags | MM_FLAG_WRITE;
          
          // Remove from list of copies
          remove_from_list(inside->copies);
          return 0;
        }
      }
    }
    if(inside->flags & MM_FLAG_ADDONUSE)
    {
      if(!(flags & PF_PRESENT))
      {
        // The address is in an area, but no page is present in space
        // so just add it.
        uint32_t vmm_flags = PAGE_PRESENT | PAGE_USER;
        if(inside->flags & MM_FLAG_WRITE) // Write enabled
          vmm_flags |= PAGE_WRITE;
        vmm_page_set(address & PAGE_MASK, vmm_page_val(pmm_alloc_page(), vmm_flags));
          return 0;
      }
    }
  } else {
    mem_area_t *above = find_above(current->proc, address);

    if(above)
    {
      if((above->start - address) >= PAGE_SIZE)
      {
          return 1;
      }
      if(!(above->flags & MM_FLAG_GROWSDOWN))
      {
        return 1;
      }
      if(address <= current->proc->mm.stack_limit)
      {
        return 1;
      }

      // Expand stack
      new_area(current->proc, address & PAGE_MASK, above->start, above->flags, above->type);
      return 0;
    }
      
  }

  return 1;
}

int procmm_check_address(uintptr_t address)
{
  // Return values:
  // 1: Present, not writable
  // 2: Present, on stack
  // 3: Present, writable
  mem_area_t *area = find_including(current->proc, address);
  if(area)
  {
    if(area->flags & MM_FLAG_WRITE || area->flags & MM_FLAG_COW)
      return 3;
    else
      return 1;
  } else{
    area = find_above(current->proc, address);
    if(area && area->flags & MM_FLAG_GROWSDOWN)
      return 2;
  }
  return 0;
}

void procmm_fork(process_t *parent, process_t *child)
{
  // Copy memory information
  memcpy(&child->mm, &parent->mm, sizeof(process_mem_t));
  init_list(child->mm.mem);

  // Share all areas
  list_t *i;
  for_each_in_list(&parent->mm.mem, i)
  {
    mem_area_t *ma = list_entry(i, mem_area_t, mem);
    if((ma->flags & MM_FLAG_NOSHARE) != MM_FLAG_NOSHARE)
    share_area(child, ma);
  }

}

void procmm_exit(process_t *proc)
{
  list_t *i = (&proc->mm.mem)->next;
  while(i != &proc->mm.mem)
  {
    mem_area_t *ma = list_entry(i, mem_area_t, mem);
    i = i->next;
    free_area(ma);
  }
}

void procmm_removeall(process_t *proc)
{
  list_t *i = proc->mm.mem.next;
  while(i != &proc->mm.mem)
  {
    mem_area_t *ma = list_entry(i, mem_area_t, mem);
    i = i->next;

    spin_lock(&proc->pd_sem);
      page_dir_t old = vmm_exdir_set(proc->pd);
      uint32_t j = ma->start;
      while (j < ma->end)
      {
        vmm_expage_set(j, 0);
        j += PAGE_SIZE;
      }
      vmm_exdir_set(old);
    spin_unlock(&proc->pd_sem);
    free_area(ma);
  }
}
