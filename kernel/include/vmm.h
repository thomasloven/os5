#pragma once
#include <stdint.h>
#include <pmm.h>
#include <synch.h>

#define USER_STACK_BOTTOM 0xB0000000
#define USER_STACK_TOP 0xBFFFF000
#define KERNEL_OFFSET 0xC0000000
#define KERNEL_HEAP_START 0xE0000000
#define KERNEL_HEAP_END 0xF0000000
#define PMM_PAGE_STACK 0xFE000000
#define VMM_TEMP2 0xFF7FE000
#define VMM_TEMP1 0xFF7FF000
#define VMM_EXPAGE_TABLES 0xFF800000
#define VMM_EXPAGE_DIR 0xFFBFF000
#define VMM_PAGE_TABLES 0xFFC00000
#define VMM_PAGE_DIR 0xFFFFF000

#define SIGNAL_RETURN_ADDRESS 0xDEAD5165
#define THREAD_RETURN_ADDRESS 0xDEAD3EAD

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

#define VMM_PAGES_PER_TABLE 1024

#define PF_PRESENT 0x1
#define PF_WRITE 0x2
#define PF_USER 0x4

#define vmm_page_val(page, flags) \
  ((page & PAGE_MASK) | (flags & PAGE_FLAG_MASK))
#define vmm_table_idx(page) (page >> 12)
#define vmm_dir_idx(page) (page >> 22)

#ifdef __ASSEMBLER__

#define vmm_flush_tlb(page) invlpg (page)

#else

#define vmm_flush_tlb(page) \
  __asm__ volatile ("invlpg (%0)" : : "a" (page & PAGE_MASK))

#define assert_higher(val)  \
  ((uint32_t)(val) > KERNEL_OFFSET)?(val):(__typeof__((val)))((uint32_t)(val) \
    + KERNEL_OFFSET)

typedef uintptr_t page_dir_t;

page_dir_t kernel_pd;
semaphore_t kernel_pd_sem;
semaphore_t exdir_sem;
semaphore_t temp_sem; // VMM_TEMPn addresses

void vmm_pd_set(page_dir_t pd);
uintptr_t vmm_page_get(uintptr_t page);
uintptr_t vmm_table_get(uintptr_t page);
void vmm_table_set(uintptr_t page, uintptr_t value);
void vmm_page_set(uintptr_t page, uintptr_t value);

page_dir_t vmm_exdir_set(page_dir_t pd);
uintptr_t vmm_expage_get(uintptr_t page);
uintptr_t vmm_extable_get(uintptr_t page);
void vmm_extable_set(uintptr_t page, uintptr_t value);
void vmm_expage_set(uintptr_t page, uintptr_t value);

void vmm_clear_page(uintptr_t page);
void vmm_copy_page(uintptr_t src, uintptr_t dst);

page_dir_t vmm_clone_pd();

void vmm_init(uint32_t kernel_size);

#endif
