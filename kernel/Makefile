SUBDIR := kernel
SRCDIR := $(BUILDROOT)/$(SUBDIR)
TARGET := $(BUILDDIR)/$(SUBDIR)/kernel

VPATH := $(shell find $(SRCDIR) -type d)

SOURCES := $(SRCDIR)/boot/boot.S $(SRCDIR)/boot/kinit.c 
SOURCES += $(shell find $(SRCDIR) -name "*.S" | grep -v boot.S)
SOURCES += $(shell find $(SRCDIR) -name "*.c" | grep -v kinit.c)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
OBJECTS := $(notdir $(OBJECTS))

DEPENDENCIES := $(patsubst %.o,%.d,$(OBJECTS))


CPPFLAGS += -I$(SRCDIR)/include
LDFLAGS := -nostdlib -T $(SRCDIR)/include/Link.ld
LDLIBS := -lkernel

GITHASH := $(shell git log -1 --pretty="tformat:%h")
GITDATE := $(shell git log -1 --pretty="tformat:%ai")
GITDIRTY := $(shell [[ -n `git status -s 2> /dev/null` ]] && echo yes)

CPPFLAGS += -DGITHASH='"$(GITHASH)"' -DGITDATE='"$(GITDATE)"' -DGITDIRTY='"$(GITDIRTY)"'

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@


%.d: %.c
	$(DEP) $(CPPFLAGS) $< -o $(notdir $@) -MT "$(notdir $*).o $(notdir $*).d"
%.d: %.S
	$(DEP) $(CPPFLAGS) $< -o $(notdir $@) -MT "$(notdir $*).o $(notdir $*).d"


clean:
	-rm $(OBJECTS) 2>/dev/null
	-rm $(TARGET) 2>/dev/null
clean-all:
	-rm $(OBJECTS) 2>/dev/null
	-rm $(TARGET) 2>/dev/null
	-rm $(DEPENDENCIES) 2>/dev/null

-include $(DEPENDENCIES)
