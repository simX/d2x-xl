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
#ifndef _WIN32
#	include <unistd.h>
#endif

#include "inferno.h"
#include "u_mem.h"
#include "error.h"
#include "ogl_lib.h"
#include "ogl_fastrender.h"
#include "render.h"
#include "gameseg.h"
#include "objrender.h"
#include "lightmap.h"
#include "lightning.h"
#include "sphere.h"
#include "glare.h"
#include "transprender.h"
#include "renderthreads.h"

#define RENDER_TRANSPARENCY 1
#define RENDER_TRANSP_DECALS 1

#define RI_SPLIT_POLYS 0
#define RI_POLY_OFFSET 0
#define RI_POLY_CENTER 1

#ifdef _DEBUG
static int nDbgPoly = -1, nDbgItem = -1;
#endif

//------------------------------------------------------------------------------

tRenderItemBuffer	renderItems;

static tTexCoord2f tcDefault [4] = {{{0,0}},{{1,0}},{{1,1}},{{0,1}}};

inline int AllocRenderItems (void)
{
if (renderItems.pDepthBuffer)
	return 1;
if (!(renderItems.pDepthBuffer = (struct tRenderItem **) D2_ALLOC (ITEM_DEPTHBUFFER_SIZE * sizeof (struct tRenderItem *))))
	return 0;
if (!(renderItems.pItemList = (struct tRenderItem *) D2_ALLOC (ITEM_BUFFER_SIZE * sizeof (struct tRenderItem)))) {
	D2_FREE (renderItems.pDepthBuffer);
	return 0;
	}
renderItems.nFreeItems = 0;
ResetRenderItemBuffer ();
return 1;
}

//------------------------------------------------------------------------------

void FreeRenderItems (void)
{
D2_FREE (renderItems.pItemList);
D2_FREE (renderItems.pDepthBuffer);
}

//------------------------------------------------------------------------------

void ResetRenderItemBuffer (void)
{
memset (renderItems.pDepthBuffer, 0, ITEM_DEPTHBUFFER_SIZE * sizeof (struct tRenderItem **));
memset (renderItems.pItemList, 0, (ITEM_BUFFER_SIZE - renderItems.nFreeItems) * sizeof (struct tRenderItem));
renderItems.nFreeItems = ITEM_BUFFER_SIZE;
}


//------------------------------------------------------------------------------

void InitRenderItemBuffer (int zMin, int zMax)
{
renderItems.zMin = 0;
renderItems.zMax = zMax - renderItems.zMin;
renderItems.zScale = (double) (ITEM_DEPTHBUFFER_SIZE - 1) / (double) (zMax - renderItems.zMin);
if (renderItems.zScale < 0)
	renderItems.zScale = 1;
else if (renderItems.zScale > 1)
	renderItems.zScale = 1;
}

//------------------------------------------------------------------------------

int AddRenderItem (tRenderItemType nType, void *itemData, int itemSize, int nDepth, int nIndex)
{
#if RENDER_TRANSPARENCY
	tRenderItem *ph, *pi, *pj, **pd;
	int			nOffset;

#ifdef _DEBUG
if (nDepth < renderItems.zMin)
	return renderItems.nFreeItems;
if (nDepth > renderItems.zMax) {
	//if (nType != riParticle)
		return renderItems.nFreeItems;
	nDepth = renderItems.zMax;
	}
#else
if ((nDepth < renderItems.zMin) || (nDepth > renderItems.zMax))
	return renderItems.nFreeItems;
#endif
AllocRenderItems ();
if (!renderItems.nFreeItems)
	return 0;
#if 1
	nOffset = (int) ((double) (nDepth - renderItems.zMin) * renderItems.zScale);
#else
if (nIndex < renderItems.zMin)
	nOffset = 0;
else
	nOffset = (int) ((double) (nIndex - renderItems.zMin) * renderItems.zScale);
#endif
if (nOffset >= ITEM_DEPTHBUFFER_SIZE)
	return 0;
pd = renderItems.pDepthBuffer + nOffset;
// find the first particle to insert the new one *before* and place in pj; pi will be it's predecessor (NULL if to insert at list start)
ph = renderItems.pItemList + --renderItems.nFreeItems;
ph->nItem = renderItems.nItems++;
ph->nType = nType;
ph->z = nDepth;
memcpy (&ph->item, itemData, itemSize);
if (*pd)
	pj = *pd;
for (pi = NULL, pj = *pd; pj && ((pj->z < nDepth) || ((pj->z == nDepth) && (pj->nType < nType))); pj = pj->pNextItem)
	pi = pj;
if (pi) {
	ph->pNextItem = pi->pNextItem;
	pi->pNextItem = ph;
	}
else {
	ph->pNextItem = *pd;
	*pd = ph;
	}
if (renderItems.nMinOffs > nOffset)
	renderItems.nMinOffs = nOffset;
if (renderItems.nMaxOffs < nOffset)
	renderItems.nMaxOffs = nOffset;
return renderItems.nFreeItems;
#else
return 0;
#endif
}

//------------------------------------------------------------------------------

int AddRenderItemMT (tRenderItemType nType, void *itemData, int itemSize, int nDepth, int nIndex, int nThread)
{
if (!gameStates.app.bMultiThreaded || (nThread < 0) || !gameData.app.bUseMultiThreading [rtTransparency])
	return AddRenderItem (nType, itemData, itemSize, nDepth, nIndex);
while (tiRenderItems.ti [nThread].bExec)
	G3_SLEEP (0);
tiRenderItems.itemData [nThread].nType = nType;
tiRenderItems.itemData [nThread].nSize = itemSize;
tiRenderItems.itemData [nThread].nDepth = nDepth;
tiRenderItems.itemData [nThread].nIndex = nIndex;
memcpy (&tiRenderItems.itemData [nThread].item, itemData, itemSize);
tiRenderItems.ti [nThread].bExec = 1;
return 1;
}

//------------------------------------------------------------------------------

#if RI_SPLIT_POLYS

int RISplitPoly (tRIPoly *item, int nDepth)
{
	tRIPoly		split [2];
	fVector		vSplit;
	tRgbaColorf	color;
	int			i, l, i0, i1, i2, i3, nMinLen = 0x7fff, nMaxLen = 0;
	float			z, zMin, zMax, *c, *c0, *c1;

split [0] = split [1] = *item;
for (i = i0 = 0; i < split [0].nVertices; i++) {
	l = split [0].sideLength [i];
	if (nMaxLen < l) { 
		nMaxLen = l;
		i0 = i;
		}
	if (nMinLen > l)
		nMinLen = l;
	}
if ((nDepth > 1) || !nMaxLen || (nMaxLen < 10) || ((nMaxLen <= 30) && ((split [0].nVertices == 3) || (nMaxLen <= nMinLen / 2 * 3)))) {
	for (i = 0, zMax = 0, zMin = 1e30f; i < split [0].nVertices; i++) {
		z = split [0].vertices [i].p.z;
		if (zMax < z)
			zMax = z;
		if (zMin > z)
			zMin = z;
		}
#if RI_POLY_CENTER
	return AddRenderItem (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), fl2f (zMax), fl2f ((zMax + zMin) / 2));
#else
	return AddRenderItem (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), fl2f (zMax), fl2f (zMin));
#endif
	}
if (split [0].nVertices == 3) {
	i1 = (i0 + 1) % 3;
	split [0].vertices [i0] =
	split [1].vertices [i1] = *VmVecAvg (&vSplit, split [0].vertices + i0, split [0].vertices + i1);
	split [0].sideLength [i0] =
	split [1].sideLength [i0] = nMaxLen / 2;
	if (split [0].bmP) {
		split [0].texCoord [i0].v.u =
		split [1].texCoord [i1].v.u = (split [0].texCoord [i1].v.u + split [0].texCoord [i0].v.u) / 2;
		split [0].texCoord [i0].v.v =
		split [1].texCoord [i1].v.v = (split [0].texCoord [i1].v.v + split [0].texCoord [i0].v.v) / 2;
		}
	if (split [0].nColors == 3) {
		for (i = 4, c = (float *) &color, c0 = (float *) (split [0].color + i0), c1 = (float *) (split [0].color + i1); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i0] =
		split [1].color [i1] = color;
		}
	}
else {
	i1 = (i0 + 1) % 4;
	i2 = (i0 + 2) % 4;
	i3 = (i1 + 2) % 4;
	split [0].vertices [i1] =
	split [1].vertices [i0] = *VmVecAvg (&vSplit, split [0].vertices + i0, split [0].vertices + i1);
	split [0].vertices [i2] =
	split [1].vertices [i3] = *VmVecAvg (&vSplit, split [0].vertices + i2, split [0].vertices + i3);
	if (split [0].bmP) {
		split [0].texCoord [i1].v.u =
		split [1].texCoord [i0].v.u = (split [0].texCoord [i1].v.u + split [0].texCoord [i0].v.u) / 2;
		split [0].texCoord [i1].v.v =
		split [1].texCoord [i0].v.v = (split [0].texCoord [i1].v.v + split [0].texCoord [i0].v.v) / 2;
		split [0].texCoord [i2].v.u =
		split [1].texCoord [i3].v.u = (split [0].texCoord [i3].v.u + split [0].texCoord [i2].v.u) / 2;
		split [0].texCoord [i2].v.v =
		split [1].texCoord [i3].v.v = (split [0].texCoord [i3].v.v + split [0].texCoord [i2].v.v) / 2;
		}
	if (split [0].nColors == 4) {
		for (i = 4, c = (float *) &color, c0 = (float *) (split [0].color + i0), c1 = (float *) (split [0].color + i1); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i1] =
		split [1].color [i0] = color;
		for (i = 4, c = (float *) &color, c0 = (float *) (split [0].color + i2), c1 = (float *) (split [0].color + i3); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i2] =
		split [1].color [i3] = color;
		}
	split [0].sideLength [i0] =
	split [1].sideLength [i0] =
	split [0].sideLength [i2] =
	split [1].sideLength [i2] = nMaxLen / 2;
	}
return RISplitPoly (split, nDepth + 1) && RISplitPoly (split + 1, nDepth + 1);
}

#endif

//------------------------------------------------------------------------------

int RIAddObject (tObject *objP)
{
	tRIObject	item;
	vmsVector	vPos;

if (objP->nType == 255)
	return 0;
item.objP = objP;
G3TransformPoint (&vPos, &OBJPOS (objP)->vPos, 0);
return AddRenderItem (riObject, &item, sizeof (item), vPos.p.z, vPos.p.z);
}

//------------------------------------------------------------------------------

int RIAddPoly (grsFace *faceP, grsTriangle *triP, grsBitmap *bmP, 
					fVector *vertices, char nVertices, tTexCoord2f *texCoord, tRgbaColorf *color, 
					tFaceColor *altColor, char nColors, char bDepthMask, int nPrimitive, int nWrap, int bAdditive,
					short nSegment)
{
	tRIPoly	item;
	int		i;
	float		z, zMin, zMax, s = GrAlpha ();
#if RI_POLY_CENTER
	float		zCenter;
#endif
item.faceP = faceP;
item.triP = triP;
item.bmP = bmP;
item.nVertices = nVertices;
item.nPrimitive = nPrimitive;
item.nWrap = nWrap;
item.bDepthMask = bDepthMask;
item.bAdditive = bAdditive;
item.nSegment = nSegment;
memcpy (item.texCoord, texCoord ? texCoord : tcDefault, nVertices * sizeof (tTexCoord2f));
if ((item.nColors = nColors)) {
	if (nColors < nVertices)
		nColors = 1;
	if (color) {
		memcpy (item.color, color, nColors * sizeof (tRgbaColorf));
		for (i = 0; i < nColors; i++)
			if (bAdditive)
				item.color [i].alpha = 1;
			else
				item.color [i].alpha *= s;
		}
	else if (altColor) {
		for (i = 0; i < nColors; i++) {
			item.color [i] = altColor [i].color;
			if (bAdditive)
				item.color [i].alpha = 1;
			else
				item.color [i].alpha *= s;
			}
		}
	else
		item.nColors = 0;
	}
memcpy (item.vertices, vertices, nVertices * sizeof (fVector));
#if RI_SPLIT_POLYS
if (bDepthMask && renderItems.bSplitPolys) {
	for (i = 0; i < nVertices; i++)
		item.sideLength [i] = (short) (VmVecDist (vertices + i, vertices + (i + 1) % nVertices) + 0.5f);
	return RISplitPoly (&item, 0);
	}
else 
#endif
	{
#if RI_POLY_CENTER
	zCenter = 0;
	zMin = 1e30f;
	zMax = -1e30f;
#endif
	for (i = 0; i < item.nVertices; i++) {
		z = item.vertices [i].p.z;
#if RI_POLY_CENTER
		zCenter += z;
#endif
		if (zMin > z)
			zMin = z;
		if (zMax < z)
			zMax = z;
		}
	if ((zMax < renderItems.zMin) || (zMin > renderItems.zMax))
		return -1;
#if RI_POLY_CENTER
	zCenter /= item.nVertices;
	if (zCenter < zMin)
		return AddRenderItem (item.bmP ? riTexPoly : riFlatPoly, &item, sizeof (item), fl2f (zMin), fl2f (zMin));
	if (zCenter < zMax)
		return AddRenderItem (item.bmP ? riTexPoly : riFlatPoly, &item, sizeof (item), fl2f (zMax), fl2f (zMax));
	return AddRenderItem (item.bmP ? riTexPoly : riFlatPoly, &item, sizeof (item), fl2f (zCenter), fl2f (zCenter));
#else
	return AddRenderItem (item.bmP ? riTexPoly : riFlatPoly, &item, sizeof (item), fl2f (zMin), fl2f (zMin));
#endif
	}
}

//------------------------------------------------------------------------------

int RIAddFaceTris (grsFace *faceP)
{
	grsTriangle	*triP;
	fVector		vertices [3];
	int			h, i, j, bAdditive = FaceIsAdditive (faceP);
	grsBitmap	*bmP = faceP->bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = BmOverride (bmP, -1);
#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	faceP = faceP;
#endif
for (h = faceP->nTris, triP = gameData.segs.faces.tris + faceP->nTriIndex; h; h--, triP++) {
	for (i = 0, j = triP->nIndex; i < 3; i++, j++) {
#if 1
		G3TransformPoint (vertices + i, gameData.segs.fVertices + triP->index [i], 0);
#else
		if (gameStates.render.automap.bDisplay)
			G3TransformPoint (vertices + i, gameData.segs.fVertices + triP->index [i], 0);
		else
			VmVecFixToFloat (vertices + i, &gameData.segs.points [triP->index [i]].p3_vec);
#endif
		}
	if (!RIAddPoly (faceP, triP, bmP, vertices, 3, gameData.segs.faces.texCoord + triP->nIndex, 
						 gameData.segs.faces.color + triP->nIndex,
						 NULL, 3, 1, GL_TRIANGLES, GL_REPEAT, 
						 bAdditive, faceP->nSegment))
		return 0;
	}
return 1;
}

//------------------------------------------------------------------------------

int RIAddFace (grsFace *faceP)
{
if (gameStates.render.bTriangleMesh)
	return RIAddFaceTris (faceP);

	fVector		vertices [4];
	int			i, j;
	grsBitmap	*bmP = faceP->bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = BmOverride (bmP, -1);
#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	faceP = faceP;
#endif
for (i = 0, j = faceP->nIndex; i < 4; i++, j++) {
	if (gameStates.render.automap.bDisplay)
		G3TransformPoint (vertices + i, gameData.segs.fVertices + faceP->index [i], 0);
	else
		VmVecFixToFloat (vertices + i, &gameData.segs.points [faceP->index [i]].p3_vec);
	}
return RIAddPoly (faceP, NULL, bmP, 
						vertices, 4, gameData.segs.faces.texCoord + faceP->nIndex, 
						gameData.segs.faces.color + faceP->nIndex,
						NULL, 4, 1, GL_TRIANGLE_FAN, GL_REPEAT, 
						FaceIsAdditive (faceP), faceP->nSegment) > 0;
}

//------------------------------------------------------------------------------

int RIAddSprite (grsBitmap *bmP, vmsVector *position, tRgbaColorf *color, 
					  int nWidth, int nHeight, char nFrame, char bAdditive, float fSoftRad)
{
	tRISprite	item;
	vmsVector	vPos;

item.bmP = bmP;
if ((item.bColor = (color != NULL)))
	item.color = *color;
item.nWidth = nWidth;
item.nHeight = nHeight;
item.nFrame = nFrame;
item.bAdditive = bAdditive;
item.fSoftRad = fSoftRad;
G3TransformPoint (&vPos, position, 0);
VmVecFixToFloat (&item.position, &vPos);
AddRenderItem (riSprite, &item, sizeof (item), vPos.p.z, vPos.p.z);
return 0;
}

//------------------------------------------------------------------------------

int RIAddSpark (vmsVector *position, char nType, int nSize, char nFrame)
{
	tRISpark		item;
	vmsVector	vPos;

item.nSize = nSize;
item.nFrame = nFrame;
item.nType = nType;
G3TransformPoint (&vPos, position, 0);
VmVecFixToFloat (&item.position, &vPos);
AddRenderItem (riSpark, &item, sizeof (item), vPos.p.z, vPos.p.z);
return 0;
}

//------------------------------------------------------------------------------

int RIAddSphere (tRISphereType nType, float red, float green, float blue, float alpha, tObject *objP)
{
	tRISphere	item;
	vmsVector	vPos;

item.nType = nType;
item.color.red = red;
item.color.green = green;
item.color.blue = blue;
item.color.alpha = alpha;
item.objP = objP;
G3TransformPoint (&vPos, &objP->position.vPos, 0);
return AddRenderItem (riSphere, &item, sizeof (item), vPos.p.z, vPos.p.z);
}

//------------------------------------------------------------------------------

int RIAddParticle (tParticle *particle, float fBrightness, int nThread)
{
	tRIParticle	item;

item.particle = particle;
item.fBrightness = fBrightness;
G3TransformPoint (&particle->transPos, &particle->pos, gameStates.render.bPerPixelLighting == 2);
if (gameStates.app.bMultiThreaded && gameData.app.bUseMultiThreading [rtTransparency])
	return AddRenderItemMT (riParticle, &item, sizeof (item), particle->transPos.p.z, particle->transPos.p.z, nThread);
else
	return AddRenderItem (riParticle, &item, sizeof (item), particle->transPos.p.z, particle->transPos.p.z);
}

//------------------------------------------------------------------------------

int RIAddLightnings (tLightning *lightnings, short nLightnings, short nDepth)
{
	tRILightning item;
	vmsVector vPos;
	int z;

if (nLightnings < 1)
	return 0;
item.lightning = lightnings;
item.nLightnings = nLightnings;
item.nDepth = nDepth;
for (; nLightnings; nLightnings--, lightnings++) {
	G3TransformPoint (&vPos, &lightnings->vPos, 0);
	z = vPos.p.z;
	G3TransformPoint (&vPos, &lightnings->vEnd, 0);
	if (z < vPos.p.z)
		z = vPos.p.z;
	}
if (!AddRenderItem (riLightning, &item, sizeof (item), z, z))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

int RIAddLightningSegment (fVector *vLine, fVector *vPlasma, tRgbaColorf *color, char bPlasma, char bStart, char bEnd, short nDepth)
{
	tRILightningSegment	item;
	fix z;

memcpy (item.vLine, vLine, 2 * sizeof (fVector));
if ((item.bPlasma = bPlasma))
	memcpy (item.vPlasma, vPlasma, 4 * sizeof (fVector));
memcpy (&item.color, color, sizeof (tRgbaColorf));
item.bStart = bStart;
item.bEnd = bEnd;
item.nDepth = nDepth;
z = fl2f ((item.vLine [0].p.z + item.vLine [1].p.z) / 2);
return AddRenderItem (riLightningSegment, &item, sizeof (item), z, z);
}

//------------------------------------------------------------------------------

int RIAddThruster (grsBitmap *bmP, fVector *vThruster, tTexCoord2f *tcThruster, fVector *vFlame, tTexCoord2f *tcFlame)
{
	tRIThruster	item;
	int			i, j;
	float			z = 0;

item.bmP = bmP;
memcpy (item.vertices, vThruster, 4 * sizeof (fVector));
memcpy (item.texCoord, tcThruster, 4 * sizeof (tTexCoord2f));
if ((item.bFlame = (vFlame != NULL))) {
	memcpy (item.vertices + 4, vFlame, 3 * sizeof (fVector));
	memcpy (item.texCoord + 4, tcFlame, 3 * sizeof (tTexCoord2f));
	j = 7;
	}
else 
	j = 4;
for (i = 0; i < j; i++)
	if (z < item.vertices [i].p.z)
		z = item.vertices [i].p.z;
return AddRenderItem (riThruster, &item, sizeof (item), fl2f (z), fl2f (z));
}

//------------------------------------------------------------------------------

void RIEnableClientState (char bClientState, char bTexCoord, char bColor, char bDecal, int nTMU)
{
glActiveTexture (nTMU);
glClientActiveTexture (nTMU);
if (!bDecal && (bColor != renderItems.bClientColor)) {
	if ((renderItems.bClientColor = bColor))
		glEnableClientState (GL_COLOR_ARRAY);
	else
		glDisableClientState (GL_COLOR_ARRAY);
	}
if (bDecal || (bTexCoord != renderItems.bClientTexCoord)) {
	if ((renderItems.bClientTexCoord = bTexCoord))
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	else
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	}
if (!renderItems.bLightmaps)
	glEnableClientState (GL_NORMAL_ARRAY);
glEnableClientState (GL_VERTEX_ARRAY);
}

//------------------------------------------------------------------------------

void RIDisableClientState (int nTMU, char bDecal, char bFull)
{
#ifdef _DEBUG
if (nTMU == GL_TEXTURE0)
	nTMU = nTMU;
#endif
glActiveTexture (nTMU);
glClientActiveTexture (nTMU);
if (bFull) {
	if (bDecal) {
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		glDisableClientState (GL_COLOR_ARRAY);
			renderItems.bClientTexCoord = 0;
			renderItems.bClientColor = 0;
		}
	else {
		renderItems.bClientState = 0;
		if (renderItems.bClientTexCoord) {
			glDisableClientState (GL_TEXTURE_COORD_ARRAY);
			renderItems.bClientTexCoord = 0;
			}
		if (bDecal || renderItems.bClientColor) {
			glDisableClientState (GL_COLOR_ARRAY);
			renderItems.bClientColor = 0;
			}
		}
	glDisableClientState (GL_VERTEX_ARRAY);
	}
else {
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D);
	}
}

//------------------------------------------------------------------------------

inline void RIResetBitmaps (void)
{
renderItems.bmP [0] =
renderItems.bmP [1] =
renderItems.bmP [2] = NULL;
renderItems.bDecal = 0;
renderItems.bTextured = 0;
renderItems.nFrame = -1;
renderItems.bUseLightmaps = 0;
}

//------------------------------------------------------------------------------

int RISetClientState (char bClientState, char bTexCoord, char bColor, char bUseLightmaps, char bDecal)
{
PROF_START
#if 1
if (renderItems.bUseLightmaps != bUseLightmaps) {
	glActiveTexture (GL_TEXTURE0);
	glClientActiveTexture (GL_TEXTURE0);
	if (bUseLightmaps) {
		glEnable (GL_TEXTURE_2D);
		glEnableClientState (GL_NORMAL_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glEnableClientState (GL_COLOR_ARRAY);
		glEnableClientState (GL_VERTEX_ARRAY);
		renderItems.bClientTexCoord = 
		renderItems.bClientColor = 0;
		}
	else {
		glDisableClientState (GL_NORMAL_ARRAY);
		RIDisableClientState (GL_TEXTURE1, 0, 0);
		if (renderItems.bDecal) {
			RIDisableClientState (GL_TEXTURE2, 1, 0);
			if (renderItems.bDecal == 2)
				RIDisableClientState (GL_TEXTURE3, 1, 0);
			renderItems.bDecal = 0;
			}
		renderItems.bClientTexCoord = 
		renderItems.bClientColor = 1;
		}
	RIResetBitmaps ();
	renderItems.bUseLightmaps = bUseLightmaps;
	}
#endif
#if 0
if (renderItems.bClientState == bClientState) {
	if (bClientState) {
		glActiveTexture (GL_TEXTURE0 + bUseLightmaps);
		glClientActiveTexture (GL_TEXTURE0 + bUseLightmaps);
		if (renderItems.bClientColor != bColor) {
			if ((renderItems.bClientColor = bColor))
				glEnableClientState (GL_COLOR_ARRAY);
			else
				glDisableClientState (GL_COLOR_ARRAY);
			}
		if (renderItems.bClientTexCoord != bTexCoord) {
			if ((renderItems.bClientTexCoord = bTexCoord))
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			else
				glDisableClientState (GL_TEXTURE_COORD_ARRAY);
			}
		}
	else
		glActiveTexture (GL_TEXTURE0 + bUseLightmaps);
	return 1;
	}
else 
#endif
if (bClientState) {
	renderItems.bClientState = 1;
#if RENDER_TRANSP_DECALS
	if (bDecal) {
		if (bDecal == 2)
			RIEnableClientState (bClientState, bTexCoord, 0, 1, GL_TEXTURE2 + bUseLightmaps);
		RIEnableClientState (bClientState, bTexCoord, 0, 1, GL_TEXTURE1 + bUseLightmaps);
		renderItems.bDecal = bDecal;
		}
	else if (renderItems.bDecal) {
		if (renderItems.bDecal == 2)
			RIDisableClientState (GL_TEXTURE2 + bUseLightmaps, 1, 1);
		RIDisableClientState (GL_TEXTURE1 + bUseLightmaps, 1, 1);
		renderItems.bDecal = 0;
		}
#endif
	RIEnableClientState (bClientState, bTexCoord, bColor, 0, GL_TEXTURE0 + bUseLightmaps);
	}
else {
#if RENDER_TRANSP_DECALS
	if (renderItems.bDecal) {
		if (renderItems.bDecal == 2)
			RIDisableClientState (GL_TEXTURE2 + bUseLightmaps, 1, 1);
		RIDisableClientState (GL_TEXTURE1 + bUseLightmaps, 1, 1);
		renderItems.bDecal = 0;
		}
#endif
	RIDisableClientState (GL_TEXTURE0 + bUseLightmaps, 0, 1);
	glActiveTexture (GL_TEXTURE0);
	}
//renderItems.bmP = NULL;
PROF_END(ptRenderStates)
return 1;
}

//------------------------------------------------------------------------------

void RIResetShader (void)
{
if (gameStates.ogl.bShadersOk && (gameStates.render.history.nShader >= 0)) {
	gameData.render.nShaderChanges++;
	if (gameStates.render.history.nShader == 999)
		UnloadGlareShader ();
	else
		glUseProgramObject (0);
	gameStates.render.history.nShader = -1;
	}
}

//------------------------------------------------------------------------------

int LoadRenderItemImage (grsBitmap *bmP, char nColors, char nFrame, int nWrap, 
								 int bClientState, int nTransp, int bShader, int bUseLightmaps, 
								 int bHaveDecal, int bDecal)
{
if (bmP) {
	if (bDecal || RISetClientState (bClientState, 1, nColors > 1, bUseLightmaps, bHaveDecal) || (renderItems.bTextured < 1)) {
		glActiveTexture (GL_TEXTURE0 + bUseLightmaps + bDecal);
		glClientActiveTexture (GL_TEXTURE0 + bUseLightmaps + bDecal);
		glEnable (GL_TEXTURE_2D);
		//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		renderItems.bTextured = 1;
		}
	if (bDecal == 1)
		bmP = BmOverride (bmP, -1);
	{//if ((bmP != renderItems.bmP [bDecal]) || (nFrame != renderItems.nFrame) || (nWrap != renderItems.nWrap)) {
		gameData.render.nStateChanges++;
		if (bmP) {
			if (OglBindBmTex (bmP, 1, nTransp)) {
				renderItems.bmP [bDecal] = NULL;
				return 0;
				}
			if (bDecal != 2)
				bmP = BmOverride (bmP, nFrame);
			OglTexWrap (bmP->glTexture, nWrap);
			renderItems.nWrap = nWrap;
			renderItems.nFrame = nFrame;
			}
		else
			OGL_BINDTEX (0);
		renderItems.bmP [bDecal] = bmP;
		}
	}
else if (RISetClientState (bClientState, 0, /*!renderItems.bLightmaps &&*/ (nColors > 1), bUseLightmaps, 0) || renderItems.bTextured) {
	if (renderItems.bTextured) {
		OGL_BINDTEX (0);
		glDisable (GL_TEXTURE_2D);
		RIResetBitmaps ();
		}
	}
if (!bShader)
	RIResetShader ();
return (renderItems.bClientState == bClientState);
}

//------------------------------------------------------------------------------

inline void RISetRenderPointers (int nTMU, int nIndex, int bDecal)
{
glActiveTexture (nTMU);
glClientActiveTexture (nTMU);
if (renderItems.bTextured)
	glTexCoordPointer (2, GL_FLOAT, 0, (bDecal ? gameData.segs.faces.ovlTexCoord : gameData.segs.faces.texCoord) + nIndex);
glVertexPointer (3, GL_FLOAT, 0, gameData.segs.faces.vertices + nIndex);
}

//------------------------------------------------------------------------------

void RIRenderPoly (tRIPoly *item)
{
PROF_START
	grsFace		*faceP;
	grsTriangle	*triP;
	grsBitmap	*bmTop = NULL, *bmMask;
	int			i, j, nIndex, bLightmaps, bDecal;

#if RI_POLY_OFFSET
if (!item->bmP) {
	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1,1);
	glPolygonMode (GL_FRONT, GL_FILL);
	}
#endif
#ifdef _DEBUG
if (item->bmP && strstr (item->bmP->szName, "glare.tga"))
	item = item;
#endif
#if 1
faceP = item->faceP;
triP = item->triP;
bLightmaps = renderItems.bLightmaps && (faceP != NULL);
#ifdef _DEBUG
if (!bLightmaps)
	bLightmaps = bLightmaps;
if (faceP) {
	if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
	}
else
	nDbgSeg = nDbgSeg;
//renderItems.bUseLightmaps = 0;
#endif
if (renderItems.bDepthMask != item->bDepthMask)
	glDepthMask (renderItems.bDepthMask = item->bDepthMask);
#if RENDER_TRANSP_DECALS
bDecal = faceP && (bmTop = BmOverride (faceP->bmTop, -1));
bmMask = (bDecal && ((bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0) && gameStates.render.textures.bHaveMaskShader) ? BM_MASK (bmTop) : NULL;
#else
bDecal = 0;
bmMask = NULL;
#endif
if (LoadRenderItemImage (item->bmP, bLightmaps ? 0 : item->nColors, 0, item->nWrap, 1, 3, 
	 (faceP != NULL) || gameOpts->render.effects.bSoftParticles, bLightmaps, bmMask ? 2 : bDecal, 0) &&
	 (!bDecal || LoadRenderItemImage (bmTop, 0, 0, item->nWrap, 1, 3, 1, bLightmaps, 0, 1)) &&
	 (!bmMask || LoadRenderItemImage (bmMask, 0, 0, item->nWrap, 1, 3, 1, bLightmaps, 0, 2))) {
	nIndex = triP ? triP->nIndex : faceP ? faceP->nIndex : 0;
	if (triP || faceP) {
		RISetRenderPointers (GL_TEXTURE0 + bLightmaps, nIndex, 0);
		if (!bLightmaps)
			glNormalPointer (GL_FLOAT, 0, gameData.segs.faces.normals + nIndex);
		if (bDecal) {
			RISetRenderPointers (GL_TEXTURE1 + bLightmaps, nIndex, 1);
			if (bmMask)
				RISetRenderPointers (GL_TEXTURE2 + bLightmaps, nIndex, 1);
			}
		}
	else {
		glActiveTexture (GL_TEXTURE0);
		glClientActiveTexture (GL_TEXTURE0);
		if (renderItems.bTextured)
			glTexCoordPointer (2, GL_FLOAT, 0, item->texCoord);
		glVertexPointer (3, GL_FLOAT, sizeof (fVector), item->vertices);
		}
	if (item->nColors > 1) {
		glActiveTexture (GL_TEXTURE0);
		glClientActiveTexture (GL_TEXTURE0);
		glEnableClientState (GL_COLOR_ARRAY);
		glColorPointer (4, GL_FLOAT, 0, item->color);
		if (bLightmaps) {
			glTexCoordPointer (2, GL_FLOAT, 0, gameData.segs.faces.lMapTexCoord + nIndex);
			glNormalPointer (GL_FLOAT, 0, gameData.segs.faces.normals + nIndex);
			glVertexPointer (3, GL_FLOAT, 0, gameData.segs.faces.vertices + nIndex);
			}
		}
	else if (item->nColors == 1)
		glColor4fv ((GLfloat *) item->color);
	else
		glColor3d (1, 1, 1);
	OglSetupTransform (faceP != NULL);
	i = item->bAdditive;
	glEnable (GL_BLEND);
	if (i == 1)
		glBlendFunc (GL_ONE, GL_ONE);
	else if (i == 2)
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	else if (i == 3)
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	else 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef _DEBUG
	if (faceP && (faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
#endif
	if (faceP && gameStates.render.bPerPixelLighting) {
		if (!faceP->bColored) {
			G3SetupGrayScaleShader ((int) faceP->nRenderType, &faceP->color);
			glDrawArrays (item->nPrimitive, 0, item->nVertices);
			}
		else {
			bool bAdditive = false;
#if 0
			if (gameData.render.lights.dynamic.headlights.nLights && !gameStates.render.automap.bDisplay) {
				G3SetupHeadlightShader (renderItems.bTextured, 1, renderItems.bTextured ? NULL : &faceP->color);
				glDrawArrays (item->nPrimitive, 0, item->nVertices);
				bAdditive = true;
				glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				glDepthFunc (GL_LEQUAL);
				}
#endif
#if 1
#	if 0
			if (faceP)
				SetNearestFaceLights (faceP, renderItems.bTextured);
#	endif
			if (gameStates.render.bPerPixelLighting == 1) {
#	if RENDER_TRANSP_DECALS
				G3SetupLightmapShader (faceP, 0, (int) faceP->nRenderType, false);
#	else
				G3SetupLightmapShader (faceP, 0, (int) faceP->nRenderType != 0, false);
#	endif
				glDrawArrays (item->nPrimitive, 0, item->nVertices);
				}
			else {
				gameStates.ogl.iLight = 0;
				gameData.render.lights.dynamic.shader.index [0][0].nActive = -1;
				for (;;) {
					G3SetupPerPixelShader (faceP, 0, faceP->nRenderType, false);	
					glDrawArrays (item->nPrimitive, 0, item->nVertices);
					if ((gameStates.ogl.iLight >= gameStates.ogl.nLights) || 
						 (gameStates.ogl.iLight >= gameStates.render.nMaxLightsPerFace))
						break;
					if (!bAdditive) {
						bAdditive = true;
						if (i)
							glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
						glDepthFunc (GL_LEQUAL);
						}
					}
				}
#endif
#if 1
			if (gameStates.render.bHeadlights) {
				if (faceP) {
					if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
						nDbgSeg = nDbgSeg;
					}
				G3SetupHeadlightShader (renderItems.bTextured, 1, renderItems.bTextured ? NULL : &faceP->color);
				if (!bAdditive) {
					bAdditive = true;
					//if (i)
						glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
					glDepthFunc (GL_LEQUAL);
					}
				glDrawArrays (item->nPrimitive, 0, item->nVertices);
				}
#endif
			if (bAdditive)
				glDepthFunc (GL_LESS);
			}
		}
	else {
		if (i && !gameStates.render.automap.bDisplay) {
			if (gameOpts->render.effects.bSoftParticles)
				LoadGlareShader (3);
			else
				RIResetShader ();
			}
		else 
			G3SetupShader (faceP, 0, 0, 0, item->bmP != NULL, 
								(item->nSegment < 0) || !gameStates.render.automap.bDisplay || gameData.render.mine.bAutomapVisited [item->nSegment],
								renderItems.bTextured ? NULL : faceP ? &faceP->color : item->color);
#if 0
		if (triP)
			glNormal3fv ((GLfloat *) (gameData.segs.faces.normals + triP->nIndex));
		else if (faceP)
			glNormal3fv ((GLfloat *) (gameData.segs.faces.normals + faceP->nIndex));
#endif
		glDrawArrays (item->nPrimitive, 0, item->nVertices);
		}
	OglResetTransform (faceP != NULL);
	if (faceP)
		gameData.render.nTotalFaces++;
	}
else 
#endif
if (LoadRenderItemImage (item->bmP, item->nColors, 0, item->nWrap, 0, 3, 1, HaveLightmaps () && (faceP != NULL), 0, 0)) {
	if (item->bAdditive == 1) {
		RIResetShader ();
		glBlendFunc (GL_ONE, GL_ONE);
		}
	else if (item->bAdditive == 2) {
		RIResetShader ();
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
	else {
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		G3SetupShader (faceP, 0, 0, 0, item->bmP != NULL, 
							(item->nSegment < 0) || !gameStates.render.automap.bDisplay || gameData.render.mine.bAutomapVisited [item->nSegment], 
							item->bmP ? NULL : item->color);
		}
	j = item->nVertices;
	glBegin (item->nPrimitive);
	if (item->nColors > 1) {
		if (item->bmP) {
			for (i = 0; i < j; i++) {
				glColor4fv ((GLfloat *) (item->color + i));
				glTexCoord2fv ((GLfloat *) (item->texCoord + i));
				glVertex3fv ((GLfloat *) (item->vertices + i));
				}
			}
		else {
			for (i = 0; i < j; i++) {
				glColor4fv ((GLfloat *) (item->color + i));
				glVertex3fv ((GLfloat *) (item->vertices + i));
				}
			}
		}
	else {
		if (item->nColors)
			glColor4fv ((GLfloat *) item->color);
		else
			glColor3d (1, 1, 1);
		if (item->bmP) {
			for (i = 0; i < j; i++) {
				glTexCoord2fv ((GLfloat *) (item->texCoord + i));
				glVertex3fv ((GLfloat *) (item->vertices + i));
				}
			}
		else {
			for (i = 0; i < j; i++) {
				glVertex3fv ((GLfloat *) (item->vertices + i));
				}
			}
		}
	glEnd ();
	}
#if RI_POLY_OFFSET
if (!item->bmP) {
	glPolygonOffset (0,0);
	glDisable (GL_POLYGON_OFFSET_FILL);
	}
#endif
if (item->bAdditive)
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
PROF_END(ptRenderFaces)
}

//------------------------------------------------------------------------------

void RIRenderObject (tRIObject *item)
{
//SEM_LEAVE (SEM_LIGHTNINGS)	//might lockup otherwise when creating damage lightnings on cloaked objects
//SEM_LEAVE (SEM_SPARKS)
DrawPolygonObject (item->objP, 0, 1);
glDisable (GL_TEXTURE_2D);
renderItems.bTextured = 0;
renderItems.bClientState = 0;
//SEM_ENTER (SEM_LIGHTNINGS)
//SEM_ENTER (SEM_SPARKS)
}

//------------------------------------------------------------------------------

void RIRenderSprite (tRISprite *item)
{
if (LoadRenderItemImage (item->bmP, item->bColor, item->nFrame, GL_CLAMP, 0, 1, 
								 gameOpts->render.effects.bSoftParticles && (item->fSoftRad > 0), 0, 0, 0)) {
	float		h, w, u, v;
	fVector	fPos = item->position;

	w = (float) f2fl (item->nWidth); 
	h = (float) f2fl (item->nHeight); 
	u = item->bmP->glTexture->u;
	v = item->bmP->glTexture->v;
	if (item->bColor)
		glColor4fv ((GLfloat *) &item->color);
	else
		glColor3f (1, 1, 1);
	if (item->bAdditive == 2)
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	else if (item->bAdditive == 1)
		glBlendFunc (GL_ONE, GL_ONE);
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (gameOpts->render.effects.bSoftParticles)
		LoadGlareShader (item->fSoftRad);
	else if (renderItems.bDepthMask)
		glDepthMask (renderItems.bDepthMask = 0);
	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	fPos.p.x -= w;
	fPos.p.y += h;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (u, 0);
	fPos.p.x += 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (u, v);
	fPos.p.y -= 2 * h;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (0, v);
	fPos.p.x -= 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	glEnd ();
	if (item->bAdditive)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (gameOpts->render.effects.bSoftParticles)
		glEnable (GL_DEPTH_TEST);
	}
}

//------------------------------------------------------------------------------

typedef struct tSparkVertex {
	fVector3		vPos;
	tTexCoord2f	texCoord;
} tSparkVertex;

#define SPARK_BUF_SIZE	1000

typedef struct tSparkBuffer {
	int					nSparks;
	tSparkVertex	info [SPARK_BUF_SIZE * 4];
} tSparkBuffer;

tSparkBuffer sparkBuffer;

//------------------------------------------------------------------------------

void RIFlushSparkBuffer (void)
{
if (sparkBuffer.nSparks &&
	 LoadRenderItemImage (bmpSparks, 0, 0, GL_CLAMP, 1, 1, 
								 gameOpts->render.effects.bSoftParticles, 0, 0, 0)) {
	if (gameOpts->render.effects.bSoftParticles) {
		LoadGlareShader (1);
		}
	else {
		RIResetShader ();
		if (renderItems.bDepthMask)
			glDepthMask (renderItems.bDepthMask = 0);
		//G3DisableClientStates (1, 1, 1, GL_TEXTURE1);
		//OGL_BINDTEX (0);
		//glDisable (GL_TEXTURE_2D);
		}
	//G3EnableClientStates (1, 0, 0, GL_TEXTURE0);
	//glEnable (GL_TEXTURE_2D);
	//OGL_BINDTEX (bmpSparks->glTexture->handle);
	//glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE);
	glColor3f (1, 1, 1);
	glTexCoordPointer (2, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.info [0].texCoord);
	glVertexPointer (3, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.info [0].vPos);
	glDrawArrays (GL_QUADS, 0, 4 * sparkBuffer.nSparks);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (gameOpts->render.effects.bSoftParticles)
		glEnable (GL_DEPTH_TEST);
	sparkBuffer.nSparks = 0;
	}
}

//------------------------------------------------------------------------------

void RIRenderSpark (tRISpark *item)
{
if (sparkBuffer.nSparks >= SPARK_BUF_SIZE)
	RIFlushSparkBuffer ();

	tSparkVertex	*infoP = sparkBuffer.info + 4 * sparkBuffer.nSparks++;
	fVector			vPos = item->position;
	float				nSize = f2fl (item->nSize);
	float				nCol = (float) (item->nFrame / 8);
	float				nRow = (float) (item->nFrame % 8);

if (!item->nType)
	nCol += 4;
infoP->vPos.p.x = vPos.p.x - nSize;
infoP->vPos.p.y = vPos.p.y + nSize;
infoP->vPos.p.z = vPos.p.z;
infoP->texCoord.v.u = nCol / 8.0f;
infoP->texCoord.v.v = (nRow + 1) / 8.0f;
infoP++;
infoP->vPos.p.x = vPos.p.x + nSize;
infoP->vPos.p.y = vPos.p.y + nSize;
infoP->vPos.p.z = vPos.p.z;
infoP->texCoord.v.u = (nCol + 1) / 8.0f;
infoP->texCoord.v.v = (nRow + 1) / 8.0f;
infoP++;
infoP->vPos.p.x = vPos.p.x + nSize;
infoP->vPos.p.y = vPos.p.y - nSize;
infoP->vPos.p.z = vPos.p.z;
infoP->texCoord.v.u = (nCol + 1) / 8.0f;
infoP->texCoord.v.v = nRow / 8.0f;
infoP++;
infoP->vPos.p.x = vPos.p.x - nSize;
infoP->vPos.p.y = vPos.p.y - nSize;
infoP->vPos.p.z = vPos.p.z;
infoP->texCoord.v.u = nCol / 8.0f;
infoP->texCoord.v.v = nRow / 8.0f;
}

//------------------------------------------------------------------------------

void RIRenderSphere (tRISphere *item)
{
	int bDepthSort = gameOpts->render.bDepthSort;

gameOpts->render.bDepthSort = -1;
RISetClientState (0, 0, 0, 0, 0);
RIResetShader ();
if (item->nType == riSphereShield)
	DrawShieldSphere (item->objP, item->color.red, item->color.green, item->color.blue, item->color.alpha);
if (item->nType == riMonsterball)
	DrawMonsterball (item->objP, item->color.red, item->color.green, item->color.blue, item->color.alpha);
RIResetBitmaps ();
glActiveTexture (GL_TEXTURE0 + renderItems.bLightmaps);
glClientActiveTexture (GL_TEXTURE0 + renderItems.bLightmaps);
OGL_BINDTEX (0);
glDisable (GL_TEXTURE_2D);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDepthMask (renderItems.bDepthMask = 0);
glEnable (GL_BLEND);
gameOpts->render.bDepthSort = bDepthSort;
}

//------------------------------------------------------------------------------

void RIRenderBullet (tParticle *pParticle)
{
	tObject	o;

memset (&o, 0, sizeof (o));
o.nType = OBJ_POWERUP;
o.position.vPos = pParticle->pos;
o.position.mOrient = pParticle->orient;
if (0 <= (o.nSegment = FindSegByPos (&o.position.vPos, pParticle->nSegment, 0, 0))) {
	gameData.render.lights.dynamic.shader.index [0][0].nActive = 0;
	o.renderType = RT_POLYOBJ;
	o.rType.polyObjInfo.nModel = BULLET_MODEL;
	o.rType.polyObjInfo.nTexOverride = -1;
	DrawPolygonObject (&o, 0, 1);
	glDisable (GL_TEXTURE_2D);
	renderItems.bTextured = 0;
	renderItems.bClientState = 0;
	gameData.models.nScale = 0;
	}
}

//------------------------------------------------------------------------------

void RIRenderParticle (tRIParticle *item)
{
if (item->particle->nType == 2)
	RIRenderBullet (item->particle);
else {
	RISetClientState (0, 0, 0, 0, 0);
	if (!gameOpts->render.effects.bSoftParticles || (gameStates.render.history.nShader != 999))
		RIResetShader ();
	if (renderItems.nPrevType != riParticle) {
		glEnable (GL_TEXTURE_2D);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		gameData.smoke.nLastType = -1;
		renderItems.bTextured = 1;
		//InitParticleBuffer (renderItems.bLightmaps);
		}
	if (!gameOpts->render.effects.bSoftParticles && renderItems.bDepthMask)
		glDepthMask (renderItems.bDepthMask = 0);
	RenderParticle (item->particle, item->fBrightness);
	RIResetBitmaps ();
	}
}

//------------------------------------------------------------------------------

void RIRenderLightning (tRILightning *item)
{
if (renderItems.bDepthMask)
	glDepthMask (renderItems.bDepthMask = 0);
RISetClientState (0, 0, 0, 0, 0);
RIResetShader ();
RenderLightning (item->lightning, item->nLightnings, item->nDepth, 0);
RIResetBitmaps ();
renderItems.bDepthMask = 1;
}

//------------------------------------------------------------------------------

void RIRenderLightningSegment (tRILightningSegment *item)
{
if (renderItems.bDepthMask)
	glDepthMask (renderItems.bDepthMask = 0);
RISetClientState (0, 0, 0, 0, 0);
RIResetShader ();
RenderLightningSegment (item->vLine, item->vPlasma, &item->color, item->bPlasma, item->bStart, item->bEnd, item->nDepth);
if (item->bPlasma) {
	RIResetBitmaps ();
	}
else
	renderItems.bTextured = 0;
}

//------------------------------------------------------------------------------

void RIRenderThruster (tRIThruster *item)
{
if (!renderItems.bDepthMask)
	glDepthMask (renderItems.bDepthMask = 1);
glBlendFunc (GL_ONE, GL_ONE);
glColor3f (0.75f, 0.75f, 0.75f);
glDisable (GL_CULL_FACE);
#if 1
if (LoadRenderItemImage (item->bmP, 0, 0, GL_CLAMP, 1, 1, 0, 0, 0, 0)) {
	glVertexPointer (3, GL_FLOAT, sizeof (fVector), item->vertices);
	glTexCoordPointer (2, GL_FLOAT, 0, item->texCoord);
	if (item->bFlame)
		glDrawArrays (GL_TRIANGLES, 4, 3);
	glDrawArrays (GL_QUADS, 0, 4);
	}
else 
#endif
if (LoadRenderItemImage (item->bmP, 0, 0, GL_CLAMP, 0, 1, 0, 0, 0, 0)) {
	int i;
	if (item->bFlame) {
		glBegin (GL_TRIANGLES);
		for (i = 0; i < 3; i++) {
			glTexCoord2fv ((GLfloat *) (item->texCoord + 4 + i));
			glVertex3fv ((GLfloat *) (item->vertices + 4 + i));
			}
		glEnd ();
		}
	glBegin (GL_QUADS);
	for (i = 0; i < 4; i++) {
		glTexCoord2fv ((GLfloat *) (item->texCoord + i));
		glVertex3fv ((GLfloat *) (item->vertices + i));
		}
	glEnd ();
	}
glEnable (GL_CULL_FACE);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//------------------------------------------------------------------------------

void RIFlushParticleBuffer (int nType)
{
if ((nType < 0) || ((nType != riParticle) && (gameData.smoke.nLastType >= 0))) {
	FlushParticleBuffer (-1.0f);
	if (nType < 0)
		CloseParticleBuffer ();
#if 1
	RIResetBitmaps ();
#endif
	gameData.smoke.nLastType = -1;
	renderItems.bUseLightmaps = 0;
	}
}

//------------------------------------------------------------------------------

static inline void RIFlushBuffers (int nType)
{
if (nType != riSpark)
	RIFlushSparkBuffer ();
RIFlushParticleBuffer (nType);
}

//------------------------------------------------------------------------------

extern int bLog;

void RenderItems (void)
{
#if RENDER_TRANSPARENCY
	struct tRenderItem	**pd, *pl, *pn;
	int						nDepth, nType, nItems, bParticles, bStencil;

if (!(gameOpts->render.bDepthSort && renderItems.pDepthBuffer && (renderItems.nFreeItems < ITEM_BUFFER_SIZE))) {
	return;
	}
PROF_START
RIResetShader ();
bStencil = StencilOff ();
renderItems.bTextured = -1;
renderItems.bClientState = -1;
renderItems.bClientTexCoord = 0;
renderItems.bClientColor = 0;
renderItems.bDepthMask = 0;
renderItems.bUseLightmaps = 0;
renderItems.bDecal = 0;
renderItems.bLightmaps = HaveLightmaps ();
renderItems.bSplitPolys = (gameStates.render.bPerPixelLighting != 2) && (gameStates.render.bSplitPolys > 0);
renderItems.nWrap = 0;
renderItems.nFrame = -1;
renderItems.bmP [0] = 
renderItems.bmP [1] = NULL;
sparkBuffer.nSparks = 0;
OglDisableLighting ();
G3DisableClientStates (1, 1, 0, GL_TEXTURE2 + renderItems.bLightmaps);
G3DisableClientStates (1, 1, 0, GL_TEXTURE1 + renderItems.bLightmaps);
G3DisableClientStates (1, 1, 0, GL_TEXTURE0 + renderItems.bLightmaps);
G3DisableClientStates (1, 1, 0, GL_TEXTURE0);
pl = renderItems.pItemList + ITEM_BUFFER_SIZE - 1;
bParticles = LoadParticleImages ();
glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDepthFunc (GL_LESS);
glDepthMask (0);
glEnable (GL_CULL_FACE);
BeginRenderSmoke (-1, 1);
nType = -1;
for (pd = renderItems.pDepthBuffer + renderItems.nMaxOffs /*ITEM_DEPTHBUFFER_SIZE - 1*/, nItems = renderItems.nItems; 
	  (pd >= renderItems.pDepthBuffer) && nItems; 
	  pd--) {
	if ((pl = *pd)) {
		nDepth = 0;
		do {
#ifdef _DEBUG
			if (pl->nItem == nDbgItem)
				nDbgItem = nDbgItem;
#endif
			nItems--;
			renderItems.nPrevType = nType;
			nType = pl->nType;
			RIFlushBuffers (nType);
			if ((nType == riTexPoly) || (nType == riFlatPoly)) {
				//RIRenderPoly (&pl->item.poly);
				}
			else if (nType == riObject) {
				RIRenderObject (&pl->item.object);
				}
			else if (nType == riSprite) {
				RIRenderSprite (&pl->item.sprite);
				}
			else if (nType == riSpark) {
				RIRenderSpark (&pl->item.spark);
				}
			else if (nType == riSphere) {
				RIRenderSphere (&pl->item.sphere);
				}
			else if (nType == riParticle) {
				if (bParticles)
					RIRenderParticle (&pl->item.particle);
				}
			else if (nType == riLightning) {
				RIRenderLightning (&pl->item.lightning);
				}
			else if (nType == riLightningSegment) {
				RIRenderLightningSegment (&pl->item.lightningSegment);
				}
			else if (nType == riThruster) {
				RIRenderThruster (&pl->item.thruster);
				}
			pn = pl->pNextItem;
			pl->pNextItem = NULL;
			pl = pn;
			nDepth++;
			} while (pl);
		*pd = NULL;
		}
	}
RIFlushBuffers (-1);
EndRenderSmoke (NULL);
RIResetShader ();
G3DisableClientStates (1, 1, 1, GL_TEXTURE0);
OGL_BINDTEX (0);
G3DisableClientStates (1, 1, 1, GL_TEXTURE1);
OGL_BINDTEX (0);
G3DisableClientStates (1, 1, 1, GL_TEXTURE2);
OGL_BINDTEX (0);
G3DisableClientStates (1, 1, 1, GL_TEXTURE3);
OGL_BINDTEX (0);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDepthFunc (GL_LEQUAL);
glDepthMask (1);
StencilOn (bStencil);
renderItems.nMinOffs = ITEM_DEPTHBUFFER_SIZE;
renderItems.nMaxOffs = 0;
renderItems.nFreeItems = ITEM_BUFFER_SIZE;
PROF_END(ptTranspPolys)
#endif
}

//------------------------------------------------------------------------------
//eof
