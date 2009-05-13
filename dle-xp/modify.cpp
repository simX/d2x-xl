// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "dlcres.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dlc.h"

#define CURRENT_POINT(a) ((Current ()->point + (a))&0x03)

//This dialog is always active,
//but is hidden or restored by user
//::ShowWindow(hwnd,SW_HIDE);
//::ShowWindow(hwnd,SW_RESTORE);

void CMine::EditGeoFwd ()
{
  double x,y,z;
  double radius;
  vms_vector center,opp_center;
  int i;
/* calculate center of current side */
    center.x = center.y = center.z = 0;
    for (i = 0; i < 4; i++) {
		 int vertnum = Segments (Current ()->segment)->verts [side_vert [Current ()->side][i]];
      center.x += Vertices (vertnum)->x;
      center.y += Vertices (vertnum)->y;
      center.z += Vertices (vertnum)->z;
    }
   center.x /= 4;
   center.y /= 4;
   center.z /= 4;

// calculate center of opposite of current side
    opp_center.x = opp_center.y = opp_center.z = 0;
    for (i = 0; i < 4; i++) {
		 int vertnum = Segments (Current ()->segment)->verts [opp_side_vert [Current ()->side][i]];
      opp_center.x += Vertices (vertnum)->x;
      opp_center.y += Vertices (vertnum)->y;
      opp_center.z += Vertices (vertnum)->z;
    }
   opp_center.x /= 4;
   opp_center.y /= 4;
   opp_center.z /= 4;

// normalize vector
    x = center.x - opp_center.x;
    y = center.y - opp_center.y;
    z = center.z - opp_center.z;

// normalize direction
    radius = sqrt(x*x + y*y + z*z);

    if (radius > (F1_0/10)) {
      x /= radius;
      y /= radius;
      z /= radius;
    } else {
      vms_vector direction;
      CalcOrthoVector(direction,Current ()->segment,Current ()->side);
      x = (double)direction.x/(double)F1_0;
      y = (double)direction.y/(double)F1_0;
      z = (double)direction.z/(double)F1_0;
    }

// move on x, y, and z
	 theApp.SetModified (TRUE);
	 theApp.LockUndo ();
    MoveOn('X', (INT32) (x*move_rate));
    MoveOn('Y', (INT32) (y*move_rate));
    MoveOn('Z', (INT32) (z*move_rate));
	 theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

void CMine::EditGeoBack() 
{
  vms_vector center,opp_center;
  double x,y,z,radius;
  int i;

/* calculate center of current side */
  center.x = center.y = center.z = 0;
  for (i = 0; i < 4; i++) {
	 int vertnum = Segments (Current ()->segment)->verts [side_vert [Current ()->side][i]];
    center.x += Vertices (vertnum)->x;
    center.y += Vertices (vertnum)->y;
    center.z += Vertices (vertnum)->z;
  }
  center.x /= 4;
  center.y /= 4;
  center.z /= 4;

// calculate center of oppisite current side
  opp_center.x = opp_center.y = opp_center.z = 0;
  for (i = 0; i < 4; i++) {
	 int vertnum = Segments (Current ()->segment)->verts [opp_side_vert [Current ()->side][i]];
    opp_center.x += Vertices (vertnum)->x;
    opp_center.y += Vertices (vertnum)->y;
    opp_center.z += Vertices (vertnum)->z;
  }
  opp_center.x /= 4;
  opp_center.y /= 4;
  opp_center.z /= 4;

// normalize vector
  x = center.x - opp_center.x;
  y = center.y - opp_center.y;
  z = center.z - opp_center.z;

  // make sure distance is positive to prevent
  // cube from turning inside out
#if 1
  // defines line orthogonal to a side at a point
  UINT8 orthog_line [6][4] = {
    {8,6,1,3},
    {0,5,7,2},
    {3,1,6,8},
    {2,7,5,0},
    {4,9,10,11},
    {11,10,9,4}
  };
  CDSegment *seg;
  INT16 point0,point1;
  vms_vector *vector0,*vector1;
  bool ok_to_move;

  ok_to_move = TRUE;
  seg = Segments () + Current ()->segment;
  switch (m_selectMode) {
    case POINT_MODE:
      point0 = line_vert [orthog_line [Current ()->side][Current ()->point]][0];
      point1 = line_vert [orthog_line [Current ()->side][Current ()->point]][1];
      vector0 = Vertices (seg->verts [point0]);
      vector1 = Vertices (seg->verts [point1]);
      if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
	ok_to_move = FALSE;
      }
      break;
    case LINE_MODE:
      for (i=0;i<2;i++) {
	point0 = line_vert [orthog_line [Current ()->side][(Current ()->line+i)%4]][0];
	point1 = line_vert [orthog_line [Current ()->side][(Current ()->line+i)%4]][1];
	vector0 = Vertices (seg->verts [point0]);
	vector1 = Vertices (seg->verts [point1]);
	if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
	  ok_to_move = FALSE;
	}
      }
      break;
    case SIDE_MODE:
      for (i = 0; i < 4; i++) {
	point0 = line_vert [orthog_line [Current ()->side][i]][0];
	point1 = line_vert [orthog_line [Current ()->side][i]][1];
	vector0 = Vertices (seg->verts [point0]);
	vector1 = Vertices (seg->verts [point1]);
	if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
	  ok_to_move = FALSE;
	}
      }
      break;
  }
  if (ok_to_move == FALSE) {
    ErrorMsg("Too small to move in that direction");
    return;
  }
#endif

  radius = sqrt(x*x + y*y + z*z);
  if ((radius-move_rate) < F1_0 / 4) {
    if (m_selectMode == POINT_MODE
	|| m_selectMode == LINE_MODE
	|| m_selectMode == SIDE_MODE) {
      ErrorMsg("Cannot make cube any smaller\n"
	       "Cube must be greater or equal to 1.0 units wide.");
    }
  } else {

    // normalize direction
    if (radius > (F1_0/10)) {
      x /= radius;
      y /= radius;
      z /= radius;
    } else {
      vms_vector direction;
      CalcOrthoVector(direction,Current ()->segment,Current ()->side);
      x = (double)direction.x/(double)F1_0;
      y = (double)direction.y/(double)F1_0;
      z = (double)direction.z/(double)F1_0;
    }

// move on x, y, and z
	 theApp.SetModified (TRUE);
	 theApp.LockUndo ();
    MoveOn('X',(INT32) (-x*move_rate));
    MoveOn('Y',(INT32) (-y*move_rate));
    MoveOn('Z',(INT32) (-z*move_rate));
	 theApp.UnlockUndo ();
  }
theApp.SetModified (TRUE);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoRotRight() 
{
  SpinSelection(angle_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoRotLeft() 
{
  SpinSelection(-angle_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoUp() 
{
  if (m_selectMode == SIDE_MODE) {
    RotateSelection(angle_rate,FALSE);
  } else {
    MovePoints(1,0);
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoDown() 
{
  if (m_selectMode == SIDE_MODE) {
    RotateSelection(-angle_rate,FALSE);
  } else {
    MovePoints(0,1);
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoRight() 
{
  if (m_selectMode == SIDE_MODE) {
    RotateSelection(angle_rate,TRUE);
  } else {
    MovePoints(3,0);
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoLeft() 
{
  if (m_selectMode == SIDE_MODE) {
    RotateSelection(-angle_rate,TRUE);
  } else {
    MovePoints(0,3);
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoGrow() 
{
  SizeItem(move_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CMine::EditGeoShrink() 
{
  SizeItem(-move_rate);
}


//------------------------------------------------------------------------
//                    RotateSelection()
//
// ACTION - rotates a side about the opposite side.  The line is drawn
//          between the center points of lines 0 and 2.  If perpendicular
//          is TRUE, then the lines 1 and 3 are used instead.
//------------------------------------------------------------------------

void CMine::RotateSelection (double angle, bool perpendicular) 
{
int nSegment = Current ()->segment;
int nSide = Current ()->side;
CDSegment *seg = Segments (nSegment);
vms_vector center,opp_center;
int i,pts [4];

switch (m_selectMode){
	case POINT_MODE:
	ErrorMsg("Cannot bend a point");
	break; /* can't spin a point */
	case LINE_MODE:
	ErrorMsg("Cannot bend a line");
	break; /* line spinning not supported */
	case SIDE_MODE:	// spin side around the opposite side
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		if (perpendicular) { // use lines 0 and 2
			pts [0] = 1;
			pts [1] = 2;
			pts [2] = 3;
			pts [3] = 0;
			} 
		else {             // use lines 1 and 3
			pts [0] = 0;
			pts [1] = 1;
			pts [2] = 2;
			pts [3] = 3;
			}
		// calculate center opp side line 0
		opp_center.x = (Vertices (seg->verts [opp_side_vert [nSide][pts [0]]])->x +
							 Vertices (seg->verts [opp_side_vert [nSide][pts [1]]])->x) / 2;
		opp_center.y = (Vertices (seg->verts [opp_side_vert [nSide][pts [0]]])->y +
							 Vertices (seg->verts [opp_side_vert [nSide][pts [1]]])->y) / 2;
		opp_center.z = (Vertices (seg->verts [opp_side_vert [nSide][pts [0]]])->z +
							 Vertices (seg->verts [opp_side_vert [nSide][pts [1]]])->z) / 2;
		// calculate center opp side line 2
		center.x = (Vertices (seg->verts [opp_side_vert [nSide][pts [2]]])->x +
						Vertices (seg->verts [opp_side_vert [nSide][pts [3]]])->x) / 2;
		center.y = (Vertices (seg->verts [opp_side_vert [nSide][pts [2]]])->y +
						Vertices (seg->verts [opp_side_vert [nSide][pts [3]]])->y) / 2;
		center.z = (Vertices (seg->verts [opp_side_vert [nSide][pts [2]]])->z +
						Vertices (seg->verts [opp_side_vert [nSide][pts [3]]])->z) / 2;
		// rotate points around a line
		for (i = 0; i < 4; i++)
			RotateVertex (Vertices (seg->verts [side_vert [nSide][i]]),
							  &center, &opp_center, angle);
		theApp.UnlockUndo ();	
		break;
	
	case CUBE_MODE:
		ErrorMsg("Cannot bend a cube");
		break; /* can't spin a point */
	
	case OBJECT_MODE:
		ErrorMsg("Cannot bend a object");
		break; /* can't spin a point */

	case BLOCK_MODE:
		ErrorMsg("Cannot bend a block");
		break; /* can't spin a point */
	}
}

//***************************************************************************
//			SizeItem()
//
// need to prevent reduction through zero
// absolute value of shorts line to size must be greater
// then incremental value if inc is negetive
//
//***************************************************************************

void CMine::SizeItem (INT32 inc) 
{
	int nSegment = Current ()->segment;
	int nSide = Current ()->side;
	CDSegment *seg = Segments (nSegment);
	int i, j, point [4];

switch (m_selectMode) {
	case POINT_MODE:
		break;

	case LINE_MODE:
		point [0] = line_vert [side_line [Current ()->side][Current ()->line]][0];
		point [1] = line_vert [side_line [Current ()->side][Current ()->line]][1];
		SizeLine(seg,point [0],point [1],inc);
		break;

	case SIDE_MODE:
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		for (i = 0; i < 4; i++)
			point [i] = side_vert [Current ()->side][i];
		// enlarge the diagonals
		SizeLine(seg,point [0],point [2],(INT32) (inc*sqrt(2.0)));
		SizeLine(seg,point [1],point [3],(INT32) (inc*sqrt(2.0)));
		theApp.UnlockUndo ();
		break;

	case CUBE_MODE:
		// enlarge the diagonals
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		SizeLine(seg,0,6,(INT32) (inc*sqrt(3.0)));
		SizeLine(seg,1,7,(INT32) (inc*sqrt(3.0)));
		SizeLine(seg,2,4,(INT32) (inc*sqrt(3.0)));
		SizeLine(seg,3,5,(INT32) (inc*sqrt(3.0)));
		theApp.UnlockUndo ();
		break;

	case OBJECT_MODE:
		break;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		vms_vector max_pt, min_pt, center, *verts;
		UINT8 *vstats;
		max_pt.x = -0x7fffffffL;
		max_pt.y = -0x7fffffffL;
		max_pt.z = -0x7fffffffL;
		min_pt.x =  0x7fffffffL;
		min_pt.y =  0x7fffffffL;
		min_pt.z =  0x7fffffffL;
		verts = Vertices ();
		vstats = VertStatus ();
		j = 0;
		for (i = VertCount (), j = 0; j < i; j++, verts++, vstats++)
			if (*vstats & MARKED_MASK) {
				max_pt.x = max (max_pt.x, verts->x);
				max_pt.y = max (max_pt.y, verts->y);
				max_pt.z = max (max_pt.z, verts->z);
				min_pt.x = min (min_pt.x, verts->x);
				min_pt.y = min (min_pt.y, verts->y);
				min_pt.z = min (min_pt.z, verts->z);
				}
		center.x = (max_pt.x + min_pt.x) / 2;
		center.y = (max_pt.y + min_pt.y) / 2;
		center.z = (max_pt.z + min_pt.z) / 2;
		double scale = ((double)(20*F1_0) + (double)inc)/(double)(20*F1_0);
		verts = Vertices ();
		vstats = VertStatus ();
		for (i = VertCount (), j = 0; j < i; j++, verts++, vstats++)
			if (*vstats & MARKED_MASK) {
				verts->x = center.x + (long) ((verts->x - center.x) * scale);
				verts->y = center.y + (long) ((verts->y - center.y) * scale);
				verts->z = center.z + (long) ((verts->z - center.z) * scale);
				}
	break;
	}
}

//--------------------------------------------------------------------------
// MovePoints()
//
// moves blocks, sides, cubes, lines, and points in the direction
// of the current line.
//--------------------------------------------------------------------------

void CMine::MovePoints(int pt0, int pt1) 
{
	vms_vector *vector0,*vector1,delta;
	int point0,point1;
	double length;
	int point;
	int i;
	vms_vector *vect;

point0  = side_vert [Current ()->side][CURRENT_POINT(pt0)];
point1  = side_vert [Current ()->side][CURRENT_POINT(pt1)];
vector0 = Vertices (Segments (Current ()->segment)->verts [point0]);
vector1 = Vertices (Segments (Current ()->segment)->verts [point1]);
length  = CalcLength(vector0,vector1);
if (length >= F1_0) {
	delta.x = (FIX)(((double)(vector1->x - vector0->x) * (double)move_rate)/length);
	delta.y = (FIX)(((double)(vector1->y - vector0->y) * (double)move_rate)/length);
	delta.z = (FIX)(((double)(vector1->z - vector0->z) * (double)move_rate)/length);
	} 
else {
	delta.x = move_rate;
	delta.y = 0;
	delta.z = 0;
	}

switch (m_selectMode){
	case POINT_MODE:
		point = side_vert [Current ()->side][CURRENT_POINT(0)];
		vect  = Vertices (Segments (Current ()->segment)->verts [point]);
		vect->x += delta.x;
		vect->y += delta.y;
		vect->z += delta.z;
		theApp.SetModified (TRUE);
		break;

	case LINE_MODE:
		point = side_vert [Current ()->side][CURRENT_POINT(0)];
		vect  = Vertices (Segments (Current ()->segment)->verts [point]);
		vect->x += delta.x;
		vect->y += delta.y;
		vect->z += delta.z;
		point = side_vert [Current ()->side][CURRENT_POINT(1)];
		vect  = Vertices (Segments (Current ()->segment)->verts [point]);
		vect->x += delta.x;
		vect->y += delta.y;
		vect->z += delta.z;
		theApp.SetModified (TRUE);
		break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++) {
			point = side_vert [Current ()->side][i];
			vect  = Vertices (Segments (Current ()->segment)->verts [point]);
			vect->x += delta.x;
			vect->y += delta.y;
			vect->z += delta.z;
			}
		theApp.SetModified (TRUE);
		break;

	case CUBE_MODE:
		for (i = 0; i < 8; i++) {
			vect = Vertices (Segments (Current ()->segment)->verts [i]);
			vect->x += delta.x;
			vect->y += delta.y;
			vect->z += delta.z;
			}
		theApp.SetModified (TRUE);
		break;

	case OBJECT_MODE:
		CurrObj ()->pos.x += delta.x;
		CurrObj ()->pos.y += delta.y;
		CurrObj ()->pos.z += delta.z;
		theApp.SetModified (TRUE);
		break;

	case BLOCK_MODE:
		bool bMoved = false;
		for (i = 0; i < MAX_VERTICES; i++) {
			if (*VertStatus (i) & MARKED_MASK) {
				Vertices (i)->x += delta.x;
				Vertices (i)->y += delta.y;
				Vertices (i)->z += delta.z;
				bMoved = true;
				}
			}
		theApp.SetModified (bMoved);
		break;
	}
}

//--------------------------------------------------------------------------
//     				SizeLine()
//
// prevent lines from being bigger than 8*20 and less than 3
//--------------------------------------------------------------------------

void CMine::SizeLine (CDSegment *seg,int point0,int point1,INT32 inc) 
{
double x,y,z,radius;

// round a bit
if (inc > 0)
	if (inc & 0xf)
		inc++;
	else if (inc & 1)
		inc--;
else
	if (inc & 0xf)
		inc--;
	else if (inc & 1)
		inc++;

vms_vector *v1 = Vertices (seg->verts [point0]),
			  *v2 = Vertices (seg->verts [point1]);
// figure out direction to modify line
x = v1->x - v2->x;
y = v1->y - v2->y;
z = v1->z - v2->z;
// normalize direction
radius = sqrt (x*x + y*y + z*z);
if (radius > (double) F1_0* (double) F1_0 - inc) 
	return;
if ((inc < 0) && (radius <= (double)-inc*3)) 
	return;
if (radius == 0) 
	return;
x /= radius;
y /= radius;
z /= radius;
// multiply by increment value
x *= inc;
y *= inc;
z *= inc;
// update vertices
v1->x += (INT32)x;
v1->y += (INT32)y;
v1->z += (INT32)z;
v2->x -= (INT32)x;
v2->y -= (INT32)y;
v2->z -= (INT32)z;
}

/***************************************************************************
				MoveOn()
***************************************************************************/

void CMine::MoveOn (char axis,INT32 inc) 
{
int nSegment = Current ()->segment;
int nSide = Current ()->side;
int nPoint = Current ()->point;
int nLine = Current ()->line;
CDSegment *seg = Segments (nSegment);
INT16 i;

theApp.SetModified (TRUE);
switch (m_selectMode) {
	case POINT_MODE:
		switch (axis) {
			case 'X':
				Vertices (seg->verts [side_vert [nSide][nPoint]])->x += inc;
				break;
			case 'Y':
				Vertices (seg->verts [side_vert [nSide][nPoint]])->y += inc;
				break;
			case 'Z':
				Vertices (seg->verts [side_vert [nSide][nPoint]])->z += inc;
				break;
			}
		break;

	case LINE_MODE:
		switch (axis) {
			case 'X':
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][0]])->x += inc;
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][1]])->x += inc;
				break;
			case 'Y':
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][0]])->y += inc;
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][1]])->y += inc;
				break;
			case 'Z':
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][0]])->z += inc;
				Vertices (seg->verts [line_vert [side_line [nSide][nLine]][1]])->z += inc;
				break;
			}
		break;

	case SIDE_MODE:
		switch (axis) {
			case 'X':
			for (i = 0; i < 4; i++)
				Vertices (seg->verts [side_vert [nSide][i]])->x += inc;
			break;
		case 'Y':
			for (i = 0; i < 4; i++)
				Vertices (seg->verts [side_vert [nSide][i]])->y += inc;
			break;
		case 'Z':
			for (i = 0; i < 4; i++)
				Vertices (seg->verts [side_vert [nSide][i]])->z += inc;
			break;
		}
		break;

	case CUBE_MODE:
		switch (axis) {
			case 'X':
				for (i = 0; i < 8; i++)
					Vertices (seg->verts [i])->x += inc;
				for (i = 0; i < GameInfo ().objects.count; i++)
					if (Objects (i)->segnum == nSegment)
						Objects (i)->pos.x += inc;
				break;
			case 'Y':
				for (i = 0; i < 8; i++)
					Vertices (seg->verts [i])->y += inc;
				for (i = 0; i < GameInfo ().objects.count; i++) 
					if (Objects (i)->segnum == nSegment)
						Objects (i)->pos.y += inc;
				break;
			case 'Z':
				for (i = 0; i < 8; i++)
					Vertices (seg->verts [i])->z += inc;
				for (i = 0; i < GameInfo ().objects.count; i++) 
					if (Objects (i)->segnum == nSegment) 
						Objects (i)->pos.z += inc;
				break;
			}
	break;

	case OBJECT_MODE:
		switch (axis) {
			case 'X':
				CurrObj ()->pos.x += inc;
				break;
			case 'Y':
				CurrObj ()->pos.y += inc;
				break;
			case 'Z':
				CurrObj ()->pos.z += inc;
				break;
		}
	break;

	case BLOCK_MODE:
		CDObject *obj = Objects ();
		switch (axis) {
			case 'X':
				for (i = 0; i < MAX_VERTICES; i++)
					if (*VertStatus (i) & MARKED_MASK)
						Vertices (i)->x += inc;
				for (i = GameInfo ().objects.count; i; i--, obj++)
					if (obj->segnum >= 0)
						if (Segments (obj->segnum)->wall_bitmask & MARKED_MASK)
							obj->pos.x += inc;
				break;
			case 'Y':
				for (i = 0; i < MAX_VERTICES; i++)
					if (*VertStatus (i) & MARKED_MASK)
						Vertices (i)->y += inc;
				for (i = GameInfo ().objects.count; i; i--, obj++)
					if (obj->segnum >= 0)
						if (Segments (obj->segnum)->wall_bitmask & MARKED_MASK)
							obj->pos.y += inc;
				break;
			case 'Z':
				for (i = 0; i < MAX_VERTICES; i++)
					if (*VertStatus (i) & MARKED_MASK)
						Vertices (i)->z += inc;
				for (i = GameInfo ().objects.count; i; i--, obj++)
					if (obj->segnum >= 0)
						if (Segments (obj->segnum)->wall_bitmask & MARKED_MASK)
							obj->pos.z += inc;
				break;
		}
	break;
	}
}

/***************************************************************************
			    SpinSelection()

  ACTION - Spins a side, cube, or object the amount specified.


***************************************************************************/

void CMine::SpinSelection(double angle) 
{
	int nSegment = Current ()->segment;
	int nSide = Current ()->side;
	CDSegment *seg = Segments (nSegment);
	CDObject *obj;
	vms_vector center,opp_center;
	INT16 i;

#ifdef SPIN_RELATIVE
	double xspin,yspin,zspin;
	vms_vector rel [3];
#endif

/* calculate segment pointer */
switch (m_selectMode) {
	case POINT_MODE:
		ErrorMsg ("Cannot spin a point");
		break; /* can't spin a point */
	
	case LINE_MODE:
		ErrorMsg ("Cannot spin a line");
		break; /* line spinning not supported */
	
	case SIDE_MODE: // spin side around its center in the plane of the side
		// calculate center of current side
		theApp.SetModified (TRUE);
		center.x = center.y = center.z = 0;
		for (i = 0; i < 4; i++) {
			center.x += Vertices (seg->verts [side_vert [nSide][i]])->x;
			center.y += Vertices (seg->verts [side_vert [nSide][i]])->y;
			center.z += Vertices (seg->verts [side_vert [nSide][i]])->z;
			}
		center.x /= 4;
		center.y /= 4;
		center.z /= 4;
		// calculate orthogonal vector from lines which intersect point 0
		//       |x  y  z |
		// AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
		//       |bx by bz|
		struct vector {double x,y,z;};
		struct vector a,b,c;
		double length;
		INT16 vertnum1,vertnum2;

		vertnum1 = seg->verts [side_vert [nSide][0]];
		vertnum2 = seg->verts [side_vert [nSide][1]];
		a.x = (double)(Vertices (vertnum2)->x - Vertices (vertnum1)->x);
		a.y = (double)(Vertices (vertnum2)->y - Vertices (vertnum1)->y);
		a.z = (double)(Vertices (vertnum2)->z - Vertices (vertnum1)->z);
		vertnum1 = seg->verts [side_vert [nSide][0]];
		vertnum2 = seg->verts [side_vert [nSide][3]];
		b.x = (double)(Vertices (vertnum2)->x - Vertices (vertnum1)->x);
		b.y = (double)(Vertices (vertnum2)->y - Vertices (vertnum1)->y);
		b.z = (double)(Vertices (vertnum2)->z - Vertices (vertnum1)->z);
		c.x = a.y*b.z - a.z*b.y;
		c.y = a.z*b.x - a.x*b.z;
		c.z = a.x*b.y - a.y*b.x;
		// normalize the vector
		length = sqrt(c.x*c.x + c.y*c.y + c.z*c.z);
		c.x /= length;
		c.y /= length;
		c.z /= length;
		// set sign (since vert numbers for most sides don't follow right-handed convention)
		if (nSide!=1 && nSide!=5) {
			c.x = -c.x;
			c.y = -c.y;
			c.z = -c.z;
			}
		// set opposite center
		opp_center.x = center.x + (FIX)(0x10000L*c.x);
		opp_center.y = center.y + (FIX)(0x10000L*c.y);
		opp_center.z = center.z + (FIX)(0x10000L*c.z);
		/* rotate points around a line */
		for (i = 0; i < 4; i++) {
			RotateVertex(Vertices (seg->verts [side_vert [nSide][i]]),
			&center,&opp_center,angle);
			}
		break;


	case CUBE_MODE:	// spin cube around the center of the cube using screen's perspective
		// calculate center of current cube
		theApp.SetModified (TRUE);
		center.x = center.y = center.z = 0;
		for (i = 0; i < 8; i++) {
			center.x += Vertices (seg->verts [i])->x;
			center.y += Vertices (seg->verts [i])->y;
			center.z += Vertices (seg->verts [i])->z;
			}
		center.x /= 8;
		center.y /= 8;
		center.z /= 8;
		// calculate center of oppisite current side
		opp_center.x = opp_center.y = opp_center.z = 0;
		for (i = 0; i < 4; i++) {
			opp_center.x += Vertices (seg->verts [opp_side_vert [nSide][i]])->x;
			opp_center.y += Vertices (seg->verts [opp_side_vert [nSide][i]])->y;
			opp_center.z += Vertices (seg->verts [opp_side_vert [nSide][i]])->z;
			}
		opp_center.x /= 4;
		opp_center.y /= 4;
		opp_center.z /= 4;
		// rotate points about a point
		for (i = 0; i < 8; i++)
			RotateVertex(Vertices (seg->verts [i]),&center,&opp_center,angle);
		break;

	case OBJECT_MODE:	// spin object vector
		theApp.SetModified (TRUE);
		vms_matrix *orient;
		orient = (Current ()->object == GameInfo ().objects.count) ? &SecretOrient () : &CurrObj ()->orient;
		switch (nSide) {
			case 0:
				RotateVmsMatrix(orient,angle,'x');
				break;
			case 2:
				RotateVmsMatrix(orient,-angle,'x');
				break;
			case 1:
				RotateVmsMatrix(orient,-angle,'y');
				break;
			case 3:
				RotateVmsMatrix(orient,angle,'y');
				break;
			case 4:
				RotateVmsMatrix(orient,angle,'z');
				break;
			case 5:
				RotateVmsMatrix(orient,-angle,'z');
				break;
			}
#ifdef SPIN_RELATIVE
		// calculate angles to spin the side into the x-y plane
		// use points 0,1, and 2 of the side
		// make point0 the origin
		// and get coordinates of points 1 and 2 relative to point 0
		for (i=0;i<3;i++) {
			rel [i].x = vertices [seg->verts [side_vert [nSide][i]]].x - vertices [seg->verts [side_vert [nSide][0]]].x;
			rel [i].y = vertices [seg->verts [side_vert [nSide][i]]].y - vertices [seg->verts [side_vert [nSide][0]]].y;
			rel [i].z = vertices [seg->verts [side_vert [nSide][i]]].z - vertices [seg->verts [side_vert [nSide][0]]].z;
			}
		// calculate z-axis spin angle to rotate point1 so it lies in x-y plane
		zspin = (rel [1].x==rel [1].y) ? PI/4 : atan2(rel [1].y,rel [1].x);
		// spin all 3 points on z axis
		for (i=0;i<3;i++)
			RotateVmsVector(&rel [i],zspin,'z');
		// calculate y-axis spin angle to rotate point1 so it lies on x axis
		yspin = (rel [1].z==rel [1].x) ? PI/4 : atan2(rel [1].z,rel [1].x);
		// spin points 1 and 2 on y axis (don't need to spin point 0 since it is at 0,0,0)
		for (i=1;i<=2;i++)
			RotateVmsVector(&rel [i],yspin,'y');
		// calculate x-axis spin angle to rotate point2 so it lies in x-y plane
		xspin = (rel [2].z==rel [2].y) ? PI/4 : atan2(rel [2].z,rel [2].y);
		// spin points 2 on x axis (don't need to spin point 1 since it is on the x-axis
		RotateVmsVector(&rel [2],xspin,'x');
		RotateVmsMatrix(&obj->orient,zspin,'z');
		RotateVmsMatrix(&obj->orient,yspin,'y');
		RotateVmsMatrix(&obj->orient,xspin,'x');
		RotateVmsMatrix(&obj->orient,-xspin,'x');
		RotateVmsMatrix(&obj->orient,-yspin,'y');
		RotateVmsMatrix(&obj->orient,-zspin,'z');
#endif //SPIN_RELATIVE
		break;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		// calculate center of current cube
		center.x = center.y = center.z = 0;
		for (i = 0; i < 8; i++) {
			center.x += Vertices (seg->verts [i])->x;
			center.y += Vertices (seg->verts [i])->y;
			center.z += Vertices (seg->verts [i])->z;
			}
		center.x /= 8;
		center.y /= 8;
		center.z /= 8;
		// calculate center of oppisite current side
		opp_center.x = opp_center.y = opp_center.z = 0;
		for (i = 0; i < 4; i++) {
			opp_center.x += Vertices (seg->verts [opp_side_vert [nSide][i]])->x;
			opp_center.y += Vertices (seg->verts [opp_side_vert [nSide][i]])->y;
			opp_center.z += Vertices (seg->verts [opp_side_vert [nSide][i]])->z;
			}
		opp_center.x /= 4;
		opp_center.y /= 4;
		opp_center.z /= 4;
		// rotate points about a point
		for (i=0;i<VertCount ();i++)
			if (*VertStatus (i) & MARKED_MASK)
				RotateVertex(Vertices (i),&center,&opp_center,angle);
		// rotate Objects () within marked cubes
		obj = Objects ();
		for (i = GameInfo ().objects.count; i; i--, obj++)
			if (Segments (obj->segnum)->wall_bitmask & MARKED_MASK)
				RotateVertex(&obj->pos, &center, &opp_center, angle);
		break;
	}
}


//-----------------------------------------------------------------------------
// RotateVmsVector
//-----------------------------------------------------------------------------

  class vms {
  public:
    FIX x,y,z;
    vms() {x=y=z=0;}
    vms(FIX x0,FIX y0,FIX z0) {
      x=x0;y=y0;z=z0;
    }
    friend vms operator +(vms v1, vms v2) {
      return vms(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z);
    }
  };

 void CMine::RotateVmsVector (vms_vector *vector,double angle,char axis) 
{
  vms_vector n;

  vms m,vect;
  vect.x = vector->x;
  vect.y = vector->y;
  vect.z = vector->z;
  switch(axis) {
    case 'x':
      m.x = (long) (vect.x*cos(angle) + vect.y*sin(angle));
      m.y = (long) (-vect.x*sin(angle) + vect.y*cos(angle));
      vect = m;
      vector->x = vect.x;
      vector->y = vect.y;
      vector->z = vect.z;
//      vector->x = n.x;
//      vector->y = n.y;
      break;
    case 'y':
      n.x = (long) (vector->x*cos(angle) - vector->z*sin(angle));
      n.z = (long) (vector->x*sin(angle) + vector->z*cos(angle));
      vector->x = n.x;
      vector->z = n.z;
      break;
    case 'z':
      n.y = (long) (vector->y*cos(angle) + vector->z*sin(angle));
      n.z = (long) (-vector->y*sin(angle) + vector->z*cos(angle));
      vector->y = n.y;
      vector->z = n.z;
      break;
  }
}

//-----------------------------------------------------------------------------
// RotateVmsMatrix
//-----------------------------------------------------------------------------

void CMine::RotateVmsMatrix(vms_matrix *matrix,double angle,char axis) 
{
  vms_matrix new_vms;
  double cosx,sinx;

  cosx = cos(angle);
  sinx = sin(angle);
      switch (axis) {
	case 'x':
// spin x
//	1	0	0
//	0	cos	sin
//	0	-sin	cos
//
	  new_vms.uvec.x = (long) (matrix->uvec.x * cosx + matrix->fvec.x * sinx);
	  new_vms.uvec.y = (long) (matrix->uvec.y * cosx + matrix->fvec.y * sinx);
	  new_vms.uvec.z = (long) (matrix->uvec.z * cosx + matrix->fvec.z * sinx);
	  new_vms.fvec.x = (long) (-matrix->uvec.x * sinx + matrix->fvec.x * cosx);
	  new_vms.fvec.y = (long) (-matrix->uvec.y * sinx + matrix->fvec.y * cosx);
	  new_vms.fvec.z = (long) (-matrix->uvec.z * sinx + matrix->fvec.z * cosx);
	  matrix->uvec.x = new_vms.uvec.x;
	  matrix->uvec.y = new_vms.uvec.y;
	  matrix->uvec.z = new_vms.uvec.z;
	  matrix->fvec.x = new_vms.fvec.x;
	  matrix->fvec.y = new_vms.fvec.y;
	  matrix->fvec.z = new_vms.fvec.z;
	  break;
	case 'y':
// spin y
//	cos	0	-sin
//	0	1	0
//	sin	0	cos
//
	  new_vms.rvec.x = (long) (matrix->rvec.x * cosx - matrix->fvec.x * sinx);
	  new_vms.rvec.y = (long) (matrix->rvec.y * cosx - matrix->fvec.y * sinx);
	  new_vms.rvec.z = (long) (matrix->rvec.z * cosx - matrix->fvec.z * sinx);
	  new_vms.fvec.x = (long) (matrix->rvec.x * sinx + matrix->fvec.x * cosx);
	  new_vms.fvec.y = (long) (matrix->rvec.y * sinx + matrix->fvec.y * cosx);
	  new_vms.fvec.z = (long) (matrix->rvec.z * sinx + matrix->fvec.z * cosx);
	  matrix->rvec.x = new_vms.rvec.x;
	  matrix->rvec.y = new_vms.rvec.y;
	  matrix->rvec.z = new_vms.rvec.z;
	  matrix->fvec.x = new_vms.fvec.x;
	  matrix->fvec.y = new_vms.fvec.y;
	  matrix->fvec.z = new_vms.fvec.z;
	  break;
	case 'z':
// spin z
//	cos	sin	0
//	-sin	cos	0
//	0	0	1
//
	  new_vms.rvec.x = (long) (matrix->rvec.x * cosx + matrix->uvec.x * sinx);
	  new_vms.rvec.y = (long) (matrix->rvec.y * cosx + matrix->uvec.y * sinx);
	  new_vms.rvec.z = (long) (matrix->rvec.z * cosx + matrix->uvec.z * sinx);
	  new_vms.uvec.x = (long) (-matrix->rvec.x * sinx + matrix->uvec.x * cosx);
	  new_vms.uvec.y = (long) (-matrix->rvec.y * sinx + matrix->uvec.y * cosx);
	  new_vms.uvec.z = (long) (-matrix->rvec.z * sinx + matrix->uvec.z * cosx);
	  matrix->rvec.x = new_vms.rvec.x;
	  matrix->rvec.y = new_vms.rvec.y;
	  matrix->rvec.z = new_vms.rvec.z;
	  matrix->uvec.x = new_vms.uvec.x;
	  matrix->uvec.y = new_vms.uvec.y;
	  matrix->uvec.z = new_vms.uvec.z;
	  break;
      }
}


/***************************************************************************
                           RotateVertex

  ACTION - Rotates a vertex around a center point perpendicular to direction
		   vector.

***************************************************************************/

void CMine::RotateVertex(vms_vector *vertex, vms_vector *origin, vms_vector *normal, double angle) 
{

  double z_spin,y_spin;
  double vx,vy,vz,nx,ny,nz;
  double x1,y1,z1,x2,y2,z2,x3,y3,z3;

  // translate coordanites to origin
  vx = vertex->x - origin->x;
  vy = vertex->y - origin->y;
  vz = vertex->z - origin->z;
  nx = normal->x - origin->x;
  ny = normal->y - origin->y;
  nz = normal->z - origin->z;

  // calculate angles to normalize direction
  // spin on z axis to get into the x-z plane
  if (ny==nx) {
    z_spin = PI/4;
  } else {
    z_spin = atan2(ny,nx);
  }
  x1 =   nx*cos(z_spin) + ny*sin(z_spin);
//  y1 = - nx*sin(z_spin) + ny*cos(z_spin); /* this should equal 0 */
  z1 = nz;

  // spin on y to get on the x axis
  if (z1==x1) {
    y_spin = PI/4;
  } else {
    y_spin = -atan2(z1,x1);
  }
//  x2 =   x1*cos(y_spin) - z1*sin(y_spin);
//  y2 =   y1;                               // this should equal 0
//  z2 =   x1*sin(y_spin) + z1*cos(y_spin);  // this should equal 0
//  if (y2!=0 || x2!=0) {
//    sprintf(message,"Rotation error (y and z should be zero):"
//                      "(x,y,z) = (%f,%f,%f)",(double)x2,(double)y2,(double)z2);
//    DEBUGMSG(message);
//  }

  // normalize vertex (spin on z then y)
  x1 =   vx*cos(z_spin) + vy*sin(z_spin);
  y1 = - vx*sin(z_spin) + vy*cos(z_spin);
  z1 =   vz;

  x2 =   x1*cos(y_spin) - z1*sin(y_spin);
  y2 =   y1;
  z2 =   x1*sin(y_spin) + z1*cos(y_spin);

  // spin x
  x3 =   x2;
  y3 =   y2*cos(angle)  + z2*sin(angle);
  z3 = - y2*sin(angle)  + z2*cos(angle);

  // spin back in negative direction (y first then z)
  x2 =   x3*cos(-y_spin) - z3*sin(-y_spin);
  y2 =   y3;
  z2 =   x3*sin(-y_spin) + z3*cos(-y_spin);

  x1 =   x2*cos(-z_spin) + y2*sin(-z_spin);
  y1 = - x2*sin(-z_spin) + y2*cos(-z_spin) ;
  z1 =   z2;

  // translate back
  vertex->x = (long) (x1 + origin->x);
  vertex->y = (long) (y1 + origin->y);
  vertex->z = (long) (z2 + origin->z);

  // round off values
//  round_off(&vertex->x,grid);
//  round_off(&vertex->y,grid);
//  round_off(&vertex->z,grid);
}

// eof modify.cpp