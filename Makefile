BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build
LIBDIR := $(BUILDROOT)/library

BINUTILS := /usr/local/Cellar/binutils/2.23.2/i386-elf/bin

AS := nasm
CPP := clang -E
CC := clang
LD := i386-elf-ld
DEP := clang -MM
AR := i386-elf-ar

ASFLAGS := -f elf

CPPFLAGS := -B $(BINUTILS)
CPPFLAGS += -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CPPFLAGS += -fno-stack-protector -ffreestanding -Wno-unused-function
CPPFLAGS += -Wno-unused-parameter -g -Wno-gnu -nostdlib
CPPFLAGS += -I$(LIBDIR)/include
CPPFLAGS += -I$(BUILDDIR)/lib/i386-elf/include

CFLAGS := -target i386-pc-linux -mno-sse -mno-mmx
CFLAGS += -ggdb

LDFLAGS := -B $(BINUTILS) -target i386-pc-linux

LIBPATH := $(BUILDDIR)/lib/i386-elf/lib/libc.a $(BUILDDIR)/library/clib.a
LDLIBS := $(LIBPATH) $(LIBPATH)

DEPFLAGS := $(CPPFLAGS)

ARFLAGS := -rc

export BUILDROOT BUILDDIR LIBDIR LIBPATH1 LIBPATH
export AS CPP CC LD DEP AR
export ASFLAGS CPPFLAGS CFLAGS LDFLAGS LDLIBS DEPFLAGS ARFLAGS

# .SILENT:

.PHONY: kernel kernel-clean library library-clean tarfs tarfs-clean clean-all emul
.DEFAULT: all emul

default: all

all: kernel library tarfs
    
kernel:
	$(MAKE) -C kernel
kernel-clean:
	$(MAKE) clean -C kernel

library:
	$(MAKE) -C library
library-clean:
	$(MAKE) clean -C library

tarfs: library
	$(MAKE) -C tarfs/bin
	tar -cf $(BUILDDIR)/tarfs.tar tarfs/*
tarfs-clean:
	$(MAKE) clean -C tarfs/bin

clean-all: kernel-clean library-clean tarfs-clean

emul:
	@echo "  \033[35mSTARTING EMULATOR\033[0m"
	@tools/qemul.sh

