sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

OBJS_$(d) := $(call findsource,$(d),c,5,$(builddir))
DEPS_$(d) := $(patsubst %,%.d,$(TGTS_$(d)))

terminal_$(d) := $(builddir)/tarfs/src/terminal
TAR_TGTS += $(terminal_$(d))

$(terminal_$(d)): $(OBJS_$(d))
	$(LINK)

d := $(dirstack_$(sp))
sp :=$(basename $(sp))
