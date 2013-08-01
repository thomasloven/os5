BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build

AS := nasm
CPP := i586-pc-myos-gcc -E
CC := i586-pc-myos-gcc
LD := i586-pc-myos-gcc
DEP := i586-pc-myos-gcc -MM

ASFLAGS := -f elf


export BUILDROOT BUILDDIR
export AS CPP CC LD DEP AR
export ASFLAGS

# .SILENT:

.PHONY: kernel kernel-clean tarfs tarfs-clean clean emul tags
.DEFAULT: all emul

default: all

all: kernel tarfs
    
kernel:
	$(MAKE) -C kernel
kernel-clean:
	$(MAKE) clean -C kernel

tarfs:
	$(MAKE) -C tarfs/bin
	tar -cf $(BUILDDIR)/tarfs.tar tarfs/*
tarfs-clean:
	$(MAKE) clean -C tarfs/bin
	rm $(BUILDDIR)/tarfs.tar

tags:
	ctags -R .

clean: kernel-clean tarfs-clean

emul:
	@util/qemul.sh

