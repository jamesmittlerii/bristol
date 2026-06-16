/*
 * Trimmed Bristol operator palette for the Vox Continental LV2 plugin.
 */
#include "bristol.h"
#include "palette.h"

extern bristolOP *envinit();
extern bristolOP *dcainit();
extern bristolOP *vchorusinit();
extern bristolOP *voxdcoinit();
extern int bristolVoxInit(audioMain *, Baudio *);

struct bristolPalette bristolPalette[BRISTOL_SYNTHCOUNT] = {
	{NULL}, {envinit}, {dcainit},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {vchorusinit},
	{NULL}, {NULL}, {NULL}, {voxdcoinit},
};

struct bristolAlgos bristolAlgos[BRISTOL_SYNTHCOUNT] = {
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL},
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL},
	{bristolVoxInit, "vox"},
};
