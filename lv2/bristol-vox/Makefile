# Bristol Vox Continental LV2 — Linux / Raspberry Pi
#
#   sudo apt install build-essential pkg-config liblv2-dev
#   make && sudo make install
#
# Cross-compile for Raspberry Pi OS 64-bit (aarch64) from x86_64 Ubuntu/Debian:
#   sudo dpkg --add-architecture arm64
#   sudo apt install gcc-aarch64-linux-gnu liblv2-dev:arm64 qemu-user-static
#   make CROSS=aarch64-linux-gnu
#   make test CROSS=aarch64-linux-gnu   # runs via qemu-aarch64-static
#
# Windows native / cross-compile: see Makefile.win and README.md

include sources.mk

# --- optional cross toolchain (e.g. CROSS=aarch64-linux-gnu) ---------------
PKG_CONFIG ?= pkg-config
ifdef CROSS
  CC         := $(CROSS)-gcc
  PKG_CONFIG := $(CROSS)-pkg-config
  ifeq ($(CROSS),aarch64-linux-gnu)
    RUN_TEST := qemu-aarch64-static -L /usr/aarch64-linux-gnu
  endif
endif

CFLAGS += -O2 -ffast-math -fPIC $(WARN) $(INC) $(DEFS) $(FORCE_INCLUDES) \
	$(shell $(PKG_CONFIG) --cflags lv2 2>/dev/null)
PLUGIN_CFLAGS := -std=gnu99 -O2 -ffast-math -fPIC $(WARN) $(INC) $(DEFS) \
	$(FORCE_INCLUDES) $(shell $(PKG_CONFIG) --cflags lv2 2>/dev/null)
LDFLAGS += -shared -lm -lpthread $(shell $(PKG_CONFIG) --libs lv2 2>/dev/null)

# No x86 SSE when cross-compiling for ARM or on non-x86 hosts
ifndef CROSS
  UNAME_M := $(shell uname -m 2>/dev/null)
  ifneq (,$(filter x86_64 i686,$(UNAME_M)))
    CFLAGS += -msse -mfpmath=sse
  endif
endif

PLUGIN := bristol-vox.lv2/bristol-vox
INSTALL_DIR ?= $(HOME)/.lv2

.PHONY: all clean install test

all: $(PLUGIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(ROOT)/bristol/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(ROOT)/libbristol/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/bristol_vox_plugin.o: bristol_vox_plugin.c | $(BUILD)
	$(CC) $(PLUGIN_CFLAGS) -c -o $@ $<

$(BUILD)/run_tests.o: tests/run_tests.c | $(BUILD)
	$(CC) $(PLUGIN_CFLAGS) -Itests -c -o $@ $<

$(BUILD)/test_engine.o: tests/test_engine.c | $(BUILD)
	$(CC) $(PLUGIN_CFLAGS) -Itests -c -o $@ $<

$(BUILD)/test_plugin.o: tests/test_plugin.c | $(BUILD)
	$(CC) $(PLUGIN_CFLAGS) -Itests -c -o $@ $<

$(TEST_BIN): $(OBJ) $(TEST_OBJ)
	$(CC) -o $@ $(OBJ) $(TEST_OBJ) -lm -lpthread

test: $(TEST_BIN)
	$(RUN_TEST) ./$(TEST_BIN)

$(PLUGIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
	cp bristol-vox.lv2/manifest.linux.ttl bristol-vox.lv2/manifest.ttl

clean:
	rm -rf $(BUILD) bristol-vox.lv2/bristol-vox $(TEST_BIN)

install: $(PLUGIN)
	mkdir -p $(INSTALL_DIR)/bristol-vox.lv2
	cp bristol-vox.lv2/bristol-vox $(INSTALL_DIR)/bristol-vox.lv2/
	cp bristol-vox.lv2/manifest.linux.ttl $(INSTALL_DIR)/bristol-vox.lv2/manifest.ttl
	rm -f $(INSTALL_DIR)/bristol-vox.lv2/plugin.ttl $(INSTALL_DIR)/bristol-vox.lv2/bristol-vox.ttl
