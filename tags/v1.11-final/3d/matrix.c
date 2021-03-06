/* $Id: matrix.c,v 1.4 2002/07/17 21:55:19 bradleyb Exp $ */
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
 
#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: matrix.c,v 1.4 2002/07/17 21:55:19 bradleyb Exp $";
#endif

#include "3d.h"
#include "globvars.h"
#include "ogl_defs.h"
#include "inferno.h"
#include "oof.h"

void ScaleMatrix (int bOglScale);

//------------------------------------------------------------------------------
//set view from x,y,z & p,b,h, xZoom.  Must call one of g3_setView_*() 
void G3SetViewAngles (vmsVector *vPos, vmsAngVec *mOrient, fix xZoom)
{
viewInfo.zoom = xZoom;
viewInfo.pos = *vPos;
VmAngles2Matrix (&viewInfo.view [0], mOrient);
VmVecFixToFloat (&viewInfo.posf, &viewInfo.pos);
VmsMatToFloat (viewInfo.viewf, viewInfo.view);
ScaleMatrix (1);
}

//------------------------------------------------------------------------------
//set view from x,y,z, viewer matrix, and xZoom.  Must call one of g3_setView_*() 
void G3SetViewMatrix (vmsVector *vPos, vmsMatrix *mOrient, fix xZoom, int bOglScale)
{
viewInfo.zoom = xZoom;
viewInfo.glZoom = (float) xZoom / 65536.0f;
if (vPos) {
	viewInfo.pos = *vPos;
	VmVecFixToFloat (&viewInfo.posf, &viewInfo.pos);
	OOF_VecVms2Gl (viewInfo.glPosf, &viewInfo.pos);
	}
if (mOrient) {
	viewInfo.view [0] = *mOrient;
	VmsMatToFloat (viewInfo.viewf, viewInfo.view);
	OOF_MatVms2Gl (OOF_GlIdent (viewInfo.glViewf), viewInfo.view);
	}
ScaleMatrix (bOglScale);
}

//------------------------------------------------------------------------------
//performs aspect scaling on global view matrix
void ScaleMatrix (int bOglScale)
{
	viewInfo.view [1] = viewInfo.view [0];		//so we can use unscaled if we want
	viewInfo.viewf [1] = viewInfo.viewf [0];		//so we can use unscaled if we want

viewInfo.scale = viewInfo.windowScale;
if (viewInfo.zoom <= f1_0) 		//xZoom in by scaling z
	viewInfo.scale.p.z = FixMul (viewInfo.scale.p.z, viewInfo.zoom);
else {			//xZoom out by scaling x&y
	fix s = FixDiv (f1_0, viewInfo.zoom);

	viewInfo.scale.p.x = FixMul (viewInfo.scale.p.x, s);
	viewInfo.scale.p.y = FixMul (viewInfo.scale.p.y, s);
	}
//now scale matrix elements
if (bOglScale)
	glScalef (f2fl (viewInfo.scale.p.x), f2fl (viewInfo.scale.p.y), -f2fl (viewInfo.scale.p.z));
else {
	//VmVecScale (&viewInfo.view [0].rVec, viewInfo.scale.p.x);
	//VmVecScale (&viewInfo.view [0].uVec, viewInfo.scale.p.y);
	VmVecScale (&viewInfo.view [0].fVec, -viewInfo.scale.p.z);
	glScalef (1, 1, 1);
	}
VmsMatToFloat (viewInfo.viewf, viewInfo.view);
}

//------------------------------------------------------------------------------
//eof
