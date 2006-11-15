/* $Id: globvars.c,v 1.4 2002/07/17 21:55:19 bradleyb Exp $ */
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
 * Global variables for 3d
 * 
 * Old Log:
 *
 * Revision 1.2  1995/09/13  11:30:47  allender
 * added fxCanvW2 and vxCanvH2 for PPC implementation
 *
 * Revision 1.1  1995/05/05  08:50:48  allender
 * Initial revision
 *
 * Revision 1.1  1995/04/17  04:24:57  matt
 * Initial revision
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: globvars.c,v 1.4 2002/07/17 21:55:19 bradleyb Exp $";
#endif

#include "3d.h"
#include "globvars.h"

tViewInfo	viewInfo;

int			nCanvasWidth;		//the actual width
int			nCanvasHeight;		//the actual height

fix			xCanvW2;				//fixed-point width/2
fix			xCanvH2;				//fixed-point height/2

#ifdef __powerc
double		fxCanvW2;
double		fxCanvH2;
#endif

//vertex buffers for polygon drawing and clipping
g3sPoint * Vbuf0[MAX_POINTS_IN_POLY];
g3sPoint *Vbuf1[MAX_POINTS_IN_POLY];

//list of 2d coords
fix polyVertList [MAX_POINTS_IN_POLY*2];



