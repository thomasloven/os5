#pragma once

#define TRUE 1
#define FALSE 0
#define NULL 0

#ifndef __ASSEMBLER__

#define iscntrl(c) (((int)c>=0 && (int)c<=0x1F) || c==0x7F)?TRUE:FALSE
#define isspace(c) (((int)c>=0x9 && (int)c<=0x0D) || c==0x20)?TRUE:FALSE
#define isupper(c) ((int)c>=0x41 && (int)c<=0x5A)?TRUE:FALSE
#define islower(c) ((int)c>=0x61 && (int)c<=0x7A)?TRUE:FALSE
#define isalpha(c) (isupper(c) || islower(c))?TRUE:FALSE
#define isdigit(c) ((int)c>=0x30 && (int)c<=0x39)?TRUE:FALSE
#define isxdigit(c) (isdigit(c) || ((int)c>=0x41 && (int)c<0x46) || ((int)c>=0x61 && (int)c<=0x66))?TRUE:FALSE
#define isalnum(c) (isdigit(c) || isalpha(c))?TRUE:FALSE
#define ispunct(c) (((int)c>=0x21 && (int)c<=0x2F) || ((int)c>=0x3A && (int)c<=0x40) || ((int)c>=0x5B && (int)c<=0x60) || ((int)c>=0x7B && (int)c<=0x7E))?TRUE:FALSE
#define isgraph(c) (isalnum(c) || ispunct(c))?TRUE:FALSE
#define isprint(c) ((int)c>=0x20 && (int)c<=0x7E)?TRUE:FALSE

#endif
