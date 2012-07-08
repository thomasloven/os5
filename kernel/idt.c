#include <idt.h>
#include <stdint.h>
#include <arch.h>
#include <k_debug.h>
#include <memory.h>

idt_entry_t idt[NUM_INTERRUPTS];
struct idt_pointer idt_p;

extern void isr0(void), isr1(void), isr2(void), isr3(void), isr4(void), isr5(void), isr6(void), isr7(void), isr8(void), isr9(void), isr10(void), isr11(void), isr12(void), isr13(void), isr14(void), isr15(void), isr16(void), isr17(void), isr18(void), isr19(void), isr20(void), isr21(void), isr22(void), isr23(void), isr24(void), isr25(void), isr26(void), isr27(void), isr28(void), isr29(void), isr30(void), isr31(void), isr32(void), isr33(void), isr34(void), isr35(void), isr36(void), isr37(void), isr38(void), isr39(void), isr40(void), isr41(void), isr42(void), isr43(void), isr44(void), isr45(void), isr46(void), isr47(void);

void *idt_raw[] = 
{
	isr0,	isr1,	isr2,	isr3,	isr4,
	isr5,	isr6,	isr7,	isr8,	isr9, //10
	isr10,	isr11,	isr12,	isr13,	isr14,
	isr15,	isr16,	isr17,	isr18,	isr19, //20
	isr20,	isr21,	isr22,	isr23,	isr24,
	isr25,	isr26,	isr27,	isr28,	isr29, //30
	isr30,	isr31,	isr32,	isr33,	isr34,
	isr35,	isr36,	isr37,	isr38,	isr39, //40
	isr40,	isr41,	isr42,	isr43,	isr44,
	isr45,	isr46,	isr47,	0,	0, //50
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //60
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //70
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //80
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //90
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //100
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //110
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //120
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //130
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //140
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //150
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //160
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //170
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //180
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //190
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //200
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //210
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //220
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //230
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //240
	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0, //250
	0,	0,	0,	0,	0,
	0
};

void idt_set(uint32_t num, uint32_t base, uint32_t segment, uint8_t flags)
{
	if(!base) return;

	idt[num].base_low = (uint16_t)(base & 0xFFFF);
	idt[num].base_high = (uint16_t)(base >> 16);
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

	uint32_t i;
	for(i = 0; i < NUM_INTERRUPTS; i++)
	{
		if(idt_raw[i])
		{
			idt_set(i, (uint32_t)idt_raw[i], SEG_KERNEL_CODE, IDT_PRESENT | IDT_INT_GATE);
		}
	}

	idt_flush((uint32_t)&idt_p);
}

registers_t *idt_handler(registers_t *r)
{
	if(ISIRQ(r->int_no))
	{
		if(INT2IRQ(r->int_no) > 8)
			outb(SPIC_CMD_PORT, PIC_EOI);
		outb(MPIC_CMD_PORT, PIC_EOI);
	}
	debug("\nInterrupt received, %x, %x", r->int_no, INT2IRQ(r->int_no));
	if(r->int_no != 32)
	{
		print_registers(r);
		for(;;);
	}
	return r;
}
