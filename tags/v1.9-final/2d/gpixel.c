/* $Id: gpixel.c,v 1.5 2002/10/10 18:55:32 btb Exp $ */
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

#include "u_mem.h"

#include "gr.h"
#include "grdef.h"
#ifdef __DJGPP__
#include "modex.h"
#include "vesa.h"
#endif

unsigned char gr_ugpixel( grsBitmap * bitmap, int x, int y )
{
#ifdef __DJGPP__
	switch(bitmap->bm_props.nType)
	{
	case BM_LINEAR:
#endif
		return bitmap->bm_texBuf[ bitmap->bm_props.rowsize*y + x ];
#ifdef __DJGPP__
	case BM_MODEX:
		x += bitmap->bm_props.x;
		y += bitmap->bm_props.y;
		gr_modex_setplane( x & 3 );
		return gr_video_memory[(bitmap->bm_props.rowsize * y) + (x/4)];
	case BM_SVGA:
		{
		unsigned int offset;
		offset = (unsigned int)bitmap->bm_texBuf + (unsigned int)bitmap->bm_props.rowsize * y + x;
		gr_vesa_setpage( offset >> 16 );
		return gr_video_memory[offset & 0xFFFF];
		}
	}
	return 0;
#endif
}

unsigned char gr_gpixel( grsBitmap * bitmap, int x, int y )
{
	if ((x<0) || (y<0) || (x>=bitmap->bm_props.w) || (y>=bitmap->bm_props.h)) return 0;
#ifdef __DJGPP__
	switch(bitmap->bm_props.nType)
	{
	case BM_LINEAR:
#endif
		return bitmap->bm_texBuf[ bitmap->bm_props.rowsize*y + x ];
#ifdef __DJGPP__
	case BM_MODEX:
		x += bitmap->bm_props.x;
		y += bitmap->bm_props.y;
		gr_modex_setplane( x & 3 );
		return gr_video_memory[(bitmap->bm_props.rowsize * y) + (x/4)];
	case BM_SVGA:
		{
		unsigned int offset;
		offset = (unsigned int)bitmap->bm_texBuf + (unsigned int)bitmap->bm_props.rowsize * y + x;
		gr_vesa_setpage( offset >> 16 );
		return gr_video_memory[offset & 0xFFFF];
		}
	}
	return 0;
#endif
}
