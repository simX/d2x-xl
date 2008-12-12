// MIDI stuff follows.
#include <stdio.h>

#include "digi.h"
#include "cfile.h"
#include "error.h"
#include "hmpfile.h"
#include "inferno.h"

#if USE_SDL_MIXER
#	ifdef __macosx__
#		include <SDL/SDL_mixer.h>
#	else
#		include <SDL_mixer.h>
#	endif

Mix_Music *mixMusic = NULL;
#endif

hmp_file *hmp = NULL;

int midi_volume = 255;
int digi_midi_song_playing = 0;

//------------------------------------------------------------------------------

void DigiSetMidiVolume(int n)
{
if (n < 0)
	midi_volume = 0;
else if (n > 127)
	midi_volume = 127;
else
	midi_volume = n;

#if USE_SDL_MIXER
if (gameOpts->sound.bUseSDLMixer)
	Mix_VolumeMusic (midi_volume);
#endif
#if defined (_WIN32)
#	if USE_SDL_MIXER
else 
#	endif
if (hmp) {
	int mm_volume;

	// scale up from 0-127 to 0-0xffff
	mm_volume = (midi_volume << 1) | (midi_volume & 1);
	mm_volume |= (mm_volume << 8);
	n = midiOutSetVolume((HMIDIOUT)hmp->hmidi, mm_volume | (mm_volume << 16));
	}
#endif
}

//------------------------------------------------------------------------------

void DigiStopCurrentSong()
{
if (digi_midi_song_playing) {
	int h = midi_volume;	// preserve it for another song being started
	DigiSetMidiVolume(0);
	midi_volume = h;

#if USE_SDL_MIXER
	if (gameOpts->sound.bUseSDLMixer) {
		Mix_HaltMusic ();
		Mix_FreeMusic (mixMusic);
		mixMusic = NULL;
		}
#endif
#if defined (_WIN32)
#	if USE_SDL_MIXER
else 
#	endif
		{
		hmp_close (hmp);
		hmp = NULL;
		digi_midi_song_playing = 0;
		}
#endif
	}
}

//------------------------------------------------------------------------------

void DigiPlayMidiSong(char *filename, char *melodic_bank, char *drum_bank, int loop, int bD1Song)
{
#if 0
if (!gameStates.sound.digi.bInitialized)
	return;
#endif

DigiStopCurrentSong();
if (filename == NULL)
	return;
if (midi_volume < 1)
	return;
if (hmp = hmp_open (filename, bD1Song)) {
#if USE_SDL_MIXER
	if (gameOpts->sound.bUseSDLMixer) {
		char	fnMusic [FILENAME_LEN];

		sprintf (fnMusic, "%s%sd2x-temp.mid", gameFolders.szDataDir, *gameFolders.szDataDir ? "/" : "");
		if (hmp_to_midi (hmp, fnMusic) && (mixMusic = Mix_LoadMUS (fnMusic))) {
			if (Mix_PlayMusic (mixMusic, loop ? -1 : 1) != -1) {
				digi_midi_song_playing = 1;
				DigiSetMidiVolume (midi_volume);
				}
			}
		}
#endif
#if defined (_WIN32)
#	if USE_SDL_MIXER
else 
#	endif
		{
		hmp_play(hmp, loop);
		digi_midi_song_playing = 1;
		DigiSetMidiVolume(midi_volume);
		}
#endif
	}
}

//------------------------------------------------------------------------------

int sound_paused = 0;

void digi_pause_midi()
{
#if 0
	if (!gameStates.sound.digi.bInitialized)
		return;
#endif

if (sound_paused == 0) {
#if USE_SDL_MIXER
	if (gameOpts->sound.bUseSDLMixer)
		Mix_PauseMusic ();
#endif
	}
sound_paused++;
}

//------------------------------------------------------------------------------

void DigiResumeMidi()
{
#if 0
	if (!gameStates.sound.digi.bInitialized)
		return;
#endif
	Assert(sound_paused > 0);
if (sound_paused == 1) {
#if USE_SDL_MIXER
	if (gameOpts->sound.bUseSDLMixer)
		Mix_ResumeMusic ();
#endif
	}
sound_paused--;
}

//------------------------------------------------------------------------------
