SUBDIR := kernel
SRCDIR := $(BUILDROOT)/$(SUBDIR)

DIRS := $(shell find $(SRCDIR) -type d)
TARGET := $(BUILDDIR)/$(SUBDIR)/kernel

VPATH := $(DIRS)

SOURCES := $(SRCDIR)/boot/boot.S $(SRCDIR)/boot/kinit.c 
SOURCES += $(shell find $(SRCDIR) -name "*.S" | grep -v boot.S)
SOURCES += $(shell find $(SRCDIR) -name "*.c" | grep -v kinit.c)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
OBJECTS :=$(notdir $(OBJECTS))

DEPENDENCIES := $(patsubst %.S,%.d,$(SOURCES))
DEPENDENCIES := $(patsubst %.c,%.d,$(DEPENDENCIES))
DEPENDENCIES := $(notdir $(DEPENDENCIES))

CPPFLAGS := -I$(SRCDIR)/include -ggdb
LDFLAGS := -nostdlib -T $(BUILDROOT)/$(SUBDIR)/include/Link.ld
LDLIBS := -lkernel


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
%.d: %.c
	$(DEP) $(DEPFLAGS) $< -o $(notdir $@) -MT "$(BUILDDIR)/$(notdir $*).o $(BUILDDIR)/$(notdir $*).d"
%.d: %.S
	$(DEP) $(DEPFLAGS) $< -o $(notdir $@) -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"


clean:
	-rm $(OBJECTS)
	-rm $(TARGET)
	-rm $(DEPENDENCIES)

-include $(DEPENDENCIES)
