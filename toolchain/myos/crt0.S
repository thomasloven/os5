#ifndef KERNEL_MODE
.global _start
.extern _init
.extern _exit
_start:
  push %ecx
  call _init
  call _exit
  .wait: hlt
  jmp .wait

#endif
