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
 * $Source: /cvs/cvsroot/d2x/include/texmap.h,v $
 * $Revision: 1.2 $
 * $Author: btb $
 * $Date: 2003/03/19 19:21:34 $
 *
 * Include file for entities using texture mapper library.
 *
 * $Log: texmap.h,v $
 * Revision 1.2  2003/03/19 19:21:34  btb
 * header/extern declaration housekeeping
 *
 * Revision 1.1.1.1  2001/01/19 03:30:16  bradleyb
 * Import of d2x-0.0.8
 *
 * Revision 1.2  1999/07/07 21:21:56  donut
 * increased recip table size to better accommodate 640 res
 *
 * Revision 1.1.1.1  1999/06/14 22:02:20  donut
 * Import of d1x 1.37 source.
 *
 * Revision 1.2  1995/09/04  14:22:10  allender
 * #defines for fixed point limits on render buffer
 *
 * Revision 1.1  1995/05/04  20:14:50  allender
 * Initial revision
 *
 * Revision 1.17  1994/11/10  11:09:16  mike
 * detail level stuff.
 * 
 * Revision 1.16  1994/11/09  22:55:32  matt
 * Added variable Current_seg_depth for detail level optimization
 * 
 * Revision 1.15  1994/06/09  16:10:04  mike
 * Add prototype for SC2000
 * 
 * Revision 1.14  1994/05/25  18:46:16  matt
 * Added gr_upoly_tmap_ylr(), which generates ylr's for a polygon
 * 
 * Revision 1.13  1994/05/25  09:47:12  mike
 * Added interface support for linear texture mapper (Mike change, Matt commnet)
 * 
 * Revision 1.12  1994/05/24  17:30:43  mike
 * Prototype a bunch of linear, vertical scanning functions.
 * 
 * Revision 1.11  1994/05/19  23:26:14  mike
 * Add constants NUM_LIGHTING_VALUES, MAX_LIGHTING_VALUE, MIN_LIGHTING_VALUE,
 * all part of new lightingValues_in_0_to_1 system.
 * 
 * Revision 1.10  1994/05/14  17:19:21  matt
 * Added externs
 * 
 * Revision 1.9  1994/04/13  23:55:44  matt
 * Increased max_tmap_verts from 16 to 25
 * 
 * Revision 1.8  1994/03/31  08:35:43  mike
 * Prototype for gr_upoly_tmap.
 * 
 * Revision 1.7  1994/02/08  15:17:54  mike
 * define label for MAX_TMAP_VERTS
 * 
 * Revision 1.6  1994/01/31  15:41:51  mike
 * Add texture_map_lin_lin_sky_v
 * 
 * Revision 1.5  1994/01/18  10:49:40  mike
 * prototype for texture_map_lin_lin_sky
 * 
 * Revision 1.4  1993/11/30  17:09:46  mike
 * prototype for compute_lightingValue.
 * 
 * Revision 1.3  1993/11/22  10:50:38  matt
 * Add ifndef around body of file
 * 
 * Revision 1.2  1993/10/06  12:41:25  mike
 * Change prototype for draw_tmap.
 * 
 * Revision 1.1  1993/09/08  17:29:11  mike
 * Initial revision
 * 
 *
 */

#ifndef _TEXMAP_H
#define _TEXMAP_H

#include "fix.h"
#include "3d.h"
#include "gr.h"

#define	NUM_LIGHTING_LEVELS 32
#define MAX_TMAP_VERTS 25
#define MAX_LIGHTING_VALUE	((NUM_LIGHTING_LEVELS-1)*F1_0/NUM_LIGHTING_LEVELS)
#define MIN_LIGHTING_VALUE	(F1_0/NUM_LIGHTING_LEVELS)

#define FIX_RECIP_TABLE_SIZE	641 //increased from 321 to 641, since this res is now quite achievable.. slight fps boost -MM
// -------------------------------------------------------------------------------------------------------
extern fix compute_lightingValue(g3sPoint *vertptr);

// -------------------------------------------------------------------------------------------------------
// This is the main texture mapper call.
//	tmap_num references a texture map defined in Texmap_ptrs.
//	nverts = number of vertices
//	vertbuf is a pointer to an array of vertex pointers
extern void draw_tmap(grsBitmap *bp, int nverts, g3sPoint **vertbuf);

// -------------------------------------------------------------------------------------------------------
// Texture map vertex.
//	The fields r,g,b and l are mutually exclusive.  r,g,b are used for rgb lighting.
//	l is used for intensity based lighting.
typedef struct g3ds_vertex {
	fix	x,y,z;
	fix	u,v;
	fix	x2d,y2d;
	fix	l;
	fix	r,g,b;
} g3ds_vertex;

// A texture map is defined as a polygon with u,v coordinates associated with
// one point in the polygon, and a pair of vectors describing the orientation
// of the texture map in the world, from which the deltas Du_dx, Dv_dy, etc.
// are computed.
typedef struct g3ds_tmap {
	int	nv;			// number of vertices
	g3ds_vertex	verts[MAX_TMAP_VERTS];	// up to 8 vertices, this is inefficient, change
} g3ds_tmap;

// -------------------------------------------------------------------------------------------------------

//	Note:	Not all interpolation method and lighting combinations are supported.
//	Set Interpolation_method to 0/1/2 for linear/linear, perspective/linear, perspective/perspective
extern	int	Interpolation_method;

// Set Lighting_on to 0/1/2 for no lighting/intensity lighting/rgb lighting
extern	int	Lighting_on;

// HACK INTERFACE: how far away the current tSegment (& thus texture) is
extern	int	nCurrentSegDepth;		
extern	int	Max_perspective_depth;		//	Deepest tSegment at which perspective interpolation will be used.
extern	int	Max_linear_depth;				//	Deepest tSegment at which linear interpolation will be used.
extern	int	Max_flat_depth;				//	Deepest tSegment at which flat shading will be used. (If not flat shading, then what?)

//	These are pointers to texture maps.  If you want to render texture map #7, then you will render
//	the texture map defined by Texmap_ptrs[7].
extern	grsBitmap Texmap_ptrs[];
extern	grsBitmap Texmap4_ptrs[];

// Interface for sky renderer
void texture_map_lin_lin_sky(grsBitmap *srcb, g3ds_tmap *t);
void texture_map_lin_lin_sky_v(grsBitmap *srcb, g3ds_tmap *t);
void texture_map_hyp_lin_v(grsBitmap *srcb, g3ds_tmap *t);

void ntexture_map_lighted_linear(grsBitmap *srcb, g3ds_tmap *t);

//	This is the gr_upoly-like interface to the texture mapper which uses texture-mapper compatible
//	(ie, avoids cracking) edge/delta computation.
void gr_upoly_tmap(int nverts, int *vert );

//This is like gr_upoly_tmap() but instead of drawing, it calls the specified
//function with ylr values
void gr_upoly_tmap_ylr(int nverts, int *vert, void (*ylr_func)(int, fix, fix) );

extern int Transparency_on,per2Flag;

//	Set to !0 to enable Sim City 2000 (or Eric's Drive Through, or Eric's Game) specific code.
extern	int	SC2000;

extern int nWindowClipLeft, nWindowClipBot, nWindowClipRight, nWindowClipTop;

// for ugly hack put in to be sure we don't overflow render buffer

#define FIX_XLIMIT	(639 * F1_0)
#define FIX_YLIMIT	(479 * F1_0)

extern void init_interface_vars_to_assembler(void);

#endif

