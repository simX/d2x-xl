//prototypes opengl functions - Added 9/15/99 Matthew Mueller
#ifndef _OGL_BITMAP_H
#define _OGL_BITMAP_H

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>
#endif

#include "ogl_defs.h"
#include "ogl_lib.h"

bool OglUBitMapMC (int x, int y, int dw, int dh, grsBitmap *bm, grsColor *c, int scale, int orient);
bool OglUBitBltI (int dw,int dh,int dx,int dy, int sw, int sh, int sx, int sy, 
						grsBitmap * src, grsBitmap * dest, int bMipMaps, int bTransp, float fAlpha);
bool OglUBitBltToLinear (int w,int h,int dx,int dy, int sx, int sy, grsBitmap * src, grsBitmap * dest);
bool OglUBitBltCopy (int w,int h,int dx,int dy, int sx, int sy, grsBitmap * src, grsBitmap * dest);

//------------------------------------------------------------------------------

static inline int OglUBitBlt (int w,int h,int dx,int dy, int sx, int sy, 
										grsBitmap *src, grsBitmap *dest, int bTransp)
{
return OglUBitBltI (w, h, dx, dy, w, h, sx, sy, src, dest, 0, bTransp, 1.0f);
}

static inline int OglUBitMapM (int x, int y,grsBitmap *bm)
{
return OglUBitMapMC (x, y, 0, 0, bm, NULL, F1_0, 0);
}

//------------------------------------------------------------------------------

#endif
