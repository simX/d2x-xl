/* $Id: lighting.h,v 1.2 2003/10/10 09:36:35 btb Exp $ */
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
 * Lighting system prototypes, structures, etc.
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:58:51  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:27:52  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.6  1994/11/28  21:50:56  mike
 * optimizations.
 *
 * Revision 1.5  1994/06/07  16:51:58  matt
 * Made tObject lighting work correctly; changed name of Ambient_light to
 * Dynamic_light; cleaned up polygobj tObject rendering a little.
 *
 * Revision 1.4  1994/05/31  18:41:35  matt
 * Added comments
 *
 * Revision 1.3  1994/05/23  15:00:08  mike
 * Change MIN_LIGHT_DIST.
 *
 * Revision 1.2  1994/05/22  15:30:09  mike
 * First version.
 *
 * Revision 1.1  1994/05/22  15:16:44  mike
 * Initial revision
 *
 *
 */

#ifndef _LIGHTING_H
#define _LIGHTING_H

#define MAX_LIGHT       0x10000     // max value

#define MIN_LIGHT_DIST  (F1_0*4)

extern fix Beam_brightness;
extern fix dynamicLight[MAX_VERTICES];
extern tRgbColorf dynamicColor[MAX_VERTICES];
extern char bGotDynColor [MAX_VERTICES];

extern void SetDynamicLight(void);

// Compute the lighting from the headlight for a given vertex on a face.
// Takes:
//  point - the 3d coords of the point
//  face_light - a scale factor derived from the surface normal of the face
// If no surface normal effect is wanted, pass F1_0 for face_light
fix compute_headlight_light(vmsVector *point,fix face_light);

// compute the average dynamic light in a tSegment.  Takes the tSegment number
fix compute_seg_dynamic_light(int nSegment);

// compute the lighting for an tObject.  Takes a pointer to the tObject,
// and possibly a rotated 3d point.  If the point isn't specified, the
// tObject's center point is rotated.
fix ComputeObjectLight(tObject *obj,vmsVector *rotated_pnt);
void ComputeEngineGlow (tObject *obj, fix *engine_glowValue);
// turn headlight boost on & off
void toggle_headlight_active(void);

// returns ptr to flickering light structure, or NULL if can't find
flickering_light *FindFlicker(int nSegment, int nSide);

// turn flickering off (because light has been turned off)
void DisableFlicker(int nSegment, int nSide);

// turn flickering off (because light has been turned on)
void EnableFlicker(int nSegment, int nSide);

// returns 1 if ok, 0 if error
int AddFlicker(int nSegment, int nSide, fix delay, unsigned long mask);

void ReadFlickeringLight(flickering_light *fl, CFILE *fp);

void InitTextureBrightness (void);

void RegisterLight (tFaceColor *pc, short nSegment, short nSide);
int AddOglLight (tRgbColorf *pc, fix xBrightness, short nSegment, short nSide, short nOwner);
int RemoveOglLight (short nSegment, short nSide, short nObject);
void AddOglLights (void);
void RemoveOglLights (void);
void SetOglLightPos (short nObject);
void MoveOglLight (short nObject);
void TransformOglLights (int bStatic, int bVariable);
short FindOglLight (short nSegment, short nSide, short nObject);
int ToggleOglLight (short nSegment, short nSide, short nObject, int bState);
void SetOglLightMaterial (short nSegment, short nSide, short nObject);
void SetNearestVertexLights (int nVertex, ubyte nType, int bStatic, int bVariable);
void SetNearestStaticLights (int nSegment, ubyte nType);
void SetNearestDynamicLights (int nSegment);
void ComputeStaticOglLighting (void);
void InitLightingShaders (void);
tFaceColor *AvgSgmColor (int nSegment, vmsVector *vPos);

#endif /* _LIGHTING_H */
