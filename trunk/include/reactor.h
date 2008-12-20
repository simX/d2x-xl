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

#ifndef _CNTRLCEN_H
#define _CNTRLCEN_H

#include "vecmat.h"
#include "object.h"
#include "wall.h"
//#include "switch.h"

#define MAX_CONTROLCEN_GUNS     8

#define CONTROLCEN_WEAPON_NUM   6

#define MAX_CONTROLCEN_LINKS    10

typedef struct tReactorTriggers {
	short   nLinks;
	short   segments [MAX_CONTROLCEN_LINKS];
	short   sides [MAX_CONTROLCEN_LINKS];
} tReactorTriggers;

typedef struct tReactorProps {
	int nModel;
	int nGuns;
	CFixVector gunPoints [MAX_CONTROLCEN_GUNS];
	CFixVector gunDirs [MAX_CONTROLCEN_GUNS];
} tReactorProps;

#define MAX_REACTORS 7

//@@extern CFixVector controlcen_gun_points[MAX_CONTROLCEN_GUNS];
//@@extern CFixVector controlcen_gun_dirs[MAX_CONTROLCEN_GUNS];
extern CFixVector Gun_pos[MAX_CONTROLCEN_GUNS];

// do whatever this thing does in a frame
void DoReactorFrame(CObject *obj);
// Initialize control center for a level.
// Call when a new level is started.
void InitReactorForLevel(int bRestore);
void DoReactorDestroyedStuff(CObject *objp);
void DoReactorDeadFrame(void);
fix ReactorStrength (void);

/*
 * reads n reactor structs from a CFILE
 */
int ReadReactors (CFile& cf);

/*
 * reads n tReactorTriggers structs from a CFILE
 */
int ReadReactorTriggers (CFile& cf);

int FindReactor (CObject *objP);
void InitCountdown (CTrigger *trigP, int bReactorDestroyed, int nTimer);

#endif /* _CNTRLCEN_H */
