#include <common.h>
#include <vmm.h>
#include <pmm.h>
#include <stdint.h>
#include <k_debug.h>
#include <ctype.h>
#include <synch.h>
#include <string.h>

uint8_t vmm_running = 0;

uintptr_t *page_directory = (uintptr_t *)VMM_PAGE_DIR;
uintptr_t *page_tables = (uintptr_t *)VMM_PAGE_TABLES;

uintptr_t *expage_directory = (uintptr_t *)VMM_EXPAGE_DIR;
uintptr_t *expage_tables = (uintptr_t *)VMM_EXPAGE_TABLES;


void vmm_pd_set(page_dir_t pd)
{
  // Load a new page directory
  __asm__ volatile("mov %0, %%cr3" : : "r" (pd));
}

uintptr_t vmm_page_get(uintptr_t page)
{
  // Get the value of a page table entry in the current memory space
  page &= PAGE_MASK;

  if(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
    if(page_tables[vmm_table_idx(page)] & PAGE_PRESENT)
      return page_tables[vmm_table_idx(page)];
  return 0;
}

uintptr_t vmm_table_get(uintptr_t page)
{
  // Get the value of a page dir entry in the current memory space

  page &= PAGE_MASK;

    if(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
      return page_directory[vmm_dir_idx(page)];
    return 0;
}

void vmm_table_set(uintptr_t page, uintptr_t value)
{
  // Set the value of a page dir entry in the current memory space
  
  page &= PAGE_MASK;

  page_directory[vmm_dir_idx(page)] = value;
}

void vmm_page_touch(uintptr_t page, uint32_t flags)
{
  // Make sure a page table for the address page exists in the current memory
  // space

  page &= PAGE_MASK;
  flags &= PAGE_FLAG_MASK;

  if(!(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT))
  {
    // There is no page table for the page

    if(page >= KERNEL_OFFSET && vmm_running)
    {
      // The page we want to add is in the kernel space
      spin_lock(&kernel_pd_sem);
      spin_lock(&exdir_sem);
        page_dir_t old_exdir = vmm_exdir_set(kernel_pd);
        if(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
        {
          // If the page table exists in the master page directory, just
          // copy it.
          page_directory[vmm_dir_idx(page)] = expage_directory[vmm_dir_idx(page)];
        } else {
          // Otherwise allocate a new table
          page_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
          expage_directory[vmm_dir_idx(page)] = page_directory[vmm_dir_idx(page)];
        }
        vmm_exdir_set(old_exdir);
      spin_unlock(&exdir_sem);
      spin_unlock(&kernel_pd_sem);
    } else {
      // If the page is in user space, just allocate a new table.
      page_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
    }

    // Clear the new page table
    vmm_flush_tlb((uintptr_t)&page_tables[vmm_table_idx(page)] & PAGE_MASK);
    memset((void *)((uintptr_t)&page_tables[vmm_table_idx(page)] & PAGE_MASK), 0, \
    PAGE_SIZE);
  }
}

void vmm_page_set(uintptr_t page, uintptr_t value)
{
  // Set the value of a page table entry in the current memory space
  page &= PAGE_MASK;

  vmm_page_touch(page, value & PAGE_FLAG_MASK);
  page_tables[vmm_table_idx(page)] = value;
  vmm_flush_tlb(page);
}


page_dir_t vmm_exdir_set(page_dir_t pd)
{
  // Temporarily load or unload another page directory and its tables into the
  // current memory space
  
  page_dir_t old = \
    page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] & PAGE_MASK;

  if(pd)
    page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] = \
      pd | PAGE_PRESENT | PAGE_WRITE;
  else
    page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] = 0;

  vmm_flush_tlb(VMM_EXPAGE_DIR);

  return old;
}

uintptr_t vmm_expage_get(uintptr_t page)
{
  // Get the value of a page table entry in the currently loaded external page
  // directory
  
  if(page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] == 0)
    return 0;

  page &= PAGE_MASK;
  if(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
    if(expage_tables[vmm_table_idx(page)] & PAGE_PRESENT)
      return expage_tables[vmm_table_idx(page)];
  return 0;
}

uintptr_t vmm_extable_get(uintptr_t page)
{
  // Get the value of a page dir entry in the current memory space

  page &= PAGE_MASK;

    if(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
      return expage_directory[vmm_dir_idx(page)];
    return 0;
}

void vmm_extable_set(uintptr_t page, uintptr_t value)
{
  // Set the value of a page dir entry in the current memory space
  
  page &= PAGE_MASK;

  expage_directory[vmm_dir_idx(page)] = value;
}

void vmm_expage_touch(uintptr_t page, uint32_t flags)
{
  // Make sure a page table exists for address page in the currently loaded
  // external page directory

  page &= PAGE_MASK;
  flags &= PAGE_FLAG_MASK;

  if(page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] == 0)
    return;

  if(!(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT))
  {
    expage_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
    vmm_flush_tlb((uintptr_t)&expage_tables[vmm_table_idx(page)] & PAGE_MASK);
    memset((void *)((uintptr_t)&expage_tables[vmm_table_idx(page)] & PAGE_MASK), 0, \
    PAGE_SIZE);
  }
}

void vmm_expage_set(uintptr_t page, uintptr_t value)
{
  // Set the value of a page table entry in the currently loaded external page
  // directory.
  page &= PAGE_MASK;

  vmm_expage_touch(page, value & PAGE_FLAG_MASK);
  expage_tables[vmm_table_idx(page)] = value;
  vmm_flush_tlb(page);
}


void vmm_clear_page(uintptr_t page)
{
  // Clear a physical page

  spin_lock(&temp_sem);
    uintptr_t old = vmm_page_get(VMM_TEMP1);
    vmm_page_set(VMM_TEMP1, vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
    memset((void *)VMM_TEMP1, 0, PAGE_SIZE),
    vmm_page_set(VMM_TEMP1, old);
  spin_unlock(&temp_sem);
}

void vmm_copy_page(uintptr_t src, uintptr_t dst)
{
  // Make a copy of a physical page
  
  spin_lock(&temp_sem);
    uintptr_t old_temp1 = vmm_page_get(VMM_TEMP1);
    uintptr_t old_temp2 = vmm_page_get(VMM_TEMP2);
    vmm_page_set(VMM_TEMP1, vmm_page_val(src, PAGE_PRESENT | PAGE_WRITE));
    vmm_page_set(VMM_TEMP2, vmm_page_val(dst, PAGE_PRESENT | PAGE_WRITE));

    // There's probably an x86 instruction to do this faster?
    memcpy((void *)VMM_TEMP2, (const void *)VMM_TEMP1, PAGE_SIZE);

    vmm_page_set(VMM_TEMP1, old_temp1);
    vmm_page_set(VMM_TEMP2, old_temp2);
  spin_unlock(&temp_sem);
}

page_dir_t vmm_new_pd()
{
  // Returns the physical address of a page directory prepared for
  // recursive paging.

  page_dir_t pd = pmm_alloc_page();

  spin_lock(&temp_sem);
    // Clear page directory.
    uintptr_t old = vmm_page_get(VMM_TEMP1);
    vmm_page_set(VMM_TEMP1, vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE));
    memset((void *)VMM_TEMP1, 0, PAGE_SIZE);

    // Make page directory recursive.
    uint32_t *pdir = (uint32_t *)VMM_TEMP1;
    pdir[VMM_PAGES_PER_TABLE - 1] = vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE);

    vmm_page_set(VMM_TEMP1, old);
  spin_unlock(&temp_sem);

  return pd;
}

page_dir_t vmm_clone_pd()
{
  // Make a copy of the current page directory

  page_dir_t pd = vmm_new_pd();

  spin_lock(&exdir_sem);
    page_dir_t old_exdir = vmm_exdir_set(pd);

    uint32_t table;

    // Copy each page table except the kernel space
    for(table = 0; table < vmm_dir_idx(KERNEL_OFFSET); table++)
    {
      if(page_directory[table])
      {
        expage_directory[table] = vmm_page_val(pmm_alloc_page(), \
          page_directory[table] & PAGE_FLAG_MASK);
        vmm_copy_page(page_directory[table] & PAGE_MASK, \
          expage_directory[table] & PAGE_MASK);
      }
    }

    // Point to the same page tables for the kernel space
    for(table = vmm_dir_idx(KERNEL_OFFSET); \
        table < VMM_PAGES_PER_TABLE - 2; table++)
    {
      expage_directory[table] = page_directory[table];
    }
    vmm_exdir_set(old_exdir);
  spin_unlock(&exdir_sem);

  return pd;
}


void vmm_init()
{
  // Setup the physical memory manager

  // Save the address of the kernel page directory (used as refference
  // for all others)
  __asm__ volatile("mov %%cr3, %0" : "=r" (kernel_pd));

  // Set current page directory to a copy of the kernel page directory.
  vmm_pd_set(vmm_clone_pd());
  vmm_running = TRUE;
}

