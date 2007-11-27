/* $Id: tObject.c, v 1.9 2003/10/04 03:14:47 btb Exp $ */
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

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inferno.h"
#include "game.h"
#include "gr.h"
#include "stdlib.h"
#include "bm.h"
//#include "error.h"
#include "mono.h"
#include "3d.h"
#include "segment.h"
#include "texmap.h"
#include "laser.h"
#include "key.h"
#include "gameseg.h"
#include "textures.h"

#include "lightning.h"
#include "object.h"
#include "objsmoke.h"
#include "physics.h"
#include "slew.h"	
#include "render.h"
#include "wall.h"
#include "vclip.h"
#include "polyobj.h"
#include "fireball.h"
#include "laser.h"
#include "error.h"
#include "pa_enabl.h"
#include "ai.h"
#include "hostage.h"
#include "morph.h"
#include "cntrlcen.h"
#include "powerup.h"
#include "fuelcen.h"
#include "endlevel.h"
#include "timer.h"

#include "sounds.h"
#include "collide.h"

#include "lighting.h"
#include "interp.h"
#include "newdemo.h"
#include "player.h"
#include "weapon.h"
#include "network.h"
#include "newmenu.h"
#include "gauges.h"
#include "multi.h"
#include "menu.h"
#include "args.h"
#include "text.h"
#include "piggy.h"
#include "switch.h"
#include "gameseq.h"
#include "vecmat.h"
#include "particles.h"
#include "hudmsg.h"
#include "oof.h"
#include "sphere.h"
#include "globvars.h"
#ifdef TACTILE
#include "tactile.h"
#endif
#include "ogl_defs.h"
#include "input.h"
#include "automap.h"
#include "u_mem.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#ifdef _3DFX
#include "3dfx_des.h"
#endif

#define LIMIT_PHYSICS_FPS	0

extern vmsVector playerThrust;
extern int bSpeedBost;

void DetachAllObjects (tObject *parent);
void DetachOneObject (tObject *sub);
int FreeObjectSlots (int num_used);

/*
 *  Global variables
 */

//Data for gameData.objs.objects

// -- Object stuff

//info on the various types of gameData.objs.objects
#ifdef _DEBUG
tObject	Object_minus_one;
tObject	*dbgObjP = NULL;
#endif

#define fabsf(_f)	(float) fabs (_f)

//------------------------------------------------------------------------------
// grsBitmap *robot_bms [MAX_ROBOT_BITMAPS];	//all bitmaps for all robots

// int robot_bm_nums [MAX_ROBOT_TYPES];		//starting bitmap num for each robot
// int robot_n_bitmaps [MAX_ROBOT_TYPES];		//how many bitmaps for each robot

// char *gameData.bots.names [MAX_ROBOT_TYPES];		//name of each robot

//--unused-- int NumRobotTypes = 0;

int bPrintObjectInfo = 0;
//@@int ObjectViewer = 0;

//tObject * SlewObject = NULL;	// Object containing slew tObject info.

//--unused-- int Player_controllerType = 0;

tWindowRenderedData windowRenderedData [MAX_RENDERED_WINDOWS];

#ifdef _DEBUG
char	szObjectTypeNames [MAX_OBJECT_TYPES][9] = {
	"WALL    ", 
	"FIREBALL", 
	"ROBOT   ", 
	"HOSTAGE ", 
	"PLAYER  ", 
	"WEAPON  ", 
	"CAMERA  ", 
	"POWERUP ", 
	"DEBRIS  ", 
	"CNTRLCEN", 
	"FLARE   ", 
	"CLUTTER ", 
	"GHOST   ", 
	"LIGHT   ", 
	"COOP    ", 
	"MARKER  ", 
	"CAMBOT  ",
	"M-BALL  "
};
#endif

// -----------------------------------------------------------------------------

float ObjectDamage (tObject *objP)
{
	float	fDmg;
	fix	xMaxShields;

if (objP->nType == OBJ_PLAYER)
	fDmg = f2fl (gameData.multiplayer.players [objP->id].shields) / 100;
else if (objP->nType == OBJ_ROBOT) {
	xMaxShields = RobotDefaultShields (objP);
	fDmg = f2fl (objP->shields) / f2fl (xMaxShields);
#if 0
	if (gameData.bots.info [0][objP->id].companion)
		fDmg /= 2;
#endif
	}
else if (objP->nType == OBJ_CNTRLCEN)
	fDmg = f2fl (objP->shields) / f2fl (ReactorStrength ());
else if ((objP->nType == 255) || (objP->flags & (OF_EXPLODING | OF_SHOULD_BE_DEAD | OF_DESTROYED | OF_ARMAGEDDON)))
	fDmg = 0.0f;
else
	fDmg = 1.0f;
return (fDmg > 1.0f) ? 1.0f : (fDmg < 0.0f) ? 0.0f : fDmg;
}

// -----------------------------------------------------------------------------

int FindBoss (int nBossObj)
{
	int	i, j = BOSS_COUNT;

for (i = 0; i < j; i++)
	if (gameData.boss [i].nObject == nBossObj)
		return i;
return -1;
}

//------------------------------------------------------------------------------

void InitGateIntervals (void)
{
	int	i;

for (i = 0; i < MAX_BOSS_COUNT; i++)
	gameData.boss [i].nGateInterval = F1_0 * 4 - gameStates.app.nDifficultyLevel * i2f (2) / 3;
}

//------------------------------------------------------------------------------

#ifdef _DEBUG
//set viewer tObject to next tObject in array
void ObjectGotoNextViewer ()
{
	int i, nStartObj = 0;

	nStartObj = OBJ_IDX (gameData.objs.viewer);		//get viewer tObject number
	for (i = 0;i<=gameData.objs.nLastObject;i++) {
		nStartObj++;
		if (nStartObj > gameData.objs.nLastObject) nStartObj = 0;

		if (gameData.objs.objects [nStartObj].nType != OBJ_NONE)	{
			gameData.objs.viewer = gameData.objs.objects + nStartObj;
			return;
		}
	}
	Error ("Couldn't find a viewer tObject!");
}

//------------------------------------------------------------------------------
//set viewer tObject to next tObject in array
void ObjectGotoPrevViewer ()
{
	int i, nStartObj = 0;

nStartObj = OBJ_IDX (gameData.objs.viewer);		//get viewer tObject number
for (i = 0; i<=gameData.objs.nLastObject; i++) {
	nStartObj--;
	if (nStartObj < 0) nStartObj = gameData.objs.nLastObject;
	if (gameData.objs.objects [nStartObj].nType != OBJ_NONE)	{
		gameData.objs.viewer = gameData.objs.objects + nStartObj;
		return;
		}
	}
Error ("Couldn't find a viewer tObject!");
}
#endif

//------------------------------------------------------------------------------

tObject *ObjFindFirstOfType (int nType)
 {
  int i;
  tObject	*objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->nType==nType)
	 return (objP);
  return ((tObject *)NULL);
 }


int obj_return_num_ofType (int nType)
 {
  int i, count = 0;
	tObject *objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->nType==nType)
	 count++;
  return (count);
 }


int obj_return_num_ofTypeid (int nType, int id)
 {
  int i, count = 0;
	tObject	*objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->nType==nType && objP->id==id)
	 count++;
  return (count);
 }

//------------------------------------------------------------------------------
// These variables are used to keep a list of the 3 closest robots to the viewer.
// The code works like this: Every time render tObject is called with a polygon model, 
// it finds the distance of that robot to the viewer.  If this distance if within 10
// segments of the viewer, it does the following: If there aren't already 3 robots in
// the closet-robots list, it just sticks that tObject into the list along with its distance.
// If the list already contains 3 robots, then it finds the robot in that list that is
// farthest from the viewer. If that tObject is farther than the tObject currently being
// rendered, then the new tObject takes over that far tObject's slot.  *Then* after all
// gameData.objs.objects are rendered, object_render_targets is called an it draws a target on top
// of all the gameData.objs.objects.

//091494: #define MAX_CLOSE_ROBOTS 3
//--unused-- static int Object_draw_lock_boxes = 0;
//091494: static int Object_num_close = 0;
//091494: static tObject * Object_close_ones [MAX_CLOSE_ROBOTS];
//091494: static fix Object_closeDistance [MAX_CLOSE_ROBOTS];

//091494: set_closeObjects (tObject *objP)
//091494: {
//091494: 	fix dist;
//091494:
//091494: 	if ((objP->nType != OBJ_ROBOT) || (Object_draw_lock_boxes == 0))
//091494: 		return;
//091494:
//091494: 	// The following code keeps a list of the 10 closest robots to the
//091494: 	// viewer.  See comments in front of this function for how this works.
//091494: 	dist = VmVecDist (&objP->position.vPos, &gameData.objs.viewer->position.vPos);
//091494: 	if (dist < i2f (20*10))	{			
//091494: 		if (Object_num_close < MAX_CLOSE_ROBOTS)	{
//091494: 			Object_close_ones [Object_num_close] = obj;
//091494: 			Object_closeDistance [Object_num_close] = dist;
//091494: 			Object_num_close++;
//091494: 		} else {
//091494: 			int i, farthestRobot;
//091494: 			fix farthestDistance;
//091494: 			// Find the farthest robot in the list
//091494: 			farthestRobot = 0;
//091494: 			farthestDistance = Object_closeDistance [0];
//091494: 			for (i=1; i<Object_num_close; i++)	{
//091494: 				if (Object_closeDistance [i] > farthestDistance)	{
//091494: 					farthestDistance = Object_closeDistance [i];
//091494: 					farthestRobot = i;
//091494: 				}
//091494: 			}
//091494: 			// If this tObject is closer to the viewer than
//091494: 			// the farthest in the list, replace the farthest with this tObject.
//091494: 			if (farthestDistance > dist)	{
//091494: 				Object_close_ones [farthestRobot] = obj;
//091494: 				Object_closeDistance [farthestRobot] = dist;
//091494: 			}
//091494: 		}
//091494: 	}
//091494: }


//	------------------------------------------------------------------------------------------------------------------

void CreateSmallFireballOnObject (tObject *objP, fix size_scale, int bSound)
{
	fix			size;
	vmsVector	pos, rand_vec;
	short			nSegment;

pos = objP->position.vPos;
MakeRandomVector (&rand_vec);
VmVecScale (&rand_vec, objP->size / 2);
VmVecInc (&pos, &rand_vec);
size = FixMul (size_scale, F1_0 / 2 + d_rand () * 4 / 2);
nSegment = FindSegByPoint (&pos, objP->nSegment, 1);
if (nSegment != -1) {
	tObject *explObjP = ObjectCreateExplosion (nSegment, &pos, size, VCLIP_SMALL_EXPLOSION);
	if (!explObjP)
		return;
	AttachObject (objP, explObjP);
	if (d_rand () < 8192) {
		fix	vol = F1_0 / 2;
		if (objP->nType == OBJ_ROBOT)
			vol *= 2;
		else if (bSound)
			DigiLinkSoundToObject (SOUND_EXPLODING_WALL, OBJ_IDX (objP), 0, vol);
		}
	}
}

//	------------------------------------------------------------------------------------------------------------------

void CreateVClipOnObject (tObject *objP, fix size_scale, ubyte vclip_num)
{
	fix			size;
	vmsVector	pos, rand_vec;
	short			nSegment;

pos = objP->position.vPos;
MakeRandomVector (&rand_vec);
VmVecScale (&rand_vec, objP->size / 2);
VmVecInc (&pos, &rand_vec);
size = FixMul (size_scale, F1_0 + d_rand ()*4);
nSegment = FindSegByPoint (&pos, objP->nSegment, 1);
if (nSegment != -1) {
	tObject *explObjP = ObjectCreateExplosion (nSegment, &pos, size, vclip_num);
	if (!explObjP)
		return;

	explObjP->movementType = MT_PHYSICS;
	explObjP->mType.physInfo.velocity.p.x = objP->mType.physInfo.velocity.p.x / 2;
	explObjP->mType.physInfo.velocity.p.y = objP->mType.physInfo.velocity.p.y / 2;
	explObjP->mType.physInfo.velocity.p.z = objP->mType.physInfo.velocity.p.z / 2;
	}
}

//------------------------------------------------------------------------------

void CheckAndFixMatrix (vmsMatrix *m);

#define VmAngVecZero(v) (v)->p= (v)->b= (v)->h = 0

void ResetPlayerObject ()
{
	int i;

//Init physics
VmVecZero (&gameData.objs.console->mType.physInfo.velocity);
VmVecZero (&gameData.objs.console->mType.physInfo.thrust);
VmVecZero (&gameData.objs.console->mType.physInfo.rotVel);
VmVecZero (&gameData.objs.console->mType.physInfo.rotThrust);
gameData.objs.console->mType.physInfo.brakes = gameData.objs.console->mType.physInfo.turnRoll = 0;
gameData.objs.console->mType.physInfo.mass = gameData.pig.ship.player->mass;
gameData.objs.console->mType.physInfo.drag = gameData.pig.ship.player->drag;
gameData.objs.console->mType.physInfo.flags |= PF_TURNROLL | PF_LEVELLING | PF_WIGGLE | PF_USES_THRUST;
//Init render info
gameData.objs.console->renderType = RT_POLYOBJ;
gameData.objs.console->rType.polyObjInfo.nModel = gameData.pig.ship.player->nModel;		//what model is this?
gameData.objs.console->rType.polyObjInfo.nSubObjFlags = 0;		//zero the flags
gameData.objs.console->rType.polyObjInfo.nTexOverride = -1;		//no tmap override!
for (i = 0; i < MAX_SUBMODELS; i++)
	VmAngVecZero (gameData.objs.console->rType.polyObjInfo.animAngles + i);
// Clear misc
gameData.objs.console->flags = 0;
}

//------------------------------------------------------------------------------
//make object0 the tPlayer, setting all relevant fields
void InitPlayerObject ()
{
gameData.objs.console->nType = OBJ_PLAYER;
gameData.objs.console->id = 0;					//no sub-types for tPlayer
gameData.objs.console->nSignature = 0;			//tPlayer has zero, others start at 1
gameData.objs.console->size = gameData.models.polyModels [gameData.pig.ship.player->nModel].rad;
gameData.objs.console->controlType = CT_SLEW;			//default is tPlayer slewing
gameData.objs.console->movementType = MT_PHYSICS;		//change this sometime
gameData.objs.console->lifeleft = IMMORTAL_TIME;
gameData.objs.console->attachedObj = -1;
ResetPlayerObject ();
}

//------------------------------------------------------------------------------

void InitIdToOOF (void)
{
memset (gameData.objs.idToOOF, 0, sizeof (gameData.objs.idToOOF));
gameData.objs.idToOOF [MEGAMSL_ID] = OOF_MEGA;
}

//------------------------------------------------------------------------------
//sets up the D2_FREE list & init tPlayer & whatever else
void InitObjects ()
{
	int i;
	tSegment *pSeg;

CollideInit ();
for (i = 0; i < MAX_OBJECTS; i++) {
	gameData.objs.freeList [i] = i;
	gameData.objs.objects [i].nType = OBJ_NONE;
	gameData.objs.objects [i].nSegment =
	gameData.objs.objects [i].cType.explInfo.nNextAttach =
	gameData.objs.objects [i].cType.explInfo.nPrevAttach =
	gameData.objs.objects [i].cType.explInfo.nAttachParent =
	gameData.objs.objects [i].attachedObj = -1;
	gameData.objs.objects [i].flags = 0;
	}
for (i = 0, pSeg = gameData.segs.segments;i<MAX_SEGMENTS;i++, pSeg++)
	pSeg->objects = -1;
gameData.objs.console = 
gameData.objs.viewer = gameData.objs.objects;
InitPlayerObject ();
LinkObject (OBJ_IDX (gameData.objs.console), 0);	//put in the world in tSegment 0
gameData.objs.nObjects = 1;						//just the tPlayer
gameData.objs.nLastObject = 0;
InitIdToOOF ();
}

//------------------------------------------------------------------------------
//after calling initObject (), the network code has grabbed specific
//tObject slots without allocating them.  Go though the gameData.objs.objects & build
//the D2_FREE list, then set the apporpriate globals
void SpecialResetObjects (void)
{
	int i;

gameData.objs.nObjects = MAX_OBJECTS;
gameData.objs.nLastObject = 0;
Assert (gameData.objs.objects [0].nType != OBJ_NONE);		//0 should be used
for (i = MAX_OBJECTS; i--; )
	if (gameData.objs.objects [i].nType == OBJ_NONE)
		gameData.objs.freeList [--gameData.objs.nObjects] = i;
	else
		if (i > gameData.objs.nLastObject)
			gameData.objs.nLastObject = i;
}

//------------------------------------------------------------------------------
#ifdef _DEBUG
int IsObjectInSeg (int nSegment, int objn)
{
	int nObject, count = 0;

for (nObject = gameData.segs.segments [nSegment].objects;
	  nObject != -1;
	  nObject = gameData.objs.objects [nObject].next)	{
	if (count > MAX_OBJECTS) 	{
		Int3 ();
		return count;
		}
	if (nObject==objn) 
		count++;
	}
 return count;
}

//------------------------------------------------------------------------------

int SearchAllSegsForObject (int nObject)
{
	int i;
	int count = 0;

	for (i = 0; i<=gameData.segs.nLastSegment; i++) {
		count += IsObjectInSeg (i, nObject);
	}
	return count;
}

//------------------------------------------------------------------------------

void JohnsObjUnlink (int nSegment, int nObject)
{
	tObject  *objP = gameData.objs.objects+nObject;
	tSegment *seg = gameData.segs.segments+nSegment;

Assert (nObject != -1);
if (objP->prev == -1)
	seg->objects = objP->next;
else
	gameData.objs.objects [objP->prev].next = objP->next;
if (objP->next != -1)
	gameData.objs.objects [objP->next].prev = objP->prev;
}

//------------------------------------------------------------------------------

void RemoveIncorrectObjects ()
{
	int nSegment, nObject, count;

for (nSegment = 0; nSegment <= gameData.segs.nLastSegment; nSegment++) {
	count = 0;
	for (nObject = gameData.segs.segments [nSegment].objects;
		  nObject != -1;
		  nObject=gameData.objs.objects [nObject].next) {
		count++;
		#ifdef _DEBUG
		if (count > MAX_OBJECTS)	{
#if TRACE			
			con_printf (1, "Object list in tSegment %d is circular.\n", nSegment);
#endif
			Int3 ();
		}
		#endif
		if (gameData.objs.objects [nObject].nSegment != nSegment)	{
			#ifdef _DEBUG
#if TRACE			
			con_printf (CONDBG, "Removing tObject %d from tSegment %d.\n", nObject, nSegment);
#endif
			Int3 ();
			#endif
			JohnsObjUnlink (nSegment, nObject);
			}
		}
	}
}

//------------------------------------------------------------------------------

void RemoveAllObjectsBut (int nSegment, int nObject)
{
	int i;

for (i = 0; i <= gameData.segs.nLastSegment; i++)
	if (nSegment != i)
		if (IsObjectInSeg (i, nObject))
			JohnsObjUnlink (i, nObject);
}

//------------------------------------------------------------------------------

int check_duplicateObjects ()
{
	int i, count = 0;

for (i = 0; i <= gameData.objs.nLastObject; i++) {
	if (gameData.objs.objects [i].nType != OBJ_NONE)	{
		count = SearchAllSegsForObject (i);
		if (count > 1)	{
#ifdef _DEBUG
#	if TRACE			
			con_printf (1, "Object %d is in %d segments!\n", i, count);
#	endif
			Int3 ();
#endif
			RemoveAllObjectsBut (gameData.objs.objects [i].nSegment,  i);
			return count;
			}
		}
	}
	return count;
}

//------------------------------------------------------------------------------

void list_segObjects (int nSegment)
{
	int nObject, count = 0;

for (nObject = gameData.segs.segments [nSegment].objects;
	  nObject != -1;
	  nObject = gameData.objs.objects [nObject].next) {
	count++;
	if (count > MAX_OBJECTS) 	{
		Int3 ();
		return;
		}
	}
return;
}
#endif

//------------------------------------------------------------------------------

//link the tObject into the list for its tSegment
void LinkObject (int nObject, int nSegment)
{
	tObject *objP;

Assert (nObject != -1);
objP = gameData.objs.objects + nObject;
Assert (objP->nSegment == -1);
Assert (nSegment >= 0 && nSegment <= gameData.segs.nLastSegment);
if ((nSegment < 0) || (nSegment >= gameData.segs.nSegments)) {
	nSegment = FindSegByPoint (&objP->position.vPos, 0, 0);
	if (nSegment < 0)
		return;
	}
objP->nSegment = nSegment;
objP->next = gameData.segs.segments [nSegment].objects;
objP->prev = -1;
gameData.segs.segments [nSegment].objects = nObject;
if (objP->next != -1)
		gameData.objs.objects [objP->next].prev = nObject;

//list_segObjects (nSegment);
//check_duplicateObjects ();

//Assert (gameData.objs.objects [0].next != 0);
if (gameData.objs.objects [0].next == 0)
	gameData.objs.objects [0].next = -1;

//Assert (gameData.objs.objects [0].prev != 0);
if (gameData.objs.objects [0].prev == 0)
	gameData.objs.objects [0].prev = -1;
}

//------------------------------------------------------------------------------

void UnlinkObject (int nObject)
{
	tObject  *objP = gameData.objs.objects+nObject;
	tSegment *segP= gameData.segs.segments+objP->nSegment;

Assert (nObject != -1);
if (objP->prev == -1)
	segP->objects = objP->next;
else
	gameData.objs.objects [objP->prev].next = objP->next;
if (objP->next != -1) 
	gameData.objs.objects [objP->next].prev = objP->prev;
objP->nSegment = -1;
Assert (gameData.objs.objects [0].next != 0);
Assert (gameData.objs.objects [0].prev != 0);
}

//------------------------------------------------------------------------------

int nDebrisObjectCount = 0;
int nUnusedObjectsSlots;

//returns the number of a D2_FREE tObject, updating gameData.objs.nLastObject.
//Generally, CreateObject () should be called to get an tObject, since it
//fills in important fields and does the linking.
//returns -1 if no D2_FREE gameData.objs.objects
int AllocObject (void)
{
	int nObject;
	tObject *objP;

if (gameData.objs.nObjects >= MAX_OBJECTS - 2)
	FreeObjectSlots (MAX_OBJECTS - 10);
if (gameData.objs.nObjects >= MAX_OBJECTS)
	return -1;
nObject = gameData.objs.freeList [gameData.objs.nObjects++];
if (nObject > gameData.objs.nLastObject) {
	gameData.objs.nLastObject = nObject;
	if (gameData.objs.nLastObject > gameData.objs.nObjectLimit)
		gameData.objs.nObjectLimit = gameData.objs.nLastObject;
	}
objP = gameData.objs.objects + nObject;
objP->flags = 0;
objP->attachedObj =
objP->cType.explInfo.nNextAttach =
objP->cType.explInfo.nPrevAttach =
objP->cType.explInfo.nAttachParent = -1;
return nObject;
}

//------------------------------------------------------------------------------
//frees up an tObject.  Generally, ReleaseObject () should be called to get
//rid of an tObject.  This function deallocates the tObject entry after
//the tObject has been unlinked
void FreeObject (int nObject)
{
DelObjChildrenN (nObject);
DelObjChildN (nObject);
KillObjectSmoke (nObject);
DestroyObjectLightnings (OBJECTS + nObject);
RemoveDynLight (-1, -1, nObject);
gameData.objs.freeList [--gameData.objs.nObjects] = nObject;
Assert (gameData.objs.nObjects >= 0);
if (nObject == gameData.objs.nLastObject)
	while (gameData.objs.objects [--gameData.objs.nLastObject].nType == OBJ_NONE);
#ifdef _DEBUG
if (dbgObjP && (OBJ_IDX (dbgObjP) == nObject))
	dbgObjP = NULL;
#endif
}

//-----------------------------------------------------------------------------
//	Scan the tObject list, freeing down to num_used gameData.objs.objects
//	Returns number of slots freed.
int FreeObjectSlots (int num_used)
{
	int		i, olind;
	int		objList [MAX_OBJECTS_D2X];
	int		nAlreadyFree, nToFree, nOrgNumToFree;
	tObject	*objP;

olind = 0;
nAlreadyFree = MAX_OBJECTS - gameData.objs.nLastObject - 1;

if (MAX_OBJECTS - nAlreadyFree < num_used)
	return 0;

for (i = 0; i <= gameData.objs.nLastObject; i++) {
	if (gameData.objs.objects [i].flags & OF_SHOULD_BE_DEAD) {
		nAlreadyFree++;
		if (MAX_OBJECTS - nAlreadyFree < num_used)
			return nAlreadyFree;
		}
	else
		switch (gameData.objs.objects [i].nType) {
			case OBJ_NONE:
				nAlreadyFree++;
				if (MAX_OBJECTS - nAlreadyFree < num_used)
					return 0;
				break;
			case OBJ_WALL:
			case OBJ_FLARE:
				Int3 ();		//	This is curious.  What is an tObject that is a tWall?
				break;
			case OBJ_FIREBALL:
			case OBJ_EXPLOSION:
			case OBJ_WEAPON:
			case OBJ_DEBRIS:
				objList [olind++] = i;
				break;
			case OBJ_ROBOT:
			case OBJ_HOSTAGE:
			case OBJ_PLAYER:
			case OBJ_CNTRLCEN:
			case OBJ_CLUTTER:
			case OBJ_GHOST:
			case OBJ_LIGHT:
			case OBJ_CAMERA:
			case OBJ_POWERUP:
			case OBJ_MONSTERBALL:
				break;
			}
	}

nToFree = MAX_OBJECTS - num_used - nAlreadyFree;
nOrgNumToFree = nToFree;
if (nToFree > olind) {
#if TRACE			
	con_printf (1, "Warning: Asked to D2_FREE %i gameData.objs.objects, but can only D2_FREE %i.\n", nToFree, olind);
#endif
	nToFree = olind;
	}
for (i = 0; i < nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if (objP->nType == OBJ_DEBRIS) {
		nToFree--;
		KillObject (objP);
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i < nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->nType == OBJ_FIREBALL) && (objP->cType.explInfo.nDeleteObj == -1)) {
		nToFree--;
		KillObject (objP);
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i < nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->nType == OBJ_WEAPON) && (objP->id == FLARE_ID)) {
		nToFree--;
		KillObject (objP);
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i < nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->nType == OBJ_WEAPON) && (objP->id != FLARE_ID)) {
		nToFree--;
		KillObject (objP);
		}
	}
return nOrgNumToFree - nToFree;
}

//-----------------------------------------------------------------------------
//initialize a new tObject.  adds to the list for the given tSegment
//note that nSegment is really just a suggestion, since this routine actually
//searches for the correct tSegment
//returns the tObject number

int CreateObject (ubyte nType, ubyte id, short owner, short nSegment, vmsVector *pos, 
					   vmsMatrix *orient, fix size, ubyte cType, ubyte mType, ubyte rType,
						int bIgnoreLimits)
{
	short		nObject;
	tObject	*objP;
#ifdef _DEBUG
	short		i;
#endif

#ifdef _DEBUG
if (nType == OBJ_WEAPON) {
	nType = nType;
	if ((owner >= 0) && (gameData.objs.objects [owner].nType == OBJ_ROBOT))
		nType = nType;
	if (id == FLARE_ID)
		nType = nType;
	}
else if (nType == OBJ_ROBOT) {
	if (ROBOTINFO (id).bossFlag && (BOSS_COUNT >= MAX_BOSS_COUNT))
		return -1;
	}
else if (nType == OBJ_HOSTAGE)
	nType = nType;
else if (nType == OBJ_FIREBALL)
	nType = nType;
else if (nType == OBJ_CNTRLCEN)
	nType = nType;
else if (nType == OBJ_DEBRIS)
	nType = nType;
else
#endif
if (nType == OBJ_POWERUP) {
#ifdef _DEBUG
	nType = nType;
#endif
	if (!bIgnoreLimits && TooManyPowerups (id)) {
#if 0//def _DEBUG
		HUDInitMessage ("%c%c%c%cDiscarding excess %s!", 1, 127 + 128, 64 + 128, 128, pszPowerup [id]);
		TooManyPowerups (id);
#endif
		return -2;
		}
	}
Assert (nSegment <= gameData.segs.nLastSegment);
Assert (nSegment >= 0);
if ((nSegment < 0) || (nSegment > gameData.segs.nLastSegment))
	return -1;
Assert (cType <= CT_CNTRLCEN);
if ((nType == OBJ_DEBRIS) && (nDebrisObjectCount >= gameStates.render.detail.nMaxDebrisObjects))
	return -1;
if (GetSegMasks (pos, nSegment, 0).centerMask)
	if ((nSegment = FindSegByPoint (pos, nSegment, 1)) == -1) {
#ifdef _DEBUG
#	if TRACE			
		con_printf (CONDBG, "Bad segment in CreateObject (nType=%d)\n", nType);
#	endif
#endif
		return -1;		//don't create this tObject
	}
// Find next D2_FREE tObject
nObject = AllocObject ();
if (nObject == -1)		//no D2_FREE gameData.objs.objects
	return -1;
Assert (gameData.objs.objects [nObject].nType == OBJ_NONE);		//make sure unused
objP = gameData.objs.objects + nObject;
Assert (objP->nSegment == -1);
// Zero out tObject structure to keep weird bugs from happening
// in uninitialized fields.
memset (objP, 0, sizeof (tObject));
objP->nSignature = gameData.objs.nNextSignature++;
objP->nType = nType;
objP->id = id;
objP->vLastPos = *pos;
objP->position.vPos = *pos;
objP->size = size;
objP->matCenCreator = (sbyte) owner;
objP->position.mOrient = orient ? *orient : vmdIdentityMatrix;
objP->controlType = cType;
objP->movementType = mType;
objP->renderType = rType;
objP->containsType = -1;
if ((gameData.app.nGameMode & GM_ENTROPY) && (nType == OBJ_POWERUP) && (id == POW_HOARD_ORB))
	objP->lifeleft = (extraGameInfo [1].entropy.nVirusLifespan <= 0) ? 
							IMMORTAL_TIME : i2f (extraGameInfo [1].entropy.nVirusLifespan);
else
	objP->lifeleft = IMMORTAL_TIME;		//assume immortal
objP->attachedObj = -1;

if (objP->controlType == CT_POWERUP)
	objP->cType.powerupInfo.count = 1;

// Init physics info for this tObject
if (objP->movementType == MT_PHYSICS) {
#if 0 //we did memset with 0 further up ...
	VmVecZero (&objP->mType.physInfo.velocity);
	VmVecZero (&objP->mType.physInfo.thrust);
	VmVecZero (&objP->mType.physInfo.rotVel);
	VmVecZero (&objP->mType.physInfo.rotThrust);
	objP->mType.physInfo.mass = 0;
	objP->mType.physInfo.drag = 0;
	objP->mType.physInfo.brakes = 0;
	objP->mType.physInfo.turnRoll = 0;
	objP->mType.physInfo.flags = 0;
#endif
	}
if (objP->renderType == RT_POLYOBJ)
	objP->rType.polyObjInfo.nTexOverride = -1;
objP->shields = 20 * F1_0;
#ifdef _DEBUG
i = nSegment;
#endif
if (0 > (nSegment = FindSegByPoint (pos, nSegment, 1))) {	//find correct tSegment
#ifdef _DEBUG
	FindSegByPoint (pos, i, 1);
#endif
	return -1;
	}
Assert (nSegment != -1);
objP->nSegment = -1;					//set to zero by memset, above
LinkObject (nObject, nSegment);
//	Set (or not) persistent bit in physInfo.
gameData.objs.xCreationTime [nObject] = gameData.time.xGame;
if (objP->nType == OBJ_WEAPON) {
	Assert (objP->controlType == CT_WEAPON);
	objP->mType.physInfo.flags |= WI_persistent (objP->id) * PF_PERSISTENT;
	objP->cType.laserInfo.creationTime = gameData.time.xGame;
	objP->cType.laserInfo.nLastHitObj = 0;
	objP->cType.laserInfo.multiplier = F1_0;
	}
if (objP->controlType == CT_POWERUP)
	objP->cType.powerupInfo.creationTime = gameData.time.xGame;
else if (objP->controlType == CT_EXPLOSION)
	objP->cType.explInfo.nNextAttach = 
	objP->cType.explInfo.nPrevAttach = 
	objP->cType.explInfo.nAttachParent = -1;
#ifdef _DEBUG
#if TRACE			
if (bPrintObjectInfo)
	con_printf (CONDBG, "Created tObject %d of nType %d\n", nObject, objP->nType);
#endif
#endif
if (objP->nType == OBJ_DEBRIS)
	nDebrisObjectCount++;
if (IsMultiGame && (nType == OBJ_POWERUP) && PowerupClass (id)) {
	gameData.multiplayer.powerupsInMine [(int) id]++;
	if (MultiPowerupIs4Pack (id))
		gameData.multiplayer.powerupsInMine [(int) id - 1] += 4;
	}
return nObject;
}

//------------------------------------------------------------------------------

#ifdef EDITOR
//create a copy of an tObject. returns new tObject number
int CreateObjectCopy (int nObject, vmsVector *new_pos, int newsegnum)
{
	tObject *objP;
	int newObjNum = AllocObject ();

// Find next D2_FREE tObject
if (newObjNum == -1)
	return -1;
obj = gameData.objs.objects + newObjNum;
*objP = gameData.objs.objects [nObject];
objP->position.vPos = objP->vLastPos = *new_pos;
objP->next = objP->prev = objP->nSegment = -1;
LinkObject (newObjNum, newsegnum);
objP->nSignature = gameData.objs.nNextSignature++;
//we probably should initialize sub-structures here
return newObjNum;
}
#endif

//------------------------------------------------------------------------------

extern void NDRecordGuidedEnd ();

//remove tObject from the world
void ReleaseObject (short nObject)
{
	int nParent;
	tObject *objP = gameData.objs.objects + nObject;

Assert (nObject != -1);
Assert (nObject != 0);
Assert (objP->nType != OBJ_NONE);
if (objP->nType == OBJ_PLAYER)
	objP = objP;
Assert (objP != gameData.objs.console);
if (objP->nType == OBJ_WEAPON) {
	RespawnDestroyedWeapon (nObject);
	if (objP->id == GUIDEDMSL_ID) {
		nParent = gameData.objs.objects [objP->cType.laserInfo.nParentObj].id;
		if (nParent != gameData.multiplayer.nLocalPlayer)
			gameData.objs.guidedMissile [nParent] = NULL;
		else if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordGuidedEnd ();
		}
	}
if (objP == gameData.objs.viewer)		//deleting the viewer?
	gameData.objs.viewer = gameData.objs.console;						//..make the tPlayer the viewer
if (objP->flags & OF_ATTACHED)		//detach this from tObject
	DetachOneObject (objP);
if (objP->attachedObj != -1)		//detach all gameData.objs.objects from this
	DetachAllObjects (objP);
if (objP->nType == OBJ_DEBRIS)
	nDebrisObjectCount--;
UnlinkObject (nObject);
Assert (gameData.objs.objects [0].next != 0);
if ((objP->nType == OBJ_ROBOT) || (objP->nType == OBJ_CNTRLCEN))
	ExecObjTriggers (nObject, 0);
objP->nType = OBJ_NONE;		//unused!
objP->nSignature = -1;
objP->nSegment = -1;				// zero it!
FreeObject (nObject);
SpawnLeftoverPowerups (nObject);
}

//------------------------------------------------------------------------------

#define	DEATH_SEQUENCE_LENGTH			 (F1_0*5)
#define	DEATH_SEQUENCE_EXPLODE_TIME	 (F1_0*2)

tObject	*viewerSaveP;
int		nPlayerFlagsSave;
fix		xCameraToPlayerDistGoal=F1_0*4;
ubyte		nControlTypeSave, nRenderTypeSave;

//------------------------------------------------------------------------------

void DeadPlayerEnd (void)
{
if (!gameStates.app.bPlayerIsDead)
	return;
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordRestoreCockpit ();
gameStates.app.bPlayerIsDead = 0;
gameStates.app.bPlayerExploded = 0;
if (gameData.objs.deadPlayerCamera) {
	ReleaseObject (OBJ_IDX (gameData.objs.deadPlayerCamera));
	gameData.objs.deadPlayerCamera = NULL;
	}
SelectCockpit (gameStates.render.cockpit.nModeSave);
gameStates.render.cockpit.nModeSave = -1;
gameData.objs.viewer = viewerSaveP;
gameData.objs.console->nType = OBJ_PLAYER;
gameData.objs.console->flags = nPlayerFlagsSave;
Assert ((nControlTypeSave == CT_FLYING) || (nControlTypeSave == CT_SLEW));
gameData.objs.console->controlType = nControlTypeSave;
gameData.objs.console->renderType = nRenderTypeSave;
LOCALPLAYER.flags &= ~PLAYER_FLAGS_INVULNERABLE;
gameStates.app.bPlayerEggsDropped = 0;
}

//------------------------------------------------------------------------------

//	Camera is less than size of tPlayer away from
void SetCameraPos (vmsVector *vCameraPos, tObject *objP)
{
	vmsVector	vPlayerCameraOffs;
	int			count = 0;
	fix			xCameraPlayerDist;
	fix			xFarScale;

xCameraPlayerDist = VmVecMag (VmVecSub (&vPlayerCameraOffs, vCameraPos, &objP->position.vPos));
if (xCameraPlayerDist < xCameraToPlayerDistGoal) { // 2*objP->size) {
	//	Camera is too close to tPlayer tObject, so move it away.
	tVFIQuery	fq;
	tFVIData		hit_data;
	vmsVector	local_p1;

	if ((vPlayerCameraOffs.p.x == 0) && (vPlayerCameraOffs.p.y == 0) && (vPlayerCameraOffs.p.z == 0))
		vPlayerCameraOffs.p.x += F1_0/16;

	hit_data.hit.nType = HIT_WALL;
	xFarScale = F1_0;

	while ((hit_data.hit.nType != HIT_NONE) && (count++ < 6)) {
		vmsVector	closer_p1;
		VmVecNormalize (&vPlayerCameraOffs);
		VmVecScale (&vPlayerCameraOffs, xCameraToPlayerDistGoal);

		fq.p0 = &objP->position.vPos;
		VmVecAdd (&closer_p1, &objP->position.vPos, &vPlayerCameraOffs);		//	This is the actual point we want to put the camera at.
		VmVecScale (&vPlayerCameraOffs, xFarScale);						//	...but find a point 50% further away...
		VmVecAdd (&local_p1, &objP->position.vPos, &vPlayerCameraOffs);		//	...so we won't have to do as many cuts.

		fq.p1					= &local_p1;
		fq.startSeg			= objP->nSegment;
		fq.radP0				=
		fq.radP1				= 0;
		fq.thisObjNum		= OBJ_IDX (objP);
		fq.ignoreObjList	= NULL;
		fq.flags				= 0;
		FindVectorIntersection (&fq, &hit_data);

		if (hit_data.hit.nType == HIT_NONE)
			*vCameraPos = closer_p1;
		else {
			MakeRandomVector (&vPlayerCameraOffs);
			xFarScale = 3*F1_0 / 2;
			}
		}
	}
}

//------------------------------------------------------------------------------

extern void DropPlayerEggs (tObject *objP);
//extern int GetExplosionVClip (tObject *objP, int stage);
extern void MultiCapObjects ();
extern int nProximityDropped, nSmartminesDropped;

void DeadPlayerFrame (void)
{
	fix			xTimeDead, h;
	vmsVector	fVec;

if (gameStates.app.bPlayerIsDead) {
	xTimeDead = gameData.time.xGame - gameStates.app.nPlayerTimeOfDeath;

	//	If unable to create camera at time of death, create now.
	if (!gameData.objs.deadPlayerCamera) {
		tObject *player = gameData.objs.objects + LOCALPLAYER.nObject;
		int nObject = CreateObject (OBJ_CAMERA, 0, -1, player->nSegment, &player->position.vPos, 
											 &player->position.mOrient, 0, CT_NONE, MT_NONE, RT_NONE, 1);
		if (nObject != -1)
			gameData.objs.viewer = gameData.objs.deadPlayerCamera = gameData.objs.objects + nObject;
		else
			Int3 ();
		}	
	h = DEATH_SEQUENCE_EXPLODE_TIME - xTimeDead;
	h = max (0, h);
	gameData.objs.console->mType.physInfo.rotVel.p.x = h / 4;
	gameData.objs.console->mType.physInfo.rotVel.p.y = h / 2;
	gameData.objs.console->mType.physInfo.rotVel.p.z = h / 3;
	xCameraToPlayerDistGoal = min (xTimeDead * 8, F1_0 * 20) + gameData.objs.console->size;
	SetCameraPos (&gameData.objs.deadPlayerCamera->position.vPos, gameData.objs.console);
	VmVecSub (&fVec, &gameData.objs.console->position.vPos, &gameData.objs.deadPlayerCamera->position.vPos);
	VmVector2Matrix (&gameData.objs.deadPlayerCamera->position.mOrient, &fVec, NULL, NULL);
	if (xTimeDead > DEATH_SEQUENCE_EXPLODE_TIME) {
		if (!gameStates.app.bPlayerExploded) {
		if (LOCALPLAYER.hostages_on_board > 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_2, LOCALPLAYER.hostages_on_board);
		else if (LOCALPLAYER.hostages_on_board == 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_1);
		else
			HUDInitMessage (TXT_SHIP_DESTROYED_0);

#ifdef TACTILE
			if (TactileStick)
				ClearForces ();
#endif
			gameStates.app.bPlayerExploded = 1;
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
			DropPlayerEggs (gameData.objs.console);
			gameStates.app.bPlayerEggsDropped = 1;
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
			ExplodeBadassPlayer (gameData.objs.console);
			//is this next line needed, given the badass call above?
			ExplodeObject (gameData.objs.console, 0);
			gameData.objs.console->flags &= ~OF_SHOULD_BE_DEAD;		//don't really kill tPlayer
			gameData.objs.console->renderType = RT_NONE;				//..just make him disappear
			gameData.objs.console->nType = OBJ_GHOST;						//..and kill intersections
			LOCALPLAYER.flags &= ~PLAYER_FLAGS_HEADLIGHT_ON;
#if 0
			if (gameOpts->gameplay.bFastRespawn)
				gameStates.app.bDeathSequenceAborted = 1;
#endif				
			}
		}
	else {
		if (d_rand () < gameData.time.xFrame * 4) {
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendCreateExplosion (gameData.multiplayer.nLocalPlayer);
			CreateSmallFireballOnObject (gameData.objs.console, F1_0, 1);
			}
		}
	if (gameStates.app.bDeathSequenceAborted) { //xTimeDead > DEATH_SEQUENCE_LENGTH) {
		StopPlayerMovement ();
		gameStates.app.bEnterGame = 2;
		if (!gameStates.app.bPlayerEggsDropped) {
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
			DropPlayerEggs (gameData.objs.console);
			gameStates.app.bPlayerEggsDropped = 1;
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
			}
		DoPlayerDead ();		//kill_player ();
		}
	}
}

//------------------------------------------------------------------------------

void AdjustMineSpawn ()
{
if (!(gameData.app.nGameMode & GM_NETWORK))
	return;  // No need for this function in any other mode
if (!(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)))
	LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX]+=nProximityDropped;
if (!(gameData.app.nGameMode & GM_ENTROPY))
	LOCALPLAYER.secondaryAmmo [SMARTMINE_INDEX]+=nSmartminesDropped;
nProximityDropped = 0;
nSmartminesDropped = 0;
}



int nKilledInFrame = -1;
short nKilledObjNum = -1;
extern char bMultiSuicide;

//	------------------------------------------------------------------------

void StartPlayerDeathSequence (tObject *player)
{
	int	nObject;

Assert (player == gameData.objs.console);
gameData.objs.speedBoost [OBJ_IDX (gameData.objs.console)].bBoosted = 0;
if (gameStates.app.bPlayerIsDead)
	return;
if (gameData.objs.deadPlayerCamera) {
	ReleaseObject (OBJ_IDX (gameData.objs.deadPlayerCamera));
	gameData.objs.deadPlayerCamera = NULL;
	}
StopConquerWarning ();
//Assert (gameStates.app.bPlayerIsDead == 0);
//Assert (gameData.objs.deadPlayerCamera == NULL);
ResetRearView ();
if (!(gameData.app.nGameMode & GM_MULTI))
	HUDClearMessages ();
nKilledInFrame = gameData.app.nFrameCount;
nKilledObjNum = OBJ_IDX (player);
gameStates.app.bDeathSequenceAborted = 0;
if (gameData.app.nGameMode & GM_MULTI) {
	MultiSendKill (LOCALPLAYER.nObject);
//		If Hoard, increase number of orbs by 1
//    Only if you haven't killed yourself
//		This prevents cheating
	if (gameData.app.nGameMode & GM_HOARD)
		if (!bMultiSuicide)
			if (LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX]<12)
				LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX]++;
	}
gameStates.ogl.palAdd.red = 40;
gameStates.app.bPlayerIsDead = 1;
#ifdef TACTILE
   if (TactileStick)
	Buffeting (70);
#endif
//LOCALPLAYER.flags &= ~ (PLAYER_FLAGS_AFTERBURNER);
VmVecZero (&player->mType.physInfo.rotThrust);
VmVecZero (&player->mType.physInfo.thrust);
gameStates.app.nPlayerTimeOfDeath = gameData.time.xGame;
nObject = CreateObject (OBJ_CAMERA, 0, -1, player->nSegment, &player->position.vPos, 
								&player->position.mOrient, 0, CT_NONE, MT_NONE, RT_NONE, 1);
viewerSaveP = gameData.objs.viewer;
if (nObject != -1)
	gameData.objs.viewer = gameData.objs.deadPlayerCamera = gameData.objs.objects + nObject;
else {
	Int3 ();
	gameData.objs.deadPlayerCamera = NULL;
	}
if (gameStates.render.cockpit.nModeSave == -1)		//if not already saved
	gameStates.render.cockpit.nModeSave = gameStates.render.cockpit.nMode;
SelectCockpit (CM_LETTERBOX);
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordLetterbox ();
nPlayerFlagsSave = player->flags;
nControlTypeSave = player->controlType;
nRenderTypeSave = player->renderType;
player->flags &= ~OF_SHOULD_BE_DEAD;
//	LOCALPLAYER.flags |= PLAYER_FLAGS_INVULNERABLE;
player->controlType = CT_NONE;
if (!gameStates.entropy.bExitSequence) {
	player->shields = F1_0*1000;
	MultiSendShields ();
	}
PALETTE_FLASH_SET (0, 0, 0);
}

//------------------------------------------------------------------------------

void DeleteAllObjsThatShouldBeDead ()
{
	int		i;
	tObject	*objP;
	int		nLocalDeadPlayerObj = -1;

for (i = 0; i <= gameData.objs.nLastObject; i++) {
	objP = gameData.objs.objects + i;
	if (objP->nType == OBJ_NONE)
		continue;
	if (!(objP->flags & OF_SHOULD_BE_DEAD)) {
#if 1
#	ifdef _DEBUG
		if (objP->shields < 0)
			objP = objP;
#	endif
		continue;
#else
		if (objP->shields >= 0)
			continue;
		KillObject (objP);
		if (objP->nType == OBJ_ROBOT) {
			if (ROBOTINFO (objP->id).bossFlag)
				StartBossDeathSequence (objP);
			else
				StartRobotDeathSequence (objP);
			continue;
			}
#endif
		}
	Assert ((objP->nType != OBJ_FIREBALL) || (objP->cType.explInfo.nDeleteTime == -1));
	if (objP->nType != OBJ_PLAYER) 
		ReleaseObject ((short) i);
	else {
		if (objP->id == gameData.multiplayer.nLocalPlayer) {
			if (nLocalDeadPlayerObj == -1) {
				StartPlayerDeathSequence (objP);
				nLocalDeadPlayerObj = OBJ_IDX (objP);
				}
			else
				Int3 ();
			}
		}
	}
}

//--------------------------------------------------------------------
//when an tObject has moved into a new tSegment, this function unlinks it
//from its old tSegment, and links it into the new tSegment
void RelinkObject (int nObject, int newsegnum)
{
Assert ((nObject >= 0) && (nObject <= gameData.objs.nLastObject));
Assert ((newsegnum <= gameData.segs.nLastSegment) && (newsegnum >= 0));
UnlinkObject (nObject);
LinkObject (nObject, newsegnum);
#ifdef _DEBUG
#if TRACE			
if (GetSegMasks (&gameData.objs.objects [nObject].position.vPos, 
					  gameData.objs.objects [nObject].nSegment, 0).centerMask)
	con_printf (1, "RelinkObject violates seg masks.\n");
#endif
#endif
}

//--------------------------------------------------------------------
//process a continuously-spinning tObject
void SpinObject (tObject *objP)
{
	vmsAngVec rotangs;
	vmsMatrix rotmat, new_pm;

Assert (objP->movementType == MT_SPINNING);
rotangs.p = (fixang) FixMul (objP->mType.spinRate.p.x, gameData.time.xFrame);
rotangs.h = (fixang) FixMul (objP->mType.spinRate.p.y, gameData.time.xFrame);
rotangs.b = (fixang) FixMul (objP->mType.spinRate.p.z, gameData.time.xFrame);
VmAngles2Matrix (&rotmat, &rotangs);
VmMatMul (&new_pm, &objP->position.mOrient, &rotmat);
objP->position.mOrient = new_pm;
CheckAndFixMatrix (&objP->position.mOrient);
}

extern void MultiSendDropBlobs (char);
extern void FuelCenCheckForGoal (tSegment *);

//see if tWall is volatile, and if so, cause damage to tPlayer
//returns true if tPlayer is in lava
int CheckVolatileWall (tObject *objP, int nSegment, int nSide, vmsVector *hitpt);
int CheckVolatileSegment (tObject *objP, int nSegment);

//	Time at which this tObject last created afterburner blobs.

//--------------------------------------------------------------------
//reset tObject's movement info
//--------------------------------------------------------------------

void StopObjectMovement (tObject *objP)
{
Controls [0].headingTime = 0;
Controls [0].pitchTime = 0;
Controls [0].bankTime = 0;
Controls [0].verticalThrustTime = 0;
Controls [0].sidewaysThrustTime = 0;
Controls [0].forwardThrustTime = 0;
VmVecZero (&objP->mType.physInfo.rotThrust);
VmVecZero (&objP->mType.physInfo.thrust);
VmVecZero (&objP->mType.physInfo.velocity);
VmVecZero (&objP->mType.physInfo.rotVel);
}

//--------------------------------------------------------------------

void StopPlayerMovement (void)
{
if (!gameData.objs.speedBoost [OBJ_IDX (gameData.objs.console)].bBoosted) {
	StopObjectMovement (gameData.objs.objects + LOCALPLAYER.nObject);
	memset (&playerThrust, 0, sizeof (playerThrust));
//	gameData.time.xFrame = F1_0;
	gameData.objs.speedBoost [OBJ_IDX (gameData.objs.console)].bBoosted = 0;
	}
}

//--------------------------------------------------------------------

void MoveCamera (tObject *objP)
{

#define	DEG90		 (F1_0 / 4)	
#define	DEG45		 (F1_0 / 8)			
#define	DEG675	 (DEG45 + (F1_0 / 16))
#define	DEG1		 (F1_0 / (4 * 90))

	tCamera	*pc = gameData.cameras.cameras + gameData.objs.cameraRef [OBJ_IDX (objP)];
	fixang	curAngle = pc->curAngle;
	fixang	curDelta = pc->curDelta;

#if 1
	time_t	t0 = pc->t0;
	time_t	t = gameStates.app.nSDLTicks;

if ((t0 < 0) || (t - t0 >= 1000 / 90)) 
#endif
	if (objP->cType.aiInfo.behavior == AIB_NORMAL) {
		vmsAngVec	a;
		vmsMatrix	r;

		int	h = abs (curDelta);
		int	d = DEG1 / (gameOpts->render.cameras.nSpeed / 1000);
		int	s = h ? curDelta / h : 1;

	if (h != d)
		curDelta = s * d;
#if 1
	objP->mType.physInfo.brakes = (fix) t;
#endif
	if ((curAngle >= DEG45) || (curAngle <= -DEG45)) {
		if (curAngle * s - DEG45 >= curDelta * s)
			curAngle = s * DEG45 + curDelta - s;
		curDelta = -curDelta;
		}

	curAngle += curDelta;
	a.h = curAngle;
	a.b =	a.p = 0;
	VmAngles2Matrix (&r, &a);
	VmMatMul (&objP->position.mOrient, &pc->orient, &r);
	pc->curAngle = curAngle;
	pc->curDelta = curDelta;
	}
}

//--------------------------------------------------------------------

void CheckObjectInVolatileWall (tObject *objP)
{
	int bChkVolaSeg = 1, nType, sideMask, bUnderLavaFall = 0;
	static int nLavaFallHissPlaying [MAX_PLAYERS]={0};

if (objP->nType != OBJ_PLAYER)
	return;
sideMask = GetSegMasks (&objP->position.vPos, objP->nSegment, objP->size).sideMask;
if (sideMask) {
	short nSide, nWall;
	int bit;
	tSide *pSide = gameData.segs.segments [objP->nSegment].sides;
	for (nSide = 0, bit = 1; nSide < 6; bit <<= 1, nSide++, pSide++) {
		if (!(sideMask & bit))
			continue;
		nWall = pSide->nWall;
		if (!IS_WALL (nWall))
			continue;
		if (gameData.walls.walls [nWall].nType != WALL_ILLUSION)
			continue;
		if ((nType = CheckVolatileWall (objP, objP->nSegment, nSide, &objP->position.vPos))) {
			short sound = (nType==1) ? SOUND_LAVAFALL_HISS : SOUND_SHIP_IN_WATERFALL;
			bUnderLavaFall = 1;
			bChkVolaSeg = 0;
			if (!nLavaFallHissPlaying [objP->id]) {
				DigiLinkSoundToObject3 (sound, OBJ_IDX (objP), 1, F1_0, i2f (256), -1, -1, NULL);
				nLavaFallHissPlaying [objP->id] = 1;
				}
			}
		}
	}
if (bChkVolaSeg) {
	if ((nType = CheckVolatileSegment (objP, objP->nSegment))) {
		short sound = (nType==1) ? SOUND_LAVAFALL_HISS : SOUND_SHIP_IN_WATERFALL;
		bUnderLavaFall = 1;
		if (!nLavaFallHissPlaying [objP->id]) {
			DigiLinkSoundToObject3 (sound, OBJ_IDX (objP), 1, F1_0, i2f (256), -1, -1, NULL);
			nLavaFallHissPlaying [objP->id] = 1;
			}
		}
	}
if (!bUnderLavaFall && nLavaFallHissPlaying [objP->id]) {
	DigiKillSoundLinkedToObject (OBJ_IDX (objP));
	nLavaFallHissPlaying [objP->id] = 0;
	}
}

//--------------------------------------------------------------------

void HandleSpecialSegments (tObject *objP)
{
	fix fuel, shields;
	tSegment *segP = gameData.segs.segments + objP->nSegment;
	xsegment *xsegP = gameData.segs.xSegments + objP->nSegment;
	tPlayer *playerP = gameData.multiplayer.players + gameData.multiplayer.nLocalPlayer;

if ((objP->nType == OBJ_PLAYER) && (gameData.multiplayer.nLocalPlayer == objP->id)) {
   if (gameData.app.nGameMode & GM_CAPTURE)
		 FuelCenCheckForGoal (segP);
   else if (gameData.app.nGameMode & GM_HOARD)
		 FuelCenCheckForHoardGoal (segP);
   else if (gameData.app.nGameMode & GM_ENTROPY) {
		if (Controls [0].forwardThrustTime || 
			 Controls [0].verticalThrustTime || 
			 Controls [0].sidewaysThrustTime ||
			 (xsegP->owner < 0) ||
			 (xsegP->owner == GetTeam (gameData.multiplayer.nLocalPlayer) + 1)) {
			StopConquerWarning ();
			gameStates.entropy.nTimeLastMoved = -1;
			}
		else if (gameStates.entropy.nTimeLastMoved < 0)
			gameStates.entropy.nTimeLastMoved = 0;
		}
	shields = HostileRoomDamageShields (segP, playerP->shields + 1);
	if (shields > 0) {
		playerP->shields -= shields;
		MultiSendShields ();
		if (playerP->shields < 0)
			StartPlayerDeathSequence (objP);
		else
			CheckConquerRoom (xsegP);
		}
	else {
		StopConquerWarning ();
		fuel = FuelCenGiveFuel (segP, INITIAL_ENERGY - playerP->energy);
		if (fuel > 0)
			playerP->energy += fuel;
		shields = RepairCenGiveShields (segP, INITIAL_SHIELDS - playerP->shields);
		if (shields > 0) {
			playerP->shields += shields;
			MultiSendShields ();
			}
		if (!xsegP->owner)
			CheckConquerRoom (xsegP);
		}
	}
}

//--------------------------------------------------------------------

int HandleObjectControl (tObject *objP)
{
switch (objP->controlType) {
	case CT_NONE: 
		break;

	case CT_FLYING:
		ReadFlyingControls (objP);
		break;

	case CT_REPAIRCEN: 
		Int3 ();	// -- hey!these are no longer supported!!-- do_repair_sequence (objP); break;

	case CT_POWERUP: 
		DoPowerupFrame (objP); 
		break;

	case CT_MORPH:			//morph implies AI
		DoMorphFrame (objP);
		//NOTE: FALLS INTO AI HERE!!!!

	case CT_AI:
		//NOTE LINK TO CT_MORPH ABOVE!!!
		if (gameStates.gameplay.bNoBotAI || (gameStates.app.bGameSuspended & SUSP_ROBOTS)) {
			VmVecZero (&objP->mType.physInfo.velocity);
			VmVecZero (&objP->mType.physInfo.thrust);
			VmVecZero (&objP->mType.physInfo.rotThrust);
			DoAnyRobotDyingFrame (objP);
#if 1//ndef _DEBUG
			return 1;
#endif
			}
		else
			DoAIFrame (objP);
		break;

	case CT_CAMERA:	
		MoveCamera (objP); 
		break;

	case CT_WEAPON:	
		LaserDoWeaponSequence (objP); 
		break;

	case CT_EXPLOSION:
		DoExplosionSequence (objP); 
		break;

	case CT_SLEW:
#ifdef _DEBUG
		if (keyd_pressed [KEY_PAD5]) 
			slew_stop ();
		if (keyd_pressed [KEY_NUMLOCK]) {
			slew_reset_orient ();
			* (ubyte *) 0x417 &= ~0x20;		//kill numlock
		}
		slew_frame (0);		// Does velocity addition for us.
#endif
		break;	//ignore

	case CT_DEBRIS: 
		DoDebrisFrame (objP);
		break;

	case CT_LIGHT: 
		break;		//doesn't do anything

	case CT_REMOTE: 
		break;		//movement is handled in com_process_input

	case CT_CNTRLCEN: 
		if (gameStates.gameplay.bNoBotAI)
			return 1;
		DoReactorFrame (objP); 
		break;

	default:
#ifdef __DJGPP__
		Error ("Unknown control nType %d in tObject %li, sig/nType/id = %i/%i/%i", objP->controlType, OBJ_IDX (objP), objP->nSignature, objP->nType, objP->id);
#else
		Error ("Unknown control nType %d in tObject %i, sig/nType/id = %i/%i/%i", objP->controlType, OBJ_IDX (objP), objP->nSignature, objP->nType, objP->id);
#endif
	}
return 0;
}

//--------------------------------------------------------------------

void HandleObjectMovement (tObject *objP)
{
switch (objP->movementType) {
	case MT_NONE:		
		break;								//this doesn't move

	case MT_PHYSICS:
#ifdef _DEBUG
		if (objP->nType != OBJ_PLAYER)
			objP = objP;
#endif
		DoPhysicsSim (objP);
		SetDynLightPos (OBJ_IDX (objP));
		MoveObjectLightnings (objP);
		break;	//move by physics

	case MT_SPINNING:	
		SpinObject (objP); 
		break;
	}
}

//--------------------------------------------------------------------

int CheckObjectHitTriggers (tObject *objP, short nPrevSegment)
{
		short	nConnSide, i;
		int	nOldLevel;

if ((objP->movementType != MT_PHYSICS) || (nPrevSegment == objP->nSegment))
	return 0;
nOldLevel = gameData.missions.nCurrentLevel;
for (i = 0; i < nPhysSegs - 1; i++) {
#ifdef _DEBUG
	if (physSegList [i] > gameData.segs.nLastSegment)
		LogErr ("invalid segment in physSegList\n");
#endif
	nConnSide = FindConnectedSide (gameData.segs.segments + physSegList [i+1], gameData.segs.segments + physSegList [i]);
	if (nConnSide != -1)
		CheckTrigger (gameData.segs.segments + physSegList [i], nConnSide, OBJ_IDX (objP), 0);
#ifdef _DEBUG
	else	// segments are not directly connected, so do binary subdivision until you find connected segments.
		LogErr ("UNCONNECTED SEGMENTS %d, %d\n", physSegList [i+1], physSegList [i]);
#endif
	//maybe we've gone on to the next level.  if so, bail!
	if (gameData.missions.nCurrentLevel != nOldLevel)
		return 1;
	}
return 0;
}

//--------------------------------------------------------------------

void CheckGuidedMissileThroughExit (tObject *objP, short nPrevSegment)
{
if ((objP == gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer]) && 
	 (objP->nSignature == gameData.objs.guidedMissileSig [gameData.multiplayer.nLocalPlayer])) {
	if (nPrevSegment != objP->nSegment) {
		short	nConnSide = FindConnectedSide (gameData.segs.segments + objP->nSegment, gameData.segs.segments+nPrevSegment);
		if (nConnSide != -1) {
			short nWall, nTrigger;
			nWall = WallNumI (nPrevSegment, nConnSide);
			if (IS_WALL (nWall)) {
				nTrigger = gameData.walls.walls [nWall].nTrigger;
				if ((nTrigger < gameData.trigs.nTriggers) &&
					 (gameData.trigs.triggers [nTrigger].nType == TT_EXIT))
					gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer]->lifeleft = 0;
				}
			}
		}
	}
}

//--------------------------------------------------------------------

void CheckAfterburnerBlobDrop (tObject *objP)
{
if (gameStates.render.bDropAfterburnerBlob) {
	Assert (objP == gameData.objs.console);
	DropAfterburnerBlobs (objP, 2, i2f (5) / 2, -1, NULL, 0);	//	-1 means use default lifetime
	if (IsMultiGame)
		MultiSendDropBlobs ((char) gameData.multiplayer.nLocalPlayer);
	gameStates.render.bDropAfterburnerBlob = 0;
	}

if ((objP->nType == OBJ_WEAPON) && (gameData.weapons.info [objP->id].afterburner_size)) {
	int	nObject, bSmoke;
	fix	vel;
	fix	delay, lifetime, nSize;

#if 1
	if ((objP->nType == OBJ_WEAPON) && gameData.objs.bIsMissile [objP->id]) {
		if (SHOW_SMOKE && gameOpts->render.smoke.bMissiles)
			return;
		if ((gameStates.app.bNostalgia || EGI_FLAG (bThrusterFlames, 1, 1, 0)) && 
			 (objP->id != MERCURYMSL_ID))
			return;
		}
#endif
	if ((vel = VmVecMagQuick (&objP->mType.physInfo.velocity)) > F1_0 * 200)
		delay = F1_0 / 16;
	else if (vel > F1_0 * 40)
		delay = FixDiv (F1_0 * 13, vel);
	else
		delay = DEG90;

	if (bSmoke = SHOW_SMOKE && gameOpts->render.smoke.bMissiles) {
		nSize = F1_0 * 3;
		lifetime = F1_0 / 12;
		delay = 0;
		}
	else {
		nSize = i2f (gameData.weapons.info [objP->id].afterburner_size) / 16;
		lifetime = 3 * delay / 2;
		if (!IsMultiGame) {
			delay /= 2;
			lifetime *= 2;
			}
		}

	nObject = OBJ_IDX (objP);
	if (bSmoke ||
		 (gameData.objs.xLastAfterburnerTime [nObject] + delay < gameData.time.xGame) || 
		 (gameData.objs.xLastAfterburnerTime [nObject] > gameData.time.xGame)) {
		DropAfterburnerBlobs (objP, 1, nSize, lifetime, NULL, bSmoke);
		gameData.objs.xLastAfterburnerTime [nObject] = gameData.time.xGame;
		}
	}
}

//--------------------------------------------------------------------

void HandleObjectEffects (tObject *objP)
{
if (objP->nType == OBJ_ROBOT) {
	if (ROBOTINFO (objP->id).energyDrain) {
			static tRgbaColorf color = {1.0f, 0.8f, 0.3f, 0.2f};

		CreateRobotLightnings (objP, &color);
		}
	}
else if ((objP->nType == OBJ_PLAYER) && gameOpts->render.lightnings.bPlayers) {
	int s = gameData.segs.segment2s [objP->nSegment].special;
	if (s == SEGMENT_IS_FUELCEN) {
			static tRgbaColorf color = {1.0f, 0.8f, 0.3f, 0.2f};

		CreatePlayerLightnings (objP, &color);
		}
	else if (s == SEGMENT_IS_REPAIRCEN) {
			tRgbaColorf color = {0.3f, 0.5f, 0.1f, 0.2f};

		CreatePlayerLightnings (objP, &color);
		}
	else
		DestroyObjectLightnings (objP);
	}
}

//--------------------------------------------------------------------
//move an tObject for the current frame

int UpdateObject (tObject * objP)
{
	short	nPrevSegment = (short) objP->nSegment;

if (objP->nType == OBJ_ROBOT) {
	fix xMaxShields = RobotDefaultShields (objP);
	if (objP->shields > xMaxShields)
		objP->shields = xMaxShields;
	}
objP->vLastPos = objP->position.vPos;			// Save the current position
HandleSpecialSegments (objP);
if ((objP->lifeleft != IMMORTAL_TIME) && 
	 (objP->lifeleft != ONE_FRAME_TIME)&& 
	 (gameData.physics.xTime != F1_0))
	objP->lifeleft -= (fix) (gameData.physics.xTime / gameStates.gameplay.slowmo [0].fSpeed);		//...inevitable countdown towards death
gameStates.render.bDropAfterburnerBlob = 0;
if (HandleObjectControl (objP)) {
	HandleObjectEffects (objP);
	return 1;
	}
if (objP->lifeleft < 0) {		// We died of old age
	KillObject (objP);
	if ((objP->nType == OBJ_WEAPON) && WI_damage_radius (objP->id))
		ExplodeBadassWeapon (objP, &objP->position.vPos);
	else if (objP->nType == OBJ_ROBOT)	//make robots explode
		ExplodeObject (objP, 0);
	}
if ((objP->nType == OBJ_NONE) || (objP->flags & OF_SHOULD_BE_DEAD)) {
	return 1;			//tObject has been deleted
	}
HandleObjectMovement (objP);
HandleObjectEffects (objP);
if (CheckObjectHitTriggers (objP, nPrevSegment))
	return 0;
CheckObjectInVolatileWall (objP);
CheckGuidedMissileThroughExit (objP, nPrevSegment);
CheckAfterburnerBlobDrop (objP);
return 1;
}

//--------------------------------------------------------------------
//move all gameData.objs.objects for the current frame
int UpdateAllObjects ()
{
	int i;
	tObject *objP;

//	check_duplicateObjects ();
//	RemoveIncorrectObjects ();
if (gameData.objs.nLastObject > gameData.objs.nMaxUsedObjects)
	FreeObjectSlots (gameData.objs.nMaxUsedObjects);		//	Free all possible tObject slots.
#if LIMIT_PHYSICS_FPS
if (!gameStates.app.tick60fps.bTick)
	return 1;
gameData.physics.xTime = secs2f (gameStates.app.tick60fps.nTime);
#else
gameData.physics.xTime = gameData.time.xFrame;
#endif
DeleteAllObjsThatShouldBeDead ();
if (gameOpts->gameplay.nAutoLeveling)
	gameData.objs.console->mType.physInfo.flags |= PF_LEVELLING;
else
	gameData.objs.console->mType.physInfo.flags &= ~PF_LEVELLING;

// Move all gameData.objs.objects
objP = gameData.objs.objects;
#ifndef DEMO_ONLY
gameStates.entropy.bConquering = 0;
UpdatePlayerOrient ();
for (i = 0; i <= gameData.objs.nLastObject; i++) {
	if ((objP->nType != OBJ_NONE) && !(objP->flags & OF_SHOULD_BE_DEAD)) {
		if (!UpdateObject (objP))
			return 0;
	}
	objP++;
}
#else
	i = 0;	//kill warning
#endif
return 1;
//	check_duplicateObjects ();
//	RemoveIncorrectObjects ();

}

//	-----------------------------------------------------------------------------------------------------------

static int nSlowMotionChannel = -1;

void SetSlowMotionState (int i)
{
	int	nState = 0;

if (gameStates.gameplay.slowmo [i].nState) {
	gameStates.gameplay.slowmo [i].nState = -gameStates.gameplay.slowmo [i].nState;
	if (nSlowMotionChannel >= 0) {
		DigiStopSound (nSlowMotionChannel);
		nSlowMotionChannel = -1;
		}
	}
else if (gameStates.gameplay.slowmo [i].fSpeed > 1) {
	gameStates.gameplay.slowmo [i].nState = -1;
	}
else {
	gameStates.gameplay.slowmo [i].nState = 1;
	}
gameStates.gameplay.slowmo [i].tUpdate = gameStates.app.nSDLTicks;
}

//	-----------------------------------------------------------------------------------------------------------

void SlowMotionMessage (void)
{
if (gameStates.gameplay.slowmo [0].nState > 0) {
	if (gameOpts->sound.bUseSDLMixer)
		nSlowMotionChannel = DigiPlayWAV ("slowdown.wav", F1_0);
	HUDInitMessage (TXT_SLOWING_DOWN);
	}
else if ((gameStates.gameplay.slowmo [0].nState < 0) ||
			((gameStates.gameplay.slowmo [0].nState == 0) && (gameStates.gameplay.slowmo [0].fSpeed == 1)) || 
			(gameStates.gameplay.slowmo [1].nState < 0) || 
			((gameStates.gameplay.slowmo [1].nState == 0) && (gameStates.gameplay.slowmo [1].fSpeed == 1))) {
	if (gameOpts->sound.bUseSDLMixer)
		nSlowMotionChannel = DigiPlayWAV ("speedup.wav", F1_0);
	HUDInitMessage (TXT_SPEEDING_UP);
	}
else {
	if (gameOpts->sound.bUseSDLMixer)
		nSlowMotionChannel = DigiPlayWAV ("slowdown.wav", F1_0);
	HUDInitMessage (TXT_SLOWING_DOWN);
	}
}

//	-----------------------------------------------------------------------------------------------------------

void InitBulletTime (int nState)
{
gameStates.gameplay.slowmo [1].nState = nState;
SetSlowMotionState (1);
}

//	-----------------------------------------------------------------------------------------------------------

void InitSlowMotion (int nState)
{
gameStates.gameplay.slowmo [0].nState = nState;
SetSlowMotionState (0);
}

//	-----------------------------------------------------------------------------------------------------------

int SlowMotionActive (void)
{
return gameStates.gameplay.slowmo [0].bActive =
		 (gameStates.gameplay.slowmo [0].nState > 0) || (gameStates.gameplay.slowmo [0].fSpeed > 1);
}

//	-----------------------------------------------------------------------------------------------------------

int BulletTimeActive (void)
{
return gameStates.gameplay.slowmo [1].bActive =
		 SlowMotionActive () && 
		 ((gameStates.gameplay.slowmo [1].nState < 0) || (gameStates.gameplay.slowmo [1].fSpeed == 1));
}

//	-----------------------------------------------------------------------------------------------------------

void SlowMotionOff (void)
{
if (SlowMotionActive () && (gameStates.gameplay.slowmo [0].nState != -1)) {
	InitSlowMotion (1);
	if (!BulletTimeActive ())
		InitBulletTime (1);
	SlowMotionMessage ();
	}
}

//	-----------------------------------------------------------------------------------------------------------

void BulletTimeOn (void)
{
if (!BulletTimeActive ())
	InitBulletTime (1);
if (!SlowMotionActive ())
	InitSlowMotion (-1);
SlowMotionMessage ();
}

//	-----------------------------------------------------------------------------------------------------------

int ToggleSlowMotion (void)
{
	int	bSlowMotionOk = gameStates.app.cheats.bSpeed || ((LOCALPLAYER.energy > F1_0 * 10) && (LOCALPLAYER.flags & PLAYER_FLAGS_CONVERTER));
	int	bBulletTimeOk = bSlowMotionOk && (gameStates.app.cheats.bSpeed || (LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER));
	int	bSlowMotion = bSlowMotionOk && (Controls [0].slowMotionCount > 0);
	int	bBulletTime = bBulletTimeOk && (Controls [0].bulletTimeCount > 0);

Controls [0].bulletTimeCount =
Controls [0].slowMotionCount = 0;
#if 1//def RELEASE
if (SlowMotionActive ()) {
	if (!gameStates.app.cheats.bSpeed)
#if 0
		LOCALPLAYER.energy -= gameData.time.xFrame * (1 + BulletTimeActive ());
#else
		LOCALPLAYER.energy -= ((4 + gameStates.app.nDifficultyLevel) * gameData.time.xFrame * (1 + BulletTimeActive ())) / 6;
#endif
	if (!bSlowMotionOk) {
		if (gameStates.gameplay.slowmo [0].nState != -1) {
			InitSlowMotion (1);
			if (!BulletTimeActive () && (gameStates.gameplay.slowmo [1].nState != -1))
				InitBulletTime (1);
			SlowMotionMessage ();
			}
		return 0;
		}
	if (!bBulletTimeOk) {
		if (gameStates.gameplay.slowmo [1].nState != -1)
			InitBulletTime (-1);
		bBulletTime = 0;
		}
	}
#endif
if (!(bSlowMotion || bBulletTime))
	return 0;
if (bBulletTime) {	//toggle bullet time and slow motion
	if (SlowMotionActive ()) {
		if (BulletTimeActive ())
			InitSlowMotion (1);
		InitBulletTime (1);
		}
	else {
		InitSlowMotion (-1);
		if (!BulletTimeActive ())
			InitBulletTime (1);
		}
	}
else {
	if (SlowMotionActive ()) {
		if (BulletTimeActive ())
			InitBulletTime (-1);
		else {
			InitSlowMotion (1);
			if (!BulletTimeActive ())
				InitBulletTime (1);
			}
		}
	else {
		InitSlowMotion (-1);
		if (BulletTimeActive ())
			InitBulletTime (-1);
		}
	}
return 1;
}

//	-----------------------------------------------------------------------------------------------------------

#define SLOWDOWN_SECS	2
#define SLOWDOWN_FPS		40

void DoSlowMotionFrame (void)
{
	int	i, bMsg;
	float	f, h;

if (gameStates.app.bNostalgia || IsMultiGame)
	return;
bMsg = ToggleSlowMotion ();
f = (float) gameOpts->gameplay.nSlowMotionSpeedup / 2;
h = (f - 1) / (SLOWDOWN_SECS * SLOWDOWN_FPS);
for (i = 0; i < 2; i++) {
	if (gameStates.gameplay.slowmo [i].nState && (gameStates.app.nSDLTicks - gameStates.gameplay.slowmo [i].tUpdate > 25)) {
		gameStates.gameplay.slowmo [i].fSpeed += gameStates.gameplay.slowmo [i].nState * h;
		if (gameStates.gameplay.slowmo [i].fSpeed >= f) {
			gameStates.gameplay.slowmo [i].fSpeed = f;
			gameStates.gameplay.slowmo [i].nState = 0;
			if (!i) {
				DigiExit ();
				DigiInit (f);
				}
			}
		else if (gameStates.gameplay.slowmo [i].fSpeed <= 1) {
			gameStates.gameplay.slowmo [i].fSpeed = 1;
			gameStates.gameplay.slowmo [i].nState = 0;
			if (!i) {
				DigiExit ();
				DigiInit (1);
				PlayLevelSong (gameData.missions.nCurrentLevel, 0);
				}
			}
		gameStates.gameplay.slowmo [i].tUpdate = gameStates.app.nSDLTicks;
		}
	}
if (bMsg)
	SlowMotionMessage ();
#if 0//def _DEBUG
HUDMessage (0, "%1.2f %1.2f %d", 
				gameStates.gameplay.slowmo [0].fSpeed, gameStates.gameplay.slowmo [1].fSpeed,
				gameStates.gameplay.slowmo [1].bActive);
#endif
}

//	-----------------------------------------------------------------------------------------------------------

//--unused-- // -----------------------------------------------------------
//--unused-- //	Moved here from eobject.c on 02/09/94 by MK.
//--unused-- int find_last_obj (int i)
//--unused-- {
//--unused-- 	for (i=MAX_OBJECTS;--i >= 0;)
//--unused-- 		if (gameData.objs.objects [i].nType != OBJ_NONE) break;
//--unused--
//--unused-- 	return i;
//--unused--
//--unused-- }


//make tObject array non-sparse
void compressObjects (void)
{
	int start_i;	//, last_i;

	//last_i = find_last_obj (MAX_OBJECTS);

	//	Note: It's proper to do < (rather than <=) gameData.objs.nLastObject here because we
	//	are just removing gaps, and the last tObject can't be a gap.
	for (start_i = 0;start_i<gameData.objs.nLastObject;start_i++)

		if (gameData.objs.objects [start_i].nType == OBJ_NONE) {

			int	segnum_copy;

			segnum_copy = gameData.objs.objects [gameData.objs.nLastObject].nSegment;

			UnlinkObject (gameData.objs.nLastObject);

			gameData.objs.objects [start_i] = gameData.objs.objects [gameData.objs.nLastObject];

			#ifdef EDITOR
			if (CurObject_index == gameData.objs.nLastObject)
				CurObject_index = start_i;
			#endif

			gameData.objs.objects [gameData.objs.nLastObject].nType = OBJ_NONE;

			LinkObject (start_i, segnum_copy);

			while (gameData.objs.objects [--gameData.objs.nLastObject].nType == OBJ_NONE);

			//last_i = find_last_obj (last_i);
		
		}

	ResetObjects (gameData.objs.nObjects);

}

//------------------------------------------------------------------------------

int ObjectCount (int nType)
{
	int		h, i;
	tObject	*objP = gameData.objs.objects;

for (h = 0, i = gameData.objs.nLastObject + 1; i; i--, objP++)
	if (objP->nType == nType)
		h++;
return h;
}

//------------------------------------------------------------------------------

void ConvertSmokeObject (tObject *objP)
{
	int			j;
	tTrigger		*trigP;

objP->nType = OBJ_EFFECT;
objP->id = SMOKE_ID;
objP->renderType = RT_SMOKE;
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_LIFE, -1);
#if 1
j = (trigP && trigP->value) ? trigP->value : 5;
objP->rType.smokeInfo.nLife = (j * (j + 1)) / 2;
#else
objP->rType.smokeInfo.nLife = (trigP && trigP->value) ? trigP->value : 5;
#endif
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_BRIGHTNESS, -1);
objP->rType.smokeInfo.nBrightness = (trigP && trigP->value) ? trigP->value * 10 : 75;
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_SPEED, -1);
j = (trigP && trigP->value) ? trigP->value : 5;
#if 1
objP->rType.smokeInfo.nSpeed = (j * (j + 1)) / 2;
#else
objP->rType.smokeInfo.nSpeed = j;
#endif
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_DENS, -1);
objP->rType.smokeInfo.nParts = j * ((trigP && trigP->value) ? trigP->value * 50 : STATIC_SMOKE_MAX_PARTS);
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_DRIFT, -1);
objP->rType.smokeInfo.nDrift = (trigP && trigP->value) ? j * trigP->value * 50 : objP->rType.smokeInfo.nSpeed * 50;
trigP = FindObjTrigger (OBJ_IDX (objP), TT_SMOKE_SIZE, -1);
j = (trigP && trigP->value) ? trigP->value : 5;
objP->rType.smokeInfo.nSize [0] = j + 1;
objP->rType.smokeInfo.nSize [1] = (j * (j + 1)) / 2;
}

//------------------------------------------------------------------------------

void ConvertObjects (void)
{
	tObject	*objP;
	int		i;

for (i = gameData.objs.nLastObject + 1, objP = OBJECTS; i; i--, objP++)
	if (objP->nType == OBJ_SMOKE)
		ConvertSmokeObject (objP);
}

//------------------------------------------------------------------------------

void SetupSmokeEffect (tObject *objP)
{
	tSmokeInfo	*psi = &objP->rType.smokeInfo;
	int			j;

objP->renderType = RT_SMOKE;
j = psi->nLife ? psi->nLife : 5;
#if 1
psi->nLife = (j * (j + 1)) / 2;
#else
psi->nLife = psi->value ? psi->value : 5;
#endif
psi->nBrightness = psi->nBrightness ? psi->nBrightness * 10 : 50;
j = psi->nSpeed ? psi->nSpeed : 5;
#if 1
psi->nSpeed = (j * (j + 1)) / 2;
#else
psi->nSpeed = j;
#endif
psi->nParts = j * (psi->nParts ? psi->nParts * 100 : STATIC_SMOKE_MAX_PARTS);
psi->nDrift = psi->nDrift ? j * psi->nDrift * 75 : psi->nSpeed * 50;
j = psi->nSize [0] ? psi->nSize [0] : 5;
psi->nSize [0] = j + 1;
psi->nSize [1] = (j * (j + 1)) / 2;
}

//------------------------------------------------------------------------------

void SetupEffects (void)
{
	tObject	*objP;
	int		i;

for (i = gameData.objs.nLastObject + 1, objP = OBJECTS; i; i--, objP++)
	if (objP->nType == OBJ_EFFECT)
		if (objP->id == SMOKE_ID)
			SetupSmokeEffect (objP);
}

//------------------------------------------------------------------------------
//called after load.  Takes number of gameData.objs.objects,  and gameData.objs.objects should be
//compressed.  resets D2_FREE list, marks unused gameData.objs.objects as unused
void ResetObjects (int nObjects)
{
	int i;

gameData.objs.nObjects = nObjects;
Assert (gameData.objs.nObjects > 0);
for (i = gameData.objs.nObjects; i < MAX_OBJECTS; i++) {
	gameData.objs.freeList [i] = i;
	gameData.objs.objects [i].nType = OBJ_NONE;
	gameData.objs.objects [i].nSegment =
	gameData.objs.objects [i].cType.explInfo.nNextAttach =
	gameData.objs.objects [i].cType.explInfo.nPrevAttach =
	gameData.objs.objects [i].cType.explInfo.nAttachParent =
	gameData.objs.objects [i].attachedObj = -1;
	gameData.objs.objects [i].flags = 0;
	//KillObjectSmoke (i);
	}
gameData.objs.nLastObject = gameData.objs.nObjects - 1;
nDebrisObjectCount = 0;
}

//------------------------------------------------------------------------------
//Tries to find a tSegment for an tObject, using FindSegByPoint ()
int FindObjectSeg (tObject * objP)
{
return FindSegByPoint (&objP->position.vPos, objP->nSegment, 1);
}


//------------------------------------------------------------------------------
//If an tObject is in a tSegment, set its nSegment field and make sure it's
//properly linked.  If not in any tSegment, returns 0, else 1.
//callers should generally use FindVectorIntersection ()
int UpdateObjectSeg (tObject * objP)
{
	int newseg;

newseg = FindObjectSeg (objP);
if (newseg == -1)
	return 0;
if (newseg != objP->nSegment)
	RelinkObject (OBJ_IDX (objP), newseg);
return 1;
}

//------------------------------------------------------------------------------
//go through all gameData.objs.objects and make sure they have the correct tSegment numbers
void FixObjectSegs ()
{
	int i;
return;
for (i = 0; i <= gameData.objs.nLastObject; i++)
	if (gameData.objs.objects [i].nType != OBJ_NONE)
		if (UpdateObjectSeg (gameData.objs.objects + i) == 0) {
#if TRACE			
			con_printf (1, "Cannot find tSegment for tObject %d in FixObjectSegs ()\n");
#endif
			Int3 ();
			COMPUTE_SEGMENT_CENTER_I (&gameData.objs.objects [i].position.vPos, 
											  gameData.objs.objects [i].nSegment);
			}
}

//------------------------------------------------------------------------------
//go through all gameData.objs.objects and make sure they have the correct size
void FixObjectSizes (void)
{
	int i;
	tObject	*objP = gameData.objs.objects;

for (i = 0; i <= gameData.objs.nLastObject; i++, objP++)
	if (objP->nType == OBJ_ROBOT)
		objP->size = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad;
}

//------------------------------------------------------------------------------

//delete gameData.objs.objects, such as weapons & explosions, that shouldn't stay between levels
//	Changed by MK on 10/15/94, don't remove proximity bombs.
//if clear_all is set, clear even proximity bombs
void ClearTransientObjects (int clear_all)
{
	short nObject;
	tObject *objP;

	for (nObject = 0, objP = gameData.objs.objects; nObject <= gameData.objs.nLastObject; nObject++, objP++)
		if (((objP->nType == OBJ_WEAPON) && !(gameData.weapons.info [objP->id].flags&WIF_PLACABLE) && (clear_all || ((objP->id != PROXMINE_ID) && (objP->id != SMARTMINE_ID)))) ||
			   objP->nType == OBJ_FIREBALL ||
			   objP->nType == OBJ_DEBRIS ||
			   ((objP->nType != OBJ_NONE) && (objP->flags & OF_EXPLODING))) {

			#ifdef _DEBUG
#if TRACE			
			if (gameData.objs.objects [nObject].lifeleft > i2f (2))
				con_printf (CONDBG, "Note: Clearing tObject %d (nType=%d, id=%d) with lifeleft=%x\n", 
								nObject, gameData.objs.objects [nObject].nType, 
								gameData.objs.objects [nObject].id, gameData.objs.objects [nObject].lifeleft);
#endif
			#endif
			ReleaseObject (nObject);
		}
		#ifdef _DEBUG
#if TRACE			
		 else if (gameData.objs.objects [nObject].nType!=OBJ_NONE && gameData.objs.objects [nObject].lifeleft < i2f (2))
			con_printf (CONDBG, "Note: NOT clearing tObject %d (nType=%d, id=%d) with lifeleft=%x\n", 
							nObject, gameData.objs.objects [nObject].nType, gameData.objs.objects [nObject].id, 
							gameData.objs.objects [nObject].lifeleft);
#endif
		#endif
}

//------------------------------------------------------------------------------
//attaches an tObject, such as a fireball, to another tObject, such as a robot
void AttachObject (tObject *parentObjP, tObject *childObjP)
{
Assert (childObjP->nType == OBJ_FIREBALL);
Assert (childObjP->controlType == CT_EXPLOSION);
Assert (childObjP->cType.explInfo.nNextAttach==-1);
Assert (childObjP->cType.explInfo.nPrevAttach==-1);
Assert (parentObjP->attachedObj == -1 || 
			gameData.objs.objects [parentObjP->attachedObj].cType.explInfo.nPrevAttach==-1);
childObjP->cType.explInfo.nNextAttach = parentObjP->attachedObj;
if (childObjP->cType.explInfo.nNextAttach != -1)
	gameData.objs.objects [childObjP->cType.explInfo.nNextAttach].cType.explInfo.nPrevAttach = OBJ_IDX (childObjP);
parentObjP->attachedObj = OBJ_IDX (childObjP);
childObjP->cType.explInfo.nAttachParent = OBJ_IDX (parentObjP);
childObjP->flags |= OF_ATTACHED;
Assert (childObjP->cType.explInfo.nNextAttach != OBJ_IDX (childObjP));
Assert (childObjP->cType.explInfo.nPrevAttach != OBJ_IDX (childObjP));
}

//------------------------------------------------------------------------------
//dettaches one tObject
void DetachOneObject (tObject *sub)
{
	Assert (sub->flags & OF_ATTACHED);
	Assert (sub->cType.explInfo.nAttachParent != -1);

if ((gameData.objs.objects [sub->cType.explInfo.nAttachParent].nType != OBJ_NONE) &&
	 (gameData.objs.objects [sub->cType.explInfo.nAttachParent].attachedObj != -1)) {
	if (sub->cType.explInfo.nNextAttach != -1) {
		Assert (gameData.objs.objects [sub->cType.explInfo.nNextAttach].cType.explInfo.nPrevAttach=OBJ_IDX (sub));
		gameData.objs.objects [sub->cType.explInfo.nNextAttach].cType.explInfo.nPrevAttach = sub->cType.explInfo.nPrevAttach;
		}
	if (sub->cType.explInfo.nPrevAttach != -1) {
		Assert (gameData.objs.objects [sub->cType.explInfo.nPrevAttach].cType.explInfo.nNextAttach=OBJ_IDX (sub));
		gameData.objs.objects [sub->cType.explInfo.nPrevAttach].cType.explInfo.nNextAttach = 
			sub->cType.explInfo.nNextAttach;
		}
	else {
		Assert (gameData.objs.objects [sub->cType.explInfo.nAttachParent].attachedObj=OBJ_IDX (sub));
		gameData.objs.objects [sub->cType.explInfo.nAttachParent].attachedObj = sub->cType.explInfo.nNextAttach;
		}
	}
sub->cType.explInfo.nNextAttach = 
sub->cType.explInfo.nPrevAttach =
sub->cType.explInfo.nAttachParent = -1;
sub->flags &= ~OF_ATTACHED;
}

//------------------------------------------------------------------------------
//dettaches all gameData.objs.objects from this tObject
void DetachAllObjects (tObject *parent)
{
while (parent->attachedObj != -1)
	DetachOneObject (gameData.objs.objects + parent->attachedObj);
}

//------------------------------------------------------------------------------
//creates a marker tObject in the world.  returns the tObject number
int DropMarkerObject (vmsVector *pos, short nSegment, vmsMatrix *orient, ubyte marker_num)
{
	short nObject;

Assert (gameData.models.nMarkerModel != -1);
nObject = CreateObject (OBJ_MARKER, marker_num, -1, nSegment, pos, orient, 
								gameData.models.polyModels [gameData.models.nMarkerModel].rad, CT_NONE, MT_NONE, RT_POLYOBJ, 1);
if (nObject >= 0) {
	tObject *objP = &gameData.objs.objects [nObject];
	objP->rType.polyObjInfo.nModel = gameData.models.nMarkerModel;
	VmVecCopyScale (&objP->mType.spinRate, &objP->position.mOrient.uVec, F1_0 / 2);
	//	MK, 10/16/95: Using lifeleft to make it flash, thus able to trim lightlevel from all gameData.objs.objects.
	objP->lifeleft = IMMORTAL_TIME - 1;
	}
return nObject;
}

//------------------------------------------------------------------------------

extern int nAiLastMissileCamera;

//	*viewer is a viewer, probably a missile.
//	wake up all robots that were rendered last frame subject to some constraints.
void WakeupRenderedObjects (tObject *viewer, int window_num)
{
	int	i;

	//	Make sure that we are processing current data.
	if (gameData.app.nFrameCount != windowRenderedData [window_num].frame) {
#if TRACE			
		con_printf (1, "Warning: Called WakeupRenderedObjects with a bogus window.\n");
#endif
		return;
	}

	nAiLastMissileCamera = OBJ_IDX (viewer);

	for (i = 0; i<windowRenderedData [window_num].numObjects; i++) {
		int	nObject;
		tObject *objP;
		int	fcval = gameData.app.nFrameCount & 3;

		nObject = windowRenderedData [window_num].renderedObjects [i];
		if ((nObject & 3) == fcval) {
			objP = &gameData.objs.objects [nObject];

			if (objP->nType == OBJ_ROBOT) {
				if (VmVecDistQuick (&viewer->position.vPos, &objP->position.vPos) < F1_0*100) {
					tAILocal		*ailp = &gameData.ai.localInfo [nObject];
					if (ailp->playerAwarenessType == 0) {
						objP->cType.aiInfo.SUB_FLAGS |= SUB_FLAGS_CAMERA_AWAKE;
						ailp->playerAwarenessType = PA_WEAPON_ROBOT_COLLISION;
						ailp->playerAwarenessTime = F1_0*3;
						ailp->nPrevVisibility = 2;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

void ResetChildObjects (void)
{
	int	i;

for (i = 0; i < MAX_OBJECTS; i++) {
	gameData.objs.childObjs [i].objIndex = -1;
	gameData.objs.childObjs [i].nextObj = i + 1;
	}
gameData.objs.childObjs [i - 1].nextObj = -1;
gameData.objs.nChildFreeList = 0;
memset (gameData.objs.firstChild, 0xff, sizeof (*gameData.objs.firstChild) * MAX_OBJECTS);
memset (gameData.objs.parentObjs, 0xff, sizeof (*gameData.objs.parentObjs) * MAX_OBJECTS);
}

//------------------------------------------------------------------------------

int CheckChildList (int nParent)
{
	int h, i, j;

if (gameData.objs.firstChild [nParent] == gameData.objs.nChildFreeList)
	return 0;
for (h = 0, i = gameData.objs.firstChild [nParent]; i >= 0; i = j, h++) {
	j = gameData.objs.childObjs [i].nextObj;
	if (j == i)
		return 0;
	if (h > gameData.objs.nLastObject)
		return 0;
	}
return 1;
}

//------------------------------------------------------------------------------

int AddChildObjectN (int nParent, int nChild)
{
	int	h, i;

if (gameData.objs.nChildFreeList < 0)
	return 0;
h = gameData.objs.firstChild [nParent];
i = gameData.objs.firstChild [nParent] = gameData.objs.nChildFreeList;
gameData.objs.nChildFreeList = gameData.objs.childObjs [gameData.objs.nChildFreeList].nextObj;
gameData.objs.childObjs [i].nextObj = h;
gameData.objs.childObjs [i].objIndex = nChild;
gameData.objs.parentObjs [nChild] = nParent;
CheckChildList (nParent);
return 1;
}

//------------------------------------------------------------------------------

int AddChildObjectP (tObject *pParent, tObject *pChild)
{
return pParent ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
//return (pParent->nType == OBJ_PLAYER) ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
}

//------------------------------------------------------------------------------

int DelObjChildrenN (int nParent)
{
	int	i, j;

for (i = gameData.objs.firstChild [nParent]; i >= 0; i = j) {
	j = gameData.objs.childObjs [i].nextObj;
	gameData.objs.childObjs [i].nextObj = gameData.objs.nChildFreeList;
	gameData.objs.childObjs [i].objIndex = -1;
	gameData.objs.nChildFreeList = i;
	}
return 1;
}

//------------------------------------------------------------------------------

int DelObjChildrenP (tObject *pParent)
{
return DelObjChildrenN (OBJ_IDX (pParent));
}

//------------------------------------------------------------------------------

int DelObjChildN (int nChild)
{
	int	nParent, h = -1, i, j;

if (0 > (nParent = gameData.objs.parentObjs [nChild]))
	return 0;
for (i = gameData.objs.firstChild [nParent]; i >= 0; i = j) {
	j = gameData.objs.childObjs [i].nextObj;
	if (gameData.objs.childObjs [i].objIndex == nChild) {
		if (h < 0)
			gameData.objs.firstChild [nParent] = j;
		else
			gameData.objs.childObjs [h].nextObj = j;
		gameData.objs.childObjs [i].nextObj = gameData.objs.nChildFreeList;
		gameData.objs.childObjs [i].objIndex = -1;
		gameData.objs.nChildFreeList = i;
		CheckChildList (nParent);
		return 1;
		}
	h = i;
	}
return 0;
}

//------------------------------------------------------------------------------

int DelObjChildP (tObject *pChild)
{
return DelObjChildN (OBJ_IDX (pChild));
}

//------------------------------------------------------------------------------

tObjectRef *GetChildObjN (short nParent, tObjectRef *pChildRef)
{
	int i = pChildRef ? pChildRef->nextObj : gameData.objs.firstChild [nParent];

return (i < 0) ? NULL : gameData.objs.childObjs + i;
}

//------------------------------------------------------------------------------

tObjectRef *GetChildObjP (tObject *pParent, tObjectRef *pChildRef)
{
return GetChildObjN (OBJ_IDX (pParent), pChildRef);
}

//------------------------------------------------------------------------------

int CountPlayerObjects (int nPlayer, int nType, int nId)
{
	int		i, h = 0;
	tObject	*objP;

for (i = gameData.objs.nLastObject + 1, objP = gameData.objs.objects; i; i--, objP++)
	if ((objP->nType == nType) && (objP->id == nId) &&
		 (objP->cType.laserInfo.parentType == OBJ_PLAYER) &&
		 (gameData.objs.objects [objP->cType.laserInfo.nParentObj].id == nPlayer))
	h++;
return h;
}

//------------------------------------------------------------------------------

void InitWeaponFlags (void)
{
memset (gameData.objs.bIsMissile, 0, sizeof (gameData.objs.bIsMissile));
gameData.objs.bIsMissile [CONCUSSION_ID] =
gameData.objs.bIsMissile [HOMINGMSL_ID] =
gameData.objs.bIsMissile [SMARTMSL_ID] =
gameData.objs.bIsMissile [MEGAMSL_ID] =
gameData.objs.bIsMissile [FLASHMSL_ID] =
gameData.objs.bIsMissile [GUIDEDMSL_ID] =
gameData.objs.bIsMissile [MERCURYMSL_ID] =
gameData.objs.bIsMissile [EARTHSHAKER_ID] =
gameData.objs.bIsMissile [EARTHSHAKER_MEGA_ID] =
gameData.objs.bIsMissile [ROBOT_CONCUSSION_ID] =
gameData.objs.bIsMissile [ROBOT_HOMINGMSL_ID] =
gameData.objs.bIsMissile [ROBOT_FLASHMSL_ID] =
gameData.objs.bIsMissile [ROBOT_MERCURYMSL_ID] =
gameData.objs.bIsMissile [ROBOT_VERTIGO_FLASHMSL_ID] =
gameData.objs.bIsMissile [ROBOT_SMARTMSL_ID] =
gameData.objs.bIsMissile [ROBOT_MEGAMSL_ID] =
gameData.objs.bIsMissile [ROBOT_EARTHSHAKER_ID] =
gameData.objs.bIsMissile [ROBOT_SHAKER_MEGA_ID] = 1;

memset (gameData.objs.bIsWeapon, 0, sizeof (gameData.objs.bIsWeapon));
gameData.objs.bIsWeapon [VULCAN_ID] =
gameData.objs.bIsWeapon [GAUSS_ID] =
gameData.objs.bIsWeapon [ROBOT_VULCAN_ID] = 0;
gameData.objs.bIsWeapon [LASER_ID] =
gameData.objs.bIsWeapon [LASER_ID + 1] =
gameData.objs.bIsWeapon [LASER_ID + 2] =
gameData.objs.bIsWeapon [LASER_ID + 3] =
gameData.objs.bIsWeapon [REACTOR_BLOB_ID] =
gameData.objs.bIsWeapon [ROBOT_SLOW_PHOENIX_ID] =
gameData.objs.bIsWeapon [SMARTMSL_BLOB_ID] =
gameData.objs.bIsWeapon [SMARTMINE_BLOB_ID] =
gameData.objs.bIsWeapon [ROBOT_SMARTMINE_BLOB_ID] =
gameData.objs.bIsWeapon [FLARE_ID] =
gameData.objs.bIsWeapon [SPREADFIRE_ID] =
gameData.objs.bIsWeapon [PLASMA_ID] =
gameData.objs.bIsWeapon [FUSION_ID] =
gameData.objs.bIsWeapon [SUPERLASER_ID] =
gameData.objs.bIsWeapon [SUPERLASER_ID + 1] =
gameData.objs.bIsWeapon [HELIX_ID] =
gameData.objs.bIsWeapon [PHOENIX_ID] =
gameData.objs.bIsWeapon [OMEGA_ID] =
gameData.objs.bIsWeapon [ROBOT_PLASMA_ID] =
gameData.objs.bIsWeapon [ROBOT_MEDIUM_FIREBALL_ID] =
gameData.objs.bIsWeapon [ROBOT_SMARTMSL_BLOB_ID] =
gameData.objs.bIsWeapon [ROBOT_SMALL_FIREBALL_ID] =
gameData.objs.bIsWeapon [ROBOT_PHOENIX_ID] =
gameData.objs.bIsWeapon [ROBOT_FAST_PHOENIX_ID] =
gameData.objs.bIsWeapon [ROBOT_PHASE_ENERGY_ID] =
gameData.objs.bIsWeapon [ROBOT_VERTIGO_FLASHMSL_ID] =
gameData.objs.bIsWeapon [ROBOT_VERTIGO_FLASHMSL_ID + 1] =
gameData.objs.bIsWeapon [ROBOT_VERTIGO_FIREBALL_ID] =
gameData.objs.bIsWeapon [ROBOT_VERTIGO_PHOENIX_ID] =
gameData.objs.bIsWeapon [ROBOT_HELIX_ID] =
gameData.objs.bIsWeapon [ROBOT_BLUE_ENERGY_ID] =
gameData.objs.bIsWeapon [ROBOT_WHITE_ENERGY_ID] =
gameData.objs.bIsWeapon [ROBOT_BLUE_LASER_ID] =
gameData.objs.bIsWeapon [ROBOT_RED_LASER_ID] =
gameData.objs.bIsWeapon [ROBOT_GREEN_LASER_ID] =
gameData.objs.bIsWeapon [ROBOT_WHITE_LASER_ID] = 1;

memset (gameData.objs.bIsSlowWeapon, 0, sizeof (gameData.objs.bIsSlowWeapon));
gameData.objs.bIsSlowWeapon [REACTOR_BLOB_ID] =
gameData.objs.bIsSlowWeapon [ROBOT_SMALL_FIREBALL_ID] =
gameData.objs.bIsSlowWeapon [SMARTMINE_BLOB_ID] =
gameData.objs.bIsSlowWeapon [ROBOT_SMARTMINE_BLOB_ID] =
gameData.objs.bIsSlowWeapon [SMARTMSL_BLOB_ID] =
gameData.objs.bIsSlowWeapon [ROBOT_SMARTMSL_BLOB_ID] = 1;
}

//------------------------------------------------------------------------------
//eof
