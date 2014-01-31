sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

PREFIX=/usr/local/Cellar/osdev/1.0
TARGET=i586-pc-myos
export PREFIX TARGET

TGT_LIB := $(PREFIX)/$(TARGET)/lib/libc.a
TGT_TOOLCHAIN := /usr/local/bin/$(TARGET)-gcc

OBJS_$(d) := $(shell find $(d)/myos -name "*")

$(TGT_LIB): $(OBJS_$(d))
	toolchain/rebuild_newlib.sh force

$(TGT_TOOLCHAIN):
	toolchain/rebuild_toolchain.sh

d := $(dirstack_$(sp))
sp := $(basename $(sp))
