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

#ifndef _OBJECT_H
#define _OBJECT_H

#include <time.h>

#include "pstypes.h"
#include "vecmat.h"
//#include "segment.h"
//#include "gameseg.h"
#include "piggy.h"
#include "aistruct.h"
#include "segment.h"
#include "gr.h"

/*
 * CONSTANTS
 */

#define MAX_OBJECTS_D2  	350 // increased on 01/24/95 for multiplayer. --MK;  total number of objects in world
#define MAX_OBJECTS_D2X	   MAX_SEGMENTS_D2X
#define MAX_OBJECTS     	MAX_SEGMENTS
#define MAX_HIT_OBJECTS		20

// Object types
#define OBJ_NONE        255 // unused tObject
#define OBJ_WALL        0   // A tWall... not really an tObject, but used for collisions
#define OBJ_FIREBALL    1   // a fireball, part of an explosion
#define OBJ_ROBOT       2   // an evil enemy
#define OBJ_HOSTAGE     3   // a hostage you need to rescue
#define OBJ_PLAYER      4   // the tPlayer on the console
#define OBJ_WEAPON      5   // a laser, missile, etc
#define OBJ_CAMERA      6   // a camera to slew around with
#define OBJ_POWERUP     7   // a powerup you can pick up
#define OBJ_DEBRIS      8   // a piece of robot
#define OBJ_REACTOR     9   // the control center
#define OBJ_FLARE       10  // a flare
#define OBJ_CLUTTER     11  // misc objects
#define OBJ_GHOST       12  // what the tPlayer turns into when dead
#define OBJ_LIGHT       13  // a light source, & not much else
#define OBJ_COOP        14  // a cooperative tPlayer tObject.
#define OBJ_MARKER      15  // a map marker
#define OBJ_CAMBOT		16	 // a camera
#define OBJ_MONSTERBALL	17	 // a monsterball
#define OBJ_SMOKE		18	 // static smoke
#define OBJ_EXPLOSION	19	 // static explosion clouds
#define OBJ_EFFECT		20	 // lightnings

// WARNING!! If you add a nType here, add its name to ObjectType_names
// in tObject.c
#define MAX_OBJECT_TYPES 21

// Result types
#define RESULT_NOTHING  0   // Ignore this collision
#define RESULT_CHECK    1   // Check for this collision

// Control types - what tells this tObject what do do
#define CT_NONE         0   // doesn't move (or change movement)
#define CT_AI           1   // driven by AI
#define CT_EXPLOSION    2   // explosion sequencer
#define CT_FLYING       4   // the tPlayer is flying
#define CT_SLEW         5   // slewing
#define CT_FLYTHROUGH   6   // the flythrough system
#define CT_WEAPON       9   // laser, etc.
#define CT_REPAIRCEN    10  // under the control of the repair center
#define CT_MORPH        11  // this tObject is being morphed
#define CT_DEBRIS       12  // this is a piece of debris
#define CT_POWERUP      13  // animating powerup blob
#define CT_LIGHT        14  // doesn't actually do anything
#define CT_REMOTE       15  // controlled by another net tPlayer
#define CT_CNTRLCEN     16  // the control center/main reactor
#define CT_CAMERA			17

// Movement types
#define MT_NONE         0   // doesn't move
#define MT_PHYSICS      1   // moves by physics
#define MT_STATIC		2	 // completely still and immoveable
#define MT_SPINNING     3   // this tObject doesn't move, just sits and spins

// Render types
#define RT_NONE         0   // does not render
#define RT_POLYOBJ      1   // a polygon model
#define RT_FIREBALL     2   // a fireball
#define RT_LASER        3   // a laser
#define RT_HOSTAGE      4   // a hostage
#define RT_POWERUP      5   // a powerup
#define RT_MORPH        6   // a robot being morphed
#define RT_WEAPON_VCLIP 7   // a weapon that renders as a tVideoClip
#define RT_THRUSTER		8	 // like afterburner, but doesn't cast light
#define RT_EXPLBLAST	9	 // white explosion light blast
#define RT_SHRAPNELS	10	 // white explosion light blast
#define RT_SMOKE		11
#define RT_LIGHTNING    12

#define SMOKE_ID			0
#define LIGHTNING_ID		1

#define SINGLE_LIGHT_ID		0
#define CLUSTER_LIGHT_ID	1

// misc tObject flags
#define OF_EXPLODING        1   // this tObject is exploding
#define OF_SHOULD_BE_DEAD   2   // this tObject should be dead, so next time we can, we should delete this tObject.
#define OF_DESTROYED        4   // this has been killed, and is showing the dead version
#define OF_SILENT           8   // this makes no sound when it hits a tWall.  Added by MK for weapons, if you extend it to other types, do it completely!
#define OF_ATTACHED         16  // this tObject is a fireball attached to another tObject
#define OF_HARMLESS         32  // this tObject does no damage.  Added to make quad lasers do 1.5 damage as normal lasers.
#define OF_PLAYER_DROPPED   64  // this tObject was dropped by the tPlayer...
#define OF_ARMAGEDDON		 128 // destroyed by cheat

// Different Weapon ID types...
#define WEAPON_ID_LASER         0
#define WEAPON_IDMSLLE        1
#define WEAPON_ID_CANNONBALL    2

// Object Initial shields...
#define OBJECT_INITIAL_SHIELDS F1_0/2

// physics flags
#define PF_TURNROLL         0x01    // roll when turning
#define PF_LEVELLING        0x02    // level tObject with closest tSide
#define PF_BOUNCE           0x04    // bounce (not slide) when hit will
#define PF_WIGGLE           0x08    // wiggle while flying
#define PF_STICK            0x10    // tObject sticks (stops moving) when hits tWall
#define PF_PERSISTENT       0x20    // tObject keeps going even after it hits another tObject (eg, fusion cannon)
#define PF_USES_THRUST      0x40    // this tObject uses its thrust
#define PF_BOUNCED_ONCE     0x80    // Weapon has bounced once.
#define PF_FREE_SPINNING    0x100   // Drag does not apply to rotation of this tObject
#define PF_BOUNCES_TWICE    0x200   // This weapon bounces twice, then dies

#define IMMORTAL_TIME   0x3fffffff  // Time assigned to immortal objects, about 32768 seconds, or about 9 hours.
#define ONE_FRAME_TIME  0x3ffffffe  // Objects with this lifeleft will live for exactly one frame

#define MAX_VELOCITY I2X(50)

#define PF_SPAT_BY_PLAYER   1 //this powerup was spat by the tPlayer

extern char szObjectTypeNames [MAX_OBJECT_TYPES][10];

// List of objects rendered last frame in order.  Created at render
// time, used by homing missiles in laser.c
#define MAX_RENDERED_OBJECTS    100

/*
 * STRUCTURES
 */

// A compressed form for sending crucial data about via slow devices,
// such as modems and buggies.
typedef struct tShortPos {
	sbyte   orient [9];
	short   pos [3];
	short   nSegment;
	short   vel [3];
} tShortPos;

class CShortPos {
	private:
		tShortPos	m_pos;
	public:
		inline sbyte& Orient(int i) { return m_pos.orient [i]; }
		inline short& Pos (int i) { return m_pos.pos [i]; }
		inline short& Segment () { return m_pos.nSegment; }
		inline short& Vel (int i) { return m_pos.vel [i]; }
};

// This is specific to the tShortPos extraction routines in gameseg.c.
#define RELPOS_PRECISION    10
#define MATRIX_PRECISION    9
#define MATRIX_MAX          0x7f    // This is based on MATRIX_PRECISION, 9 => 0x7f

#if 0
class MovementInfo { };
class PhysicsMovementInfo : public MovementInfo { };
class SpinMovementInfo    : public MovementInfo { };

class ControlInfo { };
class ControlLaserInfo : public ControlInfo { };
class ControlExplosionInfo : public ControlInfo { };
class ControlAIStaticInfo : public ControlInfo { };
class ControlLightInfo : public ControlInfo { };     // why put this here?  Didn't know what else to do with it.
class ControlPowerupInfo : public ControlInfo { };

class RenderInfo { };
class RenderPolyObjInfo : public RenderInfo { };      // polygon model
class RenderVClipInfo : public RenderInfo { };     // tVideoClip
class RenderSmokeInfo : public RenderInfo { };
class RenderLightningInfo : public RenderInfo { };
#endif

// information for physics sim for an tObject
typedef struct tPhysicsInfo {
	vmsVector	velocity;   // velocity vector of this tObject
	vmsVector	thrust;     // constant force applied to this tObject
	fix         mass;       // the mass of this tObject
	fix         drag;       // how fast this slows down
	fix         brakes;     // how much brakes applied
	vmsVector	rotVel;     // rotational velecity (angles)
	vmsVector	rotThrust;  // rotational acceleration
	fixang      turnRoll;   // rotation caused by turn banking
	ushort      flags;      // misc physics flags
} tPhysicsInfo;

class CPhysicsInfo {
	private:
		tPhysicsInfo	m_info;
	public:
		inline vmsVector& Velocity () { return m_info.velocity; }
		inline vmsVector& Thrust () { return m_info.thrust; }
		inline vmsVector& RotVel () { return m_info.rotVel; }
		inline vmsVector& RotThrust () { return m_info.rotThrust; }
		inline fix& Mass () { return m_info.mass; }
		inline fix& Drag () { return m_info.drag; }
		inline fix& Brakes () { return m_info.brakes; }
		inline fixang& TurnRoll () { return m_info.turnRoll; }
		inline ushort& Flags () { return m_info.flags; }
};
// stuctures for different kinds of simulation

typedef struct nParentInfo {
	short		nType;
	short		nObject;
	int		nSignature;
} tParentInfo;

typedef struct tLaserInfo  {
	tParentInfo	parent;
	fix     xCreationTime;      // Absolute time of creation.
	short   nLastHitObj;       // For persistent weapons (survive tObject collision), tObject it most recently hit.
	short   nHomingTarget;				// Object this tObject is tracking.
	fix     xScale;        // Power if this is a fusion bolt (or other super weapon to be added).
} tLaserInfo;

class CLaserInfo {
	private:
		tLaserInfo	m_info;
	public:
		inline short& ParentType () { return m_info.parent.nType; }
		inline short& ParentObj () { return m_info.parent.nObject; }
		inline int& ParentSig () { return m_info.parent.nSignature; }
		inline short& LastHitObj () { return m_info.nLastHitObj; }
		inline short& MslLock () { return m_info.nHomingTarget; }
		inline fix& CreationTime () { return m_info.xCreationTime; }
		inline fix& Multiplier () { return m_info.xScale; }
};

typedef struct tAttachedObjInfo {
	short	nParent;	// explosion is attached to this tObject
	short	nPrev;	// previous explosion in attach list
	short	nNext;	// next explosion in attach list
} tAttachedObjInfo;

typedef struct tExplosionInfo {
    fix     nSpawnTime;       // when lifeleft is < this, spawn another
    fix     nDeleteTime;      // when to delete tObject
    short   nDeleteObj;			// and what tObject to delete
	 tAttachedObjInfo	attached;
} tExplosionInfo;

class CExplosionInfo {
	private:
		tExplosionInfo	m_info;
	public:
		inline fix& SpawnTime () { return m_info.nSpawnTime; }
		inline fix& DeleteTime () { return m_info.nDeleteTime; }
		inline short& DeleteObj () { return m_info.nDeleteObj; }
		inline short& Parent () { return m_info.attached.nParent; }
		inline short& PrevAttached () { return m_info.attached.nPrev; }
		inline short& NextAttached () { return m_info.attached.nNext; }
};

typedef struct tObjLightInfo {
    fix				intensity;  // how bright the light is
	 short			nSegment;
	 short			nObjects;
	 tRgbaColorf	color;
} tObjLightInfo;

class CObjLightInfo {
	private:
		tObjLightInfo	m_info;
	public:
		inline fix& Intensity () { return m_info.intensity; }
		inline short& Segment () { return m_info.nSegment; }
		inline short& Objects () { return m_info.nObjects; }
		inline tRgbaColorf& Color () { return m_info.color; }
};

typedef struct tPowerupInfo {
	int     nCount;          // how many/much we pick up (vulcan cannon only?)
	fix     xCreationTime;  // Absolute time of creation.
	int     nFlags;          // spat by tPlayer?
}  tPowerupInfo;

class CPowerupInfo {
	private:
		tPowerupInfo	m_info;
	public:
		inline int& Count () { return m_info.nCount; }
		inline fix& CreationTime () { return m_info.xCreationTime; }
		inline int& Flags () { return m_info.nFlags; }
};

typedef struct tVClipInfo {
public:
	int     nClipIndex;
	fix	  xTotalTime;
	fix     xFrameTime;
	sbyte   nCurFrame;
} tVClipInfo;

class CVClipInfo {
	private:
		tVClipInfo	m_info;
	public:
		inline int& ClipIndex () { return m_info.nClipIndex; }
		inline fix& TotalTime () { return m_info.xTotalTime; }
		inline fix& FrameTime () { return m_info.xFrameTime; }
		inline sbyte& CurFrame () { return m_info.nCurFrame; }
};

typedef struct tSmokeInfo {
public:
	int			nLife;
	int			nSize [2];
	int			nParts;
	int			nSpeed;
	int			nDrift;
	int			nBrightness;
	tRgbaColorb	color;
	char			nSide;
} tSmokeInfo;

class CSmokeInfo {
	private:
		tSmokeInfo	m_info;
	public:
		inline int& Life () { return m_info.nLife; }
		inline int& Size (int i) { return m_info.nSize [i]; }
		inline int& Parts () { return m_info.nParts; }
		inline int& Speed () { return m_info.nSpeed; }
		inline int& Drift () { return m_info.nDrift; }
		inline int& Brightness () { return m_info.nBrightness; }
		inline tRgbaColorb& Color () { return m_info.color; }
		inline char& Side () { return m_info.nSide; }
};

typedef struct tLightningInfo {
public:
	int			nLife;
	int			nDelay;
	int			nLength;
	int			nAmplitude;
	int			nOffset;
	short			nLightnings;
	short			nId;
	short			nTarget;
	short			nNodes;
	short			nChildren;
	short			nSteps;
	char			nAngle;
	char			nStyle;
	char			nSmoothe;
	char			bClamp;
	char			bPlasma;
	char			bSound;
	char			bRandom;
	char			bInPlane;
	tRgbaColorb color;
} tLightningInfo;

class CLightningInfo {
	private:
		tLightningInfo	m_info;
	public:
		inline int& Life () { return m_info.nLife; }
		inline int& Delay () { return m_info.nDelay; }
		inline int& Length () { return m_info.nLength; }
		inline int& Amplitude () { return m_info.nAmplitude; }
		inline int& Offset () { return m_info.nOffset; }
		inline short& Lightnings () { return m_info.nLightnings; }
		inline short& Id () { return m_info.nId; }
		inline short& Target () { return m_info.nTarget; }
		inline short& Nodes () { return m_info.nNodes; }
		inline short& Children () { return m_info.nChildren; }
		inline short& Steps () { return m_info.nSteps; }
		inline char& Angle () { return m_info.nAngle; }
		inline char& Style () { return m_info.nStyle; }
		inline char& Smoothe () { return m_info.nSmoothe; }
		inline char& Clamp () { return m_info.bClamp; }
		inline char& Plasma () { return m_info.bPlasma; }
		inline char& Sound () { return m_info.bSound; }
		inline char& Random () { return m_info.bRandom; }
		inline char& InPlane () { return m_info.bInPlane; }
};

// structures for different kinds of rendering

typedef struct tPolyObjInfo {
public:
	int     		nModel;          // which polygon model
	vmsAngVec 	animAngles [MAX_SUBMODELS]; // angles for each subobject
	int     		nSubObjFlags;       // specify which subobjs to draw
	int     		nTexOverride;      // if this is not -1, map all face to this
	int     		nAltTextures;       // if not -1, use these textures instead
} tPolyObjInfo;

class CPolyObjInfo {
	private:
		tPolyObjInfo	m_info;
	public:
		inline int& Model() { return m_info.nModel; }
		inline vmsAngVec& AnimAngles(int i) { return m_info.animAngles [i]; }
		inline int& SubObjFlags() { return m_info.nSubObjFlags; }
		inline int& TexOverride() { return m_info.nTexOverride; }
		inline int& AltTextures() { return m_info.nAltTextures; }
};

typedef struct tTransformation {
	vmsVector	vPos;				// absolute x,y,z coordinate of center of object
	vmsMatrix	mOrient;			// orientation of object in world
	} tTransformation;

class CTransformation {
private:
	tTransformation	m_t;
public:
	inline vmsVector& Pos() { return m_t.vPos; }
	inline vmsMatrix& Orient() { return m_t.mOrient; }
};

typedef struct tObjContainerInfo {
	sbyte			nType;
	sbyte			nId;
	sbyte			nCount;
} tObjContainerInfo;

class CObjContainerInfo {
	private:
		tObjContainerInfo	m_info;
	public:
		inline sbyte& ContainsType () { return m_info.nType; }
		inline sbyte& ContainsId () { return m_info.nId; }
		inline sbyte& ContainsCount () { return m_info.nCount; }
};

typedef struct tObjectInfo {
	int     				nSignature;    // Every tObject ever has a unique nSignature...
	ubyte   				nType;         // what nType of tObject this is... robot, weapon, hostage, powerup, fireball
	ubyte   				nId;           // which form of tObject...which powerup, robot, etc.
#ifdef WORDS_NEED_ALIGNMENT
	short   				pad;
#endif
	short   				nNextInSeg, 
							nPrevInSeg;  // id of next and previous connected tObject in Objects, -1 = no connection
	ubyte   				controlType;   // how this tObject is controlled
	ubyte   				movementType;  // how this tObject moves
	ubyte   				renderType;    // how this tObject renders
	ubyte   				nFlags;        // misc flags
	short					nSegment;
	short   				nAttachedObj;  // number of attached fireball tObject
	tTransformation	position;
	fix     				xSize;         // 3d size of tObject - for collision detection
	fix     				xShields;      // Starts at maximum, when <0, tObject dies..
	vmsVector 			vLastPos;		// where tObject was last frame
	tObjContainerInfo	contains;
	sbyte   				nCreator; // Materialization center that created this tObject, high bit set if matcen-created
	fix     				xLifeLeft;      // how long until goes away, or 7fff if immortal
} tObjectInfo;

class CObjectInfo : public CTransformation, public CObjContainerInfo {
	private:
		tObjectInfo	m_info;
	public:
		CObjectInfo () { memset (&m_info, 0, sizeof (m_info)); }

		inline int& Signature () { return m_info.nSignature; }
		inline ubyte& Id () { return m_info.nId; }
		inline fix& Size () { return m_info.xSize; }
		inline fix& Shields () { return m_info.xShields; }
		inline fix& LifeLeft () { return m_info.xLifeLeft; }
		inline short& Segment () { return m_info.nSegment; }
		inline short& AttachedObj () { return m_info.nAttachedObj; }
		inline short& NextInSeg () { return m_info.nNextInSeg; }
		inline short& PrevInSeg () { return m_info.nPrevInSeg; }
		inline sbyte& Creator () { return m_info.nCreator; }
		inline ubyte& Type () { return m_info.nType; }
		inline ubyte& ControlType () { return m_info.controlType; }
		inline ubyte& MovementType () { return m_info.movementType; }
		inline ubyte& RenderType () { return m_info.renderType; }
		inline ubyte& Flags () { return m_info.nFlags; }
		inline vmsVector& LastPos () { return m_info.vLastPos; }
};

// TODO get rid of the structs (former unions) and the union
typedef struct tCoreObject {
	tObjectInfo			info;
	// movement info, determined by MOVEMENT_TYPE
	union {
		tPhysicsInfo	physInfo; // a physics tObject
		vmsVector   	spinRate; // for spinning objects
		} mType;
	// control info, determined by CONTROL_TYPE
	union {
		tLaserInfo		laserInfo;
		tExplosionInfo explInfo;      // NOTE: debris uses this also
		tAIStaticInfo  aiInfo;
		tObjLightInfo  lightInfo;     // why put this here?  Didn't know what else to do with it.
		tPowerupInfo   powerupInfo;
		} cType;
	// render info, determined by RENDER_TYPE
	union {
		tPolyObjInfo   polyObjInfo;      // polygon model
		tVClipInfo     vClipInfo;     // tVideoClip
		tSmokeInfo		smokeInfo;
		tLightningInfo	lightningInfo;
		} rType;
#ifdef WORDS_NEED_ALIGNMENT
	short   nPad;
#endif
} tCoreObject;

struct tObject;

typedef struct tObjListLink {
	tObject	*prev, *next;
} tObjListLink;

typedef struct tObject : public tCoreObject {
	tObjListLink	links [3];		// link into list of objects in same category (0: all, 1: same type, 2: same class)
	ubyte				nLinkedType;
} tObject;

class CObject : public CObjectInfo {
	private:
		CObject	*m_prevObjP, *m_nextObjP;

	public:
		CObject ();
		~CObject ();
		// initialize a new tObject.  adds to the list for the given tSegment
		// returns the tObject number
		static int Create (ubyte nType, ubyte nId, short nCreator, short nSegment, const vmsVector& vPos,
								 const vmsMatrix& mOrient, fix xSize, ubyte cType, ubyte mType, ubyte rType, int bIgnoreLimits);

		inline CObject*& Prev () { return m_prevObjP; }
		inline CObject*& Next () { return m_nextObjP; }
		inline void Kill (void) { Flags () |= OF_SHOULD_BE_DEAD; }
		inline bool Exists (void) { return !(Flags () & (OF_EXPLODING | OF_SHOULD_BE_DEAD | OF_DESTROYED)); }
		// unlinks an tObject from a tSegment's list of objects
		void Link (short nSegment);
		void Unlink (void);
		void Initialize (ubyte nType, ubyte nId, short nCreator, short nSegment, const vmsVector& vPos,
							  const vmsMatrix& mOrient, fix xSize, ubyte cType, ubyte mType, ubyte rType);
		void ToStruct (tCoreObject *objP);
};


class CRobotObject : public CObject, public CPhysicsInfo, public CAIStaticInfo, public CPolyObjInfo {
	public:
		CRobotObject () {}
		~CRobotObject () {}
		void Initialize (void) {};
		void ToStruct (tCoreObject *objP);
};

class CPowerupObject : public CObject, public CPhysicsInfo, public CPolyObjInfo {
	public:
		CPowerupObject () {}
		~CPowerupObject () {}
		void Initialize (void) {};
		void ToStruct (tCoreObject *objP);
};

class CWeaponObject : public CObject, public CPhysicsInfo, public CPolyObjInfo {
	public:
		CWeaponObject () {}
		~CWeaponObject () {}
		void Initialize (void) {};
		void ToStruct (tCoreObject *objP);
};

class CLightObject : public CObject, public CObjLightInfo {
	public:
		CLightObject () {};
		~CLightObject () {};
		void Initialize (void) {};
		void ToStruct (tCoreObject *objP);
};

class CLightningObject : public CObject, public CLightningInfo {
	public:
		CLightningObject () {};
		~CLightningObject () {};
		void Initialize (void) {};
		void ToStruct (tCoreObject *objP);
};

class CSmokeObject : public CObject, public CSmokeInfo {
	public:
		CSmokeObject () {};
		~CSmokeObject () {};
		void Initialize (void) {};
};




typedef struct tObjPosition {
	tTransformation	position;
	short					nSegment;     // tSegment number containing tObject
	short					nSegType;		// nType of tSegment
} tObjPosition;

class CObjPosition : public CTransformation {
	private:
		short		m_nSegment;
		short		m_nSegType;
	public:
		inline short& Segment () { return m_nSegment; }
		inline short& SegType () { return m_nSegType; }
};

typedef struct tWindowRenderedData {
	int     nFrame;
	tObject *viewerP;
	int     bRearView;
	int     nUser;
	int     nObjects;
	short   renderedObjects [MAX_RENDERED_OBJECTS];
} tWindowRenderedData;

class WIndowRenderedData {
	private:
		tWindowRenderedData	m_data;
	public:
		inline int& Frame () { return m_data.nFrame; }
		inline int& RearView () { return m_data.bRearView; }
		inline int& User () { return m_data.nUser; }
		inline int& Objects () { return m_data.nObjects; }
		inline short& RenderedObjects (int i) { return m_data.renderedObjects [i]; }
		inline tObject *Viewer () { return m_data.viewerP; }
};

typedef struct tObjDropInfo {
	time_t	nDropTime;
	short		nPowerupType;
	short		nPrevPowerup;
	short		nNextPowerup;
	short		nObject;
} tObjDropInfo;

class CObjDropInfo {
	private:
		tObjDropInfo	m_info;
	public:
		inline time_t& Time () { return m_info.nDropTime; }
		inline short& Type () { return m_info.nPowerupType; }
		inline short& Prev () { return m_info.nPrevPowerup; }
		inline short& Next () { return m_info.nNextPowerup; }
		inline short& Object () { return m_info.nObject; }
};

class tObjectRef {
public:
	short		objIndex;
	short		nextObj;
};

#define MAX_RENDERED_WINDOWS    3

extern tWindowRenderedData windowRenderedData [MAX_RENDERED_WINDOWS];

/*
 * VARIABLES
 */

// ie gameData.objs.collisionResult[a][b]==  what happens to a when it collides with b

extern char *robot_names[];         // name of each robot

extern tObject Follow;

/*
 * FUNCTIONS
 */


// do whatever setup needs to be done
void InitObjects();

int CreateObject (ubyte nType, ubyte nId, short nCreator, short nSegment, const vmsVector& vPos, const vmsMatrix& mOrient,
					   fix xSize, ubyte cType, ubyte mType, ubyte rType);
int CloneObject (tObject *objP);
int CreateRobot (ubyte nId, short nSegment, const vmsVector& vPos);
int CreatePowerup (ubyte nId, short nCreator, short nSegment, const vmsVector& vPos, int bIgnoreLimits);
int CreateWeapon (ubyte nId, short nCreator, short nSegment, const vmsVector& vPos, fix xSize, ubyte rType);
int CreateFireball (ubyte nId, short nSegment, const vmsVector& vPos, fix xSize, ubyte rType);
int CreateDebris (tObject *parentP, short nSubModel);
int CreateCamera (tObject *parentP);
int CreateLight (ubyte nId, short nSegment, const vmsVector& vPos);
// returns tSegment number tObject is in.  Searches out from tObject's current
// seg, so this shouldn't be called if the tObject has "jumped" to a new seg
// -- unused --
//int obj_get_new_seg(tObject *obj);

// when an tObject has moved into a new tSegment, this function unlinks it
// from its old tSegment, and links it into the new tSegment
void RelinkObjToSeg(int nObject,int nNewSeg);

// move an tObject from one tSegment to another. unlinks & relinks
// -- unused --
//void obj_set_new_seg(int nObject,int newsegnum);

// links an tObject into a tSegment's list of objects.
// takes tObject number and tSegment number
void LinkObjToSeg(int nObject,int nSegment);

// unlinks an tObject from a tSegment's list of objects
void UnlinkObjFromSeg (tObject *objP);

// initialize a new tObject.  adds to the list for the given tSegment
// returns the tObject number
//int CObject::Create(ubyte nType, char id, short owner, short nSegment, const vmsVector& pos,
//               const vmsMatrix& orient, fix size, ubyte ctype, ubyte mtype, ubyte rtype, int bIgnoreLimits);

// make a copy of an tObject. returs num of new tObject
int ObjectCreateCopy(int nObject, vmsVector *new_pos, int newsegnum);

// remove tObject from the world
void ReleaseObject(short nObject);

// called after load.  Takes number of objects, and objects should be
// compressed
void ResetObjects (int nObjects);
void ConvertObjects (void);
void SetupEffects (void);

// make tObject array non-sparse
void compressObjects(void);

// Draw a blob-nType tObject, like a fireball
// Deletes all objects that have been marked for death.
void CleanupObjects();

// Toggles whether or not lock-boxes draw.
void object_toggle_lock_targets();

// move all objects for the current frame
int UpdateAllObjects();     // moves all objects

// set viewer tObject to next tObject in array
void object_goto_nextViewer();

// draw target boxes for nearby robots
void object_render_targets(void);

// move an tObject for the current frame
int UpdateObject(tObject * obj);

// make object0 the tPlayer, setting all relevant fields
void InitPlayerObject();

// check if tObject is in tObject->nSegment.  if not, check the adjacent
// segs.  if not any of these, returns false, else sets obj->nSegment &
// returns true callers should really use FindVectorIntersection()
// Note: this function is in gameseg.c
extern int UpdateObjectSeg(tObject *obj);


// Finds what tSegment *obj is in, returns tSegment number.  If not in
// any tSegment, returns -1.  Note: This function is defined in
// gameseg.h, but object[HA] depends on gameseg.h, and object[HA] is where
// tObject is defined...get it?
extern int FindObjectSeg(tObject * obj);

// go through all objects and make sure they have the correct tSegment
// numbers used when debugging is on
void FixObjectSegs();

// Drops objects contained in objp.
int ObjectCreateEgg(tObject *objp);

// Interface to ObjectCreateEgg, puts count objects of nType nType, id
// = id in objp and then drops them.
int CallObjectCreateEgg(tObject *objp, int count, int nType, int id);

extern void DeadPlayerEnd(void);

// Extract information from an tObject (objp->orient, objp->pos,
// objp->nSegment), stuff in a tShortPos structure.  See typedef
// tShortPos.
extern void CreateShortPos(tShortPos *spp, tObject *objp, int swap_bytes);

// Extract information from a tShortPos, stuff in objp->orient
// (matrix), objp->pos, objp->nSegment
extern void ExtractShortPos(tObject *objp, tShortPos *spp, int swap_bytes);

// delete objects, such as weapons & explosions, that shouldn't stay
// between levels if clear_all is set, clear even proximity bombs
void ClearTransientObjects(int clear_all);

// returns the number of a free tObject, updating HighestObject_index.
// Generally, CObject::Create() should be called to get an tObject, since it
// fills in important fields and does the linking.  returns -1 if no
// free objects
int AllocObject(void);
int InsertObject (int nObject);

// frees up an tObject.  Generally, ReleaseObject() should be called to
// get rid of an tObject.  This function deallocates the tObject entry
// after the tObject has been unlinked
void FreeObject(int nObject);

// after calling initObject(), the network code has grabbed specific
// tObject slots without allocating them.  Go though the objects &
// build the free list, then set the apporpriate globals Don't call
// this function if you don't know what you're doing.
void SpecialResetObjects(void);

// attaches an tObject, such as a fireball, to another tObject, such as
// a robot
void AttachObject(tObject *parent, tObject *sub);

extern void CreateSmallFireballOnObject(tObject *objp, fix size_scale, int soundFlag);

// returns tObject number
int DropMarkerObject(vmsVector *pos, short nSegment, vmsMatrix *orient, ubyte marker_num);

extern void WakeupRenderedObjects(tObject *gmissp, int window_num);

extern void AdjustMineSpawn();

void ResetPlayerObject(void);
void StopObjectMovement (tObject *obj);
void StopPlayerMovement (void);

int ObjectSoundClass (tObject *objP);

void ObjectGotoNextViewer();
void ObjectGotoPrevViewer();

int ObjectCount (int nType);

void ResetChildObjects (void);
int AddChildObjectN (int nParent, int nChild);
int AddChildObjectP (tObject *pParent, tObject *pChild);
int DelObjChildrenN (int nParent);
int DelObjChildrenP (tObject *pParent);
int DelObjChildN (int nChild);
int DelObjChildP (tObject *pChild);

void LinkObject (tObject *objP);
void UnlinkObject (tObject *objP);

void BuildObjectModels (void);

tObjectRef *GetChildObjN (short nParent, tObjectRef *pChildRef);
tObjectRef *GetChildObjP (tObject *pParent, tObjectRef *pChildRef);

tObject *ObjFindFirstOfType (int nType);
void InitWeaponFlags (void);
float ObjectDamage (tObject *objP);
int FindBoss (int nObject);
void InitGateIntervals (void);
int CountPlayerObjects (int nPlayer, int nType, int nId);
void FixObjectSizes (void);
void DoSlowMotionFrame (void);
vmsMatrix *ObjectView (tObject *objP);

vmsVector *PlayerSpawnPos (int nPlayer);
vmsMatrix *PlayerSpawnOrient (int nPlayer);
void GetPlayerSpawn (int nPlayer, tObject *objP);
void RecreateThief(tObject *objP);
void DeadPlayerFrame (void);

void SetObjectType (tObject *objP, ubyte nNewType);

extern ubyte bIsMissile [];

#define	OBJ_CLOAKED(_objP)	((_objP)->ctype.aiInfo.flags [6])

#define	SHOW_SHADOWS \
			(!gameStates.render.bRenderIndirect && \
			 gameStates.app.bEnableShadows && \
			 EGI_FLAG (bShadows, 0, 1, 0) && \
			 !COMPETITION)

#define	SHOW_OBJ_FX \
			(!(gameStates.app.bNostalgia || COMPETITION))

#define	IS_BOSS(_objP)			(((_objP)->info.nType == OBJ_ROBOT) && ROBOTINFO ((_objP)->info.nId).bossFlag)
#define	IS_GUIDEBOT(_objP)	(((_objP)->info.nType == OBJ_ROBOT) && ROBOTINFO ((_objP)->info.nId).companion)
#define	IS_THIEF(_objP)		(((_objP)->info.nType == OBJ_ROBOT) && ROBOTINFO ((_objP)->info.nId).thief)
#define	IS_BOSS(_objP)			(((_objP)->info.nType == OBJ_ROBOT) && ROBOTINFO ((_objP)->info.nId).bossFlag)
#define	IS_BOSS_I(_i)			IS_BOSS (gameData.objs.objects + (_i))
#define	IS_MISSILE(_objP)		(((_objP)->info.nType == OBJ_WEAPON) && gameData.objs.bIsMissile [(_objP)->info.nId])
#define	IS_MISSILE_I(_i)		IS_MISSILE (gameData.objs.objects + (_i))

#if DBG
extern tObject *dbgObjP;
#endif

#define SET_COLLISION(type1, type2, result) \
	gameData.objs.collisionResult [type1][type2] = result; \
	gameData.objs.collisionResult [type2][type1] = result;

#define ENABLE_COLLISION(type1, type2)		SET_COLLISION(type1, type2, RESULT_CHECK)

#define DISABLE_COLLISION(type1, type2)	SET_COLLISION(type1, type2, RESULT_NOTHING)

#define OBJECT_EXISTS(_objP)	 ((_objP) && !((_objP)->info.nFlags & (OF_EXPLODING | OF_SHOULD_BE_DEAD | OF_DESTROYED)))

#	define FORALL_OBJSi(_objP,_i)						for ((_objP) = OBJECTS, (_i) = 0; (_i) <= gameData.objs.nLastObject [0]; (_i)++, (_objP)++)
#if 0
#	define FORALL_CLASS_OBJS(_type,_objP,_i)		for ((_objP) = OBJECTS, (_i) = 0; i <= gameData.objs.nLastObject [0]; (_i)++, (_objP)++) if ((_objP)->info.nType == _type)
#	define FORALL_ACTOR_OBJS(_objP,_i)				FORALL_CLASS_OBJS (OBJ_ROBOT, _objP, _i)
#	define FORALL_POWERUP_OBJS(_objP,_i)			FORALL_CLASS_OBJS (OBJ_POWERUP, _objP, _i)
#	define FORALL_WEAPON_OBJS(_objP,_i)				FORALL_CLASS_OBJS (OBJ_WEAPON, _objP, _i)
#	define FORALL_EFFECT_OBJS(_objP,_i)				FORALL_CLASS_OBJS (OBJ_EFFECT, _objP, _i)
�	define IS_OBJECT(_objP, _i)						((_i) <= gameData.objs.nLastObject [0])
#else
#	define FORALL_OBJS(_objP,_i)							for ((_objP) = gameData.objs.lists.all.head; (_objP); (_objP) = (_objP)->links [0].next)
#	define FORALL_SUPERCLASS_OBJS(_list,_objP,_i)	for ((_objP) = (_list).head; (_objP); (_objP) = (_objP)->links [2].next)
#	define FORALL_CLASS_OBJS(_list,_objP,_i)			for ((_objP) = (_list).head; (_objP); (_objP) = (_objP)->links [1].next)
#	define FORALL_PLAYER_OBJS(_objP,_i)					FORALL_CLASS_OBJS (gameData.objs.lists.players, _objP, _i)
#	define FORALL_ROBOT_OBJS(_objP,_i)					FORALL_CLASS_OBJS (gameData.objs.lists.robots, _objP, _i)
#	define FORALL_POWERUP_OBJS(_objP,_i)				FORALL_CLASS_OBJS (gameData.objs.lists.powerups, _objP, _i)
#	define FORALL_WEAPON_OBJS(_objP,_i)					FORALL_CLASS_OBJS (gameData.objs.lists.weapons, _objP, _i)
#	define FORALL_EFFECT_OBJS(_objP,_i)					FORALL_CLASS_OBJS (gameData.objs.lists.effects, _objP, _i)
#	define FORALL_LIGHT_OBJS(_objP,_i)					FORALL_CLASS_OBJS (gameData.objs.lists.lights, _objP, _i)
#	define FORALL_ACTOR_OBJS(_objP,_i)					FORALL_SUPERCLASS_OBJS (gameData.objs.lists.actors, _objP, _i)
#	define FORALL_STATIC_OBJS(_objP,_i)					FORALL_SUPERCLASS_OBJS (gameData.objs.lists.statics, _objP, _i)
#	define IS_OBJECT(_objP, _i)							((_objP) != NULL)
#endif

//	-----------------------------------------------------------------------------------------------------------

static inline void KillObject (tObject *objP)
{
objP->info.nFlags |= OF_SHOULD_BE_DEAD;
#if DBG
if (objP == dbgObjP)
	objP = objP;
#endif
}

//	-----------------------------------------------------------------------------------------------------------

#endif
