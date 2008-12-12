/* $Id: clipper.h,v 1.2 2002/07/17 21:55:19 bradleyb Exp $ */
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
 * Header for clipper.c
 * 
 * Old Log:
 *
 * Revision 1.1  1995/05/05  08:50:13  allender
 * Initial revision
 *
 * Revision 1.1  1995/04/17  19:56:58  matt
 * Initial revision
 * 
 * 
 */

#ifndef _CLIPPER_H
#define _CLIPPER_H

extern void free_temp_point(g3sPoint *p);
extern g3sPoint **clip_polygon(g3sPoint **src,g3sPoint **dest,int *nv,g3sCodes *cc);
extern void init_free_points(void);
extern void clip_line(g3sPoint **p0,g3sPoint **p1,ubyte codes_or);

#endif
