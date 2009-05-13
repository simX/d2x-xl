// Segment.cpp

#include "stdafx.h"
#include "dlcres.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dlc.h"
#include "robot.h"

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

double CMine::CalcLength(vms_vector *center1, vms_vector *center2)
{
	vms_vector direction; 

	// calculate distance vector between the centers
	direction.x = center1->x - center2->x; 
	direction.y = center1->y - center2->y; 
	direction.z = center1->z - center2->z; 

	// calculate the length of the new cube
	return (sqrt((double)direction.x*(double)direction.x
		 +  (double)direction.y*(double)direction.y
		 +  (double)direction.z*(double)direction.z)); 
}

// ----------------------------------------------------------------- 
// ----------------------------------------------------------------- 
double CMine::dround_off(double value, double round) {
	if (value >= 0) {
		value += round/2; 
	} else {
		value -= round/2; 
	}
	return value; 
}

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

void CMine::DeleteSegmentWalls (INT16 segnum)
{
	CDSide *side =Segments (segnum)->sides; 

for (int i = MAX_SIDES_PER_SEGMENT; i; i--, side++)
	if (side->nWall != NO_WALL)
		DeleteWall (side->nWall); 
}

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 
void CMine::DeleteSegment(INT16 delSegNum)
{
	CDSegment	*seg, *deleted_seg; 
	CDSegment	*seg2; 
	CDObject		*obj; 
	CDTrigger	*trigP;
	UINT16		segnum, real_segnum; 
	INT16			child; 
	//  UINT8 vert_check = 0;  // a 1 means that the vertex is being used by another segment
	INT16			i, j; 
	//  INT16 vert, unused_vert; 

	if (SegCount () < 2)
		return; 
	if (delSegNum < 0)
		delSegNum = Current ()->segment; 
	if (delSegNum < 0 || delSegNum >= SegCount ()) 
		return; 

	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	deleted_seg = Segments (delSegNum); 
	UndefineSegment (delSegNum);

	// delete any flickering lights that use this segment
	for (int sidenum = 0; sidenum < 6; sidenum++) {
		DeleteTriggerTargets (delSegNum, sidenum); 
		INT16 index = GetFlickeringLight(delSegNum, sidenum); 
		if (index != -1) {
			FlickerLightCount ()--; 
			// put last light in place of deleted light
			memcpy(FlickeringLights (index), FlickeringLights (FlickerLightCount ()), 
				sizeof(FLICKERING_LIGHT)); 
		}
	}

	// delete any Walls () within segment (if defined)
	DeleteSegmentWalls (delSegNum); 

	// delete any Walls () on child Segments () that connect to this segment
	for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
		child = deleted_seg->children [i]; 
		if (child >= 0 && child < SegCount ()) {
			INT16	oppSegNum, oppSideNum;
			GetOppositeSide (oppSegNum, oppSideNum, delSegNum, i);
			if (Segments (oppSegNum)->sides [oppSideNum].nWall != NO_WALL)
				DeleteWall (Segments (oppSegNum)->sides [oppSideNum].nWall); 
			}
		}

	// delete any Objects () within segment
	for (i = (UINT16)GameInfo ().objects.count - 1; i >= 0; i--) {
		if (Objects (i)->segnum == delSegNum) {
			DeleteObject(i); 
		}
	}
#if 0 // done by UndefineSegment ()
	// delete any robot centers with this
	for (i = (UINT16)GameInfo ().botgen.count - 1; i >= 0; i--) {
		segnum = BotGens (i)->segnum; 
		if (segnum == delSegNum) {
			int nMatCens = --GameInfo ().botgen.count; 
			if (i < nMatCens)
			memcpy ((void *) BotGens (i), (void *) BotGens (nMatCens), sizeof (matcen_info)); 
			}
		}

	// delete any equipment centers with this
	for (i = (UINT16) GameInfo ().equipgen.count - 1; i >= 0; i--) {
		segnum = EquipGens (i)->segnum; 
		if (segnum == delSegNum) {
			GameInfo ().equipgen.count--; 
			memcpy ((void *) EquipGens (i), (void *) EquipGens (i + 1), 
					  (GameInfo ().equipgen.count - i) * sizeof (matcen_info)); 
			}
		}
#endif
	for (j = 0; j < GameInfo ().botgen.count; j++)
		if (BotGens (i)->segnum > delSegNum)
			BotGens (i)->segnum--;
	for (j = 0; j < GameInfo ().equipgen.count; j++)
		if (EquipGens (i)->segnum > delSegNum)
			EquipGens (i)->segnum--;
	// delete any control seg with this segment
	for (i = (UINT16)GameInfo ().control.count - 1; i >= 0; i--) {
		int num_links = CCTriggers (i)->num_links; 
		for (j = num_links - 1; j>0; j--) {
			segnum = CCTriggers (i)->seg [j]; 
			if (segnum == delSegNum) {
				// move last segment into this spot
				CCTriggers (i)->seg [j]  = 
					CCTriggers (i)->seg [num_links - 1]; 
				CCTriggers (i)->side [j]  = 
					CCTriggers (i)->side [num_links - 1]; 
				CCTriggers (i)->seg [num_links - 1] = 0; 
				CCTriggers (i)->side [num_links - 1] = 0; 
				CCTriggers (i)->num_links--; 
			}
		}
	}

	// update secret cube number if out of range now
	segnum = (UINT16) SecretCubeNum (); 
	if (segnum >= SegCount () || segnum== delSegNum) {
		SecretCubeNum () = 0; 
	}

	// update segment flags
	deleted_seg->wall_bitmask &= (~MARKED_MASK); 

	// unlink any children with this segment number
	for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++) {
		for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
			if (seg->children [child]== delSegNum) {

				// subtract by 1 if segment is above deleted segment
				Current ()->segment = segnum; 
				if (segnum > delSegNum) {
					Current ()->segment--; 
				}

				// remove child number and update child bitmask
				seg->children [child] =-1; 
				seg->child_bitmask &= ~(1 << child); 

				// define textures, (u, v) and light
				CDSide *side = deleted_seg->sides + child;
				SetTexture (segnum, child, side->nBaseTex, side->nOvlTex); 
				SetUV (segnum, child, 0, 0, 0); 
				double scale = pTextures [file_type][side->nBaseTex].Scale (side->nBaseTex);
				for (i = 0; i < 4; i++) {
					seg->sides [child].uvls [i].u = (INT16) ((double) default_uvls [i].u / scale); 
					seg->sides [child].uvls [i].v = (INT16) ((double) default_uvls [i].v / scale); 
					seg->sides [child].uvls [i].l = deleted_seg->sides [child].uvls [i].l; 
				}
			}
		}
	}

	// move other Segments () to deleted segment location
	if (delSegNum != SegCount ()-1) { // if this is not the last segment

		// mark each segment with it's real number
		real_segnum = 0; 
		for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++)
			if(delSegNum != segnum)
				seg->seg_number = real_segnum++; 

		// replace all children with real numbers
		for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++) {
			for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
				if (seg->child_bitmask & (1 << child)
					&& seg->children [child] >= 0 && seg->children [child] < SegCount ()) { // debug fix
					seg2 = &Segments () [seg->children [child]]; 
					seg->children [child] = seg2->seg_number; 
				}
			}
		}

		// replace all wall seg numbers with real numbers
		for (i = 0; i < GameInfo ().walls.count; i++) {
			segnum = (INT16)Walls (i)->segnum; 
			if (segnum < SegCount ()) {
				seg = Segments (segnum); 
				Walls (i)->segnum = seg->seg_number; 
			} else {
				Walls (i)->segnum = 0; // fix wall segment number
			}
		}

		// replace all trigger seg numbers with real numbers
		for (i = NumTriggers (), trigP = Triggers (); i; i--, trigP++) {
			for (j = 0; j < trigP->num_links; j++) {
				segnum = trigP->seg [j]; 
				if (segnum < SegCount ()) {
					seg = Segments (segnum); 
					trigP->seg [j] = seg->seg_number; 
					}
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all trigger seg numbers with real numbers
		for (i = NumObjTriggers (), trigP = ObjTriggers (); i; i--, trigP++) {
			for (j = 0; j < trigP->num_links; j++) {
				segnum = trigP->seg [j]; 
				if (segnum < SegCount ()) {
					seg = Segments (segnum); 
					trigP->seg [j] = seg->seg_number; 
					}
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all object seg numbers with real numbers
		for (i = 0; i < GameInfo ().objects.count; i++) {
			obj = Objects (i); 
			segnum = obj->segnum; 
			if (segnum < SegCount ()) {
				seg = Segments (segnum); 
				obj->segnum = seg->seg_number; 
			} else {
				obj->segnum = 0; // fix object segment number
			}
		}

		// replace robot centers seg numbers with real numbers
		for (i = 0; i < GameInfo ().botgen.count; i++) {
			segnum = BotGens (i)->segnum; 
			if (segnum < SegCount ()) {
				seg = Segments (segnum); 
				BotGens (i)->segnum = seg->seg_number; 
			} else {
				BotGens (i)->segnum = 0; // fix robot center segnum
			}
		}

		// replace equipment centers seg numbers with real numbers
		for (i = 0; i < GameInfo ().equipgen.count; i++) {
			segnum = EquipGens (i)->segnum; 
			if (segnum < SegCount ()) {
				seg = Segments (segnum); 
				EquipGens (i)->segnum = seg->seg_number; 
			} else {
				EquipGens (i)->segnum = 0; // fix robot center segnum
			}
		}

		// replace control seg numbers with real numbers
		for (i = 0; i < GameInfo ().control.count; i++) {
			for (j = 0; j < CCTriggers (i)->num_links; j++) {
				segnum = CCTriggers (i)->seg [j]; 
				if (segnum < SegCount ()) {
					seg = Segments (segnum); 
					CCTriggers (i)->seg [j] = seg->seg_number; 
				} else {
					CCTriggers (i)->seg [j] = 0; // fix control center segment number
				}
			}
		}

		// replace flickering light seg numbers with real numbers
		for (i = 0; i < FlickerLightCount (); i++) {
			segnum = FlickeringLights (i)->segnum; 
			if (segnum < SegCount ()) {
				seg = Segments (segnum); 
				FlickeringLights (i)->segnum = seg->seg_number; 
			} else {
				FlickeringLights (i)->segnum = 0; // fix object segment number
			}
		}

		// replace secret cubenum with real number
		segnum = (UINT16) SecretCubeNum (); 
		if (segnum < SegCount ()) {
			seg = Segments (segnum); 
			SecretCubeNum () = seg->seg_number; 
		} else {
			SecretCubeNum () = 0; // fix secret cube number
		}

		// move remaining Segments () down by 1
  }
#if 1
		if (int segC = (--SegCount () - delSegNum)) {
			memcpy (Segments (delSegNum), Segments (delSegNum + 1), segC * sizeof (CDSegment));
			memcpy (LightColors (delSegNum), LightColors (delSegNum + 1), segC * 6 * sizeof (CDColor));
			}
#else
		for (segnum = delSegNum; segnum < (SegCount ()-1); segnum++) {
			seg = Segments (segnum); 
			seg2 = Segments (segnum + 1); 
			memcpy(seg, seg2, sizeof(CDSegment)); 
			}
  SegCount ()-- ; 
#endif


  // delete all unused vertices
  DeleteUnusedVertices(); 

  // make sure current segment numbers are valid
  if (Current1 ().segment >= SegCount ()) Current1 ().segment--; 
  if (Current2 ().segment >= SegCount ()) Current2 ().segment--; 
  if (Current1 ().segment < 0) Current1 ().segment = 0; 
  if (Current2 ().segment < 0) Current2 ().segment = 0; 
theApp.MineView ()->Refresh (false); 
theApp.ToolView ()->Refresh (); 
theApp.UnlockUndo ();
}



// -------------------------------------------------------------------------- 
// DeleteVertex()
//
// ACTION - Removes a vertex from the vertices array and updates all the
//	    Segments () vertices who's vertex is greater than the deleted vertex
// -------------------------------------------------------------------------- 

void CMine::DeleteVertex(INT16 deleted_vertnum)
{
	INT16 vertnum, segnum; 

	theApp.SetModified (TRUE); 
	// fill in gap in vertex array and status
	memcpy (Vertices (deleted_vertnum), Vertices (deleted_vertnum + 1), (VertCount ()-1 - deleted_vertnum) * sizeof (vms_vector));
	memcpy (VertStatus (deleted_vertnum), VertStatus (deleted_vertnum + 1), (VertCount ()-1 - deleted_vertnum) * sizeof (*VertStatus ()));
/*
	for (vertnum = deleted_vertnum; vertnum < VertCount ()-1; vertnum++) {
		memcpy(&vertices [vertnum], &vertices [vertnum + 1], sizeof(vms_vector)); 
		*VertStatus (vertnum] = *VertStatus (vertnum + 1]; 
	}
*/
	// update anyone pointing to this vertex
	CDSegment *seg = Segments ();
	for (segnum = 0; segnum < SegCount (); segnum++, seg++)
		for (vertnum = 0; vertnum < 8; vertnum++)
			if (seg->verts [vertnum] > deleted_vertnum)
				seg->verts [vertnum]--; 

	// update number of vertices
	VertCount ()--; 
}

// -------------------------------------------------------------------------- 
// DeleteUnusedVertices()
//
// ACTION - Deletes unused vertices
// -------------------------------------------------------------------------- 

void CMine::DeleteUnusedVertices()
{
	INT16 vertnum, segnum, point; 

for (vertnum = 0; vertnum < VertCount (); vertnum++)
	*VertStatus (vertnum) &= ~NEW_MASK; 
// mark all used verts
CDSegment *seg = Segments ();
for (segnum = 0; segnum < SegCount (); segnum++, seg++)
	for (point = 0; point < 8; point++)
		*VertStatus (seg->verts [point]) |= NEW_MASK; 
for (vertnum = VertCount ()-1; vertnum >= 0; vertnum--)
	if (!(*VertStatus (vertnum) & NEW_MASK))
		DeleteVertex(vertnum); 
}

// -------------------------------------------------------------------------- 
//	AddSegment()
//
//  ACTION - Add new segment at the end. solidifyally joins to adjacent
//           Segments () if sides are identical.  Uses other current cube for
//           textures.
//
//  Returns - TRUE on success
//
//  Changes - Now auto aligns u, v numbers based on parent textures
//
//  NEW - If there is a flickering light on the current side of this segment, 
//        it is deleted.
//
//        If cube is special (fuel center, robot maker, etc..) then textures
//        are set to default texture.
// -------------------------------------------------------------------------- 

void CMine::InitSegment (INT16 segNum)
{
	CDSegment	*segP = Segments (segNum);
	INT16			sideNum;

// define special, etc..
segP->owner = -1;
segP->group = -1;
segP->special = 0; 
segP->matcen_num = -1; 
segP->value = -1; 
segP->child_bitmask = 0;
// define Walls ()
segP->wall_bitmask = 0; // unmarked cube
for (sideNum = 0; sideNum < MAX_SIDES_PER_SEGMENT; sideNum++) {
	segP->sides [sideNum].nWall = NO_WALL; 
	segP->sides [sideNum].nBaseTex =
	segP->sides [sideNum].nOvlTex = 0; 
	for (int i = 0; i < 4; i++)
		segP->sides [sideNum].uvls [i].l = (UINT16) DEFAULT_LIGHTING; 
	SetUV (segNum, sideNum, 0, 0, 0); 
	}
segP->static_light = 0; 
memset (segP->children, 0xFF, sizeof (segP->children));
}

// -------------------------------------------------------------------------- 

bool CMine::AddSegment ()
{
	CDSegment *seg, *currSeg; 
	INT16 i, nNewSeg, nNewSide, nCurrSide = Current ()->side; 
	INT16 new_verts [4]; 
	INT16 segnum, sidenum; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return FALSE; 
	}

currSeg = Segments (Current ()->segment); 

if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg("Cannot add a new cube because\nthe maximum number of cubes has been reached."); 
	return FALSE;
	}
if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg("Cannot add a new cube because\nthe maximum number of vertices has been reached."); 
	return FALSE;
	}
if (currSeg->children [nCurrSide] >= 0) {
	ErrorMsg("Can not add a new cube to a side\nwhich already has a cube attached."); 
	return FALSE;
	}

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// get new verts
new_verts [0] = VertCount () + 0; 
new_verts [1] = VertCount () + 1; 
new_verts [2] = VertCount () + 2; 
new_verts [3] = VertCount () + 3; 

// get new segment
nNewSeg = SegCount (); 
seg = Segments (nNewSeg); 

// define vertices
DefineVertices (new_verts); 

// define vert numbers for common side
seg->verts [opp_side_vert [nCurrSide][0]] = currSeg->verts [side_vert [nCurrSide][0]]; 
seg->verts [opp_side_vert [nCurrSide][1]] = currSeg->verts [side_vert [nCurrSide][1]]; 
seg->verts [opp_side_vert [nCurrSide][2]] = currSeg->verts [side_vert [nCurrSide][2]]; 
seg->verts [opp_side_vert [nCurrSide][3]] = currSeg->verts [side_vert [nCurrSide][3]]; 

// define vert numbers for new side
seg->verts [side_vert [nCurrSide][0]] = new_verts [0]; 
seg->verts [side_vert [nCurrSide][1]] = new_verts [1]; 
seg->verts [side_vert [nCurrSide][2]] = new_verts [2]; 
seg->verts [side_vert [nCurrSide][3]] = new_verts [3]; 

InitSegment (nNewSeg);
// define children and special child
seg->child_bitmask = 1 << opp_side [nCurrSide]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	seg->children [i] = (seg->child_bitmask & (1 << i)) ? Current ()->segment : -1;

// define textures
for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
	if (seg->children [sidenum] < 0) {
		// if other segment does not have a child (therefore it has a texture)
		if (currSeg->children [sidenum] < 0 && currSeg->special == SEGMENT_IS_NOTHING) {
			seg->sides [sidenum].nBaseTex = currSeg->sides [sidenum].nBaseTex; 
			seg->sides [sidenum].nOvlTex = currSeg->sides [sidenum].nOvlTex; 
			for (i = 0; i < 4; i++) 
				seg->sides [sidenum].uvls [i].l = currSeg->sides [sidenum].uvls [i].l; 
			} 
		}
	else {
		memset (seg->sides [sidenum].uvls, 0, sizeof (seg->sides [sidenum].uvls));
		}
	}

// define static light
seg->static_light = currSeg->static_light; 

// delete flickering light if it exists
INT16 index = GetFlickeringLight (Current ()->segment, nCurrSide); 
if (index != -1) {
	FlickerLightCount ()--; 
	// put last light in place of deleted light
	memcpy( FlickeringLights (index), FlickeringLights (FlickerLightCount ()), sizeof (FLICKERING_LIGHT)); 
	}

// update current segment
currSeg->children [nCurrSide] = nNewSeg; 
currSeg->child_bitmask |= (1 << nCurrSide); 
currSeg->sides [nCurrSide].nBaseTex = 0; 
currSeg->sides [nCurrSide].nOvlTex = 0; 
memset (currSeg->sides [nCurrSide].uvls, 0, sizeof (currSeg->sides [nCurrSide].uvls));
 
// update number of Segments () and vertices and clear vert_status
SegCount ()++; 
memset (VertStatus (VertCount ()), 0, 4 * sizeof (*VertStatus ()));
VertCount () += 4;


// link the new segment with any touching Segments ()
CDSegment *pSeg = Segments ();
vms_vector *vNewSeg = Vertices (Segments (nNewSeg)->verts [0]);
vms_vector *vSeg;
for (segnum = 0; segnum < SegCount (); segnum++, pSeg++) {
	if (segnum!= nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (pSeg->verts [0]);
		if (labs (vNewSeg->x - vSeg->x) < 0xA00000L &&
			 labs (vNewSeg->y - vSeg->y) < 0xA00000L &&
			 labs (vNewSeg->z - vSeg->z) < 0xA00000L)
			for (nNewSide = 0; nNewSide < 6; nNewSide++)
				for (sidenum = 0; sidenum < 6; sidenum++)
					LinkSegments(nNewSeg,nNewSide,segnum,sidenum,3*F1_0);
		}
	}
// auto align textures new segment
for (nNewSide = 0; nNewSide < 6; nNewSide++)
	AlignTextures (Current ()->segment, nNewSide, nNewSeg, TRUE, TRUE); 
// set current segment to new segment
Current ()->segment = nNewSeg; 
//		SetLinesToDraw(); 
theApp.MineView ()->Refresh (false); 
theApp.ToolView ()->Refresh (); 
theApp.UnlockUndo ();
return TRUE; 
}

// -------------------------------------------------------------------------- 
// DefineVertices()
//
//  ACTION - Calculates vertices when adding a new segment.
//
// -------------------------------------------------------------------------- 

#define CURRENT_POINT(a) ((Current ()->point + (a))&0x03)

void CMine::DefineVertices (INT16 new_verts [4])
{
	CDSegment *currSeg; 
	struct dvector A [8], B [8], C [8], D [8], E [8], a, b, c, d; 
	double angle1, angle2, angle3; 
	double length; 
	INT16 vertnum; 
	INT16 i; 
	vms_vector center, opp_center, orthog; 
	vms_vector *vert, new_center; 

	currSeg = Segments (Current ()->segment); 

	// METHOD 1: orthogonal with right angle on new side and standard cube side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
	switch (add_segment_mode)
	{
		case(ORTHOGONAL):
		{
			CalcCenter(center, Current ()->segment, Current ()->side); 
			CalcCenter(opp_center, Current ()->segment, opp_side [Current ()->side]); 

			CalcOrthoVector(orthog, Current ()->segment, Current ()->side); 

			// set the length of the new cube to be one standard cube length
			length = 20; 

			// scale the vector
			orthog.x = (long) ((double) orthog.x * length); 
			orthog.y = (long) ((double) orthog.y * length); 
			orthog.z = (long) ((double) orthog.z * length); 

			// figure out new center
			new_center.x = center.x + orthog.x; 
			new_center.y = center.y + orthog.y; 
			new_center.z = center.z + orthog.z; 

			// new method: extend points 0 and 1 with orthog, then move point 0 toward point 1.
			double factor; 

			// point 0
			vertnum = currSeg->verts [side_vert [Current ()->side][CURRENT_POINT(0)]];
			vert = Vertices (vertnum);
			a.x = orthog.x + vert->x; 
			a.y = orthog.y + vert->y; 
			a.z = orthog.z + vert->z; 

			// point 1
			vertnum = currSeg->verts [side_vert [Current ()->side][CURRENT_POINT(1)]]; 
			vert = Vertices (vertnum);
			b.x = orthog.x + vert->x; 
			b.y = orthog.y + vert->y; 
			b.z = orthog.z + vert->z; 

			// center
			c.x = (a.x + b.x) / 2; 
			c.y = (a.y + b.y) / 2; 
			c.z = (a.z + b.z) / 2; 

			// vector from center to point0 and its length
			d.x = a.x - c.x; 
			d.y = a.y - c.y; 
			d.z = a.z - c.z; 
			length = sqrt(d.x*d.x + d.y*d.y + d.z*d.z); 

			// factor to mul
			if (length > 0) {
				factor = 10.0*F1_0 /length; 
			} else {
				factor = 1.0; 
			}

			// set point 0
			A [CURRENT_POINT(0)].x = (c.x + factor * d.x); 
			A [CURRENT_POINT(0)].y = (c.y + factor * d.y); 
			A [CURRENT_POINT(0)].z = (c.z + factor * d.z); 

			// set point 1
			A [CURRENT_POINT(1)].x = (c.x - factor * d.x); 
			A [CURRENT_POINT(1)].y = (c.y - factor * d.y); 
			A [CURRENT_POINT(1)].z = (c.z - factor * d.z); 

			// point 2 is orthogonal to the vector 01 and the orthog vector
			a.x = orthog.x; 
			a.y = orthog.y; 
			a.z = orthog.z; 
			b.x = A [CURRENT_POINT(0)].x - A [CURRENT_POINT(1)].x; 
			b.y = A [CURRENT_POINT(0)].y - A [CURRENT_POINT(1)].y; 
			b.z = A [CURRENT_POINT(0)].z - A [CURRENT_POINT(1)].z; 
			c.x = a.y*b.z - a.z*b.y; 
			c.y = a.z*b.x - a.x*b.z; 
			c.z = a.x*b.y - a.y*b.x; 
			// normalize the vector
			length = sqrt(c.x*c.x + c.y*c.y + c.z*c.z); 
			if (length>0) {
				c.x /= length; 
				c.y /= length; 
				c.z /= length; 
			}
			A [CURRENT_POINT(2)].x = A [CURRENT_POINT(1)].x + c.x * 20*F1_0; 
			A [CURRENT_POINT(2)].y = A [CURRENT_POINT(1)].y + c.y * 20*F1_0; 
			A [CURRENT_POINT(2)].z = A [CURRENT_POINT(1)].z + c.z * 20*F1_0; 

			A [CURRENT_POINT(3)].x = A [CURRENT_POINT(0)].x + c.x * 20*F1_0; 
			A [CURRENT_POINT(3)].y = A [CURRENT_POINT(0)].y + c.y * 20*F1_0; 
			A [CURRENT_POINT(3)].z = A [CURRENT_POINT(0)].z + c.z * 20*F1_0; 

			// now center the side along about the new_center
			a.x = (A [0].x + A [1].x + A [2].x + A [3].x)/4; 
			a.y = (A [0].y + A [1].y + A [2].y + A [3].y)/4; 
			a.z = (A [0].z + A [1].z + A [2].z + A [3].z)/4; 
			for (i = 0; i < 4; i++) {
				A [i].x += new_center.x - a.x; 
				A [i].y += new_center.y - a.y; 
				A [i].z += new_center.z - a.z; 
			}

			// set the new vertices
			for (i = 0; i < 4; i++) {
				//vertnum = currSeg->verts [side_vert [Current ()->side][i]]; 
				vertnum = new_verts [i];
				Vertices (vertnum)->x = (long) dround_off(A [i].x, 1.0); 
				Vertices (vertnum)->y = (long) dround_off(A [i].y, 1.0); 
				Vertices (vertnum)->z = (long) dround_off(A [i].z, 1.0); 
			}
		}
		break; 
		// METHOD 2: orghogonal with right angle on new side
		case(EXTEND):
		{
			CalcCenter(center, Current ()->segment, Current ()->side); 
			CalcCenter(opp_center, Current ()->segment, opp_side [Current ()->side]); 

			CalcOrthoVector(orthog, Current ()->segment, Current ()->side); 

			// calculate the length of the new cube
			length = CalcLength(&center, &opp_center) / 0x10000L; 

			// scale the vector
			orthog.x = (long) ((double) orthog.x * length); 
			orthog.y = (long) ((double) orthog.y * length); 
			orthog.z = (long) ((double) orthog.z * length); 

			// set the new vertices
			for (i = 0; i < 4; i++) {
				int v1 = currSeg->verts [side_vert [Current ()->side][i]]; 
				int v2 = new_verts [i];
				Vertices (v2)->x = orthog.x + Vertices (v1)->x; 
				Vertices (v2)->y = orthog.y + Vertices (v1)->y; 
				Vertices (v2)->z = orthog.z + Vertices (v1)->z; 
			}
		}
		break; 

		// METHOD 3: mirror relative to plane of side
		case(MIRROR):
		{
			// copy side's four points into A
			for (i = 0; i < 4; i++) {
				vertnum = currSeg->verts [side_vert [Current ()->side][i]]; 
				A [i].x = Vertices (vertnum)->x; 
				A [i].y = Vertices (vertnum)->y; 
				A [i].z = Vertices (vertnum)->z; 
				vertnum = currSeg->verts [opp_side_vert [Current ()->side][i]]; 
				A [i + 4].x = Vertices (vertnum)->x; 
				A [i + 4].y = Vertices (vertnum)->y; 
				A [i + 4].z = Vertices (vertnum)->z; 
			}

			// subtract point 0 from all points in A to form B points
			for (i = 0; i < 8; i++) {
				B [i].x = A [i].x - A [0].x; 
				B [i].y = A [i].y - A [0].y; 
				B [i].z = A [i].z - A [0].z; 
			}

			// calculate angle to put point 1 in x - y plane by spinning on x - axis
			// then rotate B points on x - axis to form C points.
			// check to see if on x - axis already
			//    if (B [1].z== B [1].y) {
			//      angle1 = PI/4; 
			//    } else {
			angle1 = atan3(B [1].z, B [1].y); 
			//    }
			for (i = 0; i < 8; i++) {
				C [i].x = B [i].x; 
				C [i].y = B [i].y * cos(angle1) + B [i].z * sin(angle1); 
				C [i].z = - B [i].y * sin(angle1) + B [i].z * cos(angle1); 
			}

			// calculate angle to put point 1 on x axis by spinning on z - axis
			// then rotate C points on z - axis to form D points
			// check to see if on z - axis already
			//    if (C [1].y== C [1].x) {
			//      angle2 = PI/4; 
			//    } else {
			angle2 = atan3(C [1].y, C [1].x); 
			//    }
			for (i = 0; i < 8; i++) {
				D [i].x = C [i].x * cos(angle2) + C [i].y * sin(angle2); 
				D [i].y = - C [i].x * sin(angle2) + C [i].y * cos(angle2); 
				D [i].z = C [i].z; 
			}

			// calculate angle to put point 2 in x - y plane by spinning on x - axis
			// the rotate D points on x - axis to form E points
			// check to see if on x - axis already
			//    if (D [2].z== D [2].y) {
			//      angle3 = PI/4; 
			//    } else {
			angle3 = atan3(D [2].z, D [2].y); 
			//    }
			for (i = 0; i < 8; i++) {
				E [i].x = D [i].x; 
				E [i].y = D [i].y * cos(angle3) + D [i].z * sin(angle3); 
				E [i].z = - D [i].y * sin(angle3) + D [i].z * cos(angle3); 
			}

			// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
			// mirror new points on z axis
			for (i = 4; i < 8; i++) {
				E [i].z = - E [i].z; 
			}

			// now reverse rotations
			for (i = 4; i < 8; i++) {
				D [i].x = E [i].x; 
				D [i].y = E [i].y * cos(- angle3) + E [i].z * sin(- angle3); 
				D [i].z = - E [i].y * sin(- angle3) + E [i].z * cos(- angle3); 
			}
			for (i = 4; i < 8; i++) {
				C [i].x = D [i].x * cos(- angle2) + D [i].y * sin(- angle2); 
				C [i].y = - D [i].x * sin(- angle2) + D [i].y * cos(- angle2); 
				C [i].z = D [i].z; 
			}
			for (i = 4; i < 8; i++) {
				B [i].x = C [i].x; 
				B [i].y = C [i].y * cos(- angle1) + C [i].z * sin(- angle1); 
				B [i].z = - C [i].y * sin(- angle1) + C [i].z * cos(- angle1); 
			}
			// and translate back
			vertnum = currSeg->verts [side_vert [Current ()->side][0]]; 
			for (i = 4; i < 8; i++) {
				A [i].x = B [i].x + Vertices (vertnum)->x; 
				A [i].y = B [i].y + Vertices (vertnum)->y; 
				A [i].z = B [i].z + Vertices (vertnum)->z; 
			}

			for (i = 0; i < 4; i++) {
				int vertnum = new_verts [i];
				Vertices (vertnum)->x = (long) dround_off(A [i + 4].x, 1.0); 
				Vertices (vertnum)->y = (long) dround_off(A [i + 4].y, 1.0); 
				Vertices (vertnum)->z = (long) dround_off(A [i + 4].z, 1.0); 
			}
		}
	}
}

// -------------------------------------------------------------------------- 
// LinkSegments()
//
//  Action - checks 2 Segments () and 2 sides to see if the vertices are identical
//           If they are, then the segment sides are linked and the vertices
//           are removed (sidenum1 is the extra side).
//
//  Change - no longer links if segment already has a child
//           no longer links Segments () if vert numbers are not in the right order
//
// -------------------------------------------------------------------------- 

bool CMine::LinkSegments (INT16 segnum1, INT16 sidenum1, INT16 segnum2, INT16 sidenum2, FIX margin)
{
	CDSegment *seg1, *seg2; 
	INT16 i, j; 
	vms_vector v1 [4], v2 [4]; 
	INT16 fail;
	tVertMatch match [4]; 

	seg1 = Segments (segnum1); 
	seg2 = Segments (segnum2); 

// don't link to a segment which already has a child
if (seg1->children [sidenum1]!=-1 || seg2->children [sidenum2]!=-1)
	return FALSE; 

// copy vertices for comparison later (makes code more readable)
for (i = 0; i < 4; i++) {
	int vertnum = seg1->verts [side_vert [sidenum1][i]];
	memcpy (v1 + i, Vertices (vertnum), sizeof (*Vertices ()));
/*
	v1 [i].x = Vertices (vertnum)->x; 
	v1 [i].y = Vertices (vertnum)->y; 
	v1 [i].z = Vertices (vertnum)->z; 
*/
	vertnum = seg2->verts [side_vert [sidenum2][i]];
	memcpy (v2 + i, Vertices (vertnum), sizeof (*Vertices ()));
/*
	v2 [i].x = Vertices (vertnum)->x; 
	v2 [i].y = Vertices (vertnum)->y; 
	v2 [i].z = Vertices (vertnum)->z; 
*/
	match [i].i =-1; 
}

// check to see if all 4 vertices match exactly one of each of the 4 other cube's vertices
fail = 0;   // assume test will pass for now
for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		if (labs (v1 [i].x - v2 [j].x) < margin &&
			 labs (v1 [i].y - v2 [j].y) < margin &&
			 labs (v1 [i].z - v2 [j].z) < margin)
			if (match [j].i != -1) // if this vertex already matched another vertex then abort
				return FALSE; 
			else
				match [j].i = i;  // remember which vertex it matched
/*
for (i = 0; i < 4; i++)
	match [i] = -1;
for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		if (labs (v1 [i].x - v2 [j].x) < margin &&
			 labs (v1 [i].y - v2 [j].y) < margin &&
			 labs (v1 [i].z - v2 [j].z) < margin)
			match [i] = j;  // remember which vertex it matched
*/
if (match [0].i == -1)
	return FALSE;
static int matches [][4] = {{0,3,2,1},{1,0,3,2},{2,1,0,3},{3,2,1,0}};
for (i = 1; i < 4; i++)
	if (match [i].i != matches [match [0].i][i])
		return FALSE;
// make sure verts match in the correct order
/*
if ((match [0] == 0) && (match [1] != 3 || match [2] != 2 || match [3] != 1)) fail = 1; 
else if ((match [0] == 1) && (match [1] != 0 || match [2] != 3 || match [3] != 2)) fail = 1; 
else if ((match [0] == 2) && (match [1] != 1 || match [2] != 0 || match [3] != 3)) fail = 1; 
else if ((match [0] == 3) && (match [1] != 2 || match [2] != 1 || match [3] != 0)) fail = 1; 
*/
// if not failed and match found for each
LinkSides (segnum1, sidenum1, segnum2, sidenum2, match); 
return TRUE; 
}


// -------------------------------------------------------------------------- 
// LinkSides()
// -------------------------------------------------------------------------- 

void CMine::LinkSides (INT16 segnum1, INT16 sidenum1, INT16 segnum2, INT16 sidenum2, tVertMatch match [4]) 
{
	CDSegment *seg1, *seg2; 
	seg1 = Segments (segnum1); 
	seg2 = Segments (segnum2); 
	int i; 
	INT16 segnum, vertnum, oldVertex, newVertex; 

	seg1->children [sidenum1] = segnum2; 
	seg1->child_bitmask |= (1 << sidenum1); 
	seg1->sides [sidenum1].nBaseTex = 0; 
	seg1->sides [sidenum1].nOvlTex = 0; 
	for (i = 0; i < 4; i++) {
		seg1->sides [sidenum1].uvls [i].u = 0; 
		seg1->sides [sidenum1].uvls [i].v = 0; 
		seg1->sides [sidenum1].uvls [i].l = 0; 
	}
	seg2->children [sidenum2] = segnum1; 
	seg2->child_bitmask |= (1 << sidenum2); 
	seg2->sides [sidenum2].nBaseTex = 0; 
	seg2->sides [sidenum2].nOvlTex = 0; 
	for (i = 0; i < 4; i++) {
		seg2->sides [sidenum2].uvls [i].u = 0; 
		seg2->sides [sidenum2].uvls [i].v = 0; 
		seg2->sides [sidenum2].uvls [i].l = 0; 
	}

	// merge vertices
	for (i = 0; i < 4; i++) {
		oldVertex = seg1->verts [side_vert [sidenum1][i]]; 
		newVertex = seg2->verts [side_vert [sidenum2][match [i].i]]; 

		// if either vert was marked, then mark the new vert
		*VertStatus (newVertex) |= *VertStatus (oldVertex) & MARKED_MASK; 

		// update all Segments () that use this vertex
		if (oldVertex != newVertex) {
			CDSegment *seg = Segments ();
			for (segnum = 0; segnum < SegCount (); segnum++, seg++)
				for (vertnum = 0; vertnum < 8; vertnum++)
					if (seg->verts [vertnum] == oldVertex)
						seg->verts [vertnum] = newVertex; 
			// then delete the vertex
			DeleteVertex (oldVertex); 
		}
	}
}
// ------------------------------------------------------------------------- 
// calculate_segment_center()
// ------------------------------------------------------------------------- 

void CMine::CalcSegCenter(vms_vector &pos, INT16 segnum) 
{
  INT16	*nVerts =Segments (segnum)->verts; 
  vms_vector *vert;
  
memset (&pos, 0, sizeof (pos));
for (int i = 0; i < 8; i++) {
	vert = Vertices (nVerts [i]);
	pos.x += vert->x;
	pos.y += vert->y;
	pos.z += vert->z;
	}
pos.x /= 8;
pos.y /= 8;
pos.z /= 8;
/*
  pos.x  = 
      (Vertices (verts [0])->x
       + Vertices (verts [1])->x
       + Vertices (verts [2])->x
       + Vertices (verts [3])->x
       + Vertices (verts [4])->x
       + Vertices (verts [5])->x
       + Vertices (verts [6])->x
       + Vertices (verts [7])->x)/8; 
  pos.y  = 
      (Vertices (verts [0])->y
       + Vertices (verts [1])->y
       + Vertices (verts [2])->y
       + Vertices (verts [3])->y
       + Vertices (verts [4])->y
       + Vertices (verts [5])->y
       + Vertices (verts [6])->y
       + Vertices (verts [7])->y)/8; 
  pos.z  = 
      (Vertices (verts [0])->z
       + Vertices (verts [1])->z
       + Vertices (verts [2])->z
       + Vertices (verts [3])->z
       + Vertices (verts [4])->z
       + Vertices (verts [5])->z
       + Vertices (verts [6])->z
       + Vertices (verts [7])->z)/8; 
*/
}

//========================================================================== 
// SideIsMarked
//========================================================================== 

bool CMine::SideIsMarked (INT16 segnum, INT16 sidenum)
{
GetCurrent (segnum, sidenum);
CDSegment *seg = Segments (segnum);
for (int i = 0;  i < 4; i++) {
	if (!(*VertStatus (seg->verts [side_vert [sidenum][i]]) & MARKED_MASK))
		return false;
	}
return true;
}

bool CMine::SegmentIsMarked (INT16 segnum)
{
CDSegment *seg = Segments (segnum);
for (int i = 0;  i < 8; i++)
	if (!(*VertStatus (seg->verts [i]) & MARKED_MASK))
		return false;
return true;
}

//========================================================================== 
// MENU - Mark
//========================================================================== 
void CMine::Mark()
{
	bool	bCubeMark = false; 
	CDSegment *seg = CurrSeg (); 
	int i, p [8], n_points; 

switch (theApp.MineView ()->GetSelectMode ()) {
	case eSelectPoint:
		n_points = 1; 
		p [0] = seg->verts [side_vert [Current ()->side][Current ()->point]]; 
		break; 
	case eSelectLine:
		n_points = 2; 
		p [0] = seg->verts [side_vert [Current ()->side][Current ()->point]]; 
		p [1] = seg->verts [side_vert [Current ()->side][(Current ()->point + 1)&3]]; 
		break; 
	case eSelectSide:
		n_points = 4; 
		for (i = 0; i < n_points; i++)
			p [i] = seg->verts [side_vert [Current ()->side][i]]; 
		break; 
	default:
		bCubeMark = true; 
	}

if (bCubeMark)
		MarkSegment (Current ()->segment); 
else {
	// set i to n_points if all verts are marked
	for (i = 0; i < n_points; i++)
		if (!(*VertStatus (p [i]) & MARKED_MASK)) break; 
		// if all verts are marked, then unmark them
	if (i== n_points)
		for (i = 0; i < n_points; i++)
			*VertStatus (p [i]) &= ~MARKED_MASK; 
	else
		// otherwise mark all the points
		for (i = 0; i < n_points; i++)
			*VertStatus (p [i]) |= MARKED_MASK; 
		UpdateMarkedCubes(); 
	}
theApp.MineView ()->Refresh (); 
}

// -------------------------------------------------------------------------- 
//			 mark_segment()
//
//  ACTION - Toggle marked bit of segment and mark/unmark vertices.
//
// -------------------------------------------------------------------------- 
void CMine::MarkSegment(INT16 segnum)
{
  CDSegment *seg = Segments () + segnum; 

	seg->wall_bitmask ^= MARKED_MASK; /* flip marked bit */

	// update vertices's marked status
	// ..first clear all marked verts
	INT16 vertnum; 
	for (vertnum = 0; vertnum < MAX_VERTICES; vertnum++)
		*VertStatus (vertnum) &= ~MARKED_MASK; 
	// ..then mark all verts for marked Segments ()
	for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++)
		if (seg->wall_bitmask & MARKED_MASK)
			for (vertnum = 0; vertnum < 8; vertnum++)
				*VertStatus (seg->verts [vertnum]) |=  MARKED_MASK; 
}

// -------------------------------------------------------------------------- 
// update_marked_cubes()
// -------------------------------------------------------------------------- 
void CMine::UpdateMarkedCubes()
{
	CDSegment *seg; 
	int i; 
	// mark all cubes which have all 8 verts marked
	for (i = 0, seg = Segments (); i < SegCount (); i++, seg++)
		if ((*VertStatus (seg->verts [0]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [1]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [2]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [3]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [4]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [5]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [6]) & MARKED_MASK) &&
			 (*VertStatus (seg->verts [7]) & MARKED_MASK))
			seg->wall_bitmask |= MARKED_MASK; 
		else
			seg->wall_bitmask &= ~MARKED_MASK; 
}

//========================================================================== 
// MENU - Mark all cubes
//========================================================================== 
void CMine::MarkAll() {
	int i; 
	for (i = 0; i < SegCount (); i++) {
		Segments (i)->wall_bitmask |= MARKED_MASK; 
	}
	for (i = 0; i < VertCount (); i++) {
		*VertStatus (i) |= MARKED_MASK; 
	}
	theApp.MineView ()->Refresh (); 
}

//========================================================================== 
// MENU - Unmark all cubes
//========================================================================== 
void CMine::UnmarkAll() {
	int i; 
	CDSegment *seg = Segments ();
	for (i = 0; i < MAX_SEGMENTS; i++, seg++)
		seg->wall_bitmask &= ~MARKED_MASK; 
	UINT8 *stat = VertStatus ();
	for (i = 0; i < MAX_VERTICES; i++, stat++)
		*stat &= ~MARKED_MASK; 
	theApp.MineView ()->Refresh (); 
}

// -------------------------------------------------------------------------- 
// ResetSide()
//
// Action - sets side to have no child and a default texture
// -------------------------------------------------------------------------- 

void CMine::ResetSide (INT16 segnum, INT16 sidenum)
{
if (segnum < 0 || segnum >= SegCount ()) 
	return; 
theApp.SetModified (TRUE); 
theApp.LockUndo ();
CDSegment *seg = Segments () + segnum; 
seg->children [sidenum] =-1; 
seg->child_bitmask &= ~(1 << sidenum); 
CDSide *side = seg->sides + sidenum;
side->nBaseTex = 0; 
side->nOvlTex = 0; 
uvl *uvls = side->uvls;
double scale = pTextures [file_type][side->nBaseTex].Scale (side->nBaseTex);
for (int i = 0; i < 4; i++, uvls++) {
	uvls->u = (INT16) (default_uvls [i].u / scale); 
	uvls->v = (INT16) (default_uvls [i].v / scale); 
	uvls->l = (UINT16) DEFAULT_LIGHTING; 
	}
theApp.UnlockUndo ();
}

// -------------------------------------------------------------------------- 
// unlink_child()
//
// Action - unlinks current cube's children which don't share all four points
//
// Note: 2nd parameter "sidenum" is ignored
// -------------------------------------------------------------------------- 

void CMine::UnlinkChild (INT16 parent_segnum, INT16 sidenum) 
{
  CDSegment *parent_seg = Segments (parent_segnum); 

// loop on each side of the parent
//  for (int sidenum = 0; sidenum < 6; sidenum++) {
int child_segnum = parent_seg->children [sidenum]; 
// does this side have a child?
if (child_segnum < 0 || child_segnum >= SegCount ())
	return;
CDSegment *child_seg = Segments () + child_segnum; 
// yes, see if child has a side which points to the parent
for (int child_sidenum = 0; child_sidenum < 6; child_sidenum++)
	if (child_seg->children [child_sidenum]== parent_segnum) break; 
// if we found the matching side
if (child_sidenum < 6) {
// define vert numbers for comparison
	INT16 pv [4], cv [4]; // (short names given for clarity)
	for (int i = 0; i < 4; i++) {
		pv [i] = parent_seg->verts [side_vert [sidenum][i]]; // parent vert
		cv [i] = child_seg->verts [side_vert [child_sidenum][i]]; // child vert
		}
	// if they share all four vertices..
	// note: assumes verts increase clockwise looking outward
	if ((pv [0]== cv [3] && pv [1]== cv [2] && pv [2]== cv [1] && pv [3]== cv [0]) ||
		 (pv [0]== cv [2] && pv [1]== cv [1] && pv [2]== cv [0] && pv [3]== cv [3]) ||
		 (pv [0]== cv [1] && pv [1]== cv [0] && pv [2]== cv [3] && pv [3]== cv [2]) ||
		 (pv [0]== cv [0] && pv [1]== cv [3] && pv [2]== cv [2] && pv [3]== cv [1]))
		; // they match, don't mess with them
	else {
		// otherwise, they don't share all four points correctly
		// so unlink the child from the parent
		// and unlink the parent from the child
		theApp.SetModified (TRUE); 
		theApp.LockUndo ();
		ResetSide (child_segnum, child_sidenum); 
		ResetSide (parent_segnum, sidenum); 
		theApp.UnlockUndo ();
		}
	}
else {
	// if the child does not point to the parent, 
	// then just unlink the parent from the child
	ResetSide (parent_segnum, sidenum); 
	}
}

// -------------------------------------------------------------------------- 

bool CMine::IsPointOfSide (CDSegment *seg, int sidenum, int pointnum)
{
	int	i;

for (i = 0; i < 4; i++)
	if (side_vert [sidenum][i] == pointnum)
		return true;
return false;
}

// -------------------------------------------------------------------------- 

bool CMine::IsLineOfSide (CDSegment *seg, int sidenum, int linenum)
{
	int	i;

for (i = 0; i < 2; i++)
	if (!IsPointOfSide (seg, sidenum, line_vert [linenum][i]))
		return false;
return true;
}

// -------------------------------------------------------------------------- 
//                          Splitpoints()
//
// Action - Splits one point shared between two cubes into two points.
//          New point is added to current cube, other cube is left alone.
//
// -------------------------------------------------------------------------- 

void CMine::SplitPoints () 
{
CDSegment *seg; 
INT16 vert, segnum, vertnum, opp_segnum, opp_sidenum; 
bool found; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}
if (VertCount () > (MAX_VERTICES - 1)) {
	ErrorMsg("Cannot unjoin these points because the\n"
				"maximum number of points is reached."); 
	return; 
	}

seg = Segments (Current ()->segment); 
vert = seg->verts [side_vert [Current ()->side][Current ()->point]]; 

// check to see if current point is shared by any other cubes
found = FALSE; 
seg = Segments ();
for (segnum = 0; (segnum < SegCount ()) && !found; segnum++, seg++)
	if (segnum != Current ()->segment)
		for (vertnum = 0; vertnum < 8; vertnum++)
			if (seg->verts [vertnum] == vert) {
				found = TRUE; 
				break; 
				}
if (!found) {
	ErrorMsg("This point is not joined with any other point."); 
	return; 
	}

if (QueryMsg("Are you sure you want to unjoin this point?") != IDYES) 
	return; 

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// create a new point (copy of other vertex)
memcpy (Vertices (VertCount ()), Vertices (vert), sizeof (*Vertices ()));
/*
Vertices (VertCount ()).x = Vertices (vert).x; 
Vertices (VertCount ()).y = Vertices (vert).y; 
Vertices (VertCount ()).z = Vertices (vert).z; 
*/
// replace existing point with new point
seg = Segments (Current ()->segment); 
seg->verts [side_vert [Current ()->side][Current ()->point]] = VertCount (); 
seg->wall_bitmask &= ~MARKED_MASK; 

// update total number of vertices
*VertStatus (VertCount ()++) = 0; 

for (int sidenum = 0; sidenum < 6; sidenum++)
	if (IsPointOfSide (seg, sidenum, seg->verts [side_vert [Current ()->side][Current ()->point]]) &&
		 GetOppositeSide (opp_segnum, opp_sidenum, Current ()->segment, sidenum)) {
		UnlinkChild (seg->children [sidenum], opp_side [sidenum]);
		UnlinkChild (Current ()->segment, sidenum); 
		}

	UnlinkChild(Current ()->segment, sidenum); 

//  *VertStatus (VertCount ()-1] &= ~DELETED_MASK; 
SetLinesToDraw(); 
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
INFOMSG("A new point was made for the current point."); 
}

// -------------------------------------------------------------------------- 
//                         Splitlines()
//
// Action - Splits common lines of two cubes into two lines.
//
// -------------------------------------------------------------------------- 

void CMine::SplitLines() 
{
  CDSegment *seg; 
  INT16 vert [2], segnum, vertnum, linenum, opp_segnum, opp_sidenum, i; 
  bool found [2]; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}
if (VertCount () > (MAX_VERTICES - 2)) {
	if (!bExpertMode)
		ErrorMsg("Cannot unjoin these lines because\nthere are not enought points left."); 
	return; 
	}

seg = Segments (Current ()->segment); 
for (i = 0; i < 2; i++) {
	linenum = side_line [Current ()->side][Current ()->line]; 
	vert [i] = Segments (Current ()->segment)->verts [line_vert [linenum][i]]; 
	// check to see if current points are shared by any other cubes
	found [i] = FALSE; 
	seg = Segments ();
	for (segnum = 0; (segnum < SegCount ()) && !found [i]; segnum++, seg++) {
		if (segnum != Current ()->segment) {
			for (vertnum = 0; vertnum < 8; vertnum++) {
				if (seg->verts [vertnum] == vert [i]) {
					found [i] = TRUE; 
					break; 
					}
				}
			}
		}
	}
if (!(found [0] && found [1])) {
	if (!bExpertMode)
		ErrorMsg("One or both of these points are not joined with any other points."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this line?") != IDYES)
	return; 
theApp.SetModified (TRUE); 
theApp.LockUndo ();
seg = Segments (Current ()->segment); 
// create a new points (copy of other vertices)
for (i = 0; i < 2; i++)
	if (found [i]) {
		memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices ()));
		/*
		vertices [VertCount ()].x = vertices [vert [i]].x; 
		vertices [VertCount ()].y = vertices [vert [i]].y; 
		vertices [VertCount ()].z = vertices [vert [i]].z; 
		*/
		// replace existing points with new points
		linenum = side_line [Current ()->side][Current ()->line]; 
		seg->verts [line_vert [linenum][i]] = VertCount (); 
		seg->wall_bitmask &= ~MARKED_MASK; 
		// update total number of vertices
		*VertStatus (VertCount ()++) = 0; 
		}
for (int sidenum = 0; sidenum < 6; sidenum++) {
	if (IsLineOfSide (seg, sidenum, linenum) && 
		 GetOppositeSide (opp_segnum, opp_sidenum, Current ()->segment, sidenum)) {
		UnlinkChild (opp_segnum, opp_sidenum);
		UnlinkChild (Current ()->segment, sidenum); 
		}
	}
//  *VertStatus (VertCount ()-1] &= ~DELETED_MASK; 
SetLinesToDraw(); 
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
INFOMSG ("Two new points were made for the current line."); 
}

// -------------------------------------------------------------------------- 
//                       Splitsegments()
//
// ACTION - Splits a cube from all other points which share its coordinates
//
//  Changes - Added option to make thin side
// If solidify == 1, the side will keep any points it has in common with other
// sides, unless one or more of its vertices are already solitaire, in which
// case the side needs to get disconnected from its child anyway because that 
// constitutes an error in the level structure.
// -------------------------------------------------------------------------- 

void CMine::SplitSegments (int solidify, int sidenum) 
{
  CDSegment *seg; 
  int vert [4], segnum, vertnum, i, nFound = 0; 
  bool found [4]; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}

seg = CurrSeg (); 
if (sidenum < 0)
	sidenum = Current ()->side;
int child_segnum = seg->children [sidenum]; 
if (child_segnum == -1) {
	ErrorMsg ("The current side is not connected to another cube"); 
	return; 
	}

for (i = 0; i < 4; i++)
	vert [i] = seg->verts [side_vert [sidenum][i]]; 
	// check to see if current points are shared by any other cubes
for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++)
	if (segnum != Current ()->segment)
		for (i = 0, nFound = 0; i < 4; i++) {
			found [i] = FALSE;
			for (vertnum = 0; vertnum < 8; vertnum++)
				if (seg->verts [vertnum] == vert [i]) {
					found [i] = TRUE;
					if (++nFound == 4)
						goto found;
					}
			}
// If a side has one or more solitary points but has a child, there is 
// something wrong. However, nothing speaks against completely unjoining it.
// In fact, this does even cure the problem. So, no error message.
//	ErrorMsg ("One or more of these points are not joined with any other points."); 
//	return; 

found:

if (!solidify && (VertCount () > (MAX_VERTICES - nFound))) {
	ErrorMsg("Cannot unjoin this side because\nthere are not enough vertices left."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this side?") != IDYES)
	return; 

theApp.SetModified (TRUE); 
theApp.LockUndo ();
seg = Segments (Current ()->segment); 
if (nFound < 4)
	solidify = 0;
if (!solidify) {
	// create new points (copy of other vertices)
	for (i = 0; i < 4; i++) {
		if (found [i]) {
			memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices ()));
			/*
			vertices [VertCount ()].x = vertices [vert [i]].x; 
			vertices [VertCount ()].y = vertices [vert [i]].y; 
			vertices [VertCount ()].z = vertices [vert [i]].z; 
			*/
			// replace existing points with new points
			seg->verts [side_vert [sidenum][i]] = VertCount (); 
			seg->wall_bitmask &= ~MARKED_MASK; 

			// update total number of vertices
			*VertStatus (VertCount ()++) = 0; 
			}
		}
	for (int sidenum = 0; sidenum < 6; sidenum++)
		if (sidenum != opp_side [sidenum])
			UnlinkChild (Current ()->segment, sidenum); 
	SetLinesToDraw(); 
	INFOMSG (" Four new points were made for the current side."); 
	}
else {
	// does this side have a child?
	CDSegment *child_seg = Segments (child_segnum); 
	// yes, see if child has a side which points to the parent
	for (int child_sidenum = 0; child_sidenum < 6; child_sidenum++)
		if (child_seg->children [child_sidenum]== Current ()->segment) 
			break; 
	// if we found the matching side
	if (child_sidenum < 6)
		ResetSide (child_segnum, child_sidenum); 
	ResetSide (Current ()->segment, Current ()->side); 
	SetLinesToDraw(); 
	}
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
}

// -------------------------------------------------------------------------- 
// Mine - Joinpoints
// -------------------------------------------------------------------------- 

void CMine::JoinPoints() 
{
  CDSegment *seg1, *seg2; 
 double distance; //v1x, v1y, v1z, v2x, v2y, v2z; 
  int vert1, vert2; 
  CDSelection *cur1, *cur2; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}
if (Current1 ().segment== Current2 ().segment) {
	ErrorMsg("You cannot joint two points on the same cube.\n\n"
				"Hint: The two golden circles represent the current point, \n"
				"and the 'other' cube's current point.  Press 'P' to change the\n"
				"Current () point or press the space bar to switch to the other cube."); 
	return;
	}

if (Current () == &Current1 ()) {
	seg1 = Segments () + Current1 ().segment; 
	seg2 = Segments () + Current2 ().segment; 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	}
else {
	seg1 = Segments () + Current2 ().segment; 
	seg2 = Segments () + Current1 ().segment; 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}
vert1 = seg1->verts [side_vert [cur1->side][cur1->point]]; 
vert2 = seg2->verts [side_vert [cur2->side][cur2->point]]; 
// make sure verts are different
if (vert1== vert2) {
	ErrorMsg("These points are already joined."); 
	return; 
	}
// make sure there are distances are close enough
distance = CalcLength(Vertices (vert1), Vertices (vert2)); 
if (distance > JOIN_DISTANCE) {
	ErrorMsg("Points are too far apart to join"); 
	return; 
	}
if (QueryMsg("Are you sure you want to join the current point\n"
				 "with the 'other' cube's current point?") != IDYES)
	return; 
theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define vert numbers
seg1->verts [side_vert [cur1->side][cur1->point]] = vert2; 
// delete any unused vertices
//  delete_unused_vertices(); 
FixChildren(); 
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
}

// -------------------------------------------------------------------------- 
// Mine - Joinlines
// -------------------------------------------------------------------------- 

void CMine::JoinLines() 
{
  CDSegment *seg1, *seg2; 
  double v1x [2], v1y [2], v1z [2], v2x [2], v2y [2], v2z [2]; 
  double distance, min_radius; 
  int v1, v2, vert1 [2], vert2 [2]; 
  INT16 match [2]; 
  INT16 i, j, linenum; 
  bool fail; 
  CDSelection *cur1, *cur2; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}

if (Current1 ().segment == Current2 ().segment) {
	ErrorMsg("You cannot joint two lines on the same cube.\n\n"
				"Hint: The two green lines represent the current line, \n"
				"and the 'other' cube's current line.  Press 'L' to change\n"
				"the current line or press the space bar to switch to the other cube."); 
	return;
	}

if (Current ()== &Current1 ()) {
	seg1 = Segments () + Current1 ().segment; 
	seg2 = Segments () + Current2 ().segment; 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	} 
else {
	seg1 = Segments () + Current2 ().segment; 
	seg2 = Segments () + Current1 ().segment; 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}

for (i = 0; i < 2; i++) {
	linenum = side_line [cur1->side][cur1->line]; 
	v1 = vert1 [i] = seg1->verts [line_vert [linenum][i]]; 
	linenum = side_line [cur2->side][cur2->line]; 
	v2 = vert2 [i] = seg2->verts [line_vert [linenum][i]]; 
	v1x [i] = Vertices (v1)->x; 
	v1y [i] = Vertices (v1)->y; 
	v1z [i] = Vertices (v1)->z; 
	v2x [i] = Vertices (v2)->x; 
	v2y [i] = Vertices (v2)->y; 
	v2z [i] = Vertices (v2)->z; 
	match [i] =-1; 
	}

// make sure verts are different
if (vert1 [0]== vert2 [0] || vert1 [0]== vert2 [1] ||
	 vert1 [1]== vert2 [0] || vert1 [1]== vert2 [1]) {
	ErrorMsg("Some or all of these points are already joined."); 
	return; 
	}

// find closest for each point for each corner
for (i = 0; i < 2; i++) {
	min_radius = JOIN_DISTANCE; 
	for (j = 0; j < 2; j++) {
		distance = sqrt((v1x [i] - v2x [j]) * (v1x [i] - v2x [j])
					+ (v1y [i] - v2y [j]) * (v1y [i] - v2y [j])
					+ (v1z [i] - v2z [j]) * (v1z [i] - v2z [j])); 
		if (distance < min_radius) {
			min_radius = distance; 
			match [i] = j;  // remember which vertex it matched
			}
		}
	}

// make sure there are distances are close enough
if (min_radius== JOIN_DISTANCE) {
	ErrorMsg("Lines are too far apart to join"); 
	return; 
	}

if (QueryMsg("Are you sure you want to join the current line\n"
				 "with the 'other' cube's current line?") != IDYES)
	return; 
fail = FALSE; 
// make sure there are matches for each and they are unique
fail = (match [0] == match [1]);
if (fail) {
	match [0] = 1; 
	match [1] = 0; 
	}
theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define vert numbers
for (i = 0; i < 2; i++) {
	linenum = side_line [cur1->side][cur1->line]; 
	seg1->verts [line_vert [linenum][i]] = vert2 [match [i]]; 
	}
FixChildren(); 
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
}


// -------------------------------------------------------------------------- 
//			  set_lines_to_draw()
//
//  ACTION - Determines which lines will be shown when drawing 3d image of
//           the mine->  This helps speed up drawing by avoiding drawing lines
//           multiple times.
//
// -------------------------------------------------------------------------- 

void CMine::SetLinesToDraw()
{
  CDSegment *seg; 
  INT16 segnum, side; 

for (segnum = SegCount (), seg = Segments (); segnum; segnum--, seg++) {
	seg->map_bitmask |= 0xFFF; 
	// if segment side has a child, clear bit for drawing line
	for (side = 0; side < MAX_SIDES_PER_SEGMENT; side++) {
		if (seg->children [side] > -1) { // -1 = no child,  - 2 = outside of world
			seg->map_bitmask &= ~(1 << (side_line [side][0])); 
			seg->map_bitmask &= ~(1 << (side_line [side][1])); 
			seg->map_bitmask &= ~(1 << (side_line [side][2])); 
			seg->map_bitmask &= ~(1 << (side_line [side][3])); 
			}
		}
	}
}

// -------------------------------------------------------------------------- 
// FixChildren()
//
// Action - Updates linkage between current segment and all other Segments ()
// -------------------------------------------------------------------------- 

void CMine::FixChildren()
{
INT16 nNewSide, sidenum, segnum, nNewSeg; 

nNewSeg = Current ()->segment; 
nNewSide = Current ()->side; 
CDSegment *pSeg = Segments (),
			 *pNewSeg = Segments (nNewSeg);
vms_vector *vSeg, 
			  *vNewSeg = Vertices (pNewSeg->verts [0]);
for (segnum = 0; segnum < SegCount (); segnum++, pSeg) {
	if (segnum != nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (pSeg->verts [0]);
		if (fabs ((double) (vNewSeg->x - vSeg->x)) < 0xA00000L &&
		    fabs ((double) (vNewSeg->y - vSeg->y)) < 0xA00000L &&
		    fabs ((double) (vNewSeg->z - vSeg->z)) < 0xA00000L) {
			for (sidenum = 0; sidenum < 6; sidenum++) {
				if (!LinkSegments(nNewSeg, nNewSide, segnum, sidenum, 3*F1_0)) {
					// if these Segments () were linked, then unlink them
					if (pNewSeg->children [nNewSide]== segnum && pSeg->children [sidenum]== nNewSeg) {
						pNewSeg->children [nNewSide] =-1; 
						pNewSeg->child_bitmask &= ~(1 << nNewSide); 
						pSeg->children [sidenum] =-1; 
						pSeg->child_bitmask &= ~(1 << sidenum); 
						}
					}
				}
			}
		}
	}
}

// -------------------------------------------------------------------------- 
//		       Joinsegments()
//
//  ACTION - Joins sides of current Segments ().  Finds closest corners.
//	     If sides use vertices with the same coordinates, these vertices
//	     are merged and the cube's are connected together.  Otherwise, a
//           new cube is added added.
//
//  Changes - Added option to solidifyally figure out "other cube"
// -------------------------------------------------------------------------- 

void CMine::JoinSegments(int solidify)
{
	CDSegment *seg; 
	CDSegment *seg1, *seg2; 
	INT16 h, i, j, sidenum, nNewSeg, segnum; 
	vms_vector v1 [4], v2 [4]; 
	double radius, min_radius, max_radius, dx, dy, dz, totalRad, minTotalRad; 
	tVertMatch match [4]; 
	bool fail; 
	CDSelection *cur1, *cur2, my_cube; 

if (m_bSplineActive) {
	ErrorMsg(spline_error_message); 
	return; 
	}

// figure out "other' cube
if (solidify) {
	if (Segments (Current ()->segment)->children [Current ()->side] != -1) {
		if (!bExpertMode)
			ErrorMsg("The current side is already joined to another cube"); 
		return; 
		}
	cur1 = Current (); 
	cur2 = &my_cube; 
	my_cube.segment = -1;
	// find first cube (other than this cube) which shares all 4 points
	// of the current side (points must be < 5.0 away)
	seg1 = Segments (cur1->segment); 
	for (i = 0; i < 4; i++) {
#if 1
		memcpy (v1 + i, Vertices (seg1->verts [side_vert [cur1->side][i]]), sizeof (*Vertices ()));
#else
		int vertnum = seg1->verts [side_vert [cur1->side][i]];
		v1 [i].x = Vertices (vertnum)->x; 
		v1 [i].y = Vertices (vertnum)->y; 
		v1 [i].z = Vertices (vertnum)->z; 
#endif
		}
	minTotalRad = 1e300;
	for (segnum = 0, seg2 = Segments (); segnum < SegCount (); segnum++, seg2++) {
		if (segnum== cur1->segment)
			continue; 
		for (sidenum = 0; sidenum < 6; sidenum++) {
			fail = FALSE; 
			for (i = 0; i < 4; i++) {
#if 1
				memcpy (v2 + i, Vertices (seg2->verts[side_vert[sidenum][i]]), sizeof (*Vertices ()));
#else
				int vertnum = seg2->verts [side_vert [sidenum][i]];
				v2 [i].x = Vertices (vertnum)->x; 
				v2 [i].y = Vertices (vertnum)->y; 
				v2 [i].z = Vertices (vertnum)->z; 
#endif
				}
			for (i = 0; i < 4; i++)
				match [i].b = 0; 
			for (i = 0; i < 4; i++) {
				match [i].i = -1; 
				match [i].d = 1e300;
				for (j = 0, h = -1; j < 4; j++) {
					if (match [j].b)
						continue;
					dx = (double) v1 [i].x - (double) v2 [j].x;
					dy = (double) v1 [i].y - (double) v2 [j].y;
					dz = (double) v1 [i].z - (double) v2 [j].z;
					radius = sqrt (dx * dx + dy * dy + dz * dz);
					if ((radius <= 10.0 * F1_0) && (radius < match [i].d)) {
						h = j;  // remember which vertex it matched
						match [i].d = radius;
						}
					}
				if (h < 0) {
					fail = TRUE;
					break;
					}
				match [i].i = h;
				match [h].b = i;
				}

#if 0
			if (fail)
				continue;
			for (i = 0; i < 4; i++)
				if (match [i].i == -1) {
					fail = TRUE; 
					break; 
					}
		// make sure there are matches for each and they are unique
		// Actually, if there's a match for each vertex, it must be unique here,
		// because if it wasn't, one entry in match[] must have been left out and thus be -1 [DM]
#	if 1
			for (i = 0; i < 3; i++)
				for (j = i + 1; j < 4; j++)
					if (match [i] == match [j]) {
						fail = TRUE;
						break;
						}
#	else
			if (match [0] == match [1]) fail = TRUE; 
			else if (match [0] == match [2]) fail = TRUE; 
			else if (match [0] == match [3]) fail = TRUE; 
			else if (match [1] == match [2]) fail = TRUE; 
			else if (match [1] == match [3]) fail = TRUE; 
			else if (match [2] == match [3]) fail = TRUE; 
#	endif
#endif
			if (fail)
				continue;
			totalRad = 0;
			for (i = 0; i < 4; i++)
				totalRad += match [i].d;
			if (minTotalRad > totalRad) {
				minTotalRad = totalRad;
				my_cube.segment = segnum; 
				my_cube.side = sidenum; 
				my_cube.point = 0; // should not be used
			// force break from loops
				if (minTotalRad == 0) {
					sidenum = 6; 
					segnum = SegCount (); 
					}
				}
			}
		}
	if (my_cube.segment < 0) {
		if (!bExpertMode)
			ErrorMsg("Could not find another cube whose side is within\n"
						"10.0 units from the current side"); 
		return; 
		}
	}
else
	if (Current ()== &Current1 ()) {
		cur1 = &Current1 (); 
		cur2 = &Current2 (); 
		}
	else {
		cur1 = &Current2 (); 
		cur2 = &Current1 (); 
		}

if (cur1->segment == cur2->segment) {
	if (!bExpertMode)
		ErrorMsg("You cannot joint two sides on the same cube.\n\n"
					"Hint: The two red squares represent the current side, \n"
					"and the 'other' cube's current side.  Press 'S' to change\n"
					"the current side or press the space bar to switch to the other cube."); 
	return; 
	}

seg1 = Segments (cur1->segment); 
seg2 = Segments (cur2->segment); 

// figure out matching corners to join to.
// get coordinates for calulaction and set match = none
for (i = 0; i < 4; i++) {
	memcpy (v1 + i, Vertices (seg1->verts [side_vert [cur1->side][i]]), sizeof (*Vertices ())); 
	memcpy (v2 + i, Vertices (seg2->verts [side_vert [cur2->side][i]]), sizeof (*Vertices ())); 
/*
	v1 [i].x = vertices [seg1->verts [side_vert [cur1->side][i]]].x; 
	v1 [i].y = vertices [seg1->verts [side_vert [cur1->side][i]]].y; 
	v1 [i].z = vertices [seg1->verts [side_vert [cur1->side][i]]].z; 
	v2 [i].x = vertices [seg2->verts [side_vert [cur2->side][i]]].x; 
	v2 [i].y = vertices [seg2->verts [side_vert [cur2->side][i]]].y; 
	v2 [i].z = vertices [seg2->verts [side_vert [cur2->side][i]]].z; 
*/
	match [i].i = -1; 
	}

// find closest for each point for each corner
for (i = 0; i < 4; i++) {
	min_radius = JOIN_DISTANCE; 
	for (j = 0; j < 4; j++) {
		dx = (double) v1 [i].x - (double) v2 [j].x;
		dy = (double) v1 [i].y - (double) v2 [j].y;
		dz = (double) v1 [i].z - (double) v2 [j].z;
		radius = sqrt(dx * dx + dy * dy + dz * dz);
		if (radius < min_radius) {
			min_radius = radius; 
			match [i].i = j;  // remember which vertex it matched
			}
		}
	}

fail = FALSE; 
for (i = 0; i < 4; i++)
	if (match [i].i == -1) {
		fail = TRUE; 
		break; 
	}

// make sure there are matches for each and they are unique
if (match [0].i == match [1].i) fail = TRUE; 
else if (match [0].i == match [2].i) fail = TRUE; 
else if (match [0].i == match [3].i) fail = TRUE; 
else if (match [1].i == match [2].i) fail = TRUE; 
else if (match [1].i == match [3].i) fail = TRUE; 
else if (match [2].i == match [3].i) fail = TRUE; 

if (fail) {
	//    ErrorMsg("Can't figure out how to attach these sides\n"
	//	     "because the closest point to each point\n"
	//	     "on the current side is not a unique point\n"
	//	     "on the other side."); 
	//    return; 
	// go method #2, use current points
	int offset; 
	offset = (4 + cur1->point - (3 - cur2->point))%4; 
	match [0].i = (offset + 3)%4; 
	match [1].i = (offset + 2)%4; 
	match [2].i = (offset + 1)%4; 
	match [3].i = (offset + 0)%4; 
	}

// determine min and max distances
min_radius = JOIN_DISTANCE; 
max_radius = 0; 
for (i = 0; i < 4; i++) {
	j = match [i].i; 
	radius = sqrt(((double)v1 [i].x - (double)v2 [j].x) * ((double)v1 [i].x - (double)v2 [j].x)
				 +  ((double)v1 [i].y - (double)v2 [j].y) * ((double)v1 [i].y - (double)v2 [j].y)
				 +  ((double)v1 [i].z - (double)v2 [j].z) * ((double)v1 [i].z - (double)v2 [j].z)); 
	min_radius = min(min_radius, radius); 
	max_radius = max(max_radius, radius); 
	}

// make sure there are distances are close enough
if (max_radius >= JOIN_DISTANCE) {
	if (!bExpertMode)
		ErrorMsg("Sides are too far apart to join.\n\n"
					"Hint: Cubes should not exceed 200 in any dimension\n"
					"or they will distort when viewed from close up."); 
	return; 
	}

// if Segments () are too close to put a new segment between them, 
// then solidifyally link them together without asking
if (min_radius <= 5*F1_0) {
	theApp.SetModified (TRUE); 
	theApp.LockUndo ();
	LinkSides (cur1->segment, cur1->side, cur2->segment, cur2->side, match); 
	SetLinesToDraw(); 
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	return; 
	}

if (QueryMsg("Are you sure you want to create a new cube which\n"
				 "connects the current side with the 'other' side?\n\n"
				 "Hint: Make sure you have the current point of each cube\n"
				 "on the corners you to connected.\n"
				 "(the 'P' key selects the current point)") != IDYES)
	return; 

//  nNewSeg = first_free_segment(); 
//  if (nNewSeg== -1) {
nNewSeg = SegCount (); 
if (!(SegCount () < MAX_SEGMENTS)) {
	if (!bExpertMode)
		ErrorMsg("The maximum number of Segments () has been reached.\n"
					"Cannot add any more Segments ()."); 
	return; 
	}
seg = Segments (nNewSeg); 

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define children and special child
// first clear all sides
seg->child_bitmask = 0; 
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  /* no remaining children */
	seg->children [i] =-1; 

// now define two sides:
// near side has opposite side number cube 1
seg->child_bitmask |= (1 << (opp_side [cur1->side])); 
seg->children [opp_side [cur1->side]] = cur1->segment; 
// far side has same side number as cube 1
seg->child_bitmask |= (1 << cur1->side); 
seg->children [cur1->side] = cur2->segment; 
seg->owner = -1;
seg->group = -1;
seg->special = 0; 
seg->matcen_num =-1; 
seg->value =-1; 

// define vert numbers
for (i = 0; i < 4; i++) {
	seg->verts [opp_side_vert [cur1->side][i]] = seg1->verts [side_vert [cur1->side][i]]; 
	seg->verts [side_vert [cur1->side][i]] = seg2->verts [side_vert [cur2->side][match [i].i]]; 
	}

// define Walls ()
seg->wall_bitmask = 0; // unmarked
for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++)
	seg->sides [sidenum].nWall = NO_WALL; 

// define sides
for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
	if (seg->children [sidenum]==-1) {
		SetTexture (nNewSeg, sidenum, seg1->sides [cur1->side].nBaseTex, seg1->sides [cur1->side].nOvlTex); 
//        for (i = 0; i < 4; i++) {
//	  seg->sides [sidenum].uvls [i].u = seg1->sides [cur1->side].uvls [i].u; 
//	  seg->sides [sidenum].uvls [i].v = seg1->sides [cur1->side].uvls [i].v; 
//	  seg->sides [sidenum].uvls [i].l = seg1->sides [cur1->side].uvls [i].l; 
//        }
		SetUV (nNewSeg, sidenum, 0, 0, 0); 
		}
	else {
		SetTexture (nNewSeg, sidenum, 0, 0); 
		for (i = 0; i < 4; i++) {
			seg->sides [sidenum].uvls [i].u = 0; 
			seg->sides [sidenum].uvls [i].v = 0; 
			seg->sides [sidenum].uvls [i].l = 0; 
			}
		}
	}

// define static light
seg->static_light = seg1->static_light; 

// update cur segment
seg1->children [cur1->side] = nNewSeg; 
seg1->child_bitmask |= (1 << cur1->side); 
SetTexture (cur1->segment, cur1->side, 0, 0); 
for (i = 0; i < 4; i++) {
	seg1->sides [cur1->side].uvls [i].u = 0; 
	seg1->sides [cur1->side].uvls [i].v = 0; 
	seg1->sides [cur1->side].uvls [i].l = 0; 
	}
seg2->children [cur2->side] = nNewSeg; 
seg2->child_bitmask |= (1 << cur2->side); 
SetTexture (cur2->segment, cur2->side, 0, 0); 
for (i = 0; i < 4; i++) {
	seg2->sides [cur2->side].uvls [i].u = 0; 
	seg2->sides [cur2->side].uvls [i].v = 0; 
	seg2->sides [cur2->side].uvls [i].l = 0; 
	}

// update number of Segments () and vertices
SegCount ()++; 
theApp.UnlockUndo ();
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
}

// ------------------------------------------------------------------------ 

void CMine::LoadSideTextures (INT16 segNum, INT16 sideNum)
{
GetCurrent (segNum, sideNum);
CDSide	*sideP = Segments (segNum)->sides + sideNum;
pTextures [file_type][sideP->nBaseTex].Read (sideP->nBaseTex);
if ((sideP->nOvlTex & 0x3fff) > 0)
	pTextures [file_type][sideP->nOvlTex & 0x3fff].Read (sideP->nOvlTex & 0x3fff);
}

// ------------------------------------------------------------------------ 
// Mine - SetUV ()
// ------------------------------------------------------------------------ 

void CMine::SetUV (INT16 segnum, INT16 sidenum, INT16 x, INT16 y, double dummy)
{
	struct vector {
		double x, y, z; 
		}; 

	struct vector A [4], B [4], C [4], D [4], E [4]; 
	int i, vertnum; 
	double angle; 

// for testing, x is used to tell how far to convert vector
// 0, 1, 2, 3 represent B, C, D, E coordinate transformations

// copy side's four points into A
int h = sizeof (*Vertices ());
for (i = 0; i < 4; i++) {
	vertnum = Segments (segnum)->verts [side_vert [sidenum][i]]; 
	A [i].x = Vertices (vertnum)->x; 
	A [i].y = Vertices (vertnum)->y; 
	A [i].z = Vertices (vertnum)->z; 
	}

// subtract point 0 from all points in A to form B points
for (i = 0; i < 4; i++) {
	B [i].x = A [i].x - A [0].x; 
	B [i].y = A [i].y - A [0].y; 
	B [i].z = A [i].z - A [0].z; 
	}

// calculate angle to put point 1 in x - y plane by spinning on x - axis
// then rotate B points on x - axis to form C points.
// check to see if on x - axis already
angle = atan3(B [1].z, B [1].y); 
for (i = 0; i < 4; i++) {
	C [i].x = B [i].x; 
	C [i].y = B [i].y * cos(angle) + B [i].z * sin(angle); 
	C [i].z = -B [i].y * sin(angle) + B [i].z * cos(angle); 
	}

#if UV_DEBUG
if (abs((int)C [1].z) != 0) {
	sprintf(message, "SetUV: point 1 not in x/y plane\n(%f); angle = %f", (float)C [1].z, (float)angle); 
	DEBUGMSG (message); 
	}
#endif

// calculate angle to put point 1 on x axis by spinning on z - axis
// then rotate C points on z - axis to form D points
// check to see if on z - axis already
angle = atan3(C [1].y, C [1].x); 
for (i = 0; i < 4; i++) {
	D [i].x = C [i].x * cos(angle) + C [i].y * sin(angle); 
	D [i].y = -C [i].x * sin(angle) + C [i].y * cos(angle); 
	D [i].z = C [i].z; 
	}
#if UV_DEBUG
if (abs((int)D [1].y) != 0) {
	DEBUGMSG (" SetUV: Point 1 not in x axis"); 
	}
#endif

// calculate angle to put point 2 in x - y plane by spinning on x - axis
// the rotate D points on x - axis to form E points
// check to see if on x - axis already
angle = atan3(D [2].z, D [2].y); 
for (i = 0; i < 4; i++) {
	E [i].x = D [i].x; 
	E [i].y = D [i].y * cos(angle) + D [i].z * sin(angle); 
	E [i].z = -D [i].y * sin(angle) + D [i].z * cos(angle); 
	}

// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
// set v to x axis and u to negative u axis to match default (u, v)
// (remember to scale by dividing by 640)
CDSide *sideP = Segments (segnum)->sides + sidenum;
uvl *uvls = sideP->uvls;
#if UV_DEBUG
switch (x) {
	case 0:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (B [i].x/640); 
			uvls [i].u = - (B [i].y/640); 
			}
		break; 
	case 1:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (C [i].x/640); 
			uvls [i].u = 0x400/10 - (C [i].y/640); 
			}
		break; 
	case 2:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (D [i].x/640); 
			uvls [i].u = 2*0x400/10 - (D [i].y/640); 
			}
		break; 
	case 3:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (E [i].x/640); 
			uvls [i].u = 3*0x400/10 - (E [i].y/640); 
			}
	break; 
	}
#else
theApp.SetModified (TRUE); 
LoadSideTextures (segnum, sidenum);
double scale = 1.0; //pTextures [file_type][sideP->nBaseTex].Scale (sideP->nBaseTex);
for (i = 0; i < 4; i++, uvls++) {
	uvls->v = (INT16) ((y + (E [i].x / 640)) / scale); 
	uvls->u = (INT16) ((x - (E [i].y / 640)) / scale); 
	}
#endif
}

                        /* -------------------------- */

bool CMine::GotMarkedSides ()
{
int	segnum, sidenum; 

for (segnum = 0; segnum < SegCount (); segnum++)
	for (sidenum = 0; sidenum < 6; sidenum++)
		if (SideIsMarked (segnum, sidenum))
			return true;
return false; 
}

                        /* -------------------------- */

INT16 CMine::MarkedSegmentCount (bool bCheck)
{
	int	segnum, nCount; 
	CDSegment *seg = Segments ();
for (segnum = SegCount (), nCount = 0; segnum; segnum--, seg++)
	if (seg->wall_bitmask & MARKED_MASK)
		if (bCheck)
			return 1; 
		else
			++nCount; 
return nCount; 
}

                        /* -------------------------- */

int CMine::IsWall (INT16 segnum, INT16 sidenum)
{
GetCurrent (segnum, sidenum); 
return (Segments (segnum)->children [sidenum]== -1) ||
		 (Segments (segnum)->sides [sidenum].nWall < GameInfo ().walls.count); 
}

                        /* -------------------------- */

int CMine::ScrollSpeed (UINT16 texture, int *x, int *y)
{
if (file_type == RDL_FILE)
	return 0;
*x = 0; 
*y = 0; 
switch (texture) {
	case 399: *x = - 2; break; 
	case 400: *y = - 8; break; 
	case 402: *x = - 4; break; 
	case 405: *y = - 2; break; 
	case 406: *y = - 4; break; 
	case 407: *y = - 2; break; 
	case 348: *x = - 2; *y = - 2; break; 
	case 349: *x = - 2; *y = - 2; break; 
	case 350: *x = + 2; *y = - 2; break; 
	case 401: *y = - 8; break; 
	case 408: *y = - 2; break; 
	default:
		return 0; 
	}
return 1; 
}

                        /* -------------------------- */

int CMine::AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides)
{
	CDSegment *seg = Segments (start_segment); 
	CDSegment *childSeg; 
	CDSide *side = seg->sides + start_side; 
	CDSide *childSide; 

	int return_code =-1; 
	INT16 i; 
	INT16 sidenum, childnum, linenum; 
	INT16 point0, point1, vert0, vert1; 
	INT16 childs_side, childs_line; 
	INT16 childs_point0, childs_point1, childs_vert0, childs_vert1; 
	INT16 u0, v0; 
	double sangle, cangle; 
	double angle, length; 
	static int side_child [6][4] = {
		{4, 3, 5, 1}, //{5, 1, 4, 3}, 
		{2, 4, 0, 5}, //{5, 0, 4, 2}, 
		{5, 3, 4, 1}, //{5, 3, 4, 1}, 
		{0, 4, 2, 5}, //{5, 0, 4, 2}, 
		{2, 3, 0, 1}, //{2, 3, 0, 1}, 
		{0, 3, 2, 1} //{2, 3, 0, 1}
		}; 

theApp.SetModified (TRUE);
theApp.LockUndo ();
for (linenum = 0; linenum < 4; linenum++) {
	// find vert numbers for the line's two end points
	point0 = line_vert [side_line [start_side][linenum]][0]; 
	point1 = line_vert [side_line [start_side][linenum]][1]; 
	vert0  = seg->verts [point0]; 
	vert1  = seg->verts [point1]; 
	// check child for this line
	if (start_segment == only_child) {
		sidenum = start_side;
		childnum = start_segment;
		}
	else {
		sidenum = side_child [start_side][linenum]; 
		childnum = seg->children [sidenum]; 
		}
	childSeg = Segments (childnum); 
	if ((childnum < 0) || ((only_child != -1) && (childnum != only_child)))
		continue;
	// figure out which side of child shares two points w/ start_side
	for (childs_side = 0; childs_side < 6; childs_side++) {
		if ((start_segment == only_child) && (childs_side == start_side))
			continue;
		if (bAlignedSides & (1 << childs_side))
			continue;
		// ignore children of different textures (or no texture)
		if (!IsWall (childnum, childs_side))
			continue;
		if (childSeg->sides [childs_side].nBaseTex != side->nBaseTex)
			continue;
		for (childs_line = 0; childs_line < 4; childs_line++) {
			// find vert numbers for the line's two end points
			childs_point0 = line_vert [side_line [childs_side][childs_line]][0]; 
			childs_point1 = line_vert [side_line [childs_side][childs_line]][1]; 
			childs_vert0  = childSeg->verts [childs_point0]; 
			childs_vert1  = childSeg->verts [childs_point1]; 
			// if points of child's line== corresponding points of parent
			if (!((childs_vert0 == vert1 && childs_vert1 == vert0) ||
					(childs_vert0 == vert0 && childs_vert1 == vert1)))
				continue;
			// now we know the child's side & line which touches the parent
			// child:  childnum, childs_side, childs_line, childs_point0, childs_point1
			// parent: start_segment, start_side, linenum, point0, point1
			childSide = childSeg->sides + childs_side; 
			if (bAlign1st) {
				// now translate all the childs (u, v) coords so child_point1 is at zero
				u0 = childSide->uvls [(childs_line + 1)&3].u; 
				v0 = childSide->uvls [(childs_line + 1)&3].v; 
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0; 
					childSide->uvls [i].v -= v0; 
					}
				// find difference between parent point0 and child point1
				u0 = childSide->uvls [(childs_line + 1)&3].u - side->uvls [linenum].u; 
				v0 = childSide->uvls [(childs_line + 1)&3].v - side->uvls [linenum].v; 
				// find the angle formed by the two lines
				sangle = atan3(side->uvls [(linenum + 1)&3].v - side->uvls [linenum].v, 
				side->uvls [(linenum + 1)&3].u - side->uvls [linenum].u); 
				cangle = atan3(childSide->uvls [childs_line].v - childSide->uvls [(childs_line + 1)&3].v, 
									childSide->uvls [childs_line].u - childSide->uvls [(childs_line + 1)&3].u); 
				// now rotate childs (u, v) coords around child_point1 (cangle - sangle)
				for (i = 0; i < 4; i++) {
					angle = atan3(childSide->uvls [i].v, childSide->uvls [i].u); 
					length = sqrt((double)childSide->uvls [i].u*(double)childSide->uvls [i].u +
									  (double)childSide->uvls [i].v*(double)childSide->uvls [i].v); 
					angle -= (cangle - sangle); 
					childSide->uvls [i].u = (INT16)(length*cos(angle)); 
					childSide->uvls [i].v = (INT16)(length*sin(angle)); 
					}
				// now translate all the childs (u, v) coords to parent point0
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0; 
					childSide->uvls [i].v -= v0; 
					}
				// modulo points by 0x800 (== 64 pixels)
				u0 = childSide->uvls [0].u/0x800; 
				v0 = childSide->uvls [0].v/0x800; 
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0*0x800; 
					childSide->uvls [i].v -= v0*0x800; 
					}
				if (only_child != -1)
					return_code = childs_side; 
				}
			if (bAlign2nd && side->nOvlTex && childSide->nOvlTex) {
				int r;
				switch (side->nOvlTex & 0xC000) {
					case 0:
						r = 0;
						break;
					case 0xC000:
						r = 1;
						break;
					case 0x8000:
						r = 2;
						break;
					case 0x4000:
						r = 3;
						break;
					}
				int h = (int) (fabs (angle) * 180.0 / PI / 90 + 0.5); 
//				h +=(childs_line + linenum + 2) % 4; //(childs_line > linenum) ? childs_line - linenum : linenum - childs_line;
				h = (h + r) % 4;
				childSide->nOvlTex &= ~0xC000;
				switch (h) {
					case 0:
						break;
					case 1:
						childSide->nOvlTex |= 0xC000;
						break;
					case 2:
						childSide->nOvlTex |= 0x8000;
						break;
					case 3:
						childSide->nOvlTex |= 0x4000;
						break;
					}
				}
			break;
			}
		}
	}
theApp.UnlockUndo ();
return return_code; 
}

// --------------------------------------------------------------------------- 
// get_opposing_side()
//
// Action - figures out childs segnum and side for a given side
// Returns - TRUE on success
// --------------------------------------------------------------------------- 

bool CMine::GetOppositeSide (INT16& opp_segnum, INT16& opp_sidenum, INT16 segnum, INT16 sidenum)
{
  INT16 childseg, childside; 

opp_segnum = 0; 
opp_sidenum = 0; 
GetCurrent (segnum, sidenum); 
if (segnum < 0 || segnum >= SegCount ())
	return false; 
if (sidenum < 0 || sidenum >= 6)
	return false; 
childseg =Segments (segnum)->children [sidenum]; 
if (childseg < 0 || childseg >= SegCount ())
	return false; 
for (childside = 0; childside < 6; childside++) {
	if (Segments () [childseg].children [childside]== segnum) {
		opp_segnum = childseg; 
		opp_sidenum = childside; 
		return true; 
		}
	}
return false; 
}

                        /* -------------------------- */

CDSide *CMine::OppSide () 
{
INT16 opp_segnum, opp_sidenum;
if (!GetOppositeSide (opp_segnum, opp_sidenum))
	return NULL;
return Segments (opp_segnum)->sides + opp_sidenum;
}

                        /* -------------------------- */

bool CMine::SetTexture (INT16 segnum, INT16 sidenum, INT16 tmapnum, INT16 tmapnum2)
{
	bool bUndo, bChange = false;
	CDTexture pTx [2];

bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo (); 
GetCurrent (segnum, sidenum); 
if (tmapnum2 == tmapnum)
   tmapnum2 = 0; 
CDSide *side = Segments (segnum)->sides + sidenum; 
if ((tmapnum >= 0) && (tmapnum != side->nBaseTex)) {
	side->nBaseTex = tmapnum; 
	if (tmapnum == (side->nOvlTex & 0x3fff)) {
		side->nOvlTex = 0; 
		}
	bChange = true; 
	}
if (tmapnum2 >= 0) {
	if (tmapnum2 == side->nBaseTex)
		tmapnum2 = 0; 
	if (tmapnum2) {
		side->nOvlTex &= ~(0x3fff);	//preserve light settings
		side->nOvlTex |= tmapnum2; 
		}
	else
		side->nOvlTex = 0; 
	bChange = true; 
	}
if (!bChange) {
	theApp.ResetModified (bUndo);
	return false;
	}
pTextures [file_type][side->nBaseTex].Read (side->nBaseTex);
pTextures [file_type][side->nOvlTex & 0x3fff].Read (side->nOvlTex & 0x3fff);
#if 0
if (((side->nOvlTex & 0x3fff) > 0) &&
    (pTextures [file_type][side->nBaseTex].m_size != 
	  pTextures [file_type][side->nOvlTex & 0x3fff].m_size))
	side->nOvlTex = 0;
#endif
if ((IsLight (side->nBaseTex)== -1) && (IsLight (side->nOvlTex & 0x3fff)== -1))
	DeleteFlickeringLight (segnum, sidenum); 
if (!WallClipFromTexture (segnum, sidenum))
	CheckForDoor (segnum, sidenum); 
theApp.UnlockUndo (); 
sprintf (message, "side has textures %d, %d", side->nBaseTex & 0x3fff, side->nOvlTex & 0x3fff); 
INFOMSG (message); 
return true;
}

                        /* -------------------------- */

void CMine::RenumberBotGens () 
{
	int			i, nMatCens, value, segnum; 
	CDSegment	*seg; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < GameInfo ().botgen.count; i++) {
	segnum = BotGens (i)->segnum; 
	if (segnum >= 0) {
		seg = Segments () + segnum; 
		seg->value = i; 
		if (seg->special== SEGMENT_IS_ROBOTMAKER)
			seg->matcen_num = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, seg = Segments (); i < SegCount (); i++, seg++)
	if (seg->special== SEGMENT_IS_NOTHING)
		seg->value = 0; 
	else
		seg->value = value++; 
}

                        /* -------------------------- */

void CMine::RenumberEquipGens () 
{
	int			i, nMatCens, value, segnum; 
	CDSegment	*seg; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < GameInfo ().equipgen.count; i++) {
	segnum = EquipGens (i)->segnum; 
	if (segnum >= 0) {
		seg = Segments () + segnum; 
		seg->value = i; 
		if (seg->special== SEGMENT_IS_EQUIPMAKER)
			seg->matcen_num = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, seg = Segments (); i < SegCount (); i++, seg++)
	if (seg->special== SEGMENT_IS_NOTHING)
		seg->value = 0; 
	else
		seg->value = value++; 
}

                        /* -------------------------- */

void CMine::CopyOtherCube ()
{
	bool bUndo, bChange = false;

if (Current1 ().segment == Current2 ().segment)
	return; 
INT16 segnum = Current ()->segment; 
CDSegment *otherSeg = OtherSeg (); 
bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo ();
for (int sidenum = 0; sidenum < 6; sidenum++)
	if (SetTexture (segnum, sidenum, 
						 otherSeg->sides [sidenum].nBaseTex, 
						 otherSeg->sides [sidenum].nOvlTex))
		bChange = true;
if (!bChange)
	theApp.ResetModified (bUndo);
else {
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh (); 
	}
}

                        /* -------------------------- */

bool CMine::SplitSegment ()
{
	CDSegment	*centerSegP = CurrSeg (), *segP, *childSegP;
	INT16			centerSegNum = centerSegP - Segments ();
	INT16			segNum, childSegNum;
	INT16			sideNum, oppSideNum, childSideNum;
	INT16			vertNum, wallNum;
	vms_vector	segCenter, *v, *segVert, *centerSegVert;
	bool			bVertDone [8], bUndo;
	int			h, i, j, k;
	INT16			oppSides [6] = {2,3,0,1,5,4};

if (SegCount () >= MAX_SEGMENTS - 6) {
	ErrorMsg("Cannot split this cube because\nthe maximum number of cubes would be exceeded."); 
	return false;
	}
bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo ();
h = VertCount ();
#if 0
// isolate segment
for (sideNum = 0; sideNum < 6; sideNum++) {
	if (segP->children [sideNum] < 0)
		continue;
	for (vertNum = 0; vertNum < 4; vertNum++, h++)
		*Vertices (h) = *Vertices (segP->verts [side_vert [sideNum][vertNum]]);
	}
VertCount () = h;
#endif
// compute segment center
memset (&segCenter, 0, sizeof (segCenter));
for (i = 0; i < 8; i++) {
	v = Vertices (centerSegP->verts [i]);
	segCenter.x += v->x;
	segCenter.y += v->y;
	segCenter.z += v->z;
	}
segCenter.x /= 8;
segCenter.y /= 8;
segCenter.z /= 8;
// add center segment
// compute center segment vertices
memset (bVertDone, 0, sizeof (bVertDone));
for (sideNum = 0; sideNum < 6; sideNum++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = side_vert [sideNum][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegVert = Vertices (centerSegP->verts [j]);
		segVert = Vertices (h + j);
		segVert->x = (centerSegVert->x + segCenter.x) / 2;
		segVert->y = (centerSegVert->y + segCenter.y) / 2;
		segVert->z = (centerSegVert->z + segCenter.z) / 2;
		//centerSegP->verts [j] = h + j;
		}
	}
VertCount () = h + 8;
#if 1
// create the surrounding segments
for (segNum = SegCount (), sideNum = 0; sideNum < 6; segNum++, sideNum++) {
	segP = Segments (segNum);
	oppSideNum = oppSides [sideNum];
	for (vertNum = 0; vertNum < 4; vertNum++) {
		i = side_vert [sideNum][vertNum];
		segP->verts [i] = centerSegP->verts [i];
#if 0
		j = side_vert [oppSideNum][vertNum];
		segP->verts [j] = h + i;
#else
		if ((sideNum & 1) || (sideNum >= 4)) {
			i = line_vert [side_line [sideNum][0]][0];
			j = line_vert [side_line [oppSideNum][2]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][0]][1];
			j = line_vert [side_line [oppSideNum][2]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][2]][0];
			j = line_vert [side_line [oppSideNum][0]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][2]][1];
			j = line_vert [side_line [oppSideNum][0]][1];
			segP->verts [j] = h + i;
			}
		else {
			i = line_vert [side_line [sideNum][0]][0];
			j = line_vert [side_line [oppSideNum][2]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][0]][1];
			j = line_vert [side_line [oppSideNum][2]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][2]][0];
			j = line_vert [side_line [oppSideNum][0]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [sideNum][2]][1];
			j = line_vert [side_line [oppSideNum][0]][0];
			segP->verts [j] = h + i;
			}
#endif
		}
	InitSegment (segNum);
	if ((segP->children [sideNum] = centerSegP->children [sideNum]) > -1) {
		segP->child_bitmask |= (1 << sideNum);
		for (childSegP = Segments (segP->children [sideNum]), childSideNum = 0;
			  childSideNum < 6; 
			  childSideNum++)
			if (childSegP->children [childSideNum] == centerSegNum) {
				childSegP->children [childSideNum] = segNum;
				break;
				}
			}
	segP->children [oppSideNum] = centerSegNum;
	segP->child_bitmask |= (1 << oppSideNum);
	centerSegP->children [sideNum] = segNum;
	centerSegP->child_bitmask |= (1 << sideNum);
	wallNum = centerSegP->sides [sideNum].nWall;
	segP->sides [sideNum].nWall = wallNum;
	if ((wallNum >= 0) && (wallNum != NO_WALL)) {
		Walls (wallNum)->segnum = segNum;
		centerSegP->sides [sideNum].nWall = NO_WALL;
		}
	}
// relocate center segment vertex indices
memset (bVertDone, 0, sizeof (bVertDone));
for (sideNum = 0; sideNum < 6; sideNum++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = side_vert [sideNum][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegP->verts [j] = h + j;
		}
	}
// join adjacent sides of the segments surrounding the center segment
#if 1
for (segNum = 0, segP = Segments (SegCount ()); segNum < 5; segNum++, segP++) {
	for (sideNum = 0; sideNum < 6; sideNum++) {
		if (segP->children [sideNum] >= 0)
			continue;
		for (childSegNum = segNum + 1, childSegP = Segments (SegCount () + childSegNum); 
			  childSegNum < 6; 
			  childSegNum++, childSegP++) {
			for (childSideNum = 0; childSideNum < 6; childSideNum++) {
				if (childSegP->children [childSideNum] >= 0)
					continue;
				h = 0;
				for (i = 0; i < 4; i++) {
					k = segP->verts [side_vert [sideNum][i]];
					for (j = 0; j < 4; j++) {
						if (k == childSegP->verts [side_vert [childSideNum][j]]) {
							h++;
							break;
							}
						}
					}
				if (h == 4) {
					segP->children [sideNum] = SegCount () + childSegNum;
					segP->child_bitmask |= (1 << sideNum);
					childSegP->children [childSideNum] = SegCount () + segNum;
					childSegP->child_bitmask |= (1 << childSideNum);
					break;
					}
				}
			}
		}
	}
#endif
SegCount () += 6;
#endif
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

                        /* -------------------------- */
//eof segment.cpp