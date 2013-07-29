
[global _start]
[extern _init]

_start:
  push ecx
  call _init
  jmp $
