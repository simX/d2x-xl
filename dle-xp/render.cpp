// render.cpp

#include "stdafx.h"
#include <math.h>

#include "types.h"
#include "matrix.h"
#include "global.h"
#include "mine.h"
#include "segment.h"
#include "dlc.h"
#include "dlcdoc.h"
#include "mainfrm.h"
#include "mineview.h"

extern UINT8 side_vert[6][4];
int enable_delta_shading = 0;

//-----------------------------------------------------------------
//-----------------------------------------------------------------

double dround_off(double value, double round) 
{
return (value >= 0) ? value + round / 2 : value - round / 2;
}

//------------------------------------------------------------------------
// multiply_matrix()
//------------------------------------------------------------------------
void multiply_matrix(double C[3][3], double A[3][3], double B[3][3]) {
  int i,j;

  for (i=0;i<3;i++) {
    for (j=0;j<3;j++)	{
      C[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j];
    }
  }
}

//------------------------------------------------------------------------
// scale_matrix()
//------------------------------------------------------------------------
void scale_matrix(double A[3][3], double scale) {
  int i,j;
  double B[3][3]; // temporary copy of A
  double C[3][3]; // scale matrix

  // copy matrix and clear C[][] while were at it
  for (i=0;i<3;i++) {
    for (j=0;j<3;j++)	{
      B[i][j] = A[i][j];
      C[i][j] = 0;
    }
  }
  // set scale matrix
  C[0][0] = scale;
  C[1][1] = scale;
  C[2][2] = 1.0;

  // multiply A=B*C
  multiply_matrix(A,B,C);
}

//------------------------------------------------------------------------
// adjoint_matrix()
//------------------------------------------------------------------------
void adjoint_matrix(double A[3][3],double B[3][3]) {
    B[0][0] = A[1][1]*A[2][2] - A[1][2]*A[2][1];
    B[0][1] = A[0][2]*A[2][1] - A[0][1]*A[2][2];
    B[0][2] = A[0][1]*A[1][2] - A[0][2]*A[1][1];
    B[1][0] = A[1][2]*A[2][0] - A[1][0]*A[2][2];
    B[1][1] = A[0][0]*A[2][2] - A[0][2]*A[2][0];
    B[1][2] = A[0][2]*A[1][0] - A[0][0]*A[1][2];
    B[2][0] = A[1][0]*A[2][1] - A[1][1]*A[2][0];
    B[2][1] = A[0][1]*A[2][0] - A[0][0]*A[2][1];
    B[2][2] = A[0][0]*A[1][1] - A[0][1]*A[1][0];
}

//------------------------------------------------------------------------
// define_square2quad_matrix()
//------------------------------------------------------------------------
void square2quad_matrix(double A[3][3],POINT a[4]) {
    double dx1,dx2,dx3,dy1,dy2,dy3; // temporary storage variables
    double w;

    // infer "unity square" to "quad" prespective transformation
    // see page 55-56 of Digital Image Warping by George Wolberg (3rd edition) 
    dx1 = a[1].x - a[2].x;
    dx2 = a[3].x - a[2].x;
    dx3 = a[0].x - a[1].x + a[2].x - a[3].x;
    dy1 = a[1].y - a[2].y;
    dy2 = a[3].y - a[2].y;
    dy3 = a[0].y - a[1].y + a[2].y - a[3].y;
    w = (dx1*dy2 - dx2*dy1);
    if (w==0) w=1;
    A[0][2] = (dx3*dy2 - dx2*dy3) / w;
    A[1][2] = (dx1*dy3 - dx3*dy1) / w;
    A[0][0] = a[1].x - a[0].x + A[0][2]*a[1].x;
    A[1][0] = a[3].x - a[0].x + A[1][2]*a[3].x;
    A[2][0] = a[0].x;
    A[0][1] = a[1].y - a[0].y + A[0][2]*a[1].y;
    A[1][1] = a[3].y - a[0].y + A[1][2]*a[3].y;
    A[2][1] = a[0].y;
    A[2][2] = 1;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
#if 0

void CalcCenter(CMine *mine, vms_vector &center,INT16 segnum,INT16 sidenum) {
  int i;
  vms_vector	*v;
  // calculate center of current side
  center.x = center.y = center.z = 0;
  for (i=0;i<4;i++) {
		v = mine->vertices + mine->Segments (segnum)->verts[side_vert[sidenum][i]];
    center.x += v->x>>2;
    center.y += v->y>>2;
    center.z += v->z>>2;
  }
//  center.x /= 4;
//  center.y /= 4;
//  center.z /= 4;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CalcOrthoVector(CMine &mine, vms_vector &result,INT16 segnum,INT16 sidenum) {
  struct dvector a,b,c;
  double length;
  INT16 vertnum1,vertnum2;
  CDSegment	*Segments () = mine->Segments ();
  vms_vector	*vertices = mine->vertices;
    // calculate orthogonal vector from lines which intersect point 0
    //
    //       |x  y  z |
    // AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
    //       |bx by bz|

    vertnum1 = Segments (segnum)->verts[side_vert[sidenum][0]];
    vertnum2 = Segments (segnum)->verts[side_vert[sidenum][1]];
    a.x = (double)(vertices[vertnum2].x - vertices[vertnum1].x);
    a.y = (double)(vertices[vertnum2].y - vertices[vertnum1].y);
    a.z = (double)(vertices[vertnum2].z - vertices[vertnum1].z);
    vertnum1 = Segments (segnum)->verts[side_vert[sidenum][0]];
    vertnum2 = Segments (segnum)->verts[side_vert[sidenum][3]];
    b.x = (double)(vertices[vertnum2].x - vertices[vertnum1].x);
    b.y = (double)(vertices[vertnum2].y - vertices[vertnum1].y);
    b.z = (double)(vertices[vertnum2].z - vertices[vertnum1].z);

    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;

    // normalize the vector
    length = my_sqrt(c.x*c.x + c.y*c.y + c.z*c.z);
    if (length>0) {
      c.x /= length;
      c.y /= length;
      c.z /= length;
    }

    result.x = dround_off(-c.x * 0x10000L,1.0);
    result.y = dround_off(-c.y * 0x10000L,1.0);
    result.z = dround_off(-c.z * 0x10000L,1.0);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
double CalcLength (vms_vector &center1,vms_vector &center2) 
{
  vms_vector direction;

  // calculate distance vector between the centers
  direction.x = center1.x - center2.x;
  direction.y = center1.y - center2.y;
  direction.z = center1.z - center2.z;

  // calculate the length of the new cube
  return (my_sqrt((double)direction.x*(double)direction.x
          + (double)direction.y*(double)direction.y
	       + (double)direction.z*(double)direction.z));
}
#endif //0
//------------------------------------------------------------------------
// TextureMap()
//------------------------------------------------------------------------
void TextureMap(int resolution,
				CDSegment *segment,
				INT16 sidenum,
				UINT8 *bmData,
				UINT16 bmWidth,
				UINT16 bmHeight,
				UINT8 *light_index,
				UINT8 *pScrnMem,
				APOINT* scrn,
				UINT16 width,
				UINT16 height,
				UINT16 rowOffset)
{
	
	int h,i,j;
	int x,y;
	LONG yi, yj;
	POINT a[4];
	POINT minpt,maxpt;
	double A[3][3],IA[3][3],B[3][3],UV[3][3]; // transformation matrices
	double w;
	UINT8 *ptr;
	uvl *uvls;
	CMine *mine = theApp.GetMine ();
	bool bD2XLights = (level_version >= 15) && (mine->GameInfo ().fileinfo_version >= 34);
	
	
	// TEMPORARY
	int inc_resolution = 1<<resolution;
	INT16 segnum = segment - mine->Segments ();
	INT16 flick_light = mine->GetFlickeringLight (segnum, sidenum);
	INT16 dscan_light,scan_light;
	INT16 light[4];
	UINT16 bmWidth2;
	bool enable_shading = (light_index != NULL);

bmHeight = bmWidth;
bmWidth2 = bmWidth / 2;

// define 4 corners of texture to be displayed on the screen
for (i=0;i<4;i++) {
	INT16 vertnum;
	vertnum = segment->verts[side_vert[sidenum][i]];
	a[i].x = scrn[vertnum].x;
	a[i].y = scrn[vertnum].y;
	}
	
	// determin min/max points
minpt.x = minpt.y = 10000; // some number > any screen resolution
maxpt.x = maxpt.y = 0;
for (i=0;i<4;i++) {
	minpt.x = min(minpt.x,a[i].x);
	maxpt.x = max(maxpt.x,a[i].x);
	minpt.y = min(minpt.y,a[i].y);
	maxpt.y = max(maxpt.y,a[i].y);
	}

	// clip min/max with screen min/max
minpt.x = max(minpt.x, 0);
maxpt.x = min(maxpt.x, width);
minpt.y = max(minpt.y, 0);
maxpt.y = min(maxpt.y, height);

// fill in texture
POINT b[4];  // Descent's (u,v) coordinates for textures

// map unit square into texture coordinate
square2quad_matrix(A,a);

// calculate adjoint matrix (same as inverse)
adjoint_matrix(A,IA);

// store uv coordinates into b[]
uvls = segment->sides[sidenum].uvls;
for (i=0;i<4;i++) {
	b[i].x = uvls[i].u;
	b[i].y = uvls[i].v;
	}
	
	// define texture light
for (i=0;i<4;i++) {
	light[i] = uvls[i].l;
	// clip light
	if (light[i] & 0x8000){
		light[i] = 0x7fff;
		}
	}
	// reduce texture light if current side is on a delta light
	// first make sure we have allocated space for delta lights
if (enable_delta_shading) {
	dl_index *dl_indices;
	int dlIdxCount = mine->GameInfo ().dl_indices.count;
	delta_light  *delta_lights;
	if (!light_status [segnum][sidenum].bIsOn &&
		 (dl_indices = mine->DLIndex ()) &&
		 (delta_lights = mine->DeltaLights ())) {
		// search delta light index to see if current side has a light
		dl_index	*dli = dl_indices;
		for (i=0;i<dlIdxCount;i++,dli ++) {
//				if (dli->segnum == mine->current->segment) {
			// loop on each delta light till the segment/side is found
				delta_light *dl = delta_lights + dli->d2.index;
				h = bD2XLights ? dli->d2x.count : dli->d2.count;
				for (j = 0; j < h; j++, dl++) {
					if (dl->segnum==segnum && dl->sidenum==sidenum) {
						for (int k=0;k<4;k++) {
							INT16 dlight = dl->vert_light[k];
							if (dlight >= 0x20)
								dlight = 0x7fff;
							else
								dlight <<= 10;
							if (light[k] > dlight)
								light[k] -= dlight;
							else
								light[k] = 0;
							}
						}
//					}
				}
			}
		}
	}
	// map unit square into uv coordinates
	// uv of 0x800 = 64 pixels in texture
	// therefore uv of 32 = 1 pixel
square2quad_matrix(UV,b);
scale_matrix(UV,1.0/2048.0);
multiply_matrix(B,IA,UV);
for (y=minpt.y;y<maxpt.y;y+=inc_resolution) {
	int x0,x1;
	// Determine min and max x for this y.
	// Check each of the four lines of the quadrilaterial
	// to figure out the min and max x
	x0 = maxpt.x; // start out w/ min point all the way to the right
	x1 = minpt.x; // and max point to the left
	for (i=0;i<4;i++) {
		// if line intersects this y then update x0 & x1
		j = (i+1)&3; // j = other point of line
		yi = a[i].y;
		yj = a[j].y;
		if ((y >= yi && y <= yj) || (y >= yj && y <= yi)) {
			w = yi - yj;
			if (w != 0) { // avoid divide by zero
				x = (int)((
					(double)a[i].x * ((double)y - (double)yj) -
					(double)a[j].x * ((double)y - (double)yi)
					) / w);
				if (x<x0) {
					scan_light = (int)((
						(double)light[i] * ((double)y - (double)yj) -
						(double)light[j] * ((double)y - (double)yi)
						) / w);
					x0 = x;
				}
				if (x>x1) {
					dscan_light = (int)((
						(double)light[i] * ((double)y - (double)yj) -
						(double)light[j] * ((double)y - (double)yi)
						) / w);
					x1 = x;
				}
			}
		}
	} // end for
	
	// clip
	x0 = max(x0,minpt.x);
	x1 = min(x1,maxpt.x);
	
	// Instead of finding every point using the matrix transformation,
	// just define the end points and delta values then simply
	// add the delta values to u and v
	if (fabs((double) (x0 - x1)) >= 1) {
		double u0,u1,v0,v1,w0,w1, h, scale, x0d, x1d;
		UINT32 u,v,du,dv,m,vd,vm,dx;
		dscan_light = (dscan_light - scan_light)/(x1-x0);
		dscan_light <<= resolution;
		
		// loop for every 32 bytes
		int end_x = x1;
		for (;x0 < end_x ;x0 += bmWidth2) {
			if (end_x - x0 > bmWidth2)
				x1 = bmWidth2 + x0;
			else
				x1 = end_x;

			scale = (double) max (bmWidth, bmHeight);
			h = B[1][2]*(double)y + B[2][2];
			x0d = (double)x0;
			x1d = (double)x1;
			w0 = (B[0][2]*x0d + h) / scale; // scale factor (64 pixels = 1.0 unit)
			w1 = (B[0][2]*x1d + h) / scale;
			if (fabs(w0)>0.0001 && fabs(w1)>0.0001) {
				h = B[1][0]*(double)y + B[2][0];
				u0 = (B[0][0]*x0d + h) / w0;
				u1 = (B[0][0]*x1d + h) / w1;
				h = B[1][1]*(double)y + B[2][1];
				v0 = (B[0][1]*x0d + h) / w0;
				v1 = (B[0][1]*x1d + h) / w1;
				
				// use 22.10 integer math
				// the 22 allows for large texture bitmap sizes
				// the 10 gives more then enough accuracy for the delta values

				m = min (bmWidth, bmHeight);
				if (!m)
					m = 64;
				m *= 1024;
				dx = x1-x0;
				if (!dx)
					dx = 1;
				du = ((UINT32) (((u1 - u0)*1024.0)/dx) % m);
				du <<= resolution;
				// v0 & v1 swapped since bmData is flipped
				dv = ((UINT32) (((v0 - v1)*1024.0)/dx) % m);
				dv <<= resolution;
				u = ((UINT32) (u0 * 1024.0)) % m;
				v = ((UINT32) (-v0 * 1024.0)) % m;
				vd = 1024 / bmHeight;
				vm = bmWidth * (bmHeight - 1);
				
				if (resolution) 
					x0 &= ~1; // round down if low res
				ptr = pScrnMem + (UINT32)(height-y-1) * (UINT32)rowOffset + x0;
				
				int k = (x1-x0)>>resolution;
				if (y<(height-1) && k>0)  {
					UINT8 *pixelP;
					pixelP = ptr;
					if (resolution == 0) {
						if (enable_shading) {
#if 0
							// TEXTURE_MAP_LIGHT_HIRES
							UINT8 temp1 = descent_side_colors[sidenum];
							while (k--) {
								*pixelP++ = light_index[temp1 + ((scan_light / 4) & 0x1f00)];
								scan_light += dscan_light;
								}
#else
							do {
								UINT8 temp;
								u+=du;
								u%=m;
								v+=dv;
								v%=m;
								temp = bmData [(u / 1024) + ((v / vd) & vm)];
								if (temp<254) {
									temp = light_index[temp + ((scan_light / 4) & 0x1f00)];
									*pixelP = temp;
									}
								pixelP++;
								scan_light += dscan_light;
								} while (--k);
#endif
							} 
						else {
							// TEXTURE_MAP_NOLIGHT_HIRES
#if 0
							UINT8 temp = descent_side_colors[sidenum];
							while (k--) {
								*pixelP++ = temp;
								}
#else
							do {
								UINT8 temp;
								u+=du;
								u%=m;
								v+=dv;
								v%=m;
								temp = bmData [(u / 1024) + ((v / vd) & vm)];
								if (temp<254) {
									*pixelP = temp;
								}
								pixelP++;
								} while (--k);
#endif
							}
						} 
					else {
						// if doing a splash, define multiple points as the same color
						if (enable_shading) {
							// TEXTURE_MAP_LIGHT_LOWRES
#if 0
							UINT8 temp1 = descent_side_colors[sidenum];
							while (k--) {
								pixelP[0] = 
								pixelP[1] = light_index[temp1 + ((scan_light / 4) & 0x1f00)];
								pixelP += 2;
								scan_light += dscan_light;
								}
#else
							do {
								UINT8 temp;
								u+=du;
								u%=m;
								v+=dv;
								v%=m;
								temp = bmData [(u / 1024) + ((v / vd) & vm)];
								if (temp<254) {
									temp = light_index[temp + ((scan_light / 4) & 0x1f00)];
									pixelP[0] = 
									pixelP[1] = temp;
									}
								pixelP += 2;
								scan_light += dscan_light;
								} while (--k);
#endif
							} 
						else {
							// TEXTURE_MAP_NOLIGHT_LOWRES
#if 0
							UINT8 emp = descent_side_colors[sidenum];
							while (k--) {
								pixelP[0] = 
								pixelP[1] = temp;
								pixelP += 2;
								}
#else
							do {
								UINT8 temp;
								u+=du;
								v+=dv;
								u%=m;
								v%=m;
								temp = bmData [(u / 1024) + ((v / vd) & vm)];
								if (temp<254) {
									pixelP[0] =
									pixelP[1] = temp;
									}
								pixelP += 2;
								} while (--k);
#endif
							}
						}
					}
				}
			} // end of 32 byte loop
		}
	}
}
