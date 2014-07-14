sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

PREFIX=~/osdev/root
TARGET=i586-pc-myos
export PREFIX TARGET

TGT_LIB := $(PREFIX)/$(TARGET)/lib/libc.a
TGT_TOOLCHAIN := $(PREFIX)/bin/$(TARGET)-gcc

OBJS_$(d) := $(shell find $(d)/myos -name "*")

$(TGT_LIB): $(OBJS_$(d))
	echo $(TGT_LIB)
	util/build_newlib -f

$(TGT_TOOLCHAIN):
	util/build_toolchain

d := $(dirstack_$(sp))
sp := $(basename $(sp))
