SUBDIR := kernel

DIRS := $(shell find . -type d)
TARGET := $(BUILDDIR)/$(SUBDIR)/kernel

VPATH := ./ $(DIRS)

SOURCES := ./boot/boot.S ./boot/kinit.c 
SOURCES += $(shell find . -name "*.S" | grep -v boot.S)
SOURCES += $(shell find . -name "*.c" | grep -v kinit.c)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
OBJECTS := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(OBJECTS)))

DEPENDENCIES := $(patsubst %.S,%.d,$(SOURCES))
DEPENDENCIES := $(patsubst %.c,%.d,$(DEPENDENCIES))
DEPENDENCIES := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(DEPENDENCIES)))


CFLAGS += -Iinclude 
CPPFLAGS += -I$(LIBDIR)/include
CPPFLAGS += -Iinclude
DEPFLAGS += -I$(LIBDIR)/include
DEPFLAGS += -Iinclude
LDFLAGS := -T $(BUILDROOT)/$(SUBDIR)/include/Link.ld

all: $(DEPENDENCIES) $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "   \033[32mln\033[0m     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $+ $(BUILDDIR)/lib/i386-elf/lib/libc.a

$(BUILDDIR)/$(SUBDIR)/%.d: %.c
	@echo "    \033[31mdep\033[0m " $*".c"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"
$(BUILDDIR)/$(SUBDIR)/%.d: %.S
	@echo "    \033[31mdep\033[0m " $*".S"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"

$(BUILDDIR)/$(SUBDIR)/%.o: %.c
	@echo "  \033[33m gcc\033[0m    " $<
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.s
	@echo "   \033[33mnasm\033[0m   " $<
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/$(SUBDIR)/%.s: %.S
	@echo "   \033[31mgcc\033[0m     " $<
	@$(CPP) $(CPPFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-@rm $(OBJECTS) 2>/dev/null
	-@rm $(TARGET) 2>/dev/null
	-@rm $(DEPENDENCIES) 2>/dev/null

-include $(DEPENDENCIES)
