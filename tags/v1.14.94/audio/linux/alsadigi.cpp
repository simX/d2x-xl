/* $Id: alsadigi.c,v 1.2 2003/03/13 00:20:21 btb Exp $ */
/*
 *
 * ALSA digital audio support
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#if !(defined (USE_SDL_MIXER) && USE_SDL_MIXER)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/asoundlib.h>
#include <pthread.h>

#include "error.h"
#include "mono.h"
#include "fix.h"
#include "vecmat.h"
#include "gr.h" // needed for piggy.h
#include "piggy.h"
#include "digi.h"
#include "sounds.h"
#include "wall.h"
#include "newdemo.h"
#include "kconfig.h"

//edited 05/17/99 Matt Mueller - added ifndef NO_ASM
//added on 980905 by adb to add inline FixMul for mixer on i386
#ifndef NO_ASM
#ifdef __i386__
#define do_fixmul(x,y)				\
({						\
	int _ax, _dx;				\
	asm("imull %2\n\tshrdl %3,%1,%0"	\
	    : "=a"(_ax), "=d"(_dx)		\
	    : "rm"(y), "i"(16), "0"(x);	\
	_ax;					\
})
extern inline fix FixMul(fix x, fix y) { return do_fixmul(x,y); }
#endif
#endif
//end edit by adb
//end edit -MM

//changed on 980905 by adb to increase number of concurrent sounds
#define MAX_SOUND_SLOTS 32
//end changes by adb
#define SOUND_BUFFER_SIZE 512

#define MIN_VOLUME 10

/* This table is used to add two sound values together and pin
 * the value to avoid overflow.  (used with permission from ARDI)
 * DPH: Taken from SDL/src/SDL_mixer.c.
 */
static const ubyte mix8[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
  0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
  0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
  0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x
, 0x43, 0x44, 0x45,
  0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
  0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B,
  0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
  0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C,
  0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92,
  0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D,
  0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
  0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
  0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE,
  0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
  0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
  0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
  0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5,
  0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

#define SOF_USED			1		// Set if this sample is used
#define SOF_PLAYING			2		// Set if this sample is playing on a channel
#define SOF_LINK_TO_OBJ		4		// Sound is linked to a moving CObject. If CObject dies, then finishes play and quits.
#define SOF_LINK_TO_POS		8		// Sound is linked to tSegment, pos
#define SOF_PLAY_FOREVER	16		// Play forever (or until level is stopped), otherwise plays once

typedef struct tSoundObject {
	short		nSignature;		// A unique nSignature to this sound
	ubyte		flags;			// Used to tell if this slot is used and/or currently playing, and how long.
	fix		maxVolume;		// Max volume that this sound is playing at
	fix		maxDistance;	        // The max distance that this sound can be heard at...
	int		volume;			// Volume that this sound is playing at
	int 		pan;			// Pan value that this sound is playing at
	int		handle; 		// What handle this sound is playing on.  Valid only if SOF_PLAYING is set.
	short		nSound;		// The sound number that is playing
	union {
		struct {
			short		nSegment; 			// Used if SOF_LINK_TO_POS field is used
			short		nSide;
			CFixVector	position;
		}pos;
		struct {
			short		 nObject;			 // Used if SOF_LINK_TO_OBJ field is used
			short		 objsignature;
		}obj;
	}link;
} tSoundObject;
#define lp_segnum link.position.vPos.nSegment
#define lp_sidenum link.position.vPos.nSide
#define lp_position link.position.vPos.position.vPosition

#define lo_objnum link.obj.nObject
#define lo_objsignature link.obj.objsignature

#define MAX_SOUND_OBJECTS 16
tSoundObject SoundObjects[MAX_SOUND_OBJECTS];
short gameStates.sound.digi.nNextSignature=0;

//added/changed on 980905 by adb to make sfx volume work, on 990221 by adb changed I2X (1) to I2X (1) / 2
#define SOUND_MAX_VOLUME (I2X (1) / 2)

int gameStates.sound.digi.nVolume = SOUND_MAX_VOLUME;
//end edit by adb

int gameStates.sound.digi.bLoMem = 0;

static int gameStates.sound.digi.bInitialized = 0;

struct sound_slot {
 int nSound;
 int playing;   // Is there a sample playing on this channel?
 int looped;    // Play this sample looped?
 fix pan;       // 0 = far left, 1 = far right
 fix volume;    // 0 = nothing, 1 = fully on
 //changed on 980905 by adb from char * to ubyte * 
 ubyte *samples;
 //end changes by adb
 uint length; // Length of the sample
 uint position; // Position we are at at the moment.
} SoundSlots[MAX_SOUND_SLOTS];

static int gameStates.sound.digi.bSoundsInitialized = 0;

//added on 980905 by adb to add rotating/volume based sound kill system
static int gameStates.sound.digi.nMaxChannels = 16;
static int next_handle = 0;
int SampleHandles[32];
void resetSounds_on_channel(int channel);
//end edit by adb

/* Threading/ALSA stuff */
#define LOCK() pthread_mutex_lock(&mutex)
#define UNLOCK() pthread_mutex_unlock(&mutex)
void *snd_devhandle;
pthread_t thread_id;
pthread_mutex_t mutex;


void digi_reset_digiSounds(void);


/* Audio mixing callback */
//changed on 980905 by adb to cleanup, add pan support and optimize mixer
static void AudioMixCallback(void *userdata, ubyte *stream, int len)
{
 ubyte *streamend = stream + len;
 struct sound_slot *sl;

 for (sl = SoundSlots; sl < SoundSlots + MAX_SOUND_SLOTS; sl++)
 {
  if (sl->playing)
  {
   ubyte *sldata = sl->samples + sl->position.vPosition, *slend = sl->samples + sl->length;
   ubyte *sp = stream;
   signed char v;
   fix vl, vr;
   int x;

   if ((x = sl->pan) & 0x8000)
   {
    vl = 0x20000 - x * 2;
    vr = 0x10000;
   }
   else
   {
    vl = 0x10000;
    vr = x * 2;
   }
   vl = FixMul(vl, (x = sl->volume);
   vr = FixMul(vr, x);
   while (sp < streamend) 
   {
    if (sldata == slend)
    {
     if (!sl->looped)
     {
      sl->playing = 0;
      break;
     }
     sldata = sl->samples;
    }
    v = *(sldata++) - 0x80;
    *(sp++) = mix8 [*sp + FixMul(v, vl) + 0x80];
    *(sp++) = mix8 [*sp + FixMul(v, vr) + 0x80];
   }
   sl->position.vPosition = sldata - sl->samples;
  }
 }
}
//end changes by adb

void *MixerThread (void *data) 
{
	ubyte buffer [512];
 /* Allow ourselves to be asynchronously cancelled */
 pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
 while (1) {
//   //printf("i=%d\n",i++);
   memset(buffer, 0x80, 512);
   LOCK();
   AudioMixCallback(NULL,buffer,512);
   UNLOCK();
   snd_pcm_write(snd_devhandle, buffer, 512);
 } 
 return 0;
}


/* Initialise audio devices. */
int DigiInit()
{
 int card=0, device=0, err;
 snd_pcm_format_t format;
 snd_pcm_playback_params_t params;
 pthread_attr_t attr;
 pthread_mutexattr_t mutexattr;

 //added on 980905 by adb to init sound kill system
 memset(SampleHandles, 255, sizeof(SampleHandles);
 //end edit by adb

 /* Open the ALSA sound device */
 if ((err = snd_pcm_open(&snd_devhandle, card, device, 
     SND_PCM_OPEN_PLAYBACK)) < 0) {  
     fprintf(stderr, "open failed: %s\n", snd_strerror(err);  
     return -1; 
 } 

 memset(&format, 0, sizeof(format);
 format.format = SND_PCM_SFMT_U8;
 format.rate = 11025;
 format.channels = 2;
 if ((err = snd_pcm_playback_format(snd_devhandle, &format)) < 0) { 
    fprintf(stderr, "format setup failed: %s\n", snd_strerror(err);
    snd_pcm_close(snd_devhandle); 
    return -1; 
 } 

 memset(&params, 0, sizeof(params);
 params.fragment_size=512;
 params.fragments_max=2;
 params.fragments_room=1;
 if ((err = snd_pcm_playback_params(snd_devhandle, &params)) < 0) { 
    fprintf(stderr, "params setup failed: %s\n", snd_strerror(err);
    snd_pcm_close(snd_devhandle); 
    return -1; 
 }

 /* Start the mixer thread */

 /* We really should check the results of these */
 pthread_mutexattr_init(&mutexattr);
 pthread_mutex_init(&mutex,&mutexattr);
 pthread_mutexattr_destroy(&mutexattr);
 
 if (pthread_attr_init(&attr) != 0) {
  fprintf(stderr, "failed to init attr\n");
  snd_pcm_close(snd_devhandle); 
  return -1;
 }

 pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

 pthread_create(&thread_id,&attr,MixerThread,NULL);
 pthread_attr_destroy(&attr);

 //atexit(DigiClose);
 gameStates.sound.digi.bInitialized = 1;
 return 0;
}

/* Toggle audio */
void DigiReset() { }

/* Shut down audio */
void DigiClose()
{
if (!gameStates.sound.digi.bInitialized) 
 	return;
PrintLog ("shutting down sound system\n");
gameStates.sound.digi.bInitialized = 0;
snd_pcm_close(snd_devhandle);
pthread_mutex_destroy(&mutex);
pthread_cancel(thread_id);
}

/* Find the sound which actually equates to a sound number */
int DigiXlatSound(int nSound)
{
	if (nSound < 0) return -1;

	if (gameStates.sound.digi.bLoMem)	{
		nSound = AltSounds[nSound];
		if (nSound == 255) return -1;
	}
	return Sounds [gameStates.sound.bD1Sound][nSound];
}

static int get_free_slot()
{
 int i;
 for (i=0; i<MAX_SOUND_SLOTS; i++)
 {
  if (!SoundSlots[i].playing) return i;
 }
 return -1;
}

int DigiStartSound(int nSound, fix volume, fix pan)
{
 int ntries;
 int slot;

 if (!gameStates.sound.digi.bInitialized) return -1;
 LOCK();
 //added on 980905 by adb from original source to add sound kill system
 // play at most digi_max_channel samples, if possible kill sample with low volume
 ntries = 0;

TryNextChannel:
 if ((SampleHandles[next_handle] >= 0) && (SoundSlots[SampleHandles[next_handle]].playing))
 {
  if ((SoundSlots[SampleHandles[next_handle]].volume > gameStates.sound.digi.nVolume) && (ntries<gameStates.sound.digi.nMaxChannels))
  {
   //con_printf (0, "Not stopping loud sound %d.\n", next_handle);
   next_handle++;
   if (next_handle >= gameStates.sound.digi.nMaxChannels)
    next_handle = 0;
   ntries++;
   goto TryNextChannel;
  }
  //con_printf (0, "[SS:%d]", next_handle);
  SoundSlots[SampleHandles[next_handle]].playing = 0;
  SampleHandles[next_handle] = -1;
 }
 //end edit by adb

 slot = get_free_slot();
 if (slot<0) return -1;

 SoundSlots[slot].nSound = nSound;
 SoundSlots[slot].samples = gameData.pig.sound.sounds[nSound].data;
 SoundSlots[slot].length = gameData.pig.sound.sounds[nSound].length;
 SoundSlots[slot].volume = FixMul(gameStates.sound.digi.nVolume, volume);
 SoundSlots[slot].pan = pan;
 SoundSlots[slot].info.position.vPosition = 0;
 SoundSlots[slot].looped = 0;
 SoundSlots[slot].playing = 1;

 //added on 980905 by adb to add sound kill system from original sos digi.c
 resetSounds_on_channel(slot);
 SampleHandles[next_handle] = slot;
 next_handle++;
 if (next_handle >= gameStates.sound.digi.nMaxChannels)
  next_handle = 0;
 //end edit by adb
 UNLOCK();
 return slot;
}

 //added on 980905 by adb to add sound kill system from original sos digi.c
void resetSounds_on_channel(int channel)
{
 int i;

 for (i=0; i<gameStates.sound.digi.nMaxChannels; i++)
  if (SampleHandles[i] == channel)
   SampleHandles[i] = -1;
}

//------------------------------------------------------------------------------
//end edit by adb

int DigiStartSoundObject (int obj)
{
 int slot;

 if (!gameStates.sound.digi.bInitialized) return -1;
 LOCK();
 slot = get_free_slot();

 if (slot<0) { UNLOCK(); return -1; }


 SoundSlots[slot].nSound = SoundObjects[obj].nSound;
 SoundSlots[slot].samples = gameData.pig.sound.sounds[SoundObjects[obj].nSound].data;
 SoundSlots[slot].length = gameData.pig.sound.sounds[SoundObjects[obj].nSound].length;
 SoundSlots[slot].volume = FixMul(gameStates.sound.digi.nVolume, SoundObjects[obj].volume);
 SoundSlots[slot].pan = SoundObjects[obj].pan;
 SoundSlots[slot].info.position.vPosition = 0;
 SoundSlots[slot].looped = (SoundObjects[obj].flags & SOF_PLAY_FOREVER);
 SoundSlots[slot].playing = 1;

 SoundObjects[obj].nSignature = gameStates.sound.digi.nNextSignature++;
 SoundObjects[obj].handle = slot;

 SoundObjects[obj].flags |= SOF_PLAYING;
 //added on 980905 by adb to add sound kill system from original sos digi.c
 resetSounds_on_channel(slot);
 //end edit by adb
 
 UNLOCK();
 return 0;
}


// Play the given sound number.
// Volume is max at I2X (1).
void DigiPlaySample(int nSound, fix maxVolume)
{
#ifdef NEWDEMO
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordSound(nSound);
#endif
	nSound = DigiXlatSound(nSound);

	if (!gameStates.sound.digi.bInitialized) return;

	if (nSound < 0) return;

	DigiStartSound(nSound, maxVolume, (I2X (1) / 2));
}

//------------------------------------------------------------------------------
// Play the given sound number. If the sound is already playing,
// restart it.
void CAudio::PlaySampleOnce (int nSound, fix maxVolume)
{
	int i;

#ifdef NEWDEMO
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordSound(nSound);
#endif
nSound = DigiXlatSound(nSound);
if (!gameStates.sound.digi.bInitialized) 
	return;
if (nSound < 0) 
	return;
LOCK();
for (i=0; i < MAX_SOUND_SLOTS; i++)
	if (SoundSlots [i].nSound == nSound)
		SoundSlots [i].playing = 0;
UNLOCK();
StartSound (nSound, maxVolume, (I2X (1) / 2));
}

void DigiPlaySample (int nSound, int volume, int angle, int no_dups) // Volume from 0-0x7fff
{
	no_dups = 1;

#ifdef NEWDEMO
	if (gameData.demo.nState == ND_STATE_RECORDING)		{
		if (no_dups)
			NDRecordSound3DOnce(nSound, angle, volume);
		else
			NDRecordSound3D(nSound, angle, volume);
	}
#endif
	nSound = DigiXlatSound(nSound);

	if (!gameStates.sound.digi.bInitialized) return;
	if (nSound < 0) return;

	if (volume < MIN_VOLUME) return;
	DigiStartSound(nSound, volume, angle);
}

//------------------------------------------------------------------------------

void CAudio::GetVolPan (CFixMatrix * listener, CFixVector * vListenerPos, int nListenerSeg, CFixVector * vSoundPos, 
						 		int nSoundSeg, fix maxVolume, int *volume, int *pan, fix maxDistance, int nDecay)
{	  
	CFixVector	vector_toSound;
	fix angle_from_ear, cosang,sinang;
	fix distance;
	fix pathDistance;

	*volume = 0;
	*pan = 0;

	maxDistance = (maxDistance*5)/4;		// Make all sounds travel 1.25 times as far.

	//	Warning: Made the VmVecNormalizedDir be VmVecNormalizedDir and got illegal values to acos in the fang computation.
	distance = VmVecNormalizedDir(&vector_toSound, vSoundPos, vListenerPos);
	
	if (distance < maxDistance)	{
		int nSearchSegs = X2I(maxDistance/20);
		if (nSearchSegs < 1) 
			nSearchSegs = 1;

		pathDistance = FindConnectedDistance (vListenerPos, nListenerSeg, vSoundPos, nSoundSeg, nSearchSegs, WID_RENDPAST_FLAG);
		if (pathDistance > -1)	{
			if (nDecay)
				*volume = I2X (sqrt ((double) pathDistance / double (maxDistance)));
			else
				*volume = maxVolume - FixDiv (pathDistance, maxDistance);
			//con_printf (CONDBG, "Sound path distance %.2f, volume is %d / %d\n", X2F(distance), *volume, maxVolume);
			if (*volume <= 0)
				*volume = 0;
			else {
				angle_from_ear = VmVecDeltaAngNorm(&listener->rVec,&vector_toSound,&listener->uVec);
				FixSinCos(angle_from_ear,&sinang,&cosang);
				//con_printf (CONDBG, "volume is %.2f\n", X2F(*volume));
				if (gameConfig.bReverseChannels || gameOpts->sound.bHires) 
					cosang = -cosang;
				*pan = (cosang + I2X (1))/2;
				}
			}
		}
	}																					  
}

//------------------------------------------------------------------------------

int CreateObjectSound (int nOrgSound, short nObject, int forever, fix maxVolume, fix  maxDistance)
{
	int i,volume,pan;
	CObject * objP;
	int nSound;

	nSound = DigiXlatSound(nOrgSound);

	if (maxVolume < 0) 
		return -1;
	if (!gameStates.sound.digi.bInitialized) 
		return -1;
	if (nSound < 0) 
		return -1;
	if (!gameData.pig.sound.sounds[nSound].data) {
		Int3();
		return -1;
		}
	if ((nObject<0)||(nObject>gameData.objs.nLastObject [0]))
		return -1;

	if (!forever)	{
		// Hack to keep sounds from building up...
		DigiGetSoundLoc(&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
							 &OBJECTS[nObject].info.position.vPos, OBJECTS[nObject].nSegment, maxVolume,&volume, &pan, 
							 maxDistance, 0);
		DigiPlaySample (nOrgSound, volume, pan);
		return -1;
		}

      for (i=0; i<MAX_SOUND_OBJECTS; i++)
        	if (SoundObjects[i].flags==0)
	           break;

	if (i==MAX_SOUND_OBJECTS) {
		con_printf (1, "Too many sound OBJECTS!\n");
		return -1;
	}

	SoundObjects[i].nSignature=gameStates.sound.digi.nNextSignature++;
	SoundObjects[i].flags = SOF_USED | SOF_LINK_TO_OBJ;
	if (forever)
		SoundObjects[i].flags |= SOF_PLAY_FOREVER;
	SoundObjects[i].lo_objnum = nObject;
	SoundObjects[i].lo_objsignature = OBJECTS[nObject].nSignature;
	SoundObjects[i].maxVolume = maxVolume;
	SoundObjects[i].maxDistance = maxDistance;
	SoundObjects[i].volume = 0;
	SoundObjects[i].pan = 0;
	SoundObjects[i].nDecay = 0;
	SoundObjects[i].nSound = nSound;

	objP = &OBJECTS[SoundObjects[i].lo_objnum];
	DigiGetSoundLoc(&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
                       &objP->info.position.vPos, objP->info.nSegment, maxVolume,
                       &SoundObjects[i].volume, &SoundObjects[i].pan, maxDistance, 0);

	if (!forever || SoundObjects[i].volume >= MIN_VOLUME)
	       DigiStartSoundObject(i);

	return SoundObjects[i].nSignature;
}

//------------------------------------------------------------------------------

int CAudio::CreateSegmentSound (int nOrgSound, int nSoundClass, short nSegment, short nSide, CFixVector * pos, int forever, fix maxVolume, fix maxDistance, int nDecay)
{
	int i, volume, pan;
	int nSound;

	nSound = DigiXlatSound(nOrgSound);

if (maxVolume < 0) 
	return -1;
if (!gameStates.sound.digi.bInitialized) 
	return -1;
if (nSound < 0) 
	return -1;
if (!gameData.pig.sound.sounds [nSound].data)
	return -1;
if ((nSegment < 0) || (nSegment > gameData.segs.nLastSegment))
	return -1;
if (!forever)	{
	// Hack to keep sounds from building up...
	audio.GetVolPan (&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
						  pos, nSegment, maxVolume, &volume, &pan, maxDistance, nDecay);
	audio.PlaySound (nOrgSound, SOUNDCLASS_GENERIC, volume, pan);
	return -1;
	}

for (i = 0; i < MAX_SOUND_OBJECTS; i++)
	if (SoundObjects [i].flags == 0)
		break;

if (i == MAX_SOUND_OBJECTS) {
	con_printf (1, "Too many sound objects!\n");
	return -1;
	}


SoundObjects[i].nSignature=gameStates.sound.digi.nNextSignature++;
SoundObjects[i].flags = SOF_USED | SOF_LINK_TO_POS;
if (forever)
	SoundObjects[i].flags |= SOF_PLAY_FOREVER;
SoundObjects[i].lp_segnum = nSegment;
SoundObjects[i].lp_sidenum = nSide;
SoundObjects[i].lp_position = *pos;
SoundObjects[i].nSound = nSound;
SoundObjects[i].maxVolume = maxVolume;
SoundObjects[i].maxDistance = maxDistance;
SoundObjects[i].volume = 0;
SoundObjects[i].pan = 0;
SoundObjects[i].nDecay = nDecay;
GetVolPan (&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
			  &SoundObjects[i].lp_position, lp_segnum,
			  maxVolume, &SoundObjects[i].volume, &SoundObjects[i].pan, maxDistance, nDecay);

if (!forever || SoundObjects[i].volume >= MIN_VOLUME)
	StartSoundObject(i);
return SoundObjects[i].nSignature;
}

void CAudio::DestroySegmentSound (int nSegment, int nSide, int nSound)
{
	int i,killed;

	nSound = DigiXlatSound(nSound);

	if (!gameStates.sound.digi.bInitialized) 
		return;

	killed = 0;

	for (i=0; i<MAX_SOUND_OBJECTS; i++)	{
		if ((SoundObjects[i].flags & SOF_USED) && (SoundObjects[i].flags & SOF_LINK_TO_POS))	{
			if ((SoundObjects[i].lp_segnum == nSegment) && (SoundObjects[i].nSound==nSound) && (SoundObjects[i].lp_sidenum==nSide)) {
				if (SoundObjects[i].flags & SOF_PLAYING)	{
					LOCK();
				        SoundSlots[SoundObjects[i].handle].playing = 0;
					UNLOCK();
				}
				SoundObjects[i].flags = 0;	// Mark as dead, so some other sound can use this sound
				killed++;
			}
		}
	}
	// If this assert happens, it means that there were 2 sounds
	// that got deleted. Weird, get John.
	if (killed > 1)	{
		con_printf (1, "ERROR: More than 1 sounds were deleted from seg %d\n", nSegment);
	}
}

void CAudio::DestroyObjectSound (int nObject)
{
	int i,killed;

	if (!gameStates.sound.digi.bInitialized) return;

	killed = 0;

	for (i=0; i<MAX_SOUND_OBJECTS; i++)	{
		if ((SoundObjects[i].flags & SOF_USED) && (SoundObjects[i].flags & SOF_LINK_TO_OBJ))	{
			if (SoundObjects[i].lo_objnum == nObject)   {
				if (SoundObjects[i].flags & SOF_PLAYING)	{
				     LOCK();
                                     SoundSlots[SoundObjects[i].handle].playing = 0;
				     UNLOCK();
				}
				SoundObjects[i].flags = 0;	// Mark as dead, so some other sound can use this sound
				killed++;
			}
		}
	}
	// If this assert happens, it means that there were 2 sounds
	// that got deleted. Weird, get John.
	if (killed > 1)	{
		con_printf (1, "ERROR: More than 1 sounds were deleted from CObject %d\n", nObject);
	}
}

void CAudio::SyncSounds (void)
{
	int i;
	int oldvolume, oldpan;

	if (!gameStates.sound.digi.bInitialized) return;

	for (i=0; i<MAX_SOUND_OBJECTS; i++)	{
		if (SoundObjects[i].flags & SOF_USED)	{
			oldvolume = SoundObjects[i].volume;
			oldpan = SoundObjects[i].pan;

			if (!(SoundObjects[i].flags & SOF_PLAY_FOREVER))	{
			 	// Check if its done.
				if (SoundObjects[i].flags & SOF_PLAYING) {
					LOCK();
					if (!SoundSlots[SoundObjects[i].handle].playing) {
						UNLOCK();
						SoundObjects[i].flags = 0;	// Mark as dead, so some other sound can use this sound
						continue;		// Go on to next sound...
					}
					UNLOCK();
				}
			}		
	
			if (SoundObjects[i].flags & SOF_LINK_TO_POS)	{
				DigiGetSoundLoc(&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
								&SoundObjects[i].lp_position, SoundObjects[i].lp_segnum,
								SoundObjects[i].maxVolume,
                        &SoundObjects[i].volume, &SoundObjects[i].pan, SoundObjects[i].maxDistance, SoundObjects [i].nDecay);

			} else if (SoundObjects[i].flags & SOF_LINK_TO_OBJ)	{
				CObject * objP;

				objP = &OBJECTS[SoundObjects[i].lo_objnum];
	
				if ((objP->info.nType==OBJ_NONE) || (objP->info.nSignature!=SoundObjects[i].lo_objsignature))  {
					// The CObject that this is linked to is dead, so just end this sound if it is looping.
					if ((SoundObjects[i].flags & SOF_PLAYING)  && (SoundObjects[i].flags & SOF_PLAY_FOREVER))	{
					     LOCK();
					     SoundSlots[SoundObjects[i].handle].playing = 0;
					     UNLOCK();
					}
					SoundObjects[i].flags = 0;	// Mark as dead, so some other sound can use this sound
					continue;		// Go on to next sound...
				} else {
					DigiGetSoundLoc(&gameData.objs.viewerP->info.position.mOrient, &gameData.objs.viewerP->info.position.vPos, gameData.objs.viewerP->info.nSegment, 
	                                &objP->info.position.vPos, objP->info.nSegment, SoundObjects[i].maxVolume,
                                   &SoundObjects[i].volume, &SoundObjects[i].pan, SoundObjects[i].maxDistance, SoundObjects[i].nDecay);
				}
			}
			 
			if (oldvolume != SoundObjects[i].volume) 	{
				if (SoundObjects[i].volume < MIN_VOLUME)	 {
					// Sound is too far away, so stop it from playing.
					if ((SoundObjects[i].flags & SOF_PLAYING)&&(SoundObjects[i].flags & SOF_PLAY_FOREVER))	{
						LOCK();
                                        	SoundSlots[SoundObjects[i].handle].playing = 0;
						UNLOCK();
						SoundObjects[i].flags &= ~SOF_PLAYING;		// Mark sound as not playing
					}
				} else {
					if (!(SoundObjects[i].flags & SOF_PLAYING))	{
						DigiStartSoundObject(i);
					} else {
						LOCK();
					        SoundSlots[SoundObjects[i].handle].volume = FixMulDiv(SoundObjects[i].volume,gameStates.sound.digi.nVolume,I2X (1));
						UNLOCK();
					}
				}
			}
			
			if (oldpan != SoundObjects[i].pan) 	{
				if (SoundObjects[i].flags & SOF_PLAYING) {
					LOCK();
                                        SoundSlots[SoundObjects[i].handle].pan = SoundObjects[i].pan;
					UNLOCK();
				}
			}
		}
	}
}

void DigiInitSounds()
{
	int i;

	if (!gameStates.sound.digi.bInitialized) return;

	digi_reset_digiSounds();

	for (i=0; i<MAX_SOUND_OBJECTS; i++)	{
		if (gameStates.sound.digi.bSoundsInitialized) {
			if (SoundObjects[i].flags & SOF_PLAYING)	{
				LOCK();
			        SoundSlots[SoundObjects[i].handle].playing=0;
				UNLOCK();
			}
		}
		SoundObjects[i].flags = 0;	// Mark as dead, so some other sound can use this sound
	}
	gameStates.sound.digi.bSoundsInitialized = 1;
}

//added on 980905 by adb from original source to make sfx volume work
void DigiSetFxVolume(int dvolume)
{
	dvolume = FixMulDiv(dvolume, SOUND_MAX_VOLUME, 0x7fff);
	if (dvolume > SOUND_MAX_VOLUME)
		gameStates.sound.digi.nVolume = SOUND_MAX_VOLUME;
	else if (dvolume < 0)
		gameStates.sound.digi.nVolume = 0;
	else
		gameStates.sound.digi.nVolume = dvolume;

	if (!gameStates.sound.digi.bInitialized) return;

	DigiSyncSounds();
}
//end edit by adb

void DigiMidiVolume(int dvolume, int mvolume) { }

int DigiIsSoundPlaying(int nSound)
{
	int i;

	nSound = DigiXlatSound(nSound);

	LOCK();
	for (i = 0; i < MAX_SOUND_SLOTS; i++)
		  //changed on 980905 by adb: added SoundSlots[i].playing &&
		  if (SoundSlots[i].playing && SoundSlots[i].nSound == nSound)
		  //end changes by adb
		  { UNLOCK();	return 1; }
	UNLOCK();
	return 0;
}


void DigiPauseAll() { }
void DigiResumeAll() { }
void DigiStopAll() { }

 //added on 980905 by adb to make sound channel setting work
void DigiSetMaxChannels(int n) { 
	gameStates.sound.digi.nMaxChannels	= n;

	if (gameStates.sound.digi.nMaxChannels < 1) 
		gameStates.sound.digi.nMaxChannels = 1;
	if (gameStates.sound.digi.nMaxChannels > (MAX_SOUND_SLOTS-MAX_SOUND_OBJECTS)) 
		gameStates.sound.digi.nMaxChannels = (MAX_SOUND_SLOTS-MAX_SOUND_OBJECTS);

	if (!gameStates.sound.digi.bInitialized) return;

	digi_reset_digiSounds();
}

int DigiGetMaxChannels() { 
	return gameStates.sound.digi.nMaxChannels; 
}
// end edit by adb

void digi_reset_digiSounds() {
 int i;

 LOCK();
 for (i=0; i< MAX_SOUND_SLOTS; i++)
  SoundSlots[i].playing=0;
 UNLOCK();
 
 //added on 980905 by adb to reset sound kill system
 memset(SampleHandles, 255, sizeof(SampleHandles);
 next_handle = 0;
 //end edit by adb
}


// MIDI stuff follows.
//added/killed on 11/25/98 by Matthew Mueller
//void DigiSetMidiVolume(int mvolume) { }
//void DigiPlayMidiSong(char * filename, char * melodic_bank, char * drum_bank, int loop) {}
//void DigiStopCurrentSong()
//{
//#ifdef HMIPLAY
//        char buf[10];
//    
//        sprintf(buf,"s");
//        send_ipc(buf);
//#endif
//}
//end this section kill - MM

#endif //!USE_SDL_MIXER
