SUBDIR := kernel
SRCDIR := $(BUILDROOT)/$(SUBDIR)

DIRS := $(shell find $(SRCDIR) -type d)
TARGET := $(BUILDDIR)/$(SUBDIR)/kernel

VPATH := $(DIRS)

SOURCES := ./boot/boot.S ./boot/kinit.c 
SOURCES += $(shell find $(SRCDIR) -name "*.S" | grep -v boot.S)
SOURCES += $(shell find $(SRCDIR) -name "*.c" | grep -v kinit.c)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
# OBJECTS := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(OBJECTS)))
OBJECTS :=$(notdir $(OBJECTS))

DEPENDENCIES := $(patsubst %.S,%.d,$(SOURCES))
DEPENDENCIES := $(patsubst %.c,%.d,$(DEPENDENCIES))
# DEPENDENCIES := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(DEPENDENCIES)))
DEPEDENCIES :=$(notdir $(DEPENDENCIES))

CPPFLAGS := -I$(SRCDIR)/include -ggdb
LDFLAGS := -nostdlib -T $(BUILDROOT)/$(SUBDIR)/include/Link.ld

AS = nasm


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(TARGET) $+ -lkernel

%.d: %.c
	$(DEP) $(DEPFLAGS) $< -o $(notdir $@) -MT "$(BUILDDIR)/$(notdir $*).o $(BUILDDIR)/$(notdir $*).d"
%.d: %.S
	$(DEP) $(DEPFLAGS) $< -o $(notdir $@) -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.s: %.S
	$(CPP) $(CPPFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-rm $(OBJECTS) 2>/dev/null
	-rm $(TARGET) 2>/dev/null
	-rm $(DEPENDENCIES) 2>/dev/null

# -include $(DEPENDENCIES)
