/* $Id: wall.h,v 1.7 2003/10/10 09:36:35 btb Exp $ */
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
 * Header for wall.c
 *
 * Old Log:
 * Revision 1.1  1995/05/16  16:05:14  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:31:36  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.47  1994/11/19  15:20:35  mike
 * rip out unused code and data
 *
 * Revision 1.46  1994/10/25  15:40:12  yuan
 * MAX_WALLS pumped up
 *
 * Revision 1.45  1994/10/23  19:17:07  matt
 * Fixed bug with "no key" messages
 *
 * Revision 1.44  1994/10/18  15:38:03  mike
 * Define hidden walls.
 *
 * Revision 1.43  1994/10/04  13:32:26  adam
 * commented out MAX_DOOR_ANIMS
 *
 * Revision 1.42  1994/10/04  13:31:21  adam
 * upped MAX_WALL_ANIMS to 30
 *
 * Revision 1.41  1994/09/29  00:42:30  matt
 * Made hitting a locked door play a sound
 *
 * Revision 1.40  1994/09/27  15:42:41  mike
 * Prototype Wall_names.
 *
 * Revision 1.39  1994/09/23  22:15:32  matt
 * Made doors not close on objects, made doors open again if shot while
 * closing, and cleaned up walls/doors a bit.
 *
 * Revision 1.38  1994/09/22  15:31:33  matt
 * Mucked with, and hopefully improved, exploding walls
 *
 * Revision 1.37  1994/09/21  17:17:05  mike
 * Make objects stuck in doors go away when door opens.
 *
 * Revision 1.36  1994/09/13  21:10:46  matt
 * Added wclips that use tmap1 instead of tmap2, saving lots of merging
 *
 * Revision 1.35  1994/09/13  20:11:08  yuan
 * *** empty log message ***
 *
 * Revision 1.34  1994/09/10  13:31:53  matt
 * Made exploding walls a type of blastable walls.
 * Cleaned up blastable walls, making them tmap2 bitmaps.
 *
 * Revision 1.33  1994/08/17  12:55:34  matt
 * Added external walls to WallIsDoorWay
 *
 * Revision 1.32  1994/08/15  17:54:35  john
 * *** empty log message ***
 *
 * Revision 1.31  1994/08/15  17:46:56  yuan
 * Added external walls, fixed blastable walls.
 *
 * Revision 1.30  1994/08/05  21:17:21  matt
 * Allow two doors to be linked together
 *
 * Revision 1.29  1994/08/01  10:39:10  matt
 * Parenthesized parms to WID() macro
 *
 * Revision 1.28  1994/07/20  17:35:03  yuan
 * Some minor bug fixes and new key gauges...
 *
 * Revision 1.27  1994/07/19  14:32:03  yuan
 * Fixed keys bug... renumbered some constants.
 *
 * Revision 1.26  1994/07/14  22:38:29  matt
 * Added exploding doors
 *
 * Revision 1.25  1994/07/11  15:08:43  yuan
 * Wall anim file names stored in structure.
 *
 */

#ifndef _WALL_H
#define _WALL_H

#include "inferno.h"
#include "segment.h"
#include "object.h"
#include "cfile.h"

//#include "vclip.h"

#define MAX_WALLS               2047 // Maximum number of walls
#define MAX_WALL_ANIMS          60  // Maximum different types of doors
#define D1_MAX_WALL_ANIMS       30  // Maximum different types of doors
#define MAX_DOORS               90  // Maximum number of open doors
#define MAX_CLOAKING_WALLS		  100
#define MAX_EXPLODING_WALLS     10

// Various wall types.
#define WALL_NORMAL             0   // Normal wall
#define WALL_BLASTABLE          1   // Removable (by shooting) wall
#define WALL_DOOR               2   // Door
#define WALL_ILLUSION           3   // Wall that appears to be there, but you can fly thru
#define WALL_OPEN               4   // Just an open side. (Trigger)
#define WALL_CLOSED             5   // Wall.  Used for transparent walls.
#define WALL_OVERLAY            6   // Goes over an actual solid side.  For triggers
#define WALL_CLOAKED            7   // Can see it, and see through it
#define WALL_TRANSPARENT        8   // like cloaked, but fixed transparency and colored

// Various wall flags.
#define WALL_BLASTED            1   // Blasted out wall.
#define WALL_DOOR_OPENED        2   // Open door.
#define WALL_DOOR_LOCKED        8   // Door is locked.
#define WALL_DOOR_AUTO          16  // Door automatically closes after time.
#define WALL_ILLUSION_OFF       32  // Illusionary wall is shut off.
#define WALL_WALL_SWITCH        64  // This wall is openable by a wall switch.
#define WALL_BUDDY_PROOF        128 // Buddy assumes he cannot get through this wall.

// Wall states
#define WALL_DOOR_CLOSED        0       // Door is closed
#define WALL_DOOR_OPENING       1       // Door is opening.
#define WALL_DOOR_WAITING       2       // Waiting to close
#define WALL_DOOR_CLOSING       3       // Door is closing
#define WALL_DOOR_OPEN          4       // Door is open, and staying open
#define WALL_DOOR_CLOAKING      5       // Wall is going from closed -> open
#define WALL_DOOR_DECLOAKING    6       // Wall is going from open -> closed

//note: a door is considered opened (i.e., it has WALL_OPENED set) when it
//is more than half way open.  Thus, it can have any of OPENING, CLOSING,
//or WAITING bits set when OPENED is set.

#define KEY_NONE                1
#define KEY_BLUE                2
#define KEY_RED                 4
#define KEY_GOLD                8

#define WALL_HPS                100*F1_0    // Normal wall's hp
#define WALL_DOOR_INTERVAL      5*F1_0      // How many seconds a door is open

#define DOOR_OPEN_TIME          i2f(2)      // How long takes to open
#define DOOR_WAIT_TIME          i2f(5)      // How long before auto door closes

#define MAX_CLIP_FRAMES         50
#define D1_MAX_CLIP_FRAMES      20

// WALL_IS_DOORWAY flags.
#define WID_FLY_FLAG            1
#define WID_RENDER_FLAG         2
#define WID_RENDPAST_FLAG       4
#define WID_EXTERNAL_FLAG       8
#define WID_CLOAKED_FLAG        16
#define WID_TRANSPARENT_FLAG    32

//@@//  WALL_IS_DOORWAY return values          F/R/RP
//@@#define WID_WALL                    2   // 0/1/0        wall
//@@#define WID_TRANSPARENT_WALL        6   // 0/1/1        transparent wall
//@@#define WID_ILLUSORY_WALL           3   // 1/1/0        illusory wall
//@@#define WID_TRANSILLUSORY_WALL      7   // 1/1/1        transparent illusory wall
//@@#define WID_NO_WALL                 5   //  1/0/1       no wall, can fly through
//@@#define WID_EXTERNAL                8   // 0/0/0/1  don't see it, dont fly through it

#define MAX_STUCK_OBJECTS   32

typedef struct stuckobj {
	short   objnum, wallnum;
	int     signature;
} stuckobj;

//Start old wall structures

typedef struct v16_wall {
	sbyte   type;               // What kind of special wall.
	sbyte   flags;              // Flags for the wall.
	fix     hps;                // "Hit points" of the wall.
	sbyte   trigger;            // Which trigger is associated with the wall.
	sbyte   clip_num;           // Which animation associated with the wall.
	sbyte   keys;
} __pack__ v16_wall;

typedef struct v19_wall {
	int     segnum,sidenum;     // Seg & side for this wall
	sbyte   type;               // What kind of special wall.
	sbyte   flags;              // Flags for the wall.
	fix     hps;                // "Hit points" of the wall.
	sbyte   trigger;            // Which trigger is associated with the wall.
	sbyte   clip_num;           // Which animation associated with the wall.
	sbyte   keys;
	int linked_wall;            // number of linked wall
} __pack__ v19_wall;

typedef struct v19_door {
	int     n_parts;            // for linked walls
	short   seg[2];             // Segment pointer of door.
	short   side[2];            // Side number of door.
	short   type[2];            // What kind of door animation.
	fix     open;               // How long it has been open.
} __pack__ v19_door;

//End old wall structures

typedef struct wall {
	int     segnum,sidenum;     // Seg & side for this wall
	fix     hps;                // "Hit points" of the wall.
	int     linked_wall;        // number of linked wall
	ubyte   type;               // What kind of special wall.
	ubyte   flags;              // Flags for the wall.
	ubyte   state;              // Opening, closing, etc.
	ubyte   trigger;            // Which trigger is associated with the wall.
	sbyte   clip_num;           // Which animation associated with the wall.
	ubyte   keys;               // which keys are required
	sbyte   controlling_trigger;// which trigger causes something to happen here.  Not like "trigger" above, which is the trigger on this wall.
                                //  Note: This gets stuffed at load time in gamemine.c.  Don't try to use it in the editor.  You will be sorry!
	sbyte   cloak_value;        // if this wall is cloaked, the fade value
} __pack__ wall;

typedef struct active_door {
	int     n_parts;            // for linked walls
	short   front_wallnum[2];   // front wall numbers for this door
	short   back_wallnum[2];    // back wall numbers for this door
	fix     time;               // how long been opening, closing, waiting
} __pack__ active_door;

// data for exploding walls (such as hostage door)
typedef struct expl_wall {
	int	segnum, sidenum;
	fix	time;
} expl_wall;

typedef struct cloaking_wall {
	short   front_wallnum;  // front wall numbers for this door
	short   back_wallnum;   // back wall numbers for this door
	fix     front_ls[4];        // front wall saved light values
	fix     back_ls[4];         // back wall saved light values
	fix     time;               // how long been cloaking or decloaking
} __pack__ cloaking_wall;

//wall clip flags
#define WCF_EXPLODES    1       //door explodes when opening
#define WCF_BLASTABLE   2       //this is a blastable wall
#define WCF_TMAP1       4       //this uses primary tmap, not tmap2
#define WCF_HIDDEN      8       //this uses primary tmap, not tmap2
#define WCF_ALTFMT		16
#define WCF_FROMPOG		32
#define WCF_INITIALIZED	64

typedef struct {
	fix     xTotalTime;
	short   nFrameCount;
	short   frames[MAX_CLIP_FRAMES];
	short   open_sound;
	short   close_sound;
	short   flags;
	char    filename[13];
	char    pad;
} __pack__ wclip;

typedef struct {
	fix     play_time;
	short   num_frames;
	short   frames[D1_MAX_CLIP_FRAMES];
	short   open_sound;
	short   close_sound;
	short   flags;
	char    filename[13];
	char    pad;
} __pack__ D1_wclip;

extern char pszWallNames[7][10];

//#define WALL_IS_DOORWAY(seg,side) WallIsDoorWay(seg, side)

extern int WALL_IS_DOORWAY (segment *seg, short side, object *objP);

// Initializes all walls (i.e. no special walls.)
extern void WallInit();

// Automatically checks if a there is a doorway (i.e. can fly through)
extern int WallIsDoorWay ( segment *seg, short side );

// Deteriorate appearance of wall. (Changes bitmap (paste-ons))
extern void WallDamage(segment *seg, short side, fix damage);

// Destroys a blastable wall. (So it is an opening afterwards)
extern void WallDestroy(segment *seg, short side);

void WallIllusionOn(segment *seg, short side);

void WallIllusionOff(segment *seg, short side);

// Opens a door, including animation and other processing.
void DoDoorOpen(int door_num);

// Closes a door, including animation and other processing.
void DoDoorClose(int door_num);

// Opens a door
extern void WallOpenDoor(segment *seg, short side);

// Closes a door
extern void WallCloseDoor(segment *seg, short side);

//return codes for WallHitProcess()
#define WHP_NOT_SPECIAL     0       //wasn't a quote-wall-unquote
#define WHP_NO_KEY          1       //hit door, but didn't have key
#define WHP_BLASTABLE       2       //hit blastable wall
#define WHP_DOOR            3       //a door (which will now be opening)

int AnimFrameCount (wclip *anim);

// Determines what happens when a wall is shot
//obj is the object that hit...either a weapon or the player himself
extern int WallHitProcess(segment *seg, short side, fix damage, int playernum, object *obj );

// Opens/destroys specified door.
extern void WallToggle(segment *seg, short side);

// Tidy up Walls array for load/save purposes.
extern void ResetWalls();

extern void UnlockAllWalls (int bOnlyDoors);

// Called once per frame..
void WallFrameProcess();

extern stuckobj Stuck_objects[MAX_STUCK_OBJECTS];

//  An object got stuck in a door (like a flare).
//  Add global entry.
extern void AddStuckObject(object *objp, short segnum, short sidenum);
extern void RemoveObsoleteStuckObjects(void);

//set the tmap_num or tmap_num2 field for a wall/door
extern void WallSetTMapNum(segment *seg,short side,segment *csegp, short cside,int anim_num,int frame_num);

extern void InitDoorAnims (void);

// Remove any flares from a wall
void KillStuckObjects(int wallnum);

//start wall open <-> closed transitions
void StartWallCloak(segment *seg, short side);
void StartWallDecloak(segment *seg, short side);

extern int wclip_read_n_d1(wclip *wc, int n, CFILE *fp);
#ifdef FAST_FILE_IO
#define wclip_read_n(wc, n, fp) CFRead(wc, sizeof(wclip), n, fp)
#define v16_wall_read(w, fp) CFRead(w, sizeof(v16_wall), 1, fp)
#define v19_wall_read(w, fp) CFRead(w, sizeof(v19_wall), 1, fp)
#define wall_read(w, fp) CFRead(w, sizeof(wall), 1, fp)
#define v19_door_read(d, fp) CFRead(d, sizeof(v19_door), 1, fp)
#define active_door_read(d, fp) CFRead(d, sizeof(active_door), 1, fp)
#else
/*
 * reads n wclip structs from a CFILE
 */
extern int wclip_read_n(wclip *wc, int n, CFILE *fp);

/*
 * reads a v16_wall structure from a CFILE
 */
extern void v16_wall_read(v16_wall *w, CFILE *fp);

/*
 * reads a v19_wall structure from a CFILE
 */
extern void v19_wall_read(v19_wall *w, CFILE *fp);

/*
 * reads a wall structure from a CFILE
 */
extern void wall_read(wall *w, CFILE *fp);

/*
 * reads a v19_door structure from a CFILE
 */
extern void v19_door_read(v19_door *d, CFILE *fp);

/*
 * reads an active_door structure from a CFILE
 */
extern void active_door_read(active_door *ad, CFILE *fp);
#endif

#endif
