/* $Id: gamesave.c,v 1.21 2003/06/16 07:15:59 btb Exp $ */
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
#include <string.h>

#include "inferno.h"
#include "pstypes.h"
#include "strutil.h"
#include "mono.h"
#include "key.h"
#include "gr.h"
#include "palette.h"
#include "menu.h"
#ifdef EDITOR
#	include "editor/editor.h"
#endif
#include "error.h"
#include "dynlight.h"
#include "object.h"
#include "game.h"
#include "screens.h"
#include "wall.h"
#include "gamemine.h"
#include "robot.h"
#include "fireball.h"

#include "cfile.h"
#include "bm.h"
#include "menu.h"
#include "switch.h"
#include "fuelcen.h"
#include "reactor.h"
#include "powerup.h"
#include "weapon.h"
#include "newdemo.h"
#include "loadgame.h"
#include "automap.h"
#include "polyobj.h"
#include "text.h"
#include "gamefont.h"
#include "gamesave.h"
#include "gamepal.h"
#include "laser.h"
#include "byteswap.h"
#include "multi.h"
#include "makesig.h"
#include "gameseg.h"
#include "light.h"
#include "objrender.h"
#include "createmesh.h"
#include "lightmap.h"

#define GAME_VERSION            32
#define GAME_COMPATIBLE_VERSION 22

//version 28->29  add delta light support
//version 27->28  controlcen id now is reactor number, not model number
//version 28->29  ??
//version 29->30  changed CTrigger structure
//version 30->31  changed CTrigger structure some more
//version 31->32  change CSegment structure, make it 512 bytes w/o editor, add SEGMENTS array.

#define MENU_CURSOR_X_MIN       MENU_X
#define MENU_CURSOR_X_MAX       MENU_X+6

tGameFileInfo	gameFileInfo;
game_top_fileinfo	gameTopFileInfo;

//  LINT: adding function prototypes
void ReadObject(CObject *objP, CFile *f, int version);
#ifdef EDITOR
void writeObject(CObject *objP, FILE *f);
void DoLoadSaveLevels(int save);
#endif
#if DBG
void dump_mine_info(void);
#endif

#ifdef EDITOR
extern char mine_filename [];
extern int save_mine_data_compiled(FILE * SaveFile);
//--unused-- #else
//--unused-- char mine_filename [128];
#endif

int nGameSavePlayers = 0;
int nGameSaveOrgRobots = 0;
int nSavePOFNames = 0;
char szSavePOFNames [MAX_POLYGON_MODELS][SHORT_FILENAME_LEN];

//--unused-- CBitmap * Gamesave_saved_bitmap = NULL;

//------------------------------------------------------------------------------
#ifdef EDITOR
// Return true if this level has a name of the form "level??"
// Note that a pathspec can appear at the beginning of the filename.
int IsRealLevel(char *filename)
{
	int len = (int) strlen(filename);

if (len < 6)
	return 0;
return !strnicmp(&filename [len-11], "level", 5);
}
#endif

//------------------------------------------------------------------------------

void VerifyObject (CObject * objP)
{
objP->info.xLifeLeft = IMMORTAL_TIME;		//all loaded CObject are immortal, for now
if (objP->info.nType == OBJ_ROBOT) {
	nGameSaveOrgRobots++;
	// Make sure valid id...
	if (objP->info.nId >= gameData.bots.nTypes [gameStates.app.bD1Data])
		objP->info.nId %= gameData.bots.nTypes [0];
	// Make sure model number & size are correct...
	if (objP->info.renderType == RT_POLYOBJ) {
		Assert(ROBOTINFO (objP->info.nId).nModel != -1);
			//if you fail this assert, it means that a robot in this level
			//hasn't been loaded, possibly because he's marked as
			//non-shareware.  To see what robot number, print objP->info.nId.
		Assert(ROBOTINFO (objP->info.nId).always_0xabcd == 0xabcd);
			//if you fail this assert, it means that the robot_ai for
			//a robot in this level hasn't been loaded, possibly because
			//it's marked as non-shareware.  To see what robot number,
			//print objP->info.nId.
		objP->rType.polyObjInfo.nModel = ROBOTINFO (objP->info.nId).nModel;
		objP->info.xSize = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad;
		}
	if (objP->info.nId == 65)						//special "reactor" robots
		objP->info.movementType = MT_NONE;
	if (objP->info.movementType == MT_PHYSICS) {
		objP->mType.physInfo.mass = ROBOTINFO (objP->info.nId).mass;
		objP->mType.physInfo.drag = ROBOTINFO (objP->info.nId).drag;
		}
	}
else {		//Robots taken care of above
	if (objP->info.renderType == RT_POLYOBJ) {
		char *name = szSavePOFNames [objP->rType.polyObjInfo.nModel];
		for (int i = 0; i < gameData.models.nPolyModels; i++)
			if (!stricmp (Pof_names [i], name)) {		//found it!
				objP->rType.polyObjInfo.nModel = i;
				break;
				}
		}
	}
if (objP->info.nType == OBJ_POWERUP) {
	if (objP->info.nId >= gameData.objs.pwrUp.nTypes + POWERUP_ADDON_COUNT) {
		objP->info.nId = 0;
		Assert(objP->info.renderType != RT_POLYOBJ);
		}
	objP->info.controlType = CT_POWERUP;
	if (objP->info.nId >= MAX_POWERUP_TYPES_D2)
		InitAddonPowerup (objP);
	else {
		objP->info.xSize = gameData.objs.pwrUp.info [objP->info.nId].size;
		objP->cType.powerupInfo.xCreationTime = 0;
		if (gameData.app.nGameMode & GM_NETWORK) {
		if (MultiPowerupIs4Pack (objP->info.nId)) {
				gameData.multiplayer.powerupsInMine [objP->info.nId-1] += 4;
	 			gameData.multiplayer.maxPowerupsAllowed [objP->info.nId-1] += 4;
				}
			gameData.multiplayer.powerupsInMine [objP->info.nId]++;
			gameData.multiplayer.maxPowerupsAllowed [objP->info.nId]++;
#if TRACE
			console.printf (CON_DBG, "PowerupLimiter: ID=%d\n", objP->info.nId);
			if (objP->info.nId > MAX_POWERUP_TYPES)
				console.printf (1,"POWERUP: Overwriting array bounds!\n");
#endif
			}
		}
	}
else if (objP->info.nType == OBJ_WEAPON) {
	if (objP->info.nId >= gameData.weapons.nTypes [0]) {
		objP->info.nId = 0;
		Assert(objP->info.renderType != RT_POLYOBJ);
		}
	if (objP->info.nId == SMALLMINE_ID) {		//make sure pmines have correct values
		objP->mType.physInfo.mass = gameData.weapons.info [objP->info.nId].mass;
		objP->mType.physInfo.drag = gameData.weapons.info [objP->info.nId].drag;
		objP->mType.physInfo.flags |= PF_FREE_SPINNING;
		// Make sure model number & size are correct...	
		Assert(objP->info.renderType == RT_POLYOBJ);
		objP->rType.polyObjInfo.nModel = gameData.weapons.info [objP->info.nId].nModel;
		objP->info.xSize = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad;
		}
	}
else if (objP->info.nType == OBJ_REACTOR) {
	objP->info.renderType = RT_POLYOBJ;
	objP->info.controlType = CT_CNTRLCEN;
	if (gameData.segs.nLevelVersion <= 1) { // descent 1 reactor
		objP->info.nId = 0;                         // used to be only one kind of reactor
		objP->rType.polyObjInfo.nModel = gameData.reactor.props [0].nModel;// descent 1 reactor
		}
#ifdef EDITOR
 {
	int i;
	// Check, and set, strength of reactor
	for (i = 0; i < gameData.objs.types.count; i++)
		if ((gameData.objs.types.nType [i] == OL_CONTROL_CENTER) && 
			 (gameData.objs.types.nType.nId [i] == objP->info.nId)) {
			objP->info.xShields = gameData.objs.types.nType.nStrength [i];
			break;	
			}
		Assert(i < gameData.objs.types.count);		//make sure we found it
		}
#endif
	}
else if (objP->info.nType == OBJ_PLAYER) {
	if (objP == gameData.objs.consoleP)	
		InitPlayerObject();
	else
		if (objP->info.renderType == RT_POLYOBJ)	//recover from Matt's pof file matchup bug
			objP->rType.polyObjInfo.nModel = gameData.pig.ship.player->nModel;
	//Make sure orient matrix is orthogonal
	gameOpts->render.nMathFormat = 0;
	objP->info.position.mOrient.CheckAndFix();
	gameOpts->render.nMathFormat = gameOpts->render.nDefMathFormat;
	objP->info.nId = nGameSavePlayers++;
	}
else if (objP->info.nType == OBJ_HOSTAGE) {
	objP->info.renderType = RT_HOSTAGE;
	objP->info.controlType = CT_POWERUP;
	}
objP->Link ();
}

//------------------------------------------------------------------------------
//static gs_skip(int len,CFile *file)
//{
//
//	cf.Seek (file,len,SEEK_CUR);
//}

#ifdef EDITOR
static void gs_write_int(int i,FILE *file)
{
	if (fwrite(&i, sizeof(i), 1, file) != 1)
		Error("Error reading int in gamesave.c");

}

static void gs_write_fix(fix f,FILE *file)
{
	if (fwrite(&f, sizeof(f), 1, file) != 1)
		Error("Error reading fix in gamesave.c");

}

static void gs_write_short(short s,FILE *file)
{
	if (fwrite(&s, sizeof(s), 1, file) != 1)
		Error("Error reading short in gamesave.c");

}

static void gs_write_fixang(fixang f,FILE *file)
{
	if (fwrite(&f, sizeof(f), 1, file) != 1)
		Error("Error reading fixang in gamesave.c");

}

static void gs_write_byte(byte b,FILE *file)
{
	if (fwrite(&b, sizeof(b), 1, file) != 1)
		Error("Error reading byte in gamesave.c");

}

static void gr_write_vector(CFixVector *v,FILE *file)
{
	gs_write_fix(v->x,file);
	gs_write_fix(v->y,file);
	gs_write_fix(v->z,file);
}

static void gs_write_matrix(CFixMatrix *m,FILE *file)
{
	gr_write_vector(&m->rVec,file);
	gr_write_vector(&m->uVec,file);
	gr_write_vector(&m->fVec,file);
}

static void gs_write_angvec(CAngleVector *v,FILE *file)
{
	gs_write_fixang(v->p,file);
	gs_write_fixang(v->b,file);
	gs_write_fixang(v->h,file);
}

#endif

//------------------------------------------------------------------------------

int MultiPowerupIs4Pack(int);
//reads one CObject of the given version from the given file
void CObject::Read (CFile& cf)
{
#if DBG
if (OBJ_IDX (this) == nDbgObj)
	nDbgObj = nDbgObj;
#endif
info.nType = cf.ReadByte ();
info.nId = cf.ReadByte ();
info.controlType = cf.ReadByte ();
info.movementType = cf.ReadByte ();
info.renderType = cf.ReadByte ();
info.nFlags = cf.ReadByte ();
info.nSegment = cf.ReadShort ();
info.nAttachedObj = -1;
cf.ReadVector (info.position.vPos);
cf.ReadMatrix (info.position.mOrient);
info.xSize = cf.ReadFix ();
info.xShields = cf.ReadFix ();
cf.ReadVector (info.vLastPos);
info.contains.nType = cf.ReadByte ();
info.contains.nId = cf.ReadByte ();
info.contains.nCount = cf.ReadByte ();
switch (info.movementType) {
	case MT_PHYSICS:
		cf.ReadVector (mType.physInfo.velocity);
		cf.ReadVector (mType.physInfo.thrust);
		mType.physInfo.mass = cf.ReadFix ();
		mType.physInfo.drag = cf.ReadFix ();
		mType.physInfo.brakes = cf.ReadFix ();
		cf.ReadVector (mType.physInfo.rotVel);
		cf.ReadVector (mType.physInfo.rotThrust);
		mType.physInfo.turnRoll	= cf.ReadFixAng ();
		mType.physInfo.flags	= cf.ReadShort ();
		break;

	case MT_SPINNING:
		cf.ReadVector (mType.spinRate);
		break;

	case MT_NONE:
		break;

	default:
		Int3();
	}

switch (info.controlType) {
	case CT_AI: 
		cType.aiInfo.behavior = cf.ReadByte ();
		for (int i = 0; i < MAX_AI_FLAGS; i++)
			cType.aiInfo.flags [i] = cf.ReadByte ();
		cType.aiInfo.nHideSegment = cf.ReadShort ();
		cType.aiInfo.nHideIndex = cf.ReadShort ();
		cType.aiInfo.nPathLength = cf.ReadShort ();
		cType.aiInfo.nCurPathIndex = (char) cf.ReadShort ();
		if (gameTopFileInfo.fileinfoVersion <= 25) {
			cf.ReadShort ();	//				cType.aiInfo.follow_path_start_seg	= 
			cf.ReadShort ();	//				cType.aiInfo.follow_path_end_seg		= 
			}
		break;

	case CT_EXPLOSION:
		cType.explInfo.nSpawnTime = cf.ReadFix ();
		cType.explInfo.nDeleteTime	= cf.ReadFix ();
		cType.explInfo.nDeleteObj = cf.ReadShort ();
		cType.explInfo.attached.nNext = cType.explInfo.attached.nPrev = cType.explInfo.attached.nParent = -1;
		break;

	case CT_WEAPON: //do I really need to read these?  Are they even saved to disk?
		cType.laserInfo.parent.nType = cf.ReadShort ();
		cType.laserInfo.parent.nObject = cf.ReadShort ();
		cType.laserInfo.parent.nSignature = cf.ReadInt ();
		break;

	case CT_LIGHT:
		cType.lightInfo.intensity = cf.ReadFix ();
		break;

	case CT_POWERUP:
		if (gameTopFileInfo.fileinfoVersion >= 25)
			cType.powerupInfo.nCount = cf.ReadInt ();
		else
			cType.powerupInfo.nCount = 1;
		if (info.nId == POW_VULCAN)
			cType.powerupInfo.nCount = VULCAN_WEAPON_AMMO_AMOUNT;
		else if (info.nId == POW_GAUSS)
			cType.powerupInfo.nCount = VULCAN_WEAPON_AMMO_AMOUNT;
		else if (info.nId == POW_OMEGA)
			cType.powerupInfo.nCount = MAX_OMEGA_CHARGE;
		break;

	case CT_NONE:
	case CT_FLYING:
	case CT_DEBRIS:
		break;

	case CT_SLEW:		//the CPlayerData is generally saved as slew
		break;

	case CT_CNTRLCEN:
		break;

	case CT_MORPH:
	case CT_FLYTHROUGH:
	case CT_REPAIRCEN:
		default:
		Int3();
	}

switch (info.renderType) {
	case RT_NONE:
		break;

	case RT_MORPH:
	case RT_POLYOBJ: {
		rType.polyObjInfo.nModel = cf.ReadInt ();
		for (int i = 0; i <MAX_SUBMODELS; i++)
			cf.ReadAngVec(rType.polyObjInfo.animAngles [i]);
		rType.polyObjInfo.nSubObjFlags = cf.ReadInt ();
		int tmo = cf.ReadInt ();
#ifndef EDITOR
		rType.polyObjInfo.nTexOverride = tmo;
#else
		if (tmo==-1)
			rType.polyObjInfo.nTexOverride = -1;
		else {
			int xlated_tmo = tmap_xlate_table [tmo];
			if (xlated_tmo < 0) {
#if TRACE
				console.printf (CON_DBG, "Couldn't find texture for demo CObject, nModel = %d\n", rType.polyObjInfo.nModel);
#endif
				Int3();
				xlated_tmo = 0;
				}
			rType.polyObjInfo.nTexOverride	= xlated_tmo;
			}
#endif
		rType.polyObjInfo.nAltTextures = 0;
		break;
		}

	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		rType.vClipInfo.nClipIndex	= cf.ReadInt ();
		rType.vClipInfo.xFrameTime	= cf.ReadFix ();
		rType.vClipInfo.nCurFrame	= cf.ReadByte ();
		break;

	case RT_THRUSTER:
	case RT_LASER:
		break;

	case RT_SMOKE:
		rType.particleInfo.nLife = cf.ReadInt ();
		rType.particleInfo.nSize [0] = cf.ReadInt ();
		rType.particleInfo.nParts = cf.ReadInt ();
		rType.particleInfo.nSpeed = cf.ReadInt ();
		rType.particleInfo.nDrift = cf.ReadInt ();
		rType.particleInfo.nBrightness = cf.ReadInt ();
		rType.particleInfo.color.red = cf.ReadByte ();
		rType.particleInfo.color.green = cf.ReadByte ();
		rType.particleInfo.color.blue = cf.ReadByte ();
		rType.particleInfo.color.alpha = cf.ReadByte ();
		rType.particleInfo.nSide = cf.ReadByte ();
		if (gameData.segs.nLevelVersion < 18)
			rType.particleInfo.nType = 0;
		else
			rType.particleInfo.nType = cf.ReadByte ();
		break;

	case RT_LIGHTNING:
		rType.lightningInfo.nLife = cf.ReadInt ();
		rType.lightningInfo.nDelay = cf.ReadInt ();
		rType.lightningInfo.nLength = cf.ReadInt ();
		rType.lightningInfo.nAmplitude = cf.ReadInt ();
		rType.lightningInfo.nOffset = cf.ReadInt ();
		rType.lightningInfo.nLightnings = cf.ReadShort ();
		rType.lightningInfo.nId = cf.ReadShort ();
		rType.lightningInfo.nTarget = cf.ReadShort ();
		rType.lightningInfo.nNodes = cf.ReadShort ();
		rType.lightningInfo.nChildren = cf.ReadShort ();
		rType.lightningInfo.nSteps = cf.ReadShort ();
		rType.lightningInfo.nAngle = cf.ReadByte ();
		rType.lightningInfo.nStyle = cf.ReadByte ();
		rType.lightningInfo.nSmoothe = cf.ReadByte ();
		rType.lightningInfo.bClamp = cf.ReadByte ();
		rType.lightningInfo.bPlasma = cf.ReadByte ();
		rType.lightningInfo.bSound = cf.ReadByte ();
		rType.lightningInfo.bRandom = cf.ReadByte ();
		rType.lightningInfo.bInPlane = cf.ReadByte ();
		rType.lightningInfo.color.red = cf.ReadByte ();
		rType.lightningInfo.color.green = cf.ReadByte ();
		rType.lightningInfo.color.blue = cf.ReadByte ();
		rType.lightningInfo.color.alpha = cf.ReadByte ();
		break;

	default:
		Int3();
	}
}

//------------------------------------------------------------------------------
#ifdef EDITOR

//writes one CObject to the given file
void writeObject(CObject *objP,FILE *f)
{
	gs_write_byte(objP->info.nType,f);
	gs_write_byte(objP->info.nId,f);

	gs_write_byte(objP->info.controlType,f);
	gs_write_byte(objP->info.movementType,f);
	gs_write_byte(objP->info.renderType,f);
	gs_write_byte(objP->info.nFlags,f);

	gs_write_short(objP->info.nSegment,f);

	gr_write_vector(&objP->info.position.vPos,f);
	gs_write_matrix(&objP->info.position.mOrient,f);

	gs_write_fix(objP->info.xSize,f);
	gs_write_fix(objP->info.xShields,f);

	gr_write_vector(&objP->info.vLastPos,f);

	gs_write_byte(objP->info.contains.nType,f);
	gs_write_byte(objP->info.contains.nId,f);
	gs_write_byte(objP->info.contains.count,f);

	switch (objP->info.movementType) {

		case MT_PHYSICS:

	 		gr_write_vector(&objP->mType.physInfo.velocity,f);
			gr_write_vector(&objP->mType.physInfo.thrust,f);

			gs_write_fix(objP->mType.physInfo.mass,f);
			gs_write_fix(objP->mType.physInfo.drag,f);
			gs_write_fix(objP->mType.physInfo.brakes,f);

			gr_write_vector(&objP->mType.physInfo.rotVel,f);
			gr_write_vector(&objP->mType.physInfo.rotThrust,f);

			gs_write_fixang(objP->mType.physInfo.turnRoll,f);
			gs_write_short(objP->mType.physInfo.flags,f);

			break;

		case MT_SPINNING:

			gr_write_vector(&objP->mType.spinRate,f);
			break;

		case MT_NONE:
			break;

		default:
			Int3();
	}

	switch (objP->info.controlType) {

		case CT_AI: {
			int i;

			gs_write_byte(objP->cType.aiInfo.behavior,f);

			for (i=0;i<MAX_AI_FLAGS;i++)
				gs_write_byte(objP->cType.aiInfo.flags [i],f);

			gs_write_short(objP->cType.aiInfo.nHideSegment,f);
			gs_write_short(objP->cType.aiInfo.nHideIndex,f);
			gs_write_short(objP->cType.aiInfo.nPathLength,f);
			gs_write_short(objP->cType.aiInfo.nCurPathIndex,f);

			// -- unused! mk, 08/13/95 -- gs_write_short(objP->cType.aiInfo.follow_path_start_seg,f);
			// -- unused! mk, 08/13/95 -- gs_write_short(objP->cType.aiInfo.follow_path_end_seg,f);

			break;
		}

		case CT_EXPLOSION:

			gs_write_fix(objP->cType.explInfo.nSpawnTime,f);
			gs_write_fix(objP->cType.explInfo.nDeleteTime,f);
			gs_write_short(objP->cType.explInfo.nDeleteObj,f);

			break;

		case CT_WEAPON:

			//do I really need to write these OBJECTS?

			gs_write_short(objP->cType.laserInfo.parent.nType,f);
			gs_write_short(objP->cType.laserInfo.parent.nObject,f);
			gs_write_int(objP->cType.laserInfo.parent.nSignature,f);

			break;

		case CT_LIGHT:

			gs_write_fix(objP->cType.lightInfo.intensity,f);
			break;

		case CT_POWERUP:

			gs_write_int(objP->cType.powerupInfo.count,f);
			break;

		case CT_NONE:
		case CT_FLYING:
		case CT_DEBRIS:
			break;

		case CT_SLEW:		//the CPlayerData is generally saved as slew
			break;

		case CT_CNTRLCEN:
			break;			//control center CObject.

		case CT_MORPH:
		case CT_REPAIRCEN:
		case CT_FLYTHROUGH:
		default:
			Int3();

	}

	switch (objP->info.renderType) {

		case RT_NONE:
			break;

		case RT_MORPH:
		case RT_POLYOBJ: {
			int i;

			gs_write_int(objP->rType.polyObjInfo.nModel,f);

			for (i=0;i<MAX_SUBMODELS;i++)
				gs_write_angvec(&objP->rType.polyObjInfo.animAngles [i],f);

			gs_write_int(objP->rType.polyObjInfo.nSubObjFlags,f);

			gs_write_int(objP->rType.polyObjInfo.nTexOverride,f);

			break;
		}

		case RT_WEAPON_VCLIP:
		case RT_HOSTAGE:
		case RT_POWERUP:
		case RT_FIREBALL:

			gs_write_int(objP->rType.vClipInfo.nClipIndex,f);
			gs_write_fix(objP->rType.vClipInfo.xFrameTime,f);
			gs_write_byte(objP->rType.vClipInfo.nCurFrame,f);

			break;

		case RT_THRUSTER:
		case RT_LASER:
			break;

		default:
			Int3();

	}

}
#endif

extern int RemoveTriggerNum (int trigger_num);

// -----------------------------------------------------------------------------

static void InitGameFileInfo (void)
{
gameFileInfo.level				=	-1;
gameFileInfo.player.offset		=	-1;
gameFileInfo.player.size		=	sizeof(CPlayerData);
gameFileInfo.objects.offset	=	-1;
gameFileInfo.objects.count		=	0;
gameFileInfo.objects.size		=	sizeof(CObject);  
gameFileInfo.walls.offset		=	-1;
gameFileInfo.walls.count		=	0;
gameFileInfo.walls.size			=	sizeof(CWall);  
gameFileInfo.doors.offset		=	-1;
gameFileInfo.doors.count		=	0;
gameFileInfo.doors.size			=	sizeof(CActiveDoor);  
gameFileInfo.triggers.offset	=	-1;
gameFileInfo.triggers.count	=	0;
gameFileInfo.triggers.size		=	sizeof(CTrigger);  
gameFileInfo.control.offset	=	-1;
gameFileInfo.control.count		=	0;
gameFileInfo.control.size		=	sizeof(tReactorTriggers);
gameFileInfo.botGen.offset		=	-1;
gameFileInfo.botGen.count		=	0;
gameFileInfo.botGen.size		=	sizeof(tMatCenInfo);
gameFileInfo.equipGen.offset	=	-1;
gameFileInfo.equipGen.count	=	0;
gameFileInfo.equipGen.size		=	sizeof(tMatCenInfo);
gameFileInfo.lightDeltaIndices.offset = -1;
gameFileInfo.lightDeltaIndices.count =	0;
gameFileInfo.lightDeltaIndices.size =	sizeof(CLightDeltaIndex);
gameFileInfo.lightDeltas.offset	=	-1;
gameFileInfo.lightDeltas.count	=	0;
gameFileInfo.lightDeltas.size		=	sizeof(CLightDelta);
}

// -----------------------------------------------------------------------------

static int ReadGameFileInfo (CFile& cf, int nStartOffset)
{
gameTopFileInfo.fileinfo_signature = cf.ReadShort ();
gameTopFileInfo.fileinfoVersion = cf.ReadShort ();
gameTopFileInfo.fileinfo_sizeof = cf.ReadInt ();
// Check signature
if (gameTopFileInfo.fileinfo_signature != 0x6705)
	return -1;
// Check version number
if (gameTopFileInfo.fileinfoVersion < GAME_COMPATIBLE_VERSION)
	return -1;
// Now, Read in the fileinfo
if (cf.Seek (nStartOffset, SEEK_SET)) 
	Error ("Error seeking to gameFileInfo in gamesave.c");
gameFileInfo.fileinfo_signature = cf.ReadShort ();
gameFileInfo.fileinfoVersion = cf.ReadShort ();
gameFileInfo.fileinfo_sizeof = cf.ReadInt ();
cf.Read (gameFileInfo.mine_filename, sizeof (char), 15);
gameFileInfo.level = cf.ReadInt ();
gameFileInfo.player.offset = cf.ReadInt ();				// Player info
gameFileInfo.player.size = cf.ReadInt ();
gameFileInfo.objects.offset = cf.ReadInt ();				// Object info
gameFileInfo.objects.count = cf.ReadInt ();    
gameFileInfo.objects.size = cf.ReadInt ();  
gameFileInfo.walls.offset = cf.ReadInt ();
gameFileInfo.walls.count = cf.ReadInt ();
gameFileInfo.walls.size = cf.ReadInt ();
gameFileInfo.doors.offset = cf.ReadInt ();
gameFileInfo.doors.count = cf.ReadInt ();
gameFileInfo.doors.size = cf.ReadInt ();
gameFileInfo.triggers.offset = cf.ReadInt ();
gameFileInfo.triggers.count = cf.ReadInt ();
gameFileInfo.triggers.size = cf.ReadInt ();
gameFileInfo.links.offset = cf.ReadInt ();
gameFileInfo.links.count = cf.ReadInt ();
gameFileInfo.links.size = cf.ReadInt ();
gameFileInfo.control.offset = cf.ReadInt ();
gameFileInfo.control.count = cf.ReadInt ();
gameFileInfo.control.size = cf.ReadInt ();
gameFileInfo.botGen.offset = cf.ReadInt ();
gameFileInfo.botGen.count = cf.ReadInt ();
gameFileInfo.botGen.size = cf.ReadInt ();
if (gameTopFileInfo.fileinfoVersion >= 29) {
	gameFileInfo.lightDeltaIndices.offset = cf.ReadInt ();
	gameFileInfo.lightDeltaIndices.count = cf.ReadInt ();
	gameFileInfo.lightDeltaIndices.size = cf.ReadInt ();

	gameFileInfo.lightDeltas.offset = cf.ReadInt ();
	gameFileInfo.lightDeltas.count = cf.ReadInt ();
	gameFileInfo.lightDeltas.size = cf.ReadInt ();
	}
if (gameData.segs.nLevelVersion >= 17) {
	gameFileInfo.equipGen.offset = cf.ReadInt ();
	gameFileInfo.equipGen.count = cf.ReadInt ();
	gameFileInfo.equipGen.size = cf.ReadInt ();
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLevelInfo (CFile& cf)
{
if (gameTopFileInfo.fileinfoVersion >= 31) { //load mine filename
	// read newline-terminated string, not sure what version this changed.
	cf.GetS (gameData.missions.szCurrentLevel, sizeof (gameData.missions.szCurrentLevel));

	if (gameData.missions.szCurrentLevel [strlen (gameData.missions.szCurrentLevel) - 1] == '\n')
		gameData.missions.szCurrentLevel [strlen (gameData.missions.szCurrentLevel) - 1] = 0;
}
else if (gameTopFileInfo.fileinfoVersion >= 14) { //load mine filename
	// read null-terminated string
	char *p = gameData.missions.szCurrentLevel;
	//must do read one char at a time, since no cf.GetS()
	do {
		*p = cf.GetC ();
		} while (*p++);
}
else
	gameData.missions.szCurrentLevel [0] = 0;
if (gameTopFileInfo.fileinfoVersion >= 19) {	//load pof names
	nSavePOFNames = cf.ReadShort ();
	if ((nSavePOFNames != 0x614d) && (nSavePOFNames != 0x5547)) { // "Ma"de w/DMB beta/"GU"ILE
		if (nSavePOFNames >= MAX_POLYGON_MODELS)
			return -1;
		cf.Read (szSavePOFNames, nSavePOFNames, SHORT_FILENAME_LEN);
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadObjectInfo (CFile& cf)
{
	int	i;

if (gameFileInfo.objects.offset > -1) {
	CObject	*objP = OBJECTS.Buffer ();
	if (cf.Seek (gameFileInfo.objects.offset, SEEK_SET)) {
		Error ("Error seeking to object data\n(file damaged or invalid)");
		return -1;
		}
	OBJECTS.Clear (0, gameFileInfo.objects.count);
	for (i = 0; i < gameFileInfo.objects.count; i++, objP++) {
		objP->Read (cf);
		objP->info.nSignature = gameData.objs.nNextSignature++;
#if DBG
		if (i == nDbgObj) {
			extern int dbgObjInstances;
			dbgObjInstances++;
			}
#endif
		VerifyObject (objP);
		gameData.objs.init [i] = *objP;
		}
	}
for (i = 0; i < LEVEL_OBJECTS - 1; i++)
	gameData.objs.dropInfo [i].nNextPowerup = i + 1;
gameData.objs.dropInfo [i].nNextPowerup = -1;
gameData.objs.nFirstDropped =
gameData.objs.nLastDropped = -1;
gameData.objs.nFreeDropped = 0;
return 0;
}

// -----------------------------------------------------------------------------

static int ReadWallInfo (CFile& cf)
{
if (gameFileInfo.walls.offset > -1) {
	int	i;

	if (!gameData.walls.walls.Resize (gameFileInfo.walls.count)) {
		Error ("Not enough memory for wall data\n");
		return -1;
		}
	if (cf.Seek (gameFileInfo.walls.offset, SEEK_SET)) {
		Error ("Error seeking to wall data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.walls.count; i++) {
		if (gameTopFileInfo.fileinfoVersion >= 20)
			WALLS [i].Read (cf); // v20 walls and up.
		else if (gameTopFileInfo.fileinfoVersion >= 17) {
			tWallV19 w;

			ReadWallV19(w, cf);
			WALLS [i].nSegment	   = w.nSegment;
			WALLS [i].nSide			= w.nSide;
			WALLS [i].nLinkedWall	= w.nLinkedWall;
			WALLS [i].nType			= w.nType;
			WALLS [i].flags			= w.flags;
			WALLS [i].hps				= w.hps;
			WALLS [i].nTrigger		= w.nTrigger;
			WALLS [i].nClip			= w.nClip;
			WALLS [i].keys				= w.keys;
			WALLS [i].state			= WALL_DOOR_CLOSED;
			}
		else {
			tWallV16 w;

			ReadWallV16(w, cf);
			WALLS [i].nSegment = WALLS [i].nSide = WALLS [i].nLinkedWall = -1;
			WALLS [i].nType		= w.nType;
			WALLS [i].flags		= w.flags;
			WALLS [i].hps			= w.hps;
			WALLS [i].nTrigger	= w.nTrigger;
			WALLS [i].nClip		= w.nClip;
			WALLS [i].keys			= w.keys;
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadDoorInfo (CFile& cf)
{
if (gameFileInfo.doors.offset > -1) {
	int	i;
	if (cf.Seek (gameFileInfo.doors.offset, SEEK_SET)) {
		Error ("Error seeking to door data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.doors.count; i++) {
		if (gameTopFileInfo.fileinfoVersion >= 20)
			ReadActiveDoor (gameData.walls.activeDoors [i], cf); // version 20 and up
		else {
			v19_door d;
			short nConnSeg, nConnSide;
			CSegment* segP;

			ReadActiveDoorV19 (d, cf);
			gameData.walls.activeDoors [i].nPartCount = d.nPartCount;
			for (int j = 0; j < d.nPartCount; j++) {
				segP = SEGMENTS + d.seg [j];
				nConnSeg = segP->m_children [d.nSide [j]];
				nConnSide = segP->ConnectedSide (SEGMENTS + nConnSeg);
				gameData.walls.activeDoors [i].nFrontWall [j] = segP->WallNum (d.nSide [j]);
				gameData.walls.activeDoors [i].nBackWall [j] = SEGMENTS [nConnSeg].WallNum (nConnSide);
				}
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadTriggerInfo (CFile& cf)
{
	int		h, i, j;
	CTrigger	*trigP;

if (gameFileInfo.triggers.offset > -1) {
#if TRACE
	console.printf(CON_DBG, "   loading CTrigger data ...\n");
#endif
	if (cf.Seek (gameFileInfo.triggers.offset, SEEK_SET)) {
		Error ("Error seeking to trigger data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0, trigP = TRIGGERS.Buffer (); i < gameFileInfo.triggers.count; i++, trigP++) {
		if (gameTopFileInfo.fileinfoVersion >= 31) 
			trigP->Read (cf, 0);
		else {
			tTriggerV30 trig;
			int t, nType = 0, flags = 0;
			if (gameTopFileInfo.fileinfoVersion == 30)
				V30TriggerRead (trig, cf);
			else {
				tTriggerV29 trig29;
				V29TriggerRead (trig29, cf);
				trig.flags = trig29.flags;
				trig.nLinks	= (char) trig29.nLinks;
				trig.value = trig29.value;
				trig.time = trig29.time;
				for (t = 0; t < trig.nLinks; t++) {
					trig.segments [t] = trig29.segments [t];
					trig.sides [t] = trig29.sides [t];
					}
				}
			//Assert(trig.flags & TRIGGER_ON);
			trig.flags &= ~TRIGGER_ON;
			if (trig.flags & TRIGGER_CONTROL_DOORS)
				nType = TT_OPEN_DOOR;
			else if (trig.flags & TRIGGER_SHIELD_DAMAGE)
				nType = TT_SHIELD_DAMAGE;
			else if (trig.flags & TRIGGER_ENERGY_DRAIN)
				nType = TT_ENERGY_DRAIN;
			else if (trig.flags & TRIGGER_EXIT)
				nType = TT_EXIT;
			else if (trig.flags & TRIGGER_MATCEN)
				nType = TT_MATCEN;
			else if (trig.flags & TRIGGER_ILLUSION_OFF)
				nType = TT_ILLUSION_OFF;
			else if (trig.flags & TRIGGER_SECRET_EXIT)
				nType = TT_SECRET_EXIT;
			else if (trig.flags & TRIGGER_ILLUSION_ON)
				nType = TT_ILLUSION_ON;
			else if (trig.flags & TRIGGER_UNLOCK_DOORS)
				nType = TT_UNLOCK_DOOR;
			else if (trig.flags & TRIGGER_OPEN_WALL)
				nType = TT_OPEN_WALL;
			else if (trig.flags & TRIGGER_CLOSE_WALL)
				nType = TT_CLOSE_WALL;
			else if (trig.flags & TRIGGER_ILLUSORY_WALL)
				nType = TT_ILLUSORY_WALL;
			else
				Int3();
			if (trig.flags & TRIGGER_ONE_SHOT)
				flags = TF_ONE_SHOT;

			trigP->nType = nType;
			trigP->flags = flags;
			trigP->nLinks = trig.nLinks;
			trigP->nLinks = trig.nLinks;
			trigP->value = trig.value;
			trigP->time = trig.time;
			for (t = 0; t < trig.nLinks; t++) {
				trigP->segments [t] = trig.segments [t];
				trigP->sides [t] = trig.sides [t];
				}
			}
		if (trigP->nLinks < 0)
			trigP->nLinks = 0;
		else if (trigP->nLinks > MAX_TRIGGER_TARGETS)
			trigP->nLinks = MAX_TRIGGER_TARGETS;
		for (h = trigP->nLinks, j = 0; j < h; ) {
			if ((trigP->segments [j] >= 0) && (trigP->segments [j] < gameData.segs.nSegments) &&
				 (trigP->sides [j] >= 0) && (trigP->sides [j] < 6))
				j++;
			else if (--h) {
				trigP->segments [j] = trigP->segments [h];
				trigP->sides [j] = trigP->sides [h];
				}
			}
		trigP->nLinks = h;
		}
	if (gameTopFileInfo.fileinfoVersion >= 33) {
		gameData.trigs.nObjTriggers = cf.ReadInt ();
		if (gameData.trigs.nObjTriggers) {
			for (i = 0; i < gameData.trigs.nObjTriggers; i++)
				OBJTRIGGERS [i].Read (cf, 1);
			for (i = 0; i < gameData.trigs.nObjTriggers; i++) {
				gameData.trigs.objTriggerRefs [i].prev = cf.ReadShort ();
				gameData.trigs.objTriggerRefs [i].next = cf.ReadShort ();
				gameData.trigs.objTriggerRefs [i].nObject = cf.ReadShort ();
				}
			}
		if (gameTopFileInfo.fileinfoVersion < 36) {
			for (i = 0; i < 700; i++)
				gameData.trigs.firstObjTrigger [i] = cf.ReadShort ();
			}
		else {
			gameData.trigs.firstObjTrigger.Clear (0xff);
			for (i = cf.ReadShort (); i; i--) {
				j = cf.ReadShort ();
				gameData.trigs.firstObjTrigger [j] = cf.ReadShort ();
				}
			}
		}
	else {
		gameData.trigs.nObjTriggers = 0;
		OBJTRIGGERS.Clear ();
		gameData.trigs.objTriggerRefs.Clear (0xff);
		gameData.trigs.firstObjTrigger.Clear (0xff);
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadReactorInfo (CFile& cf)
{
if (gameFileInfo.control.offset > -1) {
#if TRACE
	console.printf(CON_DBG, "   loading reactor data ...\n");
#endif
	if (cf.Seek (gameFileInfo.control.offset, SEEK_SET)) {
		Error ("Error seeking to reactor data\n(file damaged or invalid)");
		return -1;
		}
	ReadReactorTriggers (cf);
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadBotGenInfo (CFile& cf)
{
if (gameFileInfo.botGen.offset > -1) {
	int	i, j;

	if (cf.Seek (gameFileInfo.botGen.offset, SEEK_SET)) {
		Error ("Error seeking to robot generator data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.botGen.count; i++) {
		if (gameTopFileInfo.fileinfoVersion < 27) {
			old_tMatCenInfo m;

			OldMatCenInfoRead (&m, cf);

			gameData.matCens.botGens [i].objFlags [0] = m.objFlags;
			gameData.matCens.botGens [i].objFlags [1] = 0;
			gameData.matCens.botGens [i].xHitPoints = m.xHitPoints;
			gameData.matCens.botGens [i].xInterval = m.xInterval;
			gameData.matCens.botGens [i].nSegment = m.nSegment;
			gameData.matCens.botGens [i].nFuelCen = m.nFuelCen;
		}
		else
			MatCenInfoRead (gameData.matCens.botGens + i, cf);

		//	Set links in gameData.matCens.botGens to gameData.matCens.fuelCenters array
		for (j = 0; j <= gameData.segs.nLastSegment; j++)
			if ((SEGMENTS [j].m_nType == SEGMENT_IS_ROBOTMAKER) &&
					(SEGMENTS [j].m_nMatCen == i)) {
				gameData.matCens.botGens [i].nFuelCen = SEGMENTS [j].m_value;
				break;
				}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadEquipGenInfo (CFile& cf)
{
if (gameFileInfo.equipGen.offset > -1) {
	int	i, j;

	if (cf.Seek (gameFileInfo.equipGen.offset, SEEK_SET)) {
		Error ("Error seeking to equipment generator data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.equipGen.count; i++) {
		MatCenInfoRead (gameData.matCens.equipGens + i, cf);
		//	Set links in gameData.matCens.botGens to gameData.matCens.fuelCenters array
		CSegment* segP = SEGMENTS.Buffer ();
		for (j = 0; j <= gameData.segs.nLastSegment; j++, segP++)
			if ((segP->m_nType == SEGMENT_IS_EQUIPMAKER) && (segP->m_nMatCen == i))
				gameData.matCens.equipGens [i].nFuelCen = segP->m_value;
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLightDeltaIndexInfo (CFile& cf)
{
if (gameFileInfo.lightDeltaIndices.offset > -1) {
	int	i;

	if (!gameData.render.lights.deltaIndices.Resize (gameFileInfo.lightDeltaIndices.count)) {
		Error ("Not enough memory for light delta index data");
		return -1;
		}
	if (cf.Seek (gameFileInfo.lightDeltaIndices.offset, SEEK_SET)) {
		Error ("Error seeking to light delta index data\n(file damaged or invalid)");
		return -1;
		}
	gameData.render.lights.nStatic = gameFileInfo.lightDeltaIndices.count;
	if (gameTopFileInfo.fileinfoVersion < 29) {
#if TRACE
		console.printf (CON_DBG, "Warning: Old mine version.  Not reading gameData.render.lights.deltaIndices info.\n");
#endif
		Int3();	//shouldn't be here!!!
		return 0;
		}
	else {
		for (i = 0; i < gameFileInfo.lightDeltaIndices.count; i++) {
			//PrintLog ("reading DL index %d\n", i);
			ReadLightDeltaIndex (gameData.render.lights.deltaIndices [i], cf);
			}
		}
	gameData.render.lights.deltaIndices.SortAscending ();
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLightDeltaInfo (CFile& cf)
{
if (gameFileInfo.lightDeltas.offset > -1) {
	int	i;

#if TRACE
	console.printf(CON_DBG, "   loading light data ...\n");
#endif
	if (!gameData.render.lights.deltas.Resize (gameFileInfo.lightDeltas.count)) {
		Error ("Not enough memory for light delta data");
		return -1;
		}
	if (cf.Seek (gameFileInfo.lightDeltas.offset, SEEK_SET)) {
		Error ("Error seeking to light delta data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.lightDeltas.count; i++) {
		if (gameTopFileInfo.fileinfoVersion >= 29) 
			ReadLightDelta (gameData.render.lights.deltas + i, cf);
		else {
#if TRACE
			console.printf (CON_DBG, "Warning: Old mine version.  Not reading delta light info.\n");
#endif
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static void CheckAndLinkObjects (void)
{
	int		i, nObjSeg;
	CObject	*objP = OBJECTS.Buffer ();

for (i = 0; i < gameFileInfo.objects.count; i++, objP++) {
	objP->info.nNextInSeg = objP->info.nPrevInSeg = -1;
	if (objP->info.nType != OBJ_NONE) {
		nObjSeg = objP->info.nSegment;
		if ((nObjSeg < 0) || (nObjSeg > gameData.segs.nLastSegment))	
			objP->info.nType = OBJ_NONE;
		else {
			objP->info.nSegment = -1;	
			OBJECTS [i].LinkToSeg (nObjSeg);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Make sure non-transparent doors are set correctly.
static void CheckAndFixDoors (void)
{
	int	i, j;
	CSide	*sideP;

for (i = 0; i < gameData.segs.nSegments; i++) {
	sideP = SEGMENTS [i].m_sides;
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		CWall* wallP = sideP->Wall ();
		if (!wallP || (wallP->nClip == -1))
			continue;
		if (gameData.walls.animP [wallP->nClip].flags & WCF_TMAP1) {
			sideP->m_nBaseTex = gameData.walls.animP [wallP->nClip].frames [0];
			sideP->m_nOvlTex = 0;
			}
		}
	}
}

// -----------------------------------------------------------------------------
//go through all walls, killing references to invalid triggers
static void CheckAndFixWalls (void)
{
	int		i;
	short		nSegment, nSide;
	CWall*	wallP;

for (i = 0; i < gameData.walls.nWalls; i++)
	if (WALLS [i].nTrigger >= gameData.trigs.nTriggers) {
#if TRACE
		console.printf (CON_DBG,"Removing reference to invalid CTrigger %d from CWall %d\n",WALLS [i].nTrigger,i);
#endif
		WALLS [i].nTrigger = NO_TRIGGER;	//kill CTrigger
		}
if (gameTopFileInfo.fileinfoVersion < 17) {
	for (nSegment = 0; nSegment <= gameData.segs.nLastSegment; nSegment++)
		for (nSide = 0; nSide < 6; nSide++)
			if ((wallP = SEGMENTS [nSegment].Wall (nSide))) {
				wallP->nSegment = nSegment;
				wallP->nSide = nSide;
				}
	}
}

// -----------------------------------------------------------------------------
//go through all triggers, killing unused ones
static void CheckAndFixTriggers (void)
{
	int	i, j;
	short	nSegment, nSide, nWall;

for (i = 0; i < gameData.trigs.nTriggers; ) {
	//	Find which CWall this CTrigger is connected to.
	for (j = 0; j < gameData.walls.nWalls; j++)
		if (WALLS [j].nTrigger == i)
			break;
#ifdef EDITOR
	if (j == gameData.walls.nWalls) {
#if TRACE
		console.printf (CON_DBG,"Removing unreferenced CTrigger %d\n",i);
#endif
		RemoveTriggerNum (i);
		}
	else
#endif
		i++;
	}

for (i = 0; i < gameData.walls.nWalls; i++)
	WALLS [i].controllingTrigger = -1;

//	MK, 10/17/95: Make walls point back at the triggers that control them.
//	Go through all triggers, stuffing controllingTrigger field in WALLS.

CTrigger* trigP = TRIGGERS.Buffer ();
for (i = 0; i < gameData.trigs.nTriggers; i++, trigP++) {
	for (j = 0; j < trigP->nLinks; j++) {
		nSegment = trigP->segments [j];
		nSide = trigP->sides [j];
		nWall = SEGMENTS [nSegment].WallNum (nSide);
		//check to see that if a CTrigger requires a CWall that it has one,
		//and if it requires a botGen that it has one
		if (trigP->nType == TT_MATCEN) {
			if (SEGMENTS [nSegment].m_nType != SEGMENT_IS_ROBOTMAKER)
				continue;		//botGen CTrigger doesn'i point to botGen
			}
		else if ((trigP->nType != TT_LIGHT_OFF) && (trigP->nType != TT_LIGHT_ON)) { //light triggers don't require walls
			if (IS_WALL (nWall))
				WALLS [nWall].controllingTrigger = i;
			else {
				Int3();	//	This is illegal.  This ttrigger requires a CWall
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Load game 
// Loads all the relevant data for a level.
// If level != -1, it loads the filename with extension changed to .min
// Otherwise it loads the appropriate level mine.
// returns 0=everything ok, 1=old version, -1=error
int LoadMineDataCompiled (CFile& cf, int bFileInfo)
{
	int 	nStartOffset;

nStartOffset = cf.Tell ();
InitGameFileInfo ();
if (ReadGameFileInfo (cf, nStartOffset))
	return -1;
if (ReadLevelInfo (cf))
	return -1;
gameStates.render.bD2XLights = gameStates.app.bD2XLevel && (gameTopFileInfo.fileinfoVersion >= 34);
if (bFileInfo)
	return 0;

gameData.objs.nNextSignature = 0;
gameData.render.lights.nStatic = 0;
nGameSaveOrgRobots = 0;
nGameSavePlayers = 0;
if (ReadObjectInfo (cf))
	return -1;
if (ReadWallInfo (cf))
	return -1;
if (ReadDoorInfo (cf))
	return -1;
if (ReadTriggerInfo (cf))
	return -1;
if (ReadReactorInfo (cf))
	return -1;
if (ReadBotGenInfo (cf))
	return -1;
if (ReadEquipGenInfo (cf))
	return -1;
if (ReadLightDeltaIndexInfo (cf))
	return -1;
if (ReadLightDeltaInfo (cf))
	return -1;
ClearLightSubtracted ();
ResetObjects (gameFileInfo.objects.count);
CheckAndLinkObjects ();
ClearTransientObjects (1);		//1 means clear proximity bombs
CheckAndFixDoors ();
//gameData.walls.nOpenDoors = gameFileInfo.doors.count;
gameData.trigs.nTriggers = gameFileInfo.triggers.count;
gameData.walls.nWalls = gameFileInfo.walls.count;
CheckAndFixWalls ();
CheckAndFixTriggers ();
gameData.matCens.nBotCenters = gameFileInfo.botGen.count;
FixObjectSegs ();
#if DBG
dump_mine_info ();
#endif
if ((gameTopFileInfo.fileinfoVersion < GAME_VERSION) && 
	 ((gameTopFileInfo.fileinfoVersion != 25) || (GAME_VERSION != 26)))
	return 1;		//means old version
return 0;
}

// ----------------------------------------------------------------------------

int CheckSegmentConnections(void);

extern void	SetAmbientSoundFlags(void);


#define LEVEL_FILE_VERSION      8
//1 -> 2  add palette name
//2 -> 3  add control center explosion time
//3 -> 4  add reactor strength
//4 -> 5  killed hostage text stuff
//5 -> 6  added gameData.segs.secret.nReturnSegment and gameData.segs.secret.returnOrient
//6 -> 7  added flickering lights
//7 -> 8  made version 8 to be not compatible with D2 1.0 & 1.1

#if DBG
char *Level_being_loaded=NULL;
#endif

int no_oldLevel_file_error=0;

// ----------------------------------------------------------------------------
//loads a level (.LVL) file from disk
//returns 0 if success, else error code
int LoadLevelSub (char * pszFilename, int nLevel)
{
#ifdef EDITOR
	int bUseCompiledLevel = 1;
#endif
	CFile cf;
	char filename [128];
	int sig, nMineDataOffset, nGameDataOffset;
	int nError;
	//int i;

SetDataVersion (-1);
gameData.segs.bHaveSlideSegs = 0;
if (gameData.app.nGameMode & GM_NETWORK) {
	memset (gameData.multiplayer.maxPowerupsAllowed, 0, sizeof (gameData.multiplayer.maxPowerupsAllowed));
	memset (gameData.multiplayer.powerupsInMine, 0, sizeof (gameData.multiplayer.powerupsInMine));
	}
#if DBG
Level_being_loaded = pszFilename;
#endif

gameStates.render.nMeshQuality = gameOpts->render.nMeshQuality;

reloadLevel:

strcpy (filename, pszFilename);
if (!cf.Open (filename, "", "rb", gameStates.app.bD1Mission))
	return 1;

strcpy(gameData.segs.szLevelFilename, filename);

//	#ifdef NEWDEMO
//	if (gameData.demo.nState == ND_STATE_RECORDING)
//		NDRecordStartDemo();
//	#endif

sig = cf.ReadInt ();
gameData.segs.nLevelVersion = cf.ReadInt ();
gameStates.app.bD2XLevel = (gameData.segs.nLevelVersion >= 10);
#if TRACE
console.printf (CON_DBG, "gameData.segs.nLevelVersion = %d\n", gameData.segs.nLevelVersion);
#endif
nMineDataOffset = cf.ReadInt ();
nGameDataOffset = cf.ReadInt ();

Assert(sig == MAKE_SIG('P','L','V','L'));
if (gameData.segs.nLevelVersion >= 8) {    //read dummy data
	cf.ReadInt ();
	cf.ReadShort ();
	cf.ReadByte ();
}

if (gameData.segs.nLevelVersion < 5)
	cf.ReadInt ();       //was hostagetext_offset

if (gameData.segs.nLevelVersion > 1) {
	cf.GetS (szCurrentLevelPalette, sizeof (szCurrentLevelPalette));
	if (szCurrentLevelPalette [strlen(szCurrentLevelPalette) - 1] == '\n')
		szCurrentLevelPalette [strlen(szCurrentLevelPalette) - 1] = 0;
}
if ((gameData.segs.nLevelVersion <= 1) || (szCurrentLevelPalette [0] == 0)) // descent 1 level
	strcpy (szCurrentLevelPalette, DEFAULT_LEVEL_PALETTE); //D1_PALETTE

if (gameData.segs.nLevelVersion >= 3)
	gameStates.app.nBaseCtrlCenExplTime = cf.ReadInt ();
else
	gameStates.app.nBaseCtrlCenExplTime = DEFAULT_CONTROL_CENTER_EXPLOSION_TIME;

if (gameData.segs.nLevelVersion >= 4)
	gameData.reactor.nStrength = cf.ReadInt ();
else
	gameData.reactor.nStrength = -1;  //use old defaults

if (gameData.segs.nLevelVersion >= 7) {
	int i;

#if TRACE
console.printf (CON_DBG, "   loading dynamic lights ...\n");
#endif
gameData.render.lights.flicker.nLights = cf.ReadInt ();
Assert ((gameData.render.lights.flicker.nLights >= 0) && (gameData.render.lights.flicker.nLights < MAX_FLICKERING_LIGHTS));
for (i = 0; i < gameData.render.lights.flicker.nLights; i++)
	ReadVariableLight (&gameData.render.lights.flicker.lights [i], cf);
}
else
	gameData.render.lights.flicker.nLights = 0;

if (gameData.segs.nLevelVersion < 6) {
	gameData.segs.secret.nReturnSegment = 0;
	gameData.segs.secret.returnOrient = CFixMatrix::IDENTITY;
	}
else {
	gameData.segs.secret.nReturnSegment = cf.ReadInt ();
	for (int i = 0; i < 9; i++)
		gameData.segs.secret.returnOrient [i] = cf.ReadInt ();
	}

//NOTE LINK TO ABOVE!!
cf.Seek (nGameDataOffset, SEEK_SET);
nError = LoadMineDataCompiled (cf, 1);
cf.Seek (nMineDataOffset, SEEK_SET);
nError = LoadMineSegmentsCompiled (cf);
if (nError == -1) {   //error!!
	cf.Close ();
	return 2;
	}
cf.Seek (nGameDataOffset, SEEK_SET);
nError = LoadMineDataCompiled (cf, 0);
if (nError == -1) {   //error!!
	cf.Close ();
	return 3;
	}
cf.Close ();

if (!meshBuilder.Build (nLevel))
	goto reloadLevel;

if (!gameStates.app.bNostalgia) {
#if !SHADOWS
	if (SHOW_DYN_LIGHT || !gameStates.app.bD2XLevel)
#endif
	 {
		AddDynGeometryLights ();
		ComputeNearestLights (nLevel);
		if (gameStates.render.bPerPixelLighting) {
			lightmapManager.Create (nLevel);
			if (lightmapManager.HaveLightmaps ())
				meshBuilder.RebuildLightmapTexCoord ();	//rebuild to create proper lightmap texture coordinates
			else
				gameOpts->render.bUseLightmaps = 0;
			}
		}
	}

SetAmbientSoundFlags ();
#ifdef EDITOR
write_game_text_file(filename);
if (Errors_in_mine) {
	if (IsRealLevel(filename)) {
		char  ErrorMessage [200];

		sprintf(ErrorMessage, TXT_MINE_ERRORS, Errors_in_mine, Level_being_loaded);
		StopTime();
		paletteManager.LoadEffect ();
		MsgBox(NULL, 1, TXT_CONTINUE, ErrorMessage);
		StartTime();
	} else {
#if TRACE
		console.printf (1, TXT_MINE_ERRORS, Errors_in_mine, Level_being_loaded);
#endif
	}
}
#endif

#ifdef EDITOR
//If an old version, ask the use if he wants to save as new version
if (!no_oldLevel_file_error && (gameStates.app.nFunctionMode == FMODE_EDITOR) && 
    (((LEVEL_FILE_VERSION > 3) && gameData.segs.nLevelVersion < LEVEL_FILE_VERSION) || nError == 1 || nError == 1)) {
	char  ErrorMessage [200];

	sprintf(ErrorMessage,
				"You just loaded a old version\n"
				"level.  Would you like to save\n"
				"it as a current version level?");

	StopTime();
	paletteManager.LoadEffect ();
	if (MsgBox(NULL, 2, "Don't Save", "Save", ErrorMessage)==1)
		SaveLevel(filename);
	StartTime();
}
#endif

#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR)
	editor_status("Loaded NEW mine %s, \"%s\"",filename,gameData.missions.szCurrentLevel);
#endif

#ifdef EDITOR
if (CheckSegmentConnections())
	MsgBox("ERROR", 1, "Ok", 
			"Connectivity errors detected in\n"
			"mine.  See monochrome screen for\n"
			"details, and contact Matt or Mike.");
#endif

return 0;
}

#ifdef EDITOR
void GetLevelName()
{
//NO_UI!!!	UI_WINDOW 				*NameWindow = NULL;
//NO_UI!!!	UI_GADGET_INPUTBOX	*NameText;
//NO_UI!!!	UI_GADGET_BUTTON 		*QuitButton;
//NO_UI!!!
//NO_UI!!!	// Open a window with a quit button
//NO_UI!!!	NameWindow = ui_open_window(20, 20, 300, 110, WIN_DIALOG);
//NO_UI!!!	QuitButton = ui_add_gadget_button(NameWindow, 150-24, 60, 48, 40, "Done", NULL);
//NO_UI!!!
//NO_UI!!!	ui_wprintf_at(NameWindow, 10, 12,"Please enter a name for this mine:");
//NO_UI!!!	NameText = ui_add_gadget_inputbox(NameWindow, 10, 30, LEVEL_NAME_LEN, LEVEL_NAME_LEN, gameData.missions.szCurrentLevel);
//NO_UI!!!
//NO_UI!!!	NameWindow->keyboard_focus_gadget = reinterpret_cast<UI_GADGET*> (NameText);
//NO_UI!!!	QuitButton->hotkey = KEY_ENTER;
//NO_UI!!!
//NO_UI!!!	ui_gadget_calc_keys(NameWindow);
//NO_UI!!!
//NO_UI!!!	while (!QuitButton->pressed && last_keypress!=KEY_ENTER) {
//NO_UI!!!		ui_mega_process();
//NO_UI!!!		ui_window_do_gadgets(NameWindow);
//NO_UI!!!	}
//NO_UI!!!
//NO_UI!!!	strcpy(gameData.missions.szCurrentLevel, NameText->text);
//NO_UI!!!
//NO_UI!!!	if (NameWindow!=NULL) {
//NO_UI!!!		ui_close_window(NameWindow);
//NO_UI!!!		NameWindow = NULL;
//NO_UI!!!	}
//NO_UI!!!

	CMenuItem m [2];

	memset (m, 0, sizeof (m));
	m [0].nType = NM_TYPE_TEXT; 
	m [0].text = "Please enter a name for this mine:";
	m [1].nType = NM_TYPE_INPUT; 
	m [1].text = gameData.missions.szCurrentLevel; 
	m [1].nTextLen = LEVEL_NAME_LEN;

	ExecMenu(NULL, "Enter mine name", 2, m, NULL);

}
#endif


#ifdef EDITOR

int	Errors_in_mine;

// -----------------------------------------------------------------------------

int CountDeltaLightRecords(void)
{
	int	i;
	int	total = 0;

	for (i=0; i<gameData.render.lights.nStatic; i++) {
		total += gameData.render.lights.deltaIndices [i].count;
	}

	return total;

}

// -----------------------------------------------------------------------------
// Save game
int SaveGameData(FILE * SaveFile)
{
	int  player.offset, CObject.offset, walls.offset, doors.offset, triggers.offset, control.offset, botGen.offset; //, links.offset;
	int	gameData.render.lights.deltaIndices.offset, deltaLight.offset;
	int start_offset,end_offset;

	start_offset = ftell(SaveFile);

	//===================== SAVE FILE INFO ========================

	gameFileInfo.fileinfo_signature =	0x6705;
	gameFileInfo.fileinfoVersion	=	GAME_VERSION;
	gameFileInfo.level					=  gameData.missions.nCurrentLevel;
	gameFileInfo.fileinfo_sizeof		=	sizeof(gameFileInfo);
	gameFileInfo.player.offset		=	-1;
	gameFileInfo.player.size		=	sizeof(CPlayerData);
	gameFileInfo.objects.offset		=	-1;
	gameFileInfo.objects.count		=	gameData.objs.nLastObject [0]+1;
	gameFileInfo.objects.size		=	sizeof(CObject);
	gameFileInfo.walls.offset			=	-1;
	gameFileInfo.walls.count		=	gameData.walls.nWalls;
	gameFileInfo.walls.size			=	sizeof(CWall);
	gameFileInfo.doors.offset			=	-1;
	gameFileInfo.doors.count		=	gameData.walls.nOpenDoors;
	gameFileInfo.doors.size			=	sizeof(CActiveDoor);
	gameFileInfo.triggers.offset		=	-1;
	gameFileInfo.triggers.count	=	gameData.trigs.nTriggers;
	gameFileInfo.triggers.size		=	sizeof(CTrigger);
	gameFileInfo.control.offset		=	-1;
	gameFileInfo.control.count		=  1;
	gameFileInfo.control.size		=  sizeof(tReactorTriggers);
 	gameFileInfo.botGen.offset		=	-1;
	gameFileInfo.botGen.count		=	gameData.matCens.nBotCenters;
	gameFileInfo.botGen.size		=	sizeof(tMatCenInfo);

 	gameFileInfo.lightDeltaIndices.offset		=	-1;
	gameFileInfo.lightDeltaIndices.count		=	gameData.render.lights.nStatic;
	gameFileInfo.lightDeltaIndices.size		=	sizeof(CLightDeltaIndex);

 	gameFileInfo.lightDeltas.offset		=	-1;
	gameFileInfo.lightDeltas.count	=	CountDeltaLightRecords();
	gameFileInfo.lightDeltas.size		=	sizeof(CLightDelta);

	// Write the fileinfo
	fwrite(&gameFileInfo, sizeof(gameFileInfo), 1, SaveFile);

	// Write the mine name
	fprintf(SaveFile,"%s\n",gameData.missions.szCurrentLevel);

	fwrite(&gameData.models.nPolyModels,2,1,SaveFile);
	fwrite(Pof_names,gameData.models.nPolyModels,sizeof(*Pof_names),SaveFile);

	//==================== SAVE PLAYER INFO ===========================

	player.offset = ftell(SaveFile);
	fwrite(&LOCALPLAYER, sizeof(CPlayerData), 1, SaveFile);

	//==================== SAVE OBJECT INFO ===========================

	CObject.offset = ftell(SaveFile);
	//fwrite(&OBJECTS, sizeof(CObject), gameFileInfo.objects.count, SaveFile);
 {
		int i;
		for (i=0;i<gameFileInfo.objects.count;i++)
			writeObject(&OBJECTS [i],SaveFile);
	}

	//==================== SAVE WALL INFO =============================

	walls.offset = ftell(SaveFile);
	fwrite(WALLS, sizeof(CWall), gameFileInfo.walls.count, SaveFile);

	//==================== SAVE DOOR INFO =============================

	doors.offset = ftell(SaveFile);
	fwrite(gameData.walls.activeDoors, sizeof(CActiveDoor), gameFileInfo.doors.count, SaveFile);

	//==================== SAVE TRIGGER INFO =============================

	triggers.offset = ftell(SaveFile);
	fwrite(TRIGGERS, sizeof(CTrigger), gameFileInfo.triggers.count, SaveFile);

	//================ SAVE CONTROL CENTER TRIGGER INFO ===============

	control.offset = ftell(SaveFile);
	fwrite(&gameData.reactor.triggers, sizeof(tReactorTriggers), 1, SaveFile);


	//================ SAVE MATERIALIZATION CENTER TRIGGER INFO ===============

	botGen.offset = ftell(SaveFile);
	fwrite(gameData.matCens.botGens, sizeof(tMatCenInfo), gameFileInfo.botGen.count, SaveFile);

	//================ SAVE DELTA LIGHT INFO ===============
	gameData.render.lights.deltaIndices.offset = ftell(SaveFile);
	fwrite(gameData.render.lights.deltaIndices, sizeof(CLightDeltaIndex), gameFileInfo.lightDeltaIndices.count, SaveFile);

	deltaLight.offset = ftell(SaveFile);
	fwrite(gameData.render.lights.deltas, sizeof(CLightDelta), gameFileInfo.lightDeltas.count, SaveFile);

	//============= REWRITE FILE INFO, TO SAVE OFFSETS ===============

	// Update the offset fields
	gameFileInfo.player.offset		=	player.offset;
	gameFileInfo.objects.offset		=	CObject.offset;
	gameFileInfo.walls.offset			=	walls.offset;
	gameFileInfo.doors.offset			=	doors.offset;
	gameFileInfo.triggers.offset		=	triggers.offset;
	gameFileInfo.control.offset		=	control.offset;
	gameFileInfo.botGen.offset		=	botGen.offset;
	gameFileInfo.lightDeltaIndices.offset	=	gameData.render.lights.deltaIndices.offset;
	gameFileInfo.lightDeltas.offset	=	deltaLight.offset;


	end_offset = ftell(SaveFile);

	// Write the fileinfo
	fseek( SaveFile, start_offset, SEEK_SET);  // Move to TOF
	fwrite(&gameFileInfo, sizeof(gameFileInfo), 1, SaveFile);

	// Go back to end of data
	fseek(SaveFile,end_offset, SEEK_SET);

	return 0;
}

int save_mine_data(FILE * SaveFile);

// -----------------------------------------------------------------------------
// Save game
int saveLevel_sub(char * filename, int compiledVersion)
{
	FILE * SaveFile;
	char temp_filename [128];
	int sig = MAKE_SIG('P','L','V','L'),version=LEVEL_FILE_VERSION;
	int nMineDataOffset=0,nGameDataOffset=0;

	if (!compiledVersion) {
		write_game_text_file(filename);

		if (Errors_in_mine) {
			if (IsRealLevel(filename)) {
				char  ErrorMessage [200];

				sprintf(ErrorMessage, TXT_MINE_ERRORS2, Errors_in_mine);
				StopTime();
				paletteManager.LoadEffect ();
	 
				if (MsgBox(NULL, 2, TXT_CANCEL_SAVE, TXT_DO_SAVE, ErrorMessage)!=1) {
					StartTime();
					return 1;
				}
				StartTime();
			}
		}
		ChangeFilenameExtension(temp_filename,filename,".LVL");
	}
	else
 {
		// macs are using the regular hog/rl2 files for shareware
	}

	SaveFile = fopen(temp_filename, "wb");
	if (!SaveFile)
 {
		char ErrorMessage [256];

		char fname [20];
		_splitpath(temp_filename, NULL, NULL, fname, NULL);

		sprintf(ErrorMessage, \
			"ERROR: Cannot write to '%s'.\nYou probably need to check out a locked\nversion of the file. You should save\nthis under a different filename, and then\ncheck out a locked copy by typing\n\'co -l %s.lvl'\nat the DOS prompt.\n" 
			, temp_filename, fname);
		StopTime();
		paletteManager.LoadEffect ();
		MsgBox(NULL, 1, "Ok", ErrorMessage);
		StartTime();
		return 1;
	}

	if (gameData.missions.szCurrentLevel [0] == 0)
		strcpy(gameData.missions.szCurrentLevel,"Untitled");

	ClearTransientObjects(1);		//1 means clear proximity bombs

	compressObjects();		//after this, gameData.objs.nLastObject [0] == num OBJECTS

	//make sure CPlayerData is in a CSegment
	if (!UpdateObjectSeg(OBJECTS + gameData.multiplayer.players [0].nObject)) {
		if (gameData.objs.consoleP->info.nSegment > gameData.segs.nLastSegment)
			gameData.objs.consoleP->info.nSegment = 0;
		gameData.objs.consoleP->info.position.vPos = SEGMENTS [gameData.objs.consoleP->info.nSegment].Center ();
	}
	FixObjectSegs();

	//Write the header

	gs_write_int(sig,SaveFile);
	gs_write_int(version,SaveFile);

	//save placeholders
	gs_write_int(nMineDataOffset,SaveFile);
	gs_write_int(nGameDataOffset,SaveFile);

	//Now write the damn data

	//write the version 8 data (to make file unreadable by 1.0 & 1.1)
	gs_write_int(gameData.time.xGame,SaveFile);
	gs_write_short(gameData.app.nFrameCount,SaveFile);
	gs_write_byte(gameData.time.xFrame,SaveFile);

	// Write the palette file name
	fprintf(SaveFile,"%s\n",szCurrentLevelPalette);

	gs_write_int(gameStates.app.nBaseCtrlCenExplTime,SaveFile);
	gs_write_int(gameData.reactor.nStrength,SaveFile);

	gs_write_int(gameData.render.lights.flicker.nLights,SaveFile);
	fwrite(gameData.render.lights.flicker.lights,sizeof(*gameData.render.lights.flicker.lights),gameData.render.lights.flicker.nLights,SaveFile);

	gs_write_int(gameData.segs.secret.nReturnSegment, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.RVec ()[X], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.RVec ()[Y], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.RVec ()[Z], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.FVec ()[X], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.FVec ()[Y], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.FVec ()[Z], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.UVec ()[X], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.UVec ()[Y], SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.UVec ()[Z], SaveFile);

	nMineDataOffset = ftell(SaveFile);
	if (!compiledVersion)
		save_mine_data(SaveFile);
	else
		save_mine_data_compiled(SaveFile);
	nGameDataOffset = ftell(SaveFile);
	SaveGameData(SaveFile);

	fseek(SaveFile,sizeof(sig)+sizeof(version), SEEK_SET);
	gs_write_int(nMineDataOffset,SaveFile);
	gs_write_int(nGameDataOffset,SaveFile);

	//==================== CLOSE THE FILE =============================
	fclose(SaveFile);

	if (!compiledVersion) {
		if (gameStates.app.nFunctionMode == FMODE_EDITOR)
			editor_status("Saved mine %s, \"%s\"",filename,gameData.missions.szCurrentLevel);
	}

	return 0;

}

// -----------------------------------------------------------------------------

#if 0 //dunno - 3rd party stuff?
extern void compress_uv_coordinates_all(void);
#endif

int SaveLevel(char * filename)
{
	int r1;

	// Save Normal version...
	r1 = saveLevel_sub(filename, 0);

	// Save compiled version...
	saveLevel_sub(filename, 1);

	return r1;
}

#endif	//EDITOR

#if DBG
void dump_mine_info(void)
{
	int	nSegment, nSide;
	fix	min_u, max_u, min_v, max_v, min_l, max_l, max_sl;

	min_u = I2X (1000);
	min_v = min_u;
	min_l = min_u;

	max_u = -min_u;
	max_v = max_u;
	max_l = max_u;

	max_sl = 0;

	for (nSegment=0; nSegment<=gameData.segs.nLastSegment; nSegment++) {
		for (nSide=0; nSide<MAX_SIDES_PER_SEGMENT; nSide++) {
			int	vertnum;
			CSide	*sideP = &SEGMENTS [nSegment].m_sides [nSide];

			if (SEGMENTS [nSegment].m_xAvgSegLight > max_sl)
				max_sl = SEGMENTS [nSegment].m_xAvgSegLight;

			for (vertnum=0; vertnum<4; vertnum++) {
				if (sideP->m_uvls [vertnum].u < min_u)
					min_u = sideP->m_uvls [vertnum].u;
				else if (sideP->m_uvls [vertnum].u > max_u)
					max_u = sideP->m_uvls [vertnum].u;

				if (sideP->m_uvls [vertnum].v < min_v)
					min_v = sideP->m_uvls [vertnum].v;
				else if (sideP->m_uvls [vertnum].v > max_v)
					max_v = sideP->m_uvls [vertnum].v;

				if (sideP->m_uvls [vertnum].l < min_l)
					min_l = sideP->m_uvls [vertnum].l;
				else if (sideP->m_uvls [vertnum].l > max_l)
					max_l = sideP->m_uvls [vertnum].l;
			}

		}
	}
}

#endif

#ifdef EDITOR

// -----------------------------------------------------------------------------
//read in every level in mission and save out compiled version 
void save_all_compiledLevels(void)
{
DoLoadSaveLevels(1);
}

// -----------------------------------------------------------------------------
//read in every level in mission
void LoadAllLevels(void)
{
DoLoadSaveLevels(0);
}

// -----------------------------------------------------------------------------

void DoLoadSaveLevels(int save)
{
	int level_num;

	if (! SafetyCheck())
		return;

	no_oldLevel_file_error=1;

	for (level_num=1;level_num<=gameData.missions.nLastLevel;level_num++) {
		LoadLevelSub(gameData.missions.szLevelNames [level_num-1]);
		paletteManager.Load(szCurrentLevelPalette,1,1,0);		//don't change screen
		if (save)
			saveLevel_sub(gameData.missions.szLevelNames [level_num-1],1);
	}

	for (level_num = -1; level_num >= gameData.missions.nLastSecretLevel; level_num--) {
		LoadLevelSub(gameData.missions.szSecretLevelNames [-level_num-1]);
		paletteManager.Load(szCurrentLevelPalette,1,1,0);		//don't change screen
		if (save)
			saveLevel_sub (gameData.missions.szSecretLevelNames [-level_num-1],1);
	}

	no_oldLevel_file_error=0;

}

#endif
