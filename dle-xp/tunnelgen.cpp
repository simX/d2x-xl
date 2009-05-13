// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dlc.h"
#include "global.h"
#include "dlcdoc.h"
#include <signal.h>

#define CURRENT_POINT(a) ((Current ()->point + (a))&0x03)
#define JOIN_DISTANCE (20*20*F1_0)

//-------------------------------------------------------------------------
// _matherr()
//
// defined to prevent crashes
//-------------------------------------------------------------------------
#if 0
char *whyS [] = {
    "argument domain error",
    "argument singularity ",
    "overflow range error ",
    "underflow range error",
    "total loss of significance",
    "partial loss of significance"
};

int _matherr (struct exception *e) {
  sprintf (message,"DMB has detected a math error\n"
		   "%s (%8g,%8g): %s\n\n"
		   "Press OK to continue, or Cancel to close DMB",
		    e->name, e->arg1, e->arg2, whyS [e->type - 1]);
  if (MessageBox(NULL,message,"Descent Level Editor XP - Error",
	    MB_ICONEXCLAMATION|MB_OKCANCEL+MB_TASKMODAL) == IDCANCEL) {
    if (QueryMsg("Are you sure you want to abort DLE-XP?") == IDYES) {
      return 0;
    }
  }
  return 1; // let program continue
}
#endif
//-------------------------------------------------------------------------
//   Faculty (q) - returns n! (n factorial)
//-------------------------------------------------------------------------
long Faculty (int n) {
  long i;
  int j;

  i=1;
  for (j=n;j>=2;j--) {
    i *= j;
  }
  return i;
}

//-------------------------------------------------------------------------
//   Coeff(n,i) - returns n!/(i!*(n-i)!)
//-------------------------------------------------------------------------
double Coeff(int n, int i) {
  return ((double)Faculty (n)/((double)Faculty (i)*(double)Faculty (n-i)));
}

//-------------------------------------------------------------------------
//   Blend(i,n,u) - returns a weighted coefficient for each point in a spline
//-------------------------------------------------------------------------
double Blend(int i, int n, double u) {
  double partial;
  int j;

  partial = Coeff(n,i);
  for(j=1;j<=i;j++) {
    partial *= u;
  }
  for(j=1;j<=(n-i);j++) {
    partial *= (1-u);
  }
  return partial;
}

//-------------------------------------------------------------------------
//   BezierFcn(pt,u,n,p [] []) - sets (x,y,z) for u=#/segs based on points p
//-------------------------------------------------------------------------
void BezierFcn(vms_vector *pt, double u, int npts, vms_vector *p) {
  int i;
  double b;
  pt->x = 0;
  pt->y = 0;
  pt->z = 0;
  for (i=0;i<npts;i++) {
    b = Blend(i,npts-1,u);
    pt->x += (long) (p [i].x * b + 0.5);
    pt->y += (long) (p [i].y * b + 0.5);
    pt->z += (long) (p [i].z * b + 0.5);
  }
}

//--------------------------------------------------------------------------
// UntwistSegment ()
//
// Action - swaps vertices of opposing side if cube is twisted
//--------------------------------------------------------------------------

void CMine::UntwistSegment (INT16 segnum,INT16 sidenum) 
{
  double len,min_len;
  INT16 index,j;
  CDSegment *seg;
  INT16 verts [4];

seg = Segments (segnum);
// calculate length from point 0 to opp_points
for (j=0;j<4;j++) {
	len = CalcLength (Vertices (seg->verts [side_vert [sidenum] [0]]),
							Vertices (seg->verts [opp_side_vert [sidenum] [j]]));
	if (j==0) {
		min_len = len;
		index = 0;
		}
	else if (len < min_len) {
		min_len = len;
		index = j;
		}
	}
// swap verts if index != 0
if (index != 0) {
	for (j=0;j<4;j++)
		verts [j] = seg->verts [opp_side_vert [sidenum] [(j+index)%4]];
	for (j=0;j<4;j++)
		seg->verts [opp_side_vert [sidenum] [j]] = verts [j];
	}
}

//--------------------------------------------------------------------------
// SpinPoint () - spin on y-axis then z-axis
//--------------------------------------------------------------------------

void SpinPoint (vms_vector *point,double y_spin,double z_spin) 
{
  double tx,ty,tz;
  tx      =   point->x*cos(y_spin) - point->z*sin(y_spin);
  ty      =   point->y;
  tz      =   point->x*sin(y_spin) + point->z*cos(y_spin);
  point->x = (long) (  tx     *cos(z_spin) + ty     *sin(z_spin));
  point->y = (long) (- tx     *sin(z_spin) + ty     *cos(z_spin));
  point->z = (long) (  tz);
}

//--------------------------------------------------------------------------
// SpinBackPoint () - spin on z-axis then y-axis
//--------------------------------------------------------------------------

void SpinBackPoint (vms_vector *point,double y_spin,double z_spin) 
{
  double tx,ty,tz;
  tx      =   point->x*cos(-z_spin) + point->y*sin(-z_spin);
  ty      = - point->x*sin(-z_spin) + point->y*cos(-z_spin);
  tz      =   point->z;
  point->x = (long) (  tx     *cos(-y_spin) - tz     *sin(-y_spin));
  point->y = (long) (  ty);
  point->z = (long) (  tx     *sin(-y_spin) + tz     *cos(-y_spin));
}

//--------------------------------------------------------------------------
// MatchingSide ()
//
// Action - Returns matching side depending on the current points
//--------------------------------------------------------------------------

int CMine::MatchingSide (int j) 
{
  int ret [4] [4] = {{3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  int offset;

  offset = (4+ Current1 ().point - Current2 ().point)%4;
  return ret [offset] [j];
}

//--------------------------------------------------------------------------
//
// Action - Spins points which lie in the y-z plane orthagonal to a normal
//          Uses normal as center for translating points.
//
// Changes - Chooses axis to normalizes on based on "normal" direction
//--------------------------------------------------------------------------

void RectPoints(double angle,double radius,
                  vms_vector *vertex,vms_vector *orgin,vms_vector *normal) 
{
  double y_spin,z_spin;
  double nx,ny,nz;
  double x1,y1,z1,x2,y2,z2;
  char spin_on;

  // translate coordanites to orgin
  nx = normal->x - orgin->x;
  ny = normal->y - orgin->y;
  nz = normal->z - orgin->z;

  // choose rotation order
  if (fabs(nz) > fabs(ny)) {
    spin_on = 'Y';
  } else {
    spin_on = 'Z';
  }


  spin_on = 'Y';

  // start by defining vertex in rectangular coordinates (xz plane)
  vertex->x = 0;
  vertex->y = (long) (radius * cos(angle));
  vertex->z = (long) (radius * sin(angle));

  switch(spin_on) {
    case 'Y':
      // calculate angles to normalize direction
      // spin on y axis to get into the y-z plane
      y_spin = - atan3(nz,nx);
      x1 = nx*cos(y_spin) - nz*sin(y_spin);
      y1 = ny;
      z1 = nx*sin(y_spin) + nz*cos(y_spin);

      // spin on z to get on the x axis
      z_spin = atan3(y1,x1);

      // spin vertex back in negative direction (z first then y)
      x2 =   vertex->x*cos(-z_spin) + vertex->y*sin(-z_spin);
      y2 = - vertex->x*sin(-z_spin) + vertex->y*cos(-z_spin) ;
      z2 =   vertex->z;

      x1 =   x2*cos(-y_spin) - z2*sin(-y_spin);
      y1 =   y2;
      z1 =   x2*sin(-y_spin) + z2*cos(-y_spin);
      break;
    case 'Z':
      // calculate angles to normalize direction
      // spin on z axis to get into the x-z plane
      z_spin = atan3(ny,nx);
      x1 = nx*cos(z_spin) + ny*sin(z_spin);
      y1 = -nx*sin(z_spin) + ny*cos(z_spin);
      z1 = nz;

      // spin on y to get on the x axis
      y_spin = -atan3(z1,x1);

      // spin vertex back in negative direction (y first then z)
      x2 =   vertex->x*cos(-y_spin) - vertex->z*sin(-y_spin);
      y2 =   vertex->y;
      z2 =   vertex->x*sin(-y_spin) + vertex->z*cos(-y_spin);

      x1 =   x2*cos(-z_spin) + y2*sin(-z_spin);
      y1 = - x2*sin(-z_spin) + y2*cos(-z_spin) ;
      z1 =   z2;
      break;
  }
  vertex->x = (long) (normal->x + x1);
  vertex->y = (long) (normal->y + y1);
  vertex->z = (long) (normal->z + z1);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void PolarPoints (double *angle,double *radius,
		  vms_vector *vertex,vms_vector *orgin,vms_vector *normal) 
{
  double z_spin,y_spin;
  double vx,vy,vz,nx,ny,nz;
  double x1,y1,z1,y2,z2;

  // translate coordanites to orgin
  vx = vertex->x - orgin->x;
  vy = vertex->y - orgin->y;
  vz = vertex->z - orgin->z;
  nx = normal->x - orgin->x;
  ny = normal->y - orgin->y;
  nz = normal->z - orgin->z;

  // calculate angles to normalize direction
  // spin on z axis to get into the x-z plane
  z_spin = atan3(ny,nx);
  x1 =   nx*cos(z_spin) + ny*sin(z_spin);
  z1 = nz;

  // spin on y to get on the x axis
  y_spin = -atan3(z1,x1);
  // spin vertex (spin on z then y)
  x1 =   vx*cos(z_spin) + vy*sin(z_spin);
  y1 = - vx*sin(z_spin) + vy*cos(z_spin);
  z1 =   vz;

//  x2 =   x1*cos(y_spin) - z1*sin(y_spin);
  y2 =   y1;
  z2 =   x1*sin(y_spin) + z1*cos(y_spin);

  // convert to polar
  *radius = sqrt(y2*y2 + z2*z2);  // ignore any x offset
  *angle = atan3(z2,y2);
}

//==========================================================================
// MENU - Tunnel Generator
//
// Action - This is like a super "join" feature which uses splines to
//          connect the cubes.  The number of cubes is determined
//          automatically.
//==========================================================================

void CMine::TunnelGenerator() {

//  UpdateUndoBuffer(0);

double length;
int i,j,vertnum,spline;
CDSegment *seg;

if (!m_bSplineActive) {
	m_nMaxSplines = MAX_SEGMENTS - SegCount ();
	if (m_nMaxSplines > MAX_SPLINES)
		m_nMaxSplines = MAX_SPLINES;
	else if (m_nMaxSplines < 3) {
//	if ((VertCount () + 3 /*MAX_SPLINES*/ * 4 > MAX_VERTICES) ||
//		 (SegCount () + 3 /*MAX_SPLINES*/ > MAX_SEGMENTS)) {
		ErrorMsg("Insufficient number of free vertices and/or segments\n"
					"to use the tunnel generator.");
		return;
		}
	// make sure there are no children on either segment/side
	if ((Segments (Current1 ().segment)->children [Current1 ().side] != -1) ||
		 (Segments (Current2 ().segment)->children [Current2 ().side] != -1)) {
		ErrorMsg("Starting and/or ending point of spline\n"
					"already have cube(s) attached.\n\n"
					"Hint: Put the current cube and the alternate cube\n"
					"on sides which do not have cubes attached.");
		return;
		}
	spline_segment1 = Current1 ().segment;
	spline_segment2 = Current2 ().segment;
	spline_side1 = Current1 ().side;
	spline_side2 = Current2 ().side;
	// define 4 data points for spline to work from
	// center of current cube
	CalcCenter (points [0],spline_segment1,spline_side1);
	// center of other cube
	CalcCenter (points [3],spline_segment2,spline_side2);
	// calculate length between cubes
	length = CalcLength (points, points + 3);
	// base spline length on distance between cubes
	m_splineLength1 = (INT16) (length/0x30000L);
	if (m_splineLength1 < MIN_SPLINE_LENGTH)
		m_splineLength1 = MIN_SPLINE_LENGTH;
	if (m_splineLength1 > MAX_SPLINE_LENGTH)
		m_splineLength1 = MAX_SPLINE_LENGTH;
	m_splineLength2 = (INT16) (length/0x30000L);
	if (m_splineLength2 < MIN_SPLINE_LENGTH)
		m_splineLength2 = MIN_SPLINE_LENGTH;
	if (m_splineLength2 > MAX_SPLINE_LENGTH)
		m_splineLength2 = MAX_SPLINE_LENGTH;
	if (length < 50*F1_0) {
		ErrorMsg("End points of tunnel are too close.\n\n"
					"Hint: Select two sides which are further apart\n"
					"using the spacebar and left/right arrow keys,\n"
					"then try again.");
		return;
		}
	if (!bExpertMode)
		ErrorMsg("Place the current cube on one of the tunnel end points.\n\n"
				  "Use the ']' and '[' keys to adjust the length of the red\n"
				  "spline segment.\n\n"
				  "Press 'P' to rotate the point connections.\n\n"
				  "Press 'G' or select Tools/Tunnel Generator when you are finished.");
	m_bSplineActive = TRUE;
	}
else {
	m_bSplineActive = FALSE;
	// ask if user wants to keep the new spline
	if (Query2Msg ("Do you want to keep this tunnel?", MB_YESNO) != IDYES) {
		theApp.MineView ()->Refresh (false);
		return;
		}
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	for (spline = 0; spline < n_splines; spline++) {
		seg = Segments (SegCount ());
		// copy current segment
		*seg = *Segments (Current ()->segment);
		// use last "n_spline" segments
		vertnum = (MAX_VERTICES-1)-(spline*4);
		for (j = 0; j < 4; j++) {
		//	  memcpy(&vertices [VertCount ()],&vertices [vertnum-j],sizeof(vms_vector));
			if (VertCount () >= MAX_VERTICES)
				DEBUGMSG (" Tunnel generator: Vertex count out of range.")
			else if ((vertnum - j < 0) || (vertnum - j >= MAX_VERTICES))
				DEBUGMSG (" Tunnel generator: Vertex number out of range.")
			else
				memcpy (Vertices (VertCount ()), Vertices (vertnum - j), sizeof (*Vertices ()));
/*
			vertices [VertCount ()].x = vertices [vertnum-j].x;
			vertices [VertCount ()].y = vertices [vertnum-j].y;
			vertices [VertCount ()].z = vertices [vertnum-j].z;
*/
			if (spline == 0) {         // 1st segment
				seg->verts [side_vert [spline_side1] [j]] = VertCount ();
				seg->verts [opp_side_vert [spline_side1] [j]] = Segments (spline_segment1)->verts [side_vert [spline_side1] [j]];
				*VertStatus (VertCount ()++) = 0;
				}
			else if(spline < n_splines - 1) { // center segments
				seg->verts [side_vert [spline_side1] [j]] = VertCount ();
				seg->verts [opp_side_vert [spline_side1] [j]] = VertCount () - 4;
				*VertStatus (VertCount ()++) = 0;
				}
			else {          // last segment
				seg->verts [side_vert [spline_side1] [j]] = Segments (spline_segment2)->verts [side_vert [spline_side2] [MatchingSide (j)]];
				seg->verts [opp_side_vert [spline_side1] [j]] = VertCount () - 4 + j;
				}
			}
		// fix twisted segments
		UntwistSegment (SegCount (), spline_side1);
		// define children and sides (textures and nWall)
		for (j = 0; j < 6; j++) {
			seg->children [j] = -1;
			seg->sides [j].nBaseTex = 0;
			seg->sides [j].nOvlTex = 0;
			seg->sides [j].nWall = NO_WALL;
			for (i=0;i<4;i++) {
//	    seg->sides [j].uvls [i].u = default_uvls [i].u;
//	    seg->sides [j].uvls [i].v = default_uvls [i].v;
				seg->sides [j].uvls [i].l = (UINT16) DEFAULT_LIGHTING;
				}
			SetUV (SegCount (),j,0,0,0);
			}
		if (spline==0) {
			seg->children [opp_side [spline_side1]] = spline_segment1;
			seg->children [spline_side1] = SegCount ()+1;
			Segments (spline_segment1)->children [spline_side1] = SegCount ();
			Segments (spline_segment1)->child_bitmask |= (1<<spline_side1);
			} 
		else if (spline<n_splines-1) {
			seg->children [opp_side [spline_side1]] = SegCount ()-1;
			seg->children [spline_side1] = SegCount ()+1;
			}
		else {
			seg->children [opp_side [spline_side1]] = SegCount ()-1;
			seg->children [spline_side1] = spline_segment2;
			Segments (spline_segment2)->children [spline_side2] = SegCount ();
			Segments (spline_segment2)->child_bitmask |= (1<<spline_side2);
			}
		// define child bitmask, special, matcen, value, and wall bitmask
		seg->child_bitmask = (1<<spline_side1) | (1<<opp_side [spline_side1]);
		seg->owner = -1;
		seg->group = -1;
		seg->special = 0;
		seg->matcen_num = -1;
		seg->value = -1;
		seg->wall_bitmask = 0; // make sure segment is not marked
		SegCount ()++;
		}
	theApp.UnlockUndo ();
	}
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// TMainWindow - CM_IncreaseSpline
//==========================================================================

void CMine::IncreaseSpline() 
{

//UpdateUndoBuffer(0);

if (Current ()->segment == spline_segment1)
	if (m_splineLength1 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength1 += SPLINE_INTERVAL;
if (Current ()->segment == spline_segment2)
	if (m_splineLength2 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength2 += SPLINE_INTERVAL;
theApp.MineView ()->Refresh ();
}

//==========================================================================
// TMainWindow - CM_DecreaseSpline
//==========================================================================

void CMine::DecreaseSpline() 
{

//  UpdateUndoBuffer(0);

if (Current ()->segment == spline_segment1)
	if (m_splineLength1 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength1 -= SPLINE_INTERVAL;
if (Current ()->segment == spline_segment2)
	if (m_splineLength2 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength2 -= SPLINE_INTERVAL;
theApp.MineView ()->Refresh ();
}

//-------------------------------------------------------------------------
// CalcSpline ()
//
// Note: this routine is called by dmb.cpp - update_display() everytime
//       the display is redrawn.
//-------------------------------------------------------------------------

void CMine::CalcSpline () 
{
  double length;
  double angle;
  int i,j;
  CDSegment *seg;
  INT16 vertnum;
  vms_vector vertex;
//  vms_vector opp_center;
  double theta [2] [4],radius [2] [4]; // polor coordinates of sides
  double delta_angle [4];
  vms_vector rel_side_pts [2] [4]; // side points reletave to center of side 1
  vms_vector rel_pts [4]; // 4 points of spline reletave to 1st point
  vms_vector rel_spline_pts [MAX_SPLINES];
  double y,z;
  double y_spin,z_spin;
//  double tx,ty,tz;

  // center of both cubes
  CalcCenter (points [0],spline_segment1,spline_side1);
  CalcCenter (points [3],spline_segment2,spline_side2);

  // point orthogonal to center of current cube
  CalcOrthoVector (points [1],spline_segment1,spline_side1);
  points [1].x = m_splineLength1*points [1].x + points [0].x;
  points [1].y = m_splineLength1*points [1].y + points [0].y;
  points [1].z = m_splineLength1*points [1].z + points [0].z;


  // point orthogonal to center of other cube
  CalcOrthoVector (points [2],spline_segment2,spline_side2);
  points [2].x = m_splineLength2*points [2].x + points [3].x;
  points [2].y = m_splineLength2*points [2].y + points [3].y;
  points [2].z = m_splineLength2*points [2].z + points [3].z;

  // calculate number of segments (min=1)
  length = CalcLength (points,points + 3);
  n_splines = (int)(abs(m_splineLength1)+abs(m_splineLength2))/20 + (int)(length / (40.0*(double)0x10000L));
  n_splines = min(n_splines,m_nMaxSplines-1);

  // calculate spline points
  for (i=0;i<=n_splines;i++) {
    BezierFcn (&spline_points [i], (double)i / (double)n_splines, 4, points);
  }

  // make all points reletave to first face (translation)
  for (i=0;i<4;i++) {
    rel_pts [i].x = points [i].x - points [0].x;
    rel_pts [i].y = points [i].y - points [0].y;
    rel_pts [i].z = points [i].z - points [0].z;
  }
  seg = Segments (spline_segment1);
  vms_vector *vert;
  for (i=0;i<4;i++) {
    vertnum = side_vert [spline_side1] [i];
	 vert = Vertices (seg->verts [vertnum]);
    rel_side_pts [0] [i].x = vert->x - points [0].x;
    rel_side_pts [0] [i].y = vert->y - points [0].y;
    rel_side_pts [0] [i].z = vert->z - points [0].z;
  }
  seg = Segments (spline_segment2);
  for (i=0;i<4;i++) {
    vertnum = side_vert [spline_side2] [i];
	 vert = Vertices (seg->verts [vertnum]);
    rel_side_pts [1] [i].x = vert->x - points [0].x;
    rel_side_pts [1] [i].y = vert->y - points [0].y;
    rel_side_pts [1] [i].z = vert->z - points [0].z;
  }
  for (i=0;i<n_splines;i++) {
    rel_spline_pts [i].x = spline_points [i].x - points [0].x;
    rel_spline_pts [i].y = spline_points [i].y - points [0].y;
    rel_spline_pts [i].z = spline_points [i].z - points [0].z;
  }

  // determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
  y_spin = - atan3(rel_pts [1].z, rel_pts [1].x                                     ); // to y-z plane
  z_spin =   atan3(rel_pts [1].y, rel_pts [1].x*cos(y_spin)-rel_pts [1].z*sin(y_spin)); // to x axis

  // spin all points reletave to first face (rotation)
  for (i=0;i<4;i++) {
    SpinPoint (rel_pts + i,y_spin,z_spin);
    for (j=0;j<2;j++) {
      SpinPoint (rel_side_pts [j] + i,y_spin,z_spin);
    }
  }
  for (i=0;i<n_splines;i++) {
    SpinPoint (rel_spline_pts + i,y_spin,z_spin);
  }

  // determine polar coordinates of the 1st side (simply y,z coords)
  for (i=0;i<4;i++) {
    theta [0] [i] = atan3(rel_side_pts [0] [i].z,rel_side_pts [0] [i].y);
    y = (float) rel_side_pts [0] [i].y;
    z = (float) rel_side_pts [0] [i].z;
    radius [0] [i] = sqrt(y*y + z*z);
  }

  // determine polar coordinates of the 2nd side by rotating to x-axis first
  for (i=0;i<4;i++) {
    // flip orthoginal vector to point into cube
    vertex.x = -(rel_pts [2].x-rel_pts [3].x) + rel_pts [3].x;
    vertex.y = -(rel_pts [2].y-rel_pts [3].y) + rel_pts [3].y;
    vertex.z = -(rel_pts [2].z-rel_pts [3].z) + rel_pts [3].z;
    PolarPoints (&theta [1] [i],&radius [1] [i],&rel_side_pts [1] [i],&rel_pts [3],&vertex);
  }

  // figure out the angle differences to be in range (-pi to pi)
  for (j=0;j<4;j++) {
    delta_angle [j] = theta [1] [MatchingSide (j)] - theta [0] [j];
    if (delta_angle [j] < M_PI) {
      delta_angle [j] += 2*M_PI;
    }
    if (delta_angle [j] > M_PI) {
      delta_angle [j] -= 2*M_PI;
    }
  }

  // make sure delta angles do not cross PI & -PI
  for (i=1;i<4;i++) {
    if (delta_angle [i] > delta_angle [0] + M_PI) delta_angle [i] -= 2*M_PI;
    if (delta_angle [i] < delta_angle [0] - M_PI) delta_angle [i] += 2*M_PI;
  }

#if 0
  sprintf(message,"theta [0] = %d,%d,%d,%d\n"
                  "theta [1] = %d,%d,%d,%d\n"
		  "radius [0] = %d,%d,%d,%d\n"
		  "radius [1] = %d,%d,%d,%d\n"
		  "delta_angles = %d,%d,%d,%d\n"
		  "y_spin = %d\n"
		  "z_spin = %d",
		   (int)(theta [0] [0]*180/M_PI),(int)(theta [0] [1]*180/M_PI),(int)(theta [0] [2]*180/M_PI),(int)(theta [0] [3]*180/M_PI),
		   (int)(theta [1] [0]*180/M_PI),(int)(theta [1] [1]*180/M_PI),(int)(theta [1] [2]*180/M_PI),(int)(theta [1] [3]*180/M_PI),
		   (int)(radius [0] [0]*180/M_PI),(int)(radius [0] [1]*180/M_PI),(int)(radius [0] [2]*180/M_PI),(int)(radius [0] [3]*180/M_PI),
		   (int)(radius [1] [0]*180/M_PI),(int)(radius [1] [1]*180/M_PI),(int)(radius [1] [2]*180/M_PI),(int)(radius [1] [3]*180/M_PI),
		   (int)(delta_angle [0]*180/M_PI),(int)(delta_angle [1]*180/M_PI),(int)(delta_angle [2]*180/M_PI),(int)(delta_angle [3]*180/M_PI),
		   (int)(y_spin*180/M_PI),(int)(z_spin*180/M_PI)
		   );
  DebugMsg(message);
#endif

  // calculate segment verticies as weighted average between the two sides
  // then spin verticies in the direction of the segment vector
  for (i=0;i<n_splines;i++) {
    vertnum = (MAX_VERTICES-1)-(i*4);
    for (j=0;j<4;j++) {
	   vert = Vertices (vertnum - j);
      angle  = ((float)i/(float)n_splines) * delta_angle [j] + theta [0] [j];
      length = ((float)i/(float)n_splines) * radius [1] [MatchingSide (j)] + (((float)n_splines-(float)i)/(float)n_splines) * radius [0] [j];
      RectPoints(angle,length,vert,&rel_spline_pts [i],&rel_spline_pts [i+1]);
      // spin vertices
      SpinBackPoint (vert,y_spin,z_spin);
      // translate point back
      vert->x += points [0].x;
      vert->y += points [0].y;
      vert->z += points [0].z;
    }
  }

  // define segment vert numbers
  for (i=0;i<n_splines;i++) {
    // use last "n_spline" segments
    seg = Segments (MAX_SEGMENTS - 1 - i);
    vertnum = MAX_VERTICES - 1 - i * 4;
    for (j = 0; j < 4; j++) {
      if (i == 0) {         // 1st segment
	  seg->verts [side_vert [spline_side1] [j]] = vertnum - j;
	  seg->verts [opp_side_vert [spline_side1] [j]]
	    = Segments (spline_segment1)->verts [side_vert [spline_side1] [j]];
      } else {
	if(i<n_splines-1) { // center segments
	  seg->verts [side_vert [spline_side1] [j]] = vertnum - j;
	  seg->verts [opp_side_vert [spline_side1] [j]] = vertnum + 4 - j;
	} else {          // last segment
	  seg->verts [side_vert [spline_side1] [j]]
	    = Segments (spline_segment2)->verts [side_vert [spline_side2] [MatchingSide (j)]];
	  seg->verts [opp_side_vert [spline_side1] [j]] = vertnum + 4 - j;
        }
      }
    }
  }

  // fix twisted segments
  for (i=0;i<n_splines;i++) {
    UntwistSegment ((MAX_SEGMENTS-1)-i,spline_side1);
  }
}

//eof tunnelgen.cpp