/* $Id: digiobj.c,v 1.1 2004/11/29 05:25:58 btb Exp $ */
/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.  
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/


#ifdef RCS
static char rcsid [] = "$Id: digiobj.c,v 1.1 2004/11/29 05:25:58 btb Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 
#include <string.h>
#include <ctype.h>

#include "fix.h"
#include "inferno.h"
#include "mono.h"
#include "timer.h"
#include "joy.h"
#include "digi.h"
#include "sounds.h"
#include "args.h"
#include "key.h"
#include "newdemo.h"
#include "game.h"
#include "error.h"
#include "wall.h"
#include "cfile.h"
#include "piggy.h"
#include "text.h"
#include "kconfig.h"
#include "gameseg.h"

#ifdef __macosx__
# include <SDL/SDL.h>
#else
# include <SDL.h>
# if USE_SDL_MIXER
#  include <SDL_mixer.h>
# endif
#endif

#define SOF_USED				1 		// Set if this sample is used
#define SOF_PLAYING			2		// Set if this sample is playing on a channel
#define SOF_LINK_TO_OBJ		4		// Sound is linked to a moving tObject. If tObject dies, then finishes play and quits.
#define SOF_LINK_TO_POS		8		// Sound is linked to tSegment, pos
#define SOF_PLAY_FOREVER	16		// Play bForever (or until level is stopped), otherwise plays once
#define SOF_PERMANENT		32		// Part of the level, like a waterfall or fan

typedef struct tSoundObject {
	short			nSignature;		// A unique nSignature to this sound
	ubyte			flags;			// Used to tell if this slot is used and/or currently playing, and how long.
	ubyte			pad;				//	Keep alignment
	fix			maxVolume;		// Max volume that this sound is playing at
	fix			maxDistance;	// The max distance that this sound can be heard at...
	int			volume;			// Volume that this sound is playing at
	int			pan;				// Pan value that this sound is playing at
	int			channel;			// What channel this is playing on, -1 if not playing
	short			nSound;			// The sound number that is playing
	char			szSound [SHORT_FILENAME_LEN];	// file name of custom sound to be played
	int			nLoopStart;		// The start point of the loop. -1 means no loop
	int			nLoopEnd;		// The end point of the loop
	union {	
		struct {
			short			nSegment;				// Used if SOF_LINK_TO_POS field is used
			short			nSide;				
			vmsVector	position;
		} pos;
		struct {
			short			nObject;				// Used if SOF_LINK_TO_OBJ field is used
			short			nObjSig;
		} obj;
	} linkType;
} tSoundObject;

#define MAX_SOUND_OBJECTS 150
tSoundObject SoundObjects [MAX_SOUND_OBJECTS];

//------------------------------------------------------------------------------
/* Find the sound which actually equates to a sound number */
short DigiXlatSound (short nSound)
{
if (nSound < 0)
	return -1;
if (gameStates.sound.digi.bLoMem) {
	nSound = AltSounds [gameOpts->sound.bD1Sound][nSound];
	if (nSound == 255)
		return -1;
	}
//Assert (Sounds [gameOpts->sound.bD1Sound][nSound] != 255);	//if hit this, probably using undefined sound
if (Sounds [gameOpts->sound.bD1Sound][nSound] == 255)
	return -1;
return Sounds [gameOpts->sound.bD1Sound][nSound];
}

//------------------------------------------------------------------------------

int DigiUnXlatSound (int nSound)
{
	int i;
	ubyte *table = (gameStates.sound.digi.bLoMem ? AltSounds [gameOpts->sound.bD1Sound] :Sounds [gameOpts->sound.bD1Sound]);

if (nSound < 0) 
	return -1;
for (i = 0;i < MAX_SOUNDS; i++)
	if (table [i] == nSound)
		return i;
Int3 ();
return 0;
}

//------------------------------------------------------------------------------

void DigiGetSoundLoc (
	vmsMatrix * mListener, vmsVector * vListenerPos, short nListenerSeg, vmsVector * vSoundPos, 
	short nSoundSeg, fix maxVolume, int *volume, int *pan, fix maxDistance)	
{	  
	vmsVector	vecToSound;
	fix 			angleFromEar, cosang,sinang;
	fix 			distance;
	fix 			pathDistance;

*volume = 0;
*pan = 0;
maxDistance = (5 * maxDistance) / 4;	// Make all sounds travel 1.25 times as far.
//	Warning: Made the VmVecNormalizedDir be VmVecNormalizedDirQuick and got illegal values to acos in the fang computation.
distance = VmVecNormalizedDirQuick (&vecToSound, vSoundPos, vListenerPos);
if (distance < maxDistance) {
	int nSearchSegs = f2i (maxDistance / 20);
	if (nSearchSegs < 1)
		nSearchSegs = 1;
	pathDistance = FindConnectedDistance (vListenerPos, nListenerSeg, vSoundPos, nSoundSeg, nSearchSegs, WID_RENDPAST_FLAG + WID_FLY_FLAG);
	if (pathDistance > -1) {
		*volume = maxVolume - FixDiv (pathDistance, maxDistance);
		if (*volume <= 0) 
			*volume = 0;
		else {
			angleFromEar = VmVecDeltaAngNorm (&mListener->rVec, &vecToSound, &mListener->uVec);
			FixSinCos (angleFromEar, &sinang, &cosang);
			if (gameConfig.bReverseChannels) 
				cosang = -cosang;
			*pan = (cosang + F1_0) / 2;
			}
		}
	}																					  
}

//------------------------------------------------------------------------------

void DigiPlaySampleOnce (short nSound, fix maxVolume)	
{
	int channel;

#ifdef NEWDEMO
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordSound (nSound);
#endif
nSound = DigiXlatSound (nSound);
if (nSound < 0) 
	return;
channel = DigiFindChannel (nSound);
if (channel > -1)
	DigiStopSound (channel);
// start the sample playing
DigiStartSound (nSound, maxVolume, 0xffff/2, 0, -1, -1, -1, F1_0, NULL, NULL);
}

//------------------------------------------------------------------------------

int DigiPlaySampleSpeed (short nSound, fix maxVolume, int nSpeed, int nLoops, char *pszWAV)
{
if (!pszWAV) {
#ifdef NEWDEMO
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordSound (nSound);
#endif
	nSound = (nSound < 0) ? - nSound : DigiXlatSound (nSound);
	if (nSound < 0) 
		return -1;
	}
// start the sample playing
if (nLoops > 0)
	return DigiStartSound (nSound, maxVolume, 0xffff / 2, 0, 0, nLoops - 1, -1, nSpeed, pszWAV, NULL);
else
	return DigiStartSound (nSound, maxVolume, 0xffff / 2, 0, -1, -1, -1, nSpeed, pszWAV, NULL);
}

//------------------------------------------------------------------------------

int DigiPlaySample (short nSound, fix maxVolume)
{
return DigiPlaySampleSpeed (nSound, maxVolume, F1_0, 0, NULL);
}

//------------------------------------------------------------------------------

int DigiPlaySampleLooped (short nSound, fix maxVolume, int nLoops)
{
return DigiPlaySampleSpeed (nSound, maxVolume, F1_0, nLoops, NULL);
}

//------------------------------------------------------------------------------

int DigiPlayWAV (char *pszWAV, fix maxVolume)
{
return DigiPlaySampleSpeed (-1, maxVolume, F1_0, 0, pszWAV);
}

//------------------------------------------------------------------------------

int DigiPlayWAVLooped (char *pszWAV, fix maxVolume, int nLoops)
{
return DigiPlaySampleSpeed (-1, maxVolume, F1_0, nLoops, pszWAV);
}

//------------------------------------------------------------------------------

void DigiPlaySample3D (short nSound, int angle, int volume, int no_dups, vmsVector *vPos, char *pszSound)	
{

	no_dups = 1;

#ifdef NEWDEMO
if (gameData.demo.nState == ND_STATE_RECORDING)	{
	if (no_dups)
		NDRecordSound3DOnce (nSound, angle, volume);
	else
		NDRecordSound3D (nSound, angle, volume);
	}
#endif
nSound = DigiXlatSound (nSound);
if (nSound < 0) 
	return;
if (volume < 10) 
	return;
// start the sample playing
DigiStartSound (nSound, volume, angle, 0, -1, -1, -1, F1_0, pszSound, vPos);
}

//------------------------------------------------------------------------------

void SoundQInit ();
void SoundQProcess ();
void SoundQPause ();

void DigiInitSounds ()
{
	int i;

SoundQInit ();
DigiStopAllChannels ();
DigiStopLoopingSound ();
for (i = 0; i < MAX_SOUND_OBJECTS; i++) {
	SoundObjects [i].channel = -1;
	SoundObjects [i].flags = 0;	// Mark as dead, so some other sound can use this sound
	}
gameStates.sound.digi.nActiveObjects = 0;
gameStates.sound.digi.bSoundsInitialized = 1;
}

//------------------------------------------------------------------------------

// plays a sample that loops bForever.  
// Call digi_stop_channe (channel) to stop it.
// Call DigiSetChannelVolume (channel, volume) to change volume.
// if nLoopStart is -1, entire sample loops
// Returns the channel that sound is playing on, or -1 if can't play.
// This could happen because of no sound drivers loaded or not enough channels.

void DigiPlaySampleLoopingSub (void)
{
if (gameStates.sound.digi.nLoopingSound > -1)
	gameStates.sound.digi.nLoopingChannel  = 
		DigiStartSound (gameStates.sound.digi.nLoopingSound, 
								gameStates.sound.digi.nLoopingVolume, 
								0xFFFF/2, 
								1, 
								gameStates.sound.digi.nLoopingStart, 
								gameStates.sound.digi.nLoopingEnd, 
								-1, 
								F1_0, NULL, 
								NULL);
}

//------------------------------------------------------------------------------

void DigiPlaySampleLooping (short nSound, fix maxVolume,int nLoopStart, int nLoopEnd)
{
nSound = DigiXlatSound (nSound);
if (nSound < 0) 
	return;
if (gameStates.sound.digi.nLoopingChannel>-1)
	DigiStopSound (gameStates.sound.digi.nLoopingChannel);
gameStates.sound.digi.nLoopingSound = (short) nSound;
gameStates.sound.digi.nLoopingVolume = (short) maxVolume;
gameStates.sound.digi.nLoopingStart = (short) nLoopStart;
gameStates.sound.digi.nLoopingEnd = (short) nLoopEnd;
DigiPlaySampleLoopingSub ();
}

//------------------------------------------------------------------------------

void DigiChangeLoopingVolume (fix volume)
{
if (gameStates.sound.digi.nLoopingChannel > -1)
	DigiSetChannelVolume (gameStates.sound.digi.nLoopingChannel, volume);
gameStates.sound.digi.nLoopingVolume = (short) volume;
}

//------------------------------------------------------------------------------

void DigiStopLoopingSound ()
{
if (gameStates.sound.digi.nLoopingChannel > -1)
	DigiStopSound (gameStates.sound.digi.nLoopingChannel);
gameStates.sound.digi.nLoopingChannel = -1;
gameStates.sound.digi.nLoopingSound = -1;
}
	
//------------------------------------------------------------------------------

void DigiPauseLoopingSound ()
{
if (gameStates.sound.digi.nLoopingChannel > -1)
	DigiStopSound (gameStates.sound.digi.nLoopingChannel);
gameStates.sound.digi.nLoopingChannel = -1;
}

//------------------------------------------------------------------------------

void DigiResumeLoopingSound ()
{
DigiPlaySampleLoopingSub ();
}

//------------------------------------------------------------------------------
//hack to not start tObject when loading level

void DigiStartSoundObject (int i)
{
	tSoundObject	*soP = SoundObjects + i;
	// start sample structures 
soP->channel = -1;
if (soP->volume <= 0)
	return;
if (gameStates.sound.bDontStartObjects)
	return;
// -- MK, 2/22/96 -- 	if (gameData.demo.nState == ND_STATE_RECORDING)
// -- MK, 2/22/96 -- 		NDRecordSound3DOnce (DigiUnXlatSound (soP->nSound), soP->pan, soP->volume);
// only use up to half the sound channels for "permanent" sounts
if ((soP->flags & SOF_PERMANENT) && 
	 (gameStates.sound.digi.nActiveObjects >= max (1, DigiGetMaxChannels () / 4)))
	return;
// start the sample playing
soP->channel = 
	DigiStartSound (
		soP->nSound, 
		soP->volume, 
		soP->pan, 
		soP->flags & SOF_PLAY_FOREVER, 
		soP->nLoopStart, 
		soP->nLoopEnd, i, F1_0, 
		soP->szSound,
		(soP->flags & SOF_LINK_TO_OBJ) ? &gameData.objs.objects [soP->linkType.obj.nObject].position.vPos : &soP->linkType.pos.position);
if (soP->channel > -1)
	gameStates.sound.digi.nActiveObjects++;
}

//------------------------------------------------------------------------------
//sounds longer than this get their 3d aspects updated
#define SOUND_3D_THRESHHOLD  (gameOpts->sound.digiSampleRate * 3 / 2)	//1.5 seconds

int DigiLinkSoundToObject3 (
	short nOrgSound, short nObject, int bForever, fix maxVolume, fix maxDistance, 
	int nLoopStart, int nLoopEnd, char *pszSound)
{
	tObject			*objP;
	tSoundObject 	*soP;
	int				i, volume, pan;
	short				nSound;

if (maxVolume < 0) 
	return -1;
if ((nObject < 0) || (nObject > gameData.objs.nLastObject))
	return -1;
if (!(pszSound && *pszSound)) {
	nSound = DigiXlatSound (nOrgSound);
	if (nSound < 0) 
		return -1;
	if (!gameData.pig.sound.sounds [gameOpts->sound.bD1Sound][nSound].data) {
		Int3 ();
		return -1;
		}
	}
objP = gameData.objs.objects + nObject;
if (!bForever) { 	// Hack to keep sounds from building up...
	DigiGetSoundLoc (
		&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, 
		gameData.objs.viewer->nSegment, &objP->position.vPos, objP->nSegment, maxVolume, &volume, &pan, maxDistance);
	DigiPlaySample3D (nOrgSound, pan, volume, 0, &objP->position.vPos, pszSound);
	return -1;
	}
#ifdef NEWDEMO
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordLinkSoundToObject3 (nOrgSound, nObject, maxVolume, maxDistance, nLoopStart, nLoopEnd);
#endif
for (i = 0, soP = SoundObjects; i < MAX_SOUND_OBJECTS; i++, soP++)
	if (!soP->flags)
		break;
if (i == MAX_SOUND_OBJECTS) 
	return -1;
soP->nSignature = gameStates.sound.digi.nNextSignature++;
soP->flags = SOF_USED | SOF_LINK_TO_OBJ;
if (bForever)
	soP->flags |= SOF_PLAY_FOREVER;
soP->linkType.obj.nObject = nObject;
soP->linkType.obj.nObjSig = objP->nSignature;
soP->maxVolume = maxVolume;
soP->maxDistance = maxDistance;
soP->volume = 0;
soP->pan = 0;
soP->nSound = (pszSound && *pszSound) ? -1 : nSound;
if (pszSound)
	strncpy (soP->szSound, pszSound, sizeof (soP->szSound));
else
	*(soP->szSound) = '\0';
soP->nLoopStart = nLoopStart;
soP->nLoopEnd = nLoopEnd;
if (gameStates.sound.bDontStartObjects) { 		//started at level start
	soP->flags |= SOF_PERMANENT;
	soP->channel = -1;
	}
else {
	DigiGetSoundLoc (
		&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, 
		gameData.objs.viewer->nSegment, &objP->position.vPos, objP->nSegment, soP->maxVolume,
      &soP->volume, &soP->pan, soP->maxDistance);
	DigiStartSoundObject (i);
	// If it's a one-shot sound effect, and it can't start right away, then
	// just cancel it and be done with it.
	if ((soP->channel < 0) && (! (soP->flags & SOF_PLAY_FOREVER))) {
		soP->flags = 0;
		return -1;
		}
	}
return soP->nSignature;
}

//------------------------------------------------------------------------------

int DigiLinkSoundToObject2 (
	short nOrgSound, short nObject, int bForever, fix maxVolume, fix maxDistance)
{		
return DigiLinkSoundToObject3 (nOrgSound, nObject, bForever, maxVolume, maxDistance, -1, -1, NULL);
}

//------------------------------------------------------------------------------

int DigiLinkSoundToObject (short nSound, short nObject, int bForever, fix maxVolume)
{		
return DigiLinkSoundToObject2 (nSound, nObject, bForever, maxVolume, 256 * F1_0);
}

//------------------------------------------------------------------------------

int DigiLinkSoundToPos2 (
	short nOrgSound, short nSegment, short nSide, vmsVector * pos, int bForever, 
	fix maxVolume, fix maxDistance)
{

	int i, volume, pan;
	int nSound;
	tSoundObject *soP;

nSound = DigiXlatSound (nOrgSound);
if (maxVolume < 0) 
	return -1;
//	if (maxVolume > F1_0) maxVolume = F1_0;
if (nSound < 0) 
	return -1;
if (!gameData.pig.sound.sounds [gameOpts->sound.bD1Sound][nSound].data) {
	Int3 ();
	return -1;
	}
if ((nSegment < 0)|| (nSegment > gameData.segs.nLastSegment))
	return -1;
if (!bForever) { 	//&& gameData.pig.sound.sounds [nSound - SOUND_OFFSET].length < SOUND_3D_THRESHHOLD)	{
	// Hack to keep sounds from building up...
	DigiGetSoundLoc (&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, gameData.objs.viewer->nSegment, 
						  pos, nSegment, maxVolume, &volume, &pan, maxDistance);
	DigiPlaySample3D (nOrgSound, pan, volume, 0, pos, NULL);
	return -1;
	}
for (i=0, soP = SoundObjects; i < MAX_SOUND_OBJECTS; i++, soP++)
	if (soP->flags == 0)
		break;
if (i == MAX_SOUND_OBJECTS) {
	//mprintf ((1, "Too many sound gameData.objs.objects!\n"));
	return -1;
	}
soP->nSignature = gameStates.sound.digi.nNextSignature++;
soP->flags = SOF_USED | SOF_LINK_TO_POS;
if (bForever)
	soP->flags |= SOF_PLAY_FOREVER;
soP->linkType.pos.nSegment = nSegment;
soP->linkType.pos.nSide = nSide;
soP->linkType.pos.position = *pos;
soP->nSound = nSound;
soP->maxVolume = maxVolume;
soP->maxDistance = maxDistance;
soP->volume = 0;
soP->pan = 0;
soP->nLoopStart = soP->nLoopEnd = -1;
if (gameStates.sound.bDontStartObjects) {		//started at level start
	soP->flags |= SOF_PERMANENT;
	soP->channel = -1;
	}
else {
	DigiGetSoundLoc (
		&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, 
		gameData.objs.viewer->nSegment, &soP->linkType.pos.position, 
		soP->linkType.pos.nSegment, soP->maxVolume, &soP->volume, &soP->pan, soP->maxDistance);
	DigiStartSoundObject (i);
	// If it's a one-shot sound effect, and it can't start right away, then
	// just cancel it and be done with it.
	if ((soP->channel < 0) && (! (soP->flags & SOF_PLAY_FOREVER)))    {
		soP->flags = 0;
		return -1;
		}
	}
return soP->nSignature;
}

//------------------------------------------------------------------------------

int DigiLinkSoundToPos (
	short nSound, short nSegment, short nSide, vmsVector * pos, int bForever, fix maxVolume)
{
return DigiLinkSoundToPos2 (nSound, nSegment, nSide, pos, bForever, maxVolume, F1_0 * 256);
}

//------------------------------------------------------------------------------
//if nSound==-1, kill any sound
int DigiKillSoundLinkedToSegment (short nSegment, short nSide, short nSound)
{
	int i,killed;
	tSoundObject	*soP;

	if (nSound != -1)
		nSound = DigiXlatSound (nSound);


	killed = 0;

	for (i=0, soP = SoundObjects; i<MAX_SOUND_OBJECTS; i++, soP++)	{
		if ((soP->flags & (SOF_USED | SOF_LINK_TO_POS)) == (SOF_USED | SOF_LINK_TO_POS))	{
			if ((soP->linkType.pos.nSegment == nSegment) && (soP->linkType.pos.nSide==nSide) && 
				 (nSound==-1 || soP->nSound==nSound))	{
				if (soP->channel > -1)	{
					DigiStopSound (soP->channel);
					gameStates.sound.digi.nActiveObjects--;
				}
				soP->channel = -1;
				soP->flags = 0;	// Mark as dead, so some other sound can use this sound
				killed++;
			}
		}
	}
#ifdef _DEBUG
	// If this assert happens, it means that there were 2 sounds
	// that got deleted. Weird, get John.
	if (killed > 1)	{
		//mprintf ((1, "ERROR: More than 1 sounds were deleted from seg %d\n", nSegment));
	}
#endif
return (killed > 0);
}

//------------------------------------------------------------------------------

int DigiKillSoundLinkedToObject (int nObject)
{

	int i,killed;
	tSoundObject	*soP;

	killed = 0;

#ifdef NEWDEMO
	if (gameData.demo.nState == ND_STATE_RECORDING)		{
		NDRecordKillSoundLinkedToObject (nObject);
	}
#endif

	for (i = 0, soP = SoundObjects; i < MAX_SOUND_OBJECTS; i++, soP++)	{
		if ((soP->flags & (SOF_USED | SOF_LINK_TO_OBJ)) == (SOF_USED | SOF_LINK_TO_OBJ))	{
			if (soP->linkType.obj.nObject == nObject)	{
				if (soP->channel > -1)	{
					DigiStopSound (soP->channel);
					gameStates.sound.digi.nActiveObjects--;
				}
				soP->channel = -1;
				soP->flags = 0;	// Mark as dead, so some other sound can use this sound
				killed++;
			}
		}
	}
#ifdef _DEBUG
	// If this assert happens, it means that there were 2 sounds
	// that got deleted. Weird, get John.
	if (killed > 1)	{
		//mprintf ((1, "ERROR: More than 1 sounds were deleted from tObject %d\n", nObject));
	}
#endif
return (killed > 0);
}

//------------------------------------------------------------------------------
//	John's new function, 2/22/96.
void DigiRecordSoundObjects ()
{
	int i;

	for (i=0; i<MAX_SOUND_OBJECTS; i++)	{
		if ((SoundObjects [i].flags & (SOF_USED | SOF_LINK_TO_OBJ | SOF_PLAY_FOREVER))
			 == (SOF_USED | SOF_LINK_TO_OBJ | SOF_PLAY_FOREVER))
		{
			NDRecordLinkSoundToObject3 (DigiUnXlatSound (SoundObjects [i].nSound), SoundObjects [i].linkType.obj.nObject, 
			SoundObjects [i].maxVolume, SoundObjects [i].maxDistance, SoundObjects [i].nLoopStart, SoundObjects [i].nLoopEnd);
		}
	}
}

//------------------------------------------------------------------------------

void Mix_VolPan (int channel, int vol, int pan);

void DigiSyncSounds ()
{
	int i;
	int oldvolume, oldpan;
	tSoundObject *soP;

if (gameData.demo.nState == ND_STATE_RECORDING)	{
	if (!gameStates.sound.bWasRecording)
		DigiRecordSoundObjects ();
	gameStates.sound.bWasRecording = 1;
	}
else
	gameStates.sound.bWasRecording = 0;
SoundQProcess ();
for (i = 0, soP = SoundObjects; i < MAX_SOUND_OBJECTS; i++, soP++) {
	if (soP->flags & SOF_USED)	{
		oldvolume = soP->volume;
		oldpan = soP->pan;
		// Check if its done.
		if (!(soP->flags & SOF_PLAY_FOREVER) && 
			 (soP->channel > -1) && 
			 !DigiIsChannelPlaying (soP->channel)) {
			DigiEndSound (soP->channel);
			soP->flags = 0;	// Mark as dead, so some other sound can use this sound
			gameStates.sound.digi.nActiveObjects--;
			continue;		// Go on to next sound...
			}			
		if (soP->flags & SOF_LINK_TO_POS) {
			DigiGetSoundLoc (
				&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, gameData.objs.viewer->nSegment, 
				&soP->linkType.pos.position, soP->linkType.pos.nSegment, soP->maxVolume,
				&soP->volume, &soP->pan, soP->maxDistance);
#if USE_SDL_MIXER
			if (gameOpts->sound.bUseSDLMixer)
				Mix_VolPan (soP->channel, soP->volume, soP->pan);
#endif
			}
		else if (soP->flags & SOF_LINK_TO_OBJ) {
			tObject * objP;
			if (gameData.demo.nState == ND_STATE_PLAYBACK) {
				int nObject = NDFindObject (soP->linkType.obj.nObjSig);
				objP = gameData.objs.objects + ((nObject > -1) ? nObject : 0);
				}
			else 
				objP = gameData.objs.objects + soP->linkType.obj.nObject;
			if ((objP->nType == OBJ_NONE) || (objP->nSignature != soP->linkType.obj.nObjSig)) {
			// The tObject that this is linked to is dead, so just end this sound if it is looping.
				if (soP->channel > -1)	{
					if (soP->flags & SOF_PLAY_FOREVER)
						DigiStopSound (soP->channel);
					else
						DigiEndSound (soP->channel);
					gameStates.sound.digi.nActiveObjects--;
					}
				soP->flags = 0;	// Mark as dead, so some other sound can use this sound
				continue;		// Go on to next sound...
				}
			else {
				DigiGetSoundLoc (
					&gameData.objs.viewer->position.mOrient, &gameData.objs.viewer->position.vPos, 
					gameData.objs.viewer->nSegment, &objP->position.vPos, objP->nSegment, soP->maxVolume,
					&soP->volume, &soP->pan, soP->maxDistance);
#if USE_SDL_MIXER
				if (gameOpts->sound.bUseSDLMixer)
					Mix_VolPan (soP->channel, soP->volume, soP->pan);
#endif
				}
			}
		if (oldvolume != soP->volume) {
			if (soP->volume < 1)	{
			// Sound is too far away, so stop it from playing.
				if (soP->channel > -1)	{
					if (soP->flags & SOF_PLAY_FOREVER)
						DigiStopSound (soP->channel);
					else
						DigiEndSound (soP->channel);
					gameStates.sound.digi.nActiveObjects--;
					soP->channel = -1;
					}
				if (!(soP->flags & SOF_PLAY_FOREVER)) {
					soP->flags = 0;	// Mark as dead, so some other sound can use this sound
					continue;
					}
				}
			else {
				if (soP->channel < 0)
					DigiStartSoundObject (i);
				else
					DigiSetChannelVolume (soP->channel, soP->volume);
				}
			}
		if ((oldpan != soP->pan) && (soP->channel > -1))
			DigiSetChannelPan (soP->channel, soP->pan);
	}
}
	
//------------------------------------------------------------------------------

#ifdef _DEBUG
//	DigiSoundDebug ();
#endif
}

void DigiPauseDigiSounds ()
{
	int i;

DigiPauseLoopingSound ();
for (i = 0; i < MAX_SOUND_OBJECTS; i++)	{
	if ((SoundObjects [i].flags & SOF_USED) && (SoundObjects [i].channel > -1)) {
		DigiStopSound (SoundObjects [i].channel);
		if (!(SoundObjects [i].flags & SOF_PLAY_FOREVER))
			SoundObjects [i].flags = 0;	// Mark as dead, so some other sound can use this sound
		gameStates.sound.digi.nActiveObjects--;
		SoundObjects [i].channel = -1;
		}			
	}
DigiStopAllChannels ();
SoundQPause ();
}

//------------------------------------------------------------------------------

void DigiPauseAll ()
{
DigiPauseMidi ();
DigiPauseDigiSounds ();
}

//------------------------------------------------------------------------------

void DigiResumeDigiSounds ()
{
DigiSyncSounds ();	//don't think we really need to do this, but can't hurt
DigiResumeLoopingSound ();
}

//------------------------------------------------------------------------------

extern void DigiResumeMidi ();

void DigiResumeAll ()
{
DigiResumeMidi ();
DigiResumeLoopingSound ();
}

//------------------------------------------------------------------------------
// Called by the code in digi.c when another sound takes this sound tObject's
// slot because the sound was done playing.
void DigiEndSoundObj (int i)
{
Assert (SoundObjects [i].flags & SOF_USED);
Assert (SoundObjects [i].channel > -1);
gameStates.sound.digi.nActiveObjects--;
SoundObjects [i].channel = -1;
}

//------------------------------------------------------------------------------

void DigiStopDigiSounds ()
{
	int i;

DigiStopLoopingSound ();
for (i = 0; i < MAX_SOUND_OBJECTS; i++) {
	if (SoundObjects [i].flags & SOF_USED)	{
		if (SoundObjects [i].channel > -1)	{
			DigiStopSound (SoundObjects [i].channel);
			gameStates.sound.digi.nActiveObjects--;
			}
		SoundObjects [i].flags = 0;	// Mark as dead, so some other sound can use this sound
		}
	}
DigiStopAllChannels ();
SoundQInit ();
}

//------------------------------------------------------------------------------

void DigiStopAll ()
{
DigiStopCurrentSong ();
DigiStopDigiSounds ();
}

//------------------------------------------------------------------------------

#ifdef _DEBUG
int VerifySoundChannelFree (int channel)
{
	int i;

for (i = 0; i < MAX_SOUND_OBJECTS; i++) {
	if (SoundObjects [i].flags & SOF_USED)	{
		if (SoundObjects [i].channel == channel)	{
			//mprintf ((0, "ERROR STARTING SOUND CHANNEL ON USED SLOT!!\n"));
			Int3 ();	// Get John!
			}
		}
	}
return 0;
}

//------------------------------------------------------------------------------

void DigiSoundDebug ()
{
	int i;
	int n_activeSound_objs=0;
	int nSound_objs=0;

for (i = 0; i < MAX_SOUND_OBJECTS; i++)	{
	if (SoundObjects [i].flags & SOF_USED) {
		nSound_objs++;
		if (SoundObjects [i].channel > -1)
			n_activeSound_objs++;
		}
	}
DigiDebug ();
}
#endif

//------------------------------------------------------------------------------

tSoundQueue soundQueue;

void SoundQInit ()
{
soundQueue.nHead = 
soundQueue.nTail = 
soundQueue.nSounds = 0;
soundQueue.nChannel = -1;
}

//------------------------------------------------------------------------------

void SoundQPause ()
{	
soundQueue.nChannel = -1;
}

//------------------------------------------------------------------------------

void SoundQEnd ()
{
	// Current playing sound is stopped, so take it off the Queue
if (++soundQueue.nHead >= MAX_SOUND_QUEUE) 
	soundQueue.nHead = 0;
soundQueue.nSounds--;
soundQueue.nChannel = -1;
}

//------------------------------------------------------------------------------

void SoundQProcess ()
{	
	fix curtime = TimerGetApproxSeconds ();
	tSoundQueueEntry *q;

if (soundQueue.nChannel > -1)	{
	if (DigiIsChannelPlaying (soundQueue.nChannel))
		return;
	SoundQEnd ();
	}
while (soundQueue.nHead != soundQueue.nTail)	{
	q = soundQueue.queue + soundQueue.nHead;
	if (q->time_added + MAX_LIFE > curtime) {
		soundQueue.nChannel = DigiStartSound (q->nSound, F1_0+1, 0xFFFF/2, 0, -1, -1, -1, F1_0, NULL, NULL);
		return;
		}
	SoundQEnd ();
	}
}

//------------------------------------------------------------------------------

void DigiStartSoundQueued (short nSound, fix volume)
{
	int					i;
	tSoundQueueEntry *q;

nSound = DigiXlatSound (nSound);
if (nSound < 0) 
	return;
i = soundQueue.nTail+1;
if (i>=MAX_SOUND_QUEUE) 
	i = 0;
// Make sure its loud so it doesn't get cancelled!
if (volume < F1_0 + 1)
	volume = F1_0 + 1;
if (i != soundQueue.nHead)	{
	q = soundQueue.queue + soundQueue.nTail;
	q->time_added = TimerGetApproxSeconds ();
	q->nSound = nSound;
	soundQueue.nSounds++;
	soundQueue.nTail = i;
	}
else {
	//mprintf ((0, "Sound Q full!\n"));
}
// Try to start it!
SoundQProcess ();
}

//------------------------------------------------------------------------------

int DigiGetSoundByName (char *pszSound)
{
	char	szSound [FILENAME_LEN];
	int	nSound;
	
strcpy (szSound, pszSound);
nSound = PiggyFindSound (szSound);
return (nSound == 255) ? -1 : DigiUnXlatSound (nSound);
}

//------------------------------------------------------------------------------

int DigiSetObjectSound (int nObject, int nSound, char *pszSound)
{
return (nObject < 0) ? -1 : DigiLinkSoundToObject3 (nSound, nObject, 1, F1_0, i2f (256), -1, -1, pszSound);
}

//------------------------------------------------------------------------------
//eof
