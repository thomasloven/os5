BUILDDIR := $(PWD)

PATH := /usr/local/cross/bin:$(PATH)
DIRS := kernel
TARGET := i386-elf

AS := nasm
CC := i386-elf-gcc
LD := i386-elf-ld

ASFLAGS := -f elf
CCFLAGS := -nostdlib -nostdinc -fno-builtin -fno-exceptions -m32
CCFLAGS += -fomit-frame-pointer -fno-asynchronous-unwind-tables 
CCFLAGS += -fno-unwind-tables -I$(BUILDDIR)/library/include
LDFLAGS := -T $(BUILDDIR)/library/include/Link.ld

export BUILDDIR AS CC LD ASFLAGS CCFLAGS LDFLAGS

.SILENT:

.PHONY: $(DIRS) floppy emul
.default: all floppy emul

nd: all floppy emul

d: all floppy demul

all: $(DIRS)
    
$(DIRS): force
	@echo "  MAKE    " $@
	@cd $@; $(MAKE) $(MFLAGS)

clean:
	@for DIR in $(DIRS); do echo "  CLEAN   " $$DIR; cd $(BUILDDIR)/$$DIR; make clean; done;

floppy: force
	@echo "  UPDATING IMAGE"
	@build/update_image.sh

emul: force
	@echo "  STARTING EMULATOR"
	@build/emul.sh

demul: force
	@echo "  STARTING EMULATOR (DEBUGGER)"
	@build/demul.sh

force:
	true
