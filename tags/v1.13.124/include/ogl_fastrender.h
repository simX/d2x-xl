//prototypes opengl functions - Added 9/15/99 Matthew Mueller
#ifndef _OGL_FASTRENDER_H
#define _OGL_FASTRENDER_H

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>
#endif

#include "ogl_defs.h"

#define G3_BUFFER_FACES	1

//------------------------------------------------------------------------------

int G3DrawFaceSimple (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
int G3DrawFaceArrays (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
int G3DrawFaceArraysPPLM (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
int G3DrawHeadlights (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
int G3DrawHeadlightsPPLM (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
int G3DrawFaceArraysLM (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);
void G3FlushFaceBuffer (int bForce);
int G3SetupPerPixelShader (grsFace *faceP, int bDepthOnly, int nType, bool bHeadlight);
int G3SetupLightmapShader (grsFace *faceP, int bDepthOnly, int nType, bool bHeadlight);
int G3SetupHeadlightShader (int nType, int bLightmaps, tRgbaColorf *colorP);
int G3SetupTexMergeShader (int bColorKey, int bColored, int nType);
int G3SetupGrayScaleShader (int nType, tRgbaColorf *colorP);
int G3SetupShader (grsFace *faceP, int bDepthOnly, int bColorKey, int bMultiTexture, int bTextured, int bColored, tRgbaColorf *colorP);
void InitGrayScaleShader (void);

typedef int (*tG3FaceDrawerP) (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly);

extern tG3FaceDrawerP g3FaceDrawer;

//------------------------------------------------------------------------------

static inline int FaceIsAdditive (grsFace *faceP)
{
return (int) ((faceP->bAdditive == 1) ? !(faceP->bmBot && faceP->bmBot->bmFromPog) : faceP->bAdditive);
}

//------------------------------------------------------------------------------

static inline int G3DrawFace (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, 
										 int bBlend, int bTextured, int bDepthOnly, int bVertexArrays, int bPPLM)
{
return bPPLM ? 
		 G3DrawFaceArraysPPLM (faceP, bmBot, bmTop, bBlend, bTextured, bDepthOnly) :
		 bVertexArrays ?
			G3DrawFaceArrays (faceP, bmBot, bmTop, bBlend, bTextured, bDepthOnly) :
			G3DrawFaceSimple (faceP, bmBot, bmTop, bBlend, bTextured, bDepthOnly);
}

//------------------------------------------------------------------------------

static inline void SetFaceDrawer (int nType)
{
if (nType < 0)
	nType = gameStates.render.bPerPixelLighting;
if (nType == 2)
	g3FaceDrawer = G3DrawFaceArraysPPLM;
else if (nType == 1)
	g3FaceDrawer = G3DrawFaceArraysLM;
else
	g3FaceDrawer = G3DrawFaceArrays;
}

//------------------------------------------------------------------------------

#endif
