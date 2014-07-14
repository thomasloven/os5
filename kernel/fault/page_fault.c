#include <stdint.h>
#include <idt.h>
#include <vmm.h>
#include <k_debug.h>
#include <thread.h>
#include <arch.h>
#include <procmm.h>
#include <signals.h>

registers_t *page_fault_handler(registers_t *r)
{
  // Page fault handler

  // Get the faulting address from the cr2 register
  uint32_t fault_address;
  __asm__ volatile("mov %%cr2, %0" : "=r" (fault_address));

  // If the procmm can handle the fault, let it do so and return to the
  // thread.
  if(!procmm_handle_page_fault(fault_address, r->err_code))
    return r;

  if(!(r->cs & 0x3))
  {
    // The faulting thread was running in kernel mode

    if(fault_address >= KERNEL_OFFSET && !(r->err_code & PF_PRESENT))
    {
      int handled = 0;
      spin_lock(&exdir_sem);
      page_dir_t old_exdir = vmm_exdir_set(kernel_pd);
      uintptr_t pt_val = vmm_extable_get(fault_address);
      if(pt_val & PAGE_PRESENT)
      {
        // Kernel tried to access a page table that's not in the current page
        // directory but exists in the master directory. So just copy it over.
        
        vmm_table_set(fault_address, pt_val);
        handled = 1;
      }
      vmm_exdir_set(old_exdir);
      spin_unlock(&exdir_sem);

      if(handled)
        return r;
    }

    // Processor was in kernel mode
    disable_interrupts();
    debug("[error]Page fault in kernel!\n");
    debug(" At: %x\n", fault_address);
    debug(" Code: %x (%s,%s,%s)\n", r->err_code, \
        (r->err_code & 0x4)?"user":"kernel", \
        (r->err_code & 0x2)?"write":"read", \
        (r->err_code & 0x1)?"protection":"non-present");
    print_registers(r);
    /*print_stack_trace();*/
    for(;;);

  } else {

    if(fault_address == SIGNAL_RETURN_ADDRESS)
    {
      return_from_signal(r);
    }

    // Processor was in user mode
    if(current->proc->flags & PROC_FLAG_DEBUG)
    {
      debug("[error]Page fault hapened!\n");
      debug("[error] At: %x\n", fault_address);
      debug("[error] Code: %x (%s,%s,%s)\n", r->err_code, \
          (r->err_code & 0x4)?"user":"kernel", \
          (r->err_code & 0x2)?"write":"read", \
          (r->err_code & 0x1)?"protection":"non-present");
      debug("[error] From thread: %x\n", current->tid);
      debug("[error] From process: %x\n", current->proc->pid);
      if(current->proc->cmdline)
        debug("Name: %s\n", current->proc->cmdline);
      print_registers(r);
    }

    signal_process(current->proc->pid, SIGSEGV);
    schedule();

    debug("[error]Shouldn't reach this point...\n");
    for(;;);
  }
}
