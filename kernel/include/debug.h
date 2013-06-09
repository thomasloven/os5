#pragma once

#ifndef __ASSEMBLER__

#define BREAK asm volatile ("xchg %bx, %bx;");

#endif
#ifdef __ASSEMBLER__

#define BREAK xchg bx, bx

#endif
