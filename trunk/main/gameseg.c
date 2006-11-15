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

/*
 *
 * Functions moved from tSegment.c to make editor separable from game.
 *
 * Old Log:
 * Revision 1.9  1995/11/08  16:26:04  allender
 * minor bug fix in FindConnectedDistance
 *
 * Revision 1.8  1995/10/12  17:36:55  allender
 * made TraceSegs only recurse 100 times max
 *
 * Revision 1.7  1995/10/11  18:29:01  allender
 * removed Int3 from TraceSegs
 *
 * Revision 1.6  1995/10/11  14:13:54  allender
 * put in stack check code into trace-segs
 *
 * Revision 1.5  1995/09/23  09:40:25  allender
 * put in casts in ExtractShortPos to try and solve shortpos problem
 * with appletalk
 *
 * Revision 1.4  1995/09/20  14:26:50  allender
 * added flag to swap bytes on extract shortpot
 *
 * Revision 1.3  1995/08/12  12:01:27  allender
 * added flag to CreateShortPos to swap bytes
 *
 * Revision 1.2  1995/06/06  10:42:07  allender
 * made shortpos routines swap bytes when extracting and making shortpos structures
 *
 * Revision 1.1  1995/05/16  15:25:46  allender
 * Initial revision
 *
 * Revision 2.2  1995/03/20  18:15:39  john
 * Added code to not store the normals in the tSegment structure.
 *
 * Revision 2.1  1995/03/08  12:11:39  allender
 * fix shortpos reading/writing
 *
 * Revision 2.0  1995/02/27  11:29:21  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.78  1995/02/22  13:52:22  allender
 * remove anonymous unions from tObject structure
 *
 * Revision 1.77  1995/02/22  13:24:47  john
 * Removed the vecmat anonymous unions.
 *
 * Revision 1.76  1995/02/13  20:35:01  john
 * Lintized
 *
 * Revision 1.75  1995/02/09  13:10:51  mike
 * remove an annoying con_printf.
 *
 * Revision 1.74  1995/02/05  17:49:28  rob
 * Added assert to gameseg.c.
 *
 * Revision 1.73  1995/02/02  00:49:26  mike
 * new automap tSegment-nDepth functionality.
 *
 * Revision 1.72  1995/01/16  21:06:51  mike
 * Move function pick_random_point_in_segment from fireball.c to gameseg.c.
 *
 * Revision 1.71  1994/12/21  19:54:32  matt
 * Added error checking
 *
 * Revision 1.70  1994/12/11  21:34:09  matt
 * Changed assert () to int3 ()
 *
 * Revision 1.69  1994/12/01  21:04:37  matt
 * Several important changes:
 *  (1) Checking against triangulated sides has been standardized a bit
 *  (2) Code has been added to de-triangulate some sides
 *  (3) BIG ONE: the tolerance for checking a point against a plane has
 *      been drastically relaxed
 *
 *
 * Revision 1.67  1994/11/27  23:12:21  matt
 * Made changes for new con_printf calling convention
 *
 * Revision 1.66  1994/11/26  22:51:40  matt
 * Removed editor-only fields from tSegment structure when editor is compiled
 * out, and padded tSegment structure to even multiple of 4 bytes.
 *
 * Revision 1.65  1994/11/22  16:55:38  mike
 * use memset in place of loop to clear array.
 *
 * Revision 1.64  1994/11/19  15:20:37  mike
 * rip out unused code and data
 *
 * Revision 1.63  1994/11/18  18:31:48  matt
 * Fixed code again (and maybe for real)
 *
 * Revision 1.62  1994/11/18  16:54:24  matt
 * Fixed ExtractOrientFromSegment ()
 *
 * Revision 1.61  1994/11/17  14:56:50  mike
 * moved tSegment validation functions from editor to main.
 *
 * Revision 1.60  1994/11/16  23:38:53  mike
 * new improved boss teleportation behavior.
 *
 * Revision 1.59  1994/10/30  14:12:46  mike
 * rip out local segments stuff.
 *
 * Revision 1.58  1994/10/27  10:53:39  matt
 * Made connectivity error checking put up warning if errors found
 *
 * Revision 1.57  1994/10/25  21:19:26  mike
 * debugging code.
 *
 * Revision 1.56  1994/10/25  11:26:09  mike
 * *** empty log message ***
 *
 * Revision 1.55  1994/10/22  22:36:08  matt
 * Improved error finding routine
 *
 * Revision 1.54  1994/10/22  18:56:51  matt
 * Fixed obscure bug in tSegment trace code
 * Added error find routine, CheckSegmentConnections ()
 *
 * Revision 1.53  1994/10/17  14:05:19  matt
 * Don't give recursion assert if doing lighting
 *
 * Revision 1.52  1994/10/15  19:03:48  mike
 * Don't do exhaustive search in smooth lighting.
 *
 * Revision 1.51  1994/10/12  09:46:44  mike
 * Add debug code for trapping exhaustive searches.
 *
 * Revision 1.50  1994/10/11  20:50:41  matt
 * Made FindSegByPoint () take -1 as nSegment, meaning to search all segments
 *
 * Revision 1.49  1994/10/11  17:40:31  matt
 * Fixed bug that caused tSegment trace to only go through sides you can fly through
 *
 * Revision 1.48  1994/10/10  14:48:16  matt
 * Fixed mistake that caused odd pauses and occasional int3's
 *
 * Revision 1.47  1994/10/09  23:50:41  matt
 * Made FindHitPointUV () work with triangulated sides
 *
 * Revision 1.46  1994/10/08  23:06:52  matt
 * TraceSegs () didn't know about external walls
 *
 * Revision 1.45  1994/10/07  22:18:57  mike
 * Put in asserts to trap bad segnums.
 *
 * Revision 1.44  1994/10/06  14:08:07  matt
 * Added new function, ExtractOrientFromSegment ()
 *
 * Revision 1.43  1994/10/04  16:24:11  mike
 * Set global gameData.fcd.nConnSegDist for debug reasons for aipath.c.
 *
 * Revision 1.42  1994/10/04  09:18:42  mike
 * Comment out a variable definition, preventing a warning message.
 *
 * Revision 1.41  1994/10/03  23:43:42  mike
 * Put in a warning for overrunning tPointSegs buffer.
 *
 * Revision 1.40  1994/10/03  20:55:43  rob
 * Added velocity to shortpos.
 *
 * Revision 1.39  1994/09/27  11:46:06  rob
 * re-fixed that same bug (ugh).
 *
 * Revision 1.38  1994/09/27  10:10:51  rob
 * Fixed bug in ExtractShortPos (RelinkObject added).
 *
 * Revision 1.37  1994/09/25  23:41:02  matt
 * Changed the tObject load & save code to read/write the structure fields one
 * at a time (rather than the whole structure at once).  This mean that the
 * tObject structure can be changed without breaking the load/save functions.
 * As a result of this change, the localObject data can be and has been
 * incorporated into the tObject array.  Also, timeleft is now a property
 * of all gameData.objs.objects, and the tObject structure has been otherwise cleaned up.
 *
 * Revision 1.36  1994/09/22  19:03:05  mike
 * Add shortpos manipulation functions CreateShortPos and ExtractShortPos.
 *
 * Revision 1.35  1994/09/19  21:21:16  mike
 * Minor optimization to FindConnectedDistance.
 *
 * Revision 1.34  1994/09/19  21:05:25  mike
 * Write function FindConnectedDistance, 
 * returns distance between two points as travellable through the mine.
 *
 * Revision 1.33  1994/08/30  15:07:15  matt
 * Changed FindSegByPoint () to deal with some infinite recursion problems.
 *
 * Revision 1.32  1994/08/11  18:58:32  mike
 * Use ints in place of shorts for optimization.
 *
 * Revision 1.31  1994/08/04  00:20:09  matt
 * Cleaned up fvi & physics error handling; put in code to make sure gameData.objs.objects
 * are in correct tSegment; simplified tSegment finding for gameData.objs.objects and points
 *
 * Revision 1.30  1994/08/03  16:46:12  mike
 * not much...
 *
 * Revision 1.29  1994/08/02  20:41:31  matt
 * Fixed bug in GetSideVerts ()
 *
 * Revision 1.28  1994/08/02  19:04:25  matt
 * Cleaned up vertex list functions
 *
 * Revision 1.27  1994/08/01  10:39:44  matt
 * find_new_seg () now will look through any kind of wall but a totally solid one
 *
 * Revision 1.26  1994/07/28  19:15:59  matt
 * Fixed yet another bug in GetSegMasks ()
 *
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
#include "lighting.h"

#ifdef RCS
static char rcsid [] = "$Id: gameseg.c, v 1.5 2004/04/14 08:54:35 btb Exp $";
#endif

// How far a point can be from a plane, and still be "in" the plane

// -------------------------------------------------------------------------------

#ifdef COMPACT_SEGS

//#define CACHE_DEBUG 1
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
#ifdef CACHE_DEBUG
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

#ifdef CACHE_DEBUG
#if TRACE		
	if (( (++normCache.nCounter % 5000) == 1) && (normCache.nHits+normCache.nMisses > 0))
		con_printf (0, "NCACHE %d%% missed, H:%d, M:%d\n", (normCache.nMisses*100)/ (normCache.nHits+normCache.nMisses), normCache.nHits, normCache.nMisses);
#endif
#endif

	i = ((nSegment<<2) ^ nSide) & CACHE_MASK;
	if ((normCache.cache [i].nSegment == nSegment) && ((normCache.cache [i].nSide&0xf) == nSide)) {
		uint f1;
#ifdef CACHE_DEBUG
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
#ifdef CACHE_DEBUG
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
	char	*vs = sideToVerts [nSide];

switch (segP->sides [nSide].nType) {
	case SIDE_IS_QUAD:
		bFlip = GetVertsForNormal (segP->verts [vs [0]], segP->verts [vs [1]], segP->verts [vs [2]], segP->verts [vs [3]], 
											&vm0, &vm1, &vm2, &vm3, &bFlip);
		VmVecNormal (vm, 
						 gameData.segs.vertices + vm0, 
						 gameData.segs.vertices + vm1, 
						 gameData.segs.vertices + vm2);
		if (bFlip)
			VmVecNegate (vm);
		break;
	case SIDE_IS_TRI_02:
		if (face_num == 0)
			VmVecNormal (vm, 
							 gameData.segs.vertices + segP->verts [vs [0]], 
							 gameData.segs.vertices + segP->verts [vs [1]], 
							 gameData.segs.vertices + segP->verts [vs [2]]);
		else
			VmVecNormal (vm, 
							 gameData.segs.vertices + segP->verts [vs [0]], 
							 gameData.segs.vertices + segP->verts [vs [2]], 
							 gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	case SIDE_IS_TRI_13:
		if (face_num == 0)
			VmVecNormal (vm, 
							 gameData.segs.vertices + segP->verts [vs [0]], 
							 gameData.segs.vertices + segP->verts [vs [1]], 
							 gameData.segs.vertices + segP->verts [vs [3]]);
		else
			VmVecNormal (vm, 
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
	char	*vs = sideToVerts [nSide];

switch (segP->sides [nSide].nType)	{
	case SIDE_IS_QUAD:
		bFlip = GetVertsForNormal (segP->verts [vs [0]], segP->verts [vs [1]], segP->verts [vs [2]], segP->verts [vs [3]], 
											&vvm0, &vvm1, &vvm2, &vvm3);
		VmVecNormal (vm1, 
						 gameData.segs.vertices + vvm0, 
						 gameData.segs.vertices + vvm1, 
						 gameData.segs.vertices + vvm2);
		if (bFlip)
			VmVecNegate (vm1);
		*vm2 = *vm1;
		break;
	case SIDE_IS_TRI_02:
		VmVecNormal (vm1, 
						 gameData.segs.vertices + segP->verts [vs [0]], 
						 gameData.segs.vertices + segP->verts [vs [1]], 
						 gameData.segs.vertices + segP->verts [vs [2]]);
		VmVecNormal (vm2, 
						 gameData.segs.vertices + segP->verts [vs [0]], 
						 gameData.segs.vertices + segP->verts [vs [2]], 
						 gameData.segs.vertices + segP->verts [vs [3]]);
		break;
	case SIDE_IS_TRI_13:
		VmVecNormal (vm1, 
						 gameData.segs.vertices + segP->verts [vs [0]], 
						 gameData.segs.vertices + segP->verts [vs [1]], 
						 gameData.segs.vertices + segP->verts [vs [3]]);
		VmVecNormal (vm2, 
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
vp->x /= 4;
vp->y /= 4;
vp->z /= 4;
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
vp->x /= 8;
vp->y /= 8;
vp->z /= 8;
}

// -----------------------------------------------------------------------------
//	Given two segments, return the tSide index in the connecting tSegment which connects to the base tSegment
//	Optimized by MK on 4/21/94 because it is a 2% load.
int FindConnectedSide (tSegment *baseSegP, tSegment *connSegP)
{
	int	s;
	short	nBaseSeg = SEG_IDX (baseSegP);
	short *childs = connSegP->children;

for (s = 0; s < MAX_SIDES_PER_SEGMENT; s++)
	if (*childs++ == nBaseSeg)
		return s;
// legal to return -1, used in MoveOneObject (), mk, 06/08/94: Assert (0);		// Illegal -- there is no connecting tSide between these two segments
return -1;
}

// -----------------------------------------------------------------------------------
//	Given a tSide, return the number of faces
int GetNumFaces (tSide *sideP)
{
switch (sideP->nType) {
	case SIDE_IS_QUAD:	
		return 1;	
		break;
	case SIDE_IS_TRI_02:
	case SIDE_IS_TRI_13:	
		return 2;	
		break;
	default:
		Error ("Illegal nType = %i\n", sideP->nType);
		break;
	}
return 0;
}

// -----------------------------------------------------------------------------------
// Fill in array with four absolute point numbers for a given tSide
void GetSideVerts (short *vertlist, int nSegment, int nSide)
{
	sbyte *sv = sideToVerts [nSide];
	short	*vp = gameData.segs.segments [nSegment].verts;

vertlist [0] = vp [sv [0]];
vertlist [1] = vp [sv [1]];
vertlist [2] = vp [sv [2]];
vertlist [3] = vp [sv [3]];
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

		vertices [0] = sv [0];
		vertices [1] = sv [1];
		vertices [2] = sv [2];

		vertices [3] = sv [2];
		vertices [4] = sv [3];
		vertices [5] = sv [0];

		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	case SIDE_IS_TRI_13:
		*nFaces = 2;

		vertices [0] = sv [3];
		vertices [1] = sv [0];
		vertices [2] = sv [1];

		vertices [3] = sv [1];
		vertices [4] = sv [2];
		vertices [5] = sv [3];

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

		vertnums [0] = 0;
		vertnums [1] = 1;
		vertnums [2] = 2;

		vertnums [3] = 2;
		vertnums [4] = 3;
		vertnums [5] = 0;

		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	case SIDE_IS_TRI_13:
		*nFaces = 2;

		vertnums [0] = 3;
		vertnums [1] = 0;
		vertnums [2] = 1;

		vertnums [3] = 1;
		vertnums [4] = 2;
		vertnums [5] = 3;

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
void CreateAbsVertexLists (int *nFaces, int *vertices, int nSegment, int nSide)
{
	short	*vp = gameData.segs.segments [nSegment].verts;
	tSide	*sideP = gameData.segs.segments [nSegment].sides + nSide;
	int  *sv = sideToVertsInt [nSide];

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
switch (sideP->nType) {
	case SIDE_IS_QUAD:

		vertices [0] = vp [sv [0]];
		vertices [1] = vp [sv [1]];
		vertices [2] = vp [sv [2]];
		vertices [3] = vp [sv [3]];

		*nFaces = 1;
		break;
	case SIDE_IS_TRI_02:
		*nFaces = 2;

		vertices [0] = vp [sv [0]];
		vertices [1] = vp [sv [1]];
		vertices [2] = vp [sv [2]];

		vertices [3] = vp [sv [2]];
		vertices [4] = vp [sv [3]];
		vertices [5] = vp [sv [0]];

		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS (), 
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	case SIDE_IS_TRI_13:
		*nFaces = 2;

		vertices [0] = vp [sv [3]];
		vertices [1] = vp [sv [0]];
		vertices [2] = vp [sv [1]];

		vertices [3] = vp [sv [1]];
		vertices [4] = vp [sv [2]];
		vertices [5] = vp [sv [3]];

		//IMPORTANT: DON'T CHANGE THIS CODE WITHOUT CHANGING GET_SEG_MASKS ()
		//CREATE_ABS_VERTEX_LISTS (), CREATE_ALL_VERTEX_LISTS (), CREATE_ALL_VERTNUM_LISTS ()
		break;
	default:
		Error ("Illegal tSide nType (3), nType = %i, tSegment # = %i, tSide # = %i\n", sideP->nType, nSegment, nSide);
		break;
	}
}

// -------------------------------------------------------------------------------
//returns 3 different bitmasks with info telling if this sphere is in
//this tSegment.  See segmasks structure for info on fields  
segmasks GetSegMasks (vmsVector *checkp, int nSegment, fix xRad)
{
	int			sn, faceBit, sideBit;
	int			nFaces;
	int			nVertex, fn;
	int			bSidePokesOut;
	int			nSideCount, nCenterCount;
	int			vertexList [6];
	fix			xDist;
	tSegment		*segP;
	tSide			*sideP;
	segmasks		masks;

masks.sideMask = 0;
masks.faceMask = 0;
masks.centerMask = 0;
if (nSegment == -1) {
	Error ("nSegment == -1 in GetSegMasks ()");
	return masks;
	}
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
segP = gameData.segs.segments + nSegment;
//check point against each tSide of tSegment. return bitmask
for (sn = 0, faceBit = sideBit = 1, sideP = segP->sides; sn < 6; sn++, sideBit <<= 1, sideP++) {
	// Get number of faces on this tSide, and at vertexList, store vertices.
	//	If one face, then vertexList indicates a quadrilateral.
	//	If two faces, then 0, 1, 2 define one triangle, 3, 4, 5 define the second.
	CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
	//ok...this is important.  If a tSide has 2 faces, we need to know if
	//those faces form a concave or convex tSide.  If the tSide pokes out, 
	//then a point is on the back of the tSide if it is behind BOTH faces, 
	//but if the tSide pokes in, a point is on the back if behind EITHER face.

	if (nFaces == 2) {
#ifdef COMPACT_SEGS
		vmsVector normals [2];
#endif

		nVertex = min (vertexList [0], vertexList [2]);
#ifdef COMPACT_SEGS
		GetSideNormals (segP, sn, normals, normals + 1);
#endif
		
		if (vertexList [4] < vertexList [1])
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], normals, gameData.segs.vertices + nVertex);
#else				
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
#endif
		else
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], normals + 1, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
#endif
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nSideCount = nCenterCount = 0;

		for (fn = 0; fn < 2; fn++, faceBit <<= 1) {
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (checkp, normals + fn, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (checkp, sideP->normals + fn, gameData.segs.vertices + nVertex);
#endif

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
#ifdef COMPACT_SEGS			
		vmsVector normal;
#endif
		//use lowest point number
		nVertex = vertexList [0];
		//some manual loop unrolling here ...
		if (nVertex > vertexList [1])
			nVertex = vertexList [1];
		if (nVertex > vertexList [2])
			nVertex = vertexList [2];
		if (nVertex > vertexList [3])
			nVertex = vertexList [3];
#ifdef COMPACT_SEGS
		GetSideNormal (segP, sn, 0, &normal);
		xDist = VmDistToPlane (checkp, &normal, gameData.segs.vertices + nVertex);
#else
		xDist = VmDistToPlane (checkp, sideP->normals, gameData.segs.vertices + nVertex);
#endif
		if (xDist < -PLANE_DIST_TOLERANCE)
			masks.centerMask |= sideBit;
		if (xDist - xRad < -PLANE_DIST_TOLERANCE) {
			masks.faceMask |= faceBit;
			masks.sideMask |= sideBit;
			}
		faceBit <<= 2;
		}
	}
return masks;
}

// -------------------------------------------------------------------------------
//this was converted from GetSegMasks ()...it fills in an array of 6
//elements for the distace behind each tSide, or zero if not behind
//only gets centerMask, and assumes zero rad
ubyte GetSideDists (vmsVector *checkp, int nSegment, fix *xSideDists)
{
	int			sn, faceBit, sideBit;
	ubyte			mask;
	int			nFaces;
	int			vertexList [6];
	tSegment		*segP;
	tSide			*sideP;

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
if (nSegment == -1)
	Error ("nSegment == -1 in GetSideDists ()");

segP = gameData.segs.segments + nSegment;
sideP = segP->sides;
//check point against each tSide of tSegment. return bitmask
mask = 0;
for (sn = 0, faceBit = sideBit = 1; sn < 6; sn++, sideBit <<= 1, sideP++) {
		int	bSidePokesOut;
		int	fn;

	xSideDists [sn] = 0;
	// Get number of faces on this tSide, and at vertexList, store vertices.
	//	If one face, then vertexList indicates a quadrilateral.
	//	If two faces, then 0, 1, 2 define one triangle, 3, 4, 5 define the second.
	CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
	//ok...this is important.  If a tSide has 2 faces, we need to know if
	//those faces form a concave or convex tSide.  If the tSide pokes out, 
	//then a point is on the back of the tSide if it is behind BOTH faces, 
	//but if the tSide pokes in, a point is on the back if behind EITHER face.

	if (nFaces == 2) {
		fix	xDist;
		int	nCenterCount;
		int	nVertex;
#ifdef COMPACT_SEGS
		vmsVector normals [2];
#endif
		nVertex = min (vertexList [0], vertexList [2]);
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
#endif
#ifdef COMPACT_SEGS
		GetSideNormals (segP, sn, normals, normals + 1);
#endif
		if (vertexList [4] < vertexList [1])
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], normals, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
#endif
		else
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], normals + 1, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
#endif
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nCenterCount = 0;
		for (fn = 0; fn < 2; fn++, faceBit <<= 1) {
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (checkp, normals + fn, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (checkp, sideP->normals + fn, gameData.segs.vertices + nVertex);
#endif
			if (xDist < -PLANE_DIST_TOLERANCE) {	//in front of face
				nCenterCount++;
				xSideDists [sn] += xDist;
				}
			}
		if (!bSidePokesOut) {		//must be behind both faces
			if (nCenterCount == 2) {
				mask |= sideBit;
				xSideDists [sn] /= 2;		//get average
				}
			}
		else {							//must be behind at least one face
			if (nCenterCount) {
				mask |= sideBit;
				if (nCenterCount == 2)
					xSideDists [sn] /= 2;		//get average
				}
			}
		}
	else {				//only one face on this tSide
		fix xDist;
		int nVertex;
#ifdef COMPACT_SEGS			
		vmsVector normal;
#endif
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
			CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
#endif

#ifdef COMPACT_SEGS
		GetSideNormal (segP, sn, 0, &normal);
		xDist = VmDistToPlane (checkp, &normal, gameData.segs.vertices + nVertex);
#else
		xDist = VmDistToPlane (checkp, &sideP->normals [0], gameData.segs.vertices + nVertex);
#endif
		if (xDist < -PLANE_DIST_TOLERANCE) {
			mask |= sideBit;
			xSideDists [sn] = xDist;
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
ubyte GetSideDistsAll (vmsVector *checkp, int nSegment, fix *xSideDists)
{
	int			sn, faceBit, sideBit;
	ubyte			mask;
	int			nFaces;
	int			vertexList [6];
	tSegment		*segP;
	tSide			*sideP;

Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= 0));
if (nSegment == -1)
	Error ("nSegment == -1 in GetSideDistsAll ()");

segP = gameData.segs.segments + nSegment;
sideP = segP->sides;
//check point against each tSide of tSegment. return bitmask
mask = 0;
for (sn = 0, faceBit = sideBit = 1; sn < 6; sn++, sideBit <<= 1, sideP++) {
		int	bSidePokesOut;
		int	fn;

	xSideDists [sn] = 0;
	// Get number of faces on this tSide, and at vertexList, store vertices.
	//	If one face, then vertexList indicates a quadrilateral.
	//	If two faces, then 0, 1, 2 define one triangle, 3, 4, 5 define the second.
	CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
	//ok...this is important.  If a tSide has 2 faces, we need to know if
	//those faces form a concave or convex tSide.  If the tSide pokes out, 
	//then a point is on the back of the tSide if it is behind BOTH faces, 
	//but if the tSide pokes in, a point is on the back if behind EITHER face.

	if (nFaces == 2) {
		fix	xDist;
		int	nCenterCount;
		int	nVertex;
#ifdef COMPACT_SEGS
		vmsVector normals [2];
#endif
		nVertex = min (vertexList [0], vertexList [2]);
#ifdef _DEBUG
		if ((nVertex < 0) || (nVertex >= gameData.segs.nVertices))
			CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
#endif
#ifdef COMPACT_SEGS
		GetSideNormals (segP, sn, normals, normals + 1);
#endif
		if (vertexList [4] < vertexList [1])
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], normals, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [4], sideP->normals, gameData.segs.vertices + nVertex);
#endif
		else
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], normals + 1, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (gameData.segs.vertices + vertexList [1], sideP->normals + 1, gameData.segs.vertices + nVertex);
#endif
		bSidePokesOut = (xDist > PLANE_DIST_TOLERANCE);
		nCenterCount = 0;
		for (fn = 0; fn < 2; fn++, faceBit <<= 1) {
#ifdef COMPACT_SEGS
			xDist = VmDistToPlane (checkp, normals + fn, gameData.segs.vertices + nVertex);
#else
			xDist = VmDistToPlane (checkp, sideP->normals + fn, gameData.segs.vertices + nVertex);
#endif
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
#ifdef COMPACT_SEGS			
		vmsVector normal;
#endif
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
			CreateAbsVertexLists (&nFaces, vertexList, nSegment, sn);
#endif

#ifdef COMPACT_SEGS
		GetSideNormal (segP, sn, 0, &normal);
		xDist = VmDistToPlane (checkp, &normal, gameData.segs.vertices + nVertex);
#else
		xDist = VmDistToPlane (checkp, &sideP->normals [0], gameData.segs.vertices + nVertex);
#endif
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
#ifndef NDEBUG
#ifndef COMPACT_SEGS
//returns true if errors detected
int CheckNorms (int nSegment, int nSide, int facenum, int csegnum, int csidenum, int cfacenum)
{
	vmsVector *n0, *n1;

	n0 = &gameData.segs.segments [nSegment].sides [nSide].normals [facenum];
	n1 = &gameData.segs.segments [csegnum].sides [csidenum].normals [cfacenum];

	if (n0->x != -n1->x  ||  n0->y != -n1->y  ||  n0->z != -n1->z) {
#if TRACE
		con_printf (CON_DEBUG, "Seg %x, tSide %d, norm %d doesn't match seg %x, tSide %d, norm %d:\n"
				"   %8x %8x %8x\n"
				"   %8x %8x %8x (negated)\n", 
				nSegment, nSide, facenum, csegnum, csidenum, cfacenum, 
				n0->x, n0->y, n0->z, -n1->x, -n1->y, -n1->z);
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

			CreateAbsVertexLists (&nFaces, vertexList, nSegment, nSide);

			csegnum = seg->children [nSide];

			if (csegnum >= 0) {
				cseg = &gameData.segs.segments [csegnum];
				csidenum = FindConnectedSide (seg, cseg);

				if (csidenum == -1) {
#if TRACE
					con_printf (CON_DEBUG, "Could not find connected tSide for seg %x back to seg %x, tSide %d\n", csegnum, nSegment, nSide);
#endif
					errors = 1;
					continue;
				}

				cs = &cseg->sides [csidenum];

				CreateAbsVertexLists (&con_num_faces, con_vertex_list, csegnum, csidenum);

				if (con_num_faces != nFaces) {
#if TRACE
					con_printf (CON_DEBUG, "Seg %x, tSide %d: nFaces (%d) mismatch with seg %x, tSide %d (%d)\n", nSegment, nSide, nFaces, csegnum, csidenum, con_num_faces);
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
							con_printf (CON_DEBUG, "Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
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
								con_printf (CON_DEBUG, 
									"Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
									"  %x %x %x  %x %x %x\n"
									"  %x %x %x  %x %x %x\n", 
									nSegment, nSide, csegnum, csidenum, 
									vertexList [0], vertexList [1], vertexList [2], vertexList [3], vertexList [4], vertexList [5], 
									con_vertex_list [0], con_vertex_list [1], con_vertex_list [2], con_vertex_list [3], con_vertex_list [4], con_vertex_list [5]);
								con_printf (CON_DEBUG, 
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
								con_printf (CON_DEBUG, 
									"Seg %x, tSide %d: vertex list mismatch with seg %x, tSide %d\n"
									"  %x %x %x  %x %x %x\n"
									"  %x %x %x  %x %x %x\n", 
									nSegment, nSide, csegnum, csidenum, 
									vertexList [0], vertexList [1], vertexList [2], vertexList [3], vertexList [4], vertexList [5], 
									con_vertex_list [0], con_vertex_list [1], con_vertex_list [2], con_vertex_list [3], con_vertex_list [4], vertexList [5]);
								con_printf (CON_DEBUG, 
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
//	this for omega blob FindSegByPoint calls.  Would be better to pass a paremeter
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
	static char		bVisited [MAX_SEGMENTS];
	
Assert ((nOldSeg <= gameData.segs.nLastSegment) && (nOldSeg >= 0));
if (nTraceDepth >= gameData.segs.nSegments) {
#if TRACE
	con_printf (CON_DEBUG, "TraceSegs: Segment not found\n");
	con_printf (CON_DEBUG, "TraceSegs (gameseg.c) - Something went wrong - infinite loop\n");
#endif
	return -1;
}
if (!nTraceDepth)
	memset (bVisited, 0, sizeof (bVisited));
if (bVisited [nOldSeg] || (gameData.segs.segment2s [nOldSeg].special == SEGMENT_IS_SKYBOX))
	return -1;
nTraceDepth++;
bVisited [nOldSeg] = 1;
centerMask = GetSideDists (p0, nOldSeg, xSideDists);		//check old tSegment
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

//Tries to find a tSegment for a point, in the following way:
// 1. Check the given tSegment
// 2. Recursively trace through attached segments
// 3. Check all the segmentns
//Returns nSegment if found, or -1
int FindSegByPoint (vmsVector *p, int nSegment)
{
int nNewSeg;

//allow nSegment == -1, meaning we have no idea what tSegment point is in
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= -1));
if (nSegment != -1) {
	nNewSeg = TraceSegs (p, nSegment);
	if (nNewSeg != -1)			//we found a tSegment!
		return nNewSeg;
	}
//couldn't find via attached segs, so search all segs
if (bDoingLightingHack)
	return -1;
++nExhaustiveCount;
#if 0 //TRACE
con_printf (1, "Warning: doing exhaustive search to find point tSegment (%i times)\n", nExhaustiveCount);
#endif
for (nNewSeg = 0; nNewSeg <= gameData.segs.nLastSegment; nNewSeg++)
	if ((gameData.segs.segment2s [nNewSeg].special != SEGMENT_IS_SKYBOX) && 
	    (!GetSegMasks (p, nNewSeg, 0).centerMask))
		return nNewSeg;
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
fix FindConnectedDistance (vmsVector *p0, short seg0, vmsVector *p1, short seg1, int nMaxDepth, int widFlag)
{
	short				nConnSide;
	short				nCurSeg, nParentSeg, nThisSeg;
	short				nSide;
	int				qTail = 0, qHead = 0;
	int				i, nCurDepth, nPoints;
	sbyte				visited [MAX_SEGMENTS];
	segQueueEntry	segmentQ [MAX_SEGMENTS];
	short				nDepth [MAX_SEGMENTS];
	tPointSeg		pointSegs [MAX_LOC_POINT_SEGS];
	fix				dist;
	tSegment			*segP;
	tFCDCacheData	*pc;

	//	If > this, will overrun pointSegs buffer
#ifdef WINDOWS
if (nMaxDepth == -1) 
	nMaxDepth = 200;
#endif	

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
for (i = MAX_FCD_CACHE, pc = gameData.fcd.cache; i; i--, pc++)
	if ((pc->seg0 == seg0) && (pc->seg1 == seg1)) {
		gameData.fcd.nConnSegDist = pc->csd;
		return pc->dist;
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
		return f >> MATRIX_PRECISION;
}

#define VEL_PRECISION 12

// -------------------------------------------------------------------------------
//	Create a shortpos struct from an tObject.
//	Extract the matrix into byte values.
//	Create a position relative to vertex 0 with 1/256 normal "fix" precision.
//	Stuff tSegment in a short.
void CreateShortPos (shortpos *spp, tObject *objP, int swap_bytes)
{
	// int	nSegment;
	vmsMatrix orient = objP->orient;
	sbyte   *segP = spp->bytemat;

	*segP++ = convert_to_byte (orient.rVec.x);
	*segP++ = convert_to_byte (orient.uVec.x);
	*segP++ = convert_to_byte (orient.fVec.x);
	*segP++ = convert_to_byte (orient.rVec.y);
	*segP++ = convert_to_byte (orient.uVec.y);
	*segP++ = convert_to_byte (orient.fVec.y);
	*segP++ = convert_to_byte (orient.rVec.z);
	*segP++ = convert_to_byte (orient.uVec.z);
	*segP++ = convert_to_byte (orient.fVec.z);

	spp->xo = (objP->pos.x - gameData.segs.vertices [gameData.segs.segments [objP->nSegment].verts [0]].x) >> RELPOS_PRECISION;
	spp->yo = (objP->pos.y - gameData.segs.vertices [gameData.segs.segments [objP->nSegment].verts [0]].y) >> RELPOS_PRECISION;
	spp->zo = (objP->pos.z - gameData.segs.vertices [gameData.segs.segments [objP->nSegment].verts [0]].z) >> RELPOS_PRECISION;

	spp->tSegment = objP->nSegment;

 	spp->velx = (objP->mType.physInfo.velocity.x) >> VEL_PRECISION;
	spp->vely = (objP->mType.physInfo.velocity.y) >> VEL_PRECISION;
	spp->velz = (objP->mType.physInfo.velocity.z) >> VEL_PRECISION;

// swap the short values for the big-endian machines.

	if (swap_bytes) {
		spp->xo = INTEL_SHORT (spp->xo);
		spp->yo = INTEL_SHORT (spp->yo);
		spp->zo = INTEL_SHORT (spp->zo);
		spp->tSegment = INTEL_SHORT (spp->tSegment);
		spp->velx = INTEL_SHORT (spp->velx);
		spp->vely = INTEL_SHORT (spp->vely);
		spp->velz = INTEL_SHORT (spp->velz);
	}
}

// -------------------------------------------------------------------------------

void ExtractShortPos (tObject *objP, shortpos *spp, int swap_bytes)
{
	int	nSegment;
	sbyte   *segP;

	segP = spp->bytemat;

	objP->orient.rVec.x = *segP++ << MATRIX_PRECISION;
	objP->orient.uVec.x = *segP++ << MATRIX_PRECISION;
	objP->orient.fVec.x = *segP++ << MATRIX_PRECISION;
	objP->orient.rVec.y = *segP++ << MATRIX_PRECISION;
	objP->orient.uVec.y = *segP++ << MATRIX_PRECISION;
	objP->orient.fVec.y = *segP++ << MATRIX_PRECISION;
	objP->orient.rVec.z = *segP++ << MATRIX_PRECISION;
	objP->orient.uVec.z = *segP++ << MATRIX_PRECISION;
	objP->orient.fVec.z = *segP++ << MATRIX_PRECISION;

	if (swap_bytes) {
		spp->xo = INTEL_SHORT (spp->xo);
		spp->yo = INTEL_SHORT (spp->yo);
		spp->zo = INTEL_SHORT (spp->zo);
		spp->tSegment = INTEL_SHORT (spp->tSegment);
		spp->velx = INTEL_SHORT (spp->velx);
		spp->vely = INTEL_SHORT (spp->vely);
		spp->velz = INTEL_SHORT (spp->velz);
	}

	nSegment = spp->tSegment;

	Assert ((nSegment >= 0) && (nSegment <= gameData.segs.nLastSegment));

	objP->pos.x = (spp->xo << RELPOS_PRECISION) + gameData.segs.vertices [gameData.segs.segments [nSegment].verts [0]].x;
	objP->pos.y = (spp->yo << RELPOS_PRECISION) + gameData.segs.vertices [gameData.segs.segments [nSegment].verts [0]].y;
	objP->pos.z = (spp->zo << RELPOS_PRECISION) + gameData.segs.vertices [gameData.segs.segments [nSegment].verts [0]].z;

	objP->mType.physInfo.velocity.x = (spp->velx << VEL_PRECISION);
	objP->mType.physInfo.velocity.y = (spp->vely << VEL_PRECISION);
	objP->mType.physInfo.velocity.z = (spp->velz << VEL_PRECISION);

	RelinkObject (OBJ_IDX (objP), nSegment);

}

//--unused-- void test_shortpos (void)
//--unused-- {
//--unused-- 	shortpos	spp;
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
	vmsVector	norm;
	sbyte       *vs = sideToVerts [nSide];
	short			v0 = segP->verts [vs [0]];
	short			v1 = segP->verts [vs [1]];
	short			v2 = segP->verts [vs [2]];
	short			v3 = segP->verts [vs [3]];
	fix			dot;
	vmsVector	vec_13;		//	vector from vertex 1 to vertex 3

	tSide	*sideP = segP->sides + nSide;

	//	Choose how to triangulate.
	//	If a wall, then
	//		Always triangulate so tSegment is convex.
	//		Use Matt's formula: Na . AD > 0, where ABCD are vertices on tSide, a is face formed by A, B, C, Na is normal from face a.
	//	If not a wall, then triangulate so whatever is on the other tSide is triangulated the same (ie, between the same absoluate vertices)
#if 0
VmVecNormal (sideP->normals, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v2);
VmVecNormal (sideP->normals + 1, gameData.segs.vertices + v0, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
dot = VmVecDot (sideP->normals, sideP->normals + 1);
if (dot >= 0)
	sideP->nType = SIDE_IS_TRI_02;
else {
	sideP->nType = SIDE_IS_TRI_13;
	VmVecNormal (sideP->normals, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v3);
	VmVecNormal (sideP->normals + 1, gameData.segs.vertices + v1, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
	}
#else
if (!IS_CHILD (segP->children [nSide])) {
	VmVecNormal (&norm, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v2);
	VmVecSub (&vec_13, gameData.segs.vertices + v3, gameData.segs.vertices + v1);
	dot = VmVecDot (&norm, &vec_13);

	//	Now, signify whether to triangulate from 0:2 or 1:3
	if (dot >= 0)
		sideP->nType = SIDE_IS_TRI_02;
	else
		sideP->nType = SIDE_IS_TRI_13;

	#ifndef COMPACT_SEGS
	//	Now, based on triangulation nType, set the normals.
	if (sideP->nType == SIDE_IS_TRI_02) {
		//VmVecNormal (&norm, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v2);
		sideP->normals [0] = norm;
		VmVecNormal (&norm, gameData.segs.vertices + v0, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
		sideP->normals [1] = norm;
		}
	else {
		VmVecNormal (&norm, gameData.segs.vertices + v0, gameData.segs.vertices + v1, gameData.segs.vertices + v3);
		sideP->normals [0] = norm;
		VmVecNormal (&norm, gameData.segs.vertices + v1, gameData.segs.vertices + v2, gameData.segs.vertices + v3);
		sideP->normals [1] = norm;
		}
	#endif
	}
else {
	int	i, v [4], vSorted [4];
	int	bFlip;

	for (i=0; i<4; i++)
		v [i] = segP->verts [vs [i]];

	bFlip = GetVertsForNormal (v [0], v [1], v [2], v [3], vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
	if ((vSorted [0] == v [0]) || (vSorted [0] == v [2])) {
		sideP->nType = SIDE_IS_TRI_02;
#ifndef COMPACT_SEGS
		//	Now, get vertices for normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (v [0], v [1], v [2], 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormal (&norm, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (&norm);
		sideP->normals [0] = norm;
		bFlip = GetVertsForNormal (v [0], v [2], v [3], 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormal (&norm, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (&norm);
		sideP->normals [1] = norm;
#endif
		}
	else {
		sideP->nType = SIDE_IS_TRI_13;
#ifndef COMPACT_SEGS
		//	Now, get vertices for normal for each triangle based on triangulation nType.
		bFlip = GetVertsForNormal (v [0], v [1], v [3], 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormal (&norm, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (&norm);
		sideP->normals [0] = norm;
		bFlip = GetVertsForNormal (v [1], v [2], v [3], 32767, vSorted, vSorted + 1, vSorted + 2, vSorted + 3);
		VmVecNormal (&norm, 
						 gameData.segs.vertices + vSorted [0], 
						 gameData.segs.vertices + vSorted [1], 
						 gameData.segs.vertices + vSorted [2]);
		if (bFlip)
			VmVecNegate (&norm);
		sideP->normals [1] = norm;
#endif
		}
	}
#endif
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

pn->nFaces++;
pn->vNormal.p.x += f2fl (pvNormal->x);
pn->vNormal.p.y += f2fl (pvNormal->y);
pn->vNormal.p.z += f2fl (pvNormal->z);
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
	VmVecNormal (&vn, gameData.segs.vertices + vm0, gameData.segs.vertices + vm1, gameData.segs.vertices + vm2);
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

			CreateAbsVertexLists (&nFaces, vertexList, SEG_IDX (segP), nSide);
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
	int			i;
	g3sPoint	*pp;

for (i = gameData.segs.nVertices, pp = gameData.segs.points; i; i--, pp++) {
	pp->p3_normal.vNormal.p.x /= pp->p3_normal.nFaces;
	pp->p3_normal.vNormal.p.y /= pp->p3_normal.nFaces;
	pp->p3_normal.vNormal.p.z /= pp->p3_normal.nFaces;
	VmVecNormalizef (&pp->p3_normal.vNormal, &pp->p3_normal.vNormal);
	pp->p3_normal.nFaces = 1;
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
memset (gameData.segs.points, 0, sizeof (gameData.segs.points));
for (s = 0; s <= gameData.segs.nLastSegment; s++)
#ifdef EDITOR
	if (gameData.segs.segments [s].nSegment != -1)
#endif
		ValidateSegment (gameData.segs.segments + s);
#ifdef EDITOR
	{
	int said=0;
	for (s=gameData.segs.nLastSegment+1; s < MAX_SEGMENTS; s++)
		if (gameData.segs.segments [s].nSegment != -1) {
			if (!said) {
#if TRACE		
				con_printf (CON_DEBUG, "Segment %i has invalid nSegment.  Bashing to -1.  Silently bashing all others...", s);
#endif
				}
			said++;
			gameData.segs.segments [s].nSegment = -1;
			}
	if (said) {
#if TRACE		
		con_printf (CON_DEBUG, "%i fixed.\n", said);
#endif
		}
	}
#endif

#ifndef NDEBUG
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
//	Set the tSegment nDepth of all segments from nStartSeg in *segbuf.
//	Returns maximum nDepth value.
int SetSegmentDepths (int nStartSeg, ubyte *segbuf)
{
	int	i, curseg;
	ubyte	visited [MAX_SEGMENTS];
	int	queue [MAX_SEGMENTS];
	int	head, tail;
	int	nDepth;
	int	parent_depth=0;

	nDepth = 1;
	head = 0;
	tail = 0;

#if 1
	memset (visited, 0, sizeof (visited));
#else
	for (i=0; i<=gameData.segs.nLastSegment; i++)
		visited [i] = 0;
#endif
	if (segbuf [nStartSeg] == 0)
		return 1;

	queue [tail++] = nStartSeg;
	visited [nStartSeg] = 1;
	segbuf [nStartSeg] = nDepth++;

	if (nDepth == 0)
		nDepth = 255;

	while (head < tail) {
		curseg = queue [head++];
		parent_depth = segbuf [curseg];

		for (i=0; i<MAX_SIDES_PER_SEGMENT; i++) {
			int	childnum;

			childnum = gameData.segs.segments [curseg].children [i];
			if (childnum != -1)
				if (segbuf [childnum])
					if (!visited [childnum]) {
						visited [childnum] = 1;
						segbuf [childnum] = parent_depth+1;
						queue [tail++] = childnum;
					}
		}
	}

	return (parent_depth+1) * gameStates.render.bViewDist;
}

//these constants should match the ones in seguvs
#define	LIGHT_DISTANCE_THRESHOLD	 (F1_0*80)
#define	MAGIC_LIGHT_CONSTANT			 (F1_0*16)

#define MAX_CHANGED_SEGS 30
short changedSegs [MAX_CHANGED_SEGS];
int nChangedSegs;

//	------------------------------------------------------------------------------------------
//cast static light from a tSegment to nearby segments
void ApplyLightToSegment (tSegment *segp, vmsVector *segment_center, fix light_intensity, int nCallDepth)
{
	vmsVector	rSegmentCenter;
	fix			xDistToRSeg;
	int 			i;
	short			nSide, 
					nSegment = SEG_IDX (segp);

for (i=0;i<nChangedSegs;i++)
	if (changedSegs [i] == nSegment)
		break;
if (i == nChangedSegs) {
	COMPUTE_SEGMENT_CENTER (&rSegmentCenter, segp);
	xDistToRSeg = VmVecDistQuick (&rSegmentCenter, segment_center);

	if (xDistToRSeg <= LIGHT_DISTANCE_THRESHOLD) {
		fix	xLightAtPoint = (xDistToRSeg > F1_0) ? 
									 FixDiv (MAGIC_LIGHT_CONSTANT, xDistToRSeg) :
									 MAGIC_LIGHT_CONSTANT;

		if (xLightAtPoint >= 0) {
			segment2	*seg2p = gameData.segs.segment2s + nSegment;
			xLightAtPoint = FixMul (xLightAtPoint, light_intensity);
			if (xLightAtPoint >= F1_0)
				xLightAtPoint = F1_0-1;
			else if (xLightAtPoint <= -F1_0)
				xLightAtPoint = - (F1_0-1);
			seg2p->static_light += xLightAtPoint;
			if (seg2p->static_light < 0)	// if it went negative, saturate
				seg2p->static_light = 0;
			}	//	end if (xLightAtPoint...
		}	//	end if (xDistToRSeg...
	changedSegs [nChangedSegs++] = nSegment;
	}

if (nCallDepth < 2)
	for (nSide=0; nSide<6; nSide++) {
		if (WALL_IS_DOORWAY (segp, nSide, NULL) & WID_RENDPAST_FLAG)
			ApplyLightToSegment (&gameData.segs.segments [segp->children [nSide]], segment_center, light_intensity, nCallDepth+1);
		}
}

extern tObject *old_viewer;

//	------------------------------------------------------------------------------------------
//update the static_light field in a tSegment, which is used for tObject lighting
//this code is copied from the editor routine calim_process_all_lights ()
void ChangeSegmentLight (short nSegment, short nSide, int dir)
{
	tSegment *segp = gameData.segs.segments+nSegment;

if (WALL_IS_DOORWAY (segp, nSide, NULL) & WID_RENDER_FLAG) {
	tSide	*sideP = segp->sides+nSide;
	fix	light_intensity;
	light_intensity = gameData.pig.tex.pTMapInfo [sideP->nBaseTex].lighting + gameData.pig.tex.pTMapInfo [sideP->nOvlTex].lighting;
	light_intensity *= dir;
	nChangedSegs = 0;
	if (light_intensity) {
		vmsVector	segment_center;
		COMPUTE_SEGMENT_CENTER (&segment_center, segp);
		ApplyLightToSegment (segp, &segment_center, light_intensity, 0);
		}
	}
//this is a horrible hack to get around the horrible hack used to
//smooth lighting values when an tObject moves between segments
old_viewer = NULL;
}

//	------------------------------------------------------------------------------------------

int FindDLIndexD2X (short nSegment, short nSide)
{
int	m, 
		l = 0, 
		r = gameData.render.lights.nStatic;
dl_index	*p;
do {
	m = (l + r) / 2;
	p = gameData.render.lights.deltaIndices + m;
	if ((p->d2x.nSegment < nSegment) || ((p->d2x.nSegment == nSegment) && (p->d2x.nSide < nSide)))
		r = m - 1;
	else if ((p->d2x.nSegment > nSegment) || ((p->d2x.nSegment == nSegment) && (p->d2x.nSide > nSide)))
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

dl_index	*p;
do {
	m = (l + r) / 2;
	p = gameData.render.lights.deltaIndices + m;
	if ((p->d2.nSegment < nSegment) || ((p->d2.nSegment == nSegment) && (p->d2.nSide < nSide)))
		r = m - 1;
	else if ((p->d2.nSegment > nSegment) || ((p->d2.nSegment == nSegment) && (p->d2.nSide > nSide)))
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
	int			i, j, k;
	fix			dl, lNew, *pSegLightDelta;
	uvl			*uvlP;
	dl_index		*dliP;
	delta_light	*dlP;
	short			iSeg, iSide;

if ((dir < 0) && RemoveOglLight (nSegment, nSide, -1))
	return;
if (ToggleOglLight (nSegment, nSide, -1, dir >= 0) >= 0)
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
	if ((iSeg > nSegment) || ((iSeg == nSegment) && (iSide > nSide)))
		return;
	if ((iSeg == nSegment) && (iSide == nSide)) {
		dlP = gameData.render.lights.deltas + dliP->d2.index;
		for (j = (gameStates.render.bD2XLights ? dliP->d2x.count : dliP->d2.count); j; j--, dlP++) {
			uvlP = gameData.segs.segments [dlP->nSegment].sides [dlP->nSide].uvls;
			pSegLightDelta = &gameData.render.lights.segDeltas [dlP->nSegment][dlP->nSide];
			for (k = 0; k < 4; k++, uvlP++) {
				dl = dir * dlP->vert_light [k] * DL_SCALE;
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

//	-----------------------------------------------------------------------------

fix FindConnectedDistanceSegments (short seg0, short seg1, int nDepth, int widFlag)
{
	vmsVector	p0, p1;

COMPUTE_SEGMENT_CENTER_I (&p0, seg0);
COMPUTE_SEGMENT_CENTER_I (&p1, seg1);
return FindConnectedDistance (&p0, seg0, &p1, seg1, nDepth, widFlag);
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
	segment2	*seg2p;
	static sbyte   marked_segs [MAX_SEGMENTS];

	//	Now, all segments containing ambient lava or water sound makers are flagged.
	//	Additionally flag all segments which are within range of them.
for (i=0; i<=gameData.segs.nLastSegment; i++) {
	marked_segs [i] = 0;
	gameData.segs.segment2s [i].s2Flags &= ~s2f_bit;
	}

//	Mark all segments which are sources of the sound.
for (i=0; i<=gameData.segs.nLastSegment; i++) {
	tSegment	*segp = &gameData.segs.segments [i];
	segment2	*seg2p = &gameData.segs.segment2s [i];

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
