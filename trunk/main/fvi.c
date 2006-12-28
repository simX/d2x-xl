/* $Id: fvi.c, v 1.3 2003/10/10 09:36:35 btb Exp $ */
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

#define NEW_FVI_STUFF 1

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inferno.h"
#include "u_mem.h"
#include "error.h"
#include "mono.h"

#include "inferno.h"
#include "fvi.h"
#include "segment.h"
#include "object.h"
#include "wall.h"
#include "laser.h"
#include "rle.h"
#include "robot.h"
#include "piggy.h"
#include "player.h"
#include "gameseg.h"

#define faceType_num(nfaces, face_num, tri_edge) ((nfaces==1)?0:(tri_edge*2 + face_num))

#include "fvi_a.h"

//	-----------------------------------------------------------------------------
//find the point on the specified plane where the line intersects
//returns true if point found, false if line parallel to plane
//new_pnt is the found point on the plane
//plane_pnt & plane_norm describe the plane
//p0 & p1 are the ends of the line
int FindPlaneLineIntersection (vmsVector *new_pnt, vmsVector *plane_pnt, vmsVector *plane_norm, 
										 vmsVector *p0, vmsVector *p1, fix rad)
{
	vmsVector d, w;
	fix num, den, k;

VmVecSub (&d, p1, p0);
VmVecSub (&w, p0, plane_pnt);
num =  VmVecDot (plane_norm, &w);
den = -VmVecDot (plane_norm, &d);
num -= rad;			//move point out by rad
if (!den)
	return 0;
else if (den > 0) {
	if ((num > den) || (-num >> 15 >= den)) //frac greater than one
		return 0;
	}
else {
	if (num < den)
		return 0;
	}
//do check for potenial overflow
if (labs (num) / (f1_0 / 2) >= labs (den))	
	return 0;
k = FixDiv (num, den);
Assert (k <= f1_0);		//should be trapped above
VmVecScaleFrac (&d, num, den);
VmVecAdd (new_pnt, p0, &d);
return 1;
}

//	-----------------------------------------------------------------------------

typedef struct vec2d {
	fix i, j;
} vec2d;

//given largest componant of normal, return i & j
//if largest componant is negative, swap i & j
int ij_table [3][2] =        {
							{2, 1},          //pos x biggest
							{0, 2},          //pos y biggest
							{1, 0},          //pos z biggest
						};

//intersection types
#define IT_NONE 0       //doesn't touch face at all
#define IT_FACE 1       //touches face
#define IT_EDGE 2       //touches edge of face
#define IT_POINT        3       //touches vertex

//	-----------------------------------------------------------------------------
//see if a point is inside a face by projecting into 2d
uint CheckPointToFace (vmsVector *checkP, tSide *s, int iFace, int nv, int *vertList)
{
	vmsVector norm;
	vmsVector t;
	int biggest;
///
	int i, j, nEdge;
	uint edgemask;
	fix check_i, check_j;
	vmsVector *v0, *v1;
	vec2d vEdge, vCheck;
	fix d;

if (gameStates.render.bRendering)
	norm = s->rotNorms [iFace];
else
	norm = s->normals [iFace];
//now do 2d check to see if point is in tSide
//project polygon onto plane by finding largest component of normal
t.p.x = labs (norm.v [0]); 
t.p.y = labs (norm.v [1]); 
t.p.z = labs (norm.v [2]);

if (t.p.x > t.p.y) 
	if (t.p.x > t.p.z) 
		biggest=0; 
	else 
		biggest=2;
	else if (t.p.y > t.p.z) 
		biggest = 1; 
	else 
		biggest=2;
if (norm.v [biggest] > 0) {
	i = ij_table [biggest][0];
	j = ij_table [biggest][1];
	}
else {
	i = ij_table [biggest][1];
	j = ij_table [biggest][0];
	}
//now do the 2d problem in the i, j plane
check_i = checkP->v [i];
check_j = checkP->v [j];
for (nEdge = edgemask = 0; nEdge < nv; nEdge++) {
	if (gameStates.render.bRendering) {
		v0 = &gameData.segs.points [vertList [iFace * 3 + nEdge]].p3_vec;
		v1 = &gameData.segs.points [vertList [iFace * 3 + ((nEdge + 1) % nv)]].p3_vec;
		}
	else {
		v0 = gameData.segs.vertices + vertList [iFace * 3 + nEdge];
		v1 = gameData.segs.vertices + vertList [iFace * 3 + ((nEdge + 1) % nv)];
		}
	vEdge.i = v1->v [i] - v0->v [i];
	vEdge.j = v1->v [j] - v0->v [j];
	vCheck.i = check_i - v0->v [i];
	vCheck.j = check_j - v0->v [j];
	d = FixMul (vCheck.i, vEdge.j) - FixMul (vCheck.j, vEdge.i);
	if (d < 0)              		//we are outside of triangle
		edgemask |= (1 << nEdge);
	}
return edgemask;
}

//	-----------------------------------------------------------------------------
//check if a sphere intersects a face
int CheckSphereToFace (vmsVector *pnt, tSide *s, int iFace, int nv, fix rad, int *vertList)
{
	vmsVector	checkP = *pnt;
	vmsVector	vEdge, vCheck;            //this time, real 3d vectors
	vmsVector	vClosestPoint;
	fix			edgelen, d, dist;
	vmsVector	*v0, *v1, v [2];
	int			iType;
	int			nEdge;
	uint			edgemask;

//now do 2d check to see if point is in side
edgemask = CheckPointToFace (pnt, s, iFace, nv, vertList);
//we've gone through all the sides, are we inside?
if (edgemask == 0)
	return IT_FACE;
//get verts for edge we're behind
for (nEdge = 0; !(edgemask & 1); (edgemask >>= 1), nEdge++)
	;
if (gameStates.render.bRendering) {
	v0 = &gameData.segs.points [vertList [iFace * 3 + nEdge]].p3_vec;
	v1 = &gameData.segs.points [vertList [iFace * 3 + ((nEdge + 1) % nv)]].p3_vec;
	}
else 
	{
	v0 = gameData.segs.vertices + vertList [iFace * 3 + nEdge];
	v1 = gameData.segs.vertices + vertList [iFace * 3 + ((nEdge + 1) % nv)];
	}
//check if we are touching an edge or point
VmVecSub (&vCheck, &checkP, v0);
edgelen = VmVecNormalizedDir (&vEdge, v1, v0);
//find point dist from planes of ends of edge
d = VmVecDot (&vEdge, &vCheck);
if (d + rad < 0) 
	return IT_NONE;                  //too far behind start point
if (d - rad > edgelen) 
	return IT_NONE;    //too far part end point
//find closest point on edge to check point
iType = IT_POINT;
if (d < 0) 
	vClosestPoint = *v0;
else if (d > edgelen) 
	vClosestPoint = *v1;
else {
	iType = IT_EDGE;
	VmVecScaleAdd (&vClosestPoint, v0, &vEdge, d);
	}
dist = VmVecDist (&checkP, &vClosestPoint);
if (dist <= rad)
	return (iType == IT_POINT) ? IT_NONE : iType;
return IT_NONE;
}

//	-----------------------------------------------------------------------------
//returns true if line intersects with face. fills in newP with intersection
//point on plane, whether or not line intersects tSide
//iFace determines which of four possible faces we have
//note: the seg parm is temporary, until the face itself has a point field
int CheckLineToFace (vmsVector *newP, vmsVector *p0, vmsVector *p1, tSegment *segP, int nSide, int iFace, int nv, fix rad)
{
	vmsVector	checkP, vNormal, v1;
	tSide			*sideP = segP->sides + nSide;
	int			vertexList [6];
	int			pli, nFaces, nVertex;

if (gameStates.render.bRendering)
	vNormal = sideP->rotNorms [iFace];
else
	vNormal = sideP->normals [iFace];
if ((SEG_IDX (segP))==-1)
	Error ("nSegment == -1 in CheckLineToFace()");
CreateAbsVertexLists (&nFaces, vertexList, SEG_IDX (segP), nSide);
//use lowest point number
nVertex = vertexList [0];
if (nVertex > vertexList [2])
	nVertex = vertexList [2];
if (nFaces == 1) {
	if (nVertex > vertexList [1])
		nVertex = vertexList [1];
	if (nVertex > vertexList [3])
		nVertex = vertexList [3];
	}
#if 1
if (p1 == p0) {
	v1 = vNormal;
	VmVecNegate (&v1);
	VmVecScale (&v1, rad);
	VmVecInc (&v1, p0);
	rad = 0;
	p1 = &v1;
	}
#endif
//LogErr ("         FindPlaneLineIntersection...");
pli = FindPlaneLineIntersection (newP, 
											gameStates.render.bRendering ? 
											&gameData.segs.points [nVertex].p3_vec : 
											gameData.segs.vertices + nVertex, 
											&vNormal, p0, p1, rad);
//LogErr ("done\n");
if (!pli) 
	return IT_NONE;
checkP = *newP;
//if rad != 0, project the point down onto the plane of the polygon
if (rad)
	VmVecScaleInc (&checkP, &vNormal, -rad);
return CheckSphereToFace (&checkP, sideP, iFace, nv, rad, vertexList);
}

//	-----------------------------------------------------------------------------
//returns the value of a determinant
fix CalcDetValue (vmsMatrix *det)
{
#if 1
	fix	xDet;
//LogErr ("            CalcDetValue (R: %d, %d, %d; F: %d, %d, %d; U: %d, %d, %d)\n", det->rVec.p.x, det->rVec.p.y, det->rVec.p.z, det->fVec.p.x, det->fVec.p.y, det->fVec.p.z, det->uVec.p.x, det->uVec.p.y, det->uVec.p.z);
//LogErr ("               xDet = FixMul (det->rVec.p.x, FixMul (det->uVec.p.y, det->fVec.p.z))\n");
xDet = FixMul (det->rVec.p.x, FixMul (det->uVec.p.y, det->fVec.p.z));
//LogErr ("               xDet -= FixMul (det->rVec.p.x, FixMul (det->uVec.p.z, det->fVec.p.y))\n");
xDet -= FixMul (det->rVec.p.x, FixMul (det->uVec.p.z, det->fVec.p.y));
//LogErr ("               xDet -= FixMul (det->rVec.p.y, FixMul (det->uVec.p.x, det->fVec.p.z))\n");
xDet -= FixMul (det->rVec.p.y, FixMul (det->uVec.p.x, det->fVec.p.z));
//LogErr ("               xDet += FixMul (det->rVec.p.y, FixMul (det->uVec.p.z, det->fVec.p.x))\n");
xDet += FixMul (det->rVec.p.y, FixMul (det->uVec.p.z, det->fVec.p.x));
//LogErr ("               xDet += FixMul (det->rVec.p.z, FixMul (det->uVec.p.x, det->fVec.p.y))\n");
xDet += FixMul (det->rVec.p.z, FixMul (det->uVec.p.x, det->fVec.p.y));
//LogErr ("               xDet -= FixMul (det->rVec.p.z, FixMul (det->uVec.p.y, det->fVec.p.x))\n");
xDet -= FixMul (det->rVec.p.z, FixMul (det->uVec.p.y, det->fVec.p.x));
return xDet;
//LogErr ("             det = %d\n", xDet);
#else
return FixMul (det->rVec.p.x, FixMul (det->uVec.p.y, det->fVec.p.z)) -
		 FixMul (det->rVec.p.x, FixMul (det->uVec.p.z, det->fVec.p.y)) -
		 FixMul (det->rVec.p.y, FixMul (det->uVec.p.x, det->fVec.p.z)) +
		 FixMul (det->rVec.p.y, FixMul (det->uVec.p.z, det->fVec.p.x)) +
	 	 FixMul (det->rVec.p.z, FixMul (det->uVec.p.x, det->fVec.p.y)) -
		 FixMul (det->rVec.p.z, FixMul (det->uVec.p.y, det->fVec.p.x));
#endif
}

//	-----------------------------------------------------------------------------
//computes the parameters of closest approach of two lines
//fill in two parameters, t0 & t1.  returns 0 if lines are parallel, else 1
int CheckLineToLine (fix *t1, fix *t2, vmsVector *p1, vmsVector *v1, vmsVector *p2, vmsVector *v2)
{
	vmsMatrix det;
	fix d, cross_mag2;		//mag squared Cross product

//LogErr ("         VmVecSub\n");
VmVecSub (&det.rVec, p2, p1);
//LogErr ("         VmVecCross\n");
VmVecCross (&det.fVec, v1, v2);
//LogErr ("         VmVecDot\n");
cross_mag2 = VmVecDot (&det.fVec, &det.fVec);
if (!cross_mag2)
	return 0;			//lines are parallel
det.uVec = *v2;
d = CalcDetValue (&det);
if (oflow_check (d, cross_mag2))
	return 0;
//LogErr ("         FixDiv (%d)\n", cross_mag2);
*t1 = FixDiv (d, cross_mag2);
det.uVec = *v1;
//LogErr ("         CalcDetValue\n");
d = CalcDetValue (&det);
if (oflow_check (d, cross_mag2))
	return 0;
//LogErr ("         FixDiv (%d)\n", cross_mag2);
*t2 = FixDiv (d, cross_mag2);
return 1;		//found point
}

#ifdef NEW_FVI_STUFF
int bSimpleFVI = 0;
#else
#define bSimpleFVI 1
#endif

//	-----------------------------------------------------------------------------
//this version is for when the start and end positions both poke through
//the plane of a tSide.  In this case, we must do checks against the edge
//of faces
int SpecialCheckLineToFace (vmsVector *newP, vmsVector *p0, vmsVector *p1, tSegment *segP, 
									 int nSide, int iFace, int nv, fix rad)
{
	vmsVector	move_vec;
	fix			edge_t, move_t, edge_t2, move_t2, closestDist;
	fix			edge_len, move_len;
	int			vertList [6];
	int			h, num_faces, nEdge;
	uint			edgemask;
	vmsVector	*edge_v0, *edge_v1, edge_vec;
	tSide			*sideP = segP->sides + nSide;
	vmsVector	closest_point_edge, closest_point_move;

if (bSimpleFVI) {
	//LogErr ("      CheckLineToFace ...");
	h = CheckLineToFace (newP, p0, p1, segP, nSide, iFace, nv, rad);
	//LogErr ("done\n");
	return h;
	}
//calc some basic stuff
if ((SEG_IDX (segP)) == -1)
	Error ("nSegment == -1 in SpecialCheckLineToFace()");
//LogErr ("      CreateAbsVertexLists ...");
CreateAbsVertexLists (&num_faces, vertList, SEG_IDX (segP), nSide);
//LogErr ("done\n");
VmVecSub (&move_vec, p1, p0);
//figure out which edge(sideP) to check against
//LogErr ("      CheckPointToFace ...\n");
if (!(edgemask = CheckPointToFace (p0, sideP, iFace, nv, vertList))) {
	//LogErr ("      CheckLineToFace ...");
	return CheckLineToFace (newP, p0, p1, segP, nSide, iFace, nv, rad);
	//LogErr ("done\n");
	}
for (nEdge = 0; !(edgemask & 1); edgemask >>= 1, nEdge++)
	;
//LogErr ("      setting edge vertices (%d, %d)...\n", vertList [iFace * 3 + nEdge], vertList [iFace * 3 + ((nEdge + 1) % nv)]);
edge_v0 = gameData.segs.vertices + vertList [iFace * 3 + nEdge];
edge_v1 = gameData.segs.vertices + vertList [iFace * 3 + ((nEdge + 1) % nv)];
//LogErr ("      setting edge vector...\n");
VmVecSub (&edge_vec, edge_v1, edge_v0);
//is the start point already touching the edge?
//first, find point of closest approach of vec & edge
//LogErr ("      getting edge length...\n");
edge_len = VmVecNormalize (&edge_vec);
//LogErr ("      getting move length...\n");
move_len = VmVecNormalize (&move_vec);
//LogErr ("      CheckLineToLine...");
CheckLineToLine (&edge_t, &move_t, edge_v0, &edge_vec, p0, &move_vec);
//LogErr ("done\n");
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
VmVecScaleAdd (&closest_point_edge, edge_v0, &edge_vec, edge_t2);
VmVecScaleAdd (&closest_point_move, p0, &move_vec, move_t2);
//find dist between closest points
//LogErr ("      computing closest dist.p...\n");
closestDist = VmVecDist (&closest_point_edge, &closest_point_move);
//could we hit with this dist?
//note massive tolerance here
if (closestDist < (rad * 15) / 20) {		//we hit.  figure out where
	//now figure out where we hit
	VmVecScaleAdd (newP, p0, &move_vec, move_t-rad);
	return IT_EDGE;
	}
return IT_NONE;			//no hit
}

//	-----------------------------------------------------------------------------
//maybe this routine should just return the distance and let the caller
//decide it it's close enough to hit
//determine if and where a vector intersects with a sphere
//vector defined by p0, p1
//returns dist if intersects, and fills in intp
//else returns 0
int CheckVectorToSphere1(vmsVector *intp, vmsVector *p0, vmsVector *p1, vmsVector *sphere_pos, 
									  fix sphere_rad)
{
	vmsVector d, dn, w, vClosestPoint;
	fix mag_d, dist, wDist, intDist;

//this routine could be optimized if it's taking too much time!

VmVecSub(&d, p1, p0);
VmVecSub(&w, sphere_pos, p0);
mag_d = VmVecCopyNormalize(&dn, &d);
if (mag_d == 0) {
	intDist = VmVecMag(&w);
	*intp = *p0;
	return ((sphere_rad < 0) || (intDist<sphere_rad))?intDist:0;
	}
wDist = VmVecDot(&dn, &w);
if (wDist < 0)		//moving away from tObject
	return 0;
if (wDist > mag_d+sphere_rad)
	return 0;		//cannot hit
VmVecScaleAdd(&vClosestPoint, p0, &dn, wDist);
dist = VmVecDist(&vClosestPoint, sphere_pos);
if (dist < sphere_rad) {
	fix	dist2, radius2, nShorten;

	dist2 = FixMul(dist, dist);
	radius2 = FixMul(sphere_rad, sphere_rad);
	nShorten = fix_sqrt(radius2 - dist2);
	intDist = wDist-nShorten;
	if (intDist > mag_d || intDist < 0) {
		//past one or the other end of vector, which means we're inside
		*intp = *p0;		//don't move at all
		return 1;
		}
	VmVecScaleAdd(intp, p0, &dn, intDist);         //calc intersection point
	return intDist;
	}
return 0;
}

//	-----------------------------------------------------------------------------
//determine if a vector intersects with an tObject
//if no intersects, returns 0, else fills in intp and returns dist
fix CheckVectorToObject (vmsVector *intp, vmsVector *p0, vmsVector *p1, fix rad, 
								 tObject *objP, tObject *otherObjP)
{
	fix size;
	
if (rad < 0)
	size = 0;
else {
	size = objP->size;
	if (objP->nType == OBJ_ROBOT && gameData.bots.pInfo [objP->id].attackType)
		size = (size*3)/4;
	//if obj is tPlayer, and bumping into other tPlayer or a weapon of another coop tPlayer, reduce radius
	if (objP->nType == OBJ_PLAYER &&
			((otherObjP->nType == OBJ_PLAYER) ||
	 		((gameData.app.nGameMode&GM_MULTI_COOP) && otherObjP->nType == OBJ_WEAPON && otherObjP->cType.laserInfo.parentType == OBJ_PLAYER)))
		size = size/2;
	}
return CheckVectorToSphere1 (intp, p0, p1, &objP->position.vPos, size+rad);
}


#define MAX_SEGS_VISITED 100
int nSegsVisited;
short segsVisited [MAX_SEGS_VISITED];

fvi_hit_info fviHitData;

//	-----------------------------------------------------------------------------

int FVICompute (vmsVector *intP, short *intS, vmsVector *p0, short nStartSeg, vmsVector *p1, 
					 fix rad, short nThisObject, short *ignoreObjList, int flags, short *segList, 
					 short *nSegments, int entrySegP);

//Find out if a vector intersects with anything.
//Fills in hitData, an fvi_info structure (see header file).
//Parms:
//  p0 & startseg 	describe the start of the vector
//  p1 					the end of the vector
//  rad 					the radius of the cylinder
//  thisObjNum 		used to prevent an tObject with colliding with itself
//  ingore_obj			ignore collisions with this tObject
//  check_objFlag	determines whether collisions with gameData.objs.objects are checked
//Returns the hitData->nHitType
int FindVectorIntersection (fvi_query *fq, fvi_info *hitData)
{
	int			nHitType, nNewHitType;
	short			nHitSegment, nHitSegment2;
	vmsVector	vHitPoint;
	int			i;
	segmasks		masks;

Assert(fq->ignoreObjList != (short *)(-1));
Assert((fq->startSeg <= gameData.segs.nLastSegment) && (fq->startSeg >= 0));

fviHitData.nSegment = -1;
fviHitData.nSide = -1;
fviHitData.nObject = -1;

//check to make sure start point is in seg its supposed to be in
//Assert(check_point_in_seg(p0, startseg, 0).centerMask==0);	//start point not in seg

// gameData.objs.viewer is not in tSegment as claimed, so say there is no hit.
masks = GetSegMasks (fq->p0, fq->startSeg, 0);
if (masks.centerMask) {
	hitData->hit.nType = HIT_BAD_P0;
	hitData->hit.vPoint = *fq->p0;
	hitData->hit.nSegment = fq->startSeg;
	hitData->hit.nSide = 0;
	hitData->hit.nObject = 0;
	hitData->hit.nSideSegment = -1;
	return hitData->hit.nType;
	}
segsVisited [0] = fq->startSeg;
nSegsVisited = 1;
fviHitData.nNestCount = 0;
nHitSegment2 = fviHitData.nSegment2 = -1;
nHitType = FVICompute (&vHitPoint, &nHitSegment2, fq->p0, (short) fq->startSeg, fq->p1, fq->rad, 
							  (short) fq->thisObjNum, fq->ignoreObjList, fq->flags, 
							  hitData->segList, &hitData->nSegments, -2);
//!!nHitSegment = FindSegByPoint(&vHitPoint, fq->startSeg);
if ((nHitSegment2 != -1) && !GetSegMasks (&vHitPoint, nHitSegment2, 0).centerMask)
	nHitSegment = nHitSegment2;
else {
	nHitSegment = FindSegByPoint (&vHitPoint, fq->startSeg);
	}
//MATT: TAKE OUT THIS HACK AND FIX THE BUGS!
if ((nHitType == HIT_WALL) && (nHitSegment == -1))
	if ((fviHitData.nSegment2 != -1) && !GetSegMasks (&vHitPoint, fviHitData.nSegment2, 0).centerMask)
		nHitSegment = fviHitData.nSegment2;

if (nHitSegment == -1) {
	//int nNewHitType;
	short nNewHitSeg2=-1;
	vmsVector vNewHitPoint;

	//because of code that deal with tObject with non-zero radius has
	//problems, try using zero radius and see if we hit a wall
	nNewHitType = FVICompute (&vNewHitPoint, &nNewHitSeg2, fq->p0, (short) fq->startSeg, fq->p1, 0, 
								     (short) fq->thisObjNum, fq->ignoreObjList, fq->flags, hitData->segList, 
									  &hitData->nSegments, -2);
	if (nNewHitSeg2 != -1) {
		nHitType = nNewHitType;
		nHitSegment = nNewHitSeg2;
		vHitPoint = vNewHitPoint;
		}
	}

if ((nHitSegment != -1) && (fq->flags & FQ_GET_SEGLIST))
	if ((nHitSegment != hitData->segList [hitData->nSegments - 1]) && 
		 (hitData->nSegments < MAX_FVI_SEGS - 1))
		hitData->segList [hitData->nSegments++] = nHitSegment;

if ((nHitSegment != -1) && (fq->flags & FQ_GET_SEGLIST))
	for (i = 0; i < (hitData->nSegments) && i < (MAX_FVI_SEGS - 1); i++)
		if (hitData->segList [i] == nHitSegment) {
			hitData->nSegments = i + 1;
			break;
		}
Assert ((nHitType != HIT_OBJECT) || (fviHitData.nObject != -1));
hitData->hit = fviHitData;
hitData->hit.nType = nHitType;
hitData->hit.vPoint = vHitPoint;
hitData->hit.nSegment = nHitSegment;
return nHitType;
}


//	-----------------------------------------------------------------------------

int ObjectInList(short nObject, short *obj_list)
{
	short t;

	while ((t=*obj_list)!=-1 && t!=nObject) 
		obj_list++;

	return (t==nObject);

}

//	-----------------------------------------------------------------------------

#define FVI_NEWCODE 2

int CheckTransWall (vmsVector *pnt, tSegment *seg, short nSide, short iFace);

int FVICompute (vmsVector *vIntP, short *intS, vmsVector *p0, short nStartSeg, vmsVector *p1, 
					 fix rad, short nThisObject, short *ignoreObjList, int flags, short *segList, 
					 short *nSegments, int entrySegP)
{
	tSegment		*segP;				//the tSegment we're looking at
	int			startMask, endMask, centerMask;	//mask of faces
	//@@int sideMask;				//mask of sides - can be on back of face but not tSide
	short			nObject;
	segmasks		masks;
	vmsVector	vHitPoint, vClosestHitPoint; 	//where we hit
	fix			d, dMin = 0x7fffffff;					//distance to hit point
	int			nHitType = HIT_NONE;							//what sort of hit
	int			nHitSegment = -1;
	int			nHitNoneSegment = -1;
	int			nHitNoneSegs = 0;
	int			hitNoneSegList [MAX_FVI_SEGS];
	int			nCurNestLevel = fviHitData.nNestCount;
#if FVI_NEWCODE
	int			nFudgedRad;
	int			nThisType, nOtherType;
	tObject		*otherObjP,
					*thisObjP = (nThisObject < 0) ? NULL : gameData.objs.objects + nThisObject;
#endif
//LogErr ("Entry FVICompute\n");
if (flags & FQ_GET_SEGLIST)
	*segList = nStartSeg;
*nSegments = 1;
segP = gameData.segs.segments + nStartSeg;
fviHitData.nNestCount++;
//first, see if vector hit any objects in this tSegment
#if !FVI_NEWCODE
if (flags & FQ_CHECK_OBJS)
	for (nObject = segP->objects; nObject != -1; nObject = gameData.objs.objects [nObject].next)
		if (!(gameData.objs.objects [nObject].flags & OF_SHOULD_BE_DEAD) &&
				!(nThisObject == nObject) &&
				(ignoreObjList==NULL || !ObjectInList(nObject, ignoreObjList)) &&
				!LasersAreRelated (nObject, nThisObject) &&
				!((nThisObject > -1) &&
				(CollisionResult [gameData.objs.objects [nThisObject].nType][gameData.objs.objects [nObject].nType] == RESULT_NOTHING) &&
			 	(CollisionResult [gameData.objs.objects [nObject].nType][gameData.objs.objects [nThisObject].nType] == RESULT_NOTHING))) {
			int nFudgedRad = rad;

			//	If this is a powerup, don't do collision if flag FQ_IGNORE_POWERUPS is set
			if (gameData.objs.objects [nObject].nType == OBJ_POWERUP)
				if (flags & FQ_IGNORE_POWERUPS)
					continue;
			//	If this is a robot:robot collision, only do it if both of them have attackType != 0 (eg, green guy)
			if (gameData.objs.objects [nThisObject].nType == OBJ_ROBOT) {
				if (gameData.objs.objects [nObject].nType == OBJ_ROBOT)
					// -- MK: 11/18/95, 4claws glomming together...this is easy.  -- if (!(gameData.bots.pInfo [gameData.objs.objects [nObject].id].attackType && gameData.bots.pInfo [gameData.objs.objects [nThisObject].id].attackType))
						continue;
				if (gameData.bots.pInfo [gameData.objs.objects [nThisObject].id].attackType)
					nFudgedRad = (rad*3)/4;
					}
			//if obj is tPlayer, and bumping into other tPlayer or a weapon of another coop tPlayer, reduce radius
			if (gameData.objs.objects [nThisObject].nType == OBJ_PLAYER &&
					((gameData.objs.objects [nObject].nType == OBJ_PLAYER) ||
					((gameData.app.nGameMode&GM_MULTI_COOP) &&  gameData.objs.objects [nObject].nType == OBJ_WEAPON && gameData.objs.objects [nObject].cType.laserInfo.parentType == OBJ_PLAYER)))
				nFudgedRad = rad/2;	//(rad*3)/4;

			d = CheckVectorToObject (&vHitPoint, p0, p1, nFudgedRad, gameData.objs.objects + nObject, 
												&gameData.objs.objects [nThisObject]);

			if (d)          //we have intersection
				if (d < dMin) {
					fviHitData.nObject = nObject;
					Assert(fviHitData.nObject!=-1);
					dMin = d;
					vClosestHitPoint = vHitPoint;
					nHitType = HIT_OBJECT;
				}
		}

if ((nThisObject > -1) && (CollisionResult [gameData.objs.objects [nThisObject].nType][OBJ_WALL] == RESULT_NOTHING))
	rad = 0;		//HACK - ignore when edges hit walls
#else
nThisType = (nThisObject < 0) ? -1 : gameData.objs.objects [nThisObject].nType;
if (flags & FQ_CHECK_OBJS) {
	//LogErr ("   checking objects...");
	for (nObject = segP->objects; nObject != -1; nObject = otherObjP->next) {
		otherObjP = gameData.objs.objects + nObject;
		nOtherType = otherObjP->nType;
		if (otherObjP->flags & OF_SHOULD_BE_DEAD)
			continue;
		if (nThisObject == nObject)
			continue;
		if (ignoreObjList && ObjectInList (nObject, ignoreObjList))
			continue;
		if (LasersAreRelated (nObject, nThisObject))
			continue;
		if (nThisObject > -1) {
			if ((CollisionResult [nThisType][nOtherType] == RESULT_NOTHING) &&
				 (CollisionResult [nOtherType][nThisType] == RESULT_NOTHING))
				continue;
			}
		nFudgedRad = rad;

		//	If this is a powerup, don't do collision if flag FQ_IGNORE_POWERUPS is set
		if ((nOtherType == OBJ_POWERUP) && (flags & FQ_IGNORE_POWERUPS))
			continue;
		//	If this is a robot:robot collision, only do it if both of them have attackType != 0 (eg, green guy)
		if (nThisType == OBJ_ROBOT) {
			if (nOtherType == OBJ_ROBOT)
				continue;
			if (gameData.bots.pInfo [thisObjP->id].attackType)
				nFudgedRad = (rad * 3) / 4;
			}
		//if obj is tPlayer, and bumping into other tPlayer or a weapon of another coop tPlayer, reduce radius
		if ((nThisType == OBJ_PLAYER )&&
			 ((nOtherType == OBJ_PLAYER) ||
			  (IsCoopGame && (nOtherType == OBJ_WEAPON) && (otherObjP->cType.laserInfo.parentType == OBJ_PLAYER))))
			nFudgedRad = rad / 2;

		d = CheckVectorToObject (&vHitPoint, p0, p1, nFudgedRad, otherObjP, thisObjP);

		if (d && (d < dMin)) {
			fviHitData.nObject = nObject;
			Assert(fviHitData.nObject != -1);
			dMin = d;
			vClosestHitPoint = vHitPoint;
			nHitType = HIT_OBJECT;
			}
		}
	//LogErr ("done\n");
	}

if ((nThisObject > -1) && (CollisionResult [nThisType][OBJ_WALL] == RESULT_NOTHING))
	rad = 0;		//HACK - ignore when edges hit walls
#endif
//now, check tSegment walls
startMask = GetSegMasks (p0, nStartSeg, (p1 == NULL) ? 0 : rad).faceMask;
masks = GetSegMasks (p1, nStartSeg, rad);    //on back of which faces?
if (!(centerMask = masks.centerMask))
	nHitNoneSegment = nStartSeg;
if (endMask = masks.faceMask) { //on the back of at least one face
	short tSide, face, bit;

	//for each face we are on the back of, check if intersected
	for (tSide = 0, bit = 1; (tSide < 6) && (endMask >= bit); tSide++) {
		int nFaces = GetNumFaces (segP->sides + tSide);
		if (!nFaces)
			nFaces = 1;
		// commented out by mk on 02/13/94:: if ((nFaces=segP->sides [tSide].nFaces)==0) nFaces=1;
		for (face = 0; face < 2; face++, bit <<= 1) {
			if (endMask & bit) {            //on the back of this face
				int nFaceHitType;      //in what way did we hit the face?
				if (segP->children [tSide] == entrySegP)
					continue;		//don't go back through entry tSide
				//did we go through this wall/door?
				if (startMask & bit)	{	//start was also though.  Do extra check
					//LogErr ("   SpecialCheckLineToFace...");
					nFaceHitType = SpecialCheckLineToFace (&vHitPoint, p0, p1, segP, tSide, face, 5 - nFaces, rad);
					//LogErr ("done\n");
					}
				else {
					//NOTE LINK TO ABOVE!!
					//LogErr ("   CheckLineToFace...");
					nFaceHitType = CheckLineToFace (&vHitPoint, p0, p1, segP, tSide, face, 5 - nFaces, rad);
					//LogErr ("done\n");
					}
				//LogErr ("   nFaceHitType = %d\n", nFaceHitType);
				if (nFaceHitType) { //through this wall/door
					int widFlag;
					//LogErr ("   WALL_IS_DOORWAY...");
					widFlag = WALL_IS_DOORWAY (segP, tSide, gameData.objs.objects + nThisObject);
					//LogErr ("done\n");
					//if what we have hit is a door, check the adjoining segP
					if ((nThisObject == gameData.multi.players [gameData.multi.nLocalPlayer].nObject) && 
						 (gameStates.app.cheats.bPhysics == 0xBADA55)) {
						int childSide = segP->children [tSide];
						if (childSide >= 0) {
							int special = gameData.segs.segment2s [childSide].special;
							if (((special != SEGMENT_IS_BLOCKED) && (special != SEGMENT_IS_SKYBOX)) ||
								 (gameData.objs.speedBoost [nThisObject].bBoosted &&
								  ((gameData.segs.segment2s [nStartSeg].special != SEGMENT_IS_SPEEDBOOST) ||
								   (special == SEGMENT_IS_SPEEDBOOST))))
 								widFlag |= WID_FLY_FLAG;
							}
						}

					if ((widFlag & WID_FLY_FLAG) ||
						 (((widFlag & (WID_RENDER_FLAG | WID_RENDPAST_FLAG)) == (WID_RENDER_FLAG | WID_RENDPAST_FLAG)) &&
						  ((flags & FQ_TRANSWALL) || ((flags & FQ_TRANSPOINT) && CheckTransWall (&vHitPoint, segP, tSide, face))))) {

						int			i, nNewSeg, subHitType;
						short			subHitSeg, nSaveHitObj = fviHitData.nObject;
						vmsVector	subHitPoint, vSaveWallNorm = fviHitData.vNormal;

						//do the check recursively on the next tSegment.p.
						nNewSeg = segP->children [tSide];
						//LogErr ("   check next seg (%d)\n", nNewSeg);
						for (i = 0; i < nSegsVisited && (nNewSeg != segsVisited [i]); i++)
							;
						if (i == nSegsVisited) {                //haven't visited here yet
							short tempSegList [MAX_FVI_SEGS], nTempSegs;
							if (nSegsVisited >= MAX_SEGS_VISITED)
								goto quit_looking;		//we've looked a long time, so give up
							segsVisited [nSegsVisited++] = nNewSeg;
							subHitType = FVICompute (&subHitPoint, &subHitSeg, p0, (short) nNewSeg, 
															 p1, rad, nThisObject, ignoreObjList, flags, 
															 tempSegList, &nTempSegs, nStartSeg);
							if (subHitType != HIT_NONE) {
								d = VmVecDist (&subHitPoint, p0);
								if (d < dMin) {
									dMin = d;
									vClosestHitPoint = subHitPoint;
									nHitType = subHitType;
									if (subHitSeg != -1) 
										nHitSegment = subHitSeg;
									//copy segList
									if (flags & FQ_GET_SEGLIST) {
#if FVI_NEWCODE != 2
										int i;
										for (i = 0; (i < nTempSegs) && (*nSegments < MAX_FVI_SEGS - 1); i++)
											segList [(*nSegments)++] = tempSegList [i];
#else
										int i = MAX_FVI_SEGS - 1 - *nSegments;
										if (i > nTempSegs)
											i = nTempSegs;
										//LogErr ("   segList <- tempSegList ...");
										memcpy (segList + *nSegments, tempSegList, i * sizeof (*segList));
										//LogErr ("done\n");
										*nSegments += i;
#endif
										}
									Assert (*nSegments < MAX_FVI_SEGS);
									}
								else {
									fviHitData.vNormal = vSaveWallNorm;     //global could be trashed
									fviHitData.nObject = nSaveHitObj;
 									}
								}
							else {
								fviHitData.vNormal = vSaveWallNorm;     //global could be trashed
								if (subHitSeg != -1) 
									nHitNoneSegment = subHitSeg;
								//copy segList
								if (flags&FQ_GET_SEGLIST) {
#if FVI_NEWCODE != 2
									int i;
									for (i = 0; (i < nTempSegs) && (i < MAX_FVI_SEGS - 1); i++)
										hitNoneSegList [i] = tempSegList [i];
#else
									int i = MAX_FVI_SEGS - 1;
									if (i > nTempSegs)
										i = nTempSegs;
									//LogErr ("   hitNoneSegList <- tempSegList ...");
									memcpy (hitNoneSegList, tempSegList, i * sizeof (*hitNoneSegList));
									//LogErr ("done\n");
#endif
									}
								nHitNoneSegs = nTempSegs;
								}
							}
						}
					else {          //a wall
						//is this the closest hit?
						d = VmVecDist (&vHitPoint, p0);
						if (d < dMin) {
							dMin = d;
							vClosestHitPoint = vHitPoint;
							nHitType = HIT_WALL;
							fviHitData.vNormal = segP->sides [tSide].normals [face];	
							if (!GetSegMasks (&vHitPoint, nStartSeg, rad).centerMask)
								nHitSegment = nStartSeg;             //hit in this tSegment
							else
								fviHitData.nSegment2 = nStartSeg;
							fviHitData.nSegment = nHitSegment;
							fviHitData.nSide = tSide;
							fviHitData.nSideSegment = nStartSeg;
							}
						}
					}
				}
			}
		}
	}
quit_looking:
	;

if (nHitType == HIT_NONE) {     //didn't hit anything, return end point
	int i;

	*vIntP = *p1;
	*intS = nHitNoneSegment;
	if (nHitNoneSegment != -1) {			//(centerMask == 0)
		if (flags & FQ_GET_SEGLIST) {
#if FVI_NEWCODE != 2
			for (i=0;i<nHitNoneSegs && *nSegments<MAX_FVI_SEGS-1;)
				segList [(*nSegments)++] = hitNoneSegList [i++];
#else
			i = MAX_FVI_SEGS - 1 - *nSegments;
			if (i > nHitNoneSegs)
				i = nHitNoneSegs;
			//LogErr ("   segList <- hitNoneSegList ...");
			memcpy (segList + *nSegments, hitNoneSegList, i * sizeof (*segList));
			//LogErr ("done\n");
			*nSegments += i;
#endif
			}
		}
	else
		if (nCurNestLevel!=0)
			*nSegments=0;
	}
else {
	*vIntP = vClosestHitPoint;
	if (nHitSegment == -1)
		if (fviHitData.nSegment2 != -1)
			*intS = fviHitData.nSegment2;
		else
			*intS = nHitNoneSegment;
	else
		*intS = nHitSegment;
	}
Assert(!(nHitType==HIT_OBJECT && fviHitData.nObject==-1));
//LogErr ("Exit FVICompute\n");
return nHitType;
}

#include "textures.h"
#include "texmerge.h"

#define Cross(v0, v1) (FixMul((v0)->i, (v1)->j) - FixMul((v0)->j, (v1)->i))

//	-----------------------------------------------------------------------------
//finds the uv coords of the given point on the given seg & tSide
//fills in u & v. if l is non-NULL fills it in also
void FindHitPointUV (fix *u, fix *v, fix *l, vmsVector *pnt, tSegment *seg, int nSide, int iFace)
{
	vmsVector	*pnt_array;
	vmsVector	normal_array;
	int			nSegment = SEG_IDX (seg);
	int			num_faces;
	int			biggest, ii, jj;
	tSide			*sideP = &seg->sides [nSide];
	int			vertList [6], vertnum_list [6];
 	vec2d			p1, vec0, vec1, checkP;	
	uvl			uvls [3];
	fix			k0, k1;
	int			h, i;

//do lasers pass through illusory walls?
//when do I return 0 & 1 for non-transparent walls?
if ((nSegment < 0) || (nSegment > gameData.segs.nLastSegment)) {
#if TRACE
	con_printf (CON_DEBUG, "Bad nSegment (%d) in FindHitPointUV()\n", nSegment);
#endif
	*u = *v = 0;
	return;
	}
if (nSegment == -1) {
	Error ("nSegment == -1 in FindHitPointUV()");
	return;
	}
CreateAbsVertexLists (&num_faces, vertList, nSegment, nSide);
CreateAllVertNumLists (&num_faces, vertnum_list, nSegment, nSide);
//now the hard work.
//1. find what plane to project this wall onto to make it a 2d case
memcpy (&normal_array, sideP->normals + iFace, sizeof (vmsVector));
biggest = 0;
if (abs (normal_array.v [1]) > abs (normal_array.v [biggest])) 
	biggest = 1;
if (abs (normal_array.v [2]) > abs (normal_array.v [biggest])) 
	biggest = 2;
ii = (biggest == 0);
jj = (biggest == 2) ? 1 : 2;
//2. compute u, v of intersection point
//vec from 1 -> 0
h = iFace * 3;
pnt_array = (vmsVector *) (gameData.segs.vertices + vertList [h+1]);
p1.i = pnt_array->v [ii];
p1.j = pnt_array->v [jj];

pnt_array = (vmsVector *) (gameData.segs.vertices + vertList [h]);
vec0.i = pnt_array->v [ii] - p1.i;
vec0.j = pnt_array->v [jj] - p1.j;

//vec from 1 -> 2
pnt_array = (vmsVector *) (gameData.segs.vertices + vertList [h+2]);
vec1.i = pnt_array->v [ii] - p1.i;
vec1.j = pnt_array->v [jj] - p1.j;

//vec from 1 -> checkPoint
pnt_array = (vmsVector *)pnt;
checkP.i = pnt_array->v [ii];
checkP.j = pnt_array->v [jj];

k1 = -FixDiv (Cross (&checkP, &vec0) + Cross (&vec0, &p1), Cross (&vec0, &vec1));
if (abs(vec0.i) > abs(vec0.j))
	k0 = FixDiv (FixMul (-k1, vec1.i) + checkP.i - p1.i, vec0.i);
else
	k0 = FixDiv (FixMul (-k1, vec1.j) + checkP.j - p1.j, vec0.j);
for (i = 0; i < 3; i++)
	uvls [i] = sideP->uvls [vertnum_list [h+i]];
*u = uvls [1].u + FixMul (k0, uvls [0].u - uvls [1].u) + FixMul (k1, uvls [2].u - uvls [1].u);
*v = uvls [1].v + FixMul (k0, uvls [0].v - uvls [1].v) + FixMul (k1, uvls [2].v - uvls [1].v);
if (l)
	*l = uvls [1].l + FixMul (k0, uvls [0].l - uvls [1].l) + FixMul (k1, uvls [2].l - uvls [1].l);
}

//	-----------------------------------------------------------------------------

int PixelTranspType (short nTexture, short nOrient, fix u, fix v)
{
	grsBitmap *bmP;
	int bmx, bmy, w, h, offs;
	unsigned char	c;
	tBitmapIndex *bmiP;

//	Assert(WALL_IS_DOORWAY(seg, nSide) == WID_TRANSPARENT_WALL);

bmiP = gameData.pig.tex.pBmIndex + (nTexture);
PIGGY_PAGE_IN (*bmiP, gameStates.app.bD1Data);
bmP = BmOverride (gameData.pig.tex.pBitmaps + bmiP->index);
if (bmP->bm_props.flags & BM_FLAG_RLE)
	bmP = rle_expand_texture (bmP);
w = bmP->bm_props.w;
h = ((bmP->bmType == BM_TYPE_ALT) && BM_FRAMES (bmP)) ? w : bmP->bm_props.h; 
if (nOrient == 0) {
	bmx = ((unsigned) f2i (u * w)) % w;
	bmy = ((unsigned) f2i (v * h)) % h;
	}
else if (nOrient == 1) {
	bmx = ((unsigned) f2i ((F1_0 - v) * w)) % w;
	bmy = ((unsigned) f2i (u * h)) % h;
	}
else if (nOrient == 2) {
	bmx = ((unsigned) f2i ((F1_0 - u) * w)) % w;
	bmy = ((unsigned) f2i ((F1_0 - v) * h)) % h;
	}
else {
	bmx = ((unsigned) f2i (v * w)) % w;
	bmy = ((unsigned) f2i ((F1_0 - u) * h)) % h;
	}
offs = bmy * w + bmx;
if (bmP->bm_props.flags & BM_FLAG_TGA) {
	ubyte *p = bmP->bm_texBuf + offs * 4;
	// check super transparency color
	if ((gameOpts->ogl.bGlTexMerge && gameStates.render.textures.bGlsTexMergeOk) ?
	    (p[3] == 1) : ((p[0] == 120) && (p[1] == 88) && (p[2] == 128)))
		return -1;
	// check alpha
	if (!p[3])
		return 1;
	}
else {
	c = bmP->bm_texBuf[offs];
	if (c == SUPER_TRANSP_COLOR) 
		return -1;
	if (c == TRANSPARENCY_COLOR) 
		return 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------
//check if a particular point on a wall is a transparent pixel
//returns 1 if can pass though the wall, else 0
int CheckTransWall (vmsVector *pnt, tSegment *seg, short nSide, short iFace)
{
	tSide *sideP = seg->sides + nSide;
	fix	u, v;
	int	nTranspType;

//	Assert(WALL_IS_DOORWAY(seg, nSide) == WID_TRANSPARENT_WALL);
//LogErr ("      FindHitPointUV (%d)...", iFace);
FindHitPointUV (&u, &v, NULL, pnt, seg, nSide, iFace);	//	Don't compute light value.
//LogErr ("done\n");
if (sideP->nOvlTex)	{
	//LogErr ("      PixelTranspType...");
	nTranspType = PixelTranspType (sideP->nOvlTex, sideP->nOvlOrient,u, v);
	//LogErr ("done\n");
	if (nTranspType < 0)
		return 1;
	if (!nTranspType)
		return 0;
	}
//LogErr ("      PixelTranspType...");
nTranspType = PixelTranspType (sideP->nBaseTex, 0, u, v) != 0;
//LogErr ("done\n");
return nTranspType;
}

//	-----------------------------------------------------------------------------
//new function for Mike
//note: nSegsVisited must be set to zero before this is called
int SphereIntersectsWall(vmsVector *pnt, int nSegment, fix rad)
{
	int faceMask;
	tSegment *seg;

segsVisited [nSegsVisited++] = nSegment;

faceMask = GetSegMasks(pnt, nSegment, rad).faceMask;

seg = gameData.segs.segments + nSegment;

if (faceMask != 0) {				//on the back of at least one face
	int tSide, bit, face, child, i;
	int nFaceHitType;      //in what way did we hit the face?
	int num_faces, vertList [6];

//for each face we are on the back of, check if intersected
	for (tSide = 0, bit = 1; (tSide < 6) && (faceMask >= bit); tSide++) {
		for (face = 0; face < 2; face++, bit <<= 1) {
			if (faceMask & bit) {            //on the back of this face
				//did we go through this wall/door?
				if ((SEG_IDX (seg))==-1) {
					Error("nSegment == -1 in SphereIntersectsWall()");
					return 0;
					}
				CreateAbsVertexLists(&num_faces, vertList, SEG_IDX (seg), tSide);
				nFaceHitType = CheckSphereToFace(pnt, &seg->sides [tSide], 
									face, ((num_faces==1)?4:3), rad, vertList);
				if (nFaceHitType) {            //through this wall/door
					//if what we have hit is a door, check the adjoining seg
					child = seg->children [tSide];
					for (i = 0; (i < nSegsVisited) && (child != segsVisited [i]); i++)
						;
					if (i == nSegsVisited) {                //haven't visited here yet
						if (!IS_CHILD(child))
							return 1;
						else {
							if (SphereIntersectsWall(pnt, child, rad))
								return 1;
							}
						}
					}
				}
			}
		}
	}
return 0;
}

//	-----------------------------------------------------------------------------
//Returns true if the tObject is through any walls
int ObjectIntersectsWall(tObject *objP)
{
return SphereIntersectsWall(&objP->position.vPos, objP->position.nSegment, objP->size);
}

//	-----------------------------------------------------------------------------
//eof
