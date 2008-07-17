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

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "inferno.h"
#include "gauges.h"
#include "newmenu.h"
#include "error.h"
#include "gameseg.h"
#include "texmap.h"
#include "newdemo.h"
#include "endlevel.h"
#include "network.h"
#include "timer.h"
#include "segment.h"
#include "input.h"
#include "text.h"
#include "light.h"
#include "textdata.h"
#include "marker.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#define MAX_ORIENT_STEPS	10

int oppTrigTypes  [] = {
	TT_CLOSE_DOOR,
	TT_OPEN_DOOR,
	TT_MATCEN,
	TT_EXIT,
	TT_SECRET_EXIT,
	TT_ILLUSION_ON,
	TT_ILLUSION_OFF,
	TT_LOCK_DOOR,
	TT_UNLOCK_DOOR,
	TT_CLOSE_WALL,
	TT_OPEN_WALL,
	TT_ILLUSORY_WALL,
	TT_LIGHT_ON,
	TT_LIGHT_OFF,
	TT_TELEPORT,
	TT_SPEEDBOOST,
	TT_CAMERA,
	TT_SHIELD_DAMAGE,
	TT_ENERGY_DRAIN,
	TT_CHANGE_TEXTURE
	};

//link Links [MAX_WALL_LINKS];
//int Num_links;

#ifdef EDITOR
fix triggerTimeCount=F1_0;

//-----------------------------------------------------------------
// Initializes all the switches.
void TriggerInit ()
{
	int i;

	gameData.trigs.nTriggers = 0;

for (i = 0; i < MAX_TRIGGERS; i++) {
	gameData.trigs.triggers [i].nType = 0;
	gameData.trigs.triggers [i].flags = 0;
	gameData.trigs.triggers [i].nLinks = 0;
	gameData.trigs.triggers [i].value = 0;
	gameData.trigs.triggers [i].time = -1;
	}
memset (gameData.trigs.delay, -1, sizeof (gameData.trigs.delay));
}
#endif

//-----------------------------------------------------------------
// Executes a link, attached to a tTrigger.
// Toggles all walls linked to the switch.
// Opens doors, Blasts blast walls, turns off illusions.
void DoLink (tTrigger *trigP)
{
	int i;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++)
	WallToggle (gameData.segs.segments + *segs, *sides);
}

//-----------------------------------------------------------------

void DoChangeTexture (tTrigger *trigP)
{
	int	i, 
			baseTex = trigP->value & 0xffff,
			ovlTex = (trigP->value >> 16);

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	gameData.segs.segments [*segs].sides [*sides].nBaseTex = baseTex;
	if (ovlTex > 0)
		gameData.segs.segments [*segs].sides [*sides].nOvlTex = ovlTex;
	}
}

//-----------------------------------------------------------------

inline int DoExecObjTrigger (tTrigger *trigP, short nObject, int bDamage)
{
	fix	v = 10 - trigP->value;

if (bDamage != ((trigP->nType == TT_TELEPORT) || (trigP->nType == TT_SPAWN_BOT)))
	return 0;
if (!bDamage)
	return 1;
if (v >= 10)
	return 0;
if ((fix) (ObjectDamage (OBJECTS + nObject) * 100) > v * 10)
	return 0;
if (!(trigP->flags & TF_PERMANENT))
	trigP->value = 0;
return 1;
}

//-----------------------------------------------------------------

void DoSpawnBot (tTrigger *trigP, short nObject)
{
SpawnBotTrigger (OBJECTS + nObject, trigP->nLinks ? trigP->nSegment [0] : -1);
}

//-----------------------------------------------------------------

void DoTeleportBot (tTrigger *trigP, short nObject)
{
if (trigP->nLinks) {
	tObject *objP = OBJECTS + nObject;
	short nSegment = trigP->nSegment [d_rand () % trigP->nLinks];
	if (objP->nSegment != nSegment) {
		objP->nSegment = nSegment;
		COMPUTE_SEGMENT_CENTER_I (&objP->position.vPos, nSegment);
		RelinkObject (nObject, nSegment);
		if (ROBOTINFO (objP->id).bossFlag) {
			int	i = FindBoss (nObject);

			if (i >= 0)
				InitBossData (i, nObject);
			}
		}
	}
}

//------------------------------------------------------------------------------
//close a door
void DoCloseDoor (tTrigger *trigP)
{
	int i;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++)
	WallCloseDoor (gameData.segs.segments+*segs, *sides);
}

//------------------------------------------------------------------------------
//turns lighting on.  returns true if lights were actually turned on. (they
//would not be if they had previously been shot out).
int DoLightOn (tTrigger *trigP)
{
	int i,ret=0;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
short nSegment,nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	nSegment = *segs;
	nSide = *sides;

	//check if tmap2 casts light before turning the light on.  This
	//is to keep us from turning on blown-out lights
	if (gameData.pig.tex.pTMapInfo [gameData.segs.segments [nSegment].sides [nSide].nOvlTex].lighting) {
		ret |= AddLight (nSegment, nSide); 		//any light sets flag
		EnableVariableLight (nSegment, nSide);
	}
}
return ret;
}

//------------------------------------------------------------------------------
//turns lighting off.  returns true if lights were actually turned off. (they
//would not be if they had previously been shot out).
int DoLightOff (tTrigger *trigP)
{
	int i,ret=0;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
short nSegment,nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	nSegment = *segs;
	nSide = *sides;

	//check if tmap2 casts light before turning the light off.  This
	//is to keep us from turning off blown-out lights
	if (gameData.pig.tex.pTMapInfo [gameData.segs.segments [nSegment].sides [nSide].nOvlTex].lighting) {
		ret |= SubtractLight (nSegment, nSide); 	//any light sets flag
		DisableVariableLight (nSegment, nSide);
	}
}
return ret;
}

//------------------------------------------------------------------------------
// Unlocks all doors linked to the switch.
void DoUnlockDoors (tTrigger *trigP)
{
	int i;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
short nSegment,nSide, nWall;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	nSegment = *segs;
	nSide = *sides;
	nWall=WallNumI (nSegment, nSide);
	gameData.walls.walls [nWall].flags &= ~WALL_DOOR_LOCKED;
	gameData.walls.walls [nWall].keys = KEY_NONE;
}
}

//------------------------------------------------------------------------------
// Return tTrigger number if door is controlled by a tWall switch, else return -1.
int DoorIsWallSwitched (int nWall)
{
	int i, nTrigger;
	tTrigger *trigP = gameData.trigs.triggers;
	short *segs, *sides;

for (nTrigger=0; nTrigger < gameData.trigs.nTriggers; nTrigger++, trigP++) {
	segs = trigP->nSegment;
	sides = trigP->nSide;
	for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
		if (WallNumI (*segs, *sides) == nWall) {
			return nTrigger;
			}
	  	}
	}
return -1;
}

//------------------------------------------------------------------------------

void FlagWallSwitchedDoors (void)
{
	int	i;

for (i = 0; i < gameData.walls.nWalls; i++)
	if (DoorIsWallSwitched (i))
		gameData.walls.walls [i].flags |= WALL_WALL_SWITCH;
}

//------------------------------------------------------------------------------
// Locks all doors linked to the switch.
void DoLockDoors (tTrigger *trigP)
{
	int i;
	short *segs = trigP->nSegment;
	short *sides = trigP->nSide;

for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	gameData.walls.walls [WallNumI (*segs, *sides)].flags |= WALL_DOOR_LOCKED;
}
}

//------------------------------------------------------------------------------
// Changes player spawns according to triggers segments,sides list

int DoSetSpawnPoints (tTrigger *trigP, short nObject)
{
	int 		h, i, j;
	short 	*segs = trigP->nSegment;
	short 	*sides = trigP->nSide;
	short		nSegment;

for (h = trigP->nLinks, i = j = 0; i < MAX_PLAYERS; i++) {
	nSegment = segs [j];
	TriggerSetOrient (&gameData.multiplayer.playerInit [i].position, nSegment, sides [j], 1, 0);
	gameData.multiplayer.playerInit [i].nSegment = nSegment;
	gameData.multiplayer.playerInit [i].nSegType = gameData.segs.segment2s [nSegment].special;
	if (i == gameData.multiplayer.nLocalPlayer)
		MoveSpawnMarker (&gameData.multiplayer.playerInit [i].position, nSegment);
	j = (j + 1) % h;
	}
// delete any spawn markers that have been set before passing through this trigger to 
// avoid players getting stuck when respawning at that marker
if (0 <= (gameData.marker.nHighlight = SpawnMarkerIndex (-1)))
	DeleteMarker (1);
return 1;
}

//------------------------------------------------------------------------------
// Changes player spawns according to triggers segments,sides list

int DoMasterTrigger (tTrigger *masterP, short nObject)
{
	int 		h, i;
	short 	*segs = masterP->nSegment;
	short 	*sides = masterP->nSide;
	short		nWall, nTrigger;
	tWall		*wallP;

for (h = masterP->nLinks, i = 0; i < h; i++) {
	if (IS_WALL (nWall = WallNumP (SEGMENTS + segs [i], sides [i]))) {
		wallP = WALLS + nWall;
		nTrigger = wallP->nTrigger;
		if ((nTrigger >= 0) && (nTrigger < gameData.trigs.nTriggers)) 
			CheckTriggerSub (nObject, gameData.trigs.triggers, gameData.trigs.nTriggers, nTrigger, gameData.multiplayer.nLocalPlayer, 0, 0);
		}
	}
return 1;
}

//------------------------------------------------------------------------------

int DoShowMessage (tTrigger *trigP, short nObject)
{
ShowGameMessage (gameData.messages, f2i (trigP->value), trigP->time);
return 1;
}

//------------------------------------------------------------------------------

int DoPlaySound (tTrigger *trigP, short nObject)
{
	tTextIndex	*indexP = FindTextData (&gameData.sounds, f2i (trigP->value));

if (!indexP)
	return 0;
if (trigP->time < 0)
	DigiStartSound (-1, F1_0, 0xffff / 2, -1, -1, -1, -1, F1_0, indexP->pszText, NULL, 0);
else
	DigiStartSound (-1, F1_0, 0xffff / 2, 0, 0, trigP->time - 1, -1, F1_0, indexP->pszText, NULL, 0);
return 1;
}

//------------------------------------------------------------------------------
// Changes walls pointed to by a tTrigger. returns true if any walls changed
int DoChangeWalls (tTrigger *trigP)
{
	int 		i,ret = 0;
	short 	*segs = trigP->nSegment;
	short 	*sides = trigP->nSide;
	short 	nSide,nConnSide,nWall,nConnWall;
	int 		nNewWallType;
	tSegment *segP, *cSegP;

for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {

	segP = gameData.segs.segments + *segs;
	nSide = *sides;

	if (segP->children [nSide] < 0) {
		if (gameOpts->legacy.bSwitches)
			Warning (TXT_TRIG_SINGLE, *segs, nSide, TRIG_IDX (trigP));
		cSegP = NULL;
		nConnSide = -1;
		}
	else {
		cSegP = gameData.segs.segments + segP->children [nSide];
		nConnSide = FindConnectedSide (segP, cSegP);
		}
	switch (trigP->nType) {
		case TT_OPEN_WALL:
			nNewWallType = WALL_OPEN; 
			break;
		case TT_CLOSE_WALL:	
			nNewWallType = WALL_CLOSED; 
			break;
		case TT_ILLUSORY_WALL:
			nNewWallType = WALL_ILLUSION; 
			break;
		default:
			Assert (0); /* nNewWallType unset */
			return 0;
			break;
		}
	nWall = WallNumP (segP, nSide);
	if (!IS_WALL (nWall)) {
#ifdef _DEBUG
		PrintLog ("WARNING: Wall trigger %d targets non-existant tWall @ %d,%d\n", 
				  trigP - gameData.trigs.triggers, SEG_IDX (segP), nSide);
#endif
		continue;
		}
	nConnWall = (nConnSide < 0) ? NO_WALL : WallNumP (cSegP, nConnSide);
	if ((gameData.walls.walls [nWall].nType == nNewWallType) &&
		 (!IS_WALL (nConnWall) || (gameData.walls.walls [nConnWall].nType == nNewWallType)))
		continue;		//already in correct state, so skip
	ret = 1;
	switch (trigP->nType) {
		case TT_OPEN_WALL:
			if (!(gameData.pig.tex.pTMapInfo [segP->sides [nSide].nBaseTex].flags & TMI_FORCE_FIELD)) 
				StartWallCloak (segP,nSide);
			else {
				vmsVector pos;
				COMPUTE_SIDE_CENTER (&pos, segP, nSide);
				DigiLinkSoundToPos (SOUND_FORCEFIELD_OFF, SEG_IDX (segP), nSide, &pos, 0, F1_0);
				gameData.walls.walls [nWall].nType = nNewWallType;
				DigiKillSoundLinkedToSegment (SEG_IDX (segP),nSide,SOUND_FORCEFIELD_HUM);
				if (IS_WALL (nConnWall)) {
					gameData.walls.walls [nConnWall].nType = nNewWallType;
					DigiKillSoundLinkedToSegment (SEG_IDX (cSegP),nConnSide,SOUND_FORCEFIELD_HUM);
					}
				}
			ret = 1;
			break;

		case TT_CLOSE_WALL:
			if (!(gameData.pig.tex.pTMapInfo [segP->sides [nSide].nBaseTex].flags & TMI_FORCE_FIELD)) 
				StartWallDecloak (segP,nSide);
			else {
				vmsVector pos;
				COMPUTE_SIDE_CENTER (&pos, segP, nSide);
				DigiLinkSoundToPos (SOUND_FORCEFIELD_HUM, SEG_IDX (segP),nSide,&pos,1, F1_0/2);
				gameData.walls.walls [nWall].nType = nNewWallType;
				if (IS_WALL (nConnWall))
					gameData.walls.walls [nConnWall].nType = nNewWallType;
				}
			break;

		case TT_ILLUSORY_WALL:
			gameData.walls.walls [WallNumP (segP, nSide)].nType = nNewWallType;
			if (IS_WALL (nConnWall))
				gameData.walls.walls [nConnWall].nType = nNewWallType;
			break;
		}
	KillStuckObjects (WallNumP (segP, nSide));
	if (IS_WALL (nConnWall))
		KillStuckObjects (nConnWall);
  	}
return ret;
}

//------------------------------------------------------------------------------

void PrintTriggerMessage (int nPlayer, int trig, int shot, const char *message)
 {
	char		*pl;		//points to 's' or nothing for plural word
	tTrigger	*triggers;

if (nPlayer < 0)
	triggers = gameData.trigs.objTriggers;
else {
	if (nPlayer != gameData.multiplayer.nLocalPlayer)
		return;
	triggers = gameData.trigs.triggers;
	}
pl = (triggers [trig].nLinks > 1) ? (char *) "s" : (char *) "";
if (!(triggers [trig].flags & TF_NO_MESSAGE) && shot)
	HUDInitMessage (message, pl);
}


//------------------------------------------------------------------------------

void DoMatCen (tTrigger *trigP, int bMessage)
{
	int i, h [3] = {0,0,0};

short *segs = trigP->nSegment;
for (i = trigP->nLinks; i > 0; i--, segs++)
	h [MatCenTrigger (*segs)]++;
if (bMessage) {
	if (h [1])
		HUDInitMessage (TXT_EQUIPGENS_ON, (h [1] == 1) ? "" : "s");
	if (h [2])
		HUDInitMessage (TXT_EQUIPGENS_OFF, (h [2] == 1) ? "" : "s");
	}
}

//------------------------------------------------------------------------------

void DoIllusionOn (tTrigger *trigP)
{
	int i;

short *segs = trigP->nSegment;
short *sides = trigP->nSide;
for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	WallIllusionOn (&gameData.segs.segments [*segs], *sides);
}
}

//------------------------------------------------------------------------------

void DoIllusionOff (tTrigger *trigP)
{
	int i;
	short *segs = trigP->nSegment;
	short *sides = trigP->nSide;
	tSegment *seg;

for (i = trigP->nLinks; i > 0; i--, segs++, sides++) {
	vmsVector	cp;
	seg = gameData.segs.segments + *segs;
	WallIllusionOff (seg, *sides);
	COMPUTE_SIDE_CENTER (&cp, seg, *sides);
	DigiLinkSoundToPos (SOUND_WALL_REMOVED, SEG_IDX (seg), *sides, &cp, 0, F1_0);
  	}
}

//------------------------------------------------------------------------------

void TriggerSetOrient (tPosition *posP, short nSegment, short nSide, int bSetPos, int nStep)
{
	vmsAngVec	an;
	vmsVector	n;

if (nStep <= 0) {
	n = *gameData.segs.segments [nSegment].sides [nSide].normals;
	n.p.x = -n.p.x;
	n.p.y = -n.p.y;
	n.p.z = -n.p.z;
	gameStates.gameplay.vTgtDir = n;
	if (nStep < 0)
		nStep = MAX_ORIENT_STEPS;
	}
else
	n = gameStates.gameplay.vTgtDir;
// turn the ship so that it is facing the destination nSide of the destination tSegment
// invert the normal as it points into the tSegment
// compute angles from the normal
VmExtractAnglesVector (&an, &n);
// create new orientation matrix
if (!nStep)
	VmAngles2Matrix (&posP->mOrient, &an);
if (bSetPos)
	COMPUTE_SEGMENT_CENTER_I (&posP->vPos, nSegment); 
// rotate the ships vel vector accordingly
//StopPlayerMovement ();
}

//------------------------------------------------------------------------------

void TriggerSetObjOrient (short nObject, short nSegment, short nSide, int bSetPos, int nStep)
{
	vmsAngVec	ad, an, av;
	vmsVector	vel, n;
	vmsMatrix	rm;
	tObject		*objP = OBJECTS + nObject;

TriggerSetOrient (&objP->position, nSegment, nSide, bSetPos, nStep);
if (nStep <= 0) {
	n = *gameData.segs.segments [nSegment].sides [nSide].normals;
	n.p.x = -n.p.x;
	n.p.y = -n.p.y;
	n.p.z = -n.p.z;
	gameStates.gameplay.vTgtDir = n;
	if (nStep < 0)
		nStep = MAX_ORIENT_STEPS;
	}
else
	n = gameStates.gameplay.vTgtDir;
VmExtractAnglesVector (&an, &n);
VmExtractAnglesVector (&av, &objP->mType.physInfo.velocity);
av.p -= an.p;
av.b -= an.b;
av.h -= an.h;
if (nStep) {
	if (nStep > 1) {
		av.p /= nStep;
		av.b /= nStep;
		av.h /= nStep;
		VmExtractAnglesMatrix (&ad, &objP->position.mOrient);
		ad.p += (an.p - ad.p) / nStep;
		ad.b += (an.b - ad.b) / nStep;
		ad.h += (an.h - ad.h) / nStep;
		VmAngles2Matrix (&objP->position.mOrient, &ad);
		}
	else
		VmAngles2Matrix (&objP->position.mOrient, &an);
	}
VmAngles2Matrix (&rm, &av);
VmVecRotate (&vel, &objP->mType.physInfo.velocity, &rm);
objP->mType.physInfo.velocity = vel;
//StopPlayerMovement ();
}

//------------------------------------------------------------------------------

void TriggerSetObjPos (short nObject, short nSegment)
{
RelinkObject (nObject, nSegment);
}

//------------------------------------------------------------------------------

void DoTeleport (tTrigger *trigP, short nObject)
{
if (trigP->nLinks > 0) {
		int		i;
		short		nSegment, nSide;

	d_srand (TimerGetFixedSeconds ());
	i = d_rand () % trigP->nLinks;
	nSegment = trigP->nSegment [i];
	nSide = trigP->nSide [i];
	// set new tPlayer direction, facing the destination nSide
	TriggerSetObjOrient (nObject, nSegment, nSide, 1, 0);
	TriggerSetObjPos (nObject, nSegment);
	gameStates.render.bDoAppearanceEffect = 1;
	MultiSendTeleport ((char) nObject, nSegment, (char) nSide);
	}
}

//------------------------------------------------------------------------------

tWall *TriggerParentWall (short nTrigger)
{
	int	i;

for (i = 0; i < gameData.walls.nWalls; i++)
	if (gameData.walls.walls [i].nTrigger == nTrigger)
		return gameData.walls.walls + i;
return NULL;
}

//------------------------------------------------------------------------------

fix			speedBoostSpeed = 0;

void SetSpeedBoostVelocity (short nObject, fix speed, 
									 short srcSegnum, short srcSidenum,
									 short destSegnum, short destSidenum,
									 vmsVector *pSrcPt, vmsVector *pDestPt,
									 int bSetOrient)
{
	vmsVector			n, h;
	tObject				*objP = OBJECTS + nObject;
	int					v;
	tSpeedBoostData	sbd = gameData.objs.speedBoost [nObject];

if (speed < 0)
	speed = speedBoostSpeed;
if ((speed <= 0) || (speed > 10))
	speed = 10;
speedBoostSpeed = speed;
v = 60 + (COMPETITION ? 100 : extraGameInfo [IsMultiGame].nSpeedBoost) * 4 * speed;
if (sbd.bBoosted) {
	if (pSrcPt && pDestPt) {
		VmVecSub (&n, pDestPt, pSrcPt);
		VmVecNormalize (&n);
		}
	else if (srcSegnum >= 0) {
		COMPUTE_SIDE_CENTER (&sbd.vSrc, gameData.segs.segments + srcSegnum, srcSidenum);
		COMPUTE_SIDE_CENTER (&sbd.vDest, gameData.segs.segments + destSegnum, destSidenum);
		if (memcmp (&sbd.vSrc, &sbd.vDest, sizeof (vmsVector))) {
			VmVecSub (&n, &sbd.vDest, &sbd.vSrc);
			VmVecNormalize (&n);
			}
		else {
			Controls [0].verticalThrustTime =
			Controls [0].forwardThrustTime =
			Controls [0].sidewaysThrustTime = 0;
			memcpy (&n, gameData.segs.segments [destSegnum].sides [destSidenum].normals, sizeof (n));
		// turn the ship so that it is facing the destination nSide of the destination tSegment
		// invert the normal as it points into the tSegment
			n.p.x = -n.p.x;
			n.p.y = -n.p.y;
			n.p.z = -n.p.z;
			}
		}
	else {
		memcpy (&n, gameData.segs.segments [destSegnum].sides [destSidenum].normals, sizeof (n));
	// turn the ship so that it is facing the destination nSide of the destination tSegment
	// invert the normal as it points into the tSegment
		n.p.x = -n.p.x;
		n.p.y = -n.p.y;
		n.p.z = -n.p.z;
		}
	sbd.vVel.p.x = n.p.x * v;
	sbd.vVel.p.y = n.p.y * v;
	sbd.vVel.p.z = n.p.z * v;
#if 0
	d = (double) (labs (n.p.x) + labs (n.p.y) + labs (n.p.z)) / ((double) F1_0 * 60.0);
	h.p.x = n.p.x ? (fix) ((double) n.p.x / d) : 0;
	h.p.y = n.p.y ? (fix) ((double) n.p.y / d) : 0;
	h.p.z = n.p.z ? (fix) ((double) n.p.z / d) : 0;
#else
#	if 1
	h.p.x =
	h.p.y =
	h.p.z = F1_0 * 60;
#	else
	h.p.x = (n.p.x ? n.p.x : F1_0) * 60;
	h.p.y = (n.p.y ? n.p.y : F1_0) * 60;
	h.p.z = (n.p.z ? n.p.z : F1_0) * 60;
#	endif
#endif
	VmVecSub (&sbd.vMinVel, &sbd.vVel, &h);
/*
	if (!sbd.vMinVel.p.x)
		sbd.vMinVel.p.x = F1_0 * -60;
	if (!sbd.vMinVel.p.y)
		sbd.vMinVel.p.y = F1_0 * -60;
	if (!sbd.vMinVel.p.z)
		sbd.vMinVel.p.z = F1_0 * -60;
*/
	VmVecAdd (&sbd.vMaxVel, &sbd.vVel, &h);
/*
	if (!sbd.vMaxVel.p.x)
		sbd.vMaxVel.p.x = F1_0 * 60;
	if (!sbd.vMaxVel.p.y)
		sbd.vMaxVel.p.y = F1_0 * 60;
	if (!sbd.vMaxVel.p.z)
		sbd.vMaxVel.p.z = F1_0 * 60;
*/
	if (sbd.vMinVel.p.x > sbd.vMaxVel.p.x) {
		fix h = sbd.vMinVel.p.x;
		sbd.vMinVel.p.x = sbd.vMaxVel.p.x;
		sbd.vMaxVel.p.x = h;
		}
	if (sbd.vMinVel.p.y > sbd.vMaxVel.p.y) {
		fix h = sbd.vMinVel.p.y;
		sbd.vMinVel.p.y = sbd.vMaxVel.p.y;
		sbd.vMaxVel.p.y = h;
		}
	if (sbd.vMinVel.p.z > sbd.vMaxVel.p.z) {
		fix h = sbd.vMinVel.p.z;
		sbd.vMinVel.p.z = sbd.vMaxVel.p.z;
		sbd.vMaxVel.p.z = h;
		}
	objP->mType.physInfo.velocity = sbd.vVel;
	if (bSetOrient) {
		TriggerSetObjOrient (nObject, destSegnum, destSidenum, 0, -1);
		gameStates.gameplay.nDirSteps = MAX_ORIENT_STEPS - 1;
		}
	gameData.objs.speedBoost [nObject] = sbd;
	}
else {
	objP->mType.physInfo.velocity.p.x = objP->mType.physInfo.velocity.p.x / v * 60;
	objP->mType.physInfo.velocity.p.y = objP->mType.physInfo.velocity.p.y / v * 60;
	objP->mType.physInfo.velocity.p.z = objP->mType.physInfo.velocity.p.z / v * 60;
	}
}

//------------------------------------------------------------------------------

void UpdatePlayerOrient (void)
{
if (gameStates.app.tick40fps.bTick && gameStates.gameplay.nDirSteps)
	TriggerSetObjOrient (LOCALPLAYER.nObject, -1, -1, 0, gameStates.gameplay.nDirSteps--);
}

//------------------------------------------------------------------------------

void DoSpeedBoost (tTrigger *trigP, short nObject)
{
if (!(COMPETITION || IsCoopGame) || extraGameInfo [IsMultiGame].nSpeedBoost) {
	tWall *w = TriggerParentWall (TRIG_IDX (trigP));
	gameData.objs.speedBoost [nObject].bBoosted = (trigP->value && (trigP->nLinks > 0));
	SetSpeedBoostVelocity ((short) nObject, trigP->value, 
								  (short) (w ? w->nSegment : -1), (short) (w ? w->nSide : -1),
								  trigP->nSegment [0], trigP->nSide [0], NULL, NULL, (trigP->flags & TF_SET_ORIENT) != 0);
	}
}

//------------------------------------------------------------------------------

extern void EnterSecretLevel (void);
extern void ExitSecretLevel (void);
extern int PSecretLevelDestroyed (void);

int WallIsForceField (tTrigger *trigP)
{
	int i;
	short *segs = trigP->nSegment;
	short *sides = trigP->nSide;

for (i = trigP->nLinks; i > 0; i--, segs++, sides++)
	if ((gameData.pig.tex.pTMapInfo [gameData.segs.segments [*segs].sides [*sides].nBaseTex].flags & TMI_FORCE_FIELD))
		break;
return (i > 0);
}

//------------------------------------------------------------------------------

int CheckTriggerSub (short nObject, tTrigger *triggers, int nTriggerCount, 
							int nTrigger, int nPlayer, int shot, int bObjTrigger)
{
	tTrigger	*trigP;
	tObject	*objP = OBJECTS + nObject;
	ubyte		bIsPlayer = (objP->nType == OBJ_PLAYER);

if (nTrigger >= nTriggerCount)
	return 1;
trigP = triggers + nTrigger;
if (trigP->flags & TF_DISABLED)
	return 1;		//1 means don't send trigger hit to other players
if (bIsPlayer) {
	if (!IsMultiGame && (nObject != LOCALPLAYER.nObject))
		return 1;
	}
else {
	nPlayer = -1;
	if ((trigP->nType != TT_TELEPORT) && (trigP->nType != TT_SPEEDBOOST)) {
		if ((objP->nType != OBJ_ROBOT) && (objP->nType != OBJ_REACTOR))
			return 1;
		if (!bObjTrigger)
			return 1;
		}
	else
		if ((objP->nType != OBJ_ROBOT) && (objP->nType != OBJ_REACTOR))
			return 1;
		}
#if 1
if ((triggers == gameData.trigs.triggers) && 
	 (trigP->nType != TT_TELEPORT) && (trigP->nType != TT_SPEEDBOOST)) {
	int t = gameStates.app.nSDLTicks;
	if ((gameData.trigs.delay [nTrigger] >= 0) && (t - gameData.trigs.delay [nTrigger] < 750))
		return 1;
	gameData.trigs.delay [nTrigger] = t;
	}
#endif
if (trigP->flags & TF_ONE_SHOT)		//if this is a one-shot...
	trigP->flags |= TF_DISABLED;		//..then don't let it happen again

switch (trigP->nType) {

	case TT_EXIT:
		if (nPlayer != gameData.multiplayer.nLocalPlayer)
			break;
		DigiStopAll ();		//kill the sounds
		if ((gameData.missions.nCurrentLevel > 0) || gameStates.app.bD1Mission) {
			StartEndLevelSequence (0);
			} 
		else if (gameData.missions.nCurrentLevel < 0) {
			if ((LOCALPLAYER.shields < 0) || 
					gameStates.app.bPlayerIsDead)
				break;
			ExitSecretLevel ();
			return 1;
			}
		else {
#ifdef EDITOR
				ExecMessageBox ("Yo!", 1, "You have hit the exit tTrigger!", "");
#else
				Int3 ();		//level num == 0, but no editor!
			#endif
			}
		return 1;
		break;

	case TT_SECRET_EXIT: {
		int	truth;

		if (nPlayer != gameData.multiplayer.nLocalPlayer)
			break;
		if ((LOCALPLAYER.shields < 0) || 
				gameStates.app.bPlayerIsDead)
			break;
		if (gameData.app.nGameMode & GM_MULTI) {
			HUDInitMessage (TXT_TELEPORT_MULTI);
			DigiPlaySample (SOUND_BAD_SELECTION, F1_0);
			break;
		}
		truth = PSecretLevelDestroyed ();

		if (gameData.demo.nState == ND_STATE_RECORDING)			// record whether we're really going to the secret level
			NDRecordSecretExitBlown (truth);

		if ((gameData.demo.nState != ND_STATE_PLAYBACK) && truth) {
			HUDInitMessage (TXT_SECRET_DESTROYED);
			DigiPlaySample (SOUND_BAD_SELECTION, F1_0);
			break;
		}

		if (gameData.demo.nState == ND_STATE_RECORDING)		// stop demo recording
			gameData.demo.nState = ND_STATE_PAUSED;
		DigiStopAll ();		//kill the sounds
		DigiPlaySample (SOUND_SECRET_EXIT, F1_0);
		GrPaletteFadeOut (NULL, 32, 0);
		EnterSecretLevel ();
		gameData.reactor.bDestroyed = 0;
		return 1;
		break;
	}

	case TT_OPEN_DOOR:
		DoLink (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Door%s opened!");
		break;

	case TT_CLOSE_DOOR:
		DoCloseDoor (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Door%s closed!");
		break;

	case TT_UNLOCK_DOOR:
		DoUnlockDoors (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Door%s unlocked!");
		break;

	case TT_LOCK_DOOR:
		DoLockDoors (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Door%s locked!");
		break;

	case TT_OPEN_WALL:
		if (DoChangeWalls (trigP))	{
			if (WallIsForceField (trigP))
				PrintTriggerMessage (nPlayer, nTrigger, shot, "Force field%s deactivated!");
			else
				PrintTriggerMessage (nPlayer, nTrigger, shot, "Wall%s opened!");
			}
		break;

	case TT_CLOSE_WALL:
		if (DoChangeWalls (trigP)) {
			if (WallIsForceField (trigP))
				PrintTriggerMessage (nPlayer, nTrigger, shot, "Force field%s activated!");
			else
				PrintTriggerMessage (nPlayer, nTrigger, shot, "Wall%s closed!");
		}
		break;

	case TT_ILLUSORY_WALL:
		//don't know what to say, so say nothing
		DoChangeWalls (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Creating Illusion!");
		break;

	case TT_MATCEN:
		if (!(gameData.app.nGameMode & GM_MULTI) || (gameData.app.nGameMode & GM_MULTI_ROBOTS))
			DoMatCen (trigP, nPlayer == gameData.multiplayer.nLocalPlayer);
		break;

	case TT_ILLUSION_ON:
		DoIllusionOn (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Illusion%s on!");
		break;

	case TT_ILLUSION_OFF:
		DoIllusionOff (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, shot, "Illusion%s off!");
		break;

	case TT_LIGHT_OFF:
		if (DoLightOff (trigP))
			PrintTriggerMessage (nPlayer, nTrigger, shot, "Lights off!");
		break;

	case TT_LIGHT_ON:
		if (DoLightOn (trigP))
			PrintTriggerMessage (nPlayer, nTrigger, shot, "Lights on!");
		break;

	case TT_TELEPORT:
		if (bObjTrigger) {
			DoTeleportBot (trigP, nObject);
			PrintTriggerMessage (nPlayer, nTrigger, shot, "Robot is fleeing!");
			}
		else {
			if (bIsPlayer) {
				if (nPlayer != gameData.multiplayer.nLocalPlayer)
					break;
				if ((LOCALPLAYER.shields < 0) || 
						gameStates.app.bPlayerIsDead)
					break;
				}
			DigiPlaySample (SOUND_SECRET_EXIT, F1_0);
			DoTeleport (trigP, nObject);
			if (bIsPlayer)
				PrintTriggerMessage (nPlayer, nTrigger, shot, "Teleport!");
			}
		break;

	case TT_SPEEDBOOST:
		if (bIsPlayer) {
			if (nPlayer != gameData.multiplayer.nLocalPlayer)
				break;
			if ((LOCALPLAYER.shields < 0) || 
				 gameStates.app.bPlayerIsDead)
				break;
			}
		DoSpeedBoost (trigP, nObject);
		if (bIsPlayer)
			PrintTriggerMessage (nPlayer, nTrigger, shot, "Speed Boost!");
		break;

	case TT_SHIELD_DAMAGE:
		if (gameStates.app.bD1Mission)
			LOCALPLAYER.shields += gameData.trigs.triggers [nTrigger].value;
		else
			LOCALPLAYER.shields += (fix) (LOCALPLAYER.shields * f2fl (gameData.trigs.triggers [nTrigger].value) / 100);
		break;

	case TT_ENERGY_DRAIN:
		if (gameStates.app.bD1Mission)
			LOCALPLAYER.energy += gameData.trigs.triggers [nTrigger].value;
		else
			LOCALPLAYER.energy += (fix) (LOCALPLAYER.energy * f2fl (gameData.trigs.triggers [nTrigger].value) / 100);
		break;

	case TT_CHANGE_TEXTURE:
		DoChangeTexture (trigP);
		PrintTriggerMessage (nPlayer, nTrigger, 2, "Walls have been changed!");
		break;

	case TT_SPAWN_BOT:
		DoSpawnBot (trigP, nObject);
		PrintTriggerMessage (nPlayer, nTrigger, 1, "Robot is summoning help!");
		break;

	case TT_SET_SPAWN:
		DoSetSpawnPoints (trigP, nObject);
		PrintTriggerMessage (nPlayer, nTrigger, 1, "New spawn points set!");
		break;

	case TT_SMOKE_LIFE:
	case TT_SMOKE_SPEED:
	case TT_SMOKE_DENS:
	case TT_SMOKE_SIZE:
	case TT_SMOKE_DRIFT:
		break;

	case TT_COUNTDOWN:
		InitCountdown (trigP, 1, -1);
		break;

	case TT_MESSAGE:
		DoShowMessage (trigP, nObject);
		break;

	case TT_SOUND:
		DoPlaySound (trigP, nObject);
		break;

	case TT_MASTER:
		DoMasterTrigger (trigP, nObject);

	default:
		Int3 ();
		break;
	}
if (trigP->flags & TF_ALTERNATE)
	trigP->nType = oppTrigTypes [trigP->nType];
return 0;
}

//------------------------------------------------------------------------------

tTrigger *FindObjTrigger (short nObject, short nType, short nTrigger)
{
	short i = (nTrigger < 0) ? gameData.trigs.firstObjTrigger [nObject] : gameData.trigs.objTriggerRefs [nTrigger].next;

while (i >= 0) {
	if (gameData.trigs.objTriggerRefs [i].nObject < 0)
		break;
	if (gameData.trigs.objTriggers [i].nType == nType)
		return gameData.trigs.objTriggers + i;
	i = gameData.trigs.objTriggerRefs [i].next;
	}
return NULL;
}

//------------------------------------------------------------------------------

void ExecObjTriggers (short nObject, int bDamage)
{
	short		i = gameData.trigs.firstObjTrigger [nObject], j = 0;

while ((i >= 0) && (j < 256)) {
	if (gameData.trigs.objTriggerRefs [i].nObject < 0)
		break;
	if (DoExecObjTrigger (gameData.trigs.objTriggers + i, nObject, bDamage)) {
		CheckTriggerSub (nObject, gameData.trigs.objTriggers, gameData.trigs.nObjTriggers, i, -1, 1, 1);
		if (IsMultiGame)
			MultiSendObjTrigger (i);
		}
	if (!bDamage)
		gameData.trigs.objTriggerRefs [i].nObject = -1;
	i = gameData.trigs.objTriggerRefs [i].next;
	j++;
	}
}

//-----------------------------------------------------------------
// Checks for a tTrigger whenever an tObject hits a tTrigger nSide.
void CheckTrigger (tSegment *segP, short nSide, short nObject, int shot)
{
	int 		nWall;
	ubyte		nTrigger;	//, cnTrigger;
	tObject	*objP = OBJECTS + nObject;

nWall = WallNumP (segP, nSide);
if (!IS_WALL (nWall)) 
	return;
nTrigger = gameData.walls.walls [nWall].nTrigger;
if (CheckTriggerSub (nObject, gameData.trigs.triggers, gameData.trigs.nTriggers, nTrigger, 
							(objP->nType == OBJ_PLAYER) ? objP->id : -1, shot, 0))
	return;
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordTrigger (SEG_IDX (segP), nSide, nObject, shot);
if (IsMultiGame)
	MultiSendTrigger (nTrigger, nObject);
}

//------------------------------------------------------------------------------

void TriggersFrameProcess ()
{
	int		i;
	tTrigger	*trigP = gameData.trigs.triggers;

for (i = gameData.trigs.nTriggers; i > 0; i--, trigP++)
	if ((trigP->nType != TT_COUNTDOWN) && (trigP->nType != TT_MESSAGE) && (trigP->nType != TT_SOUND) && (trigP->time >= 0))
		trigP->time -= gameData.time.xFrame;
}

//------------------------------------------------------------------------------

static inline int TriggerHasTarget (tTrigger *triggerP, short nSegment, short nSide)
{
	int	i;
		
for (i = 0; i < triggerP->nLinks; i++)
	if ((triggerP->nSegment [i] == nSegment) && 
		 ((nSide < 0) || (triggerP->nSide [i] == nSide)))
	return 1;
return 0;
}

//------------------------------------------------------------------------------

tWall *FindTriggerWall (short nTrigger)
{
	int	i;

for (i = 0; i < gameData.walls.nWalls; i++)
	if (gameData.walls.walls [i].nTrigger == nTrigger)
		return gameData.walls.walls + i;
return NULL;
}

//------------------------------------------------------------------------------

int FindTriggerSegSide (short nTrigger)
{
	tWall	*wallP = FindTriggerWall (nTrigger);

return wallP ? wallP->nSegment * 65536 + wallP->nSide : -1;
}

//------------------------------------------------------------------------------

int ObjTriggerIsValid (int nTrigger)
{
	int	h, i, j;

for (i = 0; i < gameData.objs.nLastObject [0]; i++) {
	j = gameData.trigs.firstObjTrigger [i];
	if (j < 0)
		continue;
	if (gameData.trigs.objTriggerRefs [j].nObject < 0)
		continue;
	h = 0;
	while ((j >= 0) && (h < 256)) {
		if (j == nTrigger)
			return 1;
		j = gameData.trigs.objTriggerRefs [j].next;
		h++;
		}
	}
return 0;
}

//------------------------------------------------------------------------------

int FindTriggerTarget (short nSegment, short nSide)
{
	int	i, nSegSide, nOvlTex, ec;

for (i = 0; i < gameData.trigs.nTriggers; i++) {
	nSegSide = FindTriggerSegSide (i);
	if (nSegSide == -1)
		continue;
	nOvlTex = gameData.segs.segments [nSegSide / 65536].sides [nSegSide & 0xffff].nOvlTex;
	if (nOvlTex <= 0)
		continue;
	ec = gameData.pig.tex.pTMapInfo [nOvlTex].nEffectClip;
	if (ec < 0) {
		if (gameData.pig.tex.pTMapInfo [nOvlTex].destroyed == -1)
			continue;
		}
	else {
		tEffectClip *ecP = gameData.eff.pEffects + ec;
		if (ecP->flags & EF_ONE_SHOT)
			continue;
		if (ecP->nDestBm < 0)
			continue;
		}
	if (TriggerHasTarget (gameData.trigs.triggers + i, nSegment, nSide))
		return i + 1;
	}
for (i = 0; i < gameData.trigs.nObjTriggers; i++) {
	if (!TriggerHasTarget (gameData.trigs.objTriggers + i, nSegment, nSide))
		continue;
	if (!ObjTriggerIsValid (i))
		continue;
	return -i - 1;
	}
return 0;
}

//------------------------------------------------------------------------------

#if 1//ndef FAST_FILE_IO /*permanently enabled for a reason!*/
#if 0
	static char d2TriggerMap [10] = {
		TT_OPEN_DOOR,
		TT_SHIELD_DAMAGE,
		TT_ENERGY_DRAIN,
		TT_EXIT,
		-1,
		-1,
		TT_MATCEN,
		TT_ILLUSION_OFF,
		TT_ILLUSION_ON,
		TT_SECRET_EXIT
		};

	static char d2FlagMap [10] = {
		0,
		0,
		0,
		0,
		0,
		TF_ONE_SHOT,
		0,
		0,
		0,
		0
		};
#endif

/*
 * reads a tTriggerV29 structure from a CFILE
 */
extern void V29TriggerRead (tTriggerV29 *trigP, CFILE *fp)
{
	int	i;

trigP->nType = CFReadByte (fp);
trigP->flags = CFReadShort (fp);
trigP->value = CFReadFix (fp);
trigP->time = CFReadFix (fp);
trigP->link_num = CFReadByte (fp);
trigP->nLinks = CFReadShort (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSegment [i] = CFReadShort (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSide [i] = CFReadShort (fp);
}

//------------------------------------------------------------------------------

/*
 * reads a tTriggerV30 structure from a CFILE
 */
extern void V30TriggerRead (tTriggerV30 *trigP, CFILE *fp)
{
	int i;

trigP->flags = CFReadShort (fp);
trigP->nLinks = CFReadByte (fp);
trigP->pad = CFReadByte (fp);
trigP->value = CFReadFix (fp);
trigP->time = CFReadFix (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSegment [i] = CFReadShort (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSide [i] = CFReadShort (fp);
}

//------------------------------------------------------------------------------

/*
 * reads a tTrigger structure from a CFILE
 */
extern void TriggerRead (tTrigger *trigP, CFILE *fp, int bObjTrigger)
{
	int i;

trigP->nType = CFReadByte (fp);
if (bObjTrigger)
	trigP->flags = (short) CFReadShort (fp);
else
	trigP->flags = (short) CFReadByte (fp);
trigP->nLinks = CFReadByte (fp);
CFReadByte (fp);
trigP->value = CFReadFix (fp);
trigP->time = CFReadFix (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSegment [i] = CFReadShort (fp);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	trigP->nSide [i] = CFReadShort (fp);
}
#endif

//------------------------------------------------------------------------------
//eof
