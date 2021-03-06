/* $Id: gamemine.h,v 1.6 2003/10/10 09:36:35 btb Exp $ */
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
 * Header for gamemine.c
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:57:02  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:28:50  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.15  1994/11/17  11:39:34  matt
 * Ripped out code to load old mines
 *
 * Revision 1.14  1994/10/19  16:46:40  matt
 * Made tmap overrides for robots remap texture numbers
 *
 * Revision 1.13  1994/09/23  22:14:37  matt
 * Took out obsolete structure fields
 *
 * Revision 1.12  1994/07/22  12:36:32  matt
 * Cleaned up editor/game interactions some more.
 *
 * Revision 1.11  1994/06/01  11:21:37  yuan
 * Added controlcentertriggers to the gamesave.
 *
 * Revision 1.10  1994/05/10  12:14:26  yuan
 * Game save/load... Demo levels 1-5 added...
 * High scores fixed...
 *
 * Revision 1.9  1994/05/04  18:25:05  yuan
 * Working on gamesave.
 *
 * Revision 1.8  1994/05/03  15:53:27  yuan
 * Readded structure field groups for backward compatibility...
 *
 * Revision 1.7  1994/05/03  11:36:26  yuan
 * Stabilizing gamesave stuff.
 *
 * Revision 1.6  1994/04/28  11:01:32  yuan
 * Added objects in structure.
 *
 * Revision 1.5  1994/04/06  18:30:03  john
 * Added Refueling segments.
 *
 * Revision 1.4  1994/03/17  18:06:53  yuan
 * Removed switch code... Now we just have Walls, Triggers, and Links...
 *
 * Revision 1.3  1994/03/01  18:12:24  yuan
 * Wallswitches, triggers, and more!
 *
 * Revision 1.2  1994/02/10  15:36:01  matt
 * Various changes to make editor compile out.
 *
 * Revision 1.1  1994/02/09  17:11:45  matt
 * Initial revision
 *
 *
 */


#ifndef _GAMEMINE_H
#define _GAMEMINE_H

#define MINE_VERSION        20  // Current version expected
#define COMPATIBLE_VERSION  16  // Oldest version that can safely be loaded.

struct mtfi {
	ushort  fileinfo_signature;
	ushort  fileinfo_version;
	int     fileinfo_sizeof;
};    // Should be same as first two fields below...

struct mfi {
	ushort  fileinfo_signature;
	ushort  fileinfo_version;
	int     fileinfo_sizeof;
	int     header_offset;      // Stuff common to game & editor
	int     header_size;
	int     editor_offset;      // Editor specific stuff
	int     editor_size;
	int     segment_offset;
	int     segment_howmany;
	int     segment_sizeof;
	int     newseg_verts_offset;
	int     newseg_verts_howmany;
	int     newseg_verts_sizeof;
	int     group_offset;
	int     group_howmany;
	int     group_sizeof;
	int     vertex_offset;
	int     vertex_howmany;
	int     vertex_sizeof;
	int     texture_offset;
	int     texture_howmany;
	int     texture_sizeof;
	int     walls_offset;
	int     walls_howmany;
	int     walls_sizeof;
	int     triggers_offset;
	int     triggers_howmany;
	int     triggers_sizeof;
	int     links_offset;
	int     links_howmany;
	int     links_sizeof;
	int     object_offset;      // Object info
	int     object_howmany;
	int     object_sizeof;
	int     unused_offset;      // was: doors_offset
	int     unused_howmamy;     // was: doors_howmany
	int     unused_sizeof;      // was: doors_sizeof
	short   level_shake_frequency, level_shake_duration;
	// Shakes every level_shake_frequency seconds
	// for level_shake_duration seconds (on average, random).  In 16ths second.
	int     secret_return_segment;
	vmsMatrix secret_return_orient;

	int     dl_indices_offset;
	int     dl_indices_howmany;
	int     dl_indices_sizeof;

	int     delta_light_offset;
	int     delta_light_howmany;
	int     delta_light_sizeof;

	int     segment2_offset;
	int     segment2_howmany;
	int     segment2_sizeof;

};

struct mh {
	int num_vertices;
	int num_segments;
};

struct me {
	int current_seg;
	int newsegment_offset;
	int newsegment_size;
	int Curside;
	int Markedsegp;
	int Markedside;
	int Groupsegp[10];
	int Groupside[10];
	int num_groups;
	int current_group;
	//int numObjects;
};

extern struct mtfi mine_top_fileinfo;   // Should be same as first two fields below...
extern struct mfi mine_fileinfo;
extern struct mh mine_header;
extern struct me mine_editor;

// returns 1 if error, else 0
int game_load_mine(char * filename);

// loads from an already-open file
// returns 0=everything ok, 1=old version, -1=error
int load_mine_data(CFILE *LoadFile);
int LoadMineSegmentsCompiled(CFILE *LoadFile);

extern short tmap_xlate_table[];
extern fix Level_shake_frequency, Level_shake_duration;
extern int Secret_return_segment;
extern vmsMatrix Secret_return_orient;

/* stuff for loading descent.pig of descent 1 */
extern short ConvertD1Texture(short d1_tmap_num, int bForce);
extern int d1_tmap_num_unique(short d1_tmap_num); //is d1_tmap_num's texture only in d1?

int LoadMineGaugeSize ();

#endif /* _GAMEMINE_H */
