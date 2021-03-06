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
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/
#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>	// for memset ()

#include "inferno.h"
#include "u_mem.h"
#include "ogl_color.h"
#include "error.h"
#include "timer.h"
#include "gameseg.h"
#include "lightmap.h"
#include "text.h"
#include "dynlight.h"
#include "headlight.h"
#include "light.h"
#include "network.h"

#define CACHE_LIGHTS 0
#define FLICKERFIX 0
//int	Use_fvi_lighting = 0;

#define	LIGHTING_CACHE_SIZE	4096	//	Must be power of 2!
#define	LIGHTING_FRAME_DELTA	256	//	Recompute cache value every 8 frames.
#define	LIGHTING_CACHE_SHIFT	8

int	nLightingFrameDelta = 1;
int	lightingCache [LIGHTING_CACHE_SIZE];
int	nCacheHits = 0, nCacheLookups = 1;

typedef struct {
  int    nTexture;
  int		nBrightness;
} tTexBright;

#define	NUM_LIGHTS_D1     49
#define	NUM_LIGHTS_D2     85
#define	MAX_BRIGHTNESS		F2_0
tTexBright texBrightD1 [NUM_LIGHTS_D1] = {
	{250, 0x00b333L}, {251, 0x008000L}, {252, 0x008000L}, {253, 0x008000L},
	{264, 0x01547aL}, {265, 0x014666L}, {268, 0x014666L}, {278, 0x014cccL},
	{279, 0x014cccL}, {280, 0x011999L}, {281, 0x014666L}, {282, 0x011999L},
	{283, 0x0107aeL}, {284, 0x0107aeL}, {285, 0x011999L}, {286, 0x014666L},
	{287, 0x014666L}, {288, 0x014666L}, {289, 0x014666L}, {292, 0x010cccL},
	{293, 0x010000L}, {294, 0x013333L}, {328, 0x011333L}, {330, 0x010000L},
	{333, 0x010000L}, {341, 0x010000L}, {343, 0x010000L}, {345, 0x010000L},
	{347, 0x010000L}, {349, 0x010000L}, {351, 0x010000L}, {352, 0x010000L},
	{354, 0x010000L}, {355, 0x010000L}, {356, 0x020000L}, {357, 0x020000L},
	{358, 0x020000L}, {359, 0x020000L}, {360, 0x020000L}, {361, 0x020000L},
	{362, 0x020000L}, {363, 0x020000L}, {364, 0x020000L}, {365, 0x020000L},
	{366, 0x020000L}, {367, 0x020000L}, {368, 0x020000L}, {369, 0x020000L},
	{370, 0x020000L}
};
tTexBright texBrightD2 [NUM_LIGHTS_D2] = {
	{235, 0x012666L}, {236, 0x00b5c2L}, {237, 0x00b5c2L}, {243, 0x00b5c2L},
	{244, 0x00b5c2L}, {275, 0x01547aL}, {276, 0x014666L}, {278, 0x014666L},
	{288, 0x014cccL}, {289, 0x014cccL}, {290, 0x011999L}, {291, 0x014666L},
	{293, 0x011999L}, {295, 0x0107aeL}, {296, 0x011999L}, {298, 0x014666L},
	{300, 0x014666L}, {301, 0x014666L}, {302, 0x014666L}, {305, 0x010cccL},
	{306, 0x010000L}, {307, 0x013333L}, {340, 0x00b333L}, {341, 0x00b333L},
	{343, 0x004cccL}, {344, 0x003333L}, {345, 0x00b333L}, {346, 0x004cccL},
	{348, 0x003333L}, {349, 0x003333L}, {353, 0x011333L}, {356, 0x00028fL},
	{357, 0x00028fL}, {358, 0x00028fL}, {359, 0x00028fL}, {364, 0x010000L},
	{366, 0x010000L}, {368, 0x010000L}, {370, 0x010000L}, {372, 0x010000L},
	{374, 0x010000L}, {375, 0x010000L}, {377, 0x010000L}, {378, 0x010000L},
	{380, 0x010000L}, {382, 0x010000L}, {383, 0x020000L}, {384, 0x020000L},
	{385, 0x020000L}, {386, 0x020000L}, {387, 0x020000L}, {388, 0x020000L},
	{389, 0x020000L}, {390, 0x020000L}, {391, 0x020000L}, {392, 0x020000L},
	{393, 0x020000L}, {394, 0x020000L}, {395, 0x020000L}, {396, 0x020000L},
	{397, 0x020000L}, {398, 0x020000L}, {404, 0x010000L}, {405, 0x010000L},
	{406, 0x010000L}, {407, 0x010000L}, {408, 0x010000L}, {409, 0x020000L},
	{410, 0x008000L}, {411, 0x008000L}, {412, 0x008000L}, {419, 0x020000L},
	{
0, 0x020000L}, {
3, 0x010000L}, {
4, 0x010000L}, {
5, 0x020000L},
	{
6, 0x020000L}, {
7, 0x008000L}, {
8, 0x008000L}, {
9, 0x008000L},
	{430, 0x020000L}, {431, 0x020000L}, {432, 0x00e000L}, {433, 0x020000L},
	{434, 0x020000L}
};

//--------------------------------------------------------------------------

static void ResetClusterLights (void)
{
if (!gameStates.render.bClusterLights)
	return;

	tObject	*objP;
	int		i;

FORALL_LIGHT_OBJS (objP, i)
	if ((objP->info.nType == OBJ_LIGHT) && (objP->info.nId == CLUSTER_LIGHT_ID)) {
		objP->info.xLifeLeft = 0;
		memset (&objP->cType.lightInfo, 0, sizeof (objP->cType.lightInfo));
		}
}

//--------------------------------------------------------------------------

static void SetClusterLights (void)
{
if (!gameStates.render.bClusterLights)
	return;

	tObject	*objP;
	int		h, i;

FORALL_LIGHT_OBJS (objP, i) {
	if ((objP->info.nType == OBJ_LIGHT) && (objP->info.nId == CLUSTER_LIGHT_ID))	{
		i = OBJ_IDX (objP);
		if (!(h = objP->cType.lightInfo.nObjects)) {
			RemoveDynLight (-1, -1, i);
			KillObject (objP);
			}
		else {
			if (h > 1) {
				objP->info.position.vPos[X] /= h;
				objP->info.position.vPos[Y] /= h;
				objP->info.position.vPos[Z] /= h;
#if 1
				objP->cType.lightInfo.color.red /= h;
				objP->cType.lightInfo.color.green /= h;
				objP->cType.lightInfo.color.blue /= h;
				objP->cType.lightInfo.color.alpha /= h;
#endif
				}
			if (1 || (objP->cType.lightInfo.nSegment < 0)) {
				short nSegment = FindSegByPos (objP->info.position.vPos, abs (objP->cType.lightInfo.nSegment), 0, 0);
				objP->cType.lightInfo.nSegment = (nSegment < 0) ? abs (objP->cType.lightInfo.nSegment) : nSegment;
				}
			if (objP->info.nSegment != objP->cType.lightInfo.nSegment)
				RelinkObjToSeg (i, objP->cType.lightInfo.nSegment);
			AddDynLight (NULL, &objP->cType.lightInfo.color, objP->cType.lightInfo.intensity, -1, -1, i, -1, NULL);
			}
		}
	}
}

//--------------------------------------------------------------------------

int InitClusterLight (short nObject, tRgbaColorf *color, fix xObjIntensity)
{
if (!gameStates.render.bClusterLights)
	return 0;

short nLightObj = gameData.objs.lightObjs [nObject].nObject;

if (0 > nLightObj)
	return 0;
#if DBG
if (nDbgObj == nLightObj)
	nDbgObj = nDbgObj;
#endif
tObject *lightObjP = OBJECTS + nLightObj;
if (lightObjP->info.nSignature != gameData.objs.lightObjs [nObject].nSignature) {
	gameData.objs.lightObjs [nObject].nObject = -1;
	return 0;
	}
tObject *objP = OBJECTS + nObject;
if (lightObjP->info.xLifeLeft < objP->info.xLifeLeft)
	lightObjP->info.xLifeLeft = objP->info.xLifeLeft;
if (!lightObjP->cType.lightInfo.nObjects++) {
	lightObjP->info.position.vPos = objP->info.position.vPos;
	lightObjP->cType.lightInfo.nSegment = objP->info.nSegment;
	}
else {
	lightObjP->info.position.vPos += objP->info.position.vPos;
	if (lightObjP->cType.lightInfo.nSegment != objP->info.nSegment)
		lightObjP->cType.lightInfo.nSegment = -lightObjP->info.nSegment;
	}
lightObjP->cType.lightInfo.intensity += xObjIntensity;
if (color) {
	lightObjP->cType.lightInfo.color.red += color->red;
	lightObjP->cType.lightInfo.color.green += color->green;
	lightObjP->cType.lightInfo.color.blue += color->blue;
	lightObjP->cType.lightInfo.color.alpha += color->alpha;
	}
else {
	lightObjP->cType.lightInfo.color.red += 1;
	lightObjP->cType.lightInfo.color.green += 1;
	lightObjP->cType.lightInfo.color.blue += 1;
	lightObjP->cType.lightInfo.color.alpha += 1;
	}
return 1;
}

//--------------------------------------------------------------------------

int LightingMethod (void)
{
if (gameOpts->render.nLightingMethod == 1)
	return 2 + gameStates.render.bAmbientColor;
else if (gameOpts->render.nLightingMethod == 2)
	return 4;
return gameStates.render.bAmbientColor;
}

// ----------------------------------------------------------------------------------------------
//	Return true if we think vertex nVertex is visible from tSegment nSegment.
//	If some amount of time has gone by, then recompute, else use cached value.

int LightingCacheVisible (int nVertex, int nSegment, int nObject, vmsVector *vObjPos, int nObjSeg, vmsVector *vVertPos)
{
	int	cache_val = lightingCache [((nSegment << LIGHTING_CACHE_SHIFT) ^ nVertex) & (LIGHTING_CACHE_SIZE-1)];
	int	cache_frame = cache_val >> 1;
	int	cache_vis = cache_val & 1;

nCacheLookups++;
if ((cache_frame == 0) || (cache_frame + nLightingFrameDelta <= gameData.app.nFrameCount)) {
	int			bApplyLight = 0;
	tFVIQuery	fq;
	tFVIData		hit_data;
	int			nSegment, hitType;
	nSegment = -1;
	#if DBG
	nSegment = FindSegByPos (*vObjPos, nObjSeg, 1, 0);
	if (nSegment == -1) {
		Int3 ();		//	Obj_pos is not in nObjSeg!
		return 0;		//	Done processing this tObject.
	}
	#endif
	fq.p0					= vObjPos;
	fq.startSeg			= nObjSeg;
	fq.p1					= vVertPos;
	fq.radP0				=
	fq.radP1				= 0;
	fq.thisObjNum		= nObject;
	fq.ignoreObjList	= NULL;
	fq.flags				= FQ_TRANSWALL;
	hitType = FindVectorIntersection (&fq, &hit_data);
	// gameData.ai.vHitPos = gameData.ai.hitData.hit.vPoint;
	// gameData.ai.nHitSeg = gameData.ai.hitData.hit_seg;
	if (hitType == HIT_OBJECT)
		return bApplyLight;	//	Hey, we're not supposed to be checking objects!
	if (hitType == HIT_NONE)
		bApplyLight = 1;
	else if (hitType == HIT_WALL) {
		fix distDist = vmsVector::Dist(hit_data.hit.vPoint, *vObjPos);
		if (distDist < F1_0/4) {
			bApplyLight = 1;
			// -- Int3 ();	//	Curious, did fvi detect intersection with tWall containing vertex?
			}
		}
	lightingCache [((nSegment << LIGHTING_CACHE_SHIFT) ^ nVertex) & (LIGHTING_CACHE_SIZE-1)] = bApplyLight + (gameData.app.nFrameCount << 1);
	return bApplyLight;
	}
nCacheHits++;
return cache_vis;
}

// ----------------------------------------------------------------------------------------------

void InitDynColoring (void)
{
if (!gameOpts->render.nLightingMethod && gameData.render.lights.bInitDynColoring) {
	gameData.render.lights.bInitDynColoring = 0;
	memset (gameData.render.lights.bGotDynColor, 0, sizeof (*gameData.render.lights.bGotDynColor) * MAX_VERTICES);
	}
gameData.render.lights.bGotGlobalDynColor = 0;
gameData.render.lights.bStartDynColoring = 0;
}

// ----------------------------------------------------------------------------------------------

void SetDynColor (tRgbaColorf *color, tRgbColorf *pDynColor, int nVertex, char *pbGotDynColor, int bForce)
{
if (!color)
	return;
#if 1
if (!bForce && (color->red == 1.0) && (color->green == 1.0) && (color->blue == 1.0))
	return;
#endif
if (gameData.render.lights.bStartDynColoring) {
	InitDynColoring ();
	}
if (!pDynColor) {
	SetDynColor (color, &gameData.render.lights.globalDynColor, 0, &gameData.render.lights.bGotGlobalDynColor, bForce);
	pDynColor = gameData.render.lights.dynamicColor + nVertex;
	pbGotDynColor = gameData.render.lights.bGotDynColor + nVertex;
	}
if (*pbGotDynColor) {
	pDynColor->red = (pDynColor->red + color->red) / 2;
	pDynColor->green = (pDynColor->green + color->green) / 2;
	pDynColor->blue = (pDynColor->blue + color->blue) / 2;
	}
else {
	memcpy (pDynColor, color, sizeof (tRgbColorf));
	*pbGotDynColor = 1;
	}
}

// ----------------------------------------------------------------------------------------------

bool SkipPowerup (tObject *objP)
{
if (objP->info.nType != OBJ_POWERUP)
	return false;
if (!EGI_FLAG (bPowerupLights, 0, 0, 0))
	return true;
if (gameStates.render.bPerPixelLighting == 2) {
	int id = objP->info.nId;
	if ((id != POW_EXTRA_LIFE) && (id != POW_ENERGY) && (id != POW_SHIELD_BOOST) &&
		 (id != POW_HOARD_ORB) && (id != POW_MONSTERBALL) && (id != POW_INVUL)) {
		return true;
		}
	}
return false;
}

// ----------------------------------------------------------------------------------------------

void ApplyLight (
	fix			xObjIntensity,
	int			nObjSeg,
	vmsVector	*vObjPos,
	int			nRenderVertices,
	short			*renderVertexP,
	int			nObject,
	tRgbaColorf	*color)
{
	int			iVertex, bUseColor, bForceColor;
	int			nVertex;
	int			bApplyLight;
	short			nLightObj;
	ubyte			nObjType;
	vmsVector	*vVertPos;
	fix			dist, xOrigIntensity = xObjIntensity;
	tObject		*lightObjP, *objP = (nObject < 0) ? NULL : OBJECTS + nObject;
	tPlayer		*playerP = objP ? gameData.multiplayer.players + objP->info.nId : NULL;

nObjType = objP ? objP->info.nType : OBJ_NONE;
if (objP && SHOW_DYN_LIGHT) {
	if (nObjType == OBJ_PLAYER) {
		if (EGI_FLAG (headlight.bAvailable, 0, 0, 0)) {
			if (!HeadlightIsOn (objP->info.nId))
				RemoveOglHeadlight (objP);
			else if (gameData.render.lights.dynamic.nHeadlights [objP->info.nId] < 0)
				gameData.render.lights.dynamic.nHeadlights [objP->info.nId] = AddOglHeadlight (objP);
			}
		else {
			if (HeadlightIsOn (objP->info.nId)) {
				playerP->flags &= ~PLAYER_FLAGS_HEADLIGHT_ON;
				HUDInitMessage (TXT_NO_HEADLIGHTS);
				}
			}
		if (gameData.render.vertColor.bDarkness)
			return;
		xObjIntensity /= 4;
		}
	else if (nObjType == OBJ_POWERUP) {
		xObjIntensity /= 4;
		}
	else if (nObjType == OBJ_ROBOT)
		xObjIntensity /= 4;
	else if ((nObjType == OBJ_FIREBALL) || (nObjType == OBJ_EXPLOSION)) {
		xObjIntensity /= 2;
		}
#if DBG
	if (nObject == nDbgObj)
		nDbgObj = nDbgObj;
#endif
	if (0 > (nLightObj = gameData.objs.lightObjs [nObject].nObject))
		lightObjP = NULL;
	else
		lightObjP = OBJECTS + nLightObj;
	if (!InitClusterLight (nObject, color, xObjIntensity))
		AddDynLight (NULL, color, xObjIntensity, -1, -1, nObject, -1, NULL);
	return;
	}

if (xObjIntensity) {
	fix	obji_64 = xObjIntensity * 64;

	if (gameData.render.vertColor.bDarkness) {
		if (nObjType == OBJ_PLAYER)
			xObjIntensity = 0;
		}
	if (objP && (nObjType == OBJ_POWERUP) && !EGI_FLAG (bPowerupLights, 0, 0, 0))
		return;
	bUseColor = (color != NULL); //&& (color->red < 1.0 || color->green < 1.0 || color->blue < 1.0);
	bForceColor = objP && ((nObjType == OBJ_WEAPON) || (nObjType == OBJ_FIREBALL) || (nObjType == OBJ_EXPLOSION));
	// for pretty dim sources, only process vertices in tObject's own tSegment.
	//	12/04/95, MK, markers only cast light in own tSegment.
	if (objP && ((abs (obji_64) <= F1_0 * 8) || (nObjType == OBJ_MARKER))) {
		short *vp = gameData.segs.segments [nObjSeg].verts;
		for (iVertex = 0; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++) {
			nVertex = vp [iVertex];
#if !FLICKERFIX
			if ((nVertex ^ gameData.app.nFrameCount) & 1)
#endif
			{
				vVertPos = gameData.segs.vertices + nVertex;
				dist = vmsVector::Dist(*vObjPos, *vVertPos) / 4;
				dist = FixMul (dist, dist);
				if (dist < abs (obji_64)) {
					if (dist < MIN_LIGHT_DIST)
						dist = MIN_LIGHT_DIST;
					gameData.render.lights.dynamicLight [nVertex] += FixDiv (xObjIntensity, dist);
					if (bUseColor)
						SetDynColor (color, NULL, nVertex, NULL, 0);
					}
				}
			}
		}
	else {
		int	headlightShift = 0;
		fix	maxHeadlightDist = F1_0 * 200;
		if (objP && (nObjType == OBJ_PLAYER))
			if ((gameStates.render.bHeadlightOn = HeadlightIsOn (objP->info.nId))) {
				headlightShift = 3;
				if (color) {
					bUseColor = bForceColor = 1;
					color->red = color->green = color->blue = 1.0;
					}
				if (objP->info.nId != gameData.multiplayer.nLocalPlayer) {
					vmsVector	tvec;
					tFVIQuery	fq;
					tFVIData		hit_data;
					int			fate;
					tvec = *vObjPos + objP->info.position.mOrient[FVEC] * F1_0*200;
					fq.startSeg			= objP->info.nSegment;
					fq.p0					= vObjPos;
					fq.p1					= &tvec;
					fq.radP0				=
					fq.radP1				= 0;
					fq.thisObjNum		= nObject;
					fq.ignoreObjList	= NULL;
					fq.flags				= FQ_TRANSWALL;
					fate = FindVectorIntersection (&fq, &hit_data);
					if (fate != HIT_NONE) {
						tvec = hit_data.hit.vPoint - *vObjPos;
						maxHeadlightDist = tvec.Mag() + F1_0*4;
					}
				}
			}
		// -- for (iVertex=gameData.app.nFrameCount&1; iVertex<nRenderVertices; iVertex+=2) {
		for (iVertex = 0; iVertex < nRenderVertices; iVertex++) {
			nVertex = renderVertexP [iVertex];
#if DBG
			if (nVertex == nDbgVertex)
				nVertex = nVertex;
#endif
#if FLICKERFIX == 0
			if ((nVertex ^ gameData.app.nFrameCount) & 1)
#endif
			{
				vVertPos = gameData.segs.vertices + nVertex;
				dist = vmsVector::Dist (*vObjPos, *vVertPos);
				bApplyLight = 0;
				if ((dist >> headlightShift) < abs (obji_64)) {
					if (dist < MIN_LIGHT_DIST)
						dist = MIN_LIGHT_DIST;
#if 0
					bApplyLight = 1;
					if (bApplyLight)
#endif
					{
						if (bUseColor)
							SetDynColor (color, NULL, nVertex, NULL, bForceColor);
						if (!headlightShift)
							gameData.render.lights.dynamicLight [nVertex] += FixDiv (xObjIntensity, dist);
						else {
							fix			dot, maxDot;
							int			spotSize = gameData.render.vertColor.bDarkness ? 2 << (3 - extraGameInfo [1].nSpotSize) : 1;
							vmsVector	vecToPoint;
							vecToPoint = *vVertPos - *vObjPos;
							vmsVector::Normalize(vecToPoint);		//	MK, Optimization note: You compute distance about 15 lines up, this is partially redundant
							dot = vmsVector::Dot(vecToPoint, objP->info.position.mOrient[FVEC]);
							if (gameData.render.vertColor.bDarkness)
								maxDot = F1_0 / spotSize;
							else
								maxDot = F1_0 / 2;
							if (dot < maxDot)
								gameData.render.lights.dynamicLight [nVertex] += FixDiv (xOrigIntensity, FixMul (HEADLIGHT_SCALE, dist));	//	Do the Normal thing, but darken around headlight.
							else if (!IsMultiGame || (dist < maxHeadlightDist))
								gameData.render.lights.dynamicLight [nVertex] += FixMul (FixMul (dot, dot), xOrigIntensity) / 8;//(8 * spotSize);
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------

#define	FLASH_LEN_FIXED_SECONDS	 (F1_0/3)
#define	FLASH_SCALE					 (3*F1_0/FLASH_LEN_FIXED_SECONDS)

void CastMuzzleFlashLight (int nRenderVertices, short *renderVertexP)
{
	int	i;
	short	time_since_flash;
	fix currentTime = TimerGetFixedSeconds ();

for (i = 0; i < MUZZLE_QUEUE_MAX; i++) {
	if (gameData.muzzle.info [i].createTime) {
		time_since_flash = (short) (currentTime - gameData.muzzle.info [i].createTime);
		if (time_since_flash < FLASH_LEN_FIXED_SECONDS)
			ApplyLight ((FLASH_LEN_FIXED_SECONDS - time_since_flash) * FLASH_SCALE,
							gameData.muzzle.info [i].nSegment, &gameData.muzzle.info [i].pos,
							nRenderVertices, renderVertexP, -1, NULL);
		else
			gameData.muzzle.info [i].createTime = 0;		// turn off this muzzle flash
		}
	}
}

// ---------------------------------------------------------
//	Translation table to make flares flicker at different rates
fix	objLightXlat [16] =
	{0x1234, 0x3321, 0x2468, 0x1735,
	 0x0123, 0x19af, 0x3f03, 0x232a,
	 0x2123, 0x39af, 0x0f03, 0x132a,
	 0x3123, 0x29af, 0x1f03, 0x032a};

fix ComputeLightIntensity (int nObject, tRgbaColorf *colorP, char *pbGotColor)
{
	tObject		*objP = OBJECTS + nObject;
	int			nObjType = objP->info.nType;
   fix			hoardlight, s;
	static tRgbaColorf powerupColors [9] = {
		{0,1,0,1},{1,0.8f,0,1},{0,0,1,1},{1,1,1,1},{0,0,1,1},{1,0,0,1},{1,0.8f,0,1},{0,1,0,1},{1,0.8f,0,1}
	};
	static tRgbaColorf missileColor = {1,0.3f,0,1};

colorP->red =
colorP->green =
colorP->blue = 1.0;
*pbGotColor = 0;
switch (nObjType) {
	case OBJ_PLAYER:
		*pbGotColor = 1;
		 if (HeadlightIsOn (objP->info.nId)) {
			if (nHeadlights < MAX_HEADLIGHTS)
				Headlights [nHeadlights++] = objP;
			return HEADLIGHT_SCALE;
			}
		 else if ((gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)) && gameData.multiplayer.players [objP->info.nId].secondaryAmmo [PROXMINE_INDEX]) {

		// If hoard game and tPlayer, add extra light based on how many orbs you have
		// Pulse as well.
		  	hoardlight = I2X (gameData.multiplayer.players [objP->info.nId].secondaryAmmo [PROXMINE_INDEX])/2; //I2X (12);
			hoardlight++;
		   FixSinCos ((gameData.time.xGame/2) & 0xFFFF,&s,NULL); // probably a bad way to do it
			s+=F1_0;
			s>>=1;
			hoardlight = FixMul (s,hoardlight);
		   return (hoardlight);
		  }
		else if (objP->info.nId == gameData.multiplayer.nLocalPlayer) {
			return max (gameData.physics.playerThrust.Mag()/4, F1_0*2) + F1_0/2;
			}
		else {
			return max (objP->mType.physInfo.thrust.Mag()/4, F1_0*2) + F1_0/2;
			}
		break;

	case OBJ_FIREBALL:
	case OBJ_EXPLOSION:
		if (objP->info.nId == 0xff)
			return 0;
		if ((objP->info.renderType == RT_THRUSTER) || (objP->info.renderType == RT_EXPLBLAST) || (objP->info.renderType == RT_SHRAPNELS))
			return 0;
		else {
			tVideoClip *vcP = gameData.eff.vClips [0] + objP->info.nId;
			fix xLight = vcP->lightValue;
			int i, j;
			grsBitmap *bmP;
			if ((bmP = BM_OVERRIDE (gameData.pig.tex.pBitmaps + vcP->frames [0].index))) {
				colorP->red = (float) bmP->bmAvgRGB.red;
				colorP->green = (float) bmP->bmAvgRGB.green;
				colorP->blue = (float) bmP->bmAvgRGB.blue;
				*pbGotColor = 1;
				}
			else {
				colorP->red =
				colorP->green =
				colorP->blue = 0.0f;
				for (i = j = 0; i < vcP->nFrameCount; i++) {
					bmP = gameData.pig.tex.bitmaps [0] + vcP->frames [i].index;
					if (bmP->bmAvgRGB.red + bmP->bmAvgRGB.green + bmP->bmAvgRGB.blue == 0)
						if (!BitmapColor (bmP, bmP->bmTexBuf))
							continue;
					colorP->red += (float) bmP->bmAvgRGB.red / 255.0f;
					colorP->green += (float) bmP->bmAvgRGB.green / 255.0f;
					colorP->blue += (float) bmP->bmAvgRGB.blue / 255.0f;
					j++;
					}
				if (j) {
					colorP->red /= j;
					colorP->green /= j;
					colorP->blue /= j;
					*pbGotColor = 1;
					}
				}
#if 0
			if (objP->info.renderType != RT_THRUSTER)
				xLight /= 8;
#endif
			float maxColor = colorP->red;
			if (maxColor < colorP->green)
				maxColor = colorP->green;
			if (maxColor < colorP->blue)
				maxColor = colorP->blue;
			if (maxColor > 1) {
				colorP->red /= maxColor;
				colorP->green /= maxColor;
				colorP->blue /= maxColor;
				}
			if (objP->info.xLifeLeft < F1_0*4)
				return FixMul (FixDiv (objP->info.xLifeLeft,
								   gameData.eff.vClips [0][objP->info.nId].xTotalTime), xLight);
			else
				return xLight;
			}
		break;

	case OBJ_ROBOT:
		*pbGotColor = 1;
#if 0//def _DEBUG
		return ROBOTINFO (objP->info.nId).lighting;
#else
		return ROBOTINFO (objP->info.nId).lightcast ? ROBOTINFO (objP->info.nId).lighting ? ROBOTINFO (objP->info.nId).lighting : F1_0 : 0;
#endif
		break;

	case OBJ_WEAPON: {
		fix tval = gameData.weapons.info [objP->info.nId].light;
		if (gameData.objs.bIsMissile [objP->info.nId])
			*colorP = missileColor;
		else if (gameOpts->render.color.bGunLight)
			*colorP = gameData.weapons.color [objP->info.nId];
		*pbGotColor = 1;
		if (IsMultiGame)
			if (objP->info.nId == OMEGA_ID)
				if (d_rand () > 8192)
					return 0;		//	3/4 of time, omega blobs will cast 0 light!
		if (objP->info.nId == FLARE_ID) {
			return 2 * (min (tval, objP->info.xLifeLeft) + ((gameData.time.xGame ^ objLightXlat [nObject & 0x0f]) & 0x3fff));
			}
		else
			return tval;
		}

	case OBJ_MARKER: {
		fix	lightval = objP->info.xLifeLeft;
		lightval &= 0xffff;
		lightval = 8 * abs (F1_0/2 - lightval);
		if (objP->info.xLifeLeft < F1_0*1000)
			objP->info.xLifeLeft += F1_0;	//	Make sure this tObject doesn't go out.
		colorP->red = 0.1f;
		colorP->green = 1.0f;
		colorP->blue = 0.1f;
		*pbGotColor = 1;
		return lightval;
		}

	case OBJ_POWERUP:
		if (objP->info.nId < 9)
			*colorP = powerupColors [objP->info.nId];
		*pbGotColor = 1;
		return gameData.objs.pwrUp.info [objP->info.nId].light;
		break;

	case OBJ_DEBRIS:
		return F1_0/4;
		break;

	case OBJ_LIGHT:
		return objP->cType.lightInfo.intensity;
		break;

	default:
		return 0;
		break;
	}
}

// ----------------------------------------------------------------------------------------------

void SetDynamicLight (void)
{
	int			iVertex, nv;
	int			nObject, nVertex, nSegment;
	int			nRenderVertices;
	int			iRenderSeg, v;
	char			bGotColor, bKeepDynColoring = 0;
	tObject		*objP;
	vmsVector	*objPos;
	fix			xObjIntensity;
	tRgbaColorf	color;
	
nHeadlights = 0;
if (!gameOpts->render.debug.bDynamicLight)
	return;
memset (gameData.render.lights.vertexFlags, 0, gameData.segs.nLastVertex + 1);
gameData.render.vertColor.bDarkness = IsMultiGame && gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [IsMultiGame].bDarkness;
gameData.render.lights.bStartDynColoring = 1;
if (gameData.render.lights.bInitDynColoring) {
	InitDynColoring ();
	}
ResetClusterLights ();
//	Create list of vertices that need to be looked at for setting of ambient light.
nRenderVertices = 0;
if (!gameOpts->render.nLightingMethod) {
	for (iRenderSeg = 0; iRenderSeg < gameData.render.mine.nRenderSegs; iRenderSeg++) {
		nSegment = gameData.render.mine.nSegRenderList [iRenderSeg];
		if (nSegment != -1) {
			short	*vp = gameData.segs.segments [nSegment].verts;
			for (v = 0; v < MAX_VERTICES_PER_SEGMENT; v++) {
				nv = vp [v];
				if ((nv < 0) || (nv > gameData.segs.nLastVertex)) {
					Int3 ();		//invalid vertex number
					continue;	//ignore it, and go on to next one
					}
				if (!gameData.render.lights.vertexFlags [nv]) {
					Assert (nRenderVertices < MAX_VERTICES);
					gameData.render.lights.vertexFlags [nv] = 1;
					gameData.render.lights.vertices [nRenderVertices++] = nv;
					}
				}
			}
		}
	for (iVertex = 0; iVertex < nRenderVertices; iVertex++) {
		nVertex = gameData.render.lights.vertices [iVertex];
		Assert (nVertex >= 0 && nVertex <= gameData.segs.nLastVertex);
#if FLICKERFIX == 0
		if ((nVertex ^ gameData.app.nFrameCount) & 1)
#endif
			{
			gameData.render.lights.dynamicLight [nVertex] = 0;
			gameData.render.lights.bGotDynColor [nVertex] = 0;
			memset (gameData.render.lights.dynamicColor + nVertex, 0, sizeof (*gameData.render.lights.dynamicColor));
			}
		}
	}
CastMuzzleFlashLight (nRenderVertices, gameData.render.lights.vertices);
memset (gameData.render.lights.newObjects, 0, sizeof (gameData.render.lights.newObjects));
if (EGI_FLAG (bUseLightnings, 0, 0, 1) && !gameOpts->render.nLightingMethod) {
	tLightningLight	*pll;
	for (iRenderSeg = 0; iRenderSeg < gameData.render.mine.nRenderSegs; iRenderSeg++) {
		nSegment = gameData.render.mine.nSegRenderList [iRenderSeg];
		pll = gameData.lightnings.lights + nSegment;
		if (pll->nFrame == gameData.app.nFrameCount)
			ApplyLight (pll->nBrightness, nSegment, &pll->vPos, nRenderVertices, gameData.render.lights.vertices, -1, &pll->color);
		}
	}
//	July 5, 1995: New faster dynamic lighting code.  About 5% faster on the PC (un-optimized).
//	Only objects which are in rendered segments cast dynamic light.  We might want to extend this
//	one or two segments if we notice light changing as OBJECTS go offscreen.  I couldn't see any
//	serious visual degradation.  In fact, I could see no humorous degradation, either. --MK
FORALL_OBJS (objP, nObject) {
	if (objP->info.nType == OBJ_NONE)
		continue;
	if (SkipPowerup (objP))
		continue;
	nObject = OBJ_IDX (objP);
	objPos = &objP->info.position.vPos;
	xObjIntensity = ComputeLightIntensity (nObject, &color, &bGotColor);
	if (bGotColor)
		bKeepDynColoring = 1;
	if (xObjIntensity) {
		ApplyLight (xObjIntensity, objP->info.nSegment, objPos, nRenderVertices, gameData.render.lights.vertices, nObject, bGotColor ? &color : NULL);
		gameData.render.lights.newObjects [nObject] = 1;
		}
	}
//	Now, process all lights from last frame which haven't been processed this frame.
FORALL_OBJS (objP, nObject) {
	nObject = OBJ_IDX (objP);
	//	In multiplayer games, process even unprocessed OBJECTS every 4th frame, else don't know about tPlayer sneaking up.
	if ((gameData.render.lights.objects [nObject]) ||
		 (IsMultiGame && (((nObject ^ gameData.app.nFrameCount) & 3) == 0))) {
		if (gameData.render.lights.newObjects [nObject])
			//	Not lit last frame, so we don't need to light it.  (Already lit if casting light this frame.)
			//	But copy value from gameData.render.lights.newObjects to update gameData.render.lights.objects array.
			gameData.render.lights.objects [nObject] = gameData.render.lights.newObjects [nObject];
		else {
			//	Lit last frame, but not this frame.  Get intensity...
			objPos = &objP->info.position.vPos;
			xObjIntensity = ComputeLightIntensity (nObject, &color, &bGotColor);
			if (bGotColor)
				bKeepDynColoring = 1;
			if (xObjIntensity) {
				ApplyLight (xObjIntensity, objP->info.nSegment, objPos, nRenderVertices, gameData.render.lights.vertices, nObject,
								bGotColor ? &color : NULL);
				gameData.render.lights.objects [nObject] = 1;
				}
			else
				gameData.render.lights.objects [nObject] = 0;
			}
		}
	}
SetClusterLights ();
if (!bKeepDynColoring)
	InitDynColoring ();
}

// ----------------------------------------------------------------------------------------------
//compute the average dynamic light in a tSegment.  Takes the tSegment number

fix ComputeSegDynamicLight (int nSegment)
{
short *verts = gameData.segs.segments [nSegment].verts;
fix sum = gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts++];
sum += gameData.render.lights.dynamicLight [*verts];
return sum >> 3;
}
// ----------------------------------------------------------------------------------------------

tObject *oldViewer;
int bResetLightingHack;

#define LIGHT_RATE I2X (4)		//how fast the light ramps up

void StartLightingFrame (tObject *viewer)
{
bResetLightingHack = (viewer != oldViewer);
oldViewer = viewer;
}

// ----------------------------------------------------------------------------------------------
//compute the lighting for an tObject.  Takes a pointer to the tObject,
//and possibly a rotated 3d point.  If the point isn't specified, the
//object's center point is rotated.
fix ComputeObjectLight (tObject *objP, vmsVector *vRotated)
{
#if DBG
   if (!objP)
      return 0;
#endif
	fix light;
	int nObject = OBJ_IDX (objP);
	if (nObject < 0)
		return F1_0;
	if (nObject >= gameData.objs.nLastObject [0])
		return 0;
	//First, get static light for this tSegment
if (gameOpts->render.nLightingMethod && !((RENDERPATH && gameOpts->ogl.bObjLighting) || gameOpts->ogl.bLightObjects)) {
	gameData.objs.color = *AvgSgmColor (objP->info.nSegment, &objP->info.position.vPos);
	light = F1_0;
	}
else
	light = gameData.segs.segment2s [objP->info.nSegment].xAvgSegLight;
//return light;
//Now, maybe return different value to smooth transitions
if (!bResetLightingHack && (gameData.objs.nLightSig [nObject] == objP->info.nSignature)) {
	fix xDeltaLight, xFrameDelta;
	xDeltaLight = light - gameData.objs.xLight [nObject];
	xFrameDelta = FixMul (LIGHT_RATE, gameData.time.xFrame);
	if (abs (xDeltaLight) <= xFrameDelta)
		gameData.objs.xLight [nObject] = light;		//we've hit the goal
	else
		if (xDeltaLight < 0)
			light = gameData.objs.xLight [nObject] -= xFrameDelta;
		else
			light = gameData.objs.xLight [nObject] += xFrameDelta;
	}
else {		//new tObject, initialize
	gameData.objs.nLightSig [nObject] = objP->info.nSignature;
	gameData.objs.xLight [nObject] = light;
	}
//Next, add in headlight on this tObject
// -- Matt code: light += ComputeHeadlight (vRotated,f1_0);
light += ComputeHeadlightLightOnObject (objP);
//Finally, add in dynamic light for this tSegment
light += ComputeSegDynamicLight (objP->info.nSegment);
return light;
}

// ----------------------------------------------------------------------------------------------

void ComputeEngineGlow (tObject *objP, fix *xEngineGlowValue)
{
xEngineGlowValue [0] = f1_0/5;
if (objP->info.movementType == MT_PHYSICS) {
	if ((objP->info.nType == OBJ_PLAYER) && (objP->mType.physInfo.flags & PF_USES_THRUST) && (objP->info.nId == gameData.multiplayer.nLocalPlayer)) {
		fix thrust_mag = objP->mType.physInfo.thrust.Mag();
		xEngineGlowValue [0] += (FixDiv (thrust_mag,gameData.pig.ship.player->maxThrust)*4)/5;
	}
	else {
		fix speed = objP->mType.physInfo.velocity.Mag();
		xEngineGlowValue [0] += (FixDiv (speed, MAX_VELOCITY) * 3) / 5;
		}
	}
//set value for tPlayer headlight
if (objP->info.nType == OBJ_PLAYER) {
	if (PlayerHasHeadlight (objP->info.nId) &&  !gameStates.app.bEndLevelSequence)
		xEngineGlowValue [1] = HeadlightIsOn (objP->info.nId) ? -2 : -1;
	else
		xEngineGlowValue [1] = -3;			//don't draw
	}
}

//-----------------------------------------------------------------------------

void FlickerLights (void)
{
	tVariableLight	*flP = gameData.render.lights.flicker.lights;
	int				l;
	tSide				*sideP;
	short				nSegment, nSide;

for (l = 0; l < gameData.render.lights.flicker.nLights; l++, flP++) {
	//make sure this is actually a light
	if (!(WALL_IS_DOORWAY (gameData.segs.segments + flP->nSegment, flP->nSide, NULL) & WID_RENDER_FLAG))
		continue;
	nSegment = flP->nSegment;
	nSide = flP->nSide;
	sideP = gameData.segs.segments [nSegment].sides + nSide;
	if (!(gameData.pig.tex.brightness [sideP->nBaseTex] ||
			gameData.pig.tex.brightness [sideP->nOvlTex]))
		continue;
	if (flP->timer == (fix) 0x80000000)		//disabled
		continue;
	if ((flP->timer -= gameData.time.xFrame) < 0) {
		while (flP->timer < 0)
			flP->timer += flP->delay;
		flP->mask = ((flP->mask & 0x80000000) ? 1 : 0) + (flP->mask << 1);
		if (flP->mask & 1)
			AddLight (nSegment, nSide);
		else if (EGI_FLAG (bFlickerLights, 1, 0, 1))
			SubtractLight (nSegment, nSide);
		}
	}
}
//-----------------------------------------------------------------------------
//returns ptr to flickering light structure, or NULL if can't find
tVariableLight *FindVariableLight (int nSegment,int nSide)
{
	tVariableLight *flP = gameData.render.lights.flicker.lights;

for (int l = 0; l < gameData.render.lights.flicker.nLights; l++, flP++)
	if ((flP->nSegment == nSegment) && (flP->nSide == nSide))	//found it!
		return flP;
return NULL;
}
//-----------------------------------------------------------------------------
//turn flickering off (because light has been turned off)
void DisableVariableLight (int nSegment,int nSide)
{
tVariableLight *flP = FindVariableLight (nSegment ,nSide);

if (flP)
	flP->timer = 0x80000000;
}

//-----------------------------------------------------------------------------
//turn flickering off (because light has been turned on)
void EnableVariableLight (int nSegment,int nSide)
{
	tVariableLight *flP = FindVariableLight (nSegment, nSide);

if (flP)
	flP->timer = 0;
}

#ifdef EDITOR
//returns 1 if ok, 0 if error
int AddVariableLight (int nSegment, int nSide, fix delay, unsigned int mask)
{
	int l;
	tVariableLight *flP;
#if TRACE
	//con_printf (CONDBG,"AddVariableLight: %d:%d %x %x\n",nSegment,nSide,delay,mask);
#endif
	//see if there's already an entry for this seg/tSide
	flP = gameData.render.lights.flicker.lights;
	for (l = 0; l < gameData.render.lights.flicker.nLights; l++, flP++)
		if ((flP->nSegment == nSegment) && (flP->nSide == nSide))	//found it!
			break;
	if (mask==0) {		//clearing entry
		if (l == gameData.render.lights.flicker.nLights)
			return 0;
		else {
			int i;
			for (i=l;i<gameData.render.lights.flicker.nLights-1;i++)
				gameData.render.lights.flicker.lights[i] = gameData.render.lights.flicker.lights[i+1];
			gameData.render.lights.flicker.nLights--;
			return 1;
		}
	}
	if (l == gameData.render.lights.flicker.nLights) {
		if (gameData.render.lights.flicker.nLights == MAX_FLICKERING_LIGHTS)
			return 0;
		else
			gameData.render.lights.flicker.nLights++;
	}
	flP->nSegment = nSegment;
	flP->nSide = nSide;
	flP->delay = flP->timer = delay;
	flP->mask = mask;
	return 1;
}
#endif

//------------------------------------------------------------------------------

int IsLight (int tMapNum)
{
return gameData.pig.tex.brightness [tMapNum];
}

//	------------------------------------------------------------------------------------------
//cast static light from a tSegment to nearby segments
//these constants should match the ones in seguvs
#define	LIGHT_DISTANCE_THRESHOLD	 (F1_0*80)
#define	MAGIC_LIGHT_CONSTANT			 (F1_0*16)

#define MAX_CHANGED_SEGS 30
short changedSegs [MAX_CHANGED_SEGS];
int nChangedSegs;

void ApplyLightToSegment (tSegment *segP, vmsVector *vSegCenter, fix xBrightness, int nCallDepth)
{
	vmsVector	rSegmentCenter;
	fix			xDistToRSeg;
	int 			i;
	short			nSide,
					nSegment = SEG_IDX (segP);

for (i = 0; i <nChangedSegs; i++)
	if (changedSegs [i] == nSegment)
		break;
if (i == nChangedSegs) {
	COMPUTE_SEGMENT_CENTER (&rSegmentCenter, segP);
	xDistToRSeg = vmsVector::Dist(rSegmentCenter, *vSegCenter);

	if (xDistToRSeg <= LIGHT_DISTANCE_THRESHOLD) {
		fix	xLightAtPoint = (xDistToRSeg > F1_0) ?
									 FixDiv (MAGIC_LIGHT_CONSTANT, xDistToRSeg) :
									 MAGIC_LIGHT_CONSTANT;

		if (xLightAtPoint >= 0) {
			tSegment2 *seg2P = gameData.segs.segment2s + nSegment;
			xLightAtPoint = FixMul (xLightAtPoint, xBrightness);
			if (xLightAtPoint >= F1_0)
				xLightAtPoint = F1_0-1;
			else if (xLightAtPoint <= -F1_0)
				xLightAtPoint = - (F1_0-1);
			seg2P->xAvgSegLight += xLightAtPoint;
			if (seg2P->xAvgSegLight < 0)	// if it went negative, saturate
				seg2P->xAvgSegLight = 0;
			}	//	end if (xLightAtPoint...
		}	//	end if (xDistToRSeg...
	changedSegs [nChangedSegs++] = nSegment;
	}

if (nCallDepth < 2)
	for (nSide=0; nSide<6; nSide++) {
		if (WALL_IS_DOORWAY (segP, nSide, NULL) & WID_RENDPAST_FLAG)
			ApplyLightToSegment (&gameData.segs.segments [segP->children [nSide]], vSegCenter, xBrightness, nCallDepth+1);
		}
}

extern tObject *oldViewer;

//	------------------------------------------------------------------------------------------
//update the xAvgSegLight field in a tSegment, which is used for tObject lighting
//this code is copied from the editor routine calim_process_all_lights ()
void ChangeSegmentLight (short nSegment, short nSide, int dir)
{
	tSegment *segP = gameData.segs.segments+nSegment;

if (WALL_IS_DOORWAY (segP, nSide, NULL) & WID_RENDER_FLAG) {
	tSide	*sideP = segP->sides+nSide;
	fix	xBrightness;
	xBrightness = gameData.pig.tex.pTMapInfo [sideP->nBaseTex].lighting + gameData.pig.tex.pTMapInfo [sideP->nOvlTex].lighting;
	xBrightness *= dir;
	nChangedSegs = 0;
	if (xBrightness) {
		vmsVector	vSegCenter;
		COMPUTE_SEGMENT_CENTER (&vSegCenter, segP);
		ApplyLightToSegment (segP, &vSegCenter, xBrightness, 0);
		}
	}
//this is a horrible hack to get around the horrible hack used to
//smooth lighting values when an tObject moves between segments
oldViewer = NULL;
}

//	------------------------------------------------------------------------------------------

int FindDLIndexD2X (short nSegment, short nSide)
{
int	m,
		l = 0,
		r = gameData.render.lights.nStatic;
tLightDeltaIndex	*p;
do {
	m = (l + r) / 2;
	p = gameData.render.lights.deltaIndices + m;
	if ((nSegment < p->d2x.nSegment) || ((nSegment == p->d2x.nSegment) && (nSide < p->d2x.nSide)))
		r = m - 1;
	else if ((nSegment > p->d2x.nSegment) || ((nSegment == p->d2x.nSegment) && (nSide > p->d2x.nSide)))
		l = m + 1;
	else {
		while ((p->d2x.nSegment == nSegment) && (p->d2x.nSide == nSide))
			p--;
		return (int) ((p + 1) - gameData.render.lights.deltaIndices);
		}
	} while (l <= r);
return 0;
}

//	------------------------------------------------------------------------------------------

int FindDLIndexD2 (short nSegment, short nSide)
{
int	m,
		l = 0,
		r = gameData.render.lights.nStatic;

tLightDeltaIndex	*p;
do {
	m = (l + r) / 2;
	p = gameData.render.lights.deltaIndices + m;
	if ((nSegment < p->d2.nSegment) || ((nSegment == p->d2.nSegment) && (nSide < p->d2.nSide)))
		r = m - 1;
	else if ((nSegment > p->d2.nSegment) || ((nSegment == p->d2.nSegment) && (nSide > p->d2.nSide)))
		l = m + 1;
	else {
		while ((p->d2.nSegment == nSegment) && (p->d2.nSide == nSide))
			p--;
		return (int) ((p + 1) - gameData.render.lights.deltaIndices);
		}
	} while (l <= r);
return 0;
}

//	------------------------------------------------------------------------------------------

int FindDLIndex (short nSegment, short nSide)
{
return gameStates.render.bD2XLights ?
		 FindDLIndexD2X (nSegment, nSide) :
		 FindDLIndexD2 (nSegment, nSide);
}

//	------------------------------------------------------------------------------------------
//	dir = +1 -> add light
//	dir = -1 -> subtract light
//	dir = 17 -> add 17x light
//	dir =  0 -> you are dumb
void ChangeLight (short nSegment, short nSide, int dir)
{
	int					i, j, k;
	fix					dl, lNew, *pSegLightDelta;
	tUVL					*uvlP;
	tLightDeltaIndex	*dliP;
	tLightDelta			*dlP;
	short					iSeg, iSide;

if ((dir < 0) && RemoveDynLight (nSegment, nSide, -1))
	return;
if (ToggleDynLight (nSegment, nSide, -1, dir >= 0) >= 0)
	return;
i = FindDLIndex (nSegment, nSide);
for (dliP = gameData.render.lights.deltaIndices + i; i < gameData.render.lights.nStatic; i++, dliP++) {
	if (gameStates.render.bD2XLights) {
		iSeg = dliP->d2x.nSegment;
		iSide = dliP->d2x.nSide;
		}
	else {
		iSeg = dliP->d2.nSegment;
		iSide = dliP->d2.nSide;
		}
#if !DBG
	if ((iSeg > nSegment) || ((iSeg == nSegment) && (iSide > nSide)))
		return;
#endif
	if ((iSeg == nSegment) && (iSide == nSide)) {
		if (dliP->d2.index >= MAX_DELTA_LIGHTS)
			continue;	//ouch - bogus data!
		dlP = gameData.render.lights.deltas + dliP->d2.index;
		for (j = (gameStates.render.bD2XLights ? dliP->d2x.count : dliP->d2.count); j; j--, dlP++) {
			if (!dlP->bValid)
				continue;	//bogus data!
			uvlP = gameData.segs.segments [dlP->nSegment].sides [dlP->nSide].uvls;
			pSegLightDelta = gameData.render.lights.segDeltas + dlP->nSegment * 6 + dlP->nSide;
			for (k = 0; k < 4; k++, uvlP++) {
				dl = dir * dlP->vertLight [k] * DL_SCALE;
				lNew = (uvlP->l += dl);
				if (lNew < 0)
					uvlP->l = 0;
				*pSegLightDelta += dl;
				}
			}
		}
	}
//recompute static light for tSegment
ChangeSegmentLight (nSegment, nSide, dir);
}

//	-----------------------------------------------------------------------------
//	Subtract light cast by a light source from all surfaces to which it applies light.
//	This is precomputed data, stored at static light application time in the editor (the slow lighting function).
// returns 1 if lights actually subtracted, else 0
int SubtractLight (short nSegment, short nSide)
{
if (gameData.render.lights.subtracted [nSegment] & (1 << nSide))
	return 0;
gameData.render.lights.subtracted [nSegment] |= (1 << nSide);
ChangeLight (nSegment, nSide, -1);
return 1;
}

//	-----------------------------------------------------------------------------
//	Add light cast by a light source from all surfaces to which it applies light.
//	This is precomputed data, stored at static light application time in the editor (the slow lighting function).
//	You probably only want to call this after light has been subtracted.
// returns 1 if lights actually added, else 0
int AddLight (short nSegment, short nSide)
{
if (!(gameData.render.lights.subtracted [nSegment] & (1 << nSide)))
	return 0;
gameData.render.lights.subtracted [nSegment] &= ~ (1 << nSide);
ChangeLight (nSegment, nSide, 1);
return 1;
}

//	-----------------------------------------------------------------------------
//	Parse the gameData.render.lights.subtracted array, turning on or off all lights.
void ApplyAllChangedLight (void)
{
	short	i, j;
	ubyte	h;

for (i=0; i <= gameData.segs.nLastSegment; i++) {
	h = gameData.render.lights.subtracted [i];
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++)
		if (h & (1 << j))
			ChangeLight (i, j, -1);
	}
}

//	-----------------------------------------------------------------------------
//	Should call this whenever a new mine gets loaded.
//	More specifically, should call this whenever something global happens
//	to change the status of static light in the mine.
void ClearLightSubtracted (void)
{
memset (gameData.render.lights.subtracted, 0,
		  gameData.segs.nLastSegment * sizeof (gameData.render.lights.subtracted [0]));
}

// ----------------------------------------------------------------------------------------------
//eof
