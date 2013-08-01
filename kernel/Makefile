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

CPPFLAGS := -Iinclude
LDFLAGS := -nostdlib -T $(BUILDROOT)/$(SUBDIR)/include/Link.ld

all: $(DEPENDENCIES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(TARGET) $+ -lkernel

$(BUILDDIR)/$(SUBDIR)/%.d: %.c
	$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"
$(BUILDDIR)/$(SUBDIR)/%.d: %.S
	$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"

$(BUILDDIR)/$(SUBDIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/$(SUBDIR)/%.s: %.S
	$(CPP) $(CPPFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-rm $(OBJECTS) 2>/dev/null
	-rm $(TARGET) 2>/dev/null
	-rm $(DEPENDENCIES) 2>/dev/null

-include $(DEPENDENCIES)
