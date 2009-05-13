// Copyright (C) 1997 Bryan Aamot
#include "myowl.h"
#include <alloc.h>
#include <dos.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include <bios.h>>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "proto.h"
#include "global.h"
#include "control.h"

//------------------------------------------------------------------------
// DIALOG - TControlDialog (constructor)
//------------------------------------------------------------------------
TControlDialog::TControlDialog(TWindow * AParent, LPSTR name)
					:TDialog(AParent, name) {
  TriggerCombo         = new TComboBox(this, 101,10);
  TriggerList          = new TListBox (this, 102);
  AddTrigger           = new TButton  (this, 103);
  DelTrigger           = new TButton  (this, 104);
  TriggerEdit          = new TEdit    (this, 105,20);
  ReactorTimeEdit      = new TEdit    (this, 106,20);
  ReactorStrengthEdit  = new TEdit    (this, 107,20);
  SecretCubeEdit       = new TEdit    (this, 108,20);
  SecretSideEdit       = new TEdit    (this, 109,200);
}

//------------------------------------------------------------------------
// TControlDialog - SetupWindow
//------------------------------------------------------------------------
void TControlDialog::SetupWindow() {
  int i;
  TDialog::SetupWindow();
  for (i=0;i<MAX_CONTROL_CENTER_TRIGGERS;i++) {
    TriggerCombo->AddString(itoa(i,message,10));
  }
  TriggerCombo->SetSelIndex(0);
  RefreshData();
}

//------------------------------------------------------------------------
// TControlDialog - RefreshData
//------------------------------------------------------------------------
void TControlDialog::RefreshData() {
  int i,control,num_links;
  TriggerList->ClearList();
  control = TriggerCombo->GetSelIndex();
  num_links =  control_center_triggers[control].num_links;
  for (i=0;i<num_links;i++) {
    sprintf(message,"%d,%d",
	 (int)control_center_triggers[control].seg[i],
	 (int)control_center_triggers[control].side[i]);
    TriggerList->AddString(message);
  }
  if (file_type == RDL_FILE) {
    ReactorTimeEdit->Clear();
    ReactorStrengthEdit->Clear();
    SecretCubeEdit->Clear();
    SecretSideEdit->Clear();
  } else {
    sprintf(message,"%ld",mine->reactor_time);
    ReactorTimeEdit->SetText(message);
    sprintf(message,"%ld",mine->reactor_strength);
    ReactorStrengthEdit->SetText(message);
    sprintf(message,"%ld",mine->secret_cubenum);
    SecretCubeEdit->SetText(message);
    // figure out side number based on matrix
    sprintf(message,"(%08lx,%08lx,%08lx)"
		    "(%08lx,%08lx,%08lx)"
		    "(%08lx,%08lx,%08lx)",
		mine->secret_orient.rvec.x,
		mine->secret_orient.rvec.y,
		mine->secret_orient.rvec.z,
		mine->secret_orient.uvec.x,
		mine->secret_orient.uvec.y,
		mine->secret_orient.uvec.z,
		mine->secret_orient.fvec.x,
		mine->secret_orient.fvec.y,
		mine->secret_orient.fvec.z);
    SecretSideEdit->SetText(message);
  }


  GrayButtons();
}

//------------------------------------------------------------------------
// TCubeDialog - User Message
//------------------------------------------------------------------------
LRESULT TControlDialog::User( WPARAM, LPARAM ) {
  RefreshData();
  return (LRESULT)0;
}

//------------------------------------------------------------------------
// TControlDialog - GrayButtons()
//
// Action - Enables/Disables dialog buttons depending on selections
//
//------------------------------------------------------------------------
void TControlDialog::GrayButtons() {
  int control;

  // enable/disable add trigger button
  control = TriggerCombo->GetSelIndex();
  TriggerEdit->GetText(message,sizeof(message));
  if (message[0] != NULL) {
    if (control_center_triggers[control].num_links < MAX_TRIGGER_TARGETS) {
      AddTrigger->EnableWindow(TRUE);
    } else {
      AddTrigger->EnableWindow(FALSE);
    }
  } else {
    AddTrigger->EnableWindow(FALSE);
  }

  // enable/disable delete trigger button
  if (TriggerList->GetSelIndex() >=0) {
    DelTrigger->EnableWindow(TRUE);
  } else {
    DelTrigger->EnableWindow(FALSE);
  }

  if (file_type == RDL_FILE) {
    ReactorTimeEdit->EnableWindow(FALSE);
    ReactorStrengthEdit->EnableWindow(FALSE);
    SecretCubeEdit->EnableWindow(FALSE);
    SecretSideEdit->EnableWindow(FALSE);
  } else {
    ReactorTimeEdit->EnableWindow(TRUE);
    ReactorStrengthEdit->EnableWindow(TRUE);
    SecretCubeEdit->EnableWindow(TRUE);
    SecretSideEdit->EnableWindow(TRUE);
  }

}

//------------------------------------------------------------------------
// TControlDialog - Number Message
//------------------------------------------------------------------------
void TControlDialog::NumberMsg() {
  RefreshData();
}

//------------------------------------------------------------------------
// TControlDialog - Trigger List Message
//------------------------------------------------------------------------
void TControlDialog::TriggerListMsg() {
  GrayButtons();
}

//------------------------------------------------------------------------
// TControlDialog - AddTriggerMsg
//------------------------------------------------------------------------
void TControlDialog::AddTriggerMsg() {
  int cube,side,num_links,control;
  TriggerEdit->GetText(message,sizeof(message));
  cube = side = -1;
  sscanf(message,"%d,%d",&cube,&side);
  if (cube >= 0 && cube <= num_segments && side >=0 && side <=5) {
    control = TriggerCombo->GetSelIndex();
    num_links =  control_center_triggers[control].num_links;
    if (num_links < MAX_TRIGGER_TARGETS - 1) {
      sprintf(message,"%d,%d",cube,side);
      TriggerList->AddString(message);
      control_center_triggers[control].seg[num_links] = cube;
      control_center_triggers[control].side[num_links] = side;
      control_center_triggers[control].num_links++;
      ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
    }
  }
}

//------------------------------------------------------------------------
// TControlDialog - DelTriggerMsg
//------------------------------------------------------------------------
void TControlDialog::DelTriggerMsg() {
  int num_links,control,i;
  i = TriggerList->GetSelIndex();
  control = TriggerCombo->GetSelIndex();
  num_links = control_center_triggers[control].num_links;
  if (i >= 0 && num_links > 0) {
    // remove it from the list
    TriggerList->DeleteString(i);
    // move last link into this deleted link's spot
    control_center_triggers[control].seg [i] =
      control_center_triggers[control].seg[num_links-1];
    control_center_triggers[control].side[i] =
      control_center_triggers[control].side[num_links-1];
    control_center_triggers[control].seg [num_links-1] = 0;
    control_center_triggers[control].side[num_links-1] = 0;
    control_center_triggers[control].num_links--;
    ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
  }
}

//------------------------------------------------------------------------
// TControlDialog - TriggerEditMsg
//------------------------------------------------------------------------
void TControlDialog::TriggerEditMsg() {
  GrayButtons();
}



//------------------------------------------------------------------------
// TControlDialog - ReactorTimeEditMsg
//------------------------------------------------------------------------
void TControlDialog::ReactorTimeEditMsg() {
  ReactorTimeEdit->GetText(message,sizeof(message));
  mine->reactor_time = min((UINT32)atol(message),1000);
}

//------------------------------------------------------------------------
// TControlDialog - ReactorStrengthEditMsg
//------------------------------------------------------------------------
void TControlDialog::ReactorStrengthEditMsg() {
  ReactorStrengthEdit->GetText(message,sizeof(message));
  mine->reactor_strength = atol(message);
}

//------------------------------------------------------------------------
// TControlDialog - SecretCubeEditMsg
//------------------------------------------------------------------------
void TControlDialog::SecretCubeEditMsg() {
  SecretCubeEdit->GetText(message,sizeof(message));
  mine->secret_cubenum   = atoi(message);
}

//------------------------------------------------------------------------
// TControlDialog - SecretSideEditMsg
//------------------------------------------------------------------------
void TControlDialog::SecretSideEditMsg() {
}

//------------------------------------------------------------------------
AutoLinkExitToReactor()
//
// Action - Updates control center Triggers () so that exit door opens
//          when the reactor blows up.  Removes any invalid cube/sides
//          from control_center_triggers if they exist.
//------------------------------------------------------------------------
void auto_link_exit_to_reactor() 
{
  INT16 	linknum,control,num_links,segnum,sidenum;
  UINT16	wallnum;
  UINT8 	trignum;
  bool 	found;

  control = 0; // only 0 used by the game Descent

  // remove items from list that do not point to a wall
  for (linknum=0;linknum<control_center_triggers[control].num_links;linknum++) {
    num_links =  control_center_triggers[control].num_links;
    segnum = control_center_triggers[control].seg[linknum];
    sidenum = control_center_triggers[control].side[linknum];
    // search for Walls () that have a exit of type trigger
    found = FALSE;
    for (wallnum=0;wallnum<GameInfo ().walls_howmany;wallnum++) {
      if (Walls (wallnum)->segnum == segnum && Walls ()[wallnum].sidenum == sidenum) {
        found = TRUE;
        break;
      }
    }
//        trignum = Walls (wallnum)->trigger;
//      if (trignum >= 0 && trignum <GameInfo ().triggers_howmany) {
//        if (Triggers ()[trignum].flags & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT)) {
//	    found = TRUE;
//	    break;
//        }
//      }
//    }
//  }
    if (!found) {
      if (num_links > 0) { // just in case
        // move last link into this deleted link's spot
        control_center_triggers[control].seg [linknum] =
          control_center_triggers[control].seg[num_links-1];
        control_center_triggers[control].side[linknum] =
          control_center_triggers[control].side[num_links-1];
        control_center_triggers[control].seg [num_links-1] = 0;
        control_center_triggers[control].side[num_links-1] = 0;
        control_center_triggers[control].num_links--;
      }
    }
  }

  // add exit to list if not already in list
  // search for walls that have a exit of type trigger
  num_links =  control_center_triggers[control].num_links;
  for (wallnum=0;wallnum<GameInfo ().walls_howmany;wallnum++) {
    trignum = Walls (wallnum)->trigger;
    if (trignum >= 0 && trignum <GameInfo ().triggers_howmany) {
      if (   file_type == RDL_FILE &&
	     Triggers ()[trignum].flags & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT)
	  ||
	     file_type != RDL_FILE &&
	     (Triggers ()[trignum].type == TT_EXIT || Triggers ()[trignum].type == TT_SECRET_EXIT)
	 ) {
	// see if cube,side is already on the list
	segnum = (INT16)Walls (wallnum)->segnum;
	sidenum =(INT16)Walls (wallnum)->sidenum;
	found = FALSE;
        for (linknum=0;linknum<num_links;linknum++) {
          if (segnum == control_center_triggers[control].seg[linknum] && 
	      sidenum == control_center_triggers[control].side[linknum]) {
	    found = TRUE;
            break;
	  }
	}
        // if not already on the list, add it
	if (!found) {
	  linknum = control_center_triggers[control].num_links;
	  control_center_triggers[control].seg[linknum] = segnum;
          control_center_triggers[control].side[linknum] = sidenum;
          control_center_triggers[control].num_links++;
	}
      }
    }
  }
}

DEFINE_RESPONSE_TABLE1( TControlDialog, TDialog )
    EV_LBN_SELCHANGE(101,  NumberMsg ),
    EV_LBN_SELCHANGE(102,  TriggerListMsg ),
    EV_COMMAND      (103,  AddTriggerMsg ),
    EV_COMMAND      (104,  DelTriggerMsg ),
    EV_EN_CHANGE    (105,  TriggerEditMsg ),
    EV_EN_CHANGE    (106,  ReactorTimeEditMsg ),
    EV_EN_CHANGE    (107,  ReactorStrengthEditMsg ),
    EV_EN_CHANGE    (108,  SecretCubeEditMsg ),
    EV_EN_CHANGE    (109,  SecretSideEditMsg ),
    EV_MESSAGE      (1280, User),
END_RESPONSE_TABLE;
