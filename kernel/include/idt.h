#pragma once
#include <stdint.h>

#define NUM_INTERRUPTS 255

#define MPIC_CMD_PORT	0x20
#define MPIC_DATA_PORT	0x21
#define SPIC_CMD_PORT	0xA0
#define SPIC_DATA_PORT	0xA1

#define IDT_PRESENT	0x80
#define IDT_DPL_0	0x00
#define IDT_DPL_1	0x20
#define IDT_DPL_2	0x40
#define IDT_DPL_3	0x60

#define IDT_TASK_GATE	0x05
#define IDT_INT_GATE	0x0E
#define IDT_TRAP_GATE	0x0F

#define SEG_KERNEL_CODE	0x08
#define SEG_KERNEL_DATA	0x10
#define SEG_USER_CODE	0x18
#define SEG_USER_DATA	0x20


#ifndef __ASSEMBLER__

typedef struct interrupt_descriptor
{
	uint16_t base_low;
	uint16_t segment;
	uint8_t reserved;
	uint8_t flags;
	uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

struct idt_pointer
{
	uint16_t size;
	uint32_t offset;
} __attribute__((packed));

void idt_init();
#else

%macro INTNOERR 1
	[global isr%1]
	isr%1:
		cli
		push 0
		push %1
		jmp int_stub
%endmacro

%macro INTERR 1
	[global isr%1]
	isr%1:
		cli
		push %1
		jmp int_stub
%endmacro

#endif
