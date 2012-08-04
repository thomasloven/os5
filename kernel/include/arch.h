#pragma once
#include <stdint.h>

#ifndef __ASSEMBLER__

#define outb(port, val) \
	asm volatile ("outb %%al, %0" : : "dN" ((uint16_t)port), "a" ((uint16_t)val))

#define outw(port, val) \
	asm volatile ("outw %1, %0" : : "dN" ((uint16_t)port), "a" ((uint16_t)val))

#define inb(port) ({ \
	uint8_t __ret; \
	asm volatile ("inb %1, %0" : "=a" (__ret) : "dN" ((uint16_t)port)); \
	__ret; })

#define inw(port) ({ \
	uint16_t __ret; \
	asm volatile ("inw %1, %0" : "=a" (__ret) : "dN" ((uint16_t)port)); \
	__ret; })

#define enable_interrupts() asm("sti")

#define disable_interrupts() asm("cli");

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
	debug("\n\neax:%x ebx:%x ecx:%x edx:%x", (r)->eax, (r)->ebx, (r)->ecx, (r)->edx); \
	debug("\nedi:%x esi:%x ebp:%x esp:%x", (r)->edi, (r)->esi, (r)->ebp, (r)->esp); \
	debug("\neip:%x\ncs:%x ds:%x ss:%x", (r)->eip, (r)->cs, (r)->ds, (r)->ss); \
	debug("\nuse(r)esp:%x\neflags%x %b", (r)->useresp, (r)->eflags, (r)->eflags); \
	debug("\nint_no:%x, err_code:%x %b", (r)->int_no, (r)->err_code, (r)->err_code);

#endif

#ifdef __ASSEMBLER__

%macro SetSegments 2
	mov e%2, %1
	mov ds, %2
	mov es, %2
	mov fs, %2
	mov gs, %2
	mov ss, %2
%endmacro

#endif
