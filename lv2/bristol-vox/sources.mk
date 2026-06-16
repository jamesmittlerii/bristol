ROOT := ../..

INC := -I. -I$(ROOT)/include/bristol -I$(ROOT)/include/slab -I$(ROOT)/bristol
DEFS := -DBRISTOL_LV2 -DBRISTOL_LV2_BUILD -DHAVE_CONFIG_H
WARN := -Wall -Wno-deprecated-non-prototype

BRISTOL_SRC := \
	$(ROOT)/bristol/activesense.c \
	$(ROOT)/bristol/audioEngine.c \
	$(ROOT)/bristol/audiothread.c \
	$(ROOT)/bristol/blo.c \
	$(ROOT)/bristol/bristolvox.c \
	$(ROOT)/bristol/dca.c \
	$(ROOT)/bristol/envelope.c \
	$(ROOT)/bristol/midihandlers.c \
	$(ROOT)/bristol/midinote.c \
	$(ROOT)/bristol/ringbuffer.c \
	$(ROOT)/bristol/soundManager.c \
	$(ROOT)/bristol/vibrachorus.c \
	$(ROOT)/bristol/vox.c \
	$(ROOT)/libbristol/bristolcdefs.c \
	$(ROOT)/libbristol/debugging.c \
	$(ROOT)/libbristol/mixroutines.c \
	$(ROOT)/libbristol/opmgt.c

LV2_SRC := \
	bristol_vox_plugin.c \
	bristol_vox_engine.c \
	bristol_lv2_stubs.c \
	palette_lv2.c

BUILD := build
OBJ := $(addprefix $(BUILD)/,$(notdir $(BRISTOL_SRC:.c=.o)) $(LV2_SRC:.c=.o))

TEST_SRC := \
	tests/run_tests.c \
	tests/test_engine.c \
	tests/test_plugin.c

TEST_OBJ := $(addprefix $(BUILD)/,$(notdir $(TEST_SRC:.c=.o)))
TEST_BIN := $(BUILD)/run_tests

FORCE_INCLUDES := -include $(CURDIR)/bristol_lv2_platform.h -include $(CURDIR)/config.h
