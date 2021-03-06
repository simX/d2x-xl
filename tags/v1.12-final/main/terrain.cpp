/* $Id: terrain.c, v 1.4 2003/10/10 09:36:35 btb Exp $ */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inferno.h"
#include "3d.h"
#include "error.h"
#include "gr.h"
#include "texmap.h"
#include "iff.h"
#include "u_mem.h"
#include "mono.h"
#include "textures.h"
#include "object.h"
#include "endlevel.h"
#include "fireball.h"
#include "render.h"
#include "vecmat.h"
#include "ogl_render.h"

#define TERRAIN_GRID_MAX_SIZE	64
#define TERRAIN_GRID_SCALE    i2f (2*20)
#define TERRAIN_HEIGHT_SCALE  f1_0
#define f0_4						0x4000

#define GRID_SIZE				(gameData.render.terrain.nGridW * gameData.render.terrain.nGridH)
#define GRID_OFFS(_i,_j)	((_i) * gameData.render.terrain.nGridW + (_j))
#define HEIGHT(_i,_j)		(gameData.render.terrain.pHeightMap [GRID_OFFS (_i,_j)])
#define LIGHT(_i,_j)			(gameData.render.terrain.pLightMap [GRID_OFFS (_i,_j)])

//!!#define HEIGHT (_i, _j)   gameData.render.terrain.pHeightMap [(gameData.render.terrain.nGridH - 1-j)*gameData.render.terrain.nGridW+ (_i)]
//!!#define LIGHT (_i, _j)    gameData.render.terrain.pLightMap [(gameData.render.terrain.nGridH - 1-j)*gameData.render.terrain.nGridW+ (_i)]

#define LIGHTVAL(_i,_j) (((fix) LIGHT (_i,_j))) // << 8)


// LINT: adding function prototypes
void BuildTerrainLightMap (void);
void _CDECL_ FreeTerrainLightMap (void);

// ------------------------------------------------------------------------

void DrawTerrainCell (int i, int j, g3sPoint *p0, g3sPoint *p1, g3sPoint *p2, g3sPoint *p3)
{
	g3sPoint *pointList [3];

p0->p3_index =
p1->p3_index =
p2->p3_index =
p3->p3_index =  - 1;
pointList [0] = p0;
pointList [1] = p1;
pointList [2] = p3;
gameData.render.terrain.uvlList [0][0].l = LIGHTVAL (i, j);
gameData.render.terrain.uvlList [0][1].l = LIGHTVAL (i, j + 1);
gameData.render.terrain.uvlList [0][2].l = LIGHTVAL (i + 1, j);

gameData.render.terrain.uvlList [0][0].u = 
gameData.render.terrain.uvlList [0][1].u = (i) * f0_4; 
gameData.render.terrain.uvlList [0][0].v = 
gameData.render.terrain.uvlList [0][2].v = (j) * f0_4;
gameData.render.terrain.uvlList [0][1].v = (j + 1) * f0_4;
gameData.render.terrain.uvlList [0][2].u = (i + 1) * f0_4;   
#ifdef _DEBUG
G3DrawTexPoly (3, pointList, gameData.render.terrain.uvlList [0], gameData.render.terrain.bmP, NULL, 1, -1);
#else
G3CheckAndDrawTMap (3, pointList, gameData.render.terrain.uvlList [0], gameData.render.terrain.bmP, NULL, NULL);
#endif
if (gameData.render.terrain.bOutline) {
	int lSave = gameStates.render.nLighting;
	gameStates.render.nLighting = 0;
	GrSetColorRGB (255, 0, 0, 255);
	G3DrawLine (pointList [0], pointList [1]);
	G3DrawLine (pointList [2], pointList [0]);
	gameStates.render.nLighting = lSave;
	}

pointList [0] = p1;
pointList [1] = p2;
gameData.render.terrain.uvlList [1][0].l = LIGHTVAL (i, j + 1);
gameData.render.terrain.uvlList [1][1].l = LIGHTVAL (i + 1, j + 1);
gameData.render.terrain.uvlList [1][2].l = LIGHTVAL (i + 1, j);

gameData.render.terrain.uvlList [1][0].u = (i) * f0_4; 
gameData.render.terrain.uvlList [1][1].u =
gameData.render.terrain.uvlList [1][2].u = (i + 1) * f0_4;   
gameData.render.terrain.uvlList [1][0].v = 
gameData.render.terrain.uvlList [1][1].v = (j + 1) * f0_4;
gameData.render.terrain.uvlList [1][2].v = (j) * f0_4;

#ifdef _DEBUG
G3DrawTexPoly (3, pointList, gameData.render.terrain.uvlList [1], gameData.render.terrain.bmP, NULL, 1, -1);
#else
G3CheckAndDrawTMap (3, pointList, gameData.render.terrain.uvlList [1], gameData.render.terrain.bmP, NULL, NULL);
#endif
if (gameData.render.terrain.bOutline) {
	int lSave = gameStates.render.nLighting;
	gameStates.render.nLighting=0;
	GrSetColorRGB (255, 128, 0, 255);
	G3DrawLine (pointList [0], pointList [1]);
	G3DrawLine (pointList [1], pointList [2]);
	G3DrawLine (pointList [2], pointList [0]);
	gameStates.render.nLighting = lSave;
	}

if ((i == gameData.render.terrain.orgI) && (j == gameData.render.terrain.orgJ))
	gameData.render.terrain.nMineTilesDrawn |= 1;
if ((i == gameData.render.terrain.orgI - 1) && (j == gameData.render.terrain.orgJ))
	gameData.render.terrain.nMineTilesDrawn |= 2;
if ((i == gameData.render.terrain.orgI) && (j == gameData.render.terrain.orgJ - 1))
	gameData.render.terrain.nMineTilesDrawn |= 4;
if ((i == gameData.render.terrain.orgI - 1) && (j == gameData.render.terrain.orgJ - 1))
	gameData.render.terrain.nMineTilesDrawn |= 8;

if (gameData.render.terrain.nMineTilesDrawn == 0xf) {
	RenderMine (gameData.endLevel.exit.nSegNum, 0, 0);
	gameData.render.terrain.nMineTilesDrawn = -1;
	}
}

//-----------------------------------------------------------------------------

vmsVector yCache [256];
ubyte ycFlags [256];

extern vmsMatrix mSurfaceOrient;

vmsVector *get_dy_vec (int h)
{
	vmsVector *dyp;

dyp = yCache + h;
if (!ycFlags [h]) {
	vmsVector tv;
	VmVecCopyScale (&tv, &mSurfaceOrient.uVec, h * TERRAIN_HEIGHT_SCALE);
	G3RotateDeltaVec (dyp, &tv);
	ycFlags [h] = 1;
	}
return dyp;
}

//-----------------------------------------------------------------------------

int im=1;

void RenderTerrain (vmsVector *vOrgPoint, int org_2dx, int org_2dy)
{
	vmsVector	tv, delta_i, delta_j;		//delta_y;
	g3sPoint		p, p2, pLast, p2Last, pLowSave, pHighSave;
	int			i, j, iLow, iHigh, jLow, jHigh, iViewer, jViewer;

#if 1
memset (&p, 0, sizeof (g3sPoint));
memset (&p2, 0, sizeof (g3sPoint));
memset (&pLast, 0, sizeof (g3sPoint));
memset (&p2Last, 0, sizeof (g3sPoint));
#endif
gameData.render.terrain.nMineTilesDrawn = 0;	//clear flags
gameData.render.terrain.orgI = org_2dy;
gameData.render.terrain.orgJ = org_2dx;
iLow = 0;  
iHigh = gameData.render.terrain.nGridW - 1;
jLow = 0;  
jHigh = gameData.render.terrain.nGridH - 1;
memset (ycFlags, 0, sizeof (ycFlags));
gameStates.render.nInterpolationMethod = im;
VmVecCopyScale (&tv, &mSurfaceOrient.rVec, TERRAIN_GRID_SCALE);
G3RotateDeltaVec (&delta_i, &tv);
VmVecCopyScale (&tv, &mSurfaceOrient.fVec, TERRAIN_GRID_SCALE);
G3RotateDeltaVec (&delta_j, &tv);
VmVecScaleAdd (&gameData.render.terrain.vStartPoint, vOrgPoint, &mSurfaceOrient.rVec,
					-(gameData.render.terrain.orgI - iLow) * TERRAIN_GRID_SCALE);
VmVecScaleInc (&gameData.render.terrain.vStartPoint, &mSurfaceOrient.fVec, 
					-(gameData.render.terrain.orgJ - jLow) * TERRAIN_GRID_SCALE);
VmVecSub (&tv, &gameData.objs.viewer->position.vPos, &gameData.render.terrain.vStartPoint);
iViewer = VmVecDot (&tv, &mSurfaceOrient.rVec) / TERRAIN_GRID_SCALE;
if (iViewer > iHigh)
	iViewer = iHigh;
jViewer = VmVecDot (&tv, &mSurfaceOrient.fVec) / TERRAIN_GRID_SCALE;
if (jViewer > jHigh)
	jViewer = jHigh;
G3TransformAndEncodePoint (&pLast, &gameData.render.terrain.vStartPoint);
pLowSave = pLast;
for (j = jLow; j <= jHigh; j++) {
	G3AddDeltaVec (gameData.render.terrain.saveRow + j, &pLast, get_dy_vec (HEIGHT (iLow, j)));
	if (j == jHigh)
		pHighSave = pLast;
	else
		G3AddDeltaVec (&pLast, &pLast, &delta_j);
	}
for (i = iLow; i < iViewer; i++) {
	G3AddDeltaVec (&pLowSave, &pLowSave, &delta_i);
	pLast = pLowSave;
	G3AddDeltaVec (&p2Last, &pLast, get_dy_vec (HEIGHT (i + 1, jLow)));
	for (j = jLow; j < jViewer; j++) {
		G3AddDeltaVec (&p, &pLast, &delta_j);
		G3AddDeltaVec (&p2, &p, get_dy_vec (HEIGHT (i + 1, j + 1)));
		DrawTerrainCell (i, j, gameData.render.terrain.saveRow + j, 
							  gameData.render.terrain.saveRow + j + 1, &p2, &p2Last);
		pLast = p;
		gameData.render.terrain.saveRow [j] = p2Last;
		p2Last = p2;
		}
	VmVecNegate (&delta_j);			//don't have a delta sub...
	G3AddDeltaVec (&pHighSave, &pHighSave, &delta_i);
	pLast = pHighSave;
	G3AddDeltaVec (&p2Last, &pLast, get_dy_vec (HEIGHT (i + 1, jHigh)));
	for (j = jHigh - 1; j >= jViewer; j--) {
		G3AddDeltaVec (&p, &pLast, &delta_j);
		G3AddDeltaVec (&p2, &p, get_dy_vec (HEIGHT (i + 1, j)));
		DrawTerrainCell (i, j, gameData.render.terrain.saveRow + j, 
							  gameData.render.terrain.saveRow + j + 1, &p2Last, &p2);
		pLast = p;
		gameData.render.terrain.saveRow [j + 1] = p2Last;
		p2Last = p2;
		}
	gameData.render.terrain.saveRow [j + 1] = p2Last;
	VmVecNegate (&delta_j);		//restore sign of j
	}
//now do i from other end
VmVecNegate (&delta_i);		//going the other way now...
VmVecScaleInc (&gameData.render.terrain.vStartPoint, &mSurfaceOrient.rVec, 
						(iHigh-iLow)*TERRAIN_GRID_SCALE);
G3TransformAndEncodePoint (&pLast, &gameData.render.terrain.vStartPoint);
pLowSave = pLast;
for (j = jLow; j <= jHigh; j++) {
	G3AddDeltaVec (gameData.render.terrain.saveRow + j, &pLast, get_dy_vec (HEIGHT (iHigh, j)));
	if (j == jHigh)
		pHighSave = pLast;
	else
		G3AddDeltaVec (&pLast, &pLast, &delta_j);
	}
for (i = iHigh - 1; i >= iViewer; i--) {
	G3AddDeltaVec (&pLowSave, &pLowSave, &delta_i);
	pLast = pLowSave;
	G3AddDeltaVec (&p2Last, &pLast, get_dy_vec (HEIGHT (i, jLow)));
	for (j = jLow; j < jViewer; j++) {
		G3AddDeltaVec (&p, &pLast, &delta_j);
		G3AddDeltaVec (&p2, &p, get_dy_vec (HEIGHT (i, j + 1)));
		DrawTerrainCell (i, j, &p2Last, &p2, 
							  gameData.render.terrain.saveRow + j + 1, 
							  gameData.render.terrain.saveRow + j);
		pLast = p;
		gameData.render.terrain.saveRow [j] = p2Last;
		p2Last = p2;
		}
	VmVecNegate (&delta_j);			//don't have a delta sub...
	G3AddDeltaVec (&pHighSave, &pHighSave, &delta_i);
	pLast = pHighSave;
	G3AddDeltaVec (&p2Last, &pLast, get_dy_vec (HEIGHT (i, jHigh)));
	for (j = jHigh - 1; j >= jViewer; j--) {
		G3AddDeltaVec (&p, &pLast, &delta_j);
		G3AddDeltaVec (&p2, &p, get_dy_vec (HEIGHT (i, j)));
		DrawTerrainCell (i, j, &p2, &p2Last, 
							  gameData.render.terrain.saveRow + j + 1, 
							  gameData.render.terrain.saveRow + j);
		pLast = p;
		gameData.render.terrain.saveRow [j + 1] = p2Last;
		p2Last = p2;
		}
	gameData.render.terrain.saveRow [j + 1] = p2Last;
	VmVecNegate (&delta_j);		//restore sign of j
	}
}

//-----------------------------------------------------------------------------

void _CDECL_ FreeTerrainHeightMap (void)
{
if (gameData.render.terrain.pHeightMap) {
	PrintLog ("unloading terrain height map\n");
	D2_FREE (gameData.render.terrain.pHeightMap);
	}
}

//-----------------------------------------------------------------------------

void LoadTerrain (char *filename)
{
	grsBitmap	bmHeight;
	int			iff_error;
	int			i, j;
	ubyte			h, hMin, hMax;

PrintLog ("            loading terrain height map\n");
iff_error = iff_read_bitmap (filename, &bmHeight, BM_LINEAR);
if (iff_error != IFF_NO_ERROR) {
#if TRACE	
	con_printf (1, "File %s - IFF error: %s", filename, iff_errormsg (iff_error));
#endif	
	Error ("File %s - IFF error: %s", filename, iff_errormsg (iff_error));
}
if (gameData.render.terrain.pHeightMap)
	D2_FREE (gameData.render.terrain.pHeightMap)
else
	atexit (FreeTerrainHeightMap);		//first time
gameData.render.terrain.nGridW = bmHeight.bmProps.w;
gameData.render.terrain.nGridH = bmHeight.bmProps.h;
Assert (gameData.render.terrain.nGridW <= TERRAIN_GRID_MAX_SIZE);
Assert (gameData.render.terrain.nGridH <= TERRAIN_GRID_MAX_SIZE);
PrintLog ("heightmap loaded, size=%dx%d\n", gameData.render.terrain.nGridW, gameData.render.terrain.nGridH);
gameData.render.terrain.pHeightMap = bmHeight.bmTexBuf;
hMax = 0;
hMin = 255;
for (i = 0; i < gameData.render.terrain.nGridW; i++)
	for (j = 0; j < gameData.render.terrain.nGridH; j++) {
		h = HEIGHT (i, j);
		if (h > hMax)
			hMax = h;
		if (h < hMin)
			hMin = h;
		}
for (i = 0; i < gameData.render.terrain.nGridW; i++) {
	for (j = 0; j < gameData.render.terrain.nGridH; j++) {
		HEIGHT (i, j) -= hMin;
		}
	}
//	D2_FREE (bmHeight.bmTexBuf);
gameData.render.terrain.bmP = gameData.endLevel.terrain.bmP;
#if 0 //the following code turns the (palettized) terrain texture into a white TGA texture for testing
gameData.render.terrain.bmP->bmProps.rowSize *= 4;
gameData.render.terrain.bmP->bmProps.flags |= BM_FLAG_TGA;
D2_FREE (gameData.render.terrain.bmP->bmTexBuf);
gameData.render.terrain.bmP->bmTexBuf = D2_ALLOC (gameData.render.terrain.bmP->bmProps.h * gameData.render.terrain.bmP->bmProps.rowSize);
memset (gameData.render.terrain.bmP->bmTexBuf, 0xFF, gameData.render.terrain.bmP->bmProps.h * gameData.render.terrain.bmP->bmProps.rowSize);
#endif
PrintLog ("            building terrain light map\n");
BuildTerrainLightMap ();
}


//-----------------------------------------------------------------------------

static void GetTerrainPoint (vmsVector *p, int i, int j)
{
if (!gameData.render.terrain.pHeightMap) {
	PrintLog ("no heightmap available\n");
	return;
	}
if (i < 0)
	i = 0;
else if (i >= gameData.render.terrain.nGridW)
	i = gameData.render.terrain.nGridW;
if (j < 0)
	j = 0;
*p = gameData.render.terrain.pPoints [GRID_OFFS (i, j)];
}

//-----------------------------------------------------------------------------

static fix GetTerrainFaceLight (vmsVector *p0, vmsVector *p1, vmsVector *p2)
{
	static vmsVector vLightDir = {{0x2e14, 0xe8f5, 0x5eb8}};
	vmsVector vNormal;

return -VmVecDot (VmVecNormal (&vNormal, p0, p1, p2), &vLightDir);
}

//-----------------------------------------------------------------------------

fix GetAvgTerrainLight (int i, int j)
{
	vmsVector	pp, p [6];
	fix			light, totalLight;
	int			n;

GetTerrainPoint (&pp, i, j);
GetTerrainPoint (p, i - 1, j);
GetTerrainPoint (p + 1, i, j - 1);
GetTerrainPoint (p + 2, i + 1, j - 1);
GetTerrainPoint (p + 3, i + 1, j);
GetTerrainPoint (p + 4, i, j + 1);
GetTerrainPoint (p + 5, i - 1, j + 1);
for (n = 0, totalLight = 0; n < 6; n++) {
	light = abs (GetTerrainFaceLight (&pp, p + n, p + (n + 1) % 6));
	if (light > 0)
		totalLight += light;
	}
return totalLight / 6;
}

//-----------------------------------------------------------------------------

void _CDECL_ FreeTerrainLightMap ()
{
if (gameData.render.terrain.pLightMap) {
	PrintLog ("unloading terrain light map\n");
	D2_FREE (gameData.render.terrain.pLightMap);
	}
}

//-----------------------------------------------------------------------------

void ComputeTerrainPoints (void)
{
	int			i, j;
	vmsVector	*p = gameData.render.terrain.pPoints;

for (i = 0; i < gameData.render.terrain.nGridW; i++) {
	for (j = 0; j < gameData.render.terrain.nGridH; j++) {
		p = gameData.render.terrain.pPoints + GRID_OFFS (i, j);
		p->p.x = TERRAIN_GRID_SCALE * i;
		p->p.z = TERRAIN_GRID_SCALE * j;
		p->p.y = (fix) HEIGHT (i, j) * TERRAIN_HEIGHT_SCALE;
		}
	}
}

//-----------------------------------------------------------------------------

void BuildTerrainLightMap ()
{
	int i, j;
	fix l, l2, lMin = 0x7fffffff, lMax = 0;


if (gameData.render.terrain.pLightMap)
	D2_FREE (gameData.render.terrain.pLightMap)
else
	atexit (FreeTerrainLightMap);		//first time

gameData.render.terrain.pPoints = (vmsVector *) D2_ALLOC (GRID_SIZE * sizeof (vmsVector));
gameData.render.terrain.pLightMap = (fix *) D2_ALLOC (GRID_SIZE * sizeof (fix));
ComputeTerrainPoints ();
for (i = 0; i < gameData.render.terrain.nGridW; i++) {
	for (j = 0; j < gameData.render.terrain.nGridH; j++) {
		l = GetAvgTerrainLight (i, j);
		if (l > lMax)
			lMax = l;
		if (l < lMin)
			lMin = l;
		if (lMin < 0)
			l = GetAvgTerrainLight (i, j);
		}
	}
for (i = 0; i < gameData.render.terrain.nGridW; i++) {
	for (j = 0; j < gameData.render.terrain.nGridH; j++) {
		l = GetAvgTerrainLight (i, j);
		if (lMin == lMax)
			LIGHT (i, j) = l;// >> 8;
		else {
			l2 = FixDiv ((l - lMin), (lMax - lMin));
			if (l2 == f1_0)
				l2--;
			LIGHT (i, j) = l2;// >> 8;
			}
		}
	}
D2_FREE (gameData.render.terrain.pPoints);
}

//-----------------------------------------------------------------------------
//eof
