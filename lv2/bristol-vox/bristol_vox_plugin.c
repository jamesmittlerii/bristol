/*
 * Bristol Vox Continental — LV2 plugin for mod-host / Raspberry Pi.
 */
#include <stdlib.h>
#include <string.h>

#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/midi/midi.h>

#include "bristol_lv2.h"

#define BRISTOL_VOX_NS "http://bristol.sourceforge.net/lv2/vox#"

typedef enum {
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
} PortIndex;

typedef struct {
	audioMain engine;
	float *audio_l;
	float *audio_r;
	const LV2_Atom_Sequence *midi_in;
	const float *gain;
	const float *drawbar_16;
	const float *drawbar_8;
	const float *drawbar_4;
	const float *drawbar_iv;
	const float *drawbar_flute;
	const float *drawbar_reed;
	const float *vibrato;
	uint32_t block_size;
	uint32_t sample_rate;
	float last_drawbar[6];
	float last_gain;
	float last_vibrato;
} BristolVox;

static LV2_Descriptor descriptor = {
	BRISTOL_LV2_URI,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static void
connect_port(LV2_Handle instance, uint32_t port, void *data)
{
	BristolVox *plugin = (BristolVox *) instance;

	switch ((PortIndex) port) {
	case PORT_AUDIO_L: plugin->audio_l = (float *) data; break;
	case PORT_AUDIO_R: plugin->audio_r = (float *) data; break;
	case PORT_MIDI_IN: plugin->midi_in = (const LV2_Atom_Sequence *) data; break;
	case PORT_GAIN: plugin->gain = (const float *) data; break;
	case PORT_DRAWBAR_16: plugin->drawbar_16 = (const float *) data; break;
	case PORT_DRAWBAR_8: plugin->drawbar_8 = (const float *) data; break;
	case PORT_DRAWBAR_4: plugin->drawbar_4 = (const float *) data; break;
	case PORT_DRAWBAR_IV: plugin->drawbar_iv = (const float *) data; break;
	case PORT_DRAWBAR_FLUTE: plugin->drawbar_flute = (const float *) data; break;
	case PORT_DRAWBAR_REED: plugin->drawbar_reed = (const float *) data; break;
	case PORT_VIBRATO: plugin->vibrato = (const float *) data; break;
	default: break;
	}
}

static int
footages_active(BristolVox *plugin)
{
	return ((plugin->drawbar_16 != NULL && *plugin->drawbar_16 > 0.0f) ||
	    (plugin->drawbar_8 != NULL && *plugin->drawbar_8 > 0.0f) ||
	    (plugin->drawbar_4 != NULL && *plugin->drawbar_4 > 0.0f));
}

static float
resolve_drawbar_value(BristolVox *plugin, int bar, float host)
{
	/*
	 * Flute is a mix bus: tonewheel footages are silent when both mix buses
	 * are off. Hosts often keep this port at 0 (saved rack / ignored default)
	 * so keep the bus on whenever footages are up and reed is not carrying
	 * the mix.
	 */
	if (bar != 4)
		return host;

	if (host > 0.0f)
		return host;

	if (plugin->drawbar_reed != NULL && *plugin->drawbar_reed > 0.0f)
		return host;

	if (footages_active(plugin))
		return 1.0f;

	return host;
}

static void
update_controls(BristolVox *plugin)
{
	const float *bars[6];
	int i;

	bars[0] = plugin->drawbar_16;
	bars[1] = plugin->drawbar_8;
	bars[2] = plugin->drawbar_4;
	bars[3] = plugin->drawbar_iv;
	bars[4] = plugin->drawbar_flute;
	bars[5] = plugin->drawbar_reed;

	for (i = 0; i < 6; i++) {
		float value;

		if (bars[i] == NULL)
			continue;
		value = resolve_drawbar_value(plugin, i, *bars[i]);
		if (value != plugin->last_drawbar[i]) {
			bristol_vox_engine_set_drawbar(&plugin->engine, i, value);
			plugin->last_drawbar[i] = value;
		}
	}

	if (plugin->gain != NULL && *plugin->gain != plugin->last_gain) {
		bristol_vox_engine_set_volume(&plugin->engine, *plugin->gain);
		plugin->last_gain = *plugin->gain;
	}

	if (plugin->vibrato != NULL && *plugin->vibrato != plugin->last_vibrato) {
		bristol_vox_engine_set_vibrato(&plugin->engine, *plugin->vibrato >= 0.5f);
		plugin->last_vibrato = *plugin->vibrato;
	}
}

static void
run_midi(BristolVox *plugin)
{
	if (plugin->midi_in == NULL)
		return;

	LV2_ATOM_SEQUENCE_FOREACH(plugin->midi_in, ev) {
		const uint8_t *const msg = (const uint8_t *)(ev + 1);
		bristol_vox_engine_midi(&plugin->engine, msg, ev->body.size);
	}
}

static int
ensure_engine_block_size(BristolVox *plugin, uint32_t nframes)
{
	if (nframes == 0)
		return(-1);

	if (plugin->block_size == nframes)
		return(0);

	if (plugin->block_size != 0)
		bristol_vox_engine_fini(&plugin->engine);

	if (bristol_vox_engine_init(&plugin->engine, plugin->sample_rate,
			nframes) != 0)
		return(-1);

	plugin->block_size = nframes;
	plugin->last_gain = -1.0f;
	plugin->last_vibrato = -1.0f;
	{
		int i;
		for (i = 0; i < 6; i++)
			plugin->last_drawbar[i] = -1.0f;
	}
	return(0);
}

static LV2_Handle
instantiate(const LV2_Descriptor *desc, double rate, const char *path,
	const LV2_Feature *const *features)
{
	BristolVox *plugin;
	(void) desc;
	(void) path;
	(void) features;

	plugin = (BristolVox *) calloc(1, sizeof(BristolVox));
	if (plugin == NULL)
		return(NULL);

	plugin->sample_rate = (uint32_t) rate;
	plugin->block_size = 0;
	plugin->last_gain = -1.0f;
	plugin->last_vibrato = -1.0f;
	{
		int i;
		for (i = 0; i < 6; i++)
			plugin->last_drawbar[i] = -1.0f;
	}

	return(plugin);
}

static void
activate(LV2_Handle instance)
{
	(void) instance;
}

static void
run(LV2_Handle instance, uint32_t nframes)
{
	BristolVox *plugin = (BristolVox *) instance;

	if (ensure_engine_block_size(plugin, nframes) != 0)
		return;

	update_controls(plugin);
	run_midi(plugin);

	if (plugin->audio_l == NULL || plugin->audio_r == NULL)
		return;

	if (plugin->gain != NULL && plugin->engine.audiolist != NULL) {
		float g = *plugin->gain;

		if (g < 0.0f)
			g = 0.0f;
		if (g > 1.0f)
			g = 1.0f;
		/* Bristol postops still add headroom; keep master gain conservative. */
		plugin->engine.audiolist->gain = 0.15f + g * 0.55f;
	}

	bristol_vox_engine_run(&plugin->engine, plugin->audio_l, plugin->audio_r,
		nframes);
}

static void
deactivate(LV2_Handle instance)
{
	(void) instance;
}

static void
plugin_cleanup(LV2_Handle instance)
{
	BristolVox *plugin = (BristolVox *) instance;

	if (plugin->block_size != 0)
		bristol_vox_engine_fini(&plugin->engine);
	free(plugin);
}

static const void *
extension_data(const char *uri)
{
	(void) uri;
	return(NULL);
}

static void
init_descriptor(void)
{
	descriptor.instantiate = instantiate;
	descriptor.connect_port = connect_port;
	descriptor.activate = activate;
	descriptor.run = run;
	descriptor.deactivate = deactivate;
	descriptor.cleanup = plugin_cleanup;
	descriptor.extension_data = extension_data;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t index)
{
	if (index != 0)
		return(NULL);
	init_descriptor();
	return(&descriptor);
}
