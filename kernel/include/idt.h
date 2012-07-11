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

#define GDT_TSS 0x5

#define SEG_KERNEL_CODE	0x08
#define SEG_KERNEL_DATA	0x10
#define SEG_USER_CODE	0x18
#define SEG_USER_DATA	0x20
#define SEG_TSS	0x28

#define ISIRQ(num)	((num >= 32) && (num <= 47))
#define INT2IRQ(num)	(num - 32)
#define IRQ2INT(num)	(num + 32)

#define PIC_EOI	0x20

#define EFL_CPL3 0x3000
#define EFL_INT 0x200

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

typedef struct gdt_struct
{
	uint16_t limit_l;
	uint16_t base_l;
	uint8_t base_m;
	uint8_t access;
	uint8_t limit_h: 4;
	uint8_t flags :4;
	uint8_t base_h;
}__attribute__((packed)) gdt_entry_t;

typedef struct tss_struct
{
	uint32_t ptl;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t unused[15];
	uint32_t es, cs, ss, ds, fs, gs;
	uint32_t ldt;
	uint16_t trap, iomap;
}__attribute__((packed)) tss_t;

void idt_init();
void tss_init();

tss_t global_tss;

#define set_kernel_stack(stack) global_tss.esp0 = (uint32_t)(stack)

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
