#include <k_debug.h>
#include <debug.h>
#include <pmm.h>
#include <vmm.h>


int test_debug()
{

  log_err("Testing error messages. %x", 0x123);
  log_warn("Testing warning messages. %x", 0x456);
  log_info("Testing info messages. %x", 0x789);

  check(2<1, "This test fails");
  
  return 0;

error:
  return -1;
}

int test_pmm()
{

  uintptr_t page1;
  uintptr_t page2;

  page1 = pmm_alloc_page();
  page2 = pmm_alloc_page();
  check(page1 != page2, "Allocated same page twice");
  pmm_free_page(page1);
  pmm_free_page(page2);

  page1 = pmm_alloc_page();
  pmm_free_page(page1);
  page2 = pmm_alloc_page();
  check(page1 == page2, "Page stack isn't working as expected");
  pmm_free_page(page2);

  pmm_free_page(0);
  page1 = pmm_alloc_page();
  check(page1 != 0, "Allocated null page");

  return 0;
error:
  log_err("PMM failed");

  pmm_free_page(page1);
  pmm_free_page(page2);

  return -1;
}

int test_vmm()
{
  uintptr_t page1 = pmm_alloc_page();
  uintptr_t page2 = pmm_alloc_page();
  char *ch1 = (char *)VMM_TEMP1;
  char *ch2 = (char *)VMM_TEMP2;

  log_info("Beginning VMM tests.");

  vmm_page_set(VMM_TEMP1, vmm_page_val(page1, PF_PRESENT | PF_WRITE));
  ch1[0] = 'a';
  vmm_page_set(VMM_TEMP2, vmm_page_val(page1, PF_PRESENT));
  check(ch2[0] == 'a', "Double mapping didn't work.");
  vmm_page_set(VMM_TEMP1, 0);
  vmm_page_set(VMM_TEMP2, 0);

  vmm_page_set(VMM_TEMP1, vmm_page_val(page1, PF_PRESENT | PF_WRITE));
  ch1[0] = 'b';
  vmm_page_set(VMM_TEMP1, 0);
  vmm_copy_page(page1, page2);
  vmm_page_set(VMM_TEMP2, vmm_page_val(page2, PF_PRESENT | PF_WRITE));
  check(ch2[0] == 'b', "VMM_COPY_PAGE failed.");
  vmm_page_set(VMM_TEMP2, 0);

  vmm_clear_page(page1);
  vmm_page_set(VMM_TEMP1, vmm_page_val(page1, PF_PRESENT | PF_WRITE));
  check(ch1[0] == '\0', "VMM_CLEAR_PAGE failed.");
  vmm_page_set(VMM_TEMP1, 0);

  log_info("Finished VMM tests.");

  pmm_free_page(page1);
  pmm_free_page(page2);

  return 0;
error:
  pmm_free_page(page1);
  pmm_free_page(page2);
  return -1;
}

int run_tests()
{

#ifndef NDEBUG
  check(test_debug() == -1, "Debugging functions test failed.");
  check(test_pmm() == 0, "PMM function test failed.");
  check(test_vmm() == 0, "VMM function test failed.");
#endif
  
  return 0;

error:
  log_err("Tests failed!");
  return -1;
}
