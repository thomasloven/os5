#include <stdint.h>
#include <procmm.h>
#include <process.h>
#include <memory.h>
#include <lists.h>
#include <pmm.h>
#include <vmm.h>
#include <heap.h>

#include <k_debug.h>

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
  mem_area_t *ma = kmalloc(sizeof(mem_area_t));
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
  end &= PAGE_MASK;

  // Check that it doesn't collide with allready allocated space
  uintptr_t i = start;
  while(i <= end)
  {
    if(find_including(p, i))
    {
      debug("\n AREA COLLISION!");
      return 0;
    }
    i += PAGE_SIZE;
  }

  mem_area_t *area = alloc_area(start, end, flags, type, p);

  uint32_t vmm_flags = PAGE_PRESENT | PAGE_USER;
  if((flags & MM_FLAG_WRITE) && !(flags & MM_FLAG_COW)) // Write enabled
    vmm_flags |= PAGE_WRITE;

  // Add to memory space
  page_dir_t old = vmm_exdir_set(p->pd);
  i = start;
  while (i < end)
  {
     vmm_expage_set(i, vmm_page_val(pmm_alloc_page(), vmm_flags));
    i += PAGE_SIZE;
  }
  vmm_exdir_set(old);

  // Sorted insertion into list
  list_t *l = p->mm.mem.next;
  while(l != &p->mm.mem)
  {
    mem_area_t *this = list_entry(l, mem_area_t, mem);
    if(this->start > end)
      break;
    l = l->next;
  }
  list_insert_to_left(*l, area->mem);

  return glue_area(area);
}

void free_area(mem_area_t *ma)
{
  // Remove area from all lists and free it
  if(!ma)
    return;

  remove_from_list(ma->mem);
  remove_from_list(ma->copies);
  kfree(ma);
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

  list_t *area_list;
  for_each_in_list(&ma->owner->mm.mem, area_list)
  {
    mem_area_t *area = list_entry(area_list, mem_area_t, mem);
    if(area->end == ma->start) // Glue to the left
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
    if(area->start == ma->end) // Glue to the right
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
  while(area_list != &p->mm.mem)
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
  ma->flags |= MM_FLAG_SHARED;
  if(ma->flags & MM_FLAG_WRITE)
  {
    ma->flags &= ~(MM_FLAG_WRITE);
    ma->flags |= MM_FLAG_COW;

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

  }
  mem_area_t *new = new_area(copy, ma->start, ma->end, ma->flags, ma->type);
  append_to_list(ma->copies, new->copies);
}


void print_areas(process_t *p)
{
  list_t *area_list;
  mem_area_t *area;
  debug("\n Areas:");
  for_each_in_list(&p->mm.mem, area_list)
  {
    area = list_entry(area_list, mem_area_t, mem);
    debug("\n %x-%x", area->start, area->end);
  }
}
