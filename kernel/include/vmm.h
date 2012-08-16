#pragma once
#include <stdint.h>
#include <pmm.h>

#define USER_STACK_BOTTOM 0xB0000000
#define USER_STACK_TOP 0xC0000000
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

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

#define VMM_PAGES_PER_TABLE 1024

#define vmm_page_val(page, flags) \
	((page & PAGE_MASK) | (flags & PAGE_FLAG_MASK))
#define vmm_table_idx(page) (page >> 12)
#define vmm_dir_idx(page) (page >> 22)

#ifdef __ASSEMBLER__

#define vmm_flush_tlb(page) invlpg[page]

#else

#define vmm_flush_tlb(page) \
	__asm__ volatile ("invlpg (%0)" : : "a" (page & PAGE_MASK))

#define assert_higher(val)  \
	((uintptr_t)(val) > KERNEL_OFFSET)?(val):\
		(__typeof__((val)))((uintptr_t)(val)+ KERNEL_OFFSET)

uintptr_t vmm_page_get(uintptr_t page);
void vmm_page_set(uintptr_t page, uintptr_t value);
uintptr_t vmm_clone_pd();
void vmm_set_pd(uintptr_t pd);

#endif
