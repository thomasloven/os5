VPATH := ../src

TARGETS := $(shell find ../src -name "*.c")
TARGETS := $(notdir $(TARGETS))
TARGETS := $(patsubst %.c, %, $(TARGETS))

all: $(TARGETS)

clean:
	-rm $(TARGETS) 2>/dev/null
