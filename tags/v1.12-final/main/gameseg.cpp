/* $Id: gameseg.c, v 1.5 2004/04/14 08:54:35 btb Exp $ */
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
#include <string.h>	//	for memset ()

#include "u_mem.h"
#include "inferno.h"
#include "game.h"
#include "error.h"
#include "mono.h"
#include "vecmat.h"
#include "gameseg.h"
#include "wall.h"
#include "fuelcen.h"
#include "bm.h"
#include "fvi.h"
#include "byteswap.h"
#include "player.h"
#include "gamesave.h"
#include "light.h"
#include "dynlight.h"
#include "gameseg.h"
//#define _DEBUG
#ifdef RCS
static char rcsid [] = "$Id: gameseg.c, v 1.5 2004/04/14 08:54:35 btb Exp $";
#endif

// How far a point can be from a plane, and still be "in" the plane

// -------------------------------------------------------------------------------

#ifdef COMPACT_SEGS

//#define CACHEDBG 1
#define MAX_CACHE_NORMALS 128
#define CACHE_MASK 127

typedef struct normCacheElement {
	short			nSegment;
	ubyte			nSide;
	vmsVector	normals [2];
} normCacheElement;

typedef struct tNormCache {
	int					bInitialized;
	normCacheElement	cache [MAX_CACHE_NORMALS];
#ifdef CACHEDBG
	int					nCounter;
	int					nHits;
	int					nMisses;
#endif
} tNormCache;

tNormCache	normCache = {0};

// -------------------------------------------------------------------------------

void NormCacheInit ()
{
NormCacheFlush ();
normCache.bInitialized = 1;
}

// -------------------------------------------------------------------------------

void NormCacheFlush ()
{
	int i;

for (i = 0; i < MAX_CACHE_NORMALS; i++)
	normCache.cache [i].nSegment = -1;
}


// -------------------------------------------------------------------------------

int FindNormCacheElement (int nSegment, int nSide, int faceFlags)
{
	uint i;

if (!normCache.bInitialized) 
	NormCacheInit ();

#ifdef CACHEDBG
#if TRACE	
	if (( (++normCache.nCounter % 5000) == 1) && (normCache.nHits+normCache.nMisses > 0))
		con_printf (0, "NCACHE %d%% missed, H:%d, M:%d\n", (normCache.nMisses*100)/ (normCache.nHits+normCache.nMisses), normCache.nHits, normCache.nMisses);
#endif
#endif

	i = ((nSegment<<2) ^ nSide) & CACHE_MASK;
	if ((normCache.cache [i].nSegment == nSegment) && ((normCache.cache [i].nSide&0xf) == nSide)) {
		uint f1;
#ifdef CACHEDBG
		normCache.nHits++;
#endif
		f1 = normCache.cache [i].nSide>>4;
		if ((f1&faceFlags) == faceFlags)
			return i;
		if (f1 & 1)
			UncachedGetSideNormal (&gameData.segs.segments [nSegment], nSide, 1, &normCache.cache [i].normals [1]);
		else
			UncachedGetSideNormal (&gameData.segs.segments [nSegment], nSide, 0, &normCache.cache [i].normals [0]);
		normCache.cache [i].nSide |= faceFlags<<4;
		return i;
	}
#ifdef CACHEDBG
	normCache.nMisses++;
#endif

	switch (faceFlags)	{
	case 1:
		UncachedGetSideNormal (&gameData.segs.segments [nSegment], nSide, 0, &normCache.cache [i].normals [0]);
		break;
	case 2:
		UncachedGetSideNormal (&gameData.segs.segments [nSegment], nSide, 1, normCache.cache [i].normals + 1);
		break;
	case 3:
		UncachedGetSideNormals (&gameData.segs.segments [nSegment], nSide, normCache.cache [i].normals, normCache.cache [i].normals + 1);
		break;
	}
	normCache.cache [i].nSegment = nSegment;
	normCache.cache [i].nSide = nSide | (faceFlags<<4);
	return i;
}

// -------------------------------------------------------------------------------

void GetSideNormal (tSegment *segP, int nSide, int face_num, vmsVector * vm)
{
	int i;
	i = FindNormCacheElement (SEG_IDX (segP), nSide, 1 << face_num);

*vm = normCache.cache [i].normals [face_num];
if (0) {
	vmsVector tmp;
	UncachedGetSideNormal (segP, nSide, face_num, &tmp);
	Assert (tmp.x == vm->x);
	Assert (tmp.y == vm->y);
	Assert (tmp.z == vm->z);
	}
}

// -------------------------------------------------------------------------------

void GetSideNormals (tSegment *segP, int nSide, vmsVector * vm1, vmsVector * vm2)
{
	int i = FindNormCacheElement (SEG_IDX (segP), nSide, 3);

*vm1 = normCache.cache [i].normals [0];
*vm2 = normCache.cache [i].normals [1];

if (0) {
	vmsVector tmp;
	UncachedGetSideNormal (segP, nSide, 0, &tmp);
	Assert (tmp.x == vm1->x);
	Assert (tmp.y == vm1->y);
	Assert (tmp.z == vm1->z);
	UncachedGetSideNormal (segP, nSide, 1, &tmp);
	Assert (tmp.x == vm2->x);
	Assert (tmp.y == vm2->y);
	Assert (tmp.z == vm2->z);
	}
}

// -------------------------------------------------------------------------------

void UncachedGetSideNormal (tSegment *segP, int nSide, int face_num, vmsVector * vm)
{
	int	vm0, vm1, vm2, vm3, bFlip;
	sbyte	*vs = sideToVerts [nSide];

switch (segP->sides [nSide].nType) {
	case SIDE_IS_QUAD:
		bFlip = GetVertsForNormal (segP->verts [vs [0]], segP->verts [vs [1]], segP->verts [vs [2]], segP->verts [vs [3]], 
											&vm0, &vm1, &vm2, &vm3, &bFlip);
		VmVecNormalChecked (vm, 
								  gameData.segs.vertices + vm0, 
								  gameData.segs.vertices + vm1, 
								  gameData.segs.vertices + vm2);
		if (bFlip)
			VmVecNegate (vm);
		break;
	case SIDE_IS_TRI_02:
		if (face_num == 0)
			VmVecNormalChecked (vm, 
									  gameData.segs.vertices + segP->verts [vs [0]], 
									  gameData.segs.vertices + segP->verts [vs [1]], 
									  gameData.segs.vertices + segP->verts [vs [2]]);
		else
			VmVecNormalChecked (vm, 
									  gameData.segs.vertices + segP->verts [vs [0]], 
									  gameData.segs.vertices + segP->verts [vs [2]], 
									  gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	case SIDE_IS_TRI_13:
		if (face_num == 0)
			VmVecNormalChecked (vm, 
									  gameData.segs.vertices + segP->verts [vs [0]], 
									  gameData.segs.vertices + segP->verts [vs [1]], 
									  gameData.segs.vertices + segP->verts [vs [3]]);
		else
			VmVecNormalChecked (vm, 
									  gameData.segs.vertices + segP->verts [vs [1]], 
									  gameData.segs.vertices + segP->verts [vs [2]], 
									  gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	}
}

// -------------------------------------------------------------------------------

void UncachedGetSideNormals (tSegment *segP, int nSide, vmsVector * vm1, vmsVector * vm2)
#else
void GetSideNormals (tSegment *segP, int nSide, vmsVector * vm1, vmsVector * vm2)
#endif
{
	int	vvm0, vvm1, vvm2, vvm3, bFlip;
	sbyte	*vs = sideToVerts [nSide];

#ifdef _DEBUG
if ((SEG_IDX (segP) == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
switch (segP->sides [nSide].nType)	{
	case SIDE_IS_QUAD:
		bFlip = GetVertsForNormal (segP->verts [vs [0]], segP->verts [vs [1]], segP->verts [vs [2]], segP->verts [vs [3]], 
											&vvm0, &vvm1, &vvm2, &vvm3);
		VmVecNormalChecked (vm1, 
								  gameData.segs.vertices + vvm0, 
								  gameData.segs.vertices + vvm1, 
								  gameData.segs.vertices + vvm2);
		if (bFlip)
			VmVecNegate (vm1);
		*vm2 = *vm1;
		break;
	case SIDE_IS_TRI_02:
		VmVecNormalChecked (vm1, 
								  gameData.segs.vertices + segP->verts [vs [0]], 
								  gameData.segs.vertices + segP->verts [vs [1]], 
								  gameData.segs.vertices + segP->verts [vs [2]]);
		VmVecNormalChecked (vm2, 
								  gameData.segs.vertices + segP->verts [vs [0]], 
								  gameData.segs.vertices + segP->verts [vs [2]], 
								  gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	case SIDE_IS_TRI_13:
		VmVecNormalChecked (vm1, 
								  gameData.segs.vertices + segP->verts [vs [0]], 
								  gameData.segs.vertices + segP->verts [vs [1]], 
								  gameData.segs.vertices + segP->verts [vs [3]]);
		VmVecNormalChecked (vm2, 
								  gameData.segs.vertices + segP->verts [vs [1]], 
							 	  gameData.segs.vertices + segP->verts [vs [2]], 
								  gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	}
}

// ------------------------------------------------------------------------------------------
// Compute the center point of a tSide of a tSegment.
//	The center point is defined to be the average of the 4 points defining the tSide.
void ComputeSideCenter (vmsVector *vp, tSegment *segP, int tSide)
{
	sbyte	*s2v = sideToVerts [tSide];
	short	*sv = segP->verts;

*vp = gameData.segs.vertices [sv [*s2v++]];
VmVecInc (vp, gameData.segs.vertices + sv [*s2v++]);
VmVecInc (vp, gameData.segs.vertices + sv [*s2v++]);
VmVecInc (vp, gameData.segs.vertices + sv [*s2v]);
vp->p.x /= 4;
vp->p.y /= 4;
vp->p.z /= 4;
}

// ------------------------------------------------------------------------------------------
// Compute the center point of a tSide of a tSegment.
//	The center point is defined to be the average of the 4 points defining the tSide.
void ComputeSideRads (short nSegment, short tSide, fix *prMin, fix *prMax)
{
	tSegment		*segP = gameData.segs.segments + nSegment;
	sbyte			*s2v = sideToVerts [tSide];
	short			*sv = segP->verts;
	vmsVector	v, vCenter, *vp;
	fix 			d, rMin = 0x7fffffff, rMax = 0;
	int			i;

COMPUTE_SIDE_CENTER (&vCenter, segP, tSide);
for (i = 0; i < 4; i++) {
	vp = gameData.segs.vertices + sv [*s2v++];
	VmVecSub (&v, &vCenter, vp);
	d = VmVecMag (&v);
	if (rMin > d)
		rMin = d;
	if (rMax < d)
		rMax = d;
	}
if (prMin)
	*prMin = rMin;
if (prMax)
	*prMax = rMax;
}

// ------------------------------------------------------------------------------------------
// Compute tSegment center.
//	The center point is defined to be the average of the 8 points defining the tSegment.
void ComputeSegmentCenter (vmsVector *vp, tSegment *segP)
{
	int i;
	short	*sv = segP->verts;

*vp = gameData.segs.vertices [*sv++];
for (i = 7; i; i--)
	VmVecInc (vp, gameData.segs.vertices + *sv++);
vp->p.x /= 8;
vp->p.y /= 8;
vp->p.z /= 8;
}

// -----------------------------------------------------------------------------
//	Given two segments, return the side index in the connecting segment which connects to the base segment

int FindConnectedSide (tSegment *baseSegP, tSegment *connSegP)
{
	short	nBaseSeg = SEG_IDX (baseSegP);
	short *childP = connSegP->children;

if (childP [0] == nBaseSeg)
		return 0;
if (childP [1] == nBaseSeg)
		return 1;
if (childP [2] == nBaseSeg)
		return 2;
if (childP [3] == nBaseSeg)
		return 3;
if (childP [4] == nBaseSeg)
		return 4;
if (childP [5] == nBaseSeg)
		return 5;
return -1;
}

// -----------------------------------------------------------------------------------
//	Given a tSide, return the number of faces
int GetNumFaces (tSide *sideP)
{
	short nType = sideP->nType;

if (nType == SIDE_IS_QUAD)
	return 1;
if ((nType == SIDE_IS_TRI_02) || (nType == SIDE_IS_TRI_13))
	return 2;
Error ("Illegal nType = %i\n", sideP->nType);
return 0;
}

// -----------------------------------------------------------------------------------
// Fill in array with four absolute point numbers for a given tSide
void GetSideVertIndex (short *vertIndex, int nSegment, int nSide)
{
	sbyte *sv = sideToVerts [nSide];
	short	*vp = gameData.segs.segments [nSegment].verts;

vertIndex [0] = vp [sv [0]];
vertIndex [1] = vp [sv [1]];
vertIndex [2] = vp [sv [2]];
vertIndex [3] = vp [sv [3]];
}

// -----------------------------------------------------------------------------------
// Fill in array with four absolute point numbers for a given tSide
void GetSideVerts (vmsVector *vertices, int nSegment, int nSide)
{
	short i, vertIndex [4];

GetSideVertIndex (vertIndex, nSegment, nSide);
for (i = 0; i < 4; i++)
	vertices [i] = gameData.segs.vertices [vertIndex [i]];
}

#ifdef EDITOR
// -----------------------------------------------------------------------------------
//	Create all vertex lists (1 or 2) for faces on a tSide.
//	Sets:
//		nFaces		number of lists
//		vertices			vertices in all (1 or 2) faces
//	If there is one face, it has 4 vertices.
//	If there are two faces, they both have three vertices, so face #0 is stored in vertices 0, 1, 2, 
//	face #1 is stored in vertices 3, 4, 5.
// Note: these are not absolute vertex numbers, but are relative to the tSegment
// Note:  for triagulated sides, the middle vertex of each trianle is the one NOT
//   adjacent on the diagonal edge
void CreateAllVertexLists (int *nFaces, int *vertices, int nSegment, int nSide)
{
	tSide	*sideP = &gameData.segs.segments [nSegment].sides [nSide];
	int  *sv = sideToVertsInt [nSide];

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0);
Assert ((nSide >= 0) && (nSide < 6);

switch (sideP->nType) {
	case SIDE_IS_QUAD:

		vertices [0] = sv [0];
		vertices [1] = sv [1];
		vertices [2] = sv [2];
		vertices [3] = sv [3];

		*nFaces = 1;
		break;
	case SIDE_IS_TRI_02:
		*nFaces = 2;
		vertices [0] =
		vertices [5] = sv [0];
		vertices [1] = sv [1];
		vertices [2] =
		vertices [3] = sv [2];
		vertices [4] = sv [3];

		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	case SIDE_IS_TRI_13:
		*nFaces = 2;
		vertices [0] = 
		vertices [5] = sv [3];
		vertices [1] = sv [0];
		vertices [2] = 
		vertices [3] = sv [1];
		vertices [4] = sv [2];
		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	default:
		Error ("Illegal tSide nType (1), nType = %i, tSegment # = %i, tSide # = %i\n", sideP->nType, nSegment, nSide);
		break;
	}
}
#endif

// -----------------------------------------------------------------------------------
// Like create all vertex lists, but returns the vertnums (relative to
// the tSide) for each of the faces that make up the tSide. 
//	If there is one face, it has 4 vertices.
//	If there are two faces, they both have three vertices, so face #0 is stored in vertices 0, 1, 2, 
//	face #1 is stored in vertices 3, 4, 5.
void CreateAllVertNumLists (int *nFaces, int *vertnums, int nSegment, int nSide)
{
	tSide	*sideP = &gameData.segs.segments [nSegment].sides [nSide];

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
switch (sideP->nType) {
	case SIDE_IS_QUAD:
		vertnums [0] = 0;
		vertnums [1] = 1;
		vertnums [2] = 2;
		vertnums [3] = 3;
		*nFaces = 1;
		break;

	case SIDE_IS_TRI_02:
		*nFaces = 2;
		vertnums [0] =
		vertnums [5] = 0;
		vertnums [1] = 1;
		vertnums [2] =
		vertnums [3] = 2;
		vertnums [4] = 3;
		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;

	case SIDE_IS_TRI_13:
		*nFaces = 2;
		vertnums [0] = 
		vertnums [5] = 3;
		vertnums [1] = 0;
		vertnums [2] = 
		vertnums [3] = 1;
		vertnums [4] = 2;
		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;

	default:
		Error ("Illegal tSide nType (2), nType = %i, tSegment # = %i, tSide # = %i\n", sideP->nType, nSegment, nSide);
		break;
	}
}

// -------------------------------------------------------------------------------
//like CreateAllVertexLists (), but generate absolute point numbers
int CreateAbsVertexLists (int *vertices, int nSegment, int nSide)
{
	short	*vp = gameData.segs.segments [nSegment].verts;
	tSide	*sideP = gameData.segs.segments [nSegment].sides + nSide;
	int  *sv = sideToVertsInt [nSide];
	int nFaces;

if (gameData.physics.side.bCache &&
	 (gameData.physics.side.nSegment == nSegment) && 
	 (gameData.physics.side.nSide == nSide) &&
	 (gameData.physics.side.nType == sideP->nType)) {
	memcpy (vertices, gameData.physics.side.vertices, sizeof (gameData.physics.side.vertices));
	return gameData.physics.side.nFaces;
	}
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
switch (sideP->nType) {
	case SIDE_IS_QUAD:
		vertices [0] = vp [sv [0]];
		vertices [1] = vp [sv [1]];
		vertices [2] = vp [sv [2]];
		vertices [3] = vp [sv [3]];
		nFaces = 1;
		break;

	case SIDE_IS_TRI_02:
		gameData.physics.side.nFaces = 2;
		vertices [0] = 
		vertices [5] = vp [sv [0]];
		vertices [1] = vp [sv [1]];
		vertices [2] =
		vertices [3] = vp [sv [2]];
		vertices [4] = vp [sv [3]];
		nFaces = 2;
		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS (), 
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;

	case SIDE_IS_TRI_13:
		gameData.physics.side.nFaces = 2;
		vertices [0] = 
		vertices [5] = vp [sv [3]];
		vertices [1] = vp [sv [0]];
		vertices [2] = 
		vertices [3] = vp [sv [1]];
		vertices [4] = vp [sv [2]];
		nFaces = 2;
		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;

	default:
		Error ("Illegal tSide nType (3), nType = %i, tSegment # = %i, tSide # = %i\n", sideP->nType, nSegment, nSide);
		break;
	}
if (gameData.physics.side.bCache) {
	gameData.physics.side.nSegment = nSegment;
	gameData.physics.side.nSide = nSide;
	gameData.physics.side.nFaces = nFaces;
	gameData.physics.side.nType = sideP->nType;
	memcpy (gameData.physics.side.vertices, vertices, sizeof (gameData.physics.side.vertices));
	}
return nFaces;
}

// -------------------------------------------------------------------------------
//returns 3 different bitmasks with info telling if this sphere is in
//this tSegment.  See tSegMasks structure for info on fields  
tSegMasks GetSideMasks (vmsVector *checkP, int nSegment, int nSide, fix xRad)
{
	int			faceBit, sideBit;
	int			nFaces;
	int			nVertex, nFace;
	int			bSidePokesOut;
	int			nSideCount, nCenterCount;
	int			vertexList [6];
	fix			xDist;
	tSegment		*segP;
	tSide			*sideP;
	tSegment2	*seg2P;
	tSide2		*side2P;
	tSegMasks	masks;

masks.centerMask = 0;
masks.faceMask = 0;
masks.sideMask = 0;
masks.valid = 1;
if (nSegment == -1) {
	return masks;
	}
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
segP = gameData.segs.segments + nSegment;
sideP = segP->sides + nSide;
seg2P = gameData.segs.segment2s + nSegment;
side2P = seg2P->sides + nSide;
nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
faceBit = sideBit = 1;
if (nFaces == 2) {
	nVertex = min (vertexList [0], vertexList [2]);
	if (vertexList [4] < vertexList [1])
		if (gameStates.render.bRendering)
			xDist = VmDistToPlane (&gameData.segs.points [vertexList [4]].p3_vec, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
		else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
	else
		if (gameStates.render.bRendering)
			xDist = VmDistToPlane (&gameData.segs.points [vertexList [1]].p3_vec, side2P->rotNorms + 1, &gameData.segs.points [nVertex].p3_vec);
		else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
	bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
	nSideCount = nCenterCount = 0;

	for (nFace = 0; nFace < 2; nFace++, faceBit <<= 1) {
		if (gameStates.render.bRendering)
			xDist = VmDistToPlane (checkP, side2P->rotNorms + nFace, &gameData.segs.points [nVertex].p3_vec);
		else
			xDist = VmDistToPlane (checkP, sideP->normals + nFace, gameData.segs.vertices + nVertex);
		if (xDist < -PLANE_DIST_TOLERANCE) //in front of face
			nCenterCount++;
		if ((xDist - xRad < -PLANE_DIST_TOLERANCE) && (xDist + xRad >= -PLANE_DIST_TOLERANCE)) {
			masks.faceMask |= faceBit;
			nSideCount++;
			}
		}
	if (bSidePokesOut) {		//must be behind at least one face
		if (nSideCount)
			masks.sideMask |= sideBit;
		if (nCenterCount)
			masks.centerMask |= sideBit;
		}
	else {	//must be behind both faces
		if (nSideCount == 2)
			masks.sideMask |= sideBit;
		if (nCenterCount == 2)
			masks.centerMask |= sideBit;
		}
	}
else {	//only one face on this tSide
	nVertex = vertexList [0];
	if (nVertex > vertexList [1])
		nVertex = vertexList [1];
	if (nVertex > vertexList [2])
		nVertex = vertexList [2];
	if (nVertex > vertexList [3])
		nVertex = vertexList [3];
	if (gameStates.render.bRendering)
		xDist = VmDistToPlane (checkP, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
	else
		xDist = VmDistToPlane (checkP, sideP->normals, gameData.segs.vertices + nVertex);
	if (xDist < -PLANE_DIST_TOLERANCE)
		masks.centerMask |= 1;
	if ((xDist - xRad < -PLANE_DIST_TOLERANCE) && (xDist + xRad >= -PLANE_DIST_TOLERANCE)) {
		masks.faceMask |= 1;
		masks.sideMask |= 1;
		}
	}
masks.valid = 1;
return masks;
}

// -------------------------------------------------------------------------------
//returns 3 different bitmasks with info telling if this sphere is in
//this tSegment.  See tSegMasks structure for info on fields  
tSegMasks GetSegMasks (vmsVector *checkP, int nSegment, fix xRad)
{
	short			nSide, nFace, nFaces, faceBit, sideBit;
	int			nVertex, nSideCount, nCenterCount, bSidePokesOut;
	int			vertexList [6];
	fix			xDist;
	tSegment		*segP;
	tSide			*sideP;
	tSegment2	*seg2P;
	tSide2		*side2P;
	tSegMasks	masks;

masks.centerMask = 0;
masks.faceMask = 0;
masks.sideMask = 0;
masks.valid = 1;
if (nSegment == -1) {
	//Error ("nSegment == -1 in GetSegMasks ()");
	return masks;
	}
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
segP = gameData.segs.segments + nSegment;
sideP = segP->sides;
seg2P = gameData.segs.segment2s + nSegment;
side2P = seg2P->sides;
//check point against each tSide of tSegment. return bitmask
for (nSide = 0, faceBit = sideBit = 1; nSide < 6; nSide++, sideBit <<= 1, sideP++, side2P++) {
	// Get number of faces on this tSide, and at vertexList, store vertices.
	//	If one face, then vertexList indicates a quadrilateral.
	//	If two faces, then 0, 1, 2 define one triangle, 3, 4, 5 define the second.
	nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
	//ok...this is important.  If a tSide has 2 faces, we need to know if
	//those faces form a concave or convex tSide.  If the tSide pokes out, 
	//then a point is on the back of the tSide if it is behind BOTH faces, 
	//but if the tSide pokes in, a point is on the back if behind EITHER face.

	if (nFaces == 2) {
		nVertex = min (vertexList [0], vertexList [2]);
		if (vertexList [4] < vertexList [1])
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [4]].p3_vec, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
		else
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [1]].p3_vec, side2P->rotNorms + 1, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nSideCount = nCenterCount = 0;

		for (nFace = 0; nFace < 2; nFace++, faceBit <<= 1) {
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (checkP, side2P->rotNorms + nFace, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (checkP, sideP->normals + nFace, gameData.segs.vertices + nVertex);
			if (xDist < -PLANE_DIST_TOLERANCE) //in front of face
				// check if the intersection of a line through the point that is orthogonal to the 
				// plane of the current triangle lies in is inside that triangle
				nCenterCount++;
			if (xDist - xRad < -PLANE_DIST_TOLERANCE) {
				masks.faceMask |= faceBit;
				nSideCount++;
				}
			}
		if (bSidePokesOut) {		//must be behind at least one face
			if (nSideCount)
				masks.sideMask |= sideBit;
			if (nCenterCount)
				masks.centerMask |= sideBit;
			}
		else {							//must be behind both faces
			if (nSideCount == 2)
				masks.sideMask |= sideBit;
			if (nCenterCount == 2)
				masks.centerMask |= sideBit;
			}
		}
	else {				//only one face on this tSide
		//use lowest point number
		nVertex = vertexList [0];
		//some manual loop unrolling here ...
		if (nVertex > vertexList [1])
			nVertex = vertexList [1];
		if (nVertex > vertexList [2])
			nVertex = vertexList [2];
		if (nVertex > vertexList [3])
			nVertex = vertexList [3];
		if (gameStates.render.bRendering)
			xDist = VmDistToPlane (checkP, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
		else
			xDist = VmDistToPlane (checkP, sideP->normals, gameData.segs.vertices + nVertex);
		if (xDist < -PLANE_DIST_TOLERANCE)
			masks.centerMask |= sideBit;
		if (xDist - xRad < -PLANE_DIST_TOLERANCE) {
			masks.faceMask |= faceBit;
			masks.sideMask |= sideBit;
			}
		faceBit <<= 2;
		}
	}
masks.valid = 1;
return masks;
}

// -------------------------------------------------------------------------------
//this was converted from GetSegMasks ()...it fills in an array of 6
//elements for the distace behind each tSide, or zero if not behind
//only gets centerMask, and assumes zero rad
ubyte GetSideDists (vmsVector *checkP, int nSegment, fix *xSideDists, int bBehind)
{
	short			nSide, nFaces, faceBit, sideBit;
	ubyte			mask;
	int			vertexList [6];
	tSegment		*segP;
	tSide			*sideP;
	tSegment2	*seg2P;
	tSide2		*side2P;

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
if (nSegment == -1)
	Error ("nSegment == -1 in GetSideDists ()");

segP = gameData.segs.segments + nSegment;
sideP = segP->sides;
seg2P = gameData.segs.segment2s + nSegment;
side2P = seg2P->sides;
//check point against each tSide of tSegment. return bitmask
mask = 0;
for (nSide = 0, faceBit = sideBit = 1; nSide < 6; nSide++, sideBit <<= 1, sideP++, side2P++) {
		int	bSidePokesOut;
		int	nFace;

	xSideDists [nSide] = 0;
	nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
	if (nFaces == 2) {
		fix	xDist;
		int	nCenterCount;
		int	nVertex;
		nVertex = min (vertexList [0], vertexList [2]);
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
#endif
		if (vertexList [4] < vertexList [1])
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [4]].p3_vec, 
											  side2P->rotNorms, 
											  &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], 
											  sideP->normals, 
											  gameData.segs.vertices + nVertex);
		else
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [1]].p3_vec, 
											  side2P->rotNorms + 1, 
											  &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], 
											  sideP->normals + 1, 
											  gameData.segs.vertices + nVertex);
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nCenterCount = 0;
		for (nFace = 0; nFace < 2; nFace++, faceBit <<= 1) {
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (checkP, side2P->rotNorms + nFace, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (checkP, sideP->normals + nFace, gameData.segs.vertices + nVertex);
			if ((xDist < -PLANE_DIST_TOLERANCE) == bBehind) {	//in front of face
				nCenterCount++;
				xSideDists [nSide] += xDist;
				}
			}
		if (!bSidePokesOut) {		//must be behind both faces
			if (nCenterCount == 2) {
				mask |= sideBit;
				xSideDists [nSide] /= 2;		//get average
				}
			}
		else {							//must be behind at least one face
			if (nCenterCount) {
				mask |= sideBit;
				if (nCenterCount == 2)
					xSideDists [nSide] /= 2;		//get average
				}
			}
		}
	else {				//only one face on this tSide
		fix xDist;
		int nVertex;
		//use lowest point number
		nVertex = vertexList [0];
		if (nVertex > vertexList [1])
			nVertex = vertexList [1];
		if (nVertex > vertexList [2])
			nVertex = vertexList [2];
		if (nVertex > vertexList [3])
			nVertex = vertexList [3];
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
#endif
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (checkP, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (checkP, sideP->normals, gameData.segs.vertices + nVertex);
		if ((xDist < -PLANE_DIST_TOLERANCE) == bBehind) {
			mask |= sideBit;
			xSideDists [nSide] = xDist;
			}
		faceBit <<= 2;
		}
	}
return mask;
}

// -------------------------------------------------------------------------------
//this was converted from GetSegMasks ()...it fills in an array of 6
//elements for the distace behind each tSide, or zero if not behind
//only gets centerMask, and assumes zero rad
ubyte GetSideDistsAll (vmsVector *checkP, int nSegment, fix *xSideDists)
{
	int			sn, faceBit, sideBit;
	ubyte			mask;
	int			nFaces;
	int			vertexList [6];
	tSegment		*segP;
	tSide			*sideP;
	tSegment2	*seg2P;
	tSide2		*side2P;

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
if (nSegment == -1)
	Error ("nSegment == -1 in GetSideDistsAll ()");

segP = gameData.segs.segments + nSegment;
sideP = segP->sides;
seg2P = gameData.segs.segment2s + nSegment;
side2P = seg2P->sides;
//check point against each tSide of tSegment. return bitmask
mask = 0;
for (sn = 0, faceBit = sideBit = 1; sn < 6; sn++, sideBit <<= 1, sideP++) {
		int	bSidePokesOut;
		int	fn;

	xSideDists [sn] = 0;
	// Get number of faces on this tSide, and at vertexList, store vertices.
	//	If one face, then vertexList indicates a quadrilateral.
	//	If two faces, then 0, 1, 2 define one triangle, 3, 4, 5 define the second.
	while (0 > (nFaces = CreateAbsVertexLists (vertexList, nSegment, sn)))
		;
	//ok...this is important.  If a tSide has 2 faces, we need to know if
	//those faces form a concave or convex tSide.  If the tSide pokes out, 
	//then a point is on the back of the tSide if it is behind BOTH faces, 
	//but if the tSide pokes in, a point is on the back if behind EITHER face.

	if (nFaces == 2) {
		fix	xDist;
		int	nCenterCount;
		int	nVertex;
		nVertex = min (vertexList [0], vertexList [2]);
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			nFaces = CreateAbsVertexLists (vertexList, nSegment, sn);
#endif
		if (vertexList [4] < vertexList [1])
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [4]].p3_vec, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
		else
			if (gameStates.render.bRendering)
				xDist = VmDistToPlane (&gameData.segs.points [vertexList [1]].p3_vec, side2P->rotNorms + 1, &gameData.segs.points [nVertex].p3_vec);
			else
				xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nCenterCount = 0;
		for (fn = 0; fn < 2; fn++, faceBit <<= 1) {
			xDist = VmDistToPlane (checkP, sideP->normals + fn, gameData.segs.vertices + nVertex);
			if (xDist < -PLANE_DIST_TOLERANCE) {	//in front of face
				nCenterCount++;
				}
			xSideDists [sn] += xDist;
			}
		if (!bSidePokesOut) {		//must be behind both faces
			if (nCenterCount == 2)
				mask |= sideBit;
			}
		else {							//must be behind at least one face
			if (nCenterCount)
				mask |= sideBit;
			}
		xSideDists [sn] /= 2;		//get average
		}
	else {				//only one face on this tSide
		fix xDist;
		int nVertex;
		//use lowest point number
		nVertex = vertexList [0];
		if (nVertex > vertexList [1])
			nVertex = vertexList [1];
		if (nVertex > vertexList [2])
			nVertex = vertexList [2];
		if (nVertex > vertexList [3])
			nVertex = vertexList [3];
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			nFaces = CreateAbsVertexLists (vertexList, nSegment, sn);
#endif
		if (gameStates.render.bRendering)
			xDist = VmDistToPlane (checkP, side2P->rotNorms, &gameData.segs.points [nVertex].p3_vec);
		else
			xDist = VmDistToPlane (checkP, sideP->normals, gameData.segs.vertices + nVertex);
		if (xDist < -PLANE_DIST_TOLERANCE) {
			mask |= sideBit;
			}
		xSideDists [sn] = xDist;
		faceBit <<= 2;
		}
	}
return mask;
}

// -------------------------------------------------------------------------------
#ifdef _DEBUG
#ifndef COMPACT_SEGS
//returns true if errors detected
int CheckNorms (int nSegment, int nSide, int facenum, int csegnum, int csidenum, int cfacenum)
{
	vmsVector *n0, *n1;

	n0 = &gameData.segs.segments [nSegment].sides [nSide].normals [facenum];
	n1 = &gameData.segs.segments [csegnum].sides [csidenum].normals [cfacenum];

	if (n0->p.x != -n1->p.x  ||  n0->p.y != -n1->p.y  ||  n0->p.z != -n1->p.z) {
#if TRACE
		con_printf (CONDBG, "Seg %x, tSide %d, norm %d doesn't match seg %x, tSide %d, norm %d:\n"
				"   %8x %8x %8x\n"
				"   %8x %8x %8x (negated)\n", 
				nSegment, nSide, facenum, csegnum, csidenum, cfacenum, 
				n0->p.x, n0->p.y, n0->p.z, -n1->p.x, -n1->p.y, -n1->p.z);
#endif
		return 1;
	}
	else
		return 0;
}

// -------------------------------------------------------------------------------
//heavy-duty error checking
int CheckSegmentConnections (void)
{
	int nSegment, nSide;
	int errors=0;

	for (nSegment=0;nSegment<=gameData.segs.nLastSegment;nSegment++) {
		tSegment *seg;

		seg = &gameData.segs.segments [nSegment];

		for (nSide=0;nSide<6;nSide++) {
			tSide *s;
			tSegment *cseg;
			tSide *cs;
			int nFaces, csegnum, csidenum, con_num_faces;
			int vertexList [6], con_vertex_list [6];

			s = &seg->sides [nSide];

			nFaces = CreateAbsVertexLists (vertexList, nSegment, nSide);
			csegnum = seg->children [nSide];
			if (csegnum >= 0) {
				cseg = &gameData.segs.segments [csegnum];
				csidenum = FindConnectedSide (seg, cseg);

				if (csidenum == -1) {
#if TRACE
					con_printf (CONDBG, "Could not find connected tSide for seg %x back to seg %x, tSide %d\n", csegnum, nSegment, nSide);
#endif
					errors = 1;
					continue;
				}

				cs = &cseg->sides [csidenum];

				con_num_faces = CreateAbsVertexLists (con_vertex_list, csegnum, csidenum);

				if (con_num_faces != nFaces) {
#if TRACE
					con_printf (CONDBG, "Seg %x, tSide %d: nFaces (%d) mismatch with seg %x, tSide %d (%d)\n", nSegment, nSide, nFaces, csegnum, csidenum, con_num_faces);
#endif
					errors = 1;
				}
				else
					if (nFaces == 1) {
						int t;

						for (t=0;t<4 && con_vertex_list [t]!=vertexList [0];t++);

						if (t == 4 ||
							 vertexList [0] != con_vertex_list [t] ||
							 vertexList [1] != con_vertex_list [ (t+3)%4] ||
							 vertexList [2] != con_vertex_list [ (t+2)%4] ||
							 vertexList [3] != con_vertex_list [ (t+1)%4]) {
#if TRACE
							con_printf (CONDBG, "Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
									"  %x %x %x %x\n"
									"  %x %x %x %x\n", 
									nSegment, nSide, csegnum, csidenum, 
									vertexList [0], vertexList [1], vertexList [2], vertexList [3], 
									con_vertex_list [0], con_vertex_list [1], con_vertex_list [2], con_vertex_list [3]);
#endif
							errors = 1;
						}
						else
							errors |= CheckNorms (nSegment, nSide, 0, csegnum, csidenum, 0);

					}
					else {

						if (vertexList [1] == con_vertex_list [1]) {
	
							if (vertexList [4] != con_vertex_list [4] ||
								 vertexList [0] != con_vertex_list [2] ||
								 vertexList [2] != con_vertex_list [0] ||
								 vertexList [3] != con_vertex_list [5] ||
								 vertexList [5] != con_vertex_list [3]) {
#if TRACE
								con_printf (CONDBG, 
									"Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
									"  %x %x %x  %x %x %x\n"
									"  %x %x %x  %x %x %x\n", 
									nSegment, nSide, csegnum, csidenum, 
									vertexList [0], vertexList [1], vertexList [2], vertexList [3], vertexList [4], vertexList [5], 
									con_vertex_list [0], con_vertex_list [1], con_vertex_list [2], con_vertex_list [3], con_vertex_list [4], con_vertex_list [5]);
								con_printf (CONDBG, 
									"Changing seg:tSide %4i:%i from %i to %i\n", 
									csegnum, csidenum, gameData.segs.segments [csegnum].sides [csidenum].nType, 5-gameData.segs.segments [csegnum].sides [csidenum].nType);
#endif
								gameData.segs.segments [csegnum].sides [csidenum].nType = 5-gameData.segs.segments [csegnum].sides [csidenum].nType;
							} else {
								errors |= CheckNorms (nSegment, nSide, 0, csegnum, csidenum, 0);
								errors |= CheckNorms (nSegment, nSide, 1, csegnum, csidenum, 1);
							}

						} else {
	
							if (vertexList [1] != con_vertex_list [4] ||
								 vertexList [4] != con_vertex_list [1] ||
								 vertexList [0] != con_vertex_list [5] ||
								 vertexList [5] != con_vertex_list [0] ||
								 vertexList [2] != con_vertex_list [3] ||
								 vertexList [3] != con_vertex_list [2]) {
#if TRACE
								con_printf (CONDBG, 
									"Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
									"  %x %x %x  %x %x %x\n"
									"  %x %x %x  %x %x %x\n", 
									nSegment, nSide, csegnum, csidenum, 
									vertexList [0], vertexList [1], vertexList [2], vertexList [3], vertexList [4], vertexList [5], 
									con_vertex_list [0], con_vertex_list [1], con_vertex_list [2], con_vertex_list [3], con_vertex_list [4], vertexList [5]);
								con_printf (CONDBG, 
									"Changing seg:tSide %4i:%i from %i to %i\n", 
									csegnum, csidenum, gameData.segs.segments [csegnum].sides [csidenum].nType, 5-gameData.segs.segments [csegnum].sides [csidenum].nType);
#endif
								gameData.segs.segments [csegnum].sides [csidenum].nType = 5-gameData.segs.segments [csegnum].sides [csidenum].nType;
							} else {
								errors |= CheckNorms (nSegment, nSide, 0, csegnum, csidenum, 1);
								errors |= CheckNorms (nSegment, nSide, 1, csegnum, csidenum, 0);
							}
						}
					}
			}
		}
	}
	return errors;

}
#endif
#endif

// -------------------------------------------------------------------------------
//	Used to become a constant based on editor, but I wanted to be able to set
//	this for omega blob FindSegByPos calls.  Would be better to pass a paremeter
//	to the routine...--MK, 01/17/96
int	bDoingLightingHack=0;

//figure out what seg the given point is in, tracing through segments
//returns tSegment number, or -1 if can't find tSegment
int TraceSegs (vmsVector *p0, int nOldSeg)
{
	int				centerMask, nMaxSide;
	tSegment			*segP;
	fix				xSideDists [6], xMaxDist;
	int				nSide, bit, check = -1;
	static int		nTraceDepth = 0;
	static char		bVisited [MAX_SEGMENTS_D2X];

Assert ((nOldSeg <= gameData.segs.nLastSegment) && (nOldSeg >= 0));
if (nTraceDepth >= gameData.segs.nSegments) {
#if TRACE
	con_printf (CONDBG, "TraceSegs: Segment not found\n");
	con_printf (CONDBG, "TraceSegs (gameseg.c) - Something went wrong - infinite loop\n");
#endif
	return -1;
}
if (!nTraceDepth)
	memset (bVisited, 0, sizeof (bVisited));
if (bVisited [nOldSeg])
	return -1;
nTraceDepth++;
bVisited [nOldSeg] = 1;
centerMask = GetSideDists (p0, nOldSeg, xSideDists, 1);		//check old tSegment
if (!centerMask) {		//we're in the old tSegment
	nTraceDepth--;
	return nOldSeg;		//..say so
	}
segP = gameData.segs.segments + nOldSeg;
for (;;) {
	nMaxSide = -1; 
	xMaxDist = 0;
	for (nSide = 0, bit = 1; nSide < 6; nSide ++, bit <<= 1)
		if ((centerMask & bit) && (segP->children [nSide] > -1) && (xSideDists [nSide] < xMaxDist)) {
			xMaxDist = xSideDists [nSide];
			nMaxSide = nSide;
			}
	if (nMaxSide == -1)
		break;
	xSideDists [nMaxSide] = 0;
	check = TraceSegs (p0, segP->children [nMaxSide]);	//trace into adjacent tSegment
	if (check >= 0)
		break;
	}
nTraceDepth--;
return check;		//we haven't found a tSegment
}



int	nExhaustiveCount=0, nExhaustiveFailedCount=0;

// -------------------------------------------------------------------------------
//Tries to find a tSegment for a point, in the following way:
// 1. Check the given tSegment
// 2. Recursively trace through attached segments
// 3. Check all the segmentns
//Returns nSegment if found, or -1
int FindSegByPos (vmsVector *p, int nSegment, int bExhaustive, int bSkyBox)
{
	int			nNewSeg = -1, i;
	short		*segP;

//allow nSegment == -1, meaning we have no idea what tSegment point is in
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= -1));
if (nSegment != -1) {
	nNewSeg = TraceSegs (p, nSegment);
	if (nNewSeg != -1)			//we found a tSegment!
		return nNewSeg;
	}
//couldn't find via attached segs, so search all segs
if (bDoingLightingHack || !bExhaustive)
	return -1;
++nExhaustiveCount;
#if 0 //TRACE
con_printf (1, "Warning: doing exhaustive search to find point tSegment (%i times)\n", nExhaustiveCount);
#endif
if (bSkyBox) {
	for (i = gameData.segs.skybox.nSegments, segP = gameData.segs.skybox.segments; i; i--, segP++) 
		if (!GetSegMasks (p, *segP, 0).centerMask)
			return nNewSeg;
	}
else {
	for (nNewSeg = 0; nNewSeg <= gameData.segs.nLastSegment; nNewSeg++)
		if ((gameData.segs.segment2s [nNewSeg].special != SEGMENT_IS_SKYBOX) && !GetSegMasks (p, nNewSeg, 0).centerMask)
			return nNewSeg;
	}
++nExhaustiveFailedCount;
#if TRACE
con_printf (1, "Warning: could not find point tSegment (%i times)\n", nExhaustiveFailedCount);
#endif
return -1;		//no tSegment found
}


//--repair-- //	------------------------------------------------------------------------------
//--repair-- void clsd_repair_center (int nSegment)
//--repair-- {
//--repair-- 	int	nSide;
//--repair--
//--repair-- 	//	--- Set repair center bit for all repair center segments.
//--repair-- 	if (gameData.segs.segments [nSegment].special == SEGMENT_IS_REPAIRCEN) {
//--repair-- 		Lsegments [nSegment].specialType |= SS_REPAIR_CENTER;
//--repair-- 		Lsegments [nSegment].special_segment = nSegment;
//--repair-- 	}
//--repair--
//--repair-- 	//	--- Set repair center bit for all segments adjacent to a repair center.
//--repair-- 	for (nSide=0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
//--repair-- 		int	s = gameData.segs.segments [nSegment].children [nSide];
//--repair--
//--repair-- 		if ((s != -1) && (gameData.segs.segments [s].special == SEGMENT_IS_REPAIRCEN)) {
//--repair-- 			Lsegments [nSegment].specialType |= SS_REPAIR_CENTER;
//--repair-- 			Lsegments [nSegment].special_segment = s;
//--repair-- 		}
//--repair-- 	}
//--repair-- }

//--repair-- //	------------------------------------------------------------------------------
//--repair-- //	--- Set destination points for all Materialization centers.
//--repair-- void clsd_materialization_center (int nSegment)
//--repair-- {
//--repair-- 	if (gameData.segs.segments [nSegment].special == SEGMENT_IS_ROBOTMAKER) {
//--repair--
//--repair-- 	}
//--repair-- }
//--repair--
//--repair-- int	Lsegment_highest_segment_index, Lsegment_highest_vertex_index;
//--repair--
//--repair-- //	------------------------------------------------------------------------------
//--repair-- //	Create data specific to mine which doesn't get written to disk.
//--repair-- //	gameData.segs.nLastSegment and gameData.objs.nLastObject must be valid.
//--repair-- //	07/21:	set repair center bit
//--repair-- void create_local_segment_data (void)
//--repair-- {
//--repair-- 	int	nSegment;
//--repair--
//--repair-- 	//	--- Initialize all Lsegments.
//--repair-- 	for (nSegment=0; nSegment <= gameData.segs.nLastSegment; nSegment++) {
//--repair-- 		Lsegments [nSegment].specialType = 0;
//--repair-- 		Lsegments [nSegment].special_segment = -1;
//--repair-- 	}
//--repair--
//--repair-- 	for (nSegment=0; nSegment <= gameData.segs.nLastSegment; nSegment++) {
//--repair--
//--repair-- 		clsd_repair_center (nSegment);
//--repair-- 		clsd_materialization_center (nSegment);
//--repair-- 
//--repair-- 	}
//--repair--
//--repair-- 	//	Set check variables.
//--repair-- 	//	In main game loop, make sure these are valid, else Lsegments is not valid.
//--repair-- 	Lsegment_highest_segment_index = gameData.segs.nLastSegment;
//--repair-- 	Lsegment_highest_vertex_index = gameData.segs.nLastVertex;
//--repair-- }
//--repair--
//--repair-- //	------------------------------------------------------------------------------------------
//--repair-- //	Sort of makes sure create_local_segment_data has been called for the currently executing mine.
//--repair-- //	It is not failsafe, as you will see if you look at the code.
//--repair-- //	Returns 1 if Lsegments appears valid, 0 if not.
//--repair-- int check_lsegments_validity (void)
//--repair-- {
//--repair-- 	return ((Lsegment_highest_segment_index == gameData.segs.nLastSegment) && (Lsegment_highest_vertex_index == gameData.segs.nLastVertex);
//--repair-- }

#define	MAX_LOC_POINT_SEGS	64

#define	MIN_CACHE_FCD_DIST	 (F1_0*80)	//	Must be this far apart for cache lookup to succeed.  Recognizes small changes in distance matter at small distances.
//	----------------------------------------------------------------------------------------------------------

void FlushFCDCache (void)
{
	int	i;

gameData.fcd.nIndex = 0;
for (i = 0; i < MAX_FCD_CACHE; i++)
	gameData.fcd.cache [i].seg0 = -1;
}

//	----------------------------------------------------------------------------------------------------------

void AddToFCDCache (int seg0, int seg1, int nDepth, fix dist)
{
	if (dist > MIN_CACHE_FCD_DIST) {
		gameData.fcd.cache [gameData.fcd.nIndex].seg0 = seg0;
		gameData.fcd.cache [gameData.fcd.nIndex].seg1 = seg1;
		gameData.fcd.cache [gameData.fcd.nIndex].csd = nDepth;
		gameData.fcd.cache [gameData.fcd.nIndex].dist = dist;

		gameData.fcd.nIndex++;

		if (gameData.fcd.nIndex >= MAX_FCD_CACHE)
			gameData.fcd.nIndex = 0;

	} else {
		//	If it's in the cache, remove it.
		int	i;

		for (i=0; i<MAX_FCD_CACHE; i++)
			if (gameData.fcd.cache [i].seg0 == seg0)
				if (gameData.fcd.cache [i].seg1 == seg1) {
					gameData.fcd.cache [gameData.fcd.nIndex].seg0 = -1;
					break;
				}
	}

}

//	----------------------------------------------------------------------------------------------------------
//	Determine whether seg0 and seg1 are reachable in a way that allows sound to pass.
//	Search up to a maximum nDepth of nMaxDepth.
//	Return the distance.
fix FindConnectedDistance (vmsVector *p0, short seg0, vmsVector *p1, short seg1, int nMaxDepth, int widFlag, int bUseCache)
{
	short				nConnSide;
	short				nCurSeg, nParentSeg, nThisSeg;
	short				nSide;
	int				qTail = 0, qHead = 0;
	int				i, nCurDepth, nPoints;
	sbyte				visited [MAX_SEGMENTS_D2X];
	segQueueEntry	segmentQ [MAX_SEGMENTS_D2X];
	short				nDepth [MAX_SEGMENTS_D2X];
	tPointSeg		pointSegs [MAX_LOC_POINT_SEGS];
	fix				dist;
	tSegment			*segP;
	tFCDCacheData	*pc;

	//	If > this, will overrun pointSegs buffer
if (nMaxDepth > MAX_LOC_POINT_SEGS-2) {
#if TRACE	
	con_printf (1, "Warning: In FindConnectedDistance, nMaxDepth = %i, limited to %i\n", nMaxDepth, MAX_LOC_POINT_SEGS-2);
#endif	
	nMaxDepth = MAX_LOC_POINT_SEGS - 2;
	}
if (seg0 == seg1) {
	gameData.fcd.nConnSegDist = 0;
	return VmVecDistQuick (p0, p1);
	}
nConnSide = FindConnectedSide (gameData.segs.segments + seg0, gameData.segs.segments + seg1);
if ((nConnSide != -1) &&
	 (WALL_IS_DOORWAY (gameData.segs.segments + seg1, nConnSide, NULL) & widFlag)) {
	gameData.fcd.nConnSegDist = 1;
	return VmVecDistQuick (p0, p1);
	}
//	Periodically flush cache.
if ((gameData.time.xGame - gameData.fcd.xLastFlushTime > F1_0*2) || 
	 (gameData.time.xGame < gameData.fcd.xLastFlushTime)) {
	FlushFCDCache ();
	gameData.fcd.xLastFlushTime = gameData.time.xGame;
	}

//	Can't quickly get distance, so see if in gameData.fcd.cache.
if (bUseCache) {
	for (i = MAX_FCD_CACHE, pc = gameData.fcd.cache; i; i--, pc++)
		if ((pc->seg0 == seg0) && (pc->seg1 == seg1)) {
			gameData.fcd.nConnSegDist = pc->csd;
			return pc->dist;
			}
	}
memset (visited, 0, gameData.segs.nLastSegment + 1);
memset (nDepth, 0, sizeof (nDepth [0]) * (gameData.segs.nLastSegment + 1));

nPoints = 0;
nCurSeg = seg0;
visited [nCurSeg] = 1;
nCurDepth = 0;

while (nCurSeg != seg1) {
	segP = gameData.segs.segments + nCurSeg;

	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (WALL_IS_DOORWAY (segP, nSide, NULL) & widFlag) {
			nThisSeg = segP->children [nSide];
			Assert ((nThisSeg >= 0) && (nThisSeg < MAX_SEGMENTS));
			Assert ((qTail >= 0) && (qTail < MAX_SEGMENTS - 1));
			if (!visited [nThisSeg]) {
				segmentQ [qTail].start = nCurSeg;
				segmentQ [qTail].end = nThisSeg;
				visited [nThisSeg] = 1;
				nDepth [qTail++] = nCurDepth+1;
				if (nMaxDepth != -1) {
					if (nDepth [qTail - 1] == nMaxDepth) {
						gameData.fcd.nConnSegDist = 1000;
						AddToFCDCache (seg0, seg1, gameData.fcd.nConnSegDist, F1_0*1000);
						return -1;
						}
					}
				else if (nThisSeg == seg1) {
					goto fcd_done1;
				}
			}
		}
	}	//	for (nSide...

	if (qHead >= qTail) {
		gameData.fcd.nConnSegDist = 1000;
		AddToFCDCache (seg0, seg1, gameData.fcd.nConnSegDist, F1_0*1000);
		return -1;
		}
	Assert ((qHead >= 0) && (qHead < MAX_SEGMENTS));
	nCurSeg = segmentQ [qHead].end;
	nCurDepth = nDepth [qHead];
	qHead++;

fcd_done1: ;
	}	//	while (nCurSeg ...

//	Set qTail to the tSegment which ends at the goal.
while (segmentQ [--qTail].end != seg1)
	if (qTail < 0) {
		gameData.fcd.nConnSegDist = 1000;
		AddToFCDCache (seg0, seg1, gameData.fcd.nConnSegDist, F1_0*1000);
		return -1;
		}

while (qTail >= 0) {
	nThisSeg = segmentQ [qTail].end;
	nParentSeg = segmentQ [qTail].start;
	pointSegs [nPoints].nSegment = nThisSeg;
	COMPUTE_SEGMENT_CENTER_I (&pointSegs [nPoints].point, nThisSeg);
	nPoints++;
	if (nParentSeg == seg0)
		break;
	while (segmentQ [--qTail].end != nParentSeg)
		Assert (qTail >= 0);
	}
pointSegs [nPoints].nSegment = seg0;
COMPUTE_SEGMENT_CENTER_I (&pointSegs [nPoints].point, seg0);
nPoints++;
if (nPoints == 1) {
	gameData.fcd.nConnSegDist = nPoints;
	return VmVecDistQuick (p0, p1);
	} 
else {
	fix	ndist;
	dist = VmVecDistQuick (p1, &pointSegs [1].point);
	dist += VmVecDistQuick (p0, &pointSegs [nPoints-2].point);
	for (i = 1; i < nPoints - 2; i++) {
		ndist = VmVecDistQuick (&pointSegs [i].point, &pointSegs [i+1].point);
		dist += ndist;
		}
	}
gameData.fcd.nConnSegDist = nPoints;
AddToFCDCache (seg0, seg1, nPoints, dist);
return dist;
}

// -------------------------------------------------------------------------------

sbyte convert_to_byte (fix f)
{
	if (f >= 0x00010000)
		return MATRIX_MAX;
	else if (f <= -0x00010000)
		return -MATRIX_MAX;
	else
		return (sbyte) (f >> MATRIX_PRECISION);
}

#define VEL_PRECISION 12

// -------------------------------------------------------------------------------
//	Create a tShortPos struct from an tObject.
//	Extract the matrix into byte values.
//	Create a position relative to vertex 0 with 1/256 normal "fix" precision.
//	Stuff tSegment in a short.
void CreateShortPos (tShortPos *spp, tObject *objP, int swap_bytes)
{
	// int	nSegment;
	vmsMatrix orient = objP->position.mOrient;
	sbyte   *segP = spp->bytemat;
	vmsVector *pv;

	*segP++ = convert_to_byte (orient.rVec.p.x);
	*segP++ = convert_to_byte (orient.uVec.p.x);
	*segP++ = convert_to_byte (orient.fVec.p.x);
	*segP++ = convert_to_byte (orient.rVec.p.y);
	*segP++ = convert_to_byte (orient.uVec.p.y);
	*segP++ = convert_to_byte (orient.fVec.p.y);
	*segP++ = convert_to_byte (orient.rVec.p.z);
	*segP++ = convert_to_byte (orient.uVec.p.z);
	*segP++ = convert_to_byte (orient.fVec.p.z);

	pv = gameData.segs.vertices + gameData.segs.segments [objP->nSegment].verts [0];
	spp->xo = (short) ((objP->position.vPos.p.x - pv->p.x) >> RELPOS_PRECISION);
	spp->yo = (short) ((objP->position.vPos.p.y - pv->p.y) >> RELPOS_PRECISION);
	spp->zo = (short) ((objP->position.vPos.p.z - pv->p.z) >> RELPOS_PRECISION);

	spp->nSegment = objP->nSegment;

 	spp->velx = (short) ((objP->mType.physInfo.velocity.p.x) >> VEL_PRECISION);
	spp->vely = (short) ((objP->mType.physInfo.velocity.p.y) >> VEL_PRECISION);
	spp->velz = (short) ((objP->mType.physInfo.velocity.p.z) >> VEL_PRECISION);

// swap the short values for the big-endian machines.

	if (swap_bytes) {
		spp->xo = INTEL_SHORT (spp->xo);
		spp->yo = INTEL_SHORT (spp->yo);
		spp->zo = INTEL_SHORT (spp->zo);
		spp->nSegment = INTEL_SHORT (spp->nSegment);
		spp->velx = INTEL_SHORT (spp->velx);
		spp->vely = INTEL_SHORT (spp->vely);
		spp->velz = INTEL_SHORT (spp->velz);
	}
}

// -------------------------------------------------------------------------------

void ExtractShortPos (tObject *objP, tShortPos *spp, int swap_bytes)
{
	int	nSegment;
	sbyte   *segP;
	vmsVector *pv;

	segP = spp->bytemat;

	objP->position.mOrient.rVec.p.x = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.uVec.p.x = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.fVec.p.x = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.rVec.p.y = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.uVec.p.y = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.fVec.p.y = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.rVec.p.z = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.uVec.p.z = *segP++ << MATRIX_PRECISION;
	objP->position.mOrient.fVec.p.z = *segP++ << MATRIX_PRECISION;

	if (swap_bytes) {
		spp->xo = INTEL_SHORT (spp->xo);
		spp->yo = INTEL_SHORT (spp->yo);
		spp->zo = INTEL_SHORT (spp->zo);
		spp->nSegment = INTEL_SHORT (spp->nSegment);
		spp->velx = INTEL_SHORT (spp->velx);
		spp->vely = INTEL_SHORT (spp->vely);
		spp->velz = INTEL_SHORT (spp->velz);
	}

	nSegment = spp->nSegment;

	Assert ((nSegment >= 0) && (nSegment <= gameData.segs.nLastSegment));

	pv = gameData.segs.vertices + gameData.segs.segments [nSegment].verts [0];
	objP->position.vPos.p.x = (spp->xo << RELPOS_PRECISION) + pv->p.x;
	objP->position.vPos.p.y = (spp->yo << RELPOS_PRECISION) + pv->p.y;
	objP->position.vPos.p.z = (spp->zo << RELPOS_PRECISION) + pv->p.z;

	objP->mType.physInfo.velocity.p.x = (spp->velx << VEL_PRECISION);
	objP->mType.physInfo.velocity.p.y = (spp->vely << VEL_PRECISION);
	objP->mType.physInfo.velocity.p.z = (spp->velz << VEL_PRECISION);

	RelinkObject (OBJ_IDX (objP), nSegment);

}

//--unused-- void test_shortpos (void)
//--unused-- {
//--unused-- 	tShortPos	spp;
//--unused--
//--unused-- 	CreateShortPos (&spp, &gameData.objs.objects [0]);
//--unused-- 	ExtractShortPos (&gameData.objs.objects [0], &spp);
//--unused--
//--unused-- }

//	-----------------------------------------------------------------------------
//	Segment validation functions.
//	Moved from editor to game so we can compute surface normals at load time.
// -------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------
//	Extract a vector from a tSegment.  The vector goes from the start face to the end face.
//	The point on each face is the average of the four points forming the face.
void extract_vector_from_segment (tSegment *segP, vmsVector *vp, int start, int end)
{
	int			i;
	vmsVector	vs, ve;

	VmVecZero (&vs);
	VmVecZero (&ve);

	for (i=0; i<4; i++) {
		VmVecInc (&vs, &gameData.segs.vertices [segP->verts [sideToVerts [start][i]]]);
		VmVecInc (&ve, &gameData.segs.vertices [segP->verts [sideToVerts [end][i]]]);
	}

	VmVecSub (vp, &ve, &vs);
	VmVecScale (vp, F1_0/4);

}

// -------------------------------------------------------------------------------
//create a matrix that describes the orientation of the given tSegment
void ExtractOrientFromSegment (vmsMatrix *m, tSegment *seg)
{
	vmsVector fVec, uVec;

	extract_vector_from_segment (seg, &fVec, WFRONT, WBACK);
	extract_vector_from_segment (seg, &uVec, WBOTTOM, WTOP);

	//vector to matrix does normalizations and orthogonalizations
	VmVector2Matrix (m, &fVec, &uVec, NULL);
}

#ifdef EDITOR
// ------------------------------------------------------------------------------------------
//	Extract the forward vector from tSegment *segP, return in *vp.
//	The forward vector is defined to be the vector from the the center of the front face of the tSegment
// to the center of the back face of the tSegment.
void extract_forward_vector_from_segment (tSegment *segP, vmsVector *vp)
{
	extract_vector_from_segment (segP, vp, WFRONT, WBACK);
}

// ------------------------------------------------------------------------------------------
//	Extract the right vector from tSegment *segP, return in *vp.
//	The forward vector is defined to be the vector from the the center of the left face of the tSegment
// to the center of the right face of the tSegment.
void extract_right_vector_from_segment (tSegment *segP, vmsVector *vp)
{
	extract_vector_from_segment (segP, vp, WLEFT, WRIGHT);
}

// ------------------------------------------------------------------------------------------
//	Extract the up vector from tSegment *segP, return in *vp.
//	The forward vector is defined to be the vector from the the center of the bottom face of the tSegment
// to the center of the top face of the tSegment.
void extract_up_vector_from_segment (tSegment *segP, vmsVector *vp)
{
	extract_vector_from_segment (segP, vp, WBOTTOM, WTOP);
}
#endif

// -------------------------------------------------------------------------------

void AddSideAsQuad (tSegment *segP, int nSide, vmsVector *normal)
{
	tSide	*sideP = segP->sides + nSide;

	sideP->nType = SIDE_IS_QUAD;
	#ifdef COMPACT_SEGS
		normal = normal;		//avoid compiler warning
	#else
	sideP->normals [0] = *normal;
	sideP->normals [1] = *normal;
	#endif

	//	If there is a connection here, we only formed the faces for the purpose of determining tSegment boundaries, 
	//	so don't generate polys, else they will get rendered.
//	if (segP->children [nSide] != -1)
//		sideP->renderFlag = 0;
//	else
//		sideP->renderFlag = 1;

}


// -------------------------------------------------------------------------------
//	Return v0, v1, v2 = 3 vertices with smallest numbers.  If *bFlip set, then negate normal after computation.
//	Note, you cannot just compute the normal by treating the points in the opposite direction as this introduces
//	small differences between normals which should merely be opposites of each other.
int GetVertsForNormal (int v0, int v1, int v2, int v3, int *pv0, int *pv1, int *pv2, int *pv3)
{
	int	i, j, t;
	int	v [4], w [4] = {0, 1, 2, 3};

//	w is a list that shows how things got scrambled so we know if our normal is pointing backwards

v [0] = v0;
v [1] = v1;
v [2] = v2;
v [3] = v3;
// bubble sort v in reverse order (largest first)
for (i = 1; i < 4; i++)
	for (j = 0; j < i; j++)
		if (v [j] > v [i]) {
			t = v [j]; v [j] = v [i]; v [i] = t;
			t = w [j]; w [j] = w [i]; w [i] = t;
			}

Assert ((v [0] < v [1]) && (v [1] < v [2]) && (v [2] < v [3]));
*pv0 = v [0];
*pv1 = v [1];
*pv2 = v [2];
*pv3 = v [3];
//	Now, if for any w [i] & w [i+1]: w [i+1] = (w [i]+3)%4, then must flip normal
return ((((w [0] + 3) % 4) == w [1]) || (((w [1] + 3) % 4) == w [2]));
}

// -------------------------------------------------------------------------------

void AddSideAsTwoTriangles (tSegment *segP, int nSide)
{
	vmsVector	vNormal;
	sbyte       *vs = sideToVerts [nSide];
	short			v0 = segP->verts [vs [0]];
	short			v1 = segP->verts [vs [1]];
	short			v2 = segP->verts [vs [2]];
	short			v3 = segP->verts [vs [3]];
	fix			dot;
	vmsVector	vec_13;		//	vector from vertex 1 to vertex 3

	tSide	*sideP = segP->sides + nSide;

	//	Choose how to triangulate.
	//	If a tWall, then
	//		Always triangulate so tSegment is convex.
	//		Use Matt's formula: Na . AD > 0, where ABCD are vertices on tSide, a is face formed by A, B, C, Na is normal from face a.
	//	If not a tWall, then triangulate so whatever is on the other tSide is triangulated the same (ie, between the same absoluate vertices)
#ifdef _DEBUG
if ((SEG_IDX (segP) == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
	segP = segP;
#endif
if (!IS_CHILD (segP->children [nSide])) {
	VmVecNormalChecked (&vNormal, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v2);
	VmVecSub (&vec_13, gameData.segs.vertices + v3, gameData.segs.vertices + v1);
	dot = VmVecDot (&vNormal, &vec_13);

	//	Now, signify whether to triangulate from 0:2 or 1:3
	if (dot >= 0)
		sideP->nType = SIDE_IS_TRI_02;
	else
		sideP->nType = SIDE_IS_TRI_13;

#	ifndef COMPACT_SEGS
	//	Now, based on triangulation nType, set the normals.
	if (sideP->nType == SIDE_IS_TRI_02) {
		//VmVecNormalChecked (&vNormal, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v2);
		sideP->normals [0] = vNormal;
		VmVecNormalChecked (&vNormal, gameData.segs.vertices + v0, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
		sideP->normals [1] = vNormal;
		}
	else {
		VmVecNormalChecked (&vNormal, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v3);
		sideP->normals [0] = vNormal;
		VmVecNormalChecked (&vNormal, gameData.segs.vertices + v1, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
		sideP->normals [1] = vNormal;
		}
#	endif
	}
else {
	int	vSorted [4];
	int	bFlip;

	bFlip = GetVertsForNormal (v0, v1, v2, v3, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
	if ((vSorted [0] == v0) || (vSorted [0] == v2)) {
		sideP->nType = SIDE_IS_TRI_02;
#	ifndef COMPACT_SEGS
		//	Now, get vertices for normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (v0, v1, v2, 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormalChecked (sideP->normals, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (sideP->normals);
		bFlip = GetVertsForNormal (v0, v2, v3, 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormalChecked (sideP->normals + 1, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (sideP->normals + 1);
		GetVertsForNormal (v0, v2, v3, 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
#	endif
		}
	else {
		sideP->nType = SIDE_IS_TRI_13;
#	ifndef COMPACT_SEGS
		//	Now, get vertices for normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (v0, v1, v3, 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormalChecked (sideP->normals, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (sideP->normals);
		bFlip = GetVertsForNormal (v1, v2, v3, 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormalChecked (sideP->normals + 1, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (sideP->normals + 1);
#	endif
		}
	}
}

// -------------------------------------------------------------------------------

int sign (fix v)
{
if (v > PLANE_DIST_TOLERANCE)
	return 1;
if (v < - (PLANE_DIST_TOLERANCE+1))		//neg & pos round differently
	return -1;
return 0;
}

// -------------------------------------------------------------------------------

void AddToVertexNormal (int nVertex, vmsVector *pvNormal)
{
	g3sNormal	*pn = &gameData.segs.points [nVertex].p3_normal;

#ifdef _DEBUG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
pn->nFaces++;
pn->vNormal.p.x += f2fl (pvNormal->p.x);
pn->vNormal.p.y += f2fl (pvNormal->p.y);
pn->vNormal.p.z += f2fl (pvNormal->p.z);
}

// -------------------------------------------------------------------------------

int bRenderQuads = 0;

void CreateWallsOnSide (tSegment *segP, int nSide)
{
	int			vm0, vm1, vm2, vm3, bFlip;
	int			v0, v1, v2, v3, i;
	int			vertexList [6];
	vmsVector	vn;
	fix			xDistToPlane;
	sbyte			*s2v = sideToVerts [nSide];

	v0 = segP->verts [s2v [0]];
	v1 = segP->verts [s2v [1]];
	v2 = segP->verts [s2v [2]];
	v3 = segP->verts [s2v [3]];
	bFlip = GetVertsForNormal (v0, v1, v2, v3, &vm0, &vm1, &vm2, &vm3);
	VmVecNormalChecked (&vn, gameData.segs.vertices + vm0, gameData.segs.vertices + vm1, gameData.segs.vertices + vm2);
	xDistToPlane = abs (VmDistToPlane (gameData.segs.vertices + vm3, &vn, gameData.segs.vertices + vm0));
	if (bFlip)
		VmVecNegate (&vn);
#if 1
	if (bRenderQuads || (xDistToPlane <= PLANE_DIST_TOLERANCE))
		AddSideAsQuad (segP, nSide, &vn);
	else {
		AddSideAsTwoTriangles (segP, nSide);
		//this code checks to see if we really should be triangulated, and
		//de-triangulates if we shouldn't be.
		{
			int			nFaces;
			fix			dist0, dist1;
			int			s0, s1;
			int			nVertex;
			tSide			*s;

			nFaces = CreateAbsVertexLists (vertexList, SEG_IDX (segP), nSide);
#ifdef _DEBUG
			if (nFaces != 2)
				nFaces = CreateAbsVertexLists (vertexList, SEG_IDX (segP), nSide);
#endif
			Assert (nFaces == 2);
			s = segP->sides + nSide;
			nVertex = min (vertexList [0], vertexList [2]);
#ifdef COMPACT_SEGS
			{
			vmsVector normals [2];
			GetSideNormals (segP, nSide, &normals [0], &normals [1]);
			dist0 = VmDistToPlane (gameData.segs.vertices + vertexList [1], normals + 1, gameData.segs.vertices + nVertex);
			dist1 = VmDistToPlane (gameData.segs.vertices + vertexList [4], normals, gameData.segs.vertices + nVertex);
			}
#else
			{
#	ifdef _DEBUG
			vmsVector normals [2];
			GetSideNormals (segP, nSide, &normals [0], &normals [1]);
#	endif
			dist0 = VmDistToPlane (gameData.segs.vertices + vertexList [1], s->normals + 1, gameData.segs.vertices + nVertex);
			dist1 = VmDistToPlane (gameData.segs.vertices + vertexList [4], s->normals, gameData.segs.vertices + nVertex);
			}
#endif
			s0 = sign (dist0);
			s1 = sign (dist1);
			if (s0 == 0 || s1 == 0 || s0 != s1) {
				segP->sides [nSide].nType = SIDE_IS_QUAD; 	//detriangulate!
#ifndef COMPACT_SEGS
				segP->sides [nSide].normals [0] =
				segP->sides [nSide].normals [1] = vn;
#endif
			}
		}
	}
#endif
if (segP->sides [nSide].nType == SIDE_IS_QUAD) {
	AddToVertexNormal (v0, &vn);
	AddToVertexNormal (v1, &vn);
	AddToVertexNormal (v2, &vn);
	AddToVertexNormal (v3, &vn);
	}
else {
	vn = segP->sides [nSide].normals [0];
	for (i = 0; i < 3; i++) 
		AddToVertexNormal (vertexList [i], &vn);
	vn = segP->sides [nSide].normals [1];
	for (; i < 6; i++) 
		AddToVertexNormal (vertexList [i], &vn);
	}
}

// -------------------------------------------------------------------------------

void ValidateRemovableWall (tSegment *segP, int nSide, int nTexture)
{
CreateWallsOnSide (segP, nSide);
segP->sides [nSide].nBaseTex = nTexture;
//	assign_default_uvs_to_side (segP, nSide);
//	assign_light_to_side (segP, nSide);
}

// -------------------------------------------------------------------------------
//	Make a just-modified tSegment tSide valid.
void ValidateSegmentSide (tSegment *segP, short nSide)
{
if (IS_WALL (WallNumP (segP, nSide)))
	ValidateRemovableWall (segP, nSide, segP->sides [nSide].nBaseTex);
else
	CreateWallsOnSide (segP, nSide);
}

extern int check_for_degenerate_segment (tSegment *segP);

// -------------------------------------------------------------------------------

void ComputeVertexNormals (void)
{
	int		h, i;
	g3sPoint	*pp;

for (i = gameData.segs.nVertices, pp = gameData.segs.points; i; i--, pp++) {
	if (1 < (h = pp->p3_normal.nFaces)) {
		pp->p3_normal.vNormal.p.x /= h;
		pp->p3_normal.vNormal.p.y /= h;
		pp->p3_normal.vNormal.p.z /= h;
		VmVecNormalize (&pp->p3_normal.vNormal, &pp->p3_normal.vNormal);
		}
	pp->p3_normal.nFaces = 1;
	}
}

// -------------------------------------------------------------------------------

void ResetVertexNormals (void)
{
	int		i;
	g3sPoint	*pp;

for (i = gameData.segs.nVertices, pp = gameData.segs.points; i; i--, pp++) {
	pp->p3_normal.vNormal.p.x =
	pp->p3_normal.vNormal.p.y =
	pp->p3_normal.vNormal.p.z = 0;
	pp->p3_normal.nFaces = 0;
	}
}

// -------------------------------------------------------------------------------
//	Make a just-modified tSegment valid.
//		check all sides to see how many faces they each should have (0, 1, 2)
//		create new vector normals
void ValidateSegment (tSegment *segP)
{
	short	tSide;

#ifdef EDITOR
check_for_degenerate_segment (segP);
#endif
for (tSide = 0; tSide < MAX_SIDES_PER_SEGMENT; tSide++)
	ValidateSegmentSide (segP, tSide);
}

// -------------------------------------------------------------------------------
//	Validate all segments.
//	gameData.segs.nLastSegment must be set.
//	For all used segments (number <= gameData.segs.nLastSegment), nSegment field must be != -1.

void ValidateSegmentAll (void)
{
	int	s;

gameOpts->render.nMathFormat = 0;
memset (gameData.segs.points, 0, sizeof (*gameData.segs.points) * MAX_VERTICES);
for (s = 0; s <= gameData.segs.nLastSegment; s++)
#ifdef EDITOR
	if (gameData.segs.segments [s].nSegment != -1)
#endif
		ValidateSegment (gameData.segs.segments + s);
#ifdef EDITOR
	{
	int said = 0;
	for (s = gameData.segs.nLastSegment + 1; s < MAX_SEGMENTS; s++)
		if (gameData.segs.segments [s].nSegment != -1) {
			if (!said) {
#if TRACE	
				con_printf (CONDBG, "Segment %i has invalid nSegment.  Bashing to -1.  Silently bashing all others...", s);
#endif
				}
			said++;
			gameData.segs.segments [s].nSegment = -1;
			}
	if (said) {
#if TRACE	
		con_printf (CONDBG, "%i fixed.\n", said);
#endif
		}
	}
#endif

#ifdef _DEBUG
#	ifndef COMPACT_SEGS
if (CheckSegmentConnections ())
	Int3 ();		//Get Matt, si vous plait.
#	endif
#endif
ComputeVertexNormals ();
gameOpts->render.nMathFormat = gameOpts->render.nDefMathFormat;
}


//	------------------------------------------------------------------------------------------------------
//	Picks a random point in a tSegment like so:
//		From center, go up to 50% of way towards any of the 8 vertices.
void PickRandomPointInSeg (vmsVector *new_pos, int nSegment)
{
	int			vnum;
	vmsVector	vec2;

	COMPUTE_SEGMENT_CENTER_I (new_pos, nSegment);
	vnum = (d_rand () * MAX_VERTICES_PER_SEGMENT) >> 15;
	VmVecSub (&vec2, &gameData.segs.vertices [gameData.segs.segments [nSegment].verts [vnum]], new_pos);
	VmVecScale (&vec2, d_rand ());          // d_rand () always in 0..1/2
	VmVecInc (new_pos, &vec2);
}


//	----------------------------------------------------------------------------------------------------------
//	Set the segment depth of all segments from nStartSeg in *segbuf.
//	Returns maximum nDepth value.
int SetSegmentDepths (int nStartSeg, ushort *pDepthBuf)
{
	ubyte		bVisited [MAX_SEGMENTS_D2X];
	short		queue [MAX_SEGMENTS_D2X];
	int		head = 0;
	int		tail = 0;
	int		nDepth = 1;
	int		nSegment, nSide, nChild;
	ushort	nParentDepth = 0;
	short		*childP;

	head = 0;
	tail = 0;

if ((nStartSeg < 0) || (nStartSeg >= gameData.segs.nSegments))
	return 1;
if (pDepthBuf [nStartSeg] == 0)
	return 1;
queue [tail++] = nStartSeg;
memset (bVisited, 0, sizeof (*bVisited) * gameData.segs.nSegments);
bVisited [nStartSeg] = 1;
pDepthBuf [nStartSeg] = nDepth++;
if (nDepth == 0)
	nDepth = 0x7fff;
while (head < tail) {
	nSegment = queue [head++];
#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	nParentDepth = pDepthBuf [nSegment];
	childP = gameData.segs.segments [nSegment].children;
	for (nSide = MAX_SIDES_PER_SEGMENT; nSide; nSide--, childP++) {
		if (0 > (nChild = *childP))
			continue;
#ifdef _DEBUG
		if (nChild >= gameData.segs.nSegments) {
			Error ("Invalid segment in SetSegmentDepths()\nsegment=%d, side=%d, child=%d",
					 nSegment, nSide, nChild);
			return 1;
			}
#endif
#ifdef _DEBUG
		if (nChild == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		if (!pDepthBuf [nChild])
			continue;
		if (bVisited [nChild])
			continue;
		bVisited [nChild] = 1;
		pDepthBuf [nChild] = nParentDepth + 1;
		queue [tail++] = nChild;
		}
	}
return (nParentDepth + 1) * gameStates.render.bViewDist;
}

//	-----------------------------------------------------------------------------

fix FindConnectedDistanceSegments (short seg0, short seg1, int nDepth, int widFlag)
{
	vmsVector	p0, p1;

COMPUTE_SEGMENT_CENTER_I (&p0, seg0);
COMPUTE_SEGMENT_CENTER_I (&p1, seg1);
return FindConnectedDistance (&p0, seg0, &p1, seg1, nDepth, widFlag, 0);
}

#define	AMBIENT_SEGMENT_DEPTH		5

//	-----------------------------------------------------------------------------
//	Do a bfs from nSegment, marking slots in marked_segs if the tSegment is reachable.
void AmbientMarkBfs (short nSegment, sbyte *marked_segs, int nDepth)
{
	short	i, child;

if (nDepth < 0)
	return;
marked_segs [nSegment] = 1;
for (i=0; i<MAX_SIDES_PER_SEGMENT; i++) {
	child = gameData.segs.segments [nSegment].children [i];
	if (IS_CHILD (child) && 
	    (WALL_IS_DOORWAY (gameData.segs.segments + nSegment, i, NULL) & WID_RENDPAST_FLAG) && 
		 !marked_segs [child])
		AmbientMarkBfs (child, marked_segs, nDepth-1);
	}
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava, 
//	and so should hear ambient gurgles.
void SetAmbientSoundFlagsCommon (int tmi_bit, int s2f_bit)
{
	short		i, j;
	tSegment2	*seg2p;
	static sbyte   marked_segs [MAX_SEGMENTS_D2X];

	//	Now, all segments containing ambient lava or water sound makers are flagged.
	//	Additionally flag all segments which are within range of them.
for (i=0; i<=gameData.segs.nLastSegment; i++) {
	marked_segs [i] = 0;
	gameData.segs.segment2s [i].s2Flags &= ~s2f_bit;
	}

//	Mark all segments which are sources of the sound.
for (i=0; i<=gameData.segs.nLastSegment; i++) {
	tSegment	*segp = &gameData.segs.segments [i];
	tSegment2	*seg2p = &gameData.segs.segment2s [i];

	for (j=0; j<MAX_SIDES_PER_SEGMENT; j++) {
		tSide	*sideP = &segp->sides [j];

		if ((gameData.pig.tex.pTMapInfo [sideP->nBaseTex].flags & tmi_bit) || 
			   (gameData.pig.tex.pTMapInfo [sideP->nOvlTex].flags & tmi_bit)) {
			if (!IS_CHILD (segp->children [j]) || IS_WALL (sideP->nWall)) {
				seg2p->s2Flags |= s2f_bit;
				marked_segs [i] = 1;		//	Say it's itself that it is close enough to to hear something.
				}
			}
		}
	}
//	Next mark all segments within N segments of a source.
for (i=0; i<=gameData.segs.nLastSegment; i++) {
	seg2p = &gameData.segs.segment2s [i];
	if (seg2p->s2Flags & s2f_bit)
		AmbientMarkBfs (i, marked_segs, AMBIENT_SEGMENT_DEPTH);
	}
//	Now, flip bits in all segments which can hear the ambient sound.
for (i=0; i<=gameData.segs.nLastSegment; i++)
	if (marked_segs [i])
		gameData.segs.segment2s [i].s2Flags |= s2f_bit;
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava, 
//	and so should hear ambient gurgles.
//	Bashes values in gameData.segs.segment2s array.
void SetAmbientSoundFlags (void)
{
SetAmbientSoundFlagsCommon (TMI_VOLATILE, S2F_AMBIENT_LAVA);
SetAmbientSoundFlagsCommon (TMI_WATER, S2F_AMBIENT_WATER);
}

//	-----------------------------------------------------------------------------
//eof
