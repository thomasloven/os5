sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

TGT_TARFS := $(builddir)/tarfs.tar

OBJS_$(d) := $(call findsource,$(d),c,$(builddir))
TGTS_$(d) := $(patsubst %.o,%,$(OBJS_$(d)))
DEPS_$(d) := $(patsubst %,%.d,$(TGTS_$(d)))

CLEAN := $(CLEAN) $(TGTS_$(d)) $(TGT_TARFS)
CLEAN += $(addprefix tarfs/bin/, $(notdir $(TGTS_$(d)))) $(DEPS_$(d))

$(TGT_TARFS): $(TGTS_$(d))
	@mkdir -p tarfs/bin
	@mkdir -p tarfs/dev
	@mkdir -p tarfs/mnt
	@mkdir -p tarfs/mnt/tarfs
	cp $^ tarfs/bin
	tar -cf build/tarfs.tar tarfs/*

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS_$(d))
endif
d := $(dirstack_$(sp))
sp := $(basename $(sp))
