BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build

CC := i586-pc-myos-gcc
DEP := $(CC) -MM

CFLAGS := -ggdb
# -Wall -Wextra -pedantic -std=c99 -U__STRICT_ANSI__ -Werror

export BUILDROOT BUILDDIR
export CC DEP CFLAGS

# .SILENT:

.PHONY: kernel-clean tarfs-clean clean emul


all: kernel tarfs tags
    
kernel: $(BUILDDIR)/kernel/kernel
$(BUILDDIR)/kernel/kernel:
	$(MAKE) -C build/kernel -f ../../kernel/Makefile
kernel-clean:
	$(MAKE) clean -C build/kernel -f ../../kernel/Makefile


tarfs: $(BUILDDIR)/tarfs.tar
$(BUILDDIR)/tarfs.tar:
	@mkdir -p tarfs/bin
	@mkdir -p tarfs/dev
	@mkdir -p tarfs/mnt
	@mkdir -p tarfs/mnt/tarfs
	$(MAKE) -C tarfs/bin -f ../src/Makefile
	tar -cf $(BUILDDIR)/tarfs.tar tarfs/*
tarfs-clean:
	$(MAKE) clean -C tarfs/bin -f ../src/Makefile
	rm $(BUILDDIR)/tarfs.tar

tags: kernel tarfs
	ctags -R .

clean: kernel-clean tarfs-clean

emul: image
	@util/qemul.sh

image: image.img
image.img: kernel tarfs
	@util/makedisk.sh
