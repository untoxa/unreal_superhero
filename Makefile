GBDK = ../../gbdk
GBDKLIB = $(GBDK)/lib/small/asxxxx
CC = $(GBDK)/bin/lcc
TOOLS = ./tools

ROM_BUILD_DIR = build
OBJDIR = obj

MUSIC_DRIVER = -Wl-klib -Wl-lhUGEDriver.lib
SYMBOLS = -Wl-j -Wm-yS -Wl-m -Wl-w

CFLAGS = -Isrc/include -I$(OBJDIR) -Wa-Isrc/include -Wa-I$(GBDKLIB)

#LFLAGS_NBANKS += -Wl-yt0x1A -Wl-yoA -Wl-ya4
LFLAGS_NBANKS =

LFLAGS = -Wm-yC $(LFLAGS_NBANKS) $(SYMBOLS) -Wm-yn"UNREAL" $(MUSIC_DRIVER)

TARGET = $(ROM_BUILD_DIR)/unreal.gb

ASRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.s))) 
CSRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.c))) 

OBJS = $(CSRC:%.c=$(OBJDIR)/%.o) $(ASRC:%.s=$(OBJDIR)/%.o)

all:	directories release $(TARGET)

.PHONY: clean release debug color profile directories

release:
	$(eval CFLAGS += -Wf'--max-allocs-per-node 200000')
	@echo "RELEASE mode ON"
	
debug:
	$(eval CFLAGS += -Wf--debug $(SYMBOLS) -Wl-y)
	$(eval CFLAGS += -Wf--nolospre -Wf--nogcse)
	$(eval LFLAGS += -Wf--debug -Wl-y)
	@echo "DEBUG mode ON"

color:
	$(eval CFLAGS += -DCGB)
	$(eval LFLAGS += -Wm-yC)
	@echo "COLOR mode ON"

profile:
	$(eval CFLAGS += -Wf--profile)
	@echo "PROFILE mode ON"

.SECONDARY: $(OBJS) 
	
directories: $(ROM_BUILD_DIR) $(OBJDIR)

$(ROM_BUILD_DIR):
	mkdir -p $(ROM_BUILD_DIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(ROM_BUILD_DIR)/%.gb:	$(OBJS)
	mkdir -p $(ROM_BUILD_DIR)
	$(CC) $(LFLAGS) -o $@ $^

clean:
	@echo "CLEANUP..."
	rm -rf $(OBJDIR)
	rm -rf $(ROM_BUILD_DIR)

rom: directories $(TARGET)
