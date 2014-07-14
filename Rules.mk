.SUFFIXES:
.SUFFIXES: .c .S .o


all: kernel tarfs tags

### Modules
dir := toolchain
include $(dir)/Rules.mk
dir := kernel
include $(dir)/Rules.mk
dir := tarfs/src
include $(dir)/Rules.mk

imagefile := image.img
CLEAN := $(CLEAN) $(imagefile)
.SECONDARY: $(CLEAN)

### General targets
.PHONY: kernel tarfs image
kernel: $(TGT_KERNEL)
tarfs: $(TGT_TARFS)
image: $(imagefile)

image.img: $(TGT_KERNEL) $(TGT_TARFS)
	@util/makedisk

### Special targets
.PHONY: emul clean tags
emul: $(imagefile)
	@util/qemul
clean: 
	rm -f $(CLEAN)
tags:
	ctags -R .

### Build rules
$(builddir)/%.o: %.[cS] $(TGT_TOOLCHAIN) $(TGT_LIB)
	@mkdir -p $(dir $@)
	$(COMP)
%(builddir)/%: %.o $(TGT_TOOLCHAIN) $(TGT_LIB)
	@mkdir -p $(dir $@)
	$(LINK)
%(builddir)/%: %.c $(TGT_TOOLCHAIN) $(TGT_LIB)
	@mkdir -p $(dir $@)
	$(COMPLINK)
$(builddir)/%.d: %.[cS] $(TGT_TOOLCHAIN) $(TGT_LIB)
	@mkdir -p $(dir $@)
	$(DEP)
