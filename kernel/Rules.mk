sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

TGT_KERNEL := $(builddir)/kernel/kernel
versionfile := $(builddir)/kernel/boot/version.o
# Find all source files in the kernel subdirectories
# boot.S and kinit.o should be linked first, so they are added
# separately.
OBJS_$(d) := $(builddir)/kernel/boot/boot.o $(builddir)/kernel/boot/kinit.o
OBJS_$(d) += $(call findsource,$(d),S,$(builddir))
OBJS_$(d) += $(call findsource,$(d),c,$(builddir))
DEPS_$(d) := $(patsubst %.o,%.d,$(OBJS_$(d)))

CLEAN := $(CLEAN) $(OBJS_$(d)) $(TGT_KERNEL) $(DEPS_$(d))

$(OBJS_$(d)): CF_TGT := -I$(d)/include
$(DEPS_$(d)): CF_TGT := -I$(d)/include
$(TGT_KERNEL): LF_TGT := -nostdlib -T $(d)/include/Link.ld
$(TGT_KERNEL): LL_TGT := -lkernel

# Add git status to version.o
$(versionfile): CF_TGT += $(GITFLAGS)

# If anything was rebuilt in the kernel, rebuilt version.o again
$(TGT_KERNEL): $(OBJS_$(d))
	rm -f $(versionfile)
	$(MAKE) $(versionfile)
	$(LINK)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS_$(d))
endif
d := $(dirstack_$(sp))
sp := $(basename $(sp))
