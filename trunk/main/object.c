/* $Id: object.c, v 1.9 2003/10/04 03:14:47 btb Exp $ */
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
 * object rendering
 *
 * Old Log:
 * Revision 1.5  1995/10 / 26  14:08:03  allender
 * optimization to do physics on gameData.objs.objects that didn't render last
 * frame only every so often
 *
 * Revision 1.4  1995/10 / 20  00:50:57  allender
 * make alt texture for player ship work correctly when cloaked
 *
 * Revision 1.3  1995/09/14  14:11:32  allender
 * FixObjectSegs returns void
 *
 * Revision 1.2  1995/08/12  11:31:01  allender
 * removed #ifdef NEWDEMO -- always in
 *
 * Revision 1.1  1995/05/16  15:29:23  allender
 * Initial revision
 *
 * Revision 2.3  1995/06/15  12:30:51  john
 * Fixed bug with multiplayer ships cloaking out wrongly.
 *
 * Revision 2.2  1995/05/15  11:34:53  john
 * Fixed bug as Matt directed that fixed problems with the exit
 * triggers being missed on slow frame rate computer.
 *
 * Revision 2.1  1995/03 / 21  14:38:51  john
 * Ifdef'd out the NETWORK code.
 *
 * Revision 2.0  1995/02 / 27  11:28:14  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.335  1995/02 / 22  12:57:30  allender
 * remove anonymous unions from object structure
 *
 * Revision 1.334  1995/02/09  22:04:40  mike
 * fix lifeleft on badass weapons.
 *
 * Revision 1.333  1995/02/08  12:54:00  matt
 * Fixed object freeing code which was deleting some explosions it shouldn't
 *
 * Revision 1.332  1995/02/08  11:37:26  mike
 * Check for failures in call to CreateObject.
 *
 * Revision 1.331  1995/02/05  17:48:52  rob
 * Changed assert in RelinkObject, more robust.
 *
 * Revision 1.330  1995/02/05  13:39:48  mike
 * remove invulnerability effect code (actually, comment out).
 *
 * Revision 1.329  1995/02/04  12:29:52  rob
 * Get rid of potential assert error for explosion detachment.
 *
 * Revision 1.328  1995/02/01  18:15:57  rob
 * Removed debugging output from engine glow change.
 *
 * Revision 1.327  1995/02/01  16:20:12  matt
 * Made engine glow vary over a wider range, and made the glow be based
 * on thrust/speed, without regard to direction.
 *
 * Revision 1.326  1995/01 / 29  14:46:24  rob
 * Fixed invul. vclip to only appear on player who is invul.
 *
 * Revision 1.325  1995/01 / 29  13:48:16  mike
 * Add invulnerability graphical effect viewable by other players.
 *
 * Revision 1.324  1995/01 / 29  11:39:25  mike
 * Add invulnerability effect.
 *
 * Revision 1.323  1995/01 / 27  17:02:41  mike
 * add more information to an Error call.
 *
 * Revision 1.322  1995/01 / 26  22:11:30  mike
 * Purple chromo-blaster (ie, fusion cannon) spruce up (chromification)
 *
 * Revision 1.321  1995/01 / 25  20:04:10  matt
 * Moved matrix check to avoid orthogonalizing an uninitialize matrix
 *
 * Revision 1.320  1995/01 / 25  12:11:35  matt
 * Make sure orient matrix is orthogonal when resetting player object
 *
 * Revision 1.319  1995/01 / 21  21:46:22  mike
 * Optimize code in Assert (and prevent warning message).
 *
 * Revision 1.318  1995/01 / 21  21:22:16  rob
 * Removed HUD clear messages.
 * Added more Asserts to try and find illegal control type bug.
 *
 * Revision 1.317  1995/01/15  15:34:30  matt
 * When freeing object slots, don't d_free fireballs that will be deleting
 * other gameData.objs.objects.
 *
 * Revision 1.316  1995/01/14  19:16:48  john
 * First version of new bitmap paging code.
 *
 * Revision 1.315  1995/01/12  18:53:37  john
 * Fixed parameter passing error.
 *
 * Revision 1.314  1995/01/12  12:09:47  yuan
 * Added coop object capability.
 *
 * Revision 1.313  1994/12/15  16:45:44  matt
 * Took out slew stuff for release version
 *
 * Revision 1.312  1994/12/15  13:04:25  mike
 * Replace gameData.multi.players [gameData.multi.nLocalPlayer].time_total references with gameData.time.xGame.
 *
 * Revision 1.311  1994/12/15  11:01:04  mike
 * add Object_minus_one for debugging.
 *
 * Revision 1.310  1994/12/15  03:03:33  matt
 * Added error checking for NULL return from ObjectCreateExplosion ()
 *
 * Revision 1.309  1994/12/14  17:25:31  matt
 * Made next viewer func based on release not ndebug
 *
 * Revision 1.308  1994/12/13  12:55:42  mike
 * hostages on board messages for when you die.
 *
 * Revision 1.307  1994/12/12  17:18:11  mike
 * make boss cloak/teleport when get hit, make quad laser 3/4 as powerful.
 *
 * Revision 1.306  1994/12/12  00:27:11  matt
 * Added support for no-levelling option
 *
 * Revision 1.305  1994/12/11  22:41:14  matt
 * Added command-line option, -nolevel, which turns off player ship levelling
 *
 * Revision 1.304  1994/12/11  22:03:23  mike
 * d_free up object slots as necessary.
 *
 * Revision 1.303  1994/12/11  14:09:31  mike
 * make boss explosion sounds softer.
 *
 * Revision 1.302  1994/12/11  13:25:11  matt
 * Restored calls to FixObjectSegs () when debugging is turned off, since
 * it's not a big routine, and could fix some possibly bad problems.
 *
 * Revision 1.301  1994/12/11  12:38:25  mike
 * make boss explosion sounds louder in create_small_fireball.
 *
 * Revision 1.300  1994/12/10  15:28:37  matt
 * Added asserts for debugging
 *
 * Revision 1.299  1994/12/09  16:18:51  matt
 * Fixed InitPlayerObject, for editor
 *
 * Revision 1.298  1994/12/09  15:03:10  matt
 * Two changes for Mike:
 *   1.  Do better placement of camera during death sequence (prevents hang)
 *   2.  Only record dodging information if the player fired in a frame
 *
 * Revision 1.297  1994/12/09  14:59:12  matt
 * Added system to attach a fireball to another object for rendering purposes, 
 * so the fireball always renders on top of (after) the object.
 *
 * Revision 1.296  1994/12/08  20:05:07  matt
 * Removed unneeded debug message
 *
 * Revision 1.295  1994/12/08  12:36:02  matt
 * Added new object allocation & deallocation functions so other code
 * could stop messing around with internal object data structures.
 *
 * Revision 1.294  1994/12/07  20:13:37  matt
 * Added debris object limiter
 *
 * Revision 1.293  1994/12/06  16:58:38  matt
 * Killed warnings
 *
 * Revision 1.292  1994/12/05  22:34:35  matt
 * Make tmap_override gameData.objs.objects use override texture as alt texture.  This
 * should have the effect of making simpler models use the override texture.
 *
 * Revision 1.291  1994/12/05  12:23:53  mike
 * make camera start closer, but move away from player in death sequence.
 *
 * Revision 1.290  1994/12/02  11:11:18  mike
 * hook sound effect to player small explosions (ctrlcen, too).
 *
 * Revision 1.289  1994/11 / 28  21:50:52  mike
 * optimizations.
 *
 * Revision 1.288  1994/11 / 27  23:12:28  matt
 * Made changes for new con_printf calling convention
 *
 * Revision 1.287  1994/11 / 27  20:35:50  matt
 * Fixed dumb mistake
 *
 * Revision 1.286  1994/11 / 27  20:30:52  matt
 * Got rid of warning
 *
 * Revision 1.285  1994/11 / 21  11:43:21  mike
 * ndebug stuff.
 *
 * Revision 1.284  1994/11/19  15:19:37  mike
 * rip out unused code and data.
 *
 * Revision 1.283  1994/11/18  23:41:59  john
 * Changed some shorts to ints.
 *
 * Revision 1.282  1994/11/18  16:16:17  mike
 * Separate depth on gameData.objs.objects vs. walls.
 *
 * Revision 1.281  1994/11/18  12:05:35  rob
 * Removed unnecessary invulnerability flag set in player death.
 * (I hope its unnecessary.. its commented out if it proves crucial)
 * fixes powerup dropping bug for net play.
 *
 * Revision 1.280  1994/11/16  20:36:34  rob
 * Changed player explosion (small) code.
 *
 * Revision 1.279  1994/11/16  18:26:04  matt
 * Clear tmap override on player, to fix "rock ship" bug
 *
 * Revision 1.278  1994/11/16  14:54:12  rob
 * Moved hook for network explosions.
 *
 * Revision 1.277  1994/11/14  11:40:42  mike
 * plot inner polygon on laser based on detail level.
 *
 * Revision 1.276  1994/11/10  14:02:59  matt
 * Hacked in support for player ships with different textures
 *
 * Revision 1.275  1994/11/08  12:19:08  mike
 * Make a generally useful function for putting small explosions on any object.
 *
 * Revision 1.274  1994/11/04  19:55:54  rob
 * Changed calls to player_explode to accomodate new parameter.
 *
 * Revision 1.273  1994/11/02  21:54:27  matt
 * Delete the camera when the death sequence is done
 *
 * Revision 1.272  1994/11/02  11:36:35  rob
 * Added player-in-process-of-dying explosions to network play.
 *
 * Revision 1.271  1994/10/31  17:25:33  matt
 * Fixed cloaked bug
 *
 * Revision 1.270  1994/10/31  16:11:19  allender
 * on demo recording, store letterbox mode in demo.
 *
 * Revision 1.269  1994/10/31  10:36:18  mike
 * Make cloak effect fadein/fadeout different for robots versus player.
 *
 * Revision 1.268  1994/10/30  14:11:44  mike
 * rip out repair center stuff.
 *
 * Revision 1.267  1994/10 / 28  19:43:52  mike
 * Boss cloaking effect.
 *
 * Revision 1.266  1994/10 / 27  11:33:42  mike
 * Add gameData.objs.nObjectLimit -- high water mark in object creation.
 *
 * Revision 1.265  1994/10 / 25  10:51:12  matt
 * Vulcan cannon powerups now contain ammo count
 *
 * Revision 1.264  1994/10 / 24  20:49:24  matt
 * Made cloaked gameData.objs.objects pulse
 *
 * Revision 1.263  1994/10 / 21  12:19:45  matt
 * Clear transient gameData.objs.objects when saving (& loading) games
 *
 * Revision 1.262  1994/10 / 21  11:25:23  mike
 * Use new constant IMMORTAL_TIME.
 *
 * Revision 1.261  1994/10/19  16:50:35  matt
 * If out of segment, put player in center of segment when checking gameData.objs.objects
 *
 *
 * Revision 1.260  1994/10/17  23:21:55  mike
 * Clean up robot cloaking, move more to ai.c
 *
 * Revision 1.259  1994/10/17  21:34:49  matt
 * Added support for new Control Center/Main Reactor
 *
 * Revision 1.258  1994/10/17  21:18:04  mike
 * robot cloaking.
 *
 * Revision 1.257  1994/10/17  14:12:23  matt
 * Cleaned up problems with player dying from mine explosion
 *
 * Revision 1.256  1994/10/15  19:04:31  mike
 * Don't remove proximity bombs after you die.
 *
 * Revision 1.255  1994/10/14  15:57:00  mike
 * Don't show ids in network mode.
 * Fix, I hope, but in death sequence.
 *
 * Revision 1.254  1994/10/12  08:04:29  mike
 * Don't decloak player on death.
 *
 * Revision 1.253  1994/10/11  20:36:16  matt
 * Clear "transient" gameData.objs.objects (weapons, explosions, etc.) when starting a level
 *
 * Revision 1.252  1994/10/11  12:24:09  matt
 * Cleaned up/change badass explosion calls
 *
 * Revision 1.251  1994/10/08  19:30:20  matt
 * Fixed (I hope) a bug in cloaking of multiplayer gameData.objs.objects
 *
 * Revision 1.250  1994/10/08  14:03:15  rob
 * Changed cloaking routine.
 *
 * Revision 1.249  1994/10/07  22:17:27  mike
 * Asserts on valid segnum.
 *
 * Revision 1.248  1994/10/07  19:11:14  matt
 * Added cool cloak transition effect
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef WINDOWS
#include "desw.h"
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

#include "sounds.h"
#include "collide.h"

#include "lighting.h"
#include "newdemo.h"
#include "player.h"
#include "weapon.h"
#ifdef NETWORK
#include "network.h"
#endif
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

#ifdef OGL
#include "ogl_init.h"
#endif
#include "kconfig.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#ifdef _3DFX
#include "3dfx_des.h"
#endif

extern vms_vector player_thrust;
extern int bSpeedBost;

void DetachAllObjects (object *parent);
void DetachOneObject (object *sub);
int FreeObjectSlots (int num_used);

/*
 *  Global variables
 */

ubyte CollisionResult [MAX_OBJECT_TYPES][MAX_OBJECT_TYPES];

short idToOOF [100];

//Data for gameData.objs.objects

// -- Object stuff

//info on the various types of gameData.objs.objects
#ifdef _DEBUG
object	Object_minus_one;
#endif

//------------------------------------------------------------------------------
// grs_bitmap *robot_bms [MAX_ROBOT_BITMAPS];	//all bitmaps for all robots

// int robot_bm_nums [MAX_ROBOT_TYPES];		//starting bitmap num for each robot
// int robot_n_bitmaps [MAX_ROBOT_TYPES];		//how many bitmaps for each robot

// char *gameData.bots.names [MAX_ROBOT_TYPES];		//name of each robot

//--unused-- int Num_robot_types = 0;

int bPrintObjectInfo = 0;
//@@int Object_viewer = 0;

//object * Slew_object = NULL;	// Object containing slew object info.

//--unused-- int Player_controller_type = 0;

window_rendered_data Window_rendered_data [MAX_RENDERED_WINDOWS];

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

#ifndef RELEASE
//set viewer object to next object in array
void ObjectGotoNextViewer ()
{
	int i, nStartObj = 0;

	nStartObj = OBJ_IDX (gameData.objs.viewer);		//get viewer object number
	for (i = 0;i<=gameData.objs.nLastObject;i++) {
		nStartObj++;
		if (nStartObj > gameData.objs.nLastObject) nStartObj = 0;

		if (gameData.objs.objects [nStartObj].type != OBJ_NONE)	{
			gameData.objs.viewer = gameData.objs.objects + nStartObj;
			return;
		}
	}
	Error ("Couldn't find a viewer object!");
}

//------------------------------------------------------------------------------
//set viewer object to next object in array
void ObjectGotoPrevViewer ()
{
	int i, nStartObj = 0;

	nStartObj = OBJ_IDX (gameData.objs.viewer);		//get viewer object number
	for (i = 0; i<=gameData.objs.nLastObject; i++) {
		nStartObj--;
		if (nStartObj < 0) nStartObj = gameData.objs.nLastObject;
		if (gameData.objs.objects [nStartObj].type != OBJ_NONE)	{
			gameData.objs.viewer = gameData.objs.objects + nStartObj;
			return;
		}
	}
	Error ("Couldn't find a viewer object!");
}
#endif

//------------------------------------------------------------------------------

object *ObjFindFirstOfType (int type)
 {
  int i;
  object	*objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->type==type)
	 return (objP);
  return ((object *)NULL);
 }


int obj_return_num_of_type (int type)
 {
  int i, count = 0;
	object *objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->type==type)
	 count++;
  return (count);
 }


int obj_return_num_of_typeid (int type, int id)
 {
  int i, count = 0;
	object	*objP = gameData.objs.objects;

  for (i=gameData.objs.nLastObject+1;i;i--, objP++)
	if (objP->type==type && objP->id==id)
	 count++;
  return (count);
 }

int global_orientation = 0;

//------------------------------------------------------------------------------
//draw an object that has one bitmap & doesn't rotate

extern tRgbColorf bitmapColors [MAX_BITMAP_FILES];

void DrawObjectBlob (object *objP, bitmap_index bmi0, bitmap_index bmi, int iFrame, tRgbColorf *color, float alpha)
{
	grs_bitmap	*bmP;
	int			id, orientation = 0;
	int			transp = (objP->type == OBJ_FIREBALL) && (objP->render_type != RT_THRUSTER);
	int			bDepthInfo = 1; // (objP->type != OBJ_FIREBALL);
	fix			xSize;

if (gameOpts->render.bTransparentEffects) {
	if (transp)
		alpha = 1.0;
	else if (!alpha) {
		if (objP->type == OBJ_POWERUP) {
			id = objP->id;
			if ((id == POW_EXTRA_LIFE) ||
				 (id == POW_ENERGY) ||
				 (id == POW_SHIELD_BOOST) ||
				 (id == POW_HOARD_ORB) ||
				 (id == POW_CLOAK) ||
				 (id == POW_INVULNERABILITY))
				alpha = 2.0f / 3.0f;
			else
				alpha = 1.0f;
			orientation = OBJ_IDX (objP) & 7;
			}
		else if (objP->type != OBJ_FIREBALL)
			alpha = 1.0f;
		else {
			orientation = (OBJ_IDX (objP)) & 7;
			alpha = 2.0f / 3.0f;
			}
		}
	}
else {
	transp = 0;
	alpha = 1.0f;
	}
orientation = global_orientation;
PIGGY_PAGE_IN (bmi, 0);
bmP = gameData.pig.tex.bitmaps [0] + bmi.index;
if ((bmP->bm_type == BM_TYPE_STD) && BM_OVERRIDE (bmP)) {
	bmP = BM_OVERRIDE (bmP);
	if (BM_FRAMES (bmP))
		bmP = BM_FRAMES (bmP) + iFrame;
	}

xSize = objP->size;
if (bmP->bm_props.w > bmP->bm_props.h)
	G3DrawBitMap (&objP->pos,  xSize, FixMulDiv (xSize, bmP->bm_props.h, bmP->bm_props.w), bmP, 
					  orientation, alpha, transp, bDepthInfo);
else
	G3DrawBitMap (&objP->pos, FixMulDiv (xSize, bmP->bm_props.w, bmP->bm_props.h), xSize, bmP, 
					  orientation, alpha, transp, bDepthInfo);
if (color) {
#if 1
	memcpy (color, bitmapColors + bmi.index, sizeof (*color));
#else
#	if 0
	ubyte *p = bmP->bm_texBuf;
	int c, h, i, j = 0, r = 0, g = 0, b = 0;
	for (h = i = bmP->bm_props.w * bmP->bm_props.h; i; i--, p++) {
		if (c = *p) {
			c *= 3;
			r += grPalette [c++];
			g += grPalette [c++];
			b += grPalette [c];
			j++;
			}
		}
	j *= 63;
	color->red = (double) r / (double) j;
	color->green = (double) g / (double) j;
	color->blue = (double) b / (double) j;
#	else
	unsigned char c = bmP->bm_avgColor;
	color->red = CPAL2Tr (bmP->bm_palette, c);
	color->green = CPAL2Tg (bmP->bm_palette, c);
	color->blue = CPAL2Tb (bmP->bm_palette, c);
#	endif
#endif
	}
}

//------------------------------------------------------------------------------
//draw an object that is a texture-mapped rod
void DrawObjectRodTexPoly (object *objP, bitmap_index bmi, int lighted)
{
	grs_bitmap *bmP = gameData.pig.tex.bitmaps [0] + bmi.index;
	fix light;
	vms_vector delta, top_v, bot_v;
	g3s_point top_p, bot_p;

PIGGY_PAGE_IN (bmi, 0);
bmP = BmOverride (bmP);
//bmP->bm_handle = bmi.index;
VmVecCopyScale (&delta, &objP->orient.uvec, objP->size);
VmVecAdd (&top_v, &objP->pos, &delta);
VmVecSub (&bot_v, &objP->pos, &delta);
G3TransformAndEncodePoint (&top_p, &top_v);
G3TransformAndEncodePoint (&bot_p, &bot_v);
if (lighted)
	light = ComputeObjectLight (objP, &top_p.p3_vec);
else
	light = f1_0;
#ifdef _3DFX
_3dfx_rendering_poly_obj = 1;
#endif
#ifdef PA_3DFX_VOODOO
light = f1_0;
#endif
G3DrawRodTexPoly (bmP, &bot_p, objP->size, &top_p, objP->size, light);
#ifdef _3DFX
_3dfx_rendering_poly_obj = 0;
#endif
}

//------------------------------------------------------------------------------

int	bLinearTMapPolyObjs = 1;

extern fix Max_thrust;

//used for robot engine glow
//function that takes the same parms as draw_tmap, but renders as flat poly
//we need this to do the cloaked effect

//what darkening level to use when cloaked
#define CLOAKED_FADE_LEVEL		28

#define	CLOAK_FADEIN_DURATION_PLAYER	F2_0
#define	CLOAK_FADEOUT_DURATION_PLAYER	F2_0

#define	CLOAK_FADEIN_DURATION_ROBOT	F1_0
#define	CLOAK_FADEOUT_DURATION_ROBOT	F1_0

//------------------------------------------------------------------------------
//do special cloaked render
void DrawCloakedObject (object *objP, fix light, fix *glow, fix xCloakStartTime, fix xCloakEndTime)
{
	fix	xCloakDeltaTime, xTotalCloakedTime;
	fix	xLightScale = F1_0;
	int	nCloakValue = 0;
	int	bFading = 0;		//if true, bFading, else cloaking
	fix	xCloakFadeinDuration = F1_0;
	fix	xCloakFadeoutDuration = F1_0;


	xTotalCloakedTime = xCloakEndTime - xCloakStartTime;
	switch (objP->type) {
		case OBJ_PLAYER:
			xCloakFadeinDuration = CLOAK_FADEIN_DURATION_PLAYER;
			xCloakFadeoutDuration = CLOAK_FADEOUT_DURATION_PLAYER;
			break;
		case OBJ_ROBOT:
			xCloakFadeinDuration = CLOAK_FADEIN_DURATION_ROBOT;
			xCloakFadeoutDuration = CLOAK_FADEOUT_DURATION_ROBOT;
			break;
		default:
			Int3 ();		//	Contact Mike: Unexpected object type in DrawCloakedObject.
	}

	xCloakDeltaTime = gameData.time.xGame - xCloakStartTime;
	if (xCloakDeltaTime < xCloakFadeinDuration / 2) {

		xLightScale = FixDiv (xCloakFadeinDuration / 2 - xCloakDeltaTime, xCloakFadeinDuration / 2);
		bFading = 1;
	}
	else if (xCloakDeltaTime < xCloakFadeinDuration) {
		nCloakValue = f2i (FixDiv (xCloakDeltaTime - xCloakFadeinDuration / 2, xCloakFadeinDuration / 2) * CLOAKED_FADE_LEVEL);
	} else if (gameData.time.xGame < xCloakEndTime-xCloakFadeoutDuration) {
		static int nCloakDelta = 0, nCloakDir = 1;
		static fix xCloakTimer = 0;

		//note, if more than one cloaked object is visible at once, the
		//pulse rate will change!
		xCloakTimer -= gameData.time.xFrame;
		while (xCloakTimer < 0) {
			xCloakTimer += xCloakFadeoutDuration/12;
			nCloakDelta += nCloakDir;
			if (nCloakDelta == 0 || nCloakDelta == 4)
				nCloakDir = -nCloakDir;
		}
		nCloakValue = CLOAKED_FADE_LEVEL - nCloakDelta;
	} else if (gameData.time.xGame < xCloakEndTime - xCloakFadeoutDuration / 2) {
		nCloakValue = f2i (FixDiv (xTotalCloakedTime - xCloakFadeoutDuration / 2 - xCloakDeltaTime, xCloakFadeoutDuration / 2) * CLOAKED_FADE_LEVEL);
	} else {
		xLightScale = FixDiv (xCloakFadeoutDuration / 2 - (xTotalCloakedTime - xCloakDeltaTime), xCloakFadeoutDuration / 2);
		bFading = 1;
	}

	if (bFading) {
		fix xNewLight, xSaveGlow;
		bitmap_index * alt_textures = NULL;

#ifdef NETWORK
		if (objP->rtype.pobj_info.alt_textures > 0)
			alt_textures = multi_player_textures [objP->rtype.pobj_info.alt_textures-1];
#endif
		xNewLight = FixMul (light, xLightScale);
		xSaveGlow = glow [0];
		glow [0] = FixMul (glow [0], xLightScale);
		DrawPolygonModel (objP, &objP->pos, 
				   &objP->orient, 
				   (vms_angvec *)&objP->rtype.pobj_info.anim_angles, 
				   objP->rtype.pobj_info.model_num, objP->rtype.pobj_info.subobj_flags, 
				   xNewLight, 
				   glow, 
				   alt_textures, 
					NULL);
		glow [0] = xSaveGlow;
	}
	else {
		gameStates.render.grAlpha = (float) nCloakValue;
		GrSetColorRGB (0, 0, 0, 255);	//set to black (matters for s3)
		G3SetSpecialRender (DrawTexPolyFlat, NULL, NULL);		//use special flat drawer
		DrawPolygonModel (objP, &objP->pos, 
				   &objP->orient, 
				   (vms_angvec *)&objP->rtype.pobj_info.anim_angles, 
				   objP->rtype.pobj_info.model_num, objP->rtype.pobj_info.subobj_flags, 
				   light, 
				   glow, 
				   NULL, 
					NULL);
		G3SetSpecialRender (NULL, NULL, NULL);
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
}

//------------------------------------------------------------------------------
//draw an object which renders as a polygon model
void DrawPolygonObject (object *objP)
{
	fix xLight;
	int imSave;
	fix xEngineGlow [2];		//element 0 is for engine glow, 1 for headlight
	int bBlendPolys = 0;
	int bBrightPolys = 0;
	//tRgbColorf color;


	//	If option set for bright players in netgame, brighten them!
#ifdef NETWORK
if ((gameData.app.nGameMode & GM_MULTI) && netGame.BrightPlayers && (objP->type == OBJ_PLAYER))
	xLight = F1_0;
else
#endif
xLight = ComputeObjectLight (objP, NULL);
//make robots brighter according to robot glow field
if (objP->type == OBJ_ROBOT) {
#ifdef _DEBUG
	xLight = ComputeObjectLight (objP, NULL);
#endif
	xLight += (gameData.bots.pInfo [objP->id].glow<<12);		//convert 4:4 to 16:16
	}
else if (objP->type == OBJ_WEAPON) {
	if (objP->id == FLARE_ID)
		xLight += F1_0 * 2;
	}
else if (objP->type == OBJ_MARKER)
 	xLight += F1_0 * 2;
imSave = gameStates.render.nInterpolationMethod;
if (bLinearTMapPolyObjs)
	gameStates.render.nInterpolationMethod = 1;
//set engine glow value
ComputeEngineGlow (objP, xEngineGlow);
if (objP->rtype.pobj_info.tmap_override != -1) {
#ifdef _DEBUG
	polymodel *pm = gameData.models.polyModels + objP->rtype.pobj_info.model_num;
#endif
	bitmap_index bmiP [12];
	int i;

	Assert (pm->n_textures <= 12);
	for (i = 0;i<12;i++)		//fill whole array, in case simple model needs more
		bmiP [i] = gameData.pig.tex.bmIndex [0][objP->rtype.pobj_info.tmap_override];

	DrawPolygonModel (objP, &objP->pos, 
				&objP->orient, 
				(vms_angvec *)&objP->rtype.pobj_info.anim_angles, 
				objP->rtype.pobj_info.model_num, 
				objP->rtype.pobj_info.subobj_flags, 
				xLight, 
				xEngineGlow, 
				bmiP, 
				NULL);
}
else {
	if ((objP->type == OBJ_PLAYER) && (gameData.multi.players [objP->id].flags & PLAYER_FLAGS_CLOAKED))
		DrawCloakedObject (objP, xLight, xEngineGlow, gameData.multi.players [objP->id].cloak_time, gameData.multi.players [objP->id].cloak_time+CLOAK_TIME_MAX);
	else if ((objP->type == OBJ_ROBOT) && (objP->ctype.ai_info.CLOAKED)) {
		if (gameData.bots.pInfo [objP->id].boss_flag)
			DrawCloakedObject (objP, xLight, xEngineGlow, gameData.boss.nCloakStartTime, gameData.boss.nCloakEndTime);
		else
			DrawCloakedObject (objP, xLight, xEngineGlow, gameData.time.xGame-F1_0*10, gameData.time.xGame+F1_0*10);
		}
	else {
		bitmap_index *bmiAltTex = NULL;
		#ifdef NETWORK
		if (objP->rtype.pobj_info.alt_textures > 0)
			bmiAltTex = multi_player_textures [objP->rtype.pobj_info.alt_textures-1];
		#endif

		//	Snipers get bright when they fire.
		if (gameData.ai.localInfo [OBJ_IDX (objP)].next_fire < F1_0/8) {
			if (objP->ctype.ai_info.behavior == AIB_SNIPE)
				xLight = 2*xLight + F1_0;
		}
		bBlendPolys = (objP->type == OBJ_WEAPON) && (gameData.weapons.info [objP->id].model_num_inner > -1);
		bBrightPolys = bBlendPolys && WI_energy_usage (objP->id);
		if (bBlendPolys) {
#if OGL_ZBUF
			fix xDistToEye = VmVecDistQuick (&gameData.objs.viewer->pos, &objP->pos);
			if (!gameOpts->legacy.bRender) {
				gameStates.render.grAlpha = 2.0f;
				OglBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				}
#endif
			if (xDistToEye < gameData.models.nSimpleModelThresholdScale * F1_0*2)
				DrawPolygonModel (
					objP, &objP->pos, &objP->orient, 
					(vms_angvec *)&objP->rtype.pobj_info.anim_angles, 
					gameData.weapons.info [objP->id].model_num_inner, 
					objP->rtype.pobj_info.subobj_flags, 
					bBrightPolys ? F1_0 : xLight, 
					xEngineGlow, 
					bmiAltTex, 
					NULL /*gameData.weapons.color + objP->id*/);
			}
		DrawPolygonModel (
			objP, &objP->pos, &objP->orient, 
			(vms_angvec *)&objP->rtype.pobj_info.anim_angles, 
			objP->rtype.pobj_info.model_num, 
			objP->rtype.pobj_info.subobj_flags, 
			bBrightPolys ? F1_0 : xLight, 
			xEngineGlow, 
			bmiAltTex, 
			(objP->type == OBJ_WEAPON) ? gameData.weapons.color + objP->id : NULL);
#if OGL_ZBUF
		if (bBlendPolys && !gameOpts->legacy.bRender) {
			gameStates.render.grAlpha = (float) GR_ACTUAL_FADE_LEVELS;
			OglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
#endif
		}
	}
gameStates.render.nInterpolationMethod = imSave;
}

//------------------------------------------------------------------------------
// These variables are used to keep a list of the 3 closest robots to the viewer.
// The code works like this: Every time render object is called with a polygon model, 
// it finds the distance of that robot to the viewer.  If this distance if within 10
// segments of the viewer, it does the following: If there aren't already 3 robots in
// the closet-robots list, it just sticks that object into the list along with its distance.
// If the list already contains 3 robots, then it finds the robot in that list that is
// farthest from the viewer. If that object is farther than the object currently being
// rendered, then the new object takes over that far object's slot.  *Then* after all
// gameData.objs.objects are rendered, object_render_targets is called an it draws a target on top
// of all the gameData.objs.objects.

//091494: #define MAX_CLOSE_ROBOTS 3
//--unused-- static int Object_draw_lock_boxes = 0;
//091494: static int Object_num_close = 0;
//091494: static object * Object_close_ones [MAX_CLOSE_ROBOTS];
//091494: static fix Object_close_distance [MAX_CLOSE_ROBOTS];

//091494: set_close_objects (object *objP)
//091494: {
//091494: 	fix dist;
//091494:
//091494: 	if ((objP->type != OBJ_ROBOT) || (Object_draw_lock_boxes == 0))	
//091494: 		return;
//091494:
//091494: 	// The following code keeps a list of the 10 closest robots to the
//091494: 	// viewer.  See comments in front of this function for how this works.
//091494: 	dist = VmVecDist (&objP->pos, &gameData.objs.viewer->pos);
//091494: 	if (dist < i2f (20*10))	{				
//091494: 		if (Object_num_close < MAX_CLOSE_ROBOTS)	{
//091494: 			Object_close_ones [Object_num_close] = obj;
//091494: 			Object_close_distance [Object_num_close] = dist;
//091494: 			Object_num_close++;
//091494: 		} else {
//091494: 			int i, farthest_robot;
//091494: 			fix farthest_distance;
//091494: 			// Find the farthest robot in the list
//091494: 			farthest_robot = 0;
//091494: 			farthest_distance = Object_close_distance [0];
//091494: 			for (i=1; i<Object_num_close; i++)	{
//091494: 				if (Object_close_distance [i] > farthest_distance)	{
//091494: 					farthest_distance = Object_close_distance [i];
//091494: 					farthest_robot = i;
//091494: 				}
//091494: 			}
//091494: 			// If this object is closer to the viewer than
//091494: 			// the farthest in the list, replace the farthest with this object.
//091494: 			if (farthest_distance > dist)	{
//091494: 				Object_close_ones [farthest_robot] = obj;
//091494: 				Object_close_distance [farthest_robot] = dist;
//091494: 			}
//091494: 		}
//091494: 	}
//091494: }


// -----------------------------------------------------------------------------
//this routine checks to see if an robot rendered near the middle of
//the screen, and if so and the player had fired, "warns" the robot
void SetRobotLocationInfo (object *objP)
{
if (gameStates.app.bPlayerFiredLaserThisFrame != -1) {
	g3s_point temp;

	G3TransformAndEncodePoint (&temp, &objP->pos);
	if (temp.p3_codes & CC_BEHIND)		//robot behind the screen
		return;
	//the code below to check for object near the center of the screen
	//completely ignores z, which may not be good
	if ((abs (temp.p3_x) < F1_0*4) && (abs (temp.p3_y) < F1_0*4)) {
		objP->ctype.ai_info.danger_laser_num = gameStates.app.bPlayerFiredLaserThisFrame;
		objP->ctype.ai_info.danger_laser_signature = gameData.objs.objects [gameStates.app.bPlayerFiredLaserThisFrame].signature;
		}
	}
}

//	------------------------------------------------------------------------------------------------------------------

void CreateSmallFireballOnObject (object *objP, fix size_scale, int bSound)
{
	fix			size;
	vms_vector	pos, rand_vec;
	short			segnum;

pos = objP->pos;
MakeRandomVector (&rand_vec);
VmVecScale (&rand_vec, objP->size / 2);
VmVecInc (&pos, &rand_vec);
size = FixMul (size_scale, F1_0 / 2 + d_rand ()*4 / 2);
segnum = FindSegByPoint (&pos, objP->segnum);
if (segnum != -1) {
	object *explObjP = ObjectCreateExplosion (segnum, &pos, size, VCLIP_SMALL_EXPLOSION);
	if (!explObjP)
		return;
	AttachObject (objP, explObjP);
	if (d_rand () < 8192) {
		fix	vol = F1_0 / 2;
		if (objP->type == OBJ_ROBOT)
			vol *= 2;
		else if (bSound)
			DigiLinkSoundToObject (SOUND_EXPLODING_WALL, OBJ_IDX (objP), 0, vol);
		}
	}
}

//	------------------------------------------------------------------------------------------------------------------
void CreateVClipOnObject (object *objP, fix size_scale, ubyte vclip_num)
{
	fix			size;
	vms_vector	pos, rand_vec;
	short			segnum;

pos = objP->pos;
MakeRandomVector (&rand_vec);
VmVecScale (&rand_vec, objP->size / 2);
VmVecInc (&pos, &rand_vec);
size = FixMul (size_scale, F1_0 + d_rand ()*4);
segnum = FindSegByPoint (&pos, objP->segnum);
if (segnum != -1) {
	object *explObjP = ObjectCreateExplosion (segnum, &pos, size, vclip_num);
	if (!explObjP)
		return;

	explObjP->movement_type = MT_PHYSICS;
	explObjP->mtype.phys_info.velocity.x = objP->mtype.phys_info.velocity.x / 2;
	explObjP->mtype.phys_info.velocity.y = objP->mtype.phys_info.velocity.y / 2;
	explObjP->mtype.phys_info.velocity.z = objP->mtype.phys_info.velocity.z / 2;
	}
}

// -- mk, 02/05/95 -- #define	VCLIP_INVULNERABILITY_EFFECT	VCLIP_SMALL_EXPLOSION
// -- mk, 02/05/95 --
// -- mk, 02/05/95 -- // -----------------------------------------------------------------------------
// -- mk, 02/05/95 -- void do_player_invulnerability_effect (object *objP)
// -- mk, 02/05/95 -- {
// -- mk, 02/05/95 -- 	if (d_rand () < gameData.time.xFrame*8) {
// -- mk, 02/05/95 -- 		CreateVClipOnObject (objP, F1_0, VCLIP_INVULNERABILITY_EFFECT);
// -- mk, 02/05/95 -- 	}
// -- mk, 02/05/95 -- }

// -----------------------------------------------------------------------------
//	Render an object.  Calls one of several routines based on type

// -----------------------------------------------------------------------------

void RenderPlayerShield (object *objP)
{
	int i = objP->id;

if (EGI_FLAG (bRenderShield, 0, 0) &&
	 !(gameData.multi.players [i].flags & PLAYER_FLAGS_CLOAKED)) {
	UseSpherePulse (gameData.multi.spherePulse + i);
	if (gameData.multi.players [i].flags & PLAYER_FLAGS_INVULNERABLE)
		DrawObjectSphere (objP, 1.0f, 0.8f, 0.6f, 0.6f);
	else if (gameData.multi.bWasHit [i]) {
		if (gameData.multi.bWasHit [i] < 0) {
			gameData.multi.bWasHit [i] = 1;
			gameData.multi.nLastHitTime [i] = gameStates.app.nSDLTicks;
			SetSpherePulse (gameData.multi.spherePulse + i, 0.1f, 0.5f);
			}
		else if (gameStates.app.nSDLTicks - gameData.multi.nLastHitTime [i] >= 300) {
			gameData.multi.bWasHit [i] = 0;
			SetSpherePulse (gameData.multi.spherePulse + i, 0.02f, 0.4f);
			}
		}
	if (gameData.multi.bWasHit [i])
		DrawObjectSphere (objP, 1.0f, 0.5f, 0.0f, 0.5f);
	else {
		if (gameData.multi.spherePulse [i].fSpeed == 0.0f)
			SetSpherePulse (gameData.multi.spherePulse + i, 0.02f, 0.5f);
		DrawObjectSphere (objP, 0.0f, 0.5f, 1.0f, (float) f2ir (gameData.multi.players [i].shields) / 300.0f);
		}
	}
}

// -----------------------------------------------------------------------------

static inline tRgbColorf *ObjectFrameColor (object *objP, tRgbColorf *pc)
{
	static tRgbColorf	defaultColor = {0,1.0f,0};
	static tRgbColorf	botDefColor = {1.0f,0,0};
	static tRgbColorf	playerDefColors [] = {{0,1.0f,0},{0,0,1.0f},{1.0f,0,0}};

if (pc)
	return pc;
if (objP)
	if (objP->type == OBJ_ROBOT) {
		if (!gameData.bots.pInfo [objP->id].companion)
			return &botDefColor;
		}
	else if (objP->type == OBJ_PLAYER) {
		if (IsTeamGame)
			return playerDefColors + GetTeam (objP->id) + 1;
		return playerDefColors;
		}
return &defaultColor;
}

// -----------------------------------------------------------------------------

static inline float ObjectDamage (object *objP)
{
	float	fDmg;

if (objP->type == OBJ_PLAYER)
	fDmg = f2fl (gameData.multi.players [objP->id].shields) / 100;
else if (objP->type != OBJ_ROBOT)
	fDmg = 1.0f;
else {
		fDmg = f2fl (objP->shields) / f2fl (gameData.bots.info [gameStates.app.bD1Mission][objP->id].strength);
	if (gameData.bots.pInfo [objP->id].companion)
		fDmg /= 2;
	}
return (fDmg > 1.0f) ? 1.0f : (fDmg < 0.0f) ? 0.0f : fDmg;
}

// -----------------------------------------------------------------------------

void RenderDamageIndicator (object *objP, tRgbColorf *pc)
{
	fVector3		fPos, fVerts [4];
	float			r, r2, w;

if (EGI_FLAG (bDamageIndicators, 0, 0) &&
	 (extraGameInfo [IsMultiGame].bTargetIndicators == 1)) {
	pc = ObjectFrameColor (objP, pc);
	VmsVecToFloat (&fPos, &objP->pos);
	G3TransformPointf (&fPos, &fPos);
	r = f2fl (objP->size);
	r2 = r / 10;
	r = r2 * 9;
	w = 2 * r;
	fPos.p.x -= r;
	fPos.p.y += r;
	fPos.p.z = -fPos.p.z;
	w *= ObjectDamage (objP);
	fVerts [0].p.x = fVerts [3].p.x = fPos.p.x;
	fVerts [1].p.x = fVerts [2].p.x = fPos.p.x + w;
	fVerts [0].p.y = fVerts [1].p.y = fPos.p.y;
	fVerts [2].p.y = fVerts [3].p.y = fPos.p.y - r2;
	fVerts [0].p.z = fVerts [1].p.z = fVerts [2].p.z = fVerts [3].p.z = fPos.p.z;
	glColor4f (pc->red, pc->green, pc->blue, 2.0f / 3.0f);
	glBegin (GL_QUADS);
#if 1
	glVertex3fv ((GLfloat *) fVerts);
	glVertex3fv ((GLfloat *) (fVerts + 1));
	glVertex3fv ((GLfloat *) (fVerts + 2));
	glVertex3fv ((GLfloat *) (fVerts + 3));
#else
	glVertex3f (fPos.p.x, fPos.p.y, fPos.p.z);
	glVertex3f (fPos.p.x + w, fPos.p.y, fPos.p.z);
	glVertex3f (fPos.p.x + w, fPos.p.y - r2, fPos.p.z);
	glVertex3f (fPos.p.x, fPos.p.y - r2, fPos.p.z);
#endif
	glEnd ();
	w = 2 * r;
	fVerts [1].p.x = fVerts [2].p.x = fPos.p.x + w;
	glColor3fv ((GLfloat *) pc);
	glBegin (GL_LINE_LOOP);
	glVertex3fv ((GLfloat *) fVerts);
	glVertex3fv ((GLfloat *) (fVerts + 1));
	glVertex3fv ((GLfloat *) (fVerts + 2));
	glVertex3fv ((GLfloat *) (fVerts + 3));
	glEnd ();
	glDisable (GL_TEXTURE_2D);
	}
}

// -----------------------------------------------------------------------------

void RenderTargetIndicator (object *objP, tRgbColorf *pc)
{
	fVector3		fPos, fVerts [4];
	float			r, r2, r3;
	int			nPlayer = (objP->type == OBJ_PLAYER) ? objP->id : -1;

#if 0
if (!CanSeeObject (OBJ_IDX (objP), 1))
	return;
#endif
if (gameStates.app.bNostalgia || !EGI_FLAG (bCloakedIndicators, 0, 0)) {
	if (nPlayer >= 0) {
		if (gameData.multi.players [nPlayer].flags & PLAYER_FLAGS_CLOAKED)
			return;
		}
	else if (objP->type == OBJ_ROBOT) {
		if (objP->ctype.ai_info.CLOAKED)
			return;
		}
	}
if (IsTeamGame && EGI_FLAG (bFriendlyIndicators, 0, 0)) {
	if (GetTeam (nPlayer) != GetTeam (gameData.multi.nLocalPlayer)) {
		if (!(gameData.multi.players [nPlayer].flags & PLAYER_FLAGS_FLAG))
			return;
		pc = ObjectFrameColor (NULL, NULL);
		}
	}
if (EGI_FLAG (bTargetIndicators, 0, 0)) {
	pc = ObjectFrameColor (objP, pc);
	VmsVecToFloat (&fPos, &objP->pos);
	G3TransformPointf (&fPos, &fPos);
	fPos.p.z = -fPos.p.z;
	r = f2fl (objP->size);
	glDisable (GL_TEXTURE_2D);
	glColor3fv ((GLfloat *) pc);
	if (extraGameInfo [IsMultiGame].bTargetIndicators == 1) {	//square brackets
		r2 = r * 2 / 3;
		fVerts [0].p.x = fVerts [3].p.x = fPos.p.x - r2;
		fVerts [1].p.x = fVerts [2].p.x = fPos.p.x - r;
		fVerts [0].p.y = fVerts [1].p.y = fPos.p.y - r;
		fVerts [2].p.y = fVerts [3].p.y = fPos.p.y + r;
		fVerts [0].p.z =
		fVerts [1].p.z =
		fVerts [2].p.z =
		fVerts [3].p.z = fPos.p.z;

		glBegin (GL_LINE_STRIP);
		glVertex3fv ((GLfloat *) fVerts);
		glVertex3fv ((GLfloat *) (fVerts + 1));
		glVertex3fv ((GLfloat *) (fVerts + 2));
		glVertex3fv ((GLfloat *) (fVerts + 3));
		glEnd ();
		fVerts [0].p.x = fVerts [3].p.x = fPos.p.x + r2;
		fVerts [1].p.x = fVerts [2].p.x = fPos.p.x + r;
		glBegin (GL_LINE_STRIP);
		glVertex3fv ((GLfloat *) fVerts);
		glVertex3fv ((GLfloat *) (fVerts + 1));
		glVertex3fv ((GLfloat *) (fVerts + 2));
		glVertex3fv ((GLfloat *) (fVerts + 3));
		glEnd ();
		}
	else {	//triangle
		r2 = r / 3;
		fVerts [0].p.x = fPos.p.x - r2;
		fVerts [1].p.x = fPos.p.x + r2;
		fVerts [2].p.x = fPos.p.x;
		fVerts [0].p.y = fVerts [1].p.y = fPos.p.y + r;
		fVerts [2].p.y = fPos.p.y + r - r2;
		fVerts [0].p.z =
		fVerts [1].p.z =
		fVerts [2].p.z = fPos.p.z;
		glBegin (GL_LINE_LOOP);
		glVertex3fv ((GLfloat *) fVerts);
		glVertex3fv ((GLfloat *) (fVerts + 1));
		glVertex3fv ((GLfloat *) (fVerts + 2));
		glEnd ();
		if (EGI_FLAG (bDamageIndicators, 0, 0)) {
			r3 = ObjectDamage (objP);
			if (r3 < 1.0f) {
				if (r3 < 0.0f)
					r3 = 0.0f;
				fVerts [0].p.x = fPos.p.x - r2 * r3;
				fVerts [1].p.x = fPos.p.x + r2 * r3;
				fVerts [2].p.x = fPos.p.x;
				fVerts [0].p.y = fVerts [1].p.y = fPos.p.y + r - r2 * (1.0f - r3);
				//fVerts [2].p.y = fPos.p.y + r - r2;
				}
			}
		glBegin (GL_TRIANGLES);
		glColor4f (pc->red, pc->green, pc->blue, 2.0f / 3.0f);
		glVertex3fv ((GLfloat *) fVerts);
		glVertex3fv ((GLfloat *) (fVerts + 1));
		glVertex3fv ((GLfloat *) (fVerts + 2));
		glEnd ();
		}
	}
RenderDamageIndicator (objP, pc);
}

// -----------------------------------------------------------------------------

void RenderTowedFlag (object *objP)
{
	static fVector3 fVerts [4] = {
		{0.0f, 2.0f / 3.0f, 0.0f},
		{0.0f, 2.0f / 3.0f, -1.0f},
		{0.0f, -(1.0f / 3.0f), -1.0f},
		{0.0f, -(1.0f / 3.0f), 0.0f}
	};

	typedef struct uv {
		float	u, v;
	} uv;

	static uv uvList [4] = {{0.0f, -0.3f}, {1.0f, -0.3f}, {1.0f, 0.7f}, {0.0f, 0.7f}};

if (!gameStates.app.bNostalgia && IsTeamGame && (gameData.multi.players [objP->id].flags & PLAYER_FLAGS_FLAG)) {
		vms_vector		vPos = objP->pos;
		fVector3			vPosf;
		tFlagData		*pf = gameData.pig.flags + !GetTeam (objP->id);
		tPathPoint		*pp = GetPathPoint (&pf->path);
		int				i;
		float				r;
		grs_bitmap		*bmP;

	if (pp) {
		OglActiveTexture (GL_TEXTURE0_ARB);
		glEnable (GL_TEXTURE_2D);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		PIGGY_PAGE_IN (pf->bmi, 0);
		bmP = gameData.pig.tex.pBitmaps + pf->vcP->frames [pf->vci.nCurFrame].index;
		if (OglBindBmTex (bmP, 0))
			return;
		bmP = BmCurFrame (bmP);
		OglTexWrap (bmP->glTexture, GL_REPEAT);
		VmVecScaleInc (&vPos, &objP->orient.fvec, -objP->size);
		r = f2fl (objP->size);
		G3StartInstanceMatrix (&vPos, &pp->mOrient);
		glBegin (GL_QUADS);
		glColor3f (1.0f, 1.0f, 1.0f);
		for (i = 0; i < 4; i++) {
			vPosf.p.x = 0;
			vPosf.p.y = fVerts [i].p.y * r;
			vPosf.p.z = fVerts [i].p.z * r;
			G3TransformPointf (&vPosf, &vPosf);
			vPosf.p.z = -vPosf.p.z;
			glTexCoord2fv ((GLfloat *) (uvList + i));
			glVertex3fv ((GLfloat *) &vPosf);
			}
		for (i = 3; i >= 0; i--) {
			vPosf.p.x = 0;
			vPosf.p.y = fVerts [i].p.y * r;
			vPosf.p.z = fVerts [i].p.z * r;
			G3TransformPointf (&vPosf, &vPosf);
			vPosf.p.z = -vPosf.p.z;
			glTexCoord2fv ((GLfloat *) (uvList + i));
			glVertex3fv ((GLfloat *) &vPosf);
			}
		glEnd ();
		G3DoneInstance ();
		OGL_BINDTEX (0);
		}
	}
}

// -----------------------------------------------------------------------------

#define	RING_SIZE		16
#define	THRUSTER_SEGS	14

static fVector3	vFlame [THRUSTER_SEGS][RING_SIZE];
static int			bHaveFlame = 0;

void CreateThrusterFlame (void)
{
	static fVector3	vRing [RING_SIZE] = {
		{-0.5f, -0.5f, 0.0f},
		{-0.6533f, -0.2706f, 0.0f},
		{-0.7071f, 0.0f, 0.0f},
		{-0.6533f, 0.2706f, 0.0f},
		{-0.5f, 0.5f, 0.0f},
		{-0.2706f, 0.6533f, 0.0f},
		{0.0f, 0.7071f, 0.0f},
		{0.2706f, 0.6533f, 0.0f},
		{0.5f, 0.5f, 0.0f},
		{0.6533f, 0.2706f, 0.0f},
		{0.7071f, 0.0f, 0.0f},
		{0.6533f, -0.2706f, 0.0f},
		{0.5f, -0.5f, 0.0f},
		{0.2706f, -0.6533f, 0.0f},
		{0.0f, -0.7071f, 0.0f},
		{-0.2706f, -0.6533f, 0.0f}
	};

if (!bHaveFlame) {
		fVector3		*pv;
		int			i, j, m, n;
		double		phi, sinPhi;
		float			z = 0, 
						fScale = 2.0f / 3.0f, 
						fStep [2] = {1.0f / 4.0f, 1.0f / 3.0f};

	pv = &vFlame [0][0];
	for (i = 0, phi = 0; i < 5; i++, phi += Pi / 8, z -= fStep [0]) {
		sinPhi = (1 + sin (phi) / 2) * fScale;
		for (j = 0; j < RING_SIZE; j++, pv++) {
			pv->p.x = vRing [j].p.x * (float) sinPhi;
			pv->p.y = vRing [j].p.y * (float) sinPhi;
			pv->p.z = z;
			}
		}
	m = n = THRUSTER_SEGS - i + 1;
	for (phi = Pi / 2; i < THRUSTER_SEGS; i++, phi += Pi / 8, z -= fStep [1], m--) {
		sinPhi = (1 + sin (phi) / 2) * fScale * m / n;
		for (j = 0; j < RING_SIZE; j++, pv++) {
			pv->p.x = vRing [j].p.x * (float) sinPhi;
			pv->p.y = vRing [j].p.y * (float) sinPhi;
			pv->p.z = z;
			}
		}
	bHaveFlame = 1;
	}
}

// -----------------------------------------------------------------------------

void RenderThrusterFlames (object *objP)
{
	static int		nStripIdx [] = {0,15,1,14,2,13,3,12,4,11,5,10,6,9,7,8};
	int				h, i, j, k, l, nThrusters;
	tRgbaColorf		c [2];
	vms_vector		vPos [2];
	fVector3			v;
	float				fSize, fLength, fSpeed, fPulse, fFade [4];
	tThrusterData	*pt = NULL;
	tPathPoint		*pp = NULL;
	
	static time_t	tPulse = 0;
	static int		nPulse = 10;

#if 1//ndef _DEBUG
if (gameStates.app.bNostalgia || !EGI_FLAG (bThrusterFlames, 0, 0))
	return;
#endif
fSpeed = f2fl (VmVecMag (&objP->mtype.phys_info.velocity));
fLength = fSpeed / 60.0f;
fLength += 0.2f;
if (!pt || (fSpeed >= pt->fSpeed)) {
	fFade [0] = 0.95f;
	fFade [1] = 0.85f;
	fFade [2] = 0.75f;
	fFade [3] = 0.65f;
	}
else {
	fFade [0] = 0.9f;
	fFade [1] = 0.8f;
	fFade [2] = 0.7f;
	fFade [3] = 0.6f;
	}
if (pt)
	pt->fSpeed = fSpeed;
if (objP->type == OBJ_PLAYER) {
		tThrusterData	*pt = gameData.render.thrusters + objP->id;
		tPathPoint		*pp = GetPathPoint (&pt->path);

	if (gameStates.app.nSDLTicks - pt->tPulse > 10) {
		pt->tPulse = gameStates.app.nSDLTicks;
		pt->nPulse = d_rand () % 11;
		}
	fPulse = (float) pt->nPulse / 10.0f;
	fSize = 0.5f + fLength * 0.5f;
	fLength = fSize = 1.0;
	nThrusters = 2;
	if (gameOpts->render.bHiresModels) {
		VmVecScaleAdd (vPos, &objP->pos, &objP->orient.fvec, -objP->size);
		VmVecScaleInc (vPos, &objP->orient.rvec, -(8 * objP->size / 44));
		VmVecScaleAdd (vPos + 1, vPos, &objP->orient.rvec, 8 * objP->size / 22);
		}
	else {
		VmVecScaleAdd (vPos, &objP->pos, &objP->orient.fvec, -objP->size / 10 * 9);
		VmVecScaleInc (vPos, &objP->orient.rvec, -(8 * objP->size / 50));
		VmVecScaleInc (vPos, &objP->orient.uvec, -(objP->size / 20));
		VmVecScaleAdd (vPos + 1, vPos, &objP->orient.rvec, 8 * objP->size / 25);
		}
	}
else {
	if (!gameData.weapons.info [objP->id].afterburner_size)
		return;
	if (gameStates.app.nSDLTicks - tPulse > 10) {
		tPulse = gameStates.app.nSDLTicks;
		nPulse = d_rand () % 11;
		}
	fPulse = (float) nPulse / 10.0f;
	if (objP->id == EARTHSHAKER_ID)
		fSize = 1.0f;
	else if ((objP->id == MEGA_ID) || (objP->id == EARTHSHAKER_MEGA_ID))
		fSize = 0.8f;
	else if (objP->id == SMART_ID)
		fSize = 0.6f;
	else
		fSize = 0.5f;
	nThrusters = 1;
	VmVecScaleAdd (vPos, &objP->pos, &objP->orient.fvec, -objP->size);
	}
CreateThrusterFlame ();
glLineWidth (3);

for (h = 0; h < nThrusters; h++) {
	c [1].red = 0.5f + 0.05f * fPulse;
	c [1].green = 0.45f + 0.045f * fPulse;
	c [1].blue = 0.0f;
	c [1].alpha = 0.9f;
	G3StartInstanceMatrix (vPos + h, pp ? &pp->mOrient : &objP->orient);
	glDisable (GL_TEXTURE_2D);
	glDepthMask (0);
	glCullFace (GL_BACK);
	for (i = 0; i < THRUSTER_SEGS - 1; i++) {
#if 1
		c [0] = c [1];
		c [1].red *= 0.975f;
		c [1].green *= 0.8f;
		c [1].alpha *= fFade [i / 4];
		glBegin (GL_QUAD_STRIP);
		for (j = 0; j < RING_SIZE + 1; j++) {
			for (l = 0; l < 2; l++) {
				k = j % RING_SIZE;
				v = vFlame [i + l][k];
				v.p.x *= fSize;
				v.p.y *= fSize;
				v.p.z *= fLength;
				G3TransformPointf (&v, &v);
				v.p.z = -v.p.z;
				glColor4f (c [l].red, c [l].green, c [l].blue, c [l].alpha);
				glVertex3fv ((GLfloat *) &v);
				}
			}
		glEnd ();
#else
		glBegin (GL_LINE_LOOP);
		glColor4f (c [1].red, c [1].green, c [1].blue, c [1].alpha);
		for (j = 0; j < RING_SIZE; j++) {
			G3TransformPointf (&v, vFlame [i] + j);
			v.p.z = -v.p.z;
			glVertex3fv ((GLfloat *) &v);
			}
		glEnd ();
#endif
		}
	glBegin (GL_TRIANGLE_STRIP);
	for (j = 0; j < RING_SIZE; j++) {
		G3TransformPointf (&v, vFlame [0] + nStripIdx [j]);
		v.p.z = -v.p.z;
		glVertex3fv ((GLfloat *) &v);
		}
	glEnd ();
	glLineWidth (1);
	glCullFace (GL_FRONT);
	glDepthMask (1);
	G3DoneInstance ();
	}
}

// -----------------------------------------------------------------------------

bool G3DrawSphere3D  (g3s_point *p0, int nSides, int rad);

void RenderObject (object *objP, int nWindowNum)
{
	int	mld_save, oofIdx;
	float fLight [3];
	fix	nGlow [2];

if ((objP == gameData.objs.viewer) && 
	 (gameStates.render.nShadowPass != 2) && 
#ifdef _DEBUG
	 (!gameStates.render.bExternalView || nWindowNum)) {
#else	 
	 ((IsMultiGame && !IsCoopGame && !EGI_FLAG (bEnableCheats, 0, 0)) || !gameStates.render.bExternalView || nWindowNum)) {
#endif	 	
	DoPlayerSmoke (objP, -1);
	return;		
	}
if ((objP->type==OBJ_NONE)/* || (objP->type==OBJ_CAMBOT)*/){
#if TRACE				
	con_printf (1, "ERROR!!!Bogus obj %d in seg %d is rendering!\n", OBJ_IDX (objP), objP->segnum);
#endif
	return;
	}
mld_save = gameStates.render.detail.nMaxLinearDepth;
gameStates.render.nState = 1;
gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
SetNearestStaticLights (objP->segnum, 1);
SetNearestDynamicLights (objP->segnum);
switch (objP->render_type) {
	case RT_NONE:	
		break;		//doesn't render, like the player

	case RT_POLYOBJ:
		if (!(extraGameInfo [0].bShadows && extraGameInfo [IsMultiGame].bShadows) || 
			 (gameStates.render.nShadowPass == 3))
			DoObjectSmoke (objP);
		if (objP->type == OBJ_PLAYER) {
			if (gameData.models.bHaveHiresModel [0]) {
				ComputeEngineGlow (objP, nGlow);
				fLight [0] = (float) ComputeObjectLight (objP, NULL) / 65536.0f;
				fLight [1] = (float) nGlow [0] / 65536.0f;				
				fLight [2] = (float) nGlow [1] / 65536.0f;				
				OOF_Render (objP, gameData.models.hiresModels + OOF_PYRO, fLight, 
								(gameData.multi.players [objP->id].flags & PLAYER_FLAGS_CLOAKED) != 0);
				}
			else
				DrawPolygonObject (objP);
			RenderThrusterFlames (objP);
			RenderPlayerShield (objP);
			RenderTargetIndicator (objP, NULL);
			RenderTowedFlag (objP);
			}
		else if (objP->type == OBJ_ROBOT) {
			DrawPolygonObject (objP);
			RenderTargetIndicator (objP, NULL);
			SetRobotLocationInfo (objP);
			}
		else if (gameStates.render.nShadowPass != 2) {
			if (gameData.models.nHiresModels && 
				 (oofIdx = idToOOF [objP->id]) && 
				 gameData.models.bHaveHiresModel [oofIdx]) {
				ComputeEngineGlow (objP, nGlow);
				fLight [0] = (float) ComputeObjectLight (objP, NULL) / 65536.0f;
				fLight [1] = (float) nGlow [0] / 65536.0f;				
				fLight [2] = (float) nGlow [1] / 65536.0f;				
				OOF_Render (objP, gameData.models.hiresModels + oofIdx, fLight, 0);
				}
			else
				DrawPolygonObject (objP);
			if (objP->type == OBJ_WEAPON)
				RenderThrusterFlames (objP);
			}
		break;

	case RT_MORPH:	
		if (gameStates.render.nShadowPass != 2)
			MorphDrawObject (objP); 
		break;

	case RT_THRUSTER: 
		if (nWindowNum && (objP->mtype.phys_info.flags & PF_WIGGLE))
			break;
			
	case RT_FIREBALL: 
		if (gameStates.render.nShadowPass != 2)
			DrawFireball (objP); 
		break;

	case RT_WEAPON_VCLIP: 
		if (gameStates.render.nShadowPass != 2)
			DrawWeaponVClip (objP); 
		break;

	case RT_HOSTAGE: 
		if (gameStates.render.nShadowPass != 2)
			DrawHostage (objP); 
		break;

	case RT_POWERUP: 
		if (gameStates.render.nShadowPass != 2)
			DrawPowerup (objP); 
		break;

	case RT_LASER: 
		if (gameStates.render.nShadowPass != 2)
			RenderLaser (objP); 
		break;

	default: 
		Error ("Unknown render_type <%d>", objP->render_type);
	}
SetNearestStaticLights (objP->segnum, 0);

#ifdef NEWDEMO
if (objP->render_type != RT_NONE)
	if (gameData.demo.nState == ND_STATE_RECORDING) {
		if (!gameData.demo.bWasRecorded [OBJ_IDX (objP)]) {
			NDRecordRenderObject (objP);
			gameData.demo.bWasRecorded [OBJ_IDX (objP)]=1;
		}
	}
#endif
gameStates.render.detail.nMaxLinearDepth = mld_save;
}

//------------------------------------------------------------------------------

void CheckAndFixMatrix (vms_matrix *m);

#define VmAngVecZero(v) (v)->p= (v)->b= (v)->h = 0

void ResetPlayerObject ()
{
	int i;

//Init physics
VmVecZero (&gameData.objs.console->mtype.phys_info.velocity);
VmVecZero (&gameData.objs.console->mtype.phys_info.thrust);
VmVecZero (&gameData.objs.console->mtype.phys_info.rotvel);
VmVecZero (&gameData.objs.console->mtype.phys_info.rotthrust);
gameData.objs.console->mtype.phys_info.brakes = gameData.objs.console->mtype.phys_info.turnroll = 0;
gameData.objs.console->mtype.phys_info.mass = gameData.pig.ship.player->mass;
gameData.objs.console->mtype.phys_info.drag = gameData.pig.ship.player->drag;
gameData.objs.console->mtype.phys_info.flags |= PF_TURNROLL | PF_LEVELLING | PF_WIGGLE | PF_USES_THRUST;
//Init render info
gameData.objs.console->render_type = RT_POLYOBJ;
gameData.objs.console->rtype.pobj_info.model_num = gameData.pig.ship.player->model_num;		//what model is this?
gameData.objs.console->rtype.pobj_info.subobj_flags = 0;		//zero the flags
gameData.objs.console->rtype.pobj_info.tmap_override = -1;		//no tmap override!
for (i = 0; i < MAX_SUBMODELS; i++)
	VmAngVecZero (gameData.objs.console->rtype.pobj_info.anim_angles + i);
// Clear misc
gameData.objs.console->flags = 0;
}

//------------------------------------------------------------------------------
//make object0 the player, setting all relevant fields
void InitPlayerObject ()
{
gameData.objs.console->type = OBJ_PLAYER;
gameData.objs.console->id = 0;					//no sub-types for player
gameData.objs.console->signature = 0;			//player has zero, others start at 1
gameData.objs.console->size = gameData.models.polyModels [gameData.pig.ship.player->model_num].rad;
gameData.objs.console->control_type = CT_SLEW;			//default is player slewing
gameData.objs.console->movement_type = MT_PHYSICS;		//change this sometime
gameData.objs.console->lifeleft = IMMORTAL_TIME;
gameData.objs.console->attached_obj = -1;
ResetPlayerObject ();
}

//------------------------------------------------------------------------------

void InitIdToOOF (void)
{
memset (idToOOF, 0, sizeof (idToOOF));
idToOOF [MEGA_ID] = OOF_MEGA;
}

//------------------------------------------------------------------------------
//sets up the d_free list & init player & whatever else
void InitObjects ()
{
	int i;
	segment *pSeg;

CollideInit ();
for (i = 0; i < MAX_OBJECTS; i++) {
	gameData.objs.freeList [i] = i;
	gameData.objs.objects [i].type = OBJ_NONE;
	gameData.objs.objects [i].segnum =
	gameData.objs.objects [i].ctype.expl_info.next_attach =
	gameData.objs.objects [i].ctype.expl_info.prev_attach =
	gameData.objs.objects [i].ctype.expl_info.attach_parent =
	gameData.objs.objects [i].attached_obj = -1;
	gameData.objs.objects [i].flags = 0;
	}
for (i = 0, pSeg = gameData.segs.segments;i<MAX_SEGMENTS;i++, pSeg++)
	pSeg->objects = -1;
gameData.objs.console = 
gameData.objs.viewer = 
gameData.objs.objects;
InitPlayerObject ();
LinkObject (OBJ_IDX (gameData.objs.console), 0);	//put in the world in segment 0
gameData.objs.nObjects = 1;						//just the player
gameData.objs.nLastObject = 0;
InitIdToOOF ();
}

//------------------------------------------------------------------------------
//after calling init_object (), the network code has grabbed specific
//object slots without allocating them.  Go though the gameData.objs.objects & build
//the d_free list, then set the apporpriate globals
void SpecialResetObjects (void)
{
	int i;

gameData.objs.nObjects = MAX_OBJECTS;
gameData.objs.nLastObject = 0;
Assert (gameData.objs.objects [0].type != OBJ_NONE);		//0 should be used
for (i=MAX_OBJECTS;i--;)
	if (gameData.objs.objects [i].type == OBJ_NONE)
		gameData.objs.freeList [--gameData.objs.nObjects] = i;
	else
		if (i > gameData.objs.nLastObject)
			gameData.objs.nLastObject = i;
}

//------------------------------------------------------------------------------
#ifdef _DEBUG
int IsObjectInSeg (int segnum, int objn)
{
	int objnum, count = 0;

for (objnum=gameData.segs.segments [segnum].objects;objnum!=-1;objnum=gameData.objs.objects [objnum].next)	{
	if (count > MAX_OBJECTS) 	{
		Int3 ();
		return count;
		}
	if (objnum==objn) count++;
	}
 return count;
}

//------------------------------------------------------------------------------

int SearchAllSegsForObject (int objnum)
{
	int i;
	int count = 0;

	for (i = 0; i<=gameData.segs.nLastSegment; i++) {
		count += IsObjectInSeg (i, objnum);
	}
	return count;
}

//------------------------------------------------------------------------------

void JohnsObjUnlink (int segnum, int objnum)
{
	object  *objP = gameData.objs.objects+objnum;
	segment *seg = gameData.segs.segments+segnum;

Assert (objnum != -1);
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
	int segnum, objnum, count;

for (segnum = 0; segnum <= gameData.segs.nLastSegment; segnum++) {
	count = 0;
	for (objnum = gameData.segs.segments [segnum].objects;
		  objnum != -1;
		  objnum=gameData.objs.objects [objnum].next) {
		count++;
		#ifdef _DEBUG
		if (count > MAX_OBJECTS)	{
#if TRACE				
			con_printf (1, "Object list in segment %d is circular.\n", segnum);
#endif
			Int3 ();
		}
		#endif
		if (gameData.objs.objects [objnum].segnum != segnum)	{
			#ifdef _DEBUG
#if TRACE				
			con_printf (CON_DEBUG, "Removing object %d from segment %d.\n", objnum, segnum);
#endif
			Int3 ();
			#endif
			JohnsObjUnlink (segnum, objnum);
			}
		}
	}
}

//------------------------------------------------------------------------------

void RemoveAllObjectsBut (int segnum, int objnum)
{
	int i;

for (i = 0; i <= gameData.segs.nLastSegment; i++)
	if (segnum != i)
		if (IsObjectInSeg (i, objnum))
			JohnsObjUnlink (i, objnum);
}

//------------------------------------------------------------------------------

int check_duplicate_objects ()
{
	int i, count = 0;
	
for (i = 0; i <= gameData.objs.nLastObject; i++) {
	if (gameData.objs.objects [i].type != OBJ_NONE)	{
		count = SearchAllSegsForObject (i);
		if (count > 1)	{
#ifdef _DEBUG
#	if TRACE				
			con_printf (1, "Object %d is in %d segments!\n", i, count);
#	endif
			Int3 ();
#endif
			RemoveAllObjectsBut (gameData.objs.objects [i].segnum,  i);
			return count;
			}
		}
	}
	return count;
}

//------------------------------------------------------------------------------

void list_seg_objects (int segnum)
{
	int objnum, count = 0;

for (objnum = gameData.segs.segments [segnum].objects;
	  objnum != -1;
	  objnum = gameData.objs.objects [objnum].next) {
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

//link the object into the list for its segment
void LinkObject (int objnum, int segnum)
{
	object *objP;
	
Assert (objnum != -1);
objP = gameData.objs.objects + objnum;
Assert (objP->segnum == -1);
Assert (segnum >= 0 && segnum <= gameData.segs.nLastSegment);
objP->segnum = segnum;
objP->next = gameData.segs.segments [segnum].objects;
objP->prev = -1;
gameData.segs.segments [segnum].objects = objnum;
if (objP->next != -1)
		gameData.objs.objects [objP->next].prev = objnum;

//list_seg_objects (segnum);
//check_duplicate_objects ();

Assert (gameData.objs.objects [0].next != 0);
if (gameData.objs.objects [0].next == 0)
	gameData.objs.objects [0].next = -1;

Assert (gameData.objs.objects [0].prev != 0);
if (gameData.objs.objects [0].prev == 0)
	gameData.objs.objects [0].prev = -1;
}

//------------------------------------------------------------------------------

void UnlinkObject (int objnum)
{
	object  *objP = gameData.objs.objects+objnum;
	segment *segP= gameData.segs.segments+objP->segnum;

Assert (objnum != -1);
if (objP->prev == -1)
	segP->objects = objP->next;
else
	gameData.objs.objects [objP->prev].next = objP->next;
if (objP->next != -1) 
	gameData.objs.objects [objP->next].prev = objP->prev;
objP->segnum = -1;
Assert (gameData.objs.objects [0].next != 0);
Assert (gameData.objs.objects [0].prev != 0);
}

//------------------------------------------------------------------------------

int nDebrisObjectCount = 0;
int nUnusedObjectsSlots;

//returns the number of a d_free object, updating gameData.objs.nLastObject.
//Generally, CreateObject () should be called to get an object, since it
//fills in important fields and does the linking.
//returns -1 if no d_free gameData.objs.objects
int AllocObject (void)
{
	int objnum;
	object *objP;

if (gameData.objs.nObjects >= MAX_OBJECTS - 2)
	FreeObjectSlots (MAX_OBJECTS - 10);
if (gameData.objs.nObjects >= MAX_OBJECTS)
	return -1;
objnum = gameData.objs.freeList [gameData.objs.nObjects++];
if (objnum > gameData.objs.nLastObject) {
	gameData.objs.nLastObject = objnum;
	if (gameData.objs.nLastObject > gameData.objs.nObjectLimit)
		gameData.objs.nObjectLimit = gameData.objs.nLastObject;
	}
objP = gameData.objs.objects + objnum;
objP->flags = 0;
objP->attached_obj =
objP->ctype.expl_info.next_attach =
objP->ctype.expl_info.prev_attach =
objP->ctype.expl_info.attach_parent = -1;
return objnum;
}

//------------------------------------------------------------------------------
//frees up an object.  Generally, ReleaseObject () should be called to get
//rid of an object.  This function deallocates the object entry after
//the object has been unlinked
void FreeObject (int objnum)
{
	int id = gameData.objs.objects [objnum].id;

DelObjChildrenN (objnum);
DelObjChildN (objnum);
KillObjectSmoke (objnum);
RemoveOglLight (-1, -1, objnum);
if (gameData.objs.objects [objnum].type == OBJ_ROBOT)
	id = gameData.objs.objects [objnum].id;
gameData.objs.freeList [--gameData.objs.nObjects] = objnum;
Assert (gameData.objs.nObjects >= 0);
if (objnum == gameData.objs.nLastObject)
	while (gameData.objs.objects [--gameData.objs.nLastObject].type == OBJ_NONE);
}

//-----------------------------------------------------------------------------
//	Scan the object list, freeing down to num_used gameData.objs.objects
//	Returns number of slots freed.
int FreeObjectSlots (int num_used)
{
	int		i, olind;
	int		objList [MAX_OBJECTS];
	int		nAlreadyFree, nToFree, nOrgNumToFree;
	object	*objP;

olind = 0;
nAlreadyFree = MAX_OBJECTS - gameData.objs.nLastObject - 1;

if (MAX_OBJECTS - nAlreadyFree < num_used)
	return 0;

for (i = 0; i<=gameData.objs.nLastObject; i++) {
	if (gameData.objs.objects [i].flags & OF_SHOULD_BE_DEAD) {
		nAlreadyFree++;
		if (MAX_OBJECTS - nAlreadyFree < num_used)
			return nAlreadyFree;
		}
	else
		switch (gameData.objs.objects [i].type) {
			case OBJ_NONE:
				nAlreadyFree++;
				if (MAX_OBJECTS - nAlreadyFree < num_used)
					return 0;
				break;
			case OBJ_WALL:
			case OBJ_FLARE:
				Int3 ();		//	This is curious.  What is an object that is a wall?
				break;
			case OBJ_FIREBALL:
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
	con_printf (1, "Warning: Asked to d_free %i gameData.objs.objects, but can only d_free %i.\n", nToFree, olind);
#endif
	nToFree = olind;
	}
for (i = 0; i<nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if (objP->type == OBJ_DEBRIS) {
		nToFree--;
		objP->flags |= OF_SHOULD_BE_DEAD;
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i<nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->type == OBJ_FIREBALL)  && (objP->ctype.expl_info.delete_objnum==-1)) {
		nToFree--;
		objP->flags |= OF_SHOULD_BE_DEAD;
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i<nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->type == OBJ_WEAPON) && (objP->id == FLARE_ID)) {
		nToFree--;
		objP->flags |= OF_SHOULD_BE_DEAD;
		}
	}
if (!nToFree)
	return nOrgNumToFree;
for (i = 0; i<nToFree; i++) {
	objP = gameData.objs.objects + objList [i];
	if ((objP->type == OBJ_WEAPON) && (objP->id != FLARE_ID)) {
		nToFree--;
		objP->flags |= OF_SHOULD_BE_DEAD;
		}
	}
return nOrgNumToFree - nToFree;
}

//-----------------------------------------------------------------------------
//initialize a new object.  adds to the list for the given segment
//note that segnum is really just a suggestion, since this routine actually
//searches for the correct segment
//returns the object number
int CreateObject (ubyte type, ubyte id, short owner, short segnum, vms_vector *pos, 
					   vms_matrix *orient, fix size, ubyte ctype, ubyte mtype, ubyte rtype,
						int bIgnoreLimits)
{
	short objnum;
	object *objP;

if ((type == OBJ_POWERUP) && !bIgnoreLimits) {
	if (TooManyPowerups (id)) {
#ifdef _DEBUG
		HUDInitMessage ("%c%cDiscarding excess powerup!", 1, 
							 (char) GrFindClosestColor (gamePalette, 63, 31, 0));
#endif
		return -2;
		}
	}
Assert (segnum <= gameData.segs.nLastSegment);
Assert (segnum >= 0);
if ((segnum < 0) || (segnum > gameData.segs.nLastSegment))
	return -1;
Assert (ctype <= CT_CNTRLCEN);
if (type == OBJ_DEBRIS && nDebrisObjectCount>=gameStates.render.detail.nMaxDebrisObjects)
	return -1;
if (GetSegMasks (pos, segnum, 0).centerMask)
	if ((segnum=FindSegByPoint (pos, segnum))==-1) {
#ifdef _DEBUG
#	if TRACE				
		con_printf (CON_DEBUG, "Bad segnum in CreateObject (type=%d)\n", type);
#	endif
#endif
		return -1;		//don't create this object
	}
// Find next d_free object
objnum = AllocObject ();
if (objnum == -1)		//no d_free gameData.objs.objects
	return -1;
Assert (gameData.objs.objects [objnum].type == OBJ_NONE);		//make sure unused
objP = gameData.objs.objects + objnum;
Assert (objP->segnum == -1);
// Zero out object structure to keep weird bugs from happening
// in uninitialized fields.
memset (objP, 0, sizeof (object));
objP->signature = gameData.objs.nNextSignature++;
objP->type = type;
objP->id = id;
objP->last_pos = *pos;
objP->pos = *pos;
objP->size = size;
objP->flags = 0;
objP->matcen_creator = (sbyte) owner;
//@@if (orient != NULL)
//@@	objP->orient = *orient;
objP->orient = orient?*orient:vmdIdentityMatrix;
objP->control_type = ctype;
objP->movement_type = mtype;
objP->render_type = rtype;
objP->contains_type = -1;
if ((gameData.app.nGameMode & GM_ENTROPY) && (type == OBJ_POWERUP) && (id == POW_HOARD_ORB))
	objP->lifeleft = (extraGameInfo [1].entropy.nVirusLifespan <= 0) ? 
									IMMORTAL_TIME : i2f (extraGameInfo [1].entropy.nVirusLifespan);
else
	objP->lifeleft = IMMORTAL_TIME;		//assume immortal
objP->attached_obj = -1;

if (objP->control_type == CT_POWERUP)
	objP->ctype.powerup_info.count = 1;

// Init physics info for this object
if (objP->movement_type == MT_PHYSICS) {
	VmVecZero (&objP->mtype.phys_info.velocity);
	VmVecZero (&objP->mtype.phys_info.thrust);
	VmVecZero (&objP->mtype.phys_info.rotvel);
	VmVecZero (&objP->mtype.phys_info.rotthrust);
	objP->mtype.phys_info.mass = 0;
	objP->mtype.phys_info.drag = 0;
	objP->mtype.phys_info.brakes = 0;
	objP->mtype.phys_info.turnroll = 0;
	objP->mtype.phys_info.flags = 0;
	}
if (objP->render_type == RT_POLYOBJ)
	objP->rtype.pobj_info.tmap_override = -1;
objP->shields = 20*F1_0;
segnum = FindSegByPoint (pos, segnum);		//find correct segment
Assert (segnum!=-1);
objP->segnum = -1;					//set to zero by memset, above
LinkObject (objnum, segnum);
//	Set (or not) persistent bit in phys_info.
if (objP->type == OBJ_WEAPON) {
	Assert (objP->control_type == CT_WEAPON);
	objP->mtype.phys_info.flags |= WI_persistent (objP->id)*PF_PERSISTENT;
	objP->ctype.laser_info.creation_time = gameData.time.xGame;
	objP->ctype.laser_info.last_hitobj = -1;
	objP->ctype.laser_info.multiplier = F1_0;
	}
if (objP->control_type == CT_POWERUP)
	objP->ctype.powerup_info.creation_time = gameData.time.xGame;
else if (objP->control_type == CT_EXPLOSION)
	objP->ctype.expl_info.next_attach = 
	objP->ctype.expl_info.prev_attach = 
	objP->ctype.expl_info.attach_parent = -1;
#ifdef _DEBUG
#if TRACE				
if (bPrintObjectInfo)	
	con_printf (CON_DEBUG, "Created object %d of type %d\n", objnum, objP->type);
#endif
#endif
if (objP->type == OBJ_DEBRIS)
	nDebrisObjectCount++;
if (IsMultiGame && (type == OBJ_POWERUP) && PowerupClass (id)) {
	gameData.multi.powerupsInMine [(int) id]++;
	if (MultiPowerupIs4Pack (id))
		gameData.multi.powerupsInMine [(int) id - 1] += 4;
	}
return objnum;
}

//------------------------------------------------------------------------------

#ifdef EDITOR
//create a copy of an object. returns new object number
int CreateObjectCopy (int objnum, vms_vector *new_pos, int newsegnum)
{
	object *objP;
	int newObjNum = AllocObject ();

// Find next d_free object
if (newObjNum == -1)
	return -1;
obj = gameData.objs.objects + newObjNum;
*objP = gameData.objs.objects [objnum];
objP->pos = objP->last_pos = *new_pos;
objP->next = objP->prev = objP->segnum = -1;
LinkObject (newObjNum, newsegnum);
objP->signature = gameData.objs.nNextSignature++;
//we probably should initialize sub-structures here
return newObjNum;
}
#endif

//------------------------------------------------------------------------------

extern void NDRecordGuidedEnd ();

//remove object from the world
void ReleaseObject (short objnum)
{
	int pnum;
	object *objP = gameData.objs.objects + objnum;

Assert (objnum != -1);
Assert (objnum != 0);
Assert (objP->type != OBJ_NONE);
Assert (objP != gameData.objs.console);
if (objP->type == OBJ_WEAPON) {
	RespawnDestroyedWeapon (objnum);
	if (objP->id == GUIDEDMISS_ID) {
		pnum=gameData.objs.objects [objP->ctype.laser_info.parent_num].id;
		if (pnum!=gameData.multi.nLocalPlayer)
			gameData.objs.guidedMissile [pnum]=NULL;
		else if (gameData.demo.nState==ND_STATE_RECORDING)
			NDRecordGuidedEnd ();
		}
	}
if (objP == gameData.objs.viewer)		//deleting the viewer?
	gameData.objs.viewer = gameData.objs.console;						//..make the player the viewer
if (objP->flags & OF_ATTACHED)		//detach this from object
	DetachOneObject (objP);
if (objP->attached_obj != -1)		//detach all gameData.objs.objects from this
	DetachAllObjects (objP);
if (objP->type == OBJ_DEBRIS)
	nDebrisObjectCount--;
UnlinkObject (objnum);
Assert (gameData.objs.objects [0].next != 0);
if (objP->type == OBJ_ROBOT)
	ExecObjTriggers (objnum);
objP->type = OBJ_NONE;		//unused!
objP->signature = -1;
objP->segnum=-1;				// zero it!
FreeObject (objnum);
SpawnLeftoverPowerups (objnum);
}

//------------------------------------------------------------------------------

#define	DEATH_SEQUENCE_LENGTH			 (F1_0*5)
#define	DEATH_SEQUENCE_EXPLODE_TIME	 (F1_0*2)

object	*viewerSaveP;
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
ReleaseObject (OBJ_IDX (gameData.objs.deadPlayerCamera));
gameData.objs.deadPlayerCamera = NULL;
SelectCockpit (gameStates.render.cockpit.nModeSave);
gameStates.render.cockpit.nModeSave = -1;
gameData.objs.viewer = viewerSaveP;
gameData.objs.console->type = OBJ_PLAYER;
gameData.objs.console->flags = nPlayerFlagsSave;
Assert ((nControlTypeSave == CT_FLYING) || (nControlTypeSave == CT_SLEW));
gameData.objs.console->control_type = nControlTypeSave;
gameData.objs.console->render_type = nRenderTypeSave;
gameData.multi.players [gameData.multi.nLocalPlayer].flags &= ~PLAYER_FLAGS_INVULNERABLE;
gameStates.app.bPlayerEggsDropped = 0;
}

//------------------------------------------------------------------------------

//	Camera is less than size of player away from
void SetCameraPos (vms_vector *camera_pos, object *objP)
{
	int	count = 0;
	fix	xCameraPlayerDist;
	fix	xFarScale;

xCameraPlayerDist = VmVecDistQuick (camera_pos, &objP->pos);
if (xCameraPlayerDist < xCameraToPlayerDistGoal) { // 2*objP->size) {
	//	Camera is too close to player object, so move it away.
	vms_vector	player_camera_vec;
	fvi_query	fq;
	fvi_info		hit_data;
	vms_vector	local_p1;

	VmVecSub (&player_camera_vec, camera_pos, &objP->pos);
	if ((player_camera_vec.x == 0) && (player_camera_vec.y == 0) && (player_camera_vec.z == 0))
		player_camera_vec.x += F1_0/16;

	hit_data.hit.nType = HIT_WALL;
	xFarScale = F1_0;

	while ((hit_data.hit.nType != HIT_NONE) && (count++ < 6)) {
		vms_vector	closer_p1;
		VmVecNormalizeQuick (&player_camera_vec);
		VmVecScale (&player_camera_vec, xCameraToPlayerDistGoal);

		fq.p0 = &objP->pos;
		VmVecAdd (&closer_p1, &objP->pos, &player_camera_vec);		//	This is the actual point we want to put the camera at.
		VmVecScale (&player_camera_vec, xFarScale);						//	...but find a point 50% further away...
		VmVecAdd (&local_p1, &objP->pos, &player_camera_vec);		//	...so we won't have to do as many cuts.

		fq.p1					= &local_p1;
		fq.startSeg			= objP->segnum;
		fq.rad				= 0;
		fq.thisObjNum		= OBJ_IDX (objP);
		fq.ignoreObjList	= NULL;
		fq.flags				= 0;
		FindVectorIntersection (&fq, &hit_data);

		if (hit_data.hit.nType == HIT_NONE)
			*camera_pos = closer_p1;
		else {
			MakeRandomVector (&player_camera_vec);
			xFarScale = 3*F1_0 / 2;
			}
		}
	}
}

//------------------------------------------------------------------------------

extern void DropPlayerEggs (object *objP);
//extern int GetExplosionVClip (object *objP, int stage);
extern void MultiCapObjects ();
extern int Proximity_dropped, Smartmines_dropped;

void DeadPlayerFrame (void)
{
	fix			xTimeDead;
	vms_vector	fvec;

if (gameStates.app.bPlayerIsDead) {
	xTimeDead = gameData.time.xGame - gameStates.app.nPlayerTimeOfDeath;

	//	If unable to create camera at time of death, create now.
	if (gameData.objs.deadPlayerCamera == viewerSaveP) {
		object *player = gameData.objs.objects + gameData.multi.players [gameData.multi.nLocalPlayer].objnum;
		int objnum = CreateObject (OBJ_CAMERA, 0, -1, player->segnum, &player->pos, &player->orient, 0, 
											CT_NONE, MT_NONE, RT_NONE, 1);
		if (objnum != -1)
			gameData.objs.viewer = gameData.objs.deadPlayerCamera = gameData.objs.objects + objnum;
		else
			Int3 ();
		}		
	gameData.objs.console->mtype.phys_info.rotvel.x = max (0, DEATH_SEQUENCE_EXPLODE_TIME - xTimeDead)/4;
	gameData.objs.console->mtype.phys_info.rotvel.y = max (0, DEATH_SEQUENCE_EXPLODE_TIME - xTimeDead) / 2;
	gameData.objs.console->mtype.phys_info.rotvel.z = max (0, DEATH_SEQUENCE_EXPLODE_TIME - xTimeDead)/3;
	xCameraToPlayerDistGoal = min (xTimeDead*8, F1_0*20) + gameData.objs.console->size;
	SetCameraPos (&gameData.objs.deadPlayerCamera->pos, gameData.objs.console);
	VmVecSub (&fvec, &gameData.objs.console->pos, &gameData.objs.deadPlayerCamera->pos);
	VmVector2Matrix (&gameData.objs.deadPlayerCamera->orient, &fvec, NULL, NULL);
	if (xTimeDead > DEATH_SEQUENCE_EXPLODE_TIME) {
		if (!gameStates.app.bPlayerExploded) {
		if (gameData.multi.players [gameData.multi.nLocalPlayer].hostages_on_board > 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_2, gameData.multi.players [gameData.multi.nLocalPlayer].hostages_on_board);
		else if (gameData.multi.players [gameData.multi.nLocalPlayer].hostages_on_board == 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_1);
		else
			HUDInitMessage (TXT_SHIP_DESTROYED_0);

#ifdef TACTILE
			if (TactileStick)
				ClearForces ();
#endif
			gameStates.app.bPlayerExploded = 1;
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
#endif
			DropPlayerEggs (gameData.objs.console);
			gameStates.app.bPlayerEggsDropped = 1;
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
#endif
			ExplodeBadassPlayer (gameData.objs.console);
			//is this next line needed, given the badass call above?
			ExplodeObject (gameData.objs.console, 0);
			gameData.objs.console->flags &= ~OF_SHOULD_BE_DEAD;		//don't really kill player
			gameData.objs.console->render_type = RT_NONE;				//..just make him disappear
			gameData.objs.console->type = OBJ_GHOST;						//..and kill intersections
			gameData.multi.players [gameData.multi.nLocalPlayer].flags &= ~PLAYER_FLAGS_HEADLIGHT_ON;
#if 0
			if (gameOpts->gameplay.bFastRespawn)
				gameStates.app.bDeathSequenceAborted = 1;
#endif					
			}
		}
	else {
		if (d_rand () < gameData.time.xFrame*4) {
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendCreateExplosion (gameData.multi.nLocalPlayer);
#endif
			CreateSmallFireballOnObject (gameData.objs.console, F1_0, 1);
			}
		}
	if (gameStates.app.bDeathSequenceAborted) { //xTimeDead > DEATH_SEQUENCE_LENGTH) {
		StopPlayerMovement ();
		gameStates.app.bEnterGame = 2;
		if (!gameStates.app.bPlayerEggsDropped) {
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
#endif
			DropPlayerEggs (gameData.objs.console);
			gameStates.app.bPlayerEggsDropped = 1;
#ifdef NETWORK
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
#endif
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
	gameData.multi.players [gameData.multi.nLocalPlayer].secondary_ammo [PROXIMITY_INDEX]+=Proximity_dropped;
if (!(gameData.app.nGameMode & GM_ENTROPY))
	gameData.multi.players [gameData.multi.nLocalPlayer].secondary_ammo [SMART_MINE_INDEX]+=Smartmines_dropped;
Proximity_dropped = 0;
Smartmines_dropped = 0;
}



int nKilledInFrame = -1;
short nKilledObjNum = -1;
extern char bMultiSuicide;

//	------------------------------------------------------------------------

void StartPlayerDeathSequence (object *player)
{
	int	objnum;
	
gameStates.gameplay.bSpeedBoost = 0;
Assert (player == gameData.objs.console);
if ((gameStates.app.bPlayerIsDead != 0) || (gameData.objs.deadPlayerCamera != NULL))
	return;
StopConquerWarning ();
//Assert (gameStates.app.bPlayerIsDead == 0);
//Assert (gameData.objs.deadPlayerCamera == NULL);
ResetRearView ();
if (!(gameData.app.nGameMode & GM_MULTI))
	HUDClearMessages ();
nKilledInFrame = gameData.app.nFrameCount;
nKilledObjNum = OBJ_IDX (player);
gameStates.app.bDeathSequenceAborted = 0;
#ifdef NETWORK
if (gameData.app.nGameMode & GM_MULTI) {
	MultiSendKill (gameData.multi.players [gameData.multi.nLocalPlayer].objnum);
//		If Hoard, increase number of orbs by 1
//    Only if you haven't killed yourself
//		This prevents cheating
	if (gameData.app.nGameMode & GM_HOARD)
		if (!bMultiSuicide)
			if (gameData.multi.players [gameData.multi.nLocalPlayer].secondary_ammo [PROXIMITY_INDEX]<12)
				gameData.multi.players [gameData.multi.nLocalPlayer].secondary_ammo [PROXIMITY_INDEX]++;
	}
#endif
gameStates.ogl.palAdd.red = 40;
gameStates.app.bPlayerIsDead = 1;
#ifdef TACTILE
   if (TactileStick)
	Buffeting (70);
#endif
//gameData.multi.players [gameData.multi.nLocalPlayer].flags &= ~ (PLAYER_FLAGS_AFTERBURNER);
VmVecZero (&player->mtype.phys_info.rotthrust);
VmVecZero (&player->mtype.phys_info.thrust);
gameStates.app.nPlayerTimeOfDeath = gameData.time.xGame;
objnum = CreateObject (OBJ_CAMERA, 0, -1, player->segnum, &player->pos, &player->orient, 0, CT_NONE, MT_NONE, RT_NONE, 1);
viewerSaveP = gameData.objs.viewer;
if (objnum != -1)
	gameData.objs.viewer = gameData.objs.deadPlayerCamera = gameData.objs.objects + objnum;
else {
	Int3 ();
	gameData.objs.deadPlayerCamera = gameData.objs.viewer;
	}
if (gameStates.render.cockpit.nModeSave == -1)		//if not already saved
	gameStates.render.cockpit.nModeSave = gameStates.render.cockpit.nMode;
SelectCockpit (CM_LETTERBOX);
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordLetterbox ();
nPlayerFlagsSave = player->flags;
nControlTypeSave = player->control_type;
nRenderTypeSave = player->render_type;
player->flags &= ~OF_SHOULD_BE_DEAD;
//	gameData.multi.players [gameData.multi.nLocalPlayer].flags |= PLAYER_FLAGS_INVULNERABLE;
player->control_type = CT_NONE;
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
	object	*objP;
	int		nLocalDeadPlayerObj = -1;

for (i = 0; i <= gameData.objs.nLastObject; i++) {
	objP = gameData.objs.objects + i;
	if ((objP->type != OBJ_NONE) && (objP->flags & OF_SHOULD_BE_DEAD)) {
		Assert ((objP->type != OBJ_FIREBALL) || (objP->ctype.expl_info.delete_time == -1));
		if (objP->type != OBJ_PLAYER) 
			ReleaseObject ((short) i);
		else {
			if (objP->id == gameData.multi.nLocalPlayer) {
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
}

//--------------------------------------------------------------------
//when an object has moved into a new segment, this function unlinks it
//from its old segment, and links it into the new segment
void RelinkObject (int objnum, int newsegnum)
{
Assert ((objnum >= 0) && (objnum <= gameData.objs.nLastObject));
Assert ((newsegnum <= gameData.segs.nLastSegment) && (newsegnum >= 0));
UnlinkObject (objnum);
LinkObject (objnum, newsegnum);
#if 0//def _DEBUG
#if TRACE				
if (GetSegMasks (&gameData.objs.objects [objnum].pos, 
					  gameData.objs.objects [objnum].segnum, 0).centerMask)
	con_printf (1, "RelinkObject violates seg masks.\n");
#endif
#endif
}

//--------------------------------------------------------------------
//process a continuously-spinning object
void SpinObject (object *objP)
{
	vms_angvec rotangs;
	vms_matrix rotmat, new_pm;

Assert (objP->movement_type == MT_SPINNING);
rotangs.p = FixMul (objP->mtype.spin_rate.x, gameData.time.xFrame);
rotangs.h = FixMul (objP->mtype.spin_rate.y, gameData.time.xFrame);
rotangs.b = FixMul (objP->mtype.spin_rate.z, gameData.time.xFrame);
VmAngles2Matrix (&rotmat, &rotangs);
VmMatMul (&new_pm, &objP->orient, &rotmat);
objP->orient = new_pm;
CheckAndFixMatrix (&objP->orient);
}

extern void MultiSendDropBlobs (char);
extern void FuelCenCheckForGoal (segment *);

//see if wall is volatile, and if so, cause damage to player
//returns true if player is in lava
int CheckVolatileWall (object *objP, int segnum, int sidenum, vms_vector *hitpt);
int CheckVolatileSegment (object *objP, int segnum);

//	Time at which this object last created afterburner blobs.

//--------------------------------------------------------------------
//reset object's movement info
//--------------------------------------------------------------------

void StopObjectMovement (object *objP)
{
Controls.heading_time = 0;
Controls.pitch_time = 0;
Controls.bank_time = 0;
Controls.vertical_thrust_time = 0;
Controls.sideways_thrust_time = 0;
Controls.forward_thrust_time = 0;
VmVecZero (&objP->mtype.phys_info.rotthrust);
VmVecZero (&objP->mtype.phys_info.thrust);
VmVecZero (&objP->mtype.phys_info.velocity);
VmVecZero (&objP->mtype.phys_info.rotvel);
}

//--------------------------------------------------------------------

void StopPlayerMovement (void)
{
if (!gameStates.gameplay.bSpeedBoost) {
	StopObjectMovement (gameData.objs.objects + gameData.multi.players [gameData.multi.nLocalPlayer].objnum);
	memset (&player_thrust, 0, sizeof (player_thrust));
//	gameData.time.xFrame = F1_0;
	gameStates.gameplay.bSpeedBoost = 0;
	}
}

//--------------------------------------------------------------------

void MoveCamera (object *objP)
{

#define	DEG90		 (F1_0 / 4)		
#define	DEG45		 (F1_0 / 8)				
#define	DEG675	 (DEG45 + (F1_0 / 16))
#define	DEG1		 (F1_0 / (4 * 90))	
	
	tCamera	*pc = cameras + gameData.objs.cameraRef [OBJ_IDX (objP)];
	fixang	curAngle = pc->curAngle;
	fixang	curDelta = pc->curDelta;

#if 1
	time_t	t0 = pc->t0;
	time_t	t = gameStates.app.nSDLTicks;

if ((t0 < 0) || (t - t0 >= 1000 / 90)) 
#endif
	if (objP->ctype.ai_info.behavior == AIB_NORMAL) {
		vms_angvec	a;
		vms_matrix	r;

		int	h = abs (curDelta);
		int	d = DEG1 / (gameOpts->render.cameras.nSpeed / 1000);
		int	s = h ? curDelta / h : 1;

	if (h != d)
		curDelta = s * d;
#if 1
	objP->mtype.phys_info.brakes = (fix) t;
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
	VmMatMul (&objP->orient, &pc->orient, &r);
	pc->curAngle = curAngle;
	pc->curDelta = curDelta;
	}
}

//--------------------------------------------------------------------

void CheckObjectInVolatileWall (object *objP)
{
	int bChkVolaSeg = 1, type, sideMask, bUnderLavaFall = 0;
	static int nLavaFallHissPlaying [MAX_PLAYERS]={0};

if (objP->type != OBJ_PLAYER)
	return;
sideMask = GetSegMasks (&objP->pos, objP->segnum, objP->size).sideMask;
if (sideMask) {
	short nSide, nWall;
	int bit;
	side *pSide = gameData.segs.segments [objP->segnum].sides;
	for (nSide = 0, bit = 1; nSide < 6; bit <<= 1, nSide++, pSide++) {
		if (!(sideMask & bit))
			continue;
		nWall = pSide->wall_num;
		if (!IS_WALL (nWall))
			continue;
		if (gameData.walls.walls [nWall].type != WALL_ILLUSION)
			continue;
		if (type = CheckVolatileWall (objP, objP->segnum, nSide, &objP->pos)) {
			short sound = (type==1) ? SOUND_LAVAFALL_HISS : SOUND_SHIP_IN_WATERFALL;
			bUnderLavaFall = 1;
			bChkVolaSeg = 0;
			if (!nLavaFallHissPlaying [objP->id]) {
				DigiLinkSoundToObject3 (sound, OBJ_IDX (objP), 1, F1_0, i2f (256), -1, -1);
				nLavaFallHissPlaying [objP->id] = 1;
				}
			}
		}
	}
if (bChkVolaSeg) {
	if (type=CheckVolatileSegment (objP, objP->segnum)) {
		short sound = (type==1) ? SOUND_LAVAFALL_HISS : SOUND_SHIP_IN_WATERFALL;
		bUnderLavaFall = 1;
		if (!nLavaFallHissPlaying [objP->id]) {
			DigiLinkSoundToObject3 (sound, OBJ_IDX (objP), 1, F1_0, i2f (256), -1, -1);
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

void HandleSpecialSegments (object *objP)
{
	fix fuel, shields;
	segment *segP = gameData.segs.segments + objP->segnum;
	xsegment *xsegP = gameData.segs.xSegments + objP->segnum;
	player *playerP = gameData.multi.players + gameData.multi.nLocalPlayer;

if ((objP->type == OBJ_PLAYER) && (gameData.multi.nLocalPlayer == objP->id)) {
#ifdef NETWORK
   if (gameData.app.nGameMode & GM_CAPTURE)
		 FuelCenCheckForGoal (segP);
   else if (gameData.app.nGameMode & GM_HOARD)
		 FuelCenCheckForHoardGoal (segP);
   else if (gameData.app.nGameMode & GM_ENTROPY) {
		if (Controls.forward_thrust_time || 
			 Controls.vertical_thrust_time || 
			 Controls.sideways_thrust_time ||
			 (xsegP->owner < 0) ||
			 (xsegP->owner == GetTeam (gameData.multi.nLocalPlayer) + 1)) {
			StopConquerWarning ();
			gameStates.entropy.nTimeLastMoved = -1;
			}
		else if (gameStates.entropy.nTimeLastMoved < 0)
			gameStates.entropy.nTimeLastMoved = 0;
		}
#endif
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
		fuel=FuelCenGiveFuel (segP, INITIAL_ENERGY - playerP->energy);
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

int HandleObjectControl (object *objP)
{
switch (objP->control_type) {
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
		if (gameStates.gameplay.bNoBotAI || (gameStates.app.bGameSuspended & SUSP_ROBOTS))
			return 1;
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
		Error ("Unknown control type %d in object %li, sig/type/id = %i/%i/%i", objP->control_type, OBJ_IDX (objP), objP->signature, objP->type, objP->id);
#else
		Error ("Unknown control type %d in object %i, sig/type/id = %i/%i/%i", objP->control_type, OBJ_IDX (objP), objP->signature, objP->type, objP->id);
#endif
	}
return 0;
}

//--------------------------------------------------------------------

void HandleObjectMovement (object *objP)
{
switch (objP->movement_type) {
	case MT_NONE:			
		break;								//this doesn't move

	case MT_PHYSICS:	
		DoPhysicsSim (objP);	
		SetOglLightPos (OBJ_IDX (objP));
		break;	//move by physics

	case MT_SPINNING:		
		SpinObject (objP); 
		break;
	}
}

//--------------------------------------------------------------------

int CheckPlayerHitTriggers (object *objP, short nPrevSegment)
{
		short	nConnSide, i;
		int	nOldLevel;

if ((objP->type != OBJ_PLAYER) || (objP->movement_type != MT_PHYSICS) || (nPrevSegment == objP->segnum))
	return 0;
#ifdef NETWORK
nOldLevel = gameData.missions.nCurrentLevel;
#endif
for (i = 0; i < nPhysSegs - 1; i++) {
	nConnSide = FindConnectedSide (gameData.segs.segments + physSegList [i+1], gameData.segs.segments + physSegList [i]);
	if (nConnSide != -1)
		CheckTrigger (gameData.segs.segments + physSegList [i], nConnSide, OBJ_IDX (objP), 0);
#ifdef _DEBUG
	else	// segments are not directly connected, so do binary subdivision until you find connected segments.
		LogErr ("UNCONNECTED SEGMENTS %d, %d\n", physSegList [i+1], physSegList [i]);
#endif
	//maybe we've gone on to the next level.  if so, bail!
#ifdef NETWORK
	if (gameData.missions.nCurrentLevel != nOldLevel)
		return 1;
#endif
	}
return 0;
}

//--------------------------------------------------------------------

void CheckGuidedMissileThroughExit (object *objP, short nPrevSegment)
{
if ((objP == gameData.objs.guidedMissile [gameData.multi.nLocalPlayer]) && 
	 (objP->signature == gameData.objs.guidedMissileSig [gameData.multi.nLocalPlayer])) {
	if (nPrevSegment != objP->segnum) {
		short	nConnSide = FindConnectedSide (gameData.segs.segments + objP->segnum, gameData.segs.segments+nPrevSegment);
		if (nConnSide != -1) {
			short nWall, nTrigger;
			nWall = WallNumI (nPrevSegment, nConnSide);
			if (IS_WALL (nWall)) {
				nTrigger = gameData.walls.walls [nWall].trigger;
				if ((nTrigger < gameData.trigs.nTriggers) &&
					 (gameData.trigs.triggers [nTrigger].type == TT_EXIT))
					gameData.objs.guidedMissile [gameData.multi.nLocalPlayer]->lifeleft = 0;
				}
			}
		}
	}
}

//--------------------------------------------------------------------

void CheckAfterburnerBlobDrop (object *objP)
{
if (gameStates.render.bDropAfterburnerBlob) {
	Assert (objP == gameData.objs.console);
	DropAfterburnerBlobs (objP, 2, i2f (5) / 2, -1, NULL, 0);	//	-1 means use default lifetime
#ifdef NETWORK
	if (gameData.app.nGameMode & GM_MULTI)
		MultiSendDropBlobs ((char) gameData.multi.nLocalPlayer);
#endif
	gameStates.render.bDropAfterburnerBlob = 0;
	}

if ((objP->type == OBJ_WEAPON) && (gameData.weapons.info [objP->id].afterburner_size)) {
	int	objnum = OBJ_IDX (objP);
	fix	vel = VmVecMagQuick (&objP->mtype.phys_info.velocity);
	fix	delay, lifetime;

	if (vel > F1_0*200)
		delay = F1_0/16;
	else if (vel > F1_0*40)
		delay = FixDiv (F1_0*13, vel);
	else
		delay = DEG90;

	lifetime = (delay * 3) / 2;
	if (!(gameData.app.nGameMode & GM_MULTI)) {
		delay /= 2;
		lifetime *= 2;
		}

	if ((objP->type == OBJ_WEAPON) &&
		 ((SHOW_SMOKE && gameOpts->render.smoke.bMissiles)
		  || gameStates.app.bNostalgia
		 // || EGI_FLAG (bThrusterFlames, 0, 0)
		 ))
		return;
	if ((gameData.objs.xLastAfterburnerTime [objnum] + delay < gameData.time.xGame) || 
		 (gameData.objs.xLastAfterburnerTime [objnum] > gameData.time.xGame)) {
		DropAfterburnerBlobs (objP, 1, i2f (gameData.weapons.info [objP->id].afterburner_size)/16, lifetime, NULL, 0);
		gameData.objs.xLastAfterburnerTime [objnum] = gameData.time.xGame;
		}
	}
}

//--------------------------------------------------------------------
//move an object for the current frame

int MoveOneObject (object * objP)
{
	short	nPrevSegment = (short) objP->segnum;

#if 1//def _DEBUG
if ((objP->type == OBJ_PLAYER) && (gameData.multi.players [objP->id].shields < 1)) {
	if ((gameData.multi.players [objP->id].shields < 0) && !(objP->flags & OF_SHOULD_BE_DEAD))
		HUDInitMessage ("Player should be dead but isn't!");
	}
#endif
objP->last_pos = objP->pos;			// Save the current position
HandleSpecialSegments (objP);
if (objP->lifeleft != IMMORTAL_TIME) {	//if not immortal...
	//	Ok, this is a big hack by MK.
	//	If you want an object to last for exactly one frame, then give it a lifeleft of ONE_FRAME_TIME.
	if (objP->lifeleft != ONE_FRAME_TIME)
		if (gameData.time.xFrame != F1_0)
			objP->lifeleft -= gameData.time.xFrame;		//...inevitable countdown towards death
}

gameStates.render.bDropAfterburnerBlob = 0;
if (HandleObjectControl (objP))
	return 1;

if (objP->lifeleft < 0) {		// We died of old age
	objP->flags |= OF_SHOULD_BE_DEAD;
	if (objP->type==OBJ_WEAPON && WI_damage_radius (objP->id))
		ExplodeBadassWeapon (objP, &objP->pos);
	else if (objP->type == OBJ_ROBOT)	//make robots explode
		ExplodeObject (objP, 0);
	}

if ((objP->type == OBJ_NONE) || (objP->flags & OF_SHOULD_BE_DEAD)) {
	return 1;			//object has been deleted
	}
HandleObjectMovement (objP);
if (CheckPlayerHitTriggers (objP, nPrevSegment))
	return 0;
CheckObjectInVolatileWall (objP);
CheckGuidedMissileThroughExit (objP, nPrevSegment);
CheckAfterburnerBlobDrop (objP);
return 1;
}

int	nMaxUsedObjects = MAX_OBJECTS - 20;

//--------------------------------------------------------------------
//move all gameData.objs.objects for the current frame
int MoveAllObjects ()
{
	int i;
	object *objP;

//	check_duplicate_objects ();
//	RemoveIncorrectObjects ();

	if (gameData.objs.nLastObject > nMaxUsedObjects)
		FreeObjectSlots (nMaxUsedObjects);		//	Free all possible object slots.

	DeleteAllObjsThatShouldBeDead ();
	if (gameOpts->gameplay.bAutoLeveling)
		gameData.objs.console->mtype.phys_info.flags |= PF_LEVELLING;
	else
		gameData.objs.console->mtype.phys_info.flags &= ~PF_LEVELLING;

	// Move all gameData.objs.objects
	objP = gameData.objs.objects;

#ifndef DEMO_ONLY
	gameStates.entropy.bConquering = 0;
	UpdatePlayerOrient ();
	for (i = 0;i<=gameData.objs.nLastObject;i++) {
		if ((objP->type != OBJ_NONE) && (!(objP->flags&OF_SHOULD_BE_DEAD)))	{
			if (!MoveOneObject (objP))
				return 0;
		}
		objP++;
	}
#else
		i = 0;	//kill warning
#endif
return 1;
//	check_duplicate_objects ();
//	RemoveIncorrectObjects ();

}


//--unused-- // -----------------------------------------------------------
//--unused-- //	Moved here from eobject.c on 02/09/94 by MK.
//--unused-- int find_last_obj (int i)
//--unused-- {
//--unused-- 	for (i=MAX_OBJECTS;--i >= 0;)
//--unused-- 		if (gameData.objs.objects [i].type != OBJ_NONE) break;
//--unused--
//--unused-- 	return i;
//--unused--
//--unused-- }


//make object array non-sparse
void compress_objects (void)
{
	int start_i;	//, last_i;

	//last_i = find_last_obj (MAX_OBJECTS);

	//	Note: It's proper to do < (rather than <=) gameData.objs.nLastObject here because we
	//	are just removing gaps, and the last object can't be a gap.
	for (start_i = 0;start_i<gameData.objs.nLastObject;start_i++)

		if (gameData.objs.objects [start_i].type == OBJ_NONE) {

			int	segnum_copy;

			segnum_copy = gameData.objs.objects [gameData.objs.nLastObject].segnum;

			UnlinkObject (gameData.objs.nLastObject);

			gameData.objs.objects [start_i] = gameData.objs.objects [gameData.objs.nLastObject];

			#ifdef EDITOR
			if (Cur_object_index == gameData.objs.nLastObject)
				Cur_object_index = start_i;
			#endif

			gameData.objs.objects [gameData.objs.nLastObject].type = OBJ_NONE;

			LinkObject (start_i, segnum_copy);

			while (gameData.objs.objects [--gameData.objs.nLastObject].type == OBJ_NONE);

			//last_i = find_last_obj (last_i);
			
		}

	ResetObjects (gameData.objs.nObjects);

}

//------------------------------------------------------------------------------
//called after load.  Takes number of gameData.objs.objects,  and gameData.objs.objects should be
//compressed.  resets d_free list, marks unused gameData.objs.objects as unused
void ResetObjects (int n_objs)
{
	int i;

gameData.objs.nObjects = n_objs;
Assert (gameData.objs.nObjects > 0);
for (i = gameData.objs.nObjects; i < MAX_OBJECTS; i++) {
	gameData.objs.freeList [i] = i;
	gameData.objs.objects [i].type = OBJ_NONE;
	gameData.objs.objects [i].segnum =
	gameData.objs.objects [i].ctype.expl_info.next_attach =
	gameData.objs.objects [i].ctype.expl_info.prev_attach =
	gameData.objs.objects [i].ctype.expl_info.attach_parent =
	gameData.objs.objects [i].attached_obj = -1;
	gameData.objs.objects [i].flags = 0;
	//KillObjectSmoke (i);
	}
gameData.objs.nLastObject = gameData.objs.nObjects - 1;
nDebrisObjectCount = 0;
}

//------------------------------------------------------------------------------
//Tries to find a segment for an object, using FindSegByPoint ()
int FindObjectSeg (object * objP)
{
	return FindSegByPoint (&objP->pos, objP->segnum);
}


//------------------------------------------------------------------------------
//If an object is in a segment, set its segnum field and make sure it's
//properly linked.  If not in any segment, returns 0, else 1.
//callers should generally use FindVectorIntersection ()
int UpdateObjectSeg (object * objP)
{
	int newseg;

newseg = FindObjectSeg (objP);
if (newseg == -1)
	return 0;
if (newseg != objP->segnum)
	RelinkObject (OBJ_IDX (objP), newseg);
return 1;
}

//------------------------------------------------------------------------------
//go through all gameData.objs.objects and make sure they have the correct segment numbers
void FixObjectSegs ()
{
	int i;

	for (i = 0;i<=gameData.objs.nLastObject;i++)
		if (gameData.objs.objects [i].type != OBJ_NONE)
			if (UpdateObjectSeg (gameData.objs.objects + i) == 0) {
#if TRACE				
				con_printf (1, "Cannot find segment for object %d in FixObjectSegs ()\n");
#endif
				Int3 ();
				COMPUTE_SEGMENT_CENTER_I (&gameData.objs.objects [i].pos, gameData.objs.objects [i].segnum);
			}
}


//--unused-- void object_use_new_object_list (object * new_list)
//--unused-- {
//--unused-- 	int i, segnum;
//--unused-- 	object *objP;
//--unused--
//--unused-- 	// First, unlink all the old gameData.objs.objects for the segments array
//--unused-- 	for (segnum = 0; segnum <= gameData.segs.nLastSegment; segnum++) {
//--unused-- 		gameData.segs.segments [segnum].objects = -1;
//--unused-- 	}
//--unused-- 	// Then, erase all the gameData.objs.objects
//--unused-- 	ResetObjects (1);
//--unused--
//--unused-- 	// Fill in the object array
//--unused-- 	memcpy (gameData.objs.objects, new_list, sizeof (object)*MAX_OBJECTS);
//--unused--
//--unused-- 	gameData.objs.nLastObject=-1;
//--unused--
//--unused-- 	// Relink 'em
//--unused-- 	for (i = 0; i<MAX_OBJECTS; i++)	{
//--unused-- 		obj = &gameData.objs.objects [i];
//--unused-- 		if (objP->type != OBJ_NONE)	{
//--unused-- 			gameData.objs.nObjects++;
//--unused-- 			gameData.objs.nLastObject = i;
//--unused-- 			segnum = objP->segnum;
//--unused-- 			objP->next = objP->prev = objP->segnum = -1;
//--unused-- 			LinkObject (i, segnum);
//--unused-- 		} else {
//--unused-- 			objP->next = objP->prev = objP->segnum = -1;
//--unused-- 		}
//--unused-- 	}
//--unused-- 	
//--unused-- }

//delete gameData.objs.objects, such as weapons & explosions, that shouldn't stay between levels
//	Changed by MK on 10/15/94, don't remove proximity bombs.
//if clear_all is set, clear even proximity bombs
void clear_transient_objects (int clear_all)
{
	short objnum;
	object *objP;

	for (objnum = 0, objP = gameData.objs.objects; objnum <= gameData.objs.nLastObject; objnum++, objP++)
		if (((objP->type == OBJ_WEAPON) && !(gameData.weapons.info [objP->id].flags&WIF_PLACABLE) && (clear_all || ((objP->id != PROXIMITY_ID) && (objP->id != SUPERPROX_ID)))) ||
			   objP->type == OBJ_FIREBALL ||
			   objP->type == OBJ_DEBRIS ||
			   objP->type == OBJ_DEBRIS ||
			   ((objP->type != OBJ_NONE) && (objP->flags & OF_EXPLODING))) {

			#ifdef _DEBUG
#if TRACE				
			if (gameData.objs.objects [objnum].lifeleft > i2f (2))
				con_printf (CON_DEBUG, "Note: Clearing object %d (type=%d, id=%d) with lifeleft=%x\n", objnum, gameData.objs.objects [objnum].type, gameData.objs.objects [objnum].id, gameData.objs.objects [objnum].lifeleft);
#endif
			#endif
			ReleaseObject (objnum);
		}
		#ifdef _DEBUG
#if TRACE				
		 else if (gameData.objs.objects [objnum].type!=OBJ_NONE && gameData.objs.objects [objnum].lifeleft < i2f (2))
			con_printf (CON_DEBUG, "Note: NOT clearing object %d (type=%d, id=%d) with lifeleft=%x\n", objnum, gameData.objs.objects [objnum].type, gameData.objs.objects [objnum].id, gameData.objs.objects [objnum].lifeleft);
#endif
		#endif
}

//------------------------------------------------------------------------------
//attaches an object, such as a fireball, to another object, such as a robot
void AttachObject (object *parent, object *sub)
{
	Assert (sub->type == OBJ_FIREBALL);
	Assert (sub->control_type == CT_EXPLOSION);

	Assert (sub->ctype.expl_info.next_attach==-1);
	Assert (sub->ctype.expl_info.prev_attach==-1);

	Assert (parent->attached_obj == -1 || 
			 gameData.objs.objects [parent->attached_obj].ctype.expl_info.prev_attach==-1);

	sub->ctype.expl_info.next_attach = parent->attached_obj;

	if (sub->ctype.expl_info.next_attach != -1)
		gameData.objs.objects [sub->ctype.expl_info.next_attach].ctype.expl_info.prev_attach = OBJ_IDX (sub);

	parent->attached_obj = OBJ_IDX (sub);

	sub->ctype.expl_info.attach_parent = OBJ_IDX (parent);
	sub->flags |= OF_ATTACHED;

	Assert (sub->ctype.expl_info.next_attach != OBJ_IDX (sub));
	Assert (sub->ctype.expl_info.prev_attach != OBJ_IDX (sub));
}

//------------------------------------------------------------------------------
//dettaches one object
void DetachOneObject (object *sub)
{
	Assert (sub->flags & OF_ATTACHED);
	Assert (sub->ctype.expl_info.attach_parent != -1);

if ((gameData.objs.objects [sub->ctype.expl_info.attach_parent].type != OBJ_NONE) &&
	 (gameData.objs.objects [sub->ctype.expl_info.attach_parent].attached_obj != -1)) {
	if (sub->ctype.expl_info.next_attach != -1) {
		Assert (gameData.objs.objects [sub->ctype.expl_info.next_attach].ctype.expl_info.prev_attach=OBJ_IDX (sub));
		gameData.objs.objects [sub->ctype.expl_info.next_attach].ctype.expl_info.prev_attach = sub->ctype.expl_info.prev_attach;
		}
	if (sub->ctype.expl_info.prev_attach != -1) {
		Assert (gameData.objs.objects [sub->ctype.expl_info.prev_attach].ctype.expl_info.next_attach=OBJ_IDX (sub));
		gameData.objs.objects [sub->ctype.expl_info.prev_attach].ctype.expl_info.next_attach = 
			sub->ctype.expl_info.next_attach;
		}
	else {
		Assert (gameData.objs.objects [sub->ctype.expl_info.attach_parent].attached_obj=OBJ_IDX (sub));
		gameData.objs.objects [sub->ctype.expl_info.attach_parent].attached_obj = sub->ctype.expl_info.next_attach;
		}
	}
sub->ctype.expl_info.next_attach = 
sub->ctype.expl_info.prev_attach =
sub->ctype.expl_info.attach_parent = -1;
sub->flags &= ~OF_ATTACHED;
}

//------------------------------------------------------------------------------
//dettaches all gameData.objs.objects from this object
void DetachAllObjects (object *parent)
{
while (parent->attached_obj != -1)
	DetachOneObject (gameData.objs.objects + parent->attached_obj);
}

//------------------------------------------------------------------------------
//creates a marker object in the world.  returns the object number
int DropMarkerObject (vms_vector *pos, short segnum, vms_matrix *orient, ubyte marker_num)
{
	short objnum;

	Assert (Marker_model_num != -1);
	objnum = CreateObject (OBJ_MARKER, marker_num, -1, segnum, pos, orient, 
								  gameData.models.polyModels [Marker_model_num].rad, CT_NONE, MT_NONE, RT_POLYOBJ, 1);
	if (objnum >= 0) {
		object *objP = &gameData.objs.objects [objnum];
		objP->rtype.pobj_info.model_num = Marker_model_num;
		VmVecCopyScale (&objP->mtype.spin_rate, &objP->orient.uvec, F1_0 / 2);
		//	MK, 10/16/95: Using lifeleft to make it flash, thus able to trim lightlevel from all gameData.objs.objects.
		objP->lifeleft = IMMORTAL_TIME - 1;
	}

	return objnum;	
}

//------------------------------------------------------------------------------

extern int nAiLastMissileCamera;

//	*viewer is a viewer, probably a missile.
//	wake up all robots that were rendered last frame subject to some constraints.
void WakeupRenderedObjects (object *viewer, int window_num)
{
	int	i;

	//	Make sure that we are processing current data.
	if (gameData.app.nFrameCount != Window_rendered_data [window_num].frame) {
#if TRACE				
		con_printf (1, "Warning: Called WakeupRenderedObjects with a bogus window.\n");
#endif
		return;
	}

	nAiLastMissileCamera = OBJ_IDX (viewer);

	for (i = 0; i<Window_rendered_data [window_num].num_objects; i++) {
		int	objnum;
		object *objP;
		int	fcval = gameData.app.nFrameCount & 3;

		objnum = Window_rendered_data [window_num].rendered_objects [i];
		if ((objnum & 3) == fcval) {
			objP = &gameData.objs.objects [objnum];
	
			if (objP->type == OBJ_ROBOT) {
				if (VmVecDistQuick (&viewer->pos, &objP->pos) < F1_0*100) {
					ai_local		*ailp = &gameData.ai.localInfo [objnum];
					if (ailp->player_awareness_type == 0) {
						objP->ctype.ai_info.SUB_FLAGS |= SUB_FLAGS_CAMERA_AWAKE;
						ailp->player_awareness_type = PA_WEAPON_ROBOT_COLLISION;
						ailp->player_awareness_time = F1_0*3;
						ailp->previous_visibility = 2;
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
memset (gameData.objs.firstChild, 0xff, sizeof (gameData.objs.firstChild));
memset (gameData.objs.parentObjs, 0xff, sizeof (gameData.objs.parentObjs));
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

int AddChildObjectP (object *pParent, object *pChild)
{
return pParent ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
//return (pParent->type == OBJ_PLAYER) ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
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

int DelObjChildrenP (object *pParent)
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

int DelObjChildP (object *pChild)
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

tObjectRef *GetChildObjP (object *pParent, tObjectRef *pChildRef)
{
return GetChildObjN (OBJ_IDX (pParent), pChildRef);
}

//------------------------------------------------------------------------------
//eof
