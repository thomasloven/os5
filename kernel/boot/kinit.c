#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <idt.h>
#include <arch.h>
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
#include <version.h>

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

  mboot_mod_t *mods = (mboot_mod_t *)(assert_higher(mboot->mods_addr));

  kernel_elf_init(mboot);
  pmm_init(mboot);
  vmm_init(mods[0].mod_end);
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


  tar_header_t *tarfs_location = assert_higher((tar_header_t *)mods[0].mod_start);

  vfs_mount("/", tarfs_init(tarfs_location));

  fopen("/dev/kbd", "r");
  fopen("/dev/debug", "w");
  fopen("/dev/debug", "w");

  execve("/bin/init",0,0);

  debug("[status]========================\n");
  debug("[status]Os5 by Thomas Lovén\n");
  debug("[status]Kernel git data: [%s (%s)] %s\n", __kernel_git_hash, (__kernel_git_dirty)?"dirty":"clean", __kernel_git_date);
  debug("[status]%s: %s\n", __kernel_git_branch, __kernel_git_message);
  debug("[status]Kernel compilation: %s %s\n", __kernel_build_date, __kernel_build_time);


  thread_t *init = new_thread((void(*)(void))current->proc->mm.code_entry,1);
  init->proc = current->proc;

  return switch_kernel_thread(0);
}
