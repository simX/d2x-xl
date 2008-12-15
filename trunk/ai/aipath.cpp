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

#include <stdio.h>		//	for printf ()
#include <stdlib.h>		// for d_rand () and qsort ()
#include <string.h>		// for memset ()

#include "inferno.h"
#include "mono.h"
#include "u_mem.h"

#include "error.h"
#include "physics.h"
#include "gameseg.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#define	PARALLAX	0		//	If !0, then special debugging for Parallax eyes enabled.

//	Length in segments of avoidance path
#define	AVOID_SEG_LENGTH	7
//#define _DEBUG
#if !DBG
#	define	PATH_VALIDATION	0
#else
#	define	PATH_VALIDATION	1
#endif

void AIPathSetOrientAndVel (CObject *objP, CFixVector* vGoalPoint, int nPlayerVisibility, CFixVector *vec_to_player);
void MaybeAIPathGarbageCollect (void);
void AIPathGarbageCollect (void);
#if PATH_VALIDATION
void ValidateAllPaths (void);
int ValidatePath (int debugFlag, tPointSeg* pointSegP, int numPoints);
#endif

//	------------------------------------------------------------------------

void CreateRandomXlate (sbyte *xt)
{
	int	i, j;
	sbyte h;

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	xt [i] = i;
for (i = 0; i<MAX_SIDES_PER_SEGMENT; i++) {
	j = (d_rand () * MAX_SIDES_PER_SEGMENT) / (D_RAND_MAX + 1);
	Assert ((j >= 0) && (j < MAX_SIDES_PER_SEGMENT));
	h = xt [j];
	xt [j] = xt [i];
	xt [i] = h;
	}
}

//	-----------------------------------------------------------------------------------------------------------

tPointSeg *InsertTransitPoint (tPointSeg *curSegP, tPointSeg *predSegP, tPointSeg *succSegP, ubyte nConnSide)
{
	CFixVector	vCenter, vPoint;
	short			nSegment;

COMPUTE_SIDE_CENTER (&vCenter, gameData.segs.segments + predSegP->nSegment, nConnSide);
vPoint = predSegP->point - vCenter;
vPoint[X] /= 16;
vPoint[Y] /= 16;
vPoint[Z] /= 16;
curSegP->point = vCenter - vPoint;
nSegment = FindSegByPos (curSegP->point, succSegP->nSegment, 1, 0);
if (nSegment == -1) {
#if TRACE
	con_printf (1, "Warning: point not in ANY CSegment in aipath.c/InsertCenterPoints().\n");
#endif
	curSegP->point = vCenter;
	FindSegByPos (curSegP->point, succSegP->nSegment, 1, 0);
	}
curSegP->nSegment = succSegP->nSegment;
return curSegP;
}

//	-----------------------------------------------------------------------------------------------------------

int OptimizePath (tPointSeg *pointSegP, int nSegs)
{
	int			i, j;
	CFixVector	temp1, temp2;
	fix			dot, mag1, mag2 = 0;

for (i = 1; i < nSegs - 1; i += 2) {
	if (i == 1) {
		temp1 = pointSegP [i].point - pointSegP [i-1].point;
		mag1 = temp1.Mag();
		}
	else {
		temp1 = temp2;
		mag1 = mag2;
		}
	temp2 = pointSegP [i + 1].point - pointSegP [i].point;
	mag2 = temp1.Mag();
	dot = CFixVector::Dot(temp1, temp2);
	if (dot * 9/8 > FixMul (mag1, mag2))
		pointSegP [i].nSegment = -1;
	}
//	Now, scan for points with nSegment == -1
for (i = j = 0; i < nSegs; i++)
	if (pointSegP [i].nSegment != -1)
		pointSegP [j++] = pointSegP [i];
return j;
}

//	-----------------------------------------------------------------------------------------------------------
//	Insert the point at the center of the tSide connecting two segments between the two points.
// This is messy because we must insert into the list.  The simplest (and not too slow) way to do this is to start
// at the end of the list and go backwards.
int InsertCenterPoints (tPointSeg *pointSegP, int numPoints)
{
	int	i, j;

for (i = 0; i < numPoints; i++) {
	j = i + 2;
	InsertTransitPoint (pointSegP + i + 1, pointSegP + i, pointSegP + j, pointSegP [j].nConnSide);
	}
return OptimizePath (pointSegP, numPoints);
}

#ifdef EDITOR
int	SafetyFlag_override = 0;
int	RandomFlag_override = 0;
int	Ai_path_debug=0;
#endif

//	-----------------------------------------------------------------------------------------------------------
//	Move points halfway to outside of segment.
static tPointSeg newPtSegs [MAX_SEGMENTS_D2X];

void MoveTowardsOutside (tPointSeg *ptSegs, int *nPoints, CObject *objP, int bRandom)
{
	int			i, j;
	int			nNewSeg;
	fix			xSegSize;
	int			nSegment;
	CFixVector	a, b, c, d, e;
	CFixVector	vGoalPos;
	int			count;
	int			nTempSeg;
	tFVIQuery	fq;
	tFVIData		hitData;
	int			nHitType;

j = *nPoints;
if (j > MAX_SEGMENTS)
	j = MAX_SEGMENTS;
for (i = 1, --j; i < j; i++) {
	nTempSeg = FindSegByPos (ptSegs [i].point, ptSegs [i].nSegment, 1, 0);
	if (nTempSeg < 0)
		break;
	ptSegs [i].nSegment = nTempSeg;
	nSegment = ptSegs [i].nSegment;

	if (i == 1) {
		a = ptSegs [i].point - ptSegs [i-1].point;
		CFixVector::Normalize(a);
		}
	else
		a = b;
	b = ptSegs [i + 1].point - ptSegs [i].point;
	c = ptSegs [i + 1].point - ptSegs [i-1].point;
	CFixVector::Normalize(b);
	if (abs (CFixVector::Dot(a, b)) > 3*F1_0/4) {
		if (abs (a[Z]) < F1_0/2) {
			if (bRandom) {
				e[X] = (d_rand ()- 16384) / 2;
				e[Y] = (d_rand ()- 16384) / 2;
				e[Z] = abs (e[X]) + abs (e[Y]) + 1;
				CFixVector::Normalize(e);
				}
			else {
				e[X] =
				e[Y] = 0;
				e[Z] = F1_0;
				}
			}
		else {
			if (bRandom) {
				e[Y] = (d_rand ()-16384)/2;
				e[Z] = (d_rand ()-16384)/2;
				e[X] = abs (e[Y]) + abs (e[Z]) + 1;
				CFixVector::Normalize(e);
				}
			else {
				e[X] = F1_0;
				e[Y] =
				e[Z] = 0;
				}
			}
		}
	else {
		d = CFixVector::Cross(a, b);
		e = CFixVector::Cross(c, d);
		CFixVector::Normalize(e);
		}
#if DBG
	if (e.Mag () < F1_0/2)
		Int3 ();
#endif
	xSegSize = CFixVector::Dist (gameData.segs.vertices [gameData.segs.segments [nSegment].verts [0]], 
										 gameData.segs.vertices [gameData.segs.segments [nSegment].verts [6]]);
	if (xSegSize > F1_0*40)
		xSegSize = F1_0*40;
	vGoalPos = ptSegs [i].point + e * (xSegSize/4);
	count = 3;
	while (count) {
		fq.p0					= &ptSegs [i].point;
		fq.startSeg			= ptSegs [i].nSegment;
		fq.p1					= &vGoalPos;
		fq.radP0				=
		fq.radP1				= objP->info.xSize;
		fq.thisObjNum		= OBJ_IDX (objP);
		fq.ignoreObjList	= NULL;
		fq.flags				= 0;
		nHitType = FindVectorIntersection (&fq, &hitData);
		if (nHitType == HIT_NONE)
			count = 0;
		else {
			if ((count == 3) && (nHitType == HIT_BAD_P0))
				return;
			vGoalPos[X] = ((*fq.p0)[X] + hitData.hit.vPoint[X])/2;
			vGoalPos[Y] = ((*fq.p0)[Y] + hitData.hit.vPoint[Y])/2;
			vGoalPos[Z] = ((*fq.p0)[Z] + hitData.hit.vPoint[Z])/2;
			if (!--count)	//	Couldn't move towards outside, that's ok, sometimes things can't be moved.
				vGoalPos = ptSegs [i].point;
			}
		}
	//	Only move towards outside if remained inside CSegment.
	nNewSeg = FindSegByPos (vGoalPos, ptSegs [i].nSegment, 1, 0);
	if (nNewSeg == ptSegs [i].nSegment) {
		newPtSegs [i].point = vGoalPos;
		newPtSegs [i].nSegment = nNewSeg;
		}
	else {
		newPtSegs [i].point = ptSegs [i].point;
		newPtSegs [i].nSegment = ptSegs [i].nSegment;
		}
	}
if (j > 1)
	memcpy (ptSegs + 1, newPtSegs + 1, (j - 1) * sizeof (tPointSeg));
}

//	-----------------------------------------------------------------------------------------------------------
//	Create a path from objP->info.position.vPos to the center of nEndSeg.
//	Return a list of (segment_num, point_locations) at pointSegP
//	Return number of points in *numPoints.
//	if nMaxDepth == -1, then there is no maximum depth.
//	If unable to create path, return -1, else return 0.
//	If randomFlag !0, then introduce randomness into path by looking at sides in random order.  This means
//	that a path between two segments won't always be the same, unless it is unique.p.
//	If bSafeMode is set, then additional points are added to "make sure" that points are reachable.p.  I would
//	like to say that it ensures that the CObject can move between the points, but that would require knowing what
//	the CObject is (which isn't passed, right?) and making fvi calls (slow, right?).  So, consider it the more_or_less_safeFlag.
//	If nEndSeg == -2, then end seg will never be found and this routine will drop out due to depth (xProbably called by CreateNSegmentPath).
int CreatePathPoints (CObject *objP, int nStartSeg, int nEndSeg, tPointSeg *pointSegP, short *numPoints,
							 int nMaxDepth, int bRandom, int bSafeMode, int nAvoidSeg)
{
	short				nCurSeg;
	short				nSide, hSide;
	int				qTail = 0, qHead = 0;
	int				h, i, j;
	sbyte				bVisited [MAX_SEGMENTS_D2X];
	segQueueEntry	segmentQ [MAX_SEGMENTS_D2X];
	short				depth [MAX_SEGMENTS_D2X];
	int				nCurDepth;
	sbyte				randomXlate [MAX_SIDES_PER_SEGMENT];
	tPointSeg		*origPointSegs = pointSegP;
	int				lNumPoints;
	CSegment			*segP;
	CFixVector		vCenter;
	int				nParentSeg, nDestSeg;
	tFVIQuery		fq;
	tFVIData			hitData;
	int				hitType;
	int				bAvoidPlayer;

#if PATH_VALIDATION
ValidateAllPaths ();
#endif

if ((objP->info.nType == OBJ_ROBOT) && (objP->cType.aiInfo.behavior == AIB_RUN_FROM) && (nAvoidSeg != -32767)) {
	bRandom = 1;
	nAvoidSeg = gameData.objs.consoleP->info.nSegment;
	}
bAvoidPlayer = gameData.objs.consoleP->info.nSegment == nAvoidSeg;
if (nMaxDepth == -1)
	nMaxDepth = MAX_PATH_LENGTH;
lNumPoints = 0;
memset (bVisited, 0, sizeof (bVisited [0]) * (gameData.segs.nLastSegment + 1));
memset (depth, 0, sizeof (depth [0]) * (gameData.segs.nLastSegment + 1));
//	If there is a CSegment we're not allowed to visit, mark it.
if (nAvoidSeg != -1) {
	Assert (nAvoidSeg <= gameData.segs.nLastSegment);
	if ((nStartSeg != nAvoidSeg) && (nEndSeg != nAvoidSeg)) {
		bVisited [nAvoidSeg] = 1;
		depth [nAvoidSeg] = 0;
		}
	}

nCurSeg = nStartSeg;
bVisited [nCurSeg] = 1;
nCurDepth = 0;

#if DBG
if (OBJ_IDX (objP) == nDbgObj)
	nDbgObj = nDbgObj;
#endif
if (bRandom)
	CreateRandomXlate (randomXlate);
nCurSeg = nStartSeg;
bVisited [nCurSeg] = 1;
while (nCurSeg != nEndSeg) {
	segP = SEGMENTS + nCurSeg;
	if (bRandom && (d_rand () < 8192))	//create a different xlate at random time intervals
		CreateRandomXlate (randomXlate);

	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		hSide = bRandom ? randomXlate [nSide] : nSide;
		if (!IS_CHILD (segP->children [hSide]))
			continue;
		if (!((WALL_IS_DOORWAY (segP, hSide, NULL) & WID_FLY_FLAG) ||
			  (AIDoorIsOpenable (objP, segP, hSide))))
			continue;
		nDestSeg = segP->children [hSide];
		if (bVisited [nDestSeg])
			continue;
		if (bAvoidPlayer && ((nCurSeg == nAvoidSeg) || (nDestSeg == nAvoidSeg))) {
			COMPUTE_SIDE_CENTER (&vCenter, segP, hSide);
			fq.p0					= &objP->info.position.vPos;
			fq.startSeg			= objP->info.nSegment;
			fq.p1					= &vCenter;
			fq.radP0				=
			fq.radP1				= objP->info.xSize;
			fq.thisObjNum		= OBJ_IDX (objP);
			fq.ignoreObjList	= NULL;
			fq.flags				= 0;
			hitType = FindVectorIntersection (&fq, &hitData);
			if (hitType != HIT_NONE)
				continue;
			}
		if (nDestSeg < 0)
			continue;
		if (nCurSeg < 0)
			continue;
		segmentQ [qTail].start = nCurSeg;
		segmentQ [qTail].end = nDestSeg;
		segmentQ [qTail].nConnSide = (ubyte) hSide;
		bVisited [nDestSeg] = 1;
		depth [qTail++] = nCurDepth + 1;
		if (depth [qTail-1] == nMaxDepth) {
			nEndSeg = segmentQ [qTail-1].end;
			goto pathTooLong;
			}	// end if (depth [...
		}	//	for (nSide.p...

	if (qHead >= qTail) {
		//	Couldn't get to goal, return a path as far as we got, which is probably acceptable to the unparticular caller.
		nEndSeg = segmentQ [qTail-1].end;
		break;
		}
	nCurSeg = segmentQ [qHead].end;
	nCurDepth = depth [qHead];
	qHead++;

pathTooLong: ;
	}	//	while (nCurSeg ...
//	Set qTail to the CSegment which ends at the goal.
while (segmentQ [--qTail].end != nEndSeg)
	if (qTail < 0) {
		*numPoints = lNumPoints;
		return -1;
		}
for (i = qTail; i >= 0; ) {
	nParentSeg = segmentQ [i].start;
	lNumPoints++;
	if (nParentSeg == nStartSeg)
		break;
	while (segmentQ [--i].end != nParentSeg)
		Assert (i >= 0);
	}

if (bSafeMode && ((pointSegP - gameData.ai.pointSegs) + 2 * lNumPoints + 1 >= MAX_POINT_SEGS)) {
	//	Ouch! Cannot insert center points in path.  So return unsafe path.
#if TRACE
	con_printf (CONDBG, "Resetting all paths because of bSafeMode.p.\n");
#endif
	AIResetAllPaths ();
	*numPoints = lNumPoints;
	return -1;
	}
pointSegP->nSegment = nStartSeg;
COMPUTE_SEGMENT_CENTER_I (&pointSegP->point, nStartSeg);
if (bSafeMode)
	lNumPoints *= 2;
j = lNumPoints++;
h = bSafeMode + 1;
for (i = qTail; i >= 0; j -= h) {
	nDestSeg = segmentQ [i].end;
	nParentSeg = segmentQ [i].start;
	pointSegP [j].nSegment = nDestSeg;
	COMPUTE_SEGMENT_CENTER_I (&pointSegP [j].point, nDestSeg);
	pointSegP [j].nConnSide = segmentQ [i].nConnSide;
	if (nParentSeg == nStartSeg)
		break;
	while (segmentQ [--i].end != nParentSeg)
		Assert (qTail >= 0);
	}
if (bSafeMode) {
	for (i = 0; i < lNumPoints - 1; i = j) {
		j = i + 2;
		InsertTransitPoint (pointSegP + i + 1, pointSegP + i, pointSegP + j, pointSegP [j].nConnSide);
		}
	lNumPoints = OptimizePath (pointSegP, lNumPoints);
	}
pointSegP += lNumPoints;

#if PATH_VALIDATION
ValidatePath (2, origPointSegs, lNumPoints);
#endif

#if PATH_VALIDATION
ValidatePath (3, origPointSegs, lNumPoints);
#endif

// -- MK, 10/30/95 -- This code causes apparent discontinuities in the path, moving a point
//	into a new CSegment.  It is not necessarily bad, but it makes it hard to track down actual
//	discontinuity xProblems.
if ((objP->info.nType == OBJ_ROBOT) && ROBOTINFO (objP->info.nId).companion)
	MoveTowardsOutside (origPointSegs, &lNumPoints, objP, 0);

#if PATH_VALIDATION
ValidatePath (4, origPointSegs, lNumPoints);
#endif

*numPoints = lNumPoints;
return 0;
}

int	Last_buddy_polish_path_frame;

//	-------------------------------------------------------------------------------------------------------
//	SmoothPath
//	Takes an existing path and makes it nicer.
//	Drops as many leading points as possible still maintaining direct accessibility
//	from current position to first point.
//	Will not shorten path to fewer than 3 points.
//	Returns number of points.
//	Starting position in pointSegP doesn't change.p.
//	Changed, MK, 10/18/95.  I think this was causing robots to get hung up on walls.
//				Only drop up to the first three points.
int SmoothPath (CObject *objP, tPointSeg *pointSegP, int numPoints)
{
#if 1
return numPoints;
#else
	int			i, nFirstPoint = 0;
	tFVIQuery	fq;
	tFVIData		hitData;
	int			hitType;


if (numPoints <= 4)
	return numPoints;

//	Prevent the buddy from polishing his path twice in one frame, which can cause him to get hung up.  Pretty ugly, huh?
if (ROBOTINFO (objP->info.nId).companion) {
	if (gameData.app.nFrameCount == Last_buddy_polish_path_frame)
		return numPoints;
	Last_buddy_polish_path_frame = gameData.app.nFrameCount;
	}
fq.p0					= &objP->info.position.vPos;
fq.startSeg			= objP->info.nSegment;
fq.radP0				=
fq.radP1				= objP->info.xSize;
fq.thisObjNum		= OBJ_IDX (objP);
fq.ignoreObjList	= NULL;
fq.flags				= 0;
for (i = 0; i < 2; i++) {
	fq.p1 = &pointSegP [i].point;
	hitType = FindVectorIntersection (&fq, &hitData);
	if (hitType != HIT_NONE)
		break;
	nFirstPoint = i + 1;
	}
if (nFirstPoint) {
	//	Scrunch down all the pointSegP.
	for (i = nFirstPoint; i < numPoints; i++)
		pointSegP [i - nFirstPoint] = pointSegP [i];
	}
return numPoints - nFirstPoint;
#endif
}

//	-------------------------------------------------------------------------------------------------------
//	Make sure that there are connections between all segments on path.
//	Note that if path has been optimized, connections may not be direct, so this function is useless, or worse.p.
//	Return true if valid, else return false.p.

#if PATH_VALIDATION

int ValidatePath (int debugFlag, tPointSeg *pointSegP, int numPoints)
{
	int i, nCurSeg, nSide, nNextSeg;

nCurSeg = pointSegP->nSegment;
if ((nCurSeg < 0) || (nCurSeg > gameData.segs.nLastSegment)) {
#if TRACE
	con_printf (CONDBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.pointSegs, numPoints);
#endif
	Int3 ();		//	Contact Mike: Debug trap for elusive, nasty bug.
	return 0;
	}
#if TRACE
if (debugFlag == 999)
	con_printf (CONDBG, "That's curious...\n");
#endif
if (numPoints == 0)
	return 1;
for (i = 1; i < numPoints; i++) {
	nNextSeg = pointSegP [i].nSegment;
	if ((nNextSeg < 0) || (nNextSeg > gameData.segs.nLastSegment)) {
#if TRACE
		con_printf (CONDBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.pointSegs, numPoints);
#endif
		Int3 ();		//	Contact Mike: Debug trap for elusive, nasty bug.
		return 0;
		}
	if (nCurSeg != nNextSeg) {
		for (nSide=0; nSide<MAX_SIDES_PER_SEGMENT; nSide++)
			if (gameData.segs.segments [nCurSeg].children [nSide] == nNextSeg)
				break;
		if (nSide == MAX_SIDES_PER_SEGMENT) {
#if TRACE
			con_printf (CONDBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.pointSegs, numPoints);
#endif
			Int3 ();
			return 0;
			}
		nCurSeg = nNextSeg;
		}
	}
return 1;
}

#endif

//	-----------------------------------------------------------------------------------------------------------

#if PATH_VALIDATION

void ValidateAllPaths (void)
{
	int				i;
	CObject			*objP;
	tAIStaticInfo	*aiP;

FORALL_ROBOT_OBJS (objP, i) {
	aiP = &objP->cType.aiInfo;
	if ((objP->info.controlType == CT_AI) &&
		 (aiP->nHideIndex != -1) && (aiP->nPathLength > 0) &&
		 !ValidatePath (4, &gameData.ai.pointSegs [aiP->nHideIndex], aiP->nPathLength))
		aiP->nPathLength = 0;	//	This allows people to resume without harm...
	}
}
#endif

// -- //	-------------------------------------------------------------------------------------------------------
// -- //	Creates a path from the OBJECTS current CSegment (objP->info.nSegment) to the specified CSegment for the CObject to
// -- //	hide in gameData.ai.localInfo [nObject].nGoalSegment.
// -- //	Sets	objP->cType.aiInfo.nHideIndex, 		a pointer into gameData.ai.pointSegs, the first tPointSeg of the path.
// -- //			objP->cType.aiInfo.nPathLength, 		length of path
// -- //			gameData.ai.freePointSegs				global pointer into gameData.ai.pointSegs array
// -- void create_path (CObject *objP)
// -- {
// -- 	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
// -- 	tAILocalInfo		*ailP = &gameData.ai.localInfo [OBJ_IDX (objP)];
// -- 	int			nStartSeg, nEndSeg;
// --
// -- 	nStartSeg = objP->info.nSegment;
// -- 	nEndSeg = ailP->nGoalSegment;
// --
// -- 	if (nEndSeg == -1)
// -- 		CreateNSegmentPath (objP, 3, -1);
// --
// -- 	if (nEndSeg == -1) {
// -- 		; //con_printf (CONDBG, "Object %i, nHideSegment = -1, not creating path.\n", OBJ_IDX (objP));
// -- 	} else {
// -- 		CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, -1, 0, 0, -1);
// -- 		aiP->nHideIndex = gameData.ai.freePointSegs - gameData.ai.pointSegs;
// -- 		aiP->nCurPathIndex = 0;
// -- #if PATH_VALIDATION
// -- 		ValidatePath (5, gameData.ai.freePointSegs, aiP->nPathLength);
// -- #endif
// -- 		gameData.ai.freePointSegs += aiP->nPathLength;
// -- 		if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
// -- 			//Int3 ();	//	Contact Mike: This is curious, though not deadly. /eip++;g
// -- 			//force_dump_aiObjects_all ("Error in create_path");
// -- 			AIResetAllPaths ();
// -- 		}
// -- 		aiP->PATH_DIR = 1;		//	Initialize to moving forward.
// -- 		aiP->SUBMODE = AISM_HIDING;		//	Pretend we are hiding, so we sit here until bothered.
// -- 	}
// --
// -- 	MaybeAIPathGarbageCollect ();
// --
// -- }

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the OBJECTS current CSegment (objP->info.nSegment) to the specified CSegment for the CObject to
//	hide in gameData.ai.localInfo [nObject].nGoalSegment.
//	Sets	objP->cType.aiInfo.nHideIndex, 		a pointer into gameData.ai.pointSegs, the first tPointSeg of the path.
//			objP->cType.aiInfo.nPathLength, 		length of path
//			gameData.ai.freePointSegs				global pointer into gameData.ai.pointSegs array
//	Change, 10/07/95: Used to create path to gameData.objs.consoleP->info.position.vPos.p.  Now creates path to gameData.ai.vBelievedPlayerPos.p.

void CreatePathToPlayer (CObject *objP, int nMaxDepth, int bSafeMode)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = gameData.ai.localInfo + OBJ_IDX (objP);
	int			nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;

ailP->timePlayerSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.
ailP->nGoalSegment = gameData.ai.nBelievedPlayerSeg;

nStartSeg = objP->info.nSegment;
nEndSeg = ailP->nGoalSegment;

if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, bSafeMode, -1);
	aiP->nPathLength = SmoothPath (objP, gameData.ai.freePointSegs, aiP->nPathLength);
	aiP->nHideIndex = (int) (gameData.ai.freePointSegs - gameData.ai.pointSegs);
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	// -- UNUSED!aiP->SUBMODE = AISM_GOHIDE;		//	This forces immediate movement.
	ailP->mode = AIM_FOLLOW_PATH;
	if (ailP->playerAwarenessType < PA_RETURN_FIRE)
		ailP->playerAwarenessType = 0;		//	If robot too aware of CPlayerData, will set mode to chase
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the CObject's current CSegment (objP->info.nSegment) to CSegment goalseg.
void CreatePathToSegment (CObject *objP, short goalseg, int nMaxDepth, int bSafeMode)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = &gameData.ai.localInfo [OBJ_IDX (objP)];
	short			nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;
ailP->timePlayerSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.
ailP->nGoalSegment = goalseg;
nStartSeg = objP->info.nSegment;
nEndSeg = ailP->nGoalSegment;
if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, bSafeMode, -1);
	aiP->nHideIndex = (int) (gameData.ai.freePointSegs - gameData.ai.pointSegs);
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	// -- UNUSED!aiP->SUBMODE = AISM_GOHIDE;		//	This forces immediate movement.
	if (ailP->playerAwarenessType < PA_RETURN_FIRE)
		ailP->playerAwarenessType = 0;		//	If robot too aware of CPlayerData, will set mode to chase
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the OBJECTS current CSegment (objP->info.nSegment) to the specified CSegment for the CObject to
//	hide in gameData.ai.localInfo [nObject].nGoalSegment
//	Sets	objP->cType.aiInfo.nHideIndex, 		a pointer into gameData.ai.pointSegs, the first tPointSeg of the path.
//			objP->cType.aiInfo.nPathLength, 		length of path
//			gameData.ai.freePointSegs				global pointer into gameData.ai.pointSegs array
void CreatePathToStation (CObject *objP, int nMaxDepth)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = &gameData.ai.localInfo [OBJ_IDX (objP)];
	int			nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;

ailP->timePlayerSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.

nStartSeg = objP->info.nSegment;
nEndSeg = aiP->nHideSegment;


if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, 1, -1);
	aiP->nPathLength = SmoothPath (objP, gameData.ai.freePointSegs, aiP->nPathLength);
	aiP->nHideIndex = (int) (gameData.ai.freePointSegs - gameData.ai.pointSegs);
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	ailP->mode = AIM_FOLLOW_PATH;
	if (ailP->playerAwarenessType < PA_RETURN_FIRE)
		ailP->playerAwarenessType = 0;
	}
MaybeAIPathGarbageCollect ();
}


//	-------------------------------------------------------------------------------------------------------
//	Create a path of length nPathLength for an CObject, stuffing info in aiInfo field.

static int nObject = 0;

void CreateNSegmentPath (CObject *objP, int nPathLength, short nAvoidSeg)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = gameData.ai.localInfo + OBJ_IDX (objP);
	nObject = OBJ_IDX (objP);

if (CreatePathPoints (objP, objP->info.nSegment, -2, gameData.ai.freePointSegs, &aiP->nPathLength, nPathLength, 1, 0, nAvoidSeg) == -1) {
	gameData.ai.freePointSegs += aiP->nPathLength;
	while ((CreatePathPoints (objP, objP->info.nSegment, -2, gameData.ai.freePointSegs, &aiP->nPathLength, --nPathLength, 1, 0, -1) == -1)) {
		Assert (nPathLength);
		}
	}
aiP->nHideIndex = (int) (gameData.ai.freePointSegs - gameData.ai.pointSegs);
aiP->nCurPathIndex = 0;
#if PATH_VALIDATION
ValidatePath (8, gameData.ai.freePointSegs, aiP->nPathLength);
#endif
gameData.ai.freePointSegs += aiP->nPathLength;
if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
	AIResetAllPaths ();
	}
aiP->PATH_DIR = 1;		//	Initialize to moving forward.
ailP->mode = AIM_FOLLOW_PATH;
//	If this robot is visible (nPlayerVisibility is not available) and it's running away, move towards outside with
//	randomness to prevent a stream of bots from going away down the center of a corridor.
if (gameData.ai.localInfo [OBJ_IDX (objP)].nPrevVisibility) {
	if (aiP->nPathLength) {
		int nPoints = aiP->nPathLength;
		MoveTowardsOutside (gameData.ai.pointSegs + aiP->nHideIndex, &nPoints, objP, 1);
		aiP->nPathLength = nPoints;
		}
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------

void CreateNSegmentPathToDoor (CObject *objP, int nPathLength, short nAvoidSeg)
{
CreateNSegmentPath (objP, nPathLength, nAvoidSeg);
}

#define Int3_if (cond) if (!cond) Int3 ();

//	----------------------------------------------------------------------------------------------------

void MoveObjectToGoal (CObject *objP, CFixVector *vGoalPoint, short nGoalSeg)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	int			nSegment;

if (aiP->nPathLength < 2)
	return;
Assert (objP->info.nSegment != -1);
#if DBG
if (objP->info.nSegment != nGoalSeg)
	if (FindConnectedSide (gameData.segs.segments + objP->info.nSegment, gameData.segs.segments + nGoalSeg) == -1) {
		fix dist = FindConnectedDistance (&objP->info.position.vPos, objP->info.nSegment, vGoalPoint, nGoalSeg, 30, WID_FLY_FLAG, 0);
#	if TRACE
		if (gameData.fcd.nConnSegDist > 2)	//	This global is set in FindConnectedDistance
			con_printf (1, "Warning: Object %i hopped across %i segments, a distance of %7.3f.\n", OBJ_IDX (objP), gameData.fcd.nConnSegDist, X2F (dist));
#	endif
		}
#endif
Assert (aiP->nPathLength >= 2);
if (aiP->nCurPathIndex <= 0) {
	if (aiP->behavior == AIB_STATION) {
		CreatePathToStation (objP, 15);
		return;
		}
	aiP->nCurPathIndex = 1;
	aiP->PATH_DIR = 1;
	}
else if (aiP->nCurPathIndex >= aiP->nPathLength - 1) {
	if (aiP->behavior == AIB_STATION) {
		CreatePathToStation (objP, 15);
		if (!aiP->nPathLength) {
			tAILocalInfo	*ailP = &gameData.ai.localInfo [OBJ_IDX (objP)];
			ailP->mode = AIM_IDLING;
			}
		return;
		}
	Assert (aiP->nPathLength != 0);
	aiP->nCurPathIndex = aiP->nPathLength - 2;
	aiP->PATH_DIR = -1;
	}
else
	aiP->nCurPathIndex += aiP->PATH_DIR;
objP->info.position.vPos = *vGoalPoint;
nSegment = FindObjectSeg (objP);
#if TRACE
if (nSegment != nGoalSeg)
	con_printf (1, "Object #%i goal supposed to be in CSegment #%i, but in CSegment #%i\n", OBJ_IDX (objP), nGoalSeg, nSegment);
#endif
if (nSegment == -1) {
	Int3 ();	//	Oops, CObject is not in any CSegment.
				// Contact Mike: This is impossible.p.
	//	Hack, move CObject to center of CSegment it used to be in.
	COMPUTE_SEGMENT_CENTER_I (&objP->info.position.vPos, objP->info.nSegment);
	}
else
	objP->RelinkToSeg (nSegment);
}

// -- too much work -- //	----------------------------------------------------------------------------------------------------------
// -- too much work -- //	Return true if the CObject the companion wants to kill is reachable.p.
// -- too much work -- int attackKillObject (CObject *objP)
// -- too much work -- {
// -- too much work -- 	CObject		*kill_objp;
// -- too much work -- 	tFVIData		hitData;
// -- too much work -- 	int			fate;
// -- too much work -- 	tFVIQuery	fq;
// -- too much work --
// -- too much work -- 	if (gameData.escort.nKillObject == -1)
// -- too much work -- 		return 0;
// -- too much work --
// -- too much work -- 	kill_objp = &OBJECTS [gameData.escort.nKillObject];
// -- too much work --
// -- too much work -- 	fq.p0						= &objP->info.position.vPos;
// -- too much work -- 	fq.startSeg				= objP->info.nSegment;
// -- too much work -- 	fq.p1						= &kill_objP->info.position.vPos;
// -- too much work -- 	fq.rad					= objP->info.xSize;
// -- too much work -- 	fq.thisObjNum			= OBJ_IDX (objP);
// -- too much work -- 	fq.ignoreObjList	= NULL;
// -- too much work -- 	fq.flags					= 0;
// -- too much work --
// -- too much work -- 	fate = FindVectorIntersection (&fq, &hitData);
// -- too much work --
// -- too much work -- 	if (fate == HIT_NONE)
// -- too much work -- 		return 1;
// -- too much work -- 	else
// -- too much work -- 		return 0;
// -- too much work -- }

//	----------------------------------------------------------------------------------------------------------
//	Optimization: If current velocity will take robot near goal, don't change velocity
void AIFollowPath (CObject *objP, int nPlayerVisibility, int nPrevVisibility, CFixVector *vec_to_player)
{
	tAIStaticInfo		*aiP = &objP->cType.aiInfo;

	CFixVector	vGoalPoint, new_vGoalPoint;
	fix			xDistToGoal;
	tRobotInfo	*botInfoP = &ROBOTINFO (objP->info.nId);
	int			forced_break, original_dir, original_index;
	fix			xDistToPlayer;
	short			nGoalSeg;
	tAILocalInfo		*ailP = gameData.ai.localInfo + OBJ_IDX (objP);
	fix			thresholdDistance;


if ((aiP->nHideIndex == -1) || (aiP->nPathLength == 0)) {
	if (ailP->mode == AIM_RUN_FROM_OBJECT) {
		CreateNSegmentPath (objP, 5, -1);
		//--Int3_if ((aiP->nPathLength != 0);
		ailP->mode = AIM_RUN_FROM_OBJECT;
		}
	else {
		CreateNSegmentPath (objP, 5, -1);
		//--Int3_if ((aiP->nPathLength != 0);
		}
	}

if ((aiP->nHideIndex + aiP->nPathLength > gameData.ai.freePointSegs - gameData.ai.pointSegs) && (aiP->nPathLength>0)) {
	Int3 ();	//	Contact Mike: Bad.  Path goes into what is believed to be free space.
	//	This is debugging code.p.  Figure out why garbage collection
	//	didn't compress this CObject's path information.
	AIPathGarbageCollect ();
	//force_dump_aiObjects_all ("Error in AIFollowPath");
	AIResetAllPaths ();
	}

if (aiP->nPathLength < 2) {
	if ((aiP->behavior == AIB_SNIPE) || (ailP->mode == AIM_RUN_FROM_OBJECT)) {
		if (gameData.objs.consoleP->info.nSegment == objP->info.nSegment) {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, -1);			//	Can't avoid CSegment CPlayerData is in, robot is already in it!(That's what the -1 is for)
			//--Int3_if ((aiP->nPathLength != 0);
			}
		else {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, gameData.objs.consoleP->info.nSegment);
				//--Int3_if ((aiP->nPathLength != 0);
			}
		if (aiP->behavior == AIB_SNIPE) {
			if (botInfoP->thief)
				ailP->mode = AIM_THIEF_ATTACK;	//	It gets bashed in CreateNSegmentPath
			else
				ailP->mode = AIM_SNIPE_FIRE;	//	It gets bashed in CreateNSegmentPath
			}
		else {
			ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
			}
		}
	else if (botInfoP->companion == 0) {
		ailP->mode = AIM_IDLING;
		aiP->nPathLength = 0;
		return;
		}
	}

vGoalPoint = gameData.ai.pointSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
nGoalSeg = gameData.ai.pointSegs [aiP->nHideIndex + aiP->nCurPathIndex].nSegment;
xDistToGoal = CFixVector::Dist(vGoalPoint, objP->info.position.vPos);
if (gameStates.app.bPlayerIsDead)
	xDistToPlayer = CFixVector::Dist(objP->info.position.vPos, gameData.objs.viewerP->info.position.vPos);
else
	xDistToPlayer = CFixVector::Dist(objP->info.position.vPos, OBJPOS (gameData.objs.consoleP)->vPos);
	//	Efficiency hack: If far away from CPlayerData, move in big quantized jumps.
if (!(nPlayerVisibility || nPrevVisibility) && (xDistToPlayer > F1_0*200) && !IsMultiGame) {
	if (xDistToGoal < F1_0*2) {
		MoveObjectToGoal (objP, &vGoalPoint, nGoalSeg);
		return;
		}
	else {
		tRobotInfo	*botInfoP = &ROBOTINFO (objP->info.nId);
		fix	xCurSpeed = botInfoP->xMaxSpeed [gameStates.app.nDifficultyLevel]/2;
		fix	xCoverableDist = FixMul (gameData.time.xFrame, xCurSpeed);
		// int	nConnSide = FindConnectedSide (objP->info.nSegment, nGoalSeg);
		//	Only move to goal if allowed to fly through the tSide.p.
		//	Buddy-bot can create paths he can't fly, waiting for player.
		// -- bah, this isn't good enough, buddy will fail to get through any door!if (WALL_IS_DOORWAY (&gameData.segs.segments]objP->info.nSegment], nConnSide) & WID_FLY_FLAG) {
		if (!(botInfoP->companion || botInfoP->thief)) {
			if ((xCoverableDist >= xDistToGoal) || ((d_rand () >> 1) < FixDiv (xCoverableDist, xDistToGoal)))
				MoveObjectToGoal (objP, &vGoalPoint, nGoalSeg);
			return;
			}
		}
	}
//	If running from CPlayerData, only run until can't be seen.
if (ailP->mode == AIM_RUN_FROM_OBJECT) {
	if ((nPlayerVisibility == 0) && (ailP->playerAwarenessType == 0)) {
		fix xVelScale = F1_0 - gameData.time.xFrame/2;
		if (xVelScale < F1_0/2)
			xVelScale = F1_0/2;
		objP->mType.physInfo.velocity *= xVelScale;
		return;
		}
	else if (!(gameData.app.nFrameCount ^ ((OBJ_IDX (objP)) & 0x07))) {		//	Done 1/8 frames.
		//	If CPlayerData on path (beyond point robot is now at), then create a new path.
		tPointSeg	*curPSP = &gameData.ai.pointSegs [aiP->nHideIndex];
		short			nPlayerSeg = gameData.objs.consoleP->info.nSegment;
		int			i;
		//	This is xProbably being done every frame, which is wasteful.
		for (i = aiP->nCurPathIndex; i < aiP->nPathLength; i++) {
			if (curPSP [i].nSegment == nPlayerSeg) {
				CreateNSegmentPath (objP, AVOID_SEG_LENGTH, (nPlayerSeg == objP->info.nSegment) ? -1 : nPlayerSeg);
				Assert (aiP->nPathLength != 0);
				ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
				break;
				}
			}
		if (nPlayerVisibility) {
			ailP->playerAwarenessType = 1;
			ailP->playerAwarenessTime = F1_0;
			}
		}
	}
if (aiP->nCurPathIndex < 0) {
	aiP->nCurPathIndex = 0;
	}
else if (aiP->nCurPathIndex >= aiP->nPathLength) {
	if (ailP->mode == AIM_RUN_FROM_OBJECT) {
		CreateNSegmentPath (objP, AVOID_SEG_LENGTH, gameData.objs.consoleP->info.nSegment);
		ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
		Assert (aiP->nPathLength != 0);
		}
	else {
		aiP->nCurPathIndex = aiP->nPathLength-1;
		}
	}
vGoalPoint = gameData.ai.pointSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
//	If near goal, pick another goal point.
forced_break = 0;		//	Gets set for short paths.
original_dir = aiP->PATH_DIR;
original_index = aiP->nCurPathIndex;
thresholdDistance = FixMul (objP->mType.physInfo.velocity.Mag(), gameData.time.xFrame)*2 + F1_0*2;
new_vGoalPoint = gameData.ai.pointSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
while ((xDistToGoal < thresholdDistance) && !forced_break) {
	//	Advance to next point on path.
	aiP->nCurPathIndex += aiP->PATH_DIR;
	//	See if next point wraps past end of path (in either direction), and if so, deal with it based on mode.p.
	if ((aiP->nCurPathIndex >= aiP->nPathLength) || (aiP->nCurPathIndex < 0)) {
		//	If mode = hiding, then stay here until get bonked or hit by player.
		// --	if (ailP->mode == AIM_BEHIND) {
		// --		ailP->mode = AIM_IDLING;
		// --		return;		// Stay here until bonked or hit by player.
		// --	} else

		//	Buddy bot.  If he's in mode to get away from CPlayerData and at end of line,
		//	if CPlayerData visible, then make a new path, else just return.
		if (botInfoP->companion) {
			if (gameData.escort.nSpecialGoal == ESCORT_GOAL_SCRAM) {
				if (nPlayerVisibility) {
					CreateNSegmentPath (objP, 16 + d_rand () * 16, -1);
					aiP->nPathLength = SmoothPath (objP, &gameData.ai.pointSegs [aiP->nHideIndex], aiP->nPathLength);
					Assert (aiP->nPathLength != 0);
					ailP->mode = AIM_WANDER;	//	Special buddy mode.p.
					//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
					return;
					}
				else {
					ailP->mode = AIM_WANDER;	//	Special buddy mode.p.
					objP->mType.physInfo.velocity.SetZero();
					objP->mType.physInfo.rotVel.SetZero();
					//!!Assert ((aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength);
					return;
					}
				}
			}
		if (aiP->behavior == AIB_FOLLOW) {
			CreateNSegmentPath (objP, 10, gameData.objs.consoleP->info.nSegment);
			//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		else if (aiP->behavior == AIB_STATION) {
			CreatePathToStation (objP, 15);
			if ((aiP->nHideSegment != gameData.ai.pointSegs [aiP->nHideIndex+aiP->nPathLength-1].nSegment) ||
				 (aiP->nPathLength == 0)) {
				ailP->mode = AIM_IDLING;
				}
			else {
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
				}
			return;
			}
		else if (ailP->mode == AIM_FOLLOW_PATH) {
			CreatePathToPlayer (objP, 10, 1);
			if (aiP->nHideSegment != gameData.ai.pointSegs [aiP->nHideIndex+aiP->nPathLength-1].nSegment) {
				ailP->mode = AIM_IDLING;
				return;
				}
			else {
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
				}
			}
		else if (ailP->mode == AIM_RUN_FROM_OBJECT) {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, gameData.objs.consoleP->info.nSegment);
			ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
			if (aiP->nPathLength < 1) {
				CreateNSegmentPath (objP, AVOID_SEG_LENGTH, gameData.objs.consoleP->info.nSegment);
				ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
				if (aiP->nPathLength < 1) {
					aiP->behavior = AIB_NORMAL;
					ailP->mode = AIM_IDLING;
					return;
					}
				}
			//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		else {
			//	Reached end of the line.p.  First see if opposite end point is reachable, and if so, go there.p.
			//	If not, turn around.
			int			nOppositeEndIndex;
			CFixVector	*vOppositeEndPoint;
			tFVIData		hitData;
			int			fate;
			tFVIQuery	fq;

			// See which end we're nearer and look at the opposite end point.
			if (abs (aiP->nCurPathIndex - aiP->nPathLength) < aiP->nCurPathIndex) {
				//	Nearer to far end (ie, index not 0), so try to reach 0.
				nOppositeEndIndex = 0;
				}
			else {
				//	Nearer to 0 end, so try to reach far end.
				nOppositeEndIndex = aiP->nPathLength-1;
				}
			//--Int3_if (( (nOppositeEndIndex >= 0) && (nOppositeEndIndex < aiP->nPathLength));
			vOppositeEndPoint = &gameData.ai.pointSegs [aiP->nHideIndex + nOppositeEndIndex].point;
			fq.p0					= &objP->info.position.vPos;
			fq.startSeg			= objP->info.nSegment;
			fq.p1					= vOppositeEndPoint;
			fq.radP0				=
			fq.radP1				= objP->info.xSize;
			fq.thisObjNum		= OBJ_IDX (objP);
			fq.ignoreObjList	= NULL;
			fq.flags				= 0; 				//what about trans walls???
			fate = FindVectorIntersection (&fq, &hitData);
			if (fate != HIT_WALL) {
				//	We can be circular! Do it!
				//	Path direction is unchanged.
				aiP->nCurPathIndex = nOppositeEndIndex;
				}
			else {
				aiP->PATH_DIR = -aiP->PATH_DIR;
				aiP->nCurPathIndex += aiP->PATH_DIR;
				}
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		break;
		}
	else {
		new_vGoalPoint = gameData.ai.pointSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
		vGoalPoint = new_vGoalPoint;
		xDistToGoal = CFixVector::Dist(vGoalPoint, objP->info.position.vPos);
		//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
		}
	//	If went all the way around to original point, in same direction, then get out of here!
	if ((aiP->nCurPathIndex == original_index) && (aiP->PATH_DIR == original_dir)) {
		CreatePathToPlayer (objP, 3, 1);
		//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
		forced_break = 1;
		}
		//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
	}	//	end while
//	Set velocity (objP->mType.physInfo.velocity) and orientation (objP->info.position.mOrient) for this CObject.
//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
AIPathSetOrientAndVel (objP, &vGoalPoint, nPlayerVisibility, vec_to_player);
//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
}

//	----------------------------------------------------------------------------------------------------------

typedef struct {
	short	path_start, nObject;
} obj_path;

int _CDECL_ QSCmpPathIndex (obj_path *i1, obj_path *i2)
{
if (i1->path_start < i2->path_start)
	return -1;
if (i1->path_start > i2->path_start)
	return 1;
return 0;
}

//	----------------------------------------------------------------------------------------------------------
//	Set orientation matrix and velocity for objP based on its desire to get to a point.
void AIPathSetOrientAndVel (CObject *objP, CFixVector *vGoalPoint, int nPlayerVisibility, CFixVector *vec_to_player)
{
	CFixVector	vCurVel = objP->mType.physInfo.velocity;
	CFixVector	vNormCurVel;
	CFixVector	vNormToGoal;
	CFixVector	vCurPos = objP->info.position.vPos;
	CFixVector	vNormFwd;
	fix			xSpeedScale;
	fix			dot;
	tRobotInfo	*botInfoP = &ROBOTINFO (objP->info.nId);
	fix			xMaxSpeed;

//	If evading CPlayerData, use highest difficulty level speed, plus something based on diff level
xMaxSpeed = botInfoP->xMaxSpeed [gameStates.app.nDifficultyLevel];
if ((gameData.ai.localInfo [OBJ_IDX (objP)].mode == AIM_RUN_FROM_OBJECT) || (objP->cType.aiInfo.behavior == AIB_SNIPE))
	xMaxSpeed = xMaxSpeed*3/2;
vNormToGoal = *vGoalPoint - vCurPos;
CFixVector::Normalize(vNormToGoal);
vNormCurVel = vCurVel;
CFixVector::Normalize(vNormCurVel);
vNormFwd = objP->info.position.mOrient.FVec ();
CFixVector::Normalize(vNormFwd);
dot = CFixVector::Dot(vNormToGoal, vNormFwd);
//	If very close to facing opposite desired vector, perturb vector
if (dot < -15*F1_0/16) {
	vNormCurVel = vNormToGoal;
	}
else {
	vNormCurVel[X] += vNormToGoal[X]/2;
	vNormCurVel[Y] += vNormToGoal[Y]/2;
	vNormCurVel[Z] += vNormToGoal[Z]/2;
	}
CFixVector::Normalize(vNormCurVel);
//	Set speed based on this robot nType's maximum allowed speed and how hard it is turning.
//	How hard it is turning is based on the dot product of (vector to goal) and (current velocity vector)
//	Note that since 3*F1_0/4 is added to dot product, it is possible for the robot to back up.
//	Set speed and orientation.
if (dot < 0)
	dot /= -4;

//	If in snipe mode, can move fast even if not facing that direction.
if ((objP->cType.aiInfo.behavior == AIB_SNIPE) && (dot < F1_0/2))
	dot = (dot + F1_0) / 2;
xSpeedScale = FixMul (xMaxSpeed, dot);
vNormCurVel *= xSpeedScale;
objP->mType.physInfo.velocity = vNormCurVel;
if ((gameData.ai.localInfo [OBJ_IDX (objP)].mode == AIM_RUN_FROM_OBJECT) || (botInfoP->companion == 1) || (objP->cType.aiInfo.behavior == AIB_SNIPE)) {
	if (gameData.ai.localInfo [OBJ_IDX (objP)].mode == AIM_SNIPE_RETREAT_BACKWARDS) {
		if ((nPlayerVisibility) && (vec_to_player != NULL))
			vNormToGoal = *vec_to_player;
		else
			vNormToGoal = -vNormToGoal;
		}
	AITurnTowardsVector (&vNormToGoal, objP, botInfoP->turnTime [NDL-1]/2);
	}
else
	AITurnTowardsVector (&vNormToGoal, objP, botInfoP->turnTime [gameStates.app.nDifficultyLevel]);
}

int	nLastFrameGarbageCollected = 0;

//	----------------------------------------------------------------------------------------------------------
//	Garbage colledion -- Free all unused records in gameData.ai.pointSegs and compress all paths.
void AIPathGarbageCollect (void)
{
	int				nFreePathIdx = 0;
	int				nPathObjects = 0;
	int				nObject;
	int				nObjIdx, i, nOldIndex;
	CObject			*objP;
	tAIStaticInfo	*aiP;
	obj_path			objectList [MAX_OBJECTS_D2X];

#if DBG
force_dump_aiObjects_all ("***** Start AIPathGarbageCollect *****");
#endif
nLastFrameGarbageCollected = gameData.app.nFrameCount;
#if PATH_VALIDATION
ValidateAllPaths ();
#endif
	//	Create a list of OBJECTS which have paths of length 1 or more.p.
FORALL_ROBOT_OBJS (objP, nObject) {
	if ((objP->info.controlType == CT_AI) || (objP->info.controlType == CT_MORPH)) {
		aiP = &objP->cType.aiInfo;
		if (aiP->nPathLength) {
			objectList [nPathObjects].path_start = aiP->nHideIndex;
			objectList [nPathObjects++].nObject = OBJ_IDX (objP);
			}
		}
	}

qsort (objectList, nPathObjects, sizeof (objectList [0]),
		 (int (_CDECL_ *) (void const *, void const *))QSCmpPathIndex);

for (nObjIdx=0; nObjIdx < nPathObjects; nObjIdx++) {
	nObject = objectList [nObjIdx].nObject;
	objP = OBJECTS + nObject;
	aiP = &objP->cType.aiInfo;
	nOldIndex = aiP->nHideIndex;
	aiP->nHideIndex = nFreePathIdx;
	for (i = 0; i < aiP->nPathLength; i++)
		gameData.ai.pointSegs [nFreePathIdx++] = gameData.ai.pointSegs [nOldIndex++];
	}
gameData.ai.freePointSegs = gameData.ai.pointSegs + nFreePathIdx;

#if DBG
force_dump_aiObjects_all ("***** Finish AIPathGarbageCollect *****");
FORALL_ROBOT_OBJS (objP, i)
	if (objP->info.controlType == CT_AI) {
		aiP = &objP->cType.aiInfo; 
		if ((aiP->nHideIndex + aiP->nPathLength > gameData.ai.freePointSegs - gameData.ai.pointSegs) && (aiP->nPathLength > 0))
			Int3 ();		//	Contact Mike: Debug trap for nasty, elusive bug.
		}
#	if PATH_VALIDATION
ValidateAllPaths ();
#	endif
#endif
}

//	-----------------------------------------------------------------------------
//	Do garbage collection if not been done for awhile, or things getting really critical.
void MaybeAIPathGarbageCollect (void)
{
if (gameData.ai.freePointSegs - gameData.ai.pointSegs > MAX_POINT_SEGS - MAX_PATH_LENGTH) {
	if (nLastFrameGarbageCollected + 1 >= gameData.app.nFrameCount) {
		//	This is kind of bad.  Garbage collected last frame or this frame.p.
		//	Just destroy all paths.  Too bad for the robots.  They are memory wasteful.
		AIResetAllPaths ();
#if TRACE
		con_printf (1, "Warning: Resetting all paths.  gameData.ai.pointSegs buffer nearly exhausted.\n");
#endif
		}
	else {
			//	We are really close to full, but didn't just garbage collect, so maybe this is recoverable.p.
#if TRACE
		con_printf (1, "Warning: Almost full garbage collection being performed: ");
#endif
		AIPathGarbageCollect ();
#if TRACE
		con_printf (1, "Free records = %i/%i\n", MAX_POINT_SEGS - (gameData.ai.freePointSegs - gameData.ai.pointSegs), MAX_POINT_SEGS);
#endif
		}
	}
else if (gameData.ai.freePointSegs - gameData.ai.pointSegs > 3*MAX_POINT_SEGS/4) {
	if (nLastFrameGarbageCollected + 16 < gameData.app.nFrameCount) {
		AIPathGarbageCollect ();
		}
	}
else if (gameData.ai.freePointSegs - gameData.ai.pointSegs > MAX_POINT_SEGS/2) {
	if (nLastFrameGarbageCollected + 256 < gameData.app.nFrameCount) {
		AIPathGarbageCollect ();
		}
	}
}

//	-----------------------------------------------------------------------------
//	Reset all paths.  Do garbage collection.
//	Should be called at the start of each level.
void AIResetAllPaths (void)
{
	int		i;
	CObject	*objP = OBJECTS.Buffer ();

FORALL_OBJS (objP, i)
	if (objP->info.controlType == CT_AI) {
		objP->cType.aiInfo.nHideIndex = -1;
		objP->cType.aiInfo.nPathLength = 0;
		}
AIPathGarbageCollect ();
}

//	---------------------------------------------------------------------------------------------------------
//	Probably called because a robot bashed a tWall, getting a bunch of retries.
//	Try to resume path.
void AttemptToResumePath (CObject *objP)
{
	//int				nObject = OBJ_IDX (objP);
	tAIStaticInfo		*aiP = &objP->cType.aiInfo;
//	int				nGoalSegnum, object_segnum,
	int				nAbsIndex, nNewPathIndex;

if ((aiP->behavior == AIB_STATION) && (ROBOTINFO (objP->info.nId).companion != 1))
	if (d_rand () > 8192) {
		tAILocalInfo			*ailP = &gameData.ai.localInfo [OBJ_IDX (objP)];

		aiP->nHideSegment = objP->info.nSegment;
//Int3 ();
		ailP->mode = AIM_IDLING;
#if TRACE
		con_printf (1, "Note: Bashing hide CSegment of robot %i to current CSegment because he's lost.\n", OBJ_IDX (objP));
#endif
		}

//	object_segnum = objP->info.nSegment;
nAbsIndex = aiP->nHideIndex+aiP->nCurPathIndex;
//	nGoalSegnum = gameData.ai.pointSegs [nAbsIndex].nSegment;

nNewPathIndex = aiP->nCurPathIndex - aiP->PATH_DIR;

if ((nNewPathIndex >= 0) && (nNewPathIndex < aiP->nPathLength)) {
	aiP->nCurPathIndex = nNewPathIndex;
	}
else {
	//	At end of line and have nowhere to go.
	MoveTowardsSegmentCenter (objP);
	CreatePathToStation (objP, 15);
	}
}

//	----------------------------------------------------------------------------------------------------------
//					DEBUG FUNCTIONS FOLLOW
//	----------------------------------------------------------------------------------------------------------

#ifdef EDITOR
int	Test_size = 1000;

void test_create_path_many (void)
{
	tPointSeg	tPointSegs [200];
	short			numPoints;
	int			i;

for (i=0; i<Test_size; i++) {
	Cursegp = &gameData.segs.segments [ (d_rand () * (gameData.segs.nLastSegment + 1)) / D_RAND_MAX];
	Markedsegp = &gameData.segs.segments [ (d_rand () * (gameData.segs.nLastSegment + 1)) / D_RAND_MAX];
	CreatePathPoints (&OBJECTS [0], CurSEG_IDX (segp), MarkedSEG_IDX (segp), tPointSegs, &numPoints, -1, 0, 0, -1);
	}

}

void test_create_path (void)
{
	tPointSeg	tPointSegs [200];
	short			numPoints;

	CreatePathPoints (&OBJECTS [0], CurSEG_IDX (segp), MarkedSEG_IDX (segp), tPointSegs, &numPoints, -1, 0, 0, -1);

}

void show_path (int nStartSeg, int nEndSeg, tPointSeg *psp, short length)
{
	//printf (" [%3i:%3i (%3i):] ", nStartSeg, nEndSeg, length);

	while (length--)
		//printf ("%3i ", psp [length].nSegment);

	//printf ("\n");
}

//	For all segments in mine, create paths to all segments in mine, print results.
void test_create_all_paths (void)
{
	int	nStartSeg, nEndSeg;
	short	resultant_length;

	gameData.ai.freePointSegs = gameData.ai.pointSegs;

	for (nStartSeg=0; nStartSeg<=gameData.segs.nLastSegment-1; nStartSeg++) {
		if (gameData.segs.segments [nStartSeg].nSegment != -1) {
			for (nEndSeg=nStartSeg + 1; nEndSeg<=gameData.segs.nLastSegment; nEndSeg++) {
				if (gameData.segs.segments [nEndSeg].nSegment != -1) {
					CreatePathPoints (&OBJECTS [0], nStartSeg, nEndSeg, gameData.ai.freePointSegs, &resultant_length, -1, 0, 0, -1);
					show_path (nStartSeg, nEndSeg, gameData.ai.freePointSegs, resultant_length);
				}
			}
		}
	}
}

//--anchor--int	Num_anchors;
//--anchor--int	AnchorDistance = 3;
//--anchor--int	EndDistance = 1;
//--anchor--int	Anchors [MAX_SEGMENTS];

//--anchor--int get_nearest_anchorDistance (int nSegment)
//--anchor--{
//--anchor--	short	resultant_length, minimum_length;
//--anchor--	int	anchor_index;
//--anchor--
//--anchor--	minimum_length = 16383;
//--anchor--
//--anchor--	for (anchor_index=0; anchor_index<Num_anchors; anchor_index++) {
//--anchor--		CreatePathPoints (&OBJECTS [0], nSegment, Anchors [anchor_index], gameData.ai.freePointSegs, &resultant_length, -1, 0, 0, -1);
//--anchor--		if (resultant_length != 0)
//--anchor--			if (resultant_length < minimum_length)
//--anchor--				minimum_length = resultant_length;
//--anchor--	}
//--anchor--
//--anchor--	return minimum_length;
//--anchor--
//--anchor--}
//--anchor--
//--anchor--void create_new_anchor (int nSegment)
//--anchor--{
//--anchor--	Anchors [Num_anchors++] = nSegment;
//--anchor--}
//--anchor--
//--anchor--//	A set of anchors is within N units of all segments in the graph.
//--anchor--//	AnchorDistance = how close anchors can be.p.
//--anchor--//	EndDistance = how close you can be to the end.
//--anchor--void test_create_all_anchors (void)
//--anchor--{
//--anchor--	int	nearest_anchorDistance;
//--anchor--	int	nSegment, i;
//--anchor--
//--anchor--	Num_anchors = 0;
//--anchor--
//--anchor--	for (nSegment=0; nSegment<=gameData.segs.nLastSegment; nSegment++) {
//--anchor--		if (gameData.segs.segments [nSegment].nSegment != -1) {
//--anchor--			nearest_anchorDistance = get_nearest_anchorDistance (nSegment);
//--anchor--			if (nearest_anchorDistance > AnchorDistance)
//--anchor--				create_new_anchor (nSegment);
//--anchor--		}
//--anchor--	}
//--anchor--
//--anchor--	//	Set selected segs.
//--anchor--	for (i=0; i<Num_anchors; i++)
//--anchor--		Selected_segs [i] = Anchors [i];
//--anchor--	N_selected_segs = Num_anchors;
//--anchor--
//--anchor--}
//--anchor--
//--anchor--int	Test_path_length = 5;
//--anchor--
//--anchor--void test_create_n_segment_path (void)
//--anchor--{
//--anchor--	tPointSeg	tPointSegs [200];
//--anchor--	short			numPoints;
//--anchor--
//--anchor--	CreatePathPoints (&OBJECTS [0], CurSEG_IDX (segp), -2, tPointSegs, &numPoints, Test_path_length, 0, 0, -1);
//--anchor--}

short	Player_path_length=0;
int	Player_hide_index=-1;
int	Player_cur_path_index=0;
int	Player_following_pathFlag=0;

//	------------------------------------------------------------------------------------------------------------------
//	Set orientation matrix and velocity for objP based on its desire to get to a point.
void player_path_set_orient_and_vel (CObject *objP, CFixVector *vGoalPoint)
{
	CFixVector	vCurVel = objP->mType.physInfo.velocity;
	CFixVector	vNormCurVel;
	CFixVector	vNormToGoal;
	CFixVector	vCurPos = objP->info.position.vPos;
	CFixVector	vNormFwd;
	fix			xSpeedScale;
	fix			dot;
	fix			xMaxSpeed;

	xMaxSpeed = ROBOTINFO (objP->info.nId).xMaxSpeed [gameStates.app.nDifficultyLevel];

	VmVecSub (&vNormToGoal, vGoalPoint, &vCurPos);
	CFixVector::Normalize(vNormToGoal);

	vNormCurVel = vCurVel;
	CFixVector::Normalize(vNormCurVel);

	vNormFwd = objP->info.position.mOrient.FVec ();
	CFixVector::Normalize(vNormFwd);

	dot = CFixVector::Dot(vNormToGoal, &vNormFwd);
	if (gameData.ai.localInfo [OBJ_IDX (objP)].mode == AIM_SNIPE_RETREAT_BACKWARDS) {
		dot = -dot;
	}

	//	If very close to facing opposite desired vector, perturb vector
	if (dot < -15*F1_0/16) {
		vNormCurVel = vNormToGoal;
	} else {
		vNormCurVel[X] += vNormToGoal[X]/2;
		vNormCurVel[Y] += vNormToGoal[Y]/2;
		vNormCurVel[Z] += vNormToGoal[Z]/2;
	}

	CFixVector::Normalize(vNormCurVel);

	//	Set speed based on this robot nType's maximum allowed speed and how hard it is turning.
	//	How hard it is turning is based on the dot product of (vector to goal) and (current velocity vector)
	//	Note that since 3*F1_0/4 is added to dot product, it is possible for the robot to back up.

	//	Set speed and orientation.
	if (dot < 0)
		dot /= 4;

	xSpeedScale = FixMul (xMaxSpeed, dot);
	VmVecScale (&vNormCurVel, xSpeedScale);
	objP->mType.physInfo.velocity = vNormCurVel;
	AITurnTowardsVector (&vNormToGoal, objP, F1_0);

}

//	----------------------------------------------------------------------------------------------------------
//	Optimization: If current velocity will take robot near goal, don't change velocity
void player_follow_path (CObject *objP)
{
	CFixVector	vGoalPoint;
	fix			xDistToGoal;
	int			count, forced_break, original_index;
	int			nGoalSeg;
	fix			thresholdDistance;

	if (!Player_following_pathFlag)
		return;

	if (Player_hide_index == -1)
		return;

	if (Player_path_length < 2)
		return;

	vGoalPoint = gameData.ai.pointSegs [Player_hide_index + Player_cur_path_index].point;
	nGoalSeg = gameData.ai.pointSegs [Player_hide_index + Player_cur_path_index].nSegment;
	Assert ((nGoalSeg >= 0) && (nGoalSeg <= gameData.segs.nLastSegment);
	xDistToGoal = CFixVector::Dist(vGoalPoint, &objP->info.position.vPos);

	if (Player_cur_path_index < 0)
		Player_cur_path_index = 0;
	else if (Player_cur_path_index >= Player_path_length)
		Player_cur_path_index = Player_path_length-1;

	vGoalPoint = gameData.ai.pointSegs [Player_hide_index + Player_cur_path_index].point;

	count=0;

	//	If near goal, pick another goal point.
	forced_break = 0;		//	Gets set for short paths.
	//original_dir = 1;
	original_index = Player_cur_path_index;
	thresholdDistance = FixMul (VmVecMagQuick (&objP->mType.physInfo.velocity), gameData.time.xFrame)*2 + F1_0*2;

	while ((xDistToGoal < thresholdDistance) && !forced_break) {

		//	----- Debug stuff -----
		if (count++ > 20) {
#if TRACE
			con_printf (1, "Problem following path for player.  Aborting.\n");
#endif
			break;
		}

		//	Advance to next point on path.
		Player_cur_path_index += 1;

		//	See if next point wraps past end of path (in either direction), and if so, deal with it based on mode.p.
		if ((Player_cur_path_index >= Player_path_length) || (Player_cur_path_index < 0)) {
			Player_following_pathFlag = 0;
			forced_break = 1;
		}

		//	If went all the way around to original point, in same direction, then get out of here!
		if (Player_cur_path_index == original_index) {
#if TRACE
			con_printf (CONDBG, "Forcing break because CPlayerData path wrapped, count = %i.\n", count);
#endif
			Player_following_pathFlag = 0;
			forced_break = 1;
		}

		vGoalPoint = gameData.ai.pointSegs [Player_hide_index + Player_cur_path_index].point;
		xDistToGoal = CFixVector::Dist(vGoalPoint, &objP->info.position.vPos);

	}	//	end while

	//	Set velocity (objP->mType.physInfo.velocity) and orientation (objP->info.position.mOrient) for this CObject.
	player_path_set_orient_and_vel (objP, &vGoalPoint);

}


//	------------------------------------------------------------------------------------------------------------------
//	Create path for CPlayerData from current CSegment to goal CSegment.
void create_player_path_to_segment (int nSegment)
{
	CObject		*objP = gameData.objs.consoleP;

	Player_path_length=0;
	Player_hide_index=-1;
	Player_cur_path_index=0;
	Player_following_pathFlag=0;

	if (CreatePathPoints (objP, objP->info.nSegment, nSegment, gameData.ai.freePointSegs, &Player_path_length, 100, 0, 0, -1) == -1) {
#if TRACE
		con_printf (CONDBG, "Unable to form path of length %i for myself\n", 100);
#endif
		}

	Player_following_pathFlag = 1;

	Player_hide_index = gameData.ai.freePointSegs - gameData.ai.pointSegs;
	Player_cur_path_index = 0;
	gameData.ai.freePointSegs += Player_path_length;
	if (gameData.ai.freePointSegs - gameData.ai.pointSegs + MAX_PATH_LENGTH*2 > MAX_POINT_SEGS) {
		//Int3 ();	//	Contact Mike: This is curious, though not deadly. /eip++;g
		AIResetAllPaths ();
	}

}

int	Player_nGoalSegment = -1;

void check_create_player_path (void)
{
	if (Player_nGoalSegment != -1)
		create_player_path_to_segment (Player_nGoalSegment);

	Player_nGoalSegment = -1;
}

#endif

//	----------------------------------------------------------------------------------------------------------
//					DEBUG FUNCTIONS ENDED
//	----------------------------------------------------------------------------------------------------------

