/*
 * Headless Bristol Vox engine glue for LV2.
 */
#include <string.h>
#include <stdint.h>

#include "bristol_lv2.h"
#include "bristolmidiapi.h"
#include "bristolmessages.h"
#include "ringbuffer.h"

extern void generateBLOwaveforms(int, float, int, float, int, float, int);
extern void initMidiRoutines(audioMain *, midiHandler midiRoutines[]);
extern bristolMidiHandler bristolMidiRoutines;
extern int initBristolAudio(audioMain *, Baudio *);
extern int bristolMidiController(Baudio *, int, float);
extern void rbMidiNote(audioMain *, bristolMidiMsg *);
extern int doAudioOps(audioMain *, float *, float *);
extern void freeBristolAudio(audioMain *, Baudio *);
extern void freeAudioMain(audioMain *);

static float *lv2_run_outbuf;
static float *lv2_run_startbuf;
static size_t lv2_run_buffloats;

static void
lv2_run_buffers_free(void)
{
	bristolfree(lv2_run_outbuf);
	bristolfree(lv2_run_startbuf);
	lv2_run_outbuf = NULL;
	lv2_run_startbuf = NULL;
	lv2_run_buffloats = 0;
}

static int
lv2_run_buffers_alloc(size_t nfloats)
{
	if (lv2_run_buffloats == nfloats && lv2_run_outbuf != NULL
			&& lv2_run_startbuf != NULL)
		return(0);

	lv2_run_buffers_free();
	lv2_run_outbuf = (float *) bristolmalloc(nfloats * sizeof(float));
	lv2_run_startbuf = (float *) bristolmalloc(nfloats * sizeof(float));
	if (lv2_run_outbuf == NULL || lv2_run_startbuf == NULL) {
		lv2_run_buffers_free();
		return(-1);
	}
	lv2_run_buffloats = nfloats;
	return(0);
}

static void
vox_set_drawbar_raw(audioMain *audiomain, int bar, int level)
{
	int value;

	if (bar < 0 || bar > 5)
		return;
	if (level < 0)
		level = 0;
	if (level > 8)
		level = 8;

	value = bar * 16 + level;
	bristolLv2SysexParam(audiomain, 0, 0, 0, value);
}

int
bristol_vox_engine_init(audioMain *audiomain, uint32_t sample_rate,
	uint32_t block_size)
{
	Baudio *baudio;

	memset(audiomain, 0, sizeof(*audiomain));

	audiomain->SysID = 0x534C6162;
	audiomain->samplerate = (int) sample_rate;
	audiomain->samplecount = (int) block_size;
	audiomain->voiceCount = BRISTOL_VOICECOUNT;
	audiomain->atStatus = BRISTOL_OK;
	audiomain->debuglevel = 0;

	generateBLOwaveforms(31, BRISTOL_VPO, 0, 0.0f, 0, 1.0f,
		(int) sample_rate);

	bristolLv2EngineAudioInit(audiomain);
	initMidiRoutines(audiomain, bristolMidiRoutines.bmr);

	baudio = (Baudio *) bristolmalloc0(sizeof(Baudio));
	baudio->mixflags = 0;
	baudio->sid = 0;
	baudio->midichannel = 0;
	baudio->samplerate = (int) sample_rate;
	baudio->samplecount = (int) block_size;
	baudio->voicecount = BRISTOL_VOICECOUNT;
	baudio->gain = 1.0f;
	audiomain->audiolist = baudio;

	audiomain->midiflags = BRISTOL_VOX;
	initBristolAudio(audiomain, baudio);
	baudio->midi = bristolMidiController;

	/* Classic drawbar starter: 16' and 8' up, flute mix on (reed optional). */
	vox_set_drawbar_raw(audiomain, 0, 8);
	vox_set_drawbar_raw(audiomain, 1, 8);
	vox_set_drawbar_raw(audiomain, 2, 4);
	vox_set_drawbar_raw(audiomain, 3, 0);
	vox_set_drawbar_raw(audiomain, 4, 8);
	vox_set_drawbar_raw(audiomain, 5, 0);
	bristolLv2SysexParam(audiomain, 0, 0, 1, 64);

	if (lv2_run_buffers_alloc((size_t) audiomain->segmentsize * 2) != 0)
		return(-1);

	return(0);
}

void
bristol_vox_engine_fini(audioMain *audiomain)
{
	Baudio *baudio = audiomain->audiolist;

	if (baudio != NULL) {
		freeBristolAudio(audiomain, baudio);
		bristolfree(baudio);
		audiomain->audiolist = NULL;
	}
	if (audiomain->rb != NULL) {
		jack_ringbuffer_free(audiomain->rb);
		audiomain->rb = NULL;
	}
	freeAudioMain(audiomain);
	lv2_run_buffers_free();
}

void
bristol_vox_engine_run(audioMain *audiomain, float *out_l, float *out_r,
	uint32_t nframes)
{
	float *outbuf;
	float *startbuf;
	uint32_t i;
	size_t nfloats;

	if ((int) nframes != audiomain->samplecount)
		return;

	nfloats = (size_t) audiomain->segmentsize * 2;
	if (lv2_run_buffers_alloc(nfloats) != 0)
		return;

	outbuf = lv2_run_outbuf;
	startbuf = lv2_run_startbuf;

	bristolbzero(outbuf, nfloats * sizeof(float));
	bristolbzero(startbuf, nfloats * sizeof(float));
	doAudioOps(audiomain, outbuf, startbuf);

	for (i = 0; i < nframes; i++) {
		float l = outbuf[i * 2];
		float r = outbuf[i * 2 + 1];

		if (l > 1.0f)
			l = 1.0f;
		else if (l < -1.0f)
			l = -1.0f;
		if (r > 1.0f)
			r = 1.0f;
		else if (r < -1.0f)
			r = -1.0f;
		out_l[i] = l;
		out_r[i] = r;
	}
}

void
bristol_vox_engine_midi(audioMain *audiomain, const uint8_t *data,
	uint32_t size)
{
	bristolMidiMsg msg;
	int status, event;

	if (size < 1)
		return;

	status = data[0];
	event = (status & ~MIDI_STATUS_MASK) >> 4;

	memset(&msg, 0, sizeof(msg));
	msg.channel = status & 0x0f;

	switch (event) {
	case 0: /* note off */
		if (size < 3)
			return;
		msg.command = MIDI_NOTE_OFF | msg.channel;
		msg.params.key.key = data[1];
		msg.params.key.velocity = data[2];
		rbMidiNote(audiomain, &msg);
		break;
	case 1: /* note on */
		if (size < 3)
			return;
		msg.command = MIDI_NOTE_ON | msg.channel;
		msg.params.key.key = data[1];
		msg.params.key.velocity = data[2];
		if (msg.params.key.velocity == 0) {
			msg.command = MIDI_NOTE_OFF | msg.channel;
			msg.params.key.velocity = 64;
		}
		rbMidiNote(audiomain, &msg);
		break;
	default:
		break;
	}
}

void
bristol_vox_engine_set_drawbar(audioMain *audiomain, int bar, float level)
{
	vox_set_drawbar_raw(audiomain, bar, (int) (level * 8.0f + 0.5f));
}

void
bristol_vox_engine_set_volume(audioMain *audiomain, float level)
{
	int value = (int) (level * 127.0f + 0.5f);

	if (value < 0)
		value = 0;
	if (value > 127)
		value = 127;
	bristolLv2SysexParam(audiomain, 0, 0, 1, value);
}

void
bristol_vox_engine_set_vibrato(audioMain *audiomain, int on)
{
	Baudio *baudio = audiomain->audiolist;

	if (baudio != NULL && baudio->param != NULL)
		baudio->param(baudio, 126, 0, on ? 1.0f : 0.0f);
}
