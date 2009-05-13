// Copyright (C) 1997 Bryan Aamot

#include "stdafx.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "mine.h"
#include "global.h"
#include "matrix.h"

CMatrix::CMatrix () 
{
Set (0,0,0,1,1,1,0,0,0);
_scale = 1;
_angles [0] = 
_angles [1] =
_angles [2] = 0;
}

//--------------------------------------------------------------------------
// SetViewInfo
//--------------------------------------------------------------------------

void CMatrix::SetViewInfo(double depthPerception, INT16 viewWidth, INT16 viewHeight)
{
	_depthPerception = depthPerception;
	_viewWidth = viewWidth / 2;
	_viewHeight = viewHeight / 2;
}

//--------------------------------------------------------------------------
// Set()
//--------------------------------------------------------------------------

void CMatrix::Set (
	double movex, double movey, double movez, 
	double sizex, double sizey, double sizez,
	double spinx, double spiny, double spinz) 
{

	double divisor;
	
	/* calculate matrix values for spin z then y then x */
	
	M[0][0] = 1;
	M[0][1] = 0;
	M[0][2] = 0;
	M[0][3] = 0;
	
	M[1][0] = movex;
	M[2][0] = movey;
	M[3][0] = movez;
	
	M[1][1] = (double) ( sizex * cos (spinz) * cos (spiny));
	M[1][2] = (double) ( sizey * sin (spinz) * cos (spiny));
	M[1][3] = (double) (-sizez * sin (spiny));
	M[2][1] = (double) (-sizex * sin (spinz) * cos (spinx)) +
				 (double) ( sizex * cos (spinz) * sin (spiny) * sin (spinx));
	M[2][2] = (double) ( sizey * sin (spinz) * sin (spiny) * sin (spinx)) +
				 (double) (+sizey * cos (spinz) * cos (spinx));
	M[2][3] = (double) ( sizez * cos (spiny) * sin (spinx));
	M[3][1] = (double) ( sizex * cos (spinz) * sin (spiny) * cos (spinx)) +
				 (double) (-sizex * sin (spinz) * sin (spinx));
	M[3][2] = (double) ( sizey * sin (spinz) * sin (spiny) * cos (spinx)) +
				 (double) (-sizey * cos (spinz) * sin (spinx));
	M[3][3] = (double) ( sizez * cos (spiny) * cos (spinx));
	
	IM[0][0] = 1;
	IM[0][1] = 0;
	IM[0][2] = 0;
	IM[0][3] = 0;
	
	divisor  = 
		  M[1][1] * (M[3][2] * M[2][3] - M[2][2] * M[3][3])
		+ M[2][1] * (M[1][2] * M[3][3] - M[3][2] * M[1][3])
		+ M[3][1] * (M[2][2] * M[1][3] - M[1][2] * M[2][3]);
	if (divisor != 0) { 
		IM[1][1] = (M[3][2] * M[2][3] - M[2][2] * M[3][3]) / divisor;
		IM[1][2] = (M[1][2] * M[3][3] - M[3][2] * M[1][3]) / divisor;
		IM[1][3] = (M[2][2] * M[1][3] - M[1][2] * M[2][3]) / divisor;
		IM[2][1] = (M[2][1] * M[3][3] - M[3][1] * M[2][3]) / divisor;
		IM[2][2] = (M[3][1] * M[1][3] - M[1][1] * M[3][3]) / divisor;
		IM[2][3] = (M[1][1] * M[2][3] - M[2][1] * M[1][3]) / divisor;
		IM[3][1] = (M[3][1] * M[2][2] - M[2][1] * M[3][2]) / divisor;
		IM[3][2] = (M[1][1] * M[3][2] - M[3][1] * M[1][2]) / divisor;
		IM[3][3] = (M[2][1] * M[1][2] - M[1][1] * M[2][2]) / divisor;
	}
	IM[1][0] = movex * IM[1][1] + movey * IM[1][2] + movez * IM[1][3];
	IM[2][0] = movex * IM[2][1] + movey * IM[2][2] + movez * IM[2][3];
	IM[3][0] = movex * IM[3][1] + movey * IM[3][2] + movez * IM[3][3];
	
}

//--------------------------------------------------------------------------
// Rotate()
//--------------------------------------------------------------------------

void CMatrix::ClampAngle (int i)
{
if (_angles [i] < 0)
	_angles [i] += (int) (-_angles [i] / 360) * 360;
else
	_angles [i] -= (int) (_angles [i] / 360) * 360;
}

//--------------------------------------------------------------------------

void CMatrix::RotateAngle (int i, double a)
{
_angles [i] += a;
//ClampAngle (i);
}

//--------------------------------------------------------------------------

void CMatrix::Push (void)
{
memcpy (Msave, M, sizeof (M));
memcpy (IMsave, IM, sizeof (IM));
memcpy (_angleSave, _angles, sizeof (_angles));
_scaleSave = _scale;
}

//--------------------------------------------------------------------------

void CMatrix::Pop (void)
{
memcpy (M, Msave, sizeof (M));
memcpy (IM, IMsave, sizeof (IM));
memcpy (_angles, _angleSave, sizeof (_angles));
_scale = _scaleSave;
}

//--------------------------------------------------------------------------

void CMatrix::Unrotate (void)
{
#if 0
Rotate ('X', -_angles [0]);
Rotate ('Y', -_angles [1]);
Rotate ('Z', -_angles [2]);
#else
Set (0,0,0,1,1,1,0,0,0);
#endif
Scale (_scale);
Calculate (Msave [1][0], Msave [2][0], Msave [3][0]);
}

//--------------------------------------------------------------------------

void CMatrix::Rotate(char axis, double angle) 
{
if (angle) {
	double cosa = (double)cos(angle);
	double sina = (double)sin(angle);
	double S[4][4];
	switch(axis) {
    default:
    case 'X':
		S[2][3] = sina;
		S[3][2] = -sina; 
		S[2][2] =
		S[3][3] = cosa;
		S[1][2] =
		S[1][3] =
		S[2][1] =
		S[3][1] = 0.f; 
		S[1][1] = 1.f; 
		RotateAngle (0, angle);
		break;
    case 'Y':
		S[1][3] = -sina;
		S[3][1] = sina; 
		S[1][1] = 
		S[3][3] = cosa;
		S[1][2] = 
		S[2][1] = 
		S[2][3] = 
		S[3][2] = 0.f; 
		S[2][2] = 1.f; 
		RotateAngle (1, angle);
		break;
    case 'Z':
		S[1][2] = sina; 
		S[2][1] = -sina; 
		S[1][1] = 
		S[2][2] = cosa; 
		S[1][3] = 
		S[2][3] = 
		S[3][1] = 
		S[3][2] = 0.f; 
		S[3][3] = 1.f;
		RotateAngle (2, angle);
		break;
	}
#if 1
	Multiply(IM,S); // rotate relative to screen
#else
	Multiply(M,S); // rotate relative to x,y,z axii
	CalculateInverse();
#endif
	}
}

//--------------------------------------------------------------------------
// Scale()
//--------------------------------------------------------------------------

void CMatrix::Scale(double scale) 
{
	double S[4][4];

	S[1][1] = scale; 
	S[1][2] = 0; 
	S[1][3] = 0;
	S[2][1] = 0; 
	S[2][2] = scale; 
	S[2][3] = 0;
	S[3][1] = 0; 
	S[3][2] = 0; 
	S[3][3] = scale;
	Multiply(IM,S);
	_scale *= scale;
}

//--------------------------------------------------------------------------
// Multiply()
//--------------------------------------------------------------------------

void CMatrix::Multiply(double A[4][4], double B[4][4]) 
{
	double T[4][4];
	
	T[1][1] = A[1][1]*B[1][1]+A[1][2]*B[2][1]+A[1][3]*B[3][1];
	T[2][1] = A[2][1]*B[1][1]+A[2][2]*B[2][1]+A[2][3]*B[3][1];
	T[3][1] = A[3][1]*B[1][1]+A[3][2]*B[2][1]+A[3][3]*B[3][1];
	
	T[1][2] = A[1][1]*B[1][2]+A[1][2]*B[2][2]+A[1][3]*B[3][2];
	T[2][2] = A[2][1]*B[1][2]+A[2][2]*B[2][2]+A[2][3]*B[3][2];
	T[3][2] = A[3][1]*B[1][2]+A[3][2]*B[2][2]+A[3][3]*B[3][2];
	
	T[1][3] = A[1][1]*B[1][3]+A[1][2]*B[2][3]+A[1][3]*B[3][3];
	T[2][3] = A[2][1]*B[1][3]+A[2][2]*B[2][3]+A[2][3]*B[3][3];
	T[3][3] = A[3][1]*B[1][3]+A[3][2]*B[2][3]+A[3][3]*B[3][3];
	
	A[1][1] = T[1][1];
	A[1][2] = T[1][2];
	A[1][3] = T[1][3];
	A[2][1] = T[2][1];
	A[2][2] = T[2][2];
	A[2][3] = T[2][3];
	A[3][1] = T[3][1];
	A[3][2] = T[3][2];
	A[3][3] = T[3][3];
}

//--------------------------------------------------------------------------
// Calculate()
//--------------------------------------------------------------------------

void CMatrix::Calculate(double movex, double movey, double movez) 
{
	double divisor;
	
	divisor  = IM[1][1]*(IM[3][2]*IM[2][3]-IM[2][2]*IM[3][3])
				+ IM[2][1]*(IM[1][2]*IM[3][3]-IM[3][2]*IM[1][3])
				+ IM[3][1]*(IM[2][2]*IM[1][3]-IM[1][2]*IM[2][3]);
	if (divisor != 0) {
		M[1][1] = (IM[3][2]*IM[2][3]-IM[2][2]*IM[3][3])/divisor;
		M[1][2] = (IM[1][2]*IM[3][3]-IM[3][2]*IM[1][3])/divisor;
		M[1][3] = (IM[2][2]*IM[1][3]-IM[1][2]*IM[2][3])/divisor;
		M[2][1] = (IM[2][1]*IM[3][3]-IM[3][1]*IM[2][3])/divisor;
		M[2][2] = (IM[3][1]*IM[1][3]-IM[1][1]*IM[3][3])/divisor;
		M[2][3] = (IM[1][1]*IM[2][3]-IM[2][1]*IM[1][3])/divisor;
		M[3][1] = (IM[3][1]*IM[2][2]-IM[2][1]*IM[3][2])/divisor;
		M[3][2] = (IM[1][1]*IM[3][2]-IM[3][1]*IM[1][2])/divisor;
		M[3][3] = (IM[2][1]*IM[1][2]-IM[1][1]*IM[2][2])/divisor;
	}
	M[1][0] = movex;
	M[2][0] = movey;
	M[3][0] = movez;
}

//--------------------------------------------------------------------------
// CalculateInverse()
//--------------------------------------------------------------------------

void CMatrix::CalculateInverse(double movex, double movey, double movez) 
{
	double divisor;
	divisor  = M[1][1]*(M[3][2]*M[2][3]-M[2][2]*M[3][3])
				+ M[2][1]*(M[1][2]*M[3][3]-M[3][2]*M[1][3])
				+ M[3][1]*(M[2][2]*M[1][3]-M[1][2]*M[2][3]);
	if (divisor != 0) {
		IM[1][1] = (M[3][2]*M[2][3]-M[2][2]*M[3][3])/divisor;
		IM[1][2] = (M[1][2]*M[3][3]-M[3][2]*M[1][3])/divisor;
		IM[1][3] = (M[2][2]*M[1][3]-M[1][2]*M[2][3])/divisor;
		IM[2][1] = (M[2][1]*M[3][3]-M[3][1]*M[2][3])/divisor;
		IM[2][2] = (M[3][1]*M[1][3]-M[1][1]*M[3][3])/divisor;
		IM[2][3] = (M[1][1]*M[2][3]-M[2][1]*M[1][3])/divisor;
		IM[3][1] = (M[3][1]*M[2][2]-M[2][1]*M[3][2])/divisor;
		IM[3][2] = (M[1][1]*M[3][2]-M[3][1]*M[1][2])/divisor;
		IM[3][3] = (M[2][1]*M[1][2]-M[1][1]*M[2][2])/divisor;
	}
	IM[1][0] = movex*IM[1][1] + movey*IM[1][2]+ movez*IM[1][3];
	IM[2][0] = movex*IM[2][1] + movey*IM[2][2]+ movez*IM[2][3];
	IM[3][0] = movex*IM[3][1] + movey*IM[3][2]+ movez*IM[3][3];
}

//--------------------------------------------------------------------------
// SetPoint()
//--------------------------------------------------------------------------

void CMatrix::SetPoint(vms_vector *vert, APOINT *apoint) 
{
	double x,y,z,x1,y1,z1,x2,y2,z2,x3,y3;
	
	// translation
	x = M[1][0] + ((double) (vert->x)) / 65536.0;
	y = M[2][0] + ((double) (vert->y)) / 65536.0;
	z = M[3][0] + ((double) (vert->z)) / 65536.0;
	
	// rotation
	x1 = M[1][1] * x + M[1][2] * y + M[1][3] * z;
	y1 = M[2][1] * x + M[2][2] * y + M[2][3] * z;
	z1 = M[3][1] * x + M[3][2] * y + M[3][3] * z;
	
	// scaling
	x2 = x1*5;
	y2 = y1*5;
	z2 = z1;
	
	// unrotate TEST ONLY
	/***
	x = scrn[n].x;
	y = scrn[n].y;
	z = scrn[n].z;
	scrn[n].x = IM[1][1]*x + IM[1][2]*y + IM[1][3]*z;
	scrn[n].y = IM[2][1]*x + IM[2][2]*y + IM[2][3]*z;
	scrn[n].z = IM[3][1]*x + IM[3][2]*y + IM[3][3]*z;
	***/
	
	// add perspective
	if ((_depthPerception < 10000) && (z2 > -_depthPerception)) {
		x3 = (x2 * (double) _depthPerception) / (z2 + (double) _depthPerception);
		y3 = (y2 * (double) _depthPerception) / (z2 + (double) _depthPerception);
		} 
	else {
		x3 = x2; //10000;
		y3 = y2; //10000;
	}
	
	/* adjust for screen */
//	x3      *= aspect_ratio;
	apoint->x  = (INT16) ((long) (x3 + _viewWidth) % 32767);
	apoint->y  = (INT16) ((long) (_viewHeight - y3) % 32767);
	apoint->z  = (INT16) z2;
}

//--------------------------------------------------------------------------
//			     unset_point()
//--------------------------------------------------------------------------

void CMatrix::UnsetPoint(vms_vector *vert,APOINT *apoint) 
{
  double x,y,z,x1,y1,z1,x2,y2,z2,x3,y3;

  // unadjust for screen
  z2 = apoint->z;
  y3 = y_center - apoint->y;
  x3 = apoint->x - x_center;
  //x3 /= aspect_ratio;

  // remove perspective
  x2 = (x3 * (z2 + depth_perception)) / depth_perception;
  y2 = (y3 * (z2 + depth_perception)) / depth_perception;
  // unscale
  x1 = x2/5;
  y1 = y2/5;
  z1 = z2;

  // unrotate
  x = IM[1][1]*x1 + IM[1][2]*y1 + IM[1][3]*z1;
  y = IM[2][1]*x1 + IM[2][2]*y1 + IM[2][3]*z1;
  z = IM[3][1]*x1 + IM[3][2]*y1 + IM[3][3]*z1;

  // untranslate
  vert->x = (long) ((x - M[1][0]) * 0x10000L);
  vert->y = (long) ((y - M[2][0]) * 0x10000L);
  vert->z = (long) ((z - M[3][0]) * 0x10000L);
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
int CMatrix::CheckNormal(CDObject *obj, vms_vector *a, vms_vector *b) {
  double ax,ay,az;
  double bx,by,bz;
  double x,y,z;
  double z1,z2;

  // rotate point using Objects () rotation matrix
  ax =    (double)obj->orient.rvec.x * (double)a->x
		+ (double)obj->orient.uvec.x * (double)a->y
		+ (double)obj->orient.fvec.x * (double)a->z;
  ay =    (double)obj->orient.rvec.y * (double)a->x
		+ (double)obj->orient.uvec.y * (double)a->y
		+ (double)obj->orient.fvec.y * (double)a->z;
  az =    (double)obj->orient.rvec.z * (double)a->x
		+ (double)obj->orient.uvec.z * (double)a->y
		+ (double)obj->orient.fvec.z * (double)a->z;
  bx =    (double)obj->orient.rvec.x * (double)b->x
		+ (double)obj->orient.uvec.x * (double)b->y
		+ (double)obj->orient.fvec.x * (double)b->z;
  by =    (double)obj->orient.rvec.y * (double)b->x
		+ (double)obj->orient.uvec.y * (double)b->y
		+ (double)obj->orient.fvec.y * (double)b->z;
  bz =    (double)obj->orient.rvec.z * (double)b->x
		+ (double)obj->orient.uvec.z * (double)b->y
		+ (double)obj->orient.fvec.z * (double)b->z;

  // set point to be in world coordinates
  ax += obj->pos.x;
  ay += obj->pos.y;
  az += obj->pos.z;
  bx += ax;
  by += ay;
  bz += az;

  x  = M[1][0] + ax / F1_0;
  y  = M[2][0] + ay / F1_0;
  z  = M[3][0] + az / F1_0;
  z1 = M[3][1]*x + M[3][2]*y + M[3][3]*z;
  x  = M[1][0] + bx / F1_0;
  y  = M[2][0] + by / F1_0;
  z  = M[3][0] + bz / F1_0;
  z2 = M[3][1]*x + M[3][2]*y + M[3][3]*z;

  if (z1 > z2) return 1;
  else         return 0;
}
