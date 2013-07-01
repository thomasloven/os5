file build/kernel/kernel
target remote localhost:1234
break idt.c:217
break page_fault.c:45
break pmm.c:35

