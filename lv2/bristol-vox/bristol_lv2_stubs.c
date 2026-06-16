/*
 * Stubs so the Vox LV2 build does not need libbristolmidi or ALSA/JACK audio.
 */
#include "bristol.h"
#include "bristolmidiapi.h"
#include <bristolmidiapidata.h>
#include "bristolmessages.h"
#include "bristolmidi.h"
#include "bristolarpeggiation.h"

bristolMidiHandler bristolMidiRoutines;
audioMain audiomain;
int dupfd = -1;
char *outputfile = NULL;

int
bristolAudioOpen()
{
	return(0);
}

int
bristolAudioClose()
{
	return(0);
}

int
bristolAudioWrite()
{
	return(0);
}

int
bristolAudioRead()
{
	return(0);
}

int
bristolAudioStart()
{
	return(0);
}

void
bristolMidiPrint(bristolMidiMsg *msg)
{
	(void) msg;
}

void
bristolMidiPrintGM2(bristolMidiMsg *msg)
{
	(void) msg;
}

void
bristolMidiToGM2(GM2values, midimap, valuemap, msg)
	int GM2values[128];
	int midimap[128];
	u_char valuemap[128][128];
	bristolMidiMsg *msg;
{
	(void) GM2values;
	(void) midimap;
	(void) valuemap;
	(void) msg;
}

int
bristolParseScala(char *file, float *freq)
{
	(void) file;
	(void) freq;
	return(0);
}

int
bristolSystem(audioMain *audiomain, bristolMidiMsg *msg)
{
	(void) audiomain;
	(void) msg;
	return(0);
}

void
alterAllNotes(Baudio *baudio)
{
	(void) baudio;
}

int
bristolGetMap(char *file, char *match, float *points, int count, int flags)
{
	(void) file;
	(void) match;
	(void) points;
	(void) count;
	(void) flags;
	return(0);
}

int
bristolGetFreqMap(char *file, char *match, fTab *points, int count, int flags,
	int samplerate)
{
	(void) file;
	(void) match;
	(void) points;
	(void) count;
	(void) flags;
	(void) samplerate;
	return(0);
}

int
bristolMidiOption(int handle, int option, int value)
{
	(void) handle;
	(void) option;
	(void) value;
	return(0);
}

void
bristolArpeggiator(audioMain *audiomain, bristolMidiMsg *msg)
{
	(void) audiomain;
	(void) msg;
}

void
bristolArpeggiatorNoteEvent(Baudio *baudio, bristolMidiMsg *msg)
{
	(void) baudio;
	(void) msg;
}

int
bristolArpegReAudio(audioMain *audiomain, Baudio *baudio)
{
	(void) audiomain;
	(void) baudio;
	return(0);
}

int
bristolArpegReVoice(Baudio *baudio, bristolVoice *voice, float sr)
{
	(void) baudio;
	(void) voice;
	(void) sr;
	return(-1);
}

void
bristolArpeggiatorInit(Baudio *baudio)
{
	(void) baudio;
}

void
bristolMidiValueMappingTable(u_char valuemap[128][128], int midimap[128],
	char *synth)
{
	(void) valuemap;
	(void) midimap;
	(void) synth;
}

void
midithreadexit(void)
{
}
