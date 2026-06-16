/*
 * Unit tests for bristol_vox_engine_* (direct API, no LV2 host).
 */
#include "test_common.h"
#include "bristol_lv2.h"
#include "bristol.h"
#include "ringbuffer.h"

#include <stdlib.h>

#define TEST_SR     44100u
#define TEST_BLOCK  128u

static audioMain *
engine_alloc(void)
{
	return (audioMain *) calloc(1, sizeof(audioMain));
}

static void
engine_free(audioMain *am)
{
	free(am);
}

static void
engine_init(audioMain *am, uint32_t block)
{
	ASSERT(bristol_vox_engine_init(am, TEST_SR, block) == 0);
}

TEST(test_engine_smoke)
{
	ASSERT(1);
}

TEST(test_engine_ringbuffer_only)
{
	audioMain *am = engine_alloc();

	ASSERT(am != NULL);
	am->samplecount = (int) TEST_BLOCK;
	am->segmentsize = (int) TEST_BLOCK * (int) sizeof(float);
	am->rb = jack_ringbuffer_create(8192);
	ASSERT(am->rb != NULL);
	jack_ringbuffer_free(am->rb);
	am->rb = NULL;
	engine_free(am);
}

TEST(test_engine_init_ringbuffer)
{
	audioMain *am = engine_alloc();

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	ASSERT(am->rb != NULL);
	bristol_vox_engine_fini(am);
	ASSERT(am->rb == NULL);
	engine_free(am);
}

TEST(test_engine_init_audiolist)
{
	audioMain *am = engine_alloc();

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	ASSERT(am->audiolist != NULL);
	ASSERT(am->samplecount == (int) TEST_BLOCK);
	ASSERT(am->samplerate == (int) TEST_SR);
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_run_silent_without_midi)
{
	audioMain *am = engine_alloc();
	float out_l[TEST_BLOCK];
	float out_r[TEST_BLOCK];
	int i;

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	for (i = 0; i < 8; i++)
		bristol_vox_engine_run(am, out_l, out_r, TEST_BLOCK);
	ASSERT(test_buffer_peak(out_l, TEST_BLOCK) < 1e-6f);
	ASSERT(test_buffer_peak(out_r, TEST_BLOCK) < 1e-6f);
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_run_with_midi_no_crash)
{
	audioMain *am = engine_alloc();
	float out_l[TEST_BLOCK];
	float out_r[TEST_BLOCK];
	int i;
	const uint8_t note_on[] = { 0x90, 60, 100 };

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	bristol_vox_engine_midi(am, note_on, sizeof(note_on));
	for (i = 0; i < 32; i++)
		bristol_vox_engine_run(am, out_l, out_r, TEST_BLOCK);
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_midi_note_on_off)
{
	audioMain *am = engine_alloc();
	const uint8_t note_on[] = { 0x90, 64, 80 };
	const uint8_t note_off[] = { 0x80, 64, 0 };

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	bristol_vox_engine_midi(am, note_on, sizeof(note_on));
	bristol_vox_engine_midi(am, note_off, sizeof(note_off));
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_controls)
{
	audioMain *am = engine_alloc();

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	bristol_vox_engine_set_drawbar(am, 0, 1.0f);
	bristol_vox_engine_set_drawbar(am, 5, 0.5f);
	bristol_vox_engine_set_volume(am, 0.75f);
	bristol_vox_engine_set_vibrato(am, 1);
	bristol_vox_engine_set_vibrato(am, 0);
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_block_size_mismatch_no_crash)
{
	audioMain *am = engine_alloc();
	float out_l[256];
	float out_r[256];

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	bristol_vox_engine_run(am, out_l, out_r, 256);
	bristol_vox_engine_fini(am);
	engine_free(am);
}

TEST(test_engine_reinit_block_sizes)
{
	audioMain *am = engine_alloc();
	float out_l[512];
	float out_r[512];
	uint32_t sizes[] = { 64, 128, 256, 512 };
	unsigned i;

	ASSERT(am != NULL);
	for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		engine_init(am, sizes[i]);
		ASSERT(am->rb != NULL);
		ASSERT(am->samplecount == (int) sizes[i]);
		bristol_vox_engine_run(am, out_l, out_r, sizes[i]);
		bristol_vox_engine_fini(am);
	}
	engine_free(am);
}

TEST(test_engine_midi_edge_cases)
{
	audioMain *am = engine_alloc();
	const uint8_t short_msg[] = { 0x90 };
	const uint8_t vel0[] = { 0x90, 60, 0 };

	ASSERT(am != NULL);
	engine_init(am, TEST_BLOCK);
	bristol_vox_engine_midi(am, short_msg, sizeof(short_msg));
	bristol_vox_engine_midi(am, vel0, sizeof(vel0));
	bristol_vox_engine_fini(am);
	engine_free(am);
}

void
test_engine_suite(void)
{
	test_suite_begin("engine");
	RUN_TEST(test_engine_smoke);
	RUN_TEST(test_engine_ringbuffer_only);
	RUN_TEST(test_engine_init_ringbuffer);
	RUN_TEST(test_engine_init_audiolist);
	RUN_TEST(test_engine_run_silent_without_midi);
	RUN_TEST(test_engine_run_with_midi_no_crash);
	RUN_TEST(test_engine_midi_note_on_off);
	RUN_TEST(test_engine_controls);
	RUN_TEST(test_engine_block_size_mismatch_no_crash);
	RUN_TEST(test_engine_reinit_block_sizes);
	RUN_TEST(test_engine_midi_edge_cases);
}
