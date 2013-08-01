BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build

AS := nasm
CPP := i586-pc-myos-gcc -E
CC := i586-pc-myos-gcc
LD := i586-pc-myos-gcc
DEP := i586-pc-myos-gcc -MM


export BUILDROOT BUILDDIR
export AS CPP CC LD DEP AR

# .SILENT:

.PHONY: kernel kernel-clean tarfs tarfs-clean clean emul tags
.DEFAULT: all emul

default: all

all: kernel tarfs
    
kernel:
	$(MAKE) -C build/kernel -f ../../kernel/Makefile
kernel-clean:
	$(MAKE) clean -C build/kernel -f ../../kernel/Makefile

tarfs:
	$(MAKE) -C tarfs/bin -f ../src/Makefile
	tar -cf $(BUILDDIR)/tarfs.tar tarfs/*
tarfs-clean:
	$(MAKE) clean -C tarfs/bin -f ../src/Makefile
	rm $(BUILDDIR)/tarfs.tar

tags:
	ctags -R .

clean: kernel-clean tarfs-clean

emul:
	@util/qemul.sh

