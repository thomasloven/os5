BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build

CC := i586-pc-myos-gcc
DEP := $(CC) -MM

CFLAGS := -ggdb

export BUILDROOT BUILDDIR
export CC DEP CFLAGS

# .SILENT:

.PHONY: kernel kernel-clean tarfs tarfs-clean clean emul tags


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

