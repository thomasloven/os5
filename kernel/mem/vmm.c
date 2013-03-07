#include <vmm.h>
#include <pmm.h>
#include <stdint.h>
#include <memory.h>
#include <k_debug.h>
#include <ctype.h>

uint8_t vmm_running = 0;

uintptr_t *page_directory = (uintptr_t *)VMM_PAGE_DIR;
uintptr_t *page_tables = (uintptr_t *)VMM_PAGE_TABLES;

uintptr_t *expage_directory = (uintptr_t *)VMM_EXPAGE_DIR;
uintptr_t *expage_tables = (uintptr_t *)VMM_EXPAGE_TABLES;

void vmm_pd_set(page_dir_t pd)
{
  // Replace the page directory
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
    page_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
    if(page >= KERNEL_OFFSET && vmm_running)
    {
      // Add a copy of the page table to the kernel page directory
      page_dir_t old_exdir = vmm_exdir_set(kernel_pd);
      expage_directory[vmm_dir_idx(page)] = page_directory[vmm_dir_idx(page)];
      vmm_exdir_set(old_exdir);
    }

    vmm_flush_tlb((uintptr_t)&page_tables[vmm_table_idx(page)] & PAGE_MASK);
  memset((uintptr_t)&page_tables[vmm_table_idx(page)] & PAGE_MASK, 0, \
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
  memset((uintptr_t)&expage_tables[vmm_table_idx(page)] & PAGE_MASK, 0, \
    PAGE_SIZE);
  }
}

void vmm_expage_set(uintptr_t page, uintptr_t value)
{
  // Set the value of a page table entry in the currently loaded external page
  // directory
  page &= PAGE_MASK;

  vmm_expage_touch(page, value & PAGE_FLAG_MASK);
  expage_tables[vmm_table_idx(page)] = value;
  vmm_flush_tlb(page);
}


void vmm_clear_page(uintptr_t page)
{
  // Clear a physical page

  uintptr_t old = vmm_page_get(VMM_TEMP1);
  vmm_page_set(VMM_TEMP1, vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
  memset(VMM_TEMP1, 0, PAGE_SIZE),
  vmm_page_set(VMM_TEMP1, old);
}

void vmm_copy_page(uintptr_t src, uintptr_t dst)
{
  // Make a copy of a physical page
  
  uintptr_t old_temp1 = vmm_page_get(VMM_TEMP1);
  uintptr_t old_temp2 = vmm_page_get(VMM_TEMP2);
  vmm_page_set(VMM_TEMP1, vmm_page_val(src, PAGE_PRESENT | PAGE_WRITE));
  vmm_page_set(VMM_TEMP2, vmm_page_val(dst, PAGE_PRESENT | PAGE_WRITE));
  memcopy(VMM_TEMP1, VMM_TEMP2, PAGE_SIZE);
  vmm_page_set(VMM_TEMP1, old_temp1);
  vmm_page_set(VMM_TEMP2, old_temp2);
}

page_dir_t vmm_new_pd()
{
  page_dir_t pd = pmm_alloc_page();

  uintptr_t old = vmm_page_get(VMM_TEMP1);
  vmm_page_set(VMM_TEMP1, vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE));
  memset(VMM_TEMP1, 0, PAGE_SIZE);

  uint32_t *pdir = (uint32_t *)VMM_TEMP1;
  pdir[VMM_PAGES_PER_TABLE - 1] = vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE);

  vmm_page_set(VMM_TEMP1, old);

  return pd;
}

page_dir_t vmm_clone_pd()
{
  // Make a copy of the current page directory

  page_dir_t pd = vmm_new_pd();

  page_dir_t old_exdir = vmm_exdir_set(pd);

  uint32_t table;

  // ADD STUFF TO SHARE TABLES UNTIL WRITE HERE

  for(table = vmm_dir_idx(KERNEL_OFFSET); \
      table < VMM_PAGES_PER_TABLE - 2; table++)
  {
    expage_directory[table] = page_directory[table];
  }
  vmm_exdir_set(old_exdir);

  return pd;
}


void vmm_init()
{
  __asm__ volatile("mov %%cr3, %0" : "=r" (kernel_pd));

  vmm_pd_set(vmm_clone_pd());
  vmm_running = TRUE;
}

