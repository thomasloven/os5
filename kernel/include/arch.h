#pragma once
#include <stdint.h>

#ifndef __ASSEMBLER__

#define outb(port, val) \
  __asm__ volatile ("outb %%al, %0" : : "dN" ((uint16_t)(port)), "a" ((uint16_t)(val)))

#define outw(port, val) \
  __asm__ volatile ("outw %1, %0" : : "dN" ((uint16_t)(port)), "a" ((uint16_t)(val)))

#define inb(port) ({ \
  uint8_t __ret; \
  __asm__ volatile ("inb %1, %0" : "=a" (__ret) : "dN" ((uint16_t)(port))); \
  __ret; })

#define inw(port) ({ \
  uint16_t __ret; \
  __asm__ volatile ("inw %1, %0" : "=a" (__ret) : "dN" ((uint16_t)(port))); \
  __ret; })

#define enable_interrupts() __asm__("sti")

#define disable_interrupts() __asm__("cli");

typedef struct
{
uint32_t ds;
uint32_t edi, esi, ebp, esp;
uint32_t ebx, edx, ecx, eax;
uint32_t int_no, err_code;
uint32_t eip;
uint32_t cs;
uint32_t eflags, useresp, ss;
} registers_t;

#define print_registers(r) \
  debug("eax:%x ebx:%x ecx:%x edx:%x\n", (r)->eax, (r)->ebx, (r)->ecx, (r)->edx); \
  debug("edi:%x esi:%x ebp:%x esp:%x\n", (r)->edi, (r)->esi, (r)->ebp, (r)->esp); \
  debug("eip:%x\ncs:%x ds:%x ss:%x\n", (r)->eip, (r)->cs, (r)->ds, (r)->ss); \
  debug("useresp:%x\neflags%x %b\n", (r)->useresp, (r)->eflags, (r)->eflags); \
  debug("int_no:%x, err_code:%x %b\n", (r)->int_no, (r)->err_code, (r)->err_code);

#endif

#ifdef __ASSEMBLER__

.macro SetSegments val reg
  mov \val, %e\reg
  mov %\reg, %ds
  mov %\reg, %es
  mov %\reg, %fs
  mov %\reg, %gs
  mov %\reg, %ss
.endm

#endif
