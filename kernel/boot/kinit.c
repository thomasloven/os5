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
    debug("C");
  for(;;)
  {
    __asm__ ("sti; hlt");
  }
}

void usermode_proc(void)
{
  int a = 0;
  a ++;
  for(;;);
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

  process_t *p = process_init((void(*)(void))0x1000);
  switch_process(p);

  new_area(p, 0x1000, 0x5000, MM_FLAG_WRITE, MM_TYPE_CODE);
  new_area(p, 0xC0000000, 0xC0000000, MM_FLAG_WRITE | MM_FLAG_GROWSDOWN | MM_FLAG_ADDONUSE, MM_TYPE_STACK);
  memcopy(0x1000, &usermode_proc, 0x1000);
  print_areas(p);

  new_thread(&_clock,0);
  new_thread(&_idle,0);

  return switch_kernel_thread(0);
}

