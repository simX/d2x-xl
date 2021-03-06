/* $Id: points.c,v 1.5 2002/10/2819:49:15 btb Exp $ */
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
COPYRIGHT 1993-1998PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: points.c,v 1.5 2002/10/2819:49:15 btb Exp $";
#endif

#include "3d.h"
#include "globvars.h"

// -----------------------------------------------------------------------------------
//checks for overflow & divides if ok, fillig in r
//returns true if div is ok, else false
#ifdef _WIN32
inline 
#endif
int CheckMulDiv (fix *r, fix a, fix b, fix c)
{
#ifdef _WIN32
	QLONG	q;
	if (!c)
		return 0;
	q = mul64 (a, b) / (QLONG) c;
	if ((q > 0x7fffffff) || (q < -0x7fffffff))
		return 0;
	*r = (fix) q;
	return 1;
#else
	quadint q,qt;

	q.low=q.high=0;
	fixmulaccum (&q,a,b);
	qt = q;
	if (qt.high < 0)
		fixquadnegate (&qt);
	qt.high *= 2;
	if (qt.low > 0x7fff)
		qt.high++;
	if (qt.high >= c)
		return 0;
	else {
		*r = fixdivquadlong (q.low,q.high,c);
		return 1;
	}
#endif
}

// -----------------------------------------------------------------------------------
//projects a point
void G3ProjectPoint (g3sPoint *p)
{
	vmsVector	v;
#ifndef __powerc
	fix tx, ty;

if ((p->p3_flags & PF_PROJECTED) || (p->p3_codes & CC_BEHIND))
	return;
v = p->p3_vec;
v.p.x = FixMul (v.p.x, viewInfo.scale.p.x);
v.p.y = FixMul (v.p.y, viewInfo.scale.p.y);
v.p.z = FixMul (v.p.z, viewInfo.scale.p.z);
if (CheckMulDiv (&tx, v.p.x, xCanvW2, v.p.z) && 
	 CheckMulDiv (&ty, v.p.y, xCanvH2, v.p.z)) {
	p->p3_screen.x = xCanvW2 + tx;
	p->p3_screen.y = xCanvH2 - ty;
	p->p3_flags |= PF_PROJECTED;
	}
else
	p->p3_flags |= PF_OVERFLOW;
#else
double fz;
if ((p->p3_flags & PF_PROJECTED) || (p->p3_codes & CC_BEHIND))
	return;
if (p->p3_z <= 0)	{
	p->p3_flags |= PF_OVERFLOW;
	return;
	}
fz = f2fl (p->p3_z);
p->p3_screen.x = fl2f (fxCanvW2 + (f2fl (v.p.x) * fxCanvW2 / fz);
p->p3_screen.y = fl2f (fxCanvH2 - (f2fl (v.p.y) * fxCanvH2 / fz);
p->p3_flags |= PF_PROJECTED;
#endif
}

// -----------------------------------------------------------------------------------
//from a 2d point, compute the vector through that point
void G3Point2Vec (vmsVector *v,short sx,short sy)
{
	vmsVector tempv;
	vmsMatrix tempm;

tempv.p.x =  FixMulDiv (FixDiv ((sx<<16) - xCanvW2,xCanvW2),viewInfo.scale.p.z,viewInfo.scale.p.x);
tempv.p.y = -FixMulDiv (FixDiv ((sy<<16) - xCanvH2,xCanvH2),viewInfo.scale.p.z,viewInfo.scale.p.y);
tempv.p.z = f1_0;
VmVecNormalize (&tempv);
VmCopyTransposeMatrix (&tempm,&viewInfo.view [1]);
VmVecRotate (v,&tempv,&tempm);
}

// -----------------------------------------------------------------------------------
//delta rotation functions
vmsVector *G3RotateDeltaX (vmsVector *dest,fix dx)
{
	dest->p.x = FixMul (viewInfo.view [0].rVec.p.x,dx);
	dest->p.y = FixMul (viewInfo.view [0].uVec.p.x,dx);
	dest->p.z = FixMul (viewInfo.view [0].fVec.p.x,dx);

	return dest;
}

// -----------------------------------------------------------------------------------

vmsVector *G3RotateDeltaY (vmsVector *dest,fix dy)
{
	dest->p.x = FixMul (viewInfo.view [0].rVec.p.y,dy);
	dest->p.y = FixMul (viewInfo.view [0].uVec.p.y,dy);
	dest->p.z = FixMul (viewInfo.view [0].fVec.p.y,dy);

	return dest;
}

// -----------------------------------------------------------------------------------

vmsVector *G3RotateDeltaZ (vmsVector *dest,fix dz)
{
	dest->p.x = FixMul (viewInfo.view [0].rVec.p.z,dz);
	dest->p.y = FixMul (viewInfo.view [0].uVec.p.z,dz);
	dest->p.z = FixMul (viewInfo.view [0].fVec.p.z,dz);

	return dest;
}

// -----------------------------------------------------------------------------------

vmsVector *G3RotateDeltaVec (vmsVector *dest,vmsVector *src)
{
	return VmVecRotate (dest,src,&viewInfo.view [0]);
}

// -----------------------------------------------------------------------------------

ubyte G3AddDeltaVec (g3sPoint *dest, g3sPoint *src, vmsVector *vDelta)
{
VmVecAdd (&dest->p3_vec, &src->p3_vec, vDelta);
dest->p3_flags = 0;		//not projected
return G3EncodePoint (dest);
}

// -----------------------------------------------------------------------------------
//calculate the depth of a point - returns the z coord of the rotated point
fix G3CalcPointDepth (vmsVector *pnt)
{
#ifdef _WIN32
	QLONG q = mul64 (pnt->p.x - viewInfo.pos.p.x, viewInfo.view [0].fVec.p.x);
	q += mul64 (pnt->p.y - viewInfo.pos.p.y, viewInfo.view [0].fVec.p.y);
	q += mul64 (pnt->p.z - viewInfo.pos.p.z, viewInfo.view [0].fVec.p.z);
	return (fix) (q >> 16);
#else
	quadint q;

	q.low=q.high=0;
	fixmulaccum (&q, (pnt->p.x - viewInfo.pos.p.x),viewInfo.view [0].fVec.p.x);
	fixmulaccum (&q, (pnt->p.y - viewInfo.pos.p.y),viewInfo.view [0].fVec.p.y);
	fixmulaccum (&q, (pnt->p.z - viewInfo.pos.p.z),viewInfo.view [0].fVec.p.z);
	return fixquadadjust (&q);
#endif
}

// -----------------------------------------------------------------------------------
//eof

