#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <multiboot.h>
#include <heap.h>
#include <idt.h>
#include <arch.h>

void kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	pmm_init(mboot);
	idt_init();
	debug("Hello, world!\n%x%d",0x123ABC, 123);
	debug("\nheader%x\nmagic%x",mboot, mboot_magic);



	enable_interrupts();

	assert(3<2);


}
