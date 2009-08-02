// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "afxpriv.h"
#include "dlc.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "io.h"

#include <math.h>

typedef struct tBugPos {
	int	segnum;
	int	sidenum;
	int	linenum;
	int	pointnum;
	int	childnum;
	int	wallnum;
	int	trignum;
	int	objnum;
} tBugPos;

                        /*--------------------------*/

void CDiagTool::ClearBugList ()
{
if (!m_bInited)
	return;

	CListBox	*plb = LBBugs ();
	int h = plb->GetCount ();
	char szText [256];

tBugPos *pbp;
int i;
for (i = 0; i < h; i++) {
	plb->GetText (i, szText);
	if (!strchr (szText, '['))
		if (pbp = (tBugPos *) plb->GetItemDataPtr (i)) {
			delete pbp;
			plb->SetItemDataPtr (i, NULL);
			}
	}
plb->ResetContent ();
m_nErrors [0] =
m_nErrors [1] = 0;
}

                        /*--------------------------*/

bool CDiagTool::MarkSegment (INT16 segnum) 
{
if ((segnum < 0) || (segnum >= m_mine->SegCount ()))
	return false;
m_mine->Segments (segnum)->wall_bitmask &= ~MARKED_MASK;
m_mine->MarkSegment (segnum);
return true;
}

                        /*--------------------------*/

void CDiagTool::OnShowBug (void)
{
	bool bCurSeg;
	CDWall *pWall;
	int nWall;

if (!GetMine ())
	return;
int i = LBBugs ()->GetCurSel ();
if ((i < 0) || (i >= LBBugs ()->GetCount ()))
	return;
tBugPos *pbp = (tBugPos *) LBBugs ()->GetItemDataPtr (i);
if (!pbp)
	return;
m_mine->UnmarkAll ();
if (bCurSeg = MarkSegment (pbp->segnum))
	m_mine->Current ()->segment = pbp->segnum;
MarkSegment (pbp->childnum);
if ((pbp->sidenum >= 0) && (pbp->sidenum < MAX_SIDES_PER_SEGMENT))
	m_mine->Current ()->side = pbp->sidenum;
if ((pbp->linenum >= 0) && (pbp->linenum < 4))
	m_mine->Current ()->line = pbp->linenum;
if ((pbp->pointnum >= 0) && (pbp->pointnum < 8))
	m_mine->Current ()->point = pbp->pointnum;
if ((pbp->wallnum >= 0) && (pbp->wallnum < m_mine->GameInfo ().walls.count))
	nWall = pbp->wallnum;
else if ((pbp->trignum >= 0) && (pbp->trignum < m_mine->GameInfo ().triggers.count))
	nWall = m_mine->FindTriggerWall (pbp->trignum);
else
	nWall = -1;
if ((nWall >= 0) && MarkSegment ((pWall = m_mine->Walls (nWall))->segnum))
	if (bCurSeg) {
		m_mine->Other ()->segment = pWall->segnum;
		m_mine->Other ()->side = pWall->sidenum;
		}
	else {
		m_mine->Current ()->segment = pWall->segnum;
		m_mine->Current ()->side = pWall->sidenum;
		}
if ((pbp->objnum >= 0) && (pbp->objnum < m_mine->GameInfo ().objects.count))
	m_mine->Current ()->object = pbp->objnum;
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CalcFlatnessRatio ()
//
// 1) calculates the average length lines 0 and 2
// 2) calculates the midpoints of lines 1 and 3
// 3) calculates the length between these two midpoints
// 4) calculates the ratio of the midpoint length over the line lengths
// 5) repeats this for lines 1 and 3 and midpoints 0 and 2
// 6) returns the minimum of the two ratios 
//------------------------------------------------------------------------

double CDiagTool::CalcFlatnessRatio (INT16 segnum, INT16 sidenum) 
{
  INT16 vertnum[4],i;
  vms_vector midpoint1,midpoint2;
  double length1,length2,ave_length,mid_length;
  double ratio1,ratio2;
	vms_vector *vert [4];
  // copy vertnums into an array
	CDSegment *seg = m_mine->Segments (segnum);
  for (i=0;i<4;i++) {
    vertnum[i] = seg->verts[side_vert[sidenum][i]];
	 vert [i] = m_mine->Vertices (vertnum [i]);
  }

  length1 = CalcDistance (vert [0], vert [1], vert [2]);
  length2 = CalcDistance (vert [0], vert [1], vert [3]);
  ave_length = (length1 + length2) / 2;

  midpoint1.x = (vert [0]->x + vert [1]->x) / 2;
  midpoint1.y = (vert [0]->y + vert [1]->y) / 2;
  midpoint1.z = (vert [0]->z + vert [1]->z) / 2;

  midpoint2.x = (vert [2]->x + vert [3]->x) / 2;
  midpoint2.y = (vert [2]->y + vert [3]->y) / 2;
  midpoint2.z = (vert [2]->z + vert [3]->z) / 2;

  mid_length = m_mine->CalcLength (&midpoint1, &midpoint2);

  ratio1 = mid_length/ave_length;

  length1 = CalcDistance (vert [1], vert [2], vert [3]);
  length2 = CalcDistance (vert [1], vert [2], vert [0]);
  ave_length = (length1 + length2) / 2;

  midpoint1.x = (vert [1]->x + vert [2]->x) / 2;
  midpoint1.y = (vert [1]->y + vert [2]->y) / 2;
  midpoint1.z = (vert [1]->z + vert [2]->z) / 2;

  midpoint2.x = (vert [3]->x + vert [0]->x) / 2;
  midpoint2.y = (vert [3]->y + vert [0]->y) / 2;
  midpoint2.z = (vert [3]->z + vert [0]->z) / 2;

  mid_length = m_mine->CalcLength (&midpoint1, &midpoint2);

  ratio2 = mid_length/ave_length;

  return (min (ratio1,ratio2));
}

//--------------------------------------------------------------------------
// CalcDistance
//
// Action - calculates the distance from a line to a point
//--------------------------------------------------------------------------

double CDiagTool::CalcDistance (vms_vector *v1,vms_vector *v2,vms_vector *v3)
{
  dvector A,B,B2;
  double c,a2,distance;

  // normalize all points to vector 1
  A.x = v2->x - v1->x;
  A.y = v2->y - v1->y;
  A.z = v2->z - v1->z;
  B.x = v3->x - v1->x;
  B.y = v3->y - v1->y;
  B.z = v3->z - v1->z;

  // use formula from page 505 of "Calculase and Analytical Geometry" Fifth Addition
  // by Tommas/Finney, Addison-Wesley Publishing Company, June 1981
  //          B * A
  // B2 = B - ----- A
  //          A * A

  a2 = A.x*A.x + A.y*A.y + A.z*A.z;
  if (a2 != 0) {
    c = (B.x*A.x + B.y*A.y + B.z*A.z) / a2;
  } else {
    c = 0;
  }
  B2.x = B.x - c*A.x;
  B2.y = B.y - c*A.y;
  B2.z = B.z - c*A.z;

  distance = sqrt (B2.x*B2.x + B2.y*B2.y + B2.z*B2.z);
  return (distance);
}

//--------------------------------------------------------------------------
// CalcAngle ()
//--------------------------------------------------------------------------

double CDiagTool::CalcAngle (INT16 vert0,INT16 vert1,INT16 vert2,INT16 vert3) 
{
  dvector line1,line2,line3,orthog;
  double ratio;
  double dot_product,magnitude1,magnitude2,angle;
  vms_vector *v0 = m_mine->Vertices (vert0);
  vms_vector *v1 = m_mine->Vertices (vert1);
  vms_vector *v2 = m_mine->Vertices (vert2);
  vms_vector *v3 = m_mine->Vertices (vert3);
      // define lines
      line1.x = ((double) v1->x - (double) v0->x)/F1_0;
      line1.y = ((double) v1->y - (double) v0->y)/F1_0;
      line1.z = ((double) v1->z - (double) v0->z)/F1_0;
      line2.x = ((double) v2->x - (double) v0->x)/F1_0;
      line2.y = ((double) v2->y - (double) v0->y)/F1_0;
      line2.z = ((double) v2->z - (double) v0->z)/F1_0;
      line3.x = ((double) v3->x - (double) v0->x)/F1_0;
      line3.y = ((double) v3->y - (double) v0->y)/F1_0;
      line3.z = ((double) v3->z - (double) v0->z)/F1_0;
      // use cross product to calcluate orthogonal vector
      orthog.x = - (line1.y*line2.z - line1.z*line2.y);
      orthog.y = - (line1.z*line2.x - line1.x*line2.z);
      orthog.z = - (line1.x*line2.y - line1.y*line2.x);
      // use dot product to determine angle A dot B = |A|*|B|*cos (angle)
      // therfore: angle = acos (A dot B / |A|*|B|)
      dot_product = line3.x*orthog.x
		  + line3.y*orthog.y
		  + line3.z*orthog.z;
      magnitude1 = sqrt ( line3.x*line3.x
			+line3.y*line3.y
			+line3.z*line3.z);
      magnitude2 = sqrt ( orthog.x*orthog.x
			+orthog.y*orthog.y
			+orthog.z*orthog.z);
      if (dot_product == 0 || magnitude1 == 0 || magnitude2 == 0) {
        angle = (200.0 * M_PI)/180.0; 
      } else {
	ratio = dot_product/ (magnitude1*magnitude2);
	ratio = ( (double) ( (int) (ratio*1000.0))) / 1000.0; // bug fix 9/21/96
	if (ratio < -1.0 || ratio > (double)1.0) {
	  angle = (199.0 * M_PI)/180.0;
	} else {
	  angle = acos (ratio);
        }
      }
  return fabs (angle);  // angle should be positive since acos returns 0 to PI but why not be sure
}

//--------------------------------------------------------------------------
//  CheckId (local subroutine)
//
//  ACTION - Checks the range of the ID number of an object of a particular
//           type.
//
//  RETURN - Returns TRUE if ID is out of range.  Otherwise, FALSE.
//--------------------------------------------------------------------------

int CDiagTool::CheckId (CDObject *obj) 
{
	int error_flag = 0;
	int type = obj->type;
	int id = obj->id;
	switch (type) {
	case OBJ_ROBOT    : /* an evil enemy */
		if (id < 0 || id >= ((file_type == RDL_FILE) ? ROBOT_IDS1 : ROBOT_IDS2) ) {
			error_flag = 1;
		}
		break;
	case OBJ_HOSTAGE  : /* a hostage you need to rescue */
#if 0
		if (id< 0 || id >1) {
			error_flag = 1;
		}
#endif
		break;
	case OBJ_PLAYER   : /* the player on the console */
		if (id< 0 || id >7) {
			error_flag = 1;
		}
		break;
	case OBJ_POWERUP  : /* a powerup you can pick up */
		if (id< 0 || id >MAX_POWERUP_IDS) {
			error_flag = 1;
		}
		break;
	case OBJ_CNTRLCEN : /* the control center */
		if (file_type == RDL_FILE) {
			if (id< 0 || id >25) {
				error_flag = 1;
			}
		} else {
			if (id< 1 || id >6) {
				error_flag = 1;
			}
		}
		if (error_flag && m_bAutoFixBugs) {
			obj->id = (file_type == RDL_FILE) ? 1 : 2;
			error_flag = 2;
			}
		break;
	case OBJ_COOP     : /* a cooperative player object */
#if 0
		if (id< 8 || id >10) {
			error_flag = 1;
		}
#endif
		break;
	case OBJ_WEAPON: // (d2 only)
		if (id != WEAPON_MINE) {
			error_flag = 1;
		}
	}
	return error_flag;
}

//--------------------------------------------------------------------------
// CDiagTool - WMTimer
//
// Action - Fills list box with data about the mine->  This
//          routine is called once after the dialog is opened.
//
//--------------------------------------------------------------------------

void CDiagTool::OnCheckMine ()
{
if (!GetMine ())
	return;

UpdateData (TRUE);
ClearBugList ();
m_bCheckMsgs = true;
if (m_bAutoFixBugs) {
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	}
  // set mode to BLOCK mode to make errors appear in red
theApp.MineView ()->SetSelectMode (BLOCK_MODE);

// now do actual checking
theApp.MainFrame ()->InitProgress (m_mine->SegCount () * 3 + 
											  m_mine->VertCount () +
											  m_mine->GameInfo ().walls.count * 2 +
											  m_mine->GameInfo ().triggers.count * 3 +
											  m_mine->GameInfo ().objects.count * 2 +
											  m_mine->NumObjTriggers ());
if (!CheckBotGens ())
	if (!CheckEquipGens ())
		if (!CheckSegments ())
			if (!CheckSegTypes ())
				if (!CheckWalls ())
					if (!CheckTriggers ())
						if (!CheckObjects ())
							CheckVertices ();
theApp.MainFrame ()->Progress ().DestroyWindow ();
LBBugs ()->SetCurSel (0);
if (m_bAutoFixBugs)
	theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// CDiagTool - UpdateStats ()
//
// Action  - Updates error and warning counts.
//
// Returns - TRUE if 1000 warnings or errors have occured 
//
// Parameters - error_msg = null terminated string w/ error message
//              error     = set to a 1 if this is an error
//              warning   = set to a 1 if this is a warning 
//--------------------------------------------------------------------------

bool CDiagTool::UpdateStats (char *szError, int nErrorLevel, 
									  int segnum, int sidenum, int linenum, int pointnum, 
									  int childnum, int wallnum, int trignum, int objnum)
{
if (!(szError && *szError))
	return false;
if (!(m_bShowWarnings || !strstr (szError, "WARNING:")))
	return true;
int h = AddMessage (szError, -1, true);
if (h >= 0) {
	tBugPos *pbp = new tBugPos;
	if (!pbp)
		return false;
	pbp->segnum = segnum;
	pbp->sidenum = sidenum;
	pbp->linenum = linenum;
	pbp->pointnum = pointnum;
	pbp->wallnum = wallnum;
	pbp->trignum = trignum;
	pbp->objnum = objnum;
	pbp->childnum = childnum;
	LBBugs ()->SetItemDataPtr (h, (void *) pbp);
	}
*szError = '\0';
m_nErrors [nErrorLevel]++;
if (m_nErrors [0] < 1000 && m_nErrors [1] < 1000)
	return false;
LBBugs ()->AddString ("Maximum number of errors/warnings reached");
return true;
}

//--------------------------------------------------------------------------

bool CDiagTool::CheckSegTypes () 
{
if (!GetMine ())
	return false;

	INT16	i, nBotGens = 0, nEquipGens = 0, nFuelCens = 0;
	CDSegment	*segP = m_mine->Segments ();

for (i = m_mine->SegCount (); i; i--, segP++)
	switch (segP->special) {
		case SEGMENT_IS_ROBOTMAKER:
			nBotGens++;
			break;
		case SEGMENT_IS_EQUIPMAKER:
			nEquipGens++;
			break;
		case SEGMENT_IS_FUELCEN:
			nFuelCens++;
			break;
		default:
			break;
	}
if (nBotGens != m_mine->RobotMakerCount ()) {
	if (m_bAutoFixBugs) {
		sprintf (message, "FIXED: Invalid robot maker count");
		m_mine->RobotMakerCount () = nBotGens;
		}
	else
		sprintf (message,"ERROR: Invalid robot maker count");
	if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, -1))
		return true;
	}
return false;
}

//--------------------------------------------------------------------------
// CDiagTool  LBBugs ()()
//
//  Checks for:
//    out of range data
//    valid cubes geometry
//--------------------------------------------------------------------------

bool CDiagTool::CheckSegments () 
{
if (!GetMine ())
	return false;

  INT16 segnum,sidenum,child,sidenum2,pointnum;
  INT16 vert0,vert1,vert2,vert3;
  INT16 i,j;
  double angle,flatness;
  INT16 match[4];
  CDSegment *seg = m_mine->Segments ();

  // check Segments ()
  //--------------------------------------------------------------
INT16 sub_errors = m_nErrors [0];
INT16 sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Cubes]");

for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++) {
	theApp.MainFrame ()->Progress ().StepIt ();
// check geometry of segment
// 	Given that each point has 3 lines (called L1, L2, and L3),
//	and an orthogonal vector of L1 and L2 (called V1), the angle
//	between L3 and V1 must be less than PI/2.
//
	if (seg->special == SEGMENT_IS_ROBOTMAKER) {
		if ((seg->matcen_num >= m_mine->GameInfo ().botgen.count) || (m_mine->BotGens (seg->matcen_num)->segnum != segnum)) {
	 		sprintf (message, "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", segnum);
			if (m_bAutoFixBugs)
				m_mine->UndefineSegment (segnum);
			}
		}
	if (seg->special == SEGMENT_IS_EQUIPMAKER) {
		if ((seg->matcen_num >= m_mine->GameInfo ().equipgen.count) || (m_mine->BotGens (seg->matcen_num)->segnum != segnum)) {
	 		sprintf (message, "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", segnum);
			if (m_bAutoFixBugs)
				m_mine->UndefineSegment (segnum);
			}
		}

	for (pointnum = 0; pointnum < 8; pointnum++) {
// define vert numbers
		vert0 = seg->verts[pointnum];
		vert1 = seg->verts[connect_points[pointnum][0]];
		vert2 = seg->verts[connect_points[pointnum][1]];
		vert3 = seg->verts[connect_points[pointnum][2]];
		angle = CalcAngle (vert0,vert1,vert2,vert3);
		angle = max (angle,CalcAngle (vert0,vert2,vert3,vert1));
		angle = max (angle,CalcAngle (vert0,vert3,vert1,vert2));
		if (angle > M_PI_2) {
			sprintf (message, "WARNING: Illegal cube geometry (cube=%d,point=%d,angle=%d)",segnum,pointnum, (int) ( (angle*180.0)/M_PI));
			if (UpdateStats (message, 0, segnum, -1, -1, pointnum))
				return true;
			break; // from for loop
			}
		}
	if (pointnum == 8) { // angles must be ok from last test
// now test for flat sides
		for (sidenum = 0; sidenum < 6; sidenum++) {
			flatness = CalcFlatnessRatio (segnum,sidenum);
			if (flatness < 0.80) {
				sprintf (message,"ERROR: Illegal cube geometry (cube=%d,side=%d,flatness=%d%%)",segnum,sidenum, (int) (flatness*100));
				if (UpdateStats (message, 1, segnum, sidenum))
					return true;
				break; // from for loop
				}
			} 
		}

// Check length of each line.
#if 0
	for (linenum=0;linenum<12;linenum++) {
		length = m_mine->CalcLength (m_mine->Vertices (seg->verts[line_vert[linenum][0]]),
											  m_mine->Vertices (seg->verts[line_vert[linenum][1]]));
		if (length < (double)F1_0) {
			sprintf (message,"WARNING: Line length too short (cube=%d,line=%d)",segnum,linenum);
			if (UpdateStats (message, 0, segnum, -1, linenum))
				return true;
			}
		}
#endif
	for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
		child = m_mine->Segments (segnum)->children[sidenum];
// check child range 
		if (child != -1 && child != -2) {
			if (child < -2) {
				sprintf (message,"ERROR: Illegal child number %d (cube=%d,side=%d)",child,segnum,sidenum);
				if (UpdateStats (message, 1, segnum, sidenum))
					return true;
				}
			else if (child >= m_mine->SegCount ()) {
				sprintf (message,"ERROR: Child out of range %d (cube=%d,side=%d)",child,segnum,sidenum);
				if (UpdateStats (message, 1, segnum, sidenum)) 
					return true;
				}
			else {
			// make sure child segment has a child from this segment
			// and that it shares the same vertices as this segment
				for (sidenum2 = 0; sidenum2 < MAX_SIDES_PER_SEGMENT; sidenum2++) {
					if (m_mine->Segments (child)->children[sidenum2] == segnum) 
						break;
					}					
				if (sidenum2 < MAX_SIDES_PER_SEGMENT) {
					memset (match, 0, sizeof (match));
					for (i = 0; i < 4; i++)
						for (j = 0; j < 4; j++)
							if (m_mine->Segments (segnum)->verts[side_vert[sidenum][i]] ==
								 m_mine->Segments (child)->verts[side_vert[sidenum2][j]])
								match[i]++;
					if (match[0]!=1 || match[1]!=1 || match[2]!=1 || match[3]!=1) {
						sprintf (message,"WARNING:Child cube does not share points of cube (cube=%d,side=%d,child=%d)",segnum,sidenum,child);
						if (UpdateStats (message, 0, segnum, -1, -1, -1, child))
							return true;
						}
					}
				else {
					sprintf (message, "WARNING:Child cube does not connect to this cube (cube=%d,side=%d,child=%d)",segnum,sidenum,child);
					if (UpdateStats (message, 1, segnum, -1, -1, -1, child))
						return false;
					}
				}
			}  
		}
	}
if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Cubes] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------
// CDiagTool - check_objects ()
//
// ACTION - Checks object's: segment number, type, id, position, container.
//          Makes sure the correct number of players coop-players, and
//          control centers (robots) are used.
//          Control center belongs to segment with special = SEGMENT_IS_CONTROLCEN
//--------------------------------------------------------------------------

bool CDiagTool::CheckObjects () 
{
if (!GetMine ())
	return false;

  int h,objectnum,type,id,count,player[11],segnum,flags,corner,coops;
  vms_vector center;
  double x,y,z,radius,max_radius,object_radius;
  CDObject *obj = m_mine->Objects ();
	CDObject *pPlayer = NULL;
  int	objCount = m_mine->GameInfo ().objects.count;
  CDSegment *seg;

INT16 sub_errors = m_nErrors [0];
INT16 sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Objects]");
for (objectnum = 0;objectnum < objCount ; objectnum++, obj++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	// check segment range
	segnum = obj->segnum;
	if (segnum < 0 || segnum >= m_mine->SegCount ()) {
		if (m_bAutoFixBugs) {
			obj->segnum = segnum = 0;
			sprintf (message,"FIXED: Bad segment number (object=%d,segnum=%d)",objectnum,segnum);
			}
		else
			sprintf (message,"ERROR: Bad segment number (object=%d,segnum=%d)",objectnum,segnum);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}

	if (segnum < 0 || segnum >= m_mine->SegCount ()) {
		if (m_bAutoFixBugs) {
			obj->segnum = 0;
			sprintf (message,"FIXED: Bad segment number (object=%d,segnum=%d)",objectnum,segnum);
			}
		else
			sprintf (message,"ERROR: Bad segment number (object=%d,segnum=%d)",objectnum,segnum);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}

	seg = m_mine->Segments (segnum);
    // make sure object is within its cube
    // find center of segment then find maximum distance
	// of corner to center.  Calculate Objects () distance
    // from center and make sure it is less than max corner.
    center.x = center.y = center.z = 0;
    for (corner=0;corner<8;corner++) {
		 vms_vector *v = m_mine->Vertices (seg->verts[corner]);
      center.x += v->x;
      center.y += v->y;
      center.z += v->z;
    }
    center.x /= 8; center.y /= 8; center.z /= 8;
    max_radius = 0;
    for (corner=0;corner<8;corner++) {
		 vms_vector *v = m_mine->Vertices (seg->verts[corner]);
      x = v->x - center.x;
      y = v->y - center.y;
      z = v->z - center.z;
	  radius = sqrt (x*x + y*y + z*z);
	  max_radius = max (max_radius,radius);
    }
    x = obj->pos.x - center.x;
    y = obj->pos.y - center.y;
    z = obj->pos.z - center.z;
	object_radius = sqrt (x*x + y*y + z*z);
    if (object_radius > max_radius) {
      sprintf (message,"ERROR: Object is outside of cube (object=%d,cube=%d)",objectnum,segnum);
      if (UpdateStats (message, 1, segnum, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
    }

    // check for non-zero flags (I don't know what these flags are for)
   flags = obj->flags;
	if (flags != 0) {
		if (m_bAutoFixBugs) {
			obj->flags = 0;
			sprintf (message,"FIXED: Flags for object non-zero (object=%d,flags=%d)",objectnum,flags);
			}
		else
			sprintf (message,"ERROR: Flags for object non-zero (object=%d,flags=%d)",objectnum,flags);
      if (UpdateStats (message, 1, segnum, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

    // check type range
	 if ((obj->id < 0) || (obj->id > 255)) {
		 if (m_bAutoFixBugs) {
			sprintf (message,"FIXED: Illegal object id (object=%d,id =%d)",objectnum, obj->id);
			obj->id = 0;
			}
		 else
			sprintf (message,"WARNING: Illegal object id (object=%d,id =%d)",objectnum, obj->id);
		}
	type = obj->type;
    switch (type) {
	  case OBJ_PLAYER:
		  if (!pPlayer)
			  pPlayer = obj;
		  if (obj->id > 7) {
			if (m_bAutoFixBugs) {
				sprintf (message,"FIXED: Illegal player id (object=%d,id =%d)",objectnum, obj->id);
			obj->id = 7;
				}
			else
				sprintf (message,"WARNING: Illegal player id (object=%d,id =%d)",objectnum, obj->id);
			}
	  case OBJ_COOP:
		  if (obj->id > 2) {
			if (m_bAutoFixBugs) {
				sprintf (message,"FIXED: Illegal coop player id (object=%d,id =%d)",objectnum, obj->id);
				obj->id = 2;
				}
			else
				sprintf (message,"WARNING: Illegal coop player id (object=%d,id =%d)",objectnum, obj->id);
			}
			break;
	  case OBJ_EFFECT:
		  if (obj->id > 1) {
			if (m_bAutoFixBugs) {
				sprintf (message,"FIXED: effect id (object=%d,id =%d)",objectnum, obj->id);
				obj->id = 1;
				}
			else
				sprintf (message,"WARNING: Illegal effect id (object=%d,id =%d)",objectnum, obj->id);
			}
			break;
	  case OBJ_ROBOT:
	  case OBJ_HOSTAGE:
	  case OBJ_POWERUP:
     case OBJ_CNTRLCEN:
     case OBJ_WEAPON:
		  break;
	  case OBJ_CAMBOT:
	  case OBJ_SMOKE:
	  case OBJ_MONSTERBALL:
	  case OBJ_EXPLOSION:
			if (file_type != RDL_FILE) 
				break;
	  default:
		 if (m_bAutoFixBugs) {
			 m_mine->DeleteObject (objectnum);
			sprintf (message,"FIXED: Illegal object type (object=%d,type=%d)",objectnum,type);
			}
		 else
			sprintf (message,"WARNING: Illegal object type (object=%d,type=%d)",objectnum,type);
		if (UpdateStats (message,0, segnum, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
    }
    id = obj->id;

    // check id range
    if (h = CheckId (obj)) {
		 if (h == 2)
	      sprintf (message,"FIXED: Illegal object id (object=%d,id=%d)",objectnum,id);
		else
	      sprintf (message,"WARNING: Illegal object id (object=%d,id=%d)",objectnum,id);
      if (UpdateStats (message, 0, segnum, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

	// check contains count range
    count = obj->contains_count;
	if (count < -1) {
		if (m_bAutoFixBugs) {
			obj->contains_count = 0;
		  sprintf (message,"FIXED: Spawn count must be >= -1 (object=%d,count=%d)",objectnum,count);
			}
		else
		  sprintf (message,"WARNING: Spawn count must be >= -1 (object=%d,count=%d)",objectnum,count);
      if (UpdateStats (message, 0, segnum, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

    // check container type range
	if (count > 0) {
      type = obj->contains_type;
	  if (type != OBJ_ROBOT && type != OBJ_POWERUP) {
		if (m_bAutoFixBugs) {
			obj->contains_type = OBJ_POWERUP;
			sprintf (message,"FIXED: Illegal contained type (object=%d,contains=%d)",objectnum,type);
			}
		else
			sprintf (message,"WARNING: Illegal contained type (object=%d,contains=%d)",objectnum,type);
	if (UpdateStats (message, 0, segnum, -1, -1, -1, -1, -1, -1, objectnum)) return true;
	  }
	  id = obj->contains_id;
	  // check contains id range
	  if (CheckId (obj)) {
	sprintf (message,"WARNING: Illegal contains id (object=%d,contains id=%d)",objectnum,id);
	if (UpdateStats (message,0,1)) return true;
	  }
	}
  }

  // make sure object 0 is player 0; if not fix it
if (m_mine->Objects (0)->type != OBJ_PLAYER || m_mine->Objects (0)->id != 0) {
	if (m_bAutoFixBugs) {
		CDObject h;
		memcpy (&h, pPlayer, sizeof (CDObject));
		memcpy (pPlayer, m_mine->Objects (0), sizeof (CDObject));
		memcpy (m_mine->Objects (0), pPlayer, sizeof (CDObject));
		strcpy (message, "FIXED: Object 0 was not Player 0.");
		if (UpdateStats (message, 0, segnum, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}
	else {
		strcpy (message, "WARNING: Object 0 is not Player 0.");
		if (UpdateStats (message, 0, segnum, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}
	}

	// make sure there is the proper number of players and coops
	// reset count to zero
	for (id=0;id<11;id++) {
		player[id] = 0;
	}
	// count each
	coops = 0;
  obj = m_mine->Objects ();
	for (objectnum = 0; objectnum < objCount; objectnum++, obj++) {
		type = obj->type;
		if (type == OBJ_PLAYER) {
			id = obj->id;
			if (id >= 0 && id < 8) {
				while (m_bAutoFixBugs && (id < 8) && player [id])
					id = ++obj->id;
				if (id < 8)
					player[id]++;
			}
		}
#if 1
		if (type == OBJ_COOP) {
			coops++;
		}
#else
		if (type == OBJ_COOP) {
			id = obj->id;
			if (id >= 8 && id <= 10) {
				player[id]++;
			}
		}
#endif
  }
  // makesure each count equals 1
  for (id = 0;id < 8; id++) {
	if (player[id] == 0) {
	  sprintf (message,"WARNING: No player=%d",id);
	  if (UpdateStats (message,0)) 
		  return true;
	}
	if (player[id] > 1) {
	  sprintf (message,"WARNING: Duplicate player #%d (found %d)",id,player[id]);
	  if (UpdateStats (message,0)) 
		  return true;
	}
  }
#if 1
	if (coops != 3) {
		sprintf (message,"WARNING: %d coop players found (should be 3)",coops);
		if (UpdateStats (message,0)) 
			return true;
	}

#else
  for (id=8;id<11;id++) {
	if (player[id] == 0) {
	  sprintf (message,"WARNING: No coop player #%d",id);
	  if (UpdateStats (message,0)) 
		  return true;
	}
	if (player[id] > 1) {
	  sprintf (message,"WARNING: Duplicatecoop player #%d (found %d)",id,player[id]);
	  if (UpdateStats (message,0,1)) 
		  return true;
	}
  }
#endif
  // make sure there is only one control center
count = 0;
obj = m_mine->Objects ();
for (objectnum=0;objectnum<objCount;objectnum++, obj++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	type = obj->type;
	if (type == OBJ_CNTRLCEN) {
		if (m_mine->Segments (obj->segnum)->special != SEGMENT_IS_CONTROLCEN) {
			if (m_bAutoFixBugs && m_mine->AddRobotMaker (obj->segnum, false, false))
				sprintf (message,"FIXED: Reactor belongs to a segment of wrong type (obj=%d, seg=%d)",objectnum,obj->segnum);
			else
				sprintf (message,"WARNING: Reactor belongs to a segment of wrong type (obj=%d, seg=%d)",objectnum,obj->segnum);
			if (UpdateStats (message,0, segnum, -1, -1, -1, -1, -1, -1, objectnum))
				return true;
			}
		count++;
		if (count > 1) {
			sprintf (message,"WARNING: More than one Reactor found (object=%d)",objectnum);
			if (UpdateStats (message,0, segnum, -1, -1, -1, -1, -1, -1, objectnum))
				return true;
			}
		}
	}
if (count < 1) {
	sprintf (message,"WARNING: No Reactors found (note: ok if boss robot used)");
	if (UpdateStats (message,0)) 
		return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Objects] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------
// CDiagTool - check_triggers ()
//
//  x unlinked or overlinked Triggers ()
//
//  o key exists for all locked doors
//  o exit (normal and special)
//  o center exists
//  o non-keyed doors have trigers
//  o Triggers () point to something
//--------------------------------------------------------------------------

bool CDiagTool::CheckObjTriggerList (INT16 nTrigger)
{
	CDObjTriggerList	*ot = m_mine->ObjTriggerList (nTrigger);
	INT16					i, o, objnum = ot->objnum;
	bool					bOk = true;

while ((i = ot->next) > -1) {
	ot = m_mine->ObjTriggerList (i);
	if ((o = ot->objnum) != objnum) {
		bOk = false;
		if (m_bAutoFixBugs) {
#if 1
			*m_mine->ObjTriggerRoot (o) = -1;
#else
			if ((j = *m_mine->ObjTriggerRoot (o)) == i)
				*m_mine->ObjTriggerRoot (o) = m_mine->ObjTriggerList (j)->next;
#endif
			ot->objnum = objnum;
			}
		}
	}
return bOk;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckObjTriggers (void)
{
	INT16					h, i, j;
	short					*or = m_mine->ObjTriggerRoot ();
	CDObjTriggerList	*ot;
	bool					bOk;

for (i = 0; i < MAX_OBJECTS2; i++, or++) {
	bOk = true;
	if ((h = *or) < 0)
		continue; //object doesn't have triggers
	ot = m_mine->ObjTriggerList (h);
	//find the first trigger in the object's trigger list
	while ((j = ot->prev) > -1) {
		ot = m_mine->ObjTriggerList (j);
		if (m_bAutoFixBugs) {
			bOk = false;
			if (ot->objnum == i) {	//root index doesn't point to first trigger in list
				*or = j;
				}
			else { //two lists from different objects linked together, so split them
				ot->next = -1;
				m_mine->ObjTriggerList (h)->prev = -1;
				*or = h;
				break;
				}
			}
		h = j;
		}
	ot = m_mine->ObjTriggerList (h);
	//find the first trigger in the object's trigger list
	while ((j = ot->next) > -1) {
		ot = m_mine->ObjTriggerList (j);
		if (ot->objnum != i) {
			ot->prev = -1;
			m_mine->ObjTriggerList (h)->next = -1;
			bOk = false;
			break;
			}
		h = j;
		}
	if (!bOk) {
		sprintf (message, "%s: Object trigger list corrupted (trigger=%d, object=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", trignum, i);
		if (UpdateStats (message, 0)) return true;
		}
	}
return false;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckTriggers ()
 {
	if (!GetMine ())
		return false;

	int count, trignum, deltrignum, wallnum, i;
	int segnum, sidenum, linknum;
	INT16 opp_segnum, opp_sidenum;

	INT16 sub_errors = m_nErrors [0];
	INT16 sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Triggers]");
	int segCount = m_mine->SegCount ();
	int trigCount = m_mine->GameInfo ().triggers.count;
	CDTrigger *trigger = m_mine->Triggers ();
	int wallCount = m_mine->GameInfo ().walls.count;
	CDWall *wall;
	control_center_trigger *ccTrigger = m_mine->CCTriggers ();

	// make sure trigger is linked to exactly one wall
for (i = 0; i < ccTrigger->num_links; i++)
	if ((ccTrigger->seg [i] >= segCount) ||
		(m_mine->Segments (ccTrigger->seg [i])->sides [ccTrigger->side [i]].nWall >= wallCount)) {
		if (m_bAutoFixBugs) {
			if (i < --(ccTrigger->num_links)) {
				memcpy (ccTrigger->seg + i, ccTrigger->seg + i + 1, (ccTrigger->num_links - i) * sizeof (*(ccTrigger->seg)));
				memcpy (ccTrigger->side + i, ccTrigger->side + i + 1, (ccTrigger->num_links - i) * sizeof (*(ccTrigger->side)));
				}
			strcpy (message, "FIXED: Reactor has invalid trigger target.");
			if (UpdateStats (message, 0))
				return true;
			}
		else {
			strcpy (message, "WARNING: Reactor has invalid trigger target.");
			if (UpdateStats (message, 0))
				return true;
			}
		}
for (trignum = deltrignum = 0; trignum < trigCount; trignum++, trigger++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	count = 0;
	wall = m_mine->Walls ();
	for (wallnum = 0; wallnum < wallCount; wallnum++, wall++) {
		if (wall->trigger == trignum) {
			// if exit, make sure it is linked to control_center_trigger
			int tt = trigger->type;
			int tf = trigger->flags;
			if ((file_type == RDL_FILE) ? tf & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) : tt == TT_EXIT) {
				for (i = 0; i < ccTrigger->num_links; i++)
					if (ccTrigger->seg [i] == wall->segnum &&
						 ccTrigger->side [i] == wall->sidenum)
						break; // found it
				// if did not find it
				if (i>=m_mine->CCTriggers ()->num_links) {
					wall->segnum,wall->sidenum;
					if (m_bAutoFixBugs) {
						m_mine->AutoLinkExitToReactor ();
						sprintf (message,"FIXED: Exit not linked to reactor (cube=%d, side=%d)", wall->segnum, wall->sidenum);
						}
					else
						sprintf (message,"WARNING: Exit not linked to reactor (cube=%d, side=%d)", wall->segnum, wall->sidenum);
					if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum))
						return true;
					}
				}
			count++;
			if (count >1) {
				sprintf (message,"WARNING: Trigger belongs to more than one wall (trig=%d, wall=%d)",trignum,wallnum);
				if (UpdateStats (message,0, wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
			}
		}
	}
	if (count < 1) {
		if (m_bAutoFixBugs) {
			m_mine->DeleteTrigger (trignum);
			trignum--;
			trigger--;
			trigCount--;
			sprintf (message,"FIXED: Unused trigger (trigger=%d)",trignum + deltrignum);
			deltrignum++;
			}
		else
			sprintf (message,"WARNING: Unused trigger (trigger=%d)",trignum);
		if (UpdateStats (message,0,1)) return true;
		}
	}

INT16 trigSeg, trigSide;
trigger = m_mine->Triggers ();
for (trignum = 0; trignum < trigCount; trignum++, trigger++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	wallnum = m_mine->FindTriggerWall (trignum);
	if (wallnum < wallCount) {
		wall = m_mine->Walls (wallnum);
		trigSeg = wall->segnum;
		trigSide = wall->sidenum;
		}
	else
		trigSeg = trigSide = -1;
	// check number of links of trigger (only for
	int tt = trigger->type;
	int tf = trigger->flags;
	if (trigger->num_links == 0) {
		if ((file_type == RDL_FILE) ?
			tf & (TRIGGER_CONTROL_DOORS | TRIGGER_ON | TRIGGER_ONE_SHOT | 
					TRIGGER_MATCEN | TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) :
			   tt==TT_OPEN_DOOR		|| tt==TT_CLOSE_DOOR
			|| tt==TT_MATCEN        || tt==TT_ILLUSION_OFF
			|| tt==TT_ILLUSION_ON   || tt==TT_UNLOCK_DOOR
			|| tt==TT_LOCK_DOOR     || tt==TT_OPEN_WALL
			|| tt==TT_CLOSE_WALL		|| tt==TT_ILLUSORY_WALL
			|| tt==TT_LIGHT_OFF		|| tt==TT_LIGHT_ON
			|| tt==TT_TELEPORT
			) {
			sprintf (message,"WARNING: Trigger has no targets (trigger=%d)",trignum);
			if (UpdateStats (message,0, -1, -1, -1, -1, -1, -1, trignum))
				return true;
			}
		}
	else {
		// check range of links
		for (linknum = 0; linknum < trigger->num_links; linknum++) {
			if (linknum >= MAX_TRIGGER_TARGETS) {
				if (m_bAutoFixBugs) {
					trigger->num_links = MAX_TRIGGER_TARGETS;
					sprintf (message,"FIXED: Trigger has too many targets (trigger=%d, number of links=%d)",trignum,linknum);
					}
				else
					sprintf (message,"WARNING: Trigger has too many targets (trigger=%d, number of links=%d)",trignum,linknum);
				if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
					return true;
				break;
				}
			// check segment range
			segnum = trigger->seg [linknum];
			if (segnum < 0 || segnum >= m_mine->SegCount ()) {
				if (m_bAutoFixBugs) {
					if (m_mine->DeleteTargetFromTrigger (trigger, linknum))
						linknum--;
					else { // => trigger deleted
						linknum = MAX_TRIGGER_TARGETS;	// take care of the loops
						trigger--;
						}
					sprintf (message,"FIXED: Trigger points to non-existant cube (trigger=%d, cube=%d)",trignum,segnum);
					}
				else
					sprintf (message,"ERROR: Trigger points to non-existant cube (trigger=%d, cube=%d)",trignum,segnum);
				if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
					return true;
				}
			else {
				// check side range
				sidenum = trigger->side [linknum];
				if (sidenum < 0 || sidenum >= 6) {
					if (m_bAutoFixBugs) {
						if (m_mine->DeleteTargetFromTrigger (trigger, linknum))
							linknum--;
						else {
							linknum = MAX_TRIGGER_TARGETS;
							trigger--;
							}
						sprintf (message,"FIXED: Trigger points to non-existant side (trigger=%d, side=%d)",trignum,sidenum);
						}
					else
						sprintf (message,"ERROR: Trigger points to non-existant side (trigger=%d, side=%d)",trignum,sidenum);
					if (UpdateStats (message, 1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
						return true;
				} else {
					CDSegment *seg = m_mine->Segments (segnum);
					// check door opening trigger
//						if (trigger->flags == TRIGGER_CONTROL_DOORS) {
					if ((file_type == RDL_FILE) ? 
						 tf & TRIGGER_CONTROL_DOORS :
					    tt==TT_OPEN_DOOR || tt==TT_CLOSE_DOOR || tt==TT_LOCK_DOOR || tt==TT_UNLOCK_DOOR) {
						// make sure trigger points to a wall if it controls doors
						if (seg->sides[sidenum].nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (m_mine->DeleteTargetFromTrigger (trigger, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigger--;
									}
								sprintf (message,"FIXED: Trigger does not target a door (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
								}
							else
								sprintf (message,"WARNING: Trigger does not target a door (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
						}

						// make sure oposite segment/side has a wall too
						if (!m_mine->GetOppositeSide (opp_segnum, opp_sidenum, segnum, sidenum)) {
							sprintf (message,"WARNING: Trigger opens a single sided door (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						else {
							if (m_mine->Segments (opp_segnum)->sides [opp_sidenum].nWall >= wallCount) {
								sprintf (message,"WARNING: Trigger opens a single sided door (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
								if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
								}
							}
						}
					else if ((file_type == RDL_FILE) ? 
						 tf & (TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) :
						 tt == TT_ILLUSION_OFF || tt == TT_ILLUSION_ON ||
						 tt == TT_OPEN_WALL || tt == TT_CLOSE_WALL ||
						 tt == TT_ILLUSORY_WALL) {
						// make sure trigger points to a wall if it controls doors
						if (seg->sides [sidenum].nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (m_mine->DeleteTargetFromTrigger (trigger, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigger--;
									}
								sprintf (message,"FIXED: Trigger target does not exist (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
								}
							else
								sprintf (message,"ERROR: Trigger target does not exist (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						}
//						if (trigger->flags == TRIGGER_MATCEN) {
					else if ((file_type == RDL_FILE) ? tf & TRIGGER_MATCEN : tt == TT_MATCEN) {
						if ((seg->special != SEGMENT_IS_ROBOTMAKER) && (seg->special != SEGMENT_IS_EQUIPMAKER)) {
							sprintf (message,"WARNING: Trigger does not target a robot or equipment maker (trigger=%d, link= (%d,%d))",trignum,segnum,sidenum);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						}
					}
				}
			}
		}
	}

// make sure there is exactly one exit and its linked to the control_center_trigger
count = 0;
trigger = m_mine->Triggers ();
for (trignum = 0; trignum < trigCount; trignum++, trigger++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	wallnum = m_mine->FindTriggerWall (trignum);
	if (wallnum < wallCount) {
		wall = m_mine->Walls (wallnum);
		trigSeg = wall->segnum;
		trigSide = wall->sidenum;
		}
	else
		trigSeg = trigSide = -1;
	int tt = trigger->type;
	int tf = trigger->flags;
	if ((file_type == RDL_FILE) ? tf & TRIGGER_EXIT : tt == TT_EXIT) {
		count++;
		if (count >1) {
			sprintf (message,"WARNING: More than one exit found (trig=%d)",trignum);
			if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
			}
		}
	}

trigCount = m_mine->NumObjTriggers ();
for (trignum = 0; trignum < trigCount; trignum++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	if (!m_mine->ObjTriggerIsInList (trignum)) {
		if (m_bAutoFixBugs) {
#if 1
			CDObjTriggerList *ot = m_mine->UnlinkObjTrigger (trignum);
			m_mine->LinkObjTrigger (ot->objnum, trignum);
#else
			CDObjTriggerList *ot = m_mine->ObjTriggerList (trignum);
			m_mine->DeleteObjTrigger (ot->objnum);
#endif
			sprintf (message,"FIXED: Object trigger linked to wrong object (trigger=%d, object=%d))",trignum,ot->objnum);
			}
		else {
			CDObjTriggerList *ot = m_mine->ObjTriggerList (trignum);
			sprintf (message,"ERROR: Object trigger linked to wrong object (trigger=%d, object=%d))",trignum,ot->objnum);
			if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
			}
		}
	}
if (CheckObjTriggers ())
	return true;
if (count < 1) {
	sprintf (message,"WARNING: No exit found");
	if (UpdateStats (message,0)) return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount () - 1);
	LBBugs ()->AddString ("[Triggers] (no errors)");
	}
return false;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckBotGens (void)
{
	INT16					h = m_mine->SegCount (), j, nSegment, nPass;
	bool					bOk;
	int					nMatCens = int (m_mine->GameInfo ().botgen.count);
	CDSegment*			segP = m_mine->Segments ();
	matcen_info*		matCenP = m_mine->BotGens (0);

for (nPass = 0; nPass < 2; nPass++) {
	int i;
	for (i = 0; i < nMatCens; i++) {
		bOk = true;
		if (!nPass) {
			if (matCenP [i].fuelcen_num != i) {
				bOk = false;
				if (m_bAutoFixBugs)
					matCenP [i].fuelcen_num = i;
				}
			if (((nSegment = matCenP [i].segnum) < h) && (segP [nSegment].special == SEGMENT_IS_ROBOTMAKER) && (segP [nSegment].matcen_num >= nMatCens)) {
				bOk = false;
				if (m_bAutoFixBugs) 
					segP [nSegment].matcen_num = i;
				}
			}
		else {
			if (((nSegment = matCenP [i].segnum) >= h) || (segP [nSegment].special != SEGMENT_IS_ROBOTMAKER)) {
				bOk = false;
				if (m_bAutoFixBugs) {
					for (j = 0; j < h; j++)
						if ((segP->special == SEGMENT_IS_ROBOTMAKER) && (segP->matcen_num == i)) {
							matCenP [i].segnum = nSegment = j;
							break;
							}
					if (j == h) {
						for (j = 0; j < h; j++)
							if (segP [j].matcen_num >= nMatCens) {
								segP [j].matcen_num = i;
								matCenP [i].segnum = nSegment = j;
								break;
								}
						}
					if (!h && (i < --nMatCens)) {
						memcpy (matCenP + i, matCenP + nMatCens, sizeof (matcen_info));
						matCenP [i].fuelcen_num = i;
						i--;
						}
					}
				}
			}
		if (!bOk) {
			sprintf (message, "%s: Robot maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
			if (UpdateStats (message, 0)) return true;
			}
		}
	}
return false;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckEquipGens (void)
{
	INT16					h, nSegment;
	short					*or = m_mine->ObjTriggerRoot ();
	bool					bOk;
	int					nMatCens = int (m_mine->GameInfo ().equipgen.count);
	CDSegment*			segP;
	matcen_info*		matCenP = m_mine->EquipGens (0);

int i;
for (i = 0; i < nMatCens; i++) {
	bOk = true;
	if (matCenP [i].fuelcen_num != i) {
		bOk = false;
		if (m_bAutoFixBugs)
			matCenP [i].fuelcen_num = i;
		}
	segP = m_mine->Segments ();
	if (((nSegment = matCenP [i].segnum) >= (h = m_mine->SegCount ()))  || (segP [nSegment].special != SEGMENT_IS_EQUIPMAKER)) {
		bOk = false;
		if (m_bAutoFixBugs) {
			for (; h; h--, segP++)
				if ((segP->special == SEGMENT_IS_EQUIPMAKER) && (segP->matcen_num == i)) {
					matCenP [i].segnum = nSegment = segP - m_mine->Segments ();
					break;
					}
			if (!h) {
				segP = m_mine->Segments ();
				for (h = m_mine->SegCount (); h; h--, segP++)
					if (segP->matcen_num >= nMatCens) {
						segP->matcen_num = i;
						matCenP [i].segnum = nSegment = segP - m_mine->Segments ();
						break;
						}
				}

			if (!h && (i < --nMatCens)) {
				memcpy (matCenP + i, matCenP + nMatCens, sizeof (matcen_info));
				matCenP [i].fuelcen_num = i;
				i--;
				}
			}
		}
	if (!bOk) {
		sprintf (message, "%s: Equipment maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
		if (UpdateStats (message, 0)) return true;
		}
	}
return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CDWall *CDiagTool::OppWall (UINT16 segnum, UINT16 sidenum)
{
	INT16	oppSegnum, oppSidenum, wallnum;

if (!m_mine->GetOppositeSide (oppSegnum, oppSidenum, segnum, sidenum))
	return NULL;
wallnum = m_mine->Segments (oppSegnum)->sides [oppSidenum].nWall;
if ((wallnum < 0) || (wallnum > MAX_WALLS))
	return NULL;
return m_mine->Walls (wallnum);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CDiagTool::CheckWalls () 
{
	if (!GetMine ())
		return false;
	INT16 segnum,sidenum;
	UINT16 wallnum, wallCount = m_mine->GameInfo ().walls.count, 
			 maxWalls = MAX_WALLS;
	CDSegment *seg;
	CDSide *side;
	CDWall *wall = m_mine->Walls (), *w, *ow;
	int segCount = m_mine->SegCount ();
	UINT8 wallFixed [MAX_WALLS2];

	INT16 sub_errors = m_nErrors [0];
	INT16 sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Walls]");

memset (wallFixed, 0, sizeof (wallFixed));
*message = '\0';
for (segnum = 0, seg = m_mine->Segments (); segnum < segCount; segnum++, seg++) {
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
		wallnum = side->nWall;
		if ((wallnum < 0) || (wallnum >= wallCount) || (wallnum >= maxWalls)) {
			if (wallnum != NO_WALL)
				side->nWall = NO_WALL;
			continue;
			}
		w = m_mine->Walls (wallnum);
		if (w->segnum != segnum) {
			if (m_bAutoFixBugs) {
				sprintf (message,
							"FIXED: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							segnum, wallnum, w->segnum);
				if (wallFixed [wallnum])
					side->nWall = NO_WALL;
				else {
					if (m_mine->Segments (w->segnum)->sides [w->sidenum].nWall == wallnum)
						side->nWall = NO_WALL;
					else {
						w->segnum = segnum;
						w->sidenum = sidenum;
						}
					wallFixed [wallnum] = 1;
					}
				}
			else
				sprintf (message,
							"ERROR: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							segnum, wallnum, w->segnum);
			if (UpdateStats (message,1, segnum, sidenum, -1, -1, -1, side->nWall)) return true;
			} 
		else if (w->sidenum != sidenum) {
			if (m_bAutoFixBugs) {
				sprintf (message,
							"FIXED: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							segnum, sidenum, wallnum, w->segnum);
				if (wallFixed [wallnum])
					side->nWall = NO_WALL;
				else {
					ow = OppWall (segnum, sidenum);
					if (ow && (ow->type == w->type)) {
						seg->sides [w->sidenum].nWall = NO_WALL;
						w->sidenum = sidenum;
						}
					else if (seg->sides [w->sidenum].nWall == wallnum)
						side->nWall = NO_WALL;
					else
						w->sidenum = sidenum;
					wallFixed [wallnum] = 1;
					}
				}
			else
				sprintf (message,
							"ERROR: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							segnum, sidenum, wallnum, w->segnum);
			if (UpdateStats (message,1, -1, -1, -1, -1, -1, side->nWall)) return true;
			}
		} 
	}
for (wallnum = 0; wallnum < wallCount; wallnum++, wall++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	// check wall range type
	if (wall->type > ((file_type == RDL_FILE) ? WALL_CLOSED : (level_version < 9) ? WALL_CLOAKED : WALL_TRANSPARENT)) {
		sprintf (message,
					"ERROR: Wall type out of range (wall=%d, type=%d)",
					wallnum,wall->type);
		if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
		}
		// check range of segment number that the wall points to
	if (wall->segnum >= m_mine->SegCount ()) {
		sprintf (message,
					"ERROR: Wall sits in non-existant cube (wall=%d, cube=%d)",
					wallnum,wall->segnum);
		if (UpdateStats (message,1,-1, -1, -1, -1, -1, wallnum)) return true;
		} 
	else if (wall->sidenum >= 6) {
		// check range of side number that the wall points to
		sprintf (message,
					"ERROR: Wall sits on side which is out of range (wall=%d, side=%d)",
					wallnum,wall->sidenum);
		if (UpdateStats (message,1,-1 -1, -1, -1, -1, wallnum)) return true;
		}
	else {
		// check to make sure segment points back to wall
		side = m_mine->Segments (wall->segnum)->sides + wall->sidenum;
		if (side->nWall != wallnum) {
			w = m_mine->Walls (wallnum);
			if ((wallnum < wallCount) && (w->segnum == wall->segnum) && (w->sidenum == wall->sidenum)) {
				if (m_bAutoFixBugs) {
					sprintf (message,
								"FIXED: Duplicate wall found (wall=%d, cube=%d)", wallnum, wall->segnum);
					m_mine->DeleteWall (wallnum);
					wallnum--;
					wall--;
					wallCount--;
					continue;
					}
				else 
					sprintf (message,
								"ERROR: Duplicate wall found (wall=%d, cube=%d)", wallnum, wall->segnum);
				if (UpdateStats (message, 1, wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
				}
			else {
				if (m_bAutoFixBugs) {
					side->nWall = wallnum;
					sprintf (message,
								"FIXED: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								wallnum,wall->segnum);
					}
				else 
					sprintf (message,
								"ERROR: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								wallnum,wall->segnum);
				if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
				}
			}
			// make sure trigger number of wall is in range
		if ((wall->trigger != NO_TRIGGER) && (wall->trigger >= m_mine->GameInfo ().triggers.count)) {
			if (m_bAutoFixBugs) {
				sprintf (message,
							"FIXED: Wall has invalid trigger (wall=%d, trigger=%d)",
							wallnum, wall->trigger);
				wall->trigger = NO_TRIGGER;
				}
			else
				sprintf (message,
							"ERROR: Wall has invalid trigger (wall=%d, trigger=%d)",
							wallnum, wall->trigger);
			if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
			}
		if ((wall->linked_wall < -1) || (wall->linked_wall >= wallCount)) {
			if (m_bAutoFixBugs) {
				INT16	oppSeg, oppSide, invLinkedWall = wall->linked_wall;
				if (m_mine->GetOppositeSide (oppSeg, oppSide, wall->segnum, wall->sidenum)) {
					wall->linked_wall = m_mine->Segments (oppSeg)->sides [oppSide].nWall;
					if ((wall->linked_wall < -1) || (wall->linked_wall >= wallCount))
						wall->linked_wall = -1;
					sprintf (message,
						"FIXED: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
						wallnum, invLinkedWall, wall->linked_wall);
					}
				}
			else
				sprintf (message,
					"ERROR: Wall has invalid linked wall (wall=%d, linked wall=%d)",
					wallnum,wall->linked_wall);
			}
		else if (wall->linked_wall >= 0) {
			INT16	oppSeg, oppSide;
			if (m_mine->GetOppositeSide (oppSeg, oppSide, wall->segnum, wall->sidenum)) {
				INT16 oppWall = m_mine->Segments (oppSeg)->sides [oppSide].nWall;
				if ((oppWall < 0) || (oppWall >= wallCount)) {
					sprintf (message,
						"%s: Wall links to non-existant wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						wallnum, m_mine->Walls (wall->linked_wall)->segnum, m_mine->Walls (wall->linked_wall)->sidenum);
						if (m_bAutoFixBugs)
							wall->linked_wall = -1;
					}
				else if (wall->linked_wall != oppWall) {
					sprintf (message,
						"%s: Wall links to wrong opposite wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						wallnum, m_mine->Walls (wall->linked_wall)->segnum, m_mine->Walls (wall->linked_wall)->sidenum);
						if (m_bAutoFixBugs)
							wall->linked_wall = oppWall;
					}
				}
			else {
				sprintf (message,
					"%s: Wall links to non-existant side (wall=%d, linked side=%d,%d)",
					m_bAutoFixBugs ? "FIXED" : "ERROR",
					wallnum, m_mine->Walls (wall->linked_wall)->segnum, m_mine->Walls (wall->linked_wall)->sidenum);
				if (m_bAutoFixBugs)
					wall->linked_wall = -1;
				}
			}
		if (UpdateStats (message, 1, wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
			// check wall clip_num
		if ((wall->type == WALL_CLOAKED) && (wall->cloak_value > 31)) {
			if (m_bAutoFixBugs) {
				wall->cloak_value = 31;
				sprintf (message, "FIXED: Wall has invalid cloak value (wall=%d)", wallnum);
					}
			else
				sprintf (message, "ERROR: Wall has invalid cloak value (wall=%d)", wallnum);
			}
		if ((wall->type == WALL_BLASTABLE || wall->type == WALL_DOOR) &&
			 (   wall->clip_num < 0
			  || wall->clip_num == 2
//			     || wall->clip_num == 7
			  || wall->clip_num == 8
			  || (file_type == RDL_FILE && wall->clip_num > 25)
			  || (file_type == RL2_FILE && wall->clip_num > 50))) {
			sprintf (message,
						"ERROR: Illegal wall clip number (wall=%d, clip number=%d)",
						wallnum,wall->clip_num);
			if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
			}
			// Make sure there is a child to the segment
		if (wall->type != WALL_OVERLAY) {
			if (!(m_mine->Segments (wall->segnum)->child_bitmask & (1<< wall->sidenum))) {
				sprintf (message,
							"ERROR: No adjacent cube for this door (wall=%d, cube=%d)",
							wallnum,wall->segnum);
				if (UpdateStats (message,1,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
				}
			else {
				segnum = m_mine->Segments (wall->segnum)->children[wall->sidenum];
				CDSegment *seg = m_mine->Segments (segnum);
				if ((segnum >= 0 && segnum < m_mine->SegCount ()) &&
					 (wall->type == WALL_DOOR || wall->type == WALL_ILLUSION)) {
					// find segment's child side
					for (sidenum=0;sidenum<6;sidenum++)
						if (seg->children[sidenum] == wall->segnum)
							break;
					if (sidenum != 6) {  // if child's side found
						if (seg->sides[sidenum].nWall >= m_mine->GameInfo ().walls.count) {
							sprintf (message,
										"WARNING: No matching wall for this wall (wall=%d, cube=%d)", 
										wallnum,segnum);
							if (UpdateStats (message,0,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
							} 
						else {
							UINT16 wallnum2 = seg->sides[sidenum].nWall;
							if ((wallnum2 < wallCount) &&
								 ((wall->clip_num != m_mine->Walls (wallnum2)->clip_num
									|| wall->type != m_mine->Walls (wallnum2)->type))) {
								sprintf (message,
											"WARNING: Matching wall for this wall is of different type or clip no. (wall=%d, cube=%d)",
											wallnum,segnum);
								if (UpdateStats (message,0,wall->segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
								}
							}
						}
					}
				}
			}
		}
	}

	// make sure seg's wall points back to the segment
seg = m_mine->Segments ();
for (segnum=0;segnum<segCount;segnum++, seg++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	side = seg->sides;
	for (sidenum=0;sidenum<6;sidenum++, side++) {
		if (side->nWall <	wallCount) {
			wallnum = side->nWall;
			if (wallnum >= wallCount) {
				if (m_bAutoFixBugs) {
					side->nWall = wallCount;
					sprintf (message,"FIXED: Cube has an invalid wall number (wall=%d, cube=%d)",wallnum,segnum);
					}
				else
					sprintf (message,"ERROR: Cube has an invalid wall number (wall=%d, cube=%d)",wallnum,segnum);
				if (UpdateStats (message,1, segnum, sidenum)) return true;
			} else {
				if (m_mine->Walls (wallnum)->segnum != segnum) {
					if (m_bAutoFixBugs) {
						m_mine->Walls (wallnum)->segnum = segnum;
						sprintf (message,"FIXED: Cube's wall does not sit in cube (wall=%d, cube=%d)",wallnum,segnum);
						}
					else
						sprintf (message,"ERROR: Cube's wall does not sit in cube (wall=%d, cube=%d)",wallnum,segnum);
					if (UpdateStats (message,1,segnum, wall->sidenum, -1, -1, -1, wallnum)) return true;
					}
				}
			}
		}
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Walls] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------
// unused verticies
// make sure the mine has an exit
//--------------------------------------------------------------------------

bool CDiagTool::CheckVertices () 
{
	if (!GetMine ())
		return false;
//  bool found;
  int segnum,vertnum,point;
  int nUnused = 0;

  INT16 sub_errors = m_nErrors [0];
  INT16 sub_warnings = m_nErrors [1];
  LBBugs ()->AddString ("[Misc]");

  UINT8 *vStat = m_mine->VertStatus ();

for (vertnum= m_mine->VertCount (); vertnum; vertnum--, vStat++)
	*vStat &= ~NEW_MASK;

// mark all used verts
CDSegment *seg = m_mine->Segments ();
for (segnum = m_mine->SegCount (); segnum; segnum--, seg++)
	for (point = 0; point < 8; point++)
		*m_mine->VertStatus (seg->verts [point]) |= NEW_MASK;
vertnum = m_mine->VertCount () - 1;
for (vStat = m_mine->VertStatus (vertnum); vertnum >= 0; vertnum--, vStat--) {
	theApp.MainFrame ()->Progress ().StepIt ();
	if (!(*vStat & NEW_MASK)) {
		nUnused++;
		if (m_bAutoFixBugs) {
			if (vertnum < --m_mine->VertCount ()) {
				memcpy (m_mine->Vertices (vertnum), m_mine->Vertices (vertnum + 1), (m_mine->VertCount () - vertnum) * sizeof (*m_mine->Vertices ()));
				memcpy (m_mine->VertStatus (vertnum), m_mine->VertStatus (vertnum + 1), (m_mine->VertCount () - vertnum) * sizeof (*m_mine->VertStatus ()));
				}
			CDSegment *seg = m_mine->Segments ();
			for (segnum = m_mine->SegCount (); segnum; segnum--, seg++)
				for (point = 0; point < 8; point++)
					if (seg->verts [point] >= vertnum)
						seg->verts [point]--;
			}
		}
	}
vStat = m_mine->VertStatus ();
for (vertnum= m_mine->VertCount (); vertnum; vertnum--, vStat++)
	*vStat &= ~NEW_MASK;
if (nUnused) {
	if (m_bAutoFixBugs)
		sprintf (message,"FIXED: %d unused vertices found", nUnused);
	else
		sprintf (message,"WARNING: %d unused vertices found", nUnused);
	if (UpdateStats (message,0)) return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Misc] (no errors)");
	}
return false;
}

//eof check.cpp