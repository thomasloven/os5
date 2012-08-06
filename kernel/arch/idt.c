#include <idt.h>
#include <stdint.h>
#include <arch.h>
#include <k_debug.h>
#include <memory.h>
#include <thread.h>

idt_entry_t idt[NUM_INTERRUPTS];
int_handler_t int_handlers[NUM_INTERRUPTS];
struct idt_pointer idt_p;
extern gdt_entry_t gdt[6];

extern void isr0(void), isr1(void), isr2(void), isr3(void), isr4(void), isr5(void), isr6(void), isr7(void), isr8(void), isr9(void), isr10(void), isr11(void), isr12(void), isr13(void), isr14(void), isr15(void), isr16(void), isr17(void), isr18(void), isr19(void), isr20(void), isr21(void), isr22(void), isr23(void), isr24(void), isr25(void), isr26(void), isr27(void), isr28(void), isr29(void), isr30(void), isr31(void), isr32(void), isr33(void), isr34(void), isr35(void), isr36(void), isr37(void), isr38(void), isr39(void), isr40(void), isr41(void), isr42(void), isr43(void), isr44(void), isr45(void), isr46(void), isr47(void), isr128(void), isr130(void), isr255(void);

void *idt_raw[] = 
{
	isr0,	isr1,	isr2,	isr3,	isr4,
	isr5,	isr6,	isr7,	isr8,	isr9, //9
	isr10,	isr11,	isr12,	isr13,	isr14,
	isr15,	isr16,	isr17,	isr18,	isr19, //19
	isr20,	isr21,	isr22,	isr23,	isr24,
	isr25,	isr26,	isr27,	isr28,	isr29, //29
	isr30,	isr31,	isr32,	isr33,	isr34,
	isr35,	isr36,	isr37,	isr38,	isr39, //39
	isr40,	isr41,	isr42,	isr43,	isr44,
	isr45,	isr46,	isr47,	0,	0, //49
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //59
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //69
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //79
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //89
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //99
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //109
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //119
	0,	0,	0,	0,	0,
	0,	0,	0,	isr128,	0, //129
	isr130,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //139
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //149
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //159
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //169
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //179
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //189
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //199
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //209
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //219
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //229
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //239
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //249
	0,	0,	0,	0,	0,
	isr255
};

void idt_set(uint32_t num, uint32_t base, uint32_t segment, uint8_t flags)
{
	if(!base) return;

	idt_set_base(idt[num], base);
	idt[num].segment = segment;
	idt[num].reserved = 0;
	idt[num].flags = flags;
}

void idt_init()
{
	outb(MPIC_CMD_PORT, 0x11);
	outb(SPIC_CMD_PORT, 0x11);
	outb(MPIC_DATA_PORT, 0x20);
	outb(SPIC_DATA_PORT, 0x28);
	outb(MPIC_DATA_PORT, 0x04);
	outb(SPIC_DATA_PORT, 0x02);
	outb(MPIC_DATA_PORT, 0x01);
	outb(SPIC_DATA_PORT, 0x01);

	outb(MPIC_DATA_PORT, 0x0);
	outb(SPIC_DATA_PORT, 0x0);

	idt_p.size = (sizeof(idt_entry_t)*NUM_INTERRUPTS) - 1;
	idt_p.offset = (uint32_t)&idt;

	memset(idt,0,sizeof(idt_entry_t)*NUM_INTERRUPTS);
	memset(int_handlers,0,sizeof(int_handler_t)*NUM_INTERRUPTS);

	uint32_t i;
	for(i = 0; i < NUM_INTERRUPTS; i++)
	{
		if(idt_raw[i])
		{
			idt_set(i, (uint32_t)idt_raw[i], SEG_KERNEL_CODE, IDT_PRESENT | IDT_INT_GATE);
		}
	}

	idt[128].flags |=  IDT_DPL_3;
	idt[255].flags |= IDT_DPL_3;

	idt_flush((uint32_t)&idt_p);
}

registers_t *idt_handler(registers_t *r)
{
	if(ISIRQ(r->int_no))
	{
		if(INT2IRQ(r->int_no) > 8)
			outb(SPIC_CMD_PORT, PIC_EOI);
		outb(MPIC_CMD_PORT, PIC_EOI);

		if(INT2IRQ(r->int_no) != 0)
			debug("!");
	} 

	if(int_handlers[r->int_no])
	{
		enable_interrupts();
		registers_t *ret = int_handlers[r->int_no](r);
		if ((ret->cs & 0x3) == 0x3)
		{
			set_kernel_stack(stack_from_tcb(current));
		}
			ret->eflags |= EFL_INT;
		return ret;
	} else {
		if(!ISIRQ(r->int_no))
		{
			debug("\nUnhanded interrupt received, %x, %x", r->int_no, INT2IRQ(r->int_no));
			debug("\n Tid: %x", current->tid);
			print_registers(r);
			enable_interrupts();
			for(;;);
		}
	}

	return r;
}

int_handler_t register_int_handler(uint32_t num, int_handler_t handler)
{
	int_handler_t old = int_handlers[num];
	int_handlers[num] = handler;
	return old;
}

void tss_init()
{
	uint32_t base = (uint32_t)&global_tss;
	uint32_t limit = sizeof(tss_t);

	set_gdt_base(gdt[GDT_TSS], base);
	set_gdt_limit(gdt[GDT_TSS], limit);
	gdt[GDT_TSS].flags = 0;
	gdt[GDT_TSS].access = 0x89;

	memset(&global_tss, 0, sizeof(tss_t));
	global_tss.ss0 = SEG_KERNEL_DATA;
	global_tss.iomap = sizeof(tss_t);

	tss_flush(SEG_TSS);
}

