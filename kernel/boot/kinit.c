#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
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
#include <keyboard.h>
#include <tarfs.h>

#include <stdio.h>

void _idle(void)
{
  // Idle task. Runs when there are no others on the run queue.
  for(;;)
  {
    __asm__ ("sti; hlt");
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
  register_int_handler(INT_SCHEDULE, switch_kernel_thread);

  scheduler_init();
  timer_init(500);
  vfs_init();
  syscall_init();
  process_init((void(*)(void))&_idle);

  keyboard_init();


  mboot_mod_t *mods = (mboot_mod_t *)(assert_higher(mboot->mods_addr));
  elf_header *init_module = assert_higher((elf_header *)mods[0].mod_start);

  tar_header_t *tarfs_location = assert_higher((tar_header_t *)mods[1].mod_start);

  vfs_mount("/tarfs", tarfs_init(tarfs_location));

  vfs_print_tree();

  load_elf(init_module);

  thread_t *init = new_thread((void(*)(void))current->proc->mm.code_entry,1);
  init->proc = current->proc;

  new_area(current->proc, USER_STACK_TOP, USER_STACK_TOP, MM_FLAG_WRITE | MM_FLAG_GROWSDOWN | MM_FLAG_ADDONUSE, MM_TYPE_STACK);

  fopen("/dev/kbd", "r");
  fopen("/dev/debug", "w");
  fopen("/dev/debug", "w");

  printf("\nPrintf from kernel!\n");

  return switch_kernel_thread(0);
}
