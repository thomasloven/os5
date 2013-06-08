SUBDIR := init

TARGET := $(BUILDDIR)/$(SUBDIR)/init

VPATH := ./

SOURCES := $(shell find . -name "*.S" | grep -v boot.S)
SOURCES += $(shell find . -name "*.c" | grep -v boot.S)

OBJECTS := $(patsubst %.S,%.o,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(OBJECTS))
OBJECTS := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(OBJECTS)))

DEPENDENCIES := $(patsubst %.S,%.d,$(SOURCES))
DEPENDENCIES := $(patsubst %.c,%.d,$(DEPENDENCIES))
DEPENDENCIES := $(addprefix $(BUILDDIR)/$(SUBDIR)/,$(notdir $(DEPENDENCIES)))

all: $(DEPENDENCIES) $(TARGET)

$(TARGET): $(OBJECTS) $(LIBPATH)
	@echo "   \033[32mln\033[0m     " $(TARGET)
	@$(LD) $(LDFLAGS) -o $(TARGET) $+ $(LIBPATH1) $(LIBPATH)

$(BUILDDIR)/$(SUBDIR)/%.d: %.c
	@echo "    \033[31mdep\033[0m " $*".c"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"
$(BUILDDIR)/$(SUBDIR)/%.d: %.S
	@echo "    \033[31mdep\033[0m " $*".S"
	@$(DEP) $(DEPFLAGS) $< -o $@ -MT "$(BUILDDIR)/$*.o $(BUILDDIR)/$*.d"

$(BUILDDIR)/$(SUBDIR)/%.o: %.c
	@echo "  \033[33m gcc\033[0m    " $<
	@$(CC) $(CCFLAGS) -c $< -o $@

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
