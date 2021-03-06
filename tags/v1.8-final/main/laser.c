/* $Id: laser.c,v 1.10 2003/10/10 09:36:35 btb Exp $ */
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

/*
 *
 * This will contain the laser code
 *
 * Old Log:
 * Revision 1.1  1993/11/29  17:19:02  john
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
char laser_rcsid [] = "$Id: laser.c,v 1.10 2003/10/10 09:36:35 btb Exp $";
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "inferno.h"
#include "game.h"
#include "bm.h"
#include "object.h"
#include "laser.h"
#include "args.h"
#include "segment.h"
#include "fvi.h"
#include "segpoint.h"
#include "error.h"
#include "mono.h"
#include "key.h"
#include "texmap.h"
#include "textures.h"
#include "render.h"
#include "vclip.h"
#include "fireball.h"
#include "polyobj.h"
#include "robot.h"
#include "weapon.h"
#include "newdemo.h"
#include "timer.h"
#include "player.h"
#include "sounds.h"
#ifdef NETWORK
#include "network.h"
#endif
#include "ai.h"
#ifdef NETWORK
#include "modem.h"
#endif
#include "powerup.h"
#include "multi.h"
#include "physics.h"
#include "multi.h"
#include "hudmsg.h"
#include "gameseg.h"

#ifdef TACTILE
#include "tactile.h"
#endif

int FindHomingObjectComplete (vmsVector *curpos, tObject *tracker, int track_objType1, int track_objType2);

extern void NDRecordGuidedEnd ();
extern void NDRecordGuidedStart ();

int FindHomingObject (vmsVector *curpos, tObject *tracker);

extern int bDoingLightingHack;

//	-------------------------------------------------------------------------------------------------------------------------------
//	***** HEY ARTISTS!!*****
//	Here are the constants you're looking for!--MK

//	Change the following constants to affect the look of the omega cannon.
//	Changing these constants will not affect the damage done.
//	WARNING: If you change DESIRED_OMEGA_DIST and MAX_OMEGA_BLOBS, you don't merely change the look of the cannon,
//	you change its range.  If you decrease DESIRED_OMEGA_DIST, you decrease how far the gun can fire.
#define	MIN_OMEGA_BLOBS		3				//	No matter how close the obstruction, at this many blobs created.
#define	MIN_OMEGA_DIST			(F1_0*3)		//	At least this distance between blobs, unless doing so would violate MIN_OMEGA_BLOBS
#define	DESIRED_OMEGA_DIST	(F1_0*5)		//	This is the desired distance between blobs.  For distances > MIN_OMEGA_BLOBS*DESIRED_OMEGA_DIST, but not very large, this will apply.
#define	MAX_OMEGA_BLOBS		16				//	No matter how far away the obstruction, this is the maximum number of blobs.
#define	MAX_OMEGA_DIST			(MAX_OMEGA_BLOBS * DESIRED_OMEGA_DIST)		//	Maximum extent of lightning blobs.

//	Additionally, several constants which apply to homing gameData.objs.objects in general control the behavior of the Omega Cannon.
//	They are defined in laser.h.  They are copied here for reference.  These values are valid on 1/10/96:
//	If you want the Omega Cannon view cone to be different than the Homing Missile viewcone, contact MK to make the change.
//	 (Unless you are a programmer, in which case, do it yourself!)
#define	OMEGA_MIN_TRACKABLE_DOT			 (15*F1_0/16)		//	Larger values mean narrower cone.  F1_0 means damn near impossible.  0 means 180 degree field of view.
#define	OMEGA_MAX_TRACKABLE_DIST		MAX_OMEGA_DIST	//	An tObject must be at least this close to be tracked.

//	Note, you don't need to change these constants.  You can control damage and energy consumption by changing the
//	usual bitmaps.tbl parameters.
#define	OMEGA_DAMAGE_SCALE			32				//	Controls how much damage is done.  This gets multiplied by gameData.time.xFrame and then again by the damage specified in bitmaps.tbl in the $WEAPON line.
#define	OMEGA_ENERGY_CONSUMPTION	16				//	Controls how much energy is consumed.  This gets multiplied by gameData.time.xFrame and then again by the energy parameter from bitmaps.tbl.

#define	MIN_OMEGA_CHARGE	 (MAX_OMEGA_CHARGE/8)
#define	OMEGA_CHARGE_SCALE	4			//	gameData.time.xFrame / OMEGA_CHARGE_SCALE added to xOmegaCharge every frame.

#define	FULL_COCKPIT_OFFS 0
#define	LASER_OFFS	 ((F1_0 * 29) / 100)

#define	HOMING_MISSILE_SCALE	16

#define	MAX_SMART_DISTANCE	 (F1_0*150)
#define	MAX_OBJDISTS			30

fix	xOmegaCharge = MAX_OMEGA_CHARGE;
int	nLastOmegaFireFrame = 0;

//---------------------------------------------------------------------------------
// Called by render code.... determines if the laser is from a robot or the
// player and calls the appropriate routine.

void RenderLaser (tObject *objP)
{

//	Commented out by John (sort of, typed by Mike) on 6/8/94
#if 0
	switch (objP->id)	{
	case WEAPON_TYPE_WEAK_LASER:
	case WEAPON_TYPE_STRONG_LASER:
	case WEAPON_TYPE_CANNON_BALL:
	case WEAPON_TYPE_MISSILE:
		break;
	default:
		Error ("Invalid weapon nType in RenderLaser\n");
	}
#endif
	
switch (gameData.weapons.info [objP->id].renderType)	{
	case WEAPON_RENDER_LASER:
		Int3 ();	// Not supported anymore!
					//Laser_draw_one (OBJ_IDX (objP), gameData.weapons.info [objP->id].bitmap);
		break;
	case WEAPON_RENDER_BLOB:
		DrawObjectBlob (objP, gameData.weapons.info [objP->id].bitmap, gameData.weapons.info [objP->id].bitmap, 0, NULL, 2.0f / 3.0f);
		break;
	case WEAPON_RENDER_POLYMODEL:
		break;
	case WEAPON_RENDER_VCLIP:
		Int3 ();	//	Oops, not supported, nType added by mk on 09/09/94, but not for lasers...
	default:
		Error ("Invalid weapon render nType in RenderLaser\n");
	}
}

//---------------------------------------------------------------------------------
// Draws a texture-mapped laser bolt

//void Laser_draw_one (int nObject, grsBitmap * bmp)
//{
//	int t1, t2, t3;
//	g3sPoint p1, p2;
//	tObject *objP;
//	vmsVector start_pos,vEndPos;
//
//	obj = &gameData.objs.objects [nObject];
//
//	start_pos = objP->pos;
//	VmVecScaleAdd (&vEndPos,&start_pos,&objP->orient.fVec,-Laser_length);
//
//	G3TransformAndEncodePoint (&p1,&start_pos);
//	G3TransformAndEncodePoint (&p2,&vEndPos);
//
//	t1 = gameStates.render.nLighting;
//	t2 = gameStates.render.nInterpolationMethod;
//	t3 = gameStates.render.bTransparency;
//
//	gameStates.render.nLighting  = 0;
//	//gameStates.render.nInterpolationMethod = 3;	// Full perspective
//	gameStates.render.nInterpolationMethod = 1;	// Linear
//	gameStates.render.bTransparency = 1;
//
//	//GrSetColor (gr_getcolor (31,15,0);
//	//g3_draw_line_ptrs (p1,p2);
//	//g3_draw_rod (p1,0x2000,p2,0x2000);
//	//g3_draw_rod (p1,Laser_width,p2,Laser_width);
//	G3DrawRodTexPoly (bmp,&p2,Laser_width,&p1,Laser_width,0);
//	gameStates.render.nLighting = t1;
//	gameStates.render.nInterpolationMethod = t2;
//	gameStates.render.bTransparency = t3;
//
//}

//	Changed by MK on 09/07/94
//	I want you to be able to blow up your own bombs.
//	AND...Your proximity bombs can blow you up if they're 2.0 seconds or more old.
//	Changed by MK on 06/06/95: Now must be 4.0 seconds old.  Much valid Net-complaining.
int LasersAreRelated (int o1, int o2)
{
	tObject	*objP1, *objP2;
	short		id1, id2;
	fix		ct1, ct2;

if ((o1 < 0) || (o2 < 0))	
	return 0;
objP1 = gameData.objs.objects + o1;
objP2 = gameData.objs.objects + o2;
id1 = objP1->id;
id2 = objP2->id;
ct1 = objP1->cType.laserInfo.creationTime;
ct2 = objP2->cType.laserInfo.creationTime;
// See if o2 is the parent of o1
if (objP1->nType == OBJ_WEAPON)
	if ((objP1->cType.laserInfo.nParentObj == o2) && 
		 (objP1->cType.laserInfo.nParentSig == objP2->nSignature))
		//	o1 is a weapon, o2 is the parent of 1, so if o1 is PROXIMITY_BOMB and o2 is player, they are related only if o1 < 2.0 seconds old
		if ((id1 == PHOENIX_ID && (gameData.time.xGame > ct1 + F1_0/4)) || 
		   (id1 == GUIDEDMISS_ID && (gameData.time.xGame > ct1 + F1_0*2)) || 
		   (((id1 == PROXIMITY_ID) || (id1 == SUPERPROX_ID)) && (gameData.time.xGame > ct1 + F1_0*4)))
			return 0;
		else
			return 1;

	// See if o1 is the parent of o2
if (objP2->nType == OBJ_WEAPON)
	if ((objP2->cType.laserInfo.nParentObj == o1) && 
			 (objP2->cType.laserInfo.nParentSig == objP1->nSignature))
		//	o2 is a weapon, o1 is the parent of 2, so if o2 is PROXIMITY_BOMB and o1 is player, they are related only if o1 < 2.0 seconds old
		if ((id2 == PHOENIX_ID && (gameData.time.xGame > ct2 + F1_0/4)) || 
			  (id2 == GUIDEDMISS_ID && (gameData.time.xGame > ct2 + F1_0*2)) || 
				 (((id2 == PROXIMITY_ID) || (id2 == SUPERPROX_ID)) && (gameData.time.xGame > ct2 + F1_0*4)))
			return 0;
		else
			return 1;

// They must both be weapons
if (objP1->nType != OBJ_WEAPON || objP2->nType != OBJ_WEAPON)	
	return 0;

//	Here is the 09/07/94 change -- Siblings must be identical, others can hurt each other
// See if they're siblings...
//	MK: 06/08/95, Don't allow prox bombs to detonate for 3/4 second.  Else too likely to get toasted by your own bomb if hit by opponent.
if (objP1->cType.laserInfo.nParentSig==objP2->cType.laserInfo.nParentSig) {
	if (id1 != PROXIMITY_ID  && id2 != PROXIMITY_ID && id1 != SUPERPROX_ID && id2 != SUPERPROX_ID)
		return 1;
	//	If neither is older than 1/2 second, then can't blow up!
	if ((gameData.time.xGame > (ct1 + F1_0/2)) || (gameData.time.xGame > (ct2 + F1_0/2)))
		return 0;
	return 1;
	}

//	Anything can cause a collision with a robot super prox mine.
if (id1 == ROBOT_SUPERPROX_ID || id2 == ROBOT_SUPERPROX_ID ||
	 id1 == PROXIMITY_ID || id2 == PROXIMITY_ID ||
	 id1 == SUPERPROX_ID || id2 == SUPERPROX_ID ||
	 id1 == PMINE_ID || id2 == PMINE_ID)
	return 0;
return 1;
}

//---------------------------------------------------------------------------------
//--unused-- int Muzzle_scale=2;
int nLaserOffset=0;

void DoMuzzleStuff (int nSegment, vmsVector *pos)
{
gameData.muzzle.info [gameData.muzzle.queueIndex].createTime = TimerGetFixedSeconds ();
gameData.muzzle.info [gameData.muzzle.queueIndex].nSegment = nSegment;
gameData.muzzle.info [gameData.muzzle.queueIndex].pos = *pos;
gameData.muzzle.queueIndex++;
if (gameData.muzzle.queueIndex >= MUZZLE_QUEUE_MAX)
	gameData.muzzle.queueIndex = 0;
}

//---------------------------------------------------------------------------------
//creates a weapon tObject
int CreateWeaponObject (ubyte nWeaponType, short nSegment,vmsVector *vPosition)
{
	int rType=-1;
	fix laser_radius = -1;
	int nObject;
	tObject *objP;

switch (gameData.weapons.info [nWeaponType].renderType)	{
	case WEAPON_RENDER_BLOB:
		rType = RT_LASER;			// Render as a laser even if blob (see render code above for explanation)
		laser_radius = gameData.weapons.info [nWeaponType].blob_size;
		break;
	case WEAPON_RENDER_POLYMODEL:
		laser_radius = 0;	//	Filled in below.
		rType = RT_POLYOBJ;
		break;
	case WEAPON_RENDER_LASER:
		Int3 (); 	// Not supported anymore
		break;
	case WEAPON_RENDER_NONE:
		rType = RT_NONE;
		laser_radius = F1_0;
		break;
	case WEAPON_RENDER_VCLIP:
		rType = RT_WEAPON_VCLIP;
		laser_radius = gameData.weapons.info [nWeaponType].blob_size;
		break;
	default:
		Error ("Invalid weapon render nType in CreateNewLaser\n");
		return -1;
	}

Assert (laser_radius != -1);
Assert (rType != -1);
nObject = CreateObject ((ubyte) OBJ_WEAPON, nWeaponType, -1, nSegment, vPosition, NULL, laser_radius, (ubyte) CT_WEAPON, (ubyte) MT_PHYSICS, (ubyte) rType, 1);
objP = gameData.objs.objects + nObject;
if (gameData.weapons.info [nWeaponType].renderType == WEAPON_RENDER_POLYMODEL) {
	objP->rType.polyObjInfo.nModel = gameData.weapons.info [objP->id].nModel;
	objP->size = FixDiv (gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad, 
								gameData.weapons.info [objP->id].po_len_to_width_ratio);
	}
objP->mType.physInfo.mass = WI_mass (nWeaponType);
objP->mType.physInfo.drag = WI_drag (nWeaponType);
VmVecZero (&objP->mType.physInfo.thrust);
if (gameData.weapons.info [nWeaponType].bounce == 1)
	objP->mType.physInfo.flags |= PF_BOUNCE;
if ((gameData.weapons.info [nWeaponType].bounce == 2) || gameStates.app.cheats.bBouncingWeapons)
	objP->mType.physInfo.flags |= PF_BOUNCE + PF_BOUNCES_TWICE;
return nObject;
}

//	-------------------------------------------------------------------------------------------------------------------------------

void DeleteOldOmegaBlobs (tObject *parentObjP)
{
	short	i;
	int	count = 0;
	int	nParentObj = parentObjP->cType.laserInfo.nParentObj;

for (i = 0; i <= gameData.objs.nLastObject; i++)
	if (gameData.objs.objects [i].nType == OBJ_WEAPON)
		if (gameData.objs.objects [i].id == OMEGA_ID)
			if (gameData.objs.objects [i].cType.laserInfo.nParentObj == nParentObj) {
				ReleaseObject (i);
				count++;
				}
}

// ---------------------------------------------------------------------------------

void CreateOmegaBlobs (short nFiringSeg, vmsVector *vFiringPos, vmsVector *vGoalPos, tObject *parentObjP)
{
	short			nLastSeg, nLastCreatedObj = -1;
	vmsVector	vGoal;
	fix			xGoalDist;
	int			nOmegaBlobs;
	fix			xOmegaBlobDist;
	vmsVector	vOmegaDelta;
	vmsVector	vBlobPos, vPerturb;
	fix			xPerturbArray [MAX_OMEGA_BLOBS];
	int			i;

if (gameData.app.nGameMode & GM_MULTI)
	DeleteOldOmegaBlobs (parentObjP);
VmVecSub (&vGoal, vGoalPos, vFiringPos);
xGoalDist = VmVecNormalizeQuick (&vGoal);
if (xGoalDist < MIN_OMEGA_BLOBS * MIN_OMEGA_DIST) {
	xOmegaBlobDist = MIN_OMEGA_DIST;
	nOmegaBlobs = xGoalDist/xOmegaBlobDist;
	if (nOmegaBlobs == 0)
		nOmegaBlobs = 1;
	} 
else {
	xOmegaBlobDist = DESIRED_OMEGA_DIST;
	nOmegaBlobs = xGoalDist / xOmegaBlobDist;
	if (nOmegaBlobs > MAX_OMEGA_BLOBS) {
		nOmegaBlobs = MAX_OMEGA_BLOBS;
		xOmegaBlobDist = xGoalDist / nOmegaBlobs;
		} 
	else if (nOmegaBlobs < MIN_OMEGA_BLOBS) {
		nOmegaBlobs = MIN_OMEGA_BLOBS;
		xOmegaBlobDist = xGoalDist / nOmegaBlobs;
		}
	}
vOmegaDelta = vGoal;
VmVecScale (&vOmegaDelta, xOmegaBlobDist);
//	Now, create all the blobs
vBlobPos = *vFiringPos;
nLastSeg = nFiringSeg;

//	If nearby, don't perturb vector.  If not nearby, start halfway out.
if (xGoalDist < MIN_OMEGA_DIST*4) {
	for (i=0; i<nOmegaBlobs; i++)
		xPerturbArray [i] = 0;
	} 
else {
	VmVecScaleInc (&vBlobPos, &vOmegaDelta, F1_0/2);	//	Put first blob half way out.
	for (i=0; i<nOmegaBlobs/2; i++) {
		xPerturbArray [i] = F1_0*i + F1_0/4;
		xPerturbArray [nOmegaBlobs-1-i] = F1_0*i;
		}
	}

//	Create random perturbation vector, but favor _not_ going up in player's reference.
MakeRandomVector (&vPerturb);
VmVecScaleInc (&vPerturb, &parentObjP->orient.uVec, -F1_0/2);
//bDoingLightingHack = 1;	//	Ugly, but prevents blobs which are probably outside the mine from killing framerate.
for (i=0; i<nOmegaBlobs; i++) {
	vmsVector	temp_pos;
	short			blob_objnum, nSegment;

	//	This will put the last blob right at the destination tObject, causing damage.
	if (i == nOmegaBlobs-1)
		VmVecScaleInc (&vBlobPos, &vOmegaDelta, 15*F1_0/32);	//	Move last blob another (almost) half section
	//	Every so often, re-perturb blobs
	if ((i % 4) == 3) {
		vmsVector	temp_vec;

		MakeRandomVector (&temp_vec);
		VmVecScaleInc (&vPerturb, &temp_vec, F1_0/4);
		}
	VmVecScaleAdd (&temp_pos, &vBlobPos, &vPerturb, xPerturbArray [i]);
	nSegment = FindSegByPoint (&temp_pos, nLastSeg);
	if (nSegment != -1) {
		tObject		*objP;

		nLastSeg = nSegment;
		blob_objnum = CreateObject (OBJ_WEAPON, OMEGA_ID, -1, nSegment, &temp_pos, NULL, 0, 
											 CT_WEAPON, MT_PHYSICS, RT_WEAPON_VCLIP, 1);
		if (blob_objnum == -1)
			break;
		nLastCreatedObj = blob_objnum;
		objP = gameData.objs.objects + blob_objnum;
		objP->lifeleft = ONE_FRAME_TIME;
		objP->mType.physInfo.velocity = vGoal;
		//	Only make the last one move fast, else multiple blobs might collide with target.
		VmVecScale (&objP->mType.physInfo.velocity, F1_0*4);
		objP->size = gameData.weapons.info [objP->id].blob_size;
		objP->shields = FixMul (OMEGA_DAMAGE_SCALE*gameData.time.xFrame, WI_strength (objP->id,gameStates.app.nDifficultyLevel));
		objP->cType.laserInfo.parentType			= parentObjP->nType;
		objP->cType.laserInfo.nParentSig	= parentObjP->nSignature;
		objP->cType.laserInfo.nParentObj			= OBJ_IDX (parentObjP);
		objP->movementType = MT_NONE;	//	Only last one moves, that will get bashed below.
		}
	VmVecInc (&vBlobPos, &vOmegaDelta);
	}

	//	Make last one move faster, but it's already moving at speed = F1_0*4.
if (nLastCreatedObj != -1) {
	VmVecScale (
		&gameData.objs.objects [nLastCreatedObj].mType.physInfo.velocity, 
		gameData.weapons.info [OMEGA_ID].speed [gameStates.app.nDifficultyLevel]/4);
	gameData.objs.objects [nLastCreatedObj].movementType = MT_PHYSICS;
	}
bDoingLightingHack = 0;
}

// ---------------------------------------------------------------------------------
//	Call this every frame to recharge the Omega Cannon.
void OmegaChargeFrame (void)
{
	fix	xDeltaCharge, xOldOmegaCharge;

if (xOmegaCharge == MAX_OMEGA_CHARGE)
	return;

if (!(PlayerHasWeapon (OMEGA_INDEX, 0, -1) & HAS_WEAPON_FLAG))
	return;
if (gameStates.app.bPlayerIsDead)
	return;
if ((gameData.weapons.nPrimary == OMEGA_INDEX) && 
		!xOmegaCharge && 
		!gameData.multi.players [gameData.multi.nLocalPlayer].energy) {
	gameData.weapons.nPrimary--;
	AutoSelectWeapon (0, 1);
	}
//	Don't charge while firing.
if ((nLastOmegaFireFrame == gameData.app.nFrameCount) || 
		 (nLastOmegaFireFrame == gameData.app.nFrameCount-1))
	return;

if (gameData.multi.players [gameData.multi.nLocalPlayer].energy) {
	fix	xEnergyUsed;

	xOldOmegaCharge = xOmegaCharge;
	xOmegaCharge += gameData.time.xFrame/OMEGA_CHARGE_SCALE;
	if (xOmegaCharge > MAX_OMEGA_CHARGE)
		xOmegaCharge = MAX_OMEGA_CHARGE;
	xDeltaCharge = xOmegaCharge - xOldOmegaCharge;
	xEnergyUsed = FixMul (F1_0*190/17, xDeltaCharge);
	if (gameStates.app.nDifficultyLevel < 2)
		xEnergyUsed = FixMul (xEnergyUsed, i2f (gameStates.app.nDifficultyLevel+2)/4);

	gameData.multi.players [gameData.multi.nLocalPlayer].energy -= xEnergyUsed;
	if (gameData.multi.players [gameData.multi.nLocalPlayer].energy < 0)
		gameData.multi.players [gameData.multi.nLocalPlayer].energy = 0;
	}
}

// -- fix	Last_omega_muzzle_flashTime;

// ---------------------------------------------------------------------------------
//	*objP is the tObject firing the omega cannon
//	*pos is the location from which the omega bolt starts
void DoOmegaStuff (tObject *parentObjP, vmsVector *vFiringPos, tObject *weaponObjP)
{
	short			nLockObj, nFiringSeg;
	vmsVector	vGoalPos;
	int			pnum = parentObjP->id;

if (pnum == gameData.multi.nLocalPlayer) {
	//	If charge >= min, or (some charge and zero energy), allow to fire.
	if (!((xOmegaCharge >= MIN_OMEGA_CHARGE) || 
			 (xOmegaCharge && !gameData.multi.players [pnum].energy))) {
		ReleaseObject (OBJ_IDX (weaponObjP));
		return;
		}
	xOmegaCharge -= gameData.time.xFrame;
	if (xOmegaCharge < 0)
		xOmegaCharge = 0;
	//	Ensure that the lightning cannon can be fired next frame.
	xNextLaserFireTime = gameData.time.xGame+1;
	nLastOmegaFireFrame = gameData.app.nFrameCount;
	}

weaponObjP->cType.laserInfo.parentType = OBJ_PLAYER;
weaponObjP->cType.laserInfo.nParentObj = gameData.multi.players [pnum].nObject;
weaponObjP->cType.laserInfo.nParentSig = gameData.objs.objects [gameData.multi.players [pnum].nObject].nSignature;

if (gameStates.limitFPS.bOmega && !gameStates.app.b40fpsTick)
	nLockObj = -1;
else
	nLockObj = FindHomingObject (vFiringPos, weaponObjP);
if (0 > (nFiringSeg = FindSegByPoint (vFiringPos, parentObjP->nSegment)))
	return;

//	Play sound.
if (parentObjP == gameData.objs.viewer)
	DigiPlaySample (gameData.weapons.info [weaponObjP->id].flashSound, F1_0);
else
	DigiLinkSoundToPos (gameData.weapons.info [weaponObjP->id].flashSound, weaponObjP->nSegment, 0, &weaponObjP->pos, 0, F1_0);

// -- if ((Last_omega_muzzle_flashTime + F1_0/4 < gameData.time.xGame) || (Last_omega_muzzle_flashTime > gameData.time.xGame)) {
// -- 	DoMuzzleStuff (nFiringSeg, vFiringPos);
// -- 	Last_omega_muzzle_flashTime = gameData.time.xGame;
// -- }

//	Delete the original tObject.  Its only purpose in life was to determine which tObject to home in on.
ReleaseObject (OBJ_IDX (weaponObjP));
if (nLockObj != -1)
	vGoalPos = gameData.objs.objects [nLockObj].pos;
else {	//	If couldn't lock on anything, fire straight ahead.
	fvi_query	fq;
	fvi_info		hit_data;
	int			fate;
	vmsVector	vPerturb, perturbed_fvec;

	MakeRandomVector (&vPerturb);
	VmVecScaleAdd (&perturbed_fvec, &parentObjP->orient.fVec, &vPerturb, F1_0/16);
	VmVecScaleAdd (&vGoalPos, vFiringPos, &perturbed_fvec, MAX_OMEGA_DIST);
	fq.startSeg = nFiringSeg;
	fq.p0 = vFiringPos;
	fq.p1	= &vGoalPos;
	fq.rad = 0;
	fq.thisObjNum = OBJ_IDX (parentObjP);
	fq.ignoreObjList = NULL;
	fq.flags = FQ_IGNORE_POWERUPS | FQ_TRANSPOINT | FQ_CHECK_OBJS;		//what about trans walls???
	fate = FindVectorIntersection (&fq, &hit_data);
	if (fate != HIT_NONE) {
		Assert (hit_data.hit.nSegment != -1);		//	How can this be?  We went from inside the mine to outside without hitting anything?
		vGoalPos = hit_data.hit.vPoint;
		}
	}
//	This is where we create a pile of omega blobs!
CreateOmegaBlobs (nFiringSeg, vFiringPos, &vGoalPos, parentObjP);
}

// ---------------------------------------------------------------------------------

// Initializes a laser after Fire is pressed 
//	Returns tObject number.
int CreateNewLaser (vmsVector *vDirection, vmsVector *vPosition, short nSegment, 
						  short nParent, ubyte nWeaponType, int bMakeSound)
{
	int		nObject;
	tObject	*objP;
	fix		xParentSpeed, xWeaponSpeed;
	fix		volume;
	fix		xLaserLength=0;
	Assert (nWeaponType < gameData.weapons.nTypes [0]);

if (nWeaponType >= gameData.weapons.nTypes [0])
	nWeaponType = 0;
//	Don't let homing blobs make muzzle flash.
if (gameData.objs.objects [nParent].nType == OBJ_ROBOT)
	DoMuzzleStuff (nSegment, vPosition);
else if (gameStates.app.bD2XLevel && 
			(gameData.objs.objects + nParent == gameData.objs.console) && 
			(gameData.segs.segment2s [gameData.objs.console->nSegment].special == SEGMENT_IS_NODAMAGE))
	return -1;
#if 1
if ((nParent == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) &&
		(nWeaponType == PROXIMITY_ID) && 
		(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY))) {
	nObject = CreateObject (OBJ_POWERUP, POW_HOARD_ORB, -1, nSegment, vPosition, &vmdIdentityMatrix, 
									gameData.objs.pwrUp.info [POW_HOARD_ORB].size, CT_POWERUP, MT_PHYSICS, RT_POWERUP, 1);
	if (nObject >= 0) {
		objP = gameData.objs.objects + nObject;
		if (gameData.app.nGameMode & GM_MULTI)
			multiData.create.nObjNums [multiData.create.nLoc++] = nObject;
		objP->rType.vClipInfo.nClipIndex = gameData.objs.pwrUp.info [objP->id].nClipIndex;
		objP->rType.vClipInfo.xFrameTime = gameData.eff.vClips [0] [objP->rType.vClipInfo.nClipIndex].xFrameTime;
		objP->rType.vClipInfo.nCurFrame = 0;
		objP->matCenCreator = GetTeam (gameData.multi.nLocalPlayer) + 1;
		}
	return -1;
	}
#endif
nObject = CreateWeaponObject (nWeaponType, nSegment, vPosition);
if (nObject < 0)
	return -1;
objP = gameData.objs.objects + nObject;
//	Do the special Omega Cannon stuff.  Then return on account of everything that follows does
//	not apply to the Omega Cannon.
if (nWeaponType == OMEGA_ID) {
	// Create orientation matrix for tracking purposes.
	VmVector2Matrix (&objP->orient, vDirection, &gameData.objs.objects [nParent].orient.uVec ,NULL);
	if ((gameData.objs.objects + nParent != gameData.objs.viewer) &&
		 (gameData.objs.objects [nParent].nType != OBJ_WEAPON)) {
		// Muzzle flash		
		if (gameData.weapons.info [objP->id].flash_vclip > -1)
			ObjectCreateMuzzleFlash (objP->nSegment, &objP->pos, gameData.weapons.info [objP->id].flash_size, 
												gameData.weapons.info [objP->id].flash_vclip);
		}
	DoOmegaStuff (gameData.objs.objects + nParent, vPosition, objP);
	return nObject;
	}
if (gameData.objs.objects [nParent].nType == OBJ_PLAYER) {
	if (nWeaponType == FUSION_ID) {
		if (gameData.app.fusion.xCharge <= 0)
			objP->cType.laserInfo.multiplier = F1_0;
		else if (gameData.app.fusion.xCharge <= 4*F1_0)
			objP->cType.laserInfo.multiplier = F1_0 + gameData.app.fusion.xCharge/2;
		else
			objP->cType.laserInfo.multiplier = 4*F1_0;
		}
	else if (/* (nWeaponType >= LASER_ID) &&*/ (nWeaponType <= MAX_SUPER_LASER_LEVEL) && 
				(gameData.multi.players [gameData.objs.objects [nParent].id].flags & PLAYER_FLAGS_QUAD_LASERS))
		objP->cType.laserInfo.multiplier = F1_0*3/4;
	else if (nWeaponType == GUIDEDMISS_ID) {
		if (nParent==gameData.multi.players [gameData.multi.nLocalPlayer].nObject) {
			gameData.objs.guidedMissile [gameData.multi.nLocalPlayer]= objP;
			gameData.objs.guidedMissileSig [gameData.multi.nLocalPlayer] = objP->nSignature;
			if (gameData.demo.nState==ND_STATE_RECORDING)
				NDRecordGuidedStart ();
			}
		}
	}

//	Make children of smart bomb bounce so if they hit a wall right away, they
//	won't detonate.  The frame interval code will clear this bit after 1/2 second.
if ((nWeaponType == PLAYER_SMART_HOMING_ID) || 
	 (nWeaponType == SMART_MINE_HOMING_ID) || 
	 (nWeaponType == ROBOT_SMART_HOMING_ID) || 
	 (nWeaponType == ROBOT_SMART_MINE_HOMING_ID) || 
	 (nWeaponType == EARTHSHAKER_MEGA_ID))
	objP->mType.physInfo.flags |= PF_BOUNCE;
//CBRK (nWeaponType == ROBOT_MERCURY_ID);
if (gameData.weapons.info [nWeaponType].renderType == WEAPON_RENDER_POLYMODEL)
	xLaserLength = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad * 2;
if (nWeaponType == FLARE_ID)
	objP->mType.physInfo.flags |= PF_STICK;		//this obj sticks to walls
objP->shields = WI_strength (objP->id,gameStates.app.nDifficultyLevel);
// Fill in laser-specific data
objP->lifeleft							= WI_lifetime (objP->id);
objP->cType.laserInfo.parentType	= gameData.objs.objects [nParent].nType;
objP->cType.laserInfo.nParentSig = gameData.objs.objects [nParent].nSignature;
objP->cType.laserInfo.nParentObj	= nParent;
//	Assign nParent nType to highest level creator.  This propagates nParent nType down from
//	the original creator through weapons which create children of their own (ie, smart missile)
if (gameData.objs.objects [nParent].nType == OBJ_WEAPON) {
	int	nHighestParent = nParent;
	int	count = 0;
	while ((count++ < 10) && (gameData.objs.objects [nHighestParent].nType == OBJ_WEAPON)) {
		int	nNextParent = gameData.objs.objects [nHighestParent].cType.laserInfo.nParentObj;
		if (gameData.objs.objects [nNextParent].nSignature != gameData.objs.objects [nHighestParent].cType.laserInfo.nParentSig)
			break;	//	Probably means nParent was killed.  Just continue.
		if (nNextParent == nHighestParent) {
			Int3 ();	//	Hmm, tObject is nParent of itself.  This would seem to be bad, no?
			break;
			}
		nHighestParent = nNextParent;
		objP->cType.laserInfo.nParentObj	= nHighestParent;
		objP->cType.laserInfo.parentType	= gameData.objs.objects [nHighestParent].nType;
		objP->cType.laserInfo.nParentSig = gameData.objs.objects [nHighestParent].nSignature;
		}
	}
// Create orientation matrix so we can look from this pov
//	Homing missiles also need an orientation matrix so they know if they can make a turn.
//if ((objP->renderType == RT_POLYOBJ) || (WI_homingFlag (objP->id)))
	VmVector2Matrix (&objP->orient,vDirection, &gameData.objs.objects [nParent].orient.uVec ,NULL);
if (((gameData.objs.objects + nParent) != gameData.objs.viewer) && 
		(gameData.objs.objects [nParent].nType != OBJ_WEAPON))	{
	// Muzzle flash		
	if (gameData.weapons.info [objP->id].flash_vclip > -1)
		ObjectCreateMuzzleFlash (objP->nSegment, &objP->pos, gameData.weapons.info [objP->id].flash_size, 
											gameData.weapons.info [objP->id].flash_vclip);
	}
volume = F1_0;
if (bMakeSound && (gameData.weapons.info [objP->id].flashSound > -1))	{
	if (nParent != OBJ_IDX (gameData.objs.viewer))
		DigiLinkSoundToPos (gameData.weapons.info [objP->id].flashSound, objP->nSegment, 0, &objP->pos, 0, volume);
	else {
		if (nWeaponType == VULCAN_ID)	// Make your own vulcan gun  1/2 as loud.
			volume = F1_0 / 2;
		DigiPlaySample (gameData.weapons.info [objP->id].flashSound, volume);
		}
	}
//	Fire the laser from the gun tip so that the back end of the laser bolt is at the gun tip.
// Move 1 frame, so that the end-tip of the laser is touching the gun barrel.
// This also jitters the laser a bit so that it doesn't alias.
//	Don't do for weapons created by weapons.
if ((gameData.objs.objects [nParent].nType == OBJ_PLAYER) && (gameData.weapons.info [nWeaponType].renderType != WEAPON_RENDER_NONE) && (nWeaponType != FLARE_ID)) {
	vmsVector	vEndPos;
	int			nEndSeg;

	VmVecScaleAdd (&vEndPos, &objP->pos, vDirection, nLaserOffset+ (xLaserLength/2));
	nEndSeg = FindSegByPoint (&vEndPos, objP->nSegment);
	if (nEndSeg == objP->nSegment) 
		objP->pos = vEndPos;
	else if (nEndSeg != -1) {
		objP->pos = vEndPos;
		RelinkObject (OBJ_IDX (objP), nEndSeg);
		}
	}

//	Here's where to fix the problem with gameData.objs.objects which are moving backwards imparting higher velocity to their weaponfire.
//	Find out if moving backwards.
if ((nWeaponType != PROXIMITY_ID) && (nWeaponType != SUPERPROX_ID)) 
	xParentSpeed = 0;
else {
	xParentSpeed = VmVecMagQuick (&gameData.objs.objects [nParent].mType.physInfo.velocity);
	if (VmVecDot (&gameData.objs.objects [nParent].mType.physInfo.velocity, 
						&gameData.objs.objects [nParent].orient.fVec) < 0)
		xParentSpeed = -xParentSpeed;
	} 

xWeaponSpeed = WI_speed (objP->id, gameStates.app.nDifficultyLevel);
if (gameData.weapons.info [objP->id].speedvar != 128) {
	fix randval = F1_0 - ((d_rand () * gameData.weapons.info [objP->id].speedvar) >> 6);	//	Get a scale factor between speedvar% and 1.0.
	xWeaponSpeed = FixMul (xWeaponSpeed, randval);
	}
//	Ugly hack (too bad we're on a deadline), for homing missiles dropped by smart bomb, start them out slower.
if ((objP->id == PLAYER_SMART_HOMING_ID) || 
		(objP->id == SMART_MINE_HOMING_ID) || 
		(objP->id == ROBOT_SMART_HOMING_ID) || 
		(objP->id == ROBOT_SMART_MINE_HOMING_ID) || 
		(objP->id == EARTHSHAKER_MEGA_ID))
	xWeaponSpeed /= 4;
if (WI_thrust (objP->id) != 0)
	xWeaponSpeed /= 2;
/*test*/VmVecCopyScale (&objP->mType.physInfo.velocity, vDirection, (xWeaponSpeed + xParentSpeed));
//	Set thrust 
if (WI_thrust (nWeaponType) != 0) {
	objP->mType.physInfo.thrust = objP->mType.physInfo.velocity;
	VmVecScale (&objP->mType.physInfo.thrust, FixDiv (WI_thrust (objP->id), xWeaponSpeed + xParentSpeed));
	}
if ((objP->nType == OBJ_WEAPON) && (objP->id == FLARE_ID))
	objP->lifeleft += (d_rand () - 16384) << 2;		//	add in -2..2 seconds
return nObject;
}

//	-----------------------------------------------------------------------------------------------------------
//	Calls CreateNewLaser, but takes care of the tSegment and point computation for you.
int CreateNewLaserEasy (vmsVector * vDirection, vmsVector * vPosition, short parent, ubyte nWeaponType, int bMakeSound)
{
	fvi_query	fq;
	fvi_info		hit_data;
	tObject		*parentObjP = &gameData.objs.objects [parent];
	int			fate;

	//	Find tSegment containing laser fire vPosition.  If the robot is straddling a tSegment, the vPosition from
	//	which it fires may be in a different tSegment, which is bad news for FindVectorIntersection.  So, cast
	//	a ray from the tObject center (whose tSegment we know) to the laser vPosition.  Then, in the call to CreateNewLaser
	//	use the data returned from this call to FindVectorIntersection.
	//	Note that while FindVectorIntersection is pretty slow, it is not terribly slow if the destination point is
	//	in the same tSegment as the source point.

	fq.p0						= &parentObjP->pos;
	fq.startSeg				= parentObjP->nSegment;
	fq.p1						= vPosition;
	fq.rad					= 0;
	fq.thisObjNum			= OBJ_IDX (parentObjP);
	fq.ignoreObjList	= NULL;
	fq.flags					= FQ_TRANSWALL | FQ_CHECK_OBJS;		//what about trans walls???

	fate = FindVectorIntersection (&fq, &hit_data);
	if (fate != HIT_NONE  || hit_data.hit.nSegment==-1) {
		return -1;
	}

	return CreateNewLaser (vDirection, &hit_data.hit.vPoint, (short) hit_data.hit.nSegment, parent, nWeaponType, bMakeSound);

}

//	-----------------------------------------------------------------------------------------------------------
//	Determine if two gameData.objs.objects are on a line of sight.  If so, return true, else return false.
//	Calls fvi.
int ObjectToObjectVisibility (tObject *objP1, tObject *objP2, int transType)
{
	fvi_query	fq;
	fvi_info		hit_data;
	int			fate;

fq.p0					= &objP1->pos;
fq.startSeg			= objP1->nSegment;
fq.p1					= &objP2->pos;
fq.rad				= 0x10;
fq.thisObjNum		= OBJ_IDX (objP1);
fq.ignoreObjList	= NULL;
fq.flags				= transType;
fate = FindVectorIntersection (&fq, &hit_data);
if (fate == HIT_WALL)
	return 0;
else if (fate == HIT_NONE)
	return 1;
else
	Int3 ();		//	Contact Mike: Oops, what happened?  What is fate?
					// 2 = hit tObject (impossible), 3 = bad starting point (bad)
return 0;
}

fix	xMinTrackableDot = MIN_TRACKABLE_DOT;

//	-----------------------------------------------------------------------------------------------------------
//	Return true if weapon *tracker is able to track tObject gameData.objs.objects [nTrackGoal], else return false.
//	In order for the tObject to be trackable, it must be within a reasonable turning radius for the missile
//	and it must not be obstructed by a wall.
int object_is_trackable (int nTrackGoal, tObject *tracker, fix *xDot)
{
	vmsVector	vGoal;
	tObject		*objP;

if (nTrackGoal == -1)
	return 0;

if (gameData.app.nGameMode & GM_MULTI_COOP)
	return 0;
objP = gameData.objs.objects + nTrackGoal;
//	Don't track player if he's cloaked.
if ((nTrackGoal == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) && 
	 (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_CLOAKED))
	return 0;
//	Can't track AI tObject if he's cloaked.
if (objP->nType == OBJ_ROBOT) {
	if (objP->cType.aiInfo.CLOAKED)
		return 0;
	//	Your missiles don't track your escort.
	if (gameData.bots.pInfo [objP->id].companion && 
		 (tracker->cType.laserInfo.parentType == OBJ_PLAYER))
		return 0;
	}
VmVecSub (&vGoal, &objP->pos, &tracker->pos);
VmVecNormalizeQuick (&vGoal);
*xDot = VmVecDot (&vGoal, &tracker->orient.fVec);
if ((*xDot < xMinTrackableDot) && (*xDot > F1_0*9/10)) {
	VmVecNormalize (&vGoal);
	*xDot = VmVecDot (&vGoal, &tracker->orient.fVec);
	}

if (*xDot >= xMinTrackableDot) {
	//	xDot is in legal range, now see if tObject is visible
	return ObjectToObjectVisibility (tracker, objP, FQ_TRANSWALL);
	}
return 0;
}

//	--------------------------------------------------------------------------------------------
int call_find_homingObject_complete (tObject *tracker, vmsVector *curpos)
{
	if (gameData.app.nGameMode & GM_MULTI) {
		if (tracker->cType.laserInfo.parentType == OBJ_PLAYER) {
			//	It's fired by a player, so if robots present, track robot, else track player.
			if (gameData.app.nGameMode & GM_MULTI_COOP)
				return FindHomingObjectComplete (curpos, tracker, OBJ_ROBOT, -1);
			else
				return FindHomingObjectComplete (curpos, tracker, OBJ_PLAYER, OBJ_ROBOT);
		} else {
			int	goal2Type = -1;

			if (gameStates.app.cheats.bRobotsKillRobots)
				goal2Type = OBJ_ROBOT;
			Assert (tracker->cType.laserInfo.parentType == OBJ_ROBOT);
			return FindHomingObjectComplete (curpos, tracker, OBJ_PLAYER, goal2Type);
		}		
	} else
		return FindHomingObjectComplete (curpos, tracker, OBJ_ROBOT, -1);
}

//	--------------------------------------------------------------------------------------------
//	Find tObject to home in on.
//	Scan list of gameData.objs.objects rendered last frame, find one that satisfies function of nearness to center and distance.
int FindHomingObject (vmsVector *curpos, tObject *tracker)
{
	int	i;
	fix	max_dot = -F1_0*2;
	int	best_objnum = -1;

	//	Contact Mike: This is a bad and stupid thing.  Who called this routine with an illegal laser nType??
	Assert ((WI_homingFlag (tracker->id)) || (tracker->id == OMEGA_ID));

	//	Find an tObject to track based on game mode (eg, whether in network play) and who fired it.

	if (gameData.app.nGameMode & GM_MULTI)
		return call_find_homingObject_complete (tracker, curpos);
	else {
		int	cur_min_trackable_dot;

		cur_min_trackable_dot = MIN_TRACKABLE_DOT;
		if ((tracker->nType == OBJ_WEAPON) && (tracker->id == OMEGA_ID))
			cur_min_trackable_dot = OMEGA_MIN_TRACKABLE_DOT;

		//	Not in network mode.  If not fired by player, then track player.
		if (tracker->cType.laserInfo.nParentObj != gameData.multi.players [gameData.multi.nLocalPlayer].nObject) {
			if (!(gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_CLOAKED))
				best_objnum = OBJ_IDX (gameData.objs.console);
		} else {
			int	window_num = -1;
			fix	dist, max_trackableDist;

			//	Find the window which has the forward view.
			for (i=0; i<MAX_RENDERED_WINDOWS; i++)
				if (windowRenderedData [i].frame >= gameData.app.nFrameCount-1)
					if (windowRenderedData [i].viewer == gameData.objs.console)
						if (!windowRenderedData [i].rear_view) {
							window_num = i;
							break;
						}

			//	Couldn't find suitable view from this frame, so do complete search.
			if (window_num == -1) {
				return call_find_homingObject_complete (tracker, curpos);
			}

			max_trackableDist = MAX_TRACKABLE_DIST;
			if (tracker->id == OMEGA_ID)
				max_trackableDist = OMEGA_MAX_TRACKABLE_DIST;

			//	Not in network mode and fired by player.
			for (i=windowRenderedData [window_num].numObjects-1; i>=0; i--) {
				fix			dot; //, dist;
				vmsVector	vecToCurObj;
				int			nObject = windowRenderedData [window_num].renderedObjects [i];
				tObject		*curObjP = &gameData.objs.objects [nObject];

				if (nObject == gameData.multi.players [gameData.multi.nLocalPlayer].nObject)
					continue;

				//	Can't track AI tObject if he's cloaked.
				if (curObjP->nType == OBJ_ROBOT) {
					if (curObjP->cType.aiInfo.CLOAKED)
						continue;

					//	Your missiles don't track your escort.
					if (gameData.bots.pInfo [curObjP->id].companion)
						if (tracker->cType.laserInfo.parentType == OBJ_PLAYER)
							continue;
				}

				VmVecSub (&vecToCurObj, &curObjP->pos, curpos);
				dist = VmVecNormalizeQuick (&vecToCurObj);
				if (dist < max_trackableDist) {
					dot = VmVecDot (&vecToCurObj, &tracker->orient.fVec);

					//	Note: This uses the constant, not-scaled-by-frametime value, because it is only used
					//	to determine if an tObject is initially trackable.  FindHomingObject is called on subsequent
					//	frames to determine if the tObject remains trackable.
					if (dot > cur_min_trackable_dot) {
						if (dot > max_dot) {
							if (ObjectToObjectVisibility (tracker, &gameData.objs.objects [nObject], FQ_TRANSWALL)) {
								max_dot = dot;
								best_objnum = nObject;
							}
						}
					} else if (dot > F1_0 - (F1_0 - cur_min_trackable_dot)*2) {
						VmVecNormalize (&vecToCurObj);
						dot = VmVecDot (&vecToCurObj, &tracker->orient.fVec);
						if (dot > cur_min_trackable_dot) {
							if (dot > max_dot) {
								if (ObjectToObjectVisibility (tracker, &gameData.objs.objects [nObject], FQ_TRANSWALL)) {
									max_dot = dot;
									best_objnum = nObject;
								}
							}
						}
					}
				}
			}
		}
	}
return best_objnum;
}

//	--------------------------------------------------------------------------------------------
//	Find tObject to home in on.
//	Scan list of gameData.objs.objects rendered last frame, find one that satisfies function of nearness to center and distance.
//	Can track two kinds of gameData.objs.objects.  If you are only interested in one nType, set track_objType2 to NULL
//	Always track proximity bombs.  --MK, 06/14/95
//	Make homing gameData.objs.objects not track parent's prox bombs.
int FindHomingObjectComplete (vmsVector *curpos, tObject *tracker, int track_objType1, int track_objType2)
{
	int	nObject;
	fix	max_dot = -F1_0*2;
	int	best_objnum = -1;
	fix	max_trackableDist;
	fix	min_trackable_dot;

	//	Contact Mike: This is a bad and stupid thing.  Who called this routine with an illegal laser nType??
	Assert ((WI_homingFlag (tracker->id)) || (tracker->id == OMEGA_ID));

	max_trackableDist = MAX_TRACKABLE_DIST;
	min_trackable_dot = MIN_TRACKABLE_DOT;

	if (tracker->id == OMEGA_ID) {
		max_trackableDist = OMEGA_MAX_TRACKABLE_DIST;
		min_trackable_dot = OMEGA_MIN_TRACKABLE_DOT;
	}

	for (nObject=0; nObject<=gameData.objs.nLastObject; nObject++) {
		int			is_proximity = 0;
		fix			dot, dist;
		vmsVector	vecToCurObj;
		tObject		*curObjP = &gameData.objs.objects [nObject];

		if ((curObjP->nType != track_objType1) && (curObjP->nType != track_objType2))
		{
			if ((curObjP->nType == OBJ_WEAPON) && ((curObjP->id == PROXIMITY_ID) || (curObjP->id == SUPERPROX_ID))) {
				if (curObjP->cType.laserInfo.nParentSig != tracker->cType.laserInfo.nParentSig)
					is_proximity = 1;
				else
					continue;
			} else
				continue;
		}

		if (nObject == tracker->cType.laserInfo.nParentObj) // Don't track shooter
			continue;

		//	Don't track cloaked players.
		if (curObjP->nType == OBJ_PLAYER)
		{
			if (gameData.multi.players [curObjP->id].flags & PLAYER_FLAGS_CLOAKED)
				continue;
			// Don't track teammates in team games
			#ifdef NETWORK
			if ((gameData.app.nGameMode & GM_TEAM) && (gameData.objs.objects [tracker->cType.laserInfo.nParentObj].nType == OBJ_PLAYER) && (GetTeam (curObjP->id) == GetTeam (gameData.objs.objects [tracker->cType.laserInfo.nParentObj].id)))
				continue;
			#endif
		}

		//	Can't track AI tObject if he's cloaked.
		if (curObjP->nType == OBJ_ROBOT) {
			if (curObjP->cType.aiInfo.CLOAKED)
				continue;

			//	Your missiles don't track your escort.
			if (gameData.bots.pInfo [curObjP->id].companion)
				if (tracker->cType.laserInfo.parentType == OBJ_PLAYER)
					continue;
		}

		VmVecSub (&vecToCurObj, &curObjP->pos, curpos);
		dist = VmVecMagQuick (&vecToCurObj);

		if (dist < max_trackableDist) {
			VmVecNormalizeQuick (&vecToCurObj);
			dot = VmVecDot (&vecToCurObj, &tracker->orient.fVec);
			if (is_proximity)
				dot = ((dot << 3) + dot) >> 3;		//	I suspect Watcom would be too stupid to figure out the obvious...

			//	Note: This uses the constant, not-scaled-by-frametime value, because it is only used
			//	to determine if an tObject is initially trackable.  FindHomingObject is called on subsequent
			//	frames to determine if the tObject remains trackable.
			if (dot > min_trackable_dot) {
				if (dot > max_dot) {
					if (ObjectToObjectVisibility (tracker, &gameData.objs.objects [nObject], FQ_TRANSWALL)) {
						max_dot = dot;
						best_objnum = nObject;
					}
				}
			}
		}
	}
	return best_objnum;
}

//	------------------------------------------------------------------------------------------------------------
//	See if legal to keep tracking currently tracked tObject.  If not, see if another tObject is trackable.  If not, return -1,
//	else return tObject number of tracking tObject.
//	Computes and returns a fairly precise dot product.
int track_track_goal (int nTrackGoal, tObject *tracker, fix *dot)
{
	//	Every 8 frames for each tObject, scan all gameData.objs.objects.
	if (object_is_trackable (nTrackGoal, tracker, dot) && (((OBJ_IDX (tracker) ^ gameData.app.nFrameCount) % 8) != 0)) {
		return nTrackGoal;
	} else if (((OBJ_IDX (tracker) ^ gameData.app.nFrameCount) % 4) == 0) {
		int	rval = -2;

		//	If player fired missile, then search for an tObject, if not, then give up.
		if (gameData.objs.objects [tracker->cType.laserInfo.nParentObj].nType == OBJ_PLAYER) {
			int	goalType;

			if (nTrackGoal == -1) 
			{
				if (gameData.app.nGameMode & GM_MULTI)
				{
					if (gameData.app.nGameMode & GM_MULTI_COOP)
						rval = FindHomingObjectComplete (&tracker->pos, tracker, OBJ_ROBOT, -1);
					else if (gameData.app.nGameMode & GM_MULTI_ROBOTS)		//	Not cooperative, if robots, track either robot or player
						rval = FindHomingObjectComplete (&tracker->pos, tracker, OBJ_PLAYER, OBJ_ROBOT);
					else		//	Not cooperative and no robots, track only a player
						rval = FindHomingObjectComplete (&tracker->pos, tracker, OBJ_PLAYER, -1);
				}
				else
					rval = FindHomingObjectComplete (&tracker->pos, tracker, OBJ_PLAYER, OBJ_ROBOT);
			} 
			else 
			{
				goalType = gameData.objs.objects [tracker->cType.laserInfo.nTrackGoal].nType;
				if ((goalType == OBJ_PLAYER) || (goalType == OBJ_ROBOT))
					rval = FindHomingObjectComplete (&tracker->pos, tracker, goalType, -1);
				else
					rval = -1;
			}
		} 
		else {
			int	goalType, goal2Type = -1;

			if (gameStates.app.cheats.bRobotsKillRobots)
				goal2Type = OBJ_ROBOT;

			if (nTrackGoal == -1)
				rval = FindHomingObjectComplete (&tracker->pos, tracker, OBJ_PLAYER, goal2Type);
			else {
				goalType = gameData.objs.objects [tracker->cType.laserInfo.nTrackGoal].nType;
				rval = FindHomingObjectComplete (&tracker->pos, tracker, goalType, goal2Type);
			}
		}

		Assert (rval != -2);		//	This means it never got set which is bad! Contact Mike.
		return rval;
	}
	return -1;
}

//-------------- Initializes a laser after Fire is pressed -----------------

int LaserPlayerFireSpreadDelay (
	tObject *objP, 
	ubyte laserType, 
	int gun_num, 
	fix spreadr, 
	fix spreadu, 
	fix delayTime, 
	int bMakeSound, 
	int harmless)
{
	short			LaserSeg;
	int			Fate; 
	vmsVector	LaserPos, LaserDir, pnt;
	fvi_query	fq;
	fvi_info		hit_data;
	vmsVector	gun_point;
	vmsMatrix	m;
	int			nObject;
#if FULL_COCKPIT_OFFS
	int bLaserOffs = ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) && (OBJ_IDX (objP) == 
							gameData.multi.players [gameData.multi.nLocalPlayer].nObject));
#else
	int bLaserOffs = 0;
#endif
	CreateAwarenessEvent (objP, PA_WEAPON_WALL_COLLISION);
	// Find the initial vPosition of the laser
	pnt = gameData.pig.ship.player->gunPoints [gun_num];
	if (bLaserOffs)
		VmVecScaleInc (&pnt, &objP->orient.uVec, LASER_OFFS);
	VmCopyTransposeMatrix (&m, &objP->orient);
	VmVecRotate (&gun_point, &pnt, &m);
	memcpy (&m, &objP->orient, sizeof (vmsMatrix));
	VmVecAdd (&LaserPos, &objP->pos, &gun_point);
	//	If supposed to fire at a delayed time (delayTime), then move this point backwards.
	if (delayTime)
		VmVecScaleInc (&LaserPos, &m.fVec, -FixMul (delayTime, WI_speed (laserType,gameStates.app.nDifficultyLevel)));

//	DoMuzzleStuff (objP, &Pos);

	//--------------- Find LaserPos and LaserSeg ------------------
	fq.p0						= &objP->pos;
	fq.startSeg				= objP->nSegment;
	fq.p1						= &LaserPos;
	fq.rad					= 0x10;
	fq.thisObjNum			= OBJ_IDX (objP);
	fq.ignoreObjList	= NULL;
	fq.flags					= FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS;
	Fate = FindVectorIntersection (&fq, &hit_data);
	LaserSeg = hit_data.hit.nSegment;
	if (LaserSeg == -1) {	//some sort of annoying error
		return -1;
		}
	//SORT OF HACK... IF ABOVE WAS CORRECT THIS WOULDNT BE NECESSARY.
	if (VmVecDistQuick (&LaserPos, &objP->pos) > 0x50000) {
		return -1;
		}
	if (Fate==HIT_WALL)  {
		return -1;
		}
	if (Fate==HIT_OBJECT) {
//		if (gameData.objs.objects [hit_data.hitObject].nType == OBJ_ROBOT)
//			gameData.objs.objects [hit_data.hitObject].flags |= OF_SHOULD_BE_DEAD;
//		if (gameData.objs.objects [hit_data.hitObject].nType != OBJ_POWERUP)
//			return;		
	//as of 12/6/94, we don't care if the laser is stuck in an tObject. We
	//just fire away normally
		}

	//	Now, make laser spread out.
	LaserDir = m.fVec;
	if (spreadr || spreadu) {
		VmVecScaleInc (&LaserDir, &m.rVec, spreadr);
		VmVecScaleInc (&LaserDir, &m.uVec, spreadu);
	}
	if (bLaserOffs)
		VmVecScaleInc (&LaserDir, &m.uVec, LASER_OFFS);
	nObject = CreateNewLaser (&LaserDir, &LaserPos, LaserSeg, OBJ_IDX (objP), laserType, bMakeSound);
	//	Omega cannon is a hack, not surprisingly.  Don't want to do the rest of this stuff.
	if (laserType == OMEGA_ID)
		return -1;
	if (nObject == -1) {
		return -1;
		}
#ifdef NETWORK
	if (laserType==GUIDEDMISS_ID && multiData.bIsGuided) {
		gameData.objs.guidedMissile [objP->id]=gameData.objs.objects + nObject;
	}

	multiData.bIsGuided=0;
#endif

	if (laserType == CONCUSSION_ID ||
		 laserType == HOMING_ID ||
		 laserType == SMART_ID ||
		 laserType == MEGA_ID ||
		 laserType == FLASH_ID ||
		 //laserType == GUIDEDMISS_ID ||
		 //laserType == SUPERPROX_ID ||
		 laserType == MERCURY_ID ||
		 laserType == EARTHSHAKER_ID)
		if (gameData.objs.missileViewer == NULL && objP->id==gameData.multi.nLocalPlayer)
			gameData.objs.missileViewer = gameData.objs.objects + nObject;

	//	If this weapon is supposed to be silent, set that bit!
	if (!bMakeSound)
		gameData.objs.objects [nObject].flags |= OF_SILENT;

	//	If this weapon is supposed to be silent, set that bit!
	if (harmless)
		gameData.objs.objects [nObject].flags |= OF_HARMLESS;

	//	If the tObject firing the laser is the player, then indicate the laser tObject so robots can dodge.
	//	New by MK on 6/8/95, don't let robots evade proximity bombs, thereby decreasing uselessness of bombs.
	if ((objP == gameData.objs.console) && 
		 ((gameData.objs.objects [nObject].id != PROXIMITY_ID) && 
		 (gameData.objs.objects [nObject].id != SUPERPROX_ID)))
		gameStates.app.bPlayerFiredLaserThisFrame = nObject;

	if (gameData.weapons.info [laserType].homingFlag) {
		if (objP == gameData.objs.console) {
			gameData.objs.objects [nObject].cType.laserInfo.nTrackGoal = 
				FindHomingObject (&LaserPos, gameData.objs.objects + nObject);
			#ifdef NETWORK
			multiData.laser.nTrack = gameData.objs.objects [nObject].cType.laserInfo.nTrackGoal;
			#endif
		}
		#ifdef NETWORK
		else // Some other player shot the homing thing
		{
			Assert (gameData.app.nGameMode & GM_MULTI);
			gameData.objs.objects [nObject].cType.laserInfo.nTrackGoal = multiData.laser.nTrack;
		}
		#endif
	}
return nObject;
}

//	-----------------------------------------------------------------------------------------------------------
void CreateFlare (tObject *objP)
{
	fix	energy_usage;

	energy_usage = WI_energy_usage (FLARE_ID);

	if (gameStates.app.nDifficultyLevel < 2)
		energy_usage = FixMul (energy_usage, i2f (gameStates.app.nDifficultyLevel+2)/4);

//	MK, 11/04/95: Allowed to fire flare even if no energy.
// -- 	if (gameData.multi.players [gameData.multi.nLocalPlayer].energy >= energy_usage) {
		gameData.multi.players [gameData.multi.nLocalPlayer].energy -= energy_usage;

		if (gameData.multi.players [gameData.multi.nLocalPlayer].energy <= 0) {
			gameData.multi.players [gameData.multi.nLocalPlayer].energy = 0;	
			// -- AutoSelectWeapon (0);
		}

		LaserPlayerFire (objP, FLARE_ID, 6, 1, 0);

		#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI) {
			multiData.laser.bFired = 1;
			multiData.laser.nGun = FLARE_ADJUST;
			multiData.laser.nFlags = 0;
			multiData.laser.nLevel = 0;
		}
		#endif
// -- 	}

}

//-------------------------------------------------------------------------------------------
//	Set tObject *objP's orientation to (or towards if I'm ambitious) its velocity.
void HomingMissileTurnTowardsVelocity (tObject *objP, vmsVector *norm_vel)
{
	vmsVector	new_fvec;
	fix frameTime;

if (!gameStates.limitFPS.bHomers || gameOpts->legacy.bHomers)
	frameTime = gameData.time.xFrame;
else {
	if (!gameStates.app.b40fpsTick)
		return;
	else {
		int fps = (int) ((1000 + gameStates.app.nDeltaTime / 2) / gameStates.app.nDeltaTime);
		frameTime = fps ? (f1_0 + fps / 2) / fps : f1_0;
		}
	}
new_fvec = *norm_vel;
VmVecScale (&new_fvec, /*gameData.time.xFrame*/ frameTime * HOMING_MISSILE_SCALE);
VmVecInc (&new_fvec, &objP->orient.fVec);
VmVecNormalizeQuick (&new_fvec);
//	if ((norm_vel->x == 0) && (norm_vel->y == 0) && (norm_vel->z == 0))
//		return;
VmVector2Matrix (&objP->orient, &new_fvec, NULL, NULL);
}

//-------------------------------------------------------------------------------------------
//sequence this laser tObject for this _frame_ (underscores added here to aid MK in his searching!)
void LaserDoWeaponSequence (tObject *objP)
{
	tObject *gmObjP;
	Assert (objP->controlType == CT_WEAPON);

	//	Ok, this is a big hack by MK.
	//	If you want an tObject to last for exactly one frame, then give it a lifeleft of ONE_FRAME_TIME
	if (objP->lifeleft == ONE_FRAME_TIME) {
		if (gameData.app.nGameMode & GM_MULTI)
			objP->lifeleft = OMEGA_MULTI_LIFELEFT;
		else
			objP->lifeleft = 0;
		objP->renderType = RT_NONE;
	}

	if (objP->lifeleft < 0) {		// We died of old age
		objP->flags |= OF_SHOULD_BE_DEAD;
		if (WI_damage_radius (objP->id))
			ExplodeBadassWeapon (objP,&objP->pos);
		return;
	}

	//delete weapons that are not moving
	if (	!((gameData.app.nFrameCount ^ objP->nSignature) & 3) &&
			 (objP->id != FLARE_ID) &&
			 (gameData.weapons.info [objP->id].speed [gameStates.app.nDifficultyLevel] > 0) &&
			 (VmVecMagQuick (&objP->mType.physInfo.velocity) < F2_0)) {
		ReleaseObject (OBJ_IDX (objP));
		return;
	}

	if (objP->id == FUSION_ID) {		//always set fusion weapon to max vel

		VmVecNormalizeQuick (&objP->mType.physInfo.velocity);

		VmVecScale (&objP->mType.physInfo.velocity, WI_speed (objP->id,gameStates.app.nDifficultyLevel));
	}

// -- 	//	The Super Spreadfire (Helix) blobs travel in a sinusoidal path.  That is accomplished
// -- 	//	by modifying velocity (vDirection) in the frame interval.
// -- 	if (objP->id == SSPREADFIRE_ID) {
// -- 		fix	age, sinval, cosval;
// -- 		vmsVector	p, newp;
// -- 		fix	speed;
// -- 
// -- 		speed = VmVecMagQuick (&objP->physInfo.velocity);
// -- 
// -- 		age = gameData.weapons.info [objP->id].lifetime - objP->lifeleft;
// -- 
// -- 		fix_fast_sincos (age, &sinval, &cosval);
// -- 
// -- 		//	Note: Below code assumes x=1, y=0.  Need to scale this for values around a circle for 5 helix positions.
// -- 		p.x = cosval << 3;
// -- 		p.y = sinval << 3;
// -- 		p.z = 0;
// -- 
// -- 		VmVecRotate (&newp, &p, &objP->orient);
// -- 
// -- 		VmVecAdd (&goal_point, &objP->pos, &newp);
// -- 
// -- 		VmVecSub (&vGoal, &goal_point, obj
// -- 	}


	//	For homing missiles, turn towards target. (unless it's the guided missile)
	if (WI_homingFlag (objP->id) && 
	    !(objP->id==GUIDEDMISS_ID && 
		  (objP == (gmObjP = gameData.objs.guidedMissile [gameData.objs.objects [objP->cType.laserInfo.nParentObj].id])) && 
		   objP->nSignature==gmObjP->nSignature)) {
		vmsVector		vector_toObject, temp_vec;
		fix				dot=F1_0;
		fix				speed, xMaxSpeed;

		//	For first 1/2 second of life, missile flies straight.
		if (objP->cType.laserInfo.creationTime + HOMING_MISSILE_STRAIGHT_TIME < gameData.time.xGame) {

			int	nTrackGoal = objP->cType.laserInfo.nTrackGoal;
			int	id = objP->id;

			//	If it's time to do tracking, then it's time to grow up, stop bouncing and start exploding!.
			if ((id == ROBOT_SMART_MINE_HOMING_ID) || 
				 (id == ROBOT_SMART_HOMING_ID) || 
				 (id == SMART_MINE_HOMING_ID) || 
				 (id == PLAYER_SMART_HOMING_ID) || 
				 (id == EARTHSHAKER_MEGA_ID)) {
				objP->mType.physInfo.flags &= ~PF_BOUNCE;
			}

			//	Make sure the tObject we are tracking is still trackable.
			nTrackGoal = track_track_goal (nTrackGoal, objP, &dot);
			if (nTrackGoal == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) {
				fix	dist_to_player;

				dist_to_player = VmVecDistQuick (&objP->pos, &gameData.objs.objects [nTrackGoal].pos);
				if ((dist_to_player < gameData.multi.players [gameData.multi.nLocalPlayer].homingObjectDist) || (gameData.multi.players [gameData.multi.nLocalPlayer].homingObjectDist < 0))
					gameData.multi.players [gameData.multi.nLocalPlayer].homingObjectDist = dist_to_player;
					
			}

			if (nTrackGoal != -1) {
				VmVecSub (&vector_toObject, &gameData.objs.objects [nTrackGoal].pos, &objP->pos);

				VmVecNormalizeQuick (&vector_toObject);
				temp_vec = objP->mType.physInfo.velocity;
				speed = VmVecNormalizeQuick (&temp_vec);
				xMaxSpeed = WI_speed (objP->id,gameStates.app.nDifficultyLevel);
				if (speed+F1_0 < xMaxSpeed) {
					speed += FixMul (xMaxSpeed, gameData.time.xFrame/2);
					if (speed > xMaxSpeed)
						speed = xMaxSpeed;
				}

				// -- dot = VmVecDot (&temp_vec, &vector_toObject);
				VmVecInc (&temp_vec, &vector_toObject);
				//	The boss' smart children track better...
				if (gameData.weapons.info [objP->id].renderType != WEAPON_RENDER_POLYMODEL)
					VmVecInc (&temp_vec, &vector_toObject);
				VmVecNormalizeQuick (&temp_vec);
				objP->mType.physInfo.velocity = temp_vec;
				VmVecScale (&objP->mType.physInfo.velocity, speed);

				//	Subtract off life proportional to amount turned.
				//	For hardest turn, it will lose 2 seconds per second.
				{
					fix	lifelost, absdot;
				
					absdot = abs (F1_0 - dot);
				
					lifelost = FixMul (absdot*32, gameData.time.xFrame);
					objP->lifeleft -= lifelost;
				}

				//	Only polygon gameData.objs.objects have visible orientation, so only they should turn.
				if (gameData.weapons.info [objP->id].renderType == WEAPON_RENDER_POLYMODEL)
					HomingMissileTurnTowardsVelocity (objP, &temp_vec);		//	temp_vec is normalized velocity.
			}
		}
	}

	//	Make sure weapon is not moving faster than allowed speed.
	{
		fix	xWeaponSpeed;

		xWeaponSpeed = VmVecMagQuick (&objP->mType.physInfo.velocity);
		if (xWeaponSpeed > WI_speed (objP->id,gameStates.app.nDifficultyLevel)) {
			//	Only slow down if not allowed to move.  Makes sense, huh?  Allows proxbombs to get moved by physics force. --MK, 2/13/96
			if (WI_speed (objP->id,gameStates.app.nDifficultyLevel)) {
				fix	scale_factor;

				scale_factor = FixDiv (WI_speed (objP->id,gameStates.app.nDifficultyLevel), xWeaponSpeed);
				VmVecScale (&objP->mType.physInfo.velocity, scale_factor);
			}
		}
	}
}

fix	Last_laser_firedTime = 0;

int	Zbonkers = 0;

//	--------------------------------------------------------------------------------------------------
// Assumption: This is only called by the actual console player, not for network players

int LaserFireLocalPlayer (void)
{
	player	*playerP = gameData.multi.players + gameData.multi.nLocalPlayer;
	fix		xEnergyUsed;
	int		nAmmoUsed,nPrimaryAmmo;
	int		nWeaponIndex;
	int		rval = 0;
	int 		nfires = 1;
	fix		addval;
	static int nSpreadfireToggle = 0;
	static int nHelixOrient = 0;

if (gameStates.app.bPlayerIsDead)
	return 0;
if (gameStates.app.bD2XLevel && (gameData.segs.segment2s [gameData.objs.objects [playerP->nObject].nSegment].special == SEGMENT_IS_NODAMAGE))
	return 0;
nWeaponIndex = primaryWeaponToWeaponInfo [gameData.weapons.nPrimary];
xEnergyUsed = WI_energy_usage (nWeaponIndex);
if (gameData.weapons.nPrimary == OMEGA_INDEX)
	xEnergyUsed = 0;	//	Omega consumes energy when recharging, not when firing.
if (gameStates.app.nDifficultyLevel < 2)
	xEnergyUsed = FixMul (xEnergyUsed, i2f (gameStates.app.nDifficultyLevel+2)/4);
//	MK, 01/26/96, Helix use 2x energy in multiplayer.  bitmaps.tbl parm should have been reduced for single player.
if (nWeaponIndex == HELIX_INDEX)
	if (gameData.app.nGameMode & GM_MULTI)
		xEnergyUsed *= 2;
nAmmoUsed = WI_ammo_usage (nWeaponIndex);
addval = 2*gameData.time.xFrame;
if (addval > F1_0)
	addval = F1_0;
if ((Last_laser_firedTime + 2 * gameData.time.xFrame < gameData.time.xGame) || (gameData.time.xGame < Last_laser_firedTime))
	xNextLaserFireTime = gameData.time.xGame;
Last_laser_firedTime = gameData.time.xGame;
nPrimaryAmmo = (gameData.weapons.nPrimary == GAUSS_INDEX)? (playerP->primaryAmmo [VULCAN_INDEX]): (playerP->primaryAmmo [gameData.weapons.nPrimary]);
if	 (!((playerP->energy >= xEnergyUsed) && (nPrimaryAmmo >= nAmmoUsed)))
	AutoSelectWeapon (0, 1);		//	Make sure the player can fire from this weapon.
if (Zbonkers) {
	Zbonkers = 0;
	gameData.time.xGame = 0;
	}

while (xNextLaserFireTime <= gameData.time.xGame) {
	if	((playerP->energy >= xEnergyUsed) && (nPrimaryAmmo >= nAmmoUsed)) {
		int	nLaserLevel, flags = 0;
		if (gameStates.app.cheats.bLaserRapidFire == 0xBADA55)
			xNextLaserFireTime += F1_0/25;
		else
			xNextLaserFireTime += WI_fire_wait (nWeaponIndex);
		nLaserLevel = gameData.multi.players [gameData.multi.nLocalPlayer].laserLevel;
		if (gameData.weapons.nPrimary == SPREADFIRE_INDEX) {
			if (nSpreadfireToggle)
				flags |= LASER_SPREADFIRE_TOGGLED;
			nSpreadfireToggle = !nSpreadfireToggle;
			}
		if (gameData.weapons.nPrimary == HELIX_INDEX) {
			nHelixOrient++;
			flags |= ((nHelixOrient & LASER_HELIX_MASK) << LASER_HELIX_SHIFT);
			}
		if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_QUAD_LASERS)
			flags |= LASER_QUAD;
		rval += LaserFireObject ((short) gameData.multi.players [gameData.multi.nLocalPlayer].nObject, (ubyte) gameData.weapons.nPrimary, nLaserLevel, flags, nfires);
		playerP->energy -= (xEnergyUsed * rval) / gameData.weapons.info [nWeaponIndex].fire_count;
		if (playerP->energy < 0)
			playerP->energy = 0;
		if ((gameData.weapons.nPrimary == VULCAN_INDEX) || (gameData.weapons.nPrimary == GAUSS_INDEX)) {
			if (nAmmoUsed > playerP->primaryAmmo [VULCAN_INDEX])
				playerP->primaryAmmo [VULCAN_INDEX] = 0;
			else
				playerP->primaryAmmo [VULCAN_INDEX] -= nAmmoUsed;
			}
		AutoSelectWeapon (0, 1);		//	Make sure the player can fire from this weapon.
		}
	else {
		AutoSelectWeapon (0, 1);		//	Make sure the player can fire from this weapon.
		xNextLaserFireTime = gameData.time.xGame;	//	Prevents shots-to-fire from building up.
		break;	//	Couldn't fire weapon, so abort.
		}
	}
gameData.app.nGlobalLaserFiringCount = 0;	
return rval;
}

// -- #define	MAX_LIGHTNING_DISTANCE	 (F1_0*300)
// -- #define	MAX_LIGHTNING_BLOBS		16
// -- #define	LIGHTNING_BLOB_DISTANCE	 (MAX_LIGHTNING_DISTANCE/MAX_LIGHTNING_BLOBS)
// -- 
// -- #define	LIGHTNING_BLOB_ID			13
// -- 
// -- #define	LIGHTNING_TIME		 (F1_0/4)
// -- #define	LIGHTNING_DELAY	 (F1_0/8)
// -- 
// -- int	Lightning_gun_num = 1;
// -- 
// -- fix	Lightning_startTime = -F1_0*10, Lightning_lastTime;
// -- 
// -- //	--------------------------------------------------------------------------------------------------
// -- //	Return -1 if failed to create at least one blob.  Else return index of last blob created.
// -- int create_lightning_blobs (vmsVector *vDirection, vmsVector *start_pos, int start_segnum, int parent)
// -- {
// -- 	int			i;
// -- 	fvi_query	fq;
// -- 	fvi_info		hit_data;
// -- 	vmsVector	vEndPos;
// -- 	vmsVector	norm_dir;
// -- 	int			fate;
// -- 	int			num_blobs;
// -- 	vmsVector	tvec;
// -- 	fix			dist_to_hit_point;
// -- 	vmsVector	point_pos, delta_pos;
// -- 	int			nObject;
// -- 	vmsVector	*gun_pos;
// -- 	vmsMatrix	m;
// -- 	vmsVector	gun_pos2;
// -- 
// -- 	if (gameData.multi.players [gameData.multi.nLocalPlayer].energy > F1_0)
// -- 		gameData.multi.players [gameData.multi.nLocalPlayer].energy -= F1_0;
// -- 
// -- 	if (gameData.multi.players [gameData.multi.nLocalPlayer].energy <= F1_0) {
// -- 		gameData.multi.players [gameData.multi.nLocalPlayer].energy = 0;	
// -- 		AutoSelectWeapon (0);
// -- 		return -1;
// -- 	}
// -- 
// -- 	norm_dir = *vDirection;
// -- 
// -- 	VmVecNormalizeQuick (&norm_dir);
// -- 	VmVecScaleAdd (&vEndPos, start_pos, &norm_dir, MAX_LIGHTNING_DISTANCE);
// -- 
// -- 	fq.p0						= start_pos;
// -- 	fq.startSeg				= start_segnum;
// -- 	fq.p1						= &vEndPos;
// -- 	fq.rad					= 0;
// -- 	fq.thisObjNum			= parent;
// -- 	fq.ignoreObjList	= NULL;
// -- 	fq.flags					= FQ_TRANSWALL | FQ_CHECK_OBJS;
// -- 
// -- 	fate = FindVectorIntersection (&fq, &hit_data);
// -- 	if (hit_data.hit.nSegment == -1) {
// -- 		return -1;
// -- 	}
// -- 
// -- 	dist_to_hit_point = VmVecMag (VmVecSub (&tvec, &hit_data.hit.vPoint, start_pos);
// -- 	num_blobs = dist_to_hit_point/LIGHTNING_BLOB_DISTANCE;
// -- 
// -- 	if (num_blobs > MAX_LIGHTNING_BLOBS)
// -- 		num_blobs = MAX_LIGHTNING_BLOBS;
// -- 
// -- 	if (num_blobs < MAX_LIGHTNING_BLOBS/4)
// -- 		num_blobs = MAX_LIGHTNING_BLOBS/4;
// -- 
// -- 	// Find the initial vPosition of the laser
// -- 	gun_pos = &gameData.pig.ship.player->gunPoints [Lightning_gun_num];
// -- 	VmCopyTransposeMatrix (&m,&gameData.objs.objects [parent].orient);
// -- 	VmVecRotate (&gun_pos2, gun_pos, &m);
// -- 	VmVecAdd (&point_pos, &gameData.objs.objects [parent].pos, &gun_pos2);
// -- 
// -- 	delta_pos = norm_dir;
// -- 	VmVecScale (&delta_pos, dist_to_hit_point/num_blobs);
// -- 
// -- 	for (i=0; i<num_blobs; i++) {
// -- 		int			tPointSeg;
// -- 		tObject		*obj;
// -- 
// -- 		VmVecInc (&point_pos, &delta_pos);
// -- 		tPointSeg = FindSegByPoint (&point_pos, start_segnum);
// -- 		if (tPointSeg == -1)	//	Hey, we thought we were creating points on a line, but we left the mine!
// -- 			continue;
// -- 
// -- 		nObject = CreateNewLaser (vDirection, &point_pos, tPointSeg, parent, LIGHTNING_BLOB_ID, 0);
// -- 
// -- 		if (nObject < 0) 	{
// -- 			Int3 ();
// -- 			return -1;
// -- 		}
// -- 
// -- 		obj = &gameData.objs.objects [nObject];
// -- 
// -- 		DigiPlaySample (gameData.weapons.info [objP->id].flashSound, F1_0);
// -- 
// -- 		// -- VmVecScale (&objP->mType.physInfo.velocity, F1_0/2);
// -- 
// -- 		objP->lifeleft = (LIGHTNING_TIME + LIGHTNING_DELAY)/2;
// -- 
// -- 	}
// -- 
// -- 	return nObject;
// -- 
// -- }
// -- 
// -- //	--------------------------------------------------------------------------------------------------
// -- //	Lightning Cannon.
// -- //	While being fired, creates path of blobs forward from player until it hits something.
// -- //	Up to MAX_LIGHTNING_BLOBS blobs, spaced LIGHTNING_BLOB_DISTANCE units apart.
// -- //	When the player releases the firing key, the blobs move forward.
// -- void lightning_frame (void)
// -- {
// -- 	if ((gameData.time.xGame - Lightning_startTime < LIGHTNING_TIME) && (gameData.time.xGame - Lightning_startTime > 0)) {
// -- 		if (gameData.time.xGame - Lightning_lastTime > LIGHTNING_DELAY) {
// -- 			create_lightning_blobs (&gameData.objs.console->orient.fVec, &gameData.objs.console->pos, gameData.objs.console->nSegment, OBJ_IDX (gameData.objs.console));
// -- 			Lightning_lastTime = gameData.time.xGame;
// -- 		}
// -- 	}
// -- }

//	--------------------------------------------------------------------------------------------------
//	Object "nObject" fires weapon "weapon_num" of level "level". (Right now (9/24/94) level is used only for nType 0 laser.
//	Flags are the player flags.  For network mode, set to 0.
//	It is assumed that this is a player tObject (as in multiplayer), and therefore the gun positions are known.
//	Returns number of times a weapon was fired.  This is typically 1, but might be more for low frame rates.
//	More than one shot is fired with a pseudo-delay so that players on show machines can fire (for themselves
//	or other players) often enough for things like the vulcan cannon.
int LaserFireObject (short nObject, ubyte nWeapon, int level, int flags, int nFires)
{
	tObject	*objP = gameData.objs.objects + nObject;

	switch (nWeapon) {
		case LASER_INDEX: {
			ubyte	nLaser;
			nLaserOffset = ((F1_0*2)* (d_rand ()%8))/8;
			if (level <= MAX_LASER_LEVEL)
				nLaser = LASER_ID + level;
			else
				nLaser = SUPER_LASER_ID + (level - MAX_LASER_LEVEL-1);
			LaserPlayerFire (objP, nLaser, 0, 1, 0);
			LaserPlayerFire (objP, nLaser, 1, 0, 0);
			if (flags & LASER_QUAD) {
				//	hideous system to make quad laser 1.5x powerful as normal laser, make every other quad laser bolt harmless
				LaserPlayerFire (objP, nLaser, 2, 0, 0);
				LaserPlayerFire (objP, nLaser, 3, 0, 0);
			}
			break;
		}
		case VULCAN_INDEX: {
			//	Only make sound for 1/4 of vulcan bullets.
			int	bMakeSound = 1;
			//if (d_rand () > 24576)
			//	bMakeSound = 1;
			LaserPlayerFireSpread (objP, VULCAN_ID, 6, d_rand ()/8 - 32767/16, d_rand ()/8 - 32767/16, bMakeSound, 0);
			if (nFires > 1) {
				LaserPlayerFireSpread (objP, VULCAN_ID, 6, d_rand ()/8 - 32767/16, d_rand ()/8 - 32767/16, 0, 0);
				if (nFires > 2) {
					LaserPlayerFireSpread (objP, VULCAN_ID, 6, d_rand ()/8 - 32767/16, d_rand ()/8 - 32767/16, 0, 0);
				}
			}
			break;
		}
		case SPREADFIRE_INDEX:
			if (flags & LASER_SPREADFIRE_TOGGLED) {
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, F1_0/16, 0, 0, 0);
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, -F1_0/16, 0, 0, 0);
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, 0, 1, 0);
			} else {
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, F1_0/16, 0, 0);
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, -F1_0/16, 0, 0);
				LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, 0, 1, 0);
			}
			break;

		case PLASMA_INDEX:
			LaserPlayerFire (objP, PLASMA_ID, 0, 1, 0);
			LaserPlayerFire (objP, PLASMA_ID, 1, 0, 0);
			if (nFires > 1) {
				LaserPlayerFireSpreadDelay (objP, PLASMA_ID, 0, 0, 0, gameData.time.xFrame/2, 1, 0);
				LaserPlayerFireSpreadDelay (objP, PLASMA_ID, 1, 0, 0, gameData.time.xFrame/2, 0, 0);
			}
			break;

		case FUSION_INDEX: {
			vmsVector	vForce;

			LaserPlayerFire (objP, FUSION_ID, 0, 1, 0);
			LaserPlayerFire (objP, FUSION_ID, 1, 1, 0);

			flags = (sbyte) (gameData.app.fusion.xCharge >> 12);

			gameData.app.fusion.xCharge = 0;

			vForce.x = - (objP->orient.fVec.x << 7);
			vForce.y = - (objP->orient.fVec.y << 7);
			vForce.z = - (objP->orient.fVec.z << 7);
			PhysApplyForce (objP, &vForce);

			vForce.x = (vForce.x >> 4) + d_rand () - 16384;
			vForce.y = (vForce.y >> 4) + d_rand () - 16384;
			vForce.z = (vForce.z >> 4) + d_rand () - 16384;
			PhysApplyRot (objP, &vForce);

		}
			break;
		case SUPER_LASER_INDEX: {
			ubyte superLevel = 3;		//make some new kind of laser eventually
			LaserPlayerFire (objP, superLevel, 0, 1, 0);
			LaserPlayerFire (objP, superLevel, 1, 0, 0);

			if (flags & LASER_QUAD) {
				//	hideous system to make quad laser 1.5x powerful as normal laser, make every other quad laser bolt harmless
				LaserPlayerFire (objP, superLevel, 2, 0, 0);
				LaserPlayerFire (objP, superLevel, 3, 0, 0);
			}
			break;
		}
		case GAUSS_INDEX: {
			//	Only make sound for 1/4 of vulcan bullets.
			int	bMakeSound = 1;
			//if (d_rand () > 24576)
			//	bMakeSound = 1;
			
			LaserPlayerFireSpread (objP, GAUSS_ID, 6, (d_rand ()/8 - 32767/16)/5, (d_rand ()/8 - 32767/16)/5, bMakeSound, 0);
			if (nFires > 1) {
				LaserPlayerFireSpread (objP, GAUSS_ID, 6, (d_rand ()/8 - 32767/16)/5, (d_rand ()/8 - 32767/16)/5, 0, 0);
				if (nFires > 2) {
					LaserPlayerFireSpread (objP, GAUSS_ID, 6, (d_rand ()/8 - 32767/16)/5, (d_rand ()/8 - 32767/16)/5, 0, 0);
				}
			}
			break;
		}
		case HELIX_INDEX: {
			int helix_orient;
			fix spreadr,spreadu;
			helix_orient = (flags >> LASER_HELIX_SHIFT) & LASER_HELIX_MASK;
			switch (helix_orient) {

				case 0: spreadr =  F1_0/16; spreadu = 0;       break; // Vertical
				case 1: spreadr =  F1_0/17; spreadu = F1_0/42; break; //  22.5 degrees
				case 2: spreadr =  F1_0/22; spreadu = F1_0/22; break; //  45   degrees
				case 3: spreadr =  F1_0/42; spreadu = F1_0/17; break; //  67.5 degrees
				case 4: spreadr =  0;       spreadu = F1_0/16; break; //  90   degrees
				case 5: spreadr = -F1_0/42; spreadu = F1_0/17; break; // 112.5 degrees
				case 6: spreadr = -F1_0/22; spreadu = F1_0/22; break; // 135   degrees	
				case 7: spreadr = -F1_0/17; spreadu = F1_0/42; break; // 157.5 degrees
				default:
					Error ("Invalid helix_orientation value %x\n",helix_orient);
			}

			LaserPlayerFireSpread (objP, HELIX_ID, 6,  0,  0, 1, 0);
			LaserPlayerFireSpread (objP, HELIX_ID, 6,  spreadr,  spreadu, 0, 0);
			LaserPlayerFireSpread (objP, HELIX_ID, 6, -spreadr, -spreadu, 0, 0);
			LaserPlayerFireSpread (objP, HELIX_ID, 6,  spreadr*2,  spreadu*2, 0, 0);
			LaserPlayerFireSpread (objP, HELIX_ID, 6, -spreadr*2, -spreadu*2, 0, 0);
			break;
		}

		case PHOENIX_INDEX:
			LaserPlayerFire (objP, PHOENIX_ID, 0, 1, 0);
			LaserPlayerFire (objP, PHOENIX_ID, 1, 0, 0);
			if (nFires > 1) {
				LaserPlayerFireSpreadDelay (objP, PHOENIX_ID, 0, 0, 0, gameData.time.xFrame/2, 1, 0);
				LaserPlayerFireSpreadDelay (objP, PHOENIX_ID, 1, 0, 0, gameData.time.xFrame/2, 0, 0);
			}
			break;

		case OMEGA_INDEX:
			LaserPlayerFire (objP, OMEGA_ID, 1, 1, 0);
			break;

		default:
			Int3 ();	//	Contact Yuan: Unknown Primary weapon nType, setting to 0.
			gameData.weapons.nPrimary = 0;
	}

	// Set values to be recognized during comunication phase, if we are the
	//  one shooting
#ifdef NETWORK
	if ((gameData.app.nGameMode & GM_MULTI) && (nObject == gameData.multi.players [gameData.multi.nLocalPlayer].nObject))
	{
		multiData.laser.bFired = nFires;
		multiData.laser.nGun = nWeapon;
		multiData.laser.nFlags = flags;
		multiData.laser.nLevel = level;
	}
#endif

	return nFires;
}

//	-------------------------------------------------------------------------------------------
//	if nGoalObj == -1, then create random vector
int CreateHomingMissile (tObject *objP, int nGoalObj, ubyte objtype, int bMakeSound)
{
	short			nObject;
	vmsVector	vGoal;
	vmsVector	random_vector;
	//vmsVector	vGoalPos;

	if (nGoalObj == -1) {
		MakeRandomVector (&vGoal);
	} else {
		VmVecNormalizedDirQuick (&vGoal, &gameData.objs.objects [nGoalObj].pos, &objP->pos);
		MakeRandomVector (&random_vector);
		VmVecScaleInc (&vGoal, &random_vector, F1_0/4);
		VmVecNormalizeQuick (&vGoal);
	}		

	//	Create a vector towards the goal, then add some noise to it.
	nObject = CreateNewLaser (&vGoal, &objP->pos, objP->nSegment, 
									  OBJ_IDX (objP), objtype, bMakeSound);
	if (nObject == -1)
		return -1;

	// Fixed to make sure the right person gets credit for the kill

//	gameData.objs.objects [nObject].cType.laserInfo.nParentObj = objP->cType.laserInfo.nParentObj;
//	gameData.objs.objects [nObject].cType.laserInfo.parentType = objP->cType.laserInfo.parentType;
//	gameData.objs.objects [nObject].cType.laserInfo.nParentSig = objP->cType.laserInfo.nParentSig;

	gameData.objs.objects [nObject].cType.laserInfo.nTrackGoal = nGoalObj;

	return nObject;
}

extern void BlastNearbyGlass (tObject *objP, fix damage);

//-----------------------------------------------------------------------------
// Create the children of a smart bomb, which is a bunch of homing missiles.
void CreateSmartChildren (tObject *objP, int num_smart_children)
{
	int	parentType, nParentObj;
	int	bMakeSound;
	int	numobjs=0;
	int	objlist [MAX_OBJDISTS];
	ubyte	blob_id;

	if (objP->nType == OBJ_WEAPON) {
		parentType = objP->cType.laserInfo.parentType;
		nParentObj = objP->cType.laserInfo.nParentObj;
	} else if (objP->nType == OBJ_ROBOT) {
		parentType = OBJ_ROBOT;
		nParentObj = OBJ_IDX (objP);
	} else {
		Int3 ();	//	Hey, what kind of tObject is this!?
		parentType = 0;
		nParentObj = 0;
	}

	if (objP->id == EARTHSHAKER_ID)
		BlastNearbyGlass (objP, gameData.weapons.info [EARTHSHAKER_ID].strength [gameStates.app.nDifficultyLevel]);

// -- DEBUG --
	if ((objP->nType == OBJ_WEAPON) && ((objP->id == SMART_ID) || (objP->id == SUPERPROX_ID) || (objP->id == ROBOT_SUPERPROX_ID) || (objP->id == EARTHSHAKER_ID)))
		Assert (gameData.weapons.info [objP->id].children != -1);
// -- DEBUG --

	if (((objP->nType == OBJ_WEAPON) && (gameData.weapons.info [objP->id].children != -1)) || (objP->nType == OBJ_ROBOT)) {
		int	i, nObject;

		if (gameData.app.nGameMode & GM_MULTI)
			d_srand (8321L);

		for (nObject=0; nObject<=gameData.objs.nLastObject; nObject++) {
			tObject	*curObjP = &gameData.objs.objects [nObject];

			if ((((curObjP->nType == OBJ_ROBOT) && (!curObjP->cType.aiInfo.CLOAKED)) || (curObjP->nType == OBJ_PLAYER)) && (nObject != nParentObj)) {
				fix	dist;

				if (curObjP->nType == OBJ_PLAYER)
				{
					if ((parentType == OBJ_PLAYER) && (gameData.app.nGameMode & GM_MULTI_COOP))
						continue;
#ifdef NETWORK
					if ((gameData.app.nGameMode & GM_TEAM) && (GetTeam (curObjP->id) == GetTeam (gameData.objs.objects [nParentObj].id)))
						continue;
#endif

					if (gameData.multi.players [curObjP->id].flags & PLAYER_FLAGS_CLOAKED)
						continue;
				}

				//	Robot blobs can't track robots.
				if (curObjP->nType == OBJ_ROBOT) {
					if (parentType == OBJ_ROBOT)
						continue;

					//	Your shots won't track the buddy.
					if (parentType == OBJ_PLAYER)
						if (gameData.bots.pInfo [curObjP->id].companion)
							continue;
				}

				dist = VmVecDistQuick (&objP->pos, &curObjP->pos);
				if (dist < MAX_SMART_DISTANCE) {
					int	oovis;

					oovis = ObjectToObjectVisibility (objP, curObjP, FQ_TRANSWALL);

					if (oovis) { //ObjectToObjectVisibility (objP, curObjP, FQ_TRANSWALL)) {
						objlist [numobjs] = nObject;
						numobjs++;
						if (numobjs >= MAX_OBJDISTS) {
							numobjs = MAX_OBJDISTS;
							break;
						}
					}
				}
			}
		}

		//	Get nType of weapon for child from parent.
		if (objP->nType == OBJ_WEAPON) {
			blob_id = gameData.weapons.info [objP->id].children;
			Assert (blob_id != -1);		//	Hmm, missing data in bitmaps.tbl.  Need "children=NN" parameter.
		} else {
			Assert (objP->nType == OBJ_ROBOT);
			blob_id = ROBOT_SMART_HOMING_ID;
		}

// -- 		//determine what kind of blob to drop
// -- 		//	Note: parentType is not the nType of the weapon's parent.  It is actually the nType of the weapon's
// -- 		//	earliest ancestor.  This deals with the issue of weapons spewing weapons which spew weapons.
// -- 		switch (parentType) {
// -- 			case OBJ_WEAPON:
// -- 				Int3 ();	//	Should this ever happen?
// -- 				switch (objP->id) {
// -- 					case SUPERPROX_ID:			blob_id = SMART_MINE_HOMING_ID; break;
// -- 					case ROBOT_SUPERPROX_ID:	blob_id = ROBOT_SMART_MINE_HOMING_ID; break;
// -- 					case EARTHSHAKER_ID:			blob_id = EARTHSHAKER_MEGA_ID; break;
// -- 					default:							Int3 ();	//bogus id for weapon  
// -- 				}
// -- 				break;
// -- 			case OBJ_PLAYER:
// -- 				switch (objP->id) {
// -- 					case SUPERPROX_ID:			blob_id = SMART_MINE_HOMING_ID; break;
// -- 					case ROBOT_SUPERPROX_ID:	Int3 ();	break;
// -- 					case EARTHSHAKER_ID:			blob_id = EARTHSHAKER_MEGA_ID; break;
// -- 					case SMART_ID:					blob_id = PLAYER_SMART_HOMING_ID; break;
// -- 					default:							Int3 ();	//bogus id for weapon  
// -- 				}
// -- 				break;
// -- 			case OBJ_ROBOT:
// -- 				switch (objP->id) {
// -- 					case ROBOT_SUPERPROX_ID:	blob_id = ROBOT_SMART_MINE_HOMING_ID; break;
// -- 					// -- case EARTHSHAKER_ID:			blob_id = EARTHSHAKER_MEGA_ID; break;
// -- 					case SMART_ID:					blob_id = ROBOT_SMART_HOMING_ID; break;
// -- 					default:							blob_id = ROBOT_SMART_HOMING_ID; break;
// -- 				}
// -- 				break;
// -- 			default:					Int3 ();	//bogus nType for parent tObject
// -- 		}

		bMakeSound = 1;
		for (i=0; i<num_smart_children; i++) {
			short nObject = (numobjs==0)?-1:objlist [ (d_rand () * numobjs) >> 15];
			CreateHomingMissile (objP, nObject, blob_id, bMakeSound);
			bMakeSound = 0;
		}
	}
}

//	-------------------------------------------------------------------------------------------

int nMissileGun = 0;

//give up control of the guided missile
void ReleaseGuidedMissile (int player_num)
{
	if (player_num == gameData.multi.nLocalPlayer)
	 {			
	  if (gameData.objs.guidedMissile [player_num]==NULL)
			return;
	
		gameData.objs.missileViewer = gameData.objs.guidedMissile [player_num];
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
		 MultiSendGuidedInfo (gameData.objs.guidedMissile [gameData.multi.nLocalPlayer],1);
#endif
		if (gameData.demo.nState==ND_STATE_RECORDING)
		 NDRecordGuidedEnd ();
	 }	

	gameData.objs.guidedMissile [player_num] = NULL;
}

int nProximityDropped=0,nSmartminesDropped=0;

//	-------------------------------------------------------------------------------------------
//parameter determines whether or not to do autoselect if have run out of ammo
//this is needed because if you drop a bomb with the B key, you don't 
//want to autoselect if the bomb isn't actually selected. 
void DoMissileFiring (int bAutoSelect)
{
	int		h, i, gunFlag = 0;
	short		nObject;
	ubyte		nWeaponId;
	int		nWeaponGun;
	tObject	*gmP = gameData.objs.guidedMissile [gameData.multi.nLocalPlayer];
	player	*playerP = gameData.multi.players + gameData.multi.nLocalPlayer;

Assert (gameData.weapons.nSecondary < MAX_SECONDARY_WEAPONS);
if (gmP && (gmP->nSignature == gameData.objs.guidedMissileSig [gameData.multi.nLocalPlayer])) {
	ReleaseGuidedMissile (gameData.multi.nLocalPlayer);
	i = secondaryWeaponToWeaponInfo [gameData.weapons.nSecondary];
	xNextMissileFireTime = gameData.time.xGame + WI_fire_wait (i);
	return;
	}

if (gameStates.app.bPlayerIsDead || (playerP->secondaryAmmo [gameData.weapons.nSecondary] <= 0))
	return;

nWeaponId = secondaryWeaponToWeaponInfo [gameData.weapons.nSecondary];
if (gameStates.app.cheats.bLaserRapidFire != 0xBADA55)
	xNextMissileFireTime = gameData.time.xGame + WI_fire_wait (nWeaponId);
else
	xNextMissileFireTime = gameData.time.xGame + F1_0/25;
nWeaponGun = secondaryWeaponToGunNum [gameData.weapons.nSecondary];
h = (EGI_FLAG (bDualMissileLaunch, 1, 0)) ? 1 : 0;
for (i = 0; (i <= h) && (playerP->secondaryAmmo [gameData.weapons.nSecondary] > 0); i++) {
	playerP->secondaryAmmo [gameData.weapons.nSecondary]--;
	if (IsMultiGame)
		MultiSendWeapons (1);
	if (nWeaponGun == 4) {		//alternate left/right
		nWeaponGun += (gunFlag = (nMissileGun & 1));
		nMissileGun++;
		}
	nObject = LaserPlayerFire (gameData.objs.console, nWeaponId, nWeaponGun, 1, 0);
	if (gameData.weapons.nSecondary == PROXIMITY_INDEX) {
		if (!(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY))) {
			if (++nProximityDropped == 4) {
				nProximityDropped = 0;
#ifdef NETWORK
				MaybeDropNetPowerup (nObject, POW_PROXIMITY_WEAPON, INIT_DROP);
#endif
				}
			}
		break; //no dual prox bomb drop
		}
	else if (gameData.weapons.nSecondary == SMART_MINE_INDEX) {
		if (!(gameData.app.nGameMode & GM_ENTROPY)) {
			if (++nSmartminesDropped == 4) {
				nSmartminesDropped = 0;
#ifdef NETWORK
				MaybeDropNetPowerup (nObject, POW_SMART_MINE, INIT_DROP);
#endif
				}
			}
		break; //no dual smartmine drop
		}
#ifdef NETWORK
	else if (gameData.weapons.nSecondary != CONCUSSION_INDEX)
		MaybeDropNetPowerup (nObject, secondaryWeaponToPowerup [gameData.weapons.nSecondary], INIT_DROP);
#endif
	if ((gameData.weapons.nSecondary == GUIDED_INDEX) || (gameData.weapons.nSecondary == SMART_INDEX))
		break;
	else if ((gameData.weapons.nSecondary == MEGA_INDEX) || (gameData.weapons.nSecondary == SMISSILE5_INDEX)) {
		vmsVector vForce;

	vForce.x = - (gameData.objs.console->orient.fVec.x << 7);
	vForce.y = - (gameData.objs.console->orient.fVec.y << 7);
	vForce.z = - (gameData.objs.console->orient.fVec.z << 7);
	PhysApplyForce (gameData.objs.console, &vForce);
	vForce.x = (vForce.x >> 4) + d_rand () - 16384;
	vForce.y = (vForce.y >> 4) + d_rand () - 16384;
	vForce.z = (vForce.z >> 4) + d_rand () - 16384;
	PhysApplyRot (gameData.objs.console, &vForce);
	break; //no dual mega/smart missile launch
	}
}

#ifdef NETWORK
if (gameData.app.nGameMode & GM_MULTI) {
	multiData.laser.bFired = 1;		//how many
	multiData.laser.nGun = gameData.weapons.nSecondary + MISSILE_ADJUST;
	multiData.laser.nFlags = gunFlag;
	multiData.laser.nLevel = 0;
	}
#endif
if (bAutoSelect)
	AutoSelectWeapon (1, 1);		//select next missile, if this one out of ammo
}

//	-------------------------------------------------------------------------------------------
// eof
