SUBDIR := kernel

DIRS := $(shell find . -type d)
TARGET := $(BUILDDIR)/kernel

VPATH := ./ $(DIRS)

SOURCES := ./boot/boot.S ./boot/kinit.c 
SOURCES += $(shell find . -name "*.S" | grep -v boot.S)
SOURCES += $(shell find . -name "*.c" | grep -v kinit.c)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(OBJECTS)))

DEPENDENCIES := $(patsubst %.S,%.d,$(SOURCES))
DEPENDENCIES := $(patsubst %.c,%.d,$(DEPENDENCIES))
DEPENDENCIES := $(addprefix $(BUILDDIR)/,$(notdir $(DEPENDENCIES)))


CCFLAGS += -Iinclude 
CPPFLAGS += -Iinclude
DEPFLAGS += -Iinclude
LDFLAGS := -T $(BUILDROOT)/$(SUBDIR)/include/Link.ld


all: $(DEPENDENCIES) $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "   \033[32mln\033[0m     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $+

$(BUILDDIR)/%.d: %.c
	@echo "    \033[31mdep\033[0m " $*".c"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"
$(BUILDDIR)/%.d: %.S
	@echo "    \033[31mdep\033[0m " $*".S"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"

$(BUILDDIR)/%.o: %.c
	@echo "  \033[33m gcc\033[0m    " $<
	@$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.s
	@echo "   \033[33mnasm\033[0m   " $<
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/%.s: %.S
	@echo "   \033[31mgcc\033[0m     " $<
	@$(CPP) $(CPPFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-@rm $(OBJECTS) 2>/dev/null
	-@rm $(TARGET) 2>/dev/null
	-@rm $(DEPENDENCIES) 2>/dev/null

-include $(DEPENDENCIES)
