// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"

//------------------------------------------------------------------------
// Mine - add_trigger()
//
// Returns - TRUE on success
//
// Changes - now takes type instead of flag.
//------------------------------------------------------------------------

void CMine::InitTrigger (CDTrigger *t, INT16 type, INT16 flags)
{
t->type = (INT8) type;
t->flags = (INT8) flags;
t->value = (type == TT_SPEEDBOOST) ? 10 : (type == TT_CHANGE_TEXTURE) ? 0 : 5 * F1_0; // 5% shield or energy damage
t->time = -1;
t->num_links = 0;
for (int i = 0; i < MAX_TRIGGER_TARGETS; i++) {
	t->seg [i] = -1;
	t->side [i] = -1;
	}
}

//------------------------------------------------------------------------

CDTrigger *CMine::AddTrigger (UINT16 wallnum, INT16 type, BOOL bAutoAddWall) 
{
	INT16 flags;
	INT16 segnum, sidenum, trignum;
	static INT16 defWallTypes [NUM_TRIGGER_TYPES] = {
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_ILLUSION, 
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, 
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_ILLUSION,
		WALL_OPEN, WALL_OPEN
		};
	static INT16 defWallTextures [NUM_TRIGGER_TYPES] = {
		0, 0, 0, 0, 426, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 426,
		0, 0
		};

// check if there's already a trigger on the current side
wallnum = FindTriggerWall (&trignum);
if (trignum != NO_TRIGGER) {
	ErrorMsg ("There is already a trigger on this side");
	return NULL;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS) {
	ErrorMsg ("The maximum number of triggers has been reached.");
	return NULL;
	}
// if no wall at current side, try to add a wall of proper type
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (CurrSide ()->nWall >= GameInfo ().walls.count) {
	if (bAutoAddWall) {
		if (GameInfo ().walls.count >= MAX_WALLS) {
			ErrorMsg ("Cannot add a wall to this side,\nsince the maximum number of walls is already reached.");
			return NULL;
			}
		segnum = sidenum = -1;
		GetCurrent (segnum, sidenum);
		if (!AddWall (-1, -1, (Segments (segnum)->children [sidenum] < 0) ? WALL_OVERLAY : defWallTypes [type], 0, 0, -1, defWallTextures [type])) {
			ErrorMsg ("Cannot add a wall for this trigger.");
			theApp.ResetModified (bUndo);
			return NULL;
			}
		}
	else {
		ErrorMsg ("You must add a wall to this side before you can add a trigger.");
		return NULL;
		}
	}
// if D1, then convert type to flag value
if (file_type == RDL_FILE) {
	switch(type) {
		case TT_OPEN_DOOR:
			flags = TRIGGER_CONTROL_DOORS;
			break;
		case TT_MATCEN:
			flags = TRIGGER_MATCEN;
			break;
		case TT_EXIT:
			flags = TRIGGER_EXIT;
			break;
		case TT_SECRET_EXIT:
			flags = TRIGGER_SECRET_EXIT;
			break;
		case TT_ILLUSION_OFF:
			flags = TRIGGER_ILLUSION_OFF;
			break;
		case TT_ILLUSION_ON:
			flags = TRIGGER_ILLUSION_ON;
			break;
		case TT_ENERGY_DRAIN:
			flags = TRIGGER_ENERGY_DRAIN;
			break;
		case TT_SHIELD_DAMAGE:
			flags = TRIGGER_SHIELD_DAMAGE;
			break;
		default:
			flags = 0;
		}
	type = 0;
	}
else
	flags = 0;

trignum = (UINT16) GameInfo ().triggers.count;
// set new trigger data
InitTrigger (Triggers (trignum), type, flags);
// link trigger to the wall
Walls (wallnum)->trigger = (UINT8) trignum;
// update number of Triggers ()
GameInfo ().triggers.count++;
AutoLinkExitToReactor();
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return Triggers (trignum);
}

//------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------

void CMine::DeleteTrigger (INT16 nTrigger) 
{
	INT16	i, nSegment, nSide, nWall;

if (nTrigger < 0) {
	nWall = CurrSeg ()->sides [Current ()->side].nWall;
	if (nWall >= GameInfo ().walls.count)
		return;
	nTrigger = Walls (nWall)->trigger;
	}
if (nTrigger >= GameInfo ().triggers.count)
	return;
// update all Walls () who point to Triggers () higher than this one
// and unlink all Walls () who point to deleted trigger (should be only one wall)
theApp.SetModified (TRUE);
theApp.LockUndo ();
CDWall *wallP = Walls ();
for (i = GameInfo ().walls.count; i; i--, wallP++)
	if ((wallP->trigger != NO_TRIGGER) && (wallP->trigger > nTrigger))
		wallP->trigger--;
	else if (wallP->trigger == nTrigger) {
		wallP->trigger = NO_TRIGGER;
		nSegment = wallP->segnum;
		nSide = wallP->sidenum;
		}
// remove trigger from array
//for (i=nTrigger;i<GameInfo ().triggers.count-1;i++)
// update number of Triggers ()
CDTrigger *trigP = Triggers ();
for (i = NumTriggers (); i; i--, trigP++)
	if (trigP->type == TT_MASTER)
		DeleteTriggerTarget (trigP, nSegment, nSide, false);
if (nTrigger < --GameInfo ().triggers.count)
	memcpy(Triggers (nTrigger), Triggers (nTrigger + 1), (GameInfo ().triggers.count - nTrigger) * sizeof(CDTrigger));
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
AutoLinkExitToReactor();
}

//------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------

int CMine::DeleteTargetFromTrigger (CDTrigger *trigger, INT16 linknum, bool bAutoDeleteTrigger)
{
if (!--trigger->num_links) {
	if (bAutoDeleteTrigger)
		DeleteTrigger ();
	return 0;
	}
if (linknum < trigger->num_links) {
	memcpy (trigger->seg + linknum, trigger->seg + linknum + 1, (trigger->num_links - linknum) * sizeof (*(trigger->seg)));
	memcpy (trigger->side + linknum, trigger->side + linknum + 1, (trigger->num_links - linknum) * sizeof (*(trigger->side)));
	}
return trigger->num_links;
}


int CMine::DeleteTargetFromTrigger (INT16 trignum, INT16 linknum, bool bAutoDeleteTrigger)
{
return DeleteTargetFromTrigger (Triggers (trignum), linknum, bAutoDeleteTrigger);
}


bool CMine::DeleteTriggerTarget (CDTrigger *trigger, INT16 segnum, INT16 sidenum, bool bAutoDeleteTrigger) 
{
for (int j = 0; j < trigger->num_links; j++)
	if ((trigger->seg [j] == segnum) && (trigger->side [j] == sidenum))
		return DeleteTargetFromTrigger (trigger, j, bAutoDeleteTrigger) == 0;
return false;
}


void CMine::DeleteTriggerTargets (INT16 segnum, INT16 sidenum) 
{
int i;

for (i = 0; i < GameInfo ().triggers.count; i++)
	if (DeleteTriggerTarget (Triggers (i), segnum, sidenum))
		i--;

for (i = 0; i < NumObjTriggers (); i++)
	if (DeleteTriggerTarget (ObjTriggers (i), segnum, sidenum, false)) {
		DeleteObjTrigger (i);
		i--;
		}
}

//------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------

INT16 CMine::FindTriggerWall (INT16 *trignum, INT16 segnum, INT16 sidenum)
{
GetCurrent (segnum, sidenum);
CDWall *wall = Walls ();
for (int wallnum = GameInfo ().walls.count; wallnum; wallnum--, wall++) {
	if ((wall->segnum == segnum) && (wall->sidenum == sidenum)) {
		*trignum = wall->trigger;
		return wall - Walls ();
		}
	}
*trignum = NO_TRIGGER;
return GameInfo ().walls.count;
}

INT16 CMine::FindTriggerWall (INT16 trignum)
{
CDWall *wall = Walls ();
for (int wallnum = GameInfo ().walls.count; wallnum; wallnum--, wall++)
	if (wall->trigger == trignum)
		return wall - Walls ();
return GameInfo ().walls.count;
}

INT16 CMine::FindTriggerObject (INT16 *trignum)
{
	INT16 objnum = Current ()->object;

if ((*trignum = *ObjTriggerRoot (objnum)) < 0) {
	*trignum = NO_TRIGGER;
	return -1;
	}
return objnum;
}

//------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------

INT16 CMine::FindTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum)
{
CDTrigger *trigger = Triggers ();
int i, j;

for (i = trignum; i < GameInfo ().triggers.count; i++, trigger++)
	for (j = 0; j < trigger->num_links; j++)
		if ((trigger->seg [j] == segnum) && (trigger->side [j] == sidenum))
			return i;
return -1;
}

//------------------------------------------------------------------------
// AutoLinkExitToReactor()
//
// Action - Updates control center Triggers () so that exit door opens
//          when the reactor blows up.  Removes any invalid cube/sides
//          from control_center_triggers if they exist.
//------------------------------------------------------------------------

void CMine::AutoLinkExitToReactor () 
{
  INT16 	linknum,control,num_links,segnum,sidenum;
  UINT16	wallnum;
  INT8 	trignum;
  bool 	found;

  control = 0; // only 0 used by the game Descent
  control_center_trigger *ccTrigger = CCTriggers (control);

theApp.SetModified (TRUE);
theApp.LockUndo ();
// remove items from list that do not point to a wall
for (linknum = 0; linknum < ccTrigger->num_links; linknum++) {
	num_links = ccTrigger->num_links;
	segnum = ccTrigger->seg [linknum];
	sidenum = ccTrigger->side [linknum];
	// search for Walls () that have a exit of type trigger
	found = FALSE;
	for (wallnum=0;wallnum<GameInfo ().walls.count;wallnum++) {
		if (Walls (wallnum)->segnum == segnum && Walls (wallnum)->sidenum == sidenum) {
		found = TRUE;
		break;
		}
	}
	if (!found) {
		if (num_links > 0) { // just in case
			// move last link into this deleted link's spot
			ccTrigger->seg [linknum] = ccTrigger->seg [num_links-1];
			ccTrigger->side [linknum] = ccTrigger->side [num_links-1];
			ccTrigger->seg [num_links-1] = 0;
			ccTrigger->side [num_links-1] = 0;
			ccTrigger->num_links--;
			}
		}
	}

// add exit to list if not already in list
// search for Walls () that have a exit of type trigger
num_links =  ccTrigger->num_links;
for (wallnum = 0; wallnum < GameInfo ().walls.count; wallnum++) {
	trignum = Walls (wallnum)->trigger;
	if (trignum >= 0 && trignum <GameInfo ().triggers.count) {
		if ((file_type == RDL_FILE) ?
			 Triggers (trignum)->flags & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) :
			 Triggers (trignum)->type == TT_EXIT || Triggers (trignum)->type == TT_SECRET_EXIT) {
			// see if cube,side is already on the list
			segnum = (INT16)Walls (wallnum)->segnum;
			sidenum =(INT16)Walls (wallnum)->sidenum;
			found = FALSE;
			for (linknum = 0; linknum < num_links; linknum++) {
				if (segnum == ccTrigger->seg [linknum] && sidenum == ccTrigger->side [linknum]) {
					found = TRUE;
					break;
					}
				}
			// if not already on the list, add it
			if (!found) {
				linknum = ccTrigger->num_links;
				ccTrigger->seg [linknum] = segnum;
				ccTrigger->side [linknum] = sidenum;
				ccTrigger->num_links++;
				}
			}
		}
	}
theApp.UnlockUndo ();
}

//------------------------------------------------------------------------

CDObjTriggerList *CMine::LinkObjTrigger (INT16 objnum, INT16 trignum)
{
short i = *ObjTriggerRoot (objnum);
CDObjTriggerList *ot = ObjTriggerList (trignum);
ot->prev = -1;
ot->next = i;
ot->objnum = objnum;
*ObjTriggerRoot (objnum) = trignum;
ObjTriggerList (i)->prev = trignum;
return ot;
}

//------------------------------------------------------------------------

CDObjTriggerList *CMine::AddObjTrigger (INT16 objnum, INT16 type) 
{
if (objnum < 0)
	objnum = Current ()->object;
if ((Objects (objnum)->type != OBJ_ROBOT) && 
	 (Objects (objnum)->type != OBJ_CAMBOT) &&
	 (Objects (objnum)->type != OBJ_POWERUP) &&
	 (Objects (objnum)->type != OBJ_HOSTAGE) &&
	 (Objects (objnum)->type != OBJ_CNTRLCEN)) {
	ErrorMsg ("Object triggers can only be attached to robots, reactors, hostages, powerups and cameras.");
	return NULL;
	}
if (NumObjTriggers () >= MAX_OBJ_TRIGGERS) {
	ErrorMsg ("The maximum number of object triggers has been reached.");
	return NULL;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
short trignum = NumObjTriggers ();
CDObjTriggerList *ot = ObjTriggerList (NumObjTriggers ());
ot = LinkObjTrigger (objnum, trignum);
InitTrigger (ObjTriggers (trignum), type, 0);
NumObjTriggers ()++;
theApp.UnlockUndo ();
return ot;
}

//------------------------------------------------------------------------

bool CMine::ObjTriggerIsInList (INT16 nTrigger)
{
CDObjTriggerList	*ot0 = ObjTriggerList (nTrigger),
						*ot1 = ObjTriggerList (*ObjTriggerRoot (ot0->objnum));

for (;;) {
	if (ot1->next < 0)
		break;
	ot1 = ObjTriggerList (ot1->next);
	if (ot1->objnum != ot0->objnum)
		return false;
	}
return true;
}

//------------------------------------------------------------------------

CDObjTriggerList *CMine::UnlinkObjTrigger (INT16 nTrigger)
{
CDObjTriggerList *ot = ObjTriggerList (nTrigger);
if (ot->prev < 0) {
	*ObjTriggerRoot (ot->objnum) = ot->next;
	ObjTriggerList (ot->next)->prev = -1;
	}
else
	ObjTriggerList (ot->prev)->next = ot->next;
if (ot->next >= 0)
	ObjTriggerList (ot->next)->prev = ot->prev;
return ot;
}

//------------------------------------------------------------------------

void CMine::DeleteObjTrigger (INT16 trignum) 
{
if ((trignum < 0) || (trignum >= NumObjTriggers ()))
	return;
CDObjTriggerList *ot = UnlinkObjTrigger (trignum);
if (trignum < --NumObjTriggers ()) {
	// move the last trigger in the object trigger table to the free table entry and relink it
	*ot = *ObjTriggerList (NumObjTriggers ());
	if (ot->prev < 0)
		*ObjTriggerRoot (ot->objnum) = trignum;
	else
		ObjTriggerList (ot->prev)->next = trignum;
	if (ot->next >= 0)
		ObjTriggerList (ot->next)->prev = trignum;
	*ObjTriggers (trignum) = *ObjTriggers (NumObjTriggers ());
	}
else
	ot->prev = 
	ot->next = 
	ot->objnum = -1;
}

//------------------------------------------------------------------------

void CMine::DeleteObjTriggers (INT16 objnum) 
{
	INT16				h, i = *ObjTriggerRoot (objnum); 
	
while (i >= 0) {
	h = ObjTriggerList (i)->next;
	DeleteObjTrigger (i);
	i = h;
	}
}

//------------------------------------------------------------------------

INT16 CMine::FindObjTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum)
{
CDObjTriggerList *ot = ObjTriggerList ();
CDTrigger *t = ObjTriggers ();
int i, j;

for (i = trignum; i < NumObjTriggers (); i++, ot++, t++)
	for (j = 0; j < t->num_links; j++)
		if ((t->seg [j] == segnum) && (t->side [j] == sidenum))
			return i;
return -1;
}

//------------------------------------------------------------------------
//eof trigger.cpp