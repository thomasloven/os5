
[global _start]
[extern _init]

_start:
  push ecx
  push edx
  call _init
  jmp $
