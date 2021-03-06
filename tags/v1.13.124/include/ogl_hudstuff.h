//prototypes opengl functions - Added 9/15/99 Matthew Mueller
#ifndef _OGL_HUDSTUFF_H
#define _OGL_HUDSTUFF_H

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>
#endif

#include "inferno.h"

extern grsBitmap *bmpDeadzone;
extern int bHaveDeadzone;

int LoadDeadzone (void);
void FreeDeadzone (void);
void OglDrawMouseIndicator (void);
void OglDrawReticle (int cross, int primary, int secondary);

#endif
