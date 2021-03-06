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

// Various tWall types.
#define WALL_NORMAL             0   // Normal tWall
#define WALL_BLASTABLE          1   // Removable (by shooting) tWall
#define WALL_DOOR               2   // Door
#define WALL_ILLUSION           3   // Wall that appears to be there, but you can fly thru
#define WALL_OPEN               4   // Just an open tSide. (Trigger)
#define WALL_CLOSED             5   // Wall.  Used for transparent walls.
#define WALL_OVERLAY            6   // Goes over an actual solid tSide.  For triggers
#define WALL_CLOAKED            7   // Can see it, and see through it
#define WALL_TRANSPARENT        8   // like cloaked, but fixed transparency and colored

// Various tWall flags.
#define WALL_BLASTED            1   // Blasted out tWall.
#define WALL_DOOR_OPENED        2   // Open door.
#define WALL_RENDER_ADDITIVE	  4
#define WALL_DOOR_LOCKED        8   // Door is locked.
#define WALL_DOOR_AUTO          16  // Door automatically closes after time.
#define WALL_ILLUSION_OFF       32  // Illusionary tWall is shut off.
#define WALL_WALL_SWITCH        64  // This tWall is openable by a tWall switch.
#define WALL_BUDDY_PROOF        128 // Buddy assumes he cannot get through this tWall.

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

#define WALL_HPS                I2X(100)    // Normal wall's hp
#define WALL_DOOR_INTERVAL      I2X(5)      // How many seconds a door is open

#define DOOR_OPEN_TIME          I2X(2)      // How long takes to open
#define DOOR_WAIT_TIME          I2X(5)      // How long before auto door closes

#define MAX_CLIP_FRAMES         50
#define D1_MAX_CLIP_FRAMES      20

// WALL_IS_DOORWAY flags.
#define WID_FLY_FLAG            1
#define WID_RENDER_FLAG         2
#define WID_RENDPAST_FLAG       4
#define WID_EXTERNAL_FLAG       8
#define WID_CLOAKED_FLAG        16
#define WID_TRANSPARENT_FLAG    32

//define these here so I don't have to change WallIsDoorWay and run
//the risk of screwing it up.
#define WID_WALL						2	// 0/1/0		tWall
#define WID_TRANSPARENT_WALL		6	//	0/1/1		transparent tWall
#define WID_ILLUSORY_WALL			3	//	1/1/0		illusory tWall
#define WID_TRANSILLUSORY_WALL	7	//	1/1/1		transparent illusory tWall
#define WID_NO_WALL					5	//	1/0/1		no tWall, can fly through
#define WID_EXTERNAL					8	// 0/0/0/1	don't see it, dont fly through it

//@@//  WALL_IS_DOORWAY return values          F/R/RP
//@@#define WID_WALL                    2   // 0/1/0        tWall
//@@#define WID_TRANSPARENT_WALL        6   // 0/1/1        transparent tWall
//@@#define WID_ILLUSORY_WALL           3   // 1/1/0        illusory tWall
//@@#define WID_TRANSILLUSORY_WALL      7   // 1/1/1        transparent illusory tWall
//@@#define WID_NO_WALL                 5   //  1/0/1       no tWall, can fly through
//@@#define WID_EXTERNAL                8   // 0/0/0/1  don't see it, dont fly through it

#define MAX_STUCK_OBJECTS   64

typedef struct tStuckObject {
	short   nObject, nWall;
	int     nSignature;
} tStuckObject;

//Start old tWall structures

typedef struct tWallV16 {
	sbyte   nType;             // What kind of special tWall.
	sbyte   flags;             // Flags for the tWall.
	fix     hps;               // "Hit points" of the tWall.
	sbyte   nTrigger;          // Which tTrigger is associated with the tWall.
	sbyte   nClip;					// Which animation associated with the tWall.
	sbyte   keys;
} __pack__ tWallV16;

typedef struct tWallV19 {
	int     nSegment,nSide;     // Seg & tSide for this tWall
	sbyte   nType;              // What kind of special tWall.
	sbyte   flags;              // Flags for the tWall.
	fix     hps;                // "Hit points" of the tWall.
	sbyte   nTrigger;            // Which tTrigger is associated with the tWall.
	sbyte   nClip;           // Which animation associated with the tWall.
	sbyte   keys;
	int nLinkedWall;            // number of linked tWall
} __pack__ tWallV19;

typedef struct v19_door {
	int     nPartCount;            // for linked walls
	short   seg[2];             // Segment pointer of door.
	short   nSide[2];            // Side number of door.
	short   nType[2];            // What kind of door animation.
	fix     open;               // How long it has been open.
} __pack__ v19_door;

//End old tWall structures

typedef struct tWall {
	int	  nSegment, nSide;		// Seg & tSide for this tWall
	fix     hps;						// "Hit points" of the tWall.
	int     nLinkedWall;				// number of linked tWall
	ubyte   nType;						// What kind of special tWall.
	ubyte   flags;						// Flags for the tWall.
	ubyte   state;						// Opening, closing, etc.
	ubyte   nTrigger;					// Which tTrigger is associated with the tWall.
	sbyte   nClip;						// Which animation associated with the tWall.
	ubyte   keys;						// which keys are required
	sbyte   controllingTrigger;	// which tTrigger causes something to happen here.  Not like "tTrigger" above, which is the tTrigger on this tWall.
											//  Note: This gets stuffed at load time in gamemine.c.  Don't try to use it in the editor.  You will be sorry!
	sbyte   cloakValue;				// if this tWall is cloaked, the fade value
} __pack__ tWall;

typedef struct tActiveDoor {
	int     nPartCount;           // for linked walls
	short   nFrontWall[2];			// front tWall numbers for this door
	short   nBackWall[2];			// back tWall numbers for this door
	fix     time;						// how long been opening, closing, waiting
} __pack__ tActiveDoor;

// data for exploding walls (such as hostage door)
typedef struct tExplWall {
	int	nSegment, nSide;
	fix	time;
} tExplWall;

typedef struct tCloakingWall {
	short   nFrontWall;			 // front tWall numbers for this door
	short   nBackWall;			 // back tWall numbers for this door
	fix     front_ls[4];        // front tWall saved light values
	fix     back_ls[4];         // back tWall saved light values
	fix     time;               // how long been cloaking or decloaking
} __pack__ tCloakingWall;

//tWall clip flags
#define WCF_EXPLODES    1       //door explodes when opening
#define WCF_BLASTABLE   2       //this is a blastable tWall
#define WCF_TMAP1       4       //this uses primary tmap, not tmap2
#define WCF_HIDDEN      8       //this uses primary tmap, not tmap2
#define WCF_ALTFMT		16
#define WCF_FROMPOG		32
#define WCF_INITIALIZED	64

typedef struct {
	fix     xTotalTime;
	short   nFrameCount;
	short   frames[MAX_CLIP_FRAMES];
	short   openSound;
	short   closeSound;
	short   flags;
	char    filename[13];
	char    pad;
} __pack__ tWallClip;

typedef struct {
	fix     playTime;
	short   nFrameCount;
	short   frames[D1_MAX_CLIP_FRAMES];
	short   openSound;
	short   closeSound;
	short   flags;
	char    filename[13];
	char    pad;
} __pack__ tD1WallClip;

extern char pszWallNames[7][10];

//#define WALL_IS_DOORWAY(seg,tSide) WallIsDoorWay(seg, tSide)

extern int WALL_IS_DOORWAY (tSegment *segP, short nSide, tObject *objP);

// Initializes all walls (i.e. no special walls.)
void WallInit();

// Automatically checks if a there is a doorway (i.e. can fly through)
int WallIsDoorWay (tSegment *segP, short nSide, tObject *objP );

// Deteriorate appearance of tWall. (Changes bitmap (paste-ons))
void WallDamage(tSegment *segP, short nSide, fix damage);

// Destroys a blastable tWall. (So it is an opening afterwards)
void WallDestroy(tSegment *segP, short nSide);

void WallIllusionOn(tSegment *segP, short nSide);

void WallIllusionOff(tSegment *segP, short nSide);

// Opens a door, including animation and other processing.
void DoDoorOpen(int nDoor);

// Closes a door, including animation and other processing.
void DoDoorClose(int nDoor);

// Opens a door
void WallOpenDoor(tSegment *segP, short nSide);

// Closes a door
void WallCloseDoor(tSegment *segP, short nSide);

//return codes for WallHitProcess()
#define WHP_NOT_SPECIAL     0       //wasn't a quote-tWall-unquote
#define WHP_NO_KEY          1       //hit door, but didn't have key
#define WHP_BLASTABLE       2       //hit blastable tWall
#define WHP_DOOR            3       //a door (which will now be opening)

int AnimFrameCount (tWallClip *anim);

// Determines what happens when a tWall is shot
//obj is the tObject that hit...either a weapon or the tPlayer himself
extern int WallHitProcess(tSegment *segP, short nSide, fix damage, int playernum, tObject *obj );

// Opens/destroys specified door.
void WallToggle(tSegment *segP, short nSide);

// Tidy up Walls array for load/save purposes.
void ResetWalls();

void UnlockAllWalls (int bOnlyDoors);

// Called once per frame..
void WallFrameProcess();

extern tStuckObject StuckObjects [MAX_STUCK_OBJECTS];

//  An tObject got stuck in a door (like a flare).
//  Add global entry.
void AddStuckObject (tObject *objP, short nSegment, short nSide);
void RemoveObsoleteStuckObjects(void);

//set the tmap_num or tmap_num2 field for a tWall/door
void WallSetTMapNum(tSegment *segP,short nSide,tSegment *csegp, short cside,int anim_num,int nFrame);

void InitDoorAnims (void);
int AnimateOpeningDoor (tSegment *segP, short nSide, fix xElapsedTime);
void BlastBlastableWall (tSegment *segP, short nSide);

// Remove any flares from a tWall
void KillStuckObjects(int nWall);

//start tWall open <-> closed transitions
void StartWallCloak(tSegment *segP, short nSide);
void StartWallDecloak(tSegment *segP, short nSide);

bool WallIsTriggerTarget (short nWall);
bool WallIsVolatile (short nWall);
bool WallIsInvisible (short nWall);

extern int wclip_read_n_d1(tWallClip *wc, int n, CFile& cf);
#if 0
#define WClipReadN(wc, n, fp) CFRead(wc, sizeof(tWallClip), n, fp)
#define ReadWallV16(w, fp) CFRead(w, sizeof(tWallV16), 1, fp)
#define ReadWallV19(w, fp) CFRead(w, sizeof(tWallV19), 1, fp)
#define ReadWall(w, fp) CFRead(w, sizeof(tWall), 1, fp)
#define ReadActiveDoorV19(d, fp) CFRead(d, sizeof(v19_door), 1, fp)
#define ReadActiveDoor(d, fp) CFRead(d, sizeof(tActiveDoor), 1, fp)
#else
/*
 * reads n tWallClip structs from a CFILE
 */
extern int WClipReadN(tWallClip *wc, int n, CFile& cf);

/*
 * reads a tWallV16 structure from a CFILE
 */
void ReadWallV16(tWallV16 *w, CFile& cf);

/*
 * reads a tWallV19 structure from a CFILE
 */
void ReadWallV19(tWallV19 *w, CFile& cf);

/*
 * reads a tWall structure from a CFILE
 */
void ReadWall(tWall *w, CFile& cf);

/*
 * reads a v19_door structure from a CFILE
 */
void ReadActiveDoorV19(v19_door *d, CFile& cf);

/*
 * reads an tActiveDoor structure from a CFILE
 */
void ReadActiveDoor(tActiveDoor *ad, CFile& cf);
#endif

#endif
