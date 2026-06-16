/*
 * LV2 plugin lifecycle tests via lv2_descriptor().
 */
#include "test_common.h"

#include <stdlib.h>
#include <string.h>

#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/midi/midi.h>

#include "bristol_lv2.h"

extern const LV2_Descriptor *lv2_descriptor(uint32_t index);

enum {
	PORT_AUDIO_L = 0,
	PORT_AUDIO_R,
	PORT_MIDI_IN,
	PORT_GAIN,
	PORT_DRAWBAR_16,
	PORT_DRAWBAR_8,
	PORT_DRAWBAR_4,
	PORT_DRAWBAR_IV,
	PORT_DRAWBAR_FLUTE,
	PORT_DRAWBAR_REED,
	PORT_VIBRATO,
	PORT_COUNT
};

#define PLUGIN_SR 44100.0

static const LV2_Descriptor *
get_descriptor(void)
{
	return lv2_descriptor(0);
}

#define TEST_URID_SEQUENCE 1u
#define TEST_URID_MIDI     2u

static void
build_note_on_sequence(uint8_t *buf, size_t bufsize, uint8_t note, uint8_t vel,
	LV2_Atom_Sequence **out_seq)
{
	LV2_Atom_Sequence *seq = (LV2_Atom_Sequence *) buf;
	uint8_t event_storage[sizeof(LV2_Atom_Event) + 8];
	LV2_Atom_Event *ev = (LV2_Atom_Event *) event_storage;
	uint8_t *midi;

	seq->atom.size = sizeof(LV2_Atom_Sequence_Body);
	seq->atom.type = TEST_URID_SEQUENCE;
	seq->body.unit = 0;
	seq->body.pad = 0;

	ev->time.frames = 0;
	ev->body.type = TEST_URID_MIDI;
	ev->body.size = 3;
	midi = (uint8_t *) (ev + 1);
	midi[0] = 0x90;
	midi[1] = note;
	midi[2] = vel;

	ASSERT(lv2_atom_sequence_append_event(seq, (uint32_t) bufsize, ev) != NULL);
	*out_seq = seq;
}

TEST(test_descriptor)
{
	const LV2_Descriptor *desc = get_descriptor();

	ASSERT(lv2_descriptor(1) == NULL);
	ASSERT(desc != NULL);
	ASSERT(strcmp(desc->URI, BRISTOL_LV2_URI) == 0);
	ASSERT(desc->instantiate != NULL);
	ASSERT(desc->connect_port != NULL);
	ASSERT(desc->run != NULL);
	ASSERT(desc->cleanup != NULL);
}

TEST(test_instantiate_cleanup)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	if (desc->activate != NULL)
		desc->activate(handle);
	if (desc->deactivate != NULL)
		desc->deactivate(handle);
	desc->cleanup(handle);
}

TEST(test_run_without_ports)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	uint32_t block = 128;
	unsigned i;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	for (i = 0; i < 4; i++)
		desc->run(handle, block);
	desc->cleanup(handle);
}

TEST(test_run_with_audio_buffers)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	float out_l[128];
	float out_r[128];
	uint32_t block = 128;
	unsigned i;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->connect_port(handle, PORT_AUDIO_L, out_l);
	desc->connect_port(handle, PORT_AUDIO_R, out_r);
	for (i = 0; i < 8; i++) {
		desc->run(handle, block);
	}
	desc->cleanup(handle);
}

TEST(test_run_with_midi_sequence)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	float out_l[128];
	float out_r[128];
	uint8_t midi_buf[256];
	LV2_Atom_Sequence *midi_seq;
	uint32_t block = 128;
	unsigned i;

	build_note_on_sequence(midi_buf, sizeof(midi_buf), 60, 100, &midi_seq);

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->connect_port(handle, PORT_AUDIO_L, out_l);
	desc->connect_port(handle, PORT_AUDIO_R, out_r);
	desc->connect_port(handle, PORT_MIDI_IN, midi_seq);
	for (i = 0; i < 32; i++)
		desc->run(handle, block);
	desc->cleanup(handle);
}

TEST(test_run_block_size_change)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	float out_l[512];
	float out_r[512];
	uint32_t blocks[] = { 64, 128, 256, 512 };
	unsigned i;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->connect_port(handle, PORT_AUDIO_L, out_l);
	desc->connect_port(handle, PORT_AUDIO_R, out_r);
	for (i = 0; i < sizeof(blocks) / sizeof(blocks[0]); i++)
		desc->run(handle, blocks[i]);
	desc->cleanup(handle);
}

TEST(test_run_control_ports)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	float out_l[128];
	float out_r[128];
	float gain = 0.8f;
	float drawbars[6] = { 1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.0f };
	float vibrato = 1.0f;
	uint32_t block = 128;
	unsigned i;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->connect_port(handle, PORT_AUDIO_L, out_l);
	desc->connect_port(handle, PORT_AUDIO_R, out_r);
	desc->connect_port(handle, PORT_GAIN, &gain);
	desc->connect_port(handle, PORT_DRAWBAR_16, &drawbars[0]);
	desc->connect_port(handle, PORT_DRAWBAR_8, &drawbars[1]);
	desc->connect_port(handle, PORT_DRAWBAR_4, &drawbars[2]);
	desc->connect_port(handle, PORT_DRAWBAR_IV, &drawbars[3]);
	desc->connect_port(handle, PORT_DRAWBAR_FLUTE, &drawbars[4]);
	desc->connect_port(handle, PORT_DRAWBAR_REED, &drawbars[5]);
	desc->connect_port(handle, PORT_VIBRATO, &vibrato);
	for (i = 0; i < 8; i++)
		desc->run(handle, block);
	desc->cleanup(handle);
}

TEST(test_run_midi_carla_flute_zero_defaults)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;
	float out_l[128];
	float out_r[128];
	uint8_t midi_buf[256];
	LV2_Atom_Sequence *midi_seq;
	float gain = 0.7f;
	float drawbars[6] = { 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f };
	uint32_t block = 128;
	unsigned i;
	float peak = 0.0f;

	build_note_on_sequence(midi_buf, sizeof(midi_buf), 60, 100, &midi_seq);

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->connect_port(handle, PORT_AUDIO_L, out_l);
	desc->connect_port(handle, PORT_AUDIO_R, out_r);
	desc->connect_port(handle, PORT_MIDI_IN, midi_seq);
	desc->connect_port(handle, PORT_GAIN, &gain);
	desc->connect_port(handle, PORT_DRAWBAR_16, &drawbars[0]);
	desc->connect_port(handle, PORT_DRAWBAR_8, &drawbars[1]);
	desc->connect_port(handle, PORT_DRAWBAR_4, &drawbars[2]);
	desc->connect_port(handle, PORT_DRAWBAR_IV, &drawbars[3]);
	desc->connect_port(handle, PORT_DRAWBAR_FLUTE, &drawbars[4]);
	desc->connect_port(handle, PORT_DRAWBAR_REED, &drawbars[5]);
	for (i = 0; i < 64; i++) {
		float block_peak;

		desc->run(handle, block);
		block_peak = test_buffer_peak(out_l, block);
		if (block_peak > peak)
			peak = block_peak;
	}
	ASSERT(peak > 1e-4f);
	desc->cleanup(handle);
}

TEST(test_run_zero_frames)
{
	const LV2_Descriptor *desc = get_descriptor();
	LV2_Handle handle;

	ASSERT(desc != NULL);
	handle = desc->instantiate(desc, PLUGIN_SR, NULL, NULL);
	ASSERT(handle != NULL);
	desc->run(handle, 0);
	desc->cleanup(handle);
}

void
test_plugin_suite(void)
{
	test_suite_begin("plugin");
	RUN_TEST(test_descriptor);
	RUN_TEST(test_instantiate_cleanup);
	RUN_TEST(test_run_without_ports);
	RUN_TEST(test_run_with_audio_buffers);
	RUN_TEST(test_run_with_midi_sequence);
	RUN_TEST(test_run_block_size_change);
	RUN_TEST(test_run_control_ports);
	RUN_TEST(test_run_midi_carla_flute_zero_defaults);
	RUN_TEST(test_run_zero_frames);
}
