/* $Id: ogl.c, v 1.14 204/05/11 23:15:55 btb Exp $ */
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

#include "inferno.h"
#include "error.h"
#include "maths.h"
#include "lighting.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_texture.h"
#include "ogl_shader.h"
#include "ogl_render.h"
#include "ogl_fastrender.h"
#include "ogl_tmu.h"
#include "texmerge.h"
#include "transprender.h"

//------------------------------------------------------------------------------

#define FACE_BUFFER_SIZE	100

typedef struct tFaceBuffer {
	grsBitmap	*bmP;
	short			nFaces;
	short			nElements;
	int			bTextured;
	ushort		index [FACE_BUFFER_SIZE * 4];
} tFaceBuffer;

static tFaceBuffer faceBuffer = {NULL, 0, 0, 0, {0}};

//------------------------------------------------------------------------------

void G3FlushFaceBuffer (int bForce)
{
#if G3_BUFFER_FACES
if ((faceBuffer.nFaces && bForce) || (faceBuffer.nFaces >= FACE_BUFFER_SIZE)) {
	glDrawElements (GL_QUADS, faceBuffer.nElements, GL_UNSIGNED_SHORT, faceBuffer.index);
	faceBuffer.nFaces = 
	faceBuffer.nElements = 0;
	}
#endif
}

//------------------------------------------------------------------------------

void G3FillFaceBuffer (grsFace *faceP, int bTextured)
{
	int	i, j = faceP->nIndex;

faceBuffer.bmP = faceP->bmBot;
faceBuffer.bTextured = bTextured;
for (i = 4; i; i--)
	faceBuffer.index [faceBuffer.nElements++] = j++;
faceBuffer.nFaces++;
}

//------------------------------------------------------------------------------

int OglVertexLight (int nLights, int nPass, int iVertex)
{
	tShaderLight	*psl;
	int				iLightSource, iLight;
	tRgbaColorf		color = {1,1,1,1};
	GLenum			hLight;

if (nLights < 0)
	nLights = gameData.render.lights.dynamic.shader.nActiveLights [iVertex];
iLightSource = gameData.render.lights.dynamic.shader.nActiveLights [iVertex] - nLights;
if (nLights) {
	if (nPass) {
		glActiveTexture (GL_TEXTURE1);
		glBlendFunc (GL_ONE, GL_ONE);
		glActiveTexture (GL_TEXTURE0);
		glBlendFunc (GL_ONE, GL_ONE);
		}
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
#if 0
else
	glColor4f (0,0,0,0);
#endif
for (iLight = 0; (iLight < 8) && nLights; iLight++, nLights--, iLightSource++) { 
	psl = gameData.render.lights.dynamic.shader.activeLights [iVertex][iLightSource];
#if 0
	if (psl->nType > 1) {
		iLight--;
		continue;
		}	
#endif
	hLight = GL_LIGHT0 + iLight;
	glEnable (hLight);
	color.red = psl->color.c.r * psl->brightness;
	color.green = psl->color.c.g * psl->brightness;
	color.blue = psl->color.c.b * psl->brightness;
//			sprintf (szLightSources + strlen (szLightSources), "%d ", (psl->nObject >= 0) ? -psl->nObject : psl->nSegment);
	glLightfv (hLight, GL_POSITION, (GLfloat *) psl->pos);
	glLightfv (hLight, GL_DIFFUSE, (GLfloat *) &color);
	glLightfv (hLight, GL_SPECULAR, (GLfloat *) &color);
	if (psl->nType == 2) {
		glLightf (hLight, GL_CONSTANT_ATTENUATION, 0.1f);
		glLightf (hLight, GL_LINEAR_ATTENUATION, 0.01f);
		glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.001f);
		}
	else {
		glLightf (hLight, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf (hLight, GL_LINEAR_ATTENUATION, 0.1f);
		glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.04f);
		}
	}	
for (; iLight < 8; iLight++)
	glDisable (GL_LIGHT0 + iLight);
return nLights;
}

//------------------------------------------------------------------------------

void G3SetupFaceLight (grsFace *faceP, int bTextured)
{
	int	i, nLights = gameData.render.lights.dynamic.shader.nActiveLights [0];

for (i = 0; i < 4; i++) {
	if (i) {
		memcpy (gameData.render.lights.dynamic.shader.activeLights + i, 
					gameData.render.lights.dynamic.shader.activeLights,
					nLights * sizeof (void *));
		gameData.render.lights.dynamic.shader.nActiveLights [i] = nLights;
		}
	SetNearestVertexLights (faceP->index [i], 0, 0, 1, i);
	}
OglEnableLighting (0);
}

//------------------------------------------------------------------------------

extern GLhandleARB lightingShaderProgs [12];

int G3SetupShader (int bColorKey, int bMultiTexture, int bTextured, tRgbaColorf *colorP)
{
	int oglRes, nLights, nShader = gameStates.render.history.nShader, bUpdateShader = 0;

if (gameData.render.lights.dynamic.headLights.nLights) {
	nLights = IsCoopGame ? 4 : IsMultiGame ? 8 : 1;
	nShader = (nLights & ~1) + (bColorKey ? 2 : bMultiTexture) + bTextured + 4;
	if (nShader != gameStates.render.history.nShader) {
		glUseProgramObject (tmProg = lightingShaderProgs [nShader - 4]);
		if (bTextured) {
			glUniform1i (glGetUniformLocation (tmProg, "btmTex"), 0);
			if (bColorKey || bMultiTexture) {
				glUniform1i (glGetUniformLocation (tmProg, "topTex"), 1);
				if (bColorKey)
					glUniform1i (glGetUniformLocation (tmProg, "maskTex"), 2);
				}
			}
		glUniform1f (glGetUniformLocation (tmProg, "grAlpha"), 1.0f);
		glUniform1f (glGetUniformLocation (tmProg, "aspect"), (float) grdCurScreen->scWidth / (float) grdCurScreen->scHeight);
		glUniform1f (glGetUniformLocation (tmProg, "cutOff"), 0.5f);
		glUniform1f (glGetUniformLocation (tmProg, "spotExp"), 8.0f);
		glUniform3fv (glGetUniformLocation (tmProg, "lightPos"), nLights, 
						  (GLfloat *) gameData.render.lights.dynamic.headLights.pos);
		glUniform3fv (glGetUniformLocation (tmProg, "lightDir"), nLights, 
						  (GLfloat *) gameData.render.lights.dynamic.headLights.dir);
		glUniform1fv (glGetUniformLocation (tmProg, "brightness"), nLights, 
						  (GLfloat *) gameData.render.lights.dynamic.headLights.brightness);
		glUniform4fv (glGetUniformLocation (tmProg, "matColor"), 1, (GLfloat *) colorP);
		oglRes = glGetError ();
		}
	}
else if (bColorKey || bMultiTexture) {
	nShader = bColorKey ? 2 : 0;
	if (nShader != gameStates.render.history.nShader)
		glUseProgramObject (tmProg = tmShaderProgs [nShader]);
	glUniform1i (glGetUniformLocation (tmProg, "btmTex"), 0);
	glUniform1i (glGetUniformLocation (tmProg, "topTex"), 1);
	glUniform1i (glGetUniformLocation (tmProg, "maskTex"), 2);
	glUniform1f (glGetUniformLocation (tmProg, "grAlpha"), 1.0f);
	}
else if (gameStates.render.history.nShader > 0) {
	glUseProgramObject (0);
	nShader = -1;
	}
gameStates.render.history.nShader = nShader;
return nShader;
}

//------------------------------------------------------------------------------

bool G3DrawFaceSimple (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
	int			h, i, j, nTextures, nRemainingLights, nLights [4], nPass = 0;
	int			bColorKey, bOverlay, bTransparent, 
					bMonitor = 0, 
					bLighting = GEO_LIGHTING && !bDepthOnly, 
					bMultiTexture = 0;
	grsBitmap	*bmMask = NULL, *bmP [2];
	tTexCoord2f	*texCoordP, *ovlTexCoordP;

#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = BmOverride (bmBot, -1);
bTransparent = faceP->bTransparent || (bmBot && (bmBot->bmProps.flags & BM_FLAG_TRANSPARENT));

if (bDepthOnly) {
	if (bTransparent || faceP->bOverlay)
		return 0;
	bOverlay = 0;
	}
else {
	bMonitor = gameStates.render.bUseCameras && (faceP->nCamera >= 0);
#ifdef _DEBUG
	if (bMonitor)
		faceP = faceP;
#endif
	if (bTransparent && !(bMonitor || bmTop || faceP->bSplit || faceP->bOverlay)) {
#ifdef _DEBUG
		if (gameOpts->render.bDepthSort > 0) 
#endif
			{
#if 0//def _DEBUG
			if ((faceP->nSegment != nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide != nDbgSide)))
				return 1;
#endif
			RIAddFace (faceP);
			return 0;
			}
		}
	}
if (bTextured) {
	if (bmTop && !bMonitor) {
		if ((bmTop = BmOverride (bmTop, -1)) && BM_FRAMES (bmTop)) {
			bColorKey = (bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
			bmTop = BM_CURFRAME (bmTop);
			}
		else
			bColorKey = (bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
		bOverlay = (bColorKey && gameStates.ogl.bGlTexMerge) ? 1 : -1;
		bMultiTexture = GEO_LIGHTING && (bOverlay > 0);
		}
	else
		bOverlay = 0;
	if ((bmBot != gameStates.render.history.bmBot) || 
		 (bmTop != gameStates.render.history.bmTop) || 
		 (bOverlay != gameStates.render.history.bOverlay)) {
		if (bOverlay > 0) {	
			bmMask = gameStates.render.textures.bHaveMaskShader ? BM_MASK (bmTop) : NULL;
			// set base texture
			if (bmBot != gameStates.render.history.bmBot) {
				INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, 0);
				gameStates.render.history.bmBot = bmBot;
				}
			// set overlay texture
			if (bmTop != gameStates.render.history.bmTop) {
				INIT_TMU (InitTMU1, GL_TEXTURE1, bmTop, 0);
				gameStates.render.history.bmTop = bmTop;
				}
			if (bmMask) {
				INIT_TMU (InitTMU2, GL_TEXTURE2, bmMask, 0);
				glUniform1i (glGetUniformLocation (tmProg, "maskTex"), 2);
				}
			gameStates.render.history.bmMask = bmMask;
			bmTop = NULL;
			G3SetupShader (bColorKey, 1, 1, &faceP->color);
			}
		else {
			if (gameStates.render.history.bOverlay > 0) {
				glUseProgramObject (0);
				if (gameStates.render.history.bmMask) {
					glActiveTexture (GL_TEXTURE2);
					OGL_BINDTEX (0);
					gameStates.render.history.bmMask = NULL;
					}
				}
			gameStates.render.history.bmTop = NULL;
			//if (bmBot != gameStates.render.history.bmBot) 
				{
				INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, 0);
				}
			}
		G3SetupShader (0, bMultiTexture, bmBot != NULL, &faceP->color);
		}
	gameStates.render.history.bOverlay = bOverlay;
	}
else {
	bOverlay = 0;
	glDisable (GL_TEXTURE_2D);
	}
if (!bBlend)
	glDisable (GL_BLEND);

#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	if (bDepthOnly)
		nDbgSeg = nDbgSeg;
	else
		nDbgSeg = nDbgSeg;
#endif
bmP [0] = bmBot;
bmP [1] = bmTop;
nTextures = bmP [1] ? 2 : 1;
for (h = 0; h < nTextures; ) {
	gameStates.render.history.bmBot = bmP [h];
	nPass = 0;
	nLights [0] = nLights [1] = nLights [2] = nLights [3] = -1;
	nRemainingLights = bLighting;
	texCoordP = bMonitor ? faceP->pTexCoord : h ? gameData.segs.faces.ovlTexCoord : gameData.segs.faces.texCoord;
	if (bMultiTexture)
		ovlTexCoordP = bMonitor ? faceP->pTexCoord : gameData.segs.faces.ovlTexCoord;
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	do {
		j = faceP->nIndex;
		if (nPass)
			nRemainingLights = 0;
		glBegin (GL_TRIANGLE_FAN);
		for (i = 0; i < 4; i++, j++) {
	#ifdef _DEBUG
			if (faceP->index [i] == nDbgVertex)
				faceP = faceP;
	#endif
			if (bLighting && nPass) {
				if (!(nLights [i] = OglVertexLight (nLights [i], nPass, i)))
					glColor4f (0,0,0,0);
				else {
					nRemainingLights += nLights [i];
					if (bTextured)
						glColor4f (1,1,1,1);
					else
						glColor4fv ((GLfloat *) (gameData.segs.faces.color + faceP->nIndex));
					}
				}
			else
				glColor4fv ((GLfloat *) (gameData.segs.faces.color + j));
			if (bMultiTexture) {
				glMultiTexCoord2fv (GL_TEXTURE0, (GLfloat *) (texCoordP + j));
				glMultiTexCoord2fv (GL_TEXTURE1, (GLfloat *) (ovlTexCoordP + j));
				if (bmMask)
					glMultiTexCoord2fv (GL_TEXTURE2, (GLfloat *) (ovlTexCoordP + j));
				}
			else if (bmP [h])
				glTexCoord2fv ((GLfloat *) (texCoordP + j));
			glVertex3fv ((GLfloat *) (gameData.segs.faces.vertices + j));
			}
		glEnd ();
		if (bLighting && !(nPass || h)) {
			glBlendFunc (GL_ONE, GL_ONE);
			G3SetupFaceLight (faceP, bTextured);
			}
		nPass++;
		} while (nRemainingLights > 0);
	if (bLighting)
		OglDisableLighting ();
	if (++h >= nTextures)
		break;
	INIT_TMU (InitTMU0, GL_TEXTURE0, bmP [h], 0);
	}
if (bLighting) {
	for (i = 0; i < 4; i++)
		gameData.render.lights.dynamic.shader.nActiveLights [i] = gameData.render.lights.dynamic.shader.iVariableLights [i];
	}
if (!bBlend)
	glEnable (GL_BLEND);
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawFaceArrays (grsFace *faceP, grsBitmap *bmBot, grsBitmap *bmTop, int bBlend, int bTextured, int bDepthOnly)
{
	int			bColorKey, bOverlay, bTransparent, bMonitor = 0, bMultiTexture = 0;
	grsBitmap	*bmMask = NULL;
	tTexCoord2f	*ovlTexCoordP;

#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	if (bDepthOnly)
		nDbgSeg = nDbgSeg;
	else
		nDbgSeg = nDbgSeg;
if (bmBot && strstr (bmBot->szName, "door"))
	bmBot = bmBot;
#endif

if (!faceP->bTextured)
	bmBot = NULL;
else if (bmBot)
	bmBot = BmOverride (bmBot, -1);
bTransparent = faceP->bTransparent || (bmBot && (bmBot->bmProps.flags & BM_FLAG_TRANSPARENT));

if (bDepthOnly) {
	if (bTransparent || faceP->bOverlay)
		return 0;
	bOverlay = 0;
	}
else {
	bMonitor = (faceP->nCamera >= 0);
#ifdef _DEBUG
	if (bMonitor)
		faceP = faceP;
#endif
	if (bTransparent && (gameStates.render.nType < 4) && !(bMonitor || bmTop || faceP->bSplit || faceP->bOverlay)) {
#ifdef _DEBUG
		if (gameOpts->render.bDepthSort > 0) 
#endif
			{
#if 0//def _DEBUG
			if ((faceP->nSegment != nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide != nDbgSide)))
				return 1;
#endif
			RIAddFace (faceP);
			return 0;
			}
		}
	}
#if G3_BUFFER_FACES
	G3FlushFaceBuffer (bMonitor || (bTextured != faceBuffer.bTextured) || faceP->bmTop || (faceP->bmBot != faceBuffer.bmP) || (faceP->nType != SIDE_IS_QUAD));
#endif
if (bTextured) {
	if (bmTop && !bMonitor) {
		if ((bmTop = BmOverride (bmTop, -1)) && BM_FRAMES (bmTop)) {
			bColorKey = (bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
			bmTop = BM_CURFRAME (bmTop);
			}
		else
			bColorKey = (bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
		bOverlay = (bColorKey && gameStates.ogl.bGlTexMerge) ? 1 : -1;
		bMultiTexture = GEO_LIGHTING && ((bOverlay > 0) || ((bOverlay < 0) && !bMonitor));
		}
	else
		bOverlay = 0;
	if ((bmBot != gameStates.render.history.bmBot) || 
		 (bmTop != gameStates.render.history.bmTop) || 
		 (bOverlay != gameStates.render.history.bOverlay)) {
		if (bOverlay > 0) {	
			bmMask = gameStates.render.textures.bHaveMaskShader ? BM_MASK (bmTop) : NULL;
			// set base texture
			if (bmBot != gameStates.render.history.bmBot) {
				INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, 1);
				gameStates.render.history.bmBot = bmBot;
				}
			// set overlay texture
			if (bmTop != gameStates.render.history.bmTop) {
				INIT_TMU (InitTMU1, GL_TEXTURE1, bmTop, 1);
				gameStates.render.history.bmTop = bmTop;
				if (gameStates.render.history.bOverlay != 1) {	//enable multitexturing
					if (!G3EnableClientState (GL_TEXTURE_COORD_ARRAY, GL_TEXTURE1))
						return 1;
					}
				}
			INIT_TMU (InitTMU2, GL_TEXTURE2, bmMask, 2);
			gameStates.render.history.bmMask = bmMask;
			G3SetupShader (bColorKey, 1, 1, &faceP->color);
			}
		else {
			if (gameStates.render.history.bOverlay > 0) {
				glUseProgramObject (0);
				if (gameStates.render.history.bmMask) {
					glActiveTexture (GL_TEXTURE2);
					glClientActiveTexture (GL_TEXTURE2);
					glDisableClientState (GL_TEXTURE_COORD_ARRAY);
					OGL_BINDTEX (0);
					gameStates.render.history.bmMask = NULL;
					}
				}
			if (bMultiTexture) {
				if (bmTop != gameStates.render.history.bmTop) {
					glActiveTexture (GL_TEXTURE1);
					if (bmTop) {
						glClientActiveTexture (GL_TEXTURE1);
						INIT_TMU (InitTMU1, GL_TEXTURE1, bmTop, 1);
						}
					else {
						glClientActiveTexture (GL_TEXTURE1);
						OGL_BINDTEX (0);
						}
					gameStates.render.history.bmTop = bmTop;
					}
				}
			else {
				if (gameStates.render.history.bmTop) {
					glActiveTexture (GL_TEXTURE1);
					glClientActiveTexture (GL_TEXTURE1);
					OGL_BINDTEX (0);
					gameStates.render.history.bmTop = NULL;
					}
				}
			if (bmBot != gameStates.render.history.bmBot) {
				INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, 1);
				gameStates.render.history.bmBot = bmBot;
				}
			G3SetupShader (0, bMultiTexture, bmBot != NULL, &faceP->color);
			}
		}
	gameStates.render.history.bOverlay = bOverlay;
	}
else {
	bOverlay = 0;
	glDisable (GL_TEXTURE_2D);
	}
#if G3_BUFFER_FACES
if (!(bMonitor || bOverlay)) {
	G3FillFaceBuffer (faceP, bTextured);
	return 0;
	}
#endif
if (!bBlend)
	glDisable (GL_BLEND);
glDrawArrays (GL_TRIANGLE_FAN, faceP->nIndex, 4);

if (!bMultiTexture) {
	ovlTexCoordP = bMonitor ? faceP->pTexCoord - faceP->nIndex : gameData.segs.faces.ovlTexCoord;
	if (bTextured) {
		INIT_TMU (InitTMU0, GL_TEXTURE0, bmTop, 1);
		if (gameData.render.lights.dynamic.headLights.nLights)
			glUniform1i (glGetUniformLocation (tmProg, "btmTex"), 0);
		glActiveTexture (GL_TEXTURE0);
		glClientActiveTexture (GL_TEXTURE0);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		}
	else {
		glActiveTexture (GL_TEXTURE0);
		glClientActiveTexture (GL_TEXTURE0);
		glDisable (GL_TEXTURE_2D);
		OGL_BINDTEX (0);
		}
	glTexCoordPointer (2, GL_FLOAT, 0, ovlTexCoordP);
	glDrawArrays (GL_TRIANGLE_FAN, faceP->nIndex, 4);
	glTexCoordPointer (2, GL_FLOAT, 0, gameData.segs.faces.texCoord);
	gameStates.render.history.bmBot = bmTop;
	}
if (!bBlend)
	glEnable (GL_BLEND);
return 0;
}

//------------------------------------------------------------------------------
//eof