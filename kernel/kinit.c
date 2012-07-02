#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <multiboot.h>
#include <heap.h>

void kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	pmm_init(mboot);
	debug("Hello, world!\n%x%d",0x123ABC, 123);
	debug("\nheader%x\nmagic%x",mboot, mboot_magic);
uint32_t a = pmm_alloc_page();
uint32_t b = pmm_alloc_page();
uint32_t c = pmm_alloc_page();
pmm_free_page(b);
uint32_t d = pmm_alloc_page();

debug("\na:%x\nb:%x\nc:%x\nd:%x",a,b,c,d);

a = kmalloc(5);
b = kmalloc(3);
c = kmalloc(10);
kfree(b);
d = kmalloc(2);

debug("\na:%x\nb:%x\nc:%x\nd:%x",a,b,c,d);

	assert(3<2);


}
