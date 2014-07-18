sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

TGT_LIB := $(TOOLCHAIN)/$(TARGET)/lib/libc.a $(TOOLCHAIN)/$(TARGET)/lib/libkernel.a
TGT_TOOLCHAIN := $(TOOLCHAIN)/bin/$(TARGET)-gcc $(TOOLCHAIN)/bin/$(TARGET)-ld $(TOOLCHAIN)/bin/i586-elf-gdb

OBJS_$(d) := $(shell find $(d)/myos -name "*")

$(TGT_LIB): $(OBJS_$(d))
	util/build_newlib -f

$(TGT_TOOLCHAIN):
	util/build_toolchain

d := $(dirstack_$(sp))
sp := $(basename $(sp))
