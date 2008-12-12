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

int midiVolume = 255;

//------------------------------------------------------------------------------

void DigiSetMidiVolume(int n)
{
if (n < 0)
	midiVolume = 0;
else if (n > 127)
	midiVolume = 127;
else
	midiVolume = n;

#if USE_SDL_MIXER
if (gameOpts->sound.bUseSDLMixer)
	Mix_VolumeMusic (midiVolume);
#endif
#if defined (_WIN32)
#	if USE_SDL_MIXER
else 
#	endif
if (hmp) {
	int mmVolume;

	// scale up from 0-127 to 0-0xffff
	mmVolume = (midiVolume << 1) | (midiVolume & 1);
	mmVolume |= (mmVolume << 8);
	n = midiOutSetVolume((HMIDIOUT)hmp->hmidi, mmVolume | (mmVolume << 16));
	}
#endif
}

//------------------------------------------------------------------------------

void DigiStopCurrentSong ()
{
	int h;

if (gameData.songs.bPlaying) {
	DigiFadeoutMusic ();
	h = midiVolume;	// preserve it for another song being started
	DigiSetMidiVolume(0);
	midiVolume = h;
#if defined (_WIN32)
#	if USE_SDL_MIXER
if (!gameOpts->sound.bUseSDLMixer)
#	endif
		{
		hmp_close (hmp);
		hmp = NULL;
		gameData.songs.bPlaying = 0;
		}
#endif
	}
}

//------------------------------------------------------------------------------

int DigiPlayMidiSong (char *pszSong, char *melodic_bank, char *drum_bank, int bLoop, int bD1Song)
{
	int	bCustom;
#if 0
if (!gameStates.sound.digi.bInitialized)
	return 0;
#endif
LogErr ("DigiPlayMidiSong (%s)\n", pszSong);
DigiStopCurrentSong ();
if (!(pszSong && *pszSong))
	return 0;
if (midiVolume < 1)
	return 0;
bCustom = ((strstr (pszSong, ".mp3") != NULL) || (strstr (pszSong, ".ogg") != NULL));
if (!(bCustom || (hmp = hmp_open (pszSong, bD1Song))))
	return 0;
#if USE_SDL_MIXER
if (gameOpts->sound.bUseSDLMixer) {
	char	fnSong [FILENAME_LEN], *pfnSong;

	if (bCustom) {
		pfnSong = pszSong;
		if (strstr (pszSong, ".mp3") && !gameData.songs.user.bMP3) {
			DigiExit ();
			gameData.songs.user.bMP3 = 1;
			DigiInit (1);
			}
		}
	else {
		if (!strstr (pszSong, ".mp3") && gameData.songs.user.bMP3) {
			DigiExit ();
			gameData.songs.user.bMP3 = 0;
			DigiInit (1);
			}
		sprintf (fnSong, "%s/d2x-temp.mid", *gameFolders.szTempDir ? gameFolders.szTempDir : gameFolders.szHomeDir);
		if (!hmp_to_midi (hmp, fnSong)) {
			LogErr ("SDL_mixer failed to load %s\n(%s)\n", fnSong, Mix_GetError ());
			return 0;
			}
		pfnSong = fnSong;
		}
	if (!(mixMusic = Mix_LoadMUS (pfnSong))) {
		LogErr ("SDL_mixer failed to load %s\n(%s)\n", fnSong, Mix_GetError ());
		return 0;
		}
	if (-1 == Mix_FadeInMusicPos (mixMusic, bLoop ? -1 : 1, gameData.songs.tPos ? 1000 : 1500, (double) gameData.songs.tPos / 1000.0)) {
		LogErr ("SDL_mixer cannot play %s\n(%s)\n", pszSong, Mix_GetError ());
		gameData.songs.tPos = 0;
		return 0;
		}
	LogErr ("SDL_mixer playing %s\n", pszSong);
	if (gameData.songs.tPos)
		gameData.songs.tPos = 0;
	else
		gameData.songs.tStart = SDL_GetTicks ();
	
	gameData.songs.bPlaying = 1;
	DigiSetMidiVolume (midiVolume);
	return 1;
	}
#endif
#if defined (_WIN32)
if (bCustom) {
	LogErr ("Cannot play %s - enable SDL_mixer\n", pszSong);
	return 0;
	}
hmp_play (hmp, bLoop);
gameData.songs.bPlaying = 1;
DigiSetMidiVolume (midiVolume);
#endif
return 1;
}

//------------------------------------------------------------------------------

int sound_paused = 0;

void DigiPauseMidi()
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
