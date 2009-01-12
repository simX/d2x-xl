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
#include "error.h"
#include "mono.h"
#include "gameseg.h"
#include "byteswap.h"
#include "light.h"
#include "segment.h"
#include "renderlib.h"
#include "fvi.h"

// How far a point can be from a plane, and still be "in" the plane

extern bool bNewFileFormat;

// ------------------------------------------------------------------------------------------

inline CFixVector& CSide::Normal (int nFace)
{ 
return gameStates.render.bRendering ? m_rotNorms [nFace] : m_normals [nFace]; 
}

// ------------------------------------------------------------------------------------------
// Compute the center point of a CSide of a CSegment.
//	The center point is defined to be the average of the 4 points defining the CSide.
void CSide::ComputeCenter (void)
{
m_vCenter = gameData.segs.vertices [m_vertices [0]];
m_vCenter += gameData.segs.vertices [m_vertices [1]];
m_vCenter += gameData.segs.vertices [m_vertices [2]];
m_vCenter += gameData.segs.vertices [m_vertices [3]];
m_vCenter [X] /= 4;
m_vCenter [Y] /= 4;
m_vCenter [Z] /= 4;
}

// ------------------------------------------------------------------------------------------

void CSide::ComputeRads (void)
{
	fix 			d, rMin = 0x7fffffff, rMax = 0;
	CFixVector	v;

m_rads [0] = 0x7fffffff;
m_rads [1] = 0;
for (int i = 0; i < 4; i++) {
	v = CFixVector::Avg (gameData.segs.vertices [m_vertices [i]], gameData.segs.vertices [m_vertices [(i + 1) % 4]]);
	d = CFixVector::Dist (v, m_vCenter);
	if (m_rads [0] > d)
		m_rads [0] = d;
	d = CFixVector::Dist (m_vCenter, gameData.segs.vertices [m_vertices [i]]);
	if (m_rads [1] < d)
		m_rads [1] = d;
	}
}

// -----------------------------------------------------------------------------------
//	Given a CSide, return the number of faces
int CSide::FaceCount (void)
{
if (m_nType == SIDE_IS_QUAD)
	return 1;
if ((m_nType == SIDE_IS_TRI_02) || (m_nType == SIDE_IS_TRI_13))
	return 2;
Error ("Illegal side type = %i\n", m_nType);
return -1;
}

// -------------------------------------------------------------------------------

void CSide::SetupCorners (short* verts, int* index)
{
m_corners [0] = verts [index [0]];
m_corners [1] = verts [index [1]];
m_corners [2] = verts [index [2]];
m_corners [3] = verts [index [3]];
}

// -------------------------------------------------------------------------------

void CSide::SetupVertexList (short* verts, int* index)
{
m_nFaces = -1;
if (m_nType == SIDE_IS_QUAD) {
	m_vertices [0] = verts [index [0]];
	m_vertices [1] = verts [index [1]];
	m_vertices [2] = verts [index [2]];
	m_vertices [3] = verts [index [3]];
	m_nFaces = 1;
	}
else if (m_nType == SIDE_IS_TRI_02) {
	m_vertices [0] =
	m_vertices [5] = verts [index [0]];
	m_vertices [1] = verts [index [1]];
	m_vertices [2] =
	m_vertices [3] = verts [index [2]];
	m_vertices [4] = verts [index [3]];
	m_nMinVertex [1] = min (m_vertices [1], m_vertices [4]);
	m_nFaces = 2;
	}
else if (m_nType == SIDE_IS_TRI_13) {
	m_vertices [0] =
	m_vertices [5] = verts [index [3]];
	m_vertices [1] = verts [index [0]];
	m_vertices [2] =
	m_vertices [3] = verts [index [1]];
	m_vertices [4] = verts [index [2]];
	m_nFaces = 2;
	}
else {
	return;
	}

m_nMinVertex [0] = min (m_vertices [0], m_vertices [2]);
if (m_nType == SIDE_IS_QUAD) {
	if (m_nMinVertex [0] > m_vertices [1])
		m_nMinVertex [0] = m_vertices [1];
	if (m_nMinVertex [0] > m_vertices [3])
		m_nMinVertex [0] = m_vertices [3];
	m_nMinVertex [1] = m_nMinVertex [0];
	}
else
	m_nMinVertex [1] = min (m_vertices [1], m_vertices [4]);

SetupFaceVertIndex ();
}

// -----------------------------------------------------------------------------------
// Like create all vertex lists, but returns the vertnums (relative to
// the side) for each of the faces that make up the CSide.
//	If there is one face, it has 4 vertices.
//	If there are two faces, they both have three vertices, so face #0 is stored in vertices 0, 1, 2,
//	face #1 is stored in vertices 3, 4, 5.

void CSide::SetupFaceVertIndex (void)
{
if (m_nType == SIDE_IS_QUAD) {
	m_faceVerts [0] = 0;
	m_faceVerts [1] = 1;
	m_faceVerts [2] = 2;
	m_faceVerts [3] = 3;
	}
else if (m_nType == SIDE_IS_TRI_02) {
	m_faceVerts [0] =
	m_faceVerts [5] = 0;
	m_faceVerts [1] = 1;
	m_faceVerts [2] =
	m_faceVerts [3] = 2;
	m_faceVerts [4] = 3;
	}
else if (m_nType == SIDE_IS_TRI_13) {
	m_faceVerts [0] =
	m_faceVerts [5] = 3;
	m_faceVerts [1] = 0;
	m_faceVerts [2] =
	m_faceVerts [3] = 1;
	m_faceVerts [4] = 2;
	}
}

// -------------------------------------------------------------------------------

void CSide::SetupAsQuad (CFixVector& vNormal, short* verts, int* index)
{
m_nType = SIDE_IS_QUAD;
m_normals [0] = 
m_normals [1] = vNormal;
SetupVertexList (verts, index);
}

// -------------------------------------------------------------------------------

void CSide::SetupAsTriangles (bool bSolid, short* verts, int* index)
{
	CFixVector	vNormal;
	fix			dot;
	CFixVector	vec_13;		//	vector from vertex 1 to vertex 3

	//	Choose how to triangulate.
	//	If a CWall, then
	//		Always triangulate so CSegment is convex.
	//		Use Matt's formula: Na . AD > 0, where ABCD are vertices on CSide, a is face formed by A, B, C, Na is Normal from face a.
	//	If not a CWall, then triangulate so whatever is on the other CSide is triangulated the same (ie, between the same absoluate vertices)
if (bSolid) {
	vNormal = CFixVector::Normal (gameData.segs.vertices [m_corners [0]],
	                              gameData.segs.vertices [m_corners [1]],
	                              gameData.segs.vertices [m_corners [2]]);
	vec_13 = gameData.segs.vertices [m_corners [3]] - gameData.segs.vertices [m_corners [1]];
	dot = CFixVector::Dot (vNormal, vec_13);

	//	Now, signify whether to triangulate from 0:2 or 1:3
	m_nType = (dot >= 0) ? SIDE_IS_TRI_02 : SIDE_IS_TRI_13;
	//	Now, based on triangulation nType, set the normals.
	if (m_nType == SIDE_IS_TRI_02) {
#if 0
		VmVecNormalChecked (&vNormal, 
								  gameData.segs.vertices + m_corners [0], 
								  gameData.segs.vertices + m_corners [1], 
								  gameData.segs.vertices + m_corners [2]);
#endif
		m_normals [0] = vNormal;
		m_normals [1] = CFixVector::Normal (gameData.segs.vertices [m_corners [0]], 
														gameData.segs.vertices [m_corners [2]], 
														gameData.segs.vertices [m_corners [3]]);
		}
	else {
		m_normals [0] = CFixVector::Normal (gameData.segs.vertices [m_corners [0]], 
														gameData.segs.vertices [m_corners [1]], 
														gameData.segs.vertices [m_corners [3]]);
		m_normals [1] = CFixVector::Normal (gameData.segs.vertices [m_corners [1]], 
														gameData.segs.vertices [m_corners [2]], 
														gameData.segs.vertices [m_corners [3]]);
		}
	}
else {
	short	vSorted [4];
	int	bFlip;

	bFlip = GetVertsForNormal (m_corners [0], m_corners [1], m_corners [2], m_corners [3], vSorted);
	if ((vSorted [0] == m_corners [0]) || (vSorted [0] == m_corners [2])) {
		m_nType = SIDE_IS_TRI_02;
		//	Now, get vertices for Normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (m_corners [0], m_corners [1], m_corners [2], 32767, vSorted);
		m_normals [0] = CFixVector::Normal (gameData.segs.vertices [vSorted [0]], 
														gameData.segs.vertices [vSorted [1]], 
														gameData.segs.vertices [vSorted [2]]);
		if (bFlip)
			m_normals [0].Neg ();
		bFlip = GetVertsForNormal (m_corners [0], m_corners [2], m_corners [3], 32767, vSorted);
		m_normals [1] = CFixVector::Normal (gameData.segs.vertices [vSorted [0]],
														gameData.segs.vertices [vSorted [1]],
														gameData.segs.vertices [vSorted [2]]);
		if (bFlip)
			m_normals [1].Neg ();
		GetVertsForNormal (m_corners [0], m_corners [2], m_corners [3], 32767, vSorted);
		}
	else {
		m_nType = SIDE_IS_TRI_13;
		//	Now, get vertices for Normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (m_corners [0], m_corners [1], m_corners [3], 32767, vSorted);
		m_normals [0] = CFixVector::Normal (gameData.segs.vertices [vSorted [0]],
														gameData.segs.vertices [vSorted [1]],
														gameData.segs.vertices [vSorted [2]]);
		if (bFlip)
			m_normals [0].Neg ();
		bFlip = GetVertsForNormal (m_corners [1], m_corners [2], m_corners [3], 32767, vSorted);
		m_normals [1] = CFixVector::Normal (
						 gameData.segs.vertices [vSorted [0]],
						 gameData.segs.vertices [vSorted [1]],
						 gameData.segs.vertices [vSorted [2]]);
		if (bFlip)
			m_normals [1].Neg ();
		}
	}
SetupVertexList (verts, index);
}

// -------------------------------------------------------------------------------

int sign (fix v)
{
if (v > PLANE_DIST_TOLERANCE)
	return 1;
if (v < - (PLANE_DIST_TOLERANCE + 1))		//neg & pos round differently
	return -1;
return 0;
}

// -------------------------------------------------------------------------------

void CSide::Setup (short* verts, int* index, bool bSolid)
{
	short			vSorted [4], bFlip;
	int			i;
	CFixVector	vNormal;
	fix			xDistToPlane;

SetupCorners (verts, index);
bFlip = GetVertsForNormal (m_corners [0], m_corners [1], m_corners [2], m_corners [3], vSorted);
vNormal = CFixVector::Normal (gameData.segs.vertices [vSorted [0]], gameData.segs.vertices [vSorted [1]], gameData.segs.vertices [vSorted [2]]);
xDistToPlane = abs (gameData.segs.vertices [vSorted [3]].DistToPlane (vNormal, gameData.segs.vertices [vSorted [0]]));
if (bFlip)
	vNormal.Neg ();
if (xDistToPlane <= PLANE_DIST_TOLERANCE)
	SetupAsQuad (vNormal, verts, index);
else {
	SetupAsTriangles (bSolid, verts, index);
	//this code checks to see if we really should be triangulated, and
	//de-triangulates if we shouldn't be.
	Assert (m_nFaces == 2);
	fix dist0 = gameData.segs.vertices [m_vertices [1]].DistToPlane (m_normals [1], gameData.segs.vertices [m_nMinVertex [0]]);
	fix dist1 = gameData.segs.vertices [m_vertices [4]].DistToPlane (m_normals [0], gameData.segs.vertices [m_nMinVertex [0]]);
	int s0 = sign (dist0);
	int s1 = sign (dist1);
	if (s0 == 0 || s1 == 0 || s0 != s1)
		SetupAsQuad (vNormal, verts, index);
	}
if (m_nType == SIDE_IS_QUAD) {
	AddToVertexNormal (m_vertices [0], vNormal);
	AddToVertexNormal (m_vertices [1], vNormal);
	AddToVertexNormal (m_vertices [2], vNormal);
	AddToVertexNormal (m_vertices [3], vNormal);
	}
else {
	for (i = 0; i < 3; i++)
		AddToVertexNormal (m_vertices [i], m_normals [0]);
	for (; i < 6; i++)
		AddToVertexNormal (m_vertices [i], m_normals [1]);
	}
}

// -------------------------------------------------------------------------------

inline CFixVector& CSide::Vertex (int nVertex)
{
return gameStates.render.bRendering ? gameData.segs.points [nVertex].p3_vec : gameData.segs.vertices [nVertex];
}

// -------------------------------------------------------------------------------

inline CFixVector& CSide::MinVertex (void)
{
return Vertex (m_nMinVertex [0]);
}

// -------------------------------------------------------------------------------
// height of a two faced, non-planar side

inline fix CSide::Height (void)
{
return Vertex (m_nMinVertex [1]).DistToPlane (Normal (m_vertices [4] >= m_vertices [1]), Vertex (m_nMinVertex [0]));
}

// -------------------------------------------------------------------------------

inline bool CSide::IsPlanar (void)
{
return (m_nFaces < 2) || (Height () <= PLANE_DIST_TOLERANCE);
}

// -------------------------------------------------------------------------------
//returns 3 different bitmasks with info telling if this sphere is in
//this CSegment.  See CSegMasks structure for info on fields
CSegMasks CSide::Masks (const CFixVector& refPoint, fix xRad, short sideBit, short& faceBit)
{
	CSegMasks	masks;
	fix			xDist;

masks.m_valid = 1;
if (m_nFaces == 2) {
	int	nSideCount = 0, 
			nCenterCount = 0;

	CFixVector minVertex = MinVertex ();
	for (int nFace = 0; nFace < 2; nFace++, faceBit <<= 1) {
		xDist = refPoint.DistToPlane (Normal (nFace), minVertex);
		if (xDist - xRad < -PLANE_DIST_TOLERANCE) {	// xRad must be >= 0!
			masks.m_face |= faceBit;
			nSideCount++;
			if (xDist < -PLANE_DIST_TOLERANCE) //in front of face
				nCenterCount++;
			}
		}
	if (IsPlanar ()) {	//must be behind both faces
		if (nSideCount == 2)
			masks.m_side |= sideBit;
		if (nCenterCount == 2)
			masks.m_center |= sideBit;
		}
	else {	//must be behind at least one face
		if (nSideCount)
			masks.m_side |= sideBit;
		if (nCenterCount)
			masks.m_center |= sideBit;
		}
	}
else {	
	xDist = refPoint.DistToPlane (Normal (0), MinVertex ());
	if (xDist - xRad < -PLANE_DIST_TOLERANCE) {	// xRad must be >= 0!
		masks.m_face |= faceBit;
		masks.m_side |= sideBit;
		if (xDist < -PLANE_DIST_TOLERANCE)
			masks.m_center |= sideBit;
		}
	faceBit <<= 2;
	}
masks.m_valid = 1;
return masks;
}

// -------------------------------------------------------------------------------

ubyte CSide::Dist (const CFixVector& refPoint, fix& xSideDist, int bBehind, short sideBit)
{
	fix	xDist;
	ubyte mask = 0;

xSideDist = 0;
if (m_nFaces == 2) {
	int nCenterCount = 0;

	CFixVector minVertex = MinVertex ();
	for (int nFace = 0; nFace < 2; nFace++) {
		xDist = refPoint.DistToPlane (Normal (nFace), minVertex);
		if ((xDist < -PLANE_DIST_TOLERANCE) == bBehind) {	//in front of face
			nCenterCount++;
			xSideDist += xDist;
			}
		}
	if (IsPlanar ()) {	//must be behind both faces
		if (nCenterCount == 2) {
			mask |= sideBit;
			xSideDist /= 2;	//get average
			}
		}
	else {	//must be behind at least one face
		if (nCenterCount) {
			mask |= sideBit;
			if (nCenterCount == 2)
				xSideDist /= 2;	//get average
			}
		}
	}
else {				//only one face on this CSide
	xDist = refPoint.DistToPlane (Normal (0), MinVertex ());
	if ((xDist < -PLANE_DIST_TOLERANCE) == bBehind) {
		mask |= sideBit;
		xSideDist = xDist;
		}
	}
return mask;
}

//	-----------------------------------------------------------------------------

inline CWall* CSide::Wall (void) 
{ 
return IS_WALL (m_nWall) ? WALLS + m_nWall : NULL; 
}

//	-----------------------------------------------------------------------------
//see if a refP is inside a face by projecting into 2d
uint CSide::CheckPointToFace (CFixVector& intersection, short iFace, CFixVector vNormal)
{
	CFixVector	t;
	int			biggest;
	int 			h, i, j, nEdge, nVerts;
	uint 			nEdgeMask;
	fix 			check_i, check_j;
	CFixVector	*v0, *v1;
	vec2d 		vEdge, vCheck;
	fix 			d;

//now do 2d check to see if refP is in CSide
//project polygon onto plane by finding largest component of Normal
t [X] = labs (vNormal [0]);
t [Y] = labs (vNormal [1]);
t [Z] = labs (vNormal [2]);
if (t [X] > t [Y])
	if (t [X] > t [Z])
		biggest = 0;
	else
		biggest = 2;
else if (t [Y] > t [Z])
	biggest = 1;
else
	biggest = 2;
if (vNormal [biggest] > 0) {
	i = ijTable [biggest][0];
	j = ijTable [biggest][1];
	}
else {
	i = ijTable [biggest][1];
	j = ijTable [biggest][0];
	}
//now do the 2d problem in the i, j plane
check_i = intersection [i];
check_j = intersection [j];
nVerts = 5 - m_nFaces;
h = iFace * 3;
for (nEdge = nEdgeMask = 0; nEdge < nVerts; nEdge++) {
	if (gameStates.render.bRendering) {
		v0 = &gameData.segs.points [m_vertices [h + nEdge]].p3_vec;
		v1 = &gameData.segs.points [m_vertices [h + ((nEdge + 1) % nVerts)]].p3_vec;
		}
	else {
		v0 = gameData.segs.vertices + m_vertices [h + nEdge];
		v1 = gameData.segs.vertices + m_vertices [h + ((nEdge + 1) % nVerts)];
		}
	vEdge.i = (*v1) [i] - (*v0) [i];
	vEdge.j = (*v1) [j] - (*v0) [j];
	vCheck.i = check_i - (*v0) [i];
	vCheck.j = check_j - (*v0) [j];
	d = FixMul (vCheck.i, vEdge.j) - FixMul (vCheck.j, vEdge.i);
	if (d < 0)              		//we are outside of triangle
		nEdgeMask |= (1 << nEdge);
	}
return nEdgeMask;
}

//	-----------------------------------------------------------------------------
//check if a sphere intersects a face
int CSide::CheckSphereToFace (CFixVector& intersection, fix rad, short iFace, CFixVector vNormal)
{
	CFixVector	vEdge, vCheck;            //this time, real 3d vectors
	CFixVector	vClosestPoint;
	fix			xEdgeLen, d, dist;
	CFixVector	*v0, *v1;
	int			iType;
	int			nEdge, nVerts;
	uint			nEdgeMask;

//now do 2d check to see if refP is in side
nEdgeMask = CheckPointToFace (intersection, iFace, vNormal);
//we've gone through all the sides, are we inside?
if (nEdgeMask == 0)
	return IT_FACE;
//get verts for edge we're behind
for (nEdge = 0; !(nEdgeMask & 1); (nEdgeMask >>= 1), nEdge++)
	;
nVerts = 5 - m_nFaces;
if (gameStates.render.bRendering) {
	v0 = &gameData.segs.points [m_vertices [iFace * 3 + nEdge]].p3_vec;
	v1 = &gameData.segs.points [m_vertices [iFace * 3 + ((nEdge + 1) % nVerts)]].p3_vec;
	}
else {
	v0 = gameData.segs.vertices + m_vertices [iFace * 3 + nEdge];
	v1 = gameData.segs.vertices + m_vertices [iFace * 3 + ((nEdge + 1) % nVerts)];
	}
//check if we are touching an edge or refP
vCheck = intersection - *v0;
xEdgeLen = CFixVector::NormalizedDir (vEdge, *v1, *v0);
//find refP dist from planes of ends of edge
d = CFixVector::Dot (vEdge, vCheck);
if (d + rad < 0)
	return IT_NONE;                  //too far behind start refP
if (d - rad > xEdgeLen)
	return IT_NONE;    //too far part end refP
//find closest refP on edge to check refP
iType = IT_POINT;
if (d < 0)
	vClosestPoint = *v0;
else if (d > xEdgeLen)
	vClosestPoint = *v1;
else {
	iType = IT_EDGE;
	vClosestPoint = *v0 + vEdge * d;
	}
dist = CFixVector::Dist (intersection, vClosestPoint);
if (dist <= rad)
	return (iType == IT_POINT) ? IT_NONE : iType;
return IT_NONE;
}

//	-----------------------------------------------------------------------------
//returns true if line intersects with face. fills in intersection with intersection
//refP on plane, whether or not line intersects CSide
//iFace determines which of four possible faces we have
//note: the seg parm is temporary, until the face itself has a refP field
int CSide::CheckLineToFace (CFixVector& intersection, CFixVector *p0, CFixVector *p1, fix rad, short iFace, CFixVector vNormal)
{
	CFixVector	v1;
	int			pli, nVertex, bCheckRad = 0;

//use lowest refP number
#if 1 //def _DEBUG
if (m_nFaces <= iFace) {
	Error ("invalid face number in CSegment::CheckLineToFace()");
	return IT_ERROR;
	}
#endif
#if 1
if (p1 == p0) {
#if 0
	return CheckSphereToFace (p0, rad, iFace, vNormal);
#else
	if (!rad)
		return IT_NONE;
	v1 = vNormal * (-rad);
	v1 += *p0;
	bCheckRad = rad;
	rad = 0;
	p1 = &v1;
#endif
	}
#endif
nVertex = m_vertices [0];
if (nVertex > m_vertices [2])
	nVertex = m_vertices [2];
if (m_nFaces == 1) {
	if (nVertex > m_vertices [1])
		nVertex = m_vertices [1];
	if (nVertex > m_vertices [3])
		nVertex = m_vertices [3];
	}
//PrintLog ("         FindPlaneLineIntersection...");
pli = FindPlaneLineIntersection (intersection,
											gameStates.render.bRendering 
											? &gameData.segs.points [nVertex].p3_vec 
											: gameData.segs.vertices + nVertex,
											&vNormal, p0, p1, rad);
//PrintLog ("done\n");
if (!pli)
	return IT_NONE;
CFixVector vHit = intersection;
//if rad != 0, project the refP down onto the plane of the polygon
if (rad)
	vHit += vNormal * (-rad);
if ((pli = CheckSphereToFace (vHit, rad, iFace, vNormal)))
	return pli;
if (bCheckRad) {
	int			i, d;
	CFixVector	*a, *b;

	b = gameData.segs.vertices + m_vertices [0];
	for (i = 1; i <= 4; i++) {
		a = b;
		b = gameData.segs.vertices + m_vertices [i % 4];
		d = VmLinePointDist (*a, *b, *p0);
		if (d < bCheckRad)
			return IT_POINT;
		}
	}
return IT_NONE;
}

//	-----------------------------------------------------------------------------
//this version is for when the start and end positions both poke through
//the plane of a CSide.  In this case, we must do checks against the edge
//of faces
int CSide::SpecialCheckLineToFace (CFixVector& intersection, CFixVector *p0, CFixVector *p1, fix rad, short iFace, CFixVector vNormal)
{
	CFixVector	vMove;
	fix			edge_t, move_t, edge_t2, move_t2, closestDist;
	fix			edge_len, move_len;
	int			nEdge, nVerts;
	uint			nEdgeMask;
	CFixVector	*edge_v0, *edge_v1, vEdge;
	CFixVector	vClosestEdgePoint, vClosestMovePoint;

vMove = *p1 - *p0;
//figure out which edge(side) to check against
//PrintLog ("      CheckPointToSegFace ...\n");
if (!(nEdgeMask = CheckPointToFace (*p0, iFace, vNormal))) {
	//PrintLog ("      CheckLineToSegFace ...");
	return CheckLineToFace (intersection, p0, p1, rad, iFace, vNormal);
	//PrintLog ("done\n");
	}
for (nEdge = 0; !(nEdgeMask & 1); nEdgeMask >>= 1, nEdge++)
	;
nVerts = 5 - m_nFaces;
edge_v0 = gameData.segs.vertices + m_vertices [iFace * 3 + nEdge];
edge_v1 = gameData.segs.vertices + m_vertices [iFace * 3 + ((nEdge + 1) % nVerts)];
vEdge = *edge_v1 - *edge_v0;
//is the start refP already touching the edge?
//first, find refP of closest approach of vec & edge
edge_len = CFixVector::Normalize (vEdge);
move_len = CFixVector::Normalize (vMove);
CheckLineToLine (&edge_t, &move_t, edge_v0, &vEdge, p0, &vMove);
//make sure t values are in valid range
if ((move_t < 0) || (move_t > move_len + rad))
	return IT_NONE;
if (move_t > move_len)
	move_t2 = move_len;
else
	move_t2 = move_t;
if (edge_t < 0)		//clamp at points
	edge_t2 = 0;
else
	edge_t2 = edge_t;
if (edge_t2 > edge_len)		//clamp at points
	edge_t2 = edge_len;
//now, edge_t & move_t determine closest points.  calculate the points.
vClosestEdgePoint = *edge_v0 + vEdge * edge_t2;
vClosestMovePoint = *p0 + vMove * move_t2;
//find dist between closest points
closestDist = CFixVector::Dist (vClosestEdgePoint, vClosestMovePoint);
//could we hit with this dist?
//note massive tolerance here
if (closestDist < (rad * 9) / 10) {		//we hit.  figure out where
	//now figure out where we hit
	intersection = *p0 + vMove * (move_t-rad);
	return IT_EDGE;
	}
return IT_NONE;			//no hit
}

//	-----------------------------------------------------------------------------
//finds the uv coords of the given refP on the given seg & side
//fills in u & v. if l is non-NULL fills it in also
void CSide::HitPointUV (fix *u, fix *v, fix *l, CFixVector& intersection, int iFace)
{
	CFixVector	*vPoints;
	CFixVector	vNormal;
	int			biggest, ii, jj;
 	vec2d			p1, vec0, vec1, vHit;
	tUVL			uvls [3];
	fix			k0, k1;
	int			h;

if (iFace >= m_nFaces) {
	PrintLog ("invalid face number in CSide::HitPointUV\n");
	*u = *v = 0;
	return;
	}
//now the hard work.
//1. find what plane to project this CWall onto to make it a 2d case
vNormal = m_normals [iFace];
biggest = 0;
if (abs (vNormal [1]) > abs (vNormal [biggest]))
	biggest = 1;
if (abs (vNormal [2]) > abs (vNormal [biggest]))
	biggest = 2;
ii = (biggest == 0);
jj = (biggest == 2) ? 1 : 2;
//2. compute u, v of intersection refP
//vec from 1 -> 0
h = iFace * 3;
vPoints = gameData.segs.vertices + m_vertices [h+1];
p1.i = (*vPoints) [ii];
p1.j = (*vPoints) [jj];

vPoints = gameData.segs.vertices + m_vertices [h];
vec0.i = (*vPoints) [ii] - p1.i;
vec0.j = (*vPoints) [jj] - p1.j;

//vec from 1 -> 2
vPoints = gameData.segs.vertices + m_vertices [h+2];
vec1.i = (*vPoints) [ii] - p1.i;
vec1.j = (*vPoints) [jj] - p1.j;

//vec from 1 -> checkPoint
//vPoints = reinterpret_cast<CFixVector*> (refP);
vHit.i = intersection [ii];
vHit.j = intersection [jj];

#if 1 // the MSVC 9 optimizer doesn't like the code in the else branch ...
ii = Cross2D (vHit, vec0) + Cross2D (vec0, p1);
jj = Cross2D (vec0, vec1);
k1 = -FixDiv (ii, jj);
#else
k1 = -FixDiv (Cross2D (refP, vec0) + Cross2D (vec0, p1), Cross2D (vec0, vec1));
#endif
if (abs (vec0.i) > abs (vec0.j))
	k0 = FixDiv (FixMul (-k1, vec1.i) + vHit.i - p1.i, vec0.i);
else
	k0 = FixDiv (FixMul (-k1, vec1.j) + vHit.j - p1.j, vec0.j);
uvls [0] = m_uvls [m_faceVerts [h]];
uvls [1] = m_uvls [m_faceVerts [h+1]];
uvls [2] = m_uvls [m_faceVerts [h+2]];
*u = uvls [1].u + FixMul (k0, uvls [0].u - uvls [1].u) + FixMul (k1, uvls [2].u - uvls [1].u);
*v = uvls [1].v + FixMul (k0, uvls [0].v - uvls [1].v) + FixMul (k1, uvls [2].v - uvls [1].v);
if (l)
	*l = uvls [1].l + FixMul (k0, uvls [0].l - uvls [1].l) + FixMul (k1, uvls [2].l - uvls [1].l);
}

//------------------------------------------------------------------------------

bool CSide::IsOpenableDoor (void)
{
return IS_WALL (m_nWall) ? WALLS [m_nWall].IsOpenableDoor () : false;
}

//------------------------------------------------------------------------------

CFixVector* CSide::GetCorners (CFixVector* vertices) 
{ 
for (int i = 0; i < 4; i++)
	vertices [i] = gameData.segs.vertices [m_corners [i]];
return vertices;
}

//------------------------------------------------------------------------------

bool CSide::IsWall (void)
{
return IS_WALL (m_nWall); 
}

//------------------------------------------------------------------------------

CTrigger* CSide::Trigger (void)
{
return IS_WALL (m_nWall) ? Wall ()->Trigger () : NULL;
}

//------------------------------------------------------------------------------

bool CSide::IsVolatile (void)
{
return IsWall () && WALLS [m_nWall].IsVolatile ();
}

//------------------------------------------------------------------------------

int CSide::Physics (fix* damageP)
{
CWall* wallP = Wall ();
if (!wallP || (wallP->nType != WALL_ILLUSION))
	return 0;
if (gameData.pig.tex.tMapInfoP [m_nBaseTex].damage) {
	if (damageP)
		*damageP = gameData.pig.tex.tMapInfoP [m_nBaseTex].damage;
	return 1;
	}
if (gameData.pig.tex.tMapInfoP [m_nBaseTex].flags & TMI_WATER)
	return 2;
return 0;
}

//-----------------------------------------------------------------

int CSide::CheckTransparency (void)
{
	CBitmap	*bmP;

if (m_nOvlTex) {
	bmP = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [m_nOvlTex].index].Override (-1);
	if (bmP->Flags () & BM_FLAG_SUPER_TRANSPARENT)
		return 1;
	if (!(bmP->Flags () & BM_FLAG_TRANSPARENT))
		return 0;
	}
bmP = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [m_nBaseTex].index].Override (-1);
if (bmP->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
	return 1;
if ((gameStates.app.bD2XLevel) && IS_WALL (m_nWall)) {
	short c = WALLS [m_nWall].cloakValue;
	if (c && (c < FADE_LEVELS))
		return 1;
	}
return gameOpts->render.effects.bAutoTransparency && IsTransparentTexture (m_nBaseTex);
}

//------------------------------------------------------------------------------

void CSide::SetTextures (int nBaseTex, int nOvlTex)
{
if (nBaseTex >= 0)
	m_nBaseTex = nBaseTex;
if (nOvlTex >= 0)
	m_nOvlTex = nOvlTex;
}

//------------------------------------------------------------------------------

short ConvertD1Texture (short nD1Texture, int bForce);

void CSide::Read (CFile& cf, ushort* sideVerts, bool bSolid)
{
	int nType = bSolid ? 1 : IS_WALL (m_nWall) ? 2 : 0;

m_nFrame = 0;
if (!nType)
	m_nBaseTex =
	m_nOvlTex = 0;
else {
	// Read short sideP->m_nBaseTex;
	ushort nTexture;
	if (bNewFileFormat) {
		nTexture = ushort (cf.ReadShort ());
		m_nBaseTex = nTexture & 0x7fff;
		}
	else {
		nTexture = 0;
		m_nBaseTex = cf.ReadShort ();
		}
	if (gameData.segs.nLevelVersion <= 1)
		m_nBaseTex = ConvertD1Texture (m_nBaseTex, 0);
	if (bNewFileFormat && !(nTexture & 0x8000))
		m_nOvlTex = 0;
	else {
		// Read short m_nOvlTex;
		nTexture = cf.ReadShort ();
		m_nOvlTex = nTexture & 0x3fff;
		m_nOvlOrient = (nTexture >> 14) & 3;
		if ((gameData.segs.nLevelVersion <= 1) && m_nOvlTex)
			m_nOvlTex = ConvertD1Texture (m_nOvlTex, 0);
		}

	// Read tUVL m_uvls [4] (u, v>>5, write as short, l>>1 write as short)
	for (int i = 0; i < 4; i++) {
		m_uvls [i].u = fix (cf.ReadShort ()) << 5;
		m_uvls [i].v = fix (cf.ReadShort ()) << 5;
		m_uvls [i].l = fix (cf.ReadUShort ()) << 1;
		gameData.render.color.vertBright [sideVerts [i]] = X2F (m_uvls [i].l);
		}
	}
}

//------------------------------------------------------------------------------

void CSide::ReadWallNum (CFile& cf, bool bWall)
{
m_nWall = ushort (bWall ? (gameData.segs.nLevelVersion >= 13) ? cf.ReadShort () : cf.ReadByte () : -1);
}

//------------------------------------------------------------------------------
// reads a CSegment structure from a CFile
 
void CSide::SaveState (CFile& cf)
{
cf.WriteShort (m_nWall);
cf.WriteShort (m_nBaseTex);
cf.WriteShort (m_nOvlTex | (m_nOvlOrient << 14));
}

//------------------------------------------------------------------------------
// reads a CSegment structure from a CFile
 
void CSide::LoadState (CFile& cf)
{
m_nWall = cf.ReadShort ();
m_nBaseTex = cf.ReadShort ();
	short nTexture = cf.ReadShort ();
m_nOvlTex = nTexture & 0x3fff;
m_nOvlOrient = (nTexture >> 14) & 3;
}

//------------------------------------------------------------------------------

//eof
