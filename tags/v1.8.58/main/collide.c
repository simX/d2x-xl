/* $Id: Collide.c, v 1.12 2003/03/27 01:23:18 btb Exp $ */
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
 * Routines to handle collisions
 *
 * Old Log:
 * Revision 1.3  1995/11/08  17:15:21  allender
 * make CollidePlayerAndWeapon play player_hitSound if
 * shareware and not my playernum
 *
 * Revision 1.2  1995/10/31  10:24:37  allender
 * shareware stuff
 *
 * Revision 1.1  1995/05/16  15:23:34  allender
 * Initial revision
 *
 * Revision 2.5  1995/07/26  12:07:46  john
 * Made code that pages in tWeaponInfo->robot_hit_vclip not
 * page in unless it is a badass weapon.  Took out old functionallity
 * of using this if no robot exp1_vclip, since all robots have these.
 *
 * Revision 2.4  1995/03/30  16:36:09  mike
 * text localization.
 *
 * Revision 2.3  1995/03/24  15:11:13  john
 * Added ugly robot cheat.
 *
 * Revision 2.2  1995/03/21  14:41:04  john
 * Ifdef'd out the NETWORK code.
 *
 * Revision 2.1  1995/03/20  18:16:02  john
 * Added code to not store the normals in the tSegment structure.
 *
 * Revision 2.0  1995/02/27  11:32:20  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.289  1995/02/22  13:56:06  allender
 * remove anonymous unions from tObject structure
 *
 * Revision 1.288  1995/02/11  15:52:45  rob
 * Included text.h.
 *
 * Revision 1.287  1995/02/11  15:04:11  rob
 * Localized a string.
 *
 * Revision 1.286  1995/02/11  14:25:41  rob
 * Added invul. controlcen option.
 *
 * Revision 1.285  1995/02/06  15:53:00  mike
 * create awareness event for player:wall collision.
 *
 * Revision 1.284  1995/02/05  23:18:17  matt
 * Deal with gameData.objs.objects (such as fusion blobs) that get created already
 * poking through a wall
 *
 * Revision 1.283  1995/02/01  17:51:33  mike
 * fusion bolt can now toast multiple proximity bombs.
 *
 * Revision 1.282  1995/02/01  17:29:20  john
 * Lintized
 *
 * Revision 1.281  1995/02/01  15:04:00  rob
 * Changed sound of weapons hitting invulnerable players.
 *
 * Revision 1.280  1995/01/31  16:16:35  mike
 * Separate smart blobs for robot and player.
 *
 * Revision 1.279  1995/01/29  15:57:10  rob
 * Fixed another bug with robot_request_change calls.
 *
 * Revision 1.278  1995/01/28  18:15:06  rob
 * Fixed a bug in multi_requestRobot_change.
 *
 * Revision 1.277  1995/01/27  15:15:44  rob
 * Fixing problems with controlcen damage.
 *
 * Revision 1.276  1995/01/27  15:13:10  mike
 * comment out con_printf.
 *
 * Revision 1.275  1995/01/26  22:11:51  mike
 * Purple chromo-blaster (ie, fusion cannon) spruce up (chromification)
 *
 * Revision 1.274  1995/01/26  18:57:55  rob
 * Changed two uses of DigiPlaySample to DigiLinkSoundToPos which
 * made more sense.
 *
 * Revision 1.273  1995/01/25  23:37:58  mike
 * make persistent gameData.objs.objects not hit player more than once.
 * Also, make them hit player before degrading them, else they often did 0 damage.
 *
 * Revision 1.272  1995/01/25  18:23:54  rob
 * Don't let players pick up powerups in exit tunnel.
 *
 * Revision 1.271  1995/01/25  13:43:18  rob
 * Added robot transfer for player collisions.
 * Removed con_printf from Collide.c on Mike's request.
 *
 * Revision 1.270  1995/01/25  10:24:01  mike
 * Make sizzle and rock happen in lava even if you're invulnerable.
 *
 * Revision 1.269  1995/01/22  17:05:33  mike
 * Call multiRobot_request_change when a robot gets whacked by a player or
 * player weapon, if player_num != gameData.multi.nLocalPlayer
 *
 * Revision 1.268  1995/01/21  21:20:28  matt
 * Fixed stupid bug
 *
 * Revision 1.267  1995/01/21  18:47:47  rob
 * Fixed a really dumb bug with player keys.
 *
 * Revision 1.266  1995/01/21  17:39:30  matt
 * Cleaned up laser/player hit wall confusions
 *
 * Revision 1.265  1995/01/19  17:44:42  mike
 * damage_force removed, that information coming from strength field.
 *
 * Revision 1.264  1995/01/18  17:12:56  rob
 * Fixed control stuff for multiplayer.
 *
 * Revision 1.263  1995/01/18  16:12:33  mike
 * Make control center aware of a cloaked playerr when he fires.
 *
 * Revision 1.262  1995/01/17  17:48:42  rob
 * Added key syncing for coop players.
 *
 * Revision 1.261  1995/01/16  19:30:28  rob
 * Fixed an assert error in fireball.c
 *
 * Revision 1.260  1995/01/16  19:23:51  mike
 * Say Boss_been_hit if he been hit so he gates appropriately.
 *
 * Revision 1.259  1995/01/16  11:55:16  mike
 * make enemies become aware of player if he damages control center.
 *
 * Revision 1.258  1995/01/15  16:42:00  rob
 * Fixed problem with robot bumping damage.
 *
 * Revision 1.257  1995/01/14  19:16:36  john
 * First version of new bitmap paging code.
 *
 * Revision 1.256  1995/01/03  17:58:37  rob
 * Fixed scoring problems.
 *
 * Revision 1.255  1994/12/29  12:41:11  rob
 * Tweaking robot exploding in coop.
 *
 * Revision 1.254  1994/12/28  10:37:59  rob
 * Fixed ifdef of multibot stuff.
 *
 * Revision 1.253  1994/12/21  19:03:14  rob
 * Fixing score accounting for multiplayer robots
 *
 * Revision 1.252  1994/12/21  17:36:31  rob
 * Fix hostage pickup problem in network.
 * tweaking robot powerup drops.
 *
 * Revision 1.251  1994/12/19  20:32:34  rob
 * Remove awareness events from player collisions and lasers that are not the console player.
 *
 * Revision 1.250  1994/12/19  20:01:22  rob
 * Added multibot.h include.
 *
 * Revision 1.249  1994/12/19  16:36:41  rob
 * Patches damaging of multiplayer robots.
 *
 * Revision 1.248  1994/12/14  21:15:18  rob
 * play lava hiss across network.
 *
 * Revision 1.247  1994/12/14  17:09:09  matt
 * Fixed problem with no sound when lasers hit closed walls, like grates.
 *
 * Revision 1.246  1994/12/14  09:51:49  mike
 * make any weapon cause proximity bomb detonation.
 *
 * Revision 1.245  1994/12/13  12:55:25  mike
 * change number of proximity bomb powerups which get dropped.
 *
 * Revision 1.244  1994/12/12  17:17:53  mike
 * make boss cloak/teleport when get hit, make quad laser 3/4 as powerful.
 *
 * Revision 1.243  1994/12/12  12:07:51  rob
 * Don't take damage if we're in endlevel sequence.
 *
 * Revision 1.242  1994/12/11  23:44:52  mike
 * less PhysApplyRot () at higher skill levels.
 *
 * Revision 1.241  1994/12/11  12:37:02  mike
 * remove stupid robot spinning code.  it was really stupid. (actually, call here, code in ai.c).
 *
 * Revision 1.240  1994/12/10  16:44:51  matt
 * Added debugging code to track down door that turns into rock
 *
 * Revision 1.239  1994/12/09  14:59:19  matt
 * Added system to attach a fireball to another tObject for rendering purposes, 
 * so the fireball always renders on top of (after) the tObject.
 *
 * Revision 1.238  1994/12/09  09:57:02  mike
 * Don't allow robots or their weapons to pass through control center.
 *
 * Revision 1.237  1994/12/08  15:46:03  mike
 * better robot behavior.
 *
 * Revision 1.236  1994/12/08  12:32:56  mike
 * make boss dying more interesting.
 *
 * Revision 1.235  1994/12/07  22:49:15  mike
 * tweak rotation due to collision.
 *
 * Revision 1.234  1994/12/07  16:44:50  mike
 * make bump sound if supposed to, even if not taking damage.
 *
 * Revision 1.233  1994/12/07  12:55:08  mike
 * tweak rotVel applied from collisions.
 *
 * Revision 1.232  1994/12/05  19:30:48  matt
 * Fixed horrible tSegment over-dereferencing
 *
 * Revision 1.231  1994/12/05  00:32:15  mike
 * do rotVel on badass and bump collisions.
 *
 * Revision 1.230  1994/12/03  12:49:22  mike
 * don't play bonk sound when you Collide with a volatile wall (like lava).
 *
 * Revision 1.229  1994/12/02  16:51:09  mike
 * make lava sound only happen at 4 Hz.
 *
 * Revision 1.228  1994/11/30  23:55:27  rob
 * Fixed a bug where a laser hitting a wall was making 2 sounds.
 *
 * Revision 1.227  1994/11/30  20:11:00  rob
 * Fixed # of dropped laser powerups.
 *
 * Revision 1.226  1994/11/30  19:19:03  rob
 * Transmit collission sounds for net games.
 *
 * Revision 1.225  1994/11/30  16:33:01  mike
 * new boss behavior.
 *
 * Revision 1.224  1994/11/30  15:44:17  mike
 * /2 on boss smart children damage.
 *
 * Revision 1.223  1994/11/30  14:03:03  mike
 * hook for claw sounds
 *
 * Revision 1.222  1994/11/29  20:41:09  matt
 * Deleted a bunch of commented-out lines
 *
 * Revision 1.221  1994/11/27  23:15:08  matt
 * Made changes for new con_printf calling convention
 *
 * Revision 1.220  1994/11/19  16:11:28  rob
 * Collision damage with walls or lava is counted as suicides in net games
 *
 * Revision 1.219  1994/11/19  15:20:41  mike
 * rip out unused code and data
 *
 * Revision 1.218  1994/11/17  18:44:27  rob
 * Added OBJ_GHOST to list of valid player types to create eggs.
 *
 * Revision 1.217  1994/11/17  14:57:59  mike
 * moved tSegment validation functions from editor to main.
 *
 * Revision 1.216  1994/11/16  23:38:36  mike
 * new improved boss teleportation behavior.
 *
 * Revision 1.215  1994/11/16  12:16:29  mike
 * Enable collisions between robots.  A hack in fvi.c only does this for robots which lunge to attack (eg, green guy)
 *
 * Revision 1.214  1994/11/15  16:51:50  mike
 * bump player when he hits a volatile wall.
 *
 * Revision 1.213  1994/11/12  16:38:44  mike
 * allow flares to open doors.
 *
 * Revision 1.212  1994/11/10  13:09:19  matt
 * Added support for new run-length-encoded bitmaps
 *
 * Revision 1.211  1994/11/09  17:05:43  matt
 * Fixed problem with volatile walls
 *
 * Revision 1.210  1994/11/09  12:11:46  mike
 * only award points if gameData.objs.console killed robot.
 *
 * Revision 1.209  1994/11/09  11:11:03  yuan
 * Made wall volatile if either tmap_num1 or nOvlTex is a volatile wall.
 *
 * Revision 1.208  1994/11/08  12:20:15  mike
 * moved DoReactorDestroyedStuff from here to cntrlcen.c
 *
 * Revision 1.207  1994/11/02  23:22:08  mike
 * Make ` (backquote, KEY_LAPOSTRO) tell what wall was hit by laser.
 *
 * Revision 1.206  1994/11/02  18:03:00  rob
 * Fix control_center_been_hit logic so it only cares about the local player.
 * Other players take care of their own control center 'ai'.
 *
 * Revision 1.205  1994/11/01  19:37:33  rob
 * Changed the max # of consussion missiles to 4.
 * (cause they're lame and clutter things up)
 *
 * Revision 1.204  1994/11/01  18:06:35  john
 * Tweaked wall banging sound constant.
 *
 * Revision 1.203  1994/11/01  18:01:40  john
 * Made wall bang less obnoxious, but volume based.
 *
 * Revision 1.202  1994/11/01  17:11:05  rob
 * Changed some stuff in dropPlayer_eggs.
 *
 * Revision 1.201  1994/11/01  12:18:23  john
 * Added sound volume support. Made wall collisions be louder/softer.
 *
 * Revision 1.200  1994/10/31  13:48:44  rob
 * Fixed bug in opening doors over network/modem.  Added a new message
 * nType to multi.c that communicates door openings across the net.
 * Changed includes in multi.c and wall.c to accomplish this.
 *
 * Revision 1.199  1994/10/28  14:42:52  john
 * Added sound volumes to all sound calls.
 *
 * Revision 1.198  1994/10/27  16:58:37  allender
 * added demo recording of monitors blowing up
 *
 * Revision 1.197  1994/10/26  23:20:52  matt
 * Tone down flash even more
 *
 * Revision 1.196  1994/10/26  23:01:50  matt
 * Toned down red flash when damaged
 *
 * Revision 1.195  1994/10/26  15:56:29  yuan
 * Tweaked some palette flashes.
 *
 * Revision 1.194  1994/10/25  11:32:26  matt
 * Fixed bugs with vulcan powerups in mutliplayer
 *
 * Revision 1.193  1994/10/25  10:51:18  matt
 * Vulcan cannon powerups now contain ammo count
 *
 * Revision 1.192  1994/10/24  14:14:05  matt
 * Fixed bug in BumpTwoObjects ()
 *
 * Revision 1.191  1994/10/23  19:17:04  matt
 * Fixed bug with "no key" messages
 *
 * Revision 1.190  1994/10/22  00:08:46  matt
 * Fixed up problems with bonus & game sequencing
 * Player doesn't get credit for hostages unless he gets them out alive
 *
 * Revision 1.189  1994/10/21  20:42:34  mike
 * Clear number of hostages on board between levels.
 *
 * Revision 1.188  1994/10/20  15:17:43  mike
 * control center in boss handling.
 *
 * Revision 1.187  1994/10/20  10:09:47  mike
 * Only ever drop 1 shield powerup in multiplayer (as an egg).
 *
 * Revision 1.186  1994/10/20  09:47:11  mike
 * Fix bug in dropping vulcan ammo in multiplayer.
 * Also control center stuff.
 *
 * Revision 1.185  1994/10/19  15:14:32  john
 * Took % hits out of player structure, made %kills work properly.
 *
 * Revision 1.184  1994/10/19  11:33:16  john
 * Fixed hostage rescued percent.
 *
 * Revision 1.183  1994/10/19  11:16:49  mike
 * Don't allow crazy josh to open locked doors.
 * Don't allow weapons to harm parent.
 *
 * Revision 1.182  1994/10/18  18:37:01  mike
 * No more hostage killing.  Too much stuff to do to integrate into game.
 *
 * Revision 1.181  1994/10/18  16:37:35  mike
 * Debug function for Yuan: Show seg:tSide when hit by puny laser if Show_segAnd_side != 0.
 *
 * Revision 1.180  1994/10/18  10:53:17  mike
 * Support attack nType as a property of a robot, not of being == GREEN_GUY.
 *
 * Revision 1.179  1994/10/17  21:18:36  mike
 * diminish damage player does to robot due to collision, only took 2-3 hits to kill a josh.
 *
 * Revision 1.178  1994/10/17  20:30:40  john
 * Made playerHostages_rescued or whatever count properly.
 *
 * Revision 1.177  1994/10/16  12:42:56  mike
 * Trap bogus amount of vulcan ammo dropping.
 *
 * Revision 1.176  1994/10/15  19:06:51  mike
 * Drop vulcan ammo if player has it, but no vulcan cannon (when he dies).
 *
 * Revision 1.175  1994/10/13  15:42:06  mike
 * Remove afterburner.
 *
 * Revision 1.174  1994/10/13  11:12:57  mike
 * Apply damage to robots.  I hosed it a couple weeks ago when I made the green guy special.
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rle.h"
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
#include "object.h"
#include "physics.h"
#include "slew.h"		
#include "render.h"
#include "wall.h"
#include "vclip.h"
#include "polyobj.h"
#include "fireball.h"
#include "laser.h"
#include "error.h"
#include "ai.h"
#include "hostage.h"
#include "fuelcen.h"
#include "sounds.h"
#include "robot.h"
#include "weapon.h"
#include "player.h"
#include "gauges.h"
#include "powerup.h"
#ifdef NETWORK
#include "network.h"
#endif
#include "newmenu.h"
#include "scores.h"
#include "effects.h"
#include "textures.h"
#ifdef NETWORK
#include "multi.h"
#endif
#include "cntrlcen.h"
#include "newdemo.h"
#include "endlevel.h"
#include "multibot.h"
#include "piggy.h"
#include "text.h"
#include "automap.h"
#include "switch.h"
#include "palette.h"
#include "object.h"
#include "sphere.h"
#include "text.h"

#ifdef TACTILE
#include "tactile.h"
#endif 

#ifdef EDITOR
#include "editor/editor.h"
#endif

#include "collide.h"
#include "escort.h"

#define STANDARD_EXPL_DELAY (f1_0/4)

//##void CollideFireballAnd_wall (tObject *fireball, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)	{
//##	return; 
//##}

//	-------------------------------------------------------------------------------------------------------------
//	The only reason this routine is called (as of 10/12/94) is so Brain guys can open doors.
void CollideRobotAndWall (tObject * robot, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)
{
	tAILocal		*ailp = &gameData.ai.localInfo [OBJ_IDX (robot)];
	tRobotInfo	*rInfoP = gameData.bots.pInfo + robot->id;

	if ((robot->id == ROBOT_BRAIN) || 
		 (robot->cType.aiInfo.behavior == AIB_RUN_FROM) || 
		 (rInfoP->companion == 1) || 
		 (robot->cType.aiInfo.behavior == AIB_SNIPE)) {
		int	nWall = WallNumI (hitseg, hitwall);
		if (nWall != -1) {
			wall *wallP = gameData.walls.walls + nWall;
			if ((wallP->nType == WALL_DOOR) &&
				 (wallP->keys == KEY_NONE) && 
				 (wallP->state == WALL_DOOR_CLOSED) && 
				 !(wallP->flags & WALL_DOOR_LOCKED)) {
				WallOpenDoor (gameData.segs.segments + hitseg, hitwall);
			// -- Changed from this, 10/19/95, MK: Don't want buddy getting stranded from player
			//-- } else if ((rInfoP->companion == 1) && (gameData.walls.walls [nWall].nType == WALL_DOOR) && (gameData.walls.walls [nWall].keys != KEY_NONE) && (gameData.walls.walls [nWall].state == WALL_DOOR_CLOSED) && !(gameData.walls.walls [nWall].flags & WALL_DOOR_LOCKED)) {
			} else if ((rInfoP->companion == 1) && (wallP->nType == WALL_DOOR)) {
				if ((ailp->mode == AIM_GOTO_PLAYER) || (gameData.escort.nSpecialGoal == ESCORT_GOAL_SCRAM)) {
					if (wallP->keys != KEY_NONE) {
						if (wallP->keys & gameData.multi.players [gameData.multi.nLocalPlayer].flags)
							WallOpenDoor (gameData.segs.segments + hitseg, hitwall);
					} else if (!(wallP->flags & WALL_DOOR_LOCKED))
						WallOpenDoor (gameData.segs.segments + hitseg, hitwall);
				}
			} else if (rInfoP->thief) {		//	Thief allowed to go through doors to which player has key.
				if (wallP->keys != KEY_NONE)
					if (wallP->keys & gameData.multi.players [gameData.multi.nLocalPlayer].flags)
						WallOpenDoor (gameData.segs.segments + hitseg, hitwall);
			}
		}
	}

	return;
}

//##void CollideHostageAnd_wall (tObject * hostage, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)	{
//##	return;
//##}

//	-------------------------------------------------------------------------------------------------------------

int ApplyDamageToClutter (tObject *clutter, fix damage)
{
if (clutter->flags&OF_EXPLODING)
	return 0;
if (clutter->shields < 0) 
	return 0;	//clutter already dead...
clutter->shields -= damage;
if (clutter->shields < 0) {
	ExplodeObject (clutter, 0);
	return 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------

//given the specified vForce, apply damage from that vForce to an tObject
void ApplyForceDamage (tObject *objP, fix vForce, tObject *otherObjP)
{
	int	result;
	fix damage;

if (objP->flags & (OF_EXPLODING|OF_SHOULD_BE_DEAD))
	return;		//already exploding or dead
damage = FixDiv (vForce, objP->mType.physInfo.mass) / 8;
if ((otherObjP->nType == OBJ_PLAYER) && gameStates.app.cheats.bMonsterMode)
	damage = 0x7fffffff;
	switch (objP->nType) {
		case OBJ_ROBOT:
			if (gameData.bots.pInfo [objP->id].attackType == 1) {
				if (otherObjP->nType == OBJ_WEAPON)
					result = ApplyDamageToRobot (objP, damage/4, otherObjP->cType.laserInfo.nParentObj);
				else
					result = ApplyDamageToRobot (objP, damage/4, OBJ_IDX (otherObjP));
				}
			else {
				if (otherObjP->nType == OBJ_WEAPON)
					result = ApplyDamageToRobot (objP, damage/2, otherObjP->cType.laserInfo.nParentObj);
				else
					result = ApplyDamageToRobot (objP, damage/2, OBJ_IDX (otherObjP));
				}		
			if (result && (otherObjP->cType.laserInfo.nParentSig == gameData.objs.console->nSignature))
				AddPointsToScore (gameData.bots.pInfo [objP->id].scoreValue);
			break;

		case OBJ_PLAYER:
			//	If colliding with a claw nType robot, do damage proportional to gameData.time.xFrame because you can Collide with those
			//	bots every frame since they don't move.
			if ((otherObjP->nType == OBJ_ROBOT) && (gameData.bots.pInfo [otherObjP->id].attackType))
				damage = FixMul (damage, gameData.time.xFrame*2);
			//	Make trainee easier.
			if (gameStates.app.nDifficultyLevel == 0)
				damage /= 2;
			ApplyDamageToPlayer (objP, otherObjP, damage);
			break;

		case OBJ_CLUTTER:
			ApplyDamageToClutter (objP, damage);
			break;

		case OBJ_CNTRLCEN:
			ApplyDamageToReactor (objP, damage, (short) OBJ_IDX (otherObjP));
			break;

		case OBJ_WEAPON:
			break;		//weapons don't take damage

		default:
			Int3 ();
		}
}

//	-----------------------------------------------------------------------------

short nMonsterballForces [100];

short nMonsterballPyroForce;

void SetMonsterballForces (void)
{
	int	i, h = IsMultiGame;
	tMonsterballForce *forceP = extraGameInfo [IsMultiGame].monsterball.forces;

memset (nMonsterballForces, 0, sizeof (nMonsterballForces));
for (i = 0; i < MAX_MONSTERBALL_FORCES - 1; i++, forceP++)
	nMonsterballForces [forceP->nWeaponId] = 	forceP->nForce;
nMonsterballPyroForce = forceP->nForce;
gameData.objs.pwrUp.info [POW_MONSTERBALL].size = 
	(gameData.objs.pwrUp.info [POW_SHIELD_BOOST].size * extraGameInfo [IsMultiGame].monsterball.nSizeMod) / 2;
}

//	-----------------------------------------------------------------------------

void BumpThisObject (tObject *objP, tObject *otherObjP, vmsVector *vForce, int bDamage)
{
	fix			xForceMag;
	vmsVector	vRotForce;

if (!(objP->mType.physInfo.flags & PF_PERSISTENT)) {
	if (objP->nType == OBJ_PLAYER) {
		if (otherObjP->nType == OBJ_MONSTERBALL) {
			double mq;
			
			mq = (double) otherObjP->mType.physInfo.mass / ((double) objP->mType.physInfo.mass * (double) nMonsterballPyroForce);
			vRotForce.x = (fix) ((double) vForce->x * mq);
			vRotForce.y = (fix) ((double) vForce->y * mq);
			vRotForce.z = (fix) ((double) vForce->z * mq);
			PhysApplyForce (objP, vForce);
			//PhysApplyRot (objP, &vRotForce);
			}
		else {
			vmsVector force2;
			force2.x = vForce->x / 4;
			force2.y = vForce->y / 4;
			force2.z = vForce->z / 4;
			PhysApplyForce (objP, &force2);
			if (bDamage && ((otherObjP->nType != OBJ_ROBOT) || !gameData.bots.pInfo [otherObjP->id].companion)) {
				xForceMag = VmVecMagQuick (&force2);
				ApplyForceDamage (objP, xForceMag, otherObjP);
				}
			}
		}
	else {
		if (objP->nType == OBJ_ROBOT) {
			if (gameData.bots.pInfo [objP->id].bossFlag)
				return;
			vRotForce.x = vForce->x / (4 + gameStates.app.nDifficultyLevel);
			vRotForce.y = vForce->y / (4 + gameStates.app.nDifficultyLevel);
			vRotForce.z = vForce->z / (4 + gameStates.app.nDifficultyLevel);
			PhysApplyForce (objP, vForce);
			PhysApplyRot (objP, &vRotForce);
			}
		else if ((objP->nType == OBJ_CLUTTER) || (objP->nType == OBJ_CNTRLCEN)) {
			vRotForce.x = vForce->x / (4 + gameStates.app.nDifficultyLevel);
			vRotForce.y = vForce->y / (4 + gameStates.app.nDifficultyLevel);
			vRotForce.z = vForce->z / (4 + gameStates.app.nDifficultyLevel);
			PhysApplyForce (objP, vForce);
			PhysApplyRot (objP, &vRotForce);
			}	
		else if (objP->nType == OBJ_MONSTERBALL) {
			double mq;
			
			if (otherObjP->nType == OBJ_PLAYER) {
				gameData.hoard.nLastHitter = OBJ_IDX (otherObjP);
				mq = ((double) otherObjP->mType.physInfo.mass * (double) nMonsterballPyroForce) / (double) objP->mType.physInfo.mass;
				}
			else {
				gameData.hoard.nLastHitter = otherObjP->cType.laserInfo.nParentObj;
				mq = (double) nMonsterballForces [otherObjP->id] * ((double) F1_0 / (double) otherObjP->mType.physInfo.mass) / 40.0;
				}
			vRotForce.x = (fix) ((double) vForce->x * mq);
			vRotForce.y = (fix) ((double) vForce->y * mq);
			vRotForce.z = (fix) ((double) vForce->z * mq);
			PhysApplyForce (objP, &vRotForce);
			PhysApplyRot (objP, &vRotForce);
			if (gameData.hoard.nLastHitter == gameData.multi.players [gameData.multi.nLocalPlayer].nObject)
				MultiSendMonsterball (1, 0);
			}
		else
			return;
		if (bDamage) {
			xForceMag = VmVecMagQuick (vForce);
			ApplyForceDamage (objP, xForceMag, otherObjP);
			}
		}
	}
}

//	-----------------------------------------------------------------------------
//deal with two gameData.objs.objects bumping into each other.  Apply vForce from collision
//to each robot.  The flags tells whether the objects should take damage from
//the collision.
int BumpTwoObjects (tObject *objP0, tObject *objP1, int bDamage, vmsVector *vHitPt)
{
	vmsVector	vForce, p0, p1, v0, v1, vh, vr, vn0, vn1;
	fix			mag, dot, m0, m1;
	tObject		*t;
	int			bCollide = 0;

if (objP0->movementType != MT_PHYSICS)
	t = objP1;
else if (objP1->movementType != MT_PHYSICS)
	t = objP0;
else
	t = NULL;
if (t) {
	Assert (t->movementType == MT_PHYSICS);
	VmVecCopyScale (&vForce, &t->mType.physInfo.velocity, -t->mType.physInfo.mass);
#ifdef _DEBUG
	if (vForce.x || vForce.y || vForce.z)
#endif
	PhysApplyForce (t, &vForce);
	return 1;
	}
p0 = objP0->pos;
p1 = objP1->pos;
v0 = objP0->mType.physInfo.velocity;
v1 = objP1->mType.physInfo.velocity;
m0 = VmVecCopyNormalize (&vn0, &v0);
m1 = VmVecCopyNormalize (&vn1, &v1);
if (m0 && m1)
	if (m0 > m1) {
		VmVecScaleFrac (&vn0, m1, m0);
		VmVecScaleFrac (&vn1, m1, m0);
		}
	else {
		VmVecScaleFrac (&vn0, m0, m1);
		VmVecScaleFrac (&vn1, m0, m1);
		}
VmVecSub (&vh, &p0, &p1);
m0 = VmVecMag (&vh);
if (m0 > ((objP0->size + objP1->size) * 3) / 4) {
	VmVecInc (&p0, &vn0);
	VmVecInc (&p1, &vn1);
	VmVecSub (&vh, &p0, &p1);
	m1 = VmVecMag (&vh);
	if (m1 > m0) {
#if 0//def _DEBUG
		HUDMessage (0, "moving away (%d, %d)", m0, m1);
#endif
		return 0;
		}
	}
#if 0//def _DEBUG
HUDMessage (0, "colliding (%1.2f, %1.2f)", f2fl (m0), f2fl (m1));
#endif
VmVecSub (&vForce, &v0, &v1);
m0 = objP0->mType.physInfo.mass;
m1 = objP1->mType.physInfo.mass;
if (!((m0 + m1) && FixMul (m0, m1))) {
#if 0//def _DEBUG
	HUDMessage (0, "Invalid Mass!!!");
#endif
	return 0;
	}
VmVecScaleFrac (&vForce, 2 * FixMul (m0, m1), m0 + m1);
mag = VmVecMag (&vForce);
if (mag < (m0 + m1) / 200) {
#if 0//def _DEBUG
	HUDMessage (0, "bump force too low");
#endif
	return 0;	// don't bump if force too low
	}
//HUDMessage (0, "%d %d", mag, (objP0->mType.physInfo.mass + objP1->mType.physInfo.mass) / 200);
if (EGI_FLAG (bUseHitAngles, 0, 0)) {
	// exert force in the direction of the hit point to the tObject's center
	VmVecSub (&vh, vHitPt, &objP1->pos);
	if (VmVecNormalize (&vh) > F1_0 / 16) {
		vr = vh;
		VmVecNegate (&vh);
		VmVecScale (&vh, mag);
		BumpThisObject (objP1, objP0, &vh, bDamage);
		// compute reflection vector. The vector from the other tObject's center to the hit point 
		// serves as normal.
		v1 = v0;
		VmVecNormalize (&v1);
		dot = VmVecDot (&v1, &vr);
		VmVecScale (&vr, 2 * dot);
		VmVecDec (&vr, &v0);
		VmVecNormalize (&vr);
		VmVecScale (&vr, mag);
		VmVecNegate (&vr);
		BumpThisObject (objP0, objP1, &vr, bDamage);
		return 1;
		}
	}
BumpThisObject (objP1, objP0, &vForce, 0);
VmVecNegate (&vForce);
BumpThisObject (objP0, objP1, &vForce, 0);
return 1;
}

//	-----------------------------------------------------------------------------

void BumpOneObject (tObject *objP0, vmsVector *hit_dir, fix damage)
{
	vmsVector	hit_vec;

hit_vec = *hit_dir;
VmVecScale (&hit_vec, damage);
PhysApplyForce (objP0, &hit_vec);
}

//	-----------------------------------------------------------------------------

#define DAMAGE_SCALE 		128	//	Was 32 before 8:55 am on Thursday, September 15, changed by MK, walls were hurting me more than robots!
#define DAMAGE_THRESHOLD 	 (F1_0/3)
#define WALL_LOUDNESS_SCALE (20)

fix force_force = i2f (50);

void CollidePlayerAndWall (tObject * playerObjP, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)
{
	fix damage;
	char ForceFieldHit=0;
	int nBaseTex, nOvlTex;

if (playerObjP->id != gameData.multi.nLocalPlayer) // Execute only for local player
	return;
nBaseTex = gameData.segs.segments [hitseg].sides [hitwall].nBaseTex;
//	If this wall does damage, don't make *BONK* sound, we'll be making another sound.
if (gameData.pig.tex.pTMapInfo [nBaseTex].damage > 0)
	return;
if (gameData.pig.tex.pTMapInfo [nBaseTex].flags & TMI_FORCE_FIELD) {
	vmsVector vForce;
	PALETTE_FLASH_ADD (0, 0, 60);	//flash blue
	//knock player around
	vForce.x = 40 * (d_rand () - 16384);
	vForce.y = 40 * (d_rand () - 16384);
	vForce.z = 40 * (d_rand () - 16384);
	PhysApplyRot (playerObjP, &vForce);
#ifdef TACTILE
	if (TactileStick)
		Tactile_apply_force (&vForce, &playerObjP->orient);
#endif
	//make sound
	DigiLinkSoundToPos (SOUND_FORCEFIELD_BOUNCE_PLAYER, hitseg, 0, vHitPt, 0, f1_0);
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI)
		MultiSendPlaySound (SOUND_FORCEFIELD_BOUNCE_PLAYER, f1_0);
#endif
	ForceFieldHit=1;
	} 
else {
#ifdef TACTILE
	vmsVector vForce;
	if (TactileStick) {
		vForce.x = -playerObjP->mType.physInfo.velocity.x;
		vForce.y = -playerObjP->mType.physInfo.velocity.y;
		vForce.z = -playerObjP->mType.physInfo.velocity.z;
		Tactile_do_collide (&vForce, &playerObjP->orient);
	}
#endif
   WallHitProcess (gameData.segs.segments + hitseg, hitwall, 20, playerObjP->id, playerObjP);
	}
if (gameStates.app.bD2XLevel && (gameData.segs.segment2s [hitseg].special == SEGMENT_IS_NODAMAGE))
	return;
//	** Damage from hitting wall **
//	If the player has less than 10% shields, don't take damage from bump
// Note: Does quad damage if hit a vForce field - JL
damage = (hitspeed / DAMAGE_SCALE) * (ForceFieldHit * 8 + 1);
nOvlTex = gameData.segs.segments [hitseg].sides [hitwall].nOvlTex;
//don't do wall damage and sound if hit lava or water
if ((gameData.pig.tex.pTMapInfo [nBaseTex].flags & (TMI_WATER|TMI_VOLATILE)) || 
		(nOvlTex && (gameData.pig.tex.pTMapInfo [nOvlTex].flags & (TMI_WATER|TMI_VOLATILE))))
	damage = 0;
if (damage >= DAMAGE_THRESHOLD) {
	int	volume = (hitspeed- (DAMAGE_SCALE*DAMAGE_THRESHOLD)) / WALL_LOUDNESS_SCALE ;
	CreateAwarenessEvent (playerObjP, PA_WEAPON_WALL_COLLISION);
	if (volume > F1_0)
		volume = F1_0;
	if (volume > 0 && !ForceFieldHit) {  // uhhhgly hack
		DigiLinkSoundToPos (SOUND_PLAYER_HIT_WALL, hitseg, 0, vHitPt, 0, volume);
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
			MultiSendPlaySound (SOUND_PLAYER_HIT_WALL, volume);	
#endif
		}
	if (!(gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE))
		if (gameData.multi.players [gameData.multi.nLocalPlayer].shields > f1_0*10 || ForceFieldHit)
			ApplyDamageToPlayer (playerObjP, playerObjP, damage);
	}
return;
}

//	-----------------------------------------------------------------------------

fix	Last_volatile_scrapeSoundTime = 0;

int CollideWeaponAndWall (tObject * weapon, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt);
int CollideDebrisAndWall (tObject * debris, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt);

//see if wall is volatile or water
//if volatile, cause damage to player  
//returns 1=lava, 2=water
int CheckVolatileWall (tObject *objP, int nSegment, int nSide, vmsVector *vHitPt)
{
	fix	d, water;
	int	nTexture;

Assert (objP->nType == OBJ_PLAYER);
nTexture = gameData.segs.segments [nSegment].sides [nSide].nBaseTex;
d = gameData.pig.tex.pTMapInfo [nTexture].damage;
water = (gameData.pig.tex.pTMapInfo [nTexture].flags & TMI_WATER);
if (d > 0 || water) {
	if (objP->id == gameData.multi.nLocalPlayer) {
		if (d > 0) {
			fix damage = FixMul (d, gameData.time.xFrame);
			if (gameStates.app.nDifficultyLevel == 0)
				damage /= 2;
			if (!(gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE))
				ApplyDamageToPlayer (objP, objP, damage);

#ifdef TACTILE
			if (TactileStick)
				Tactile_Xvibrate (50, 25);
#endif

			PALETTE_FLASH_ADD (f2i (damage*4), 0, 0);	//flash red
			}
		objP->mType.physInfo.rotVel.x = (d_rand () - 16384)/2;
		objP->mType.physInfo.rotVel.z = (d_rand () - 16384)/2;
		}
	return (d > 0) ? 1 : 2;
	}
else {
#ifdef TACTILE
	if (TactileStick && !(gameData.app.nFrameCount & 15))
		 Tactile_Xvibrate_clear ();
#endif
	return 0;
	}
}

//	-----------------------------------------------------------------------------

int CheckVolatileSegment (tObject *objP, int nSegment)
{
	fix d;

//	Assert (objP->nType==OBJ_PLAYER);

	if (!EGI_FLAG (bFluidPhysics, 0, 0))
		return 0;
	if (gameData.segs.segment2s [nSegment].special == SEGMENT_IS_WATER)
		d = 0;
	else if (gameData.segs.segment2s [nSegment].special == SEGMENT_IS_LAVA)
		d = gameData.pig.tex.tMapInfo [0][404].damage;
	else {
#ifdef TACTILE
		if (TactileStick && !(gameData.app.nFrameCount & 15))
			Tactile_Xvibrate_clear ();
#endif
		return 0;
		}
	if (d > 0) {
		fix damage = FixMul (d, gameData.time.xFrame) / 2;
		if (gameStates.app.nDifficultyLevel == 0)
			damage /= 2;
		if (objP->nType == OBJ_PLAYER) {
			if (!(gameData.multi.players [objP->id].flags & PLAYER_FLAGS_INVULNERABLE))
				ApplyDamageToPlayer (objP, objP, damage);
			}
		if (objP->nType == OBJ_ROBOT) {
			ApplyDamageToRobot (objP, objP->shields + 1, OBJ_IDX (objP));
			}

#ifdef TACTILE
		if (TactileStick)
			Tactile_Xvibrate (50, 25);
#endif
	if ((objP->nType == OBJ_PLAYER) && (objP->id == gameData.multi.nLocalPlayer))
		PALETTE_FLASH_ADD (f2i (damage*4), 0, 0);	//flash red
	if ((objP->nType == OBJ_PLAYER) || (objP->nType == OBJ_ROBOT)) {
		objP->mType.physInfo.rotVel.x = (d_rand () - 16384) / 2;
		objP->mType.physInfo.rotVel.z = (d_rand () - 16384) / 2;
		}
	return (d > 0) ? 1 : 2;
	}
return 0;
}

//	-----------------------------------------------------------------------------
//this gets called when an tObject is scraping along the wall
void ScrapeObjectOnWall (tObject *objP, short hitseg, short hitside, vmsVector * vHitPt)
{
	switch (objP->nType) {

		case OBJ_PLAYER:

			if (objP->id==gameData.multi.nLocalPlayer) {
				int nType=CheckVolatileWall (objP, hitseg, hitside, vHitPt);

				if (nType!=0) {
					vmsVector	hit_dir, rand_vec;

					if ((gameData.time.xGame > Last_volatile_scrapeSoundTime + F1_0/4) || 
						 (gameData.time.xGame < Last_volatile_scrapeSoundTime)) {
						short sound = (nType==1)?SOUND_VOLATILE_WALL_HISS:SOUND_SHIP_IN_WATER;

						Last_volatile_scrapeSoundTime = gameData.time.xGame;

						DigiLinkSoundToPos (sound, hitseg, 0, vHitPt, 0, F1_0);
#ifdef NETWORK
						if (gameData.app.nGameMode & GM_MULTI)
							MultiSendPlaySound (sound, F1_0);
#endif
					}

					#ifdef COMPACT_SEGS
						GetSideNormal (&gameData.segs.segments [hitseg], hitside, 0, &hit_dir);	
					#else
						hit_dir = gameData.segs.segments [hitseg].sides [hitside].normals [0];
					#endif
			
					MakeRandomVector (&rand_vec);
					VmVecScaleInc (&hit_dir, &rand_vec, F1_0/8);
					VmVecNormalizeQuick (&hit_dir);
					BumpOneObject (objP, &hit_dir, F1_0*8);
				}

				//@@} else {
				//@@	//what scrape sound
				//@@	//PLAY_SOUND (SOUND_PLAYER_SCRAPE_WALL);
				//@@}
		
			}

			break;

		//these two kinds of gameData.objs.objects below shouldn't really slide, so
		//if this scrape routine gets called (which it might if the
		//tObject (such as a fusion blob) was created already poking
		//through the wall) call the Collide routine.

		case OBJ_WEAPON:
			CollideWeaponAndWall (objP, 0, hitseg, hitside, vHitPt); 
			break;

		case OBJ_DEBRIS:		
			CollideDebrisAndWall (objP, 0, hitseg, hitside, vHitPt); 
			break;
	}

}

//	-----------------------------------------------------------------------------
//if an effect is hit, and it can blow up, then blow it up
//returns true if it blew up
int CheckEffectBlowup (tSegment *seg, short tSide, vmsVector *pnt, tObject *blower, int bForceBlowup)
{
	int tm, tmf, ec, db;
	int nWall, trig_num;

	db=0;

	//	If this wall has a tTrigger and the blower-upper is not the player or the buddy, abort!
	{
		int	ok_to_blow = 0;

		if (blower->cType.laserInfo.parentType == OBJ_ROBOT)
			if (gameData.bots.pInfo [gameData.objs.objects [blower->cType.laserInfo.nParentObj].id].companion)
				ok_to_blow = 1;

		if (!(ok_to_blow || (blower->cType.laserInfo.parentType == OBJ_PLAYER))) {
			int	nWall;
			nWall = WallNumP (seg, tSide);
			if (IS_WALL (nWall)) {
				if (gameData.walls.walls [nWall].nTrigger < gameData.trigs.nTriggers)
					return 0;
			}
		}
	}


	if ((tm = seg->sides [tSide].nOvlTex)) {

		eclip *ecP;
		int bOneShot;

		tmf = seg->sides [tSide].nOvlOrient;		//tm flags
		ec = gameData.pig.tex.pTMapInfo [tm].eclip_num;
		ecP = (ec < 0) ? NULL : gameData.eff.pEffects + ec;
		db = ecP ? ecP->dest_bm_num : -1;
		bOneShot = ecP ? (ecP->flags & EF_ONE_SHOT) != 0 : 0;
		//check if it's an animation (monitor) or casts light
		if (((ec != -1) && (db != -1) && !bOneShot) ||	
			 ((ec == -1) && (gameData.pig.tex.pTMapInfo [tm].destroyed != -1))) {
			fix u, v;
			grsBitmap *bmP = gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [tm].index;
			int x = 0, y = 0;

			PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [tm], gameStates.app.bD1Data);
			//this can be blown up...did we hit it?
			if (!bForceBlowup) {
				FindHitPointUV (&u, &v, NULL, pnt, seg, tSide, 0);	//evil: always say face zero
				bForceBlowup = !PixelTranspType (tm, tmf, u, v);
				}
			if (bForceBlowup) {		//not trans, thus on effect
				short nSound, bPermaTrigger;
				ubyte vc;
				fix xDestSize;

#ifdef NETWORK
				if ((gameData.app.nGameMode & GM_MULTI) && netGame.AlwaysLighting)
			   	if (ec == -1 || db == -1 || bOneShot)
				   	return (0);
#endif
				//note: this must get called before the texture changes, 
				//because we use the light value of the texture to change
				//the static light in the tSegment
				nWall = WallNumP (seg, tSide);
				bPermaTrigger = 
					IS_WALL (nWall) && 
					 ((trig_num = gameData.walls.walls [nWall].nTrigger) != NO_TRIGGER) &&
					 (gameData.trigs.triggers [trig_num].flags & TF_PERMANENT);
				if (!bPermaTrigger)
					SubtractLight (SEG_IDX (seg), tSide);

				if (gameData.demo.nState == ND_STATE_RECORDING)
					NDRecordEffectBlowup (SEG_IDX (seg), tSide, pnt);

				if (ec!=-1) {
					xDestSize = ecP->dest_size;
					vc = ecP->dest_vclip;
					}
				else {
					xDestSize = i2f (20);
					vc = 3;
					}
				ObjectCreateExplosion (SEG_IDX (seg), pnt, xDestSize, vc);
				if ((ec!=-1) && (db!=-1) && !bOneShot) {
					if ((nSound = gameData.eff.pVClips [vc].nSound) != -1)
		  				DigiLinkSoundToPos (nSound, SEG_IDX (seg), 0, pnt,  0, F1_0);
					if ((nSound = ecP->nSound) != -1)		//kill sound
						DigiKillSoundLinkedToSegment (SEG_IDX (seg), tSide, nSound);
					if (!bPermaTrigger && (ecP->dest_eclip != -1) && (gameData.eff.pEffects [ecP->dest_eclip].nSegment == -1)) {
						eclip	*new_ec = gameData.eff.pEffects + ecP->dest_eclip;
						int 	bm_num = new_ec->changing_wall_texture;
#if TRACE
						con_printf (CON_DEBUG, "bm_num = %d \n", bm_num);
#endif
						new_ec->time_left = EffectFrameTime (new_ec);
						new_ec->nCurFrame = 0;
						new_ec->nSegment = SEG_IDX (seg);
						new_ec->nSide = tSide;
						new_ec->flags |= EF_ONE_SHOT;
						new_ec->dest_bm_num = ecP->dest_bm_num;

						Assert (bm_num!=0 && seg->sides [tSide].nOvlTex!=0);
						seg->sides [tSide].nOvlTex = bm_num;		//replace with destoyed
					}
					else {
						Assert (db!=0 && seg->sides [tSide].nOvlTex!=0);
						if (!bPermaTrigger)
							seg->sides [tSide].nOvlTex = db;		//replace with destoyed
					}
				}
				else {
					if (!bPermaTrigger)
						seg->sides [tSide].nOvlTex = gameData.pig.tex.pTMapInfo [tm].destroyed;

					//assume this is a light, and play light sound
		  			DigiLinkSoundToPos (SOUND_LIGHT_BLOWNUP, SEG_IDX (seg), 0, pnt,  0, F1_0);
				}


				return 1;		//blew up!
			}
		}
	}

	return 0;		//didn't blow up
}

//	Copied from laser.c!
#define	MIN_OMEGA_BLOBS		3				//	No matter how close the obstruction, at this many blobs created.
#define	MIN_OMEGA_DIST			 (F1_0*3)		//	At least this distance between blobs, unless doing so would violate MIN_OMEGA_BLOBS
#define	DESIRED_OMEGA_DIST	 (F1_0*5)		//	This is the desired distance between blobs.  For distances > MIN_OMEGA_BLOBS*DESIRED_OMEGA_DIST, but not very large, this will apply.
#define	MAX_OMEGA_BLOBS		16				//	No matter how far away the obstruction, this is the maximum number of blobs.
#define	MAX_OMEGA_DIST			 (MAX_OMEGA_BLOBS * DESIRED_OMEGA_DIST)		//	Maximum extent of lightning blobs.

//	-------------------------------------------------
//	Return true if ok to do Omega damage.
int OkToDoOmegaDamage (tObject *weapon)
{
	int	parent_sig = weapon->cType.laserInfo.nParentSig;
	int	nParentObj = weapon->cType.laserInfo.nParentObj;

	if (!(gameData.app.nGameMode & GM_MULTI))
		return 1;

	if (gameData.objs.objects [nParentObj].nSignature != parent_sig) {
#if TRACE
		con_printf (CON_DEBUG, "Parent of omega blob not consistent with tObject information. \n");
#endif
		}
	else {
		fix	dist = VmVecDistQuick (&gameData.objs.objects [nParentObj].pos, &weapon->pos);

		if (dist > MAX_OMEGA_DIST) {
			return 0;
		} else
			;
	}

	return 1;
}

//	-----------------------------------------------------------------------------
//these gets added to the weapon's values when the weapon hits a volitle wall
#define VOLATILE_WALL_EXPL_STRENGTH i2f (10)
#define VOLATILE_WALL_IMPACT_SIZE	i2f (3)
#define VOLATILE_WALL_DAMAGE_FORCE	i2f (5)
#define VOLATILE_WALL_DAMAGE_RADIUS	i2f (30)

// int Show_segAnd_side = 0;

int CollideWeaponAndWall (
	tObject * weapon, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)
{
	tSegment *segP = gameData.segs.segments + hitseg;
	tSide *sideP = segP->sides + hitwall;
	tWeaponInfo *wInfoP = gameData.weapons.info + weapon->id;
	tObject *wObjP = gameData.objs.objects + weapon->cType.laserInfo.nParentObj;

	int bBlewUp;
	int wallType;
	int playernum;
	int	robot_escort;

if (weapon->id == OMEGA_ID)
	if (!OkToDoOmegaDamage (weapon))
		return 1;

//	If this is a guided missile and it strikes fairly directly, clear bounce flag.
if (weapon->id == GUIDEDMISS_ID) {
	fix dot = VmVecDot (&weapon->orient.fVec, sideP->normals);
#if TRACE
	con_printf (CON_DEBUG, "Guided missile dot = %7.3f \n", f2fl (dot));
#endif
	if (dot < -F1_0/6) {
#if TRACE
		con_printf (CON_DEBUG, "Guided missile loses bounciness. \n");
#endif
		weapon->mType.physInfo.flags &= ~PF_BOUNCE;
	}
}

//if an energy weapon hits a forcefield, let it bounce
if ((gameData.pig.tex.pTMapInfo [sideP->nBaseTex].flags & TMI_FORCE_FIELD) &&
	 ((weapon->nType != OBJ_WEAPON) || wInfoP->energy_usage)) {

	//make sound
	DigiLinkSoundToPos (SOUND_FORCEFIELD_BOUNCE_WEAPON, hitseg, 0, vHitPt, 0, f1_0);
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI)
		MultiSendPlaySound (SOUND_FORCEFIELD_BOUNCE_WEAPON, f1_0);
#endif
	return 1;	//bail here. physics code will bounce this tObject
	}

#ifdef _DEBUG
if (keyd_pressed [KEY_LAPOSTRO])
	if (weapon->cType.laserInfo.nParentObj == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) {
		//	MK: Real pain when you need to know a segP:tSide and you've got quad lasers.
#if TRACE
		con_printf (CON_DEBUG, "Your laser hit at tSegment = %i, tSide = %i \n", hitseg, hitwall);
#endif
		HUDInitMessage ("Hit at tSegment = %i, tSide = %i", hitseg, hitwall);
		if (weapon->id < 4)
			SubtractLight (hitseg, hitwall);
		else if (weapon->id == FLARE_ID)
			AddLight (hitseg, hitwall);
		}
if ((weapon->mType.physInfo.velocity.x == 0) && 
	 (weapon->mType.physInfo.velocity.y == 0) && 
	 (weapon->mType.physInfo.velocity.z == 0)) {
	Int3 ();	//	Contact Matt: This is impossible.  A weapon with 0 velocity hit a wall, which doesn't move.
	return 1;
	}
#endif
bBlewUp = CheckEffectBlowup (segP, hitwall, vHitPt, weapon, 0);
if ((weapon->cType.laserInfo.parentType == OBJ_ROBOT) && 
		(gameData.bots.pInfo [wObjP->id].companion==1)) {
	robot_escort = 1;
	if (gameData.app.nGameMode & GM_MULTI) {
		Int3 ();  // Get Jason!
	   return 1;
	   }	
playernum = gameData.multi.nLocalPlayer;		//if single player, he's the player's buddy
	}
else {
	robot_escort = 0;
	if (wObjP->nType == OBJ_PLAYER)
		playernum = wObjP->id;
	else
		playernum = -1;		//not a player (thus a robot)
	}
if (bBlewUp) {		//could be a wall switch
	//for wall triggers, always say that the player shot it out.  This is
	//because robots can shoot out wall triggers, and so the tTrigger better
	//take effect  
	//	NO -- Changed by MK, 10/18/95.  We don't want robots blowing puzzles.  Only player or buddy can open!
	CheckTrigger (segP, hitwall, weapon->cType.laserInfo.nParentObj, 1);
	}
if (weapon->id == EARTHSHAKER_ID)
	ShakerRockStuff ();
wallType = WallHitProcess (segP, hitwall, weapon->shields, playernum, weapon);
// Wall is volatile if either tmap 1 or 2 is volatile
if ((gameData.pig.tex.pTMapInfo [sideP->nBaseTex].flags & TMI_VOLATILE) || 
		(sideP->nOvlTex && 
		(gameData.pig.tex.pTMapInfo [sideP->nOvlTex].flags & TMI_VOLATILE))) {
	ubyte tVideoClip;
	//we've hit a volatile wall
	DigiLinkSoundToPos (SOUND_VOLATILE_WALL_HIT, hitseg, 0, vHitPt, 0, F1_0);
	//for most weapons, use volatile wall hit.  For mega, use its special tVideoClip
	tVideoClip = (weapon->id == MEGA_ID)?wInfoP->robot_hit_vclip:VCLIP_VOLATILE_WALL_HIT;
	//	New by MK: If powerful badass, explode as badass, not due to lava, fixes megas being wimpy in lava.
	if (wInfoP->damage_radius >= VOLATILE_WALL_DAMAGE_RADIUS/2)
		ExplodeBadassWeapon (weapon, vHitPt);
	else
		ObjectCreateBadassExplosion (weapon, hitseg, vHitPt, 
			wInfoP->impact_size + VOLATILE_WALL_IMPACT_SIZE, 
			tVideoClip, 
			wInfoP->strength [gameStates.app.nDifficultyLevel]/4+VOLATILE_WALL_EXPL_STRENGTH, 	//	diminished by mk on 12/08/94, i was doing 70 damage hitting lava on lvl 1.
			wInfoP->damage_radius+VOLATILE_WALL_DAMAGE_RADIUS, 
			wInfoP->strength [gameStates.app.nDifficultyLevel]/2+VOLATILE_WALL_DAMAGE_FORCE, 
			weapon->cType.laserInfo.nParentObj);
	weapon->flags |= OF_SHOULD_BE_DEAD;		//make flares die in lava
	}
else if ((gameData.pig.tex.pTMapInfo [sideP->nBaseTex].flags & TMI_WATER) || 
			(sideP->nOvlTex && (gameData.pig.tex.pTMapInfo [sideP->nOvlTex].flags & TMI_WATER))) {
	//we've hit water
	//	MK: 09/13/95: Badass in water is 1/2 normal intensity.
	if (wInfoP->matter) {
		DigiLinkSoundToPos (SOUND_MISSILE_HIT_WATER, hitseg, 0, vHitPt, 0, F1_0);
		if (wInfoP->damage_radius) {
			DigiLinkSoundToObject (SOUND_BADASS_EXPLOSION, OBJ_IDX (weapon), 0, F1_0);
			//	MK: 09/13/95: Badass in water is 1/2 normal intensity.
			ObjectCreateBadassExplosion (weapon, hitseg, vHitPt, 
				wInfoP->impact_size/2, 
				wInfoP->robot_hit_vclip, 
				wInfoP->strength [gameStates.app.nDifficultyLevel]/4, 
				wInfoP->damage_radius, 
				wInfoP->strength [gameStates.app.nDifficultyLevel]/2, 
				weapon->cType.laserInfo.nParentObj);
			}
		else
			ObjectCreateExplosion (weapon->nSegment, &weapon->pos, wInfoP->impact_size, wInfoP->wall_hit_vclip);
		} 
	else {
		DigiLinkSoundToPos (SOUND_LASER_HIT_WATER, hitseg, 0, vHitPt, 0, F1_0);
		ObjectCreateExplosion (weapon->nSegment, &weapon->pos, wInfoP->impact_size, VCLIP_WATER_HIT);
		}
	weapon->flags |= OF_SHOULD_BE_DEAD;		//make flares die in water
	}
else {
	if (weapon->mType.physInfo.flags & PF_BOUNCE) {
		//do special bound sound & effect
		}
	else {
		//if it's not the player's weapon, or it is the player's and there
		//is no wall, and no blowing up monitor, then play sound
		if ((weapon->cType.laserInfo.parentType != OBJ_PLAYER) ||	
				((!IS_WALL (WallNumS (sideP)) || wallType==WHP_NOT_SPECIAL) && !bBlewUp))
			if ((wInfoP->wall_hitSound > -1) && (!(weapon->flags & OF_SILENT)))
			DigiLinkSoundToPos (wInfoP->wall_hitSound, weapon->nSegment, 0, &weapon->pos, 0, F1_0);
		if (wInfoP->wall_hit_vclip > -1)	{
			if (wInfoP->damage_radius)
				ExplodeBadassWeapon (weapon, vHitPt);
			else
				ObjectCreateExplosion (weapon->nSegment, &weapon->pos, wInfoP->impact_size, wInfoP->wall_hit_vclip);
			}
		}
	}
//	If weapon fired by player or companion...
if ((weapon->cType.laserInfo.parentType== OBJ_PLAYER) || robot_escort) {
	if (!(weapon->flags & OF_SILENT) && 
		 (weapon->cType.laserInfo.nParentObj == gameData.multi.players [gameData.multi.nLocalPlayer].nObject))
		CreateAwarenessEvent (weapon, PA_WEAPON_WALL_COLLISION);			// tObject "weapon" can attract attention to player

//		if (weapon->id != FLARE_ID) {
//	We now allow flares to open doors.

	if (((weapon->id != FLARE_ID) || (weapon->cType.laserInfo.parentType != OBJ_PLAYER)) && !(weapon->mType.physInfo.flags & PF_BOUNCE))
		weapon->flags |= OF_SHOULD_BE_DEAD;

	//don't let flares stick in vForce fields
	if ((weapon->id == FLARE_ID) && (gameData.pig.tex.pTMapInfo [sideP->nBaseTex].flags & TMI_FORCE_FIELD))
		weapon->flags |= OF_SHOULD_BE_DEAD;
	if (!(weapon->flags & OF_SILENT)) {
		switch (wallType) {
			case WHP_NOT_SPECIAL:
				//should be handled above
				//DigiLinkSoundToPos (wInfoP->wall_hitSound, weapon->nSegment, 0, &weapon->pos, 0, F1_0);
				break;

			case WHP_NO_KEY:
				//play special hit door sound (if/when we get it)
				DigiLinkSoundToPos (SOUND_WEAPON_HIT_DOOR, weapon->nSegment, 0, &weapon->pos, 0, F1_0);
#ifdef NETWORK
			   if (gameData.app.nGameMode & GM_MULTI)
					MultiSendPlaySound (SOUND_WEAPON_HIT_DOOR, F1_0);
#endif
				break;

			case WHP_BLASTABLE:
				//play special blastable wall sound (if/when we get it)
				if ((wInfoP->wall_hitSound > -1) && (!(weapon->flags & OF_SILENT)))
					DigiLinkSoundToPos (SOUND_WEAPON_HIT_BLASTABLE, weapon->nSegment, 0, &weapon->pos, 0, F1_0);
				break;

			case WHP_DOOR:
				//don't play anything, since door open sound will play
				break;
			}
		} 
	}
else {
	// This is a robot's laser
	if (!(weapon->mType.physInfo.flags & PF_BOUNCE))
		weapon->flags |= OF_SHOULD_BE_DEAD;
	}
return 1;
}

//	-----------------------------------------------------------------------------
//##void CollideCameraAnd_wall (tObject * camera, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)	{
//##	return;
//##}

//##void CollidePowerupAnd_wall (tObject * powerup, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)	{
//##	return;
//##}

int CollideDebrisAndWall (tObject * debris, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)	
{	
ExplodeObject (debris, 0);
return 1;
}

//##void CollideFireballAndFireball (tObject * fireball1, tObject * fireball2, vmsVector *vHitPt) {
//##	return; 
//##}

//##void CollideFireballAndRobot (tObject * fireball, tObject * robot, vmsVector *vHitPt) {
//##	return; 
//##}

//##void CollideFireballAndHostage (tObject * fireball, tObject * hostage, vmsVector *vHitPt) {
//##	return; 
//##}

//##void CollideFireballAndPlayer (tObject * fireball, tObject * player, vmsVector *vHitPt) {
//##	return; 
//##}

//##void CollideFireballAndWeapon (tObject * fireball, tObject * weapon, vmsVector *vHitPt) { 
//##	//weapon->flags |= OF_SHOULD_BE_DEAD;
//##	return; 
//##}

//##void CollideFireballAndCamera (tObject * fireball, tObject * camera, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideFireballAndPowerup (tObject * fireball, tObject * powerup, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideFireballAndDebris (tObject * fireball, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-------------------------------------------------------------------------------------------------------------------

int CollideRobotAndRobot (tObject * robotP1, tObject * robotP2, vmsVector *vHitPt) 
{ 
//		robot1-gameData.objs.objects, f2i (robot1->pos.x), f2i (robot1->pos.y), f2i (robot1->pos.z), 
//		robot2-gameData.objs.objects, f2i (robot2->pos.x), f2i (robot2->pos.y), f2i (robot2->pos.z), 
//		f2i (vHitPt->x), f2i (vHitPt->y), f2i (vHitPt->z));

BumpTwoObjects (robotP1, robotP2, 1, vHitPt);
return 1; 
}

//	-----------------------------------------------------------------------------

int CollideRobotAndReactor (tObject * objP1, tObject * obj2, vmsVector *vHitPt)
{
if (objP1->nType == OBJ_ROBOT) {
	vmsVector	hitvec;
	VmVecSub (&hitvec, &obj2->pos, &objP1->pos);
	VmVecNormalizeQuick (&hitvec);
	BumpOneObject (objP1, &hitvec, 0);
	} 
else {
	vmsVector	hitvec;
	VmVecSub (&hitvec, &objP1->pos, &obj2->pos);
	VmVecNormalizeQuick (&hitvec);
	BumpOneObject (obj2, &hitvec, 0);
	}
return 1;
}

//	-----------------------------------------------------------------------------
//##void CollideRobotAndHostage (tObject * robot, tObject * hostage, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-----------------------------------------------------------------------------

fix Last_thief_hitTime;

int  CollideRobotAndPlayer (tObject *robot, tObject *playerObjP, vmsVector *vHitPt)
{ 
	int	steal_attempt = 0;
	short	collision_seg;

if (robot->flags & OF_EXPLODING)
	return 1;
collision_seg = FindSegByPoint (vHitPt, playerObjP->nSegment);
if (collision_seg != -1)
	ObjectCreateExplosion (collision_seg, vHitPt, gameData.weapons.info [0].impact_size, gameData.weapons.info [0].wall_hit_vclip);

if (playerObjP->id == gameData.multi.nLocalPlayer) {
	if (gameData.bots.pInfo [robot->id].companion)	//	Player and companion don't Collide.
		return 1;
	if (gameData.bots.pInfo [robot->id].kamikaze) {
		ApplyDamageToRobot (robot, robot->shields+1, OBJ_IDX (playerObjP));
		if (playerObjP == gameData.objs.console)
			AddPointsToScore (gameData.bots.pInfo [robot->id].scoreValue);
		}

	if (gameData.bots.pInfo [robot->id].thief) {
		if (gameData.ai.localInfo [OBJ_IDX (robot)].mode == AIM_THIEF_ATTACK) {
			Last_thief_hitTime = gameData.time.xGame;
			AttemptToStealItem (robot, playerObjP->id);
			steal_attempt = 1;
			} 
		else if (gameData.time.xGame - Last_thief_hitTime < F1_0*2)
			return 1;	//	ZOUNDS! BRILLIANT! Thief not Collide with player if not stealing!
							// NO! VERY DUMB! makes thief look very stupid if player hits him while cloaked!-AP
		else
			Last_thief_hitTime = gameData.time.xGame;
		}
	CreateAwarenessEvent (playerObjP, PA_PLAYER_COLLISION);			// tObject robot can attract attention to player
	DoAiRobotHitAttack (robot, playerObjP, vHitPt);
	DoAiRobotHit (robot, PA_WEAPON_ROBOT_COLLISION);
	} 
#ifdef NETWORK
else
	MultiRobotRequestChange (robot, playerObjP->id);
#endif
// added this if to remove the bump sound if it's the thief.
// A "steal" sound was added and it was getting obscured by the bump. -AP 10/3/95
//	Changed by MK to make this sound unless the robot stole.
if ((!steal_attempt) && !gameData.bots.pInfo [robot->id].energyDrain)
	DigiLinkSoundToPos (SOUND_ROBOT_HIT_PLAYER, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
BumpTwoObjects (robot, playerObjP, 1, vHitPt);
return 1; 
}

//	-----------------------------------------------------------------------------
// Provide a way for network message to instantly destroy the control center
// without awarding points or anything.

//	if controlcen == NULL, that means don't do the explosion because the control center
//	was actually in another tObject.
void NetDestroyReactor (tObject *controlcen)
{
	if (gameData.reactor.bDestroyed != 1) {

		DoReactorDestroyedStuff (controlcen);

		if ((controlcen != NULL) && !(controlcen->flags& (OF_EXPLODING|OF_DESTROYED))) {
			DigiLinkSoundToPos (SOUND_CONTROL_CENTER_DESTROYED, controlcen->nSegment, 0, &controlcen->pos, 0, F1_0);
			ExplodeObject (controlcen, 0);
		}
	}

}

//	-----------------------------------------------------------------------------

void ApplyDamageToReactor (tObject *controlcen, fix damage, short who)
{
	int	whotype;

	//	Only allow a player to damage the control center.

if ((who < 0) || (who > gameData.objs.nLastObject))
	return;
whotype = gameData.objs.objects [who].nType;
if (whotype != OBJ_PLAYER) {
#if TRACE
	con_printf (CON_DEBUG, "Damage to control center by tObject of nType %i prevented by MK! \n", whotype);
#endif
	return;
	}
#ifdef NETWORK
if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP) && 
	 (gameData.multi.players [gameData.multi.nLocalPlayer].timeLevel < netGame.control_invulTime)) {
	if (gameData.objs.objects [who].id == gameData.multi.nLocalPlayer) {
		int t = netGame.control_invulTime - gameData.multi.players [gameData.multi.nLocalPlayer].timeLevel;
		int secs = f2i (t) % 60;
		int mins = f2i (t) / 60;
		HUDInitMessage ("%s %d:%02d.", TXT_CNTRLCEN_INVUL, mins, secs);
		}
	return;
	}
#endif
if (gameData.objs.objects [who].id == gameData.multi.nLocalPlayer) {
	gameData.reactor.bHit = 1;
	AIDoCloakStuff ();
	}
if (controlcen->shields >= 0)
	controlcen->shields -= damage;
if ((controlcen->shields < 0) && !(controlcen->flags& (OF_EXPLODING|OF_DESTROYED))) {
	if (gameStates.app.bD2XLevel && gameStates.gameplay.bMultiBosses)
		extraGameInfo [0].nBossCount--;
	DoReactorDestroyedStuff (controlcen);
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI) {
		if (who == gameData.multi.players [gameData.multi.nLocalPlayer].nObject)
			AddPointsToScore (CONTROL_CEN_SCORE);
		MultiSendDestroyReactor (OBJ_IDX (controlcen), gameData.objs.objects [who].id);
		}
#endif
	if (!(gameData.app.nGameMode & GM_MULTI))
		AddPointsToScore (CONTROL_CEN_SCORE);
	DigiLinkSoundToPos (SOUND_CONTROL_CENTER_DESTROYED, controlcen->nSegment, 0, &controlcen->pos, 0, F1_0);
	ExplodeObject (controlcen, 0);
	}
}

//	-----------------------------------------------------------------------------

int CollidePlayerAndReactor (tObject * controlcen, tObject * playerObjP, vmsVector *vHitPt)
{ 
if (playerObjP->id == gameData.multi.nLocalPlayer) {
	gameData.reactor.bHit = 1;
	AIDoCloakStuff ();				//	In case player cloaked, make control center know where he is.
	}
if (BumpTwoObjects (controlcen, playerObjP, 1, vHitPt))
	DigiLinkSoundToPos (SOUND_ROBOT_HIT_PLAYER, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
return 1; 
}

//	-----------------------------------------------------------------------------

int CollidePlayerAndMarker (tObject * marker, tObject * playerObjP, vmsVector *vHitPt)
{ 
#if TRACE
con_printf (CON_DEBUG, "Collided with marker %d! \n", marker->id);
#endif
if (playerObjP->id==gameData.multi.nLocalPlayer) {
	int drawn;

	if (gameData.app.nGameMode & GM_MULTI)
		drawn = HUDInitMessage (TXT_MARKER_PLRMSG, gameData.multi.players [marker->id/2].callsign, gameData.marker.szMessage [marker->id]);
	else
		if (gameData.marker.szMessage [marker->id][0])
			drawn = HUDInitMessage (TXT_MARKER_IDMSG, marker->id+1, gameData.marker.szMessage [marker->id]);
		else
			drawn = HUDInitMessage (TXT_MARKER_ID, marker->id+1);
	if (drawn)
		DigiPlaySample (SOUND_MARKER_HIT, F1_0);
	DetectEscortGoalAccomplished (OBJ_IDX (marker));
   }
return 1;
}

//	-----------------------------------------------------------------------------
//	If a persistent weapon and other tObject is not a weapon, weaken it, else kill it.
//	If both gameData.objs.objects are weapons, weaken the weapon.
void MaybeKillWeapon (tObject *weapon, tObject *otherObjP)
{
if ((weapon->id == PROXIMITY_ID) || (weapon->id == SUPERPROX_ID) || (weapon->id == PMINE_ID)) {
	weapon->flags |= OF_SHOULD_BE_DEAD;
	return;
	}
//	Changed, 10/12/95, MK: Make weapon-weapon collisions always kill both weapons if not persistent.
//	Reason: Otherwise you can't use proxbombs to detonate incoming homing missiles (or mega missiles).
if (weapon->mType.physInfo.flags & PF_PERSISTENT) {
	//	Weapons do a lot of damage to weapons, other gameData.objs.objects do much less.
	if (!(weapon->mType.physInfo.flags & PF_PERSISTENT)) {
		if (otherObjP->nType == OBJ_WEAPON)
			weapon->shields -= otherObjP->shields/2;
		else
			weapon->shields -= otherObjP->shields/4;

		if (weapon->shields <= 0) {
			weapon->shields = 0;
			weapon->flags |= OF_SHOULD_BE_DEAD;	// weapon->lifeleft = 1;
			}
		}
	} 
else
	weapon->flags |= OF_SHOULD_BE_DEAD;	// weapon->lifeleft = 1;
}

//	-----------------------------------------------------------------------------

int CollideWeaponAndReactor (tObject * weapon, tObject *controlcen, vmsVector *vHitPt)
{
if (weapon->id == OMEGA_ID)
	if (!OkToDoOmegaDamage (weapon))
		return 1;
if (weapon->cType.laserInfo.parentType == OBJ_PLAYER) {
	fix	damage = weapon->shields;
	if (gameData.objs.objects [weapon->cType.laserInfo.nParentObj].id == gameData.multi.nLocalPlayer)
		gameData.reactor.bHit = 1;
	if (WI_damage_radius (weapon->id))
		ExplodeBadassWeapon (weapon, vHitPt);
	else
		ObjectCreateExplosion (controlcen->nSegment, vHitPt, controlcen->size*3/20, VCLIP_SMALL_EXPLOSION);
	DigiLinkSoundToPos (SOUND_CONTROL_CENTER_HIT, controlcen->nSegment, 0, vHitPt, 0, F1_0);
	damage = FixMul (damage, weapon->cType.laserInfo.multiplier);
	ApplyDamageToReactor (controlcen, damage, weapon->cType.laserInfo.nParentObj);
	MaybeKillWeapon (weapon, controlcen);
	} 
else {	//	If robot weapon hits control center, blow it up, make it go away, but do no damage to control center.
	ObjectCreateExplosion (controlcen->nSegment, vHitPt, controlcen->size*3/20, VCLIP_SMALL_EXPLOSION);
	MaybeKillWeapon (weapon, controlcen);
	}
return 1;
}

//	-----------------------------------------------------------------------------

int CollideWeaponAndClutter (tObject * weapon, tObject *clutter, vmsVector *vHitPt)	
{
ubyte exp_vclip = VCLIP_SMALL_EXPLOSION;
if (clutter->shields >= 0)
	clutter->shields -= weapon->shields;
DigiLinkSoundToPos (SOUND_LASER_HIT_CLUTTER, (short) weapon->nSegment, 0, vHitPt, 0, F1_0);
ObjectCreateExplosion ((short) clutter->nSegment, vHitPt, ((clutter->size/3)*3)/4, exp_vclip);
if ((clutter->shields < 0) && !(clutter->flags & (OF_EXPLODING|OF_DESTROYED)))
	ExplodeObject (clutter, STANDARD_EXPL_DELAY);
MaybeKillWeapon (weapon, clutter);
return 1;
}

//--mk, 121094 -- extern void spinRobot (tObject *robot, vmsVector *vHitPt);

extern tObject *ExplodeBadassObject (tObject *objP, fix damage, fix distance, fix vForce);

fix	nFinalBossCountdownTime = 0;

//	------------------------------------------------------------------------------------------------------

void DoFinalBossFrame (void)
{
if (!gameStates.gameplay.bFinalBossIsDead)
	return;
if (!gameData.reactor.bDestroyed)
	return;
if (nFinalBossCountdownTime == 0)
	nFinalBossCountdownTime = F1_0*2;
nFinalBossCountdownTime -= gameData.time.xFrame;
if (nFinalBossCountdownTime > 0)
	return;
GrPaletteFadeOut (NULL, 256, 0);
StartEndLevelSequence (0);		//pretend we hit the exit tTrigger
}

//	------------------------------------------------------------------------------------------------------
//	This is all the ugly stuff we do when you kill the final boss so that you don't die or something
//	which would ruin the logic of the cut sequence.
void DoFinalBossHacks (void)
{
if (gameStates.app.bPlayerIsDead) {
	Int3 ();		//	Uh-oh, player is dead.  Try to rescue him.
	gameStates.app.bPlayerIsDead = 0;
	}
if (gameData.multi.players [gameData.multi.nLocalPlayer].shields <= 0)
	gameData.multi.players [gameData.multi.nLocalPlayer].shields = 1;
//	If you're not invulnerable, get invulnerable!
if (!(gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE)) {
	gameData.multi.players [gameData.multi.nLocalPlayer].invulnerableTime = gameData.time.xGame;
	gameData.multi.players [gameData.multi.nLocalPlayer].flags |= PLAYER_FLAGS_INVULNERABLE;
	SetSpherePulse (gameData.multi.spherePulse + gameData.multi.nLocalPlayer, 0.02f, 0.5f);
	}
if (!(gameData.app.nGameMode & GM_MULTI))
	BuddyMessage ("Nice job, %s!", gameData.multi.players [gameData.multi.nLocalPlayer].callsign);
gameStates.gameplay.bFinalBossIsDead = 1;
}

extern int MultiAllPlayersAlive ();
void MultiSendFinishGame ();

//	------------------------------------------------------------------------------------------------------
//	Return 1 if robot died, else return 0
int ApplyDamageToRobot (tObject *robot, fix damage, int nKillerObj)
{
#ifdef NETWORK
char isthief;
char temp_stolen [MAX_STOLEN_ITEMS];	
#endif

if (robot->flags & OF_EXPLODING) 
	return 0;
if (robot->shields < 0) 
	return 0;	//robot already dead...
if (gameData.bots.pInfo [robot->id].bossFlag)
	gameData.boss.nHitTime = gameData.time.xGame;

//	Buddy invulnerable on level 24 so he can give you his important messages.  Bah.
//	Also invulnerable if his cheat for firing weapons is in effect.
if (gameData.bots.pInfo [robot->id].companion) {
//		if ((gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission && gameData.missions.nCurrentLevel == gameData.missions.nLastLevel) || gameStates.app.cheats.bMadBuddy)
#ifdef NETWORK
	if ((gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission && gameData.missions.nCurrentLevel == gameData.missions.nLastLevel))
		return 0;
#endif
	}

//	if (robot->controlType == CT_REMOTE)
//		return 0; // Can't damange a robot controlled by another player

// -- MK, 10/21/95, unused!--	if (gameData.bots.pInfo [robot->id].bossFlag)
//		Boss_been_hit = 1;

robot->shields -= damage;

//	Do unspeakable hacks to make sure player doesn't die after killing boss.  Or before, sort of.
if (gameData.bots.pInfo [robot->id].bossFlag) {
#ifdef NETWORK
	if ((gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission) && gameData.missions.nCurrentLevel == gameData.missions.nLastLevel) {
#endif
		if ((robot->shields < 0) && (extraGameInfo [0].nBossCount == 1)) {
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_MULTI) {
				if (!MultiAllPlayersAlive ()) // everyones gotta be alive
					robot->shields=1;
				else {
					MultiSendFinishGame ();
					DoFinalBossHacks ();
					}		
				}		
			else
#endif
				{	// NOTE LINK TO ABOVE!!!
				if ((gameData.multi.players [gameData.multi.nLocalPlayer].shields < 0) || gameStates.app.bPlayerIsDead)
					robot->shields = 1;		//	Sorry, we can't allow you to kill the final boss after you've died.  Rough luck.
				else
					DoFinalBossHacks ();
				}
			}
		}
	}

if (robot->shields < 0) {
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI) {
		isthief = (gameData.bots.pInfo [robot->id].thief != 0);
		if (isthief)
			memcpy (temp_stolen, gameData.thief.stolenItems, sizeof (*temp_stolen) * MAX_STOLEN_ITEMS);
		if (MultiExplodeRobotSub (OBJ_IDX (robot), nKillerObj, gameData.bots.pInfo [robot->id].thief)) {
			if (isthief)	
				memcpy (gameData.thief.stolenItems, temp_stolen, sizeof (*temp_stolen) * MAX_STOLEN_ITEMS);
		MultiSendRobotExplode (OBJ_IDX (robot), nKillerObj, gameData.bots.pInfo [robot->id].thief);
		if (isthief)	
			memset (gameData.thief.stolenItems, 255, sizeof (gameData.thief.stolenItems));
		return 1;
		}
	else
		return 0;
	}
#endif

	if (nKillerObj >= 0) {
		gameData.multi.players [gameData.multi.nLocalPlayer].numKillsLevel++;
		gameData.multi.players [gameData.multi.nLocalPlayer].numKillsTotal++;
		}

	if (gameData.bots.pInfo [robot->id].bossFlag) {
		start_boss_death_sequence (robot);	//DoReactorDestroyedStuff (NULL);
		}
	else if (gameData.bots.pInfo [robot->id].bDeathRoll) {
		StartRobotDeathSequence (robot);	//DoReactorDestroyedStuff (NULL);
		}
	else {
		if (robot->id == SPECIAL_REACTOR_ROBOT)
			SpecialReactorStuff ();
	//if (gameData.bots.pInfo [robot->id].smartBlobs)
	//	CreateSmartChildren (robot, gameData.bots.pInfo [robot->id].smartBlobs);
	//if (gameData.bots.pInfo [robot->id].badass)
	//	ExplodeBadassObject (robot, F1_0*gameData.bots.pInfo [robot->id].badass, F1_0*40, F1_0*150);
		ExplodeObject (robot, gameData.bots.pInfo [robot->id].kamikaze ? 1 : STANDARD_EXPL_DELAY);		//	Kamikaze, explode right away, IN YOUR FACE!
		}
	return 1;
	}
else
	return 0;
}

extern int BossSpewRobot (tObject *objP, vmsVector *pos);

//--ubyte	Boss_teleports [NUM_D2_BOSSES] = 				{1, 1, 1, 1, 1, 1};		// Set byte if this boss can teleport
//--ubyte	Boss_cloaks [NUM_D2_BOSSES] = 					{1, 1, 1, 1, 1, 1};		// Set byte if this boss can cloak
//--ubyte	Boss_spews_bots_energy [NUM_D2_BOSSES] = 	{1, 1, 0, 0, 1, 1};		//	Set byte if boss spews bots when hit by energy weapon.
//--ubyte	Boss_spews_bots_matter [NUM_D2_BOSSES] = 	{0, 0, 1, 0, 1, 1};		//	Set byte if boss spews bots when hit by matter weapon.
//--ubyte	Boss_invulnerable_energy [NUM_D2_BOSSES] = {0, 0, 1, 1, 0, 0};		//	Set byte if boss is invulnerable to energy weapons.
//--ubyte	Boss_invulnerable_matter [NUM_D2_BOSSES] = {0, 0, 0, 1, 0, 0};		//	Set byte if boss is invulnerable to matter weapons.
//--ubyte	Boss_invulnerable_spot [NUM_D2_BOSSES] = 	{0, 0, 0, 0, 1, 1};		//	Set byte if boss is invulnerable in all but a certain spot. (Dot product fVec|vec_to_collision < BOSS_INVULNERABLE_DOT)

//#define	BOSS_INVULNERABLE_DOT	0		//	If a boss is invulnerable over most of his body, fVec (dot)vec_to_collision must be less than this for damage to occur.
int	Boss_invulnerable_dot = 0;

int	Buddy_gave_hint_count = 5;
fix	LastTime_buddy_gave_hint = 0;

//	------------------------------------------------------------------------------------------------------
//	Return true if damage done to boss, else return false.
int DoBossWeaponCollision (tObject *robot, tObject *weapon, vmsVector *vHitPt)
{
	int	d2_boss_index;
	int	bDamage;
	int	bKinetic;

	bDamage = 1;

	d2_boss_index = gameData.bots.pInfo [robot->id].bossFlag - BOSS_D2;

	Assert ((d2_boss_index >= 0) && (d2_boss_index < NUM_D2_BOSSES));

	//	See if should spew a bot.
	if (weapon->cType.laserInfo.parentType == OBJ_PLAYER) {
		bKinetic = WI_matter (weapon->id);
		if ((bKinetic && bossProps [gameStates.app.bD1Mission][d2_boss_index].bSpewBotsKinetic) || 
		    (!bKinetic && bossProps [gameStates.app.bD1Mission][d2_boss_index].bSpewBotsEnergy)) {
			if (bossProps [gameStates.app.bD1Mission][d2_boss_index].bSpewMore && (d_rand () > 16384) &&
				 (BossSpewRobot (robot, vHitPt) != -1))
				gameData.boss.nLastGateTime = gameData.time.xGame - gameData.boss.nGateInterval - 1;	//	Force allowing spew of another bot.
			BossSpewRobot (robot, vHitPt);
		}
	}

	if (bossProps [gameStates.app.bD1Mission][d2_boss_index].bInvulSpot) {
		fix			dot;
		vmsVector	tvec1;

		//	Boss only vulnerable in back.  See if hit there.
		VmVecSub (&tvec1, vHitPt, &robot->pos);
		VmVecNormalizeQuick (&tvec1);	//	Note, if BOSS_INVULNERABLE_DOT is close to F1_0 (in magnitude), then should probably use non-quick version.
		dot = VmVecDot (&tvec1, &robot->orient.fVec);
#if TRACE
		con_printf (CON_DEBUG, "Boss hit vec dot = %7.3f \n", f2fl (dot));
#endif
		if (dot > Boss_invulnerable_dot) {
			short	new_obj;
			short	nSegment;

			nSegment = FindSegByPoint (vHitPt, robot->nSegment);
			DigiLinkSoundToPos (SOUND_WEAPON_HIT_DOOR, nSegment, 0, vHitPt, 0, F1_0);
			bDamage = 0;

			if (LastTime_buddy_gave_hint == 0)
				LastTime_buddy_gave_hint = d_rand ()*32 + F1_0*16;

			if (Buddy_gave_hint_count) {
				if (LastTime_buddy_gave_hint + F1_0*20 < gameData.time.xGame) {
					int	sval;

					Buddy_gave_hint_count--;
					LastTime_buddy_gave_hint = gameData.time.xGame;
					sval = (d_rand ()*4) >> 15;
					switch (sval) {
						case 0:	
							BuddyMessage (TXT_BOSS_HIT_BACK);	
							break;
						case 1:	
							BuddyMessage (TXT_BOSS_VULNERABLE);	
							break;
						case 2:	
							BuddyMessage (TXT_BOSS_GET_BEHIND);	
							break;
						case 3:
						default:
							BuddyMessage (TXT_BOSS_GLOW_SPOT);	
							break;
					}
				}
			}

			//	Cause weapon to bounce.
			//	Make a copy of this weapon, because the physics wants to destroy it.
			if (!WI_matter (weapon->id)) {
				new_obj = CreateObject (weapon->nType, weapon->id, -1, weapon->nSegment, &weapon->pos, 
												&weapon->orient, weapon->size, weapon->controlType, weapon->movementType, weapon->renderType, 1);

				if (new_obj != -1) {
					vmsVector	vec_to_point;
					vmsVector	weap_vec;
					fix			speed;
					tObject		*newObjP = gameData.objs.objects + new_obj;

					if (weapon->renderType == RT_POLYOBJ) {
						newObjP->rType.polyObjInfo.nModel = gameData.weapons.info [newObjP->id].nModel;
						newObjP->size = FixDiv (gameData.models.polyModels [newObjP->rType.polyObjInfo.nModel].rad, gameData.weapons.info [newObjP->id].po_len_to_width_ratio);
					}

					newObjP->mType.physInfo.mass = WI_mass (weapon->nType);
					newObjP->mType.physInfo.drag = WI_drag (weapon->nType);
					VmVecZero (&newObjP->mType.physInfo.thrust);

					VmVecSub (&vec_to_point, vHitPt, &robot->pos);
					VmVecNormalizeQuick (&vec_to_point);
					weap_vec = weapon->mType.physInfo.velocity;
					speed = VmVecNormalizeQuick (&weap_vec);
					VmVecScaleInc (&vec_to_point, &weap_vec, -F1_0*2);
					VmVecScale (&vec_to_point, speed/4);
					newObjP->mType.physInfo.velocity = vec_to_point;
				}
			}
		}
	} else if ((WI_matter (weapon->id) && bossProps [gameStates.app.bD1Mission][d2_boss_index].bInvulKinetic) || 
				  (!WI_matter (weapon->id) && bossProps [gameStates.app.bD1Mission][d2_boss_index].bInvulEnergy)) {
		short	nSegment;

		nSegment = FindSegByPoint (vHitPt, robot->nSegment);
		DigiLinkSoundToPos (SOUND_WEAPON_HIT_DOOR, nSegment, 0, vHitPt, 0, F1_0);
		bDamage = 0;
	}

	return bDamage;
}

//	------------------------------------------------------------------------------------------------------

int CollideRobotAndWeapon (tObject * robot, tObject * weapon, vmsVector *vHitPt)
{ 
	int	bDamage=1;
	int	boss_invulFlag=0;
	tRobotInfo *rInfoP = gameData.bots.pInfo + robot->id;
	tWeaponInfo *wInfoP = gameData.weapons.info + weapon->id;

if (weapon->id == OMEGA_ID)
	if (!OkToDoOmegaDamage (weapon))
		return 1;
if (rInfoP->bossFlag) {
	gameData.boss.nHitTime = gameData.time.xGame;
	if (rInfoP->bossFlag >= BOSS_D2) {
		bDamage = DoBossWeaponCollision (robot, weapon, vHitPt);
		boss_invulFlag = !bDamage;
		}
	}
//	Put in at request of Jasen (and Adam) because the Buddy-Bot gets in their way.
//	MK has so much fun whacking his butt around the mine he never cared...
if ((rInfoP->companion) && 
	 ((weapon->cType.laserInfo.parentType != OBJ_ROBOT) && 
	  !gameStates.app.cheats.bRobotsKillRobots))
	return 1;
if (weapon->id == EARTHSHAKER_ID)
	ShakerRockStuff ();
//	If a persistent weapon hit robot most recently, quick abort, else we cream the same robot many times, 
//	depending on frame rate.
if (weapon->mType.physInfo.flags & PF_PERSISTENT) {
	if (weapon->cType.laserInfo.nLastHitObj == OBJ_IDX (robot))
		return 1;
	weapon->cType.laserInfo.nLastHitObj = OBJ_IDX (robot);
	}
if (weapon->cType.laserInfo.nParentSig == robot->nSignature)
	return 1;
//	Changed, 10/04/95, put out blobs based on skill level and power of weapon doing damage.
//	Also, only a weapon hit from a player weapon causes smart blobs.
if ((weapon->cType.laserInfo.parentType == OBJ_PLAYER) && (rInfoP->energyBlobs))
	if ((robot->shields > 0) && bIsEnergyWeapon [weapon->id]) {
		int	num_blobs;
		fix	probval = (gameStates.app.nDifficultyLevel+2) * min (weapon->shields, robot->shields);
		probval = rInfoP->energyBlobs * probval/ (NDL*32);
		num_blobs = probval >> 16;
		if (2*d_rand () < (probval & 0xffff))
			num_blobs++;
		if (num_blobs)
			CreateSmartChildren (robot, num_blobs);
		}

	//	Note: If weapon hits an invulnerable boss, it will still do badass damage, including to the boss, 
	//	unless this is trapped elsewhere.
	if (WI_damage_radius (weapon->id)) {
		if (boss_invulFlag) {			//don't make badass sound
			//this code copied from ExplodeBadassWeapon ()
			ObjectCreateBadassExplosion (weapon, weapon->nSegment, vHitPt, 
							wInfoP->impact_size, 
							wInfoP->robot_hit_vclip, 
							wInfoP->strength [gameStates.app.nDifficultyLevel], 
							wInfoP->damage_radius, wInfoP->strength [gameStates.app.nDifficultyLevel], 
							weapon->cType.laserInfo.nParentObj);
		
			}
		else		//normal badass explosion
			ExplodeBadassWeapon (weapon, vHitPt);
		}
	if (((weapon->cType.laserInfo.parentType==OBJ_PLAYER) || 
		 gameStates.app.cheats.bRobotsKillRobots || 
		 (EGI_FLAG (bRobotsHitRobots, 0, 0))) && 
		 !(robot->flags & OF_EXPLODING))	{	
		tObject *expl_obj = NULL;
		if (weapon->cType.laserInfo.nParentObj == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) {
			CreateAwarenessEvent (weapon, PA_WEAPON_ROBOT_COLLISION);			// tObject "weapon" can attract attention to player
			DoAiRobotHit (robot, PA_WEAPON_ROBOT_COLLISION);
			}
#ifdef NETWORK
	  	else
			MultiRobotRequestChange (robot, gameData.objs.objects [weapon->cType.laserInfo.nParentObj].id);
#endif
		if (rInfoP->nExp1VClip > -1)
			expl_obj = ObjectCreateExplosion (weapon->nSegment, vHitPt, (robot->size/2*3)/4, (ubyte) rInfoP->nExp1VClip);
		else if (gameData.weapons.info [weapon->id].robot_hit_vclip > -1)
			expl_obj = ObjectCreateExplosion (weapon->nSegment, vHitPt, wInfoP->impact_size, (ubyte) wInfoP->robot_hit_vclip);
		if (expl_obj)
			AttachObject (robot, expl_obj);
		if (bDamage && (rInfoP->nExp1Sound > -1))
			DigiLinkSoundToPos (rInfoP->nExp1Sound, robot->nSegment, 0, vHitPt, 0, F1_0);
		if (!(weapon->flags & OF_HARMLESS)) {
			fix	damage = weapon->shields;
			if (bDamage)
				damage = FixMul (damage, weapon->cType.laserInfo.multiplier);
			else
				damage = 0;
			//	Cut Gauss damage on bosses because it just breaks the game.  Bosses are so easy to
			//	hit, and missing a robot is what prevents the Gauss from being game-breaking.
			if (weapon->id == GAUSS_ID) {
				if (rInfoP->bossFlag)
					damage = damage * (2*NDL-gameStates.app.nDifficultyLevel)/ (2*NDL);
				}
			else if (gameStates.app.bHaveExtraGameInfo [IsMultiGame] && (weapon->id == FUSION_ID))
				damage = damage * extraGameInfo [IsMultiGame].nFusionPowerMod / 2;
			if (!ApplyDamageToRobot (robot, damage, weapon->cType.laserInfo.nParentObj))
				BumpTwoObjects (robot, weapon, 0, vHitPt);		//only bump if not dead. no damage from bump
			else if (weapon->cType.laserInfo.nParentSig == gameData.objs.console->nSignature) {
				AddPointsToScore (rInfoP->scoreValue);
				DetectEscortGoalAccomplished (OBJ_IDX (robot));
				}
			}
		//	If Gauss Cannon, spin robot.
		if ((robot != NULL) && (!rInfoP->companion) && (!rInfoP->bossFlag) && (weapon->id == GAUSS_ID)) {
			tAIStatic	*aip = &robot->cType.aiInfo;

			if (aip->SKIP_AI_COUNT * gameData.time.xFrame < F1_0) {
				aip->SKIP_AI_COUNT++;
				robot->mType.physInfo.rotThrust.x = FixMul ((d_rand () - 16384), gameData.time.xFrame * aip->SKIP_AI_COUNT);
				robot->mType.physInfo.rotThrust.y = FixMul ((d_rand () - 16384), gameData.time.xFrame * aip->SKIP_AI_COUNT);
				robot->mType.physInfo.rotThrust.z = FixMul ((d_rand () - 16384), gameData.time.xFrame * aip->SKIP_AI_COUNT);
				robot->mType.physInfo.flags |= PF_USES_THRUST;
				}
			}
		}
MaybeKillWeapon (weapon, robot);
return 1; 
}

//##void CollideRobotAndCamera (tObject * robot, tObject * camera, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideRobotAndPowerup (tObject * robot, tObject * powerup, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideRobotAndDebris (tObject * robot, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideHostageAndHostage (tObject * hostage1, tObject * hostage2, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-----------------------------------------------------------------------------

int CollideHostageAndPlayer (tObject * hostage, tObject * player, vmsVector *vHitPt) 
{ 
	// Give player points, etc.
if (player == gameData.objs.console) {
	DetectEscortGoalAccomplished (OBJ_IDX (hostage));
	AddPointsToScore (HOSTAGE_SCORE);
	// Do effect
	hostage_rescue (hostage->id);
	// Remove the hostage tObject.
	hostage->flags |= OF_SHOULD_BE_DEAD;
#ifdef NETWORK	
	if (gameData.app.nGameMode & GM_MULTI)
		MultiSendRemObj (OBJ_IDX (hostage));
#endif
	}
return 1; 
}

//--unused-- void CollideHostageAndWeapon (tObject * hostage, tObject * weapon, vmsVector *vHitPt)
//--unused-- { 
//--unused-- 	//	Cannot kill hostages, as per Matt's edict!
//--unused-- 	//	 (A fine edict, but in contradiction to the milestone: "Robots attack hostages.")
//--unused-- 	hostage->shields -= weapon->shields/2;
//--unused-- 
//--unused-- 	CreateAwarenessEvent (weapon, PA_WEAPON_ROBOT_COLLISION);			// tObject "weapon" can attract attention to player
//--unused-- 
//--unused-- 	//PLAY_SOUND_3D (SOUND_HOSTAGE_KILLED, vHitPt, hostage->nSegment);
//--unused-- 	DigiLinkSoundToPos (SOUND_HOSTAGE_KILLED, hostage->nSegment , 0, vHitPt, 0, F1_0);
//--unused-- 
//--unused-- 
//--unused-- 	if (hostage->shields <= 0) {
//--unused-- 		ExplodeObject (hostage, 0);
//--unused-- 		hostage->flags |= OF_SHOULD_BE_DEAD;
//--unused-- 	}
//--unused-- 
//--unused-- 	if (WI_damage_radius (weapon->id))
//--unused-- 		ExplodeBadassWeapon (weapon);
//--unused-- 
//--unused-- 	MaybeKillWeapon (weapon, hostage);
//--unused-- 
//--unused-- }

//##void CollideHostageAndCamera (tObject * hostage, tObject * camera, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideHostageAndPowerup (tObject * hostage, tObject * powerup, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideHostageAndDebris (tObject * hostage, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-----------------------------------------------------------------------------

int CollidePlayerAndPlayer (tObject * player1, tObject * player2, vmsVector *vHitPt) 
{ 
if (gameStates.app.bD2XLevel && 
	 (gameData.segs.segment2s [player1->nSegment].special == SEGMENT_IS_NODAMAGE))
	return 1;
if (BumpTwoObjects (player1, player2, 1, vHitPt))
	DigiLinkSoundToPos (SOUND_ROBOT_HIT_PLAYER, player1->nSegment, 0, vHitPt, 0, F1_0);
return 1;
}

//	-----------------------------------------------------------------------------

int MaybeDropPrimaryWeaponEgg (tObject *playerObjP, int weapon_index)
{
	int weaponFlag = HAS_FLAG (weapon_index);
	int powerup_num = primaryWeaponToPowerup [weapon_index];

if (gameData.multi.players [playerObjP->id].primaryWeaponFlags & weaponFlag)
	return CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, powerup_num);
else
	return -1;
}

//	-----------------------------------------------------------------------------

void MaybeDropSecondaryWeaponEgg (tObject *playerObjP, int weapon_index, int count)
{
	int weaponFlag = HAS_FLAG (weapon_index);
	int powerup_num = secondaryWeaponToPowerup [weapon_index];

if (gameData.multi.players [playerObjP->id].secondaryWeaponFlags & weaponFlag) {
	int	i, max_count = (EGI_FLAG (bDropAllMissiles, 0, 0)) ? count : min (count, 3);
	for (i=0; i<max_count; i++)
		CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, powerup_num);
	}
}

//	-----------------------------------------------------------------------------

void DropMissile1or4 (tObject *playerObjP, int missile_index)
{
	int num_missiles, powerup_id;

if (num_missiles = gameData.multi.players [playerObjP->id].secondaryAmmo [missile_index]) {
	powerup_id = secondaryWeaponToPowerup [missile_index];
	if (!(EGI_FLAG (bDropAllMissiles, 0, 0)) && (num_missiles > 10))
		num_missiles = 10;
	CallObjectCreateEgg (playerObjP, num_missiles/4, OBJ_POWERUP, powerup_id+1);
	CallObjectCreateEgg (playerObjP, num_missiles%4, OBJ_POWERUP, powerup_id);
	}
}

// -- int	Items_destroyed = 0;

//	-----------------------------------------------------------------------------

void DropPlayerEggs (tObject *playerObjP)
{
if ((playerObjP->nType == OBJ_PLAYER) || (playerObjP->nType == OBJ_GHOST)) {
	int	rthresh;
	int	pnum = playerObjP->id;
	short	nObject, plrObjNum = OBJ_IDX (playerObjP);
	int	vulcan_ammo=0;
	vmsVector	randvec;
	player *playerP = gameData.multi.players + pnum;

	// -- Items_destroyed = 0;

	// Seed the random number generator so in net play the eggs will always
	// drop the same way
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI) {
		multiData.create.nLoc = 0;
		d_srand (5483L);
	}
#endif

	//	If the player had smart mines, maybe arm one of them.
	rthresh = 30000;
	while ((playerP->secondaryAmmo [SMART_MINE_INDEX]%4==1) && (d_rand () < rthresh)) {
		short			newseg;
		vmsVector	tvec;

		MakeRandomVector (&randvec);
		rthresh /= 2;
		VmVecAdd (&tvec, &playerObjP->pos, &randvec);
		newseg = FindSegByPoint (&tvec, playerObjP->nSegment);
		if (newseg != -1)
			CreateNewLaser (&randvec, &tvec, newseg, plrObjNum, SUPERPROX_ID, 0);
	  	}

		//	If the player had proximity bombs, maybe arm one of them.
		if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY))) {
			rthresh = 30000;
			while ((playerP->secondaryAmmo [PROXIMITY_INDEX] % 4 == 1) && (d_rand () < rthresh)) {
				short			newseg;
				vmsVector	tvec;
	
				MakeRandomVector (&randvec);
				rthresh /= 2;
				VmVecAdd (&tvec, &playerObjP->pos, &randvec);
				newseg = FindSegByPoint (&tvec, playerObjP->nSegment);
				if (newseg != -1)
					CreateNewLaser (&randvec, &tvec, newseg, plrObjNum, PROXIMITY_ID, 0);
			}
		}

		//	If the player dies and he has powerful lasers, create the powerups here.

		if (playerP->laserLevel > MAX_LASER_LEVEL)
			CallObjectCreateEgg (playerObjP, playerP->laserLevel-MAX_LASER_LEVEL, OBJ_POWERUP, POW_SUPER_LASER);
		else if (playerP->laserLevel >= 1)
			CallObjectCreateEgg (playerObjP, playerP->laserLevel, OBJ_POWERUP, POW_LASER);	// Note: laserLevel = 0 for laser level 1.

		//	Drop quad laser if appropos
		if (playerP->flags & PLAYER_FLAGS_QUAD_LASERS)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_QUAD_FIRE);
		if (playerP->flags & PLAYER_FLAGS_CLOAKED)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_CLOAK);
		while (playerP->nInvuls--)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_INVULNERABILITY);
		while (playerP->nCloaks--)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_CLOAK);
		if (playerP->flags & PLAYER_FLAGS_MAP_ALL)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_FULL_MAP);
		if (playerP->flags & PLAYER_FLAGS_AFTERBURNER)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_AFTERBURNER);
		if (playerP->flags & PLAYER_FLAGS_AMMO_RACK)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_AMMO_RACK);
		if (playerP->flags & PLAYER_FLAGS_CONVERTER)
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_CONVERTER);
		if ((playerP->flags & PLAYER_FLAGS_HEADLIGHT) && 
			 !(gameStates.app.bHaveExtraGameInfo [1] && IsMultiGame || extraGameInfo [1].bDarkness))
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_HEADLIGHT);
		// drop the other enemies flag if you have it

	playerP->nInvuls =
	playerP->nCloaks = 0;
	playerP->flags &= ~(PLAYER_FLAGS_INVULNERABLE | PLAYER_FLAGS_CLOAKED);
#ifdef NETWORK
	if ((gameData.app.nGameMode & GM_CAPTURE) && (playerP->flags & PLAYER_FLAGS_FLAG)) {
		if ((GetTeam (pnum)==TEAM_RED))
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_FLAG_BLUE);
		else
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_FLAG_RED);
		}

#ifdef RELEASE			
		if (gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY))
#endif
		if ((gameData.app.nGameMode & GM_HOARD) || 
		    ((gameData.app.nGameMode & GM_ENTROPY) && extraGameInfo [1].entropy.nVirusStability)) {
			// Drop hoard orbs
			
			int max_count, i;
#if TRACE
			con_printf (CON_DEBUG, "HOARD MODE: Dropping %d orbs \n", playerP->secondaryAmmo [PROXIMITY_INDEX]);
#endif	
			max_count = playerP->secondaryAmmo [PROXIMITY_INDEX];
			if ((gameData.app.nGameMode & GM_HOARD) && (max_count > 12))
				max_count = 12;
			for (i = 0; i < max_count; i++)
				CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_HOARD_ORB);
		}
#endif

		//Drop the vulcan, gauss, and ammo
		vulcan_ammo = playerP->primaryAmmo [VULCAN_INDEX];
		if ((playerP->primaryWeaponFlags & HAS_FLAG (VULCAN_INDEX)) && 
			 (playerP->primaryWeaponFlags & HAS_FLAG (GAUSS_INDEX)))
			vulcan_ammo /= 2;		//if both vulcan & gauss, each gets half
		if (vulcan_ammo < VULCAN_AMMO_AMOUNT)
			vulcan_ammo = VULCAN_AMMO_AMOUNT;	//make sure gun has at least as much as a powerup
		nObject = MaybeDropPrimaryWeaponEgg (playerObjP, VULCAN_INDEX);
		if (nObject!=-1)
			gameData.objs.objects [nObject].cType.powerupInfo.count = vulcan_ammo;
		nObject = MaybeDropPrimaryWeaponEgg (playerObjP, GAUSS_INDEX);
		if (nObject!=-1)
			gameData.objs.objects [nObject].cType.powerupInfo.count = vulcan_ammo;

		//	Drop the rest of the primary weapons
		MaybeDropPrimaryWeaponEgg (playerObjP, SPREADFIRE_INDEX);
		MaybeDropPrimaryWeaponEgg (playerObjP, PLASMA_INDEX);
		MaybeDropPrimaryWeaponEgg (playerObjP, FUSION_INDEX);
		MaybeDropPrimaryWeaponEgg (playerObjP, HELIX_INDEX);
		MaybeDropPrimaryWeaponEgg (playerObjP, PHOENIX_INDEX);
		nObject = MaybeDropPrimaryWeaponEgg (playerObjP, OMEGA_INDEX);
		if (nObject!=-1)
			gameData.objs.objects [nObject].cType.powerupInfo.count = (playerObjP->id==gameData.multi.nLocalPlayer)?xOmegaCharge:MAX_OMEGA_CHARGE;
		//	Drop the secondary weapons
		//	Note, proximity weapon only comes in packets of 4.  So drop n/2, but a max of 3 (handled inside maybe_drop..)  Make sense?
		if (!(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)))
			MaybeDropSecondaryWeaponEgg (playerObjP, PROXIMITY_INDEX, (playerP->secondaryAmmo [PROXIMITY_INDEX])/4);
		MaybeDropSecondaryWeaponEgg (playerObjP, SMART_INDEX, playerP->secondaryAmmo [SMART_INDEX]);
		MaybeDropSecondaryWeaponEgg (playerObjP, MEGA_INDEX, playerP->secondaryAmmo [MEGA_INDEX]);
		if (!(gameData.app.nGameMode & GM_ENTROPY))
			MaybeDropSecondaryWeaponEgg (playerObjP, SMART_MINE_INDEX, (playerP->secondaryAmmo [SMART_MINE_INDEX])/4);
		MaybeDropSecondaryWeaponEgg (playerObjP, SMISSILE5_INDEX, playerP->secondaryAmmo [SMISSILE5_INDEX]);
		//	Drop the player's missiles in packs of 1 and/or 4
		DropMissile1or4 (playerObjP, HOMING_INDEX);
		DropMissile1or4 (playerObjP, GUIDED_INDEX);
		DropMissile1or4 (playerObjP, CONCUSSION_INDEX);
		DropMissile1or4 (playerObjP, SMISSILE1_INDEX);
		DropMissile1or4 (playerObjP, SMISSILE4_INDEX);
		//	If player has vulcan ammo, but no vulcan cannon, drop the ammo.
		if (!(playerP->primaryWeaponFlags & HAS_VULCAN_FLAG)) {
			int	amount = playerP->primaryAmmo [VULCAN_INDEX];
			if (amount > 200) {
#if TRACE
				con_printf (CON_DEBUG, "Surprising amount of vulcan ammo: %i bullets. \n", amount);
#endif
				amount = 200;
			}
			while (amount > 0) {
				CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_VULCAN_AMMO);
				amount -= VULCAN_AMMO_AMOUNT;
			}
		}

		//	Always drop a shield and energy powerup.
		if (gameData.app.nGameMode & GM_MULTI) {
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_SHIELD_BOOST);
			CallObjectCreateEgg (playerObjP, 1, OBJ_POWERUP, POW_ENERGY);
		}

//--		//	Drop all the keys.
//--		if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_BLUE_KEY) {
//--			playerObjP->containsCount = 1;
//--			playerObjP->containsType = OBJ_POWERUP;
//--			playerObjP->containsId = POW_KEY_BLUE;
//--			ObjectCreateEgg (playerObjP);
//--		}
//--		if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_RED_KEY) {
//--			playerObjP->containsCount = 1;
//--			playerObjP->containsType = OBJ_POWERUP;
//--			playerObjP->containsId = POW_KEY_RED;
//--			ObjectCreateEgg (playerObjP);
//--		}
//--		if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_GOLD_KEY) {
//--			playerObjP->containsCount = 1;
//--			playerObjP->containsType = OBJ_POWERUP;
//--			playerObjP->containsId = POW_KEY_GOLD;
//--			ObjectCreateEgg (playerObjP);
//--		}

// -- 		if (Items_destroyed) {
// -- 			if (Items_destroyed == 1)
// -- 				HUDInitMessage ("%i item was destroyed.", Items_destroyed);
// -- 			else
// -- 				HUDInitMessage ("%i items were destroyed.", Items_destroyed);
// -- 			Items_destroyed = 0;
// -- 		}
	}

}

// -- removed, 09/06/95, MK -- void destroy_primaryWeapon (int weapon_index)
// -- removed, 09/06/95, MK -- {
// -- removed, 09/06/95, MK -- 	if (weapon_index == MAX_PRIMARY_WEAPONS) {
// -- removed, 09/06/95, MK -- 		HUDInitMessage ("Quad lasers destroyed!");
// -- removed, 09/06/95, MK -- 		gameData.multi.players [gameData.multi.nLocalPlayer].flags &= ~PLAYER_FLAGS_QUAD_LASERS;
// -- removed, 09/06/95, MK -- 		update_laserWeapon_info ();
// -- removed, 09/06/95, MK -- 	} else if (weapon_index == 0) {
// -- removed, 09/06/95, MK -- 		Assert (gameData.multi.players [gameData.multi.nLocalPlayer].laserLevel > 0);
// -- removed, 09/06/95, MK -- 		HUDInitMessage ("%s degraded!", Text_string [104+weapon_index]);		//	Danger!Danger!Use of literal! Danger!
// -- removed, 09/06/95, MK -- 		gameData.multi.players [gameData.multi.nLocalPlayer].laserLevel--;
// -- removed, 09/06/95, MK -- 		update_laserWeapon_info ();
// -- removed, 09/06/95, MK -- 	} else {
// -- removed, 09/06/95, MK -- 		HUDInitMessage ("%s destroyed!", Text_string [104+weapon_index]);		//	Danger!Danger!Use of literal! Danger!
// -- removed, 09/06/95, MK -- 		gameData.multi.players [gameData.multi.nLocalPlayer].primaryWeaponFlags &= ~ (1 << weapon_index);
// -- removed, 09/06/95, MK -- 		AutoSelectWeapon (0);
// -- removed, 09/06/95, MK -- 	}
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- }
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- void destroy_secondaryWeapon (int weapon_index)
// -- removed, 09/06/95, MK -- {
// -- removed, 09/06/95, MK -- 	if (gameData.multi.players [gameData.multi.nLocalPlayer].secondaryAmmo <= 0)
// -- removed, 09/06/95, MK -- 		return;
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- 	HUDInitMessage ("%s destroyed!", Text_string [114+weapon_index]);		//	Danger!Danger!Use of literal! Danger!
// -- removed, 09/06/95, MK -- 	if (--gameData.multi.players [gameData.multi.nLocalPlayer].secondaryAmmo [weapon_index] == 0)
// -- removed, 09/06/95, MK -- 		AutoSelectWeapon (1);
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- }
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- #define	LOSE_WEAPON_THRESHOLD	 (F1_0*30)

//	-----------------------------------------------------------------------------

void ApplyDamageToPlayer (tObject *playerObjP, tObject *killer, fix damage)
{
	player *playerP = gameData.multi.players + playerObjP->id;
	player *killerP = (killer && (killer->nType == OBJ_PLAYER)) ? gameData.multi.players + killer->id : NULL;
	if (gameStates.app.bPlayerIsDead)
		return;

	if (gameStates.app.bD2XLevel && (gameData.segs.segment2s [playerObjP->nSegment].special == SEGMENT_IS_NODAMAGE))
		return;
	if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE)
		return;

	if (killer == playerObjP) {
		if (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bInhibitSuicide)
			return;
		}
	else if (killerP && gameStates.app.bHaveExtraGameInfo [1] && !extraGameInfo [1].bFriendlyFire) {
		if (gameData.app.nGameMode & GM_TEAM) {
			if (GetTeam (playerObjP->id) == GetTeam (killer->id))
				return;
			}
		else if (gameData.app.nGameMode & GM_MULTI_COOP)
			return;
		}
	if (gameStates.app.bEndLevelSequence)
		return;

	gameData.multi.bWasHit [playerObjP->id] = -1;
	//for the player, the 'real' shields are maintained in the gameData.multi.players []
	//array.  The shields value in the player's tObject are, I think, not
	//used anywhere.  This routine, however, sets the gameData.objs.objects shields to
	//be a mirror of the value in the Player structure. 

	if (playerObjP->id == gameData.multi.nLocalPlayer) {		//is this the local player?

		//	MK: 08/14/95: This code can never be reached.  See the return about 12 lines up.
// -- 		if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE) {
// -- 
// -- 			//invincible, so just do blue flash
// -- 
// -- 			PALETTE_FLASH_ADD (0, 0, f2i (damage)*4);	//flash blue
// -- 
// -- 		} 
// -- 		else {		//take damage, do red flash

			if ((gameData.app.nGameMode & GM_ENTROPY)&& extraGameInfo [1].entropy.bPlayerHandicap && killerP) {
				double h = (double) playerP->netKillsTotal / (double) (killerP->netKillsTotal + 1);
				if (h < 0.5)
					h = 0.5;
				else if (h > 1.0)
					h = 1.0;
				if (!(damage = (fix) ((double) damage * h)))
					damage = 1;
				}
			playerP->shields -= damage;
			MultiSendShields ();
			PALETTE_FLASH_ADD (f2i (damage)*4, -f2i (damage/2), -f2i (damage/2));	//flash red

// -- 		}

		if (playerP->shields < 0)	{
  			playerP->nKillerObj = OBJ_IDX (killer);
//			if (killer && (killer->nType == OBJ_PLAYER))
//				gameData.multi.players [gameData.multi.nLocalPlayer].nKillerObj = OBJ_IDX (killer);
			playerObjP->flags |= OF_SHOULD_BE_DEAD;
			if (gameData.escort.nObjNum != -1)
				if (killer && (killer->nType == OBJ_ROBOT) && (gameData.bots.pInfo [killer->id].companion))
					gameData.escort.xSorryTime = gameData.time.xGame;
		}
// -- removed, 09/06/95, MK --  else if (gameData.multi.players [gameData.multi.nLocalPlayer].shields < LOSE_WEAPON_THRESHOLD) {
// -- removed, 09/06/95, MK -- 			int	randnum = d_rand ();
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- 			if (FixMul (gameData.multi.players [gameData.multi.nLocalPlayer].shields, randnum) < damage/4) {
// -- removed, 09/06/95, MK -- 				if (d_rand () > 20000) {
// -- removed, 09/06/95, MK -- 					destroy_secondaryWeapon (gameData.weapons.nSecondary);
// -- removed, 09/06/95, MK -- 				} else if (gameData.weapons.nPrimary == 0) {
// -- removed, 09/06/95, MK -- 					if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_QUAD_LASERS)
// -- removed, 09/06/95, MK -- 						destroy_primaryWeapon (MAX_PRIMARY_WEAPONS);	//	This means to destroy quad laser.
// -- removed, 09/06/95, MK -- 					else if (gameData.multi.players [gameData.multi.nLocalPlayer].laserLevel > 0)
// -- removed, 09/06/95, MK -- 						destroy_primaryWeapon (gameData.weapons.nPrimary);
// -- removed, 09/06/95, MK -- 				} else
// -- removed, 09/06/95, MK -- 					destroy_primaryWeapon (gameData.weapons.nPrimary);
// -- removed, 09/06/95, MK -- 			} else
// -- removed, 09/06/95, MK -- 				; 
// -- removed, 09/06/95, MK -- 		}
		playerObjP->shields = playerP->shields;		//mirror
	}
}

//	-----------------------------------------------------------------------------

int CollidePlayerAndWeapon (tObject * playerObjP, tObject * weapon, vmsVector *vHitPt)
{
	fix		damage = weapon->shields;
	tObject * killer=NULL;

	//	In multiplayer games, only do damage to another player if in first frame.
	//	This is necessary because in multiplayer, due to varying framerates, omega blobs actually
	//	have a bit of a lifetime.  But they start out with a lifetime of ONE_FRAME_TIME, and this
	//	gets bashed to 1/4 second in laser_doWeapon_sequence.  This bashing occurs for visual purposes only.
if (gameStates.app.bD2XLevel && (gameData.segs.segment2s [playerObjP->nSegment].special == SEGMENT_IS_NODAMAGE))
	return 1;
if (weapon->id == OMEGA_ID)
	if (!OkToDoOmegaDamage (weapon))
		return 1;
//	Don't Collide own smart mines unless direct hit.
if (weapon->id == SUPERPROX_ID)
	if (OBJ_IDX (playerObjP) == weapon->cType.laserInfo.nParentObj)
		if (VmVecDistQuick (vHitPt, &playerObjP->pos) > playerObjP->size)
			return 1;
if (weapon->id == EARTHSHAKER_ID)
	ShakerRockStuff ();
damage = FixMul (damage, weapon->cType.laserInfo.multiplier);
if (gameStates.app.bHaveExtraGameInfo [IsMultiGame] && (weapon->id == FUSION_ID))
	damage = damage * extraGameInfo [IsMultiGame].nFusionPowerMod / 2;
#ifndef SHAREWARE
if (gameData.app.nGameMode & GM_MULTI)
	damage = FixMul (damage, gameData.weapons.info [weapon->id].multi_damage_scale);
#endif
if (weapon->mType.physInfo.flags & PF_PERSISTENT) {
	if (weapon->cType.laserInfo.nLastHitObj == OBJ_IDX (playerObjP))
		return 1;
	weapon->cType.laserInfo.nLastHitObj = OBJ_IDX (playerObjP);
}
if (playerObjP->id == gameData.multi.nLocalPlayer) {
	if (!(gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_INVULNERABLE)) {
		DigiLinkSoundToPos (SOUND_PLAYER_GOT_HIT, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
			MultiSendPlaySound (SOUND_PLAYER_GOT_HIT, F1_0);
#endif
		}
	else {
		DigiLinkSoundToPos (SOUND_WEAPON_HIT_DOOR, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
			MultiSendPlaySound (SOUND_WEAPON_HIT_DOOR, F1_0);
#endif
		}
	}
ObjectCreateExplosion (playerObjP->nSegment, vHitPt, i2f (10)/2, VCLIP_PLAYER_HIT);
if (WI_damage_radius (weapon->id))
	ExplodeBadassWeapon (weapon, vHitPt);
MaybeKillWeapon (weapon, playerObjP);
BumpTwoObjects (playerObjP, weapon, 0, vHitPt);	//no damage from bump
if (!WI_damage_radius (weapon->id)) {
	if (weapon->cType.laserInfo.nParentObj > -1)
		killer = &gameData.objs.objects [weapon->cType.laserInfo.nParentObj];

//		if (weapon->id == SMART_HOMING_ID)
//			damage /= 4;

	if (!(weapon->flags & OF_HARMLESS))
		ApplyDamageToPlayer (playerObjP, killer, damage);
}
//	Robots become aware of you if you get hit.
AIDoCloakStuff ();
return 1; 
}

//	-----------------------------------------------------------------------------
//	Nasty robots are the ones that attack you by running into you and doing lots of damage.
int CollidePlayerAndNastyRobot (tObject * playerObjP, tObject * robot, vmsVector *vHitPt)
{
//	if (!(gameData.bots.pInfo [robot->id].energyDrain && gameData.multi.players [playerObjP->id].energy))
ObjectCreateExplosion (playerObjP->nSegment, vHitPt, i2f (10)/2, VCLIP_PLAYER_HIT);
if (BumpTwoObjects (playerObjP, robot, 0, vHitPt))	{//no damage from bump
	DigiLinkSoundToPos (gameData.bots.pInfo [robot->id].clawSound, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
	ApplyDamageToPlayer (playerObjP, robot, F1_0* (gameStates.app.nDifficultyLevel+1));
	}
return 1;
}

//	-----------------------------------------------------------------------------

int CollidePlayerAndMatCen (tObject *objP)
{
	short	tSide;
	vmsVector	exit_dir;
	tSegment	*segp = gameData.segs.segments + objP->nSegment;

DigiLinkSoundToPos (SOUND_PLAYER_GOT_HIT, objP->nSegment, 0, &objP->pos, 0, F1_0);
//	DigiPlaySample (SOUND_PLAYER_GOT_HIT, F1_0);
ObjectCreateExplosion (objP->nSegment, &objP->pos, i2f (10)/2, VCLIP_PLAYER_HIT);
if (objP->id != gameData.multi.nLocalPlayer)
	return 1;
for (tSide=0; tSide<MAX_SIDES_PER_SEGMENT; tSide++)
	if (WALL_IS_DOORWAY (segp, tSide, objP) & WID_FLY_FLAG) {
		vmsVector	exit_point, rand_vec;

		COMPUTE_SIDE_CENTER (&exit_point, segp, tSide);
		VmVecSub (&exit_dir, &exit_point, &objP->pos);
		VmVecNormalizeQuick (&exit_dir);
		MakeRandomVector (&rand_vec);
		rand_vec.x /= 4;	rand_vec.y /= 4;	rand_vec.z /= 4;
		VmVecInc (&exit_dir, &rand_vec);
		VmVecNormalizeQuick (&exit_dir);
		}
BumpOneObject (objP, &exit_dir, 64*F1_0);
ApplyDamageToPlayer (objP, objP, 4*F1_0);	//	Changed, MK, 2/19/96, make killer the player, so if you die in matcen, will say you killed yourself
return 1; 
}

//	-----------------------------------------------------------------------------

int CollideRobotAndMatCen (tObject *objP)
{
	short	tSide;
	vmsVector	exit_dir;
	tSegment *segp=gameData.segs.segments + objP->nSegment;

DigiLinkSoundToPos (SOUND_ROBOT_HIT, objP->nSegment, 0, &objP->pos, 0, F1_0);
//	DigiPlaySample (SOUND_ROBOT_HIT, F1_0);

if (gameData.bots.pInfo [objP->id].nExp1VClip > -1)
	ObjectCreateExplosion ((short) objP->nSegment, &objP->pos, (objP->size/2*3)/4, (ubyte) gameData.bots.pInfo [objP->id].nExp1VClip);
for (tSide=0; tSide<MAX_SIDES_PER_SEGMENT; tSide++)
	if (WALL_IS_DOORWAY (segp, tSide, NULL) & WID_FLY_FLAG) {
		vmsVector	exit_point;

		COMPUTE_SIDE_CENTER (&exit_point, segp, tSide);
		VmVecSub (&exit_dir, &exit_point, &objP->pos);
		VmVecNormalizeQuick (&exit_dir);
	}
BumpOneObject (objP, &exit_dir, 8*F1_0);
ApplyDamageToRobot (objP, F1_0, -1);
return 1; 
}

//##void CollidePlayerAndCamera (tObject * playerObjP, tObject * camera, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-----------------------------------------------------------------------------

extern int Network_gotPowerup; // HACK!!!

int CollidePlayerAndPowerup (tObject * playerObjP, tObject * powerup, vmsVector *vHitPt) 
{ 
if (!gameStates.app.bEndLevelSequence && !gameStates.app.bPlayerIsDead && 
	(playerObjP->id == gameData.multi.nLocalPlayer)) {
	int bPowerupUsed = DoPowerup (powerup, playerObjP->id);
	if (bPowerupUsed) {
		powerup->flags |= OF_SHOULD_BE_DEAD;
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
			MultiSendRemObj (OBJ_IDX (powerup));
#endif
		}
	}
#ifndef SHAREWARE
else if ((gameData.app.nGameMode & GM_MULTI_COOP) && (playerObjP->id != gameData.multi.nLocalPlayer)) {
	switch (powerup->id) {
		case POW_KEY_BLUE:	
			gameData.multi.players [playerObjP->id].flags |= PLAYER_FLAGS_BLUE_KEY;
			break;
		case POW_KEY_RED:	
			gameData.multi.players [playerObjP->id].flags |= PLAYER_FLAGS_RED_KEY;
			break;
		case POW_KEY_GOLD:	
			gameData.multi.players [playerObjP->id].flags |= PLAYER_FLAGS_GOLD_KEY;
			break;
		default:
			break;
		}
	}
#endif
return 1; 
}

int CollidePlayerAndMonsterball (tObject * playerObjP, tObject * monsterball, vmsVector *vHitPt) 
{ 
if (!gameStates.app.bEndLevelSequence && !gameStates.app.bPlayerIsDead && 
	(playerObjP->id == gameData.multi.nLocalPlayer)) {
	if (BumpTwoObjects (playerObjP, monsterball, 0, vHitPt))
		DigiLinkSoundToPos (SOUND_ROBOT_HIT_PLAYER, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
	}
return 1;
}

//	-----------------------------------------------------------------------------
//##void CollidePlayerAndDebris (tObject * playerObjP, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

int CollidePlayerAndClutter (tObject * playerObjP, tObject * clutter, vmsVector *vHitPt) 
{ 
if (gameStates.app.bD2XLevel && 
	 (gameData.segs.segment2s [playerObjP->nSegment].special == SEGMENT_IS_NODAMAGE))
	return 1;
if (BumpTwoObjects (clutter, playerObjP, 1, vHitPt))
	DigiLinkSoundToPos (SOUND_ROBOT_HIT_PLAYER, playerObjP->nSegment, 0, vHitPt, 0, F1_0);
return 1;
}

//	-----------------------------------------------------------------------------
//	See if weapon1 creates a badass explosion.  If so, create the explosion
//	Return true if weapon does proximity (as opposed to only contact) damage when it explodes.
int MaybeDetonateWeapon (tObject *weapon1, tObject *weapon2, vmsVector *vHitPt)
{
if (gameData.weapons.info [weapon1->id].damage_radius) {
	fix	dist = VmVecDistQuick (&weapon1->pos, &weapon2->pos);
	if (dist < F1_0*5) {
		MaybeKillWeapon (weapon1, weapon2);
		if (weapon1->flags & OF_SHOULD_BE_DEAD) {
			ExplodeBadassWeapon (weapon1, vHitPt);
			DigiLinkSoundToPos (gameData.weapons.info [weapon1->id].robot_hitSound, weapon1->nSegment , 0, vHitPt, 0, F1_0);
		}
		return 1;
		} 
	else {
		weapon1->lifeleft = min (dist/64, F1_0);
		return 1;
		}
	}
else
	return 0;
}

//	-----------------------------------------------------------------------------

int CollideWeaponAndWeapon (tObject * weapon1, tObject * weapon2, vmsVector *vHitPt)
{ 
	// -- Does this look buggy??:  if (weapon1->id == PMINE_ID && weapon1->id == PMINE_ID)
if (weapon1->id == PMINE_ID && weapon2->id == PMINE_ID)
	return 1;		//these can't blow each other up  
if (weapon1->id == OMEGA_ID) {
	if (!OkToDoOmegaDamage (weapon1))
		return 1;
	}
else if (weapon2->id == OMEGA_ID) {
	if (!OkToDoOmegaDamage (weapon2))
		return 1;
	}
if (WI_destructable (weapon1->id) || WI_destructable (weapon2->id)) {
	//	Bug reported by Adam Q. Pletcher on September 9, 1994, smart bomb homing missiles were toasting each other.
	if ((weapon1->id == weapon2->id) && (weapon1->cType.laserInfo.nParentObj == weapon2->cType.laserInfo.nParentObj))
		return 1;
	if (WI_destructable (weapon1->id))
		if (MaybeDetonateWeapon (weapon1, weapon2, vHitPt))
			MaybeDetonateWeapon (weapon2, weapon1, vHitPt);
	if (WI_destructable (weapon2->id))
		if (MaybeDetonateWeapon (weapon2, weapon1, vHitPt))
			MaybeDetonateWeapon (weapon1, weapon2, vHitPt);
	}
return 1;
}

//	-----------------------------------------------------------------------------

int CollideWeaponAndMonsterball (tObject * weapon, tObject * powerup, vmsVector *vHitPt) 
{
if (weapon->cType.laserInfo.parentType == OBJ_PLAYER) {	
	DigiLinkSoundToPos (SOUND_ROBOT_HIT, weapon->nSegment , 0, vHitPt, 0, F1_0);
	if (weapon->id == EARTHSHAKER_ID)
		ShakerRockStuff ();
	if (weapon->mType.physInfo.flags & PF_PERSISTENT) {
		if (weapon->cType.laserInfo.nLastHitObj == OBJ_IDX (powerup))
			return 1;
		weapon->cType.laserInfo.nLastHitObj = OBJ_IDX (powerup);
		}
	ObjectCreateExplosion (powerup->nSegment, vHitPt, i2f (10)/2, VCLIP_PLAYER_HIT);
	if (WI_damage_radius (weapon->id))
		ExplodeBadassWeapon (weapon, vHitPt);
	MaybeKillWeapon (weapon, powerup);
	BumpTwoObjects (weapon, powerup, 1, vHitPt);
	}
return 1; 
}

//	-----------------------------------------------------------------------------
//##void CollideWeaponAndCamera (tObject * weapon, tObject * camera, vmsVector *vHitPt) { 
//##	return; 
//##}

//	-----------------------------------------------------------------------------

int CollideWeaponAndDebris (tObject * weapon, tObject * debris, vmsVector *vHitPt) 
{ 

	//	Hack! Prevent debris from causing bombs spewed at player death to detonate!
if ((weapon->id == PROXIMITY_ID) || (weapon->id == SUPERPROX_ID)) {
	if (weapon->cType.laserInfo.creationTime + F1_0/2 > gameData.time.xGame)
		return 1;
	}
if ((weapon->cType.laserInfo.parentType==OBJ_PLAYER) && !(debris->flags & OF_EXPLODING))	{	
	DigiLinkSoundToPos (SOUND_ROBOT_HIT, weapon->nSegment , 0, vHitPt, 0, F1_0);
	ExplodeObject (debris, 0);
	if (WI_damage_radius (weapon->id))
		ExplodeBadassWeapon (weapon, vHitPt);
	MaybeKillWeapon (weapon, debris);
	weapon->flags |= OF_SHOULD_BE_DEAD;
	}
return 1; 
}

//##void CollideCameraAndCamera (tObject * camera1, tObject * camera2, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideCameraAndPowerup (tObject * camera, tObject * powerup, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideCameraAndDebris (tObject * camera, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollidePowerupAndPowerup (tObject * powerup1, tObject * powerup2, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollidePowerupAndDebris (tObject * powerup, tObject * debris, vmsVector *vHitPt) { 
//##	return; 
//##}

//##void CollideDebrisAndDebris (tObject * debris1, tObject * debris2, vmsVector *vHitPt) { 
//##	return; 
//##}


/* DPH: Put these macros on one long line to avoid CR/LF problems on linux */
#define	COLLISION_OF(a, b) (((a)<<8) + (b))

#define	DO_COLLISION(type1, type2, collisionHandler) \
			case COLLISION_OF ((type1), (type2)): \
				return (collisionHandler) ((A), (B), vHitPt); \
			case COLLISION_OF ((type2), (type1)): \
				return (collisionHandler) ((B), (A), vHitPt); 

#define	DO_SAME_COLLISION(type1, type2, collisionHandler) \
				case COLLISION_OF ((type1), (type1)): \
					return (collisionHandler) ((A), (B), vHitPt);

//these next two macros define a case that does nothing
#define	NO_COLLISION(type1, type2, collisionHandler) \
				case COLLISION_OF ((type1), (type2)): \
					break; \
				case COLLISION_OF ((type2), (type1)): \
					break;

#define	NO_SAME_COLLISION(type1, type2, collisionHandler) \
				case COLLISION_OF ((type1), (type1)): \
					break;

/* DPH: These ones are never used so I'm not going to bother */
#ifndef __GNUC__
#define IGNORE_COLLISION(type1, type2, collisionHandler) \
	case COLLISION_OF ((type1), (type2)): \
		break; \
	case COLLISION_OF ((type2), (type1)): \
		break;

#define ERROR_COLLISION(type1, type2, collisionHandler) \
	case COLLISION_OF ((type1), (type2)): \
		Error ("Error in collision nType!"); \
		break; \
	case COLLISION_OF ((type2), (type1)): \
		Error ("Error in collision nType!"); \
		break;
#endif

int CollideTwoObjects (tObject * A, tObject * B, vmsVector *vHitPt)
{
	int collisionType;	
		
	collisionType = COLLISION_OF (A->nType, B->nType);

	switch (collisionType)	{
	NO_SAME_COLLISION (OBJ_FIREBALL, OBJ_FIREBALL,  CollideFireballAndFireball)
	DO_SAME_COLLISION (OBJ_ROBOT, 	OBJ_ROBOT, 		CollideRobotAndRobot)
	NO_SAME_COLLISION (OBJ_HOSTAGE, 	OBJ_HOSTAGE, 	CollideHostageAndHostage)
	DO_SAME_COLLISION (OBJ_PLAYER, 	OBJ_PLAYER, 	CollidePlayerAndPlayer)
	DO_SAME_COLLISION (OBJ_WEAPON, 	OBJ_WEAPON, 	CollideWeaponAndWeapon)
	NO_SAME_COLLISION (OBJ_CAMERA, 	OBJ_CAMERA, 	CollideCameraAndCamera)
	NO_SAME_COLLISION (OBJ_POWERUP, 	OBJ_POWERUP, 	collidePowerupAndPowerup)
	NO_SAME_COLLISION (OBJ_DEBRIS, 	OBJ_DEBRIS, 	collideDebrisAndDebris)
	NO_SAME_COLLISION (OBJ_MARKER, 	OBJ_MARKER, 	NULL)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_ROBOT, 		CollideFireballAndRobot)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_HOSTAGE, 	CollideFireballAndHostage)
	NO_COLLISION 		(OBJ_FIREBALL, OBJ_PLAYER, 	CollideFireballAndPlayer)
	NO_COLLISION 		(OBJ_FIREBALL, OBJ_WEAPON, 	CollideFireballAndWeapon)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_CAMERA, 	CollideFireballAndCamera)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_POWERUP, 	CollideFireballAndPowerup)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_DEBRIS, 	CollideFireballAndDebris)
	NO_COLLISION		(OBJ_ROBOT, 	OBJ_HOSTAGE, 	CollideRobotAndHostage)
	DO_COLLISION		(OBJ_ROBOT, 	OBJ_PLAYER, 	CollideRobotAndPlayer)
	DO_COLLISION		(OBJ_ROBOT, 	OBJ_WEAPON, 	CollideRobotAndWeapon)
	NO_COLLISION		(OBJ_ROBOT, 	OBJ_CAMERA, 	CollideRobotAndCamera)
	NO_COLLISION		(OBJ_ROBOT, 	OBJ_POWERUP, 	CollideRobotAndPowerup)
	NO_COLLISION		(OBJ_ROBOT, 	OBJ_DEBRIS, 	CollideRobotAndDebris)
	DO_COLLISION		(OBJ_ROBOT, 	OBJ_CNTRLCEN, 	CollideRobotAndReactor)
	DO_COLLISION		(OBJ_HOSTAGE, 	OBJ_PLAYER, 	CollideHostageAndPlayer)
	NO_COLLISION		(OBJ_HOSTAGE, 	OBJ_WEAPON, 	CollideHostageAndWeapon)
	NO_COLLISION		(OBJ_HOSTAGE, 	OBJ_CAMERA, 	CollideHostageAndCamera)
	NO_COLLISION		(OBJ_HOSTAGE, 	OBJ_POWERUP, 	CollideHostageAndPowerup)
	NO_COLLISION		(OBJ_HOSTAGE, 	OBJ_DEBRIS, 	CollideHostageAndDebris)
	DO_COLLISION		(OBJ_PLAYER, 	OBJ_WEAPON, 	CollidePlayerAndWeapon)
	NO_COLLISION		(OBJ_PLAYER, 	OBJ_CAMERA, 	CollidePlayerAndCamera)
	DO_COLLISION		(OBJ_PLAYER, 	OBJ_POWERUP, 	CollidePlayerAndPowerup)
	NO_COLLISION		(OBJ_PLAYER, 	OBJ_DEBRIS, 	CollidePlayerAndDebris)
	DO_COLLISION		(OBJ_PLAYER, 	OBJ_CNTRLCEN, 	CollidePlayerAndReactor)
	DO_COLLISION		(OBJ_PLAYER, 	OBJ_CLUTTER, 	CollidePlayerAndClutter)
	DO_COLLISION		(OBJ_PLAYER, 	OBJ_MONSTERBALL, 	CollidePlayerAndMonsterball)
	NO_COLLISION		(OBJ_WEAPON, 	OBJ_CAMERA, 	CollideWeaponAndCamera)
	NO_COLLISION		(OBJ_WEAPON, 	OBJ_POWERUP, 	CollideWeaponAndPowerup)
	DO_COLLISION		(OBJ_WEAPON, 	OBJ_DEBRIS, 	CollideWeaponAndDebris)
	DO_COLLISION		(OBJ_WEAPON, 	OBJ_CNTRLCEN, 	CollideWeaponAndReactor)
	DO_COLLISION		(OBJ_WEAPON, 	OBJ_CLUTTER, 	CollideWeaponAndClutter)
	DO_COLLISION		(OBJ_WEAPON, 	OBJ_MONSTERBALL, 	CollideWeaponAndMonsterball)
	NO_COLLISION		(OBJ_CAMERA, 	OBJ_POWERUP, 	CollideCameraAndPowerup)
	NO_COLLISION		(OBJ_CAMERA, 	OBJ_DEBRIS, 	CollideCameraAndDebris)
	NO_COLLISION		(OBJ_POWERUP, 	OBJ_DEBRIS, 	CollidePowerupAndDebris)

	DO_COLLISION		(OBJ_MARKER, 	OBJ_PLAYER, 	CollidePlayerAndMarker)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_ROBOT, 		NULL)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_HOSTAGE, 	NULL)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_WEAPON, 	NULL)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_CAMERA, 	NULL)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_POWERUP, 	NULL)
	NO_COLLISION		(OBJ_MARKER, 	OBJ_DEBRIS, 	NULL)

	DO_COLLISION		(OBJ_CAMBOT, 	OBJ_WEAPON, 	CollideRobotAndWeapon)
	DO_COLLISION		(OBJ_CAMBOT, 	OBJ_PLAYER, 	CollideRobotAndPlayer)
	NO_COLLISION		(OBJ_FIREBALL, OBJ_CAMBOT, 	CollideFireballAndRobot)
	default:
		Int3 ();	//Error ("Unhandled collisionType in Collide.c! \n");
	}
return 1;
}

#define ENABLE_COLLISION(type1, type2) \
	CollisionResult [type1][type2] = RESULT_CHECK; \
	CollisionResult [type2][type1] = RESULT_CHECK;

#define DISABLE_COLLISION(type1, type2) \
	CollisionResult [type1][type2] = RESULT_NOTHING; \
	CollisionResult [type2][type1] = RESULT_NOTHING;

void CollideInit ()	
{
	int i, j;

	for (i=0; i < MAX_OBJECT_TYPES; i++)
		for (j=0; j < MAX_OBJECT_TYPES; j++)
			CollisionResult [i][j] = RESULT_NOTHING;

	ENABLE_COLLISION (OBJ_WALL, OBJ_ROBOT);
	ENABLE_COLLISION (OBJ_WALL, OBJ_WEAPON);
	ENABLE_COLLISION (OBJ_WALL, OBJ_PLAYER);
	ENABLE_COLLISION (OBJ_WALL, OBJ_MONSTERBALL);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_FIREBALL);

	ENABLE_COLLISION (OBJ_ROBOT, OBJ_ROBOT);
//	DISABLE_COLLISION (OBJ_ROBOT, OBJ_ROBOT);	//	ALERT: WARNING: HACK: MK = RESPONSIBLE!TESTING!!

	DISABLE_COLLISION (OBJ_HOSTAGE, OBJ_HOSTAGE);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_PLAYER);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_WEAPON);
	DISABLE_COLLISION (OBJ_PLAYER, OBJ_CAMERA);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_PLAYER, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_CNTRLCEN)
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_CLUTTER)
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_MARKER);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_CAMBOT);
	ENABLE_COLLISION  (OBJ_PLAYER, OBJ_MONSTERBALL);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_ROBOT);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_HOSTAGE);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_PLAYER);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_WEAPON);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_CAMERA);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_FIREBALL, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_FIREBALL, OBJ_CAMBOT);
	DISABLE_COLLISION (OBJ_ROBOT, OBJ_HOSTAGE);
	ENABLE_COLLISION  (OBJ_ROBOT, OBJ_PLAYER);
	ENABLE_COLLISION  (OBJ_ROBOT, OBJ_WEAPON);
	DISABLE_COLLISION (OBJ_ROBOT, OBJ_CAMERA);
	DISABLE_COLLISION (OBJ_ROBOT, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_ROBOT, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_ROBOT, OBJ_CNTRLCEN)
	ENABLE_COLLISION  (OBJ_HOSTAGE, OBJ_PLAYER);
	ENABLE_COLLISION  (OBJ_HOSTAGE, OBJ_WEAPON);
	DISABLE_COLLISION (OBJ_HOSTAGE, OBJ_CAMERA);
	DISABLE_COLLISION (OBJ_HOSTAGE, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_HOSTAGE, OBJ_DEBRIS);
	DISABLE_COLLISION (OBJ_WEAPON, OBJ_CAMERA);
	DISABLE_COLLISION (OBJ_WEAPON, OBJ_POWERUP);
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_WEAPON);
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_CNTRLCEN)
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_CLUTTER)
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_CAMBOT);
	ENABLE_COLLISION  (OBJ_WEAPON, OBJ_MONSTERBALL);
	DISABLE_COLLISION (OBJ_CAMERA, OBJ_CAMERA);
	DISABLE_COLLISION (OBJ_CAMERA, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_CAMERA, OBJ_DEBRIS);
	DISABLE_COLLISION (OBJ_POWERUP, OBJ_POWERUP);
	DISABLE_COLLISION (OBJ_POWERUP, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_POWERUP, OBJ_WALL);
	DISABLE_COLLISION (OBJ_DEBRIS, OBJ_DEBRIS);
	ENABLE_COLLISION  (OBJ_ROBOT, OBJ_CAMBOT);

}

int CollideObjectWithWall (tObject * A, fix hitspeed, short hitseg, short hitwall, vmsVector * vHitPt)
{
switch (A->nType)	{
	case OBJ_NONE:
		Error ("A tObject of nType NONE hit a wall! \n");
		break;
	case OBJ_PLAYER:		
		CollidePlayerAndWall (A, hitspeed, hitseg, hitwall, vHitPt); 
		break;
	case OBJ_WEAPON:		
		CollideWeaponAndWall (A, hitspeed, hitseg, hitwall, vHitPt); 
		break;
	case OBJ_DEBRIS:		
		CollideDebrisAndWall (A, hitspeed, hitseg, hitwall, vHitPt); 
		break;
	case OBJ_FIREBALL:	
		break;		//collideFireballAnd_wall (A, hitspeed, hitseg, hitwall, vHitPt); 
	case OBJ_ROBOT:		
		CollideRobotAndWall (A, hitspeed, hitseg, hitwall, vHitPt); 
		break;
	case OBJ_HOSTAGE:		
		break;		//collideHostageAnd_wall (A, hitspeed, hitseg, hitwall, vHitPt); 
	case OBJ_CAMERA:		
		break;		//collideCameraAnd_wall (A, hitspeed, hitseg, hitwall, vHitPt); 
	case OBJ_POWERUP:		
		break;		//collidePowerupAnd_wall (A, hitspeed, hitseg, hitwall, vHitPt); 
	case OBJ_GHOST:		
		break;	//do nothing
	case OBJ_MONSTERBALL:		
		break;		//collidePowerupAnd_wall (A, hitspeed, hitseg, hitwall, vHitPt); 
	default:
		Error ("Unhandled tObject nType hit wall in Collide.c \n");
	}
return 1;
}


