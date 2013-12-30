sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

TGT_LIB := /usr/local/Cellar/osdev/1.0/i586-pc-myos/lib/libc.a
TGT_TOOLCHAIN := /usr/local/bin/i586-pc-myos-gcc

OBJS_$(d) := $(shell find $(d)/myos -name "*")

$(TGT_LIB): $(OBJS_$(d))
	toolchain/rebuild_newlib.sh force

$(TGT_TOOLCHAIN):
	toolchain/rebuild_toolchain.sh

d := $(dirstack_$(sp))
sp := $(basename $(sp))
