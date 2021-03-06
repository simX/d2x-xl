/* $Id: collide.h,v 1.2 2003/10/10 09:36:34 btb Exp $ */
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
 * Header for collide.c
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:55:09  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:28:59  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.23  1995/01/26  22:11:47  mike
 * Purple chromo-blaster (ie, fusion cannon) spruce up (chromification)
 *
 * Revision 1.22  1994/12/21  19:03:24  rob
 * Fixing score accounting for multiplayer robots
 *
 * Revision 1.21  1994/12/21  11:34:56  mike
 * make control center take badass damage.
 *
 * Revision 1.20  1994/10/29  16:53:16  allender
 * added CheckEffectBlowup for demo recording to record monitor explosions
 *
 * Revision 1.19  1994/10/09  13:46:48  mike
 * Make public CollidePlayerAndPowerup.
 *
 * Revision 1.18  1994/10/05  14:54:29  rob
 * Added serial game support in control center sequence..
 *
 * Revision 1.17  1994/09/15  16:32:12  mike
 * Prototype CollidePlayerAndNastyRobot.
 *
 * Revision 1.16  1994/09/11  15:49:04  mike
 * Prototype for MaybeDetonateWeapon.
 *
 * Revision 1.15  1994/09/09  14:20:07  matt
 * Added prototype for scrape function
 *
 * Revision 1.14  1994/09/02  14:00:44  matt
 * Simplified ExplodeObject() & mutliple-stage explosions
 *
 * Revision 1.13  1994/08/18  10:47:35  john
 * Cleaned up game sequencing and player death stuff
 * in preparation for making the player explode into
 * pieces when dead.
 *
 * Revision 1.12  1994/08/17  16:50:08  john
 * Added damaging fireballs, missiles.
 *
 * Revision 1.11  1994/08/03  16:45:31  mike
 * Prototype a function.
 *
 * Revision 1.10  1994/08/03  15:17:41  mike
 * make matcen whack on you if it's ready to make a robot.
 *
 * Revision 1.9  1994/07/22  12:08:03  mike
 * Make robot hit tVideoClip and sound weapon-based until robot dies, then robot-based.
 *
 * Revision 1.8  1994/07/09  17:36:31  mike
 * Prototype ApplyDamageToPlayer.
 *
 * Revision 1.7  1994/07/09  13:20:36  mike
 * Prototype ApplyDamageToPlayer.
 *
 * Revision 1.6  1994/06/20  23:35:52  john
 * Bunch of stuff.
 *
 * Revision 1.5  1994/06/17  18:04:03  yuan
 * Added Immaterialization...
 * Fixed Invulnerability to allow being hit.
 *
 * Revision 1.4  1994/05/13  20:28:02  john
 * Version II of John's new tObject code.
 *
 * Revision 1.3  1994/05/13  12:20:35  john
 * Fixed some potential problems with code using global variables
 * that are set in fvi.
 *
 * Revision 1.2  1994/05/12  23:20:32  john
 * Moved all tObject collision handling into collide.c.
 *
 * Revision 1.1  1994/05/12  20:39:09  john
 * Initial revision
 *
 *
 */

#ifndef _COLLIDE_H
#define _COLLIDE_H

void CollideInit();
int CollideTwoObjects(tObject * A, tObject * B, vmsVector *collision_point);
int CollideObjectWithWall(tObject * A, fix hitspeed, short hitseg, short hitwall, vmsVector * hitpt);
void ApplyDamageToPlayer(tObject *player, tObject *killer, fix damage);

// Returns 1 if robot died, else 0.
int ApplyDamageToRobot(tObject *robot, fix damage, int nKillerObj);

extern int Immaterial;

int CollidePlayerAndWeapon(tObject * player, tObject * weapon, vmsVector *collision_point);
int CollidePlayerAndMatCen(tObject *objp);
int CollideRobotAndMatCen(tObject *objp);

void ScrapeObjectOnWall(tObject *obj, short hitseg, short hitwall, vmsVector * hitpt);
int MaybeDetonateWeapon(tObject *obj0p, tObject *obj, vmsVector *pos);

int CollidePlayerAndNastyRobot(tObject * player, tObject * robot, vmsVector *collision_point);

void NetDestroyReactor(tObject *controlcen);
int CollidePlayerAndPowerup(tObject * player, tObject * powerup, vmsVector *collision_point);
int CheckEffectBlowup(tSegment *seg,short tSide,vmsVector *pnt, tObject *blower, int force_blowupFlag);
void ApplyDamageToReactor(tObject *controlcen, fix damage, short who);
void BumpOneObject(tObject *obj0, vmsVector *hit_dir, fix damage);
void SetMonsterballForces (void);

#endif /* _COLLIDE_H */
