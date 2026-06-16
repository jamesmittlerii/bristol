#ifndef BRISTOL_LV2_H
#define BRISTOL_LV2_H

#include "bristol.h"

#define BRISTOL_LV2_URI "http://bristol.sourceforge.net/lv2/vox"
#define BRISTOL_LV2_BLOCK_SIZE 128

extern void bristolLv2EngineAudioInit(audioMain *audiomain);
extern int bristolLv2SysexParam(audioMain *audiomain, int sid, int op,
	int ctrl, int value);

int bristol_vox_engine_init(audioMain *audiomain, uint32_t sample_rate,
	uint32_t block_size);
void bristol_vox_engine_fini(audioMain *audiomain);
void bristol_vox_engine_run(audioMain *audiomain, float *out_l, float *out_r,
	uint32_t nframes);
void bristol_vox_engine_midi(audioMain *audiomain, const uint8_t *data,
	uint32_t size);
void bristol_vox_engine_set_drawbar(audioMain *audiomain, int bar, float level);
void bristol_vox_engine_set_volume(audioMain *audiomain, float level);
void bristol_vox_engine_set_vibrato(audioMain *audiomain, int on);

#endif /* BRISTOL_LV2_H */
