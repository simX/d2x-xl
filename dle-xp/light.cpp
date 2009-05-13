// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "light.h"
#include "global.h"

// external globals
extern int enable_shading; // uvls.cpp
extern int enable_hires;   // uvls.cpp
extern int enable_delta_shading; // uvls.cpp

long lightMap [MAX_D2_TEXTURES];
long defLightMap [MAX_D2_TEXTURES];

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

int FindLight (int nTexture, TEXTURE_LIGHT *pTexLights, int nLights)
{
	int	l = 0;
	int	r = nLights - 1;
	int	m, t;

while (l <= r) {
	m = (l + r) / 2;
	t = pTexLights [m].nBaseTex;
	if (nTexture > t)
		l = m + 1;
	else if (nTexture < t)
		r = m - 1;
	else
		return m;
	}
return -1;
}

//--------------------------------------------------------------------------

void CreateLightMap (void)
{
#if 1
if (theApp.GetMine ())
	theApp.GetMine ()->LoadDefaultLightAndColor ();
#else
	TEXTURE_LIGHT	*pTexLights = (file_type == RDL_FILE) ? d1_texture_light : d2_texture_light;
	int				i = ((file_type == RDL_FILE) ? sizeof (d1_texture_light) : sizeof (d2_texture_light)) / sizeof (TEXTURE_LIGHT);

memset (lightMap, 0, sizeof (lightMap));
while (i) {
	--i;
	CBRK (pTexLights [i].nBaseTex == 0);
	lightMap [pTexLights [i].nBaseTex] = ((pTexLights [i].light * 100 + MAX_BRIGHTNESS / 2) / MAX_BRIGHTNESS) * (MAX_BRIGHTNESS / 100);
	}
memcpy (defLightMap, lightMap, sizeof (defLightMap));
#endif
}

//--------------------------------------------------------------------------

#if 0

BOOL HasCustomLightMap (void)
{
#ifdef _DEBUG
	int	i;

i = memcmp (lightMap, defLightMap, sizeof (lightMap));
if (i != 0) {
	for (i = 0; i < MAX_D2_TEXTURES; i++)
		if (lightMap [i] != defLightMap [i])
			return TRUE;
	}
return FALSE;
#else
return memcmp (lightMap, defLightMap, sizeof (lightMap)) != 0;
#endif
}

#endif

//--------------------------------------------------------------------------

int ReadLightMap (FILE *fLightMap, UINT32 nSize)
{
return fread (lightMap, nSize, 1, fLightMap) != 1;
}

//--------------------------------------------------------------------------

int WriteLightMap (FILE *fLightMap)
{
return fwrite (lightMap, sizeof (lightMap), 1, fLightMap) != 1;
}

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

UINT8 CMine::LightWeight(INT16 nBaseTex) 
{
#if 1
return (UINT8) ((lightMap [nBaseTex] - 1) / 0x0200L);
#else
  int i;

#	if 1
if (file_type == RDL_FILE) {
	i = FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1);
	if (i >= 0)
		return (UINT8) ((d1_texture_light [i].light - 1) / 0x0200L);
	}
else {
	i = FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
	if (i >= 0)
		return (UINT8) ((d2_texture_light [i].light - 1) / 0x0200L);
	}
return 0;
#	else
	UINT8 result;

if (nBaseTex >= 0 && nBaseTex < MAX_TEXTURES) {
	if (file_type == RDL_FILE) {
		for (i=0;i<NUM_LIGHTS_D1;i++)
			if (nBaseTex <= d1_texture_light[i].nBaseTex) 
				break;
		if (nBaseTex == d1_texture_light[i].nBaseTex)
			result = (UINT8)((d1_texture_light[i].light - 1) / 0x0200L);
		}
	else {
		for (i=0;i<NUM_LIGHTS_D2;i++)
			if (nBaseTex <= d2_texture_light[i].nBaseTex) 
				break;
		if (nBaseTex == d2_texture_light[i].nBaseTex)
			result = (UINT8)((d2_texture_light[i].light - 1) / 0x0200L);
		}
	}
return (result);
#	endif
#endif
}

//------------------------------------------------------------------------
// GetFlickeringLight() - returns flicker light number
//
// returns -1 if not exists
//------------------------------------------------------------------------

INT16 CMine::GetFlickeringLight (INT16 segnum, INT16 sidenum) 
{
GetCurrent (segnum, sidenum);
FLICKERING_LIGHT *pfl = FlickeringLights ();
for (int i = FlickerLightCount (); i; i--, pfl++)
	if ((pfl->segnum == segnum) && (pfl->sidenum == sidenum))
		break;
if (i > 0)
	return FlickerLightCount () - i;
return -1;
}



bool CMine::IsFlickeringLight (INT16 segnum, INT16 sidenum)
{
return GetFlickeringLight (segnum, sidenum) >= 0;
}

//------------------------------------------------------------------------
// add_flickering_light()
//
// returns index to newly created flickering light
//------------------------------------------------------------------------

INT16 CMine::AddFlickeringLight (INT16 segnum, INT16 sidenum, UINT32 mask,FIX time) 
{
GetCurrent (segnum, sidenum);
if (GetFlickeringLight (segnum,sidenum) != -1) {
	if (!bExpertMode)
		ErrorMsg("There is already a flickering light on this side");
	return -1;
	}
// we are adding a new flickering light
if (FlickerLightCount () >= MAX_FLICKERING_LIGHTS) {
	if (!bExpertMode) {
		sprintf (
			message, 
			"Maximum number of flickering lights (%d) have already been added",
			MAX_FLICKERING_LIGHTS);
		ErrorMsg(message);
		}
	return -1;
	}
INT16 tmapnum = CurrSide ()->nBaseTex & 0x1fff;
INT16 tmapnum2 = CurrSide ()->nOvlTex & 0x1fff;
if ((IsLight (tmapnum) == -1) && (IsLight (tmapnum2) == -1)) {
	if (!bExpertMode)
		ErrorMsg("Blinking lights can only be added to a side\n"
					"that has a Texture with \" - light\" at the\n"
					"end of its name.");
	return -1;
	}
theApp.SetModified (TRUE);
FLICKERING_LIGHT *pfl = FlickeringLights (FlickerLightCount ());
pfl->segnum = segnum;
pfl->sidenum = sidenum;
pfl->delay = time;
pfl->timer = time;
pfl->mask = mask;
return ++FlickerLightCount ();
}

//------------------------------------------------------------------------
// delete_flickering_light()
//
// removes a flickering light if it exists
//------------------------------------------------------------------------

bool CMine::DeleteFlickeringLight(INT16 segnum, INT16 sidenum) 
{
if (segnum < 0)
	segnum = Current ()->segment;
if (sidenum < 0)
	sidenum = Current ()->side;
INT16 index = GetFlickeringLight (segnum, sidenum);
if (index == -1) {
//ErrorMsg("There is no flickering light on this side.");
	return false;
	}
theApp.SetModified (TRUE);
if (index < --FlickerLightCount ())
// put last light in place of deleted light
	memcpy (FlickeringLights (index), 
			  FlickeringLights (FlickerLightCount ()),
			  sizeof (FLICKERING_LIGHT));
return true;
}

//------------------------------------------------------------------------
// IsLight()
//
// searches for a light in the list of lights
// returns index if found, -1 if not in the list
//------------------------------------------------------------------------

int CMine::IsLight(int nBaseTex) 
{
#if 1
return (lightMap [nBaseTex & 0x3fff] > 0) ? 0 : -1;
#else
#	if 1
return (file_type == RDL_FILE) ?
	FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1) : 
	FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
#	else
	int retval = -1;
	int i;
	TEXTURE_LIGHT *t;
	if (file_type == RDL_FILE) {
		i = NUM_LIGHTS_D1;
		t = d1_texture_light;
		}
	else {
		i = NUM_LIGHTS_D2;
		t = d2_texture_light;
		}
	for (; i; i--, t++)
		if (nBaseTex <= t->nBaseTex)
			break;
	if (nBaseTex == t->nBaseTex)
		return t - ((file_type == RDL_FILE) ? d1_texture_light : d2_texture_light);
	return -1;
#	if 0
	for (i=0;i<NUM_LIGHTS_D1;i++) {
		if (nBaseTex <= d1_texture_light[i].nBaseTex) break;
	if (nBaseTex > 0) {
		if (file_type == RDL_FILE) {
			for (i=0;i<NUM_LIGHTS_D1;i++) {
				if (nBaseTex <= d1_texture_light[i].nBaseTex) break;
			}
			if (nBaseTex == d1_texture_light[i].nBaseTex) {
				retval = i;
			}
		} else {
			for (i=0;i<NUM_LIGHTS_D2;i++) {
				if (nBaseTex <= d2_texture_light[i].nBaseTex) break;
			}
			if (nBaseTex == d2_texture_light[i].nBaseTex) {
				retval = i;
			}
		}
	}
	return retval;
#		endif
#	endif
#endif
}

//------------------------------------------------------------------------
// is_exploding_light()
//
//291 ceil024-l   292 ceil024b-f
//293 ceil025-l   294 ceil025b-f
//296 ceil028-l   297 ceil028b-f
//298 ceil029-l   299 ceil029b-f
//------------------------------------------------------------------------

int CMine::IsExplodingLight(int nBaseTex) 
{
	switch (nBaseTex) {
	case 291:
	case 292:
	case 293:
	case 294:
	case 296:
	case 297:
	case 298:
	case 299:
		return (1);
	}
	return(0);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::IsBlastableLight (int nBaseTex) 
{
nBaseTex &= 0x3fff;
if (IsExplodingLight (nBaseTex))
	return true;
if (file_type == RDL_FILE)
	return false;
for (INT16 *p = d2_blastable_lights; *p >= 0; p++)
	if (*p == nBaseTex)
		return true;
#if 0
if (IsLight (nBaseTex))
	for (p = d2_switches; *p >= 0; p++)
		if (*p == nBaseTex)
			return true;
#endif
return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::VisibleWall (UINT16 nWall)
{
if (nWall == NO_WALL)
	return false;
CDWall	*wallP = Walls (nWall);
return (wallP->type != WALL_OPEN);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetCubeLight (double fLight, bool bAll, bool bDynCubeLights)
{
	long nLight = (long) (fLight * 65536); //24.0 * 327.68);
	CDSegment *seg;
	int	h, i, j, l, c, segnum;

theApp.SetModified (TRUE);
fLight /= 100.0;
for (segnum = SegCount (), seg = Segments (); segnum; segnum--, seg++) {
	if (bAll || (seg->wall_bitmask & MARKED_MASK)) {
		if (!bDynCubeLights)
			seg->static_light = nLight;
		else {
			l = 0;
			c = 0;
			for (j = 0; j < 6; j++) {
				for (i = 0; i < 4; i++) {
					h = (UINT16) seg->sides [j].uvls [i].l;
					if (h || ((seg->children [j] == -1) && !VisibleWall (seg->sides [j].nWall))) {
						l += h;
						c++;
						}
					}
				}
			seg->static_light = (FIX) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::ScaleCornerLight (double fLight, bool bAll) 
{
	int segnum,j,i;
	double scale;
	CDSegment *seg;

theApp.SetModified (TRUE);
scale = fLight / 100.0; // 100.0% = normal
for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++)
	if (bAll || (seg->wall_bitmask & MARKED_MASK))
		for (j = 0; j < 6; j++)
			for (i = 0; i < 4; i++) {
				fLight = ((double) ((UINT16) seg->sides [j].uvls [i].l)) * scale;
				fLight = min (fLight, 0x8000);
				fLight = max (fLight, 0);
				Segments (segnum)->sides[j].uvls[i].l = (UINT16) fLight;
				}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::CalcAverageCornerLight (bool bAll)
{
  int segnum,pt,i,vertnum,count,sidenum,uvnum;
  UINT16 max_brightness;

// smooth corner light by averaging all corners which share a vertex
theApp.SetModified (TRUE);
for (vertnum = 0; vertnum < VertCount (); vertnum++) {
	if (bAll || (*VertStatus (vertnum) & MARKED_MASK)) {
		max_brightness = 0;
		count = 0;
		// find all Segments () which share this point
		CDSegment *seg = Segments ();
		for (segnum = 0; segnum < SegCount (); segnum++, seg++) {
			for (pt = 0; pt < 8; pt++) {
				if (seg->verts[pt] == vertnum) {
					// add all the uvls for this point
					for (i = 0; i < 3; i++) {
						sidenum = point_sides[pt][i];
						uvnum = point_corners[pt][i];
						if ((seg->children[sidenum] < 0) || 
							 (seg->sides[sidenum].nWall < GameInfo ().walls.count)) {
							max_brightness = max(max_brightness,(UINT16)seg->sides[sidenum].uvls[uvnum].l);
							count++;
							}
						}
					}
				}
			}
		// now go back and set these light values
		if (count > 0) {
			theApp.SetModified (TRUE);
			//	max_brightness = min(max_brightness,0x8000L);
			CDSegment *seg = Segments ();
			for (segnum=0;segnum<SegCount ();segnum++, seg++) {
				for (pt=0;pt<8;pt++) {
					if (seg->verts[pt] == vertnum) {
						for (i=0;i<3;i++) {
							sidenum = point_sides[pt][i];
							uvnum = point_corners[pt][i];
							if ((seg->children[sidenum] < 0) || 
								 (seg->sides[sidenum].nWall < GameInfo ().walls.count)) {
								seg->sides[sidenum].uvls[uvnum].l = max_brightness;
								}
							}
						}
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::AutoAdjustLight (double fLightScale, bool bAll, bool bCopyTexLights) 
{
	int			segnum;
	int			texture_num;
	int			sidenum;
	UINT32		brightness;
	CDSegment	*seg;
	CDSide		*side;

// clear all lighting on marked cubes
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	memset (VertexColors (), 0, sizeof (MineData ().vertexColors));
for (segnum = SegCount (), seg = Segments (); segnum; segnum--, seg++)
	if (bAll || (seg->wall_bitmask & MARKED_MASK))
		for (sidenum=0, side = seg->sides;sidenum < MAX_SIDES_PER_SEGMENT; sidenum++, side++) {
			for (int i = 0; i < 4; i++) {
				side->uvls [i].l = 0;
				if (!bAll)
					memset (VertexColors (seg->verts [side_vert [sidenum][i]]), 0, sizeof (CDColor));
				}
			}

// Calculate cube side corner light values
// for each marked side in the level
// (range: 0 = min, 0x8000 = max)
for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++) {
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
		if (!(bAll || SideIsMarked (segnum, sidenum)))
			continue;
		if ((seg->children [sidenum] >= 0) && !VisibleWall (side->nWall))
			continue;
		if (bCopyTexLights)
			memset (LightColor (segnum, sidenum, false), 0, sizeof (CDColor));
		brightness = 0;
		texture_num = side->nBaseTex;
		if ((texture_num >= 0) && (texture_num < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (texture_num));
		texture_num = side->nOvlTex & 0x3fff;
		if ((texture_num > 0) && (texture_num < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (texture_num));
		if (brightness > 0)
			Illuminate (segnum, sidenum, (UINT32) (brightness * 2 * fLightScale), 1.0, bAll, bCopyTexLights);
		}
	}
theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// set_illumination()
//--------------------------------------------------------------------------
// blend two colors depending on each colors brightness
// psc: source color (light source)
// pdc: destination color (vertex/side color)
// srcBr: source brightness (remaining brightness at current vertex/side)
// destBr: vertex/side brightness

void CMine::BlendColors (CDColor *psc, CDColor *pdc, double srcBr, double destBr)
{
#if 0
if (psc->index == 255) {
	if (pdc->index == 0)
		*pdc = *psc;
	return;
	}
else if ((pdc->index == 0) || (pdc->index == 255)) {
	*pdc = *psc;
	return;
	}
else
	return;
#endif
if (destBr)
	destBr /= 65536.0;
else {
	if (srcBr)
		*pdc = *psc;
	return;
	}
if (srcBr)
	srcBr /= 65536.0;
else
	return;
if (pdc->index) {
	pdc->color.r += psc->color.r * srcBr;
	pdc->color.g += psc->color.g * srcBr;
	pdc->color.b += psc->color.b * srcBr;
	double cMax = pdc->color.r;
		if (cMax < pdc->color.g)
			cMax = pdc->color.g;
		if (cMax < pdc->color. b)
			cMax = pdc->color. b;
		if (cMax > 1) {
			pdc->color.r /= cMax;
			pdc->color.g /= cMax;
			pdc->color.b /= cMax;
			}
	}
else {
	if (destBr) {
		pdc->index = 1;
		pdc->color.r = psc->color.r * destBr;
		pdc->color.g = psc->color.g * destBr;
		pdc->color.b = psc->color.b * destBr;
		}
	else
		*pdc = *psc;
	}
}

//--------------------------------------------------------------------------

#ifdef _DEBUG
static int qqq1 = -1, qqq2 = 0;
#endif

void CMine::Illuminate (
	INT16 source_segnum, 
	INT16 source_sidenum, 
	UINT32 brightness, 
	double fLightScale, 
	bool bAll, bool 
	bCopyTexLights) 
{
CDSegment	*seg = Segments ();
CDSegment	*child_seg;
double		effect[4];
// find orthogonal angle of source segment
vms_vector A;

//fLightScale /= 100.0;
CalcOrthoVector (A,source_segnum,source_sidenum);
// remember to flip the sign since we want it to point inward
A.x = -A.x;
A.y = -A.y;
A.z = -A.z;

// calculate the center of the source segment
vms_vector source_center;
CalcCenter (source_center,source_segnum,source_sidenum);
if ((source_segnum == 911) && (source_sidenum == 3))
	A = A;
// mark those Segments () within N children of current cube

// set child numbers
//Segments ()[source_segnum].seg_number = m_lightRenderDepth;
for (int i = SegCount (); i; i--, seg++)
	seg->seg_number = -1;
SetSegmentChildNum (NULL, source_segnum, m_lightRenderDepth);
CDColor *plc = LightColor (source_segnum, source_sidenum);
if (!plc->index) {
	plc->index = 255;
	plc->color.r =
	plc->color.g =
	plc->color.b = 1.0;
	}
if (UseTexColors () && bCopyTexLights) {
	CDColor	*psc = LightColor (source_segnum, source_sidenum, false);
	*psc = *plc;
	}
seg = Segments (source_segnum);
seg->seg_number = m_lightRenderDepth;
bool bWall = false; //FindWall (source_segnum, source_sidenum) != NULL;
// loop on child Segments ()
int child_segnum;
for (child_segnum=0, child_seg = Segments ();child_segnum<SegCount ();child_segnum++, child_seg++) {
	// skip if this is not viewable
	if (child_seg->seg_number < 0) 
		continue;
	// skip if not marked
//	if (!(bAll || (child_seg->wall_bitmask & MARKED_MASK)))
//		continue;
	// setup source corner vertex for length calculation later
	vms_vector source_corner[4];
	for (int j = 0; j < 4; j++) {
		int vertnum = side_vert [source_sidenum][j];
		int h = seg->verts [vertnum];
		source_corner[j].x = Vertices (h)->x;
		source_corner[j].y = Vertices (h)->y;
		source_corner[j].z = Vertices (h)->z;
		}
	// loop on child sides
	for (int child_sidenum=0;child_sidenum<6;child_sidenum++) {
		// if side has a child..
		if (!(bAll || SideIsMarked (child_segnum, child_sidenum)))
			continue;
		if (child_seg->children[child_sidenum] >= 0) {
			UINT16 nWall = child_seg->sides[child_sidenum].nWall;
			// .. but there is no wall ..
			if (nWall >= GameInfo ().walls.count)
				continue;
				// .. or its not a door ..
			if (Walls (nWall)->type == WALL_OPEN)
				continue;
			}

//		CBRK (psc->index > 0);
		// if the child side is the same as the source side, then set light and continue
#ifdef _DEBUG
		CBRK (child_segnum == qqq1 && child_sidenum == qqq2);
#endif
		if (child_sidenum == source_sidenum && child_segnum == source_segnum) {
			uvl		*uvlP = child_seg->sides [child_sidenum].uvls;
			UINT32	vBr, lBr;

			theApp.SetModified (TRUE);
			for (int j = 0; j < 4; j++, uvlP++) {
				CDColor *pvc = VertexColors (child_seg->verts [side_vert [child_sidenum][j]]);
				vBr = (UINT16) uvlP->l;
				lBr = (UINT32) (brightness * fLightScale);
				BlendColors (plc, pvc, lBr, vBr);
				vBr += lBr;
				vBr = min (0x8000, vBr);
				uvlP->l = (UINT16) vBr;
				}
			continue;
			}

		// calculate vector between center of source segment and center of child
//		CBRK (child_segnum == 1 && child_sidenum == 2);
		if (CalcSideLights (child_segnum, child_sidenum, source_center, source_corner, A, effect, fLightScale, bWall)) {
				UINT32	vBr, lBr;	//vertex brightness, light brightness
				uvl		*uvlP = child_seg->sides [child_sidenum].uvls;

			theApp.SetModified (TRUE);
			for (int j = 0; j < 4; j++, uvlP++) {
				CDColor *pvc = VertexColors (child_seg->verts [side_vert [child_sidenum][j]]);
				if (child_seg->verts [side_vert [child_sidenum][j]] == 2368)
					j = j;
				vBr = (UINT16) uvlP->l;
				lBr = (UINT16) (brightness * effect [j] / 32);
				BlendColors (plc, pvc, lBr, vBr);
				vBr += lBr;
				vBr = min (0x8000, vBr);
				uvlP->l = (UINT16) vBr;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// CalcDeltaLightData()
//
// Action - Calculates dl_indices and delta_lights arrays.
//
//	Delta lights are only created if the segment has a nOvlTex
//	which matches one of the entries in the broken[] table.
//
//	For a given side, we calculate the distance and angle to all
//	nearby sides.  A nearby side is a side which is on a cube
//	which is a child or sub-child of the current cube.  Light does
//	not shine through Walls ().
//
//	We use the following equation to calculate
//	how much the light effects a side:
//              if (angle < 120 degrees)
//		  effect = k / distance
//
//	where,
//
// 		angle = acos(A dot B / |A|*|B|)
//		A = vector orthogonal to light source side
//		B = vector from center of light source side to effected side.
//		distance is from center of light souce to effected side's corner
//		k = 0x20 (maximum value for delta_light)
//
//	Delta light values are stored for each corner of the effected side.
//	(0x00 = no effect, 0x20 is maximum effect)
//
// A little fix to take care of too many delta light values leading
// to some flickering lights not being visible in Descent:
// For each recursion step, the delta light calculation will be executed
// for all children of sides with flickering lights with exactly that
// distance (recursionDepth) to their parent. In case of too many delta 
// light values for the full lighting depth at least the lighting of the
// Segments () close to flickering lights will be computed properly.
//------------------------------------------------------------------------

void CMine::CalcDeltaLightData(double fLightScale, int force) 
{
theApp.SetModified (TRUE);
for (int recursion_depth = m_deltaLightRenderDepth; recursion_depth; recursion_depth--)
	if (CalcDeltaLights (fLightScale, force, recursion_depth))
		break;
}



bool CMine::IsLava (int nBaseTex)
{
  HINSTANCE hInst = AfxGetApp()->m_hInstance;
  char		name [20];

LoadString(hInst,texture_resource + nBaseTex, name, sizeof(name));
return (strstr((char*)name,"lava") != NULL);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

int CMine::FindDeltaLight (INT16 segnum, INT16 sidenum, INT16 *pi)
{
	int	i = pi ? *pi : 0;
	int	j	= (int)GameInfo ().dl_indices.count++;
	dl_index	*pdli = DLIndex ();

if ((level_version >= 15) && (GameInfo ().fileinfo_version >= 34)) {
	for (; i < j; i++, pdli++)
		if ((pdli->d2x.segnum == segnum) && (pdli->d2x.sidenum = (UINT8) sidenum))
			return i;
	}
else {
	for (; i < j; i++, pdli++)
		if ((pdli->d2.segnum == segnum) && (pdli->d2.sidenum = (UINT8) sidenum))
			return i;
	}
return -1;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::CalcDeltaLights (double fLightScale, int force, int recursion_depth) 
{
	// initialize totals
CDSegment *srcseg, *childseg;
int source_segnum, child_segnum;
double effect[4];

GameInfo ().delta_lights.count = 0;
GameInfo ().dl_indices.count = 0;
bool bWall, bD2XLights = (level_version >= 15) && (GameInfo ().fileinfo_version >= 34);

fLightScale = 1.0; ///= 100.0;
for (source_segnum = 0, srcseg = Segments (); 
	  source_segnum < SegCount (); 
	  source_segnum++, srcseg++) {
	// skip if not marked unless we are automatically saving
	if  (!(srcseg->wall_bitmask & MARKED_MASK) && !force) 
		continue;
	// loop on all sides
	for (int source_sidenum=0;source_sidenum<6;source_sidenum++) {
		INT16 tmapnum = srcseg->sides [source_sidenum].nBaseTex & 0x3fff;
		INT16 tmapnum2 = srcseg->sides [source_sidenum].nOvlTex & 0x3fff;
		INT16 trignum;
		bool bl1 = (bool) (IsLight (tmapnum) != -1);
		bool bl2 = (bool) (IsLight (tmapnum2) != -1);
		if (!(bl1 || bl2))
			continue;	// no lights on this side
		bool bCalcDeltas = false;
		// if the current side is a wall and has a light and is the target of a trigger
		// than can make the wall appear/disappear, calculate delta lights for it
		if ((bWall = (FindWall (source_segnum, source_sidenum) != NULL)) &&
			 ((trignum = FindTriggerTarget (0, source_segnum, source_sidenum)) >= 0)) {
			INT8 trigtype = Triggers (trignum)->type;
			bCalcDeltas =
				(trigtype == TT_ILLUSION_OFF) ||
				(trigtype == TT_ILLUSION_ON) ||
				(trigtype == TT_CLOSE_WALL) ||
				(trigtype == TT_OPEN_WALL) ||
				(trigtype == TT_LIGHT_OFF) ||
				(trigtype == TT_LIGHT_ON);
				 
			}
		if (!bCalcDeltas)
			bCalcDeltas = IsFlickeringLight (source_segnum, source_sidenum);
		if (!bCalcDeltas) {
			bool bb1 = IsBlastableLight (tmapnum);
			bool bb2 = IsBlastableLight (tmapnum2);
			if (bb1 == bb2)
				bCalcDeltas = bb1;	// both lights blastable or not
			else if (!(bb1 ? bl2 : bl1))	// i.e. one light blastable and the other texture not a non-blastable light 
				bCalcDeltas = true;
			}
		if (!bCalcDeltas) {	//check if light is target of a "light on/off" trigger
			int trignum = FindTriggerTarget (0, source_segnum, source_sidenum);
			if ((trignum >= 0) && (Triggers (trignum)->type >= TT_LIGHT_OFF))
				bCalcDeltas = true;
			}
		if (!bCalcDeltas)
			continue;
		// only set lights for textures which have a nOvlTex
		//if (tmapnum2 == 0)
		//	continue;

		INT16 srcwall = srcseg->sides [source_sidenum].nWall;
		if ((srcseg->children [source_sidenum] != -1) &&
			 ((srcwall >= GameInfo ().walls.count) || (Walls (srcwall)->type == WALL_OPEN)))
			continue;

//		if ((IsLight (tmapnum) == -1) && (IsLight (tmapnum2) == -1))
//			continue;
		if (GameInfo ().dl_indices.count >= MAX_DL_INDICES) {
			char szMsg [256];
			sprintf (szMsg, " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
			DEBUGMSG (szMsg);
			return false;
			}

		vms_vector A,source_center;

		// get index number and increment total number of dl_indices
		int dl_index_num = (int)GameInfo ().dl_indices.count++;
		dl_index *pdli = DLIndex (dl_index_num);
		if (bD2XLights) {
			pdli->d2x.segnum = source_segnum;
			pdli->d2x.sidenum = source_sidenum;
			pdli->d2x.count = 0; // will be incremented below
			}
		else {
			pdli->d2.segnum = source_segnum;
			pdli->d2.sidenum = source_sidenum;
			pdli->d2.count = 0; // will be incremented below
			}
		pdli->d2.index = (INT16)GameInfo ().delta_lights.count;

		// find orthogonal angle of source segment
		CalcOrthoVector(A,source_segnum,source_sidenum);
		// remember to flip the sign since we want it to point inward
		A.x = -A.x;
		A.y = -A.y;
		A.z = -A.z;

		// calculate the center of the source segment
		CalcCenter(source_center,source_segnum,source_sidenum);

		// mark those Segments () within N children of current cube
		//(note: this is done once per light instead of once per segment
		//       even though some Segments () have multiple lights.
		//       This actually reduces the number of calls since most
		//       Segments () do not have lights)

		for (int h = 0; h < SegCount (); h++)
			Segments (h)->seg_number = -1;
		SetSegmentChildNum (srcseg, source_segnum, recursion_depth);
		srcseg->seg_number = recursion_depth;

		// setup source corner vertex for length calculation later
		vms_vector source_corner[4];
		for (int j=0;j<4;j++) {
			UINT8 vertnum = side_vert[source_sidenum][j];
			int h = srcseg->verts[vertnum];
			source_corner[j].x = Vertices (h)->x;
			source_corner[j].y = Vertices (h)->y;
			source_corner[j].z = Vertices (h)->z;
			}

		// loop on child Segments ()
		for (child_segnum = 0, childseg = Segments ();
			  child_segnum < SegCount ();
			  child_segnum++, childseg++) {
			if (childseg->seg_number < 0)
				continue;
			// loop on child sides
			for (int child_sidenum=0;child_sidenum<6;child_sidenum++) {
				// if texture has a child..
#ifdef _DEBUG
			CBRK (source_segnum == 6 && source_sidenum == 2 &&
				child_segnum == 10 && child_sidenum == 1);
#endif
				if (childseg->children[child_sidenum] >= 0) {
					UINT16 nWall = childseg->sides[child_sidenum].nWall;
					// .. if there is no wall ..
					if (nWall >= GameInfo ().walls.count)
						continue;
					// .. or its not a door ..
					if (Walls (nWall)->type == WALL_OPEN) 
						continue; // don't put light because there is no texture here
					}
				// don't affect non-flickering light emitting textures (e.g. lava)
				tmapnum = childseg->sides [child_sidenum].nBaseTex;
				tmapnum2 = childseg->sides [child_sidenum].nOvlTex & 0x3fff;
				if (m_nNoLightDeltas == 1) {
					if (((IsLight (tmapnum) >= 0) || (IsLight (tmapnum2) >= 0))
						 && !IsFlickeringLight (child_segnum, child_sidenum))
						continue;
					}
				else if ((m_nNoLightDeltas == 2) && (IsLava (tmapnum) || IsLava (tmapnum2)))
					continue;
				// if the child side is the same as the source side, then set light and continue
				if (child_sidenum == source_sidenum && child_segnum == source_segnum) {
					if ((GameInfo ().delta_lights.count >= MAX_DELTA_LIGHTS) || 
						 (bD2XLights ? pdli->d2x.count == 8191 : pdli->d2.count == 255)) {
						char szMsg [256];
						sprintf (szMsg, " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
						DEBUGMSG (szMsg);
						return false;
						}
					delta_light *dl = DeltaLights (GameInfo ().delta_lights.count++);
					dl->segnum = child_segnum;
					dl->sidenum = child_sidenum;
					dl->dummy = 0;
					dl->vert_light [0] =
					dl->vert_light [1] =
					dl->vert_light [2] =
					dl->vert_light [3] = (UINT8) min (32, 32 * fLightScale);
					if (bD2XLights)
						pdli->d2x.count++;
					else
						pdli->d2.count++;
					continue;
					}

				// calculate vector between center of source segment and center of child
#ifdef _DEBUG
					CBRK (child_segnum == qqq1 && child_sidenum == qqq2);
#endif
					if (CalcSideLights (child_segnum, child_sidenum, source_center, source_corner, A, effect, fLightScale, bWall)) {
						theApp.SetModified (TRUE);
						if ((GameInfo ().delta_lights.count >= MAX_DELTA_LIGHTS) || 
							 (bD2XLights ? pdli->d2x.count == 8191 : pdli->d2.count == 255)) {
							char szMsg [256];
							sprintf (szMsg, " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
							DEBUGMSG (szMsg);
							return false;
							}
						delta_light *dl = DeltaLights (GameInfo ().delta_lights.count++);
						dl->segnum = child_segnum;
						dl->sidenum = child_sidenum;
						dl->dummy = 0;
						for (int iCorner = 0; iCorner < 4; iCorner++)
							dl->vert_light [iCorner] = (UINT8) min(32, effect [iCorner]);
						if (bD2XLights)
							pdli->d2x.count++;
						else
							pdli->d2.count++;
						}
					}
				}
//			}
		}
	}
return true;
}

//--------------------------------------------------------------------------
// SetSegmentChildNum()
//
// Action - Sets seg_number to child number from parent
//--------------------------------------------------------------------------

void CMine::UnlinkSeg (CDSegment *pSegment, CDSegment *pRoot)
{
#if 0
	INT16	prevSeg = pSegment->prevSeg;
	INT16	nextSeg = pSegment->nextSeg;
	INT16	thisSeg = pSegment - Segments ();

CBRK ((prevSeg >= 0) && (Segments (prevSeg)->nextSeg < 0));
if (prevSeg >= 0) {
	Segments () [prevSeg].nextSeg = nextSeg;
	pSegment->prevSeg = -1;
	}
if (nextSeg >= 0) {
	Segments () [nextSeg].prevSeg = prevSeg;
	pSegment->nextSeg = -1;
	}
#endif
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::LinkSeg (CDSegment *pSegment, CDSegment *pRoot)
{
#if 0
	INT16	prevSeg = pRoot->prevSeg;
	INT16	nextSeg = pRoot->nextSeg;
	INT16 thisSeg = pSegment - Segments ();
	INT16 rootSeg = pRoot - Segments ();

if (prevSeg < 0) {
	pRoot->nextSeg = thisSeg;
	}
else {
	Segments () [prevSeg].nextSeg = thisSeg;
	pSegment->prevSeg = prevSeg;
	}
pRoot->prevSeg = thisSeg;
pSegment->nextSeg = rootSeg;
pSegment->rootSeg = rootSeg;
#endif
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetSegmentChildNum (CDSegment *pRoot, INT16 segnum,INT16 recursion_level) 
{
	INT16			sidenum, child, nImprove = 0;
	UINT16		nWall;
	CDSegment	*seg = Segments () + segnum;
	CDSegment	*prevSeg = NULL;
	bool			bMarkChildren = false;

// mark each child if child number is lower
for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
	// Skip if this is a door
	nWall = seg->sides [sidenum].nWall;
	// .. if there is a wall and its a door
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->type == WALL_DOOR))
		continue;
	// mark segment if it has a child
	child = seg->children [sidenum];
	if ((child > -1) && (child < SegCount ()) && (recursion_level > seg->seg_number)) {
		if (seg->seg_number >= 0)
			++nImprove;
/*
		if (pRoot) {
			UnlinkSeg (seg, pRoot);
			LinkSeg (seg, pRoot);
			}
*/
		seg->seg_number = recursion_level;
		bMarkChildren = true;
		break;
		}
	}
//return if segment has no children or max recursion depth is reached
if (!bMarkChildren || (recursion_level == 1))
	return;

// check each side of this segment for more children
for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++) {
	// skip if there is a wall and its a door
	nWall = seg->sides [sidenum].nWall;
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->type == WALL_DOOR))
		continue;
	// check child
	child = seg->children [sidenum];
	if ((child > -1) && (child < SegCount ()))
		SetSegmentChildNum (pRoot, child, recursion_level - 1);
	}
}

//--------------------------------------------------------------------------

bool CMine::CalcSideLights (int segnum, int sidenum, vms_vector& source_center, 
									 vms_vector *source_corner, vms_vector& A, double *effect,
									 double fLightScale, bool bIgnoreAngle)
{
	CDSegment *seg = Segments (segnum);
// calculate vector between center of source segment and center of child
vms_vector B,center;
CalcCenter (center,segnum,sidenum);
B.x = center.x - source_center.x;
B.y = center.y - source_center.y;
B.z = center.z - source_center.z;

// calculate angle between vectors (use dot product equation)
if (!bIgnoreAngle) {
	double ratio,angle;
	double A_dot_B = (double)A.x * (double)B.x
						+ (double)A.y * (double)B.y
						+ (double)A.z * (double)B.z;
	double mag_A = my_sqrt( (double)A.x*(double)A.x
						+(double)A.y*(double)A.y
						+(double)A.z*(double)A.z);
	double mag_B = my_sqrt( (double)B.x*(double)B.x
						+(double)B.y*(double)B.y
						+(double)B.z*(double)B.z);
	if (mag_A == 0 || mag_B == 0)
		angle = (200.0 * M_PI)/180.0; // force a failure
	else {
		ratio = A_dot_B/(mag_A * mag_B);
		ratio = ((double)((int)(ratio*1000.0))) / 1000.0;
		if (ratio < -1.0 || ratio > (double)1.0)
			angle = (199.0 * M_PI)/180.0;  // force a failure
		else
			angle = acos(ratio);
		}
	// if angle is less than 110 degrees
	// then we found a match
	if (angle >= (180.0 * M_PI)/180.0)
		return false;
	}
for (int j = 0; j < 4; j++) {
	vms_vector corner;
	int vertnum = side_vert[sidenum][j];
	int h = seg->verts[vertnum];
	corner.x = Vertices (h)->x;
	corner.y = Vertices (h)->y;
	corner.z = Vertices (h)->z;
	double length = 20.0 * m_lightRenderDepth;
	for (int i = 0; i < 4; i++)
		length = min (length, CalcLength (source_corner + i, &corner) / F1_0);
	length /= 10.0 * m_lightRenderDepth / 6.0; // divide by 1/2 a cubes length so opposite side
	// light is recuded by 1/4
	effect [j] = 32;
	if (length > 1.0)//if (length < 20.0 * m_lightRenderDepth) // (roughly 4 standard cube lengths)
		effect [j] /= (length * length);
	effect [j] *= fLightScale;
//	else
//		effect [j] = 0;
	}
// if any of the effects are > 0, then increment the
// light for that side
return (effect [0] != 0 || effect [1] != 0 || effect [2] != 0 || effect [3] != 0);
}

//--------------------------------------------------------------------------

CDColor *CMine::LightColor (int i, int j, bool bUseTexColors) 
{ 
if (bUseTexColors && UseTexColors ()) {
	CDWall *pWall = SideWall (i, j);
	//if (!pWall || (pWall->type != WALL_TRANSPARENT)) 
		{	//always use a side color for transp. walls
		CDColor *pc;
		INT16 t = Segments (i)->sides [j].nOvlTex & 0x3fff;
		if ((t > 0) && (pc = GetTexColor (t)))
			return pc;
		if (pc = GetTexColor (Segments (i)->sides [j].nBaseTex, pWall && (pWall->type == WALL_TRANSPARENT)))
			return pc;
		}
	}	
return MineData ().lightColors [i] + j; 
}

//--------------------------------------------------------------------------
//eof light.cpp