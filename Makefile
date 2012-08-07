BUILDDIR := $(PWD)

PATH := /usr/local/cross/bin:$(PATH)
DIRS := kernel
TARGET := i386-elf

AS := nasm
#CC := i386-elf-gcc
#CPP := i386-elf-gcc -E
LD := i386-elf-ld

ASFLAGS := -f elf
CCFLAGS := -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CCFLAGS += -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function
CCFLAGS += -Wno-unused-parameter -g -Wno-gnu
CCFLAGS += -I$(BUILDDIR)/library/include
CPPFLAGS := $(CCFLAGS)
CCFLAGS += -ccc-host-triple i386-pc-linux-gnu
LDFLAGS := -T $(BUILDDIR)/library/include/Link.ld

export BUILDDIR AS CC CPP LD ASFLAGS CCFLAGS CPPFLAGS LDFLAGS

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
