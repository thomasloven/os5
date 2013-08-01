AS := nasm
CPP := i586-pc-myos-gcc -E
CC := i586-pc-myos-gcc
LD := i586-pc-myos-gcc

ASFLAGS := -f elf

CFLAGS += -ggdb

VPATH := ../src


TARGETS := $(shell find ../src -name "*.c")
TARGETS := $(notdir $(TARGETS))
TARGETS := $(patsubst %.c, %, $(TARGETS))

all: $(TARGETS)

clean:
	-rm $(TARGETS) 2>/dev/null
