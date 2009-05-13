// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"

//==========================================================================
// SetDefaultTexture
//==========================================================================

bool CMine::SetDefaultTexture (INT16 tmapnum, INT16 walltype)
{
if (tmapnum < 0)
	return true;
INT16 segnum = Current ()->segment;
INT16 opp_segnum, opp_sidenum;
CDSegment *seg = Segments (segnum);
CDSide *side = seg->sides;
double scale = pTextures [file_type][tmapnum].Scale (tmapnum);
seg->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
for (INT16 sidenum = 0; sidenum < 6; sidenum++, side++) {
	if (seg->children [sidenum] == -1) {
		SetTexture (segnum, sidenum, tmapnum, 0);
	/*
		side->uvls [0].l = 0x8000;
		side->uvls [1].l = 0x8000;
		side->uvls [2].l = 0x8000;
		side->uvls [3].l = 0x8000;
	*/
		for (int i = 0; i < 4; i++) {
			side->uvls [i].u = (INT16) ((double) default_uvls [i].u / scale);
			side->uvls [i].v = (INT16) ((double) default_uvls [i].v / scale);
			side->uvls [i].l = default_uvls [i].l;
			}
		SetUV (segnum, sidenum, 0, 0, 0);
		}
	else if (tmapnum >= 0) {
		if (walltype >= 0) {
			if ((GameInfo ().walls.count < MAX_WALLS) &&
				 (Segments (segnum)->sides [sidenum].nWall >= GameInfo ().walls.count))
				AddWall (segnum, sidenum, (UINT8) walltype, 0, KEY_NONE, -1, -1); // illusion
			else
				return false;
			if ((GameInfo ().walls.count < MAX_WALLS) &&
				 GetOppositeSide (opp_segnum, opp_sidenum, segnum, sidenum) &&
				 (Segments (opp_segnum)->sides [opp_sidenum].nWall >= GameInfo ().walls.count))
				AddWall (opp_segnum, opp_sidenum, (UINT8) walltype, 0, KEY_NONE, -1, -1); // illusion
			else
				return false;
			}
		}
	}
return true;
}

//==========================================================================
// UndefineSegment
//==========================================================================

void CMine::UndefineSegment (INT16 segnum)
{
	CDSegment *seg = (segnum < 0) ? CurrSeg () : Segments (segnum);

segnum = seg - Segments ();
if (seg->special == SEGMENT_IS_ROBOTMAKER) {
	// remove matcen
	int nMatCens = (int) GameInfo ().botgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = seg->matcen_num;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (BotGens (nDelMatCen), BotGens (nMatCens), sizeof (matcen_info));
			BotGens (nDelMatCen)->fuelcen_num = nDelMatCen;
			seg->matcen_num = -1;
			}
		GameInfo ().botgen.count--;
		for (int i = 0; i < 6; i++)
			DeleteTriggerTargets (segnum, i);
		CDSegment *s;
		for (i = SegCount (), s = Segments (); i; i--, s++)
			if ((seg->special == SEGMENT_IS_ROBOTMAKER) && (s->matcen_num == nMatCens)) {
				s->matcen_num = nDelMatCen;
				break;
				}
		}
	seg->matcen_num = -1;
	}
if (seg->special == SEGMENT_IS_EQUIPMAKER) {
	// remove matcen
	int nMatCens = (int) GameInfo ().equipgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = seg->matcen_num;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (EquipGens (nDelMatCen), EquipGens (nMatCens), sizeof (matcen_info));
			EquipGens (nDelMatCen)->fuelcen_num = nDelMatCen;
			seg->matcen_num = -1;
			}
		GameInfo ().equipgen.count--;
		for (int i = 0; i < 6; i++)
			DeleteTriggerTargets (segnum, i);
		CDSegment *s;
		nDelMatCen += (int) GameInfo ().botgen.count;
		for (i = SegCount (), s = Segments (); i; i--, s++)
			if ((s->special == SEGMENT_IS_EQUIPMAKER) && (s->matcen_num == nMatCens)) {
				s->matcen_num = nDelMatCen;
				break;
			}
		}
	seg->matcen_num = -1;
	}
else if (seg->special == SEGMENT_IS_FUELCEN) { //remove all fuel cell walls
	CDSegment *childseg;
	CDSide *oppside, *side = seg->sides;
	CDWall *wall;
	INT16 opp_segnum, opp_sidenum;
	for (INT16 sidenum = 0; sidenum < 6; sidenum++, side++) {
		if (seg->children [sidenum] < 0)	// assume no wall if no child segment at the current side
			continue;
		childseg = Segments (seg->children [sidenum]);
		if (childseg->special == SEGMENT_IS_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		if ((wall = GetWall (segnum, sidenum)) && 
			 (wall->type == WALL_ILLUSION) && (side->nBaseTex == (file_type == RDL_FILE) ? 322 : 333))
			DeleteWall (side->nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		if (GetOppositeSide (opp_segnum, opp_sidenum, segnum, sidenum) &&
			 (wall = GetWall (segnum, sidenum)) && (wall->type == WALL_ILLUSION)) {
			oppside = Segments (opp_segnum)->sides + opp_sidenum;
			if (oppside->nBaseTex == (file_type == RDL_FILE) ? 322 : 333)
				DeleteWall (oppside->nWall);
			}
		}
	}
seg->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
seg->special = SEGMENT_IS_NOTHING;
}

//==========================================================================
// DefineSegment
//==========================================================================

bool CMine::DefineSegment (INT16 segnum, UINT8 type, INT16 tmapnum, INT16 walltype)
{
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
UndefineSegment (segnum);
CDSegment *seg = (segnum < 0) ? CurrSeg () : Segments (segnum);
seg->special = type;
seg->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (tmapnum, walltype);
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_Reactor
//==========================================================================

bool CMine::AddReactor (INT16 segnum, bool bCreate, bool bSetDefTextures) 
{
#if 0
for (seg = Segments (), i = SegCount (); i; i--, seg++)
	if (seg->special == SEGMENT_IS_CONTROLCEN) {
		if (!bExpertMode)
			ErrorMsg ("There is already a reactor in this mine.");
		return false;
		}
#endif
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_CONTROLCEN, bSetDefTextures ? (file_type == RDL_FILE) ? 10 : 357 : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
if (bCreate) {
	if (!CopyObject (OBJ_CNTRLCEN, segnum)) {
		theApp.ResetModified (bUndo);
		return false; 
		}	
	CurrObj ()->id = (file_type == RDL_FILE) ? 0 : 2;
	AutoLinkExitToReactor ();
	}
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_EquipMaker
//==========================================================================

bool CMine::AddEquipMaker (INT16 segnum, bool bCreate, bool bSetDefTextures) 
{
int n_matcen = (int) GameInfo ().equipgen.count;
if (n_matcen >= MAX_NUM_MATCENS) {
    ErrorMsg("Maximum number of equipment makers reached");
	 return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.MineView ()->DelayRefresh (true);
if (!DefineSegment (segnum, SEGMENT_IS_EQUIPMAKER, -1)) {
	theApp.ResetModified (bUndo);
	theApp.MineView ()->DelayRefresh (false);
	return false; 
	}	
EquipGens (n_matcen)->objFlags [0] = 0;
EquipGens (n_matcen)->objFlags [1] = 0;
EquipGens (n_matcen)->hit_points = 0;
EquipGens (n_matcen)->interval = 0;
EquipGens (n_matcen)->segnum = segnum;
EquipGens (n_matcen)->fuelcen_num = n_matcen;
Segments (Current ()->segment)->value = 
Segments (Current ()->segment)->matcen_num = n_matcen;
GameInfo ().equipgen.count++;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_RobotMaker
//==========================================================================

bool CMine::AddRobotMaker (INT16 segnum, bool bCreate, bool bSetDefTextures) 
{
int n_matcen = (int) GameInfo ().botgen.count;
if (n_matcen >= MAX_NUM_MATCENS) {
    ErrorMsg("Maximum number of robot makers reached");
	 return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.MineView ()->DelayRefresh (true);
if (!DefineSegment (segnum, SEGMENT_IS_ROBOTMAKER,  bSetDefTextures ? (file_type == RDL_FILE) ? 339 : 361 : -1)) {
	theApp.ResetModified (bUndo);
	theApp.MineView ()->DelayRefresh (false);
	return false; 
	}	
BotGens (n_matcen)->objFlags [0] = 8;
BotGens (n_matcen)->objFlags [1] = 0;
BotGens (n_matcen)->hit_points = 0;
BotGens (n_matcen)->interval = 0;
BotGens (n_matcen)->segnum = segnum;
BotGens (n_matcen)->fuelcen_num = n_matcen;
Segments (Current ()->segment)->value = 
Segments (Current ()->segment)->matcen_num = n_matcen;
GameInfo ().botgen.count++;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddGoalCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Flag goals are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, nType, bSetDefTextures ? nTexture : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddTeamCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Team start positions are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, nType, bSetDefTextures ? nTexture : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddWaterOrLavaCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Water cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, nType, bSetDefTextures ? nTexture : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSkyboxCube (INT16 segnum, bool bCreate) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Blocked cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_SKYBOX, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddOutdoorCube (INT16 segnum, bool bCreate) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Blocked cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_OUTDOOR, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSpeedBoostCube (INT16 segnum, bool bCreate) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Speed boost cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_SPEEDBOOST, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddBlockedCube (INT16 segnum, bool bCreate) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("Blocked cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_BLOCKED, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddNoDamageCube (INT16 segnum, bool bCreate) 
{
if (file_type == RDL_FILE) {
	if (!bExpertMode)
		ErrorMsg ("No Damage cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (segnum, SEGMENT_IS_NODAMAGE, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddWaterCube (INT16 segnum, bool bCreate, bool bSetDefTextures) 
{
return AddWaterOrLavaCube (segnum, bCreate, bSetDefTextures, SEGMENT_IS_WATER, 403);
}

bool CMine::AddLavaCube (INT16 segnum, bool bCreate, bool bSetDefTextures) 
{
return AddWaterOrLavaCube (segnum, bCreate, bSetDefTextures, SEGMENT_IS_LAVA, 404);
}

//==========================================================================

int CMine::FuelCenterCount (void)
{
int n_fuelcen = 0;
CDSegment *seg = Segments ();
for (int i = 0; i < SegCount (); i++, seg++)
	if ((seg->special == SEGMENT_IS_FUELCEN) || (seg->special == SEGMENT_IS_REPAIRCEN))
		n_fuelcen++;
return n_fuelcen;
}

//==========================================================================
// MENU - Add_FuelCenter
//==========================================================================

bool CMine::AddFuelCenter (INT16 segnum, UINT8 nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int n_fuelcen = FuelCenterCount ();
CDSegment *seg = Segments ();
if (n_fuelcen >= MAX_NUM_FUELCENS) {
	ErrorMsg ("Maximum number of fuel centers reached.");
	return false;
	}
if ((file_type == RDL_FILE) && (nType == SEGMENT_IS_REPAIRCEN)) {
	if (!bExpertMode)
		ErrorMsg ("Repair centers are not available in Descent 1.");
	return false;
	}
int last_segment = Current ()->segment;
bool bUndo = theApp.SetModified (TRUE);
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
int new_segment = Current ()->segment;
Current ()->segment = last_segment;
if (bSetDefTextures && (nType == SEGMENT_IS_FUELCEN) && (GameInfo ().walls.count < MAX_WALLS))
	AddWall (Current ()->segment, Current ()->side, WALL_ILLUSION, 0, KEY_NONE, -1, -1); // illusion
Current ()->segment = new_segment;
if (!((nType == SEGMENT_IS_FUELCEN) ?
	   DefineSegment (segnum, nType,  bSetDefTextures ? ((file_type == RDL_FILE) ? 322 : 333) : -1, WALL_ILLUSION) :
	   DefineSegment (segnum, nType,  bSetDefTextures ? 433 : -1, -1)) //use the blue goal texture for repair centers
	) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//---------------------------------------------------------------------------
// TMainWindow - add_door()
//
// Action - Adds a wall to both sides of the current side
//---------------------------------------------------------------------------

bool CMine::AddDoor (UINT8 type, UINT8 flags, UINT8 keys, INT8 clipnum, INT16 tmapnum) 
{
  INT16 	opp_segnum, opp_sidenum;
  UINT16 wallnum;

wallnum = CurrSide ()->nWall;
if (wallnum < GameInfo ().walls.count) {
	ErrorMsg("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count + 1 >= MAX_WALLS) {
	ErrorMsg("Maximum number of Walls reached");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
// add a door to the current segment/side
if (AddWall (Current ()->segment, Current ()->side, type, flags, keys, clipnum, tmapnum)) {
	// add a door to the opposite segment/side
	if (GetOppositeSide (opp_segnum, opp_sidenum, Current ()->segment, Current ()->side) &&
		 AddWall (opp_segnum, opp_sidenum, type, flags, keys, clipnum, tmapnum)) {
		theApp.UnlockUndo ();
		theApp.MineView ()->Refresh ();
		return true;
		}
	}
theApp.ResetModified (bUndo);
return false;
}

//==========================================================================
// MENU - Add Auto Door
//==========================================================================

bool CMine::AddAutoDoor (INT8 clipnum, INT16 tmapnum) 
{
return AddDoor (WALL_DOOR, WALL_DOOR_AUTO, KEY_NONE, clipnum,  tmapnum);
}

//==========================================================================
// MENU - Add Prison Door
//==========================================================================

bool CMine::AddPrisonDoor () 
{
return AddDoor (WALL_BLASTABLE, 0, 0, -1, -1);
}

//==========================================================================
// MENU - Add Guide bot door
//==========================================================================

bool CMine::AddGuideBotDoor() 
{
if (file_type == RDL_FILE) {
	ErrorMsg ("Guide bot doors are not allowed in Descent 1");
	return false;
  }
return AddDoor (WALL_BLASTABLE, 0, 0, 46, -1);
}

//==========================================================================
// MENU - Add Fuel Cell
//==========================================================================

bool CMine::AddFuelCell () 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, (file_type == RDL_FILE) ? 328 : 353);
}

//==========================================================================
// MENU - Add Illusionary Wall
//==========================================================================

bool CMine::AddIllusionaryWall () 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, 0);
}

//==========================================================================
// MENU - Add ForceField
//==========================================================================

bool CMine::AddForceField () 
{
if (file_type == RDL_FILE) {
	ErrorMsg("Force fields are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_CLOSED, 0, 0, -1, 420);
}

//==========================================================================
// MENU - Add Fan
//
// note: -2 used for clip to so texture will be used for nOvlTex
//==========================================================================

bool CMine::AddFan ()
{
return AddDoor (WALL_CLOSED, 0, 0, -2, (file_type == RDL_FILE) ? 325 : 354);
}

//==========================================================================
// MENU - Add Water Fall
//==========================================================================

bool CMine::AddWaterFall ()
{
if (file_type == RDL_FILE) {
	ErrorMsg ("Water falls are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_ILLUSION, 0, 0, -1, 401);
}

//==========================================================================
// MENU - Add Lava Fall
//==========================================================================

bool CMine::AddLavaFall() 
{
if (file_type == RDL_FILE) {
	ErrorMsg("Lava falls are not supported in Descent 1");
   return false;
	}
  // key of 5 selects lava fall
return AddDoor (WALL_ILLUSION, 0, 0, -1, 408);
}

//==========================================================================
// MENU - Add Grate
//
// note: -2 used for clip to so texture will be used for nOvlTex
//==========================================================================

bool CMine::AddGrate() 
{
return AddDoor (WALL_CLOSED, 0, 0, -2, (file_type == RDL_FILE) ? 246 : 321);
}

//==========================================================================
// MENU - Add Exit
//==========================================================================

bool CMine::AddNormalExit() 
{
return AddExit (TT_EXIT);
}

//--------------------------------------------------------------------------
// add_exit()
//
// Action - adds a wall and a exit trigger to the current segment/side
//--------------------------------------------------------------------------

bool CMine::AddExit (INT16 type) 
{

UINT16 wallnum = Segments (Current ()->segment)->sides [Current ()->side].nWall;
if (wallnum < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count >= MAX_WALLS - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
// make a new wall and a new trigger
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (AddWall (Current ()->segment, Current ()->side, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
// set clip number and texture
	Walls () [GameInfo ().walls.count-1].clip_num = 10;
	SetTexture (Current ()->segment, Current ()->side, 0, (file_type == RDL_FILE) ? 444 : 508);
	AddTrigger (GameInfo ().walls.count - 1, type);
// add a new wall and trigger to the opposite segment/side
	INT16 opp_segnum, opp_sidenum;
	if (GetOppositeSide (opp_segnum, opp_sidenum, Current ()->segment, Current ()->side) &&
		AddWall (opp_segnum, opp_sidenum, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Walls () [GameInfo ().walls.count - 1].clip_num = 10;
		SetTexture (opp_segnum, opp_sidenum, 0, (file_type == RDL_FILE) ? 444 : 508);
		AutoLinkExitToReactor();
		theApp.UnlockUndo ();
		theApp.MineView ()->Refresh ();
		return true;
		}
	}
theApp.ResetModified (bUndo);
return false;
}

//==========================================================================
// MENU - Add secret exit
//==========================================================================

bool CMine::AddSecretExit () 
{
if (file_type == RDL_FILE) {
    AddExit (TT_SECRET_EXIT);
	 return false;
	}
if (GameInfo ().walls.count >= MAX_WALLS) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg("Maximum number of triggers reached");
	return false;
	}
int last_segment = Current ()->segment;
bool bUndo = theApp.SetModified (true);
theApp.LockUndo ();
if (!AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false;
	}
int new_segment = Current ()->segment;
Current ()->segment = last_segment;
if (AddWall (Current ()->segment, Current ()->side, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
	AddTrigger (GameInfo ().walls.count - 1, TT_SECRET_EXIT);
	SecretCubeNum () = Current ()->segment;
	SetDefaultTexture (426, -1);
	Current ()->segment = new_segment;
	SetDefaultTexture (426, -1);
	theApp.MineView ()->Refresh ();
	theApp.UnlockUndo ();
	return true;
	}
theApp.ResetModified (bUndo);
return false;
}



bool CMine::GetTriggerResources (UINT16& wallnum)
{
wallnum = CurrSide ()->nWall;
if (wallnum < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count >= MAX_WALLS - 1) {
	ErrorMsg("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg("Maximum number of triggers reached");
	return false;
	}
return true;
}

//--------------------------------------------------------------------------
// AddDoorTrigger
//
// Action - checks other cube's side to see if there is a door there,
//          then adds a trigger for that door
//--------------------------------------------------------------------------

bool CMine::AutoAddTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type) 
{
CDSelection *other = Other ();
UINT16 wallnum;
if (!GetTriggerResources (wallnum))
	return false;
// make a new wall and a new trigger
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (AddWall (Current ()->segment, Current ()->side, (UINT8) wall_type, (UINT8) wall_flags, KEY_NONE, -1, -1) &&
	 AddTrigger (GameInfo ().walls.count - 1, trigger_type)) {
	INT16 trignum = GameInfo ().triggers.count - 1;
	// set link to trigger target
	Triggers () [trignum].num_links = 1;
	Triggers () [trignum].seg [0] = other->segment;
	Triggers () [trignum].side [0] = other->side;
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	return true;
	}
theApp.ResetModified (bUndo);
return false;
}



bool CMine::AddDoorTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type) 
{
CDSelection *other = Other ();
CDSegment *other_seg = OtherSeg ();
UINT16 wallnum = other_seg->sides [other->side].nWall;
if (wallnum >= GameInfo ().walls.count) {
	ErrorMsg("Other cube's side is not on a wall.\n\n"
				"Hint: Select a wall using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
// automatically change the trigger type to open if not a door
if (Walls (wallnum)->type != WALL_DOOR)
	trigger_type = TT_OPEN_WALL;
return AutoAddTrigger (wall_type, wall_flags, trigger_type);
}

//==========================================================================
// MENU - Add Door Trigger
//==========================================================================

bool CMine::AddOpenDoorTrigger() 
{
return AddDoorTrigger (WALL_OPEN,0,TT_OPEN_DOOR);
}

//==========================================================================
// MENU - Add Robot Maker Trigger
//==========================================================================

bool CMine::AddRobotMakerTrigger () 
{
CDSegment *other_seg = OtherSeg ();
if (other_seg->special != SEGMENT_IS_ROBOTMAKER) {
	ErrorMsg("There is no robot maker cube selected.\n\n"
				"Hint: Select a robot maker cube using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_MATCEN);
}

//==========================================================================
// MENU -
//==========================================================================

bool CMine::AddShieldTrigger() 
{
if (file_type != RDL_FILE) {
	ErrorMsg ("Descent 2 does not support shield damage Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_SHIELD_DAMAGE);
}

//==========================================================================
// MENU - Add Energy Drain Trigger
//==========================================================================
bool CMine::AddEnergyTrigger() 
{
if (file_type != RDL_FILE) {
	ErrorMsg ("Descent 2 does not support energy drain Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_ENERGY_DRAIN);
}

//==========================================================================
// MENU - Unlock Trigger
//==========================================================================

bool CMine::AddUnlockTrigger() 
{
if (file_type == RDL_FILE) {
   ErrorMsg("Control Panels are not supported in Descent 1.");
   return false;
	}
return AddDoorTrigger (WALL_OVERLAY, (1<<WALL_WALL_SWITCH), TT_UNLOCK_DOOR);
}
