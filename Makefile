BUILDDIR := $(PWD)
LIBDIR := $(BUILDDIR)/library

PATH := /usr/local/cross/bin:$(PATH)
DIRS := library kernel init
TARGET := i386-elf

AS := nasm
LD := i386-elf-ld
AR := i386-elf-ar

ASFLAGS := -f elf

CCFLAGS := -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CCFLAGS += -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function
CCFLAGS += -Wno-unused-parameter -g -Wno-gnu
CCFLAGS += -I$(BUILDDIR)/library/include

CPPFLAGS := $(CCFLAGS)

CCFLAGS += -ccc-host-triple i386-pc-linux-gnu -mno-sse -mno-mmx


ARFLAGS := -rc

LIBC := $(LIBDIR)/libc.a

export BUILDDIR LIBDIR AS LD AR ASFLAGS CCFLAGS CPPFLAGS LDFLAGS ARFLAGS LIBC

.SILENT:

.PHONY: $(DIRS) floppy emul
.default: all floppy emul

nd: all floppy emul

d: all floppy demul

all: $(DIRS)
    
$(DIRS): force
	@echo "  \033[35mMAKE\033[0m    " $@
	@cd $@; $(MAKE) $(MFLAGS)

clean:
	@for DIR in $(DIRS); do echo "  \033[35mCLEAN\033[0m   " $$DIR; cd $(BUILDDIR)/$$DIR; make clean; done;

floppy: force
	@echo "  \033[35mUPDATING IMAGE\033[0m"
	@build/update_image.sh

emul: force
	@echo "  \033[35mSTARTING EMULATOR\033[0m"
	@build/emul.sh

demul: force
	@echo "  \033[35mSTARTING EMULATOR (\033[33mDEBUGGER\033[33m)\033[0m"
	@build/demul.sh

force:
	true
