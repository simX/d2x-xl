/* $Id: 3d.h,v 1.11 2004/05/11 03:35:20 btb Exp $ */
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
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

/*
 *
 * Header file for 3d library
 * except for functions implemented in interp.c
 *
 * Old Log:
 * Revision 1.2  1995/09/14  14:08:58  allender
 * return value for G3DrawSphere
 *
 * Revision 1.1  1995/05/05  08:48:41  allender
 * Initial revision
 *
 * Revision 1.34  1994/11/11  19:22:14  matt
 * Added new function, G3CalcPointDepth()
 *
 * Revision 1.33  1994/09/09  14:23:58  matt
 * Added support for glowing textures, to add engine glow to Descent.
 *
 * Revision 1.32  1994/09/01  10:42:27  matt
 * Blob routine, renamed g3_draw_bitmap(), now takes seperate 3d width & height.
 *
 * Revision 1.31  1994/07/29  18:16:14  matt
 * Added instance by angles, and corrected parms for g3_init()
 *
 * Revision 1.30  1994/07/25  00:00:00  matt
 * Made 3d no longer deal with point numbers, but only with pointers.
 *
 * Revision 1.29  1994/07/22  17:57:27  matt
 * Changed the name of the rod functions, and took out some debugging code
 *
 * Revision 1.28  1994/06/07  16:49:12  matt
 * Made interpreter take lighting value as parm, rather than in global var
 *
 * Revision 1.27  1994/05/31  18:35:28  matt
 * Added light value to g3_draw_facing_bitmap()
 *
 * Revision 1.26  1994/05/30  22:48:04  matt
 * Added support for morph effect
 *
 * Revision 1.25  1994/05/30  11:34:57  matt
 * Added G3SetSpecialRender() to allow a user to specify functions to
 * call for 2d draws.
 *
 * Revision 1.24  1994/05/19  21:46:31  matt
 * Moved texture lighting out of 3d and into the game
 *
 * Revision 1.23  1994/05/14  15:26:48  matt
 * Added extern for polyobj outline flag
 *
 * Revision 1.22  1994/04/19  18:26:33  matt
 * Added G3DrawSphere() function.
 *
 * Revision 1.21  1994/03/25  18:22:28  matt
 * G3DrawPolyModel() now takes ptr to list of angles
 *
 * Revision 1.20  1994/03/15  21:23:23  matt
 * Added interpreter functions
 *
 * Revision 1.19  1994/02/15  17:37:34  matt
 * New function, g3_draw_blob()
 *
 * Revision 1.18  1994/02/09  11:47:47  matt
 * Added rod & delta point functions
 *
 * Revision 1.17  1994/01/26  12:38:11  matt
 * Added function g3_compute_lightingValue()
 *
 * Revision 1.16  1994/01/25  18:00:02  yuan
 * Fixed variable beam_brightness...
 *
 * Revision 1.15  1994/01/24  14:08:34  matt
 * Added instancing functions
 *
 * Revision 1.14  1994/01/22  18:21:48  matt
 * New lighting stuff now done in 3d; G3DrawTMap() takes lighting parm
 *
 * Revision 1.13  1994/01/20  17:21:24  matt
 * New function g3_compute_sky_polygon()
 *
 * Revision 1.12  1994/01/14  17:20:25  matt
 * Added prototype for new function g3_draw_horizon()
 *
 * Revision 1.10  1993/12/20  20:21:52  matt
 * Added G3Point2Vec()
 *
 * Revision 1.9  1993/12/07  23:05:47  matt
 * Fixed mistyped function name.
 *
 * Revision 1.8  1993/12/05  23:47:03  matt
 * Added function g3_draw_line_ptrs()
 *
 * Revision 1.7  1993/12/05  23:13:22  matt
 * Added prototypes for G3TransformAndEncodePoint() and G3ProjectPoint()
 *
 * Revision 1.6  1993/12/05  23:03:28  matt
 * Changed uvl structs to uvl
 *
 * Revision 1.5  1993/11/22  10:51:09  matt
 * Moved uvl structure here from segment.h, made texture map functions use it
 *
 * Revision 1.4  1993/11/21  20:08:31  matt
 * Added function g3_drawObject()
 *
 * Revision 1.3  1993/11/04  18:49:19  matt
 * Added system to only rotate points once per frame
 *
 * Revision 1.2  1993/11/04  08:16:06  mike
 * Add light field (p3_l) to g3sPoint.
 *
 * Revision 1.1  1993/10/29  22:20:56  matt
 * Initial revision
 *
 */

#ifndef _3D_H
#define _3D_H

#include "fix.h"
#include "vecmat.h" //the vector/matrix library
#include "globvars.h" //the vector/matrix library
#include "gr.h"

extern int g3d_interp_outline;      //if on, polygon models outlined in white

extern vmsVector Matrix_scale;     //how the matrix is currently scaled

extern short highest_texture_num;

//Structure for storing u,v,light values.  This structure doesn't have a
//prefix because it was defined somewhere else before it was moved here
typedef struct uvl {
	fix u,v,l;
} uvl;

//Stucture to store clipping codes in a word
typedef struct g3s_codes {
	ubyte or,and;   //or is low byte, and is high byte
} g3s_codes;

//flags for point structure
#define PF_PROJECTED    1   //has been projected, so sx,sy valid
#define PF_OVERFLOW     2   //can't project
#define PF_TEMP_POINT   4   //created during clip
#define PF_UVS          8   //has uv values set
#define PF_LS           16  //has lighting values set

//clipping codes flags

#define CC_OFF_LEFT     1
#define CC_OFF_RIGHT    2
#define CC_OFF_BOT      4
#define CC_OFF_TOP      8
#define CC_BEHIND       0x80

//Used to store rotated points for mines.  Has frame count to indictate
//if rotated, and flag to indicate if projected.
typedef struct g3sNormal {
	fVector		vNormal;
	ubyte			nFaces;	// # of faces that use this vertex
} g3sNormal;


typedef struct g3sPoint {
	vmsVector	p3_vec;  //x,y,z of rotated point
#ifdef D1XD3D
	vmsVector	p3_orig;
#endif
	fix			p3_u,p3_v,p3_l; //u,v,l coords
	fix			p3_sx,p3_sy;    //screen x&y
	ubyte			p3_codes;     //clipping codes
	ubyte			p3Flags;     //projected?
	short			p3_index;     //keep structure longword aligned
	g3sNormal	p3_normal;
} g3sPoint;

//macros to reference x,y,z elements of a 3d point
#define p3_x p3_vec.x
#define p3_y p3_vec.y
#define p3_z p3_vec.z

//An tObject, such as a robot
typedef struct g3sObject {
	vmsVector o3_pos;       //location of this tObject
	vmsAngVec o3_orient;    //orientation of this tObject
	int o3_nverts;           //number of points in the tObject
	int o3_nfaces;           //number of faces in the tObject

	//this will be filled in later

} g3sObject;

typedef void tmap_drawer_func (grsBitmap *, int, g3sPoint **);
typedef void flat_drawer_func (int, int *);
typedef int line_drawer_func (fix, fix, fix, fix);
typedef tmap_drawer_func *tmap_drawer_fp;
typedef flat_drawer_func *flat_drawer_fp;
typedef line_drawer_func *line_drawer_fp;

//Functions in library

//3d system startup and shutdown:

//initialize the 3d system
void g3_init(void);

//close down the 3d system
void _CDECL_ g3_close(void);


//Frame setup functions:

//start the frame
void G3StartFrame(int bFlat, int bResetColorBuf);

//set view from x,y,z & p,b,h, zoom.  Must call one of g3_set_view_*()
void G3SetViewAngles(vmsVector *view_pos,vmsAngVec *view_orient,fix zoom);

//set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
void G3SetViewMatrix(vmsVector *view_pos,vmsMatrix *view_matrix,fix zoom);

//end the frame
void G3EndFrame(void);

//draw a horizon
void g3_draw_horizon(int sky_color,int ground_color);

//get vectors that are edge of horizon
int g3_compute_sky_polygon(fix *points_2d,vmsVector *vecs);

//Instancing

//instance at specified point with specified orientation
void G3StartInstanceMatrix(vmsVector *pos,vmsMatrix *orient);

//instance at specified point with specified orientation
void G3StartInstanceAngles(vmsVector *pos,vmsAngVec *angles);

//pops the old context
void G3DoneInstance();

//Misc utility functions:

//get current field of view.  Fills in angle for x & y
void g3_get_FOV(fixang *fov_x,fixang *fov_y);

//get zoom.  For a given window size, return the zoom which will achieve
//the given FOV along the given axis.
fix g3_get_zoom(char axis,fixang fov,short window_width,short window_height);

//returns the normalized, unscaled view vectors
void g3_get_view_vectors(vmsVector *forward,vmsVector *up,vmsVector *right);

//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
bool G3CheckNormalFacing(vmsVector *v,vmsVector *norm);

//Point definition and rotation functions:

//specify the arrays refered to by the 'pointlist' parms in the following
//functions.  I'm not sure if we will keep this function, but I need
//it now.
//void g3_set_points(g3sPoint *points,vmsVector *vecs);

//returns codes_and & codes_or of a list of points numbers
g3s_codes g3_check_codes(int nv,g3sPoint **pointlist);

//projects a point
void G3ProjectPoint(g3sPoint *point);

//code a point.  fills in the p3_codes field of the point, and returns the codes
#if 1
static inline ubyte G3EncodePoint(g3sPoint *p)
{
ubyte cc=0;
fix z = p->p3_z;

if (p->p3_x > z)
	cc |= CC_OFF_RIGHT;
if (p->p3_x < -z)
	cc |= CC_OFF_LEFT;
if (p->p3_y > z)
	cc |= CC_OFF_TOP;
if (p->p3_y < -z)
	cc |= CC_OFF_BOT;
if (z < 0)
	cc |= CC_BEHIND;
return p->p3_codes = cc;
}
#else
ubyte G3EncodePoint(g3sPoint *point);
#endif

static inline vmsVector *G3TranslatePoint (vmsVector *pDest, vmsVector *pSrc)
{
return VmVecSub (pDest, pSrc, &viewInfo.position);
}

static inline vmsVector *G3RotatePoint (vmsVector *pDest, vmsVector *pSrc)
{
return VmVecRotate (pDest, pSrc, &viewInfo.view);
}

static inline vmsVector *G3TransformPoint (vmsVector *pDest, vmsVector *pSrc)
{
vmsVector	vTrans;
return VmVecRotate (pDest, VmVecSub (&vTrans, pSrc, &viewInfo.position), &viewInfo.view);
}

static inline fVector *G3TranslatePointf (fVector *pDest, fVector *pSrc)
{
return VmVecSubf (pDest, pSrc, &viewInfo.posf);
}

static inline fVector *G3RotatePointf (fVector *pDest, fVector *pSrc)
{
return VmVecRotatef (pDest, pSrc, &viewInfo.viewf);
}

static inline fVector *G3TransformPointf (fVector *pDest, fVector *pSrc)
{
fVector vTrans;
return VmVecRotatef (pDest, VmVecSubf (&vTrans, pSrc, &viewInfo.posf), &viewInfo.viewf);
}

static inline ubyte G3TransformAndEncodePoint (g3sPoint *pDest, vmsVector *pSrc)
{
G3TransformPoint (&pDest->p3_vec, pSrc);
pDest->p3Flags = 0;	
return G3EncodePoint (pDest);
}

//calculate the depth of a point - returns the z coord of the rotated point
fix G3CalcPointDepth(vmsVector *pnt);

//from a 2d point, compute the vector through that point
void G3Point2Vec(vmsVector *v,short sx,short sy);

//delta rotation functions
vmsVector *G3RotateDeltaX(vmsVector *dest,fix dx);
vmsVector *G3RotateDeltaY(vmsVector *dest,fix dy);
vmsVector *G3RotateDeltaZ(vmsVector *dest,fix dz);
vmsVector *G3RotateDeltaVec(vmsVector *dest,vmsVector *src);
ubyte G3AddDeltaVec(g3sPoint *dest,g3sPoint *src,vmsVector *deltav);

//Drawing functions:

//draw a flat-shaded face.
//returns 1 if off screen, 0 if drew
bool G3DrawPoly(int nv,g3sPoint **pointlist);

//draw a texture-mapped face.
//returns 1 if off screen, 0 if drew
bool G3DrawTMap(int nv,g3sPoint **pointlist,uvl *uvl_list,grsBitmap *bm, int bBlend);

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
int G3DrawSphere(g3sPoint *pnt,fix rad, int bBigSphere);

//@@//return ligting value for a point
//@@fix g3_compute_lightingValue(g3sPoint *rotated_point,fix normval);


//like G3DrawPoly(), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like G3CheckNormalFacing() plus
//G3DrawPoly().
//returns -1 if not facing, 1 if off screen, 0 if drew
bool G3CheckAndDrawPoly(int nv,g3sPoint **pointlist,vmsVector *norm,vmsVector *pnt);
bool G3CheckAndDrawTMap(int nv,g3sPoint **pointlist,uvl *uvl_list,grsBitmap *bm,vmsVector *norm,vmsVector *pnt);

//draws a line. takes two points.
bool G3DrawLine(g3sPoint *p0,g3sPoint *p1);

//draw a polygon that is always facing you
//returns 1 if off screen, 0 if drew
bool G3DrawRodPoly(g3sPoint *bot_point,fix bot_width,g3sPoint *top_point,fix top_width);

//draw a bitmap tObject that is always facing you
//returns 1 if off screen, 0 if drew
bool G3DrawRodTexPoly(grsBitmap *bitmap,g3sPoint *bot_point,fix bot_width,g3sPoint *top_point,fix top_width,fix light);

//draws a bitmap with the specified 3d width & height
//returns 1 if off screen, 0 if drew
bool G3DrawBitMap (vmsVector *pos,fix width,fix height,grsBitmap *bm, int orientation, float alpha, int transp, int bDepthInfo);

//specifies 2d drawing routines to use instead of defaults.  Passing
//NULL for either or both restores defaults
void G3SetSpecialRender(tmap_drawer_fp tmap_drawer, flat_drawer_fp flat_drawer, line_drawer_fp line_drawer);

extern g3sPoint *Vbuf0[];
extern g3sPoint *Vbuf1[];

#endif
