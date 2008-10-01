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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "inferno.h"
#include "error.h"
#include "gamefont.h"
#include "texmap.h"
#include "render.h"
#include "fastrender.h"
#include "rendershadows.h"
#include "transprender.h"
#include "renderthreads.h"
#include "glare.h"
#include "radar.h"
#include "objrender.h"
#include "textures.h"
#include "screens.h"
#include "segpoint.h"
#include "texmerge.h"
#include "physics.h"
#include "gameseg.h"
#include "light.h"
#include "dynlight.h"
#include "headlight.h"
#include "newdemo.h"
#include "automap.h"
#include "key.h"
#include "u_mem.h"
#include "kconfig.h"
#include "mouse.h"
#include "interp.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_render.h"
#include "ogl_fastrender.h"
#include "gauges.h"
#include "input.h"
#include "shadows.h"
#include "textdata.h"
#include "sparkeffect.h"
#include "createmesh.h"

//------------------------------------------------------------------------------

#if LIGHTMAPS
#	define LMAP_LIGHTADJUST	1
#else
#	define LMAP_LIGHTADJUST	0
#endif

#define bPreDrawSegs			0

#define OLD_SEGLIST			1
#define SORT_RENDER_SEGS	0

//------------------------------------------------------------------------------

typedef struct tPortal {
	fix	left, right, top, bot;
	char  bProjected;
	ubyte bVisible;
} tPortal;

// ------------------------------------------------------------------------------

void StartLightingFrame (tObject *viewer);
void ShowReticle(int force_big);

unsigned int	nClearWindowColor = 0;
int				nClearWindow = 2;	// 1 = Clear whole background tPortal, 2 = clear view portals into rest of world, 0 = no clear

void RenderSkyBox (int nWindow);

//------------------------------------------------------------------------------

extern int bLog;

gsrCanvas * reticleCanvas = NULL;

void _CDECL_ FreeReticleCanvas (void)
{
if (reticleCanvas) {
	PrintLog ("unloading reticle data\n");
	D2_FREE( reticleCanvas->cvBitmap.bmTexBuf);
	D2_FREE( reticleCanvas);
	reticleCanvas	= NULL;
	}
}

//------------------------------------------------------------------------------

// Draw the reticle in 3D for head tracking
void Draw3DReticle (fix nEyeOffset)
{
	g3sPoint 	reticlePoints [4];
	tUVL			tUVL [4];
	g3sPoint		*pointList [4];
	int 			i;
	vmsVector	v1, v2;
	gsrCanvas	*saved_canvas;
	int			saved_interp_method;

//	if (!viewInfo.bUsePlayerHeadAngles) return;

for (i = 0; i < 4; i++) {
	reticlePoints [i].p3_index = -1;
	pointList [i] = reticlePoints + i;
	tUVL [i].l = MAX_LIGHT;
	}
tUVL [0].u =
tUVL [0].v =
tUVL [1].v =
tUVL [3].u = 0;
tUVL [1].u =
tUVL [2].u =
tUVL [2].v =
tUVL [3].v = F1_0;

v1 = gameData.objs.viewer->position.vPos + gameData.objs.viewer->position.mOrient[FVEC] * (F1_0*4);
v1 += gameData.objs.viewer->position.mOrient[RVEC] * nEyeOffset;
v2 = v1 + gameData.objs.viewer->position.mOrient[RVEC] * (-F1_0*1);
v2 += gameData.objs.viewer->position.mOrient[UVEC] * (F1_0*1);
G3TransformAndEncodePoint(&reticlePoints[0], v2);
v2 = v1 + gameData.objs.viewer->position.mOrient[RVEC] * (+F1_0*1);
v2 += gameData.objs.viewer->position.mOrient[UVEC] * (F1_0*1);
G3TransformAndEncodePoint(&reticlePoints[1], v2);
v2 = v1 + gameData.objs.viewer->position.mOrient[RVEC] * (+F1_0*1);
v2 += gameData.objs.viewer->position.mOrient[UVEC] * (-F1_0*1);
G3TransformAndEncodePoint(&reticlePoints[2], v2);
v2 = v1 + gameData.objs.viewer->position.mOrient[RVEC] * (-F1_0*1);
v2 += gameData.objs.viewer->position.mOrient[UVEC] * (-F1_0*1);
G3TransformAndEncodePoint(&reticlePoints[3], v2);

if ( reticleCanvas == NULL)	{
	reticleCanvas = GrCreateCanvas(64, 64);
	if ( !reticleCanvas)
		Error( "Couldn't D2_ALLOC reticleCanvas");
	atexit( FreeReticleCanvas);
	//reticleCanvas->cvBitmap.bmHandle = 0;
	reticleCanvas->cvBitmap.bmProps.flags = BM_FLAG_TRANSPARENT;
	}

saved_canvas = grdCurCanv;
GrSetCurrentCanvas(reticleCanvas);
GrClearCanvas(0);		// Clear to Xparent
ShowReticle(1);
GrSetCurrentCanvas(saved_canvas);

saved_interp_method=gameStates.render.nInterpolationMethod;
gameStates.render.nInterpolationMethod	= 3;		// The best, albiet slowest.
G3DrawTexPoly (4, pointList, tUVL, &reticleCanvas->cvBitmap, NULL, 1, -1);
gameStates.render.nInterpolationMethod	= saved_interp_method;
}


//------------------------------------------------------------------------------

//cycle the flashing light for when mine destroyed
void FlashFrame (void)
{
	static fixang flash_ang = 0;

if (!(gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)) {
	gameStates.render.nFlashScale = F1_0;
	return;
	}
if (gameStates.app.bEndLevelSequence)
	return;
if (gameStates.ogl.palAdd.blue > 10)		//whiting out
	return;
//	flash_ang += FixMul(FLASH_CYCLE_RATE, gameData.time.xFrame);
if (gameStates.gameplay.seismic.nMagnitude) {
	fix xAddedFlash = abs(gameStates.gameplay.seismic.nMagnitude);
	if (xAddedFlash < F1_0)
		xAddedFlash *= 16;
	flash_ang += (fixang) FixMul (gameStates.render.nFlashRate, FixMul(gameData.time.xFrame, xAddedFlash+F1_0));
	FixFastSinCos (flash_ang, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + F1_0*3)/4;	//	gets in range 0.5 to 1.0
	}
else {
	flash_ang += (fixang) FixMul (gameStates.render.nFlashRate, gameData.time.xFrame);
	FixFastSinCos (flash_ang, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + f1_0)/2;
	if (gameStates.app.nDifficultyLevel == 0)
		gameStates.render.nFlashScale = (gameStates.render.nFlashScale+F1_0*3)/4;
	}
}

// ----------------------------------------------------------------------------
//	Render a face.
//	It would be nice to not have to pass in nSegment and nSide, but
//	they are used for our hideously hacked in headlight system.
//	vp is a pointer to vertex ids.
//	tmap1, tmap2 are texture map ids.  tmap2 is the pasty one.

int RenderColoredSegFace (int nSegment, int nSide, int nVertices, g3sPoint **pointList)
{
	short nConnSeg = gameData.segs.segments [nSegment].children [nSide];
	int	owner = gameData.segs.xSegments [nSegment].owner;
	int	special = gameData.segs.segment2s [nSegment].special;

if ((gameData.app.nGameMode & GM_ENTROPY) && (extraGameInfo [1].entropy.nOverrideTextures == 2) && (owner > 0)) {
	if ((nConnSeg >= 0) && (gameData.segs.xSegments [nConnSeg].owner == owner))
			return 0;
	if (owner == 1)
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 1, 0, nSegment);
	else
		G3DrawPolyAlpha (nVertices, pointList, segmentColors, 0, nSegment);
	return 1;
	}
if (special == SEGMENT_IS_WATER) {
	if ((nConnSeg < 0) || (gameData.segs.segment2s [nConnSeg].special != SEGMENT_IS_WATER))
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 2, 0, nSegment);
	return 1;
	}
if (special == SEGMENT_IS_LAVA) {
	if ((nConnSeg < 0) || (gameData.segs.segment2s [nConnSeg].special != SEGMENT_IS_LAVA))
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 3, 0, nSegment);
	return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

int RenderWall (tFaceProps *propsP, g3sPoint **pointList, int bIsMonitor)
{
	short c, nWallNum = WallNumI (propsP->segNum, propsP->sideNum);
	static tRgbaColorf cloakColor = {0, 0, 0, -1};

if (IS_WALL (nWallNum)) {
	if (propsP->widFlags & (WID_CLOAKED_FLAG | WID_TRANSPARENT_FLAG)) {
		if (!bIsMonitor) {
			if (!RenderColoredSegFace (propsP->segNum, propsP->sideNum, propsP->nVertices, pointList)) {
				c = gameData.walls.walls [nWallNum].cloakValue;
				if (propsP->widFlags & WID_CLOAKED_FLAG) {
					if (c < GR_ACTUAL_FADE_LEVELS) {
						gameStates.render.grAlpha = (float) c;
						G3DrawPolyAlpha (propsP->nVertices, pointList, &cloakColor, 1, propsP->segNum);		//draw as flat poly
						}
					}
				else {
					if (!gameOpts->render.color.bWalls)
						c = 0;
					if (gameData.walls.walls [nWallNum].hps)
						gameStates.render.grAlpha = (float) fabs ((1.0f - (float) gameData.walls.walls [nWallNum].hps / ((float) F1_0 * 100.0f)) * GR_ACTUAL_FADE_LEVELS);
					else if (IsMultiGame && gameStates.app.bHaveExtraGameInfo [1])
						gameStates.render.grAlpha = COMPETITION ? GR_ACTUAL_FADE_LEVELS * 3.0f / 2.0f : (float) (GR_ACTUAL_FADE_LEVELS - extraGameInfo [1].grWallTransparency);
					else
						gameStates.render.grAlpha = (float) (GR_ACTUAL_FADE_LEVELS - extraGameInfo [0].grWallTransparency);
					if (gameStates.render.grAlpha < GR_ACTUAL_FADE_LEVELS) {
						tRgbaColorf wallColor = {CPAL2Tr (gamePalette, c), CPAL2Tg (gamePalette, c), CPAL2Tb (gamePalette, c), -1};
						G3DrawPolyAlpha (propsP->nVertices, pointList, &wallColor, 1, propsP->segNum);	//draw as flat poly
						}
					}
				}
			gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
			return 1;
			}
		}
	else if (gameStates.app.bD2XLevel) {
		c = gameData.walls.walls [nWallNum].cloakValue;
		if (c && (c < GR_ACTUAL_FADE_LEVELS))
			gameStates.render.grAlpha = (float) (GR_ACTUAL_FADE_LEVELS - c);
		}
	else if (gameOpts->render.effects.bAutoTransparency && IsTransparentFace (propsP))
		gameStates.render.grAlpha = (float) GR_ACTUAL_FADE_LEVELS * 0.8f;
	else
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
return 0;
}

//------------------------------------------------------------------------------

void RenderFace (tFaceProps *propsP)
{
	tFaceProps	props = *propsP;
	grsBitmap  *bmBot = NULL;
	grsBitmap  *bmTop = NULL;

	int			i, bIsMonitor, bIsTeleCam, bHaveMonitorBg, nCamNum, bCamBufAvail;
	g3sPoint		*pointList [8], **pp;
	tSegment		*segP = gameData.segs.segments + props.segNum;
	tSide			*sideP = segP->sides + props.sideNum;
	tCamera		*pc = NULL;

if (props.nBaseTex < 0)
	return;
if (gameStates.render.nShadowPass == 2) {
#if DBG_SHADOWS
	if (!bWallShadows)
		return;
#endif
	G3SetCullAndStencil (0, 0);
	RenderFaceShadow (propsP);
	G3SetCullAndStencil (1, 0);
	RenderFaceShadow (propsP);
	return;
	}
#ifdef _DEBUG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
	props.segNum = props.segNum;
if (props.nBaseTex == nDbgBaseTex)
	props.segNum = props.segNum;
if (props.nOvlTex == nDbgOvlTex)
	props.segNum = props.segNum;
#	if 0
else
	return;
#	endif
#endif

gameData.render.vertexList = gameData.segs.fVertices;
Assert(props.nVertices <= 4);
for (i = 0, pp = pointList; i < props.nVertices; i++, pp++)
	*pp = gameData.segs.points + props.vp [i];
if (!(gameOpts->render.debug.bTextures || IsMultiGame))
	goto drawWireFrame;
#if 1
if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (props.nVertices, pointList);
	gameData.render.vertexList = NULL;
	return;
	}
#endif
SetVertexColors (&props);
if (gameStates.render.nType == 2) {
#ifdef _DEBUG //convenient place for a debug breakpoint
	if (props.segNum == nDbgSeg && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
		props.segNum = props.segNum;
#endif
	RenderColoredSegFace (props.segNum, props.sideNum, props.nVertices, pointList);
	gameData.render.vertexList = NULL;
	return;
	}
nCamNum = IsMonitorFace (props.segNum, props.sideNum, 0);
if ((bIsMonitor = gameStates.render.bUseCameras && (nCamNum >= 0))) {
	pc = gameData.cameras.cameras + nCamNum;
	pc->bVisible = 1;
	bIsTeleCam = pc->bTeleport;
#if RENDER2TEXTURE
	bCamBufAvail = OglCamBufAvail (pc, 1) == 1;
#else
	bCamBufAvail = 0;
#endif
	bHaveMonitorBg = pc->bValid && /*!pc->bShadowMap &&*/
						  (pc->texBuf.glTexture || bCamBufAvail) &&
						  (!bIsTeleCam || EGI_FLAG (bTeleporterCams, 0, 1, 0));
	}
else
	bIsTeleCam =
	bHaveMonitorBg =
	bCamBufAvail = 0;
if (RenderWall (&props, pointList, bIsMonitor)) {	//handle semi-transparent walls
	gameData.render.vertexList = NULL;
	return;
	}
if (props.widFlags & WID_RENDER_FLAG) {
	if (props.nBaseTex >= gameData.pig.tex.nTextures [gameStates.app.bD1Data]) {
	sideP->nBaseTex = 0;
	}
if (!(bHaveMonitorBg && gameOpts->render.cameras.bFitToWall)) {
	if (gameStates.render.nType == 3) {
		bmBot = bmpCorona;
		bmTop = NULL;
		props.uvls [0].u =
		props.uvls [0].v =
		props.uvls [1].v =
		props.uvls [3].u = F1_0 / 4;
		props.uvls [1].u =
		props.uvls [2].u =
		props.uvls [2].v =
		props.uvls [3].v = 3 * F1_0 / 4;
		}
	else if (gameOpts->ogl.bGlTexMerge && gameStates.render.textures.bGlsTexMergeOk) {
		bmBot = LoadFaceBitmap (props.nBaseTex, sideP->nFrame);
		if (props.nOvlTex)
			bmTop = LoadFaceBitmap ((short) (props.nOvlTex), sideP->nFrame);
		}
	else {
		if (props.nOvlTex != 0) {
			bmBot = TexMergeGetCachedBitmap (props.nBaseTex, props.nOvlTex, props.nOvlOrient);
#ifdef _DEBUG
			if (!bmBot)
				bmBot = TexMergeGetCachedBitmap (props.nBaseTex, props.nOvlTex, props.nOvlOrient);
#endif
			}
		else {
			bmBot = gameData.pig.tex.pBitmaps + gameData.pig.tex.pBmIndex [props.nBaseTex].index;
			PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [props.nBaseTex].index, gameStates.app.bD1Mission);
			}
		}
	}

if (bHaveMonitorBg) {
	GetCameraUVL (pc, NULL, props.uvls, NULL, NULL);
	pc->texBuf.glTexture->wrapstate = -1;
	if (bIsTeleCam) {
#ifdef _DEBUG
		bmBot = &pc->texBuf;
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
#else
		bmTop = &pc->texBuf;
		gameStates.render.grAlpha = (GR_ACTUAL_FADE_LEVELS * 7) / 10;
#endif
		}
	else if (gameOpts->render.cameras.bFitToWall || (props.nOvlTex > 0))
		bmBot = &pc->texBuf;
	else
		bmTop = &pc->texBuf;
	}
SetFaceLight (&props);
#ifdef EDITOR
if (Render_only_bottom && (nSide == WBOTTOM))
	G3DrawTexPoly (props.nVertices, pointList, props.uvls, gameData.pig.tex.bitmaps + gameData.pig.tex.bmIndex [Bottom_bitmap_num].index, 1, propsP->segNum);
else
#endif
#ifdef _DEBUG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && props.sideNum == nDbgSide)
	props.segNum = props.segNum;
#endif
if ((gameOpts->render.bDepthSort > 0) && (gameStates.render.grAlpha < GR_ACTUAL_FADE_LEVELS))
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS - gameStates.render.grAlpha;
#ifdef _DEBUG
if (bmTop)
	fpDrawTexPolyMulti (
		props.nVertices, pointList, props.uvls,
#	if LIGHTMAPS
		props.uvl_lMaps,
#	endif
		bmBot, bmTop,
#	if LIGHTMAPS
		NULL, //lightmaps + props.segNum * 6 + props.sideNum,
#	endif
		&props.vNormal, props.nOvlOrient, !bIsMonitor || bIsTeleCam, props.segNum);
else
#	if LIGHTMAPS == 0
	G3DrawTexPoly (props.nVertices, pointList, props.uvls, bmBot, &props.vNormal, !bIsMonitor || bIsTeleCam, props.segNum);
#	else
	fpDrawTexPolyMulti (
		props.nVertices, pointList, props.uvls, props.uvl_lMaps, bmBot, NULL,
		NULL, //lightmaps + props.segNum * 6 + props.sideNum,
		&props.vNormal, 0, !bIsMonitor || bIsTeleCam, props.segNum);
#	endif
#else
fpDrawTexPolyMulti (
	props.nVertices, pointList, props.uvls,
#	if LIGHTMAPS
	props.uvl_lMaps,
#	endif
	bmBot, bmTop,
#	if LIGHTMAPS
	NULL, //lightmaps + props.segNum * 6 + props.sideNum,
#	endif
	&props.vNormal, props.nOvlOrient, !bIsMonitor || bIsTeleCam,
	props.segNum);
#endif
	}
gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
gameStates.ogl.fAlpha = 1;
	// render the tSegment the tPlayer is in with a transparent color if it is a water or lava tSegment
	//if (nSegment == OBJECTS->nSegment)
#ifdef _DEBUG
if (bOutLineMode)
	DrawOutline (props.nVertices, pointList);
#endif

drawWireFrame:

if (gameOpts->render.debug.bWireFrame && !IsMultiGame)
	DrawOutline (props.nVertices, pointList);
}

fix	Tulate_min_dot = (F1_0/4);
//--unused-- fix	Tulate_min_ratio = (2*F1_0);
fix	Min_n0_n1_dot	= (F1_0*15/16);

// -----------------------------------------------------------------------------------
//	Render a side.
//	Check for Normal facing.  If so, render faces on tSide dictated by sideP->nType.

#undef LMAP_LIGHTADJUST
#define LMAP_LIGHTADJUST 0

void RenderSide (tSegment *segP, short nSide)
{
	tSide			*sideP = segP->sides + nSide;
	tFaceProps	props;

#if LIGHTMAPS
#define	LMAP_SIZE	(1.0 / 16.0)

	static tUVL	uvl_lMaps [4] = {
		{F2X (LMAP_SIZE), F2X (LMAP_SIZE), 0}, 
		{F2X (1.0 - LMAP_SIZE), F2X (LMAP_SIZE), 0}, 
		{F2X (1.0 - LMAP_SIZE), F2X (1.0 - LMAP_SIZE), 0}, 
		{F2X (LMAP_SIZE), F2X (1.0 - LMAP_SIZE), 0}
	};
#endif

props.segNum = SEG_IDX (segP);
props.sideNum = nSide;
#ifdef _DEBUG
if ((props.segNum == nDbgSeg) && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
	segP = segP;
#endif
props.widFlags = WALL_IS_DOORWAY (segP, props.sideNum, NULL);
if (!(gameOpts->render.debug.bWalls || IsMultiGame) && IS_WALL (WallNumP (segP, props.sideNum)))
	return;
switch (gameStates.render.nType) {
	case -1:
		if (!(props.widFlags & WID_RENDER_FLAG) && (gameData.segs.segment2s [props.segNum].special < SEGMENT_IS_WATER))		//if (WALL_IS_DOORWAY(segP, props.sideNum) == WID_NO_WALL)
			return;
		break;
	case 0:
		if (segP->children [props.sideNum] >= 0) //&& IS_WALL (WallNumP (segP, props.sideNum)))
			return;
		break;
	case 1:
		if (!IS_WALL (WallNumP (segP, props.sideNum)))
			return;
		break;
	case 2:
		if ((gameData.segs.segment2s [props.segNum].special < SEGMENT_IS_WATER) &&
			 (gameData.segs.xSegments [props.segNum].owner < 1))
			return;
		break;
	case 3:
		if ((IsLight (sideP->nBaseTex) || (sideP->nOvlTex && IsLight (sideP->nOvlTex))))
			RenderCorona (props.segNum, props.sideNum, 1, 20);
		return;
	}
#if LIGHTMAPS
if (gameStates.render.bDoLightmaps) {
		float	Xs = 8;
		float	h = 0.5f / (float) Xs;

	props.uvl_lMaps [0].u =
	props.uvl_lMaps [0].v =
	props.uvl_lMaps [1].v =
	props.uvl_lMaps [3].u = F2X (h);
	props.uvl_lMaps [1].u =
	props.uvl_lMaps [2].u =
	props.uvl_lMaps [2].v =
	props.uvl_lMaps [3].v = F2X (1-h);
	}
#endif
props.nBaseTex = sideP->nBaseTex;
props.nOvlTex = sideP->nOvlTex;
props.nOvlOrient = sideP->nOvlOrient;

	//	========== Mark: Here is the change...beginning here: ==========

#if LIGHTMAPS
	if (gameStates.render.bDoLightmaps) {
		memcpy (props.uvl_lMaps, uvl_lMaps, sizeof (tUVL) * 4);
#if LMAP_LIGHTADJUST
		props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = props.uvls [3].l = F1_0 / 2;
#	endif
		}
#endif

#ifdef _DEBUG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && props.sideNum == nDbgSide)
	props.segNum = props.segNum;
if (props.nBaseTex == nDbgBaseTex)
	props.segNum = props.segNum;
if (props.nOvlTex == nDbgOvlTex)
	props.segNum = props.segNum;
#	if 0
else
	return;
#	endif
#endif

if (!FaceIsVisible (props.segNum, props.sideNum))
	return;
if (sideP->nType == SIDE_IS_QUAD) {
	props.vNormal = sideP->normals [0];
	props.nVertices = 4;
	memcpy (props.uvls, sideP->uvls, sizeof (tUVL) * 4);
	GetSideVertIndex (props.vp, props.segNum, props.sideNum);
	RenderFace (&props);
#ifdef EDITOR
	CheckFace (props.segNum, props.sideNum, 0, 3, props.vp, sideP->nBaseTex, sideP->nOvlTex, sideP->uvls);
#endif
	}
else {
	// new code
	// non-planar faces are still passed as quads to the renderer as it will render triangles (GL_TRIANGLE_FAN) anyway
	// just need to make sure the vertices come in the proper order depending of the the orientation of the two non-planar triangles
	props.vNormal = sideP->normals[0] + sideP->normals[1];
	props.vNormal *= (F1_0 / 2);
	props.nVertices = 4;
	if (sideP->nType == SIDE_IS_TRI_02) {
		memcpy (props.uvls, sideP->uvls, sizeof (tUVL) * 4);
		GetSideVertIndex (props.vp, props.segNum, props.sideNum);
		RenderFace (&props);
		}
	else if (sideP->nType == SIDE_IS_TRI_13) {	//just rendering the fan with vertex 1 instead of 0
		memcpy (props.uvls + 1, sideP->uvls, sizeof (tUVL) * 3);
		props.uvls [0] = sideP->uvls [3];
		GetSideVertIndex (props.vp + 1, props.segNum, props.sideNum);
		props.vp [0] = props.vp [4];
		RenderFace (&props);
		}
	else {
		Error("Illegal tSide nType in RenderSide, nType = %i, tSegment # = %i, tSide # = %i\n", sideP->nType, SEG_IDX (segP), props.sideNum);
		return;
		}
	}
}

// -----------------------------------------------------------------------------------

static int RenderSegmentFaces (short nSegment, int nWindow)
{
	tSegment		*segP = gameData.segs.segments + nSegment;
	g3sCodes 	cc;
	short			nSide;

OglSetupTransform (0);
cc = RotateVertexList (8, segP->verts);
gameData.render.pVerts = gameData.segs.fVertices;
//	return;
if (cc.ccAnd /*&& !gameStates.render.automap.bDisplay*/)	//all off screen and not rendering the automap
	return 0;
gameStates.render.nState = 0;
#ifdef _DEBUG //convenient place for a debug breakpoint
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
SetNearestSegmentLights (nSegment, -1, 0, 0, 0);
for (nSide = 0; nSide < 6; nSide++) //segP->nFaces, faceP = segP->pFaces; nSide; nSide--, faceP++)
	RenderSide (segP, nSide);
OglResetTransform (0);
OGL_BINDTEX (0);
return 1;
}

//------------------------------------------------------------------------------

extern ubyte nDemoDoingRight, nDemoDoingLeft;

void DoRenderObject (int nObject, int nWindow)
{
#ifdef EDITOR
	int save_3d_outline=0;
#endif
	tObject *objP = OBJECTS + nObject, *hObj;
	tWindowRenderedData *wrd = windowRenderedData + nWindow;
	int nType, count = 0;
	int n;

if (!(IsMultiGame || gameOpts->render.debug.bObjects))
	return;
Assert(nObject < MAX_OBJECTS);
#if 0
if (!(nWindow || gameStates.render.cameras.bActive) && (gameStates.render.nShadowPass < 2) &&
    (gameData.render.mine.bObjectRendered [nObject] == gameStates.render.nFrameFlipFlop))	//already rendered this...
	return;
#endif
if (gameData.demo.nState == ND_STATE_PLAYBACK) {
	if ((nDemoDoingLeft == 6 || nDemoDoingRight == 6) && objP->nType == OBJ_PLAYER) {
		// A nice fat hack: keeps the tPlayer ship from showing up in the
		// small extra view when guiding a missile in the big tPortal
  		return;
		}
	}
//	Added by MK on 09/07/94 (at about 5:28 pm, CDT, on a beautiful, sunny late summer day!) so
//	that the guided missile system will know what OBJECTS to look at.
//	I didn't know we had guided missiles before the release of D1. --MK
nType = objP->nType;
if ((nType == OBJ_ROBOT) || (nType == OBJ_PLAYER) ||
	 ((nType == OBJ_WEAPON) && (WeaponIsPlayerMine (objP->id) || (gameData.objs.bIsMissile [objP->id] && EGI_FLAG (bKillMissiles, 0, 0, 0))))) {
	//Assert(windowRenderedData [nWindow].renderedObjects < MAX_RENDERED_OBJECTS);
	//	This peculiar piece of code makes us keep track of the most recently rendered objects, which
	//	are probably the higher priority objects, without overflowing the buffer
	if (wrd->numObjects >= MAX_RENDERED_OBJECTS) {
		Int3();
		wrd->numObjects /= 2;
		}
	wrd->renderedObjects [wrd->numObjects++] = nObject;
	}
if ((count++ > MAX_OBJECTS) || (objP->next == nObject)) {
	Int3();					// infinite loop detected
	objP->next = -1;		// won't this clean things up?
	return;					// get out of this infinite loop!
	}
	//g3_drawObject(objP->class_id, &objP->position.vPos, &objP->position.mOrient, objP->size);
	//check for editor tObject
#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR && nObject == CurObject_index) {
	save_3d_outline = g3d_interp_outline;
	g3d_interp_outline=1;
	}
if (bSearchMode)
	RenderObjectSearch (objP);
	else
#endif
	//NOTE LINK TO ABOVE
if (RenderObject (objP, nWindow, 0))
	gameData.render.mine.bObjectRendered [nObject] = gameStates.render.nFrameFlipFlop;
for (n = objP->attachedObj; n != -1; n = hObj->cType.explInfo.nNextAttach) {
	hObj = OBJECTS + n;
	Assert (hObj->nType == OBJ_FIREBALL);
	Assert (hObj->controlType == CT_EXPLOSION);
	Assert (hObj->flags & OF_ATTACHED);
	RenderObject (hObj, nWindow, 1);
	}
#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR && nObject == CurObject_index)
	g3d_interp_outline = save_3d_outline;
#endif
}

// -----------------------------------------------------------------------------------
//increment counter for checking if points bTransformed
//This must be called at the start of the frame if RotateVertexList() will be used
void RenderStartFrame (void)
{
if (!++gameStates.render.nFrameCount) {		//wrap!
	memset(gameData.render.mine.nRotatedLast, 0, sizeof (gameData.render.mine.nRotatedLast));		//clear all to zero
	gameStates.render.nFrameCount = 1;											//and set this frame to 1
	}
}

// -----------------------------------------------------------------------------------

void SortSidesByDist (double *sideDists, char *sideNums, int left, int right)
{
	int		l = left;
	int		r = right;
	int		h, m = (l + r) / 2;
	double	hd, md = sideDists [m];

do {
	while (sideDists [l] < md)
		l++;
	while (sideDists [r] > md)
		r--;
	if (l <= r) {
		if (l < r) {
			h = sideNums [l];
			sideNums [l] = sideNums [r];
			sideNums [r] = h;
			hd = sideDists [l];
			sideDists [l] = sideDists [r];
			sideDists [r] = hd;
			}
		++l;
		--r;
		}
	} while (l <= r);
if (right > l)
   SortSidesByDist (sideDists, sideNums, l, right);
if (r > left)
   SortSidesByDist (sideDists, sideNums, left, r);
}

//------------------------------------------------------------------------------

ubyte CodePortalPoint (fix x, fix y, tPortal *p)
{
	ubyte code = 0;

if (x <= p->left)
	code |= 1;
else if (x >= p->right)
	code |= 2;
if (y <= p->top)
	code |= 4;
else if (y >= p->bot)
	code |= 8;
return code;
}

//------------------------------------------------------------------------------

ubyte CodePortal (tPortal *s, tPortal *p)
{
	ubyte code = 0;

if (s->right <= p->left)
	code |= 1;
else if (s->left >= p->right)
	code |= 2;
if (s->bot <= p->top)
	code |= 4;
else if (s->top >= p->bot)
	code |= 8;
return code;
}

//------------------------------------------------------------------------------

#ifdef _DEBUG
void DrawWindowBox (unsigned int color, short left, short top, short right, short bot)
{
	short l, t, r, b;

GrSetColorRGBi (color);
l=left;
t=top;
r=right;
b=bot;

if ( r<0 || b<0 || l>=grdCurCanv->cvBitmap.bmProps.w || (t>=grdCurCanv->cvBitmap.bmProps.h && b>=grdCurCanv->cvBitmap.bmProps.h))
	return;

if (l<0)
	l=0;
if (t<0)
	t=0;
if (r>=grdCurCanv->cvBitmap.bmProps.w) r=grdCurCanv->cvBitmap.bmProps.w-1;
if (b>=grdCurCanv->cvBitmap.bmProps.h) b=grdCurCanv->cvBitmap.bmProps.h-1;

GrLine(I2X(l), I2X(t), I2X(r), I2X(t));
GrLine(I2X(r), I2X(t), I2X(r), I2X(b));
GrLine(I2X(r), I2X(b), I2X(l), I2X(b));
GrLine(I2X(l), I2X(b), I2X(l), I2X(t));
}
#endif

//------------------------------------------------------------------------------

tPortal renderPortals [MAX_SEGMENTS_D2X];
#if !OLD_SEGLIST
tPortal sidePortals [MAX_SEGMENTS_D2X * 6];
#endif
ubyte bVisible [MAX_SEGMENTS_D2X];

//Given two sides of tSegment, tell the two verts which form the
//edge between them
short edgeBetweenTwoSides [6][6][2] = {
	{ {-1, -1}, {3, 7}, {-1, -1}, {2, 6}, {6, 7}, {2, 3} },
	{ {3, 7}, {-1, -1}, {0, 4}, {-1, -1}, {4, 7}, {0, 3} },
	{ {-1, -1}, {0, 4}, {-1, -1}, {1, 5}, {4, 5}, {0, 1} },
	{ {2, 6}, {-1, -1}, {1, 5}, {-1, -1}, {5, 6}, {1, 2} },
	{ {6, 7}, {4, 7}, {4, 5}, {5, 6}, {-1, -1}, {-1, -1} },
	{ {2, 3}, {0, 3}, {0, 1}, {1, 2}, {-1, -1}, {-1, -1} }
};

//given an edge specified by two verts, give the two sides on that edge
int edgeToSides [8][8][2] = {
	{ {-1, -1}, {2, 5}, {-1, -1}, {1, 5}, {1, 2}, {-1, -1}, {-1, -1}, {-1, -1} },
	{ {2, 5}, {-1, -1}, {3, 5}, {-1, -1}, {-1, -1}, {2, 3}, {-1, -1}, {-1, -1} },
	{ {-1, -1}, {3, 5}, {-1, -1}, {0, 5}, {-1, -1}, {-1, -1}, {0, 3}, {-1, -1} },
	{ {1, 5}, {-1, -1}, {0, 5}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {0, 1} },
	{ {1, 2}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {2, 4}, {-1, -1}, {1, 4} },
	{ {-1, -1}, {2, 3}, {-1, -1}, {-1, -1}, {2, 4}, {-1, -1}, {3, 4}, {-1, -1} },
	{ {-1, -1}, {-1, -1}, {0, 3}, {-1, -1}, {-1, -1}, {3, 4}, {-1, -1}, {0, 4} },
	{ {-1, -1}, {-1, -1}, {-1, -1}, {0, 1}, {1, 4}, {-1, -1}, {0, 4}, {-1, -1} },
};

//@@//perform simple check on tables
//@@check_check()
//@@{
//@@	int i, j;
//@@
//@@	for (i=0;i<8;i++)
//@@		for (j=0;j<8;j++)
//@@			Assert(edgeToSides [i][j][0] == edgeToSides [j][i][0] &&
//@@					edgeToSides [i][j][1] == edgeToSides [j][i][1]);
//@@
//@@	for (i=0;i<6;i++)
//@@		for (j=0;j<6;j++)
//@@			Assert(edgeBetweenTwoSides [i][j][0] == edgeBetweenTwoSides [j][i][0] &&
//@@					edgeBetweenTwoSides [i][j][1] == edgeBetweenTwoSides [j][i][1]);
//@@
//@@
//@@}


//------------------------------------------------------------------------------
//given an edge and one tSide adjacent to that edge, return the other adjacent tSide

int FindOtherSideOnEdge (tSegment *seg, short *verts, int oppSide)
{
	int	i;
	int	i0 = -1, i1 = -1;
	int	side0, side1;
	int	*eptr;
	int	sv, v0, v1;
	short	*vp;

//@@	check_check();

v0 = verts [0];
v1 = verts [1];
vp = seg->verts;
for (i = 0; i < 8; i++) {
	sv = *vp++;	// seg->verts [i];
	if (sv == v0) {
		i0 = i;
		if (i1 != -1)
			break;
		}
	if (sv == v1) {
		i1 = i;
		if (i0 != -1)
			break;
		}
	}
eptr = edgeToSides [i0][i1];
side0 = eptr [0];
side1 = eptr [1];
return (side0 == oppSide) ? side1 : side0;
}

//------------------------------------------------------------------------------
//find the two segments that join a given seg through two sides, and
//the sides of those segments the abut.

typedef struct tSideNormData {
	vmsVector	n [2];
	vmsVector	*p;
	short			t;
} tSideNormData;

int FindAdjacentSideNorms (tSegment *segP, short s0, short s1, tSideNormData *s)
{
	tSegment	*seg0, *seg1;
	tSide		*side0, *side1;
	short		edgeVerts [2];
	int		oppSide0, oppSide1;
	int		otherSide0, otherSide1;

Assert(s0 != -1 && s1 != -1);
seg0 = gameData.segs.segments + segP->children [s0];
seg1 = gameData.segs.segments + segP->children [s1];
edgeVerts [0] = segP->verts [edgeBetweenTwoSides [s0][s1][0]];
edgeVerts [1] = segP->verts [edgeBetweenTwoSides [s0][s1][1]];
Assert(edgeVerts [0] != -1 && edgeVerts [1] != -1);
oppSide0 = FindConnectedSide (segP, seg0);
Assert (oppSide0 != -1);
oppSide1 = FindConnectedSide (segP, seg1);
Assert (oppSide1 != -1);
otherSide0 = FindOtherSideOnEdge (seg0, edgeVerts, oppSide0);
otherSide1 = FindOtherSideOnEdge (seg1, edgeVerts, oppSide1);
side0 = seg0->sides + otherSide0;
side1 = seg1->sides + otherSide1;
memcpy (s [0].n, side0->normals, 2 * sizeof (vmsVector));
memcpy (s [1].n, side1->normals, 2 * sizeof (vmsVector));
s[0].p = gameData.segs.vertices + seg0->verts [sideToVerts [otherSide0][(s [0].t = side0->nType) == 3]];
s[1].p = gameData.segs.vertices + seg1->verts [sideToVerts [otherSide1][(s [1].t = side1->nType) == 3]];
return 1;
}

//------------------------------------------------------------------------------
//see if the order matters for these two children.
//returns 0 if order doesn't matter, 1 if c0 before c1, -1 if c1 before c0
#if SORT_RENDER_SEGS

static int CompareChildren (tSegment *segP, short c0, short c1)
{
	tSideNormData	s [2];
	vmsVector		temp;
	fix				d0, d1;

if (sideOpposite [c0] == c1)
	return 0;
//find normals of adjoining sides
FindAdjacentSideNorms (segP, c0, c1, s);
temp = gameData.render.mine.viewerEye - *s[0].p;
d0 = vmsVector::Dot(s[0].n[0], temp);
if (s [0].t != 1)	// triangularized face -> 2 different normals
	d0 |= vmsVector::Dot(s[0].n[1], temp);	// we only need the sign, so a bitwise or does the trick
temp = gameData.render.mine.viewerEye - *s[1].p;
d1 = vmsVector::Dot(s[1].n[0], temp);
if (s [1].t != 1)
	d1 |= vmsVector::Dot(s[1].n[1], temp);
if ((d0 & d1) < 0)	// only < 0 if both negative due to bitwise and
	return 0;
if (d0 < 0)
	return 1;
 if (d1 < 0)
	return -1;
return 0;
}

//------------------------------------------------------------------------------

int QuickSortSegChildren (tSegment *segP, short left, short right, short *childList)
{
	short	h,
			l = left,
			r = right,
			m = (l + r) / 2,
			median = childList [m],
			bSwap = 0;

do {
	while ((l < m) && CompareChildren (segP, childList [l], median) >= 0)
		l++;
	while ((r > m) && CompareChildren (segP, childList [r], median) <= 0)
		r--;
	if (l <= r) {
		if (l < r) {
			h = childList [l];
			childList [l] = childList [r];
			childList [r] = h;
			bSwap = 1;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	bSwap |= QuickSortSegChildren (segP, l, right, childList);
if (left < r)
	bSwap |= QuickSortSegChildren (segP, left, r, childList);
return bSwap;
}

//------------------------------------------------------------------------------

//short the children of tSegment to render in the correct order
//returns non-zero if swaps were made
static inline int SortSegChildren (tSegment *segP, int nChildren, short *childList)
{
#if 1

if (nChildren < 2)
	return 0;
if (nChildren == 2) {
	if (CompareChildren (segP, childList [0], childList [1]) >= 0)
		return 0;
	short h = childList [0];
	childList [0] = childList [1];
	childList [1] = h;
	return 1;
	}
return QuickSortSegChildren (segP, (short) 0, (short) (nChildren - 1), childList);

#else
	int i, j;
	int r;
	int made_swaps, count;

if (nChildren < 2)
	return 0;
	//for each child,  compare with other children and see if order matters
	//if order matters, fix if wrong

count = 0;

do {
	made_swaps = 0;

	for (i=0;i<nChildren-1;i++)
		for (j=i+1;childList [i]!=-1 && j<nChildren;j++)
			if (childList [j]!=-1) {
				r = CompareChildren(segP, childList [i], childList [j]);

				if (r == 1) {
					int temp = childList [i];
					childList [i] = childList [j];
					childList [j] = temp;
					made_swaps=1;
				}
			}

} while (made_swaps && ++count<nChildren);
return count;
#endif
}

#endif //SORT_RENDER_SEGS

//------------------------------------------------------------------------------

extern kcItem kcMouse [];

inline int ZoomKeyPressed (void)
{
#if 1
	int	v;

return gameStates.input.keys.pressed [kcKeyboard [52].value] || gameStates.input.keys.pressed [kcKeyboard [53].value] ||
		 (((v = kcMouse [30].value) < 255) && MouseButtonState (v));
#else
return (Controls [0].zoomDownCount > 0);
#endif
}

//------------------------------------------------------------------------------

void SetRenderView (fix nEyeOffset, short *pnStartSeg, int bOglScale)
{
	static int bStopZoom;
	short nStartSeg;

gameData.render.mine.viewerEye = gameData.objs.viewer->position.vPos;
if (nEyeOffset) {
	gameData.render.mine.viewerEye += gameData.objs.viewer->position.mOrient[RVEC] * nEyeOffset;
	}
#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR)
	gameData.render.mine.viewerEye = gameData.objs.viewer->position.vPos;
#endif

externalView.pPos = NULL;
if (gameStates.render.cameras.bActive) {
	nStartSeg = gameData.objs.viewer->nSegment;
	G3SetViewMatrix(gameData.render.mine.viewerEye, gameData.objs.viewer->position.mOrient, gameStates.render.xZoom, bOglScale);
	}
else {
	if (!pnStartSeg)
		nStartSeg = gameStates.render.nStartSeg;
	else {
		nStartSeg = FindSegByPos (gameData.render.mine.viewerEye, gameData.objs.viewer->nSegment, 1, 0);
		if (!gameStates.render.nWindow && (gameData.objs.viewer == gameData.objs.console)) {
			SetPathPoint (&externalView, gameData.objs.viewer);
			if (nStartSeg == -1)
				nStartSeg = gameData.objs.viewer->nSegment;
			}
		}
	if ((gameData.objs.viewer == gameData.objs.console) && viewInfo.bUsePlayerHeadAngles) {
		vmsMatrix mHead, mView;
		mHead = vmsMatrix::Create(viewInfo.playerHeadAngles);
		// TODO MM
		mView = gameData.objs.viewer->position.mOrient * mHead;
		G3SetViewMatrix(gameData.render.mine.viewerEye, mView, gameStates.render.xZoom, bOglScale);
		}
	else if (gameStates.render.bRearView && (gameData.objs.viewer == gameData.objs.console)) {
#if 1
		vmsMatrix mView;

		mView = gameData.objs.viewer->position.mOrient;
		mView[FVEC].Neg();
		mView[RVEC].Neg();
#else
		vmsMatrix mHead, mView;

		viewInfo.playerHeadAngles[PA] = 0;
		viewInfo.playerHeadAngles[BA] = 0x7fff;
		viewInfo.playerHeadAngles[HA] = 0x7fff;
		VmAngles2Matrix (&mHead, &viewInfo.playerHeadAngles);
		VmMatMul (&mView, &gameData.objs.viewer->position.mOrient, &mHead);
#endif
		G3SetViewMatrix(gameData.render.mine.viewerEye, mView,  //gameStates.render.xZoom, bOglScale);
							  FixDiv (gameStates.render.xZoom, gameStates.render.nZoomFactor), bOglScale);
		}
	else if ((gameData.objs.viewer == gameData.objs.console) && (!IsMultiGame || gameStates.app.bHaveExtraGameInfo [1])) {
		gameStates.render.nMinZoomFactor = (fix) (F1_0 * gameStates.render.glAspect); //(((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 * F1_0  / 3 : F1_0) * glAspect);
		gameStates.render.nMaxZoomFactor = gameStates.render.nMinZoomFactor * 5;
		if ((gameData.weapons.nPrimary != VULCAN_INDEX) && (gameData.weapons.nPrimary != GAUSS_INDEX))
			gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor; //(fix) (((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 * F1_0  / 3 : F1_0) * glAspect);
		else {
			switch (extraGameInfo [IsMultiGame].nZoomMode) {
				case 0:
					break;
				case 1:
					if (ZoomKeyPressed ()) {
						if (!bStopZoom) {
							gameStates.render.nZoomFactor = (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? (gameStates.render.nZoomFactor * 7) / 5 : (gameStates.render.nZoomFactor * 5) / 3;
							if (gameStates.render.nZoomFactor > gameStates.render.nMaxZoomFactor)
								gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor;
							bStopZoom = 1;
							}
						}
					else {
						bStopZoom = 0;
						}
					break;
				case 2:
					if (ZoomKeyPressed ()) {
						gameStates.render.nZoomFactor += gameData.time.xFrame * 4;
						if (gameStates.render.nZoomFactor > gameStates.render.nMaxZoomFactor)
							gameStates.render.nZoomFactor = gameStates.render.nMaxZoomFactor;
						}
					else {
						gameStates.render.nZoomFactor -= gameData.time.xFrame * 4;
						if (gameStates.render.nZoomFactor < gameStates.render.nMinZoomFactor)
							gameStates.render.nZoomFactor = gameStates.render.nMinZoomFactor;
						}
					break;
				}
			}
		if ((gameData.objs.viewer == gameData.objs.console) &&
#ifdef _DEBUG
			 gameStates.render.bExternalView) {
#else
			 gameStates.render.bExternalView && (!IsMultiGame || IsCoopGame || EGI_FLAG (bEnableCheats, 0, 0, 0))) {
#endif
			GetViewPoint ();
			G3SetViewMatrix(gameData.render.mine.viewerEye,
								  externalView.pPos ? externalView.pPos->mOrient : gameData.objs.viewer->position.mOrient,
								  gameStates.render.xZoom, bOglScale);
			}
		else
			G3SetViewMatrix(gameData.render.mine.viewerEye, gameData.objs.viewer->position.mOrient,
								  FixDiv (gameStates.render.xZoom, gameStates.render.nZoomFactor), bOglScale);
		}
	else
		G3SetViewMatrix(gameData.render.mine.viewerEye, gameData.objs.viewer->position.mOrient,
							  gameStates.render.xZoom, bOglScale);
	}
if (pnStartSeg)
	*pnStartSeg = nStartSeg;
}

//------------------------------------------------------------------------------

void RenderEffects (int nWindow)
{
	int bLightnings, bSmoke, bSparks;

#if 1
if (gameStates.app.bMultiThreaded && gameData.app.bUseMultiThreading [rtEffects] && tiEffects.pThread) {
	while (tiEffects.bExec)
		G3_SLEEP (0);
	}
#endif
if (gameStates.render.automap.bDisplay) {
	bLightnings = gameOpts->render.automap.bLightnings;
	bSmoke = gameOpts->render.automap.bSmoke;
	bSparks = gameOpts->render.automap.bSparks;
	}
else {
	bSparks = 1;
	bLightnings = !nWindow || gameOpts->render.lightnings.bAuxViews;
	bSmoke = !nWindow || gameOpts->render.smoke.bAuxViews;
	}
if (bSparks) {
	SEM_ENTER (SEM_SPARKS)
	//PrintLog ("RenderEnergySparks\n");
	RenderEnergySparks ();
	//SEM_LEAVE (SEM_SPARKS)
	}
if (bSmoke) {
	SEM_ENTER (SEM_SMOKE)
	//PrintLog ("RenderSmoke\n");
	RenderSmoke ();
	//SEM_LEAVE (SEM_SMOKE)
	}
if (bLightnings) {
	SEM_ENTER (SEM_LIGHTNINGS)
	//PrintLog ("RenderLightnings\n");
	RenderLightnings ();
	}
//PrintLog ("RenderItems\n");
if (bLightnings)
	SEM_LEAVE (SEM_LIGHTNINGS)
RenderItems ();
#if 1
if (bSmoke)
	SEM_LEAVE (SEM_SMOKE)
if (bSparks)
	SEM_LEAVE (SEM_SPARKS)
#endif
}

//------------------------------------------------------------------------------

void RenderFrame (fix nEyeOffset, int nWindow)
{
	short nStartSeg;

gameStates.render.nWindow = nWindow;
gameStates.render.nEyeOffset = nEyeOffset;
if (gameStates.app.bEndLevelSequence) {
	RenderEndLevelFrame (nEyeOffset, nWindow);
	gameData.app.nFrameCount++;
	return;
	}
#ifdef NEWDEMO
if ((gameData.demo.nState == ND_STATE_RECORDING) && (nEyeOffset >= 0)) {
   if (!gameStates.render.nRenderingType)
   	NDRecordStartFrame (gameData.app.nFrameCount, gameData.time.xFrame);
   if (gameStates.render.nRenderingType != 255)
   	NDRecordViewerObject (gameData.objs.viewer);
	}
#endif

//PrintLog ("StartLightingFrame\n");
StartLightingFrame (gameData.objs.viewer);		//this is for ugly light-smoothing hack
gameStates.ogl.bEnableScissor = !gameStates.render.cameras.bActive && nWindow;
if (!nWindow)
	gameData.render.dAspect = (double) grdCurCanv->cvBitmap.bmProps.w / (double) grdCurCanv->cvBitmap.bmProps.h;
//PrintLog ("G3StartFrame\n");
G3StartFrame (0, !(nWindow || gameStates.render.cameras.bActive));
//PrintLog ("SetRenderView\n");
SetRenderView (nEyeOffset, &nStartSeg, 1);
gameStates.render.nStartSeg = nStartSeg;
if (nClearWindow == 1) {
	if (!nClearWindowColor)
		nClearWindowColor = BLACK_RGBA;	//BM_XRGB(31, 15, 7);
	GrClearCanvas (nClearWindowColor);
	}
#ifdef _DEBUG
if (bShowOnlyCurSide)
	GrClearCanvas (nClearWindowColor);
#endif
#if SHADOWS
if (!gameStates.render.bHaveStencilBuffer)
	extraGameInfo [0].bShadows =
	extraGameInfo [1].bShadows = 0;
if (SHOW_SHADOWS &&
	 !(nWindow || gameStates.render.cameras.bActive || gameStates.render.automap.bDisplay)) {
	if (!gameStates.render.bShadowMaps) {
		gameStates.render.nShadowPass = 1;
#if SOFT_SHADOWS
		if (gameOpts->render.shadows.bSoft = 1)
			gameStates.render.nShadowBlurPass = 1;
#endif
		OglStartFrame (0, 0);
#if SOFT_SHADOWS
		OglViewport (grdCurCanv->cvBitmap.bmProps.x, grdCurCanv->cvBitmap.bmProps.y, 128, 128);
#endif
		RenderMine (nStartSeg, nEyeOffset, nWindow);
#if 1//def RELEASE
		RenderFastShadows (nEyeOffset, nWindow, nStartSeg);
#else
		if (FAST_SHADOWS)
			;//RenderFastShadows (nEyeOffset, nWindow, nStartSeg);
		else
			RenderNeatShadows (nEyeOffset, nWindow, nStartSeg);
#endif
#if SOFT_SHADOWS
		if (gameOpts->render.shadows.bSoft) {
			CreateShadowTexture ();
			gameStates.render.nShadowBlurPass = 2;
			gameStates.render.nShadowPass = 0;
#if 1
			OglStartFrame (0, 1);
			SetRenderView (nEyeOffset, &nStartSeg, 1);
			RenderMine (nStartSeg, nEyeOffset, nWindow);
#endif
			RenderShadowTexture ();
			}
#endif
		nWindow = 0;
		}
	}
else
#endif
	{
	if (gameStates.render.nRenderPass < 0)
		RenderMine (nStartSeg, nEyeOffset, nWindow);
	else {
		for (gameStates.render.nRenderPass = 0;
			gameStates.render.nRenderPass < 2;
			gameStates.render.nRenderPass++) {
			OglStartFrame (0, 1);
			RenderMine (nStartSeg, nEyeOffset, nWindow);
			}
		}
	}
StencilOff ();
if ((gameOpts->render.bDepthSort > 0) || RENDERPATH)
	RenderSkyBox (nWindow);
//PrintLog ("RenderEffects\n");
RenderEffects (nWindow);
if (!(nWindow || gameStates.render.cameras.bActive || gameStates.app.bEndLevelSequence || GuidedInMainView ())) {
	//PrintLog ("RenderRadar\n");
	RenderRadar ();
	}
if (viewInfo.bUsePlayerHeadAngles)
	Draw3DReticle (nEyeOffset);
gameStates.render.nShadowPass = 0;
//PrintLog ("G3EndFrame\n");
G3EndFrame ();
if (!ShowGameMessage (gameData.messages, -1, -1))
	ShowGameMessage (gameData.messages + 1, -1, -1);
}

//------------------------------------------------------------------------------

int nFirstTerminalSeg;

void UpdateRenderedData (int nWindow, tObject *viewer, int rearViewFlag, int user)
{
	Assert(nWindow < MAX_RENDERED_WINDOWS);
	windowRenderedData [nWindow].frame = gameData.app.nFrameCount;
	windowRenderedData [nWindow].viewer = viewer;
	windowRenderedData [nWindow].rearView = rearViewFlag;
	windowRenderedData [nWindow].user = user;
}

//------------------------------------------------------------------------------

void AddObjectToSegList (short nObject, short nSegment)
{
	tObjRenderListItem *pi = gameData.render.mine.renderObjs.objs + gameData.render.mine.renderObjs.nUsed;

pi->nNextItem = gameData.render.mine.renderObjs.ref [nSegment];
gameData.render.mine.renderObjs.ref [nSegment] = gameData.render.mine.renderObjs.nUsed++;
pi->nObject = nObject;
pi->xDist = vmsVector::Dist (OBJECTS [nObject].position.vPos, gameData.render.mine.viewerEye);
}

//------------------------------------------------------------------------------

void BuildRenderObjLists (int nSegCount)
{
PROF_START
	tObject		*objP;
	tSegment		*segP;
	tSegMasks	mask;
	short			nSegment, nNewSeg, nChild, nSide, sideFlag;
	int			nListPos;
	short			nObject;

memset (gameData.render.mine.renderObjs.ref, 0xff, gameData.segs.nSegments * sizeof (gameData.render.mine.renderObjs.ref [0]));
gameData.render.mine.renderObjs.nUsed = 0;

for (nListPos = 0; nListPos < nSegCount; nListPos++) {
	nSegment = gameData.render.mine.nSegRenderList [nListPos];
	if (nSegment == -0x7fff)
		continue;
#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	for (nObject = gameData.segs.objects [nSegment]; nObject != -1; nObject = objP->next) {
		objP = OBJECTS + nObject;
		Assert (objP->nSegment == nSegment);
		if (objP->flags & OF_ATTACHED)
			continue;		//ignore this tObject
		nNewSeg = nSegment;
		if ((objP->nType != OBJ_REACTOR) && ((objP->nType != OBJ_ROBOT) || (objP->id == 65))) { //don't migrate controlcen
			mask = GetSegMasks (OBJPOS (objP)->vPos, nNewSeg, objP->size);
			if (mask.sideMask) {
				for (nSide = 0, sideFlag = 1; nSide < 6; nSide++, sideFlag <<= 1) {
					if (!(mask.sideMask & sideFlag))
						continue;
					segP = gameData.segs.segments + nNewSeg;
					if (WALL_IS_DOORWAY (segP, nSide, NULL) & WID_FLY_FLAG) {	//can explosion migrate through
						nChild = segP->children [nSide];
						if (gameData.render.mine.bVisible [nChild] == gameData.render.mine.nVisible)
							nNewSeg = nChild;	// only migrate to segment in render list
						}
					}
				}
			}
		AddObjectToSegList (nObject, nNewSeg);
		}
	}
PROF_END(ptBuildObjList)
}

//------------------------------------------------------------------------------
//build a list of segments to be rendered
//fills in gameData.render.mine.nSegRenderList & gameData.render.mine.nRenderSegs

typedef struct tSegZRef {
	fix	z;
	//fix	d;
	short	nSegment;
} tSegZRef;

static tSegZRef segZRef [2][MAX_SEGMENTS_D2X];

void QSortSegZRef (short left, short right)
{
	tSegZRef	*ps = segZRef [0];
	tSegZRef	m = ps [(left + right) / 2];
	tSegZRef	h;
	short		l = left,
				r = right;
do {
	while ((ps [l].z > m.z))// || ((segZRef [l].z == m.z) && (segZRef [l].d > m.d)))
		l++;
	while ((ps [r].z < m.z))// || ((segZRef [r].z == m.z) && (segZRef [r].d < m.d)))
		r--;
	if (l <= r) {
		if (l < r) {
			h = ps [l];
			ps [l] = ps [r];
			ps [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortSegZRef (l, right);
if (left < r)
	QSortSegZRef (left, r);
}

//------------------------------------------------------------------------------

void InitSegZRef (int i, int j, int nThread)
{
	tSegZRef		*ps = segZRef [0] + i;
	vmsVector	v;
	int			zMax = -0x7fffffff;
	short			nSegment;

for (; i < j; i++, ps++) {
	nSegment = gameData.render.mine.nSegRenderList [i];
	COMPUTE_SEGMENT_CENTER_I (&v, nSegment);
	G3TransformPoint(v, v, 0);
	v[Z] += gameData.segs.segRads [1][nSegment];
	if (zMax < v[Z])
		zMax = v[Z];
	ps->z = v[Z];
	ps->nSegment = gameData.render.mine.nSegRenderList[i];
	}
tiRender.zMax [nThread] = zMax;
}

//------------------------------------------------------------------------------

void SortRenderSegs (void)
{
	tSegZRef	*ps, *pi, *pj;
	int		h, i, j;

if (gameData.render.mine.nRenderSegs < 2)
	return;
if (RunRenderThreads (rtInitSegZRef))
	gameData.render.zMax = max (tiRender.zMax [0], tiRender.zMax [1]);
else {
	InitSegZRef (0, gameData.render.mine.nRenderSegs, 0);
	gameData.render.zMax = tiRender.zMax [0];
	}
if (!RENDERPATH) {
	if (RunRenderThreads (rtSortSegZRef)) {
		h = gameData.render.mine.nRenderSegs;
		for (i = h / 2, j = h - i, ps = segZRef [1], pi = segZRef [0], pj = pi + h / 2; h; h--) {
			if (i && (!j || (pi->z < pj->z))) {
				*ps++ = *pi++;
				i--;
				}
			else if (j) {
				*ps++ = *pj++;
				j--;
				}
			}
		}
	else
		QSortSegZRef (0, gameData.render.mine.nRenderSegs - 1);
	ps = segZRef [gameStates.app.bMultiThreaded];
	for (i = 0; i < gameData.render.mine.nRenderSegs; i++)
		gameData.render.mine.nSegRenderList [i] = ps [i].nSegment;
	}
}

//------------------------------------------------------------------------------

void CalcRenderDepth (void)
{
vmsVector vCenter;
G3TransformPoint(vCenter, *SEGMENT_CENTER_I(gameData.objs.viewer->nSegment), 0);
vmsVector v;
G3TransformPoint(v, gameData.segs.vMin, 0);
fix d1 = vmsVector::Dist(v, vCenter);
G3TransformPoint(v, gameData.segs.vMax, 0);
fix d2 = vmsVector::Dist(v, vCenter);

if (d1 < d2)
	d1 = d2;
fix r = gameData.segs.segRads [1][gameData.objs.viewer->nSegment];
gameData.render.zMin = 0;
gameData.render.zMax = vCenter[Z] + d1 + r;
}

//-----------------------------------------------------------------

void BuildRenderSegList (short nStartSeg, int nWindow)
{
	int		lCnt, sCnt, eCnt, nSide;
	int		l, i, j;
	short		nChild;
	short		nChildSeg;
	short		*sv;
	sbyte		*s2v;
	ubyte		andCodes, andCodes3D;
	int		bRotated, nSegment, bNotProjected;
	tPortal	*curPortal;
	short		childList [MAX_SIDES_PER_SEGMENT];		//list of ordered sides to process
	int		nChildren, bCullIfBehind;					//how many sides in childList
	tSegment	*segP;

gameData.render.zMin = 0x7fffffff;
gameData.render.zMax = -0x7fffffff;
bCullIfBehind = !SHOW_SHADOWS || (gameStates.render.nShadowPass == 1);
memset (gameData.render.mine.nRenderPos, -1, sizeof (gameData.render.mine.nRenderPos [0]) * (gameData.segs.nSegments));
BumpVisitedFlag ();
#if 1
BumpProcessedFlag ();
BumpVisibleFlag ();
#else
memset (gameData.render.mine.bProcessed, 0, sizeof (gameData.render.mine.bProcessed));
memset (gameData.render.mine.bVisible, 0, sizeof (gameData.render.mine.bVisible));
for (i = 0; i < sizeofa (gameData.render.mine.nSegRenderList); i++)
	gameData.render.mine.nSegRenderList [i] = -32767;
gameData.render.mine.nProcessed = 1;
gameData.render.mine.nVisible = 1;
#endif

if (gameStates.render.automap.bDisplay && gameOpts->render.automap.bTextured && !gameStates.render.automap.bRadar) {
	for (i = gameData.render.mine.nRenderSegs = 0; i < gameData.segs.nSegments; i++)
		if ((gameStates.render.automap.bFull || gameData.render.mine.bAutomapVisited [i]) &&
			 ((gameStates.render.automap.nSegmentLimit == gameStates.render.automap.nMaxSegsAway) ||
			  (gameData.render.mine.bAutomapVisible [i] <= gameStates.render.automap.nSegmentLimit))) {
			gameData.render.mine.nSegRenderList [gameData.render.mine.nRenderSegs++] = i;
			gameData.render.mine.bVisible [i] = gameData.render.mine.nVisible;
			VISIT (i);
			}
	SortRenderSegs ();
	return;
	}

gameData.render.mine.nSegRenderList [0] = nStartSeg;
gameData.render.mine.nSegDepth [0] = 0;
VISIT (nStartSeg);
gameData.render.mine.nRenderPos [nStartSeg] = 0;
sCnt = 0;
lCnt = eCnt = 1;

#ifdef _DEBUG
if (bPreDrawSegs)
	RenderSegmentFaces (nStartSeg, nWindow);
#endif

renderPortals [0].left =
renderPortals [0].top = 0;
renderPortals [0].right = grdCurCanv->cvBitmap.bmProps.w - 1;
renderPortals [0].bot = grdCurCanv->cvBitmap.bmProps.h - 1;

//breadth-first renderer

//build list
for (l = 0; l < gameStates.render.detail.nRenderDepth; l++) {
	//while (sCnt < eCnt) {
	for (sCnt = 0; sCnt < eCnt; sCnt++) {
		if (gameData.render.mine.bProcessed [sCnt] == gameData.render.mine.nProcessed)
			continue;
		gameData.render.mine.bProcessed [sCnt] = gameData.render.mine.nProcessed;
		nSegment = gameData.render.mine.nSegRenderList [sCnt];
		curPortal = renderPortals + sCnt;
		if (nSegment == -1)
			continue;
#ifdef _DEBUG
		if (nSegment == -32767)
			continue;
		if (nSegment == nDbgSeg)
			nSegment = nSegment;
#endif
		segP = gameData.segs.segments + nSegment;
		sv = segP->verts;
		bRotated = 0;
		//look at all sides of this tSegment.
		//tricky code to look at sides in correct order follows
		for (nChild = nChildren = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {		//build list of sides
			nChildSeg = segP->children [nChild];
			if (nChildSeg < 0)
				continue;
			if (!(WALL_IS_DOORWAY (segP, nChild, NULL) & WID_RENDPAST_FLAG))
				continue;
#ifdef _DEBUG
			if (nChildSeg == nDbgSeg)
				nChildSeg = nChildSeg;
#endif
			if (bCullIfBehind) {
				andCodes = 0xff;
				s2v = sideToVerts [nChild];
				if (!bRotated) {
					RotateVertexList (8, sv);
					bRotated = 1;
					}
				if ((0xff & gameData.segs.points [sv [s2v [0]]].p3_codes
							 & gameData.segs.points [sv [s2v [1]]].p3_codes
					 	    & gameData.segs.points [sv [s2v [2]]].p3_codes
						    & gameData.segs.points [sv [s2v [3]]].p3_codes) & CC_BEHIND)
					continue;
				}
			childList [nChildren++] = nChild;
			}
		//now order the sides in some magical way
#if 0
		if (bNewSegSorting && (nChildren > 1))
			SortSegChildren (segP, nChildren, childList);
#endif
		for (nChild = 0; nChild < nChildren; nChild++) {
			nSide = childList [nChild];
			nChildSeg = segP->children [nSide];
#ifdef _DEBUG
			if ((nChildSeg == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
				nChildSeg = nChildSeg;
#endif
			fix x, y;
			tPortal p = {32767, -32767, 32767, -32767};
			bNotProjected = 0;	//a point wasn't projected
#if 0
			if (bRotated < 2) {
				if (!bRotated)
					RotateVertexList (8, segP->verts);
				ProjectVertexList (8, segP->verts);
				bRotated = 2;
				}
#endif
			s2v = sideToVerts [nSide];
			andCodes3D = 0xff;
			for (j = 0; j < 4; j++) {
				g3sPoint *pnt = gameData.segs.points + sv [s2v [j]];
#if 1
				if (pnt->p3_codes & CC_BEHIND) {
					bNotProjected = 1;
					break;
					}
				if (!(pnt->p3_flags & PF_PROJECTED))
					G3ProjectPoint (pnt);
#else
				if (!(pnt->p3_flags & PF_PROJECTED)) {
					bNotProjected = 1;
					break;
					}
#endif
				x = pnt->p3_screen.x;
				y = pnt->p3_screen.y;
				andCodes3D &= pnt->p3_codes;
				if (p.left > x)
					p.left = x;
				if (p.right < x)
					p.right = x;
				if (p.top > y)
					p.top = y;
				if (p.bot < y)
					p.bot = y;
				}
			if (bNotProjected || !(andCodes3D | (0xff & CodePortal (&p, curPortal)))) {	//maybe add this segment
				int rp = gameData.render.mine.nRenderPos [nChildSeg];
				tPortal *newPortal = renderPortals + lCnt;

				if (bNotProjected)
					*newPortal = *curPortal;
				else {
					newPortal->left = max (curPortal->left, p.left);
					newPortal->right = min (curPortal->right, p.right);
					newPortal->top = max (curPortal->top, p.top);
					newPortal->bot = min (curPortal->bot, p.bot);
					}
				//see if this segment has already been visited, and if so, does the current portal expand the old portal?
				if (rp == -1) {
					gameData.render.mine.nRenderPos [nChildSeg] = lCnt;
					gameData.render.mine.nSegRenderList [lCnt] = nChildSeg;
					gameData.render.mine.nSegDepth [lCnt++] = l;
					VISIT (nChildSeg);
					}
				else {
					tPortal *rwP = renderPortals + rp;
					bool bExpand = false;
					int h;
					h = newPortal->left - rwP->left;
					if (h > 0)
						newPortal->left = rwP->left;
					else if (h < 0)
						bExpand = true;
					h = newPortal->right - rwP->right;
					if (h < 0)
						newPortal->right = rwP->right;
					else if (h > 0)
						bExpand = true;
					h = newPortal->top - rwP->top;
					if (h > 0)
						newPortal->top = rwP->top;
					else if (h < 0)
						bExpand = true;
					h = newPortal->bot - rwP->bot;
					if (h < 0)
						newPortal->bot = rwP->bot;
					else if (h > 0)
						bExpand = true;
					if (bExpand) {
						gameData.render.mine.nSegRenderList [lCnt] = -0x7fff;
						*rwP = *newPortal;		//get updated tPortal
						gameData.render.mine.bProcessed [rp] = gameData.render.mine.nProcessed - 1;		//force reprocess
						}
					}
				}
			}
		}
	sCnt = eCnt;
	eCnt = lCnt;
	}

gameData.render.mine.lCntSave = lCnt;
gameData.render.mine.sCntSave = sCnt;
nFirstTerminalSeg = sCnt;
gameData.render.mine.nRenderSegs = lCnt;

for (i = 0; i < gameData.render.mine.nRenderSegs; i++) {
#ifdef _DEBUG
	if (gameData.render.mine.nSegRenderList [i] == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	if (gameData.render.mine.nSegRenderList [i] >= 0)
		gameData.render.mine.bVisible [gameData.render.mine.nSegRenderList [i]] = gameData.render.mine.nVisible;
	}
}

//------------------------------------------------------------------------

void BuildRenderSegListFast (short nStartSeg, int nWindow)
{
	int	nSegment;

if (!RENDERPATH)
	BuildRenderSegList (nStartSeg, nWindow);
else {
	gameData.render.mine.nRenderSegs = 0;
	for (nSegment = 0; nSegment < gameData.segs.nSegments; nSegment++) {
#ifdef _DEBUG
		if (nSegment == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		if (SEGVIS (nStartSeg, nSegment)) {
			gameData.render.mine.bVisible [nSegment] = gameData.render.mine.nVisible;
			gameData.render.mine.nSegRenderList [gameData.render.mine.nRenderSegs++] = nSegment;
			RotateVertexList (8, SEGMENTS [nSegment].verts);
			}
		}
	}
HUDMessage (0, "%d", gameData.render.mine.nRenderSegs);
}

//------------------------------------------------------------------------------

static tObjRenderListItem objRenderList [MAX_OBJECTS_D2X];

void QSortObjRenderList (int left, int right)
{
	int	l = left,
			r = right,
			m = objRenderList [(l + r) / 2].xDist;

do {
	while (objRenderList[l].xDist < m)
		l++;
	while (objRenderList[r].xDist > m)
		r--;
	if (l <= r) {
		if (l < r) {
			tObjRenderListItem h = objRenderList [l];
			objRenderList [l] = objRenderList [r];
			objRenderList [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortObjRenderList (l, right);
if (left < r)
	QSortObjRenderList (left, r);
}

//------------------------------------------------------------------------------

int SortObjList (int nSegment)
{
	tObjRenderListItem	*pi;
	int						i, j;

for (i = gameData.render.mine.renderObjs.ref [nSegment], j = 0; i >= 0; i = pi->nNextItem) {
	pi = gameData.render.mine.renderObjs.objs + i;
	objRenderList [j++] = *pi;
	}
#if 1
if (j > 1)
	QSortObjRenderList (0, j - 1);
#endif
return j;
}

//------------------------------------------------------------------------------

void RenderObjList (int nListPos, int nWindow)
{
	int i, j;
	int saveLinDepth = gameStates.render.detail.nMaxLinearDepth;

gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
for (i = 0, j = SortObjList (gameData.render.mine.nSegRenderList [nListPos]); i < j; i++)
	DoRenderObject (objRenderList [i].nObject, nWindow);	// note link to above else
gameStates.render.detail.nMaxLinearDepth = saveLinDepth;
}

//------------------------------------------------------------------------------

void RenderSegment (int nListPos)
{
	int nSegment = (nListPos < 0) ? -nListPos - 1 : gameData.render.mine.nSegRenderList [nListPos];

if (nSegment < 0)
	return;
if (gameStates.render.automap.bDisplay) {
	if (!(gameStates.render.automap.bFull || gameData.render.mine.bAutomapVisited [nSegment]))
		return;
	if (!gameOpts->render.automap.bSkybox && (gameData.segs.segment2s [nSegment].special == SEGMENT_IS_SKYBOX))
		return;
	}
else {
	if (VISITED (nSegment))
		return;
	}
#ifdef _DEBUG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
VISIT (nSegment);
if (!RenderSegmentFaces (nSegment, gameStates.render.nWindow)) {
	gameData.render.mine.nSegRenderList [nListPos] = -gameData.render.mine.nSegRenderList [nListPos] - 1;
	gameData.render.mine.bVisible [gameData.render.mine.nSegRenderList [nListPos]] = gameData.render.mine.nVisible - 1;
	return;
	}
if ((gameStates.render.nType == 0) && !gameStates.render.automap.bDisplay)
	gameData.render.mine.bAutomapVisited [nSegment] = gameData.render.mine.bSetAutomapVisited;
else if ((gameStates.render.nType == 1) && (gameData.render.mine.renderObjs.ref [gameData.render.mine.nSegRenderList [nListPos]] >= 0)) {
#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	SetNearestStaticLights (nSegment, 1, 1, 0);
	gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0) && ((RENDERPATH && gameOpts->ogl.bObjLighting) || gameOpts->ogl.bLightObjects);
	RenderObjList (nListPos, gameStates.render.nWindow);
	gameStates.render.bApplyDynLight = gameStates.render.nLightingMethod != 0;
	//gameData.render.lights.dynamic.shader.index [0][0].nActive = gameData.render.lights.dynamic.shader.iStaticLights [0];
	}
else if (gameStates.render.nType == 2)	// render objects containing transparency, like explosions
	RenderObjList (nListPos, gameStates.render.nWindow);
}

//------------------------------------------------------------------------------
//renders onto current canvas

int BeginRenderMine (short nStartSeg, fix nEyeOffset, int nWindow)
{
PROF_START
	tPortal	*rwP;
#if 0//def _DEBUG
	int		i;
#endif

if (!nWindow)
	GetPlayerMslLock ();
windowRenderedData [nWindow].numObjects = 0;
#ifdef LASER_HACK
nHackLasers = 0;
#endif
//set up for rendering
gameStates.ogl.fAlpha = GR_ACTUAL_FADE_LEVELS;
if (((gameStates.render.nRenderPass <= 0) &&
	  (gameStates.render.nShadowPass < 2) && (gameStates.render.nShadowBlurPass < 2)) ||
	 gameStates.render.bShadowMaps) {
	if (!gameStates.render.automap.bDisplay)
		RenderStartFrame ();
#if USE_SEGRADS
	TransformSideCenters ();
#endif
	if (!RENDERPATH)
		RotateSideNorms ();
#if defined(EDITOR) && defined (_DEBUG)
	if (bShowOnlyCurSide) {
		RotateVertexList (8, Cursegp->verts);
		RenderSide (Cursegp, Curside);
		goto done_rendering;
		}
#endif
	 }
#ifdef EDITOR
	if (bSearchMode)	{
		}
	else
#endif

#if 1
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
#else
if (((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2) && (gameStates.render.nShadowBlurPass < 2)) ||
	 (gameStates.render.nShadowPass == 2)) {
#endif
	gameStates.ogl.bUseTransform = RENDERPATH;
	BuildRenderSegList (nStartSeg, nWindow);		//fills in gameData.render.mine.nSegRenderList & gameData.render.mine.nRenderSegs
	if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
		BuildRenderObjLists (gameData.render.mine.nRenderSegs);
#if 1
		if (nEyeOffset <= 0)	// Do for left eye or zero.
			SetDynamicLight ();
#endif
		}
	gameStates.ogl.bUseTransform = 0;
	TransformDynLights (0, 1);
	TransformHeadlights ();
	}
if (nClearWindow == 2) {
	if (nFirstTerminalSeg < gameData.render.mine.nRenderSegs) {
		int i;

		if (nClearWindowColor == (unsigned int) -1)
			nClearWindowColor = BLACK_RGBA;
		GrSetColor (nClearWindowColor);
		for (i = nFirstTerminalSeg, rwP = renderPortals; i < gameData.render.mine.nRenderSegs; i++, rwP++) {
			if (gameData.render.mine.nSegRenderList [i] != -0x7fff) {
#ifdef _DEBUG
				if ((rwP->left == -1) || (rwP->top == -1) || (rwP->right == -1) || (rwP->bot == -1))
					Int3();
				else
#endif
					//NOTE LINK TO ABOVE!
					GrRect(rwP->left, rwP->top, rwP->right, rwP->bot);
				}
			}
		}
	}
gameStates.render.bFullBright = gameStates.render.automap.bDisplay && gameOpts->render.automap.bBright;
gameStates.ogl.bStandardContrast = gameStates.app.bNostalgia || IsMultiGame || (gameStates.ogl.nContrast == 8);
#if SHADOWS
gameStates.ogl.bScaleLight = EGI_FLAG (bShadows, 0, 1, 0) && (gameStates.render.nShadowPass < 3) && !FAST_SHADOWS;
#else
gameStates.ogl.bScaleLight = 0;
#endif
gameStates.render.bUseCameras = USE_CAMERAS;
renderItems.nItems = 0;
PROF_END(ptAux);
return !gameStates.render.cameras.bActive && (gameData.objs.viewer->nType != OBJ_ROBOT);
}

//------------------------------------------------------------------------------

void RenderSkyBoxObjects (void)
{
	short		i, nObject;
	short		*segP;

gameStates.render.nType = 1;
for (i = gameData.segs.skybox.nSegments, segP = gameData.segs.skybox.segments; i; i--, segP++)
	for (nObject = gameData.segs.objects [*segP]; nObject != -1; nObject = OBJECTS [nObject].next)
		DoRenderObject (nObject, gameStates.render.nWindow);
}

//------------------------------------------------------------------------------

void RenderSkyBox (int nWindow)
{
PROF_START
if (gameStates.render.bHaveSkyBox && (!gameStates.render.automap.bDisplay || gameOpts->render.automap.bSkybox)) {
	glDepthMask (1);
	if (RENDERPATH)
		RenderSkyBoxFaces ();
	else {
			int	i, bFullBright = gameStates.render.bFullBright;
			short	*segP;

		gameStates.render.nType = 4;
		gameStates.render.bFullBright = 1;
		for (i = gameData.segs.skybox.nSegments, segP = gameData.segs.skybox.segments; i; i--, segP++)
			RenderSegmentFaces (*segP, nWindow);
		gameStates.render.bFullBright = bFullBright;
		}
	RenderSkyBoxObjects ();
	}
PROF_END(ptRenderPass)
}

//------------------------------------------------------------------------------

inline int RenderSegmentList (int nType, int bFrontToBack)
{
PROF_START
gameStates.render.nType = nType;
if (!(EGI_FLAG (bShadows, 0, 1, 0) && FAST_SHADOWS && !gameOpts->render.shadows.bSoft && (gameStates.render.nShadowPass >= 2))) {
	BumpVisitedFlag ();
	if (RENDERPATH == 1)
		RenderFaceList (nType);
	else {
		int nListPos;

		if (bFrontToBack)
			for (nListPos = 0; nListPos < gameData.render.mine.nRenderSegs; )
				RenderSegment (nListPos++);
		else
			for (nListPos = gameData.render.mine.nRenderSegs; nListPos; )
				RenderSegment (--nListPos);
		}
	}
RenderMineObjects (nType);
ResetUsedLights (1, 0);
if (gameStates.app.bMultiThreaded)
	ResetUsedLights (1, 1);
PROF_END(ptRenderPass)
return 1;
}

//------------------------------------------------------------------------------
//renders onto current canvas

extern int bLog;

void RenderMine (short nStartSeg, fix nEyeOffset, int nWindow)
{
PROF_START
#ifdef _DEBUG
if (nWindow)
	nWindow = nWindow;
else
	nWindow = nWindow;
#endif
if (!(RENDERPATH && HaveLightmaps ()))
	gameStates.render.bPerPixelLighting = 0;
else {
	if (gameStates.render.nLightingMethod == 2)
		gameStates.render.bPerPixelLighting = 2;
	else if ((gameStates.render.nLightingMethod == 1) && gameOpts->render.bUseLightmaps)
		gameStates.render.bPerPixelLighting = 1;
	else
		gameStates.render.bPerPixelLighting = 0;
	}
gameStates.ogl.bHaveDepthBuffer =
gameData.render.nTotalFaces =
gameData.render.nTotalObjects =
gameData.render.nTotalSprites =
gameData.render.nTotalLights =
gameData.render.nMaxLights =
gameData.render.nStateChanges =
gameData.render.nShaderChanges = 0;
SetFaceDrawer (-1);
gameData.render.vertColor.bNoShadow = !FAST_SHADOWS && (gameStates.render.nShadowPass == 4);
gameData.render.vertColor.bDarkness = IsMultiGame && gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [IsMultiGame].bDarkness;
gameStates.render.bApplyDynLight =
gameStates.render.bUseDynLight = SHOW_DYN_LIGHT;
if (!EGI_FLAG (bPowerupLights, 0, 0, 0))
	gameData.render.nPowerupFilter = 0;
else if (gameStates.render.bPerPixelLighting == 2)
	gameData.render.nPowerupFilter = 1;
else
	gameData.render.nPowerupFilter = 2;
gameStates.render.bDoCameras = extraGameInfo [0].bUseCameras &&
									    (!IsMultiGame || (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bUseCameras)) &&
										 !gameStates.render.cameras.bActive;
gameStates.render.bDoLightmaps = gameStates.render.color.bLightmapsOk &&
											gameOpts->render.color.bUseLightmaps &&
											gameStates.render.bAmbientColor &&
											!IsMultiGame;
gameStates.ogl.fLightRange = fLightRanges [IsMultiGame ? 1 : extraGameInfo [IsMultiGame].nLightRange];
PROF_END(ptAux)
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
	gameData.render.mine.bSetAutomapVisited = BeginRenderMine (nStartSeg, nEyeOffset, nWindow);
	if (RENDERPATH) {
	//PrintLog  ("ResetSegmentLights\n");
		ResetSegmentLights ();
#if 1
		if (!gameStates.app.bMultiThreaded || gameStates.render.bPerPixelLighting ||
			 (CountRenderFaces () < 16) || !RunRenderThreads (rtComputeFaceLight))
#endif
			{
		//PrintLog  ("ComputeFaceLight\n");
			if (gameStates.render.bTriangleMesh || (gameData.render.mine.nRenderSegs < gameData.segs.nSegments))
				ComputeFaceLight (0, gameData.render.mine.nRenderSegs, 0);
			else
				ComputeFaceLight (0, gameData.segs.nFaces, 0);
			}
		PROF_START
		//PrintLog  ("UpdateSlidingFaces\n");
		UpdateSlidingFaces ();
		PROF_END(ptAux);
		if ((gameStates.render.bPerPixelLighting == 2) && !gameData.app.nFrameCount)
			meshBuilder.BuildVBOs ();

		//PrintLog  ("InitRenderItemBuffer\n");
		InitRenderItemBuffer (gameData.render.zMin, gameData.render.zMax);
		gameStates.render.bHeadlights = gameOpts->ogl.bHeadlight && gameData.render.lights.dynamic.headlights.nLights && 
												  !(gameStates.render.bFullBright || gameStates.render.automap.bDisplay);
		}
	}
//PrintLog  ("RenderSegmentList (0,1)\n");
RenderSegmentList (0, 1);	// render opaque geometry
if ((gameOpts->render.bDepthSort < 1) && !RENDERPATH)
	RenderSkyBox (nWindow);
//PrintLog  ("RenderSegmentList (1,1)\n");
RenderSegmentList (1, 1);		// render objects
if (FAST_SHADOWS ? (gameStates.render.nShadowPass < 2) : (gameStates.render.nShadowPass != 2)) {
	if (!gameData.app.nFrameCount || gameData.render.nColoredFaces) {
		glDepthFunc (GL_LEQUAL);
		//PrintLog  ("RenderSegmentList (2,1)\n");
		RenderSegmentList (2, 1);	// render transparent geometry
		glDepthFunc (GL_LESS);
		}
	if (!gameStates.app.bNostalgia &&
		 (!gameStates.render.automap.bDisplay || gameOpts->render.automap.bCoronas) && gameOpts->render.coronas.bUse) {
 		glEnable (GL_TEXTURE_2D);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc (GL_LEQUAL);
		if (!nWindow) {
			glDepthMask (0);
			//PrintLog  ("RenderSegmentList (3,1)\n");
			RenderSegmentList (3, 1);
			glDepthMask (1);
			}
		glDepthFunc (GL_LESS);
		glDisable (GL_TEXTURE_2D);
		}
	glDepthFunc (GL_LESS);
	}
}

// ----------------------------------------------------------------------------
//	Only called if editor active.
//	Used to determine which face was clicked on.

#ifdef EDITOR

void CheckFace(int nSegment, int nSide, int facenum, int nVertices, short *vp, int tmap1, int tmap2, tUVL *uvlp)
{
	int	i;

	if (bSearchMode) {
		int save_lighting;
		grsBitmap *bm;
		tUVL uvlCopy [8];
		g3sPoint *pointList [4];

		if (tmap2 > 0)
			bm = TexMergeGetCachedBitmap( tmap1, tmap2, nOrient);
		else
			bm = gameData.pig.tex.bitmaps + gameData.pig.tex.bmIndex [tmap1].index;

		for (i = 0; i < nVertices; i++) {
			uvlCopy [i] = uvlp [i];
			pointList [i] = gameData.segs.points + vp [i];
		}

		GrSetColor(0);
		gr_pixel(_search_x, _search_y);	//set our search pixel to color zero
		GrSetColor(1);					//and render in color one
 save_lighting = gameStates.render.nLighting;
 gameStates.render.nLighting = 2;
		//G3DrawPoly(nVertices, vp);
		G3DrawTexPoly (nVertices, pointList, (tUVL *)uvlCopy, bm, 1, -1);
 gameStates.render.nLighting = save_lighting;

		if (gr_ugpixel(&grdCurCanv->cvBitmap, _search_x, _search_y) == 1) {
			found_seg = nSegment;
			found_side = nSide;
			found_face = facenum;
		}
	}
}

//------------------------------------------------------------------------------

void RenderObjectSearch(tObject *objP)
{
	int changed=0;

	//note that we draw each pixel tObject twice, since we cannot control
	//what color the tObject draws in, so we try color 0, then color 1,
	//in case the tObject itself is rendering color 0

	GrSetColor(0);
	gr_pixel(_search_x, _search_y);	//set our search pixel to color zero
	RenderObject (objP, 0, 0);
	if (gr_ugpixel(&grdCurCanv->cvBitmap, _search_x, _search_y) != 0)
		changed=1;

	GrSetColor(1);
	gr_pixel(_search_x, _search_y);	//set our search pixel to color zero
	RenderObject (objP, 0, 0);
	if (gr_ugpixel(&grdCurCanv->cvBitmap, _search_x, _search_y) != 1)
		changed=1;

	if (changed) {
		if (objP->nSegment != -1)
			Cursegp = gameData.segs.segments+objP->nSegment;
		found_seg = -(OBJ_IDX (objP)+1);
	}
}
#endif

//------------------------------------------------------------------------------

#ifdef EDITOR

extern int render_3d_in_big_tPortal;

//finds what tSegment is at a given x&y -  seg, tSide, face are filled in
//works on last frame rendered. returns true if found
//if seg<0, then an tObject was found, and the tObject number is -seg-1
int FindSegSideFace(short x, short y, int *seg, int *tSide, int *face, int *poly)
{
	bSearchMode = -1;
	_search_x = x; _search_y = y;
	found_seg = -1;
	if (render_3d_in_big_tPortal) {
		gsrCanvas temp_canvas;

		GrInitSubCanvas(&temp_canvas, canv_offscreen, 0, 0,
			LargeView.ev_canv->cvBitmap.bmProps.w, LargeView.ev_canv->cvBitmap.bmProps.h);
		GrSetCurrentCanvas(&temp_canvas);
		RenderFrame(0, 0);
	}
	else {
		GrSetCurrentCanvas(&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
		RenderFrame(0, 0);
	}
	bSearchMode = 0;
	*seg = found_seg;
	*tSide = found_side;
	*face = found_face;
	*poly = found_poly;
	return (found_seg!=-1);
}

#endif

//------------------------------------------------------------------------------
// eof
