/* $Id: draw.c, v 1.4 2002/07/17 21:55:19 bradleyb Exp $ */
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
 * Drawing routines
 * 
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: draw.c, v 1.4 2002/07/17 21:55:19 bradleyb Exp $";
#endif

#include "error.h"

#include "3d.h"
#include "inferno.h"
#include "globvars.h"
#include "texmap.h"
#include "clipper.h"
#include "ogl_init.h"

#if 1
tmap_drawer_fp tmap_drawer_ptr = draw_tmap;
flat_drawer_fp flat_drawer_ptr = gr_upoly_tmap;
line_drawer_fp line_drawer_ptr = gr_line;
#else
void (*tmap_drawer_ptr) (grs_bitmap *bm, int nv, g3s_point **vertlist) = draw_tmap;
void (*flat_drawer_ptr) (int nv, int *vertlist) = gr_upoly_tmap;
int (*line_drawer_ptr) (fix x0, fix y0, fix x1, fix y1) = gr_line;
#endif

//------------------------------------------------------------------------------
//specifies 2d drawing routines to use instead of defaults.  Passing
//NULL for either or both restores defaults
void g3_set_special_render (tmap_drawer_fp tmap_drawer, flat_drawer_fp flat_drawer, line_drawer_fp line_drawer)
{
tmap_drawer_ptr = (tmap_drawer)?tmap_drawer:draw_tmap;
flat_drawer_ptr = (flat_drawer)?flat_drawer:gr_upoly_tmap;
line_drawer_ptr = (line_drawer)?line_drawer:gr_line;
}
#ifndef OGL
//deal with a clipped line
bool must_clip_line (g3s_point *p0, g3s_point *p1, ubyte codes_or)
{
	bool ret;

	if ((p0->p3_flags&PF_TEMP_POINT) || (p1->p3_flags&PF_TEMP_POINT))

		ret = 0;		//line has already been clipped, so give up

	else {

		clip_line (&p0, &p1, codes_or);

		ret = G3DrawLine (p0, p1);
	}

	//d_free temp points

	if (p0->p3_flags & PF_TEMP_POINT)
		free_temp_point (p0);

	if (p1->p3_flags & PF_TEMP_POINT)
		free_temp_point (p1);

	return ret;
}

//------------------------------------------------------------------------------
//draws a line. takes two points.  returns true if drew
bool G3DrawLine (g3s_point *p0, g3s_point *p1)
{
	ubyte codes_or;

	if (p0->p3_codes & p1->p3_codes)
		return 0;

	codes_or = p0->p3_codes | p1->p3_codes;

	if (codes_or & CC_BEHIND)
		return must_clip_line (p0, p1, codes_or);

	if (! (p0->p3_flags&PF_PROJECTED))
		G3ProjectPoint (p0);

	if (p0->p3_flags&PF_OVERFLOW)
		return must_clip_line (p0, p1, codes_or);


	if (! (p1->p3_flags&PF_PROJECTED))
		G3ProjectPoint (p1);

	if (p1->p3_flags&PF_OVERFLOW)
		return must_clip_line (p0, p1, codes_or);

	return (bool) (*line_drawer_ptr) (p0->p3_sx, p0->p3_sy, p1->p3_sx, p1->p3_sy);
}
#endif

//------------------------------------------------------------------------------
//returns true if a plane is facing the viewer. takes the unrotated surface 
//normal of the plane, and a point on it.  The normal need not be normalized
bool G3CheckNormalFacing (vms_vector *pv, vms_vector *pnorm)
{
vms_vector v;
return (VmVecDot (VmVecSub (&v, &viewInfo.position, pv), pnorm) > 0);
}

//------------------------------------------------------------------------------

bool DoFacingCheck (vms_vector *norm, g3s_point **vertlist, vms_vector *p)
{
if (norm) {		//have normal
	Assert (norm->x || norm->y || norm->z);
	return G3CheckNormalFacing (p, norm);
	}
else {	//normal not specified, so must compute
	vms_vector tempv;
	//get three points (rotated) and compute normal
	VmVecPerp (&tempv, &vertlist[0]->p3_vec, &vertlist[1]->p3_vec, &vertlist[2]->p3_vec);
	return (VmVecDot (&tempv, &vertlist[1]->p3_vec) < 0);
	}
}

//------------------------------------------------------------------------------
//like G3DrawPoly (), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to 
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like G3CheckNormalFacing () plus
//G3DrawPoly ().
//returns -1 if not facing, 1 if off screen, 0 if drew
bool G3CheckAndDrawPoly (int nv, g3s_point **pointlist, vms_vector *norm, vms_vector *pnt)
{
	if (DoFacingCheck (norm, pointlist, pnt))
		return G3DrawPoly (nv, pointlist);
	else
		return 255;
}

//------------------------------------------------------------------------------

bool G3CheckAndDrawTMap (
	int nv, g3s_point **pointlist, uvl *uvl_list, grs_bitmap *bm, vms_vector *norm, vms_vector *pnt)
{
if (DoFacingCheck (norm, pointlist, pnt))
	return !G3DrawTexPoly (nv, pointlist, uvl_list, bm, 1);
else
	return 0;
}

//------------------------------------------------------------------------------
//deal with face that must be clipped
bool MustClipFlatFace (int nv, g3s_codes cc)
{
	int i;
        bool ret=0;
	g3s_point **bufptr;

	bufptr = clip_polygon (Vbuf0, Vbuf1, &nv, &cc);

	if (nv>0 && ! (cc.or&CC_BEHIND) && !cc.and) {

		for (i=0;i<nv;i++) {
			g3s_point *p = bufptr[i];
	
			if (! (p->p3_flags&PF_PROJECTED))
				G3ProjectPoint (p);
	
			if (p->p3_flags&PF_OVERFLOW) {
				ret = 1;
				goto free_points;
			}

			polyVertList[i*2]   = p->p3_sx;
			polyVertList[i*2+1] = p->p3_sy;
		}
	
		 (*flat_drawer_ptr) (nv, (int *)polyVertList);
	}
	else 
		ret=1;

	//d_free temp points
free_points:
	;

	for (i=0;i<nv;i++)
		if (Vbuf1[i]->p3_flags & PF_TEMP_POINT)
			free_temp_point (Vbuf1[i]);

//	Assert (free_point_num==0);

	return ret;
}

//------------------------------------------------------------------------------

#if (! (defined (D1XD3D) || defined (OGL)))
//draw a flat-shaded face.
//returns 1 if off screen, 0 if drew
bool G3DrawPoly (int nv, g3s_point **pointlist)
{
	int i;
	g3s_point **bufptr, *p;
	g3s_codes cc;

cc.or = 0; 
cc.and = 0xff;
bufptr = Vbuf0;
for (i=0;i<nv;i++) {
	p = bufptr [i] = pointlist [i];
	cc.and &= p->p3_codes;
	cc.or |= p->p3_codes;
	}
if (cc.and)
	return 1;	//all points off screen
if (cc.or)
	return MustClipFlatFace (nv, cc);
//now make list of 2d coords (& check for overflow)
for (i=0;i<nv;i++) {
	p = bufptr[i];
	if (! (p->p3_flags&PF_PROJECTED))
		G3ProjectPoint (p);
	if (p->p3_flags&PF_OVERFLOW)
		return MustClipFlatFace (nv, cc);
	polyVertList[i*2] = p->p3_sx;
	polyVertList[i*2+1] = p->p3_sy;
	}
(*flat_drawer_ptr) (nv, (int *)polyVertList);
return 0;	//say it drew
}

//------------------------------------------------------------------------------

bool must_clip_tmap_face (int nv, g3s_codes cc, grs_bitmap *bm);

//draw a texture-mapped face.
//returns 1 if off screen, 0 if drew
bool G3DrawTexPoly (int nv, g3s_point **pointlist, uvl *uvl_list, grs_bitmap *bm)
{
	int i;
	g3s_point **bufptr, *p;
	g3s_codes cc;

cc.or = 0; 
cc.and = 0xff;
bufptr = Vbuf0;
for (i=0;i<nv;i++) {
	p = bufptr[i] = pointlist[i];
	cc.and &= p->p3_codes;
	cc.or  |= p->p3_codes;
	p->p3_u = uvl_list[i].u;
	p->p3_v = uvl_list[i].v;
	p->p3_l = uvl_list[i].l;
	p->p3_flags |= PF_UVS + PF_LS;
	}
if (cc.and)
	return 1;	//all points off screen
if (cc.or)
	return must_clip_tmap_face (nv, cc, bm);
//now make list of 2d coords (& check for overflow)
for (i=0;i<nv;i++) {
	p = bufptr[i];
	if (! (p->p3_flags&PF_PROJECTED))
		G3ProjectPoint (p);
	if (p->p3_flags&PF_OVERFLOW) {
		Int3 ();		//should not overflow after clip
		return 255;
		}
	}
(*tmap_drawer_ptr) (bm, nv, bufptr);
return 0;	//say it drew
}
#endif

//------------------------------------------------------------------------------

bool must_clip_tmap_face (int nv, g3s_codes cc, grs_bitmap *bm)
{
	g3s_point **bufptr;
	int i;

	bufptr = clip_polygon (Vbuf0, Vbuf1, &nv, &cc);

	if (nv && ! (cc.or&CC_BEHIND) && !cc.and) {

		for (i=0;i<nv;i++) {
			g3s_point *p = bufptr[i];

			if (! (p->p3_flags&PF_PROJECTED))
				G3ProjectPoint (p);
	
			if (p->p3_flags&PF_OVERFLOW) {
				Int3 ();		//should not overflow after clip
				goto free_points;
			}
		}

		 (*tmap_drawer_ptr) (bm, nv, bufptr);
	}

free_points:
	;

	for (i=0;i<nv;i++)
		if (bufptr[i]->p3_flags & PF_TEMP_POINT)
			free_temp_point (bufptr[i]);

//	Assert (free_point_num==0);
	
	return 0;

}

//------------------------------------------------------------------------------

#ifndef __powerc
int CheckMulDiv (fix *r, fix a, fix b, fix c);
#endif

#ifndef OGL
//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
int G3DrawSphere (g3s_point *pnt, fix rad)
{
	if (! (pnt->p3_codes & CC_BEHIND)) {

		if (! (pnt->p3_flags & PF_PROJECTED))
			G3ProjectPoint (pnt);

		if (! (pnt->p3_codes & PF_OVERFLOW)) {
			fix r2, t;

			r2 = fixmul (rad, viewInfo.scale.x);
#ifndef __powerc
			if (CheckMulDiv (&t, r2, xCanvW2, pnt->p3_z))
				return gr_disk (pnt->p3_sx, pnt->p3_sy, t);
#else
			if (pnt->p3_z == 0)
				return 0;
			return gr_disk (pnt->p3_sx, pnt->p3_sy, fl2f (( (f2fl (r2) * fxCanvW2) / f2fl (pnt->p3_z)));
#endif
		}
	}

	return 0;
}
#endif

//------------------------------------------------------------------------------
//eof
