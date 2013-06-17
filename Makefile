BUILDROOT := $(PWD)
BUILDDIR := $(BUILDROOT)/build
LIBDIR := $(BUILDROOT)/library

DIRS := library kernel init

AS := nasm
CPP := clang -E
CC := clang
LD := i386-elf-ld
DEP := clang -MM
AR := i386-elf-ar

ASFLAGS := -f elf

CPPFLAGS := -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CPPFLAGS += -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function
CPPFLAGS += -Wno-unused-parameter -g -Wno-gnu
CPPFLAGS += -I$(LIBDIR)/include
CPPFLAGS += -I$(BUILDDIR)/lib/i386-elf/include

CCFLAGS := $(CPPFLAGS) -target i386-pc-linux -mno-sse -mno-mmx
CCFLAGS += -ggdb

#LIBPATH1 := $(BUILDDIR)/library/clib.a
LIBPATH := $(BUILDDIR)/lib/i386-elf/lib/libc.a $(BUILDDIR)/library/clib.a

DEPFLAGS := $(CPPFLAGS)

ARFLAGS := -rc

export BUILDROOT BUILDDIR LIBDIR LIBPATH1 LIBPATH
export AS CPP CC LD DEP AR
export ASFLAGS CPPFLAGS CCFLAGS LDFLAGS DEPFLAGS ARFLAGS

#.SILENT:

.PHONY: $(DIRS) clean emul default
.DEFAULT: all emul

default: all

all: $(DIRS)
    
$(DIRS):
	@echo "  \033[35mMAKE\033[0m    " $@
	-@mkdir -p $(BUILDDIR)/$@
	@cd $@; $(MAKE) $(MFLAGS)

clean:
	@for DIR in $(DIRS); do echo "  \033[35mCLEAN\033[0m   " $$DIR; cd $(BUILDROOT)/$$DIR; make clean; done;

emul:
	@echo "  \033[35mSTARTING EMULATOR\033[0m"
	@tools/qemul.sh

