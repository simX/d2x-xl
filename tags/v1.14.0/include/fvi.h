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

#ifndef _FVI_H
#define _FVI_H

#include "vecmat.h"
#include "segment.h"
#include "object.h"

//return values for FindVectorIntersection() - what did we hit?
#define HIT_NONE		0		//we hit nothing
#define HIT_WALL		1		//we hit - guess - a CWall
#define HIT_OBJECT	2		//we hit an CObject - which one?  no way to tell...
#define HIT_BAD_P0	3		//start point is not in specified CSegment

#define Cross2D(v0, v1) (FixMul((v0).i, (v1).j) - FixMul((v0).j, (v1).i))

typedef struct vec2d {
	fix i, j;
} vec2d;

extern int ijTable [3][2];

typedef struct tFVIHitInfo {
	int 			nType;						//what sort of intersection
	short 		nSegment;					//what CSegment hit_pnt is in
	short			nSegment2;
	short 		nSide;						//if hit CWall, which CSide
	short			nFace;
	short 		nSideSegment;				//what CSegment the hit CSide is in
	short 		nObject;						//if CObject hit, which CObject
	CFixVector	vPoint;						//where we hit
	CFixVector 	vNormal;						//if hit CWall, ptr to its surface normal
	int			nNormals;
	int			nNestCount;
} tFVIHitInfo;

//this data structure gets filled in by FindVectorIntersection()
typedef struct tFVIData {
	tFVIHitInfo	hit;
	short 		nSegments;					//how many segs we went through
	short 		segList [MAX_FVI_SEGS];	//list of segs vector went through
} tFVIData;

//flags for fvi query
#define FQ_CHECK_OBJS		1		//check against objects?
#define FQ_TRANSWALL			2		//go through transparent walls
#define FQ_TRANSPOINT		4		//go through trans CWall if hit point is transparent
#define FQ_GET_SEGLIST		8		//build a list of segments
#define FQ_IGNORE_POWERUPS	16		//ignore powerups
#define FQ_SEE_OBJS			32
#define FQ_ANY_OBJECT		64
#define FQ_CHECK_PLAYER		128

//intersection types
#define IT_ERROR	-1
#define IT_NONE	0       //doesn't touch face at all
#define IT_FACE	1       //touches face
#define IT_EDGE	2       //touches edge of face
#define IT_POINT  3       //touches vertex

//this data contains the parms to fvi()
typedef struct tFVIQuery {
	CFixVector	*p0, *p1;
	short			startSeg;
	fix			radP0, radP1;
	short			thisObjNum;
	short			*ignoreObjList;
	int			flags;
	bool			bCheckVisibility;
} tFVIQuery;

//Find out if a vector intersects with anything.
//Fills in hit_data, an tFVIData structure (see above).
//Parms:
//  p0 & startseg 	describe the start of the vector
//  p1 					the end of the vector
//  rad 					the radius of the cylinder
//  thisobjnum 		used to prevent an CObject with colliding with itself
//  ingore_obj_list	NULL, or ptr to a list of objnums to ignore, terminated with -1
//  check_objFlag	determines whether collisions with objects are checked
//Returns the hit_data->hitType
int FindVectorIntersection(tFVIQuery *fq,tFVIData *hit_data);

//finds the uv coords of the given point on the given seg & CSide
//fills in u & v. if l is non-NULL fills it in also
void FindHitPointUV(fix *u,fix *v,fix *l, CFixVector *pnt,CSegment *seg,int nSide,int facenum);

//Returns true if the CObject is through any walls
int ObjectIntersectsWall (CObject *objP);

int PixelTranspType (short nTexture, short nOrient, short nFrame, fix u, fix v);	//-1: supertransp., 0: opaque, 1: transparent

int CheckLineToSegFace (CFixVector *newP, CFixVector *p0, CFixVector *p1, 
							short nSegment, short nSide, short iFace, int nv, fix rad);

int CanSeePoint (CObject *objP, CFixVector *vSource, CFixVector *vDest, short nSegment, fix xRad = 0);

int ObjectToObjectVisibility (CObject *objP1, CObject *objP2, int transType);

int FindPlaneLineIntersection (CFixVector& intersection, CFixVector *vPlanePoint, CFixVector *vPlaneNorm,
										 CFixVector *p0, CFixVector *p1, fix rad);

int CheckLineToLine (fix *t1, fix *t2, CFixVector *p1, CFixVector *v1, CFixVector *p2, CFixVector *v2);

#endif

