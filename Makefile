SHELL := /bin/bash

# If you move this project you can change the directory
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"

ifndef GBDK_HOME
GBDK_HOME = ../../gbdk-2020/build/gbdk/
endif

GBCPU = sm83

LCC = $(GBDK_HOME)bin/lcc
PNG2ASSET = $(GBDK_HOME)bin/png2asset
PNG2HICOLORGB = $(GBDK_HOME)bin/png2hicolorgb

# Set platforms to build here, spaced separated. (These are in the separate Makefile.targets)
# They can also be built/cleaned individually: "make gg" and "make gg-clean"
# Possible are: gb gbc pocket sms gg
#TARGETS = gb gbc pocket sms gg
#TARGETS = gb gbc
TARGETS = gbc

LIBRARIES = -Wl-llib/$(PORT)/hUGEDriver.lib

# Configure platform specific LCC flags here:
LCCFLAGS_gb      = $(LIBRARIES) -Wm-ys -Wl-yt0x1A -Wm-yn"$(PROJECTNAME)"
LCCFLAGS_pocket  = $(LIBRARIES) -Wm-ys -Wl-yt0x1A -Wm-yn"$(PROJECTNAME)"
LCCFLAGS_gbc     = $(LIBRARIES) -Wm-ys -Wm-yc -Wl-yt0x1A -Wm-yn"$(PROJECTNAME)"
LCCFLAGS_sms     =
LCCFLAGS_gg      =

LCCFLAGS += $(LCCFLAGS_$(EXT)) -Wm-yS # This adds the current platform specific LCC Flags

LCCFLAGS += -Wl-j -Wm-yoA -Wm-ya16 -autobank -Wb-ext=.rel
# LCCFLAGS += -debug # Uncomment to enable debug output
# LCCFLAGS += -v     # Uncomment for lcc verbose output

CFLAGS = -Iinclude -Iinclude/$(PORT) -Iinclude/$(PLAT) -I$(RESDIR) -Iobj/$(EXT)

# Optimization
CFLAGS      += -Wf'--max-allocs-per-node 200000'

# You can set the name of the ROM file here
PROJECTNAME = UNREAL

# EXT?=gb # Only sets extension to default (game boy .gb) if not populated
SRCDIR      = src
SRCPORT     = src/$(PORT)
SRCPLAT     = src/$(PLAT)
OBJDIR      = obj/$(EXT)
RESDIR      = res
BINDIR      = build/$(EXT)
MKDIRS      = $(OBJDIR) $(BINDIR) # See bottom of Makefile for directory auto-creation

# list of files that are always built
ALWAYS      =

# binaries list
BINS	    = $(OBJDIR)/$(PROJECTNAME).$(EXT)

# resources
VGM_RES	    = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/audio/$(PLAT)/sounds/*.vgm)))
FX_RES	    = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/audio/$(PLAT)/sounds/*.sav)))
UGE_RES     = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/audio/$(PLAT)/music/*.uge)))
WAV_RES     = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/audio/$(PLAT)/waveforms/*.wav)))

FONT_RES    = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/gfx/$(PLAT)/fonts/*.png)))
SPR_RES     = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/gfx/$(PLAT)/sprites/*.png)))
BKG_RES     = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/gfx/$(PLAT)/backgrounds/*.png)))
HICOLOR_RES = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/gfx/$(PLAT)/hicolor/*.png)))

DATA_RES    = $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/data/*.bin)))

# C and ASM sources
CSOURCES    = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(SRCPLAT),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(SRCPORT),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/audio/$(PLAT)/*.c))) $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/menus/*.c))) $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/overlays/*.c)))
ASMSOURCES  = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.s))) $(foreach dir,$(SRCPLAT),$(notdir $(wildcard $(dir)/*.s))) $(foreach dir,$(SRCPORT),$(notdir $(wildcard $(dir)/*.s)))

OBJS        = $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.s=$(OBJDIR)/%.o)
RESOBJ      = $(VGM_RES:%.vgm=$(OBJDIR)/%.o) $(WAV_RES:%.wav=$(OBJDIR)/%.o) $(FX_RES:%.sav=$(OBJDIR)/%.o) $(UGE_RES:%.uge=$(OBJDIR)/%.o) $(FONT_RES:%.png=$(OBJDIR)/%.o) $(SPR_RES:%.png=$(OBJDIR)/%.o) $(BKG_RES:%.png=$(OBJDIR)/%.o) $(DATA_RES:%.bin=$(OBJDIR)/%.o) $(HICOLOR_RES:%.png=$(OBJDIR)/%.o)

DEPENDANT   = $(CSOURCES:%.c=$(OBJDIR)/%.o)

# Builds all targets sequentially
all: $(TARGETS)

# Dependencies
DEPS = $(DEPENDANT:%.o=%.d)

-include $(DEPS)

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/audio/$(PLAT)/sounds/%.vgm $$(wildcard $(RESDIR)/audio/$(PLAT)/sounds/%.vgm.meta)
	python utils/vgm2data.py -5 -w -3 -d 4 `cat <$<.meta 2>/dev/null` -o $@ $<

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/audio/$(PLAT)/sounds/%.sav $$(wildcard $(RESDIR)/audio/$(PLAT)/sounds/%.sav.meta)
	python utils/fxhammer2data.py -d 4 -c `cat <$<.meta 2>/dev/null` -o $@ $<

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/audio/$(PLAT)/music/%.uge $$(wildcard $(RESDIR)/audio/$(PLAT)/music/%.uge.meta)
	utils/uge2source $< `cat <$<.meta 2>/dev/null` $(basename $(notdir $<)) $@

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/audio/$(PLAT)/waveforms/%.wav $$(wildcard $(RESDIR)/audio/$(PLAT)/waveforms/%.wav.meta)
	python utils/wav2data.py `cat <$<.meta 2>/dev/null` -o $@ $<


.SECONDEXPANSION:
$(OBJDIR)/%.o:	$(RESDIR)/data/%.bin $$(wildcard $(RESDIR)/data/%.bin.meta)
	python utils/bin2obj.py `cat <$<.meta 2>/dev/null` -o $@ $<


$(OBJDIR)/%.o:	$(RESDIR)/audio/$(PLAT)/%.c
	$(LCC) $(CFLAGS) -c -o $@ $<


.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/gfx/$(PLAT)/fonts/%.png $$(wildcard $(RESDIR)/gfx/$(PLAT)/fonts/%.png.meta)
	python utils/png2font.py `cat <$<.meta 2>/dev/null` -o $@ $<

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/gfx/$(PLAT)/sprites/%.png $$(wildcard $(RESDIR)/gfx/$(PLAT)/sprites/%.png.meta)
	$(PNG2ASSET) $< -c $@ `cat <$<.meta 2>/dev/null`

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/gfx/$(PLAT)/backgrounds/%.png $$(wildcard $(RESDIR)/gfx/$(PLAT)/backgrounds/%.png.meta)
	$(PNG2ASSET) $< -c $@ -map `cat <$<.meta 2>/dev/null`

.SECONDEXPANSION:
$(OBJDIR)/%.c:	$(RESDIR)/gfx/$(PLAT)/hicolor/%.png $$(wildcard $(RESDIR)/gfx/$(PLAT)/hicolor/%.png.meta)
	$(PNG2HICOLORGB) $< -o $@ `cat <$<.meta 2>/dev/null`


#always rebuild
.PHONY: $(ALWAYS)

# rule for the autogenerated files
$(OBJDIR)/%.o:	$(OBJDIR)/%.c
	$(LCC) $(CFLAGS) -c -o $@ $<

# Compile .c files in "res/" to .o object files
$(OBJDIR)/%.o:	$(RESDIR)/%.c
	$(LCC) -Wf-MMD $(CFLAGS) -c -o $@ $<


# Compile .c files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(LCC) -Wf-MMD $(CFLAGS) $(filter -Wf-ba%, $(subst .d,-Wf-ba,$(suffix $(<:%.c=%)))) -c -o $@ $<

# Compile .c files in "src/menus/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/menus/%.c
	$(LCC) -Wf-MMD $(CFLAGS) $(filter -Wf-ba%, $(subst .d,-Wf-ba,$(suffix $(<:%.c=%)))) -c -o $@ $<

# Compile .c files in "src/overlays/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/overlays/%.c
	$(LCC) -Wf-MMD $(CFLAGS) $(filter -Wf-ba%, $(subst .d,-Wf-ba,$(suffix $(<:%.c=%)))) -c -o $@ $<

# Compile .s assembly files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	$(LCC) $(CFLAGS) -c -o $@ $<


# Compile .c files in "src/<platform>/" to .o object files
$(OBJDIR)/%.o:	$(SRCPLAT)/%.c
	$(LCC) -Wf-MMD $(CFLAGS) -c -o $@ $<

# Compile .s assembly files in "src/<platform>/" to .o object files
$(OBJDIR)/%.o:	$(SRCPLAT)/%.s
	$(LCC) $(CFLAGS) -c -o $@ $<


# Compile .c files in "src/<target>/" to .o object files
$(OBJDIR)/%.o:	$(SRCPORT)/%.c
	$(LCC) -Wf-MMD $(CFLAGS) -c -o $@ $<

# Compile .s assembly files in "src/<target>/" to .o object files
$(OBJDIR)/%.o:	$(SRCPORT)/%.s
	$(LCC) $(CFLAGS) -c -o $@ $<


# Link the compiled object files into a .gb ROM file
$(BINS):	$(RESOBJ) $(OBJS)
	$(LCC) $(LCCFLAGS) $(CFLAGS) -o $(BINDIR)/$(PROJECTNAME).$(EXT) $^

clean:
	@echo Cleaning
	@for target in $(TARGETS); do \
		$(MAKE) $$target-clean; \
	done


# Include available build targets
include Makefile.targets


# create necessary directories after Makefile is parsed but before build
# info prevents the command from being pasted into the makefile
ifneq ($(strip $(EXT)),)           # Only make the directories if EXT has been set by a target
$(info $(shell mkdir -p $(MKDIRS)))
endif
