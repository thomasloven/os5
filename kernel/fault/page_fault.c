#include <stdint.h>
#include <idt.h>
#include <vmm.h>
#include <k_debug.h>
#include <thread.h>
#include <arch.h>
#include <procmm.h>

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
    debug("Page fault in kernel!");
    debug("\n At: %x", fault_address);
    print_registers(r);
    /*print_stack_trace();*/
    for(;;);

  } else {

    // Processor was in user mode
    // This should just kill the process, or at least send it a signal,
    // once those are implemented.

      disable_interrupts();
      debug("Page fault hapened!");
      debug("\n At: %x", fault_address);
      debug("\n Code: %x (%s,%s,%s)", r->err_code, \
          (r->err_code & 0x4)?"user":"kernel", \
          (r->err_code & 0x2)?"write":"read", \
          (r->err_code & 0x1)?"protection":"non-present");
      debug("\n From thread: %x", current->tid);
      debug("\n From process: %x", current->proc->pid);
      print_registers(r);
      for(;;);
  }
}
