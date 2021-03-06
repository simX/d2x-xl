/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <malloc.h>
# include <SDL.h>
#endif

#include "descent.h"
#include "error.h"
#include "maths.h"
#include "light.h"
#include "dynlight.h"
#include "headlight.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_texture.h"
#include "ogl_shader.h"
#include "ogl_render.h"
#include "ogl_fastrender.h"
#include "ogl_tmu.h"
#include "texmerge.h"
#include "transprender.h"
#include "gameseg.h"
#include "automap.h"

#if DBG
#	define G3_BUFFER_FACES	0
#else
#	define G3_BUFFER_FACES	0
#endif

CRenderFaceDrawerP g3FaceDrawer = G3DrawFaceArrays;

//------------------------------------------------------------------------------

#define FACE_BUFFER_SIZE			1000
#define FACE_BUFFER_INDEX_SIZE	(FACE_BUFFER_SIZE * 4 * 4)

typedef struct tFaceBuffer {
	CBitmap	*bmBot;
	CBitmap	*bmTop;
	short			nFaces;
	short			nElements;
	int			bTextured;
	int			index [FACE_BUFFER_INDEX_SIZE];
} tFaceBuffer;

static tFaceBuffer faceBuffer = {NULL, NULL, 0, 0, 0, {0}};

//------------------------------------------------------------------------------

extern GLhandleARB headlightShaderProgs [2][4];
extern GLhandleARB perPixelLightingShaderProgs [MAX_LIGHTS_PER_PIXEL][4];
extern GLhandleARB gsShaderProg [2][3];

//------------------------------------------------------------------------------
// this is a work around for OpenGL's per vertex light interpolation
// rendering a quad is always started with the brightest vertex

void G3BuildQuadIndex (CSegFace *faceP, int *indexP)
{
#if G3_BUFFER_FACES
int nIndex = faceP->nIndex;
for (int i = 0; i < 4; i++)
	*indexP++ = nIndex++;
#else
	tRgbaColorf	*pc = FACES.color + faceP->nIndex;
	float			l, lMax = 0;
	int			i, j, nIndex;
	int			iMax = 0;

for (i = 0; i < 4; i++, pc++) {
	l = pc->red + pc->green + pc->blue;
	if (lMax < l) {
		lMax = l;
		iMax = i;
		}
	}
nIndex = faceP->nIndex;
if (!iMax) {
	for (i = 0; i < 4; i++)
		*indexP++ = nIndex++;
	}
else {
	for (i = 0, j = iMax; i < 4; i++, j %= 4)
		*indexP++ = nIndex + j++;
	}
#endif
}

//------------------------------------------------------------------------------

void G3FlushFaceBuffer (int bForce)
{
#if G3_BUFFER_FACES
if (faceBuffer.nFaces && (bForce || (faceBuffer.nFaces >= FACE_BUFFER_SIZE))) {
	if (gameStates.render.bFullBright)
		glColor3f (1,1,1);
	try {
		if (!gameStates.render.bTriangleMesh)
			glDrawElements (GL_TRIANGLE_FAN, faceBuffer.nElements, GL_UNSIGNED_INT, faceBuffer.index);
		else
			glDrawElements (GL_TRIANGLES, faceBuffer.nElements, GL_UNSIGNED_INT, faceBuffer.index);
		}
	catch(...) {
		PrintLog ("error calling glDrawElements (%d, %d) in G3FlushFaceBuffer\n", gameStates.render.bTriangleMesh ? "GL_TRIANGLES" : "GL_TRIANGLE_FAN");
		}
	faceBuffer.nFaces =
	faceBuffer.nElements = 0;
	//gameStates.render.history.nShader = -1;
	}
#endif
}

//------------------------------------------------------------------------------

void G3FillFaceBuffer (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bTextured)
{
#if DBG
if (!gameOpts->render.debug.bTextures)
	return;
#endif
#if 0
if (!gameStates.render.bTriangleMesh) {
	if (gameStates.render.bFullBright)
		glColor3f (1,1,1);
	OglDrawArrays (GL_TRIANGLE_FAN, faceP->nIndex, 4);
	}
else
#endif
	{
	int	i = faceP->nIndex,
			j = gameStates.render.bTriangleMesh ? faceP->nTris * 3 : 4;

#if DBG
		if (i == nDbgVertex)
			nDbgVertex = nDbgVertex;
		if (i + j > int (FACES.vertices.Length ())) {
			PrintLog ("invalid vertex index %d in G3FillFaceBuffer\n");
			return;
			}
#endif
	if (!gameStates.render.bTriangleMesh || (faceBuffer.bmBot != bmBot) || (faceBuffer.bmTop != bmTop) || (faceBuffer.nElements + j > FACE_BUFFER_INDEX_SIZE)) {
		if (faceBuffer.nFaces)
			G3FlushFaceBuffer (1);
		faceBuffer.bmBot = bmBot;
		faceBuffer.bmTop = bmTop;
		}
	faceBuffer.bTextured = bTextured;
	for (; j; j--)
		faceBuffer.index [faceBuffer.nElements++] = i++;
	faceBuffer.nFaces++;
	}
}

//------------------------------------------------------------------------------

int G3SetupShader (CSegFace *faceP, int bDepthOnly, int bColorKey, int bMultiTexture, int bTextured, int bColored, tRgbaColorf *colorP)
{
	int	nType, nShader = gameStates.render.history.nShader;

if (!ogl.m_states.bShadersOk || (gameStates.render.nType == 4))
	return -1;
#if DBG
if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
nType = bColorKey ? 3 : bMultiTexture ? 2 : bTextured;
if (!bColored && gameOpts->render.automap.bGrayOut)
	nShader = G3SetupGrayScaleShader (nType, colorP);
else if ((gameStates.render.nType != 4) && faceP && (gameStates.render.bPerPixelLighting == 2))
	nShader = G3SetupPerPixelShader (faceP, bDepthOnly, nType, false);
else if (gameStates.render.bHeadlights && !bDepthOnly)
	nShader = lightManager.Headlights ().SetupShader (nType, lightmapManager.HaveLightmaps (), colorP);
else if (bColorKey || bMultiTexture)
	nShader = G3SetupTexMergeShader (bColorKey, bColored, nType);
else if (gameStates.render.history.nShader >= 0) {
	gameData.render.nShaderChanges++;
	glUseProgramObject (0);
	nShader = -1;
	}
ogl.ClearError (0);
gameStates.render.history.nType = nType;
return gameStates.render.history.nShader = nShader;
}

//------------------------------------------------------------------------------

#if DBG

void RenderWireFrame (CSegFace *faceP, int bTextured)
{
if (gameOpts->render.debug.bWireFrame) {
	if ((nDbgFace < 0) || (faceP - FACES.faces == nDbgFace)) {
		tFaceTriangle	*triP = FACES.tris + faceP->nTriIndex;
		ogl.DisableClientState (GL_COLOR_ARRAY);
		if (bTextured)
			glDisable (GL_TEXTURE_2D);
		glColor3f (1.0f, 0.5f, 0.0f);
		glLineWidth (6);
		glBegin (GL_LINE_LOOP);
		for (int i = 0; i < 4; i++)
			glVertex3fv (reinterpret_cast<GLfloat*> (gameData.segs.fVertices + faceP->index [i]));
		glEnd ();
		if (gameStates.render.bTriangleMesh) {
			glLineWidth (2);
			glColor3f (1,1,1);
			for (int i = 0; i < faceP->nTris; i++, triP++)
				OglDrawArrays (GL_LINE_LOOP, triP->nIndex, 3);
			glLineWidth (1);
			}
		if (gameOpts->render.debug.bDynamicLight)
			ogl.EnableClientState (GL_COLOR_ARRAY);
		if (bTextured)
			glEnable (GL_TEXTURE_2D);
		}
	glLineWidth (1);
	}
}

#endif

//------------------------------------------------------------------------------

static inline void G3SetBlendMode (CSegFace *faceP)
{
if (faceP->bAdditive)
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
else if (faceP->bTransparent)
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
else
	glBlendFunc (GL_ONE, GL_ZERO);
}

//------------------------------------------------------------------------------

int G3SetRenderStates (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bDepthOnly, int bTextured, int bColorKey, int bColored, bool bForce = false)
{
PROF_START
if (bTextured) {
	bool bStateChange = false;
	CBitmap *mask = NULL;
	if (bForce || (bmBot != gameStates.render.history.bmBot)) {
		bStateChange = true;
		gameStates.render.history.bmBot = bmBot;
		{INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, lightmapManager.Buffer (), 1, 0);}
		}
	if (bForce || (bmTop != gameStates.render.history.bmTop)) {
		bStateChange = true;
		gameStates.render.history.bmTop = bmTop;
		if (bmTop) {
			{INIT_TMU (InitTMU1, GL_TEXTURE1, bmTop, lightmapManager.Buffer (), 1, 0);}
			}
		else {
			ogl.SelectTMU (GL_TEXTURE1, true);
			OglBindTexture (0);
			glDisable (GL_TEXTURE_2D);
			mask = NULL;
			}
		}
	mask = (bColorKey && gameStates.render.textures.bHaveMaskShader) ? bmTop->Mask () : NULL;
	if (bForce || (mask != gameStates.render.history.bmMask)) {
		bStateChange = true;
		gameStates.render.history.bmMask = mask;
		if (mask) {
			{INIT_TMU (InitTMU2, GL_TEXTURE2, mask, lightmapManager.Buffer (), 2, 0);}
			ogl.EnableClientState (GL_TEXTURE_COORD_ARRAY, GL_TEXTURE2);
			}
		else {
			ogl.SelectTMU (GL_TEXTURE2, true);
			OglBindTexture (0);
			glDisable (GL_TEXTURE_2D);
			bColorKey = 0;
			}
		}
	if (bColored != gameStates.render.history.bColored) {
		bStateChange = true;
		gameStates.render.history.bColored = bColored;
		}
	if (bStateChange) {
		gameData.render.nStateChanges++;
		if (!gameStates.render.bFullBright)
			G3SetupShader (faceP, bDepthOnly, bColorKey, bmTop != NULL, bmBot != NULL, bColored, bmBot ? NULL : &faceP->color);
		}
	}
else {
	gameStates.render.history.bmBot = NULL;
	ogl.SelectTMU (GL_TEXTURE0, true);
	OglBindTexture (0);
	glDisable (GL_TEXTURE_2D);
	}
PROF_END(ptRenderStates)
return 1;
}

//------------------------------------------------------------------------------

int G3SetRenderStatesLM (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bDepthOnly, int bTextured, int bColorKey, int bColored)
{
PROF_START
if (bTextured) {
	bool bStateChange = false;
	CBitmap *mask = NULL;
	if (bmBot != gameStates.render.history.bmBot) {
		bStateChange = true;
		gameStates.render.history.bmBot = bmBot;
		{INIT_TMU (InitTMU1, GL_TEXTURE1, bmBot, lightmapManager.Buffer (), 1, 0);}
		}
	if (bmTop != gameStates.render.history.bmTop) {
		bStateChange = true;
		gameStates.render.history.bmTop = bmTop;
		if (bmTop) {
			{INIT_TMU (InitTMU2, GL_TEXTURE2, bmTop, lightmapManager.Buffer (), 1, 0);}
			}
		else {
			ogl.SelectTMU (GL_TEXTURE2, true);
			OglBindTexture (0);
			mask = NULL;
			}
		}
	if (bColorKey)
		mask = (bColorKey && gameStates.render.textures.bHaveMaskShader) ? bmTop->Mask () : NULL;
	if (mask != gameStates.render.history.bmMask) {
		bStateChange = true;
		gameStates.render.history.bmMask = mask;
		if (mask) {
			{INIT_TMU (InitTMU3, GL_TEXTURE3, mask, lightmapManager.Buffer (), 2, 0);}
			ogl.EnableClientState (GL_TEXTURE_COORD_ARRAY, GL_TEXTURE2);
			}
		else {
			ogl.SelectTMU (GL_TEXTURE3, true);
			OglBindTexture (0);
			bColorKey = 0;
			}
		}
	if (bColored != gameStates.render.history.bColored) {
		gameStates.render.history.bColored = bColored;
		}
	if (bStateChange)
		gameData.render.nStateChanges++;
	}
else {
	gameStates.render.history.bmBot = NULL;
	ogl.SelectTMU (GL_TEXTURE1, true);
	OglBindTexture (0);
	glDisable (GL_TEXTURE_2D);
	}
PROF_END(ptRenderStates)
return 1;
}

//------------------------------------------------------------------------------

void G3SetupMonitor (CSegFace *faceP, CBitmap *bmTop, int bTextured, int bLightmaps)
{
ogl.SelectTMU ((bmTop ? GL_TEXTURE1 : GL_TEXTURE0) + bLightmaps, true);
if (bTextured)
	OglTexCoordPointer (2, GL_FLOAT, 0, faceP->pTexCoord - faceP->nIndex);
else {
	glDisable (GL_TEXTURE_2D);
	OglBindTexture (0);
	}
}

//------------------------------------------------------------------------------

void G3ResetMonitor (CBitmap *bmTop, int bLightmaps)
{
if (bmTop) {
	ogl.SelectTMU (GL_TEXTURE1 + bLightmaps, true);
	glEnable (GL_TEXTURE_2D);
	OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<GLvoid*> (FACES.ovlTexCoord.Buffer ()));
	//gameStates.render.history.bmTop = NULL;
	}
else {
	ogl.SelectTMU (GL_TEXTURE0 + bLightmaps, true);
	OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<GLvoid*> (FACES.texCoord.Buffer ()));
	//gameStates.render.history.bmBot = NULL;
	}
}

//------------------------------------------------------------------------------

static inline int G3FaceIsTransparent (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop)
{
if (!bmBot)
	return faceP->color.alpha < 1.0f;
if (faceP->bTransparent || faceP->bAdditive)
	return 1;
if (bmBot->Flags () & BM_FLAG_SEE_THRU)
	return 0;
if (!(bmBot->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)))
	return 0;
if (!bmTop)
	return 1;
if (bmTop->Flags () & BM_FLAG_SEE_THRU)
	return 0;
if (bmTop->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
	return 1;
return 0;
}

//------------------------------------------------------------------------------

static inline int G3FaceIsColored (CSegFace *faceP)
{
return !automap.m_bDisplay || automap.m_visited [0][faceP->nSegment] || !gameOpts->render.automap.bGrayOut;
}

//------------------------------------------------------------------------------

int G3DrawFaceArrays (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
PROF_START
	int			bColored, bTransparent, bColorKey = 0, bMonitor = 0;
#if G3_BUFFER_FACES
	int			nBlendMode;
#endif

#if DBG
if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide))) {
	if (bDepthOnly)
		nDbgSeg = nDbgSeg;
	else
		nDbgSeg = nDbgSeg;
	}
#endif

if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot) {
	bmBot = bmBot->Override (-1);
#if 0 //DBG
	if (strstr (bmBot->Name (), "rock327"))
		bmBot = bmBot;
	else
		return 0;
#endif
	}
bTransparent = G3FaceIsTransparent (faceP, bmBot, bmTop);
if (bDepthOnly) {
	if (bTransparent || faceP->bOverlay)
		return 0;
	bColored = 0;
	}
else {
	bColored = G3FaceIsColored (faceP);
	bMonitor = (faceP->nCamera >= 0);
	if (bmTop) {
		if ((bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
			bmTop = bmTop->CurFrame ();
			}
		else
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
		}
	gameStates.render.history.nType = bColorKey ? 3 : (bmTop != NULL) ? 2 : (bmBot != NULL);
	if (bTransparent && (gameStates.render.nType < 4) && !bMonitor) {
		faceP->nRenderType = gameStates.render.history.nType;
		faceP->bColored = bColored;
		transparencyRenderer.AddFace (faceP);
		return 0;
		}
	}

#if G3_BUFFER_FACES
if (bDepthOnly)
	G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
else {
	if (bMonitor)
		G3FlushFaceBuffer (1);
	else {
		nBlendMode = faceP->bAdditive ? 2 : faceP->bTransparent ? 1 : 0;
		if (nBlendMode != gameStates.render.history.nBlendMode) {
			gameStates.render.history.nBlendMode = nBlendMode;
			G3FlushFaceBuffer (1);
			G3SetBlendMode (faceP);
			}
		G3FillFaceBuffer (faceP, bmBot, bmTop, bTextured);
		}
	if (faceBuffer.nFaces <= 1)
		G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored, true);
	}
#else
G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
#endif
ogl.m_states.iLight = 0;
//G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
if (bDepthOnly) {
	if (gameStates.render.bTriangleMesh)
		OglDrawArrays (GL_TRIANGLES, faceP->nIndex, faceP->nTris * 3);
	else
		OglDrawArrays (GL_TRIANGLE_FAN, faceP->nIndex, 4);
	return 0;
	}

gameData.render.nTotalFaces++;
#if DBG
RenderWireFrame (faceP, bTextured);
if (!gameOpts->render.debug.bTextures)
	return 0;
#endif

if (bMonitor)
	G3SetupMonitor (faceP, bmTop, bTextured, 0);
#if G3_BUFFER_FACES
else
	return 0;
#endif
if (gameStates.render.bTriangleMesh) {
#if USE_RANGE_ELEMENTS
	 {
		GLsizei nElements = faceP->nTris * 3;
		glDrawRangeElements (GL_TRIANGLES, faceP->vertIndex [0], faceP->vertIndex [nElements - 1], nElements, GL_UNSIGNED_INT, faceP->vertIndex);
		}
#else
		OglDrawArrays (GL_TRIANGLES, faceP->nIndex, faceP->nTris * 3);
#endif
	}
else {
#if 0
	int	index [4];

	G3BuildQuadIndex (faceP, index);
	glDrawElements (GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, index);
#else
	OglDrawArrays (GL_TRIANGLE_FAN, faceP->nIndex, 4);
#endif
	}

if (bMonitor)
	G3ResetMonitor (bmTop, 0);
PROF_END(ptRenderFaces)
return 0;
}

//------------------------------------------------------------------------------

#if GEOMETRY_VBOS

inline void RenderFacePP (CSegFace *faceP)
{
if (FACES.vboDataHandle) {
	int i = faceP->nIndex;
	glDrawRangeElements (GL_TRIANGLES, i, i + 5, 6, GL_UNSIGNED_SHORT, G3_BUFFER_OFFSET (i * sizeof (ushort)));
	}
else
	OglDrawArrays (GL_TRIANGLES, faceP->nIndex, 6);
}

#else

#define RenderFacePP(_faceP)	OglDrawArrays (GL_TRIANGLES, (_faceP)->nIndex, 6)

#endif

//------------------------------------------------------------------------------

int G3DrawFaceArraysPPLM (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
PROF_START
	int			bColored, bTransparent, bColorKey = 0, bMonitor = 0;

#if DBG
if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide))) {
	if (bDepthOnly)
		nDbgSeg = nDbgSeg;
	else
		nDbgSeg = nDbgSeg;
	}
#endif

if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = bmBot->Override (-1);
bTransparent = G3FaceIsTransparent (faceP, bmBot, bmTop);

if (bDepthOnly) {
	if (bTransparent || faceP->bOverlay)
		return 0;
	bColored = 0;
	}
else {
	bColored = G3FaceIsColored (faceP);
	bMonitor = (faceP->nCamera >= 0);
#if DBG
	if (bmTop)
		bmTop = bmTop;
#endif
	if (bmTop) {
		if ((bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
			bmTop = bmTop->CurFrame ();
			}
		else
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
		}
	gameStates.render.history.nType = bColorKey ? 3 : (bmTop != NULL) ? 2 : (bmBot != NULL);
	if (bTransparent && (gameStates.render.nType < 4) && !bMonitor) {
		faceP->nRenderType = gameStates.render.history.nType;
		faceP->bColored = bColored;
		transparencyRenderer.AddFace (faceP);
		return 0;
		}
	}

ogl.m_states.iLight = 0;
if (gameStates.render.bFullBright)
	G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
else
	G3SetRenderStatesLM (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
if (bDepthOnly) {
	RenderFacePP (faceP);
	PROF_END(ptRenderFaces)
	return 1;
	}
#if DBG
RenderWireFrame (faceP, bTextured);
if (!gameOpts->render.debug.bTextures)
	return 0;
#endif
G3SetBlendMode (faceP);
if (bMonitor)
	G3SetupMonitor (faceP, bmTop, bTextured, 1);
gameData.render.nTotalFaces++;
if (!bColored) {
	G3SetupGrayScaleShader (gameStates.render.history.nType, &faceP->color);
	RenderFacePP (faceP);
	}
else if (gameStates.render.bFullBright) {
	if (gameStates.render.history.nType > 1)
		G3SetupTexMergeShader (bColorKey, bColored, gameStates.render.history.nType);
	else if (gameStates.render.history.nShader != -1) {
		glUseProgramObject (0);
		gameStates.render.history.nShader = -1;
		gameData.render.nShaderChanges++;
		}
	glColor3f (1,1,1);
	RenderFacePP (faceP);
	}
else {
	bool bAdditive = false;
	for (;;) {
		G3SetupPerPixelShader (faceP, 0, gameStates.render.history.nType, false);
		RenderFacePP (faceP);
		if ((ogl.m_states.iLight >= ogl.m_states.nLights) ||
			 (ogl.m_states.iLight >= gameStates.render.nMaxLightsPerFace))
			break;
		if (!bAdditive) {
			bAdditive = true;
			//if (!faceP->bTransparent)
				glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glDepthFunc (GL_EQUAL);
			glDepthMask (0);
			}
		}
#if 0 //headlights will be rendered in a separate pass to decrease shader changes
	if (gameStates.render.bHeadlights) {
		if (!bAdditive) {
			bAdditive = true;
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			}
		lightManager.Headlights ().SetupShader (gameStates.render.history.nType, 1, bmBot ? NULL : &faceP->color);
		OglDrawArrays (GL_TRIANGLES, faceP->nIndex, 6);
		}
#endif
	if (bAdditive) {
		glDepthFunc (GL_LEQUAL);
		glDepthMask (1);
		}
	}

if (bMonitor)
	G3ResetMonitor (bmTop, 1);
PROF_END(ptRenderFaces)
return 0;
}

//------------------------------------------------------------------------------

int G3DrawFaceArraysLM (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
PROF_START
	int			bColored, bTransparent, bColorKey = 0, bMonitor = 0;

#if DBG
if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide))) {
	if (bDepthOnly)
		nDbgSeg = nDbgSeg;
	else
		nDbgSeg = nDbgSeg;
	}
#endif

if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = bmBot->Override (-1);
bTransparent = G3FaceIsTransparent (faceP, bmBot, bmTop);

if (bDepthOnly) {
	if (bTransparent || faceP->bOverlay)
		return 0;
	bColored = 0;
	}
else {
	bColored = G3FaceIsColored (faceP);
	bMonitor = (faceP->nCamera >= 0);
#if DBG
	if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
	if (bmTop)
		bmTop = bmTop;
#endif
	if (bmTop) {
		if ((bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
			bmTop = bmTop->CurFrame ();
			}
		else
			bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
		}
	gameStates.render.history.nType = bColorKey ? 3 : (bmTop != NULL) ? 2 : (bmBot != NULL);
	if (bTransparent && (gameStates.render.nType < 4) && !bMonitor) {
		faceP->nRenderType = gameStates.render.history.nType;
		faceP->bColored = bColored;
		transparencyRenderer.AddFace (faceP);
		return 0;
		}
	}

ogl.m_states.iLight = 0;
if (gameStates.render.bFullBright)
	G3SetRenderStates (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
else
	G3SetRenderStatesLM (faceP, bmBot, bmTop, bDepthOnly, bTextured, bColorKey, bColored);
if (bDepthOnly) {
	RenderFacePP (faceP);
	PROF_END(ptRenderFaces)
	return 1;
	}
#if DBG
RenderWireFrame (faceP, bTextured);
if (!gameOpts->render.debug.bTextures)
	return 0;
#endif
G3SetBlendMode (faceP);
if (bMonitor)
	G3SetupMonitor (faceP, bmTop, bTextured, 1);
gameData.render.nTotalFaces++;
if (!bColored) {
	G3SetupGrayScaleShader (gameStates.render.history.nType, &faceP->color);
	RenderFacePP (faceP);
	}
else if (gameStates.render.bFullBright) {
	if (gameStates.render.history.nType > 1)
		G3SetupTexMergeShader (bColorKey, bColored, gameStates.render.history.nType);
	else if (gameStates.render.history.nShader != -1) {
		glUseProgramObject (0);
		gameStates.render.history.nShader = -1;
		gameData.render.nShaderChanges++;
		}
	glColor3f (1,1,1);
	RenderFacePP (faceP);
	}
else {
	G3SetupLightmapShader (faceP, 0, gameStates.render.history.nType, false);
	RenderFacePP (faceP);
	}

if (bMonitor)
	G3ResetMonitor (bmTop, 1);
PROF_END(ptRenderFaces)
return 0;
}

//------------------------------------------------------------------------------

int G3DrawHeadlights (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
	int			bColorKey = 0, bMonitor = 0;

if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = bmBot->Override (-1);
if (G3FaceIsTransparent (faceP, bmBot, bmTop) && !(bMonitor || bmTop))
	return 0;
bMonitor = (faceP->nCamera >= 0);
if (bmTop) {
	if ((bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
		bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
		bmTop = bmTop->CurFrame ();
		}
	else
		bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
	}
gameStates.render.history.nType = bColorKey ? 3 : (bmTop != NULL) ? 2 : (bmBot != NULL);
G3SetRenderStates (faceP, bmBot, bmTop, 0, 1, bColorKey, 1);
if (bMonitor)
	G3SetupMonitor (faceP, bmTop, bTextured, 1);
gameData.render.nTotalFaces++;
lightManager.Headlights ().SetupShader (gameStates.render.history.nType, 1, bmBot ? NULL : &faceP->color);
RenderFacePP (faceP);

if (bMonitor)
	G3ResetMonitor (bmTop, 1);
return 0;
}

//------------------------------------------------------------------------------

int G3DrawHeadlightsPPLM (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
	int			bColorKey = 0, bMonitor = 0;

#if DBG
if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = bmBot->Override (-1);
if (G3FaceIsTransparent (faceP, bmBot, bmTop) && !(bMonitor || bmTop))
	return 0;
bMonitor = (faceP->nCamera >= 0);
if (bmTop) {
	if ((bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
		bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
		bmTop = bmTop->CurFrame ();
		}
	else
		bColorKey = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
	}
gameStates.render.history.nType = bColorKey ? 3 : (bmTop != NULL) ? 2 : (bmBot != NULL);
G3SetRenderStatesLM (faceP, bmBot, bmTop, 0, 1, bColorKey, 1);
if (bMonitor)
	G3SetupMonitor (faceP, bmTop, bTextured, 1);
gameData.render.nTotalFaces++;
lightManager.Headlights ().SetupShader (gameStates.render.history.nType, 1, bmBot ? NULL : &faceP->color);
OglDrawArrays (GL_TRIANGLES, faceP->nIndex, 6);
if (bMonitor)
	G3ResetMonitor (bmTop, 1);
return 0;
}

//------------------------------------------------------------------------------
//eof
