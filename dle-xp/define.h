#ifndef _DEFINE_H
#define _DEFINE_H

// Copyright (C) 1997 Bryan Aamot

//#define HUGE
#define ErrorMsg(a) AfxMessageBox(a)
#define QueryMsg(a) (bExpertMode ? IDYES : AfxMessageBox (a, MB_YESNO))
#define Query2Msg(a,i) AfxMessageBox(a,i)
//#define STATUSMSG(a) theApp.MainFrame () ? theApp.MainFrame ()->StatusMsg(a) : AfxMessageBox(a)
//#define DEBUGMSG(a) theApp.MainFrame () ? theApp.MainFrame ()->DebugMsg(a) : AfxMessageBox(a)
#define STATUSMSG(a) {if (theApp.MainFrame ()) theApp.MainFrame ()->StatusMsg(a);}
#define INFOMSG(a) {if (theApp.MainFrame ()) theApp.MainFrame ()->InfoMsg(a);}
#define DEBUGMSG(a) {if (theApp.MainFrame ()) theApp.MainFrame ()->DebugMsg(a);}
#define farmalloc malloc
#define farfree free
#define M_PI	3.141592653589793240
#define M_PI_2	(M_PI / 2.0)

#define JOIN_DISTANCE (20*20*F1_0)

#define PREFS_SHOW_POINT_COORDINATES 0x0001
#define PREFS_SHOW_CROSS_HAIR        0x0002
#define PREFS_HIDE_MARKED_BLOCKS     0x0004
#define PREFS_DONT_SAVE_DELTA_LIGHTS 0x0008

// My own defines
#define EXTENDED_HAM 1
#define NORMAL_HAM 0

#define MAX_FLICKERING_LIGHTS 100

#define LEVEL_VERSION		18	//current level version

#define MAX_PLAYERS_D2		8
#define MAX_PLAYERS_D2X		16
#define MAX_PLAYERS			8 //((level_version < 9) ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X)
#define MAX_COOP_PLAYERS	3

#define NUM_HOT_BUTTONS 21

// defines for reading ham files
#define N_D2_ROBOT_TYPES      66
#define N_D2_ROBOT_JOINTS     1145
#define N_D2_POLYGON_MODELS   166
#define N_D2_OBJBITMAPS       422
#define N_D2_OBJBITMAPPTRS    502 // why is this not the same as N_D2_OBJBITMAPS???

#define MAX_ROBOT_TYPES       85
#define MAX_ROBOT_JOINTS      1250
#define MAX_POLYGON_MODELS    200
#define MAX_OBJ_BITMAPS       600
#define MAX_WEAPON_TYPES		65

// defines to support robot structures
#define VCLIP_MAX_FRAMES 30
#define MAX_GUNS         8  //should be multiple of 4 for ubyte array
#define NDL              5  // Guessed at this value (B. Aamot 9/14/96)
#define N_ANIM_STATES	 5

// the following are already defined below
// #define MAX_CLIP_FRAMES 50  // (called MAX_CLIP_FRAMES2)
// #define MAX_SUBMODELS   10  // I guessed at this value (BAA)

// allow all caps types for vectors
#define VMS_VECTOR vms_vector
#define VMS_ANGVEC vms_angvec

#define RDL_FILE  0
#define RL2_FILE  1

#define WEAPON_MINE	  51
#define WEAPON_SIZE	  148158L
#define WEAPON_SHIELD	  (20*F1_0)
#define MINE_CLIP_NUMBER  159

#define N_WALL_TEXTURES 26
#define NUM_OF_CLIPS 24
#define D2_N_WALL_TEXTURES 51
#define D2_NUM_OF_CLIPS 49

#define MAX_WALL_TYPES    6
#define D2_MAX_WALL_TYPES 8

#define D1_TEXTURE_STRING_TABLE 1000
#define D2_TEXTURE_STRING_TABLE 2000
#define ROBOT_STRING_TABLE      3000
#define POWERUP_STRING_TABLE    4000
#define HOT_BUTTON_STRING_TABLE 5000

// macros
#define my_sqrt(a)	sqrt(fabs((double)(a)))
#define DISABLE_BUTTON(id) EnableWindow(GetDlgItem(HWindow,id),FALSE);
#define ENABLE_BUTTON(id)  EnableWindow(GetDlgItem(HWindow,id),TRUE);

// spline related constants
#define MAX_SPLINES 32
#define MAX_SPLINE_LENGTH 1000
#define MIN_SPLINE_LENGTH 10
#define SPLINE_INTERVAL 10

// object related constants
#define AIB_STILL                0x80
#define AIB_NORMAL               0x81
#define AIB_GET_BEHIND           0x82
#define AIB_RUN_FROM             0x83
#define AIB_SNIPE                0x84
#define AIB_STATION              0x85
#define AIB_FOLLOW_PATH          0x86

#define MIN_BEHAVIOR					0x80
#define MAX_BEHAVIOR					0x86

#define MAX_OBJECT_NUMBER			12
#define MAX_CONTAINS_NUMBER		2
#define HOSTAGE_CLIP_NUMBER		33
#define VCLIP_BIG_EXPLOSION		0
#define VCLIP_SMALL_EXPLOSION		2

#define D1_PLAYER_CLIP_NUMBER		43
#define D2_PLAYER_CLIP_NUMBER		108
#define PLAYER_CLIP_NUMBER ((file_type==RDL_FILE) ? D1_PLAYER_CLIP_NUMBER : D2_PLAYER_CLIP_NUMBER)

#define D1_COOP_CLIP_NUMBER		44
#define D2_COOP_CLIP_NUMBER		108
#define COOP_CLIP_NUMBER ((file_type==RDL_FILE) ? D1_COOP_CLIP_NUMBER : D2_COOP_CLIP_NUMBER)

#define D1_REACTOR_CLIP_NUMBER	39
#define D2_REACTOR_CLIP_NUMBER	97
#define REACTOR_CLIP_NUMBER		((file_type==RDL_FILE) ? D1_REACTOR_CLIP_NUMBER : D2_REACTOR_CLIP_NUMBER)


#define HOSTAGE_SIZE   0x50000L
#define PLAYER_SIZE    0x46c35L
#define DEFAULT_SHIELD 0x640000L
#define REACTOR_SIZE   0xC14EDL
#define REACTOR_SHIELD 0xC82000L
#define MAX_D1_AI_OPTIONS 6
#define MAX_D2_AI_OPTIONS 9

// cube insertion modes
#define ORTHOGONAL 0
#define EXTEND     1
#define MIRROR     2
#define N_CUBE_MODES 3

#define STANDARD_CUBE_SIZE 2000000L

#define USER_REFRESH    0x100

#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))

#define MAX_MACROS        100
#define MAX_D1_TEXTURES   584
#define MAX_D2_TEXTURES   910
#define MAX_D1_WALL_FLAGS 5
#define MAX_D2_WALL_FLAGS 8
#define MAX_WALL_FLAGS    8
#define NUM_LIGHTS_D1     48
#define NUM_LIGHTS_D2     85

#define COMPILED_MINE_VERSION 0

#define MAX_POLY 6
#define LEFT_BUTTON 0x01
#define RIGHT_BUTTON 0x02
#define ASPECT_TOP 3
#define ASPECT_BOT 5
#define PI	3.141592653589793240
#define TWOPI (2*PI)
#define RAD2DEG (180/PI)
#define ANGLE_RATE 0.01
#define MOVE_RATE 5

/* macro modes */
#define MACRO_OFF      0
#define MACRO_RECORD   1
#define MACRO_PLAY     2

/* select modes */
#define POINT_MODE     0
#define LINE_MODE      1
#define SIDE_MODE      2
#define CUBE_MODE      3
#define OBJECT_MODE    4
#define BLOCK_MODE     5
#define N_SELECT_MODES 6

/* edit modes */
#define EDIT_OFF     0
#define EDIT_MOVE    1
#define N_EDIT_MODES 2


#define DEFAULT_SEGMENT 0
#define DEFAULT_SIDE    4
#define DEFAULT_LINE    0 /* line of the current side (0..3) */
#define DEFAULT_POINT   0 /* point of the current side (0..3) */
#define DEFAULT_OBJECT  0

#define MARKED_MASK  0x80 /* used on wall_bitmask & vert_status */
#define DELETED_MASK 0x40 /* used on wall_bitmask & vert_status */
#define NEW_MASK     0x20 /* used on vert_status                */

#define MAX_SEGMENTS1 800  // descent 1 max # of cubes
#define MAX_SEGMENTS2 900  // descent 2 max # of cubes
#define MAX_SEGMENTS3 6000 // D2X-XL max # of cubes
#define MAX_VERTICES1 2808 // descent 1 max # of vertices
#define MAX_VERTICES2 (MAX_SEGMENTS2 * 4 + 8) // descent 2 max # of vertices
#define MAX_VERTICES3 (MAX_SEGMENTS3 * 4 + 8) // descent 2 max # of vertices
#define MAX_OBJECTS1  350
#define MAX_OBJECTS2  2000
#define MAX_WALLS1    175 // Maximum number of walls for Descent 1
#define MAX_WALLS2    255 // Maximum number of walls for Descent 2
#define MAX_WALLS3    2047 // Maximum number of walls for Descent 2
#define MAX_TRIGGERS1 100
#define MAX_TRIGGERS2 254
#define MAX_OBJ_TRIGGERS	254
#define MAX_TRIGGER_FLAGS 10
#define NO_TRIGGER	255

#define MAX_SEGMENTS ((file_type==RDL_FILE) ? MAX_SEGMENTS1  : (level_version < 9) ? MAX_SEGMENTS2 : MAX_SEGMENTS3)
#define MAX_VERTICES ((file_type==RDL_FILE) ? MAX_VERTICES1 : (level_version < 9) ? MAX_VERTICES2 : MAX_VERTICES3)
#define MAX_WALLS    ((file_type==RDL_FILE) ? MAX_WALLS1 : (level_version < 12) ? MAX_WALLS2 : MAX_WALLS3)
#define NO_WALL      (MAX_WALLS)
#define MAX_TEXTURES ((file_type==RDL_FILE) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES)
#define MAX_TRIGGERS (((file_type==RDL_FILE) || (level_version < 12)) ? MAX_TRIGGERS1 : MAX_TRIGGERS2)
#define MAX_OBJECTS	((level_version < 9) ? MAX_OBJECTS1 : MAX_OBJECTS2)
#define SHOW_LINES_POINTS    0x01
#define SHOW_LINES_PARTIAL   0x02
#define SHOW_LINES_ALL       0x04
#define SHOW_LINES_NEARBY    0x08
#define SHOW_FILLED_POLYGONS 0x10

#define POWERUP_WEAPON_MASK  0x01
#define POWERUP_KEY_MASK     0x02
#define POWERUP_POWERUP_MASK 0x04
#define POWERUP_UNKNOWN_MASK 0xff // show the type if any other mask is on

#define ROBOT_IDS1						24
#define MAX_ROBOT_IDS_TOTAL         78
#define ROBOT_IDS2              ((level_version == 7L) ? N_D2_ROBOT_TYPES : MAX_ROBOT_IDS_TOTAL)
#define MAX_POWERUP_IDS1            26
#define MAX_POWERUP_IDS2            50
#define MAX_POWERUP_IDS_D2          48
#define MAX_POWERUP_IDS    ((file_type==RDL_FILE) ? MAX_POWERUP_IDS1 : MAX_POWERUP_IDS2)
#define MAX_TRIGGER_TARGETS         10

#define POW_AMMORACK						35

//#define MAX_DOORS1                50 // Maximum number of open doors Descent 1
//#define MAX_DOORS2                90 // Maximum number of open doors Descent 2

#define MAX_DOORS                    1
#define MAX_CONTROL_CENTER_TRIGGERS 10
#define MAX_NUM_MATCENS1				20
#define MAX_NUM_MATCENS2				100
#define MAX_NUM_MATCENS					(((file_type==RDL_FILE) || (level_version < 12)) ? MAX_NUM_MATCENS1 : MAX_NUM_MATCENS2)
#define MAX_WALL_SWITCHES           50
#define MAX_WALL_LINKS              100
#define MAX_NUM_FUELCENS1				70
#define MAX_NUM_REPAIRCENS1			70
#define MAX_NUM_FUELCENS2				500
#define MAX_NUM_REPAIRCENS2			500
#define MAX_NUM_FUELCENS				(((file_type==RDL_FILE) || (level_version < 12)) ? MAX_NUM_FUELCENS1 : MAX_NUM_FUELCENS2)
#define MAX_NUM_REPAIRCENS				(((file_type==RDL_FILE) || (level_version < 12)) ? MAX_NUM_REPAIRCENS1 : MAX_NUM_REPAIRCENS2)
#define MAX_WALL_ANIMS1					30  // Maximum different types of doors Descent 1
#define MAX_WALL_ANIMS2					60  // Maximum different types of doors Descent 2
#define MAX_CLIP_FRAMES1	         20 // Descent 1
#define MAX_CLIP_FRAMES2	         50 // Descent 2
#define MAX_STUCK_OBJECTS	         32
#define MAX_SIDES_PER_SEGMENT			6
#define MAX_VERTICES_PER_SEGMENT		8
#define MAX_AI_FLAGS						11 /* This MUST cause word (4 bytes) alignment in ai_static, allowing for one byte mode */
#define MAX_SUBMODELS					10 /* I guessed at this value (BAA) */

/*Some handy constants */
#define f0_0   0
#define f1_0   0x10000L
#define f2_0   0x20000L
#define f3_0   0x30000L
#define f10_0  0xa0000L

#define f0_5   0x8000

#define f0_1   0x199a

#define F0_0   f0_0
#define F1_0   f1_0
#define F2_0   f2_0
#define F3_0   f3_0
#define F10_0  f10_0

#define F0_5   f0_5
#define F0_1   f0_1

/* wall numbers */
#define WLEFT               0
#define WTOP                1
#define WRIGHT              2
#define WBOTTOM             3
#define WBACK               4
#define WFRONT              5

/* misc */
#define DEFAULT_LIGHTING    F0_5      /* (F1_0/2) */
#define TRIGGER_DEFAULT     2*F1_0

// Trigger flags for Descent 1
#define	TRIGGER_CONTROL_DOORS	1	/* Control Trigger */
#define	TRIGGER_SHIELD_DAMAGE	2	/* Shield Damage Trigger */
#define	TRIGGER_ENERGY_DRAIN	   4	/* Energy Drain Trigger */
#define	TRIGGER_EXIT				8	/* End of level Trigger */
#define	TRIGGER_ON					16	/* Whether Trigger is active */
#define	TRIGGER_ONE_SHOT			32	/* If Trigger can only be triggered once */
#define	TRIGGER_MATCEN				64	/* Trigger for materialization centers */
#define	TRIGGER_ILLUSION_OFF		128	/* Switch Illusion OFF trigger */
#define	TRIGGER_SECRET_EXIT		256	/* Exit to secret level */
#define	TRIGGER_ILLUSION_ON		512	/* Switch Illusion ON trigger */

// Trigger types for Descent 2
#define TT_OPEN_DOOR				0	// Open a door
#define TT_CLOSE_DOOR		   1	// Close a door
#define TT_MATCEN					2	// Activate a matcen
#define TT_EXIT					3	// End the level
#define TT_SECRET_EXIT		   4	// Go to secret level
#define TT_ILLUSION_OFF		   5	// Turn an illusion off
#define TT_ILLUSION_ON		   6	// Turn an illusion on
#define TT_UNLOCK_DOOR		   7	// Unlock a door
#define TT_LOCK_DOOR				8	// Lock a door
#define TT_OPEN_WALL				9	// Makes a wall open
#define TT_CLOSE_WALL			10	//	Makes a wall closed
#define TT_ILLUSORY_WALL		11	// Makes a wall illusory
#define TT_LIGHT_OFF				12	// Turn a light off
#define TT_LIGHT_ON				13	// Turn s light on
#define TT_TELEPORT				14
#define TT_SPEEDBOOST			15
#define TT_CAMERA					16
#define TT_SHIELD_DAMAGE_D2	17
#define TT_ENERGY_DRAIN_D2		18
#define TT_CHANGE_TEXTURE		19
#define TT_SMOKE_LIFE			20
#define TT_SMOKE_SPEED			21
#define TT_SMOKE_DENS			22
#define TT_SMOKE_SIZE			23
#define TT_SMOKE_DRIFT			24
#define TT_COUNTDOWN				25
#define TT_SPAWN_BOT				26
#define TT_SMOKE_BRIGHTNESS	27
#define TT_SET_SPAWN				28
#define TT_MESSAGE				29
#define TT_SOUND					30
#define TT_MASTER					31
#define NUM_TRIGGER_TYPES		32

#define TT_SHIELD_DAMAGE         100    // added to support d1 shield damage
#define TT_ENERGY_DRAIN          101    // added to support d1 energy drain

// Trigger flags for Descent 2
#define TF_NO_MESSAGE		   1	// Don't show a message when triggered
#define TF_ONE_SHOT		   2	// Only trigger once
#define TF_DISABLED		   4	// Set after one-shot fires
#define TF_PERMANENT		   8	// indestructable switch for repeated operation
#define TF_ALTERNATE			16 // switch will assume the opposite function after operation
#define TF_SET_ORIENT		32 // switch will assume the opposite function after operation

#define	MAX_DL_INDICES_D2		500
#define	MAX_DELTA_LIGHTS_D2	10000
#define	MAX_DL_INDICES_D2X	3000
#define	MAX_DELTA_LIGHTS_D2X	50000
#define  MAX_DL_INDICES	\
	(((file_type==RDL_FILE) || (level_version < 9)) ? MAX_DL_INDICES_D2 : MAX_DL_INDICES_D2X)
#define  MAX_DELTA_LIGHTS	\
	(((file_type==RDL_FILE) || (level_version < 9)) ? MAX_DELTA_LIGHTS_D2 : MAX_DELTA_LIGHTS_D2X)
#define	DL_SCALE     			2048	// Divide light to allow 3 bits integer, 5 bits fraction.

// Texture flags
#define	BM_FLAG_TRANSPARENT        1
#define	BM_FLAG_SUPER_TRANSPARENT  2
#define	BM_FLAG_NO_LIGHTING        4
#define	BM_FLAG_RLE                8
#define	BM_FLAG_PAGED_OUT          16
#define	BM_FLAG_RLE_BIG            32

/* Trigger delay times before they can be retriggered (Recharge time) */
#define	TRIGGER_DELAY_DOOR	F1_0*1	/* 1 second for doors */
#define	TRIGGER_DELAY_ZAPS	F1_0/10	/* 1/10 second for quickie stuff */

/* New unimplemented trigger ideas  */
#define	TRIGGER_CONTROL_ROBOTS	64	/* If Trigger is a Door control trigger (Linked) */
#define	CONTROL_ROBOTS		 8	/* If Trigger modifies robot behavior */
#define	CONTROL_LIGHTS_ON	16	/* If Trigger turns on lights in a certain area */
#define	CONTROL_LIGHTS_OFF	32	/* If Trigger turns off lights in a certain area */

/*  Returns true if segnum references a child, else returns false. */
/*  Note that -1 means no connection, -2 means a connection to the outside world. */
#define  IS_CHILD(segnum) (segnum > -1)

/*Special types */
#define SPECIAL_NORMAL        0
#define SPECIAL_FUEL          1
#define SPECIAL_MATCEN        3
#define SPECIAL_REPRODUCER    4

#define SEGMENT_IS_NOTHING			0
#define SEGMENT_IS_FUELCEN			1
#define SEGMENT_IS_REPAIRCEN		2
#define SEGMENT_IS_CONTROLCEN		3
#define SEGMENT_IS_ROBOTMAKER		4
#define MAX_CENTER_TYPES1			5

#define SEGMENT_IS_GOAL_BLUE		5 // Descent 2 only
#define SEGMENT_IS_GOAL_RED		6 // Descent 2 only
#define SEGMENT_IS_WATER			7
#define SEGMENT_IS_LAVA				8
#define SEGMENT_IS_TEAM_BLUE		9
#define SEGMENT_IS_TEAM_RED		10
#define SEGMENT_IS_SPEEDBOOST		11
#define SEGMENT_IS_BLOCKED			12
#define SEGMENT_IS_NODAMAGE		13
#define SEGMENT_IS_SKYBOX			14
#define SEGMENT_IS_EQUIPMAKER		15	// matcen for powerups
#define SEGMENT_IS_OUTDOOR			16
#define MAX_CENTER_TYPES2			17 // Descent 2 only

/* Various wall types. */
#define WALL_NORMAL			0  	/* Normal wall */
#define WALL_BLASTABLE		1  	/* Removable (by shooting) wall */
#define WALL_DOOR				2  	/* Door  */
#define WALL_ILLUSION		3  	/* Wall that appears to be there, but you can fly thru */
#define WALL_OPEN				4	/* Just an open side. (Trigger) */
#define WALL_CLOSED			5	/* Wall.  Used for transparent walls. */
#define WALL_OVERLAY			6 	// Goes over an actual solid side.  For triggers (Descent 2)
#define WALL_CLOAKED			7 	// Can see it, and see through it  (Descent 2)
#define WALL_TRANSPARENT	8

/* Various wall flags. */
#define WALL_BLASTED				1  	/* Blasted out wall. */
#define WALL_DOOR_OPENED		2  	/* Open door.  */
#define WALL_RENDER_ADDITIVE	4
#define WALL_DOOR_LOCKED		8	/* Door is locked. */
#define WALL_DOOR_AUTO			16	/* Door automatically closes after time. */
#define WALL_ILLUSION_OFF		32	/* Illusionary wall is shut off. */
#define WALL_WALL_SWITCH		64	// This wall is openable by a wall switch (Descent 2)
#define WALL_BUDDY_PROOF		128	// Buddy assumes he cannot get through this wall (Descent 2)
#define WALL_IGNORE_MARKER		256

//wall clip flags (new for Descent 2)
#define WCF_EXPLODES		1	//door explodes when opening
#define WCF_BLASTABLE		2	//this is a blastable wall
#define WCF_TMAP1		4	//this uses primary tmap, not tmap2
#define WCF_HIDDEN		8	//this uses primary tmap, not tmap2

/* Wall states */
#define WALL_DOOR_CLOSED	0	/* Door is closed */
#define WALL_DOOR_OPENING	1	/* Door is opening. */
#define WALL_DOOR_WAITING	2	/* Waiting to close */
#define WALL_DOOR_CLOSING	3	/* Door is closing */
#define WALL_DOOR_CLOAKING	5	// Wall is going from closed -> open (Descent 2)
#define WALL_DOOR_DECLOAKING	6	// Wall is going from open -> closed (Descent 2)

#define KEY_NONE		1
#define KEY_BLUE		2
#define KEY_RED			4
#define KEY_GOLD		8

#define WALL_HPS		100*F1_0 /* Normal wall's hp */
#define WALL_DOOR_INTERVAL	5*F1_0	 /* How many seconds a door is open */

#define DOOR_OPEN_TIME		i2f(2)	 /* How long takes to open */
#define DOOR_WAIT_TIME		i2f(5)	 /* How long before auto door closes */

/* WALL_IS_DOORWAY flags. */
#define WID_FLY_FLAG			1
#define WID_RENDER_FLAG		2
#define WID_RENDPAST_FLAG	4
#define WID_EXTERNAL_FLAG	8
#define WID_CLOAKED_FLAG	16 // Descent 2

/* WALL_IS_DOORWAY return values F/R/RP */
#define WID_WALL						2	/* 0/1/0 wall	 */
#define WID_ILLUSORY_WALL			3	/* 1/1/0 illusory wall */
#define WID_NO_WALL					5	/* 1/0/1 no wall, can fly through */
#define WID_TRANSPARENT_WALL		6	/* 0/1/1 transparent wall */
#define WID_TRANSILLUSORY_WALL	7	/* 1/1/1 transparent illusory wall */
#define WID_EXTERNAL					8	/* 0/0/0/1 don't see it, dont fly through it */

#define WALL_IS_DOORWAY(seg,side) (((seg)->children[(side)] == -1) ? WID_RENDER_FLAG : ((seg)->children[(side)] == -2) ? WID_EXTERNAL_FLAG : ((seg)->sides[(side)].nWall == -1) ? (WID_FLY_FLAG|WID_RENDPAST_FLAG) : wall_is_doorway((seg), (side)))

/*Object types */
#define OBJ_NONE         255  /* unused object */
#define OBJ_WALL           0  /* A wall... not really an object, but used for collisions */
#define OBJ_FIREBALL       1  /* a fireball, part of an explosion */
#define OBJ_ROBOT          2  /* an evil enemy */
#define OBJ_HOSTAGE        3  /* a hostage you need to rescue */
#define OBJ_PLAYER         4  /* the player on the console */
#define OBJ_WEAPON         5  /* a laser, missile, etc */
#define OBJ_CAMERA         6  /* a camera to slew around with */
#define OBJ_POWERUP        7  /* a powerup you can pick up */
#define OBJ_DEBRIS         8  /* a piece of robot */
#define OBJ_CNTRLCEN       9  /* the control center */
#define OBJ_FLARE         10  /* a flare */
#define OBJ_CLUTTER       11  /* misc objects */
#define OBJ_GHOST         12  /* what the player turns into when dead */
#define OBJ_LIGHT         13  /* a light source, & not much else */
#define OBJ_COOP          14  /* a cooperative player object. */
#define OBJ_MARKER		  15
#define OBJ_CAMBOT		  16
#define OBJ_MONSTERBALL	  17
#define OBJ_SMOKE			  18
#define OBJ_EXPLOSION	  19
#define OBJ_EFFECT		  20
#define MAX_OBJECT_TYPES  21

#define SMOKE_ID				0
#define LIGHTNING_ID			1

/*Result types */
#define RESULT_NOTHING  0 /* Ignore this collision */
#define RESULT_CHECK    1 /* Check for this collision */

/*Control types - what tells this object what do do */
#define CT_NONE        0  /* doesn't move (or change movement) */
#define CT_AI          1  /* driven by AI */
#define CT_EXPLOSION   2  /*explosion sequencer */
#define CT_FLYING      4  /*the player is flying */
#define CT_SLEW        5  /*slewing */
#define CT_FLYTHROUGH  6  /*the flythrough system */
#define CT_WEAPON      9  /*laser, etc. */
#define CT_REPAIRCEN  10  /*under the control of the repair center */
#define CT_MORPH      11  /*this object is being morphed */
#define CT_DEBRIS     12  /*this is a piece of debris */
#define CT_POWERUP    13  /*animating powerup blob */
#define CT_LIGHT      14  /*doesn't actually do anything */
#define CT_REMOTE     15  /*controlled by another net player */
#define CT_CNTRLCEN   16  /*the control center/main reactor  */

/*Movement types */
#define MT_NONE       0  /*doesn't move */
#define MT_PHYSICS    1  /*moves by physics */
#define MT_SPINNING   3  /*this object doesn't move, just sits and spins */

/*Render types */
#define RT_NONE       0  /*does not render */
#define RT_POLYOBJ    1  /*a polygon model */
#define RT_FIREBALL   2  /*a fireball */
#define RT_LASER      3  /*a laser */
#define RT_HOSTAGE    4  /*a hostage */
#define RT_POWERUP    5  /*a powerup */
#define RT_MORPH      6  /*a robot being morphed */
#define RT_WEAPON_VCLIP  7  /*a weapon that renders as a vclip */
#define RT_THRUSTER		8	 // like afterburner, but doesn't cast light
#define RT_EXPLBLAST		9	 // white explosion light blast
#define RT_SHRAPNELS		10	 // white explosion light blast
#define RT_SMOKE			11
#define RT_LIGHTNING    12

/*misc object flags */
#define OF_EXPLODING       1  /*this object is exploding */
#define OF_SHOULD_BE_DEAD  2  /*this object should be dead, so next time we can, we should delete this object. */
#define OF_DESTROYED       4  /*this has been killed, and is showing the dead version */
#define OF_SILENT          8  /*this makes no sound when it hits a wall.  Added by MK for weapons,
                                if you extend it to other types, do it completely! */
#define OF_ATTACHED       16  /*this object is a fireball attached to another object */
#define OF_HARMLESS       32  /*this object does no damage.  Added to make quad lasers do 1.5 damage as normal lasers. */

/*Different Weapon ID types... */
#define WEAPON_ID_LASER      0
#define WEAPON_ID_MISSLE     1
#define WEAPON_ID_CANNONBALL 2

/*Object Initial shields... */
#define OBJECT_INITIAL_SHIELDS F1_0/2

/*physics flags */
#define PF_TURNROLL    0x01    /* roll when turning */
#define PF_LEVELLING   0x02    /* level object with closest side */
#define PF_BOUNCE      0x04    /* bounce (not slide) when hit will */
#define PF_WIGGLE      0x08    /* wiggle while flying */
#define PF_STICK       0x10    /* object sticks (stops moving) when hits wall */
#define PF_PERSISTENT  0x20    /* object keeps going even after it hits another object (eg, fusion cannon) */
#define PF_USES_THRUST 0x40    /* this object uses its thrust */

#define  IMMORTAL_TIME 0x3fffffff  /*  Time assigned to immortal objects, about 32768 seconds, or about 9 hours. */

#define SEGMENT_IS_ROBOTMAKER 4

#define MAX_BRIGHTNESS	0x20000

#ifdef _DEBUG
#	define BRK _asm int 3;
#	define CBRK(_cond)	if (_cond) BRK;
#else
#	define BRK
#	define CBRK(_cond)
#endif

#define atan3(y,x)  atan2((double) (y), (double) (x)) //((y)==(x)) ? atan2((double) (y) + 0.00000001, (double) (x)) : atan2((double) (y), (double) (x))

#define IMG_BKCOLOR	RGB (196,196,196)

#endif // _DEFINE_H
