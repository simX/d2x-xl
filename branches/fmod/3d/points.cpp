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
	tQuadInt q,qt;

	q.low=q.high=0;
	FixMulAccum (&q,a,b);
	qt = q;
	if (qt.high < 0)
		FixQuadNegate (&qt);
	qt.high *= 2;
	if (qt.low > 0x7fff)
		qt.high++;
	if (qt.high >= c)
		return 0;
	else {
		*r = FixDivQuadLong (q.low,q.high,c);
		return 1;
	}
#endif
}

// -----------------------------------------------------------------------------------
//projects a point

#define VIS_CULLING 1

void G3ProjectPoint (g3sPoint *p)
{
#if VIS_CULLING == 2
if ((p->p3_flags & PF_PROJECTED))
#else
if ((p->p3_flags & PF_PROJECTED) || (p->p3_codes & CC_BEHIND))
#endif
	return;
CFloatVector v;
v.Assign (p->p3_vec);
#if VIS_CULLING == 2
v.Scale (transformation.m_info.scalef);
v [Z] = fabs (v [Z]);
fix x = fix (fxCanvW2 + double (v [X]) * fxCanvW2 / double (v [Z]));
fix y = fix (fxCanvH2 - double (v [Y]) * fxCanvH2 / double (v [Z]));
#if DBG
if ((x < 0) || (x > CCanvas::Current ()->Width ()))
	x = x;
if ((y < 0) || (y > CCanvas::Current ()->Height ()))
	y = y;
#endif
p->p3_screen.x = (x < 0) ? 0 : (x > CCanvas::Current ()->Width ()) ? CCanvas::Current ()->Width () : x;
p->p3_screen.y = (y < 0) ? 0 : (y > CCanvas::Current ()->Height ()) ? CCanvas::Current ()->Height () : y;
#else
p->p3_screen.x = fix (fxCanvW2 + double (v [X]) * fxCanvW2 / double (v [Z]));
p->p3_screen.y = fix (fxCanvH2 - double (v [Y]) * fxCanvH2 / double (v [Z]));
#endif
p->p3_flags |= PF_PROJECTED;
}

// -----------------------------------------------------------------------------------
//from a 2d point, compute the vector through that point
void G3Point2Vec (CFixVector *v,short sx,short sy)
{
	CFixVector h;
	CFixMatrix m;

h [X] =  FixMulDiv (FixDiv ((sx<<16) - xCanvW2, xCanvW2), transformation.m_info.scale [Z], transformation.m_info.scale [X]);
h [Y] = -FixMulDiv (FixDiv ((sy<<16) - xCanvH2, xCanvH2), transformation.m_info.scale [Z], transformation.m_info.scale [Y]);
h [Z] = I2X (1);
CFixVector::Normalize (h);
m = transformation.m_info.view [1].Transpose();
*v = m * h;
}

// -----------------------------------------------------------------------------------

ubyte G3AddDeltaVec (g3sPoint *dest, g3sPoint *src, CFixVector *vDelta)
{
dest->p3_vec = src->p3_vec + *vDelta;
dest->p3_flags = 0;		//not projected
return G3EncodePoint (dest);
}

// -----------------------------------------------------------------------------------
//calculate the depth of a point - returns the z coord of the rotated point
fix G3CalcPointDepth (const CFixVector& pnt)
{
#ifdef _WIN32
	QLONG q = mul64 (pnt [X] - transformation.m_info.pos [X], transformation.m_info.view [0].FVec () [X]);
	q += mul64 (pnt [Y] - transformation.m_info.pos [Y], transformation.m_info.view [0].FVec () [Y]);
	q += mul64 (pnt [Z] - transformation.m_info.pos [Z], transformation.m_info.view [0].FVec () [Z]);
	return (fix) (q >> 16);
#else
	tQuadInt q;

	q.low=q.high=0;
	FixMulAccum (&q, (pnt [X] - transformation.m_info.pos [X]),transformation.m_info.view [0].FVec () [X]);
	FixMulAccum (&q, (pnt [Y] - transformation.m_info.pos [Y]),transformation.m_info.view [0].FVec () [Y]);
	FixMulAccum (&q, (pnt [Z] - transformation.m_info.pos [Z]),transformation.m_info.view [0].FVec () [Z]);
	return FixQuadAdjust (&q);
#endif
}

// -----------------------------------------------------------------------------------
//eof

