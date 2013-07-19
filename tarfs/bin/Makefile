AS := nasm
CPP := clang -E
CC := clang
LD := i386-elf-ld

ASFLAGS := -f elf

CPPFLAGS := -B /usr/local/Cellar/binutils/2.23.2/i386-elf/bin
CPPFLAGS += -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions
CPPFLAGS += -fno-stack-protector -ffreestanding -Wno-unused-function
CPPFLAGS += -Wno-unused-parameter -g -Wno-gnu
CPPFLAGS += -I$(BUILDDIR)/lib/i386-elf/include
CPPFLAGS += -I$(LIBDIR)/include
CPPFLAGS += -nostdlib

CFLAGS := -target i386-pc-linux -mno-sse -mno-mmx
CFLAGS += -ggdb

LDFLAGS := -B/usr/local/Cellar/binutils/2.23.2/i386-elf -target i386-pc-linux
LDLIBS := $(LIBPATH) $(LIBPATH)

VPATH := ../src


TARGETS := $(shell find ../src -name "*.c")
TARGETS := $(notdir $(TARGETS))
TARGETS := $(patsubst %.c, %, $(TARGETS))

all: $(TARGETS)

clean:
	-rm $(TARGETS) 2>/dev/null
