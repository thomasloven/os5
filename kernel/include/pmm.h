#pragma once
#include <stdint.h>
#include <multiboot.h>

#define PAGE_MASK 0xFFFFF000
#define PAGE_FLAG_MASK 0xFFF
#define PAGE_SIZE 0x1000

#ifndef __ASSEMBLER__

uintptr_t pmm_alloc_page();
void pmm_free_page(uintptr_t page);
void pmm_init(mboot_info_t *mboot);

#endif
