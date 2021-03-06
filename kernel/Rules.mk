sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

TGT_KERNEL := $(builddir)/kernel/kernel
# Find all source files in the kernel subdirectories
# boot.S and kinit.o should be linked first, so they are added
# separately.
OBJS_$(d) := $(builddir)/kernel/boot/boot.o
OBJS_$(d) += $(builddir)/kernel/boot/kinit.o
OBJS_$(d) += $(call findsource,$(d),S,5,$(builddir))
OBJS_$(d) += $(call findsource,$(d),c,5,$(builddir))
DEPS_$(d) := $(patsubst %.o,%.d,$(OBJS_$(d)))


CLEAN := $(CLEAN) $(OBJS_$(d)) $(TGT_KERNEL) $(DEPS_$(d))

$(OBJS_$(d)) $(DEPS_$(d)): CF_TGT := -I$(d)/include -DKERNEL_MODE
$(TGT_KERNEL): LF_TGT := -nostdlib -T $(d)/include/Link.ld
$(TGT_KERNEL): LL_TGT := -lkernel

# Add git status to version.o
versionfile := $(builddir)/kernel/boot/version.o
$(versionfile): CF_TGT += $(GITFLAGS)

# If anything was rebuilt in the kernel, rebuilt version.o again
$(TGT_KERNEL): $(OBJS_$(d))
	$(RM) -f $(versionfile)
	$(MAKE) $(versionfile)
	$(LINK)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS_$(d))
endif
d := $(dirstack_$(sp))
sp := $(basename $(sp))
