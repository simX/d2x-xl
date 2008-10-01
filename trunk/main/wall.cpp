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
#include "error.h"
#include "gameseg.h"
#include "gauges.h"
#include "text.h"
#include "fireball.h"
#include "textures.h"
#include "newdemo.h"
#include "collide.h"
#include "render.h"
#include "light.h"
#include "dynlight.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

//	Special door on boss level which is locked if not in multiplayer...sorry for this awful solution --MK.
#define	BOSS_LOCKED_DOOR_LEVEL	7
#define	BOSS_LOCKED_DOOR_SEG		595
#define	BOSS_LOCKED_DOOR_SIDE	5

//--unused-- int gameData.walls.bitmaps [MAX_WALL_ANIMS];

//door Doors [MAX_DOORS];


#define CLOAKING_WALL_TIME f1_0

//--unused-- grsBitmap *wall_title_bms [MAX_WALL_ANIMS];

//#define BM_FLAG_TRANSPARENT			1
//#define BM_FLAG_SUPER_TRANSPARENT	2

#ifdef EDITOR
char	pszWallNames [7][10] = {
	"NORMAL   ",
	"BLASTABLE",
	"DOOR     ",
	"ILLUSION ",
	"OPEN     ",
	"CLOSED   ",
	"EXTERNAL "
};
#endif

void FlushFCDCache (void);

//-----------------------------------------------------------------

int AnimFrameCount (tWallClip *anim)
{
	int	n;

grsBitmap *bmP = gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [anim->frames [0]].index;
if (BM_OVERRIDE (bmP))
	bmP = BM_OVERRIDE (bmP);
n = (bmP->bmType == BM_TYPE_ALT) ? BM_PARENT (bmP) ? BM_FRAMECOUNT (BM_PARENT (bmP)) : BM_FRAMECOUNT (bmP) : anim->nFrameCount;
return (n > 1) ? n : anim->nFrameCount;
}

//-----------------------------------------------------------------

fix AnimPlayTime (tWallClip *anim)
{
int nFrames = AnimFrameCount (anim);
fix pt = (fix) (anim->xTotalTime * gameStates.gameplay.slowmo [0].fSpeed);

if (nFrames == anim->nFrameCount)
	return pt;
return (fix) (((double) pt * (double) anim->nFrameCount) / (double) nFrames);
}

// This function determines whether the current tSegment/nSide is transparent
//		1 = YES
//		0 = NO
//-----------------------------------------------------------------

int CheckTransparency (tSegment *segP, short nSide)
{
	tSide *sideP = segP->sides + nSide;
	grsBitmap	*bmP;

if (sideP->nOvlTex) {
	bmP = BmOverride (gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [sideP->nOvlTex].index, -1);
	if (bmP->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT)
		return 1;
	if (!(bmP->bmProps.flags & BM_FLAG_TRANSPARENT))
		return 0;
	}
bmP = BmOverride (gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [sideP->nBaseTex].index, -1);
if (bmP->bmProps.flags & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
	return 1;
if (gameStates.app.bD2XLevel) {
	short	c, nWallNum = WallNumP (segP, nSide);
	if (IS_WALL (nWallNum)) {
		c = gameData.walls.walls [nWallNum].cloakValue;
		if (c && (c < GR_ACTUAL_FADE_LEVELS))
			return 1;
		}
	}
return gameOpts->render.effects.bAutoTransparency && IsTransparentTexture (sideP->nBaseTex);
}

//-----------------------------------------------------------------
// This function checks whether we can fly through the given nSide.
//	In other words, whether or not we have a 'doorway'
//	 Flags:
//		WID_FLY_FLAG				1
//		WID_RENDER_FLAG			2
//		WID_RENDPAST_FLAG			4
//	 Return values:
//		WID_WALL						2	// 0/1/0		tWall
//		WID_TRANSPARENT_WALL		6	//	0/1/1		transparent tWall
//		WID_ILLUSORY_WALL			3	//	1/1/0		illusory tWall
//		WID_TRANSILLUSORY_WALL	7	//	1/1/1		transparent illusory tWall
//		WID_NO_WALL					5	//	1/0/1		no tWall, can fly through
int WallIsDoorWay (tSegment * segP, short nSide, tObject *objP)
{
	int	flags, nType;
	int	state;
	tWall *wallP = gameData.walls.walls + WallNumP (segP, nSide);
#ifdef _DEBUG
	short nSegment = SEG_IDX (segP);

Assert(nSegment>=0 && nSegment<=gameData.segs.nLastSegment);
Assert(nSide>=0 && nSide<6);
#endif
#ifdef _DEBUG
if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
nType = wallP->nType;
flags = wallP->flags;

if (nType == WALL_OPEN)
	return WID_NO_WALL;

if (nType == WALL_ILLUSION) {
	if (flags & WALL_ILLUSION_OFF)
		return WID_NO_WALL;
	if ((wallP->cloakValue < GR_ACTUAL_FADE_LEVELS) || CheckTransparency (segP, nSide))
		return WID_TRANSILLUSORY_WALL;
	return WID_ILLUSORY_WALL;
	}

if (nType == WALL_BLASTABLE) {
	if (flags & WALL_BLASTED)
		return WID_TRANSILLUSORY_WALL;
	if ((wallP->cloakValue < GR_ACTUAL_FADE_LEVELS) || CheckTransparency (segP, nSide))
		return WID_TRANSPARENT_WALL;
	return WID_WALL;
	}

if (flags & WALL_DOOR_OPENED)
	return WID_TRANSILLUSORY_WALL;

if (nType == WALL_CLOAKED)
	return WID_RENDER_FLAG | WID_RENDPAST_FLAG | WID_CLOAKED_FLAG;

if (nType == WALL_TRANSPARENT)
	return (wallP->hps < 0) ?
			 WID_RENDER_FLAG | WID_RENDPAST_FLAG | WID_TRANSPARENT_FLAG | WID_FLY_FLAG :
			 WID_RENDER_FLAG | WID_RENDPAST_FLAG | WID_TRANSPARENT_FLAG;

state = wallP->state;
if (nType == WALL_DOOR) {
	if ((state == WALL_DOOR_OPENING) || (state == WALL_DOOR_CLOSING))
		return WID_TRANSPARENT_WALL;
	if ((wallP->cloakValue && (wallP->cloakValue < GR_ACTUAL_FADE_LEVELS)) || CheckTransparency (segP, nSide))
		return WID_TRANSPARENT_WALL;
	return WID_WALL;
	}
if (nType == WALL_CLOSED) {
	if (objP && (objP->nType == OBJ_PLAYER)) {
		if (IsTeamGame && ((wallP->keys >> 1) == GetTeam (objP->id) + 1))
			return WID_ILLUSORY_WALL;
		if ((wallP->keys == KEY_BLUE) && (gameData.multiplayer.players [objP->id].flags & PLAYER_FLAGS_BLUE_KEY))
			return WID_ILLUSORY_WALL;
		if ((wallP->keys == KEY_RED) && (gameData.multiplayer.players [objP->id].flags & PLAYER_FLAGS_RED_KEY))
			return WID_ILLUSORY_WALL;
		}
	}
// If none of the above flags are set, there is no doorway.
if ((wallP->cloakValue && (wallP->cloakValue < GR_ACTUAL_FADE_LEVELS)) || CheckTransparency (segP, nSide)) {
#ifdef _DEBUG
	CheckTransparency (segP, nSide);
#endif
	return WID_TRANSPARENT_WALL;
	}
return WID_WALL; // There are children behind the door.
}

//-----------------------------------------------------------------

int WALL_IS_DOORWAY (tSegment *segP, short nSide, tObject *objP)
{
	int	nWall, bIsWall, nSegment, nChild = segP->children [nSide];

if (nChild == -1)
	return WID_RENDER_FLAG;
if (nChild == -2)
	return WID_EXTERNAL_FLAG;
nSegment = SEG_IDX (segP);
nWall = WallNumP (segP, nSide);
bIsWall = IS_WALL (nWall);
#ifdef _DEBUG
if (OBJ_IDX (objP) == nDbgObj)
	nDbgObj = nDbgObj;
#endif
if (objP && gameData.objs.speedBoost [OBJ_IDX (objP)].bBoosted &&
	 (objP == gameData.objs.console) &&
	 (gameData.segs.segment2s [nSegment].special == SEGMENT_IS_SPEEDBOOST) &&
	 (gameData.segs.segment2s [nChild].special != SEGMENT_IS_SPEEDBOOST) &&
	 (!bIsWall || (gameData.trigs.triggers [gameData.walls.walls [nWall].nTrigger].nType != TT_SPEEDBOOST)))
	return objP ? WID_RENDER_FLAG : bIsWall ? WallIsDoorWay (segP, nSide, objP) : WID_RENDPAST_FLAG;
if ((gameData.segs.segment2s [nChild].special == SEGMENT_IS_BLOCKED) ||
	 (gameData.segs.segment2s [nChild].special == SEGMENT_IS_SKYBOX))
	return (objP && ((objP->nType == OBJ_PLAYER) || (objP->nType == OBJ_ROBOT))) ? WID_RENDER_FLAG :
			 bIsWall ? WallIsDoorWay (segP, nSide, objP) : WID_FLY_FLAG | WID_RENDPAST_FLAG;
if (!bIsWall)
	return (WID_FLY_FLAG|WID_RENDPAST_FLAG);
return WallIsDoorWay (segP, nSide, objP);
}
#ifdef EDITOR

//-----------------------------------------------------------------
// Initializes all the walls (in other words, no special walls)
void WallInit ()
{
	int i;
	tWall *wallP = gameData.walls.walls;

gameData.walls.nWalls = 0;
for (i = 0; i < MAX_WALLS; i++, wallP++) {
	wallP->nSegment =
	wallP->nSide = -1;
	wallP->nType = WALL_NORMAL;
	wallP->flags = 0;
	wallP->hps = 0;
	wallP->nTrigger = NO_TRIGGER;
	wallP->nClip = -1;
	wallP->nLinkedWall = NO_WALL;
	}
gameData.walls.nOpenDoors = 0;
gameData.walls.nCloaking = 0;
}

//-----------------------------------------------------------------
// Initializes one tWall.
void WallReset (tSegment *segP, short nSide)
{
	tWall *wallP;
	int i = WallNumP (segP, nSide);

if (!IS_WALL (i)) {
#if TRACE
	con_printf (CONDBG, "Resetting Illegal Wall\n");
#endif
	return;
	}
wallP = gameData.walls.walls;
wallP->nSegment = SEG_IDX (segP);
wallP->nSide = nSide;
wallP->nType = WALL_NORMAL;
wallP->flags = 0;
wallP->hps = 0;
wallP->nTrigger = NO_TRIGGER;
wallP->nClip = -1;
wallP->nLinkedWall = NO_WALL;
}
#endif

//-----------------------------------------------------------------
//set the nBaseTex or nOvlTex field for a tWall/door
void WallSetTMapNum (tSegment *segP, short nSide, tSegment *connSegP, short cSide, int nAnim, int nFrame)
{
	tWallClip	*anim = gameData.walls.pAnims + nAnim;
	short			tmap = anim->frames [(anim->flags & WCF_ALTFMT) ? 0 : nFrame];
	grsBitmap	*bmP;
	int			nFrames;

//if (gameData.demo.nState == ND_STATE_PLAYBACK)
//	return;
if (cSide < 0)
	connSegP = NULL;
if (anim->flags & WCF_ALTFMT) {
	nFrames = anim->nFrameCount;
	bmP = SetupHiresAnim ((short *) anim->frames, nFrames, -1, 1, 0, &nFrames);
	//if (anim->flags & WCF_TMAP1)
	if (!bmP)
		anim->flags &= ~WCF_ALTFMT;
	else {
		bmP->bmWallAnim = 1;
		if (!gameOpts->ogl.bGlTexMerge)
			anim->flags &= ~WCF_ALTFMT;
		else if (!BM_FRAMES (bmP))
			anim->flags &= ~WCF_ALTFMT;
		else {
			anim->flags |= WCF_INITIALIZED;
			BM_CURFRAME (bmP) = BM_FRAMES (bmP) + nFrame;
			OglLoadBmTexture (BM_CURFRAME (bmP), 1, 3, 1);
			nFrame++;
			if (nFrame > nFrames)
				nFrame = nFrames;
			}
		}
	}
else if ((anim->flags & WCF_TMAP1) || !segP->sides [nSide].nOvlTex) {
	segP->sides [nSide].nBaseTex = tmap;
	if (connSegP)
		connSegP->sides [cSide].nBaseTex = tmap;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordWallSetTMapNum1(
			SEG_IDX (segP), (ubyte) nSide, (short) (connSegP ? SEG_IDX (connSegP) : -1), (ubyte) cSide, tmap);
	}
else {
	segP->sides [nSide].nOvlTex = tmap;
	if (connSegP)
		connSegP->sides [cSide].nOvlTex = tmap;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordWallSetTMapNum2(
		SEG_IDX (segP), (ubyte) nSide, (short) (connSegP ? SEG_IDX (connSegP) : -1), (ubyte) cSide, tmap);
	}
segP->sides [nSide].nFrame = -nFrame;
if (connSegP)
	connSegP->sides [cSide].nFrame = -nFrame;
}


// -------------------------------------------------------------------------------
//when the tWall has used all its hitpoints, this will destroy it
void BlastBlastableWall (tSegment *segP, short nSide)
{
	short nConnSide;
	tSegment *connSegP;
	int a, n;
	short nWall, nConnWall;
	tWall *wallP;

nWall = WallNumP (segP, nSide);
Assert(IS_WALL (nWall));
wallP = gameData.walls.walls + nWall;
wallP->hps = -1;	//say it's blasted

if (segP->children [nSide] < 0) {
	if (gameOpts->legacy.bWalls)
		Warning (TXT_BLAST_SINGLE, gameData.segs.segments - segP, nSide, nWall);
	connSegP = NULL;
	nConnSide = -1;
	nConnWall = NO_WALL;
	}
else {
	connSegP = gameData.segs.segments + segP->children [nSide];
	nConnSide = FindConnectedSide (segP, connSegP);
	Assert (nConnSide != -1);
	nConnWall = WallNumP (connSegP, nConnSide);
	}
KillStuckObjects (WallNumP (segP, nSide));
if (IS_WALL (nConnWall))
	KillStuckObjects (nConnWall);

//if this is an exploding wall, explode it
if ((gameData.walls.pAnims [wallP->nClip].flags & WCF_EXPLODES) && !(wallP->flags & WALL_BLASTED))
	ExplodeWall (SEG_IDX (segP), nSide);
else {
	//if not exploding, set final frame, and make door passable
	a = wallP->nClip;
	n = AnimFrameCount (gameData.walls.pAnims + a);
	WallSetTMapNum (segP, nSide, connSegP, nConnSide, a, n - 1);
	wallP->flags |= WALL_BLASTED;
	if (IS_WALL (nConnWall))
		gameData.walls.walls [nConnWall].flags |= WALL_BLASTED;
	}
}

//-----------------------------------------------------------------
// Destroys a blastable tWall.
void WallDestroy (tSegment *segP, short nSide)
{
Assert (IS_WALL (WallNumP (segP, nSide)));
Assert (SEG_IDX (segP) != 0);
if (gameData.walls.walls [WallNumP (segP, nSide)].nType == WALL_BLASTABLE)
	BlastBlastableWall (segP, nSide);
else
	Error (TXT_WALL_INDESTRUCTIBLE);
}

//-----------------------------------------------------------------
// Deteriorate appearance of tWall. (Changes bitmap (paste-ons))
void WallDamage (tSegment *segP, short nSide, fix damage)
{
	int		a, i, n;
	short		nConnSide, nConnWall, nWall = WallNumP (segP, nSide);
	tWall		*wallP;
	tSegment *connSegP;

if (!IS_WALL (nWall)) {
#if TRACE
	con_printf (CONDBG, "Damaging illegal tWall\n");
#endif
	return;
	}
wallP = gameData.walls.walls + nWall;
if (wallP->nType != WALL_BLASTABLE)
	return;
if ((wallP->flags & WALL_BLASTED) || (wallP->hps < 0))
	return;

if (segP->children [nSide] < 0) {
	if (gameOpts->legacy.bWalls)
		Warning (TXT_DMG_SINGLE,
			gameData.segs.segments - segP, nSide, nWall);
	connSegP = NULL;
	nConnSide = -1;
	nConnWall = NO_WALL;
	}
else {
	connSegP = gameData.segs.segments + segP->children [nSide];
	nConnSide = FindConnectedSide (segP, connSegP);
	Assert(nConnSide != -1);
	nConnWall = WallNumP (connSegP, nConnSide);
	}
wallP->hps -= damage;
if (IS_WALL (nConnWall))
	gameData.walls.walls [nConnWall].hps -= damage;
a = wallP->nClip;
n = AnimFrameCount (gameData.walls.pAnims + a);
if (wallP->hps < WALL_HPS * 1 / n) {
	BlastBlastableWall (segP, nSide);
	if (IsMultiGame)
		MultiSendDoorOpen (SEG_IDX (segP), nSide, wallP->flags);
	}
else {
	for (i = 0; i < n; i++)
		if (wallP->hps < WALL_HPS * (n - i) / n)
			WallSetTMapNum (segP, nSide, connSegP, nConnSide, a, i);
	}
}

//-----------------------------------------------------------------
// Opens a door
void WallOpenDoor (tSegment *segP, short nSide)
{
	tWall			*wallP;
	tActiveDoor *doorP;
	short			nConnSide, nWall, nConnWall = NO_WALL;
	tSegment		*connSegP;

nWall = WallNumP (segP, nSide);
Assert(IS_WALL (nWall)); 	//Opening door on illegal tWall
if (!IS_WALL (nWall))
	return;

wallP = gameData.walls.walls + nWall;
//KillStuckObjects(WallNumP (segP, nSide));

if ((wallP->state == WALL_DOOR_OPENING) ||	//already opening
	 (wallP->state == WALL_DOOR_WAITING) ||	//open, waiting to close
	 (wallP->state == WALL_DOOR_OPEN))			//open, & staying open
	return;

if (wallP->state == WALL_DOOR_CLOSING) {		//closing, so reuse door
	doorP = gameData.walls.activeDoors;
	for (int i = 0; i < gameData.walls.nOpenDoors; i++, doorP++) {		//find door
		for (int j = 0; j < doorP->nPartCount; j++)
			if ((doorP->nFrontWall [j] == nWall) || (doorP->nBackWall [j] == nWall))
				goto foundDoor;
		}
	if (IsMultiGame)
		goto fastFix;

foundDoor:

	doorP->time = (fix) (gameData.walls.pAnims [wallP->nClip].xTotalTime * gameStates.gameplay.slowmo [0].fSpeed) - doorP->time;
	if (doorP->time < 0)
		doorP->time = 0;
	}
else {											//create new door
	Assert(wallP->state == WALL_DOOR_CLOSED);

fastFix:

	Assert(gameData.walls.nOpenDoors < MAX_DOORS);
	doorP = gameData.walls.activeDoors + gameData.walls.nOpenDoors++;
	doorP->time = 0;
	}
wallP->state = WALL_DOOR_OPENING;

// So that door can't be shot while opening
if (segP->children [nSide] < 0) {
	if (gameOpts->legacy.bWalls)
		Warning (TXT_OPEN_SINGLE, gameData.segs.segments - segP, nSide, nWall);
	connSegP = NULL;
	nConnSide = -1;
	nConnWall = NO_WALL;
	}
else {
	connSegP = gameData.segs.segments + segP->children [nSide];
	nConnSide = FindConnectedSide (segP, connSegP);
	if (nConnSide >= 0) {
		nConnWall = WallNumP (connSegP, nConnSide);
		if (IS_WALL (nConnWall))
			gameData.walls.walls [nConnWall].state = WALL_DOOR_OPENING;
		}
	}

//KillStuckObjects(WallNumP (connSegP, nConnSide));
doorP->nFrontWall [0] = nWall;
doorP->nBackWall [0] = nConnWall;
Assert(SEG_IDX (segP) != -1);
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordDoorOpening (SEG_IDX (segP), nSide);
if (IS_WALL (wallP->nLinkedWall) && IS_WALL (nConnWall) && (wallP->nLinkedWall == nConnWall)) {
	tWall *wallP2 = gameData.walls.walls + wallP->nLinkedWall;
	tSegment *segP2 = gameData.segs.segments + wallP2->nSegment;
	wallP2->state = WALL_DOOR_OPENING;
	connSegP = gameData.segs.segments + segP2->children [wallP2->nSide];
	nConnSide = FindConnectedSide(segP2, connSegP);
	Assert(nConnSide != -1);
	if (IS_WALL (nConnWall))
		gameData.walls.walls [nConnWall].state = WALL_DOOR_OPENING;
	doorP->nPartCount = 2;
	doorP->nFrontWall [1] = wallP->nLinkedWall;
	doorP->nBackWall [1] = nConnWall;
	}
else
	doorP->nPartCount = 1;
if (gameData.demo.nState != ND_STATE_PLAYBACK) {
	// NOTE THE LINK TO ABOVE!!!!
	vmsVector cp;
	COMPUTE_SIDE_CENTER (&cp, segP, nSide);
	if (gameData.walls.pAnims [wallP->nClip].openSound > -1)
		DigiLinkSoundToPos (gameData.walls.pAnims [wallP->nClip].openSound, SEG_IDX (segP), nSide, &cp, 0, F1_0);
	}
}

//-----------------------------------------------------------------
// start the transition from closed -> open tWall
void StartWallCloak (tSegment *segP, short nSide)
{
	tWall				*wallP;
	tCloakingWall	*cloakWallP;
	short				nConnSide;
	tSegment			*connSegP;
	int				i;
	short				nConnWall;

if (gameData.demo.nState == ND_STATE_PLAYBACK)
	return;
Assert(IS_WALL (WallNumP (segP, nSide))); 	//Opening door on illegal tWall
wallP = gameData.walls.walls + WallNumP (segP, nSide);
if (wallP->nType == WALL_OPEN || wallP->state == WALL_DOOR_CLOAKING)		//already open or cloaking
	return;

connSegP = gameData.segs.segments + segP->children [nSide];
nConnSide = FindConnectedSide (segP, connSegP);
Assert(nConnSide != -1);
nConnWall = WallNumP (connSegP, nConnSide);

if (wallP->state == WALL_DOOR_DECLOAKING) {	//decloaking, so reuse door
		int i;

	for (i = 0, cloakWallP = NULL; i < gameData.walls.nCloaking; i++) {		//find door
		cloakWallP = gameData.walls.cloaking + i;
		if ((cloakWallP->nFrontWall == wallP-gameData.walls.walls) || (cloakWallP->nBackWall == wallP-gameData.walls.walls))
			break;
		}
	Assert(i < gameData.walls.nCloaking);				//didn't find door!
	Assert(cloakWallP != NULL); // Get John!
	cloakWallP->time = (fix) (CLOAKING_WALL_TIME * gameStates.gameplay.slowmo [0].fSpeed) - cloakWallP->time;
	}
else if (wallP->state == WALL_DOOR_CLOSED) {	//create new door
	cloakWallP = gameData.walls.cloaking + gameData.walls.nCloaking;
	cloakWallP->time = 0;
	if (gameData.walls.nCloaking >= MAX_CLOAKING_WALLS) {		//no more!
		Int3();		//ran out of cloaking tWall slots
		wallP->nType = WALL_OPEN;
		if (IS_WALL (nConnWall))
			gameData.walls.walls [nConnWall].nType = WALL_OPEN;
		return;
	}
	gameData.walls.nCloaking++;
	}
else {
	Int3();		//unexpected tWall state
	return;
	}
wallP->state = WALL_DOOR_CLOAKING;
if (IS_WALL (nConnWall))
	gameData.walls.walls [nConnWall].state = WALL_DOOR_CLOAKING;
cloakWallP->nFrontWall = WallNumP (segP, nSide);
cloakWallP->nBackWall = nConnWall;
Assert(SEG_IDX (segP) != -1);
//Assert(!IS_WALL (wallP->nLinkedWall));
if (gameData.demo.nState != ND_STATE_PLAYBACK) {
	vmsVector cp;
	COMPUTE_SIDE_CENTER (&cp, segP, nSide);
	DigiLinkSoundToPos (SOUND_WALL_CLOAK_ON, SEG_IDX (segP), nSide, &cp, 0, F1_0);
	}
for (i = 0; i < 4; i++) {
	cloakWallP->front_ls [i] = segP->sides [nSide].uvls [i].l;
	if (IS_WALL (nConnWall))
		cloakWallP->back_ls [i] = connSegP->sides [nConnSide].uvls [i].l;
	}
}

//-----------------------------------------------------------------
// start the transition from open -> closed tWall
void StartWallDecloak (tSegment *segP, short nSide)
{
	tWall				*wallP;
	tCloakingWall	*cloakWallP;
	short				nConnSide;
	tSegment			*connSegP;
	int				i;
	short				nConnWall;

if (gameData.demo.nState == ND_STATE_PLAYBACK)
	return;
Assert (IS_WALL (WallNumP (segP, nSide))); 	//Opening door on illegal tWall
	wallP = gameData.walls.walls + WallNumP (segP, nSide);
if (wallP->nType == WALL_CLOSED || wallP->state == WALL_DOOR_DECLOAKING)		//already closed or decloaking
	return;
if (wallP->state == WALL_DOOR_CLOAKING) {	//cloaking, so reuse door
	for (i = 0, cloakWallP = NULL; i < gameData.walls.nCloaking; i++) {		//find door
		cloakWallP = gameData.walls.cloaking + i;
		if ((cloakWallP->nFrontWall == wallP-gameData.walls.walls) || (cloakWallP->nBackWall == wallP-gameData.walls.walls))
			break;
		}
	Assert(i < gameData.walls.nCloaking);				//didn't find door!
	Assert(cloakWallP != NULL); // Get John!
	cloakWallP->time = (fix) (CLOAKING_WALL_TIME * gameStates.gameplay.slowmo [0].fSpeed) - cloakWallP->time;
	}
else if (wallP->state == WALL_DOOR_CLOSED) {	//create new door
	cloakWallP = gameData.walls.cloaking + gameData.walls.nCloaking;
	cloakWallP->time = 0;
	if (gameData.walls.nCloaking >= MAX_CLOAKING_WALLS) {		//no more!
		Int3();		//ran out of cloaking tWall slots
		/* what is this _doing_ here?
			wallP->nType = WALL_CLOSED;
			gameData.walls.walls [WallNumP (connSegP, nConnSide)].nType = WALL_CLOSED;
		*/
		return;
		}
	gameData.walls.nCloaking++;
	}
else {
	Int3();		//unexpected tWall state
	return;
	}
wallP->state = WALL_DOOR_DECLOAKING;
// So that door can't be shot while opening
connSegP = gameData.segs.segments + segP->children [nSide];
nConnSide = FindConnectedSide (segP, connSegP);
Assert(nConnSide != -1);
nConnWall = WallNumP (connSegP, nConnSide);
if (IS_WALL (nConnWall))
	gameData.walls.walls [nConnWall].state = WALL_DOOR_DECLOAKING;
cloakWallP->nFrontWall = WallNumP (segP, nSide);
cloakWallP->nBackWall = WallNumP (connSegP, nConnSide);
Assert(SEG_IDX (segP) != -1);
Assert(!IS_WALL (wallP->nLinkedWall));
if (gameData.demo.nState != ND_STATE_PLAYBACK) {
	vmsVector cp;
	COMPUTE_SIDE_CENTER(&cp, segP, nSide);
	DigiLinkSoundToPos(SOUND_WALL_CLOAK_OFF, SEG_IDX (segP), nSide, &cp, 0, F1_0);
	}
for (i = 0; i < 4; i++) {
	cloakWallP->front_ls [i] = segP->sides [nSide].uvls [i].l;
	if (IS_WALL (nConnWall))
		cloakWallP->back_ls [i] = connSegP->sides [nConnSide].uvls [i].l;
	}
}

//-----------------------------------------------------------------

void DeleteActiveDoor (int nDoor)
{
if (--gameData.walls.nOpenDoors > nDoor)
	memcpy (gameData.walls.activeDoors + nDoor,
			  gameData.walls.activeDoors + nDoor + 1,
			  (gameData.walls.nOpenDoors - nDoor) * sizeof (gameData.walls.activeDoors [0]));
}

//-----------------------------------------------------------------
// This function closes the specified door and restores the closed
//  door texture.  This is called when the animation is done
void WallCloseDoorNum (int nDoor)
{
	int p;
	tActiveDoor *d;
	short nConnWall;

d = gameData.walls.activeDoors + nDoor;
for (p = 0; p < d->nPartCount; p++) {
		tWall		*w = gameData.walls.walls + d->nFrontWall [p];
		short		nConnSide, nSide = w->nSide;
		tSegment *segP = gameData.segs.segments + w->nSegment,
					*connSegP = gameData.segs.segments + segP->children [nSide];

	nConnSide = FindConnectedSide(segP, connSegP);
	nConnWall = WallNumP (connSegP, nConnSide);
	gameData.walls.walls [WallNumP (segP, nSide)].state = WALL_DOOR_CLOSED;
	if (IS_WALL (nConnWall)) {
		gameData.walls.walls [nConnWall].state = WALL_DOOR_CLOSED;
		WallSetTMapNum (connSegP, nConnSide, NULL, -1,
							 gameData.walls.walls [WallNumP (connSegP, nConnSide)].nClip, 0);
		}
	WallSetTMapNum (segP, nSide, NULL, -1, w->nClip, 0);
	}
DeleteActiveDoor (nDoor);
}

//-----------------------------------------------------------------

int CheckPoke (int nObject, int nSegment, short nSide)
{
tObject *objP = OBJECTS + nObject;

	//note: don't let OBJECTS with zero size block door
if (nObject == 126)
	nObject = nObject;
if (objP->size && GetSideMasks (&objP->position.vPos, nSegment, nSide, objP->size).sideMask)
	return 1;		//pokes through nSide!
return 0;		//does not!
}

//-----------------------------------------------------------------
//returns true of door in unobjstructed (& thus can close)
int DoorIsBlocked (tSegment *segP, short nSide)
{
	short		nConnSide;
	tSegment *connSegP;
	short		nObject, t;

connSegP = gameData.segs.segments + segP->children [nSide];
nConnSide = FindConnectedSide (segP, connSegP);
Assert(nConnSide != -1);

//go through each tObject in each of two segments, and see if
//it pokes into the connecting segP

for (nObject = gameData.segs.objects [SEG_IDX (segP)]; nObject != -1; nObject = OBJECTS [nObject].next) {
	t = OBJECTS [nObject].nType;
	if ((t == OBJ_WEAPON) || (t == OBJ_FIREBALL) || (t == OBJ_EXPLOSION) || (t == OBJ_EFFECT))
		continue;
	if (CheckPoke (nObject, SEG_IDX (segP), nSide) || CheckPoke (nObject, SEG_IDX (connSegP), nConnSide))
		return 1;	//not D2_FREE
		}
return 0; 	//doorway is D2_FREE!
}

//-----------------------------------------------------------------
// Closes a door
void WallCloseDoor (tSegment *segP, short nSide)
{
	tWall *wallP;
	tActiveDoor *doorP;
	short nConnSide, nWall, nConnWall;
	tSegment *connSegP;

Assert(IS_WALL (WallNumP (segP, nSide))); 	//Opening door on illegal tWall

wallP = gameData.walls.walls + WallNumP (segP, nSide);
nWall = WALL_IDX (wallP);
if ((wallP->state == WALL_DOOR_CLOSING) ||		//already closing
	 (wallP->state == WALL_DOOR_WAITING) ||		//open, waiting to close
	 (wallP->state == WALL_DOOR_CLOSED))			//closed
	return;

if (DoorIsBlocked (segP,nSide))
	return;
if (wallP->state == WALL_DOOR_OPENING) {	//reuse door
	int i;
	doorP = gameData.walls.activeDoors;
	for (i = 0; i <gameData.walls.nOpenDoors; i++, doorP++) {		//find door
		if ((doorP->nFrontWall [0]== nWall) || (doorP->nBackWall [0] == nWall) ||
			((doorP->nPartCount == 2) && ((doorP->nFrontWall [1] == nWall) || (doorP->nBackWall [1] == nWall))))
			break;
		}
	if (i >= gameData.walls.nOpenDoors)	//no matching open door found
		return;
	Assert(doorP != NULL); // Get John!
	doorP->time = (fix) (gameData.walls.pAnims [wallP->nClip].xTotalTime * gameStates.gameplay.slowmo [0].fSpeed) - doorP->time;
	if (doorP->time < 0)
		doorP->time = 0;
	}
else {											//create new door
	Assert(wallP->state == WALL_DOOR_OPEN);
	doorP = gameData.walls.activeDoors + gameData.walls.nOpenDoors;
	doorP->time = 0;
	gameData.walls.nOpenDoors++;
	Assert(gameData.walls.nOpenDoors < MAX_DOORS);
	}
wallP->state = WALL_DOOR_CLOSING;
// So that door can't be shot while opening
connSegP = gameData.segs.segments + segP->children [nSide];
nConnSide = FindConnectedSide (segP, connSegP);
Assert(nConnSide != -1);
nConnWall = WallNumP (connSegP, nConnSide);
if (IS_WALL (nConnWall))
	gameData.walls.walls [nConnWall].state = WALL_DOOR_CLOSING;
doorP->nFrontWall [0] = WallNumP (segP, nSide);
doorP->nBackWall [0] = nConnWall;
Assert(SEG_IDX (segP) != -1);
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordDoorOpening(SEG_IDX (segP), nSide);
if (IS_WALL (wallP->nLinkedWall))
	Int3();		//don't think we ever used linked walls
else
	doorP->nPartCount = 1;
if (gameData.demo.nState != ND_STATE_PLAYBACK) {
	// NOTE THE LINK TO ABOVE!!!!
	vmsVector cp;
	COMPUTE_SIDE_CENTER (&cp, segP, nSide);
	if (gameData.walls.pAnims [wallP->nClip].openSound > -1)
		DigiLinkSoundToPos (gameData.walls.pAnims [wallP->nClip].openSound, SEG_IDX (segP), nSide, &cp, 0, F1_0);
	}
}

//-----------------------------------------------------------------

int AnimateOpeningDoor (tSegment *segP, short nSide, fix xElapsedTime)
{
	tWall	*wallP;
	int	i, nFrames, nWall;
	fix	xTotalTime, xFrameTime;

if (!segP || (nSide < 0))
	return 3;
nWall = WallNumP (segP, nSide);
Assert(IS_WALL (nWall));		//Trying to DoDoorOpen on illegal tWall
wallP = gameData.walls.walls + nWall;
nFrames = AnimFrameCount (gameData.walls.pAnims + wallP->nClip);
if (!nFrames)
	return 3;
xTotalTime = (fix) (gameData.walls.pAnims [wallP->nClip].xTotalTime * gameStates.gameplay.slowmo [0].fSpeed);
xFrameTime = xTotalTime / nFrames;
i = (xElapsedTime < 0) ? nFrames : xElapsedTime / xFrameTime;
if (i < nFrames)
	WallSetTMapNum (segP, nSide, NULL, -1, wallP->nClip, i);
if (i > nFrames / 2)
	wallP->flags |= WALL_DOOR_OPENED;
if (i >= nFrames - 1) {
	WallSetTMapNum (segP, nSide, NULL, -1, wallP->nClip, nFrames - 1);
	// If our door is not automatic just remove it from the list.
	if (!(wallP->flags & WALL_DOOR_AUTO)) {
		wallP->state = WALL_DOOR_OPEN;
		return 1;
		}
	else {
		wallP->state = WALL_DOOR_WAITING;
		return 2;
		}
	}
return 0;
}

//-----------------------------------------------------------------
// Animates opening of a door.
// Called in the game loop.
void DoDoorOpen (int nDoor)
{
	tActiveDoor *doorP;
	tWall			*wallP;
	short			cSide, nSide;
	tSegment		*connSegP, *segP;
	int			i, nWall, bFlags = 3;

if (nDoor < -1)
	return;
doorP = gameData.walls.activeDoors + nDoor;
for (i = 0; i < doorP->nPartCount; i++) {
	doorP->time += gameData.time.xFrame;
	nWall = doorP->nFrontWall [i];
	if (!IS_WALL (nWall)) {
		PrintLog ("Trying to open non existant door\n");
		continue;
		}
	wallP = gameData.walls.walls + doorP->nFrontWall [i];
	KillStuckObjects (doorP->nFrontWall [i]);
	KillStuckObjects (doorP->nBackWall [i]);

	segP = gameData.segs.segments + wallP->nSegment;
	nSide = wallP->nSide;
	nWall = WallNumP (segP, nSide);
	if (!IS_WALL (nWall)) {
		PrintLog ("Trying to open non existant door @ %d,%d\n", wallP->nSegment, nSide);
		continue;
		}
	bFlags &= AnimateOpeningDoor (segP, nSide, doorP->time);
	connSegP = gameData.segs.segments + segP->children [nSide];
	cSide = FindConnectedSide (segP, connSegP);
	if (cSide < 0) {
		PrintLog ("Door %d @ %d,%d has no oppposite door in %d\n", nWall, wallP->nSegment, nSide, segP->children [nSide]);
		continue;
		}
	if (IS_WALL (WallNumP (connSegP, cSide)))
		bFlags &= AnimateOpeningDoor (connSegP, cSide, doorP->time);
	}
if (bFlags & 2)
	gameData.walls.activeDoors [gameData.walls.nOpenDoors].time = 0;	//counts up
if (bFlags & 1)
	DeleteActiveDoor (nDoor);
}

//-----------------------------------------------------------------

int AnimateClosingDoor (tSegment *segP, short nSide, fix xElapsedTime)
{
	tWall	*wallP;
	int	i, nFrames, nWall;
	fix	xTotalTime, xFrameTime;

if (!segP || (nSide < 0))
	return 3;
nWall = WallNumP (segP, nSide);
Assert(IS_WALL (nWall));		//Trying to DoDoorOpen on illegal tWall
wallP = gameData.walls.walls + nWall;
nFrames = AnimFrameCount (gameData.walls.pAnims + wallP->nClip);
if (!nFrames)
	return 0;
xTotalTime = (fix) (gameData.walls.pAnims [wallP->nClip].xTotalTime * gameStates.gameplay.slowmo [0].fSpeed);
xFrameTime = xTotalTime / nFrames;
i = nFrames - xElapsedTime / xFrameTime - 1;
if (i < nFrames / 2)
	wallP->flags &= ~WALL_DOOR_OPENED;
if (i <= 0)
	return 1;
WallSetTMapNum (segP, nSide, NULL, -1, wallP->nClip, i);
wallP->state = WALL_DOOR_CLOSING;
return 0;
}

//-----------------------------------------------------------------
// Animates and processes the closing of a door.
// Called from the game loop.
void DoDoorClose (int nDoor)
{
	tActiveDoor *doorP;
	tWall			*wallP;
	int			i, bFlags = 1;
	short			cSide, nSide;
	tSegment		*connSegP, *segP;


if (nDoor < -1)
	return;
doorP = gameData.walls.activeDoors + nDoor;
wallP = gameData.walls.walls + doorP->nFrontWall [0];

	//check for OBJECTS in doorway before closing
if (wallP->flags & WALL_DOOR_AUTO)
	if (DoorIsBlocked (gameData.segs.segments + wallP->nSegment,(short) wallP->nSide)) {
		DigiKillSoundLinkedToSegment ((short) wallP->nSegment,(short) wallP->nSide,-1);
		WallOpenDoor (&gameData.segs.segments [wallP->nSegment],(short) wallP->nSide);		//re-open door
		return;
		}

for (i = 0; i < doorP->nPartCount; i++) {
	wallP = gameData.walls.walls + doorP->nFrontWall [i];
	segP = gameData.segs.segments + wallP->nSegment;
	nSide = wallP->nSide;
	if (!IS_WALL (WallNumP (segP, nSide))) {
#ifdef _DEBUG
		PrintLog ("Trying to close non existant door\n");
#endif
		continue;
		}
	//if here, must be auto door
	//Assert(gameData.walls.walls [WallNumP (segP, nSide)].flags & WALL_DOOR_AUTO);
	//don't assert here, because now we have triggers to close non-auto doors
	// Otherwise, close it.
	connSegP = gameData.segs.segments + segP->children [nSide];
	cSide = FindConnectedSide (segP, connSegP);
	if (cSide < 0) {
		PrintLog ("Door %d @ %d,%d has no oppposite door in %d\n", WallNumP (segP, nSide), wallP->nSegment, nSide, segP->children [nSide]);
		continue;
		}
	if ((gameData.demo.nState != ND_STATE_PLAYBACK) && !(i || doorP->time)) {
		if (gameData.walls.pAnims [wallP->nClip].closeSound  > -1)
			DigiLinkSoundToPos ((short) gameData.walls.pAnims [gameData.walls.walls [WallNumP (segP, nSide)].nClip].closeSound,
									  SEG_IDX (segP), nSide, SIDE_CENTER_I (wallP->nSegment, nSide), 0, F1_0);
		}
	doorP->time += gameData.time.xFrame;
	bFlags &= AnimateClosingDoor (segP, nSide, doorP->time);
	if (IS_WALL (WallNumP (connSegP, cSide)))
		bFlags &= AnimateClosingDoor (connSegP, cSide, doorP->time);
	}
if (bFlags & 1)
	WallCloseDoorNum (nDoor);
else
	gameData.walls.activeDoors [gameData.walls.nOpenDoors].time = 0;		//counts up
}


//-----------------------------------------------------------------
// Turns off an illusionary tWall (This will be used primarily for
//  tWall switches or triggers that can turn on/off illusionary walls.)
void WallIllusionOff (tSegment *segP, short nSide)
{
	tSegment *connSegP;
	short		cSide, nWall;

if (segP->children [nSide] < 0) {
	if (gameOpts->legacy.bWalls)
		Warning (TXT_ILLUS_SINGLE,	gameData.segs.segments - segP, nSide);
	connSegP = NULL;
	cSide = -1;
	}
else {
	connSegP = gameData.segs.segments+segP->children [nSide];
	cSide = FindConnectedSide(segP, connSegP);
	Assert(cSide != -1);
	}

nWall = WallNumP (segP, nSide);
if (!IS_WALL (nWall)) {
#if TRACE
	con_printf (CONDBG, "Trying to shut off non existant illusion\n");
#endif
	return;
	}

gameData.walls.walls [nWall].flags |= WALL_ILLUSION_OFF;
KillStuckObjects (nWall);
if (connSegP) {
	nWall = WallNumP (connSegP, cSide);
	if (IS_WALL (nWall)) {
		gameData.walls.walls [nWall].flags |= WALL_ILLUSION_OFF;
		KillStuckObjects (nWall);
		}
	}
}

//-----------------------------------------------------------------
// Turns on an illusionary tWall (This will be used primarily for
//  tWall switches or triggers that can turn on/off illusionary walls.)
void WallIllusionOn (tSegment *segP, short nSide)
{
	tSegment *connSegP;
	short		cSide, nWall;

if (segP->children [nSide] < 0) {
	if (gameOpts->legacy.bWalls)
		Warning (TXT_ILLUS_SINGLE, gameData.segs.segments - segP, nSide);
	connSegP = NULL;
	cSide = -1;
	}
else {
	connSegP = gameData.segs.segments+segP->children [nSide];
	cSide = FindConnectedSide(segP, connSegP);
	Assert(cSide != -1);
	}
nWall = WallNumP (segP, nSide);
if (!IS_WALL (nWall)) {
#if TRACE
	con_printf (CONDBG, "Trying to turn on illusion illegal tWall\n");
#endif
	return;
	}
//gameData.walls.walls [nWall].nType = WALL_ILLUSION;
gameData.walls.walls [nWall].flags &= ~WALL_ILLUSION_OFF;
if (connSegP) {
	nWall = WallNumP (connSegP, cSide);
	if (IS_WALL (nWall))
		gameData.walls.walls [nWall].flags &= ~WALL_ILLUSION_OFF;
	}
}

//	-----------------------------------------------------------------------------
//	Allowed to open the normally locked special boss door if in multiplayer mode.
int AllowToOpenSpecialBossDoor (int nSegment, short nSide)
{
if (IsMultiGame)
	return (gameData.missions.nCurrentLevel == BOSS_LOCKED_DOOR_LEVEL) &&
			 (nSegment == BOSS_LOCKED_DOOR_SEG) &&
			 (nSide == BOSS_LOCKED_DOOR_SIDE);
return 0;
}

//-----------------------------------------------------------------

void UnlockAllWalls (int bOnlyDoors)
{
	int	i;
	tWall	*wallP;

for (i = gameData.walls.nWalls, wallP = gameData.walls.walls; i; wallP++, i--) {
	if (wallP->nType == WALL_DOOR) {
		wallP->flags &= ~WALL_DOOR_LOCKED;
		wallP->keys = 0;
		}
	else if (!bOnlyDoors
#ifndef _DEBUG
				&& (wallP->nType == WALL_CLOSED)
#endif
			 )
		wallP->nType = WALL_OPEN;
	}
}

//-----------------------------------------------------------------
// set up renderer for alternative highres animation method
// (all frames are stored in one texture, and struct nSide.nFrame
// holds the frame index).

void InitDoorAnims (void)
{
	int			h, i;
	tWall			*wallP;
	tSegment		*segP;
	tWallClip	*animP;

for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; wallP++, i++) {
	if (wallP->nType == WALL_DOOR) {
		animP = gameData.walls.pAnims + wallP->nClip;
		if (!(animP->flags & WCF_ALTFMT))
			continue;
		h = (animP->flags & WCF_TMAP1) ? -1 : 1;
		segP = gameData.segs.segments + wallP->nSegment;
		segP->sides [wallP->nSide].nFrame = h;
		}
	}
}

//-----------------------------------------------------------------
// Determines what happens when a tWall is shot
//returns info about tWall.  see wall[HA] for codes
//obj is the tObject that hit...either a weapon or the tPlayer himself
//nPlayer is the number the tPlayer who hit the tWall or fired the weapon,
//or -1 if a robot fired the weapon
int WallHitProcess (tSegment *segP, short nSide, fix damage, int nPlayer, tObject *objP)
{
	tWall	*wallP;
	short	nWall;
	fix	bShowMessage;

Assert (SEG_IDX (segP) != -1);

// If it is not a "tWall" then just return.
nWall = WallNumP (segP, nSide);
if (!IS_WALL (nWall))
	return WHP_NOT_SPECIAL;

wallP = gameData.walls.walls + nWall;

if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordWallHitProcess (SEG_IDX (segP), nSide, damage, nPlayer);

if (wallP->nType == WALL_BLASTABLE) {
	if (objP->cType.laserInfo.parentType == OBJ_PLAYER)
		WallDamage (segP, nSide, damage);
	return WHP_BLASTABLE;
	}

if (nPlayer != gameData.multiplayer.nLocalPlayer)	//return if was robot fire
	return WHP_NOT_SPECIAL;

Assert(nPlayer > -1);

//	Determine whether tPlayer is moving forward.  If not, don't say negative
//	messages because he probably didn't intentionally hit the door.
if (objP->nType == OBJ_PLAYER)
	bShowMessage = (vmsVector::Dot(objP->position.mOrient[FVEC], objP->mType.physInfo.velocity) > 0);
else if (objP->nType == OBJ_ROBOT)
	bShowMessage = 0;
else if ((objP->nType == OBJ_WEAPON) && (objP->cType.laserInfo.parentType == OBJ_ROBOT))
	bShowMessage = 0;
else
	bShowMessage = 1;

if (wallP->keys == KEY_BLUE) {
	if (!(gameData.multiplayer.players [nPlayer].flags & PLAYER_FLAGS_BLUE_KEY)) {
		if (bShowMessage && (nPlayer == gameData.multiplayer.nLocalPlayer))
			HUDInitMessage("%s %s",TXT_BLUE,TXT_ACCESS_DENIED);
		return WHP_NO_KEY;
		}
	}
else if (wallP->keys == KEY_RED) {
	if (!(gameData.multiplayer.players [nPlayer].flags & PLAYER_FLAGS_RED_KEY)) {
		if (bShowMessage && (nPlayer == gameData.multiplayer.nLocalPlayer))
			HUDInitMessage("%s %s",TXT_RED,TXT_ACCESS_DENIED);
		return WHP_NO_KEY;
		}
	}
else if (wallP->keys == KEY_GOLD) {
	if (!(gameData.multiplayer.players [nPlayer].flags & PLAYER_FLAGS_GOLD_KEY)) {
		if (bShowMessage && (nPlayer == gameData.multiplayer.nLocalPlayer))
			HUDInitMessage("%s %s",TXT_YELLOW,TXT_ACCESS_DENIED);
		return WHP_NO_KEY;
		}
	}
if (wallP->nType == WALL_DOOR) {
	if ((wallP->flags & WALL_DOOR_LOCKED) && !(AllowToOpenSpecialBossDoor (SEG_IDX (segP), nSide))) {
		if (bShowMessage && (nPlayer == gameData.multiplayer.nLocalPlayer))
			HUDInitMessage(TXT_CANT_OPEN_DOOR);
		return WHP_NO_KEY;
		}
	else {
		if (wallP->state != WALL_DOOR_OPENING) {
			WallOpenDoor(segP, nSide);
			if (IsMultiGame)
				MultiSendDoorOpen (SEG_IDX (segP), nSide,wallP->flags);
			}
		return WHP_DOOR;
		}
	}
return WHP_NOT_SPECIAL;		//default is treat like Normal tWall
}

//-----------------------------------------------------------------
// Opens doors/destroys tWall/shuts off triggers.
void WallToggle (tSegment *segP, short nSide)
{
	tWall	*wallP;
	int	nWall;

if (SEG_IDX (segP) > gameData.segs.nLastSegment) {
#ifdef _DEBUG
	Warning("Can't toggle nSide %d of tSegment %d - nonexistent tSegment!\n", nSide, SEG_IDX (segP));
#endif
	return;
	}
nWall = WallNumP (segP, nSide);
if (!IS_WALL (nWall)) {
#if TRACE
 	con_printf (CONDBG, "Illegal WallToggle\n");
#endif
	return;
	}
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordWallToggle (SEG_IDX (segP), nSide);
wallP = gameData.walls.walls + nWall;
if (wallP->nType == WALL_BLASTABLE)
	WallDestroy (segP, nSide);
if ((wallP->nType == WALL_DOOR) && (wallP->state == WALL_DOOR_CLOSED))
	WallOpenDoor (segP, nSide);
}

//-----------------------------------------------------------------
// Tidy up gameData.walls.walls array for load/save purposes.
void ResetWalls()
{
	int	i;
	tWall *pWall = gameData.walls.walls+gameData.walls.nWalls;

if (gameData.walls.nWalls < 0) {
#if TRACE
	con_printf (CONDBG, "Illegal gameData.walls.nWalls\n");
#endif
	return;
	}

for (i = gameData.walls.nWalls; i < MAX_WALLS; i++, pWall++) {
	pWall->nType = WALL_NORMAL;
	pWall->flags = 0;
	pWall->hps = 0;
	pWall->nTrigger = -1;
	pWall->nClip = -1;
	}
}

//-----------------------------------------------------------------

void DoCloakingWallFrame (int nCloakingWall)
{
	tCloakingWall	*cloakWallP;
	tWall				*frontWallP,*backWallP;

if (gameData.demo.nState == ND_STATE_PLAYBACK)
	return;
cloakWallP = gameData.walls.cloaking + nCloakingWall;
frontWallP = gameData.walls.walls + cloakWallP->nFrontWall;
backWallP = IS_WALL (cloakWallP->nBackWall) ? gameData.walls.walls + cloakWallP->nBackWall : NULL;
cloakWallP->time += gameData.time.xFrame;
if (cloakWallP->time > CLOAKING_WALL_TIME) {
	frontWallP->nType = WALL_OPEN;
	if (SHOW_DYN_LIGHT)
		ToggleDynLight (frontWallP->nSegment, frontWallP->nSide, -1, 0);
	frontWallP->state = WALL_DOOR_CLOSED;		//why closed? why not?
	if (backWallP) {
		backWallP->nType = WALL_OPEN;
		if (SHOW_DYN_LIGHT)
			ToggleDynLight (backWallP->nSegment, backWallP->nSide, -1, 0);
		backWallP->state = WALL_DOOR_CLOSED;		//why closed? why not?
		}
	if (nCloakingWall < --gameData.walls.nCloaking)
		memcpy (gameData.walls.cloaking + nCloakingWall,
				  gameData.walls.cloaking + nCloakingWall + 1,
				  (gameData.walls.nCloaking - nCloakingWall) * sizeof (*gameData.walls.cloaking));
	}
else if (SHOW_DYN_LIGHT || (cloakWallP->time > CLOAKING_WALL_TIME / 2)) {
	int oldType = frontWallP->nType;
	if (SHOW_DYN_LIGHT)
		frontWallP->cloakValue = (GR_ACTUAL_FADE_LEVELS * (CLOAKING_WALL_TIME - cloakWallP->time)) / CLOAKING_WALL_TIME;
	else
		frontWallP->cloakValue = ((cloakWallP->time - CLOAKING_WALL_TIME / 2) * (GR_ACTUAL_FADE_LEVELS - 2)) / (CLOAKING_WALL_TIME / 2);
	if (backWallP)
		backWallP->cloakValue = frontWallP->cloakValue;
	if (oldType != WALL_CLOAKED) {		//just switched
		int i;
		frontWallP->nType = WALL_CLOAKED;
		if (backWallP)
			backWallP->nType = WALL_CLOAKED;
		for (i = 0; i < 4; i++) {
			gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls [i].l = cloakWallP->front_ls [i];
			if (backWallP)
				gameData.segs.segments [backWallP->nSegment].sides [backWallP->nSide].uvls [i].l = cloakWallP->back_ls [i];
			}
		}
	}
else {		//fading out
	fix xLightScale = FixDiv (CLOAKING_WALL_TIME / 2 - cloakWallP->time, CLOAKING_WALL_TIME / 2);
	for (int i = 0; i < 4; i++) {
		gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls [i].l = FixMul (cloakWallP->front_ls [i], xLightScale);
		if (backWallP)
			gameData.segs.segments [backWallP->nSegment].sides [backWallP->nSide].uvls [i].l = FixMul (cloakWallP->back_ls [i], xLightScale);
		}
	if (gameData.demo.nState == ND_STATE_RECORDING) {
		tUVL *uvlP = gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls;
		NDRecordCloakingWall (cloakWallP->nFrontWall, cloakWallP->nBackWall, frontWallP->nType, frontWallP->state, frontWallP->cloakValue,
									 uvlP [0].l, uvlP [1].l, uvlP [2].l, uvlP [3].l);
		}
	}
}

//-----------------------------------------------------------------

void DoDecloakingWallFrame (int nCloakingWall)
{
	tCloakingWall	*cloakWallP;
	tWall				*frontWallP,*backWallP;

if (gameData.demo.nState == ND_STATE_PLAYBACK)
return;

cloakWallP = gameData.walls.cloaking + nCloakingWall;
frontWallP = gameData.walls.walls + cloakWallP->nFrontWall;
backWallP = IS_WALL (cloakWallP->nBackWall) ? gameData.walls.walls + cloakWallP->nBackWall : NULL;

cloakWallP->time += gameData.time.xFrame;
if (cloakWallP->time > CLOAKING_WALL_TIME) {
	int i;
	frontWallP->state = WALL_DOOR_CLOSED;
	if (backWallP)
	backWallP->state = WALL_DOOR_CLOSED;
	for (i = 0;i < 4; i++) {
		gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls [i].l = cloakWallP->front_ls [i];
		if (backWallP)
			gameData.segs.segments [backWallP->nSegment].sides [backWallP->nSide].uvls [i].l = cloakWallP->back_ls [i];
		}

	for (i = nCloakingWall; i < gameData.walls.nCloaking; i++)
		gameData.walls.cloaking [i] = gameData.walls.cloaking [i+1];
	gameData.walls.nCloaking--;
	}
else if (cloakWallP->time > CLOAKING_WALL_TIME/2) {		//fading in
	frontWallP->nType = WALL_CLOSED;
	if (SHOW_DYN_LIGHT)
		ToggleDynLight (frontWallP->nSegment, frontWallP->nSide, -1, 0);
	if (backWallP) {
		backWallP->nType = WALL_CLOSED;
		if (SHOW_DYN_LIGHT)
			ToggleDynLight (backWallP->nSegment, backWallP->nSide, -1, 0);
		}
	fix xLightScale = FixDiv(cloakWallP->time-CLOAKING_WALL_TIME/2,CLOAKING_WALL_TIME/2);
	for (int i = 0; i < 4; i++) {
		gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls [i].l = FixMul(cloakWallP->front_ls [i],xLightScale);
		if (backWallP)
			gameData.segs.segments [backWallP->nSegment].sides [backWallP->nSide].uvls [i].l = FixMul(cloakWallP->back_ls [i],xLightScale);
		}
	}
else {		//cloaking in
	frontWallP->cloakValue = ((CLOAKING_WALL_TIME/2 - cloakWallP->time) * (GR_ACTUAL_FADE_LEVELS-2)) / (CLOAKING_WALL_TIME/2);
	frontWallP->nType = WALL_CLOAKED;
	if (backWallP) {
		backWallP->cloakValue = frontWallP->cloakValue;
		backWallP->nType = WALL_CLOAKED;
		}
	}
if (gameData.demo.nState == ND_STATE_RECORDING) {
	tUVL *uvlP = gameData.segs.segments [frontWallP->nSegment].sides [frontWallP->nSide].uvls;
	NDRecordCloakingWall (cloakWallP->nFrontWall, cloakWallP->nBackWall, frontWallP->nType, frontWallP->state, frontWallP->cloakValue,
								 uvlP [0].l, uvlP [1].l, uvlP [2].l, uvlP [3].l);
	}
}

//-----------------------------------------------------------------

void WallFrameProcess (void)
{
	int				i;
	tCloakingWall	*cloakWallP;
	tActiveDoor		*doorP = gameData.walls.activeDoors;
	tWall				*wallP, *backWallP;

for (i = 0; i < gameData.walls.nOpenDoors; i++, doorP++) {
	backWallP = NULL,
	wallP = gameData.walls.walls + doorP->nFrontWall [0];
	if (wallP->state == WALL_DOOR_OPENING)
		DoDoorOpen (i);
	else if (wallP->state == WALL_DOOR_CLOSING)
		DoDoorClose (i);
	else if (wallP->state == WALL_DOOR_WAITING) {
		doorP->time += gameData.time.xFrame;

		//set flags to fix occatsional netgame problem where door is
		//waiting to close but open flag isn't set
//			Assert(doorP->nPartCount == 1);
		if (IS_WALL (doorP->nBackWall [0]))
			gameData.walls.walls [doorP->nBackWall [0]].state = WALL_DOOR_CLOSING;
		if ((doorP->time > DOOR_WAIT_TIME) &&
			 !DoorIsBlocked (gameData.segs.segments + wallP->nSegment, (short) wallP->nSide)) {
			wallP->state = WALL_DOOR_CLOSING;
			doorP->time = 0;
			}
		else {
			wallP->flags |= WALL_DOOR_OPENED;
			if (backWallP)
				backWallP->flags |= WALL_DOOR_OPENED;
			}
		}
	else if (wallP->state == WALL_DOOR_CLOSED || wallP->state == WALL_DOOR_OPEN) {
		//this shouldn't happen.  if the tWall is in one of these states,
		//there shouldn't be an activedoor entry for it.  So we'll kill
		//the activedoor entry.  Tres simple.
		for (int t = i; t < gameData.walls.nOpenDoors; t++)
			gameData.walls.activeDoors [t] = gameData.walls.activeDoors [t+1];
		gameData.walls.nOpenDoors--;
		}
	}

cloakWallP = gameData.walls.cloaking;
for (i = 0; i < gameData.walls.nCloaking; i++, cloakWallP++) {
	ubyte s = gameData.walls.walls [cloakWallP->nFrontWall].state;
	if (s == WALL_DOOR_CLOAKING)
		DoCloakingWallFrame (i);
	else if (s == WALL_DOOR_DECLOAKING)
		DoDecloakingWallFrame (i);
#ifdef _DEBUG
	else
		Int3();	//unexpected tWall state
#endif
	}
}

int	nStuckObjects=0;

tStuckObject	stuckObjects [MAX_STUCK_OBJECTS];

//-----------------------------------------------------------------
//	An tObject got stuck in a door (like a flare).
//	Add global entry.
void AddStuckObject(tObject *objP, short nSegment, short nSide)
{
	int	i;
	short	nWall;
	tStuckObject	*sto;

	nWall = WallNumI (nSegment, nSide);

	if (IS_WALL (nWall)) {
		if (gameData.walls.walls [nWall].flags & WALL_BLASTED)
			KillObject (objP);

		for (i=0, sto = stuckObjects; i<MAX_STUCK_OBJECTS; i++, sto++) {
			if (sto->nWall == -1) {
				sto->nWall = nWall;
				sto->nObject = OBJ_IDX (objP);
				sto->nSignature = objP->nSignature;
				nStuckObjects++;
				break;
			}
		}
#if TRACE
		if (i == MAX_STUCK_OBJECTS)
			con_printf (1,
				"Warning: Unable to add tObject %i which got stuck in tWall %i to stuckObjects\n",
				OBJ_IDX (objP), nWall);
#endif
	}



}

//	--------------------------------------------------------------------------------------------------
//	Look at the list of stuck OBJECTS, clean up in case an tObject has gone away, but not been removed here.
//	Removes up to one/frame.
void RemoveObsoleteStuckObjects (void)
{
	int	nObject, nWall;

	//	Safety and efficiency code.  If no stuck OBJECTS, should never get inside the IF, but this is faster.
if (!nStuckObjects)
	return;
nObject = gameData.app.nFrameCount % MAX_STUCK_OBJECTS;
nWall = stuckObjects [nObject].nWall;
if (IS_WALL (nWall)) {
	if ((gameData.walls.walls [nWall].state != WALL_DOOR_CLOSED) ||
		 (OBJECTS [stuckObjects [nObject].nObject].nSignature != stuckObjects [nObject].nSignature)) {
		nStuckObjects--;
		OBJECTS [stuckObjects [nObject].nObject].lifeleft = F1_0/8;
		stuckObjects [nObject].nWall = -1;
		}
	}
}

//	----------------------------------------------------------------------------------------------------
//	Door with tWall index nWall is opening, kill all OBJECTS stuck in it.
void KillStuckObjects (int nWall)
{
	int	i;
	tStuckObject	*stuckObjP;

if (!IS_WALL (nWall) || (nStuckObjects == 0))
	return;
nStuckObjects = 0;

for (i = 0, stuckObjP = stuckObjects; i < MAX_STUCK_OBJECTS; i++, stuckObjP++)
	if (stuckObjP->nWall == nWall) {
		if (OBJECTS [stuckObjP->nObject].nType == OBJ_WEAPON)
			OBJECTS [stuckObjP->nObject].lifeleft = F1_0/8;
		else {
#if TRACE
			con_printf (1,
				"Warning: Stuck tObject of nType %i, expected to be of nType %i, see tWall.c\n",
				OBJECTS [stuckObjects [i].nObject].nType, OBJ_WEAPON);
#endif
			// Int3();	//	What?  This looks bad.  Object is not a weapon and it is stuck in a tWall!
			stuckObjP->nWall = -1;
		}
	}
else if (IS_WALL (stuckObjects [i].nWall)) {
	nStuckObjects++;
	}
//	Ok, this is awful, but we need to do things whenever a door opens/closes/disappears, etc.
FlushFCDCache();
}

// -----------------------------------------------------------------------------------
// Initialize stuck OBJECTS array.  Called at start of level
void InitStuckObjects (void)
{
for (int i = 0; i < MAX_STUCK_OBJECTS; i++)
	stuckObjects [i].nWall = -1;
nStuckObjects = 0;
}

// -----------------------------------------------------------------------------------
// Clear out all stuck OBJECTS.  Called for a new ship
void ClearStuckObjects (void)
{
	int	nObject;

for (int i = 0; i < MAX_STUCK_OBJECTS; i++) {
	if (IS_WALL (stuckObjects [i].nWall)) {
		nObject = stuckObjects [i].nObject;
		if ((OBJECTS [nObject].nType == OBJ_WEAPON) && (OBJECTS [nObject].id == FLARE_ID))
			OBJECTS [nObject].lifeleft = F1_0/8;
		stuckObjects [i].nWall = -1;
		nStuckObjects--;
		}
	}
Assert(nStuckObjects == 0);

}

// -----------------------------------------------------------------------------------
#define	MAX_BLAST_GLASS_DEPTH	5

void BngProcessSegment(tObject *objP, fix damage, tSegment *segp, int depth, sbyte *visited)
{
	int	i;
	short	nSide;

	if (depth > MAX_BLAST_GLASS_DEPTH)
		return;

	depth++;

	for (nSide=0; nSide<MAX_SIDES_PER_SEGMENT; nSide++) {
		int			tm;
		fix			dist;
		vmsVector	pnt;

		//	Process only walls which have glass.
		if ((tm = segp->sides [nSide].nOvlTex)) {
			int	ec, db;
			tEffectClip *ecP;

			ec=gameData.pig.tex.pTMapInfo [tm].nEffectClip;
			ecP = (ec < 0) ? NULL : gameData.eff.pEffects + ec;
			db = ecP ? ecP->nDestBm : -1;

			if (((ec != -1) && (db != -1) && !(ecP->flags & EF_ONE_SHOT)) ||
			 	 ((ec == -1) && (gameData.pig.tex.pTMapInfo [tm].destroyed != -1))) {
				COMPUTE_SIDE_CENTER(&pnt, segp, nSide);
				dist = vmsVector::Dist(pnt, objP->position.vPos);
				if (dist < damage/2) {
					dist = FindConnectedDistance(&pnt, SEG_IDX (segp), &objP->position.vPos, objP->nSegment, MAX_BLAST_GLASS_DEPTH, WID_RENDPAST_FLAG, 0);
					if ((dist > 0) && (dist < damage/2))
						CheckEffectBlowup(segp, nSide, &pnt, OBJECTS + objP->cType.laserInfo.nParentObj, 1);
				}
			}
		}
	}

	for (i=0; i<MAX_SIDES_PER_SEGMENT; i++) {
		int	nSegment = segp->children [i];

		if (nSegment != -1) {
			if (!visited [nSegment]) {
				if (WALL_IS_DOORWAY(segp, (short) i, NULL) & WID_FLY_FLAG) {
					visited [nSegment] = 1;
					BngProcessSegment(objP, damage, &gameData.segs.segments [nSegment], depth, visited);
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------------

bool WallIsTriggerTarget (short nWall)
{
tWall	*wallP = WALLS + nWall;
short nSegment = wallP->nSegment;
short nSide = wallP->nSide;
tTrigger *triggerP = gameData.trigs.triggers;
for (int i = gameData.trigs.nTriggers; i; i--, triggerP++) {
	short *nSegP = triggerP->nSegment;
	short *nSideP = triggerP->nSide;
	for (int j = triggerP->nLinks; j; j--, nSegP++, nSideP++)
		if ((*nSegP == nSegment) && (*nSideP == nSide))
			return true;
	}
return false;
}

// -----------------------------------------------------------------------------------

bool WallIsVolatile (short nWall)
{
if (!IS_WALL (nWall))
	return false;
tWall	*wallP = WALLS + nWall;
if ((wallP->nType == WALL_DOOR) || (wallP->nType == WALL_BLASTABLE))
	return true;
return WallIsTriggerTarget (nWall);
}

// -----------------------------------------------------------------------------------

bool WallIsInvisible (short nWall)
{
if (!IS_WALL (nWall))
	return false;
tWall	*wallP = WALLS + nWall;
if ((wallP->nType != WALL_OPEN) && ((wallP->nType != WALL_CLOAKED) || (wallP->cloakValue <  GR_ACTUAL_FADE_LEVELS)))
	return false;
return !WallIsTriggerTarget (nWall);
}

// -----------------------------------------------------------------------------------
//	objP is going to detonate
//	blast nearby monitors, lights, maybe other things
void BlastNearbyGlass(tObject *objP, fix damage)
{
	int		i;
	sbyte   visited [MAX_SEGMENTS_D2X];
	tSegment	*cursegp;

	cursegp = &gameData.segs.segments [objP->nSegment];
	for (i=0; i<=gameData.segs.nLastSegment; i++)
		visited [i] = 0;

	visited [objP->nSegment] = 1;
	BngProcessSegment(objP, damage, cursegp, 0, visited);


}

#define MAX_CLIP_FRAMES_D1 20

/*
 * reads a tWallClip structure from a CFILE
 */
int wclip_read_n_d1(tWallClip *wc, int n, CFILE *fp)
{
	int i, j;

	for (i = 0; i < n; i++) {
		wc [i].xTotalTime = CFReadFix(fp);
		wc [i].nFrameCount = CFReadShort(fp);
		for (j = 0; j < MAX_CLIP_FRAMES_D1; j++)
			wc [i].frames [j] = CFReadShort(fp);
		wc [i].openSound = CFReadShort(fp);
		wc [i].closeSound = CFReadShort(fp);
		wc [i].flags = CFReadShort(fp);
		CFRead(wc [i].filename, 13, 1, fp);
		wc [i].pad = CFReadByte(fp);
	}
	return i;
}

#if 1//ndef FAST_FILE_IO /*permanently enabled for a reason!*/
/*
 * reads a tWallClip structure from a CFILE
 */
int WClipReadN(tWallClip *wc, int n, CFILE *fp)
{
	int i, j;

for (i = 0; i < n; i++) {
	wc [i].xTotalTime = CFReadFix(fp);
	wc [i].nFrameCount = CFReadShort(fp);
	for (j = 0; j < MAX_CLIP_FRAMES; j++)
		wc [i].frames [j] = CFReadShort(fp);
	wc [i].openSound = CFReadShort(fp);
	wc [i].closeSound = CFReadShort(fp);
	wc [i].flags = CFReadShort(fp);
	CFRead(wc [i].filename, 13, 1, fp);
	wc [i].pad = CFReadByte(fp);
	}
return i;
}

/*
 * reads a tWallV16 structure from a CFILE
 */
void ReadWallV16(tWallV16 *w, CFILE *fp)
{
w->nType = CFReadByte(fp);
w->flags = CFReadByte(fp);
w->hps = CFReadFix(fp);
w->nTrigger = (ubyte) CFReadByte(fp);
w->nClip = CFReadByte(fp);
w->keys = CFReadByte(fp);
}

/*
 * reads a tWallV19 structure from a CFILE
 */
void ReadWallV19(tWallV19 *w, CFILE *fp)
{
w->nSegment = CFReadInt(fp);
w->nSide = CFReadInt(fp);
w->nType = CFReadByte(fp);
w->flags = CFReadByte(fp);
w->hps = CFReadFix(fp);
w->nTrigger = (ubyte) CFReadByte(fp);
w->nClip = CFReadByte(fp);
w->keys = CFReadByte(fp);
w->nLinkedWall = CFReadInt(fp);
}

/*
 * reads a tWall structure from a CFILE
 */
void ReadWall(tWall *w, CFILE *fp)
{
w->nSegment = CFReadInt(fp);
w->nSide = CFReadInt(fp);
w->hps = CFReadFix(fp);
w->nLinkedWall = CFReadInt(fp);
w->nType = CFReadByte(fp);
w->flags = CFReadByte(fp);
w->state = CFReadByte(fp);
w->nTrigger = (ubyte) CFReadByte(fp);
w->nClip = CFReadByte(fp);
w->keys = CFReadByte(fp);
w->controllingTrigger = CFReadByte(fp);
w->cloakValue = CFReadByte(fp);
}

/*
 * reads a v19_door structure from a CFILE
 */
extern void ReadActiveDoorV19(v19_door *d, CFILE *fp)
{
	d->nPartCount = CFReadInt(fp);
	d->seg [0] = CFReadShort(fp);
	d->seg [1] = CFReadShort(fp);
	d->nSide [0] = CFReadShort(fp);
	d->nSide [1] = CFReadShort(fp);
	d->nType [0] = CFReadShort(fp);
	d->nType [1] = CFReadShort(fp);
	d->open = CFReadFix(fp);
}

/*
 * reads an tActiveDoor structure from a CFILE
 */
extern void ReadActiveDoor(tActiveDoor *ad, CFILE *fp)
{
	ad->nPartCount = CFReadInt(fp);
	ad->nFrontWall [0] = CFReadShort(fp);
	ad->nFrontWall [1] = CFReadShort(fp);
	ad->nBackWall [0] = CFReadShort(fp);
	ad->nBackWall [1] = CFReadShort(fp);
	ad->time = CFReadFix(fp);
}
#endif
