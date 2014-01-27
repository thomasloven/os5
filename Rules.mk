.SUFFIXES:
.SUFFIXES: .c .S .o

### Put all intermediate files in the build/ directory
#
builddir := build

all: targets tags

### Modules
#
dir := toolchain
include $(dir)/Rules.mk
dir := kernel
include $(dir)/Rules.mk
dir := tarfs/src
include $(dir)/Rules.mk

CLEAN := $(CLEAN) image.img

### General rules
# Adapted for the build directory
#
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

### Build OS modules
#
.PHONY: targets
targets: $(TGT_TARFS) $(TGT_KERNEL)

### Generate disk image
#
.PHONY: image
image: image.img
image.img: $(TGT_KERNEL) $(TGT_TARFS)
	@util/makedisk.sh

### Run emulator
#
.PHONY: emul
emul: image.img
	@util/qemul.sh

### Clean up
#
.PHONY: clean
clean: 
	rm -f $(CLEAN)

.PHONY: tags
tags:
	ctags -R .

.SECONDARY: $(CLEAN)
