/* $Id: scale.c,v 1.3 2002/07/17 21:55:19 bradleyb Exp $ */
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
static char rcsid[] = "$Id: scale.c,v 1.3 2002/07/17 21:55:19 bradleyb Exp $";
#endif

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "mono.h"
#include "fix.h"
#include "gr.h"
#include "error.h"
#include "rle.h"

#if 0
#define TRANSPARENCY_COLOR 255;
#endif

static int Transparency_color = TRANSPARENCY_COLOR;

#include "scalea.h"

void rls_stretch_scanline( char * source, char * dest, int XDelta, int YDelta );
void rls_stretch_scanline_setup( int XDelta, int YDelta );
void scale_bitmap_asm(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  );
void scale_bitmap_asm_rle(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  );
void scale_bitmap_cc_asm(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  );
void scale_bitmap_cc_asm_rle(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  );

void scale_row_c( ubyte * sbits, ubyte * dbits, int width, fix u, fix du )
{
	int i;
	ubyte c;

	for ( i=0; i<width; i++ )	{
		c = sbits[ f2i(u) ];

		if ( c != Transparency_color )
			*dbits = c;
			
		dbits++;
		u += du;
	}
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (FixMulDiv((x)-(x0),(y1)-(y0),(x1)-(x0))+(y0))

// Scales bitmap, bp, into vertbuf[0] to vertbuf[1]
void ScaleBitmap(grsBitmap *bp, grsPoint *vertbuf ,int orientation)
{
	grsBitmap * dbp = &grdCurCanv->cvBitmap;
	fix x0, y0, x1, y1;
	fix u0, v0, u1, v1;
	fix clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	fix clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	fix xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;
	int dtemp;
	// Set initial variables....

	x0 = vertbuf[0].x; y0 = vertbuf[0].y;
	x1 = vertbuf[2].x; y1 = vertbuf[2].y;

	xmin = 0; ymin = 0;
	xmax = i2f(dbp->bmProps.w)-fl2f(.5); ymax = i2f(dbp->bmProps.h)-fl2f(.5);

	u0 = i2f(0); v0 = i2f(0);
	u1 = i2f(bp->bmProps.w-1); v1 = i2f(bp->bmProps.h-1);

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<0 ) || (x0>=xmax) ) return;
	if ( (y1<0 ) || (y0>=ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = f2i(clipped_x0); dx1 = f2i(clipped_x1);
	dy0 = f2i(clipped_y0); dy1 = f2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	Assert( dx0>=0 );
	Assert( dy0>=0 );
	Assert( dx1<dbp->bmProps.w );
	Assert( dy1<dbp->bmProps.h );
	Assert( f2i(u0)<=f2i(u1) );
	Assert( f2i(v0)<=f2i(v1) );
	Assert( f2i(u0)>=0 );
	Assert( f2i(v0)>=0 );
	Assert( u1<i2f(bp->bmProps.w) );
	Assert( v1<i2f(bp->bmProps.h) );

	dtemp = f2i(clipped_u1)-f2i(clipped_u0);

	if ( bp->bmProps.flags & BM_FLAG_RLE )	{
		if ( (dtemp < (f2i(clipped_x1)-f2i(clipped_x0))) && (dtemp>0) )
			scale_bitmap_cc_asm_rle(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
		else
			scale_bitmap_asm_rle(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
	} else {
		if ( (dtemp < (f2i(clipped_x1)-f2i(clipped_x0))) && (dtemp>0) )
			scale_bitmap_cc_asm(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
		else
			scale_bitmap_asm(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
	}
}


void scale_bitmap_c(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix u, v, du, dv;
	int x, y;
	ubyte * sbits, * dbits;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		sbits = &source_bmp->bmTexBuf[source_bmp->bmProps.rowSize*f2i(v)];
		dbits = &dest_bmp->bmTexBuf[dest_bmp->bmProps.rowSize*y+x0];
		u = u0; 
		v += dv;
		for (x=x0; x<=x1; x++ )			{
			*dbits++ = sbits[ u >> 16 ];
			u += du;
		}
	}
}

void scale_bitmap_asm(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix du, dv, v;
	int y;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		scale_row_asm_transparent( &source_bmp->bmTexBuf[source_bmp->bmProps.rowSize*f2i(v)], &dest_bmp->bmTexBuf[dest_bmp->bmProps.rowSize*y+x0], x1-x0+1, u0, du );
		v += dv;
	}
}

ubyte scale_rle_data[1600];

void decode_row( grsBitmap * bmp, int y )
{
	int i, offset=4+bmp->bmProps.h;
	
	for (i=0; i<y; i++ )
		offset += bmp->bmTexBuf[4+i];
	gr_rle_decode( &bmp->bmTexBuf[offset], scale_rle_data );
}

void scale_bitmap_asm_rle(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix du, dv, v;
	int y, last_row=-1;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		if ( f2i(v) != last_row )	{
			last_row = f2i(v);
			decode_row( source_bmp, last_row );
		}
		scale_row_asm_transparent( scale_rle_data, &dest_bmp->bmTexBuf[dest_bmp->bmProps.rowSize*y+x0], x1-x0+1, u0, du );
		v += dv;
	}
}


void scale_bitmap_cc_asm(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix dv, v;
	int y;

	dv = (v1-v0) / (y1-y0);
		
	rls_stretch_scanline_setup( (int)(x1-x0), f2i(u1)-f2i(u0) );
	if ( scale_ydelta_minus_1 < 1 ) return;
	rls_do_cc_setup_asm();

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		scale_source_ptr = &source_bmp->bmTexBuf[source_bmp->bmProps.rowSize*f2i(v)+f2i(u0)];
		scale_dest_ptr = &dest_bmp->bmTexBuf[dest_bmp->bmProps.rowSize*y+x0];
		scale_do_cc_scanline();
		v += dv;
	}
}

void scale_bitmap_cc_asm_rle(grsBitmap *source_bmp, grsBitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix dv, v;
	int y, last_row = -1;

	dv = (v1-v0) / (y1-y0);
		
	rls_stretch_scanline_setup( (int)(x1-x0), f2i(u1)-f2i(u0) );
	if ( scale_ydelta_minus_1 < 1 ) return;
	rls_do_cc_setup_asm();

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		if ( f2i(v) != last_row )	{
			last_row = f2i(v);
			decode_row( source_bmp, last_row );
		}
		//scale_source_ptr = &source_bmp->bmTexBuf[source_bmp->bmProps.rowSize*f2i(v)+f2i(u0)];
		scale_source_ptr = &scale_rle_data[f2i(u0)];
		scale_dest_ptr = &dest_bmp->bmTexBuf[dest_bmp->bmProps.rowSize*y+x0];
		scale_do_cc_scanline();
		v += dv;
	}
}



// Run-length slice bitmap scan line stretcher 

void DrawHorizontalRun(char *ScreenPtr, int RunLength, int Color)
{
   int i;

   for (i=0; i<RunLength; i++)
      *ScreenPtr++ = Color;
}

void rls_stretch_scanline( char * source, char * dest, int XDelta, int YDelta )
{
	   int AdjUp, AdjDown, ErrorTerm;
   	int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;

      /* X major line */
      /* Minimum # of pixels in a run in this line */
      WholeStep = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      AdjUp = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      AdjDown = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
     /* If there're an odd number of pixels per run, we have 1 pixel that can't
     be allocated to either the initial or last partial run, so we'll add 0.5
     to error term so this pixel will be handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += YDelta;
      }
      /* Draw the first, partial run of pixels */
		//if ( *source != Transparency_color )
      	rep_stosb(dest, InitialPixelCount, *source );
		dest += InitialPixelCount;
		source++;

      /* Draw all full runs */
      for (i=0; i<(YDelta-1); i++)
      {
         RunLength = WholeStep;  /* run is at least this long */

         /* Advance the error term and add an extra pixel if the error term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */

			//if ( *source != Transparency_color )
	      	rep_stosb(dest, RunLength, *source );
			dest += RunLength;
			source++;

      }

      /* Draw the final run of pixels */
		//if ( *source != Transparency_color )
	      rep_stosb(dest, FinalPixelCount, *source );

      return;
}




void rls_stretch_scanline_setup( int XDelta, int YDelta )
{
		scale_trans_color = Transparency_color & 0xFF;
		scale_ydelta_minus_1 = YDelta - 1;

      /* X major line */
      /* Minimum # of pixels in a run in this line */
      scale_whole_step = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      scale_adj_up = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      scale_adjDown = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      scale_error_term = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      scale_initial_pixelCount = (scale_whole_step / 2) + 1;
      scale_final_pixelCount = scale_initial_pixelCount;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((scale_adj_up == 0) && ((scale_whole_step & 0x01) == 0))
      {
         scale_initial_pixelCount--;
      }
     /* If there're an odd number of pixels per run, we have 1 pixel that can't
     be allocated to either the initial or last partial run, so we'll add 0.5
     to error term so this pixel will be handled by the normal full-run loop */
      if ((scale_whole_step & 0x01) != 0)
      {
         scale_error_term += YDelta;
      }

}

