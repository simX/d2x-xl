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
#include <math.h>

#include "inferno.h"
#include "maths.h"
#include "error.h"
#include "network.h"
#include "light.h"
#include "dynlight.h"
#include "endlevel.h"
#include "ogl_lib.h"
#include "ogl_color.h"

#define CHECK_LIGHT_VERT 1
#define BRIGHT_SHOTS 0

#ifdef _DEBUG
#	define ONLY_HEADLIGHT 0
#else
#	define ONLY_HEADLIGHT 0
#endif

#define GEO_LIN_ATT	(0.05f * gameData.render.fAttScale)
#define GEO_QUAD_ATT	(0.005f * gameData.render.fAttScale)
#define OBJ_LIN_ATT	(0.05f * gameData.render.fAttScale)
#define OBJ_QUAD_ATT	(0.005f * gameData.render.fAttScale)

//------------------------------------------------------------------------------

tFaceColor lightColor = {{1.0f, 1.0f, 1.0f, 1.0f}, 0};
tFaceColor tMapColor = {{1.0f, 1.0f, 1.0f, 1.0f}, 0};
tFaceColor vertColors [8] = {
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}
	};
tRgbaColorf shadowColor [2] = {{1.0f, 0.0f, 0.0f, 80.0f}, {0.0f, 0.0f, 1.0f, 80.0f}};
tRgbaColorf modelColor [2] = {{0.0f, 0.5f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f, 0.5f}};

//------------------------------------------------------------------------------

void OglPalColor (ubyte *palette, int c)
{
	GLfloat	fc [4];

if (c < 0)
	glColor3f (1.0, 1.0, 1.0);
else {
	if (!palette)
		palette = gamePalette;
	if (!palette)
		palette = defaultPalette;
	c *= 3;
	fc [0] = (float) (palette [c]) / 63.0f;
	fc [1] = (float) (palette [c]) / 63.0f;
	fc [2] = (float) (palette [c]) / 63.0f;
	if (gameStates.render.grAlpha >= GR_ACTUAL_FADE_LEVELS)
		fc [3] = 1.0f;
	else {
		fc [3] = (float) gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS; //1.0f - (float) gameStates.render.grAlpha / ((float) GR_ACTUAL_FADE_LEVELS - 1.0f);
		glEnable (GL_BLEND);
		}
	glColor4fv (fc);
	}
}

//------------------------------------------------------------------------------

void OglGrsColor (grsColor *pc)
{
	GLfloat	fc [4];

if (!pc)
	glColor4f (1.0, 1.0, 1.0, gameStates.render.grAlpha);
else if (pc->rgb) {
	fc [0] = (float) (pc->color.red) / 255.0f;
	fc [1] = (float) (pc->color.green) / 255.0f;
	fc [2] = (float) (pc->color.blue) / 255.0f;
	fc [3] = (float) (pc->color.alpha) / 255.0f;
	if (fc [3] < 1.0f) {
		glEnable (GL_BLEND);
		glBlendFunc (gameData.render.ogl.nSrcBlend, gameData.render.ogl.nDestBlend);
		}
	glColor4fv (fc);
	}
else
	OglPalColor (gamePalette, pc->index);
}

//------------------------------------------------------------------------------

// cap tMapColor scales the color values in tMapColor so that none of them exceeds
// 1.0 if multiplied with any of the current face's corners' brightness values.
// To do that, first the highest corner brightness is determined.
// In the next step, color values are increased to match the max. brightness.
// Then it is determined whether any color value multiplied with the max. brightness would
// exceed 1.0. If so, all three color values are scaled so that their maximum multiplied
// with the max. brightness does not exceed 1.0.

inline void CapTMapColor (tUVL *uvlList, int nVertices, grsBitmap *bm)
{
#if 0
	tFaceColor *color = tMapColor.index ? &tMapColor : lightColor.index ? &lightColor : NULL;

if (! (bm->bmProps.flags & BM_FLAG_NO_LIGHTING) && color) {
		double	a, m = tMapColor.color.red;
		double	h, l = 0;
		int		i;

	for (i = 0; i < nVertices; i++, uvlList++) {
		h = (bm->bmProps.flags & BM_FLAG_NO_LIGHTING) ? 1.0 : f2fl (uvlList->l);
		if (l < h)
			l = h;
		}

	// scale brightness with the average color to avoid darkening bright areas with the color
	a = (color->color.red + color->color.green + color->color.blue) / 3;
	if (m < color->color.green)
		m = color->color.green;
	if (m < color->color.blue)
		m = color->color.blue;
	l = l / a;
	// prevent any color component getting over 1.0
	if (l * m > 1.0)
		l = 1.0 / m;
	color->color.red *= l;
	color->color.green *= l;
	color->color.blue *= l;
	}
#endif
}

//------------------------------------------------------------------------------

static inline void ScaleColor (tFaceColor *color, float l)
{
if (l >= 0) {
		float m = color->color.red;

	if (m < color->color.green)
		m = color->color.green;
	if (m < color->color.blue)
		m = color->color.blue;
	if (m > 0.0f) {
		m = l / m;
		color->color.red *= m;
		color->color.green *= m;
		color->color.blue *= m;
		}
	}
}

//------------------------------------------------------------------------------

inline float BC (float c, float b)	//biased contrast
{
return (float) ((c < 0.5) ? pow (c, 1.0f / b) : pow (c, b));
}

void OglColor4sf (float r, float g, float b, float s)
{
if (gameStates.ogl.bStandardContrast)
	glColor4f (r * s, g * s, b * s, gameStates.ogl.fAlpha);
else {
	float c = (float) gameStates.ogl.nContrast - 8.0f;

	if (c > 0.0f)
		c = 1.0f / (1.0f + c * (3.0f / 8.0f));
	else
		c = 1.0f - c * (3.0f / 8.0f);
	glColor4f (BC (r, c) * s, BC (g, c) * s, BC (b, c) * s, gameStates.ogl.fAlpha);
	}
}

//------------------------------------------------------------------------------

/*inline*/ 
void SetTMapColor (tUVL *uvlList, int i, grsBitmap *bmP, int bResetColor, tFaceColor *vertColor)
{
	float l = (bmP->bmProps.flags & BM_FLAG_NO_LIGHTING) ? 1.0f : f2fl (uvlList->l);
	float s = 1.0f;

#if SHADOWS
if (gameStates.ogl.bScaleLight)
	s *= gameStates.render.bHeadLightOn ? 0.4f : 0.3f;
#endif
if (gameStates.app.bEndLevelSequence >= EL_OUTSIDE)
	OglColor4sf (l, l, l, s);
else if (vertColor) {
	if (tMapColor.index) {
		ScaleColor (&tMapColor, l);
		vertColor->color = tMapColor.color;
		if (l >= 0)
			tMapColor.color.red =
			tMapColor.color.green =
			tMapColor.color.blue = 1.0;
		}
	else if (i >= sizeof (vertColors) / sizeof (tFaceColor))
		return;
	else if (vertColors [i].index) {
			tFaceColor *pvc = vertColors + i;

		vertColor->color = vertColors [i].color;
		if (bResetColor) {
			pvc->color.red =
			pvc->color.green =
			pvc->color.blue = 1.0;
			pvc->index = 0;
			}
		}
	else {
		vertColor->color.red = 
		vertColor->color.green = 
		vertColor->color.blue = l;
		}
	vertColor->color.alpha = s;
	}
else {
	if (tMapColor.index) {
		ScaleColor (&tMapColor, l);
		OglColor4sf (tMapColor.color.red, tMapColor.color.green, tMapColor.color.blue, s);
		if (l >= 0)
			tMapColor.color.red =
			tMapColor.color.green =
			tMapColor.color.blue = 1.0;
		}
	else if (i >= sizeof (vertColors) / sizeof (tFaceColor))
		return;
	else if (vertColors [i].index) {
			tFaceColor *pvc = vertColors + i;

		OglColor4sf (pvc->color.red, pvc->color.green, pvc->color.blue, s);
		if (bResetColor) {
			pvc->color.red =
			pvc->color.green =
			pvc->color.blue = 1.0;
			pvc->index = 0;
			}
		}
	else {
		OglColor4sf (l, l, l, s);
		}
	}
}

//------------------------------------------------------------------------------

#define G3_DOTF(_v0,_v1)	((_v0).p.x * (_v1).p.x + (_v0).p.y * (_v1).p.y + (_v0).p.z * (_v1).p.z)

#define G3_REFLECT(_vr,_vl,_vn) \
	{ \
	float	LdotN = 2 * G3_DOTF(_vl, _vn); \
	(_vr).p.x = (_vn).p.x * LdotN - (_vl).p.x; \
	(_vr).p.y = (_vn).p.y * LdotN - (_vl).p.y; \
	(_vr).p.z = (_vn).p.z * LdotN - (_vl).p.z; \
	} 

//------------------------------------------------------------------------------

inline int sqri (int i)
{
return i * i;
}

//------------------------------------------------------------------------------

#if CHECK_LIGHT_VERT

static inline int IsLightVert (int nVertex, tShaderLight *psl)
{
if ((nVertex >= 0) && psl->info.faceP) {
	ushort	*pv = gameStates.render.bTriangleMesh ? psl->info.faceP->triIndex : psl->info.faceP->index;
	int		i;
	
	for (i = psl->info.faceP->nVerts; i; i--, pv++)
		if (*pv == (ushort) nVertex)
			return 1;
	}
return 0;
}

#endif

//------------------------------------------------------------------------------

#define VECMAT_CALLS 0

float fLightRanges [5] = {0.5f, 0.7071f, 1.0f, 1.4142f, 2.0f};

#if 1//def _DEBUG

int G3AccumVertColor (int nVertex, fVector3 *pColorSum, tVertColorData *vcdP, int nThread)
{
	int						i, j, nLights, nType, bInRad, 
								bSkipHeadLight = gameOpts->ogl.bHeadLight && !gameStates.render.nState, 
								nSaturation = gameOpts->render.color.nSaturation;
	int						nBrightness, nMaxBrightness = 0;
	float						fLightDist, fAttenuation, spotEffect, NdotL, RdotE;
	fVector3					spotDir, lightDir, lightPos, vertPos, vReflect;
	fVector3					lightColor, colorSum, vertColor = {{0.0f, 0.0f, 0.0f}};
	tShaderLight			*psl;
	tShaderLightIndex		*sliP = &gameData.render.lights.dynamic.shader.index [0][nThread];
	tActiveShaderLight	*activeLightsP = gameData.render.lights.dynamic.shader.activeLights [nThread] + sliP->nFirst;
	tVertColorData			vcd = *vcdP;

#ifdef _DEBUG
if (nThread == 0)
	nThread = nThread;
if (nThread == 1)
	nThread = nThread;
#endif
colorSum = *pColorSum;
VmVecSub (&vertPos, vcd.pVertPos, (fVector3 *) &viewInfo.glPosf);
VmVecNormalize (&vertPos, VmVecNegate (&vertPos));
nLights = sliP->nActive;
if (nLights > gameData.render.lights.dynamic.nLights)
	nLights = gameData.render.lights.dynamic.nLights;
i = sliP->nLast - sliP->nFirst + 1;
#ifdef _DEBUG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
for (j = 0; (i > 0) && (nLights > 0); activeLightsP++, i--) {
#if 1
	if (!(psl = activeLightsP->psl))
#else
	if (!(psl = GetActiveShaderLight (activeLightsP, nThread)))
#endif
		continue;
#ifdef _DEBUG
	if (((char *) psl - (char *) gameData.render.lights.dynamic.shader.lights) % sizeof (*psl))
		continue;
	if ((nDbgSeg >= 0) && (psl->info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (psl->info.nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
#endif
	nLights--;
#if 0
	if (i == vcd.nMatLight)
		continue;
#endif
	nType = psl->info.nType;
	if (bSkipHeadLight && (nType == 3))
		continue;
#if ONLY_HEADLIGHT
	if (nType != 3)
		continue;
#endif
	if (psl->info.bVariable && gameData.render.vertColor.bDarkness)
		continue;
	lightColor = *((fVector3 *) &psl->info.color);
	lightPos = psl->vPosf [gameStates.render.nState && !gameStates.ogl.bUseTransform].v3;
	VmVecSub (&lightDir, &lightPos, vcd.pVertPos);
	bInRad = 0;
	fLightDist = VmVecMag (&lightDir) * gameStates.ogl.fLightRange;
	VmVecNormalize (&lightDir, &lightDir);
	if (gameStates.render.nState || (nType < 2)) {
#if CHECK_LIGHT_VERT == 2
		if (IsLightVert (nVertex, psl))
			fLightDist = 0;
		else
#endif
			fLightDist -= psl->info.fRad * gameStates.ogl.fLightRange; //make light brighter close to light source
		}
	if ((vcd.vertNorm.p.x == 0) && (vcd.vertNorm.p.y == 0) && (vcd.vertNorm.p.z == 0))
		NdotL = 1.0f;
	else
		NdotL = VmVecDot (&vcd.vertNorm, &lightDir);
	if	(/*(NdotL >= -0.125f) &&*/ ((fLightDist <= 0.0f)) || IsLightVert (nVertex, psl)) {
		bInRad = 1;
		NdotL = 1;
		fLightDist = 0;
		fAttenuation = 1.0f / psl->info.fBrightness;
		}
	else {	//make it decay faster
#if BRIGHT_SHOTS
		if (nType == 2)
			fAttenuation = (1.0f + OBJ_LIN_ATT * fLightDist + OBJ_QUAD_ATT * fLightDist * fLightDist);
		else
#endif
			fAttenuation = (1.0f + GEO_LIN_ATT * fLightDist + GEO_QUAD_ATT * fLightDist * fLightDist);
#if 0
		NdotL = 1 - ((1 - NdotL) * 0.9f);
#endif
		if (NdotL < 0)
			NdotL = 0;
		if (/*(NdotL >= -0.125f) &&*/ (psl->info.fRad > 0))
			NdotL += (1.0f - NdotL) / (0.5f + fAttenuation / 2.0f);
		fAttenuation /= psl->info.fBrightness;
		}
	if (psl->info.bSpot) {
		if (NdotL <= 0)
			continue;
		VmVecNormalize (&spotDir, &psl->vDirf.v3);
		lightDir.p.x = -lightDir.p.x;
		lightDir.p.y = -lightDir.p.y;
		lightDir.p.z = -lightDir.p.z;
		spotEffect = G3_DOTF (spotDir, lightDir);
		if (spotEffect <= psl->info.fSpotAngle)
			continue;
		if (psl->info.fSpotExponent)
			spotEffect = (float) pow (spotEffect, psl->info.fSpotExponent);
		fAttenuation /= spotEffect * gameStates.ogl.fLightRange;
		VmVecScaleAdd (&vertColor, &gameData.render.vertColor.matAmbient.v3, &gameData.render.vertColor.matDiffuse.v3, NdotL);
		}
	else {
		vertColor = gameData.render.vertColor.matAmbient.v3;
		if (NdotL < 0)
			NdotL = 0;
		else
			VmVecScaleInc (&vertColor, &gameData.render.vertColor.matDiffuse.v3, NdotL);
		}
	VmVecMul (&vertColor, &vertColor, &lightColor);
	if ((NdotL > 0.0) && (vcd.fMatShininess > 0) /* && vcd.bMatSpecular */) {
		//RdotV = max (dot (reflect (-normalize (lightDir), normal), normalize (-vertPos)), 0.0);
		if (!psl->info.bSpot)	//need direction from light to vertex now
			VmVecNegate (&lightDir);
		VmVecReflect (&vReflect, &lightDir, &vcd.vertNorm);
		VmVecNormalize (&vReflect, &vReflect);
#ifdef _DEBUG
		if (nVertex == nDbgVertex)
			nDbgVertex = nDbgVertex;
#endif
		RdotE = VmVecDot (&vReflect, &vertPos);
		if (RdotE > 0) {
			//spec = pow (reflect dot lightToEye, matShininess) * matSpecular * lightSpecular
			VmVecScaleInc (&vertColor, &lightColor, (float) pow (RdotE, vcd.fMatShininess));
			}
		}
	if ((nSaturation < 2) || gameStates.render.bLightMaps)	{//sum up color components
		VmVecScaleAdd (&colorSum, &colorSum, &vertColor, 1.0f / fAttenuation);
		}
	else {	//use max. color components
		VmVecScale (&vertColor, &vertColor, fAttenuation);
		nBrightness = sqri ((int) (vertColor.c.r * 1000)) + sqri ((int) (vertColor.c.g * 1000)) + sqri ((int) (vertColor.c.b * 1000));
		if (nMaxBrightness < nBrightness) {
			nMaxBrightness = nBrightness;
			colorSum = vertColor;
			}
		else if (nMaxBrightness == nBrightness) {
			if (colorSum.c.r < vertColor.c.r)
				colorSum.c.r = vertColor.c.r;
			if (colorSum.c.g < vertColor.c.g)
				colorSum.c.g = vertColor.c.g;
			if (colorSum.c.b < vertColor.c.b)
				colorSum.c.b = vertColor.c.b;
			}
		}
	j++;
	}
if (j) {
	if ((nSaturation == 1) || gameStates.render.bLightMaps) { //if a color component is > 1, cap color components using highest component value
		float	cMax = colorSum.c.r;
		if (cMax < colorSum.c.g)
			cMax = colorSum.c.g;
		if (cMax < colorSum.c.b)
			cMax = colorSum.c.b;
		if (cMax > 1) {
			colorSum.c.r /= cMax;
			colorSum.c.g /= cMax;
			colorSum.c.b /= cMax;
			}
		}
	*pColorSum = colorSum;
	}
#ifdef _DEBUG
if (nLights)
	nLights = 0;
#endif
if (!gameOpts->render.nPath)
	ResetNearestVertexLights (nVertex, nThread);
return j;
}

#else //RELEASE

int G3AccumVertColor (int nVertex, fVector3 *pColorSum, tVertColorData *vcdP, int nThread)
{
	int						i, j, nLights, nType, bInRad, 
								bSkipHeadLight = gameOpts->ogl.bHeadLight && !gameStates.render.nState, 
								nSaturation = gameOpts->render.color.nSaturation;
	int						nBrightness, nMaxBrightness = 0, nMeshQuality = gameOpts->render.nMeshQuality;
	float						fLightDist, fAttenuation, spotEffect, fMag, NdotL, RdotE;
	fVector3					spotDir, lightDir, lightPos, vertPos, vReflect;
	fVector3					lightColor, colorSum, vertColor = {{0.0f, 0.0f, 0.0f}};
	tShaderLight			*psl;
	tShaderLightIndex		*sliP = &gameData.render.lights.dynamic.shader.index [0][nThread];
	tActiveShaderLight	*activeLightsP = gameData.render.lights.dynamic.shader.activeLights [nThread] + sliP->nFirst;
	tVertColorData			vcd = *vcdP;

colorSum = *pColorSum;
VmVecSub (&vertPos, vcd.pVertPos, (fVector3 *) &viewInfo.glPosf);
VmVecNormalize (&vertPos, VmVecNegate (&vertPos));
nLights = sliP->nActive;
if (nLights > gameData.render.lights.dynamic.nLights)
	nLights = gameData.render.lights.dynamic.nLights;
i = sliP->nLast - sliP->nFirst + 1;
for (j = 0; (i > 0) && (nLights > 0); activeLightsP++, i--) {
	if (!(psl = activeLightsP->psl))
		continue;
	nLights--;
	nType = psl->info.nType;
	if (bSkipHeadLight && (nType == 3))
		continue;
#if ONLY_HEADLIGHT
	if (nType != 3)
		continue;
#endif
	if (psl->info.bVariable && gameData.render.vertColor.bDarkness)
		continue;
	lightColor = *((fVector3 *) &psl->info.color);
	lightPos = psl->vPosf [gameStates.render.nState && !gameStates.ogl.bUseTransform].v3;
#if VECMAT_CALLS
	VmVecSub (&lightDir, &lightPos, vcd.pVertPos);
#else
	lightDir.p.x = lightPos.p.x - vcd.pVertPos->p.x;
	lightDir.p.y = lightPos.p.y - vcd.pVertPos->p.y;
	lightDir.p.z = lightPos.p.z - vcd.pVertPos->p.z;
#endif
	//scaled quadratic attenuation depending on brightness
	bInRad = 0;
	NdotL = 1;
#if VECMAT_CALLS
	VmVecNormalize (&lightDir, &lightDir);
#else
	if ((fMag = VmVecMag (&lightDir))) {
		lightDir.p.x /= fMag;
		lightDir.p.y /= fMag;
		lightDir.p.z /= fMag;
		}
#endif
#if 0
	if (psl->info.fBrightness < 0)
		fAttenuation = 0.01f;
	else 
#endif
		{
#if VECMAT_CALLS
		fLightDist = VmVecMag (&lightDir) / gameStates.ogl.fLightRange;
#else
		fLightDist = fMag / gameStates.ogl.fLightRange;
#endif
		if (gameStates.render.nState || (nType < 2)) {
#if CHECK_LIGHT_VERT
			if (IsLightVert (nVertex, psl))
				fLightDist = 0;
			else
#endif
				fLightDist -= psl->info.fRad / gameStates.ogl.fLightRange;	//make light brighter close to light source
			}
		if (fLightDist < 0.0f) {
			bInRad = 1;
			fLightDist = 0;
			fAttenuation = 1.0f / psl->info.fBrightness;
			}
		else if (IsLightVert (nVertex, psl)) {
			bInRad = 1;
			fLightDist = 0;
			fAttenuation = 1.0f / psl->info.fBrightness;
			}
		else {	//make it decay faster
#if BRIGHT_SHOTS
			if (nType == 2)
				fAttenuation = (1.0f + OBJ_LIN_ATT * fLightDist + OBJ_QUAD_ATT * fLightDist * fLightDist);
			else
#endif
				fAttenuation = (1.0f + GEO_LIN_ATT * fLightDist + GEO_QUAD_ATT * fLightDist * fLightDist);
			NdotL = VmVecDot (&vcd.vertNorm, &lightDir);
#if 0
			NdotL = 1 - ((1 - NdotL) * 0.9f);
#endif
			if (psl->info.fRad > 0)
				NdotL += (1.0f - NdotL) / (0.5f + fAttenuation / 2.0f);
			fAttenuation /= psl->info.fBrightness;
			}
		}
	if (psl->info.bSpot) {
		if (NdotL <= 0)
			continue;
#if VECMAT_CALLS
		VmVecNormalize (&spotDir, &psl->vDirf);
#else
		fMag = VmVecMag (&psl->vDirf);
		spotDir.p.x = psl->vDirf.p.x / fMag;
		spotDir.p.y = psl->vDirf.p.y / fMag;
		spotDir.p.z = psl->vDirf.p.z / fMag;
#endif
		lightDir.p.x = -lightDir.p.x;
		lightDir.p.y = -lightDir.p.y;
		lightDir.p.z = -lightDir.p.z;
		spotEffect = G3_DOTF (spotDir, lightDir);
#if 1
		if (spotEffect <= psl->info.fSpotAngle)
			continue;
#endif
		if (psl->info.fSpotExponent)
			spotEffect = (float) pow (spotEffect, psl->info.fSpotExponent);
		fAttenuation /= spotEffect * gameStates.ogl.fLightRange;
#if VECMAT_CALLS
		VmVecScaleAdd (&vertColor, &gameData.render.vertColor.matAmbient, &gameData.render.vertColor.matDiffuse, NdotL);
#else
		vertColor.p.x = gameData.render.vertColor.matAmbient.p.x + gameData.render.vertColor.matDiffuse.p.x * NdotL;
		vertColor.p.y = gameData.render.vertColor.matAmbient.p.y + gameData.render.vertColor.matDiffuse.p.y * NdotL;
		vertColor.p.z = gameData.render.vertColor.matAmbient.p.z + gameData.render.vertColor.matDiffuse.p.z * NdotL;
#endif
		}
	else {
		vertColor.p = gameData.render.vertColor.matAmbient.v3.p;
		if (NdotL < 0)
			NdotL = 0;
		else {
		//vertColor = lightColor * (gl_FrontMaterial.diffuse * NdotL + matAmbient);
#if VECMAT_CALLS
			VmVecScaleInc (&vertColor, &gameData.render.vertColor.matDiffuse, NdotL);
#else
			vertColor.p.x += gameData.render.vertColor.matDiffuse.p.x * NdotL;
			vertColor.p.y += gameData.render.vertColor.matDiffuse.p.y * NdotL;
			vertColor.p.z += gameData.render.vertColor.matDiffuse.p.z * NdotL;
#endif
			}
		}
	vertColor.p.x *= lightColor.p.x;
	vertColor.p.y *= lightColor.p.y;
	vertColor.p.z *= lightColor.p.z;
	if ((NdotL > 0) && (vcd.fMatShininess > 0)/* && vcd.bMatSpecular */) {
		//spec = pow (reflect dot lightToEye, matShininess) * matSpecular * lightSpecular
		//RdotV = max (dot (reflect (-normalize (lightDir), normal), normalize (-vertPos)), 0.0);
		if (!psl->info.bSpot) {	//need direction from light to vertex now
			lightDir.p.x = -lightDir.p.x;
			lightDir.p.y = -lightDir.p.y;
			lightDir.p.z = -lightDir.p.z;
			}
		G3_REFLECT (vReflect, lightDir, vcd.vertNorm);
#if VECMAT_CALLS
		VmVecNormalize (&vReflect, &vReflect);
#else
		if ((fMag = VmVecMag (&vReflect))) {
			vReflect.p.x /= fMag;
			vReflect.p.y /= fMag;
			vReflect.p.z /= fMag;
			}
#endif
		RdotE = G3_DOTF (vReflect, vertPos);
		if (RdotE > 0) {
#if VECMAT_CALLS
			VmVecScale (&lightColor, &lightColor, (float) pow (RdotE, vcd.fMatShininess));
#else
			fMag = (float) pow (RdotE, vcd.fMatShininess);
			lightColor.p.x *= fMag;
			lightColor.p.y *= fMag;
			lightColor.p.z *= fMag;
#endif
			}
#if VECMAT_CALLS
		VmVecMul (&lightColor, &lightColor, &vcd.matSpecular);
		VmVecInc (&vertColor, &lightColor);
#else
		vertColor.p.x += lightColor.p.x * vcd.matSpecular.p.x;
		vertColor.p.y += lightColor.p.y * vcd.matSpecular.p.y;
		vertColor.p.z += lightColor.p.z * vcd.matSpecular.p.z;
#endif
		}
	if ((nSaturation < 2) || gameStates.render.bLightMaps)	{//sum up color components
#if VECMAT_CALLS
		VmVecScaleAdd (&colorSum, &colorSum, &vertColor, 1.0f / fAttenuation);
#else
		colorSum.p.x += vertColor.p.x / fAttenuation;
		colorSum.p.y += vertColor.p.y / fAttenuation;
		colorSum.p.z += vertColor.p.z / fAttenuation;
#endif
		}
	else {	//use max. color components
		vertColor.p.x /= fAttenuation;
		vertColor.p.y /= fAttenuation;
		vertColor.p.z /= fAttenuation;
		nBrightness = sqri ((int) (vertColor.c.r * 1000)) + sqri ((int) (vertColor.c.g * 1000)) + sqri ((int) (vertColor.c.b * 1000));
		if (nMaxBrightness < nBrightness) {
			nMaxBrightness = nBrightness;
			colorSum = vertColor;
			}
		else if (nMaxBrightness == nBrightness) {
			if (colorSum.c.r < vertColor.c.r)
				colorSum.c.r = vertColor.c.r;
			if (colorSum.c.g < vertColor.c.g)
				colorSum.c.g = vertColor.c.g;
			if (colorSum.c.b < vertColor.c.b)
				colorSum.c.b = vertColor.c.b;
			}
		}
	j++;
	}
if (j) {
	if ((nSaturation == 1) || gameStates.render.bLightMaps) { //if a color component is > 1, cap color components using highest component value
		float	cMax = colorSum.c.r;
		if (cMax < colorSum.c.g)
			cMax = colorSum.c.g;
		if (cMax < colorSum.c.b)
			cMax = colorSum.c.b;
		if (cMax > 1) {
			colorSum.c.r /= cMax;
			colorSum.c.g /= cMax;
			colorSum.c.b /= cMax;
			}
		}
	*pColorSum = colorSum;
	}
return j;
}

#endif

//------------------------------------------------------------------------------

void InitVertColorData (tVertColorData& vcd)
{
	static fVector matSpecular = {{1.0f, 1.0f, 1.0f, 1.0f}};

vcd.bExclusive = !FAST_SHADOWS && (gameStates.render.nShadowPass == 3),
vcd.fMatShininess = 0;
vcd.bMatSpecular = 0;
vcd.bMatEmissive = 0; 
vcd.nMatLight = -1;
if (gameData.render.lights.dynamic.material.bValid) {
#if 0
	if (gameData.render.lights.dynamic.material.emissive.c.r ||
		 gameData.render.lights.dynamic.material.emissive.c.g ||
		 gameData.render.lights.dynamic.material.emissive.c.b) {
		vcd.bMatEmissive = 1;
		vcd.nMatLight = gameData.render.lights.dynamic.material.nLight;
		colorSum = gameData.render.lights.dynamic.material.emissive;
		}
#endif
	vcd.bMatSpecular = 
		gameData.render.lights.dynamic.material.specular.c.r ||
		gameData.render.lights.dynamic.material.specular.c.g ||
		gameData.render.lights.dynamic.material.specular.c.b;
	if (vcd.bMatSpecular) {
		vcd.matSpecular = gameData.render.lights.dynamic.material.specular;
		vcd.fMatShininess = (float) gameData.render.lights.dynamic.material.shininess;
		}
	else
		vcd.matSpecular = matSpecular;
	}
else {
	vcd.bMatSpecular = 1;
	vcd.matSpecular = matSpecular;
	vcd.fMatShininess = 64;
	}
}

//------------------------------------------------------------------------------

extern int nDbgVertex;

#if PROFILING
time_t tG3VertexColor = 0;
#endif

void G3VertexColor (fVector3 *pvVertNorm, fVector3 *pVertPos, int nVertex, 
						  tFaceColor *pVertColor, tFaceColor *pBaseColor, 
						  float fScale, int bSetColor, int nThread)
{
	fVector3			colorSum = {{0.0f, 0.0f, 0.0f}};
	fVector3			vertPos;
	tFaceColor		*pc = NULL;
	int				bVertexLights;
#if PROFILING
	time_t			t = clock ();
#endif
	tVertColorData	vcd;

InitVertColorData (vcd);
#ifdef _DEBUG
if (!gameStates.render.nState && (nVertex == nDbgVertex))
	nVertex = nVertex;
#endif
if (gameStates.render.nFlashScale)
	fScale *= f2fl (gameStates.render.nFlashScale);
if (!FAST_SHADOWS && (gameStates.render.nShadowPass == 3))
	; //fScale = 1.0f;
else if (FAST_SHADOWS || (gameStates.render.nShadowPass != 1))
	; //fScale = 1.0f;
else
	fScale *= gameStates.render.bHeadLightOn ? 0.4f : 0.3f;
if (fScale > 1)
	fScale = 1;
#if 1//ndef _DEBUG //cache light values per frame
if (!(gameStates.render.nState || vcd.bExclusive || vcd.bMatEmissive) && (nVertex >= 0)) {
	pc = gameData.render.color.vertices + nVertex;
	if (pc->index == gameStates.render.nFrameFlipFlop + 1) {
		if (pVertColor) {
			pVertColor->index = gameStates.render.nFrameFlipFlop + 1;
			pVertColor->color.red = pc->color.red * fScale;
			pVertColor->color.green = pc->color.green * fScale;
			pVertColor->color.blue = pc->color.blue * fScale;
			pVertColor->color.alpha = 1;
			}
		if (bSetColor)
			OglColor4sf (pc->color.red * fScale, pc->color.green * fScale, pc->color.blue * fScale, 1.0);
#ifdef _DEBUG
		if (!gameStates.render.nState && (nVertex == nDbgVertex))
			nVertex = nVertex;
#endif
#if PROFILING
		tG3VertexColor += clock () - t;
#endif
		return;
		}
	}
#endif
#ifdef _DEBUG
if (!gameStates.render.nState && (nVertex == nDbgVertex))
	nVertex = nVertex;
#endif
if (gameStates.ogl.bUseTransform) 
#if 1
	vcd.vertNorm = *pvVertNorm;
#else
	VmVecNormalize (&vcd.vertNorm, pvVertNorm);
#endif
else {
	if (!gameStates.render.nState)
		VmVecNormalize (&vcd.vertNorm, pvVertNorm);
	else 
		G3RotatePoint (&vcd.vertNorm, pvVertNorm, 0);
	}
if ((bVertexLights = !(gameStates.render.nState || pVertColor))) {
	VmVecFixToFloat (&vertPos, gameData.segs.vertices + nVertex);
	pVertPos = &vertPos;
	SetNearestVertexLights (-1, nVertex, NULL, 1, 0, 1, nThread);
	}
vcd.pVertPos = pVertPos;
//VmVecNegate (&vertNorm);
//if (nLights)
#if MULTI_THREADED_LIGHTS
if (gameStates.app.bMultiThreaded) {
	SDL_SemPost (gameData.threads.vertColor.info [0].exec);
	SDL_SemPost (gameData.threads.vertColor.info [1].exec);
	SDL_SemWait (gameData.threads.vertColor.info [0].done);
	SDL_SemWait (gameData.threads.vertColor.info [1].done);
	VmVecAdd (&colorSum, vcd.colorSum, vcd.colorSum + 1);
	}
else
#endif
#if 1
if (gameStates.app.bEndLevelSequence >= EL_OUTSIDE) {
	colorSum.c.r = 
	colorSum.c.g = 
	colorSum.c.b = 1;
	}
else 
#endif
	{
	if (gameData.render.lights.dynamic.shader.index [0][nThread].nActive) {
		if (pBaseColor)
			memcpy (&colorSum, &pBaseColor->color, sizeof (colorSum));
#ifdef _DEBUG
		if (!gameStates.render.nState && (nVertex == nDbgVertex))
			nVertex = nVertex;
#endif
		G3AccumVertColor (nVertex, &colorSum, &vcd, nThread);
		}
	if ((nVertex >= 0) && !(gameStates.render.nState || gameData.render.vertColor.bDarkness)) {
		tFaceColor *pfc = gameData.render.color.ambient + nVertex;
		colorSum.c.r += pfc->color.red;
		colorSum.c.g += pfc->color.green;
		colorSum.c.b += pfc->color.blue;
#ifdef _DEBUG
		if (!gameStates.render.nState && (nVertex == nDbgVertex) && (colorSum.c.r + colorSum.c.g + colorSum.c.b < 0.1f))
			nVertex = nVertex;
#endif
		}
	if (colorSum.c.r > 1.0)
		colorSum.c.r = 1.0;
	if (colorSum.c.g > 1.0)
		colorSum.c.g = 1.0;
	if (colorSum.c.b > 1.0)
		colorSum.c.b = 1.0;
	}
#if ONLY_HEADLIGHT
if (gameData.render.lights.dynamic.headLights.nLights)
	colorSum.c.r = colorSum.c.g = colorSum.c.b = 0;
#endif
if (bSetColor)
	OglColor4sf (colorSum.c.r * fScale, colorSum.c.g * fScale, colorSum.c.b * fScale, 1.0);
#if 1
if (!vcd.bMatEmissive && pc) {
	pc->index = gameStates.render.nFrameFlipFlop + 1;
	pc->color.red = colorSum.c.r;
	pc->color.green = colorSum.c.g;
	pc->color.blue = colorSum.c.b;
	}
if (pVertColor) {
	pVertColor->index = gameStates.render.nFrameFlipFlop + 1;
	pVertColor->color.red = colorSum.c.r * fScale;
	pVertColor->color.green = colorSum.c.g * fScale;
	pVertColor->color.blue = colorSum.c.b * fScale;
	pVertColor->color.alpha = 1;
	}
#endif
#ifdef _DEBUG
if (!gameStates.render.nState && (nVertex == nDbgVertex))
	nVertex = nVertex;
#endif
#if 0
if (bVertexLights)
	gameData.render.lights.dynamic.shader.index [0][nThread].nActive = gameData.render.lights.dynamic.shader.index [0][nThread].iVertex;
#endif
#if PROFILING
tG3VertexColor += clock () - t;
#endif
#if 0//def _DEBUG
for (int k = 0; k < MAX_SHADER_LIGHTS; k++)
	if (gameData.render.lights.dynamic.shader.activeLights [0][k].nType > 1) {
		gameData.render.lights.dynamic.shader.activeLights [0][k].nType = 0;
		gameData.render.lights.dynamic.shader.activeLights [0][k].psl = NULL;
		}
	else if (gameData.render.lights.dynamic.shader.activeLights [0][k].nType == 1)
		gameData.render.lights.dynamic.shader.activeLights [0][k].nType = 1;
#endif
} 

//------------------------------------------------------------------------------
