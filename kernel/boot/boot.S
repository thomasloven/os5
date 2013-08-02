#define _STDINT_H
#include <multiboot.h>
#include <vmm.h>
#include <arch.h>
#include <pmm.h>
#include <debug.h>

.section .bss

.align 0x8

# Stack for booting
.global BootStack
BootStackTop:
  .lcomm buff, 2*PAGE_SIZE - 1
BootStack:

.section .data

.align 0x1000

# Page directory for booting up.
# First four megabytes are identity mapped as well as
# mapped to 0xC0000000
.global BootPageDirectory
BootPageDirectory:
  .long (BootPageTable - KERNEL_OFFSET)+(PAGE_PRESENT | PAGE_WRITE)
  .rept (vmm_dir_idx(KERNEL_OFFSET) - 1)
    .long 0x0
  .endr
  .long (BootPageTable - KERNEL_OFFSET)+(PAGE_PRESENT | PAGE_WRITE)
  .rept (1022 - vmm_dir_idx(KERNEL_OFFSET))
    .long 0x0
  .endr
  .long (BootPageDirectory - KERNEL_OFFSET)+(PAGE_PRESENT | PAGE_WRITE)

BootPageTable:
  .set i, 0
  .rept 1024
    .long((i << 12) | PAGE_PRESENT | PAGE_WRITE)
    .set i, (i+1)
  .endr

# Hard-coded GDT.
# GDT pointer is wrapped into the first entry
.global gdt
gdt_ptr:
gdt:
  .short 0x002F
  .long gdt
  .short 0x0000
  .long 0x0000FFFF, 0x00CF9A00
  .long 0x0000FFFF, 0x00CF9200
  .long 0x0000FFFF, 0x00CFFA00
  .long 0x0000FFFF, 0x00CFF200
  .long 0x00000000, 0x00000000

.section .text

.align 4

# GRUB Multiboot data
MultiBootHeader:
  .long MBOOT_MAGIC1
  .long MBOOT_HEADER_FLAGS
  .long MBOOT_HEADER_CHECKSUM

# Kernel start point
.global start
.func start
start:
  cli

# Load page directory and enable paging
  mov $(BootPageDirectory - KERNEL_OFFSET), %ecx
  mov %ecx, %cr3
  mov %cr0, %ecx
  or $0x80000000, %ecx
  mov %ecx, %cr0
  lea (.higherHalf), %ecx
  jmp *%ecx

.higherHalf:
  # Load GDT
  mov $gdt_ptr, %ecx
  lgdt (%ecx)

  SetSegments $0x10, cx
  jmp $0x8, $.gdtLoaded

.gdtLoaded:
  # Clear the identity mapping from the page directory
  mov $BootPageDirectory, %edx
  xor %ecx, %ecx
  mov %ecx, (%edx)
  vmm_flush_tlb(0)

  # Load a stack for booting
  mov $BootStack, %esp
  mov $0x0, %ebp

  # eax contains the magic number from GRUB 0x2BADB002
  push %eax

  # ebx contains the address of the Multiboot information structure
  add $KERNEL_OFFSET, %ebx
  push %ebx

  # Call the c function for setting up
.extern kinit
call kinit
mov %eax, %esp
.extern int_return
jmp int_return
.loop:
jmp .loop
.end
.endfunc
