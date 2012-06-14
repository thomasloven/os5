TARGET := kernel
SUBDIR := kernel

SOURCES := kinit.o boot.o
SOURCES += $(patsubst %.s,%.o,$(shell find . -name "*.s" | grep -v boot.s))
SOURCES += $(patsubst %.c,%.0,$(shell find . -name "*.c" | grep -v kinit.c))

CCFLAGS += -Iinclude
LDFLAGS := -T $(BUILDDIR)/$(SUBDIR)/include/Link.ld

.SUFFICES: .o .s .c

all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "   ln     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $(SOURCES)

.c.o:
	@echo "   gcc    " $<
	@$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	@echo "   nasm   " $<
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	-@rm $(SOURCES) 2>/dev/null
	-@rm $(TARGET) 2>/dev/null
