/* $Id: weapon.c,v 1.9 2003/10/11 09:28:38 btb Exp $ */
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
static char rcsid[] = "$Id: weapon.c,v 1.9 2003/10/11 09:28:38 btb Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inferno.h"
#include "laser.h"
#include "weapon.h"
#include "error.h"
#include "sounds.h"
#include "text.h"
#include "network.h"
#include "hudmsg.h"

//	-----------------------------------------------------------------------------

#define SPIT_SPEED 20

//this function is for when the tPlayer intentionally drops a powerup
//this function is based on DropPowerup()
int SpitPowerup (tObject *spitterP, ubyte id, int seed)
{
	short			nObject;
	tObject		*objP;
	vmsVector	newVelocity, newPos;
	tPosition	*posP = OBJPOS (spitterP);

#if 0
if ((gameData.app.nGameMode & GM_NETWORK) &&
	 (gameData.multiplayer.powerupsInMine [(int)id] + PowerupsOnShips (id) >= 
	  gameData.multiplayer.maxPowerupsAllowed [id]))
	return -1;
#endif
d_srand(seed);
VmVecScaleAdd (&newVelocity,
					&spitterP->mType.physInfo.velocity,
					&spitterP->position.mOrient.fVec,
					i2f (SPIT_SPEED));
newVelocity.p.x += (d_rand() - 16384) * SPIT_SPEED * 2;
newVelocity.p.y += (d_rand() - 16384) * SPIT_SPEED * 2;
newVelocity.p.z += (d_rand() - 16384) * SPIT_SPEED * 2;
// Give keys zero velocity so they can be tracked better in multi
if (IsMultiGame && (id >= POW_KEY_BLUE) && (id <= POW_KEY_GOLD))
	VmVecZero(&newVelocity);
//there's a piece of code which lets the tPlayer pick up a powerup if
//the distance between him and the powerup is less than 2 time their
//combined radii.  So we need to create powerups pretty far out from
//the player.
VmVecScaleAdd (&newPos, &posP->vPos, &posP->mOrient.fVec, spitterP->size);
if (IsMultiGame && (gameData.multigame.create.nLoc >= MAX_NET_CREATE_OBJECTS))
	return (-1);
nObject = CreateObject (OBJ_POWERUP, id, (short) (GetTeam (gameData.multiplayer.nLocalPlayer) + 1), 
							  (short) OBJSEG (spitterP), &newPos, &vmdIdentityMatrix, gameData.objs.pwrUp.info[id].size, 
							  CT_POWERUP, MT_PHYSICS, RT_POWERUP, 1);
if (nObject < 0) {
	Int3();
	return nObject;
	}
objP = gameData.objs.objects + nObject;
objP->mType.physInfo.velocity = newVelocity;
objP->mType.physInfo.drag = 512;	//1024;
objP->mType.physInfo.mass = F1_0;
objP->mType.physInfo.flags = PF_BOUNCE;
objP->rType.vClipInfo.nClipIndex = gameData.objs.pwrUp.info [objP->id].nClipIndex;
objP->rType.vClipInfo.xFrameTime = gameData.eff.pVClips [objP->rType.vClipInfo.nClipIndex].xFrameTime;
objP->rType.vClipInfo.nCurFrame = 0;
if (spitterP == gameData.objs.console)
	objP->cType.powerupInfo.flags |= PF_SPAT_BY_PLAYER;
switch (objP->id) {
	case POW_CONCUSSION_1:
	case POW_CONCUSSION_4:
	case POW_SHIELD_BOOST:
	case POW_ENERGY:
		objP->lifeleft = (d_rand() + F1_0*3) * 64;		//	Lives for 3 to 3.5 binary minutes (a binary minute is 64 seconds)
		if (gameData.app.nGameMode & GM_MULTI)
			objP->lifeleft /= 2;
		break;
	default:
		//if (gameData.app.nGameMode & GM_MULTI)
		//	objP->lifeleft = (d_rand() + F1_0*3) * 64;		//	Lives for 5 to 5.5 binary minutes (a binary minute is 64 seconds)
		break;
	}
MultiSendWeapons (1);
return nObject;
}

//	-----------------------------------------------------------------------------

static inline int IsBuiltInDevice (int nDeviceFlag)
{
return gameStates.app.bHaveExtraGameInfo [IsMultiGame] && ((extraGameInfo [IsMultiGame].loadout.nDevices & nDeviceFlag) != 0);
}

//	-----------------------------------------------------------------------------

static inline int IsBuiltInGun (int nGunIndex)
{
return gameStates.app.bHaveExtraGameInfo [IsMultiGame] && ((extraGameInfo [IsMultiGame].loadout.nGuns & HAS_FLAG (nGunIndex)) != 0);
}

//	-----------------------------------------------------------------------------

void DropCurrentWeapon (void)
{
	int	nObject = -1, 
			ammo = 0, 
			seed;

seed = d_rand ();
if (gameData.weapons.nPrimary == 0) {	//special laser drop handling
	if ((LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) && !IsBuiltInDevice (PLAYER_FLAGS_QUAD_LASERS)) {
		LOCALPLAYER.flags &= ~PLAYER_FLAGS_QUAD_LASERS;
		nObject = SpitPowerup (gameData.objs.console, POW_QUADLASER, seed);
		if (nObject == -1) {
			LOCALPLAYER.flags |= PLAYER_FLAGS_QUAD_LASERS;
			return;
			}
		HUDInitMessage(TXT_DROP_QLASER);
		}
	else if ((LOCALPLAYER.laserLevel > MAX_LASER_LEVEL) && !IsBuiltInGun (SUPER_LASER_INDEX)) {
		LOCALPLAYER.laserLevel--;
		nObject = SpitPowerup (gameData.objs.console, POW_SUPERLASER, seed);
		if (nObject == -1) {
			LOCALPLAYER.laserLevel++;
			return;
			}
		HUDInitMessage (TXT_DROP_SLASER);
		}
	}
else {
	if ((gameData.weapons.nPrimary == 4) && gameData.weapons.bTripleFusion)
		gameData.weapons.bTripleFusion = 0;
	else if (gameData.weapons.nPrimary && !IsBuiltInGun (gameData.weapons.nPrimary)) {//if selected weapon was not the laser
		LOCALPLAYER.primaryWeaponFlags &= (~(1 << gameData.weapons.nPrimary));
		nObject = SpitPowerup (gameData.objs.console, primaryWeaponToPowerup [gameData.weapons.nPrimary], seed);
		}
	if (nObject == -1) {
		if (gameData.weapons.nPrimary) 	//if selected weapon was not the laser
			LOCALPLAYER.primaryWeaponFlags |= (1 << gameData.weapons.nPrimary);
		return;
		}
	HUDInitMessage (TXT_DROP_WEAPON, PRIMARY_WEAPON_NAMES (gameData.weapons.nPrimary));
	}
DigiPlaySample (SOUND_DROP_WEAPON,F1_0);
if ((gameData.weapons.nPrimary == VULCAN_INDEX) || (gameData.weapons.nPrimary == GAUSS_INDEX)) {
	//if it's one of these, drop some ammo with the weapon
	ammo = LOCALPLAYER.primaryAmmo [VULCAN_INDEX];
	if ((LOCALPLAYER.primaryWeaponFlags & HAS_FLAG(VULCAN_INDEX)) && (gameData.weapons.nPrimary == GAUSS_INDEX))
		ammo /= 2;		//if both vulcan & gauss, drop half
	LOCALPLAYER.primaryAmmo [VULCAN_INDEX] -= ammo;
	if (nObject != -1)
		gameData.objs.objects [nObject].cType.powerupInfo.count = ammo;
	}
if (gameData.weapons.nPrimary == OMEGA_INDEX) {
	//dropped weapon has current energy
	if (nObject != -1)
		gameData.objs.objects [nObject].cType.powerupInfo.count = gameData.omega.xCharge [IsMultiGame];
	}
if (IsMultiGame) {
	MultiSendDropWeapon (nObject, seed);
	MultiSendWeapons (1);
	}
if (gameData.weapons.nPrimary) //if selected weapon was not the laser
	AutoSelectWeapon (0, 0);
}

//	-----------------------------------------------------------------------------

extern void DropOrb (void);

void DropSecondaryWeapon (int nWeapon)
{
	int nObject, seed, nPowerup, bHoardEntropy, bMine;

if (nWeapon < 0)
	nWeapon = gameData.weapons.nSecondary;
if (LOCALPLAYER.secondaryAmmo [nWeapon] == 0) {
	HUDInitMessage (TXT_CANT_DROP_SEC);
	return;
	}
nPowerup = secondaryWeaponToPowerup[nWeapon];
bHoardEntropy = (gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)) != 0;
bMine = (nPowerup == POW_PROXMINE) || (nPowerup == POW_SMARTMINE);
if (!bHoardEntropy && bMine &&
	  LOCALPLAYER.secondaryAmmo [nWeapon] < 4) {
	HUDInitMessage(TXT_DROP_NEED4);
	return;
	}
if (bHoardEntropy) {
	DropOrb ();
	return;
	}
if (bMine)
	LOCALPLAYER.secondaryAmmo [nWeapon] -= 4;
else
	LOCALPLAYER.secondaryAmmo [nWeapon]--;
seed = d_rand();
nObject = SpitPowerup (gameData.objs.console, nPowerup, seed);
if (nObject == -1) {
	if (bMine)
		LOCALPLAYER.secondaryAmmo [nWeapon] += 4;
	else
		LOCALPLAYER.secondaryAmmo [nWeapon]++;
	return;
	}
HUDInitMessage (TXT_DROP_WEAPON, SECONDARY_WEAPON_NAMES (gameData.weapons.nSecondary));
DigiPlaySample (SOUND_DROP_WEAPON,F1_0);
if (gameData.app.nGameMode & GM_MULTI) {
	MultiSendDropWeapon (nObject, seed);
	MultiSendWeapons (1);
	}
if (LOCALPLAYER.secondaryAmmo [nWeapon] == 0) {
	LOCALPLAYER.secondaryWeaponFlags &= (~(1<<gameData.weapons.nSecondary));
	AutoSelectWeapon (1, 0);
	}
}

//	-----------------------------------------------------------------------------
//eof
