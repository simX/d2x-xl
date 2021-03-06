/* $Id: controls.c,v 1.5 2003/08/02 20:36:12 btb Exp $ */
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
 * Code for controlling player movement
 *
 * Old Log:
 * Revision 1.3  1995/11/20  17:17:27  allender
 * call fix_fastsincos with tmp variable to prevent
 * writing to NULL
 *
 * Revision 1.2  1995/08/11  16:00:04  allender
 * fixed bug we think we never saw (overflow on max_rotthrust
 *
 * Revision 1.1  1995/05/16  15:23:53  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:27:11  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.50  1995/02/22  14:11:19  allender
 * remove anonymous unions from tObject structure
 *
 * Revision 1.49  1994/12/15  13:04:10  mike
 * Replace gameData.multi.players[gameData.multi.nLocalPlayer].timeTotal references with gameData.time.xGame.
 *
 * Revision 1.48  1994/11/27  23:12:13  matt
 * Made changes for new con_printf calling convention
 *
 * Revision 1.47  1994/11/25  22:15:54  matt
 * Added asserts to try to trap frametime < 0 bug
 *
 * Revision 1.46  1994/11/16  11:25:40  matt
 * Took out int3's since I determined that the errors are caused by neg frametime
 *
 * Revision 1.45  1994/11/15  13:07:23  matt
 * Added int3's to try to trap bug
 *
 * Revision 1.44  1994/10/14  16:18:12  john
 * Made Assert that the tObject was player just nicely exit
 * the function.
 *
 * Revision 1.43  1994/10/13  11:35:25  john
 * Made Thrustmaster FCS Hat work.  Put a background behind the
 * keyboard configure.  Took out turn_sensitivity.  Changed sound/config
 * menu to new menu. Made F6 be calibrate joystick.
 *
 * Revision 1.42  1994/09/29  11:22:02  mike
 * Zero thrust when player dies.
 *
 * Revision 1.41  1994/09/16  13:10:30  mike
 * Hook in afterburner stuff.
 *
 * Revision 1.40  1994/09/14  22:21:54  matt
 * Avoid post-death assert
 *
 * Revision 1.39  1994/09/11  20:30:27  matt
 * Cleaned up thrust vars, changing a few names
 *
 * Revision 1.38  1994/09/10  15:46:31  john
 * First version of new keyboard configuration.
 *
 * Revision 1.37  1994/09/07  15:58:12  mike
 * Check for player dead in controls so you can't fire or move after dead, logical, huh?
 *
 * Revision 1.36  1994/09/06  14:51:56  john
 * Added sensitivity adjustment, fixed bug with joystick button not
 * staying down.
 *
 * Revision 1.35  1994/09/01  15:43:26  john
 * Put pitch bak like it was.
 *
 * Revision 1.34  1994/08/31  18:59:35  john
 * Made rotThrust back like it was.
 *
 * Revision 1.33  1994/08/31  18:49:17  john
 * Slowed Maxrothrust a bit,
 * ..
 *
 * Revision 1.32  1994/08/31  18:32:05  john
 * Lower max rotational thrust
 *
 * Revision 1.31  1994/08/29  21:18:27  john
 * First version of new keyboard/oystick remapping stuff.
 *
 * Revision 1.30  1994/08/29  16:18:30  mike
 * trap divide overflow.
 *
 * Revision 1.29  1994/08/26  14:40:45  john
 * *** empty log message ***
 *
 * Revision 1.28  1994/08/26  12:23:50  john
 * MAde joystick read up to 15 times per second max.
 *
 * Revision 1.27  1994/08/26  10:50:59  john
 * Took out Controls_always_stopped.
 *
 * Revision 1.26  1994/08/26  10:47:27  john
 * *** empty log message ***
 *
 * Revision 1.25  1994/08/26  10:46:50  john
 * New version of controls.
 *
 * Revision 1.24  1994/08/25  19:41:44  john
 * *** empty log message ***
 *
 * Revision 1.23  1994/08/25  18:44:55  john
 * *** empty log message ***
 *
 * Revision 1.22  1994/08/25  18:43:46  john
 * First revision of new control code.
 *
 * Revision 1.21  1994/08/24  20:02:46  john
 * Added cyberman support; made keys work KeyDownTime
 * returning seconds instead of milliseconds,.
 *
 *
 * Revision 1.20  1994/08/24  19:00:27  john
 * Changed KeyDownTime to return fixed seconds instead of
 * milliseconds.
 *
 * Revision 1.19  1994/08/19  15:22:12  mike
 * Fix divide overflow in sliding.
 *
 * Revision 1.18  1994/08/19  14:42:50  john
 * Added joystick sensitivity.
 *
 * Revision 1.17  1994/08/17  16:50:01  john
 * Added damaging fireballs, missiles.
 *
 * Revision 1.16  1994/08/12  22:41:54  john
 * Took away Player_stats; added gameData.multi.players array.
 *
 * Revision 1.15  1994/08/09  16:03:56  john
 * Added network players to editor.
 *
 * Revision 1.14  1994/07/28  12:33:31  matt
 * Made sliding use thrust, rather than changing velocity directly
 *
 * Revision 1.13  1994/07/27  20:53:21  matt
 * Added rotational drag & thrust, so turning now has momemtum like moving
 *
 * Revision 1.12  1994/07/25  10:24:06  john
 * Victor stuff.
 *
 * Revision 1.11  1994/07/22  17:53:16  john
 * Added better victormax support
 *
 * Revision 1.10  1994/07/21  21:31:29  john
 * First cheapo version of VictorMaxx tracking.
 *
 * Revision 1.9  1994/07/15  15:16:18  john
 * Fixed some joystick stuff.
 *
 * Revision 1.8  1994/07/15  09:32:09  john
 * Changes player movement.
 *
 * Revision 1.7  1994/07/13  00:14:58  matt
 * Moved all (or nearly all) of the values that affect player movement to
 * bitmaps.tbl
 *
 * Revision 1.6  1994/07/12  12:40:14  matt
 * Revamped physics system
 *
 * Revision 1.5  1994/07/02  13:50:39  matt
 * Cleaned up includes
 *
 * Revision 1.4  1994/07/01  10:55:25  john
 * Added analog joystick throttle
 *
 * Revision 1.3  1994/06/30  20:04:28  john
 * Added -joydef support.
 *
 * Revision 1.2  1994/06/30  19:01:58  matt
 * Moved flying controls code from physics.c to controls.c
 *
 * Revision 1.1  1994/06/30  18:41:25  matt
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: controls.c,v 1.5 2003/08/02 20:36:12 btb Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pstypes.h"
#include "mono.h"
#include "key.h"
#include "joy.h"
#include "timer.h"
#include "error.h"

#include "inferno.h"
#include "game.h"
#include "object.h"
#include "player.h"

#include "controls.h"
#include "joydefs.h"
#include "render.h"
#include "args.h"
#include "palette.h"
#include "mouse.h"
#include "kconfig.h"
#include "laser.h"
#ifdef NETWORK
#include "network.h"
#include "multi.h"
#endif
#include "vclip.h"
#include "fireball.h"
#include "hudmsg.h"
#include "input.h"

// ----------------------------------------------------------------------------

static fix wiggleTime;

void WiggleObject (tObject *objP)
{
if ((gameStates.render.nShadowPass != 2) &&
	 (EGI_FLAG (bWiggle, 0, 1))) {
	fix swiggle;
	int nParent = gameData.objs.parentObjs [OBJ_IDX (objP)];
	tObject *pParent = (nParent < 0) ? NULL : gameData.objs.objects + nParent;
	fix_fastsincos (gameData.time.xGame, &swiggle, NULL);
	if (wiggleTime < F1_0)// Only scale wiggle if getting at least 1 FPS, to avoid causing the opposite problem.
		swiggle = FixMul (swiggle * 20, wiggleTime); //make wiggle fps-independent (based on pre-scaled amount of wiggle at 20 FPS)
	if ((objP->nType == OBJ_PLAYER) || !pParent)
		VmVecScaleInc (&objP->mType.physInfo.velocity,
								 &objP->orient.uVec,
								 FixMul (swiggle, gameData.pig.ship.player->wiggle));
#if 1
	else {
		VmVecScaleInc (&objP->mType.physInfo.velocity,
								&pParent->orient.uVec,
								FixMul (swiggle, gameData.pig.ship.player->wiggle));
		VmVecScaleInc (&objP->pos, &objP->mType.physInfo.velocity, wiggleTime);
		}
#endif
	}
}

// ----------------------------------------------------------------------------
//look at keyboard, mouse, joystick, CyberMan, whatever, and set 
//physics vars rotVel, velocity

fix xAfterburnerCharge=f1_0;
vmsVector player_thrust;

#define AFTERBURNER_USE_SECS	3				//use up in 3 seconds
#define DROP_DELTA_TIME			(f1_0/15)	//drop 3 per second

void ReadFlyingControls(tObject *objP)
{
	fix	forward_thrustTime;
	tObject *gmObjP;
	int	bMulti;
#if 0
	Assert(gameData.time.xFrame > 0); 		//Get MATT if hit this!
#else
	if (gameData.time.xFrame <= 0)
		return;
#endif

	if (gameStates.app.bPlayerIsDead || gameStates.app.bEnterGame) {
		StopPlayerMovement ();
		FlushInput ();
/*
		VmVecZero(&objP->mType.physInfo.rotThrust);
		VmVecZero(&objP->mType.physInfo.thrust);
		VmVecZero(&objP->mType.physInfo.velocity);
*/		
		gameStates.app.bEnterGame--;
		return;
	}

	if ((objP->nType!=OBJ_PLAYER) || (objP->id!=gameData.multi.nLocalPlayer)) 
		return;	//references to tPlayerShip require that this obj be the player

	gmObjP = gameData.objs.guidedMissile[gameData.multi.nLocalPlayer];
	if (gmObjP && (gmObjP->nSignature == gameData.objs.guidedMissileSig[gameData.multi.nLocalPlayer])) {
		vmsAngVec rotangs;
		vmsMatrix rotmat,tempm;
		fix speed;

		//this is a horrible hack.  guided missile stuff should not be
		//handled in the middle of a routine that is dealing with the player
		VmVecZero(&objP->mType.physInfo.rotThrust);
		rotangs.p = Controls.pitchTime / 2 + gameStates.gameplay.seismic.nMagnitude/64;
		rotangs.b = Controls.bankTime / 2 + gameStates.gameplay.seismic.nMagnitude/16;
		rotangs.h = Controls.headingTime / 2 + gameStates.gameplay.seismic.nMagnitude/64;
		VmAngles2Matrix(&rotmat,&rotangs);
		VmMatMul(&tempm,&gameData.objs.guidedMissile[gameData.multi.nLocalPlayer]->orient,&rotmat);
		gameData.objs.guidedMissile[gameData.multi.nLocalPlayer]->orient = tempm;
		speed = WI_speed (gmObjP->id,gameStates.app.nDifficultyLevel);
		VmVecCopyScale(&gmObjP->mType.physInfo.velocity, &gmObjP->orient.fVec,speed);
#ifdef NETWORK
		if (gameData.app.nGameMode & GM_MULTI)
			MultiSendGuidedInfo (gmObjP, 0);
#endif

	}
	else {
		objP->mType.physInfo.rotThrust.x = Controls.pitchTime;
		objP->mType.physInfo.rotThrust.y = Controls.headingTime;
		objP->mType.physInfo.rotThrust.z = Controls.bankTime;
	}
	forward_thrustTime = Controls.forward_thrustTime;
	if (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_AFTERBURNER)	{
		if (Controls.afterburner_state) {			//player has key down
			//if (forward_thrustTime >= 0) { 		//..and isn't moving backward
			{
				fix afterburner_scale;
				int old_count,new_count;
	
				//add in value from 0..1
				afterburner_scale = f1_0 + min (f1_0/2, xAfterburnerCharge) * 2;
				forward_thrustTime = FixMul (gameData.time.xFrame, afterburner_scale);	//based on full thrust
				old_count = (xAfterburnerCharge / (DROP_DELTA_TIME / AFTERBURNER_USE_SECS));
				xAfterburnerCharge -= gameData.time.xFrame / AFTERBURNER_USE_SECS;
				if (xAfterburnerCharge < 0)
					xAfterburnerCharge = 0;
				new_count = (xAfterburnerCharge / (DROP_DELTA_TIME / AFTERBURNER_USE_SECS));
				if (gameStates.app.bNostalgia && (old_count != new_count))
					gameStates.render.bDropAfterburnerBlob = 1;	//drop blob (after physics called)
			}
		}
		else {
			fix cur_energy,charge_up;
	
			//charge up to full
			charge_up = min(gameData.time.xFrame/8,f1_0 - xAfterburnerCharge);	//recharge over 8 seconds
			cur_energy = gameData.multi.players [gameData.multi.nLocalPlayer].energy - i2f (10);
			cur_energy = max(cur_energy, 0);	//don't drop below 10
			//maybe limit charge up by energy
			charge_up = min (charge_up,cur_energy / 10);
			xAfterburnerCharge += charge_up;
			gameData.multi.players [gameData.multi.nLocalPlayer].energy -= charge_up * 100 / 10;	//full charge uses 10% of energy
		}
	}
	// Set tObject's thrust vector for forward/backward
	VmVecCopyScale (&objP->mType.physInfo.thrust, &objP->orient.fVec, forward_thrustTime);
	// slide left/right
	VmVecScaleInc (&objP->mType.physInfo.thrust, &objP->orient.rVec, Controls.sideways_thrustTime);
	// slide up/down
	VmVecScaleInc (&objP->mType.physInfo.thrust, &objP->orient.uVec, Controls.vertical_thrustTime);
	if (!gameStates.input.bSkipControls)
		memcpy (&player_thrust, &objP->mType.physInfo.thrust, sizeof (player_thrust));
	//HUDMessage (0, "%d %d %d", player_thrust.x, player_thrust.y, player_thrust.z);
	bMulti = IsMultiGame;
	if ((objP->mType.physInfo.flags & PF_WIGGLE) && !gameData.objs.speedBoost [OBJ_IDX (objP)].bBoosted) {
#if 1//ndef _DEBUG
		wiggleTime = gameData.time.xFrame;
		WiggleObject (objP);
#endif		
	}
	// As of now, objP->mType.physInfo.thrust & objP->mType.physInfo.rotThrust are 
	// in units of time... In other words, if thrust==gameData.time.xFrame, that
	// means that the user was holding down the Max_thrust key for the
	// whole frame.  So we just scale them up by the max, and divide by
	// gameData.time.xFrame to make them independant of framerate

	//	Prevent divide overflows on high frame rates.
	//	In a signed divide, you get an overflow if num >= div<<15
	{
		fix	ft = gameData.time.xFrame;

		//	Note, you must check for ft < F1_0/2, else you can get an overflow  on the << 15.
		if ((ft < F1_0/2) && ((ft << 15) <= gameData.pig.ship.player->max_thrust)) {
#if TRACE
			con_printf (CON_DEBUG, "Preventing divide overflow in controls.c for Max_thrust!\n");
#endif
			ft = (gameData.pig.ship.player->max_thrust >> 15) + 1;
		}
		VmVecScale( &objP->mType.physInfo.thrust, FixDiv(gameData.pig.ship.player->max_thrust,ft) );

		if ((ft < F1_0/2) && ((ft << 15) <= gameData.pig.ship.player->max_rotthrust)) {
#if TRACE
			con_printf (CON_DEBUG, "Preventing divide overflow in controls.c for max_rotthrust!\n");
#endif
			ft = (gameData.pig.ship.player->max_thrust >> 15) + 1;
		}
		VmVecScale( &objP->mType.physInfo.rotThrust, FixDiv(gameData.pig.ship.player->max_rotthrust,ft) );
	}

}

// ----------------------------------------------------------------------------
//eof
