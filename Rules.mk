.SUFFIXES:
.SUFFIXES: .c .S .o

### Put all intermediate files in the build/ directory
#
builddir := build

all: targets

### Modules
#
dir := kernel
include $(dir)/Rules.mk
dir := tarfs/src
include $(dir)/Rules.mk

CLEAN := $(CLEAN) image.img

### General rules
# Adapted for the build directory
#
$(builddir)/%.o: %.[cS]
	@mkdir -p $(dir $@)
	$(COMP)
%(builddir)/%: %.o
	@mkdir -p $(dir $@)
	$(LINK)
%(builddir)/%: %.c
	@mkdir -p $(dir $@)
	$(COMPLINK)
$(builddir)/%.d: %.[cS]
	@mkdir -p $(dir $@)
	$(DEP)

### Build OS modules
#
.PHONY: targets
targets: $(TGT_KERNEL) $(TGT_TARFS)

### Generate disk image
#
.PHONY: image
image: image.img
image.img: $(kernel) $(tarfs)
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

.SECONDARY: $(CLEAN)
