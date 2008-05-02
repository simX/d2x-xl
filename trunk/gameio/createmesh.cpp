/* $Id: gamemine.c, v 1.26 2003/10/22 15:00:37 schaffner Exp $ */
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

#define LIGHT_VERSION 4

#ifdef RCS
char rcsid [] = "$Id: gamemine.c, v 1.26 2003/10/22 15:00:37 schaffner Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pstypes.h"
#include "mono.h"

#include "inferno.h"
#include "error.h"
#include "text.h"
#include "segment.h"
#include "textures.h"
#include "wall.h"
#include "object.h"
#include "gamemine.h"
#include "gameseg.h"
#include "switch.h"
#include "ogl_defs.h"
#include "oof.h"
#include "lightmap.h"
#include "render.h"
#include "gameseg.h"

#include "game.h"
#include "menu.h"
#include "newmenu.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#include "cfile.h"
#include "fuelcen.h"

#include "hash.h"
#include "key.h"
#include "piggy.h"

#include "byteswap.h"
#include "gamesave.h"
#include "u_mem.h"
#include "vecmat.h"
#include "gamepal.h"
#include "paging.h"
#include "maths.h"
#include "network.h"
#include "light.h"
#include "dynlight.h"
#include "renderlib.h"
#include "createmesh.h"

using namespace mesh;

//------------------------------------------------------------------------------

#define	MAX_EDGE_LEN	fMaxEdgeLen [gameOpts->render.nMeshQuality]

#define MESH_DATA_VERSION 1

//------------------------------------------------------------------------------

typedef struct tMeshDataHeader {
	int	nVersion;
	int	nSegments;
	int	nVertices;
	int	nFaceVerts;
	int	nFaces;
	int	nTris;
	} tMeshHeaderData;

//------------------------------------------------------------------------------

float fMaxEdgeLen [] = {1e30f, 30.9f, 20.9f, 19.9f, 9.9f};

CQuadMeshBuilder quadMeshBuilder;

//------------------------------------------------------------------------------

void CTriMeshBuilder::FreeData (void)
{
D2_FREE (m_triangles);
D2_FREE (m_edges);
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::AllocData (void)
{
if (m_nMaxTriangles && m_nMaxEdges) {
	if (!((m_edges = (tEdge *) D2_REALLOC (m_edges, 2 * m_nMaxTriangles * sizeof (tEdge))) &&
			(m_triangles = (tTriangle *) D2_REALLOC (m_triangles, 2 * m_nMaxTriangles * sizeof (tTriangle))))) {
		FreeData ();
		return 0;
		}
	memset (m_edges + m_nMaxTriangles, 0xff, m_nMaxTriangles * sizeof (tEdge));
	memset (m_triangles + m_nMaxEdges, 0xff, m_nMaxTriangles * sizeof (tTriangle));
	m_nMaxTriangles *= 2;
	m_nMaxEdges *= 2;
	}
else {
	m_nMaxTriangles = gameData.segs.nFaces * 4;
	m_nMaxEdges = gameData.segs.nFaces * 4;
	if (!(m_edges = (tEdge *) D2_ALLOC (m_nMaxEdges * sizeof (tEdge))))
		return 0;
	if (!(m_triangles = (tTriangle *) D2_ALLOC (m_nMaxTriangles * sizeof (tTriangle)))) {
		FreeData ();
		return 0;
		}
	memset (m_edges, 0xff, m_nMaxTriangles * sizeof (tEdge));
	memset (m_triangles, 0xff, m_nMaxTriangles * sizeof (tTriangle));
	}
return 1;
}

//------------------------------------------------------------------------------

tEdge *CTriMeshBuilder::FindEdge (ushort nVert1, ushort nVert2, int i)
{
	tEdge	*edgeP = m_edges + ++i;

for (; i < m_nEdges; i++, edgeP++)
	if ((edgeP->verts [0] == nVert1) && (edgeP->verts [1] == nVert2))
		return edgeP;
return NULL;
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::AddEdge (int nTri, ushort nVert1, ushort nVert2)
{
if (nVert2 < nVert1) {
	ushort h = nVert1;
	nVert1 = nVert2;
	nVert2 = h;
	}
#ifdef _DEBUG
if ((nTri < 0) || (nTri >= m_nTriangles))
	return -1;
#endif
tEdge *edgeP;
for (int i = -1;;) {
	if (!(edgeP = FindEdge (nVert1, nVert2, i)))
		break;
	i = edgeP - m_edges;
	if (edgeP->tris [0] < 0) {
		edgeP->tris [0] = nTri;
		return i;
		}
	if (edgeP->tris [1] < 0) {
		edgeP->tris [1] = nTri;
		return i;
		}
	}
if (m_nFreeEdges >= 0) {
	edgeP = m_edges + m_nFreeEdges;
	m_nFreeEdges = edgeP->nNext;
	edgeP->nNext = -1;
	}
else {
	if ((m_nEdges == m_nMaxEdges - 1) && !AllocData ())
		return -1;
	edgeP = m_edges + m_nEdges++;
	}
edgeP->tris [0] = nTri;
edgeP->verts [0] = nVert1;
edgeP->verts [1] = nVert2;
edgeP->fLength = VmVecDist (gameData.segs.fVertices + nVert1, gameData.segs.fVertices + nVert2);
return edgeP - m_edges;
}

//------------------------------------------------------------------------------

tTriangle *CTriMeshBuilder::CreateTriangle (tTriangle *triP, ushort index [], int nFace, int nIndex)
{
	int	h, i;

if (triP) 
	triP->nIndex = nIndex;
else {
	if ((m_nTriangles == m_nMaxTriangles - 1) && !AllocData ())
		return NULL;
	triP = m_triangles + m_nTriangles++;
	triP->nIndex = nIndex;
	if (nIndex >= 0) {
		i = TRIANGLES [nIndex].nIndex;
		memcpy (triP->texCoord, gameData.segs.faces.texCoord + i, sizeof (triP->texCoord));
		memcpy (triP->ovlTexCoord, gameData.segs.faces.ovlTexCoord + i, sizeof (triP->ovlTexCoord));
		memcpy (triP->color, gameData.segs.faces.color + i, sizeof (triP->color));
		}
	}
nIndex = triP - m_triangles;
for (i = 0; i < 3; i++) {
	if (0 > (h = AddEdge (nIndex, index [i], index [(i + 1) % 3])))
		return NULL;
	triP = m_triangles + nIndex;
	triP->lines [i] = h;
	}
triP->nFace = nFace;
memcpy (triP->index, index, sizeof (triP->index));
return triP;
}

//------------------------------------------------------------------------------

tTriangle *CTriMeshBuilder::AddTriangle (tTriangle *triP, ushort index [], grsTriangle *grsTriP)
{
return CreateTriangle (triP, index, grsTriP->nFace, grsTriP - TRIANGLES);
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::DeleteEdge (tEdge *edgeP)
{
#if 1
edgeP->nNext = m_nFreeEdges;
m_nFreeEdges = edgeP - m_edges;
#else
	tTriangle	*triP;
	int		h = edgeP - m_edges, i, j;

if (h < --m_nEdges) {
	*edgeP = m_edges [m_nEdges];
	for (i = 0; i < 2; i++) {
		if (edgeP->tris [i] >= 0) {
			triP = m_triangles + edgeP->tris [i];
			for (j = 0; j < 3; j++)
				if (triP->lines [j] == m_nEdges)
					triP->lines [j] = h;
			}
		}
	}
#endif
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::DeleteTriangle (tTriangle *triP)
{
	tEdge	*edgeP;
	int			i, nTri = triP - m_triangles;

for (i = 0; i < 3; i++) {
	edgeP = m_edges + triP->lines [i];
	if (edgeP->tris [0] == nTri)
		edgeP->tris [0] = edgeP->tris [1];
	edgeP->tris [1] = -1;
	if (edgeP->tris [0] < 0)
		DeleteEdge (edgeP);
	}
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::CreateTriangles (void)
{
PrintLog ("   adding existing triangles\n");
m_nEdges = 0;
m_nTriangles = 0;
m_nMaxTriangles = 0;
m_nMaxTriangles = 0;
m_nFreeEdges = -1;
m_nVertices = gameData.segs.nVertices;
if (!AllocData ())
	return 0;

grsFace *faceP;
grsTriangle *grsTriP;
tTriangle *triP;
int i, nFace = -1;
short nId;

for (i = gameData.segs.nTris, grsTriP = TRIANGLES; i; i--, grsTriP++) {
	if (!(triP = AddTriangle (NULL, grsTriP->index, grsTriP))) {
		FreeData ();
		return 0;
		}
	if (nFace == grsTriP->nFace) 
		nId++;
	else {
		nFace = grsTriP->nFace;
		nId = 0;
		}
	triP->nId = nId;
	faceP = FACES + grsTriP->nFace;
#ifdef _DEBUG
	if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
#endif
	if (faceP->bSlide || (faceP->nCamera >= 0))
		triP->nPass = -2;
	}
return m_nTris = m_nTriangles;
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::SplitTriangleByEdge (int nTri, ushort nVert1, ushort nVert2, short nPass)
{
if (nTri < 0)
	return 1;

tTriangle *triP = m_triangles + nTri;

if (triP->nPass < -1)
	return 1;

grsFace *faceP = FACES + triP->nFace;

#ifdef _DEBUG
if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif

	int			h, i, nIndex = triP->nIndex;
	short			nId = triP->nId;
	ushort		nFace = triP->nFace, *indexP = triP->index, index [4];
	tTexCoord2f	texCoord [4], ovlTexCoord [4];
	tRgbaColorf	color [4];

// find insertion point for the new vertex
for (i = 0; i < 3; i++) {
	h = indexP [i];
	if ((h == nVert1) || (h == nVert2))
		break;
	}
if (i == 3)
	return 0;

h = indexP [(i + 1) % 3]; //check next vertex index
if ((h == nVert1) || (h == nVert2))
	h = (i + 1) % 3; //insert before i+1
else
	h = i; //insert before i

// build new quad index containing the new vertex
// the two triangle indices will be derived from it (indices 0,1,2 and 1,2,3)
index [0] = gameData.segs.nVertices - 1;
for (i = 1; i < 4; i++) {
	index [i] = indexP [h];
	texCoord [i] = triP->texCoord [h];
	ovlTexCoord [i] = triP->ovlTexCoord [h];
	color [i] = triP->color [h++];
	h %= 3;
	}

// interpolate texture coordinates and color for the new vertex
texCoord [0].v.v = (texCoord [1].v.v + texCoord [3].v.v) / 2;
texCoord [0].v.u = (texCoord [1].v.u + texCoord [3].v.u) / 2;
ovlTexCoord [0].v.v = (ovlTexCoord [1].v.v + ovlTexCoord [3].v.v) / 2;
ovlTexCoord [0].v.u = (ovlTexCoord [1].v.u + ovlTexCoord [3].v.u) / 2;
color [0].red = (color [1].red + color [3].red) / 2;
color [0].green = (color [1].green + color [3].green) / 2;
color [0].blue = (color [1].blue + color [3].blue) / 2;
color [0].alpha = (color [1].alpha + color [3].alpha) / 2;
DeleteTriangle (triP); //remove any references to this triangle
if (!(triP = CreateTriangle (triP, index, nFace, nIndex))) //create a new triangle at this location (insert)
	return 0;
triP->nPass = nPass;
triP->nId = (faceP->nTris)++;
memcpy (triP->color, color, sizeof (triP->color));
memcpy (triP->texCoord, texCoord, sizeof (triP->texCoord));
memcpy (triP->ovlTexCoord, ovlTexCoord, sizeof (triP->ovlTexCoord));

index [1] = index [0];
if (!(triP = CreateTriangle (NULL, index + 1, nFace, -1))) //create a new triangle (append)
	return 0;
triP->nPass = nPass;
triP->nId = (faceP->nTris)++;
triP->texCoord [0] = texCoord [0];
triP->ovlTexCoord [0] = ovlTexCoord [0];
triP->color [0] = color [0];
memcpy (triP->color + 1, color + 2, 2 * sizeof (triP->color [0]));
memcpy (triP->texCoord + 1, texCoord + 2, 2 * sizeof (triP->texCoord [0]));
memcpy (triP->ovlTexCoord + 1, ovlTexCoord + 2, 2 * sizeof (triP->ovlTexCoord [0]));
FACES [triP->nFace].nVerts++;
return 1;
}

//------------------------------------------------------------------------------

float CTriMeshBuilder::NewEdgeLen (int nTri, int nVert1, int nVert2)
{
	tTriangle	*triP = m_triangles + nTri;
	int			h, i;

for (i = 0; i < 3; i++) {
	h = triP->index [i];
	if ((h != nVert1) && (h != nVert2))
		break;
	}
return VmVecDist (gameData.segs.fVertices + h, gameData.segs.fVertices + gameData.segs.nVertices);
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::SplitEdge (tEdge *edgeP, short nPass)
{
	int		tris [2];
	ushort	verts [2];

memcpy (tris, edgeP->tris, sizeof (tris));
memcpy (verts, edgeP->verts, sizeof (verts));
VmVecAvg (gameData.segs.fVertices + gameData.segs.nVertices, 
			 gameData.segs.fVertices + verts [0], 
			 gameData.segs.fVertices + verts [1]);
VmVecFloatToFix (gameData.segs.vertices + gameData.segs.nVertices, gameData.segs.fVertices + gameData.segs.nVertices);
#if 0
if (tris [1] >= 0) {
	if (NewEdgeLen (tris [0], verts [0], verts [1]) + NewEdgeLen (tris [1], verts [0], verts [1]) < MAX_EDGE_LEN)
		return -1;
	}
#endif
gameData.segs.nVertices++;
if (!SplitTriangleByEdge (tris [0], verts [0], verts [1], nPass))
	return 0;
if (!SplitTriangleByEdge (tris [1], verts [0], verts [1], nPass))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::SplitTriangle (tTriangle *triP, short nPass)
{
	int	h, i;
	float	l, lMax = 0;

for (i = 0; i < 3; i++) {
	l = m_edges [triP->lines [i]].fLength;
	if (lMax < l) {
		lMax = l;
		h = i;
		}
	}
if (lMax <= MAX_EDGE_LEN)
	return -1;
return SplitEdge (m_edges + triP->lines [h], nPass);
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::SplitTriangles (void)
{
	int	bSplit = 0, h, i, j, nSplitRes;
	short	nPass = 0, nMaxPasses = 10 * gameOpts->render.nMeshQuality;

h = 0;
do {
	bSplit = 0;
	j = m_nTriangles;
	PrintLog ("   splitting triangles (pass %d)\n", nPass);
	for (i = h, h = 0; i < j; i++) {
		if (m_triangles [i].nPass != nPass - 1)
			continue;
#ifdef _DEBUG
		grsFace *faceP = FACES + m_triangles [i].nFace;
		if ((faceP->nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->nSide == nDbgSide)))
			nDbgSeg = nDbgSeg;
#endif
		nSplitRes = SplitTriangle (m_triangles + i, nPass);
		if (gameData.segs.nVertices == 65536)
			return 1;
		if (!nSplitRes)
			return 0;
		if (nSplitRes < 0) 
			m_triangles [i].nPass = -2;
		else {
			bSplit = 1;
			if (!h)
				h = i;
			}
		}
	nPass++;
	} while (bSplit && (nPass < nMaxPasses));
return 1;
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::QSortTriangles (int left, int right)
{
	int	l = left,
			r = right,
			m = m_triangles [(l + r) / 2].nFace;
	short i = m_triangles [(l + r) / 2].nId;

do {
	while ((m_triangles [l].nFace < m) || ((m_triangles [l].nFace == m) && (m_triangles [l].nId < i)))
		l++;
	while ((m_triangles [r].nFace > m) || ((m_triangles [r].nFace == m) && (m_triangles [r].nId > i)))
		r--;
	if (l <= r) {
		if (l < r) {
			tTriangle h = m_triangles [l];
			m_triangles [l] = m_triangles [r];
			m_triangles [r] = h;
			}
		l++;
		r--;
		}	
	} while (l <= r);
if (l < right)
	QSortTriangles (l, right);
if (left < r)
	QSortTriangles (left, r);
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::SetupVertexNormals (void)
{
	grsTriangle		*triP;
	g3sPoint			*pointP;
	int				h, i, nVertex;

for (i = gameData.segs.nVertices, pointP = gameData.segs.points; i; i--, pointP++) {
	pointP->p3_normal.vNormal.v3.p.x = 
	pointP->p3_normal.vNormal.v3.p.y = 
	pointP->p3_normal.vNormal.v3.p.z = 0;
	pointP->p3_normal.nFaces = 0;
	}
for (h = 0, triP = gameData.segs.faces.tris; h < gameData.segs.nTris; h++, triP++) {
	for (i = 0; i < 3; i++) {
		nVertex = triP->index [i];
#ifdef _DEBUG
		if (nVertex == nDbgVertex)
			nVertex = nVertex;
#endif
		VmVecInc (&gameData.segs.points [nVertex].p3_normal.vNormal.v3, gameData.segs.faces.normals + 3 * h);
		gameData.segs.points [nVertex].p3_normal.nFaces++;
		}
	}
ComputeVertexNormals ();
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::InsertTriangles (void)
{
	tTriangle	*triP = m_triangles;
	grsTriangle	*grsTriP = TRIANGLES;
	grsFace		*m_faceP = NULL;
	vmsVector	vNormal;
	int			h, i, nTriVertIndex = 0, nFace = -1;
	GLuint		nIndex = 0;

PrintLog ("   inserting new triangles\n");
QSortTriangles (0, m_nTriangles - 1);
ResetVertexNormals ();
for (h = 0; h < m_nTriangles; h++, triP++, grsTriP++) {
	grsTriP->nFace = triP->nFace;
	if (grsTriP->nFace == nFace) 
		m_faceP->nTris++;
	else {
		if (m_faceP)
			m_faceP++;
		else
			m_faceP = FACES;
		nFace = grsTriP->nFace;
#ifdef _DEBUG
		if (m_faceP - FACES != nFace)
			return 0;
#endif
		m_faceP->nIndex = nIndex;
		m_faceP->nTriIndex = h;
		m_faceP->nTris = 1;
#if USE_RANGE_ELEMENTS
		m_faceP->vertIndex = gameData.segs.faces.vertIndex + nIndex;
#endif
		}
	grsTriP->nIndex = nIndex;
	memcpy (grsTriP->index, triP->index, sizeof (triP->index));
	for (i = 0; i < 3; i++)
		gameData.segs.faces.vertices [nIndex + i] = gameData.segs.fVertices [triP->index [i]].v3;
	VmVecNormal (gameData.segs.faces.normals + nIndex,
					 gameData.segs.faces.vertices + nIndex, 
					 gameData.segs.faces.vertices + nIndex + 1, 
					 gameData.segs.faces.vertices + nIndex + 2);
#ifdef _DEBUG
	if (VmVecMag (gameData.segs.faces.normals + nIndex) == 0)
		m_faceP = m_faceP;
#endif
	VmVecFloatToFix (&vNormal, gameData.segs.faces.normals + nIndex);
	for (i = 1; i < 3; i++)
		gameData.segs.faces.normals [nIndex + i] = gameData.segs.faces.normals [nIndex];
	memcpy (gameData.segs.faces.texCoord + nIndex, triP->texCoord, sizeof (triP->texCoord));
	memcpy (gameData.segs.faces.ovlTexCoord + nIndex, triP->ovlTexCoord, sizeof (triP->ovlTexCoord));
	memcpy (gameData.segs.faces.color + nIndex, triP->color, sizeof (triP->color));
#if USE_RANGE_ELEMENTS
	for (i = 0; i < 3; i++, nIndex++)
		gameData.segs.faces.vertIndex [nIndex] = nIndex;
#else
	nIndex += 3;
#endif
	}
gameData.segs.nTris = m_nTriangles;
SetupVertexNormals ();
FreeData ();
PrintLog ("   created %d new triangles and %d new vertices\n", 
			 m_nTriangles - m_nTris, gameData.segs.nVertices - m_nVertices);
CreateFaceVertLists ();
return 1;
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::SortFaceVertList (ushort *vertList, int left, int right)
{
	int	l = left,
			r = right,
			m = vertList [(l + r) / 2];

do {
	while (vertList [l] < m)
		l++;
	while (vertList [r] > m)
		r--;
	if (l <= r) {
		if (l < r) {
			int h = vertList [l];
			vertList [l] = vertList [r];
			vertList [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortFaceVertList (vertList, l, right);
if (left < r)
	SortFaceVertList (vertList, left, r);
}

//------------------------------------------------------------------------------

void CTriMeshBuilder::CreateFaceVertLists (void)
{
	int			*bTags = new int [gameData.segs.nVertices];
	grsFace		*faceP;
	grsTriangle	*triP;
	int			h, i, j, k, nFace;

//count the vertices of each face
memset (bTags, 0xFF, gameData.segs.nVertices * sizeof (bTags [0]));
gameData.segs.nFaceVerts = 0;
for (i = gameData.segs.nFaces, faceP = FACES, nFace = 0; i; i--, faceP++, nFace++) {
	faceP->nVerts = 0;
	for (j = faceP->nTris, triP = TRIANGLES + faceP->nTriIndex; j; j--, triP++) {
		for (k = 0; k < 3; k++) {
			h = triP->index [k];
			if (bTags [h] != nFace) {
				bTags [h] = nFace;
				faceP->nVerts++;
				gameData.segs.nFaceVerts++;
				}
			}
		}
	}
//insert each face's vertices' indices in the vertex index buffer
memset (bTags, 0xFF, gameData.segs.nVertices * sizeof (bTags [0]));
gameData.segs.nFaceVerts = 0;
for (i = gameData.segs.nFaces, faceP = FACES, nFace = 0; i; i--, faceP++, nFace++) {
	faceP->triIndex = gameData.segs.faces.faceVerts + gameData.segs.nFaceVerts;
#ifdef _DEBUG
	if (faceP->nSegment == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	for (j = faceP->nTris, triP = TRIANGLES + faceP->nTriIndex; j; j--, triP++) {
		for (k = 0; k < 3; k++) {
			h = triP->index [k];
			if (bTags [h] != nFace) {
				bTags [h] = nFace;
				gameData.segs.faces.faceVerts [gameData.segs.nFaceVerts++] = h;
				}
			}
		}
	}
#if 1
//sort each face's vertex index list
for (i = gameData.segs.nFaces, faceP = FACES; i; i--, faceP++)
	SortFaceVertList (faceP->triIndex, 0, faceP->nVerts - 1);
#endif
}

//------------------------------------------------------------------------------

char *CTriMeshBuilder::DataFilename (char *pszFilename, int nLevel)
{
return GameDataFilename (pszFilename, "mesh", nLevel, gameOpts->render.nMeshQuality);
}

//------------------------------------------------------------------------------

bool CTriMeshBuilder::Load (int nLevel)
{
	CFILE					cf;
	tMeshDataHeader	mdh;
	int					nSize;
	bool					bOk;
	char					szFilename [FILENAME_LEN];
	char					*ioBuffer = NULL, *bufP;

if (!(gameStates.render.bTriangleMesh && gameStates.app.bCacheMeshes))
	return false;
if (!CFOpen (&cf, DataFilename (szFilename, nLevel), gameFolders.szTempDir, "rb", 0))
	return false;
bOk = (CFRead (&mdh, sizeof (mdh), 1, &cf) == 1);
if (bOk)
	bOk = (mdh.nVersion == MESH_DATA_VERSION) && 
			(mdh.nSegments == gameData.segs.nSegments) && 
			(mdh.nFaces == gameData.segs.nFaces); 
if (bOk)
	nSize = 
		(sizeof (*gameData.segs.vertices) + 
		 sizeof (*gameData.segs.fVertices)) * mdh.nVertices + 
		sizeof (*gameData.segs.faces.faces) * mdh.nFaces + 
		sizeof (*gameData.segs.faces.tris) * mdh.nTris + 
		(sizeof (*gameData.segs.faces.vertices) + 
		 sizeof (*gameData.segs.faces.normals) +
		 sizeof (*gameData.segs.faces.texCoord) + 
		 sizeof (*gameData.segs.faces.ovlTexCoord) +
		 sizeof (*gameData.segs.faces.color)) * mdh.nTris * 3 +
		 sizeof (*gameData.segs.faces.lMapTexCoord) * mdh.nFaces * 2 + 
		 sizeof (*gameData.segs.faces.faceVerts) * mdh.nFaceVerts;
bOk = ((ioBuffer = (char *) D2_ALLOC (nSize)) != NULL);
if (bOk)
	bOk = CFRead (ioBuffer, nSize, 1, &cf) == 1;
if (bOk) {
	bufP = ioBuffer;
	memcpy (gameData.segs.vertices, bufP, sizeof (*gameData.segs.vertices) * mdh.nVertices);
	bufP += sizeof (*gameData.segs.vertices) * mdh.nVertices;
	memcpy (gameData.segs.fVertices, bufP, sizeof (*gameData.segs.fVertices) * mdh.nVertices);
	bufP += sizeof (*gameData.segs.fVertices) * mdh.nVertices;
	memcpy (gameData.segs.faces.faces, bufP, sizeof (*gameData.segs.faces.faces) * mdh.nFaces);
	bufP += sizeof (*gameData.segs.faces.faces) * mdh.nFaces;
	memcpy (gameData.segs.faces.tris, bufP, sizeof (*gameData.segs.faces.tris) * mdh.nTris);
	bufP += sizeof (*gameData.segs.faces.tris) * mdh.nTris;
	memcpy (gameData.segs.faces.vertices, bufP, sizeof (*gameData.segs.faces.vertices) * mdh.nTris * 3);
	bufP +=  sizeof (*gameData.segs.faces.vertices) * mdh.nTris * 3;
	memcpy (gameData.segs.faces.normals, bufP, sizeof (*gameData.segs.faces.normals) * mdh.nTris * 3);
	bufP += sizeof (*gameData.segs.faces.normals) * mdh.nTris * 3;
	memcpy (gameData.segs.faces.texCoord, bufP, sizeof (*gameData.segs.faces.texCoord) * mdh.nTris * 3);
	bufP += sizeof (*gameData.segs.faces.texCoord) * mdh.nTris * 3;
	memcpy (gameData.segs.faces.ovlTexCoord, bufP, sizeof (*gameData.segs.faces.ovlTexCoord) * mdh.nTris * 3);
	bufP += sizeof (*gameData.segs.faces.ovlTexCoord) * mdh.nTris * 3;
	memcpy (gameData.segs.faces.color, bufP, sizeof (*gameData.segs.faces.color) * mdh.nTris * 3);
	bufP += sizeof (*gameData.segs.faces.color) * mdh.nTris * 3;
	memcpy (gameData.segs.faces.lMapTexCoord, bufP, sizeof (*gameData.segs.faces.lMapTexCoord) * mdh.nFaces * 2);
	bufP += sizeof (*gameData.segs.faces.lMapTexCoord) * mdh.nFaces * 2;
	memcpy (gameData.segs.faces.faceVerts, bufP, sizeof (*gameData.segs.faces.faceVerts) * mdh.nFaceVerts);
	}
if (ioBuffer)
	D2_FREE (ioBuffer);
if (bOk) {
	gameData.segs.nVertices = mdh.nVertices;
	gameData.segs.nTris = mdh.nTris;
	SetupVertexNormals ();
	}
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------

bool CTriMeshBuilder::Save (int nLevel)
{
	CFILE					cf;
	bool					bOk;
	char					szFilename [FILENAME_LEN];
	char					*bufP = NULL;

	tMeshDataHeader mdh = {MESH_DATA_VERSION, 
								  gameData.segs.nSegments, 
								  gameData.segs.nVertices, 
								  gameData.segs.nFaceVerts, 
								  gameData.segs.nFaces, 
								  gameData.segs.nTris};

if (!(gameStates.render.bTriangleMesh && gameStates.app.bCacheMeshes))
	return 0;
if (!CFOpen (&cf, DataFilename (szFilename, nLevel), gameFolders.szTempDir, "wb", 0))
	return 0;
bOk = (CFWrite (&mdh, sizeof (mdh), 1, &cf) == 1) &&
		(CFWrite (gameData.segs.vertices, sizeof (*gameData.segs.vertices) * mdh.nVertices, 1, &cf) == 1) &&
		(CFWrite (gameData.segs.fVertices, sizeof (*gameData.segs.fVertices) * mdh.nVertices, 1, &cf) == 1) &&
		(CFWrite (gameData.segs.faces.faces, sizeof (*gameData.segs.faces.faces) * mdh.nFaces, 1, &cf) == 1) &&
		(CFWrite (gameData.segs.faces.tris, sizeof (*gameData.segs.faces.tris) * mdh.nTris, 1, &cf) == 1) &&
		(CFWrite (gameData.segs.faces.vertices, sizeof (*gameData.segs.faces.vertices) * mdh.nTris, 3, &cf) == 3) &&
		(CFWrite (gameData.segs.faces.normals, sizeof (*gameData.segs.faces.normals) * mdh.nTris, 3, &cf) == 3) &&
		(CFWrite (gameData.segs.faces.texCoord, sizeof (*gameData.segs.faces.texCoord) * mdh.nTris, 3, &cf) == 3) &&
		(CFWrite (gameData.segs.faces.ovlTexCoord, sizeof (*gameData.segs.faces.ovlTexCoord) * mdh.nTris, 3, &cf) == 3) &&
		(CFWrite (gameData.segs.faces.color, sizeof (*gameData.segs.faces.color) * mdh.nTris, 3, &cf) == 3) &&
		(CFWrite (gameData.segs.faces.lMapTexCoord, sizeof (*gameData.segs.faces.lMapTexCoord) * mdh.nFaces, 2, &cf) == 2) &&
		(CFWrite (gameData.segs.faces.faceVerts, sizeof (*gameData.segs.faces.faceVerts) * mdh.nFaceVerts, 1, &cf) == 1);
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------

int CTriMeshBuilder::Build (int nLevel)
{
PrintLog ("creating triangle mesh\n");
if (Load (nLevel))
	return 1;
if (!CreateTriangles ())
	return 0;
if (!SplitTriangles ())
	return 0;
if (!InsertTriangles ())
	return 0;
Save (nLevel);
return 1;
}

//------------------------------------------------------------------------------

int CQuadMeshBuilder::IsBigFace (short *m_sideVerts)
{
for (int i = 0; i < 4; i++) 
	if (VmVecDist (gameData.segs.fVertices + m_sideVerts [i], gameData.segs.fVertices + m_sideVerts [(i + 1) % 4]) > MAX_EDGE_LEN)
		return 1;
return 0;
}

//------------------------------------------------------------------------------

fVector3 *CQuadMeshBuilder::SetTriNormals (grsTriangle *triP, fVector3 *m_normalP)
{
	fVector	vNormalf;

VmVecNormal (&vNormalf, gameData.segs.fVertices + triP->index [0], 
				 gameData.segs.fVertices + triP->index [1], gameData.segs.fVertices + triP->index [2]);
*m_normalP++ = vNormalf.v3;
*m_normalP++ = vNormalf.v3;
*m_normalP++ = vNormalf.v3;
return m_normalP;
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::InitFace (short nSegment, ubyte nSide)
{
	fix	rMin, rMax;

memset (m_faceP, 0, sizeof (*m_faceP));
m_faceP->nSegment = nSegment;
m_faceP->nVerts = 4;
m_faceP->nIndex = m_vertexP - gameData.segs.faces.vertices;
if (gameStates.render.bTriangleMesh)
	m_faceP->nTriIndex = m_triP - TRIANGLES;
memcpy (m_faceP->index, m_sideVerts, sizeof (m_faceP->index));
m_faceP->nType = gameStates.render.bTriangleMesh ? m_sideP->nType : -1;
m_faceP->nSegment = nSegment;
m_faceP->nSide = nSide;
m_faceP->nWall = gameStates.app.bD2XLevel ? m_nWall : IS_WALL (m_nWall) ? m_nWall : (ushort) -1;
m_faceP->bAnimation = IsAnimatedTexture (m_faceP->nBaseTex) || IsAnimatedTexture (m_faceP->nOvlTex);
ComputeSideRads (nSegment, nSide, &rMin, &rMax);
float rMinf = f2fl (rMin);
float rMaxf = f2fl (rMax);
m_faceP->fRad = rMaxf; //(float) sqrt ((rMinf * rMinf + rMaxf * rMaxf) / 2);
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::InitTexturedFace (void)
{
m_faceP->nBaseTex = m_sideP->nBaseTex;
if ((m_faceP->nOvlTex = m_sideP->nOvlTex))
	m_nOvlTexCount++;
m_faceP->bSlide = (gameData.pig.tex.pTMapInfo [m_faceP->nBaseTex].slide_u || gameData.pig.tex.pTMapInfo [m_faceP->nBaseTex].slide_v);
m_faceP->nCamera = IsMonitorFace (m_faceP->nSegment, m_faceP->nSide, 1);
m_faceP->bIsLight = IsLight (m_faceP->nBaseTex) || (m_faceP->nOvlTex && IsLight (m_faceP->nOvlTex));
m_faceP->nOvlOrient = (ubyte) m_sideP->nOvlOrient;
m_faceP->bTextured = 1;
m_faceP->bTransparent = 0;
char *pszName = gameData.pig.tex.bitmapFiles [gameStates.app.bD1Mission][gameData.pig.tex.pBmIndex [m_faceP->nBaseTex].index].name;
m_faceP->bSparks = (strstr (pszName, "misc17") != NULL);
if (m_nWallType < 2)
	m_faceP->bAdditive = 0;
else if (WALLS [m_nWall].flags & WALL_RENDER_ADDITIVE)
	m_faceP->bAdditive = 2;
else if (strstr (pszName, "lava"))
	m_faceP->bAdditive = 2;
else
	m_faceP->bAdditive = (strstr (pszName, "force") || m_faceP->bSparks) ? 1 : 0;
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::InitColoredFace (short nSegment)
{
m_faceP->nBaseTex = -1;
m_faceP->bTransparent = 1;
m_faceP->bAdditive = gameData.segs.segment2s [nSegment].special >= SEGMENT_IS_LAVA;
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::SetupFace (void)
{
#define	LMAP_SIZE	0.0f //(1.0f / 16.0f)

	static tTexCoord2f lMapTexCoord [4] = {
		{{LMAP_SIZE, LMAP_SIZE}}, 
		{{1.0f - LMAP_SIZE, LMAP_SIZE}}, 
		{{1.0f - LMAP_SIZE, 1.0f - LMAP_SIZE}}, 
		{{LMAP_SIZE, 1.0f - LMAP_SIZE}}
	};

	int			i, j;
	vmsVector	vNormal;
	fVector3		vNormalf;

VmVecAdd (&vNormal, m_sideP->normals, m_sideP->normals + 1);
VmVecScale (&vNormal, F1_0 / 2);
VmVecFixToFloat (&vNormalf, &vNormal);
for (i = 0; i < 4; i++) {
	j = m_sideVerts [i];
	*m_vertexP++ = gameData.segs.fVertices [j].v3;
	*m_normalP++ = vNormalf;
	m_texCoordP->v.u = f2fl (m_sideP->uvls [i].u);
	m_texCoordP->v.v = f2fl (m_sideP->uvls [i].v);
	RotateTexCoord2f (m_ovlTexCoordP, m_texCoordP, (ubyte) m_sideP->nOvlOrient);
	m_texCoordP++;
	m_ovlTexCoordP++;
	*m_faceColorP++ = gameData.render.color.ambient [j].color;
	}
i = (m_faceP - gameData.segs.faces.faces) % LIGHTMAP_BUFSIZE;
float x = (float) (i % LIGHTMAP_ROWSIZE);
float y = (float) (i / LIGHTMAP_ROWSIZE);
m_lMapTexCoordP [0].v.u = x / (float) LIGHTMAP_ROWSIZE + 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [0].v.v = y / (float) LIGHTMAP_ROWSIZE + 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [1].v.u = 
m_lMapTexCoordP [2].v.u = (x + 1) / (float) LIGHTMAP_ROWSIZE - 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [1].v.v = y / (float) LIGHTMAP_ROWSIZE + 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [2].v.v = (y + 1) / (float) LIGHTMAP_ROWSIZE - 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [3].v.u = x / (float) LIGHTMAP_ROWSIZE + 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP [3].v.v = (y + 1) / (float) LIGHTMAP_ROWSIZE - 1.0f / (float) (LIGHTMAP_ROWSIZE * LIGHTMAP_WIDTH * 2);
m_lMapTexCoordP += 4;
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::SplitIn2Tris (void)
{
	static short	n2TriVerts [2][2][3] = {{{0,1,2},{0,2,3}},{{0,1,3},{1,2,3}}};

	int	h, i, j, k, v;
	short	*triVertP;

h = (m_sideP->nType == SIDE_IS_TRI_13);
for (i = 0; i < 2; i++, m_triP++) {
	gameData.segs.nTris++;
	m_faceP->nTris++;
	m_triP->nFace = m_faceP - FACES;
	m_triP->nIndex = m_vertexP - gameData.segs.faces.vertices;
	triVertP = n2TriVerts [h][i];
	for (j = 0; j < 3; j++) {
		k = triVertP [j];
		v = m_sideVerts [k];
		m_triP->index [j] = v;
		*m_vertexP++ = gameData.segs.fVertices [v].v3;
		m_texCoordP->v.u = f2fl (m_sideP->uvls [k].u);
		m_texCoordP->v.v = f2fl (m_sideP->uvls [k].v);
		RotateTexCoord2f (m_ovlTexCoordP, m_texCoordP, (ubyte) m_sideP->nOvlOrient);
		m_texCoordP++;
		m_ovlTexCoordP++;
		m_colorP = gameData.render.color.ambient + v;
		*m_faceColorP++ = m_colorP->color;
		}
	m_normalP = SetTriNormals (m_triP, m_normalP);
	}
}

//------------------------------------------------------------------------------

void CQuadMeshBuilder::SplitIn4Tris (void)
{
	static short	n4TriVerts [4][3] = {{0,1,4},{1,2,4},{2,3,4},{3,0,4}};

	fVector		vSide [4];
	tRgbaColorf	color;
	tTexCoord2f	texCoord;
	short			*triVertP;
	int			h, i, j, k, v;

texCoord.v.u = texCoord.v.v = 0;
color.red = color.green = color.blue = color.alpha = 0;
for (i = 0; i < 4; i++) {
	j = (i + 1) % 4;
	texCoord.v.u += f2fl (m_sideP->uvls [i].u + m_sideP->uvls [j].u) / 8;
	texCoord.v.v += f2fl (m_sideP->uvls [i].v + m_sideP->uvls [j].v) / 8;
	h = m_sideVerts [i];
	k = m_sideVerts [j];
	color.red += (gameData.render.color.ambient [h].color.red + gameData.render.color.ambient [k].color.red) / 8;
	color.green += (gameData.render.color.ambient [h].color.green + gameData.render.color.ambient [k].color.green) / 8;
	color.blue += (gameData.render.color.ambient [h].color.blue + gameData.render.color.ambient [k].color.blue) / 8;
	color.alpha += (gameData.render.color.ambient [h].color.alpha + gameData.render.color.ambient [k].color.alpha) / 8;
	}
VmLineLineIntersection (VmVecAvg (vSide, gameData.segs.fVertices + m_sideVerts [0], gameData.segs.fVertices + m_sideVerts [1]),
								VmVecAvg (vSide + 2, gameData.segs.fVertices + m_sideVerts [2], gameData.segs.fVertices + m_sideVerts [3]),
								VmVecAvg (vSide + 1, gameData.segs.fVertices + m_sideVerts [1], gameData.segs.fVertices + m_sideVerts [2]),
								VmVecAvg (vSide + 3, gameData.segs.fVertices + m_sideVerts [3], gameData.segs.fVertices + m_sideVerts [0]),
								gameData.segs.fVertices + gameData.segs.nVertices,
								gameData.segs.fVertices + gameData.segs.nVertices);
VmVecFloatToFix (gameData.segs.vertices + gameData.segs.nVertices, gameData.segs.fVertices + gameData.segs.nVertices);
m_sideVerts [4] = gameData.segs.nVertices++;
m_faceP->nVerts++;
for (i = 0; i < 4; i++, m_triP++) {
	gameData.segs.nTris++;
	m_faceP->nTris++;
	m_triP->nFace = m_faceP - FACES;
	m_triP->nIndex = m_vertexP - gameData.segs.faces.vertices;
	triVertP = n4TriVerts [i];
	for (j = 0; j < 3; j++) {
		k = triVertP [j];
		v = m_sideVerts [k];
		m_triP->index [j] = v;
		*m_vertexP++ = gameData.segs.fVertices [v].v3;
		if (j == 2) {
			m_texCoordP [2] = texCoord;
			m_faceColorP [2] = color;
			}
		else {
			m_texCoordP [j].v.u = f2fl (m_sideP->uvls [k].u);
			m_texCoordP [j].v.v = f2fl (m_sideP->uvls [k].v);
			m_colorP = gameData.render.color.ambient + v;
			m_faceColorP [j] = m_colorP->color;
			}
		RotateTexCoord2f (m_ovlTexCoordP, m_texCoordP + j, (ubyte) m_sideP->nOvlOrient);
		m_ovlTexCoordP++;
		}
	m_normalP = SetTriNormals (m_triP, m_normalP);
	m_texCoordP += 3;
	m_faceColorP += 3;
	}
}

//------------------------------------------------------------------------------

#define FACE_VERTS	6

void CQuadMeshBuilder::Build (int nLevel)
{
m_faceP = FACES;
m_triP = TRIANGLES;
m_vertexP = gameData.segs.faces.vertices;
m_normalP = gameData.segs.faces.normals;
m_texCoordP = gameData.segs.faces.texCoord;
m_ovlTexCoordP = gameData.segs.faces.ovlTexCoord;
m_lMapTexCoordP = gameData.segs.faces.lMapTexCoord;
m_faceColorP = gameData.segs.faces.color;
m_colorP = gameData.render.color.ambient;
m_segP = SEGMENTS;
m_segFaceP = SEGFACES;

	short			nSegment, i;
	ubyte			nSide;
	
gameStates.render.bTriangleMesh = !gameOpts->ogl.bPerPixelLighting && gameOpts->render.nMeshQuality;
gameStates.render.nFacePrimitive = gameStates.render.bTriangleMesh ? GL_TRIANGLES : GL_TRIANGLE_FAN;
if (gameStates.render.bSplitPolys)
	gameStates.render.bSplitPolys = (gameOpts->ogl.bPerPixelLighting || !gameOpts->render.nMeshQuality) ? 1 : -1;
if (gameStates.render.bTriangleMesh)
	CreateCameras ();
PrintLog ("   Creating face list\n");
gameData.segs.nFaces = 0;
gameData.segs.nTris = 0;
for (nSegment = 0; nSegment < gameData.segs.nSegments; nSegment++, m_segP++, m_segFaceP++) {
	m_bColoredSeg = ((gameData.segs.segment2s [nSegment].special >= SEGMENT_IS_WATER) &&
					     (gameData.segs.segment2s [nSegment].special <= SEGMENT_IS_TEAM_RED)) ||
					     (gameData.segs.xSegments [nSegment].group >= 0);
#ifdef _DEBUG
	if (nSegment == nDbgSeg)
		m_faceP = m_faceP;
#endif
	m_faceP->nSegment = nSegment;
	m_nOvlTexCount = 0;
	m_segFaceP->nFaces = 0;
	for (nSide = 0, m_sideP = m_segP->sides; nSide < 6; nSide++, m_sideP++) {
		m_nWall = WallNumI (nSegment, nSide);
		m_nWallType = IS_WALL (m_nWall) ? 2 : (m_segP->children [nSide] == -1) ? 1 : 0;
		if (m_bColoredSeg || m_nWallType) {
#ifdef _DEBUG
			if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
				nDbgSeg = nDbgSeg;
#endif
			GetSideVertIndex (m_sideVerts, nSegment, nSide);
			InitFace (nSegment, nSide);
			if (m_nWallType)
				InitTexturedFace ();
			else if (m_bColoredSeg)
				InitColoredFace (nSegment);
			if (gameStates.render.bTriangleMesh) {
				// split in four triangles, using the quad's center of gravity as additional vertex
				if (!gameOpts->ogl.bPerPixelLighting && (m_sideP->nType == SIDE_IS_QUAD) && !m_faceP->bSlide && (m_faceP->nCamera < 0) && IsBigFace (m_sideVerts))
					SplitIn4Tris ();
				else // split in two triangles, regarding any non-planarity
					SplitIn2Tris ();
				}
			else
				SetupFace ();
#ifdef _DEBUG
			if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
				m_faceP = m_faceP;
#endif
			if (!m_segFaceP->nFaces++)
				m_segFaceP->pFaces = m_faceP;
			m_faceP++;
			gameData.segs.nFaces++;
			}	
		else {
			m_colorP += FACE_VERTS;
			}
		}
	if (!(gameStates.render.bTriangleMesh || gameOpts->ogl.bPerPixelLighting) && 
		 gameStates.ogl.bGlTexMerge && m_nOvlTexCount) { //allow for splitting multi-textured faces into two single textured ones
		gameData.segs.nFaces += m_nOvlTexCount;
		m_faceP += m_nOvlTexCount;
		m_triP += 2;
		m_vertexP += m_nOvlTexCount * FACE_VERTS;
		m_normalP += m_nOvlTexCount * FACE_VERTS;
		m_texCoordP += m_nOvlTexCount * FACE_VERTS;
		m_ovlTexCoordP += m_nOvlTexCount * FACE_VERTS;
		m_faceColorP += m_nOvlTexCount * FACE_VERTS;
		}
	}
for (m_colorP = gameData.render.color.ambient, i = gameData.segs.nVertices; i; i--, m_colorP++)
	if (m_colorP->color.alpha > 1) {
		m_colorP->color.red /= m_colorP->color.alpha;
		m_colorP->color.green /= m_colorP->color.alpha;
		m_colorP->color.blue /= m_colorP->color.alpha;
		m_colorP->color.alpha = 1;
		}
if (!gameOpts->ogl.bPerPixelLighting && gameOpts->render.nMeshQuality)
	m_triMeshBuilder.Build (nLevel);
if (gameStates.render.bTriangleMesh)
	DestroyCameras ();
}

//------------------------------------------------------------------------------
//eof
