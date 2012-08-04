TARGET := kernel
SUBDIR := kernel

SOURCES := boot/boot.o boot/kinit.o 
SOURCES += $(patsubst %.asm,%.o,$(shell find . -name "*.asm" | grep -v boot.asm))
SOURCES += $(patsubst %.c,%.o,$(shell find . -name "*.c" | grep -v kinit.c))

CCFLAGS += -Iinclude 
LDFLAGS := -T $(BUILDDIR)/$(SUBDIR)/include/Link.ld

all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "   ln     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $(SOURCES)

%.o: %.c
	@echo "   gcc    " $<
	@$(CC) $(CCFLAGS) -c $< -o $@

%.o : %.s
	@echo "   nasm   " $<
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.asm
	@echo "   gcc     " $<
	@$(CC) -E $(CCFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-@rm $(SOURCES) 2>/dev/null
	-@rm $(TARGET) 2>/dev/null
