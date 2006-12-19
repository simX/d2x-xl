/* $Id: escort.c,v 1.7 2003/10/10 09:36:35 btb Exp $ */
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

#include <stdio.h>		// for printf()
#include <stdlib.h>		// for rand() and qsort()
#include <string.h>		// for memset()

#include "inferno.h"
#include "mono.h"
#include "fix.h"
#include "vecmat.h"
#include "gr.h"
#include "3d.h"
#include "palette.h"

#include "object.h"
#include "error.h"
#include "ai.h"
#include "robot.h"
#include "fvi.h"
#include "physics.h"
#include "wall.h"
#include "player.h"
#include "fireball.h"
#include "game.h"
#include "powerup.h"
#include "cntrlcen.h"
#include "gauges.h"
#include "key.h"
#include "fuelcen.h"
#include "sounds.h"
#include "screens.h"
#include "text.h"
#include "gamefont.h"
#include "newmenu.h"
#include "playsave.h"
#include "gameseq.h"
#include "automap.h"
#include "laser.h"
#include "pa_enabl.h"
#include "escort.h"
#include "ogl_init.h"
#include "network.h"
#include "gameseg.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

extern void MultiSendStolenItems();

#define	THIEF_ATTACK_TIME		(F1_0*10)

//	-------------------------------------------------------------------------------------------------

#define	THIEF_DEPTH	20

extern int PickConnectedSegment(tObject *objP, int max_depth);

//	-----------------------------------------------------------------------------
void _CDECL_ ThiefMessage(char * format, ... )
{
	char	gb_str[16], new_format[128];
	va_list	args;

va_start(args, format );
vsprintf(new_format, format, args);
va_end(args);

gb_str[0] = 1;
gb_str[1] = GrFindClosestColor (gamePalette, 28, 0, 0);
strcpy(&gb_str[2], "THIEF:");
gb_str[8] = 1;
gb_str[9] = GrFindClosestColor (gamePalette, 0, 31, 0);
gb_str[10] = 0;

HUDInitMessage("%s %s", gb_str, new_format);
}

//	------------------------------------------------------------------------------------------------------
//	Choose tSegment to recreate thief in.
int ChooseThiefRecreationSegment(void)
{
	int	nSegment = -1;
	int	cur_drop_depth;

	cur_drop_depth = THIEF_DEPTH;

	while ((nSegment == -1) && (cur_drop_depth > THIEF_DEPTH/2)) {
		nSegment = PickConnectedSegment(&gameData.objs.objects[gameData.multi.players[gameData.multi.nLocalPlayer].nObject], cur_drop_depth);
		if (gameData.segs.segment2s[nSegment].special == SEGMENT_IS_CONTROLCEN)
			nSegment = -1;
		cur_drop_depth--;
	}

	if (nSegment == -1) {
#if TRACE
		con_printf (1, "Warning: Unable to find a connected tSegment for thief recreation.\n");
#endif
		return (d_rand() * gameData.segs.nLastSegment) >> 15;
	} else
		return nSegment;

}

//	----------------------------------------------------------------------
void RecreateThief(tObject *objP)
{
	int			nSegment;
	vmsVector	center_point;
	tObject		*new_obj;

	nSegment = ChooseThiefRecreationSegment();
	COMPUTE_SEGMENT_CENTER_I (&center_point, nSegment);

	new_obj = CreateMorphRobot( &gameData.segs.segments[nSegment], &center_point, objP->id);
	InitAIObject(OBJ_IDX (new_obj), AIB_SNIPE, -1);
	gameData.thief.xReInitTime = gameData.time.xGame + F1_0*10;		//	In 10 seconds, re-initialize thief.
}

//	----------------------------------------------------------------------------

void DoThiefFrame(tObject *objP, fix dist_to_player, int player_visibility, vmsVector *vec_to_player)
{
	int			nObject = OBJ_IDX (objP);
	tAILocal		*ailp = &gameData.ai.localInfo[nObject];
	fix			connectedDistance;

	if ((gameData.missions.nCurrentLevel < 0) && (gameData.thief.xReInitTime < gameData.time.xGame)) {
		if (gameData.thief.xReInitTime > gameData.time.xGame - F1_0*2)
			InitThiefForLevel();
		gameData.thief.xReInitTime = 0x3f000000;
	}

	if ((dist_to_player > F1_0*500) && (ailp->nextActionTime > 0))
		return;

	if (gameStates.app.bPlayerIsDead)
		ailp->mode = AIM_THIEF_RETREAT;

	switch (ailp->mode) {
		case AIM_THIEF_WAIT:
			if (ailp->playerAwarenessType >= PA_PLAYER_COLLISION) {
				ailp->playerAwarenessType = 0;
				CreatePathToPlayer(objP, 30, 1);
				ailp->mode = AIM_THIEF_ATTACK;
				ailp->nextActionTime = THIEF_ATTACK_TIME/2;
				return;
			} else if (player_visibility) {
				CreateNSegmentPath(objP, 15, gameData.objs.console->nSegment);
				ailp->mode = AIM_THIEF_RETREAT;
				return;
			}

			if ((dist_to_player > F1_0*50) && (ailp->nextActionTime > 0))
				return;

			ailp->nextActionTime = gameData.thief.xWaitTimes[gameStates.app.nDifficultyLevel]/2;

			connectedDistance = FindConnectedDistance (&objP->position.vPos, objP->nSegment, &gameData.ai.vBelievedPlayerPos, gameData.ai.nBelievedPlayerSeg, 30, WID_FLY_FLAG);
			if (connectedDistance < F1_0*500) {
				CreatePathToPlayer(objP, 30, 1);
				ailp->mode = AIM_THIEF_ATTACK;
				ailp->nextActionTime = THIEF_ATTACK_TIME;	//	have up to 10 seconds to find player.
			}

			break;

		case AIM_THIEF_RETREAT:
			if (ailp->nextActionTime < 0) {
				ailp->mode = AIM_THIEF_WAIT;
				ailp->nextActionTime = gameData.thief.xWaitTimes[gameStates.app.nDifficultyLevel];
			} else if ((dist_to_player < F1_0*100) || player_visibility || (ailp->playerAwarenessType >= PA_PLAYER_COLLISION)) {
				AIFollowPath(objP, player_visibility, player_visibility, vec_to_player);
				if ((dist_to_player < F1_0*100) || (ailp->playerAwarenessType >= PA_PLAYER_COLLISION)) {
					tAIStatic	*aip = &objP->cType.aiInfo;
					if (((aip->nCurPathIndex <=1) && (aip->PATH_DIR == -1)) || ((aip->nCurPathIndex >= aip->nPathLength-1) && (aip->PATH_DIR == 1))) {
						ailp->playerAwarenessType = 0;
						CreateNSegmentPath(objP, 10, gameData.objs.console->nSegment);

						//	If path is real short, try again, allowing to go through tPlayer's tSegment
						if (aip->nPathLength < 4) {
							CreateNSegmentPath(objP, 10, -1);
						} else if (objP->shields* 4 < gameData.bots.pInfo[objP->id].strength) {
							//	If robot really low on hits, will run through tPlayer with even longer path
							if (aip->nPathLength < 8) {
								CreateNSegmentPath(objP, 10, -1);
							}
						}

						ailp->mode = AIM_THIEF_RETREAT;
					}
				} else
					ailp->mode = AIM_THIEF_RETREAT;

			}

			break;

		//	This means the thief goes from wherever he is to the player.
		//	Note: When thief successfully steals something, his action time is forced negative and his mode is changed
		//			to retreat to get him out of attack mode.
		case AIM_THIEF_ATTACK:
			if (ailp->playerAwarenessType >= PA_PLAYER_COLLISION) {
				ailp->playerAwarenessType = 0;
				if (d_rand() > 8192) {
					CreateNSegmentPath(objP, 10, gameData.objs.console->nSegment);
					gameData.ai.localInfo[OBJ_IDX (objP)].nextActionTime = gameData.thief.xWaitTimes[gameStates.app.nDifficultyLevel]/2;
					gameData.ai.localInfo[OBJ_IDX (objP)].mode = AIM_THIEF_RETREAT;
				}
			} else if (ailp->nextActionTime < 0) {
				//	This forces him to create a new path every second.
				ailp->nextActionTime = F1_0;
				CreatePathToPlayer(objP, 100, 0);
				ailp->mode = AIM_THIEF_ATTACK;
			} else {
				if (player_visibility && (dist_to_player < F1_0*100)) {
					//	If the tPlayer is close to looking at the thief, thief shall run away.
					//	No more stupid thief trying to sneak up on you when you're looking right at him!
					if (dist_to_player > F1_0*60) {
						fix	dot = VmVecDot(vec_to_player, &gameData.objs.console->position.mOrient.fVec);
						if (dot < -F1_0/2) {	//	Looking at least towards thief, so thief will run!
							CreateNSegmentPath(objP, 10, gameData.objs.console->nSegment);
							gameData.ai.localInfo[OBJ_IDX (objP)].nextActionTime = gameData.thief.xWaitTimes[gameStates.app.nDifficultyLevel]/2;
							gameData.ai.localInfo[OBJ_IDX (objP)].mode = AIM_THIEF_RETREAT;
						}
					} 
					AITurnTowardsVector(vec_to_player, objP, F1_0/4);
					move_towards_player(objP, vec_to_player);
				} else {
					tAIStatic	*aip = &objP->cType.aiInfo;
					//	If path length == 0, then he will keep trying to create path, but he is probably stuck in his closet.
					if ((aip->nPathLength > 1) || ((gameData.app.nFrameCount & 0x0f) == 0)) {
						AIFollowPath(objP, player_visibility, player_visibility, vec_to_player);
						ailp->mode = AIM_THIEF_ATTACK;
					}
				}
			}
			break;

		default:
#if TRACE
			con_printf (CON_DEBUG,"Thief mode (broken) = %d\n",ailp->mode);
#endif
			// -- Int3();	//	Oops, illegal mode for thief behavior.
			ailp->mode = AIM_THIEF_ATTACK;
			ailp->nextActionTime = F1_0;
			break;
	}

}

//	----------------------------------------------------------------------------
//	Return true if this item (whose presence is indicated by gameData.multi.players[player_num].flags) gets stolen.
int MaybeStealFlagItem(int player_num, int flagval)
{
	if (gameData.multi.players[player_num].flags & flagval) {
		if (d_rand() < THIEF_PROBABILITY) {
			int	powerup_index=-1;
			gameData.multi.players[player_num].flags &= (~flagval);
			switch (flagval) {
				case PLAYER_FLAGS_INVULNERABLE:
					powerup_index = POW_INVULNERABILITY;
					ThiefMessage("Invulnerability stolen!");
					break;
				case PLAYER_FLAGS_CLOAKED:
					powerup_index = POW_CLOAK;
					ThiefMessage("Cloak stolen!");
					break;
				case PLAYER_FLAGS_MAP_ALL:
					powerup_index = POW_FULL_MAP;
					ThiefMessage("Full map stolen!");
					break;
				case PLAYER_FLAGS_QUAD_LASERS:
					powerup_index = POW_QUADLASER;
					ThiefMessage("Quad lasers stolen!");
					break;
				case PLAYER_FLAGS_AFTERBURNER:
					powerup_index = POW_AFTERBURNER;
					ThiefMessage("Afterburner stolen!");
					break;
// --				case PLAYER_FLAGS_AMMO_RACK:
// --					powerup_index = POW_AMMORACK;
// --					ThiefMessage("Ammo Rack stolen!");
// --					break;
				case PLAYER_FLAGS_CONVERTER:
					powerup_index = POW_CONVERTER;
					ThiefMessage("Converter stolen!");
					break;
				case PLAYER_FLAGS_HEADLIGHT:
					powerup_index = POW_HEADLIGHT;
					ThiefMessage("Headlight stolen!");
				   gameData.multi.players[gameData.multi.nLocalPlayer].flags &= ~PLAYER_FLAGS_HEADLIGHT_ON;
					break;
			}
			Assert(powerup_index != -1);
			gameData.thief.stolenItems[gameData.thief.nStolenItem] = powerup_index;

			DigiPlaySampleOnce(SOUND_WEAPON_STOLEN, F1_0);
			return 1;
		}
	}

	return 0;
}

//	----------------------------------------------------------------------------
int MaybeStealSecondaryWeapon(int player_num, int weapon_num)
{
	if ((gameData.multi.players[player_num].secondaryWeaponFlags & HAS_FLAG(weapon_num)) && gameData.multi.players[player_num].secondaryAmmo[weapon_num])
		if (d_rand() < THIEF_PROBABILITY) {
			if (weapon_num == PROXIMITY_INDEX)
				if (d_rand() > 8192)		//	Come in groups of 4, only add 1/4 of time.
					return 0;
			gameData.multi.players[player_num].secondaryAmmo[weapon_num]--;

			//	Smart mines and proxbombs don't get dropped because they only come in 4 packs.
			if ((weapon_num != PROXIMITY_INDEX) && (weapon_num != SMART_MINE_INDEX)) {
				gameData.thief.stolenItems[gameData.thief.nStolenItem] = secondaryWeaponToPowerup[weapon_num];
			}

			ThiefMessage(TXT_WPN_STOLEN, baseGameTexts [114+weapon_num]);		//	Danger! Danger! Use of literal!  Danger!
			if (gameData.multi.players[gameData.multi.nLocalPlayer].secondaryAmmo[weapon_num] == 0)
				AutoSelectWeapon(1, 0);

			// -- compress_stolen_items();
			DigiPlaySampleOnce(SOUND_WEAPON_STOLEN, F1_0);
			return 1;
		}

	return 0;
}

//	----------------------------------------------------------------------------
int MaybeStealPrimaryWeapon(int player_num, int weapon_num)
{
	if ((gameData.multi.players[player_num].primaryWeaponFlags & HAS_FLAG(weapon_num)) && gameData.multi.players[player_num].primaryAmmo[weapon_num]) {
		if (d_rand() < THIEF_PROBABILITY) {
			if (weapon_num == 0) {
				if (gameData.multi.players[player_num].laserLevel > 0) {
					if (gameData.multi.players[player_num].laserLevel > 3) {
						gameData.thief.stolenItems[gameData.thief.nStolenItem] = POW_SUPERLASER;
					} else {
						gameData.thief.stolenItems[gameData.thief.nStolenItem] = primaryWeaponToPowerup[weapon_num];
					}
					ThiefMessage(TXT_LVL_DECREASED, baseGameTexts [104+weapon_num]);		//	Danger! Danger! Use of literal!  Danger!
					gameData.multi.players[player_num].laserLevel--;
					DigiPlaySampleOnce(SOUND_WEAPON_STOLEN, F1_0);
					return 1;
				}
			} else if (gameData.multi.players[player_num].primaryWeaponFlags & (1 << weapon_num)) {
				gameData.multi.players[player_num].primaryWeaponFlags &= ~(1 << weapon_num);
				gameData.thief.stolenItems[gameData.thief.nStolenItem] = primaryWeaponToPowerup[weapon_num];

				ThiefMessage(TXT_WPN_STOLEN, baseGameTexts [104+weapon_num]);		//	Danger! Danger! Use of literal!  Danger!
				AutoSelectWeapon(0, 0);
				DigiPlaySampleOnce(SOUND_WEAPON_STOLEN, F1_0);
				return 1;
			}
		}
	}

	return 0;
}



//	----------------------------------------------------------------------------
//	Called for a thief-nType robot.
//	If a item successfully stolen, returns true, else returns false.
//	If a wapon successfully stolen, do everything, removing it from tPlayer,
//	updating gameData.thief.stolenItems information, deselecting, etc.
int AttemptToStealItem3(tObject *objP, int player_num)
{
	int	i;

	if (gameData.ai.localInfo[OBJ_IDX (objP)].mode != AIM_THIEF_ATTACK)
		return 0;

	//	First, try to steal equipped items.

	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_INVULNERABLE))
		return 1;

	//	If primary weapon = laser, first try to rip away those nasty quad lasers!
	if (gameData.weapons.nPrimary == 0)
		if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_QUAD_LASERS))
			return 1;

	//	Makes it more likely to steal primary than secondary.
	for (i=0; i<2; i++)
		if (MaybeStealPrimaryWeapon(player_num, gameData.weapons.nPrimary))
			return 1;

	if (MaybeStealSecondaryWeapon(player_num, gameData.weapons.nSecondary))
		return 1;

	//	See what the tPlayer has and try to snag something.
	//	Try best things first.
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_INVULNERABLE))
		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_CLOAKED))
		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_QUAD_LASERS))
		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_AFTERBURNER))
		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_CONVERTER))
		return 1;
// --	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_AMMO_RACK))	//	Can't steal because what if have too many items, say 15 homing missiles?
// --		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_HEADLIGHT))
		return 1;
	if (MaybeStealFlagItem(player_num, PLAYER_FLAGS_MAP_ALL))
		return 1;

	for (i=MAX_SECONDARY_WEAPONS-1; i>=0; i--) {
		if (MaybeStealPrimaryWeapon(player_num, i))
			return 1;
		if (MaybeStealSecondaryWeapon(player_num, i))
			return 1;
	}

	return 0;
}

//	----------------------------------------------------------------------------
int AttemptToStealItem2(tObject *objP, int player_num)
{
	int	rval;

	rval = AttemptToStealItem3(objP, player_num);

	if (rval) {
		gameData.thief.nStolenItem = (gameData.thief.nStolenItem+1) % MAX_STOLEN_ITEMS;
		if (d_rand() > 20000)	//	Occasionally, boost the value again
			gameData.thief.nStolenItem = (gameData.thief.nStolenItem+1) % MAX_STOLEN_ITEMS;
	}

	return rval;
}

//	----------------------------------------------------------------------------
//	Called for a thief-nType robot.
//	If a item successfully stolen, returns true, else returns false.
//	If a wapon successfully stolen, do everything, removing it from tPlayer,
//	updating gameData.thief.stolenItems information, deselecting, etc.
int AttemptToStealItem(tObject *objP, int player_num)
{
	int	i;
	int	rval = 0;

	if (objP->cType.aiInfo.xDyingStartTime)
		return 0;

	rval += AttemptToStealItem2(objP, player_num);

	for (i=0; i<3; i++) {
		if (!rval || (d_rand() < 11000)) {	//	about 1/3 of time, steal another item
			rval += AttemptToStealItem2(objP, player_num);
		} else
			break;
	}
	CreateNSegmentPath(objP, 10, gameData.objs.console->nSegment);
	gameData.ai.localInfo[OBJ_IDX (objP)].nextActionTime = gameData.thief.xWaitTimes[gameStates.app.nDifficultyLevel]/2;
	gameData.ai.localInfo[OBJ_IDX (objP)].mode = AIM_THIEF_RETREAT;
	if (rval) {
		PALETTE_FLASH_ADD(30, 15, -20);
		UpdateLaserWeaponInfo();
//		DigiLinkSoundToPos( SOUND_NASTY_ROBOT_HIT_1, objP->nSegment, 0, &objP->position.vPos, 0 , DEFAULT_ROBOT_SOUND_VOLUME);
//	I removed this to make the "steal sound" more obvious -AP
                if (gameData.app.nGameMode & GM_NETWORK)
                 MultiSendStolenItems();
	}
	return rval;
}

// --------------------------------------------------------------------------------------------------------------
//	Indicate no items have been stolen.
void InitThiefForLevel(void)
{
	int	i;

#if 1
memset (gameData.thief.stolenItems, 255, sizeof (gameData.thief.stolenItems));
#else
	for (i=0; i<MAX_STOLEN_ITEMS; i++)
		gameData.thief.stolenItems[i] = 255;
#endif
	Assert (MAX_STOLEN_ITEMS >= 3*2);	//	Oops!  Loop below will overwrite memory!
   if (!(gameData.app.nGameMode & GM_MULTI))    
		for (i=0; i<3; i++) {
			gameData.thief.stolenItems[2*i] = POW_SHIELD_BOOST;
			gameData.thief.stolenItems[2*i+1] = POW_ENERGY;
		}

	gameData.thief.nStolenItem = 0;
}

// --------------------------------------------------------------------------------------------------------------

void DropStolenItems(tObject *objP)
{
	int	i;
#if TRACE
        con_printf (CON_DEBUG,"Dropping thief items!\n");
#endif
	// -- compress_stolen_items();

	for (i=0; i<MAX_STOLEN_ITEMS; i++) {
		if (gameData.thief.stolenItems[i] != 255)
			DropPowerup(OBJ_POWERUP, gameData.thief.stolenItems[i], -1, 1, &objP->mType.physInfo.velocity, &objP->position.vPos, objP->nSegment);
		gameData.thief.stolenItems[i] = 255;
	}

}

// --------------------------------------------------------------------------------------------------------------
