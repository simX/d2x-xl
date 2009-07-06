// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"
#include "io.h"
#include "file.h"

#define CURRENT_POINT(a) ((theApp.GetMine ()->Current ()->point + (a))&0x03)

char *BLOCKOP_HINT =
	"The block of cubes will be saved relative to the current cube.\n"
	"Later, when you paste the block, it will be placed relative to\n"
	"the current cube at that time.  You can change the current side\n"
	"and the current point to affect the relative direction and\n"
	"rotation of the block.\n"
	"\n"
	"Would you like to proceed?";

//---------------------------------------------------------------------------
// ReadSegmentInfo()
//
// ACTION - Reads a segment's information in text form from a file.  Adds
//          new vertices if non-identical one does not exist.  Aborts if
//	    MAX_VERTICES is hit.
//
// Change - Now reads verts relative to current side
//---------------------------------------------------------------------------

INT16 CMine::ReadSegmentInfo (FILE *fBlk) 
{
	CDSegment		*seg;
	CDSide			*side;
#if 0
	CDObject			*obj;
	INT16				objnum, segObjCount;
#endif
	INT16				segnum, sidenum, vertnum;
	INT16				i, j, test;
	INT16				origVertCount, k;
	FIX				x,y,z;
	vms_vector		origin,vect;
	struct dvector x_prime, y_prime, z_prime;
	struct dvector x_pprime, y_pprime, z_pprime;
	double			length;
	INT16				nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0, nNewObjects = 0;
	INT16				xlatSegNum [MAX_SEGMENTS3];
	int				byteBuf;

// remember number of vertices for later
origVertCount = VertCount ();

// set origin
seg = CurrSeg ();
sidenum = Current ()->side;
vertnum = seg->verts [side_vert [sidenum][CURRENT_POINT(0)]];
memcpy (&origin, Vertices (vertnum), sizeof (*Vertices ()));
/*
origin.x = Vertices (vertnum)->x;
origin.y = Vertices (vertnum)->y;
origin.z = Vertices (vertnum)->z;
*/
// set x'
vertnum = seg->verts [side_vert [sidenum][CURRENT_POINT(1)]];
x_prime.x = (double)(Vertices (vertnum)->x - origin.x);
x_prime.y = (double)(Vertices (vertnum)->y - origin.y);
x_prime.z = (double)(Vertices (vertnum)->z - origin.z);

// calculate y'
vertnum = seg->verts [side_vert [sidenum][CURRENT_POINT(3)]];
vect.x = (FIX)(Vertices (vertnum)->x - origin.x);
vect.y = (FIX)(Vertices (vertnum)->y - origin.y);
vect.z = (FIX)(Vertices (vertnum)->z - origin.z);
y_prime.x = x_prime.y*vect.z - x_prime.z*vect.y;
y_prime.y = x_prime.z*vect.x - x_prime.x*vect.z;
y_prime.z = x_prime.x*vect.y - x_prime.y*vect.x;

// calculate z'
z_prime.x = x_prime.y*y_prime.z - x_prime.z*y_prime.y;
z_prime.y = x_prime.z*y_prime.x - x_prime.x*y_prime.z;
z_prime.z = x_prime.x*y_prime.y - x_prime.y*y_prime.x;

// normalize all
length = sqrt(x_prime.x*x_prime.x + x_prime.y*x_prime.y + x_prime.z*x_prime.z);
x_prime.x /= length;
x_prime.y /= length;
x_prime.z /= length;
length = sqrt(y_prime.x*y_prime.x + y_prime.y*y_prime.y + y_prime.z*y_prime.z);
y_prime.x /= length;
y_prime.y /= length;
y_prime.z /= length;
length = sqrt(z_prime.x*z_prime.x + z_prime.y*z_prime.y + z_prime.z*z_prime.z);
z_prime.x /= length;
z_prime.y /= length;
z_prime.z /= length;

// now take the determinent
x_pprime.x = - z_prime.y*y_prime.z + y_prime.y*z_prime.z;
x_pprime.y = - x_prime.y*z_prime.z + z_prime.y*x_prime.z;
x_pprime.z = - y_prime.y*x_prime.z + x_prime.y*y_prime.z;
y_pprime.x = - y_prime.x*z_prime.z + z_prime.x*y_prime.z;
y_pprime.y = - z_prime.x*x_prime.z + x_prime.x*z_prime.z;
y_pprime.z = - x_prime.x*y_prime.z + y_prime.x*x_prime.z;
z_pprime.x = - z_prime.x*y_prime.y + y_prime.x*z_prime.y;
z_pprime.y = - x_prime.x*z_prime.y + z_prime.x*x_prime.y;
z_pprime.z = - y_prime.x*x_prime.y + x_prime.x*y_prime.y;

#if 0
  sprintf(message,"x'=(%0.3f,%0.3f,%0.3f)\n"
		  "y'=(%0.3f,%0.3f,%0.3f)\n"
		  "z'=(%0.3f,%0.3f,%0.3f)\n",
		  (float)x_prime.x,(float)x_prime.y,(float)x_prime.z,
		  (float)y_prime.x,(float)y_prime.y,(float)y_prime.z,
		  (float)z_prime.x,(float)z_prime.y,(float)z_prime.z);
  DebugMsg(message);
  sprintf(message,"x''=(%0.3f,%0.3f,%0.3f)\n"
		  "y''=(%0.3f,%0.3f,%0.3f)\n"
		  "z''=(%0.3f,%0.3f,%0.3f)\n",
		  (float)x_pprime.x,(float)x_pprime.y,(float)x_pprime.z,
		  (float)y_pprime.x,(float)y_pprime.y,(float)y_pprime.z,
		  (float)z_pprime.x,(float)z_pprime.y,(float)z_pprime.z);
  DebugMsg(message);
#endif

nNewSegs = 0;
memset (xlatSegNum, 0xff, sizeof (xlatSegNum));
while(!feof(fBlk)) {
	if (SegCount () >= MAX_SEGMENTS) {
		ErrorMsg("No more free segments");
		return (nNewSegs);
		}
// abort if there are not at least 8 vertices free
	if (MAX_VERTICES - VertCount () < 8) {
		ErrorMsg("No more free vertices");
		return(nNewSegs);
		}
	segnum = SegCount ();
	seg = Segments (segnum);
	seg->owner = -1;
	seg->group = -1;
	fscanf (fBlk,"segment %hd\n",&seg->seg_number);
	xlatSegNum [seg->seg_number] = segnum;
	// invert segment number so its children can be children can be fixed later
	seg->seg_number = ~seg->seg_number;

	// read in side information 
	side = seg->sides;
	int sidenum;
	for (sidenum = 0; sidenum < MAX_SIDES_PER_SEGMENT; sidenum++, side++) {
		fscanf (fBlk,"  side %hd\n", &test);
		if (test != sidenum) {
			ErrorMsg ("Invalid side number read");
			return (0);
			}
		side->nWall = NO_WALL;
		fscanf (fBlk,"    tmap_num %hd\n",&side->nBaseTex);
		fscanf (fBlk,"    tmap_num2 %hd\n",&side->nOvlTex);
		for (j = 0; j < 4; j++)
			fscanf (fBlk,"    uvls %hd %hd %hd\n",
						&side->uvls [j].u,
						&side->uvls [j].v,
						&side->uvls [j].l);
		if (bExtBlkFmt) {
			fscanf (fBlk, "    nWall %d\n",&byteBuf);
			side->nWall = (UINT16) byteBuf;
			if (side->nWall != NO_WALL) {
				CDWall w;
				CDTrigger t;
				memset (&w, 0, sizeof (w));
				memset (&t, 0, sizeof (t));
				fscanf (fBlk, "        segment %ld\n", &w.segnum);
				fscanf (fBlk, "        side %ld\n", &w.sidenum);
				fscanf (fBlk, "        hps %ld\n", &w.hps);
				fscanf (fBlk, "        type %d\n", &byteBuf);
				w.type = byteBuf;
				fscanf (fBlk, "        flags %d\n", &byteBuf);
				w.flags = byteBuf;
				fscanf (fBlk, "        state %d\n", &byteBuf);
				w.state = byteBuf;
				fscanf (fBlk, "        clip_num %d\n", &byteBuf);
				w.clip_num = byteBuf;
				fscanf (fBlk, "        keys %d\n", &byteBuf);
				w.keys = byteBuf;
				fscanf (fBlk, "        cloak %d\n", &byteBuf);
				w.cloak_value = byteBuf;
				fscanf (fBlk, "        trigger %d\n", &byteBuf);
				w.trigger = byteBuf;
				if ((w.trigger >= 0) && (w.trigger < MAX_TRIGGERS)) {
					fscanf (fBlk, "			    type %d\n", &byteBuf);
					t.type = byteBuf;
					fscanf (fBlk, "			    flags %hd\n", &t.flags);
					fscanf (fBlk, "			    value %ld\n", &t.value);
					fscanf (fBlk, "			    timer %d\n", &t.time);
					fscanf (fBlk, "			    num_links %hd\n", &t.num_links);
					int iTarget;
					for (iTarget = 0; iTarget < t.num_links; iTarget++) {
						fscanf (fBlk, "			        seg %hd\n", t.seg + iTarget);
						fscanf (fBlk, "			        side %hd\n", t.side + iTarget);
						}
					}
				if (GameInfo ().walls.count < MAX_WALLS) {
					if ((w.trigger >= 0) && (w.trigger < MAX_TRIGGERS)) {
						if (GameInfo ().triggers.count >= MAX_TRIGGERS)
							w.trigger = NO_TRIGGER;
						else {
							w.trigger = GameInfo ().triggers.count++;
							++nNewTriggers;
							*Triggers (w.trigger) = t;
							}
						}
					nNewWalls++;
					side->nWall = GameInfo ().walls.count++;
					w.segnum = segnum;
					*Walls (side->nWall) = w;
					}
				}
#if 0
			fscanf (fBlk, "    object_num %hd\n",&segObjCount);
			while (segObjCount) {
				CDObject o;
				memset (&o, 0, sizeof (o));
				fscanf (fBlk, "            signature %hd\n", &o.signature);
				fscanf (fBlk, "            type %d\n", &byteBuf);
				o.type = (INT8) byteBuf;
				fscanf (fBlk, "            id %d\n", &byteBuf);
				o.id = (INT8) byteBuf;
				fscanf (fBlk, "            control_type %d\n", &byteBuf);
				o.control_type = (UINT8) byteBuf;
				fscanf (fBlk, "            movement_type %d\n", &byteBuf);
				o.movement_type = (UINT8) byteBuf;
				fscanf (fBlk, "            render_type %d\n", &byteBuf);
				o.render_type = (UINT8) byteBuf;
				fscanf (fBlk, "            flags %d\n", &byteBuf);
				o.flags = (UINT8) byteBuf;
				o.segnum = segnum;
				fscanf (fBlk, "            pos %ld %ld %ld\n", &o.pos.x, &o.pos.y, &o.pos.z);
				memcpy (&o.last_pos, &o.pos, sizeof (o.pos));
				fscanf (fBlk, "            orient %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", 
													&o.orient.rvec.x, &o.orient.rvec.y, &o.orient.rvec.z,
													&o.orient.uvec.x, &o.orient.uvec.y, &o.orient.uvec.z,
													&o.orient.fvec.x, &o.orient.fvec.y, &o.orient.fvec.z);
				fscanf (fBlk, "            segnum %hd\n", &o.segnum);
				fscanf (fBlk, "            size %ld\n", &o.size);
				fscanf (fBlk, "            shields %ld\n", &o.shields);
				fscanf (fBlk, "            contains_type %d\n", &byteBuf);
				o.contains_type = (INT8) byteBuf;
				fscanf (fBlk, "            contains_id %d\n", &byteBuf);
				o.contains_id = (INT8) byteBuf;
				fscanf (fBlk, "            contains_count %d\n", &byteBuf);
				o.contains_count = (INT8) byteBuf;
				switch (o.type) {
					case OBJ_POWERUP:
					case OBJ_HOSTAGE:
						// has vclip
						break;
					case OBJ_PLAYER:
					case OBJ_COOP:
					case OBJ_ROBOT:
					case OBJ_WEAPON:
					case OBJ_CNTRLCEN:
						// has poly model;
						break;
					}
				switch (o.control_type) {
					case :
					}
				switch (o.movement_type) {
					case MT_PHYSICS:
						fscanf (fBlk, "            velocity %ld %ld %ld\n", 
								  &o.phys_info.velocity.x, &o.phys_info.velocity.y, &o.phys_info.velocity.z);
						fscanf (fBlk, "            thrust %ld %ld %ld\n", 
								  &o.phys_info.thrust.x, &o.phys_info.thrust.y, &o.phys_info.thrust.z);
						fscanf (fBlk, "            mass %ld\n", &o.phys_info.mass);
						fscanf (fBlk, "            drag %ld\n", &o.phys_info.drag);
						fscanf (fBlk, "            brakes %ld\n", &o.phys_info.brakes);
						fscanf (fBlk, "            rotvel %ld %ld %ld\n", 
								  &o.phys_info.rotvel.x, &o.phys_info.rotvel.y, &o.phys_info.rotvel.z);
						fscanf (fBlk, "            rotthrust %ld %ld %ld\n", 
								  &o.phys_info.rotthrust.x, &o.phys_info.rotthrust.y, &o.phys_info.rotthrust.z);
						fscanf (fBlk, "            turnroll %hd\n", &o.phys_info.turnroll);
						fscanf (fBlk, "            flags %hd\n", &o.phys_info.flags);
						break;
					case MT_SPIN:
						fscanf (fBlk, "            spinrate %ld %ld %ld\n", 
								  &o.spin_rate.x, &o.spin_rate.y, &o.spin_rate.z);
						break;
					}
				switch (o.render_type) {
					case :
						break;
					}
				}
#endif
			}
		}
	fscanf (fBlk,"  children %hd %hd %hd %hd %hd %hd\n",
				&seg->children [0],&seg->children [1],&seg->children [2],
				&seg->children [3],&seg->children [4],&seg->children [5]);
	// read in vertices
	for (i = 0; i < 8; i++) {
		fscanf (fBlk,"  vms_vector %hd %ld %ld %ld\n", &test, &vect.x, &vect.y, &vect.z);
		if (test != i) {
			ErrorMsg("Invalid vertex number read");
			return (0);
			}
		// each vertex relative to the origin has a x', y', and z' component
		// adjust vertices relative to origin
		x = origin.x + (FIX)((double) vect.x * x_pprime.x + (double) vect.y * x_pprime.y + (double) vect.z * x_pprime.z);
		y = origin.y + (FIX)((double) vect.x * y_pprime.x + (double) vect.y * y_pprime.y + (double) vect.z * y_pprime.z);
		z = origin.z + (FIX)((double) vect.x * z_pprime.x + (double) vect.y * z_pprime.y + (double) vect.z * z_pprime.z);
		// add a new vertex
		// if this is the same as another vertex, then use that vertex number instead
		vms_vector *vert = Vertices (origVertCount);
		for (k = origVertCount; k < VertCount (); k++, vert++)
			if (vert->x == x && vert->y == y && vert->z == z) {
				seg->verts [i] = k;
				break;
				}
		// else make a new vertex
		if (k == VertCount ()) {
			vertnum = VertCount ();
			*VertStatus (vertnum) |= NEW_MASK;
			seg->verts [i] = vertnum;
			Vertices (vertnum)->x = x;
			Vertices (vertnum)->y = y;
			Vertices (vertnum)->z = z;
			VertCount ()++;
			}
		}
	// mark vertices
	for (i = 0; i < 8; i++)
		*VertStatus (seg->verts [i]) |= MARKED_MASK;
	fscanf (fBlk,"  static_light %ld\n",&seg->static_light);
	if (bExtBlkFmt) {
		fscanf (fBlk,"  special %d\n", &byteBuf);
		seg->special = byteBuf;
		fscanf (fBlk,"  matcen_num %d\n", &byteBuf);
		seg->matcen_num = byteBuf;
		fscanf (fBlk,"  value %d\n", &byteBuf);
		seg->value = byteBuf;
		fscanf (fBlk,"  child_bitmask %d\n", &byteBuf);
		seg->child_bitmask = byteBuf;
		fscanf (fBlk,"  wall_bitmask %d\n", &byteBuf);
		seg->wall_bitmask = byteBuf;
		switch (seg->special) {
			case SEGMENT_IS_FUELCEN:
				if (!AddFuelCenter (segnum, SEGMENT_IS_FUELCEN, false, false))
					seg->special = 0;
				break;
			case SEGMENT_IS_REPAIRCEN:
				if (!AddFuelCenter (segnum, SEGMENT_IS_REPAIRCEN, false, false))
					seg->special = 0;
				break;
			case SEGMENT_IS_ROBOTMAKER:
				if (!AddRobotMaker (segnum, false, false))
					seg->special = 0;
				break;
			case SEGMENT_IS_EQUIPMAKER:
				if (!AddEquipMaker (segnum, false, false))
					seg->special = 0;
				break;
			case SEGMENT_IS_CONTROLCEN:
				if (!AddReactor (segnum, false, false))
					seg->special = 0;
				break;
			default:
				break;
			}
		}
	else {
		seg->special = 0;
		seg->matcen_num = -1;
		seg->value = -1;
		}
	//        fscanf (fBlk,"  child_bitmask %d\n",&test);
	//        seg->child_bitmask = test & 0x3f;
	//        fscanf (fBlk,"  wall_bitmask %d\n",&test);
	//        seg->wall_bitmask  = (test & 0x3f) | MARKED_MASK;
	seg->wall_bitmask = MARKED_MASK; // no other bits
	// calculate child_bitmask
	seg->child_bitmask = 0;
	for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
		if (seg->children [i] >= 0)
		seg->child_bitmask |= (1 << i);
	SegCount ()++;
	nNewSegs++;
	}

CDTrigger *trigger = Triggers (GameInfo ().triggers.count);
for (i = nNewTriggers; i; i--) {
	trigger--;
	for (j = 0; j < trigger->num_links; j++)
		if (trigger->seg [j] >= 0)
			trigger->seg [j] = xlatSegNum [trigger->seg [j]];
		else if (trigger->num_links == 1) {
			DeleteTrigger (trigger - Triggers ());
			i--;
			}
		else if (j < --(trigger->num_links)) {
			memcpy (trigger->seg + j, trigger->seg + j + 1, (trigger->num_links - j) * sizeof (*(trigger->seg)));
			memcpy (trigger->side + j, trigger->side + j + 1, (trigger->num_links - j) * sizeof (*(trigger->side)));
			}
	}

sprintf (message,
			" Block tool: %d blocks, %d walls, %d triggers pasted.", 
			nNewSegs, nNewWalls, nNewTriggers);
DEBUGMSG (message);
return (nNewSegs);
}

//---------------------------------------------------------------------------
// dump_seg_info()
//
// ACTION - Writes a segment's information in text form to a fBlk.  Uses
//          actual coordinate information instead of vertex number.  Only
//          saves segment information (no Walls (), Objects (), or Triggers ()).
//
// Change - Now saves verts relative to the current side (point 0)
//          Uses x',y',z' axis where:
//          y' is in the neg dirction of line0
//          z' is in the neg direction orthogonal to line0 & line3
//          x' is in the direction orghogonal to x' and y'
//
//---------------------------------------------------------------------------

void CMine::WriteSegmentInfo (FILE *fBlk, INT16 /*segnum*/) 
{
	INT16				segnum;
	CDSegment		*seg;
	CDSide			*side;
	CDWall			*wall;
	INT16				i,j;
	vms_vector		origin;
	struct dvector	x_prime,y_prime,z_prime,vect;
	INT16				vertnum;
	double			length;

#if DEMO
ErrorMsg ("You cannot save a mine in the demo.");
return;
#endif
// set origin
seg = CurrSeg ();
vertnum = seg->verts[side_vert[Current ()->side][CURRENT_POINT(0)]];
origin.x = Vertices (vertnum)->x;
origin.y = Vertices (vertnum)->y;
origin.z = Vertices (vertnum)->z;

// set x'
vertnum = seg->verts[side_vert[Current ()->side][CURRENT_POINT(1)]];
x_prime.x = (double)(Vertices (vertnum)->x - origin.x);
x_prime.y = (double)(Vertices (vertnum)->y - origin.y);
x_prime.z = (double)(Vertices (vertnum)->z - origin.z);

// calculate y'
vertnum = seg->verts[side_vert[Current ()->side][CURRENT_POINT(3)]];
vect.x = (double)(Vertices (vertnum)->x - origin.x);
vect.y = (double)(Vertices (vertnum)->y - origin.y);
vect.z = (double)(Vertices (vertnum)->z - origin.z);
y_prime.x = x_prime.y*vect.z - x_prime.z*vect.y;
y_prime.y = x_prime.z*vect.x - x_prime.x*vect.z;
y_prime.z = x_prime.x*vect.y - x_prime.y*vect.x;

// calculate z'
z_prime.x = x_prime.y*y_prime.z - x_prime.z*y_prime.y;
z_prime.y = x_prime.z*y_prime.x - x_prime.x*y_prime.z;
z_prime.z = x_prime.x*y_prime.y - x_prime.y*y_prime.x;

// normalize all
length = sqrt(x_prime.x*x_prime.x + x_prime.y*x_prime.y + x_prime.z*x_prime.z);
x_prime.x /= length;
x_prime.y /= length;
x_prime.z /= length;
length = sqrt(y_prime.x*y_prime.x + y_prime.y*y_prime.y + y_prime.z*y_prime.z);
y_prime.x /= length;
y_prime.y /= length;
y_prime.z /= length;
length = sqrt(z_prime.x*z_prime.x + z_prime.y*z_prime.y + z_prime.z*z_prime.z);
z_prime.x /= length;
z_prime.y /= length;
z_prime.z /= length;

#if 0
  sprintf(message,"x'=(%0.3f,%0.3f,%0.3f)\n"
		  "y'=(%0.3f,%0.3f,%0.3f)\n"
		  "z'=(%0.3f,%0.3f,%0.3f)\n",
		  (float)x_prime.x,(float)x_prime.y,(float)x_prime.z,
		  (float)y_prime.x,(float)y_prime.y,(float)y_prime.z,
		  (float)z_prime.x,(float)z_prime.y,(float)z_prime.z);
  DebugMsg(message);
#endif

seg = Segments ();
for (segnum = 0; segnum < SegCount (); segnum++, seg++) {
	if (seg->wall_bitmask & MARKED_MASK) {
		fprintf (fBlk,"segment %d\n",segnum);
		side = seg->sides;
		for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++, side++) {
			fprintf (fBlk,"  side %d\n",i);
			fprintf (fBlk,"    tmap_num %d\n",side->nBaseTex);
			fprintf (fBlk,"    tmap_num2 %d\n",side->nOvlTex);
			for (j = 0; j < 4; j++) {
				fprintf (fBlk,"    uvls %d %d %d\n",
				side->uvls [j].u,
				side->uvls [j].v,
				side->uvls [j].l);
				}
			if (bExtBlkFmt) {
				fprintf (fBlk, "    nWall %d\n", 
							(side->nWall < GameInfo ().walls.count) ? side->nWall : NO_WALL);
				if (side->nWall < GameInfo ().walls.count) {
					wall = Walls (side->nWall);
					fprintf (fBlk, "        segment %d\n", wall->segnum);
					fprintf (fBlk, "        side %d\n", wall->sidenum);
					fprintf (fBlk, "        hps %d\n", wall->hps);
					fprintf (fBlk, "        type %d\n", wall->type);
					fprintf (fBlk, "        flags %d\n", wall->flags);
					fprintf (fBlk, "        state %d\n", wall->state);
					fprintf (fBlk, "        clip_num %d\n", wall->clip_num);
					fprintf (fBlk, "        keys %d\n", wall->keys);
					fprintf (fBlk, "        cloak %d\n", wall->cloak_value);
					if ((wall->trigger < 0) || (wall->trigger >= GameInfo ().triggers.count))
						fprintf (fBlk, "        trigger %u\n", NO_TRIGGER);
					else {
						CDTrigger *trigger = Triggers (wall->trigger);
						int iTarget;
						int nTargets = 0;
						INT16 *tgtSegs = trigger->seg;
						// count trigger targets in marked area
						for (iTarget = 0; iTarget < trigger->num_links; iTarget++, tgtSegs++)
							if (Segments (*tgtSegs)->wall_bitmask & MARKED_MASK)
								nTargets++;
#if 0
						if (trigger->num_links && !nTargets)	// no targets in marked area
							fprintf (fBlk, "        trigger %d\n", MAX_TRIGGERS);
						else 
#endif
							{
							fprintf (fBlk, "        trigger %d\n", wall->trigger);
							fprintf (fBlk, "			    type %d\n", trigger->type);
							fprintf (fBlk, "			    flags %ld\n", trigger->flags);
							fprintf (fBlk, "			    value %ld\n", trigger->value);
							fprintf (fBlk, "			    timer %d\n", trigger->time);
							fprintf (fBlk, "			    num_links %d\n", nTargets);
							tgtSegs = trigger->seg;
							for (iTarget = 0; iTarget < trigger->num_links; iTarget++, tgtSegs++)
								if (Segments (*tgtSegs)->wall_bitmask & MARKED_MASK) {
									fprintf (fBlk, "			        seg %d\n", *tgtSegs);
									fprintf (fBlk, "			        side %d\n", trigger->side [iTarget]);
									}
							}
						}
					}
				}
			}
		fprintf (fBlk,"  children");
		for (i = 0; i < 6; i++)
			fprintf (fBlk, " %d", seg->children [i]);
		fprintf (fBlk, "\n");
		// save vertices
		for (i = 0; i < 8; i++) {
			// each vertex relative to the origin has a x', y', and z' component
			// which is a constant (k) times the axis
			// k = (B*A)/(A*A) where B is the vertex relative to the origin
			//                       A is the axis unit vector (always 1)
			vertnum = seg->verts [i];
			vect.x = (double) (Vertices (vertnum)->x - origin.x);
			vect.y = (double) (Vertices (vertnum)->y - origin.y);
			vect.z = (double) (Vertices (vertnum)->z - origin.z);
			fprintf (fBlk,"  vms_vector %d %ld %ld %ld\n",i,
						(FIX)(vect.x*x_prime.x + vect.y*x_prime.y + vect.z*x_prime.z),
						(FIX)(vect.x*y_prime.x + vect.y*y_prime.y + vect.z*y_prime.z),
						(FIX)(vect.x*z_prime.x + vect.y*z_prime.y + vect.z*z_prime.z));
			}
		fprintf (fBlk,"  static_light %ld\n",seg->static_light);
		if (bExtBlkFmt) {
			fprintf (fBlk,"  special %d\n",seg->special);
			fprintf (fBlk,"  matcen_num %d\n",seg->matcen_num);
			fprintf (fBlk,"  value %d\n",seg->value);
			fprintf (fBlk,"  child_bitmask %d\n",seg->child_bitmask);
			fprintf (fBlk,"  wall_bitmask %d\n",seg->wall_bitmask);
			}
		}
	}
}

//==========================================================================
// MENU - Cut
//==========================================================================

void CMine::CutBlock()
{
  FILE *fBlk;
  INT16 segnum;
  INT16 count;
  char szFile [256] = "\0";

if (m_bSplineActive) {
	ErrorMsg(spline_error_message);
	return;
	}

  // make sure some cubes are marked
count = MarkedSegmentCount ();
if (count==0) {
	ErrorMsg("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

//  if (disable_saves) {
//    ErrorMsg("Saves disabled, contact Interplay for your security number.");
//    return;
//  }

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;
#if 1
if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
	return;
#else
  // Initialize data for fBlk open dialog
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DMB Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "blk";
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
  if (!GetSaveFileName(&ofn)) {
	return;
  }
  strcpy (szFile, ofn.lpstrFile);
#endif
bExtBlkFmt = strstr (strlwr (szFile), ".blx") != NULL;
fBlk = fopen (szFile, "w");
if (!fBlk) {
	ErrorMsg("Unable to open block file");
	return;
	}
//UpdateUndoBuffer(0);
strcpy(m_szBlockFile, szFile); // remember file for quick paste
fprintf (fBlk, bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
WriteSegmentInfo (fBlk, 0);
// delete Segments () from last to first because SegCount ()
// is effected for each deletion.  When all Segments () are marked
// the SegCount () will be decremented for each segnum in loop.
theApp.SetModified (TRUE);
theApp.LockUndo ();
CDSegment *seg = Segments () + SegCount ();
for (segnum = SegCount () - 1; segnum; segnum--)
    if ((--seg)->wall_bitmask & MARKED_MASK) {
		if (SegCount () <= 1)
			break;
		DeleteSegment (segnum); // delete seg w/o asking "are you sure"
		}
theApp.UnlockUndo ();
fclose(fBlk);
sprintf(message," Block tool: %d blocks cut to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
  // wrap back then forward to make sure segment is valid
wrap(&Current1 ().segment,-1,0,SegCount () - 1);
wrap(&Current2 ().segment,1,0,SegCount () - 1);
wrap(&Current2 ().segment,-1,0,SegCount () - 1);
wrap(&Current2 ().segment,1,0,SegCount () - 1);
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// MENU - Copy
//==========================================================================

void CMine::CopyBlock(char *pszBlockFile)
{
  FILE *fBlk;
  char szFile [256] = "\0";
  INT16 count;

  // make sure some cubes are marked
count = MarkedSegmentCount ();
if (count==0) {
	ErrorMsg("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

//  if (disable_saves) {
//    ErrorMsg("Saves disabled, contact Interplay for your security number.");
//    return;
//  }

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;
#if 1
if (pszBlockFile && *pszBlockFile)
	strcpy (szFile, pszBlockFile);
else {
	strcpy (szFile, m_szBlockFile);
	if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
		return;
	}
#else
  // Initialize data for fBlk open dialog
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DLE-XP Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "blk";
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

  if (!GetSaveFileName(&ofn)) {
    return;
  strcpy (szFile, ofn.lpstrFile);
  }
#endif
bExtBlkFmt = strstr (strlwr (szFile), ".blx") != NULL;
fBlk = fopen (szFile, "w");
if (!fBlk) {
	sprintf(message,"Unable to open block file '%s'", szFile);
	ErrorMsg(message);
	return;
	}
//  UpdateUndoBuffer(0);
strcpy(m_szBlockFile, szFile); // remember fBlk for quick paste
fprintf (fBlk, bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
WriteSegmentInfo (fBlk, 0);
fclose (fBlk);
sprintf(message," Block tool: %d blocks copied to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// MENU - Paste
//==========================================================================

void CMine::PasteBlock() 
{
if (m_bSplineActive) {
	ErrorMsg(spline_error_message);
	return;
	}
// Initialize data for fBlk open dialog
  char szFile [256] = "\0";

#if 1
if (!BrowseForFile (TRUE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
						  theApp.MainFrame ()))
	return;
#else
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DLE-XP Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (!GetOpenFileName(&ofn)) {
    return;
  }
#endif
//  UpdateUndoBuffer(0);

if (!ReadBlock (szFile, 0))
	theApp.MineView ()->SetSelectMode (BLOCK_MODE);
}

//==========================================================================
// read_block_file()
//
// returns 0 on success
// if option == 1, then "x blocks pasted" message is suppressed
//==========================================================================

int CMine::ReadBlock (char *pszBlockFile,int option) 
{
	CDSegment *seg,*seg2;
	INT16 segnum,seg_offset;
	INT16 count,child;
	INT16 vertnum;
	FILE *fBlk;

fBlk = fopen (strlwr (pszBlockFile), "r");
if (!fBlk) {
	ErrorMsg ("Unable to open block file");
	return 1;
	}	

fscanf (fBlk,"%s\n",&message);
if (!strncmp (message, "DMB_BLOCK_FILE", 14))
	bExtBlkFmt = false;
else if (!strncmp (message, "DMB_EXT_BLOCK_FILE", 18))
	bExtBlkFmt = true;
else {
	ErrorMsg ("This is not a block file.");
	fclose (fBlk);
	return 2;
	}

strcpy (m_szBlockFile, pszBlockFile); // remember file for quick paste

// unmark all Segments ()
// set up all seg_numbers (makes sure there are no negative seg_numbers)
theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
seg = Segments ();
for (segnum = 0;segnum < MAX_SEGMENTS; segnum++, seg++) {
	seg->seg_number = segnum;
	seg->wall_bitmask &= ~MARKED_MASK;
	}

// unmark all vertices
for (vertnum = 0; vertnum < MAX_VERTICES; vertnum++) {
	*VertStatus (vertnum) &= ~MARKED_MASK;
	*VertStatus (vertnum) &= ~NEW_MASK;
	}
count = ReadSegmentInfo (fBlk);

// fix up the new Segments () children
seg = Segments ();
for (segnum = 0; segnum < SegCount (); segnum++, seg++) {
	if (seg->seg_number < 0) {  // if segment was just inserted
		// if child has a segment number that was just inserted, set it to the
		//  segment's offset number, otherwise set it to -1
		for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
			if (seg->child_bitmask & (1 << child)) {
				seg2 = Segments ();
				for (seg_offset = 0; seg_offset < SegCount (); seg_offset++, seg2++) {
					if (seg->children [child] == ~seg2->seg_number) {
						seg->children [child] = seg_offset;
						break;
						}
					}
				if (seg_offset == SegCount ()) { // no child found
					ResetSide (segnum,child);
					// auto link the new segment with any touching Segments ()
					seg2 = Segments ();
					int segnum2, sidenum2;
					for (segnum2 = 0; segnum2 < SegCount (); segnum2++, seg2++) {
						if (segnum != segnum2) {
							// first check to see if Segments () are any where near each other
							// use x, y, and z coordinate of first point of each segment for comparison
							vms_vector *v1 = Vertices (seg ->verts [0]);
							vms_vector *v2 = Vertices (seg2->verts [0]);
							if (labs (v1->x - v2->x) < 0xA00000L &&
								 labs (v1->y - v2->y) < 0xA00000L &&
								 labs (v1->z - v2->z) < 0xA00000L) {
								for (sidenum2 = 0;sidenum2 < 6; sidenum2++) {
									LinkSegments (segnum, child, segnum2, sidenum2, 3 * F1_0);
									}
								}
							}
						}
					}
				} 
			else {
				seg->children [child] = -1; // force child to agree with bitmask
				}
			}
		}
	}
// clear all new vertices as such
for (vertnum=0;vertnum<MAX_VERTICES;vertnum++)
	*VertStatus (vertnum) &= ~NEW_MASK;
// now set all seg_numbers
seg = Segments ();
for (segnum = 0; segnum < SegCount (); segnum++, seg++)
	seg->seg_number = segnum;
/*
if (option != 1) {
	sprintf (message," Block tool: %d blocks pasted.",count);
	DEBUGMSG (message);
	}
*/
fclose(fBlk);
//theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return 0;
}

//==========================================================================
// MENU - Quick Paste
//==========================================================================

void CMine::QuickPasteBlock ()
{
if (!*m_szBlockFile) {
	PasteBlock ();
//	ErrorMsg("You must first use one of the cut or paste commands\n"
//				"before you use the Quick Paste command");
	return;
	}

if (m_bSplineActive) {
	ErrorMsg(spline_error_message);
	return;
	}

//UpdateUndoBuffer(0);

if (!ReadBlock (m_szBlockFile, 1))
	theApp.MineView ()->SetSelectMode (BLOCK_MODE);
}

//==========================================================================
// MENU - Delete Block
//==========================================================================

void CMine::DeleteBlock()
{

INT16 segnum,count;

if (m_bSplineActive) {
	ErrorMsg(spline_error_message);
	return;
	}
// make sure some cubes are marked
count = MarkedSegmentCount ();
if (!count) {
	ErrorMsg("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);

// delete Segments () from last to first because SegCount ()
// is effected for each deletion.  When all Segments () are marked
// the SegCount () will be decremented for each segnum in loop.
if (QueryMsg("Are you sure you want to delete the marked cubes?")!=IDYES)
	return;

for (segnum=SegCount ()-1;segnum>=0;segnum--)
	if (Segments (segnum)->wall_bitmask & MARKED_MASK) {
		if (SegCount () <= 1)
			break;
		if (Objects () [0].segnum != segnum)
			DeleteSegment (segnum); // delete seg w/o asking "are you sure"
		}
// wrap back then forward to make sure segment is valid
wrap(&Current1 ().segment,-1,0,SegCount () - 1);
wrap(&Current2 ().segment,1,0,SegCount () - 1);
wrap(&Current2 ().segment,-1,0,SegCount () - 1);
wrap(&Current2 ().segment,1,0,SegCount () - 1);
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
}

//eof block.cpp