/* $Id: ai2.c,v 1.4 2003/10/04 03:14:47 btb Exp $ */
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

#ifdef RCS
static char rcsid [] = "$Id: ai2.c,v 1.4 2003/10/04 03:14:47 btb Exp $";
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "inferno.h"
#include "game.h"
#include "mono.h"
#include "3d.h"

#include "u_mem.h"
#include "object.h"
#include "render.h"
#include "error.h"
#include "ai.h"
#include "laser.h"
#include "fvi.h"
#include "polyobj.h"
#include "bm.h"
#include "weapon.h"
#include "physics.h"
#include "collide.h"
#include "player.h"
#include "wall.h"
#include "vclip.h"
#include "digi.h"
#include "fireball.h"
#include "morph.h"
#include "effects.h"
#include "timer.h"
#include "sounds.h"
#include "reactor.h"
#include "multibot.h"
#include "multi.h"
#include "network.h"
#include "loadgame.h"
#include "key.h"
#include "powerup.h"
#include "gauges.h"
#include "text.h"
#include "gameseg.h"
#include "escort.h"

#ifdef EDITOR
#include "editor/editor.h"
#include "editor/kdefs.h"
#endif
//#define _DEBUG
#ifdef _DEBUG
#include "string.h"
#include <time.h>
#endif

#define	FIRE_AT_NEARBY_PLAYER_THRESHOLD	 (F1_0 * 40) //(F1_0*40)

// ----------------------------------------------------------------------------
// Return firing status.
// If ready to fire a weapon, return true, else return false.
// Ready to fire a weapon if nextPrimaryFire <= 0 or nextSecondaryFire <= 0.
int ReadyToFire (tRobotInfo *botInfoP, tAILocal *ailP)
{
return (ailP->nextPrimaryFire <= 0) || ((botInfoP->nSecWeaponType != -1) && (ailP->nextSecondaryFire <= 0));
}

// ----------------------------------------------------------------------------------
void SetNextFireTime (tObject *objP, tAILocal *ailP, tRobotInfo *botInfoP, int nGun)
{
	//	For guys in snipe mode, they have a 50% shot of getting this shot in D2_FREE.
if ((nGun != 0) || (botInfoP->nSecWeaponType == -1))
	if ((objP->cType.aiInfo.behavior != AIB_SNIPE) || (d_rand () > 16384))
		ailP->nRapidFireCount++;
if (((nGun != 0) || (botInfoP->nSecWeaponType == -1)) && (ailP->nRapidFireCount < botInfoP->nRapidFireCount [gameStates.app.nDifficultyLevel])) {
	ailP->nextPrimaryFire = min (F1_0/8, botInfoP->primaryFiringWait [gameStates.app.nDifficultyLevel]/2);
	}
else {
	if ((botInfoP->nSecWeaponType == -1) || (nGun != 0)) {
		ailP->nextPrimaryFire = botInfoP->primaryFiringWait [gameStates.app.nDifficultyLevel];
		if (ailP->nRapidFireCount >= botInfoP->nRapidFireCount [gameStates.app.nDifficultyLevel])
			ailP->nRapidFireCount = 0;
		}
	else
		ailP->nextSecondaryFire = botInfoP->secondaryFiringWait [gameStates.app.nDifficultyLevel];
	}
}

// ----------------------------------------------------------------------------------
//	When some robots collide with the tPlayer, they attack.
//	If tPlayer is cloaked, then robot probably didn't actually collide, deal with that here.
void DoAiRobotHitAttack (tObject *robot, tObject *playerobjP, vmsVector *vCollision)
{
	tAILocal		*ailP = gameData.ai.localInfo + OBJ_IDX (robot);
	tRobotInfo	*botInfoP = &ROBOTINFO (robot->id);

if (!gameStates.app.cheats.bRobotsFiring)
	return;
//	If tPlayer is dead, stop firing.
if (gameData.objs.objects [LOCALPLAYER.nObject].nType == OBJ_GHOST)
	return;
if (botInfoP->attackType != 1)
	return;
if (ailP->nextPrimaryFire > 0)
	return;
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED)) {
	if (VmVecDistQuick (&OBJPOS (gameData.objs.console)->vPos, &robot->position.vPos) < 
		 robot->size + gameData.objs.console->size + F1_0 * 2) {
		CollidePlayerAndNastyRobot (playerobjP, robot, vCollision);
		if (botInfoP->energyDrain && LOCALPLAYER.energy) {
			LOCALPLAYER.energy -= botInfoP->energyDrain * F1_0;
			if (LOCALPLAYER.energy < 0)
				LOCALPLAYER.energy = 0;
			}
		}
	}
robot->cType.aiInfo.GOAL_STATE = AIS_RECOVER;
SetNextFireTime (robot, ailP, botInfoP, 1);	//	1 = nGun: 0 is special (uses nextSecondaryFire)
}

#define	FIRE_K	8		//	Controls average accuracy of robot firing.  Smaller numbers make firing worse.  Being power of 2 doesn't matter.

// ====================================================================================================================

#define	MIN_LEAD_SPEED		 (F1_0*4)
#define	MAX_LEAD_DISTANCE	 (F1_0*200)
#define	LEAD_RANGE			 (F1_0/2)

// --------------------------------------------------------------------------------------------------------------------
//	Computes point at which projectile fired by robot can hit tPlayer given positions, tPlayer vel, elapsed time
inline fix ComputeLeadComponent (fix player_pos, fix robot_pos, fix player_vel, fix elapsedTime)
{
return FixDiv (player_pos - robot_pos, elapsedTime) + player_vel;
}

// --------------------------------------------------------------------------------------------------------------------
//	Lead the tPlayer, returning point to fire at in vFirePoint.
//	Rules:
//		Player not cloaked
//		Player must be moving at a speed >= MIN_LEAD_SPEED
//		Player not farther away than MAX_LEAD_DISTANCE
//		dot (vector_to_player, player_direction) must be in -LEAD_RANGE,LEAD_RANGE
//		if firing a matter weapon, less leading, based on skill level.
int LeadPlayer (tObject *objP, vmsVector *vFirePoint, vmsVector *vBelievedPlayerPos, int nGun, vmsVector *fire_vec)
{
	fix			dot, player_speed, xDistToPlayer, max_weapon_speed, projectedTime;
	vmsVector	player_movement_dir, vVecToPlayer;
	int			nWeaponType;
	tWeaponInfo	*wptr;
	tRobotInfo	*botInfoP;

	if (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED)
		return 0;

	player_movement_dir = gameData.objs.console->mType.physInfo.velocity;
	player_speed = VmVecNormalize (&player_movement_dir);

	if (player_speed < MIN_LEAD_SPEED)
		return 0;

	VmVecSub (&vVecToPlayer, vBelievedPlayerPos, vFirePoint);
	xDistToPlayer = VmVecNormalize (&vVecToPlayer);
	if (xDistToPlayer > MAX_LEAD_DISTANCE)
		return 0;

	dot = VmVecDot (&vVecToPlayer, &player_movement_dir);

	if ((dot < -LEAD_RANGE) || (dot > LEAD_RANGE))
		return 0;

	//	Looks like it might be worth trying to lead the player.
	botInfoP = &ROBOTINFO (objP->id);
	nWeaponType = botInfoP->nWeaponType;
	if (botInfoP->nSecWeaponType != -1)
		if (nGun == 0)
			nWeaponType = botInfoP->nSecWeaponType;

	wptr = gameData.weapons.info + nWeaponType;
	max_weapon_speed = wptr->speed [gameStates.app.nDifficultyLevel];
	if (max_weapon_speed < F1_0)
		return 0;

	//	Matter weapons:
	//	At Rookie or Trainee, don't lead at all.
	//	At higher skill levels, don't lead as well.  Accomplish this by screwing up max_weapon_speed.
	if (wptr->matter)
	{
		if (gameStates.app.nDifficultyLevel <= 1)
			return 0;
		else
			max_weapon_speed *= (NDL-gameStates.app.nDifficultyLevel);
	}

	projectedTime = FixDiv (xDistToPlayer, max_weapon_speed);

	fire_vec->p.x = ComputeLeadComponent (vBelievedPlayerPos->p.x, vFirePoint->p.x, gameData.objs.console->mType.physInfo.velocity.p.x, projectedTime);
	fire_vec->p.y = ComputeLeadComponent (vBelievedPlayerPos->p.y, vFirePoint->p.y, gameData.objs.console->mType.physInfo.velocity.p.y, projectedTime);
	fire_vec->p.z = ComputeLeadComponent (vBelievedPlayerPos->p.z, vFirePoint->p.z, gameData.objs.console->mType.physInfo.velocity.p.z, projectedTime);

	VmVecNormalize (fire_vec);

	Assert (VmVecDot (fire_vec, &objP->position.mOrient.fVec) < 3*F1_0/2);

	//	Make sure not firing at especially strange angle.  If so, try to correct.  If still bad, give up after one try.
	if (VmVecDot (fire_vec, &objP->position.mOrient.fVec) < F1_0/2) {
		VmVecInc (fire_vec, &vVecToPlayer);
		VmVecScale (fire_vec, F1_0/2);
		if (VmVecDot (fire_vec, &objP->position.mOrient.fVec) < F1_0/2) {
			return 0;
		}
	}

	return 1;
}

// --------------------------------------------------------------------------------------------------------------------

void AICreateClusterLight (tObject *objP, short nObject, short nShot)
{
if (!gameStates.render.bClusterLights)
	return;
	
short nPrevShot = gameData.objs.shots [nObject].nObject;

#ifdef _DEBUG
if (nObject == nDbgObj)
	nObject = nDbgObj;
#endif
if (nPrevShot >= 0) {
	tObject *prevShotP = OBJECTS + nPrevShot;
	if (prevShotP->nSignature == gameData.objs.shots [nObject].nSignature) {
		tObject *lightP, *shotP = OBJECTS + nShot;
		short nLight = gameData.objs.lightObjs [nPrevShot].nObject;
		if (nLight < 0)
			lightP = prevShotP;
		else {
			lightP = OBJECTS + nLight;
			if (lightP->nSignature != gameData.objs.lightObjs [nPrevShot].nSignature) {
				lightP = prevShotP;
				nLight = -1;
				}
			}
		if (VmVecDist (&shotP->position.vPos, &lightP->position.vPos) < 10 * F1_0) {
			if (nLight >= 0) {
				gameData.objs.lightObjs [nShot].nObject = nLight;
				lightP->cType.lightInfo.nObjects++;
				}
			else {
				nLight = CreateClusterLight (prevShotP);
				gameData.objs.lightObjs [nShot].nObject =
				gameData.objs.lightObjs [nPrevShot].nObject = nLight;
				if (nLight >= 0) {
#ifdef _DEBUG
					HUDMessage (0, "new robot shot light cluster");
#endif
					lightP = OBJECTS + nLight;
					gameData.objs.lightObjs [nShot].nSignature =
					gameData.objs.lightObjs [nPrevShot].nSignature = lightP->nSignature;
					lightP->cType.lightInfo.nObjects = 2;
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
//	Note: Parameter gameData.ai.vVecToPlayer is only passed now because guns which aren't on the forward vector from the
//	center of the robot will not fire right at the player.  We need to aim the guns at the player.  Barring that, we cheat.
//	When this routine is complete, the parameter gameData.ai.vVecToPlayer should not be necessary.
void AIFireLaserAtPlayer (tObject *objP, vmsVector *vFirePoint, int nGun, vmsVector *vBelievedPlayerPos)
{
	short			nShot, nObject = OBJ_IDX (objP);
	tAILocal		*ailP = gameData.ai.localInfo + nObject;
	tRobotInfo	*botInfoP = &ROBOTINFO (objP->id);
	vmsVector	fire_vec;
	vmsVector	bpp_diff;
	short			nWeaponType;
	fix			aim, dot;
	int			count, i;

Assert (nObject >= 0);
//	If this robot is only awake because a camera woke it up, don't fire.
if (objP->cType.aiInfo.SUB_FLAGS & SUB_FLAGS_CAMERA_AWAKE)
	return;
if (!gameStates.app.cheats.bRobotsFiring)
	return;
if (objP->controlType == CT_MORPH)
	return;
//	If player is exploded, stop firing.
if (gameStates.app.bPlayerExploded)
	return;
if (objP->cType.aiInfo.xDyingStartTime)
	return;		//	No firing while in death roll.
//	Don't let the boss fire while in death roll.  Sorry, this is the easiest way to do this.
//	If you try to key the boss off objP->cType.aiInfo.xDyingStartTime, it will hose the endlevel stuff.
if (ROBOTINFO (objP->id).bossFlag) {
	i = FindBoss (nObject);
	if (gameData.boss [i].nDyingStartTime)
		return;
	}
//	If tPlayer is cloaked, maybe don't fire based on how long cloaked and randomness.
if (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) {
	fix	xCloakTime = gameData.ai.cloakInfo [nObject % MAX_AI_CLOAK_INFO].lastTime;
	if ((gameData.time.xGame - xCloakTime > CLOAK_TIME_MAX/4) &&
		 (d_rand () > FixDiv (gameData.time.xGame - xCloakTime, CLOAK_TIME_MAX)/2)) {
		SetNextFireTime (objP, ailP, botInfoP, nGun);
		return;
		}
	}
//	Handle problem of a robot firing through a tWall because its gun tip is on the other
//	tSide of the tWall than the robot's center.  For speed reasons, we normally only compute
//	the vector from the gun point to the player.  But we need to know whether the gun point
//	is separated from the robot's center by a tWall.  If so, don't fire!
if (objP->cType.aiInfo.SUB_FLAGS & SUB_FLAGS_GUNSEG) {
	//	Well, the gun point is in a different tSegment than the robot's center.
	//	This is almost always ok, but it is not ok if something solid is in between.
	int	nGunSeg = FindSegByPoint (vFirePoint, objP->nSegment, 1, 0);
	//	See if these segments are connected, which should almost always be the case.
	short nConnSide = FindConnectedSide (&gameData.segs.segments [nGunSeg], &gameData.segs.segments [objP->nSegment]);
	if (nConnSide != -1) {
		//	They are connected via nConnSide in tSegment objP->nSegment.
		//	See if they are unobstructed.
		if (!(WALL_IS_DOORWAY (gameData.segs.segments + objP->nSegment, nConnSide, NULL) & WID_FLY_FLAG)) {
			//	Can't fly through, so don't let this bot fire through!
			return;
			}
		}
	else {
		//	Well, they are not directly connected, so use FindVectorIntersection to see if they are unobstructed.
		tVFIQuery	fq;
		tFVIData		hit_data;
		int			fate;

		fq.startSeg			= objP->nSegment;
		fq.p0					= &objP->position.vPos;
		fq.p1					= vFirePoint;
		fq.radP0				= 
		fq.radP1				= 0;
		fq.thisObjNum		= OBJ_IDX (objP);
		fq.ignoreObjList	= NULL;
		fq.flags				= FQ_TRANSWALL;

		fate = FindVectorIntersection (&fq, &hit_data);
		if (fate != HIT_NONE) {
			Int3 ();		//	This bot's gun is poking through a tWall, so don't fire.
			MoveTowardsSegmentCenter (objP);		//	And decrease chances it will happen again.
			return;
			}
		}
	}
//	Set position to fire at based on difficulty level and robot's aiming ability
aim = FIRE_K*F1_0 - (FIRE_K-1)* (botInfoP->aim << 8);	//	F1_0 in bitmaps.tbl = same as used to be.  Worst is 50% more error.
//	Robots aim more poorly during seismic disturbance.
if (gameStates.gameplay.seismic.nMagnitude) {
	fix temp = F1_0 - abs (gameStates.gameplay.seismic.nMagnitude);
	if (temp < F1_0/2)
		temp = F1_0/2;
	aim = FixMul (aim, temp);
	}
//	Lead the tPlayer half the time.
//	Note that when leading the tPlayer, aim is perfect.  This is probably acceptable since leading is so hacked in.
//	Problem is all robots will lead equally badly.
if (d_rand () < 16384) {
	if (LeadPlayer (objP, vFirePoint, vBelievedPlayerPos, nGun, &fire_vec))		//	Stuff direction to fire at in vFirePoint.
		goto player_led;
}

dot = 0;
count = 0;			//	Don't want to sit in this loop foreverd:\temp\dm_test.
i = (NDL - gameStates.app.nDifficultyLevel - 1) * 4;
while ((count < 4) && (dot < F1_0/4)) {
	bpp_diff.p.x = vBelievedPlayerPos->p.x + FixMul ((d_rand ()-16384) * i, aim);
	bpp_diff.p.y = vBelievedPlayerPos->p.y + FixMul ((d_rand ()-16384) * i, aim);
	bpp_diff.p.z = vBelievedPlayerPos->p.z + FixMul ((d_rand ()-16384) * i, aim);
	VmVecNormalizedDir (&fire_vec, &bpp_diff, vFirePoint);
	dot = VmVecDot (&objP->position.mOrient.fVec, &fire_vec);
	count++;
	}

player_led:

nWeaponType = botInfoP->nWeaponType;
if ((botInfoP->nSecWeaponType != -1) && ((nWeaponType < 0) || !nGun))
	nWeaponType = botInfoP->nSecWeaponType;
if (nWeaponType < 0)
	return;
if (0 > (nShot = CreateNewLaserEasy (&fire_vec, vFirePoint, OBJ_IDX (objP), (ubyte) nWeaponType, 1)))
	return;

AICreateClusterLight (objP, nObject, nShot);
gameData.objs.shots [nObject].nObject = nShot;
gameData.objs.shots [nObject].nSignature = OBJECTS [nShot].nSignature;

if (IsMultiGame) {
	AIMultiSendRobotPos (nObject, -1);
	MultiSendRobotFire (nObject, objP->cType.aiInfo.CURRENT_GUN, &fire_vec);
	}
#if 1
if (++(objP->cType.aiInfo.CURRENT_GUN) >= botInfoP->nGuns) {
	if ((botInfoP->nGuns == 1) || (botInfoP->nSecWeaponType == -1))
		objP->cType.aiInfo.CURRENT_GUN = 0;
	else
		objP->cType.aiInfo.CURRENT_GUN = 1;
	}
#endif
CreateAwarenessEvent (objP, PA_NEARBY_ROBOT_FIRED);
SetNextFireTime (objP, ailP, botInfoP, nGun);
}

//	-------------------------------------------------------------------------------------------------------------------

void DoFiringStuff (tObject *objP, int nPlayerVisibility, vmsVector *vVecToPlayer)
{
if ((gameData.ai.nDistToLastPlayerPosFiredAt < FIRE_AT_NEARBY_PLAYER_THRESHOLD) || 
	 (gameData.ai.nPlayerVisibility >= 1)) {
	//	Now, if in robot's field of view, lock onto tPlayer
	fix	dot = VmVecDot (&objP->position.mOrient.fVec, &gameData.ai.vVecToPlayer);
	if ((dot >= 7 * F1_0 / 8) || (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED)) {
		tAIStatic	*aiP = &objP->cType.aiInfo;
		tAILocal		*ailP = gameData.ai.localInfo + OBJ_IDX (objP);

		switch (aiP->GOAL_STATE) {
			case AIS_NONE:
			case AIS_REST:
			case AIS_SEARCH:
			case AIS_LOCK:
				aiP->GOAL_STATE = AIS_FIRE;
				if (ailP->playerAwarenessType <= PA_NEARBY_ROBOT_FIRED) {
					ailP->playerAwarenessType = PA_NEARBY_ROBOT_FIRED;
					ailP->playerAwarenessTime = PLAYER_AWARENESS_INITIAL_TIME;
					}
				break;
			}
		} 
	else if (dot >= F1_0/2) {
		tAIStatic	*aiP = &objP->cType.aiInfo;
		switch (aiP->GOAL_STATE) {
			case AIS_NONE:
			case AIS_REST:
			case AIS_SEARCH:
				aiP->GOAL_STATE = AIS_LOCK;
				break;
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
//	If a hiding robot gets bumped or hit, he decides to find another hiding place.
void DoAiRobotHit (tObject *objP, int nType)
{
	int	r;

if (objP->controlType != CT_AI)
	return;
if ((nType != PA_WEAPON_ROBOT_COLLISION) && (nType != PA_PLAYER_COLLISION))
	return;
if (objP->cType.aiInfo.behavior != AIB_STILL)
	return;
r = d_rand ();
//	Attack robots (eg, green guy) shouldn't have behavior = still.
//Assert (ROBOTINFO (objP->id).attackType == 0);
//	1/8 time, charge tPlayer, 1/4 time create path, rest of time, do nothing
if (r < 4096) {
	CreatePathToPlayer (objP, 10, 1);
	objP->cType.aiInfo.behavior = AIB_STATION;
	objP->cType.aiInfo.nHideSegment = objP->nSegment;
	gameData.ai.localInfo [OBJ_IDX (objP)].mode = AIM_CHASE_OBJECT;
	}
else if (r < 4096 + 8192) {
	CreateNSegmentPath (objP, d_rand () / 8192 + 2, -1);
	gameData.ai.localInfo [OBJ_IDX (objP)].mode = AIM_FOLLOW_PATH;
	}
}

#ifdef _DEBUG
int	bDoAIFlag=1;
int	Cvv_test=0;
int	Cvv_lastTime [MAX_OBJECTS_D2X];
int	Gun_point_hack=0;
#endif

// --------------------------------------------------------------------------------------------------------------------
//	Returns true if this tObject should be allowed to fire at the player.
int AIMaybeDoActualFiringStuff (tObject *objP, tAIStatic *aiP)
{
if (IsMultiGame &&
	 (aiP->GOAL_STATE != AIS_FLINCH) && (objP->id != ROBOT_BRAIN) &&
	 (aiP->CURRENT_STATE == AIS_FIRE))
	return 1;
return 0;
}

// --------------------------------------------------------------------------------------------------------------------
//	If fire_anyway, fire even if tPlayer is not visible.  We're firing near where we believe him to be.  Perhaps he's
//	lurking behind a corner.
void AIDoActualFiringStuff (tObject *objP, tAIStatic *aiP, tAILocal *ailP, tRobotInfo *botInfoP, int nGun)
{
	fix	dot;

if ((gameData.ai.nPlayerVisibility == 2) || 
	 (gameData.ai.nDistToLastPlayerPosFiredAt < FIRE_AT_NEARBY_PLAYER_THRESHOLD)) {
	vmsVector vFirePos = gameData.ai.vBelievedPlayerPos;

	//	Hack: If visibility not == 2, we're here because we're firing at a nearby player.
	//	So, fire at gameData.ai.vLastPlayerPosFiredAt instead of the tPlayer position.
	if (!botInfoP->attackType && (gameData.ai.nPlayerVisibility != 2))
		vFirePos = gameData.ai.vLastPlayerPosFiredAt;

	//	Changed by mk, 01/04/95, onearm would take about 9 seconds until he can fire at you.
	//	Above comment corrected.  Date changed from 1994, to 1995.  Should fix some very subtle bugs, as well as not cause me to wonder, in the future, why I was writing AI code for onearm ten months before he existed.
	if (!gameData.ai.bObjAnimates || ReadyToFire (botInfoP, ailP)) {
		dot = VmVecDot (&objP->position.mOrient.fVec, &gameData.ai.vVecToPlayer);
		if ((dot >= 7 * F1_0 / 8) || ((dot > F1_0 / 4) && botInfoP->bossFlag)) {
			if (nGun < botInfoP->nGuns) {
				if (botInfoP->attackType == 1) {
					if (gameStates.app.bPlayerExploded || (gameData.ai.xDistToPlayer >= objP->size + gameData.objs.console->size + F1_0*2))	// botInfoP->circleDistance [gameStates.app.nDifficultyLevel] + gameData.objs.console->size) 
						return;
					if (!AIMultiplayerAwareness (objP, ROBOT_FIRE_AGITATION - 2))
						return;
					DoAiRobotHitAttack (objP, gameData.objs.console, &objP->position.vPos);
					}
				else {
#if 1
					if (AICanFireAtPlayer (objP, &gameData.ai.vGunPoint, &vFirePos)) {
#else
					if (gameData.ai.vGunPoint.p.x || gameData.ai.vGunPoint.p.y || gameData.ai.vGunPoint.p.z) {
#endif
						if (!AIMultiplayerAwareness (objP, ROBOT_FIRE_AGITATION))
							return;
						//	New, multi-weapon-nType system, 06/05/95 (life is slipping awayd:\temp\dm_test.)
						if (nGun != 0) {
							if (ailP->nextPrimaryFire <= 0) {
								AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, nGun, &vFirePos);
								gameData.ai.vLastPlayerPosFiredAt = vFirePos;
								}
							if ((ailP->nextSecondaryFire <= 0) && (botInfoP->nSecWeaponType != -1)) {
								CalcGunPoint (&gameData.ai.vGunPoint, objP, 0);
								AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, 0, &vFirePos);
								gameData.ai.vLastPlayerPosFiredAt = vFirePos;
								}
							}
						else if (ailP->nextPrimaryFire <= 0) {
							AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, nGun, &vFirePos);
							gameData.ai.vLastPlayerPosFiredAt = vFirePos;
							}
						}
					}
				}

			//	Wants to fire, so should go into chase mode, probably.
			if ((aiP->behavior != AIB_RUN_FROM) &&
				 (aiP->behavior != AIB_STILL) &&
				 (aiP->behavior != AIB_SNIPE) &&
				 (aiP->behavior != AIB_FOLLOW) &&
				 !botInfoP->attackType &&
				 ((ailP->mode == AIM_FOLLOW_PATH) || (ailP->mode == AIM_IDLING)))
				ailP->mode = AIM_CHASE_OBJECT;
				aiP->GOAL_STATE = AIS_RECOVER;
				ailP->goalState [aiP->CURRENT_GUN] = AIS_RECOVER;
				// Switch to next gun for next fire.
#if 0
				if (++(aiP->CURRENT_GUN) >= botInfoP->nGuns) {
					if ((botInfoP->nGuns == 1) || (botInfoP->nSecWeaponType == -1))
						aiP->CURRENT_GUN = 0;
					else
						aiP->CURRENT_GUN = 1;
					}
#endif
				}
			}
		}
else if ((!botInfoP->attackType && gameData.weapons.info [botInfoP->nWeaponType].homingFlag) || 
			(((botInfoP->nSecWeaponType != -1) && gameData.weapons.info [botInfoP->nSecWeaponType].homingFlag))) {
	fix dist;
	//	Robots which fire homing weapons might fire even if they don't have a bead on the player.
	if ((!gameData.ai.bObjAnimates || (ailP->achievedState [aiP->CURRENT_GUN] == AIS_FIRE))
			&& (((ailP->nextPrimaryFire <= 0) && (aiP->CURRENT_GUN != 0)) || 
				((ailP->nextSecondaryFire <= 0) && (aiP->CURRENT_GUN == 0)))
			&& ((dist = VmVecDistQuick (&gameData.ai.vHitPos, &objP->position.vPos)) > F1_0*40)) {
		if (!AIMultiplayerAwareness (objP, ROBOT_FIRE_AGITATION))
			return;
		AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, nGun, &gameData.ai.vBelievedPlayerPos);
		aiP->GOAL_STATE = AIS_RECOVER;
		ailP->goalState [aiP->CURRENT_GUN] = AIS_RECOVER;
		// Switch to next gun for next fire.
		if (++(aiP->CURRENT_GUN) >= botInfoP->nGuns)
			aiP->CURRENT_GUN = 0;
		}
	else {
		// Switch to next gun for next fire.
		if (++(aiP->CURRENT_GUN) >= botInfoP->nGuns)
			aiP->CURRENT_GUN = 0;
		}
	}
else {	//	---------------------------------------------------------------
	vmsVector	vLastPos;

	if (d_rand ()/2 < FixMul (gameData.time.xFrame, (gameStates.app.nDifficultyLevel << 12) + 0x4000)) {
		if ((!gameData.ai.bObjAnimates || ReadyToFire (botInfoP, ailP)) && 
			 (gameData.ai.nDistToLastPlayerPosFiredAt < FIRE_AT_NEARBY_PLAYER_THRESHOLD)) {
			VmVecNormalizedDir (&vLastPos, &gameData.ai.vBelievedPlayerPos, &objP->position.vPos);
			dot = VmVecDot (&objP->position.mOrient.fVec, &vLastPos);
			if (dot >= 7 * F1_0 / 8) {
				if (aiP->CURRENT_GUN < botInfoP->nGuns) {
					if (botInfoP->attackType == 1) {
						if (!gameStates.app.bPlayerExploded && (gameData.ai.xDistToPlayer < objP->size + gameData.objs.console->size + F1_0*2)) {		// botInfoP->circleDistance [gameStates.app.nDifficultyLevel] + gameData.objs.console->size) {
							if (!AIMultiplayerAwareness (objP, ROBOT_FIRE_AGITATION-2))
								return;
							DoAiRobotHitAttack (objP, gameData.objs.console, &objP->position.vPos);
							}
						else
							return;
						}
					else {
						if ((&gameData.ai.vGunPoint.p.x == 0) && (&gameData.ai.vGunPoint.p.y == 0) && (&gameData.ai.vGunPoint.p.z == 0))
							; 
						else {
							if (!AIMultiplayerAwareness (objP, ROBOT_FIRE_AGITATION))
								return;
							//	New, multi-weapon-nType system, 06/05/95 (life is slipping awayd:\temp\dm_test.)
							if (nGun != 0) {
								if (ailP->nextPrimaryFire <= 0)
									AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, nGun, &gameData.ai.vLastPlayerPosFiredAt);

								if ((ailP->nextSecondaryFire <= 0) && (botInfoP->nSecWeaponType != -1)) {
									CalcGunPoint (&gameData.ai.vGunPoint, objP, 0);
									AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, 0, &gameData.ai.vLastPlayerPosFiredAt);
									}
								} 
							else if (ailP->nextPrimaryFire <= 0)
								AIFireLaserAtPlayer (objP, &gameData.ai.vGunPoint, nGun, &gameData.ai.vLastPlayerPosFiredAt);
							}
						}
					//	Wants to fire, so should go into chase mode, probably.
					if ((aiP->behavior != AIB_RUN_FROM) && (aiP->behavior != AIB_STILL) && (aiP->behavior != AIB_SNIPE) && 
						 (aiP->behavior != AIB_FOLLOW) && ((ailP->mode == AIM_FOLLOW_PATH) || (ailP->mode == AIM_IDLING)))
						ailP->mode = AIM_CHASE_OBJECT;
					}
				aiP->GOAL_STATE = AIS_RECOVER;
				ailP->goalState [aiP->CURRENT_GUN] = AIS_RECOVER;
				// Switch to next gun for next fire.
				if (++(aiP->CURRENT_GUN) >= botInfoP->nGuns) {
					if (botInfoP->nGuns == 1)
						aiP->CURRENT_GUN = 0;
					else
						aiP->CURRENT_GUN = 1;
					}
				}
			}
		}
	}
}

//	---------------------------------------------------------------
// eof

