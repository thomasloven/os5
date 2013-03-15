
[global _start]
[extern _init]

_start:
  call _init
  jmp $
