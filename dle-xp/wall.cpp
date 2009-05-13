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
#include "mine.h"
#include "dlc.h"
#include "global.h"


//--------------------------------------------------------------------------
// Mine - add wall
//
// Returns - TRUE on success
//
// Note: clipnum & tmapnum are used for call to DefineWall only.
//--------------------------------------------------------------------------

CDWall *CMine::AddWall (INT16 segnum,INT16 sidenum,
								INT16 type, UINT8 flags, UINT8 keys,
								INT8 clipnum, INT16 tmapnum) 
{
GetCurrent (segnum, sidenum);

UINT16 wallnum;
CDSegment *seg = Segments (segnum);

// if wall is an overlay, make sure there is no child
if (type < 0)
	type = (seg->children [sidenum] == -1) ? WALL_OVERLAY : WALL_OPEN;
if (type == WALL_OVERLAY) {
	if (seg->children [sidenum] != -1) {
		ErrorMsg ("Switches can only be put on solid sides.");
		return NULL;
		}
	}
else {
	// otherwise make sure there is a child
	if (seg->children [sidenum] < 0) {
		ErrorMsg ("This side must be attached to an other cube before a wall can be added.");
		return NULL;
		}
	}

if (seg->sides [sidenum].nWall < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side.");
	return NULL;
	}

if ((wallnum = GameInfo ().walls.count) >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls () reached");
	return NULL;
	}

// link wall to segment/side
theApp.SetModified (TRUE);
theApp.LockUndo ();
seg->sides [sidenum].nWall = wallnum;
DefineWall (segnum, sidenum, wallnum, (UINT8) type, clipnum, tmapnum, false);
Walls (wallnum)->flags = flags;
Walls (wallnum)->keys = keys;
// update number of Walls () in mine
GameInfo ().walls.count++;
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return Walls (wallnum);
}

//--------------------------------------------------------------------------

bool CMine::GetOppositeWall (INT16& opp_wallnum, INT16 segnum, INT16 sidenum)
{
	INT16 opp_segnum, opp_sidenum;

if (!GetOppositeSide (opp_segnum, opp_sidenum, segnum, sidenum))
	return false;
opp_wallnum = Segments (opp_segnum)->sides [opp_sidenum].nWall;
return true;
}

//--------------------------------------------------------------------------
// DefineWall()
//
// Note: if clipnum == -1, then it is overriden for blastable and auto door
//       if tmapnum == -1, then it is overriden for illusion Walls ()
//       if clipnum == -2, then texture is applied to nOvlTex instead
//--------------------------------------------------------------------------

void CMine::DefineWall (INT16 segnum, INT16 sidenum, UINT16 wallnum,
								UINT8 type, INT8 clipnum, INT16 tmapnum,
								bool bRedefine) 
{
GetCurrent (segnum, sidenum);

	int i;
	CDSegment *seg = Segments (segnum);
	CDSide *side = seg->sides + sidenum;
	CDWall *wall = Walls (wallnum);

theApp.SetModified (TRUE);
theApp.LockUndo ();
// define new wall
wall->segnum = segnum;
wall->sidenum = sidenum;
wall->type = type;
if (!bRedefine) {
	wall->trigger = NO_TRIGGER;
	wall->linked_wall = -1; //GetOppositeWall (opp_wallnum, segnum, sidenum) ? opp_wallnum : -1;
	}
switch (type) {
	case WALL_BLASTABLE:
		wall->clip_num = (clipnum == -1) ?  6 : clipnum;
		wall->hps = WALL_HPS;
		// define door textures based on clip number
		SetWallTextures (wallnum, tmapnum);
		break;

	case WALL_DOOR:
		wall->clip_num = (clipnum == -1) ? 1 : clipnum;
		wall->hps = 0;
		// define door textures based on clip number
		SetWallTextures (wallnum, tmapnum);
		break;

	case WALL_CLOSED:
	case WALL_ILLUSION:
		wall->clip_num = -1;
		wall->hps = 0;
		// define texture to be energy
		if (tmapnum == -1)
			SetTexture (segnum, sidenum, (file_type == RDL_FILE) ? 328 : 353, 0); // energy
		else if (clipnum == -2)
			SetTexture (segnum, sidenum, 0, tmapnum);
		else
			SetTexture (segnum, sidenum, tmapnum, 0);
		break;

	case WALL_OVERLAY: // d2 only
		wall->clip_num = -1;
		wall->hps = 0;
		// define box01a
		SetTexture (segnum, sidenum, -1, 414);
		break;

	case WALL_CLOAKED:
		wall->cloak_value = 17;
		break;

	case WALL_TRANSPARENT:
		wall->cloak_value = 0;
		break;

	default:
		wall->clip_num = -1;
		wall->hps = 0;
		SetTexture (segnum, sidenum, tmapnum, 0);
		break;
	}
wall->flags = 0;
wall->state = 0;
wall->keys = 0;
//  wall->pad = 0;
wall->controlling_trigger = 0;

// set uvls of new texture
UINT32	scale = (UINT32) pTextures [file_type][tmapnum].Scale (tmapnum);
for (i = 0;i<4;i++) {
	side->uvls [i].u = default_uvls [i].u / scale;
	side->uvls [i].v = default_uvls [i].v / scale;
	side->uvls [i].l = default_uvls [i].l;
	}
SetUV (segnum, sidenum, 0, 0, 0);
theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// SetWallTextures()
//
// 1/27/97 - added wall01 and door08
//--------------------------------------------------------------------------

void CMine::SetWallTextures (UINT16 wallnum, INT16 tmapnum) 
{
static INT16 wall_texture [N_WALL_TEXTURES] [2] = {
	{371,0},{0,376},{0,0},  {0,387},{0,399},{413,0},{419,0},{0,424},  {0,0},{436,0},
	{0,444},{0,459},{0,472},{486,0},{492,0},{500,0},{508,0},{515,0},{521,0},{529,0},
	{536,0},{543,0},{0,550},{563,0},{570,0},{577,0}
	};
static INT16 d2_wall_texture [D2_N_WALL_TEXTURES] [2] = {
	{435,0},{0,440},{0,0},{0,451},{0,463},{477,0},{483,0},{0,488},{0,0},  {500,0},
	{0,508},{0,523},{0,536},{550,0},{556,0},{564,0},{572,0},{579,0},{585,0},{593,0},
	{600,0},{608,0},{0,615},{628,0},{635,0},{642,0},{0,649},{664,0},{0,672},{0,687},
	{0,702},{717,0},{725,0},{731,0},{738,0},{745,0},{754,0},{763,0},{772,0},{780,0},
	{0,790},{806,0},{817,0},{827,0},{838,0},{849,0},{858,0},{863,0},{0,871},{0,886},
	{901,0}
	};

CDWall *wall = Walls (wallnum);
CDSide *side = Segments (wall->segnum)->sides + (INT16) wall->sidenum;
INT8 clip_num = wall->clip_num;

theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((wall->type == WALL_DOOR) || (wall->type == WALL_BLASTABLE))
	if (file_type == RDL_FILE) {
		side->nBaseTex = wall_texture [clip_num] [0];
		side->nOvlTex = wall_texture [clip_num] [1];
		} 
	else {
		side->nBaseTex = d2_wall_texture [clip_num] [0];
		side->nOvlTex = d2_wall_texture [clip_num] [1];
		}
else if (tmapnum >= 0) {
	side->nBaseTex = tmapnum;
	side->nOvlTex = 0;
	}
else
	return;
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
}

//--------------------------------------------------------------------------
// Mine - delete wall
//--------------------------------------------------------------------------

void CMine::DeleteWall (UINT16 wallnum) 
{
	INT16 trignum;
	INT16 segnum, sidenum, opp_segnum, opp_sidenum;
	CDSegment *seg;
	CDSide *side;

if (wallnum < 0)
	wallnum = CurrSide ()->nWall;
if (wallnum >= GameInfo ().walls.count)
	return;
// if trigger exists, remove it as well
trignum = Walls (wallnum)->trigger;
theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((trignum > -1) && (trignum < GameInfo ().triggers.count))
	DeleteTrigger (trignum); 
// remove references to the deleted wall
if (GetOppositeSide (opp_segnum, opp_sidenum, Walls (wallnum)->segnum, Walls (wallnum)->sidenum)) {
	INT16 opp_wallnum = Segments (opp_segnum)->sides [opp_sidenum].nWall;
	if ((opp_wallnum >= 0) && (opp_wallnum < GameInfo ().walls.count))
		Walls (opp_wallnum)->linked_wall = -1;
	}
// update all Segments () that point to Walls () higher than deleted one
// and unlink all Segments () that point to deleted wall
for (segnum = 0, seg = Segments (); segnum < SegCount (); segnum++, seg++)
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++)
		if (side->nWall >= GameInfo ().walls.count)
			side->nWall = NO_WALL;
		else if (side->nWall > wallnum)
			side->nWall--;
		else if (side->nWall == wallnum) {
			side->nWall = NO_WALL;
			DeleteTriggerTargets (segnum, sidenum); //delete this wall from all Triggers () that target it
			}
// move remaining Walls () in place of deleted wall
// for (i = wallnum; i < GameInfo ().walls.count - 1; i++)
if (wallnum < --GameInfo ().walls.count)
	memcpy (Walls (wallnum), Walls (wallnum + 1), (GameInfo ().walls.count - wallnum) * sizeof(CDWall));
// update number of Walls () in mine
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
AutoLinkExitToReactor();
}

                        /*--------------------------*/

CDWall *CMine::FindWall (INT16 segnum, INT16 sidenum)
{
GetCurrent (segnum, sidenum);
CDWall *wall;
int nWall;

for (wall = Walls (), nWall = 0; nWall < GameInfo ().walls.count; nWall++, wall++)
	if ((wall->segnum == segnum) && (wall->sidenum == sidenum))
		return wall;
return NULL;
}

                        /*--------------------------*/

int CMine::FindClip (CDWall *wall, INT16 tmapnum)
{
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	char szName [80], *ps;

LoadString (hInst, texture_resource + tmapnum, szName, sizeof (szName));
if (!strcmp (szName, "wall01 - anim"))
	return wall->clip_num = 0;
if (ps = strstr (szName, "door")) {
	int i, nDoor = atol (ps + 4);
	for (i = 1; i < D2_NUM_OF_CLIPS; i++)
		if (nDoor == clip_door_number [i]) {
			wall->clip_num = clip_num [i];
			theApp.SetModified (TRUE);
			theApp.MineView ()->Refresh ();
			return i;
			}
	}
return -1;
}

                        /*--------------------------*/

CDWall *CMine::GetWall (INT16 segnum, INT16 sidenum)
{
GetCurrent (segnum, sidenum);
UINT16 wallnum = Segments (segnum)->sides [sidenum].nWall;
return (wallnum < GameInfo ().walls.count) ? Walls (wallnum) : NULL;
}

                        /*--------------------------*/

bool CMine::WallClipFromTexture (INT16 segnum, INT16 sidenum)
{
CDWall *wall = FindWall (segnum, sidenum);

if (!wall || ((wall->type != WALL_DOOR) && (wall->type != WALL_BLASTABLE)))
	return true;

CDSide *side = Segments () [segnum].sides + sidenum;

if (FindClip (wall, side->nOvlTex) >= 0)
	return true;
if (FindClip (wall, side->nBaseTex) >= 0)
	return true;
return false;
}

//------------------------------------------------------------------------
// CheckForDoor()
//------------------------------------------------------------------------

void CMine::CheckForDoor (INT16 segnum, INT16 sidenum) 
{
GetCurrent (segnum, sidenum);
// put up a warning if changing a door's texture
UINT16 wallnum = Segments (segnum)->sides [sidenum].nWall;

if (!bExpertMode &&
    (wallnum < GameInfo ().walls.count) &&
	 ((Walls (wallnum)->type == WALL_BLASTABLE) || (Walls (wallnum)->type == WALL_DOOR)))
		ErrorMsg("Changing the texture of a door only affects\n"
					"how the door will look before it is opened.\n"
					"You can use this trick to hide a door\n"
					"until it is used for the first time.\n\n"
					"Hint: To change the door animation,\n"
					"select \"Wall edit...\" from the Tools\n"
					"menu and change the clip number.");
}

	                        /*--------------------------*/

//eof wall.cpp