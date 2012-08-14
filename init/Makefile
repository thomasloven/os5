TARGET := init
SUBDIR := init

SOURCES := $(patsubst %.asm, %.o, $(shell find . -name "*.asm"))
SOURCES += $(patsubst %.c, %.o, $(shell find . -name "*.c"))

all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "   \033[32mln\033[0m     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $(SOURCES) $(LIBC)

%.o: %.c
	@echo "  \033[33m gcc\033[0m    " $<
	@$(CC) $(CCFLAGS) -c $< -o $@

%.o : %.s
	@echo "   \033[33mnasm\033[0m   " $<
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.asm
	@echo "   \033[31mgcc\033[0m     " $<
	@$(CPP) -E $(CPPFLAGS) -o $@ -x assembler-with-cpp $<

clean:
	-@rm $(SOURCES) 2>/dev/null
	-@rm $(TARGET) 2>/dev/null
