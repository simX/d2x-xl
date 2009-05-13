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

//------------------------------------------------------------------------
// SortObjects ()
//------------------------------------------------------------------------

static INT16 sortObjType [MAX_OBJECT_TYPES] = {7, 8, 5, 4, 0, 2, 9, 3, 10, 6, 11, 12, 13, 14, 1, 16, 15, 17, 18, 19, 20};


int CMine::QCmpObjects (CDObject *pi, CDObject *pm)
{
	INT16 ti = sortObjType [pi->type];
	INT16 tm = sortObjType [pm->type];
if (ti < tm)
	return -1;
if (ti > tm)
	return 1;
return (pi->id < pm->id) ? -1 : (pi->id > pm->id) ? 1 : 0;
}


void CMine::RenumberObjTriggers (short i, short objnum)
{
i = *ObjTriggerRoot (i);
while (i >= 0) {
	ObjTriggerList (i)->objnum = objnum;
	i = ObjTriggerList (i)->next;
	}
}


void CMine::QSortObjects (INT16 left, INT16 right)
{
	CDObject	median = *Objects ((left + right) / 2);
	INT16	l = left, r = right;

do {
	while (QCmpObjects (Objects (l), &median) < 0)
		l++;
	while (QCmpObjects (Objects (r), &median) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			CDObject o = *Objects (l);
			*Objects (l) = *Objects (r);
			*Objects (r) = o;
			if (Current ()->object == l)
				Current ()->object = r;
			else if (Current ()->object == r)
				Current ()->object = l;
			if (level_version > 12) {
				short h = *ObjTriggerRoot (l);
				*ObjTriggerRoot (l) = *ObjTriggerRoot (r);
				*ObjTriggerRoot (r) = h;
				}
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortObjects (l, right);
if (left < r)
	QSortObjects (left, r);
}


void CMine::SortObjects ()
{
	int	i;

if (m_bSortObjects && ((i = GameInfo ().objects.count) > 1)) {
	QSortObjects (0, i - 1);
	for (int j = 0; j < i; j++)
		RenumberObjTriggers (j, j);
	}
}

//------------------------------------------------------------------------
// make_object()
//
// Action - Defines a standard object (currently assumed to be a player)
//------------------------------------------------------------------------

void CMine::MakeObject (CDObject *obj, INT8 type, INT16 segnum) 
{
  vms_vector location;

	theApp.SetModified (TRUE);
	theApp.LockUndo ();
  CalcSegCenter (location,segnum);
  memset(obj,0,sizeof(CDObject));
  obj->signature = 0;
  obj->type = type;
  if (type==OBJ_WEAPON) {
	obj->id = WEAPON_MINE;
  } else {
	obj->id = 0;
  }
  obj->control_type = CT_NONE; /* player 0 only */
  obj->movement_type = MT_PHYSICS;
  obj->render_type   = RT_POLYOBJ;
  obj->flags         = 0;
  obj->segnum        = Current ()->segment;
  obj->pos.x         = location.x;
  obj->pos.y         = location.y;
  obj->pos.z         = location.z;
  obj->orient.rvec.x = F1_0;
  obj->orient.rvec.y = 0;
  obj->orient.rvec.z = 0;
  obj->orient.uvec.x = 0;
  obj->orient.uvec.y = F1_0;
  obj->orient.uvec.z = 0;
  obj->orient.fvec.x = 0;
  obj->orient.fvec.y = 0;
  obj->orient.fvec.z = F1_0;
  obj->size          = PLAYER_SIZE;
  obj->shields       = DEFAULT_SHIELD;
  obj->rtype.pobj_info.model_num = PLAYER_CLIP_NUMBER;
  obj->rtype.pobj_info.tmap_override = -1;
  obj->contains_type = 0;
  obj->contains_id = 0;
  obj->contains_count = 0;
	theApp.UnlockUndo ();
  return;
}

//------------------------------------------------------------------------
// set_object_data()
//
// Action - Sets control type, movement type, render type
// 	    size, and shields (also model_num & texture if robot)
//------------------------------------------------------------------------

void CMine::SetObjectData (INT8 type) 
{
  CDObject *obj;
  int  id;

theApp.SetModified (TRUE);
theApp.LockUndo ();
obj = Objects () + Current ()->object;
id = obj->id;
memset(&obj->mtype, 0, sizeof (obj->mtype));
memset(&obj->ctype, 0, sizeof (obj->ctype));
memset(&obj->rtype, 0, sizeof (obj->rtype));
switch (type) {
	case OBJ_ROBOT    : // an evil enemy
	  obj->control_type  = CT_AI;
	  obj->movement_type = MT_PHYSICS;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = robot_size[id];
	  obj->shields       = robot_shield[id];
	  obj->rtype.pobj_info.model_num = robot_clip[id];
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to none
	  obj->ctype.ai_info.behavior = AIB_NORMAL;
	  break;

	case OBJ_HOSTAGE  : // a hostage you need to rescue
	  obj->control_type = CT_POWERUP;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_HOSTAGE;
	  obj->rtype.vclip_info.vclip_num = HOSTAGE_CLIP_NUMBER;
	  obj->size          = PLAYER_SIZE;
	  obj->shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_PLAYER   : // the player on the console
	  if (obj->id == 0) {
		obj->control_type = CT_NONE; /* player 0 only */
	  } else {
		obj->control_type = CT_SLEW; /* all other players */
	  }
	  obj->movement_type = MT_PHYSICS;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = PLAYER_SIZE;
	  obj->shields       = DEFAULT_SHIELD;
	  obj->rtype.pobj_info.model_num = PLAYER_CLIP_NUMBER;
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to normal
	  break;

	case OBJ_WEAPON   : // a poly-type weapon
	  obj->control_type  = CT_WEAPON;
	  obj->movement_type = MT_PHYSICS;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = WEAPON_SIZE;
	  obj->shields       = WEAPON_SHIELD;
	  obj->rtype.pobj_info.model_num = MINE_CLIP_NUMBER;
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to normal
	  obj->mtype.phys_info.mass      = 65536L;
	  obj->mtype.phys_info.drag      = 2162;
	  obj->mtype.phys_info.rotvel.x  = 0;
	  obj->mtype.phys_info.rotvel.y  = 46482L;  // don't know exactly what to put here
	  obj->mtype.phys_info.rotvel.z  = 0;
	  obj->mtype.phys_info.flags     = 260;
	  obj->ctype.laser_info.parent_type      = 5;
	  obj->ctype.laser_info.parent_num       = 146; // don't know exactly what to put here
	  obj->ctype.laser_info.parent_signature = 146; // don't know exactly what to put here
	  break;

	case OBJ_POWERUP  : // a powerup you can pick up
	  obj->control_type  = CT_POWERUP;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_POWERUP;
	  obj->rtype.vclip_info.vclip_num = powerup_clip[id];
	  obj->size          = powerup_size[id];
	  obj->shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_CNTRLCEN : // the reactor
	  obj->control_type = CT_CNTRLCEN;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = REACTOR_SIZE;
	  obj->shields       = REACTOR_SHIELD;
	  if (file_type == RDL_FILE)
			obj->rtype.pobj_info.model_num = REACTOR_CLIP_NUMBER;
	  else {
		INT32 model;
		switch(id) {
		  case 1:  model = 95;  break;
		  case 2:  model = 97;  break;
		  case 3:  model = 99;  break;
		  case 4:  model = 101; break;
		  case 5:  model = 103; break;
		  case 6:  model = 105; break;
		  default: model = 97;  break; // level 1's reactor
		}
		obj->rtype.pobj_info.model_num = model;
	  }
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to none
	  break;

	case OBJ_COOP     : // a cooperative player object
	  obj->control_type = CT_NONE;
	  obj->movement_type = MT_PHYSICS;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = PLAYER_SIZE;
	  obj->shields       = DEFAULT_SHIELD;
	  obj->rtype.pobj_info.model_num = COOP_CLIP_NUMBER;
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to none
	  break;

	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_MONSTERBALL:
	  obj->control_type  = CT_AI;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_POLYOBJ;
	  obj->size          = robot_size[0];
	  obj->shields       = DEFAULT_SHIELD;
	  obj->rtype.pobj_info.model_num = robot_clip [0];
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to none
	  obj->ctype.ai_info.behavior = AIB_STILL;
	  break;

	case OBJ_EXPLOSION:
	  obj->control_type  = CT_POWERUP;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_POWERUP;
	  obj->size          = robot_size[0];
	  obj->shields       = DEFAULT_SHIELD;
	  obj->rtype.vclip_info.vclip_num = VCLIP_BIG_EXPLOSION;
	  obj->rtype.pobj_info.tmap_override = -1; // set texture to none
	  obj->ctype.ai_info.behavior = AIB_STILL;
	  break;

	case OBJ_EFFECT:
	  obj->control_type  = CT_NONE;
	  obj->movement_type = MT_NONE;
	  obj->render_type   = RT_NONE;
	  obj->size          = f1_0;
	  obj->shields       = DEFAULT_SHIELD;

  }
	theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
// copy_object()
//
// Action - Copies the current object to a new object
//          If object is a player or coop, it chooses the next available id
//
// Parameters - INT8 new_type = new type of object
//
// Returns - TRUE upon success
//
//------------------------------------------------------------------------

bool CMine::CopyObject (UINT8 new_type, INT16 segnum) 
{
	INT16 objnum,id;
	INT16 ids [MAX_PLAYERS_D2X + MAX_COOP_PLAYERS] = {0,0,0,0,0,0,0,0,0,0,0};
	CDObject *obj,*current_obj;
	UINT8 type;
	INT16 i,count;

if (GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg ("The maximum number of objects has already been reached.");
	return false;
	}
// If OBJ_NONE used, then make a copy of the current object type
// otherwise use the type passed as the parameter "new_type"
//--------------------------------------------------------------

type = (new_type == OBJ_NONE) ? CurrObj ()->type : new_type;

// Make sure it is ok to add another object of this type
// Set the id if it's a player or coop
//------------------------------------------------------
if (type == OBJ_PLAYER || type == OBJ_COOP) {
	obj = Objects ();
	for (objnum = GameInfo ().objects.count; objnum; objnum--, obj++)
		if (obj->type == type) {
			id = obj->id;
			if (id >= 0 && id < (MAX_PLAYERS + MAX_COOP_PLAYERS))
				ids[id]++;
			}
	if (type == OBJ_PLAYER) {
		for (id = 0; (id <= MAX_PLAYERS) && ids[id]; id++)
				;// loop until 1st id with 0
		if (id > MAX_PLAYERS) {
				char szMsg [80];

			sprintf (szMsg, "There are already %d players in the mine", MAX_PLAYERS);
			ErrorMsg(szMsg);
			return FALSE;
			}
		}
	else {
		for (id = MAX_PLAYERS; (id < MAX_PLAYERS + MAX_COOP_PLAYERS) && ids[id]; id++)
			;// loop until 1st id with 0
		if (id > MAX_PLAYERS + MAX_COOP_PLAYERS) {
				char szMsg [80];

			sprintf (szMsg, "There are already %d cooperative players in the mine", MAX_COOP_PLAYERS);
			ErrorMsg(szMsg);
			return FALSE;
			}
		}
	}

// Now we can add the object
// Make a new object
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (GameInfo ().objects.count == 0) {
	MakeObject(Objects (), OBJ_PLAYER, (segnum < 0) ? Current ()->segment : segnum);
	GameInfo ().objects.count = 1;
	objnum = 0;
	}
else {
	// Make a copy of the current object
	objnum = GameInfo ().objects.count++;
	obj = Objects (objnum);
	current_obj = CurrObj ();
	memcpy (obj, current_obj, sizeof (CDObject));
	}
obj->flags = 0;                                      // new: 1/27/97
obj->segnum = Current ()->segment;
// set object position in the center of the cube for now
CalcSegCenter (obj->pos,Current ()->segment);
obj->last_pos.x = obj->pos.x;
obj->last_pos.y = obj->pos.y;
obj->last_pos.z = obj->pos.z;
Current ()->object = objnum;
// bump position over if this is not the first object in the cube
count = 0;
for (i = 0; i < GameInfo ().objects.count - 1; i++)
	if (Objects (i)->segnum == Current ()->segment)
		count++;
obj->pos.y += count*2*F1_0;
obj->last_pos.y += count*2*F1_0;
// set the id if new object is a player or a coop
if (type == OBJ_PLAYER || type == OBJ_COOP)
	obj->id = (INT8) id;
// set object data if new object being added
if (new_type != OBJ_NONE) {
	obj->type = new_type;
	SetObjectData(obj->type);
	}
// set the contents to zero
obj->contains_type = 0;
obj->contains_id = 0;
obj->contains_count = 0;
SortObjects ();
theApp.MineView ()->Refresh (false);
theApp.ToolView ()->ObjectTool ()->Refresh ();
theApp.UnlockUndo ();
return TRUE;
}

//------------------------------------------------------------------------
// DeleteObject()
//------------------------------------------------------------------------

void CMine::DeleteObject (INT16 objectNumber)
{
if (GameInfo ().objects.count == 0) {
	if (!bExpertMode)
		ErrorMsg ("There are no Objects () in the mine.");
	return;
	}
if (GameInfo ().objects.count == 1) {
	if (!bExpertMode)
		ErrorMsg ("Cannot delete the last object.");
	return;
	}
if (objectNumber < 0)
	objectNumber = Current ()->object;
if (objectNumber == GameInfo ().objects.count) {
	if (!bExpertMode)
		ErrorMsg ("Cannot delete the secret return.");
	return;
	}
theApp.SetModified (TRUE);
theApp.LockUndo ();
DeleteObjTriggers (objectNumber);
if (objectNumber < --GameInfo ().objects.count) {
	for (int i = objectNumber; i < GameInfo ().objects.count; i++)
		RenumberObjTriggers (i, i - 1);
	memcpy (Objects () + objectNumber, 
			  Objects () + objectNumber + 1, 
		     (GameInfo ().objects.count - objectNumber) * sizeof (CDObject));
	memcpy (ObjTriggerRoot () + objectNumber,
  			  ObjTriggerRoot () + objectNumber + 1,
		     (GameInfo ().objects.count - objectNumber) * sizeof (short));
	}
if (Current1 ().object >= GameInfo ().objects.count)
	Current1 ().object = GameInfo ().objects.count-1;
if (Current2 ().object >= GameInfo ().objects.count)
	Current2 ().object = GameInfo ().objects.count-1;
theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
/// CObjectTool - DrawObject
//------------------------------------------------------------------------

void CMine::DrawObject (CWnd *pWnd, int type, int id)
{
	int powerup_lookup[48] = {
		 0, 1, 2, 3, 4, 5, 6,-1,-1,-1,
		 7, 8, 9,10,11,12,13,14,15,16,
		17,18,19,20,-1,21,-1,-1,22,23,
		24,25,26,27,28,29,30,31,32,33,
		34,35,36,37,38,39,40,41
		};
	int object_number;

// figure out object number based on object type and id
object_number = -1; // assume that we can't find the object
switch (type) {
	case OBJ_PLAYER:
		object_number = 0;
		break;
	case OBJ_COOP:
		object_number = 0;
		break;
	case OBJ_ROBOT:
		object_number = (id < 66) ? 1 + id : 118 + (id - 66);
		break;
	case OBJ_CNTRLCEN:
	if (file_type == RDL_FILE)
		object_number = 67;
	else
		switch(id) {
			case 1: object_number = 68; break;
			case 2: object_number = 69; break;
			case 3: object_number = 70; break;
			case 4: object_number = 71; break;
			case 5: object_number = 72; break;
			case 6: object_number = 73; break;
			default: object_number = 69; break; // level 1's reactor
			}
		break;
	case OBJ_WEAPON:
		object_number = 74;
		break;
	case OBJ_HOSTAGE:
		object_number = 75;
		break;
	case OBJ_POWERUP:
		if ((id >= 0) && (id < MAX_POWERUP_IDS) && (powerup_lookup[id] >= 0))
			object_number = 76 + powerup_lookup[id];
		break;
	default:
		object_number = -1; // undefined
}

CDC *pDC = pWnd->GetDC ();
CRect rc;
pWnd->GetClientRect (rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
if ((object_number >= 0) && (object_number <= 129)) {
	sprintf (message,"OBJ_%03d_BMP", object_number);
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	HRSRC hFind = FindResource (hInst,message, RT_BITMAP);
	HGLOBAL hGlobal = LoadResource (hInst, hFind);
	char *pRes = (char *)LockResource(hGlobal);
	BITMAPINFO *bmi = (BITMAPINFO *)pRes;
	if (bmi) {	//if not, there is a problem in the resource file
		int ncolors = (int)bmi->bmiHeader.biClrUsed;
		if (ncolors == 0)
			ncolors = 1 << (bmi->bmiHeader.biBitCount); // 256 colors for 8-bit data
		char *pImage = pRes + sizeof (BITMAPINFOHEADER) + ncolors * 4;
		int width = (int)bmi->bmiHeader.biWidth;
		int height = (int)bmi->bmiHeader.biHeight;
		int xoffset = (64 - width) / 2;
		int yoffset = (64 - height) / 2;
		SetDIBitsToDevice(pDC->m_hDC, xoffset,yoffset,width,height,0,0,
								0, height,pImage, bmi, DIB_RGB_COLORS);
		}
	FreeResource (hGlobal);
	}
pWnd->ReleaseDC (pDC);
pWnd->InvalidateRect (NULL, TRUE);
pWnd->UpdateWindow ();
}

// eof object.cpp