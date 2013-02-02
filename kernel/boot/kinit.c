#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <heap.h>
#include <idt.h>
#include <arch.h>
#include <memory.h>
#include <debug.h>
#include <thread.h>
#include <scheduler.h>
#include <timer.h>
#include <elf.h>
#include <strings.h>
#include <process.h>
#include <procmm.h>

void _idle(void)
{
		debug("A");
	for(;;)
	{
		__asm__ ("sti; hlt");
	}
}
void _clock(void)
{

	uint16_t data[128];
		debug("B");
	for(;;)
	{
		int i;
		for(i=0; i<128; i++)
		{
			disable_interrupts();
			outb(0x70,i);
			data[i] = inb(0x71);
			enable_interrupts();
		}
		uint16_t h = ((data[4]/16)*10 + (data[4]&0xf));
		uint16_t m = ((data[2]/16)*10 + (data[2]&0xf));
		uint16_t s = ((data[0]/16)*10 + (data[0]&0xf));
			
		uint32_t x,y;
		kdbg_getpos(&x,&y);
		kdbg_setpos(70,0);
		debug("[%d:%d:%d]",h,m,s);
		kdbg_setpos(x,y);
	}

}

registers_t *kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	assert(mboot_magic == MBOOT_MAGIC2);

	kernel_elf_init(mboot);
	pmm_init(mboot);
	vmm_init();
	idt_init();
	tss_init();
	scheduler_init();
	timer_init(500);

	register_int_handler(INT_PF, page_fault_handler);
	register_int_handler(INT_SCHEDULE, switch_kernel_thread);

	process_t *p = process_init(&_idle);


	switch_process(p);
	//
	process_t *p2 = alloc_process();
	p2->pd = p->pd;
	//
	new_area(p, 0x1000, 0x5000, MM_FLAG_WRITE, MM_TYPE_CODE);
	share_area(p2, find_including(p, 0x2000));
	print_areas(p);
	print_areas(p2);
	split_area(find_including(p, 0x2000),0x2000, 0x4000);
	print_areas(p);
	print_areas(p2);
	split_area(find_including(p2, 0x3000),0x3000, 0x4000);
	print_areas(p);
	print_areas(p2);

	new_area(p, 0x6000, 0x8000, MM_FLAG_WRITE, MM_TYPE_CODE);
	new_area(p, 0x3000, 0x5000, MM_FLAG_WRITE, MM_TYPE_CODE);
	print_areas(p);
	debug("Area start: %x", (find_above(p, 0x3800))->start);

	/*print_areas(p);*/
	/*join_area(find_including(p, 0x3000));*/
	/*print_areas(p);*/
	//new_thread(&_clock,0);



	return switch_kernel_thread(0);
}

