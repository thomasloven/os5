BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build
LIBDIR := $(BUILDROOT)/library

DIRS := kernel

AS := nasm
CPP := clang -E
CC := clang
LD := i386-elf-ld
DEP := clang -MM

ASFLAGS := -f elf

CPPFLAGS := -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CPPFLAGS += -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function
CPPFLAGS += -Wno-unused-parameter -g -Wno-gnu
CPPFLAGS += -I$(LIBDIR)/include

CCFLAGS := $(CPPFLAGS) -target i386-pc-linux -mno-sse -mno-mmx
CCFLAGS += -ggdb

LDFLAGS := -T $(LIBDIR)/include/Link.ld

DEPFLAGS := $(CPPFLAGS)

export BUILDROOT BUILDDIR LIBDIR
export AS CPP CC LD DEP 
export ASFLAGS CPPFLAGS CCFLAGS LDFLAGS DEPFLAGS

.SILENT:

.PHONY: $(DIRS) clean emul default
.DEFAULT: all emul

default: all emul

all: $(DIRS)
    
$(DIRS):
	@echo "  \033[35mMAKE\033[0m    " $@
	@cd $@; $(MAKE) $(MFLAGS)

clean:
	@for DIR in $(DIRS); do echo "  \033[35mCLEAN\033[0m   " $$DIR; cd $(BUILDROOT)/$$DIR; make clean; done;

emul:
	@echo "  \033[35mSTARTING EMULATOR\033[0m"
	@tools/qemul.sh

