/* $Id: segment.h,v 1.4 2003/10/04 03:14:47 btb Exp $ */
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
 * Include file for functions which need to access tSegment data structure.
 *
 * Old Log:
 * Revision 1.4  1995/11/03  12:53:11  allender
 * shareware changes
 *
 * Revision 1.3  1995/07/26  16:53:45  allender
 * put sides and tSegment structure back the PC way for checksumming reasons
 *
 * Revision 1.2  1995/06/19  07:55:22  allender
 * rearranged structure members for possible better alignment
 *
 * Revision 1.1  1995/05/16  16:02:22  allender
 * Initial revision
 *
 * Revision 2.1  1995/03/20  18:15:22  john
 * Added code to not store the normals in the tSegment structure.
 *
 * Revision 2.0  1995/02/27  11:26:49  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.89  1995/01/24  15:07:55  yuan
 * *** empty log message ***
 *
 * Revision 1.88  1994/12/12  01:04:06  yuan
 * Boosted MAX_GAME_VERTS.
 *
 * Revision 1.87  1994/12/11  16:18:14  mike
 * add constants so we can detect too-large mines for game while in editor.
 *
 * Revision 1.86  1994/12/08  15:07:29  yuan
 * *** empty log message ***
 *
 * Revision 1.85  1994/12/01  21:06:39  matt
 * Moved plane tolerance constant to gameseg.c, the only file that used it.
 *
 * Revision 1.84  1994/11/27  14:01:41  matt
 * Fixed tSegment structure so LVLs work
 *
 * Revision 1.83  1994/11/26  22:50:20  matt
 * Removed editor-only fields from tSegment structure when editor is compiled
 * out, and padded tSegment structure to even multiple of 4 bytes.
 *
 * Revision 1.82  1994/11/21  11:43:36  mike
 * smaller tSegment and vertex buffers.
 *
 * Revision 1.81  1994/11/17  11:39:35  matt
 * Ripped out code to load old mines
 *
 * Revision 1.80  1994/10/30  14:12:05  mike
 * rip out local segments stuff.
 *
 * Revision 1.79  1994/10/27  11:33:58  mike
 * lower number of segments by 100, saving 116K.
 *
 * Revision 1.78  1994/08/25  21:54:50  mike
 * Add macro IS_CHILD to make checking for the presence of a child centralized.
 *
 * Revision 1.77  1994/08/11  18:58:16  mike
 * Add prototype for sideToVertsInt.
 *
 * Revision 1.76  1994/08/01  11:04:13  yuan
 * New materialization centers.
 *
 * Revision 1.75  1994/07/25  00:04:19  matt
 * Various changes to accomodate new 3d, which no longer takes point numbers
 * as parms, and now only takes pointers to points.
 *
 * Revision 1.74  1994/07/21  19:01:30  mike
 * new lsegment structure.
 *
 * Revision 1.73  1994/06/08  14:30:48  matt
 * Added static_light field to tSegment structure, and padded tSide struct
 * to be longword aligned.
 *
 * Revision 1.72  1994/05/19  23:25:17  mike
 * Change MINE_VERSION to 15, DEFAULT_LIGHTING to 0
 *
 * Revision 1.71  1994/05/12  14:45:54  mike
 * New tSegment data structure (!!), group, special, tObject, value = short.
 *
 * Revision 1.70  1994/05/03  11:06:46  mike
 * Remove constants VMAG and UMAG which are editor specific..
 *
 * Revision 1.69  1994/04/18  10:40:28  yuan
 * Increased tSegment limit to 1000
 * (From 500)
 *
 *
 */

#ifndef _SEGMENT_H
#define _SEGMENT_H

#include "pstypes.h"
#include "fix.h"
#include "vecmat.h"
#include "3d.h"
//#include "inferno.h"
#include "cfile.h"
#include "gr.h"

// Version 1 - Initial version
// Version 2 - Mike changed some shorts to bytes in segments, so incompatible!

#define SIDE_IS_QUAD    1   // render tSide as quadrilateral
#define SIDE_IS_TRI_02  2   // render tSide as two triangles, triangulated along edge from 0 to 2
#define SIDE_IS_TRI_13  3   // render tSide as two triangles, triangulated along edge from 1 to 3

// Set maximum values for tSegment and face data structures.
#define MAX_VERTICES_PER_SEGMENT    8
#define MAX_SIDES_PER_SEGMENT       6
#define MAX_VERTICES_PER_POLY       4
#define WLEFT                       0
#define WTOP                        1
#define WRIGHT                      2
#define WBOTTOM                     3
#define WBACK                       4
#define WFRONT                      5

#if defined(SHAREWARE)
# define MAX_SEGMENTS           800
# define MAX_SEGMENT_VERTICES   2808
#else
# define MAX_SEGMENTS_D2        900
# define MAX_SEGMENTS           5000
# define MAX_SEGMENT_VERTICES   (MAX_SEGMENTS * 4 + 8)
#endif

//normal everyday vertices

#define DEFAULT_LIGHTING        0   // (F1_0/2)

#ifdef EDITOR   //verts for the new tSegment
# define NUM_NEW_SEG_VERTICES   8
# define NEW_SEGMENT_VERTICES   (MAX_SEGMENT_VERTICES)
# define MAX_VERTICES           (MAX_SEGMENT_VERTICES+NUM_NEW_SEG_VERTICES)
#else           //No editor
# define MAX_VERTICES           (MAX_SEGMENT_VERTICES)
#endif

// Returns true if nSegment references a child, else returns false.
// Note that -1 means no connection, -2 means a connection to the outside world.
#define IS_CHILD(nSegment) (nSegment > -1)

#if 0
//Structure for storing u,v,light values.
//NOTE: this structure should be the same as the one in 3d.h
typedef struct uvl {
	fix u, v, l;
} uvl;
#endif

#ifdef COMPACT_SEGS
typedef struct tSide {
	sbyte   nType;           // replaces num_faces and tri_edge, 1 = quad, 2 = 0:2 triangulation, 3 = 1:3 triangulation
	ubyte   pad;            //keep us longword alligned
	short   nWall;
	short   nTexture [2];
	uvl     uvls [4];
	//vmsVector normals[2];  // 2 normals, if quadrilateral, both the same.
} tSide;
#else
typedef struct tSide {
	sbyte   		nType;           // replaces num_faces and tri_edge, 1 = quad, 2 = 0:2 triangulation, 3 = 1:3 triangulation
	sbyte   		nFrame;      //keep us longword aligned
	ushort  		nWall;
	short   		nBaseTex;
	ushort		nOvlTex : 14;
	ushort		nOvlOrient : 2;
	uvl     		uvls [4];
	vmsVector	normals [2];  // 2 normals, if quadrilateral, both the same.
} tSide;
#endif

typedef struct tSegment {
#ifdef EDITOR
	short   nSegment;     // tSegment number, not sure what it means
#endif
	tSide   sides [MAX_SIDES_PER_SEGMENT];       // 6 sides
	short   children [MAX_SIDES_PER_SEGMENT];    // indices of 6 children segments, front, left, top, right, bottom, back
	short   verts [MAX_VERTICES_PER_SEGMENT];    // vertex ids of 4 front and 4 back vertices
#ifdef EDITOR
	short   group;      // group number to which the tSegment belongs.
	short   objects;    // pointer to objects in this tSegment
#else
	int     objects;    // pointer to objects in this tSegment
#endif
} tSegment;

typedef struct xsegment {
	char		owner;		  // team owning that tSegment (-1: always neutral, 0: neutral, 1: blue team, 2: red team)
	char		group;
} xsegment;


#define S2F_AMBIENT_WATER   0x01
#define S2F_AMBIENT_LAVA    0x02

typedef struct segment2 {
	ubyte   special;
	sbyte   nMatCen;
	sbyte   value;
	ubyte   s2Flags;
	fix     static_light;
} segment2;

//values for special field
#define SEGMENT_IS_NOTHING      0
#define SEGMENT_IS_FUELCEN      1
#define SEGMENT_IS_REPAIRCEN    2
#define SEGMENT_IS_CONTROLCEN   3
#define SEGMENT_IS_ROBOTMAKER   4
#define SEGMENT_IS_GOAL_BLUE    5
#define SEGMENT_IS_GOAL_RED     6
#define SEGMENT_IS_WATER        7
#define SEGMENT_IS_LAVA         8
#define SEGMENT_IS_TEAM_BLUE    9
#define SEGMENT_IS_TEAM_RED     10
#define SEGMENT_IS_SPEEDBOOST	  11
#define SEGMENT_IS_BLOCKED		  12
#define SEGMENT_IS_NODAMAGE	  13
#define SEGMENT_IS_SKYBOX		  14
#define MAX_CENTER_TYPES        15

#ifdef COMPACT_SEGS
extern void GetSideNormal(tSegment *sp, int nSide, int normal_num, vmsVector * vm );
extern void GetSideNormals(tSegment *sp, int nSide, vmsVector * vm1, vmsVector *vm2 );
#endif

// Local tSegment data.
// This is stuff specific to a tSegment that does not need to get
// written to disk.  This is a handy separation because we can add to
// this structure without obsoleting existing data on disk.

#define SS_REPAIR_CENTER    0x01    // Bitmask for this tSegment being part of repair center.

//--repair-- typedef struct {
//--repair-- 	int     specialType;
//--repair-- 	short   special_segment; // if specialType indicates repair center, this is the base of the repair center
//--repair-- } lsegment;

typedef struct {
	int     num_segments;
	int     num_vertices;
	short   segments[MAX_SEGMENTS];
	short   vertices[MAX_VERTICES];
} group;

// Globals from mglobal.c
extern sbyte sideToVerts[MAX_SIDES_PER_SEGMENT][4];       // sideToVerts[my_side] is list of vertices forming tSide my_side.
extern int  sideToVertsInt[MAX_SIDES_PER_SEGMENT][4];    // sideToVerts[my_side] is list of vertices forming tSide my_side.
extern char sideOpposite[];                                // sideOpposite[my_side] returns tSide opposite cube from my_side.

// New stuff, 10/14/95: For shooting out lights and monitors.
// Light cast upon vert_light vertices in nSegment:nSide by some light
typedef struct {
	short   nSegment;
	sbyte   nSide;
	sbyte   dummy;
	ubyte   vert_light[4];
} delta_light;

// Light at nSegment:nSide casts light on count sides beginning at index (in array gameData.render.lightDeltas)
typedef struct {
	short   nSegment;
	ubyte   nSide;
	ubyte   count;
	unsigned short   index;
} dl_index_d2;

typedef struct {
	short   nSegment;
	unsigned short nSide :3;
	unsigned short count :13;
	unsigned short   index;
} dl_index_d2x;

typedef union {
	dl_index_d2x	d2x;
	dl_index_d2		d2;
} dl_index;

#define MAX_DL_INDICES_D2    500
#define MAX_DELTA_LIGHTS_D2  10000

#define MAX_DL_INDICES       2500
#define MAX_DELTA_LIGHTS     50000

#define DL_SCALE            2048    // Divide light to allow 3 bits integer, 5 bits fraction.

int SubtractLight(short nSegment, short nSide);
int AddLight(short nSegment, short nSide);
void restore_all_lights_in_mine(void);
void ClearLightSubtracted(void);

// ----------------------------------------------------------------------------
// --------------------- Segment interrogation functions ----------------------
// Do NOT read the tSegment data structure directly.  Use these
// functions instead.  The tSegment data structure is GUARANTEED to
// change MANY TIMES.  If you read the tSegment data structure
// directly, your code will break, I PROMISE IT!

// Return a pointer to the list of vertex indices for the current
// tSegment in vp and the number of vertices in *nv.
extern void med_get_vertex_list(tSegment *s,int *nv,short **vp);

// Return a pointer to the list of vertex indices for face facenum in
// vp and the number of vertices in *nv.
extern void med_get_face_vertex_list(tSegment *s,int tSide, int facenum,int *nv,short **vp);

// Set *nf = number of faces in tSegment s.
extern void med_get_num_faces(tSegment *s,int *nf);

void med_validate_segment_side(tSegment *sp, short tSide);

// Delete tSegment function added for curves.c
extern int med_delete_segment(tSegment *sp);

// Delete tSegment from group
extern void delete_segment_from_group(int segment_num, int group_num);

// Add tSegment to group
extern void add_segment_to_group(int segment_num, int group_num);

// Verify that all vertices are legal.
extern void med_check_all_vertices();

#ifdef FAST_FILE_IO
#define segment2_read(s2, fp) CFRead(s2, sizeof(segment2), 1, fp)
#define delta_light_read(dl, fp) CFRead(dl, sizeof(delta_light), 1, fp)
#define dl_index_read(di, fp) CFRead(di, sizeof(dl_index), 1, fp)
#else
/*
 * reads a segment2 structure from a CFILE
 */
void segment2_read(segment2 *s2, CFILE *fp);

/*
 * reads a delta_light structure from a CFILE
 */
void delta_light_read(delta_light *dl, CFILE *fp);

/*
 * reads a dl_index structure from a CFILE
 */
void dl_index_read(dl_index *di, CFILE *fp);
#endif

#endif
