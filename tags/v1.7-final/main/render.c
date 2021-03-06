/* $Id: render.c,v 1.18 2003/10/10 09:36:35 btb Exp $ */
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
 * Rendering Stuff
 *
 * Old Log:
 * Revision 1.9  1995/11/20  17:17:48  allender
 * *** empty log message ***
 *
 * Revision 1.8  1995/10/26  14:08:35  allender
 * added assigment for physics optimization
 *
 * Revision 1.7  1995/09/22  14:28:46  allender
 * changed render_zoom to make game match PC aspect
 *
 * Revision 1.6  1995/08/14  14:35:54  allender
 * change transparency to 0
 *
 * Revision 1.5  1995/08/12  11:32:02  allender
 * removed #ifdef NEWDEMO -- always in
 *
 * Revision 1.4  1995/07/05  16:48:31  allender
 * kitchen stuff
 *
 * Revision 1.3  1995/06/23  10:22:54  allender
 * fix outline mode
 *
 * Revision 1.2  1995/06/16  16:11:18  allender
 * changed sort func to accept const parameters
 *
 * Revision 1.1  1995/05/16  15:30:24  allender
 * Initial revision
 *
 * Revision 2.5  1995/12/19  15:31:36  john
 * Made stereo mode only record 1 eye in demo.
 *
 * Revision 2.4  1995/03/20  18:15:53  john
 * Added code to not store the normals in the segment structure.
 *
 * Revision 2.3  1995/03/13  16:11:05  john
 * Maybe fixed bug that lighting didn't work with vr helmets.
 *
 * Revision 2.2  1995/03/09  15:33:49  john
 * Fixed bug with iglasses timeout too long, and gameData.objs.objects
 * disappearing from left eye.
 *
 * Revision 2.1  1995/03/06  15:23:59  john
 * New screen techniques.
 *
 * Revision 2.0  1995/02/27  11:31:01  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.252  1995/02/22  13:49:38  allender
 * remove anonymous unions from object structure
 *
 * Revision 1.251  1995/02/11  15:07:26  matt
 * Took out code which was mostly intended as part of a larger renderer
 * change which never happened.  This new code was causing problems with
 * the level 4 control center.
 *
 * Revision 1.250  1995/02/07  16:28:53  matt
 * Fixed problem with new code
 *
 * Revision 1.249  1995/02/06  14:38:58  matt
 * Took out some code that didn't compile when editor in
 *
 * Revision 1.248  1995/02/06  13:45:25  matt
 * Structural changes, plus small sorting improvements
 *
 * Revision 1.247  1995/02/02  15:59:26  matt
 * Changed assert to int3.
 *
 * Revision 1.246  1995/02/01  21:02:27  matt
 * Added partial fix for rendering bugs
 * Ripped out laser hack system
 *
 * Revision 1.245  1995/01/20  15:14:30  matt
 * Added parens to fix precedence bug
 *
 * Revision 1.244  1995/01/14  19:16:59  john
 * First version of new bitmap paging code.
 *
 * Revision 1.243  1995/01/03  20:19:25  john
 * Pretty good working version of game save.
 *
 * Revision 1.242  1994/12/29  13:51:05  john
 * Made the floating reticle draw in the spot
 * regardless of the eye offset.
 *
 * Revision 1.241  1994/12/23  15:02:55  john
 * Tweaked floating reticle.
 *
 * Revision 1.240  1994/12/23  14:27:45  john
 * Changed offset of floating reticle to line up with
 * lasers a bit better.
 *
 * Revision 1.239  1994/12/23  14:22:50  john
 * Added floating reticle for VR helments.
 *
 * Revision 1.238  1994/12/13  14:07:50  matt
 * Fixed tmap_num2 bug in search mode
 *
 * Revision 1.237  1994/12/11  00:45:53  matt
 * Fixed problem when object sort buffer got full
 *
 * Revision 1.236  1994/12/09  18:46:06  matt
 * Added a little debugging
 *
 * Revision 1.235  1994/12/09  14:59:16  matt
 * Added system to attach a fireball to another object for rendering purposes,
 * so the fireball always renders on top of (after) the object.
 *
 * Revision 1.234  1994/12/08  15:46:54  matt
 * Fixed buffer overflow that caused seg depth screwup
 *
 * Revision 1.233  1994/12/08  11:51:53  matt
 * Took out some unused stuff
 *
 * Revision 1.232  1994/12/06  16:31:48  mike
 * fix detriangulation problems.
 *
 * Revision 1.231  1994/12/05  15:32:51  matt
 * Changed an assert to an int3 & return
 *
 * Revision 1.230  1994/12/04  17:28:04  matt
 * Got rid of unused no_render_flag array, and took out box clear when searching
 *
 * Revision 1.229  1994/12/04  15:51:14  matt
 * Fixed linear tmap transition for gameData.objs.objects
 *
 * Revision 1.228  1994/12/03  20:16:50  matt
 * Turn off window clip for gameData.objs.objects
 *
 * Revision 1.227  1994/12/03  14:48:00  matt
 * Restored some default settings
 *
 * Revision 1.226  1994/12/03  14:44:32  matt
 * Fixed another difficult bug in the window clip system
 *
 * Revision 1.225  1994/12/02  13:19:56  matt
 * Fixed rect clears at terminus of rendering
 * Made a bunch of debug code compile out
 *
 * Revision 1.224  1994/12/02  11:58:21  matt
 * Fixed window clip bug
 *
 * Revision 1.223  1994/11/28  21:50:42  mike
 * optimizations.
 *
 * Revision 1.222  1994/11/28  01:32:15  mike
 * turn off window clearing.
 *
 * Revision 1.221  1994/11/27  23:11:52  matt
 * Made changes for new //con_printf calling convention
 *
 * Revision 1.220  1994/11/20  15:58:55  matt
 * Don't migrate the control center, since it doesn't move out of its segment
 *
 * Revision 1.219  1994/11/19  23:54:36  mike
 * change window colors.
 *
 * Revision 1.218  1994/11/19  15:20:25  mike
 * rip out unused code and data
 *
 * Revision 1.217  1994/11/18  13:21:24  mike
 * Clear only view portals into rest of world based on value of nClearWindow.
 *
 * Revision 1.216  1994/11/15  17:02:10  matt
 * Re-added accidentally deleted variable
 *
 * Revision 1.215  1994/11/15  16:51:50  matt
 * Made rear view only switch to rear cockpit if cockpit on in front view
 *
 * Revision 1.214  1994/11/14  20:47:57  john
 * Attempted to strip out all the code in the game
 * directory that uses any ui code.
 *
 * Revision 1.213  1994/11/11  15:37:07  mike
 * write orange for background to show render bugs.
 *
 * Revision 1.212  1994/11/09  22:57:18  matt
 * Keep tract of depth of segments rendered, for detail level optimization
 *
 * Revision 1.211  1994/11/01  23:40:14  matt
 * Elegantly handler buffer getting full
 *
 * Revision 1.210  1994/10/31  22:28:13  mike
 * Fix detriangulation bug.
 *
 * Revision 1.209  1994/10/31  11:48:56  mike
 * Optimize detriangulation, speedup of about 4% in many cases, 0% in many.
 *
 * Revision 1.208  1994/10/30  20:08:34  matt
 * For endlevel: added big explosion at tunnel exit; made lights in tunnel
 * go out; made more explosions on walls.
 *
 * Revision 1.207  1994/10/27  14:14:35  matt
 * Don't do light flash during endlevel sequence
 *
 * Revision 1.206  1994/10/11  12:05:42  mike
 * Improve detriangulation.
 *
 * Revision 1.205  1994/10/07  15:27:00  john
 * Commented out the code that moves your eye
 * forward.
 *
 * Revision 1.204  1994/10/05  16:07:38  mike
 * Don't detriangulate sides if in player's segment.  Prevents player going behind a wall,
 * though there are cases in which it would be ok to detriangulate these.
 *
 * Revision 1.203  1994/10/03  12:44:05  matt
 * Took out unreferenced code
 *
 * Revision 1.202  1994/09/28  14:08:45  john
 * Added Zoom stuff back in, but ifdef'd it out.
 *
 * Revision 1.201  1994/09/25  23:41:49  matt
 * Changed the object load & save code to read/write the structure fields one
 * at a time (rather than the whole structure at once).  This mean that the
 * object structure can be changed without breaking the load/save functions.
 * As a result of this change, the local_object data can be and has been
 * incorporated into the object array.  Also, timeleft is now a property
 * of all gameData.objs.objects, and the object structure has been otherwise cleaned up.
 *
 * Revision 1.200  1994/09/25  15:50:10  mike
 * Integrate my debug changes which shows how many textures were rendered
 * this frame.
 *
 * Revision 1.199  1994/09/25  15:45:22  matt
 * Added OBJ_LIGHT, a type of object that casts light
 * Added generalized lifeleft, and moved it to local_object
 *
 * Revision 1.198  1994/09/15  21:23:32  matt
 * Changed system to keep track of whether & what cockpit is up
 *
 * Revision 1.197  1994/09/15  16:30:12  mike
 * Comment out call to object_render_targets, which did nothing.
 *
 * Revision 1.196  1994/09/07  22:25:51  matt
 * Don't migrate through semi-transparent walls
 *
 * Revision 1.195  1994/09/07  19:16:21  mike
 * Homing missile.
 *
 * Revision 1.194  1994/08/31  20:54:17  matt
 * Don't do flash effect while whiting out
 *
 * Revision 1.193  1994/08/23  17:20:12  john
 * Added rear-view cockpit.
 *
 * Revision 1.192  1994/08/22  14:36:35  john
 * Made R key make a "reverse" view render.
 *
 * Revision 1.191  1994/08/19  20:09:26  matt
 * Added end-of-level cut scene with external scene
 *
 * Revision 1.190  1994/08/10  19:56:17  john
 * Changed font stuff; Took out old menu; messed up lots of
 * other stuff like game sequencing messages, etc.
 *
 * Revision 1.189  1994/08/10  14:45:05  john
 * *** empty log message ***
 *
 * Revision 1.188  1994/08/09  16:04:06  john
 * Added network players to editor.
 *
 * Revision 1.187  1994/08/05  17:07:05  john
 * Made lasers be two gameData.objs.objects, one drawing after the other
 * all the time.
 *
 * Revision 1.186  1994/08/05  10:07:57  matt
 * Disable window check checking (i.e., always use window check)
 *
 * Revision 1.185  1994/08/04  19:11:30  matt
 * Changed a bunch of vecmat calls to use multiple-function routines, and to
 * allow the use of C macros for some functions
 *
 * Revision 1.184  1994/08/04  00:21:14  matt
 * Cleaned up fvi & physics error handling; put in code to make sure gameData.objs.objects
 * are in correct segment; simplified segment finding for gameData.objs.objects and points
 *
 * Revision 1.183  1994/08/02  19:04:28  matt
 * Cleaned up vertex list functions
 *
 * Revision 1.182  1994/07/29  15:13:33  matt
 * When window check turned off, cut render depth in half
 *
 * Revision 1.181  1994/07/29  11:03:50  matt
 * Use highest_segment_index instead of num_segments so render works from
 * the editor
 *
 * Revision 1.180  1994/07/29  10:04:34  mike
 * Update Cursegp when an object is selected.
 *
 * Revision 1.179  1994/07/25  00:02:50  matt
 * Various changes to accomodate new 3d, which no longer takes point numbers
 * as parms, and now only takes pointers to points.
 *
 * Revision 1.178  1994/07/24  14:37:49  matt
 * Added angles for player head
 *
 * Revision 1.177  1994/07/20  19:08:07  matt
 * If in editor, don't move eye from center of viewer object
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pa_enabl.h"                   //$$POLY_ACC
#include "inferno.h"
#include "segment.h"
#include "error.h"
#include "bm.h"
#include "texmap.h"
#include "mono.h"
#include "render.h"
#include "game.h"
#include "object.h"
#include "laser.h"
#include "textures.h"
#include "screens.h"
#include "segpoint.h"
#include "wall.h"
#include "texmerge.h"
#include "physics.h"
#include "3d.h"
#include "gameseg.h"
#include "vclip.h"
#include "lighting.h"
#include "cntrlcen.h"
#include "newdemo.h"
#include "automap.h"
#include "endlevel.h"
#include "key.h"
#include "newmenu.h"
#include "u_mem.h"
#include "piggy.h"
#include "network.h"
#include "switch.h"
#include "hudmsg.h"
#include "cameras.h"
#include "kconfig.h"
#include "mouse.h"
#include "particles.h"
#include "globvars.h"
#include "oof.h"

#ifdef OGL
#include "ogl_init.h"
#include "lightmap.h"
#endif

//------------------------------------------------------------------------------

extern tFaceColor tMapColor, lightColor, vertColors [4];
extern tRgbColorf globalDynColor;
extern char bGotGlobalDynColor;
char bUseGlobalColor = 0;

#define INITIAL_LOCAL_LIGHT (F1_0/4)    // local light value in segment of occurence (of light emission)

#ifdef EDITOR
#include "editor/editor.h"
#endif

#if defined(POLY_ACC)
#include "poly_acc.h"
#endif

//used for checking if points have been rotated
unsigned int	nClearWindowColor = 0;
int				nClearWindow = 2;	// 1 = Clear whole background window, 2 = clear view portals into rest of world, 0 = no clear

int nRLFrameCount=-1;
int nRotatedLast [MAX_VERTICES];

// When any render function needs to know what's looking at it, it should 
// access gameData.objs.viewer members.
vms_vector viewerEye;  //valid during render

int	nRenderSegs;
static int renderState = -1;

fix nRenderZoom = 0x9000;					//the player's zoom factor
fix Render_zoom_scale = 1;					//the player's zoom factor

#ifndef NDEBUG
ubyte bObjectRendered [MAX_OBJECTS];
#endif

GLuint glSegOccQuery [MAX_SEGMENTS];
GLuint glObjOccQuery [MAX_OBJECTS];
char bSidesRendered [MAX_SEGMENTS];

#ifdef EDITOR
int	Render_only_bottom = 0;
int	Bottom_bitmap_num = 9;
#endif

fix	Face_reflectivity = (F1_0/2);

//------------------------------------------------------------------------------

#ifdef EDITOR
int bSearchMode = 0;			//true if looking for curseg,side,face
short _search_x,_search_y;	//pixel we're looking at
int found_seg,found_side,found_face,found_poly;
#else
#define bSearchMode 0
#endif

int bOutLineMode=0,bShowOnlyCurSide=0;

//------------------------------------------------------------------------------

int ToggleOutlineMode (void)
{
	return bOutLineMode = !bOutLineMode;
}

//------------------------------------------------------------------------------

int ToggleShowOnlyCurSide (void)
{
return bShowOnlyCurSide = !bShowOnlyCurSide;
}

//------------------------------------------------------------------------------

void DrawOutline (int nv, g3s_point **pointlist)
{
	int i;
	GLint depthFunc; 

#if 1 //def RELEASE
	if (gameStates.render.bQueryOcclusion) {
		G3DrawPolyAlpha (nv, pointlist, 1, 1, 0, -1);
		return;
		}
#endif

glGetIntegerv (GL_DEPTH_FUNC, &depthFunc);
glDepthFunc(GL_ALWAYS);
GrSetColorRGB (255, 255, 255, 255);
for (i = 0; i < nv; i++)
	G3DrawLine (pointlist [i], pointlist [(i + 1) % nv]);
glDepthFunc (depthFunc);
}

//------------------------------------------------------------------------------

grs_canvas * reticle_canvas = NULL;

void _CDECL_ FreeReticleCanvas (void)
{
if (reticle_canvas)	{
	LogErr ("unloading reticle data\n");
	d_free( reticle_canvas->cv_bitmap.bm_texBuf );
	d_free( reticle_canvas );
	reticle_canvas	= NULL;
	}
}

extern void ShowReticle(int force_big);

//------------------------------------------------------------------------------

// Draw the reticle in 3D for head tracking
void Draw3DReticle (fix nEyeOffset)
{
	g3s_point 	reticlePoints [4];
	uvl			uvl [4];
	g3s_point	*pointlist [4];
	int 			i;
	vms_vector	v1, v2;
	grs_canvas	*saved_canvas;
	int			saved_interp_method;

//	if (!bUsePlayerHeadAngles) return;
	
for (i = 0; i < 4; i++) {
	reticlePoints [i].p3_index = -1;
	pointlist [i] = reticlePoints + i;
	uvl [i].l = MAX_LIGHT;
	}
uvl [0].u =
uvl [0].v =
uvl [1].v =
uvl [3].u = 0; 
uvl [1].u =
uvl [2].u =
uvl [2].v =
uvl [3].v = F1_0;

VmVecScaleAdd( &v1,&gameData.objs.viewer->pos, &gameData.objs.viewer->orient.fvec, F1_0*4 );
VmVecScaleInc(&v1,&gameData.objs.viewer->orient.rvec,nEyeOffset);

VmVecScaleAdd( &v2, &v1, &gameData.objs.viewer->orient.rvec, -F1_0*1 );
VmVecScaleInc( &v2, &gameData.objs.viewer->orient.uvec, F1_0*1 );
G3TransformAndEncodePoint(reticlePoints,&v2);

VmVecScaleAdd( &v2, &v1, &gameData.objs.viewer->orient.rvec, +F1_0*1 );
VmVecScaleInc( &v2, &gameData.objs.viewer->orient.uvec, F1_0*1 );
G3TransformAndEncodePoint(reticlePoints + 1,&v2);

VmVecScaleAdd( &v2, &v1, &gameData.objs.viewer->orient.rvec, +F1_0*1 );
VmVecScaleInc( &v2, &gameData.objs.viewer->orient.uvec, -F1_0*1 );
G3TransformAndEncodePoint(reticlePoints + 2,&v2);

VmVecScaleAdd( &v2, &v1, &gameData.objs.viewer->orient.rvec, -F1_0*1 );
VmVecScaleInc( &v2, &gameData.objs.viewer->orient.uvec, -F1_0*1 );
G3TransformAndEncodePoint(reticlePoints + 3,&v2);

if ( reticle_canvas == NULL )	{
	reticle_canvas = GrCreateCanvas(64,64);
	if ( !reticle_canvas )
		Error( "Couldn't d_malloc reticle_canvas" );
	atexit( FreeReticleCanvas );
	//reticle_canvas->cv_bitmap.bm_handle = 0;
	reticle_canvas->cv_bitmap.bm_props.flags = BM_FLAG_TRANSPARENT;
	}

saved_canvas = grdCurCanv;
GrSetCurrentCanvas(reticle_canvas);
GrClearCanvas(0);		// Clear to Xparent
ShowReticle(1);
GrSetCurrentCanvas(saved_canvas);

saved_interp_method=gameStates.render.nInterpolationMethod;
gameStates.render.nInterpolationMethod	= 3;		// The best, albiet slowest.
G3DrawTexPoly (4, pointlist, uvl, &reticle_canvas->cv_bitmap, NULL, 1);
gameStates.render.nInterpolationMethod	= saved_interp_method;
}


//------------------------------------------------------------------------------

//cycle the flashing light for when mine destroyed
void FlashFrame()
{
	static fixang flash_ang=0;

if (!gameData.reactor.bDestroyed && !gameStates.gameplay.seismic.nMagnitude)
	return;
if (gameStates.app.bEndLevelSequence)
	return;
if (gameStates.ogl.palAdd.blue > 10 )		//whiting out
	return;
//	flash_ang += fixmul(FLASH_CYCLE_RATE,gameData.app.xFrameTime);
if (gameStates.gameplay.seismic.nMagnitude) {
	fix	added_flash;

	added_flash = abs(gameStates.gameplay.seismic.nMagnitude);
	if (added_flash < F1_0)
		added_flash *= 16;

	flash_ang += fixmul(gameStates.render.nFlashRate, fixmul(gameData.app.xFrameTime, added_flash+F1_0));
	fix_fastsincos(flash_ang,&gameStates.render.nFlashScale,NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + F1_0*3)/4;	//	gets in range 0.5 to 1.0
	}
else {
	flash_ang += fixmul(gameStates.render.nFlashRate,gameData.app.xFrameTime);
	fix_fastsincos(flash_ang,&gameStates.render.nFlashScale,NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + f1_0)/2;
	if (gameStates.app.nDifficultyLevel == 0)
		gameStates.render.nFlashScale = (gameStates.render.nFlashScale+F1_0*3)/4;
	}
}

// ----------------------------------------------------------------------------
//	Render a face.
//	It would be nice to not have to pass in segnum and sidenum, but
//	they are used for our hideously hacked in headlight system.
//	vp is a pointer to vertex ids.
//	tmap1, tmap2 are texture map ids.  tmap2 is the pasty one.

int RenderColoredSegment (int segnum, int sidenum, int nv, g3s_point **pointlist)
{
	short csegnum = gameData.segs.segments [segnum].children [sidenum];
	int	funcRes = 1;
	int	owner = gameData.segs.xSegments [segnum].owner;
	int	special = gameData.segs.segment2s [segnum].special;

gameStates.render.grAlpha = 6;
if ((gameData.app.nGameMode & GM_ENTROPY) && (extraGameInfo [1].entropy.nOverrideTextures == 2) && (owner > 0)) {
	if ((csegnum < 0) || (gameData.segs.xSegments [csegnum].owner != owner)) {
		if (owner == 1)
			G3DrawPolyAlpha (nv, pointlist, 0, 0, 0.5, -1);		//draw as flat poly
		else
			G3DrawPolyAlpha (nv,pointlist, 0.5, 0, 0, -1);		//draw as flat poly
		}
	}
else if (special == SEGMENT_IS_WATER) {
	if ((csegnum < 0) || (gameData.segs.segment2s [csegnum].special != SEGMENT_IS_WATER))
		G3DrawPolyAlpha(nv, pointlist, 0, 1.0 / 16.0, 0.5, -1);		//draw as flat poly
	}
else if (special == SEGMENT_IS_LAVA) {
	if ((csegnum < 0) || (gameData.segs.segment2s [csegnum].special != SEGMENT_IS_LAVA))
		G3DrawPolyAlpha (nv,pointlist, 0.5, 0, 0, -1);		//draw as flat poly
	}
else
	funcRes = 0;
gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
return funcRes;
}

//------------------------------------------------------------------------------

typedef struct tFaceProps {
	short			segNum, sideNum;
	short			tMap1, tMap2;
	uvl			uvls [4];
	short			vp [4];
#if LIGHTMAPS
	uvl			uvl_lMaps [4];
#endif
	vms_vector	vNormal;
	ubyte			nv;
	ubyte			widFlags;
	char			renderState;
} tFaceProps;

typedef struct tFaceListEntry {
	short			nextFace;
	tFaceProps	props;
} tFaceListEntry;

static tFaceListEntry	faceList [6 * 2 * MAX_SEGMENTS];
static short				faceListRoots [MAX_TEXTURES];
static short				faceListTails [MAX_TEXTURES];
static short				nFaceListSize;

//------------------------------------------------------------------------------

static inline void ScaleColor (tFaceColor *color, float l)
{
#if 1
#	if 0
color->color.red *= l;
color->color.green *= l;
color->color.blue *= l;
#	else
	float m = color->color.red;

if (m < color->color.green)
	m = color->color.green;
if (m < color->color.blue)
	m = color->color.blue;
m = l / m;
color->color.red *= m;
color->color.green *= m;
color->color.blue *= m;
#	endif
#endif
}

//------------------------------------------------------------------------------

int SetVertexColors (tFaceProps *propsP)
{
if (gameStates.ogl.bHaveLights && gameOpts->ogl.bUseLighting) {
	// set material properties specific for certain textures here
	SetOglLightMaterial (propsP->segNum, propsP->sideNum, -1);
	return 0;
	}
if (gameOpts->render.color.bAmbientLight && gameStates.app.bD2XLevel && !USE_LIGHTMAPS) { 
#if VERTEX_LIGHTING
	int i, j = propsP->nv;
	for (i = 0; i < j; i++)
		vertColors [i] = gameData.render.color.vertices [propsP->vp [i]];
	}
else
	memset (vertColors, 0, sizeof (vertColors));
#else
	tFaceColor *colorP = gameData.render.color.sides [segnum] + sidenum;
	if (colorP->index)
		lightColor = *colorP;
	else
		lightColor.color.red =
		lightColor.color.blue =
		lightColor.color.green = 1.0;
		}
else
	lightColor.index = 0;
#endif
return 1;
}

//------------------------------------------------------------------------------

int SetFaceLight (tFaceProps *propsP)
{
	int			h, i;
	tFaceColor	*pvc = vertColors;
	tRgbColorf	*pdc;
	fix			dynLight;

if (gameStates.ogl.bHaveLights && gameOpts->ogl.bUseLighting)
	return 0;
for (i = 0; i < propsP->nv; i++, pvc++) {
	//the uvl struct has static light already in it
	//scale static light for destruction effect
#if LMAP_LIGHTADJUST
	if (gameStates.render.color.bLightMapsOk && 
			gameOpts->render.color.bAmbientLight && 
			gameOpts->render.color.bUseLightMaps && 
			!IsMultiGame) {
		propsP->uvls [i].l = F1_0 / 2 + gameData.render.lights.segDeltas [propsP->segNum][propsP->sideNum];
		if (propsP->uvls [i].l < 0)
			propsP->uvls [i].l = 0;
		}
#endif
	if (gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)	//make lights flash
		propsP->uvls [i].l = fixmul (gameStates.render.nFlashScale,propsP->uvls [i].l);
	//add in dynamic light (from explosions, etc.)
	dynLight = dynamicLight [h = propsP->vp [i]];
#ifdef _DEBUG
	if (dynLight)
#endif
	propsP->uvls [i].l += dynLight;
#if 0
	if (gameData.app.nGameMode & GM_ENTROPY) {
		if (segP->owner == 1) {
			tMapColor.index = 1;
			tMapColor.color.red = 
			tMapColor.color.green = 5.0 / 63.0;
			tMapColor.color.blue = 1.0;
			}
		else if (segP->owner == 3) {
			tMapColor.index = 1;
			tMapColor.color.red = 1.0;
			tMapColor.color.green = 
			tMapColor.color.blue = 5.0 / 63.0;
			}
		}
#endif
	if (gameStates.app.bHaveExtraGameInfo [IsMultiGame] && gameOpts->render.color.bGunLight) {
		if (bUseGlobalColor) {
			if (bGotGlobalDynColor) {
				tMapColor.index = 1;
				memcpy (&tMapColor.color, &globalDynColor, sizeof (tRgbColorf));
				}
			}
		else if (bGotDynColor [h]) {
			pdc = dynamicColor + h;
			pvc->index = -1;
			if (gameOpts->render.color.bMix) {
				float dl = f2fl (dynLight);
#if 0
				pvc->color.red = (pvc->color.red + dynamicColor [h].red * gameOpts->render.color.bMix) / (float) (gameOpts->render.color.bMix + 1);
				pvc->color.green = (pvc->color.green + pdc->green * gameOpts->render.color.bMix) / (float) (gameOpts->render.color.bMix + 1);
				pvc->color.blue = (pvc->color.blue + pdc->blue * gameOpts->render.color.bMix) / (float) (gameOpts->render.color.bMix + 1);
#else
				if (gameStates.app.bD2XLevel && 
						gameOpts->render.color.bAmbientLight && 
						!gameOpts->render.color.bUseLightMaps && 
						(pvc->index != -1)) {
					pvc->color.red += pdc->red * dl;
					pvc->color.green += pdc->green * dl;
					pvc->color.blue += pdc->blue * dl;
					}
				else {
					float l = f2fl (propsP->uvls [i].l);
					pvc->color.red = l + pdc->red * dl;
					pvc->color.green = l + pdc->green * dl;
					pvc->color.blue = l + pdc->blue * dl;
					}
				if (gameOpts->render.color.bCap) {
					if (pvc->color.red > 1.0)
						pvc->color.red = 1.0;
					if (pvc->color.green > 1.0)
						pvc->color.green = 1.0;
					if (pvc->color.blue > 1.0)
						pvc->color.blue = 1.0;
					}
#endif
				}
			else {
				float dl = f2fl (propsP->uvls [i].l);
				dl = (float) pow (dl, 1.0 / 3.0); //sqrt (dl);
				pvc->color.red = pdc->red * dl;
				pvc->color.green = pdc->green * dl;
				pvc->color.blue = pdc->blue * dl;
				}
			}
		else {
			if (pvc->index)
				ScaleColor (pvc, f2fl (propsP->uvls [i].l));
			}
		}
	else {
		if (pvc->index)
			ScaleColor (pvc, f2fl (propsP->uvls [i].l));
		}
	//add in light from player's headlight
	// -- Using new headlight system...propsP->uvls [i].l += compute_headlight_light(&gameData.segs.points [propsP->vp [i]].p3_vec,face_light);
	//saturate at max value
	if (propsP->uvls [i].l > MAX_LIGHT)
		propsP->uvls [i].l = MAX_LIGHT;
	}
return 1;
}

//------------------------------------------------------------------------------

int RenderWall (tFaceProps *propsP, g3s_point **pointlist, int bIsMonitor)
{
short c, nWallNum = WallNumI (propsP->segNum, propsP->sideNum);

if (IS_WALL (nWallNum)) {
	if (propsP->widFlags & (WID_CLOAKED_FLAG | WID_TRANSPARENT_FLAG)) {
		if (!bIsMonitor) {
			if (!RenderColoredSegment (propsP->segNum, propsP->sideNum, propsP->nv, pointlist)) {
				c = gameData.walls.walls [nWallNum].cloak_value;
				if (propsP->widFlags & WID_CLOAKED_FLAG) {
					if (c < GR_ACTUAL_FADE_LEVELS) {
						gameStates.render.grAlpha = (float) c;
						G3DrawPolyAlpha (propsP->nv, pointlist, 0, 0, 0, -1);		//draw as flat poly
						}
					}
				else {
					if (!gameOpts->render.color.bWalls)
						c = 0;
					if (gameData.walls.walls [nWallNum].hps)
						gameStates.render.grAlpha = (float) fabs ((1.0f - (float) gameData.walls.walls [nWallNum].hps / ((float) F1_0 * 100.0f)) * GR_ACTUAL_FADE_LEVELS);
					else if (IsMultiGame && gameStates.app.bHaveExtraGameInfo [1])
						gameStates.render.grAlpha = (float) extraGameInfo [1].grWallTransparency;
					else
						gameStates.render.grAlpha = (float) extraGameInfo [0].grWallTransparency;
					if (gameStates.render.grAlpha < GR_ACTUAL_FADE_LEVELS)
						G3DrawPolyAlpha (propsP->nv, pointlist, CPAL2Tr (gamePalette, c), CPAL2Tg (gamePalette, c), CPAL2Tb (gamePalette, c), -1);	//draw as flat poly
					}
				}
			gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
			return 1;
			}
		}
	else if (gameStates.app.bD2XLevel) {
		c = gameData.walls.walls [nWallNum].cloak_value;
		if (c && (c < GR_ACTUAL_FADE_LEVELS))
			gameStates.render.grAlpha = (float) (GR_ACTUAL_FADE_LEVELS - c);
		}
	else	
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
return 0;
}

//------------------------------------------------------------------------------

grs_bitmap *LoadFaceBitmap (short tMapNum, short nFrameNum);

#if LIGHTMAPS
#	define LMAP_LIGHTADJUST	1
#else
#	define LMAP_LIGHTADJUST	0
#endif

void RenderFace (tFaceProps *propsP, int offs, int bRender)
{
	tFaceProps	props;

if (propsP->tMap1 < 0)
	return;
#if 1
props = *propsP;
memcpy (props.uvls, propsP->uvls + offs, props.nv * sizeof (*props.uvls));
memcpy (props.vp, propsP->vp + offs, props.nv * sizeof (*props.vp));
#else
props.segNum = propsP->segNum;
props.sideNum = propsP->sideNum;
props.tMap1 = propsP->tMap1;
props.tMap2 = propsP->tMap2;
props.nv = propsP->nv;
memcpy (props.uvls, propsP->uvls + offs, props.nv * sizeof (*props.uvls));
memcpy (props.vp, propsP->vp + offs, props.nv * sizeof (*props.vp));
#if LIGHTMAPS
memcpy (props.uvl_lMaps, propsP->uvl_lMaps + offs, props.nv * sizeof (*props.uvl_lMaps));
#endif
memcpy (&props.vNormal, &propsP->vNormal, sizeof (props.vNormal));
props.widFlags = propsP->widFlags;
#endif
#ifdef _DEBUG //convenient place for a debug breakpoint
if (props.segNum == 91 && props.sideNum == 0)
	props.segNum = props.segNum;
#endif

#if APPEND_LAYERED_TEXTURES
if (!gameOpts->render.bOptimize || bRender) 
#else
if (!bRender) 
#endif
{
	// -- Using new headlight system...fix			face_light;
	grs_bitmap  *bmBot = NULL;
#ifdef OGL
	grs_bitmap  *bmTop = NULL;
#endif

	int			i, bIsMonitor, bIsTeleCam, bHaveCamImg, nCamNum, bCamBufAvail;
	g3s_point	*pointlist [8];
	segment		*segP = gameData.segs.segments + props.segNum;
	side			*sideP = segP->sides + props.sideNum;
	tCamera		*pc = NULL;

Assert(props.nv <= 4);
	for (i = 0; i < props.nv; i++)
		pointlist [i] = gameData.segs.points + props.vp [i];
#if 1
	if (gameStates.render.bQueryOcclusion) {
		DrawOutline(props.nv, pointlist);
		return;
		}
	if (!(gameOpts->render.bTextures || IsMultiGame))
		goto drawWireFrame;
#endif
	SetVertexColors (propsP);
#ifdef OGL_ZBUF
	if (!gameOpts->legacy.bZBuf && !gameOpts->legacy.bRender && (renderState == 2)) {
		RenderColoredSegment (props.segNum, props.sideNum, props.nv, pointlist);
		return;
		}
#endif
	nCamNum = nSideCameras [props.segNum][props.sideNum];
	bIsTeleCam = 0;
	bIsMonitor = extraGameInfo [0].bUseCameras && 
					 (!IsMultiGame || (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bUseCameras)) && 
					 !gameStates.render.cameras.bActive && (nCamNum >= 0);
	if (bIsMonitor) {
		pc = cameras + nCamNum;
		bIsTeleCam = pc->bTeleport;
#if RENDER2TEXTURE
		bCamBufAvail = OglCamBufAvail (pc, 1) == 1;
#else
		bCamBufAvail = 0;
#endif
		bHaveCamImg = pc->bValid && /*!pc->bShadowMap &&*/ 
						  (pc->texBuf.glTexture || bCamBufAvail) &&
						  (!bIsTeleCam || EGI_FLAG (bTeleporterCams, 0, 0));
		}
	else {
		bHaveCamImg = 0;
		bCamBufAvail = 0;
		}
	//handle cloaked walls
	if (bIsMonitor)
		pc->bVisible = 1;
	if (RenderWall (&props, pointlist, bIsMonitor))
		return;
	// -- Using new headlight system...face_light = -VmVecDot(&gameData.objs.viewer->orient.fvec,norm);
	if (props.widFlags & WID_RENDER_FLAG) {		//if (WALL_IS_DOORWAY(segP, sidenum) == WID_NO_WALL)
		if (props.tMap1 >= gameData.pig.tex.nTextures [gameStates.app.bD1Data]) {
#if TRACE	
			//con_printf (CON_DEBUG,"Invalid tmap number %d, gameData.pig.tex.nTextures=%d, changing to 0\n",tmap1,gameData.pig.tex.nTextures);
#endif
#ifndef RELEASE
			Int3();
#endif
		sideP->tmap_num = 0;
		}
	if (!(bHaveCamImg && gameOpts->render.cameras.bFitToWall)) {
		if (gameOpts->ogl.bGlTexMerge && gameStates.render.textures.bGlsTexMergeOk) {

			bmBot = LoadFaceBitmap (props.tMap1, sideP->frame_num);
			if (props.tMap2)
				bmTop = LoadFaceBitmap ((short) (props.tMap2 & 0x3fff), sideP->frame_num);
			}
		else
			// New code for overlapping textures...
			if (props.tMap2 != 0) {
				bmBot = TexMergeGetCachedBitmap (props.tMap1, props.tMap2);
#ifdef _DEBUG
				if (!bmBot)
					bmBot = TexMergeGetCachedBitmap (props.tMap1, props.tMap2);
#endif
				}
			else {
				bmBot = gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [props.tMap1].index;
				PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [props.tMap1], gameStates.app.bD1Mission);
				}
//		Assert(!(bmBot->bm_props.flags & BM_FLAG_PAGED_OUT));
		}
	//else 
	if (bHaveCamImg) {
		GetCameraUVL (pc, props.uvls);
		pc->texBuf.glTexture->wrapstate = -1;
		if (bIsTeleCam) {
#ifdef _DEBUG
			bmBot = &pc->texBuf;
			gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
#else
			bmTop = &pc->texBuf;
			gameStates.render.grAlpha = (GR_ACTUAL_FADE_LEVELS * 7) / 10;
#endif
			}
		else if (gameOpts->render.cameras.bFitToWall || (props.tMap2 > 0))
			bmBot = &pc->texBuf;
		else
			bmTop = &pc->texBuf;
		}
	SetFaceLight (&props);
#ifdef EDITOR
	if (Render_only_bottom && (sidenum == WBOTTOM))
		G3DrawTexPoly (props.nv, pointlist, props.uvls, gameData.pig.tex.bitmaps + gameData.pig.tex.bmIndex [Bottom_bitmap_num].index,1);
	else
#endif
#ifdef OGL
	if (bmTop)
		G3DrawTexPolyMulti (
			props.nv, 
			pointlist,
			props.uvls, 
#if LIGHTMAPS
			props.uvl_lMaps, 
#endif
			bmBot, bmTop, 
#if LIGHTMAPS
			lightMaps + props.segNum * 6 + props.sideNum, 
#endif
			&props.vNormal,
			(props.tMap2 >> 14) & 3, 
			!bIsMonitor || bIsTeleCam); //(bIsMonitor || (bmBot->bm_props.flags & BM_FLAG_TGA))); //((tmap2&0xC000)>>14) & 3);
	else
#endif
#if LIGHTMAPS == 0
		G3DrawTexPoly (
			props.nv,
			pointlist, 
			props.uvls, 
			bmBot, 
			&props.vNormal,
			!bIsMonitor || bIsTeleCam); //(bIsMonitor && !gameOpts->render.cameras.bFitToWall) || (bmBot->bm_props.flags & BM_FLAG_TGA));
#else
		G3DrawTexPolyMulti (
			props.nv,
			pointlist,
			props.uvls, 
			props.uvl_lMaps,
			bmBot,
			NULL,
			lightMaps + props.segNum * 6 + props.sideNum, 
			&props.vNormal,
			0,
			!bIsMonitor || bIsTeleCam); //(bIsMonitor && !gameOpts->render.cameras.bFitToWall) || (bmBot->bm_props.flags & BM_FLAG_TGA));
#endif
		}
gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
#ifdef OGL
		// render the segment the player is in with a transparent color if it is a water or lava segment
		//if (segnum == gameData.objs.objects->segnum) 
# ifdef OGL_ZBUF
	if (gameOpts->legacy.bZBuf && !gameOpts->legacy.bRender)
# endif
		RenderColoredSegment (props.segNum, props.sideNum, props.nv, pointlist);
#endif
#ifndef NDEBUG
	if (bOutLineMode) 
		DrawOutline (props.nv, pointlist);
#endif
drawWireFrame:
	if (gameOpts->render.bWireFrame && !IsMultiGame)
		DrawOutline (props.nv, pointlist);
	}
else {
	tFaceListEntry	*flp = faceList + nFaceListSize;
#if APPEND_LAYERED_TEXTURES
	short				t = props.tMap1;
	if (!props.tMap2 || (faceListRoots [props.tMap1] < 0)) {
#else
	short				t = props.tMap2 ? props.tMap2 & 0x3fff : props.tMap1;
#endif
		flp->nextFace = faceListRoots [t];
		if (faceListTails [t] < 0)
			faceListTails [t] = nFaceListSize;
		faceListRoots [t] = nFaceListSize++;
#if APPEND_LAYERED_TEXTURES
		}
	else {
		tFaceListEntry	*flh = faceList + faceListTails [t];
		flh->nextFace = nFaceListSize;
		flp->nextFace = -1;
		faceListTails [t] = nFaceListSize++;
		}
#endif
	props.renderState = (char) renderState;
	flp->props = props;
	}
}

#ifdef EDITOR
// ----------------------------------------------------------------------------
//	Only called if editor active.
//	Used to determine which face was clicked on.
void CheckFace(int segnum, int sidenum, int facenum, int nv, short *vp, int tmap1, int tmap2, uvl *uvlp)
{
	int	i;

	if (bSearchMode) {
		int save_lighting;
		grs_bitmap *bm;
		uvl uvlCopy [8];
		g3s_point *pointlist [4];

		if (tmap2 > 0 )
			bm = TexMergeGetCachedBitmap( tmap1, tmap2 );
		else
			bm = gameData.pig.tex.bitmaps + gameData.pig.tex.bmIndex [tmap1].index;

		for (i=0; i<nv; i++) {
			uvlCopy [i] = uvlp [i];
			pointlist [i] = gameData.segs.points + vp [i];
		}

		GrSetColor(0);
		gr_pixel(_search_x,_search_y);	//set our search pixel to color zero
		GrSetColor(1);					//and render in color one
 save_lighting = gameStates.render.nLighting;
 gameStates.render.nLighting = 2;
		//G3DrawPoly(nv,vp);
		G3DrawTexPoly(nv,pointlist, (uvl *)uvlCopy, bm, 1);
 gameStates.render.nLighting = save_lighting;

		if (gr_ugpixel(&grdCurCanv->cv_bitmap,_search_x,_search_y) == 1) {
			found_seg = segnum;
			found_side = sidenum;
			found_face = facenum;
		}
	}
}
#endif

fix	Tulate_min_dot = (F1_0/4);
//--unused-- fix	Tulate_min_ratio = (2*F1_0);
fix	Min_n0_n1_dot	= (F1_0*15/16);

extern int contains_flare(segment *segP, int sidenum);
extern fix	Obj_light_xlate [16];

// -----------------------------------------------------------------------------------
//	Render a side.
//	Check for normal facing.  If so, render faces on side dictated by sideP->type.

#undef LMAP_LIGHTADJUST
#define LMAP_LIGHTADJUST 0

void RenderSide (segment *segP, short sidenum)
{
//	short			props.vp [4];
//	short			segnum = SEG_IDX (segP);
	side			*sideP = segP->sides + sidenum;
	vms_vector	tvec;
	fix			v_dot_n0, v_dot_n1;
//	uvl			temp_uvls [4];
	fix			min_dot, max_dot;
	vms_vector  normals [2];
//	ubyte			wid_flags = WALL_IS_DOORWAY (segP, sidenum, NULL);
	int			bDoLightMaps = gameStates.render.color.bLightMapsOk && 
										gameOpts->render.color.bUseLightMaps && 
										gameOpts->render.color.bAmbientLight && 
										!IsMultiGame;
	tFaceProps	props;

#if LIGHTMAPS
#define	LMAP_SIZE	(1.0 / 16.0)

	static uvl	uvl_lMaps [4] = {
		{fl2f (LMAP_SIZE), fl2f (LMAP_SIZE), 0},
		{fl2f (1.0 - LMAP_SIZE), fl2f (LMAP_SIZE), 0},
		{fl2f (1.0 - LMAP_SIZE), fl2f (1.0 - LMAP_SIZE), 0},
		{fl2f (LMAP_SIZE), fl2f (1.0 - LMAP_SIZE), 0}
	};
#endif

	props.segNum = SEG_IDX (segP);
	props.sideNum = sidenum;
	props.widFlags = WALL_IS_DOORWAY (segP, props.sideNum, NULL);
	if (!(gameOpts->render.bWalls || IsMultiGame) && IS_WALL (WallNumP (segP, props.sideNum)))
		return;
	switch (renderState) {
		case -1:
			if (!(props.widFlags & WID_RENDER_FLAG) && (gameData.segs.segment2s [props.segNum].special < SEGMENT_IS_WATER))		//if (WALL_IS_DOORWAY(segP, props.sideNum) == WID_NO_WALL)
				return;
			break;
		case 0:
			if (segP->children [props.sideNum] >= 0) //&& IS_WALL (WallNumP (segP, props.sideNum)))
				return;
			break;
		case 1:
			if (!IS_WALL (WallNumP (segP, props.sideNum))) 
				return;
			break;
		case 2:
			if ((gameData.segs.segment2s [props.segNum].special < SEGMENT_IS_WATER) &&
				 (gameData.segs.xSegments [props.segNum].owner < 1))
				return;
			break;
		}
//CBRK (props.segNum == 123 && props.sideNum == 2);
	bSidesRendered [props.segNum]++;
#ifdef COMPACT_SEGS
	GetSideNormals (segP, props.sideNum, normals, normals+1);
#else
	normals [0] = sideP->normals [0];
	normals [1] = sideP->normals [1];
#endif
#if LIGHTMAPS
if (bDoLightMaps) {
		float	Xs = 8;
		float	h = 0.5f / (float) Xs;

	props.uvl_lMaps [0].u =
	props.uvl_lMaps [0].v =
	props.uvl_lMaps [1].v =
	props.uvl_lMaps [3].u = fl2f (h);
	props.uvl_lMaps [1].u =
	props.uvl_lMaps [2].u =
	props.uvl_lMaps [2].v =
	props.uvl_lMaps [3].v = fl2f (1-h);
	}
#endif
props.tMap1 = sideP->tmap_num;
props.tMap2 = sideP->tmap_num2;

	//	========== Mark: Here is the change...beginning here: ==========

GetSideVerts (props.vp, props.segNum, props.sideNum);
if (sideP->type == SIDE_IS_QUAD) {
	VmVecSub (&tvec, &viewerEye, gameData.segs.vertices + segP->verts [sideToVerts [props.sideNum][0]]);
	v_dot_n0 = VmVecDot (&tvec, normals);
	if (v_dot_n0 < 0)
		return;
	memcpy (props.uvls, sideP->uvls, sizeof (uvl) * 4);
#if LIGHTMAPS
	if (bDoLightMaps) {
		memcpy (props.uvl_lMaps, uvl_lMaps, sizeof (uvl) * 4);
#if LMAP_LIGHTADJUST
		props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = props.uvls [3].l = F1_0 / 2;
#	endif
		}
#endif
	props.nv = 4;
	props.vNormal = normals [0];
	RenderFace (&props, 0, 0);
#ifdef EDITOR
	CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
	} 
else {
	//	Regardless of whether this side is comprised of a single quad, or two triangles, we need to know one normal, so
	//	deal with it, get the dot product.
	VmVecNormalizedDirQuick (&tvec, &viewerEye, gameData.segs.vertices + segP->verts [sideToVerts [props.sideNum][sideP->type == SIDE_IS_TRI_13]]);
	v_dot_n0 = VmVecDot (&tvec, normals);

	//	========== Mark: The change ends here. ==========

	//	Although this side has been triangulated, because it is not planar, see if it is acceptable
	//	to render it as a single quadrilateral.  This is a function of how far away the viewer is, how non-planar
	//	the face is, how normal to the surfaces the view is.
	//	Now, if both dot products are close to 1.0, then render two triangles as a single quad.
	v_dot_n1 = VmVecDot(&tvec, normals+1);
#if 1
	if (v_dot_n0 < v_dot_n1) {
		min_dot = v_dot_n0;
		max_dot = v_dot_n1;
		}
	else {
		min_dot = v_dot_n1;
		max_dot = v_dot_n0;
		}
	//	Determine whether to detriangulate side: (speed hack, assumes Tulate_min_ratio == F1_0*2, should fixmul(min_dot, Tulate_min_ratio))
	if (gameStates.render.bDetriangulation && ((min_dot+F1_0/256 > max_dot) || ((gameData.objs.viewer->segnum != props.segNum) &&  (min_dot > Tulate_min_dot) && (max_dot < min_dot*2)))) {
		//	The other detriangulation code doesn't deal well with badly non-planar sides.
		fix	n0_dot_n1 = VmVecDot(normals, normals + 1);
		if (n0_dot_n1 < Min_n0_n1_dot)
			goto im_so_ashamed;
		if (min_dot >= 0) {
			memcpy (props.uvls, sideP->uvls, sizeof (props.uvls));
#if LIGHTMAPS
			if (bDoLightMaps) {
				memcpy (props.uvl_lMaps, uvl_lMaps, sizeof (uvl) * 4);
#	if LMAP_LIGHTADJUST
				props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = props.uvls [3].l = F1_0 / 2;
#	endif
				}
#endif
			props.nv = 4;
			props.vNormal = normals [0];
			RenderFace (&props, 0, 0);
#ifdef EDITOR
			CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
			}
		}
	else 
#endif
		{
im_so_ashamed: ;
		props.nv = 3;
		if (sideP->type == SIDE_IS_TRI_02) {
			if (v_dot_n0 >= 0) {
				memcpy (props.uvls, sideP->uvls, sizeof (uvl) * 3);
#if LIGHTMAPS
				if (bDoLightMaps) {
					memcpy (props.uvl_lMaps, uvl_lMaps, sizeof (uvl) * 3);
#	if LMAP_LIGHTADJUST
					props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = F1_0 / 2;
#	endif
					}
#endif
				props.vNormal = normals [0];
				RenderFace (&props, 0, 0);
#ifdef EDITOR
				CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
				}

			if (v_dot_n1 >= 0) {
				props.uvls [0] = sideP->uvls [0];
				memcpy (props.uvls + 1, sideP->uvls + 2, sizeof (uvl) * 2);
#if LIGHTMAPS
				if (bDoLightMaps) {
					props.uvl_lMaps [0] = uvl_lMaps [0];
					memcpy (props.uvl_lMaps + 1, uvl_lMaps + 2, sizeof (uvl) * 2);
#if LMAP_LIGHTADJUST
					props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = F1_0 / 2;
#endif
					}
#endif
				props.vp [1] = props.vp [2];	
				props.vp [2] = props.vp [3];	// want to render from vertices 0, 2, 3 on side
				props.vNormal = normals [1];
				RenderFace (&props, 0, 0);
#ifdef EDITOR
				CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
				}
			}
		else if (sideP->type == SIDE_IS_TRI_13) {
			if (v_dot_n1 >= 0) {
				memcpy (props.uvls + 1, sideP->uvls + 1, sizeof (uvl) * 3);
#if LIGHTMAPS
				if (bDoLightMaps) {
					memcpy (props.uvl_lMaps + 1, uvl_lMaps + 1, sizeof (uvl) * 3);
#	if LMAP_LIGHTADJUST
					props.uvls [1].l = props.uvls [2].l = props.uvls [3].l = F1_0 / 2;
#	endif
					}
#endif
				props.vNormal = normals [0];
				RenderFace (&props, 1, 0);
#ifdef EDITOR
				CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
				}

			if (v_dot_n0 >= 0) {
				props.uvls [0] = sideP->uvls [0];		
				props.uvls [1] = sideP->uvls [1];		
				props.uvls [2] = sideP->uvls [3];
				props.vp [2] = props.vp [3];		// want to render from vertices 0,1,3
#if LIGHTMAPS
				if (bDoLightMaps) {
					props.uvl_lMaps [0] = uvl_lMaps [0];
					props.uvl_lMaps [1] = uvl_lMaps [1];
					props.uvl_lMaps [2] = uvl_lMaps [3];
#if LMAP_LIGHTADJUST
					props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = F1_0 / 2;
#endif
					}
#endif
				props.vNormal = normals [1];
				RenderFace (&props, 0, 0);
#ifdef EDITOR
				CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->tmap_num, sideP->tmap_num2, sideP->uvls);
#endif
				}
			}
		else {
			Error("Illegal side type in RenderSide, type = %i, segment # = %i, side # = %i\n", sideP->type, SEG_IDX (segP), props.sideNum);
			return;
			}
		}
	}
}

//------------------------------------------------------------------------------

#ifdef EDITOR
void render_object_search(object *objP)
{
	int changed=0;

	//note that we draw each pixel object twice, since we cannot control
	//what color the object draws in, so we try color 0, then color 1,
	//in case the object itself is rendering color 0

	GrSetColor(0);
	gr_pixel(_search_x,_search_y);	//set our search pixel to color zero
	RenderObject(objP, 0);
	if (gr_ugpixel(&grdCurCanv->cv_bitmap,_search_x,_search_y) != 0)
		changed=1;

	GrSetColor(1);
	gr_pixel(_search_x,_search_y);	//set our search pixel to color zero
	RenderObject(objP, 0);
	if (gr_ugpixel(&grdCurCanv->cv_bitmap,_search_x,_search_y) != 1)
		changed=1;

	if (changed) {
		if (objP->segnum != -1)
			Cursegp = gameData.segs.segments+objP->segnum;
		found_seg = -(OBJ_IDX (objP)+1);
	}
}
#endif

//------------------------------------------------------------------------------

extern ubyte DemoDoingRight,DemoDoingLeft;
void DoRenderObject(int objnum, int nWindowNum)
{
#ifdef EDITOR
	int save_3d_outline=0;
#endif
	object *objP = gameData.objs.objects+objnum, *hObj;
	window_rendered_data *wrd = Window_rendered_data + nWindowNum;
	int count = 0;
	int n;

	if (!(IsMultiGame || gameOpts->render.bObjects))
		return;
#if 0
	if (obj == gameData.objs.console)
		return;
#endif
#if 0 //def OGL_ZBUF
if (!gameOpts->legacy.bZBuf) {
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
#endif
	Assert(objnum < MAX_OBJECTS);
#ifdef _DEBUG
	if (bObjectRendered [objnum]) {		//already rendered this...
		Int3();		//get Matt!!!
		return;
	}
	bObjectRendered [objnum] = 1;
#endif
   if (gameData.demo.nState==ND_STATE_PLAYBACK) {
	  if ((DemoDoingLeft==6 || DemoDoingRight==6) && objP->type==OBJ_PLAYER) {
			// A nice fat hack: keeps the player ship from showing up in the
			// small extra view when guiding a missile in the big window
#if TRACE	
			//con_printf (CON_DEBUG,"Returning from RenderObject prematurely...\n");
#endif
  			return; 
			}
		}
	//	Added by MK on 09/07/94 (at about 5:28 pm, CDT, on a beautiful, sunny late summer day!) so
	//	that the guided missile system will know what gameData.objs.objects to look at.
	//	I didn't know we had guided missiles before the release of D1. --MK
	if ((objP->type == OBJ_ROBOT) || (objP->type == OBJ_PLAYER)) {
		//Assert(Window_rendered_data [nWindowNum].rendered_objects < MAX_RENDERED_OBJECTS);
		//	This peculiar piece of code makes us keep track of the most recently rendered gameData.objs.objects, which
		//	are probably the higher priority gameData.objs.objects, without overflowing the buffer
		if (wrd->num_objects >= MAX_RENDERED_OBJECTS) {
			Int3();
			wrd->num_objects /= 2;
		}
		wrd->rendered_objects [wrd->num_objects++] = objnum;
	}
	if ((count++ > MAX_OBJECTS) || (objP->next == objnum)) {
		Int3();					// infinite loop detected
		objP->next = -1;		// won't this clean things up?
		return;					// get out of this infinite loop!
	}
	//g3_draw_object(objP->class_id,&objP->pos,&objP->orient,objP->size);
	//check for editor object
#ifdef EDITOR
	if (gameStates.app.nFunctionMode==FMODE_EDITOR && objnum==Cur_object_index) {
		save_3d_outline = g3d_interp_outline;
		g3d_interp_outline=1;
	}
	if (bSearchMode)
		render_object_search(objP);
	else
#endif
		//NOTE LINK TO ABOVE
		RenderObject(objP, nWindowNum);

	for (n=objP->attached_obj; n != -1; n = hObj->ctype.expl_info.next_attach) {
		hObj = gameData.objs.objects + n;
		Assert(hObj->type == OBJ_FIREBALL);
		Assert(hObj->control_type == CT_EXPLOSION);
		Assert(hObj->flags & OF_ATTACHED);
		RenderObject(hObj, nWindowNum);
	}
#ifdef EDITOR
	if (gameStates.app.nFunctionMode==FMODE_EDITOR && objnum==Cur_object_index)
		g3d_interp_outline = save_3d_outline;
#endif
}

// -----------------------------------------------------------------------------------

#ifndef NDEBUG
int	draw_boxes=0;
int bWindowCheck=1,draw_edges=0,new_seg_sorting=1,pre_draw_segs=0;
int no_migrate_segs=1,migrate_objects=1,behind_check=1;
int check_bWindowCheck=0;
#else
#define draw_boxes			0
#define bWindowCheck			1
#define draw_edges			0
#define new_seg_sorting		1
#define pre_draw_segs		0
#define no_migrate_segs		1
#define migrate_objects		1
#define behind_check			1
#define check_bWindowCheck	0
#endif

// -----------------------------------------------------------------------------------
//increment counter for checking if points rotated
//This must be called at the start of the frame if RotateList() will be used
void RenderStartFrame()
{
if (!++nRLFrameCount) {		//wrap!
	memset(nRotatedLast,0,sizeof(nRotatedLast));		//clear all to zero
	nRLFrameCount=1;											//and set this frame to 1
	}
}

// -----------------------------------------------------------------------------------
//Given a list of point numbers, rotate any that haven't been rotated this frame
//cc.and and cc.or will contain the position/orientation of the face that is determined 
//by the vertices passed relative to the viewer
g3s_codes RotateList (int nv, short *pointNumList)
{
	int i,pnum;
	g3s_point *pnt;
	g3s_codes cc;

cc.and = 0xff;  
cc.or = 0;
for (i = 0; i < nv; i++) {
	pnum = pointNumList [i];
	pnt = gameData.segs.points + pnum;
	if (nRotatedLast [pnum] != nRLFrameCount) {
		G3TransformAndEncodePoint (pnt, gameData.segs.vertices + pnum);
		if (!gameStates.ogl.bUseTransform) {
			gameData.segs.fVertices [pnum].x = ((float) pnt->p3_vec.x) / 65536.0f;
			gameData.segs.fVertices [pnum].y = ((float) pnt->p3_vec.y) / 65536.0f;
			gameData.segs.fVertices [pnum].z = -((float) pnt->p3_vec.z) / 65536.0f;
			}
		nRotatedLast [pnum] = nRLFrameCount;
		}
	cc.and &= pnt->p3_codes;
	cc.or |= pnt->p3_codes;
	pnt->p3_index = pnum;
	}
return cc;
}

// -----------------------------------------------------------------------------------
//Given a lit of point numbers, project any that haven't been projected
void ProjectList (int nv,short *pointNumList)
{
	int i,pnum;

for (i=0;i<nv;i++) {
	pnum = pointNumList [i];
	if (!(gameData.segs.points [pnum].p3_flags & PF_PROJECTED))
		G3ProjectPoint (gameData.segs.points + pnum);
	}
}

// -----------------------------------------------------------------------------------

void SortSidesByDist (double *sideDists, char *sideNums, char left, char right)
{
	char		l = left;
	char		r = right;
	char		h, m = (l + r) / 2;
	double	hd, md = sideDists [m];

do {
	while (sideDists [l] < md)
		l++;
	while (sideDists [r] > md)
		r--;
	if (l <= r) {
		if (l < r) {
			h = sideNums [l];
			sideNums [l] = sideNums [r];
			sideNums [r] = h;
			hd = sideDists [l];
			sideDists [l] = sideDists [r];
			sideDists [r] = hd;
			}
		}
	++l;
	--r;
	} while (l <= r);
if (right > l)
   SortSidesByDist (sideDists, sideNums, l, right);
if (r > left)
   SortSidesByDist (sideDists, sideNums, left, r);
}

// -----------------------------------------------------------------------------------

void RenderSegment(short segnum, int nWindowNum)
{
	segment		*seg = gameData.segs.segments+segnum;
	g3s_codes 	cc;
	short			sn;

Assert(segnum!=-1 && segnum <= gameData.segs.nLastSegment);
if ((segnum < 0) || (segnum > gameData.segs.nLastSegment))
	return;
OglSetupTransform ();
cc = RotateList (8, seg->verts);
gameData.render.pVerts = gameData.segs.fVertices;
//	return;
if (!cc.and) {		//all off screen?
#ifdef OGL_ZBUF
	if (gameOpts->legacy.bZBuf)
#endif
	if (!gameStates.render.cameras.bActive && (gameData.objs.viewer->type!=OBJ_ROBOT))
  	   bAutomapVisited [segnum]=1;
	if (gameStates.render.bQueryOcclusion) {
			short		sideVerts [4];
			double	sideDists [6];
			char		sideNums [6];
			int		i; 
			double	d, dMin, dx, dy, dz;
			object	*objP = gameData.objs.objects + gameData.multi.players [gameData.multi.nLocalPlayer].objnum;

		for (sn = 0; sn < MAX_SIDES_PER_SEGMENT; sn++) {
			sideNums [sn] = (char) sn;
			GetSideVerts (sideVerts, segnum, sn);
			dMin = 1e300;
			for (i = 0; i < 4; i++) {
				dx = objP->pos.x - gameData.segs.vertices [sideVerts [i]].x;
				dy = objP->pos.y - gameData.segs.vertices [sideVerts [i]].y;
				dz = objP->pos.z - gameData.segs.vertices [sideVerts [i]].z;
				d = dx * dx + dy * dy + dz * dz;
				if (dMin > d)
					dMin = d;
				}
			sideDists [sn] = dMin;
			}
		SortSidesByDist (sideDists, sideNums, 0, 5);
		for (sn = 0; sn < MAX_SIDES_PER_SEGMENT; sn++)
			RenderSide (seg, sideNums [sn]);
		}
	else
		for (sn = 0; sn < MAX_SIDES_PER_SEGMENT; sn++) {
			RenderSide (seg, sn);
		}
	}
OglResetTransform ();
}

#define CROSS_WIDTH  i2f(8)
#define CROSS_HEIGHT i2f(8)

#ifndef NDEBUG

//------------------------------------------------------------------------------
//draw outline for curside
void OutlineSegSide(segment *seg,int _side,int edge,int vert)
{
	g3s_codes cc;

cc=RotateList(8,seg->verts);
if (! cc.and) {		//all off screen?
	g3s_point *pnt;
	side *s = seg->sides+_side;
	//render curedge of curside of curseg in green
	GrSetColorRGB (0, 255, 0, 255);
	G3DrawLine(gameData.segs.points + seg->verts [sideToVerts [_side][edge]],
						gameData.segs.points + seg->verts [sideToVerts [_side][(edge+1)%4]]);
	//draw a little cross at the current vert
	pnt = gameData.segs.points + seg->verts [sideToVerts [_side][vert]];
	G3ProjectPoint(pnt);		//make sure projected
	gr_line(pnt->p3_sx-CROSS_WIDTH,pnt->p3_sy,pnt->p3_sx,pnt->p3_sy-CROSS_HEIGHT);
	gr_line(pnt->p3_sx,pnt->p3_sy-CROSS_HEIGHT,pnt->p3_sx+CROSS_WIDTH,pnt->p3_sy);
	gr_line(pnt->p3_sx+CROSS_WIDTH,pnt->p3_sy,pnt->p3_sx,pnt->p3_sy+CROSS_HEIGHT);
	gr_line(pnt->p3_sx,pnt->p3_sy+CROSS_HEIGHT,pnt->p3_sx-CROSS_WIDTH,pnt->p3_sy);
	}
}

#endif

#if 0		//this stuff could probably just be deleted

//------------------------------------------------------------------------------
#define DEFAULT_PERSPECTIVE_DEPTH 6

int nPerspectiveDepth=DEFAULT_PERSPECTIVE_DEPTH;	//how many levels deep to render in perspective

int IncPerspectiveDepth(void)
{
return ++nPerspectiveDepth;
}

//------------------------------------------------------------------------------

int DecPerspectiveDepth(void)
{
return nPerspectiveDepth==1?nPerspectiveDepth:--nPerspectiveDepth;
}

//------------------------------------------------------------------------------

int ResetPerspectiveDepth(void)
{
return nPerspectiveDepth = DEFAULT_PERSPECTIVE_DEPTH;
}
#endif

//------------------------------------------------------------------------------

typedef struct window {
	short left,top,right,bot;
} window;

ubyte code_window_point(fix x,fix y,window *w)
{
	ubyte code=0;

	if (x <= w->left)  code |= 1;
	if (x >= w->right) code |= 2;

	if (y <= w->top) code |= 4;
	if (y >= w->bot) code |= 8;

	return code;
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
void draw_window_box(unsigned int color,short left,short top,short right,short bot)
{
	short l,t,r,b;

	GrSetColorRGBi (color);

	l=left; t=top; r=right; b=bot;

	if ( r<0 || b<0 || l>=grdCurCanv->cv_bitmap.bm_props.w || (t>=grdCurCanv->cv_bitmap.bm_props.h && b>=grdCurCanv->cv_bitmap.bm_props.h))
		return;

	if (l<0) l=0;
	if (t<0) t=0;
	if (r>=grdCurCanv->cv_bitmap.bm_props.w) r=grdCurCanv->cv_bitmap.bm_props.w-1;
	if (b>=grdCurCanv->cv_bitmap.bm_props.h) b=grdCurCanv->cv_bitmap.bm_props.h-1;

	gr_line(i2f(l),i2f(t),i2f(r),i2f(t));
	gr_line(i2f(r),i2f(t),i2f(r),i2f(b));
	gr_line(i2f(r),i2f(b),i2f(l),i2f(b));
	gr_line(i2f(l),i2f(b),i2f(l),i2f(t));

}
#endif

//------------------------------------------------------------------------------

int matt_find_connect_side(int seg0,int seg1);

#ifndef NDEBUG
char visited2 [MAX_SEGMENTS];
#endif

typedef struct tSegZRef {
	fix	z;
	fix	d;
	short	segnum;
} tSegZRef;

static tSegZRef segZRef [MAX_SEGMENTS];

unsigned char bVisited [MAX_SEGMENTS];
char nVisited;
short nRenderList [MAX_RENDER_SEGS];
short Seg_depth [MAX_RENDER_SEGS];		//depth for each seg in nRenderList
ubyte processed [MAX_RENDER_SEGS];		//whether each entry has been processed
int	lcnt_save,scnt_save;

#define VISITED(_ch)	(bVisited [_ch] == nVisited)
#define VISIT(_ch) (bVisited [_ch] = nVisited)
//@@short *persp_ptr;
short render_pos [MAX_SEGMENTS];	//where in render_list does this segment appear?
//ubyte no_render_flag [MAX_RENDER_SEGS];
window render_windows [MAX_RENDER_SEGS];

short render_obj_list [MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS][OBJS_PER_SEG];

//for gameData.objs.objects

//Given two sides of segment, tell the two verts which form the 
//edge between them
short edgeBetweenTwoSides [6][6][2] = {
	{ {-1,-1}, {3,7}, {-1,-1}, {2,6}, {6,7}, {2,3} },
	{ {3,7}, {-1,-1}, {0,4}, {-1,-1}, {4,7}, {0,3} },
	{ {-1,-1}, {0,4}, {-1,-1}, {1,5}, {4,5}, {0,1} },
	{ {2,6}, {-1,-1}, {1,5}, {-1,-1}, {5,6}, {1,2} },
	{ {6,7}, {4,7}, {4,5}, {5,6}, {-1,-1}, {-1,-1} },
	{ {2,3}, {0,3}, {0,1}, {1,2}, {-1,-1}, {-1,-1} }
};

//given an edge specified by two verts, give the two sides on that edge
int edgeToSides [8][8][2] = {
	{ {-1,-1}, {2,5}, {-1,-1}, {1,5}, {1,2}, {-1,-1}, {-1,-1}, {-1,-1} },
	{ {2,5}, {-1,-1}, {3,5}, {-1,-1}, {-1,-1}, {2,3}, {-1,-1}, {-1,-1} },
	{ {-1,-1}, {3,5}, {-1,-1}, {0,5}, {-1,-1}, {-1,-1}, {0,3}, {-1,-1} },
	{ {1,5}, {-1,-1}, {0,5}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {0,1} },
	{ {1,2}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {2,4}, {-1,-1}, {1,4} },
	{ {-1,-1}, {2,3}, {-1,-1}, {-1,-1}, {2,4}, {-1,-1}, {3,4}, {-1,-1} },
	{ {-1,-1}, {-1,-1}, {0,3}, {-1,-1}, {-1,-1}, {3,4}, {-1,-1}, {0,4} },
	{ {-1,-1}, {-1,-1}, {-1,-1}, {0,1}, {1,4}, {-1,-1}, {0,4}, {-1,-1} },
};

//@@//perform simple check on tables
//@@check_check()
//@@{
//@@	int i,j;
//@@
//@@	for (i=0;i<8;i++)
//@@		for (j=0;j<8;j++)
//@@			Assert(edgeToSides [i][j][0] == edgeToSides [j][i][0] && 
//@@					edgeToSides [i][j][1] == edgeToSides [j][i][1]);
//@@
//@@	for (i=0;i<6;i++)
//@@		for (j=0;j<6;j++)
//@@			Assert(edgeBetweenTwoSides [i][j][0] == edgeBetweenTwoSides [j][i][0] && 
//@@					edgeBetweenTwoSides [i][j][1] == edgeBetweenTwoSides [j][i][1]);
//@@
//@@
//@@}


//------------------------------------------------------------------------------
//given an edge and one side adjacent to that edge, return the other adjacent side

int FindOtherSideOnEdge (segment *seg, short *verts, int notside)
{
	int	i;
	int	vv0 = -1, vv1 = -1;
	int	side0, side1;
	int	*eptr;
	int	v0,v1;
	short	*vp;

//@@	check_check();

v0 = verts [0];
v1 = verts [1];
vp = seg->verts;
for (i = 0; i < 8; i++) {
	int svv = *vp++;	// seg->verts [i];
	if (vv0 == -1 && svv == v0) {
		vv0 = i;
		if (vv1 != -1)
			break;
		}
	if (vv1 == -1 && svv == v1) {
		vv1 = i;
		if (vv0 != -1)
			break;
		}
	}
Assert(vv0 != -1 && vv1 != -1);

eptr = edgeToSides [vv0][vv1];
side0 = eptr [0];
side1 = eptr [1];
Assert(side0 != -1 && side1 != -1);
if (side0 != notside) {
	Assert(side1==notside);
	return side0;
}
Assert(side0==notside);
return side1;
}

//------------------------------------------------------------------------------
//find the two segments that join a given seg through two sides, and
//the sides of those segments the abut. 

typedef struct tSideNormData {
	vms_vector	n [2];
	vms_vector	*p;
	short			t;
} tSideNormData;

int FindAdjacentSideNorms (segment *seg, short s0, short s1, tSideNormData *s)
{
	segment	*seg0, *seg1;
	side		*side0, *side1;
	short		edge_verts [2];
	int		notside0, notside1;
	int		edgeside0, edgeside1;

	Assert(s0 != -1 && s1 != -1);

	seg0 = gameData.segs.segments + seg->children [s0];
	seg1 = gameData.segs.segments + seg->children [s1];

	edge_verts [0] = seg->verts [edgeBetweenTwoSides [s0][s1][0]];
	edge_verts [1] = seg->verts [edgeBetweenTwoSides [s0][s1][1]];

	Assert(edge_verts [0] != -1 && edge_verts [1] != -1);

	notside0 = FindConnectedSide (seg, seg0);
	Assert (notside0 != -1);
	notside1 = FindConnectedSide (seg, seg1);
	Assert (notside1 != -1);

	edgeside0 = FindOtherSideOnEdge (seg0, edge_verts, notside0);
	edgeside1 = FindOtherSideOnEdge (seg1, edge_verts, notside1);

	//deal with the case where an edge is shared by more than two segments

//@@	if (IS_CHILD(seg0->children [edgeside0])) {
//@@		segment *seg00;
//@@		int notside00;
//@@
//@@		seg00 = &gameData.segs.segments [seg0->children [edgeside0]];
//@@
//@@		if (seg00 != seg1) {
//@@
//@@			notside00 = FindConnectedSide(seg0,seg00);
//@@			Assert(notside00 != -1);
//@@
//@@			edgeside0 = FindOtherSideOnEdge(seg00,edge_verts,notside00);
//@@	 		seg0 = seg00;
//@@		}
//@@
//@@	}
//@@
//@@	if (IS_CHILD(seg1->children [edgeside1])) {
//@@		segment *seg11;
//@@		int notside11;
//@@
//@@		seg11 = &gameData.segs.segments [seg1->children [edgeside1]];
//@@
//@@		if (seg11 != seg0) {
//@@			notside11 = FindConnectedSide(seg1,seg11);
//@@			Assert(notside11 != -1);
//@@
//@@			edgeside1 = FindOtherSideOnEdge(seg11,edge_verts,notside11);
//@@ 			seg1 = seg11;
//@@		}
//@@	}

//	if ( IS_CHILD(seg0->children [edgeside0]) ||
//		  IS_CHILD(seg1->children [edgeside1])) 
//		return 0;

side0 = seg0->sides + edgeside0;
side1 = seg1->sides + edgeside1;
#ifdef COMPACT_SEGS
GetSideNormals (seg0, edgeside0, s [0].n, s [0].n + 1);
GetSideNormals (seg1, edgeside1, s [1].n, s [1].n + 1);
#else 
memcpy (s [0].n, side0->normals, 2 * sizeof (vms_vector));
memcpy (s [1].n, side1->normals, 2 * sizeof (vms_vector));
#endif
s [0].p = gameData.segs.vertices + seg0->verts [sideToVerts [edgeside0][(s [0].t = side0->type) == 3]];
s [1].p = gameData.segs.vertices + seg1->verts [sideToVerts [edgeside1][(s [1].t = side1->type) == 3]];
return 1;
}

//------------------------------------------------------------------------------
//see if the order matters for these two children.
//returns 0 if order doesn't matter, 1 if c0 before c1, -1 if c1 before c0
static int CompareChildren (segment *seg,short c0,short c1)
{
	tSideNormData	s [2];
	vms_vector		temp;
	fix				d0, d1;

if (sideOpposite [c0] == c1) 
	return 0;

Assert(c0 != -1 && c1 != -1);

//find normals of adjoining sides
FindAdjacentSideNorms (seg, c0, c1, s);

VmVecSub (&temp, &viewerEye, s [0].p);
d0 = VmVecDot (s [0].n, &temp);
if (s [0].t != 1)	// triangularized face -> 2 different normals
	d0 |= VmVecDot (s [0].n + 1, &temp);	// we only need the sign, so a bitwise or does the trick
VmVecSub (&temp, &viewerEye, s [1].p);
d1 = VmVecDot (s [1].n, &temp);
if (s [1].t != 1) 
	d1 |= VmVecDot (s [1].n + 1, &temp);

#if 0
d0 = (d0_0 < 0) || (d0_1 < 0) ? -1 : 1;
d1 = (d1_0 < 0) || (d1_1 < 0) ? -1 : 1;
#endif
if ((d0 & d1) < 0)	// only < 0 if both negative due to bitwise and
	return 0;
if (d0 < 0)
	return 1;
 if (d1 < 0)
	return -1;
return 0;
}

//------------------------------------------------------------------------------

int QuicksortSegChildren (segment *seg, short left, short right, short *child_list)
{
	short	h,
			l = left,
			r = right,
			m = (l + r) / 2,
			median = child_list [m],
			bSwap = 0;

do {
	while ((l < m) && CompareChildren (seg, child_list [l], median) >= 0)
		l++;
	while ((r > m) && CompareChildren (seg, child_list [r], median) <= 0)
		r--;
	if (l <= r) {
		if (l < r) {
			h = child_list [l];
			child_list [l] = child_list [r];
			child_list [r] = h;
			bSwap = 1;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	bSwap |= QuicksortSegChildren (seg, l, right, child_list);
if (left < r)
	bSwap |= QuicksortSegChildren (seg, left, r, child_list);
return bSwap;
}

//------------------------------------------------------------------------------

int ssc_total=0,ssc_swaps=0;

//short the children of segment to render in the correct order
//returns non-zero if swaps were made
int SortSegChildren(segment *seg,int n_children,short *child_list)
{
#if 1
if (n_children < 2) 
	return 0;
return QuicksortSegChildren (seg, (short) 0, (short) (n_children - 1), child_list);
#else
	int i,j;
	int r;
	int made_swaps,count;

if (n_children < 2) 
	return 0;
ssc_total++;
	//for each child,  compare with other children and see if order matters
	//if order matters, fix if wrong

count = 0;

do {
	made_swaps = 0;

	for (i=0;i<n_children-1;i++)
		for (j=i+1;child_list [i]!=-1 && j<n_children;j++)
			if (child_list [j]!=-1) {
				r = CompareChildren(seg,child_list [i],child_list [j]);

				if (r == 1) {
					int temp = child_list [i];
					child_list [i] = child_list [j];
					child_list [j] = temp;
					made_swaps=1;
				}
			}

} while (made_swaps && ++count<n_children);
if (count)
	ssc_swaps++;
return count;
#endif
}

//------------------------------------------------------------------------------

void AddObjectToSegList(int objnum,int listnum)
{
	int i,checkn,marker;

	checkn = listnum;
	//first, find a slot
	do {
		for (i=0;render_obj_list [checkn][i] >= 0;i++);
		Assert(i < OBJS_PER_SEG);
		marker = render_obj_list [checkn][i];
		if (marker != -1) {
			checkn = -marker;
			//Assert(checkn < MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS);
			if (checkn >= MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS) {
				Int3();
				return;
			}
		}

	} while (marker != -1);


	//now we have found a slot.  put object in it

	if (i != OBJS_PER_SEG-1) {

		render_obj_list [checkn][i] = objnum;
		render_obj_list [checkn][i+1] = -1;
	}
	else {				//chain to additional list
		int lookn;

		//find an available sublist

		for (lookn=MAX_RENDER_SEGS;render_obj_list [lookn][0]!=-1 && lookn<MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS;lookn++);

		//Assert(lookn<MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS);
		if (lookn >= MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS) {
			Int3();
			return;
		}

		render_obj_list [checkn][i] = -lookn;
		render_obj_list [lookn][0] = objnum;
		render_obj_list [lookn][1] = -1;

	}

}
#ifdef __sun__
// the following is a drop-in replacement for the broken libc qsort on solaris
// taken from http://www.snippets.org/snippets/portable/RG_QSORT+C.php3

#define qsort qsort_dropin

/******************************************************************/
/* qsort.c  --  Non-Recursive ANSI Quicksort function             */
/* Public domain by Raymond Gardner, Englewood CO  February 1991  */
/******************************************************************/
#define  COMP(a, b)  ((*comp)((void *)(a), (void *)(b)))
#define  T 7 // subfiles of <= T elements will be insertion sorteded (T >= 3)
#define  SWAP(a, b)  (swap_bytes((char *)(a), (char *)(b), size))

static void swap_bytes(char *a, char *b, size_t nbytes)
{
   char tmp;
   do {
      tmp = *a; *a++ = *b; *b++ = tmp;
   } while ( --nbytes );
}

void qsort(void *basep, size_t nelems, size_t size,
                            int (*comp)(const void *, const void *))
{
   char *stack [40], **sp;       /* stack and stack pointer        */
   char *i, *j, *limit;         /* scan and limit pointers        */
   size_t thresh;               /* size of T elements in bytes    */
   char *base;                  /* base pointer as char *         */
   base = (char *)basep;        /* set up char * base pointer     */
   thresh = T * size;           /* init threshold                 */
   sp = stack;                  /* init stack pointer             */
   limit = base + nelems * size;/* pointer past end of array      */
   for ( ;; ) {                 /* repeat until break...          */
      if ( limit - base > thresh ) {  /* if more than T elements  */
                                      /*   swap base with middle  */
         SWAP((((limit-base)/size)/2)*size+base, base);
         i = base + size;             /* i scans left to right    */
         j = limit - size;            /* j scans right to left    */
         if ( COMP(i, j) > 0 )        /* Sedgewick's              */
            SWAP(i, j);               /*    three-element sort    */
         if ( COMP(base, j) > 0 )     /*        sets things up    */
            SWAP(base, j);            /*            so that       */
         if ( COMP(i, base) > 0 )     /*      *i <= *base <= *j   */
            SWAP(i, base);            /* *base is pivot element   */
         for ( ;; ) {                 /* loop until break         */
            do                        /* move i right             */
               i += size;             /*        until *i >= pivot */
            while ( COMP(i, base) < 0 );
            do                        /* move j left              */
               j -= size;             /*        until *j <= pivot */
            while ( COMP(j, base) > 0 );
            if ( i > j )              /* if pointers crossed      */
               break;                 /*     break loop           */
            SWAP(i, j);       /* else swap elements, keep scanning*/
         }
         SWAP(base, j);         /* move pivot into correct place  */
         if ( j - base > limit - i ) {  /* if left subfile larger */
            sp [0] = base;             /* stack left subfile base  */
            sp [1] = j;                /*    and limit             */
            base = i;                 /* sort the right subfile   */
         } else {                     /* else right subfile larger*/
            sp [0] = i;                /* stack right subfile base */
            sp [1] = limit;            /*    and limit             */
            limit = j;                /* sort the left subfile    */
         }
         sp += 2;                     /* increment stack pointer  */
      } else {      /* else subfile is small, use insertion sort  */
         for ( j = base, i = j+size; i < limit; j = i, i += size )
            for ( ; COMP(j, j+size) > 0; j -= size ) {
               SWAP(j, j+size);
               if ( j == base )
                  break;
            }
         if ( sp != stack ) {         /* if any entries on stack  */
            sp -= 2;                  /* pop the base and limit   */
            base = sp [0];
            limit = sp [1];
         } else                       /* else stack empty, done   */
            break;
      }
   }
}
#endif // __sun__ qsort drop-in replacement

//------------------------------------------------------------------------------

#define SORT_LIST_SIZE 100

typedef struct sort_item {
	int objnum;
	fix dist;
} sort_item;

sort_item sort_list [SORT_LIST_SIZE];
int n_sort_items;

//compare function for object sort. 
int _CDECL_ sort_func(const sort_item *a,const sort_item *b)
{
	fix delta_dist;
	object *objAP,*objBP;

	delta_dist = a->dist - b->dist;

	objAP = gameData.objs.objects + a->objnum;
	objBP = gameData.objs.objects + b->objnum;

	if (abs(delta_dist) < (objAP->size + objBP->size)) {		//same position

		//these two gameData.objs.objects are in the same position.  see if one is a fireball
		//or laser or something that should plot on top.  Don't do this for
		//the afterburner blobs, though.

		if (objAP->type == OBJ_WEAPON || (objAP->type == OBJ_FIREBALL && objAP->id != VCLIP_AFTERBURNER_BLOB))
			if (!(objBP->type == OBJ_WEAPON || objBP->type == OBJ_FIREBALL))
				return -1;	//a is weapon, b is not, so say a is closer
			else;				//both are weapons 
		else
			if (objBP->type == OBJ_WEAPON || (objBP->type == OBJ_FIREBALL && objBP->id != VCLIP_AFTERBURNER_BLOB))
				return 1;	//b is weapon, a is not, so say a is farther

		//no special case, fall through to normal return
	}

	return delta_dist;	//return distance
}

//------------------------------------------------------------------------------

void BuildObjectLists(int n_segs)
{
	int nn;

	for (nn=0;nn<MAX_RENDER_SEGS+N_EXTRA_OBJ_LISTS;nn++)
		render_obj_list [nn][0] = -1;

	for (nn=0;nn<n_segs;nn++) {
		short segnum;
		segnum = nRenderList [nn];
		if (segnum != -1) {
			int objnum;
			object *objP;
			for (objnum = gameData.segs.segments [segnum].objects; objnum != -1; objnum = objP->next) {
				int new_segnum,did_migrate,list_pos;
				objP = gameData.objs.objects+objnum;

				Assert( objP->segnum == segnum );

				if (objP->flags & OF_ATTACHED)
					continue;		//ignore this object

				new_segnum = segnum;
				list_pos = nn;

				if (objP->type != OBJ_CNTRLCEN && !(objP->type==OBJ_ROBOT && objP->id==65))		//don't migrate controlcen
				do {
					segmasks m;
					did_migrate = 0;
					m = GetSegMasks(&objP->pos,new_segnum,objP->size);
					if (m.sidemask) {
						short sn,sf;
						for (sn=0,sf=1;sn<6;sn++,sf<<=1)
							if (m.sidemask & sf) {
								segment *seg = gameData.segs.segments+new_segnum;
		
								if (WALL_IS_DOORWAY (seg, sn, NULL) & WID_FLY_FLAG) {		//can explosion migrate through
									int child = seg->children [sn];
									int checknp;
		
									for (checknp=list_pos;checknp--;)
										if (nRenderList [checknp] == child) {
											new_segnum = child;
											list_pos = checknp;
											did_migrate = 1;
										}
								}
							}
					}
	
				} while (0);	//while (did_migrate);
				AddObjectToSegList(objnum,list_pos);
			}
		}
	}

	//now that there's a list for each segment, sort the items in those lists
	for (nn=0;nn<n_segs;nn++) {
		int segnum;

		segnum = nRenderList [nn];

		if (segnum != -1) {
			int t,lookn,i,n;

			//first count the number of gameData.objs.objects & copy into sort list

			lookn = nn;
			i = n_sort_items = 0;
			while ((t=render_obj_list [lookn][i++])!=-1)
				if (t<0)
					{lookn = -t; i=0;}
				else
					if (n_sort_items < SORT_LIST_SIZE-1) {		//add if room
						sort_list [n_sort_items].objnum = t;
						//NOTE: maybe use depth, not dist - quicker computation
						sort_list [n_sort_items].dist = VmVecDistQuick(&gameData.objs.objects [t].pos,&viewerEye);
						n_sort_items++;
					}
					else {			//no room for object
						int ii;

						#ifndef NDEBUG
						FILE *tfile=fopen("sortlist.out","wt");

						//I find this strange, so I'm going to write out
						//some information to look at later
						if (tfile) {
							for (ii=0;ii<SORT_LIST_SIZE;ii++) {
								int objnum = sort_list [ii].objnum;

								fprintf(tfile,"Obj %3d  Type = %2d  Id = %2d  Dist = %08x  Segnum = %3d\n",
									objnum,gameData.objs.objects [objnum].type,gameData.objs.objects [objnum].id,sort_list [ii].dist,gameData.objs.objects [objnum].segnum);
							}
							fclose(tfile);
						}
						#endif

						Int3();	//Get Matt!!!

						//Now try to find a place for this object by getting rid
						//of an object we don't care about

						for (ii=0;ii<SORT_LIST_SIZE;ii++) {
							int objnum = sort_list [ii].objnum;
							object *objP = &gameData.objs.objects [objnum];
							int type = objP->type;

							//replace debris & fireballs
							if (type == OBJ_DEBRIS || type == OBJ_FIREBALL) {
								fix dist = VmVecDistQuick(&gameData.objs.objects [t].pos,&viewerEye);

								//don't replace same kind of object unless new 
								//one is closer

								if (gameData.objs.objects [t].type != type || dist < sort_list [ii].dist) {
									sort_list [ii].objnum = t;
									sort_list [ii].dist = dist;
									break;
								}
							}
						}
						Int3();	//still couldn't find a slot
					}


			//now call qsort
		#if defined(__WATCOMC__)
			qsort(sort_list,n_sort_items,sizeof(*sort_list),
				   sort_func);
		#else
			qsort(sort_list,n_sort_items,sizeof(*sort_list),
					(int (_CDECL_ *)(const void*,const void*))sort_func);
		#endif	

			//now copy back into list

			lookn = nn;
			i = 0;
			n = n_sort_items;
			while ((t=render_obj_list [lookn][i])!=-1 && n>0)
				if (t<0)
					{lookn = -t; i=0;}
				else
					render_obj_list [lookn][i++] = sort_list [--n].objnum;
			render_obj_list [lookn][i] = -1;	//mark (possibly new) end
		}
	}
}

//------------------------------------------------------------------------------

#define	PP_DELTAZ	-i2f(30)
#define	PP_DELTAY	i2f (10)

typedef struct tMovementPath {
	vms_vector	pos, basePos;
	vms_matrix	orient;
} tMovementPath;

#define MAX_PATH_POS		20

int nPathStart = 0;
int nPathEnd = 0;
time_t mpUpdate;

tMovementPath movementPath [MAX_PATH_POS];
tMovementPath *pPathPos = NULL;

//------------------------------------------------------------------------------

void ResetMovementPath (void)
{
nPathStart =
nPathEnd = 0;
mpUpdate = -1;
}

//------------------------------------------------------------------------------

void SetPathPoint (void)
{
	time_t	t = SDL_GetTicks () - mpUpdate;

if ((mpUpdate < 0) || (t >= 1000 / 40)) {
	mpUpdate = t;
//	h = nPathEnd;
	nPathEnd = (nPathEnd + 1) % MAX_PATH_POS;
	movementPath [nPathEnd].basePos = gameData.objs.viewer->pos;
	movementPath [nPathEnd].pos = gameData.objs.viewer->pos;
	movementPath [nPathEnd].orient = gameData.objs.viewer->orient;
	VmVecScaleInc (&movementPath [nPathEnd].pos, &gameData.objs.viewer->orient.fvec, 0);
	VmVecScaleInc (&movementPath [nPathEnd].pos, &gameData.objs.viewer->orient.uvec, 0);
//	if (!memcmp (movementPath + h, movementPath + nPathEnd, sizeof (tMovementPath)))
//		nPathEnd = h;
//	else 
	if (nPathEnd == nPathStart)
		nPathStart = (nPathStart + 1) % MAX_PATH_POS;
	}
}

//------------------------------------------------------------------------------

void GetViewPoint (void)
{
	vms_vector		*p = &movementPath [nPathEnd].pos;
	int				i;

if (nPathStart == nPathEnd) {
	VmVecScaleInc (&viewerEye, &gameData.objs.viewer->orient.fvec, PP_DELTAZ);
#if 1//ndef _DEBUG
	VmVecScaleInc (&viewerEye, &gameData.objs.viewer->orient.uvec, PP_DELTAY);
#endif
	pPathPos = NULL;
	}
else {
//	for (i = j = nPathStart; i != nPathEnd; j = i, i = (i + 1) % MAX_PATH_POS)
	i = nPathEnd;
	do {
		if (!i)
			i = MAX_PATH_POS;
		i--;
		if (VmVecDist (&movementPath [i].pos, p) >= i2f (15))
			break;
		}
	while (i != nPathStart);
/*
	if (i != nPathEnd)
		nPathStart = i = j;
	else
		i = nPathEnd;
*/
	pPathPos = movementPath + i;
	viewerEye = pPathPos->pos;
#if 0//def _DEBUG
	VmVecScaleInc (&viewerEye, &pPathPos->orient.fvec, PP_DELTAZ / 3);
#else
	VmVecScaleInc (&viewerEye, &pPathPos->orient.fvec, PP_DELTAZ * 2 / 3);
	VmVecScaleInc (&viewerEye, &pPathPos->orient.uvec, PP_DELTAY * 2 / 3);
#endif
	}
}

//------------------------------------------------------------------------------

extern kc_item kc_mouse [];

inline int ZoomKeyPressed (void)
{
#if 1
	int	v;

return keyd_pressed [kc_keyboard [52].value] || keyd_pressed [kc_keyboard [53].value] ||
		 (((v = kc_mouse [30].value) < 255) && MouseButtonState (v));
#else
return (Controls.zoomDownCount > 0);
#endif
}

//------------------------------------------------------------------------------

int bUsePlayerHeadAngles = 0;
extern int critical_error_counter_ptr, nDescentCriticalError;

extern int Num_tmaps_drawn;
extern int nTotalPixels;
//--unused-- int Total_num_tmaps_drawn=0;

void StartLightingFrame (object *viewer);

#ifndef JOHN_ZOOM
#	define JOHN_ZOOM
#endif

//------------------------------------------------------------------------------

void RenderShadow (float fDist)
{
gameStates.render.nShadowPass = 3;
OglStartFrame (0, 0);
#if 0
glPushMatrix ();
glLoadIdentity ();
glMatrixMode (GL_PROJECTION);
glPushMatrix ();
glLoadIdentity ();
glOrtho (0, 1, 1, 0, 0, 1);
#endif
glBlendFunc (GL_ONE, GL_ONE);
glDisable (GL_DEPTH_TEST);
glColor4f (1.0f, 0.5f, 0.0f, 0.9f);// / fDist);
glBegin (GL_QUADS);
glVertex2f (0, 0);
glVertex2f (0, 1);
glVertex2f (1, 1);
glVertex2f (1, 0);
glEnd ();
glEnable (GL_DEPTH_TEST);
#if 0
glPopMatrix ();
glMatrixMode (GL_MODELVIEW);
glPopMatrix ();
#endif
gameStates.render.nShadowPass = 2;
OglStartFrame (0, 0);
}

//------------------------------------------------------------------------------

int RenderShadowMap (tLightInfo *pLight)
{
	tCamera	*pc;

if (pLight->nShadowFrame == gameData.render.shadows.nFrame)
	return 0;
if (gameData.render.shadows.nShadowMaps == MAX_SHADOW_MAPS)
	return 0;
pLight->nShadowFrame = !pLight->nShadowFrame;
gameStates.render.nShadowPass = 2;
pc = gameData.render.shadows.shadowMaps + gameData.render.shadows.nShadowMaps++;
CreateCamera (pc, pLight->nSegNum, pLight->nSideNum, pLight->nSegNum, pLight->nSideNum, NULL, 1, 0);
RenderCamera (pc);
gameStates.render.nShadowPass = 2;
return 1;
}

//------------------------------------------------------------------------------
//The following code is an attempt to find all objects that cast a shadow visible
//to the player. To accomplish that, for each robot the line of sight to each
//segment visible to the player is computed. If there is a los to any of these 
//segments, the object's shadow is rendered. Far from perfect solution though. :P

void RenderObjectShadows (void)
{
	object		*objP = gameData.objs.objects;
	int			i, j, bSee;
	object		fakePlayerPos = *gameData.objs.viewer;

for (i = 0; i <= gameData.objs.nLastObject; i++, objP++)
	if (objP == gameData.objs.console)
		RenderObject (objP, 0);
	else if ((objP->type == OBJ_ROBOT) || (objP->type == OBJ_PLAYER)) {
		for (j = nRenderSegs; j--;) {
			fakePlayerPos.segnum = nRenderList [j];
			COMPUTE_SEGMENT_CENTER_I (&fakePlayerPos.pos, fakePlayerPos.segnum);
			bSee = ObjectToObjectVisibility (objP, &fakePlayerPos, FQ_TRANSWALL);
			if (bSee) {
				RenderObject (objP, 0);
				break;
				}
			}
		}
}

//------------------------------------------------------------------------------

void DestroyShadowMaps (void)
{
	tCamera	*pc;

for (pc = gameData.render.shadows.shadowMaps; gameData.render.shadows.nShadowMaps; gameData.render.shadows.nShadowMaps--, pc++)
	DestroyCamera (pc);
}

//------------------------------------------------------------------------------

void ApplyShadowMaps (short startSegNum, fix nEyeOffset, int nWindowNum)
{	
	static float mTexBiasf [] = {
		0.5f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f};

	static float mPlanef [] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};

	static GLenum nTexCoord [] = {GL_S, GL_T, GL_R, GL_Q};

	float mProjectionf [16];
	float mModelViewf [16];

	int			i;
	tCamera		*pc;

#if 1
OglActiveTexture (GL_TEXTURE0_ARB);
glEnable (GL_TEXTURE_2D); 

glEnable (GL_TEXTURE_GEN_S);
glEnable (GL_TEXTURE_GEN_T);
glEnable (GL_TEXTURE_GEN_R);
glEnable (GL_TEXTURE_GEN_Q);

for (i = 0; i < 4; i++)
	glTexGeni (nTexCoord [i], GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
for (i = 0; i < 4; i++)
	glTexGenfv (nTexCoord [i], GL_EYE_PLANE, mPlanef + 4 * i);

glGetFloatv (GL_PROJECTION_MATRIX, mProjectionf);
glMatrixMode (GL_TEXTURE);
for (i = 0, pc = gameData.render.shadows.shadowMaps; i < 1/*gameData.render.shadows.nShadowMaps*/; i++) {
	glBindTexture (GL_TEXTURE_2D, pc->fb.texId);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	glLoadMatrixf (mTexBiasf);
	glMultMatrixf (mProjectionf);
	glMultMatrixf (OOF_MatVms2Gl (mModelViewf, &pc->objP->orient));
	}
glMatrixMode (GL_MODELVIEW);
#endif
RenderMine (startSegNum, nEyeOffset, nWindowNum);
#if 1
glMatrixMode (GL_TEXTURE);
glLoadIdentity ();
glMatrixMode (GL_MODELVIEW);
glDisable (GL_TEXTURE_GEN_S);
glDisable (GL_TEXTURE_GEN_T);
glDisable (GL_TEXTURE_GEN_R);
glDisable (GL_TEXTURE_GEN_Q);
OglActiveTexture (GL_TEXTURE0_ARB);		
glDisable (GL_TEXTURE_2D);
#endif
DestroyShadowMaps ();
}

//------------------------------------------------------------------------------

void RenderFrame (fix nEyeOffset, int nWindowNum)
{
	short startSegNum;
	static int bStopZoom;

if (gameStates.app.bEndLevelSequence) {
	RenderEndLevelFrame (nEyeOffset, nWindowNum);
	gameData.app.nFrameCount++;
	return;
	}
#ifdef NEWDEMO
if ( gameData.demo.nState == ND_STATE_RECORDING && nEyeOffset >= 0 )	{
   if (gameStates.render.nRenderingType==0)
   	NDRecordStartFrame(gameData.app.nFrameCount, gameData.app.xFrameTime );
   if (gameStates.render.nRenderingType!=255)
   	NDRecordViewerObject(gameData.objs.viewer);
	}
#endif
  
StartLightingFrame (gameData.objs.viewer);		//this is for ugly light-smoothing hack
#ifdef OGL_ZBUF
if (!gameOpts->legacy.bZBuf)
	gameStates.ogl.bEnableScissor = !gameStates.render.cameras.bActive && nWindowNum;
#endif
G3StartFrame (0, !(nWindowNum || gameStates.render.cameras.bActive));
viewerEye = gameData.objs.viewer->pos;
if (nEyeOffset) {
	VmVecScaleInc(&viewerEye,&gameData.objs.viewer->orient.rvec,nEyeOffset);
	}
#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR)
	viewerEye = gameData.objs.viewer->pos;
#endif

pPathPos = NULL;
if (gameStates.render.cameras.bActive) {
	startSegNum = gameData.objs.viewer->segnum;
	G3SetViewMatrix (&viewerEye, &gameData.objs.viewer->orient, nRenderZoom);
	}
else {
	startSegNum = FindSegByPoint (&viewerEye,gameData.objs.viewer->segnum);
	if (startSegNum == -1)
		startSegNum = gameData.objs.viewer->segnum;
	if (gameData.objs.viewer == gameData.objs.console && bUsePlayerHeadAngles) {
		vms_matrix mHead, mView;
		VmAngles2Matrix (&mHead,&viewInfo.playerHeadAngles);
		VmMatMul (&mView,&gameData.objs.viewer->orient,&mHead);
		G3SetViewMatrix(&viewerEye,&mView,nRenderZoom );
		}
	else if (gameStates.render.bRearView && (gameData.objs.viewer==gameData.objs.console)) {
		vms_matrix mHead, mView;
		viewInfo.playerHeadAngles.p = 
		viewInfo.playerHeadAngles.b = 0;
		viewInfo.playerHeadAngles.h = 0x7fff;
		VmAngles2Matrix(&mHead,&viewInfo.playerHeadAngles);
		VmMatMul(&mView,&gameData.objs.viewer->orient,&mHead);
		G3SetViewMatrix(&viewerEye,&mView,fixdiv(nRenderZoom,gameStates.render.nZoomFactor));
		} 
	else if (!IsMultiGame || gameStates.app.bHaveExtraGameInfo [1]) {
#ifdef JOHN_ZOOM
		gameStates.render.nMinZoomFactor = (fix) (F1_0 * gameStates.render.glAspect); //(((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 * F1_0  / 3 : F1_0) * glAspect);
		gameStates.render.nMaxZoomFactor = gameStates.render.nMinZoomFactor * 5;
		if ((gameData.weapons.nPrimary != VULCAN_INDEX) && (gameData.weapons.nPrimary != GAUSS_INDEX))
			gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor; //(fix) (((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 * F1_0  / 3 : F1_0) * glAspect);
		else {
			switch (extraGameInfo [IsMultiGame].nZoomMode) {
				case 0:
					break;
				case 1:
					if (ZoomKeyPressed ()) {
						if (!bStopZoom) {
							gameStates.render.nZoomFactor = (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? (gameStates.render.nZoomFactor * 7) / 5 : (gameStates.render.nZoomFactor * 5) / 3;
							if (gameStates.render.nZoomFactor > gameStates.render.nMaxZoomFactor) 
								gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor;
							bStopZoom = 1;
							} 
						}
					else {
						bStopZoom = 0;
						}
					break;
				case 2:
					if (ZoomKeyPressed ()) {
						gameStates.render.nZoomFactor += gameData.app.xFrameTime * 4;
						if (gameStates.render.nZoomFactor > gameStates.render.nMaxZoomFactor) 
							gameStates.render.nZoomFactor = gameStates.render.nMaxZoomFactor;
						} 
					else {
						gameStates.render.nZoomFactor -= gameData.app.xFrameTime * 4;
						if (gameStates.render.nZoomFactor < gameStates.render.nMinZoomFactor) 
							gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor;
						}
					break;
				}
			}
		if ((gameData.objs.viewer == gameData.objs.console) && 
#ifdef _DEBUG
			 gameStates.render.bExternalView) {
#else		
			 gameStates.render.bExternalView && (!IsMultiGame || IsCoopGame)) {
#endif			 	
#if 1
			SetPathPoint ();
			GetViewPoint ();
#else
			VmVecScaleInc (&viewerEye, &gameData.objs.viewer->orient.fvec, -i2f (30));
			VmVecScaleInc (&viewerEye, &gameData.objs.viewer->orient.uvec, i2f (10));
#endif
			G3SetViewMatrix (&viewerEye, pPathPos ? &pPathPos->orient : &gameData.objs.viewer->orient, nRenderZoom);
			}
		else
			G3SetViewMatrix (&viewerEye, &gameData.objs.viewer->orient, fixdiv (nRenderZoom, gameStates.render.nZoomFactor));
		}
	else
		G3SetViewMatrix (&viewerEye, &gameData.objs.viewer->orient, nRenderZoom);
#else
	G3SetViewMatrix (&viewerEye, &gameData.objs.viewer->orient, nRenderZoom);
#endif
	}
if (nClearWindow == 1) {
	if (!nClearWindowColor)
		nClearWindowColor = BLACK_RGBA;	//BM_XRGB(31, 15, 7);
	GrClearCanvas (nClearWindowColor);
	}
#ifndef NDEBUG
if (bShowOnlyCurSide)
	GrClearCanvas (nClearWindowColor);
#endif
#ifdef _DEBUG
if (EGI_FLAG (bShadows, 0, 0) && 
	 !(nWindowNum || gameStates.render.cameras.bActive)) {
	if (!gameStates.render.bShadowMaps) {
		gameStates.render.nShadowPass = 1;
		OglStartFrame (0, 0);
		RenderMine (startSegNum, nEyeOffset, nWindowNum);
		}
	gameStates.render.nShadowPass = 2;
	OglStartFrame (0, 0);
	gameData.render.shadows.nFrame = !gameData.render.shadows.nFrame;
	RenderObjectShadows ();
	gameStates.render.nShadowPass = 3;
	OglStartFrame (0, 0);
	if (gameStates.render.bShadowMaps) {
#ifdef _DEBUG
		if (gameStates.render.bExternalView)
#else		
		if (gameStates.render.bExternalView && (!IsMultiGame || IsCoopGame))
#endif			 	
			G3SetViewMatrix (&viewerEye, pPathPos ? &pPathPos->orient : &gameData.objs.viewer->orient, nRenderZoom);
		else
			G3SetViewMatrix (&viewerEye, &gameData.objs.viewer->orient, fixdiv (nRenderZoom, gameStates.render.nZoomFactor));
		ApplyShadowMaps (startSegNum, nEyeOffset, nWindowNum);
		}
	else if (gameStates.render.bAltShadows)
		RenderShadow (0.0f);
	else
		RenderMine (startSegNum, nEyeOffset, nWindowNum);
	}
else 
#endif
	{
	if (gameStates.render.nRenderPass < 0)
		RenderMine (startSegNum, nEyeOffset, nWindowNum);
	else {
		for (gameStates.render.nRenderPass = 0;
			gameStates.render.nRenderPass < 2;
			gameStates.render.nRenderPass++) {
			OglStartFrame (0, 1);
			RenderMine (startSegNum, nEyeOffset, nWindowNum);
			}
		}
	if (!nWindowNum)
		RenderSmoke ();
	}
gameStates.render.nShadowPass = 0;
if (bUsePlayerHeadAngles) 
	Draw3DReticle (nEyeOffset);
G3EndFrame ();
}

//------------------------------------------------------------------------------

int nFirstTerminalSeg;

void update_rendered_data(int nWindowNum, object *viewer, int rear_view_flag, int user)
{
	Assert(nWindowNum < MAX_RENDERED_WINDOWS);
	Window_rendered_data [nWindowNum].frame = gameData.app.nFrameCount;
	Window_rendered_data [nWindowNum].viewer = viewer;
	Window_rendered_data [nWindowNum].rear_view = rear_view_flag;
	Window_rendered_data [nWindowNum].user = user;
}

//------------------------------------------------------------------------------
//build a list of segments to be rendered
//fills in nRenderList & nRenderSegs

void QSortSegZRef (short left, short right)
{
	tSegZRef	m = segZRef  [(left + right) / 2];
	tSegZRef	h;
	short		l = left, 
				r = right;
do {
	while ((segZRef [l].z > m.z) || ((segZRef [l].z == m.z) && (segZRef [l].d > m.d)))
		l++;
	while ((segZRef [r].z < m.z) || ((segZRef [r].z == m.z) && (segZRef [r].d < m.d)))
		r--;
	if (l <= r) {
		if (l < r) {
			h = segZRef [l];
			segZRef [l] = segZRef [r];
			segZRef [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortSegZRef (l, right);
if (left < r)
	QSortSegZRef (left, r);
}

//------------------------------------------------------------------------------
// sort segments by distance from player segment, where distance is the minimal
// number of segments that have to be traversed to reach a segment, starting at
// the player segment. If the entire mine is to be rendered, segments will then
// be rendered in reverse order (furthest first), to achieve proper rendering of
// transparent walls layered in a view axis.

static short segList [MAX_SEGMENTS];
static short segDist [MAX_SEGMENTS];
static char  bRenderSegObjs [MAX_SEGMENTS];
static char  bRenderObjs [MAX_OBJECTS];
static int nRenderObjs;
static short nSegListSize;

void BuildSegList (void)
{
	int		h, i, j, segNum, childNum, sideNum, nDist = 0;
	segment	*segP;

memset (segDist, 0xFF, sizeof (segDist));
segNum = gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].segnum;
i = j = 0;
segDist [segNum] = 0;
segList [j++] = segNum;
do {
	nDist++;
	for (h = i, i = j; h < i; h++) {
		segNum = segList [h];
		segP = gameData.segs.segments + segNum;
		for (sideNum = 0; sideNum < 6; sideNum++) {
			childNum = segP->children [sideNum];
			if (childNum < 0)
				continue;
			if (segDist [childNum] >= 0)
				continue;
			segDist [childNum] = nDist;
			segList [j++] = childNum;
			}
		}
	} while (i < j);
nSegListSize = j;
}

//------------------------------------------------------------------------------

void BuildSegmentList(short startSegNum, int nWindowNum)
{
	int	lcnt,scnt,ecnt;
	int	l;
	short	c;
	short	ch;
	short *sv;
	sbyte *s2v;

memset(bVisited, 0, sizeof(bVisited [0])*(gameData.segs.nLastSegment+1));
nVisited = 0;
memset(render_pos, -1, sizeof(render_pos [0])*(gameData.segs.nLastSegment+1));
//memset(no_render_flag, 0, sizeof(no_render_flag [0])*(MAX_RENDER_SEGS);
memset(processed, 0, sizeof(processed));
memset(nRenderList, 0xff, sizeof (nRenderList));

#ifndef NDEBUG
memset(visited2, 0, sizeof(visited2 [0])*(gameData.segs.nLastSegment+1));
#endif

lcnt = scnt = 0;

nRenderList [lcnt] = startSegNum; 
VISIT (startSegNum);
Seg_depth [lcnt] = 0;
lcnt++;
ecnt = lcnt;
render_pos [startSegNum] = 0;

#ifndef NDEBUG
if (pre_draw_segs)
	RenderSegment(startSegNum, nWindowNum);
#endif

render_windows [0].left =
render_windows [0].top = 0;
render_windows [0].right = grdCurCanv->cv_bitmap.bm_props.w - 1;
render_windows [0].bot = grdCurCanv->cv_bitmap.bm_props.h - 1;

//breadth-first renderer

//build list
for (l = 0; l < gameStates.render.detail.nRenderDepth; l++) {
	//while (scnt < ecnt) {
	for (scnt = 0;scnt < ecnt; scnt++) {
		int rotated,segnum;
		window *check_w;
		short child_list [MAX_SIDES_PER_SEGMENT];		//list of ordered sides to process
		int n_children;										//how many sides in child_list
		segment *seg;
		if (processed [scnt])
			continue;
		processed [scnt]=1;
		segnum = nRenderList [scnt];
		check_w = render_windows+scnt;
#ifndef NDEBUG
		if (draw_boxes)
			draw_window_box(RED_RGBA,check_w->left,check_w->top,check_w->right,check_w->bot);
#endif
		if (segnum == -1) 
			continue;
		seg = gameData.segs.segments + segnum;
		sv = seg->verts;
		rotated=0;
		//look at all sides of this segment.
		//tricky code to look at sides in correct order follows
		for (c = n_children = 0; c < MAX_SIDES_PER_SEGMENT; c++) {		//build list of sides
			int wid = WALL_IS_DOORWAY (seg, c, NULL);
			ch=seg->children [c];
			if ((bWindowCheck || ((ch > -1) && !VISITED (ch))) && (wid & WID_RENDPAST_FLAG)) {
				if (behind_check) {
					ubyte codes_and=0xff;
					int i;
					s2v = sideToVerts [c];
					if (!rotated) {
						RotateList (8, seg->verts);
						rotated = 1;
						}
					for (i = 0; i < 4; i++)
						codes_and &= gameData.segs.points [sv [s2v [i]]].p3_codes;
					if (codes_and & CC_BEHIND) 
						continue;
					}
				child_list [n_children++] = c;
			}
		}

		//now order the sides in some magical way
#if 1
		if (new_seg_sorting && (n_children > 1))
			SortSegChildren(seg,n_children,child_list);
#endif
		//for (c=0;c<MAX_SIDES_PER_SEGMENT;c++)	{
		//	ch=seg->children [c];
		for (c=0;c<n_children;c++) {
			int siden;
			siden = child_list [c];
			ch=seg->children [siden];
			//if ( (bWindowCheck || !bVisited [ch])&& (WALL_IS_DOORWAY(seg, c))) {
			{
				if (bWindowCheck) {
					int i;
					ubyte codes_and_3d,codes_and_2d;
					short _x,_y,min_x=32767,max_x=-32767,min_y=32767,max_y=-32767;
					int no_proj_flag=0;	//a point wasn't projected
					if (rotated<2) {
//							if (!rotated)
//								RotateList(8,seg->verts);
						ProjectList(8,seg->verts);
						rotated=2;
					}
					s2v = sideToVerts [siden];
					for (i=0,codes_and_3d=codes_and_2d=0xff;i<4;i++) {
						int p = sv [s2v [i]];
						g3s_point *pnt = gameData.segs.points+p;
						if (!(pnt->p3_flags&PF_PROJECTED)) {
							no_proj_flag=1; 
							break;
							}
						_x = f2i(pnt->p3_sx);
						_y = f2i(pnt->p3_sy);
						codes_and_3d &= pnt->p3_codes;
						codes_and_2d &= code_window_point(_x,_y,check_w);
#ifndef NDEBUG
						if (draw_edges) {
							GrSetColorRGB (128, 0, 128, 255);
							gr_uline(pnt->p3_sx,pnt->p3_sy,
								gameData.segs.points [seg->verts [sideToVerts [siden][(i+1)%4]]].p3_sx,
								gameData.segs.points [seg->verts [sideToVerts [siden][(i+1)%4]]].p3_sy);
						}
#endif
						if (_x < min_x) 
							min_x = _x;
						if (_x > max_x) 
							max_x = _x;
						if (_y < min_y) 
							min_y = _y;
						if (_y > max_y) 
							max_y = _y;
					}
#ifndef NDEBUG
					if (draw_boxes)
						draw_window_box(WHITE_RGBA,min_x,min_y,max_x,max_y);
#endif
					if (no_proj_flag || (!codes_and_3d && !codes_and_2d)) {	//maybe add this segment
						int rp = render_pos [ch];
						window *new_w = render_windows + lcnt;

						if (no_proj_flag) 
							*new_w = *check_w;
						else {
							new_w->left  = max(check_w->left,min_x);
							new_w->right = min(check_w->right,max_x);
							new_w->top   = max(check_w->top,min_y);
							new_w->bot   = min(check_w->bot,max_y);
						}
						//see if this seg already bVisited, and if so, does current window
						//expand the old window?
						if (rp != -1) {
							window *rwP = render_windows + rp;
							if (new_w->left < rwP->left ||
								 new_w->top < rwP->top ||
								 new_w->right > rwP->right ||
								 new_w->bot > rwP->bot) {
								new_w->left  = min(new_w->left,rwP->left);
								new_w->right = max(new_w->right,rwP->right);
								new_w->top   = min(new_w->top,rwP->top);
								new_w->bot   = max(new_w->bot,rwP->bot);
								if (no_migrate_segs) {
									//no_render_flag [lcnt] = 1;
									nRenderList [lcnt] = -1;
									*rwP = *new_w;		//get updated window
									processed [rp] = 0;		//force reprocess
									goto no_add;
								}
								else
									nRenderList [rp]=-1;
							}
							else 
								goto no_add;
						}
#ifndef NDEBUG
						if (draw_boxes)
							draw_window_box(5,new_w->left,new_w->top,new_w->right,new_w->bot);
#endif
						render_pos [ch] = lcnt;
						nRenderList [lcnt] = ch;
						Seg_depth [lcnt] = l;
						lcnt++;
						if (lcnt >= MAX_RENDER_SEGS) {
#if TRACE								
							//con_printf (CON_DEBUG,"Too many segs in render list!!\n"); 
#endif
							goto done_list;
							}
						VISIT (ch);

#ifndef NDEBUG
						if (pre_draw_segs)
							RenderSegment(ch, nWindowNum);
#endif
no_add:
;
					}
				}
				else {
					nRenderList [lcnt] = ch;
					Seg_depth [lcnt] = l;
					lcnt++;
					if (lcnt >= MAX_RENDER_SEGS) {
#if TRACE								
						//con_printf (CON_DEBUG,"Too many segs in render list!!\n"); 
#endif
						goto done_list;
						}
					VISIT (ch);
				}
			}
		}
	}
	scnt = ecnt;
	ecnt = lcnt;
}
done_list:

lcnt_save = lcnt;
scnt_save = scnt;

nFirstTerminalSeg = scnt;
nRenderSegs = lcnt;
}


/*
void gl_build_object_list (void)
{
}
*/

//------------------------------------------------------------------------------

int GlRenderSegments (int nStartState, int nEndState, int nnRenderSegs, int nWindowNum)
{
#if 0
	short		segnum;
#endif
#if OGL_QUERY
	int		i, di, dj, bRenderLayer, objnum;
	GLint		fragC;

	static GLuint	segFragC [MAX_SEGMENTS],
						objFragC [MAX_OBJECTS];
#endif

if (gameOpts->legacy.bZBuf)
	return 0;
#if OGL_QUERY
for (renderState = nStartState; renderState < nEndState; renderState++) {
	int bOccQuery = bOcclusionQuery && gameOpts->render.bAllSegs && !nWindowNum && !renderState;
	if (bOccQuery) {
		glGenQueries (gameData.segs.nLastSegment + 1, glSegOccQuery);
		glGenQueries (MAX_OBJECTS, glObjOccQuery);
		memset (segFragC, 0, sizeof (segFragC));
		memset (objFragC, 0, sizeof (objFragC));
		memset (bSidesRendered, 0, sizeof (bSidesRendered));
		}
	// the following code tries to determine segments up to which distance (in segments) from the 
	// start segment need to be rendered. To do that, the occlusion query OpenGL extension is used.
	// If segments in three subsequents distances do not get rendered, it is assumed that all
	// visible segments have been rendered.
	for (gameStates.render.bQueryOcclusion = bOccQuery; gameStates.render.bQueryOcclusion >= 0; gameStates.render.bQueryOcclusion--) {
		//if (gameStates.render.nShadowPass != 3)
			switch (renderState) {
				case 0: //solid sides
					glDepthFunc (GL_LESS);
					break;
				case 1: //walls
					glDepthFunc (GL_LEQUAL);
					break;
				case 2: //transparency (alpha blending)
					glDepthFunc (GL_LEQUAL);
					break;
				}
		if (gameStates.render.bQueryOcclusion) {
			di = -1;
			fragC = grdCurCanv->cv_bitmap.bm_props.w * grdCurCanv->cv_bitmap.bm_props.h;
			bRenderLayer = 4;
			for (i = 0; i < nSegListSize; i++) {
				segnum = segList [i];
				dj = segDist [segnum];
				if (di != dj) {
#if 1
					if (di && !bRenderLayer)
						break;
#endif
					di = dj;
					bRenderLayer--;
					}
		      glBeginQuery (GL_SAMPLES_PASSED_ARB, glSegOccQuery [segnum]);
				RenderSegment (segnum, nWindowNum);
		      glEndQuery (GL_SAMPLES_PASSED_ARB);
				glGetQueryObjectuiv (glSegOccQuery [segnum], GL_QUERY_RESULT_ARB, segFragC + segnum);
#if 1
				if (segFragC [segnum] > 500) { // || !bSidesRendered [segnum]) {
#	if 0
					fragC -= segFragC [segnum];
					if (fragC <= 0)
						break;
#	endif
					bRenderLayer = 3;
					for (objnum = gameData.segs.segments [segnum].objects; objnum != -1; objnum = gameData.objs.objects [objnum].next)
						objFragC [objnum] = 1;
					}
#	if 0
				else {
					for (objnum = gameData.segs.segments [segnum].objects; objnum != -1; objnum = gameData.objs.objects [objnum].next) {
				      glBeginQuery (GL_SAMPLES_PASSED_ARB, glObjOccQuery [objnum]);
						DoRenderObject (objnum, nWindowNum);
				      glEndQuery (GL_SAMPLES_PASSED_ARB);
						glGetQueryObjectuiv (glObjOccQuery [objnum], GL_QUERY_RESULT_ARB, objFragC + objnum);
						}
					}
#	endif
#endif
				}
			}
		else 
			for (i = nSegListSize; i;) {
				segnum = segList [--i];
#if 1
				if (bOcclusionQuery) {
					if (segDist [segnum] > di)
						continue;
					}
#endif
				RenderSegment (segnum, nWindowNum);
#if 1
				if (gameOpts->render.bAllSegs && !nWindowNum && !renderState && bRenderSegObjs [segnum])
					for (objnum = gameData.segs.segments [segnum].objects; objnum != -1; objnum = gameData.objs.objects [objnum].next)
						//if (objFragC [objnum])
						if (bRenderObjs [objnum])
							DoRenderObject (objnum, nWindowNum);
#endif
				}
		}
	gameStates.render.bQueryOcclusion = 0;
	if (bOccQuery) {
		glDeleteQueries (gameData.segs.nLastSegment + 1, glSegOccQuery);
		glDeleteQueries (MAX_OBJECTS, glObjOccQuery);
		}
	}
#else //OGL_QUERY
for (renderState = nStartState; renderState < nEndState; renderState++) {
	//if (gameStates.render.nShadowPass != 3)
		switch (renderState) {
			case 0: //solid sides
				glDepthFunc (GL_LESS);
				break;
			case 1: //walls
				glDepthFunc (GL_LEQUAL);
				break;
			case 2: //transparency (alpha blending)
				glDepthFunc (GL_LEQUAL);
				break;
			}
#if 0
	for (segnum = 0; segnum < gameData.segs.nSegments; segnum++)
		RenderSegment (segnum, nWindowNum);
#else
	while (nnRenderSegs)
		RenderSegment (nRenderList [--nnRenderSegs], nWindowNum);
#endif
	}
#endif //OGL_QUERY
glDepthFunc (GL_LESS);
renderState = -1;
return 1;
}

//------------------------------------------------------------------------------

inline void InitFaceList (void)
{
#if APPEND_LAYERED_TEXTURES
if (gameOpts->render.bOptimize) {
	nFaceListSize = 0;
	memset (faceListRoots, 0xFF, sizeof (faceListRoots));
	memset (faceListTails, 0xFF, sizeof (faceListTails));
	}
#endif
}

//------------------------------------------------------------------------------

void RenderObjList (int listnum, int nWindowNum)
{
if (migrate_objects) {
	int objnp = 0;
	int saveLinDepth = gameStates.render.detail.nMaxLinearDepth;
	gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
	for (;;) {
		int objNum = render_obj_list [listnum][objnp];
		if (objNum < 0) {
			if (objNum == -1)
				break;
			listnum = -objNum;
			objnp = 0;
			}					
		else {
#ifdef LASER_HACK
			object *objP = gameData.objs.objects + objNum;
			if ((objP->type == OBJ_WEAPON) && 								//if its a weapon
				 (objP->lifeleft == Laser_max_time ) && 	//  and its in it's first frame
				 (Hack_nlasers < MAX_HACKED_LASERS) && 									//  and we have space for it
				 (objP->laser_info.parent_num > -1) &&					//  and it has a parent
				 ((OBJ_IDX (gameData.objs.viewer)) == objP->laser_info.parent_num)	//  and it's parent is the viewer
				 ) {
				Hack_laser_list [Hack_nlasers++] = objNum;								//then make it draw after everything else.
				}
			else	
#endif
				DoRenderObject (objNum, nWindowNum);	// note link to above else
			objnp++;
			}
		}
	gameStates.render.detail.nMaxLinearDepth = saveLinDepth;
	}
}

//------------------------------------------------------------------------------

void RenderVisibleObjects (int nWindowNum)
{
	int	i, j;

if (gameStates.render.nShadowPass == 2)
	return;
//memset (bVisited, 0, sizeof (bVisited [0]) * (gameData.segs.nLastSegment + 1));
nVisited++;
for (i = nRenderSegs; i--;)  {
	j = nRenderList [i];
	if ((j != -1) && !VISITED (j)) {
		VISIT (j);
		RenderObjList (i, nWindowNum);
		}
	}
}

//------------------------------------------------------------------------------

void RenderFaceList (int nWindowNum)
{
#if APPEND_LAYERED_TEXTURES
if (gameOpts->render.bOptimize) {
		int				i, j;
		segment			*segP;
		side				*sideP;
		tFaceListEntry	*flp = faceList;

	for (renderState = 0; renderState < 5; renderState++) {
#if 0
		switch (renderState) {
			case 1: //walls
				glDepthFunc (GL_LEQUAL);
				break;
			case 2: //transparency (alpha blending)
				glDepthFunc (GL_LEQUAL);
				break;
			default:
				glDepthFunc (GL_LESS);
				break;
			}
#endif
		for (i = 0; i < MAX_TEXTURES; i++) {
			if (0 > faceListRoots [i])
				continue;
			for (flp = faceList + faceListRoots [i]; ; flp = faceList + flp->nextFace) {
				segP = gameData.segs.segments + flp->props.segNum;
				sideP = segP->sides + flp->props.sideNum;
				if (flp->props.renderState == renderState)
					RenderFace (&flp->props, 0, 1);
				if (flp->nextFace < 0)
					break;
				}
			}
		}
	RenderVisibleObjects (nWindowNum);
	}
#endif
}

//------------------------------------------------------------------------------

void AddVisibleLight (short nSegment, short nSide, short nTexture, int bPrimary)
{
if ((bPrimary || nTexture) && gameData.pig.tex.brightness [nTexture]) {
	tLightRef	*plr = gameData.render.color.visibleLights + gameData.render.color.nVisibleLights++;

	plr->nSegment = nSegment;
	plr->nSide = nSide;
	plr->nTexture = nTexture;
	}
}

//------------------------------------------------------------------------------

void GatherVisibleLights (void)
{
	int	i;
	short	nSegment, nSide;
	side	*sideP;

gameData.render.color.nVisibleLights = 0;
for (i = 0; i < nRenderSegs;i++) {
	nSegment = nRenderList [i];
	for (nSide = 0, sideP = gameData.segs.segments [nSegment].sides; nSide < 6; nSide++, sideP++) {
		AddVisibleLight (nSegment, nSide, sideP->tmap_num, 1);
		AddVisibleLight (nSegment, nSide, sideP->tmap_num, 0);
		}
	}
}

//------------------------------------------------------------------------------
//renders onto current canvas

void RenderMine (short startSegNum, fix nEyeOffset, int nWindowNum)
{
	int		nn;
	short		segnum;
	int		rsMin, rsMax;
	ubyte		bSetAutomapVisited;
	window	*rwP;
#ifdef _DEBUG
	int	i;
#endif

	//	Initialize number of gameData.objs.objects (actually, robots!) rendered this frame.
	Window_rendered_data [nWindowNum].num_objects = 0;

#ifdef LASER_HACK
	Hack_nlasers = 0;
#endif
#ifdef _DEBUG
memset (bObjectRendered, 0, (gameData.objs.nLastObject + 1) * sizeof (bObjectRendered [0]));
#endif
	//set up for rendering

if ((gameStates.render.nRenderPass <= 0) ||
	 gameStates.render.bShadowMaps || 
	 (gameStates.render.nShadowPass < 2)) {
	RenderStartFrame ();
	TransformOglLights ();
#if defined(EDITOR) && !defined(NDEBUG)
		if (bShowOnlyCurSide) {
			RotateList (8, Cursegp->verts);
			RenderSide (Cursegp, Curside);
			goto done_rendering;
		}
#endif
	 }
#ifdef EDITOR
	if (bSearchMode)	{
		}
	else 
#endif

if (gameStates.render.nRenderPass <= 0) {
	BuildSegmentList (startSegNum, nWindowNum);		//fills in nRenderList & nRenderSegs
	GatherVisibleLights ();
#if OGL_QUERY
	if (gameOpts->render.bAllSegs && !nWindowNum) {
		memset (bRenderSegObjs, 0, sizeof (bRenderSegObjs));
		memset (bRenderObjs, 0, sizeof (bRenderObjs));
		nRenderObjs = 0;
		}
#endif
#ifdef _DEBUG
	if (!bWindowCheck) {
		nWindowClipLeft  = nWindowClipTop = 0;
		nWindowClipRight = grdCurCanv->cv_bitmap.bm_props.w-1;
		nWindowClipBot   = grdCurCanv->cv_bitmap.bm_props.h-1;
		}
	for (i = 0; i < nRenderSegs;i++) {
		short segnum = nRenderList [i];
		if (segnum != -1)
			if (visited2 [segnum])
				Int3();		//get Matt
			else
				visited2 [segnum] = 1;
		}
#endif
	if (gameStates.render.nRenderPass <= 0) {
		BuildObjectLists (nRenderSegs);
		if (nEyeOffset <= 0)	// Do for left eye or zero.
			SetDynamicLight();
		}
	}
if (nClearWindow == 2) {
	if (nFirstTerminalSeg < nRenderSegs) {
		int i;

		if (nClearWindowColor == -1)
			nClearWindowColor = BLACK_RGBA;
		GrSetColor(nClearWindowColor);
		for (i=nFirstTerminalSeg, rwP = render_windows; i < nRenderSegs; i++, rwP++) {
			if (nRenderList [i] != -1) {
#ifndef NDEBUG
				if ((rwP->left == -1) || 
					 (rwP->top == -1) || 
					 (rwP->right == -1) || 
					 (rwP->bot == -1))
					Int3();
				else
#endif
					//NOTE LINK TO ABOVE!
					GrRect(rwP->left, rwP->top, rwP->right, rwP->bot);
				}
			}
		}
	}
#if APPEND_LAYERED_TEXTURES
if (gameOpts->render.bOptimize || (gameOpts->render.bAllSegs && !nWindowNum)) {
#else
if (gameOpts->render.bAllSegs) {
#endif
	rsMin = 0;
	rsMax = 0;
	}
else
	rsMin = rsMax = -1;
bSetAutomapVisited =
	!((extraGameInfo [0].bShadows && (gameStates.render.nShadowPass < 3)) || 
	  gameStates.render.cameras.bActive) && 
	 (gameData.objs.viewer->type != OBJ_ROBOT);
InitFaceList ();
for (renderState = rsMin; renderState <= rsMax; renderState++) {
	//memset (bVisited, 0, sizeof (bVisited [0]) * (gameData.segs.nSegments));
	nVisited++;
	for (nn = nRenderSegs; nn; ) {
		nn--;
		rwP = render_windows + nn;
		segnum = nRenderList [nn];
		nCurrentSegDepth = Seg_depth [nn];
		if ((segnum != -1) && !VISITED (segnum)) {
#if 0
			if (bWindowCheck) {
				nWindowClipLeft = rwP->left;
				nWindowClipTop  = rwP->top;
				nWindowClipRight = rwP->right;
				nWindowClipBot = rwP->bot;
				}
#endif
			if (gameStates.render.nShadowPass != 2)
				RenderSegment (segnum, nWindowNum);
#if APPEND_LAYERED_TEXTURES
			if (!(renderState || gameOpts->render.bAllSegs) || gameOpts->render.bOptimize)
#else
			if (!(renderState || gameOpts->render.bAllSegs))
#endif
				continue;
#ifdef OGL_ZBUF
			if (!gameOpts->legacy.bZBuf)
  	   		bAutomapVisited [segnum] = bSetAutomapVisited;
#endif
			VISIT (segnum);

			if (bWindowCheck) {		//reset for gameData.objs.objects
				nWindowClipLeft = nWindowClipTop = 0;
				nWindowClipRight = grdCurCanv->cv_bitmap.bm_props.w-1;
				nWindowClipBot = grdCurCanv->cv_bitmap.bm_props.h-1;
				}
			if (!gameOpts->render.bAllSegs)
				RenderObjList (nn, nWindowNum);
			}
		}
#ifdef LASER_HACK								
	for (i = 0; i < Hack_nlasers; i++ )
		DoRenderObject (Hack_laser_list [i], nWindowNum);
#endif
	}
#ifdef EDITOR
#	ifdef _DEBUG
//draw curedge stuff
if (bOutLineMode) 
	OutlineSegSide (Cursegp,Curside,Curedge,Curvert);
#	endif
#endif
//RenderVisibleObjects (nWindowNum);
if (gameStates.render.bHaveSkyBox) {
	gameStates.render.bHaveSkyBox = 0;
	renderState = 4;
	for (segnum = 0; segnum <= gameData.segs.nLastSegment; segnum++)
		if (gameData.segs.segment2s [segnum].special == SEGMENT_IS_SKYBOX) {
			gameStates.render.bHaveSkyBox = 1;
			RenderSegment (segnum, nWindowNum);
			}
	}
renderState = 1;
if (gameOpts->render.bAllSegs) {
	nVisited++;
	for (nn = nRenderSegs; nn; ) {
		segnum = nRenderList [--nn];
		if ((segnum != -1) && !VISITED (segnum)) {
			VISIT (segnum);
			if (gameStates.render.nShadowPass != 2)
				RenderSegment (segnum, nWindowNum);
			RenderObjList (nn, nWindowNum);
			}
		}
	}
#ifdef OGL_ZBUF
GlRenderSegments (2, 3, nRenderSegs, nWindowNum);
#endif
RenderFaceList (nWindowNum);
}
#ifdef EDITOR

//------------------------------------------------------------------------------

extern int render_3d_in_big_window;

//finds what segment is at a given x&y -  seg,side,face are filled in
//works on last frame rendered. returns true if found
//if seg<0, then an object was found, and the object number is -seg-1
int find_seg_side_face(short x,short y,int *seg,int *side,int *face,int *poly)
{
	bSearchMode = -1;
	_search_x = x; _search_y = y;
	found_seg = -1;
	if (render_3d_in_big_window) {
		grs_canvas temp_canvas;

		GrInitSubCanvas(&temp_canvas,canv_offscreen,0,0,
			LargeView.ev_canv->cv_bitmap.bm_props.w,LargeView.ev_canv->cv_bitmap.bm_props.h);
		GrSetCurrentCanvas(&temp_canvas);
		RenderFrame(0, 0);
	}
	else {
		GrSetCurrentCanvas(&VR_render_sub_buffer [0]);	//render off-screen
		RenderFrame(0, 0);
	}
	bSearchMode = 0;
	*seg = found_seg;
	*side = found_side;
	*face = found_face;
	*poly = found_poly;
	return (found_seg!=-1);
}

#endif

