file build/kernel/kernel
target remote localhost:1234
break idt.c:186
break page_fault.c:38

