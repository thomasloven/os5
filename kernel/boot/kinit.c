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
#include <k_syscalls.h>
#include <syscalls.h>

mboot_mod_t *modules;

void _idle()
{
		debug("A");
		if(_syscall_fork())
		{
			debug("Fork returned to parent");
		} else {
	debug("Loading elf from %x \n",modules[0].mod_start);
	/*load_elf((elf_header *)(assert_higher(modules[0].mod_start)), &proc->elf);*/
	_syscall_execv(assert_higher(modules[0].mod_start),0,0);
	}
	debug("\n pid:%x", _syscall_getpid());

	/*thread_t *init = new_thread((void *)proc->elf.entry,1);*/
	/*debug("\n Init thread %x", init);*/
	/*debug("\n proc %x ", init->proc);*/

	for(;;)
	{
		__asm__ ("sti; hlt");
	}
}

// Print a clock in upper right corner
void _clock()
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
	idt_init();
	tss_init();
	scheduler_init();
	timer_init(500);

	register_int_handler(INT_PF, page_fault_handler);
	register_int_handler(INT_SCHEDULE, switch_kernel_thread);

	syscalls_init();
	KREG_SYSCALL(putch, SYSCALL_PUTCH);
	KREG_SYSCALL(getpid, SYSCALL_GETPID);
	KREG_SYSCALL(fork, SYSCALL_FORK);
	KREG_SYSCALL(execv, SYSCALL_EXECV);

	
	/*thread_t *init_thread = threads_init((void *)&_idle);*/
	/*new_thread((void *)&_clock,0);*/

	/*process_t *init_proc = process_init(init_thread);*/
	process_init((void *)&_idle);

	modules = (mboot_mod_t *)(assert_higher(mboot->mods_addr));


	return switch_kernel_thread(0);
}

