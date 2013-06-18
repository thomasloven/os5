#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <k_heap.h>
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
#include <k_syscall.h>
#include <synch.h>
#include <vfs.h>

void _idle(void)
{
  for(;;)
  {
    __asm__ ("sti; hlt");
  }
}

void _clock(void)
{

  uint16_t data[128];
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
    spin_lock(&debug_sem);
    kdbg_getpos(&x,&y);
    kdbg_setpos(70,0);
    debug("[%d:%d:%d]",h,m,s);
    kdbg_setpos(x,y);
    spin_unlock(&debug_sem);
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
  register_int_handler(INT_PF, page_fault_handler);

  scheduler_init();
  register_int_handler(INT_SCHEDULE, switch_kernel_thread);
  timer_init(500);

  vfs_init();
  vfs_print_tree();
  syscall_init();

  process_init((void(*)(void))&_idle);

  thread_t *clk = new_thread(&_clock,0);
  clk->proc = current->proc;

  mboot_mod_t *mods = (mboot_mod_t *)(assert_higher(mboot->mods_addr));
  mods->mod_start = assert_higher(mods->mod_start);

  load_elf((elf_header *)mods->mod_start);
  thread_t *idle = new_thread((void(*)(void))current->proc->mm.code_entry,1);
  idle->proc = current->proc;
  new_area(current->proc, USER_STACK_TOP, USER_STACK_TOP, MM_FLAG_WRITE | MM_FLAG_GROWSDOWN | MM_FLAG_ADDONUSE, MM_TYPE_STACK);


  return switch_kernel_thread(0);
}

