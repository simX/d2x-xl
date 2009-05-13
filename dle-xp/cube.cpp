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
#include "proto.h"
#include "global.h"
#include "cube.h"
#include "wall.h"
#include "dlc.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CCubeTool, CToolDlg)
	ON_BN_CLICKED (IDC_CUBE_, On)
	ON_CBN_SELCHANGE (IDC_CUBE_, On)
	ON_EN_KILLFOCUS (IDC_CUBE_, On)
	ON_EN_UPDATE (IDC_CUBE_, On)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CCubeTool (CPropertySheet *pParent = NULL)
	: CToolDlg (IDD_CUBEDATA, pParent)
   {};

                        /*--------------------------*/

virtual BOOL OnInitDialog ()
{
CToolDlg::OnInitDialog ();
m_bInited = TRUE;
return TRUE;
}

                        /*--------------------------*/

virtual void DoDataExchange (CDataExchange *pDX)
{
}

                        /*--------------------------*/

virtual BOOL OnSetActive ()
{
Refresh ();
}

                        /*--------------------------*/

void InitCBWallNo ()
{
}

                        /*--------------------------*/

void Reset ()
{
}

                        /*--------------------------*/

void EnableControls (BOOL bEnable)
{
}

                        /*--------------------------*/

void Refresh ()
{
}


//------------------------------------------------------------------------
// DIALOG - TPointDialog (constructor)
//------------------------------------------------------------------------
TPointDialog::TPointDialog(TWindow * AParent, LPSTR name):TDialog(AParent, name) {
  XEdit            = new TEdit    (this, 101,20);
  YEdit            = new TEdit    (this, 102,20);
  ZEdit            = new TEdit    (this, 103,20);
}

//------------------------------------------------------------------------
// TPointDialog - SetupWindow
//------------------------------------------------------------------------
void TPointDialog::SetupWindow() {
  TDialog::SetupWindow();
  RefreshData();
}

//------------------------------------------------------------------------
// TPointDialog - RefreshData
//------------------------------------------------------------------------
void TPointDialog::RefreshData() {
  INT16 vertix = Segments ()[current->segment]->verts[side_vert[current->side][current->point]];
  sprintf(message,"%1.4f",(double)vertices[vertix].x/0x10000L);
  XEdit->SetText(message);
  sprintf(message,"%1.4f",(double)vertices[vertix].y/0x10000L);
  YEdit->SetText(message);
  sprintf(message,"%1.4f",(double)vertices[vertix].z/0x10000L);
  ZEdit->SetText(message);
}

//------------------------------------------------------------------------
// TPointDialog -
//------------------------------------------------------------------------
void TPointDialog::ApplyMsg() {
  double value;
  INT16 vertix = Segments ()[current->segment]->verts[side_vert[current->side][current->point]];
  XEdit->GetText(message,sizeof(message));
  value = atof(message);
  if (value >= -0x7fff && value < 0x7fff) {
    vertices[vertix].x = (FIX)(value*F1_0);
  }
  YEdit->GetText(message,sizeof(message));
  value = atof(message);
  if (value >= -0x7fff && value < 0x7fff) {
    vertices[vertix].y = (FIX)(value*F1_0);
  }
  ZEdit->GetText(message,sizeof(message));
  value = atof(message);
  if (value >= -0x7fff && value < 0x7fff) {
    vertices[vertix].z = (FIX)(value*F1_0);
  }
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TPointDialog - User Message
//------------------------------------------------------------------------
LRESULT TPointDialog::User( WPARAM, LPARAM ) {
  RefreshData();
  return (LRESULT)0;
}


//------------------------------------------------------------------------
// DIALOG - TCubeDialog (constructor)
//------------------------------------------------------------------------
TCubeDialog::TCubeDialog(TWindow * AParent, LPSTR name):TDialog(AParent, name) {

  SpecialCombo     = new TComboBox(this, 140,20);
  CubeNumberCombo  = new TComboBox(this, 141,10);
  SideNumberCombo  = new TComboBox(this, 142,10);
  PointNumberCombo = new TComboBox(this, 143,10);
  LightEdit        = new TEdit    (this, 170,20);
  AddCube          = new TButton  (this, 11);
  DelCube          = new TButton  (this, 12);
  UsedRobotsList   = new TListBox (this, 13);
  UnusedRobotsList = new TListBox (this, 17);
  AddRobot         = new TButton  (this, 18);
  DelRobot         = new TButton  (this, 19);
  TriggeredByList  = new TListBox (this, 28);
  OtherCube        = new TButton  (this, 29);
  EndOfExitTunnel  = new TCheckBox(this, 30, NULL);
}

//------------------------------------------------------------------------
// TCubeDialog - SetupWindow
//------------------------------------------------------------------------
void TCubeDialog::SetupWindow() {
  int i;
  TDialog::SetupWindow();
  SpecialCombo->AddString("Normal");
  SpecialCombo->AddString("Fuel Center");
  SpecialCombo->AddString("(not used)");
  SpecialCombo->AddString("Reactor");
  SpecialCombo->AddString("Robot Maker");
  if (file_type != RDL_FILE) {
    SpecialCombo->AddString("Blue Goal");
    SpecialCombo->AddString("Red Goal");
  }
  for (i=0;i<num_segments;i++) {
    CubeNumberCombo->AddString(itoa(i,message,10));
  }
  for (i=0;i<6;i++) {
    SideNumberCombo->AddString(itoa(i,message,10));
  }

  for (i=0;i<4;i++) {
    PointNumberCombo->AddString(itoa(i,message,10));
  }

  RefreshData();
  LastSeg = -1;
  LastSide = -1;
}

//------------------------------------------------------------------------
// TCubeDialog - RefreshData
//------------------------------------------------------------------------
void TCubeDialog::RefreshData() {
  segment far *seg;
  int matcen;
  int i;
  int wallnum;
  int trignum;
  int control,num_links;

  // updater automatic data
  renumber_matcen();

  // update cube number combo box if number of cubes has changed
  if (num_segments != CubeNumberCombo->GetCount()) {
    CubeNumberCombo->ClearList();
    for (i=0;i<num_segments;i++) {
      CubeNumberCombo->AddString(itoa(i,message,10));
    }
  }

  seg = Segments ()[current->segment];


  // set end of exit tunnel check box
  if (seg->children[current->side] == -2) {
    EndOfExitTunnel->Check();
  } else {
    EndOfExitTunnel->Uncheck();
  }

  CubeNumberCombo->SetSelIndex(current->segment);
  SideNumberCombo->SetSelIndex(current->side);
  PointNumberCombo->SetSelIndex(current->point);

  SpecialCombo->SetSelIndex(seg->special);

  // show Triggers () who point at this cube
  TriggeredByList->ClearList();
  for (trignum=0;trignum<GameInfo ().triggers_howmany;trignum++) {
    for (i=0;i<Triggers ()[trignum].num_links;i++) {
      if (Triggers ()[trignum].seg[i] == current->segment
	  && Triggers ()[trignum].side[i] == current->side) {
	// find the wall with this trigger
	for (wallnum=0;wallnum<GameInfo ().walls_howmany;wallnum++) {
	  if (Walls (wallnum)->trigger == trignum) break;
	}
	if (wallnum < GameInfo ().walls_howmany) {
	  sprintf(message,"%d,%d",(int)Walls (wallnum)->segnum,
				  (int)Walls (wallnum)->sidenum);
	  TriggeredByList->AddString(message);
	}
      }
    }
  }
  // show if this is cube/side is triggered by the control_center
  for (control=0;control<MAX_CONTROL_CENTER_TRIGGERS;control++) {
    num_links =  control_center_triggers[control].num_links;
    for (i=0;i<num_links;i++) {
      if (  current->segment == control_center_triggers[control].seg[i]
	 && current->side    == control_center_triggers[control].side[i]) {
	TriggeredByList->AddString("Reactor");
	break;
      }
    }
    if (i<num_links) break; // quit if for loop broke above
  }

  // show "none" if there is no Triggers ()
  if (TriggeredByList->GetCount() == 0) {
    TriggeredByList->AddString("none");
  }

  // fill static light edit
  sprintf(message,"%3.1f%%",((double)seg->static_light)/(6*4*327.68));
  LightEdit->SetText(message);

  matcen = seg->matcen_num;
  if (   seg->special == SEGMENT_IS_ROBOTMAKER
      && matcen >=0
      && matcen < GameInfo ().matcen_howmany) {
    // if # of items in list box totals to less than the number of robots
//    if (UnusedRobotsList->GetCount() + UnusedRobotsList->GetCount() < MAX_ROBOT_IDS) {
    if (LastSeg != current->segment || LastSide != current->side) {
      UsedRobotsList->ClearList();
      UnusedRobotsList->ClearList();
	  for (i=0;i<ROBOT_IDS2;i++) {
	LoadString(GetModule()->GetInstance(),ROBOT_STRING_TABLE+i,string,sizeof(string));
	if (i<32) {
	  if (robot_centers[matcen].objFlags & (1L<<i)) {
	    UsedRobotsList->AddString(string);
	  } else {
	    UnusedRobotsList->AddString(string);
	  }
	} else {
	  if (i<64) {
	    if (robot_centers[matcen].robot_flags2 & (1L<<(i-32))) {
	      UsedRobotsList->AddString(string);
	    } else {
	      UnusedRobotsList->AddString(string);
	    }
	  }
	}
      }
      UnusedRobotsList->SetSelIndex(0);
      UsedRobotsList->SetSelIndex(0);
    }
  } else {
    sprintf(message,"n/a");
    UsedRobotsList->ClearList();
    UsedRobotsList->AddString(message);
    UnusedRobotsList->ClearList();
    UnusedRobotsList->AddString(message);
  }

  GrayButtons();
  LastSeg = current->segment;
  LastSide = current->side;
}


//------------------------------------------------------------------------
// TCubeDialog - EndOfExitTunnelMsg()
//------------------------------------------------------------------------
void TCubeDialog::EndOfExitTunnelMsg()
{

  segment far *seg;
  seg = Segments ()[current->segment];
  switch (EndOfExitTunnel->GetCheck()) {
    case BF_CHECKED:
      seg->children[current->side] = -2;
      seg->child_bitmask |= (1<<current->side);
      break;
    case BF_UNCHECKED:
      seg->children[current->side] = -1;
      seg->child_bitmask &= ~(1<<current->side);
      break;
  }
}

//------------------------------------------------------------------------
// TCubeDialog - GrayButtons()
//
// Action - Enables/Disables dialog buttons depending on selections
//
//------------------------------------------------------------------------
void TCubeDialog::GrayButtons() {
  int matcen;

  // enable/disable "end of exit tunnel" button
  if (Segments ()[current->segment]->children[current->side] >= 0) {
    EndOfExitTunnel->EnableWindow(FALSE);
  } else {
    EndOfExitTunnel->EnableWindow(TRUE);
  }

  // enable/disable add cube button
  if (   (num_segments < MAX_SEGMENTS)
		&& (num_vertices < (MAX_VERTICES-4))
		&& (Segments ()[current->segment]->children[current->side] < 0)
	  ) {
    AddCube->EnableWindow(TRUE);
  } else {
    AddCube->EnableWindow(FALSE);
  }

  // enable/disable add robot button
  matcen = Segments (current->segment)->matcen_num;
  if (   Segments (current->segment)->special == SEGMENT_IS_ROBOTMAKER
		&& matcen >=0
		&& matcen < GameInfo ().matcen_howmany
		&& UnusedRobotsList->GetSelIndex() >= 0) {
    AddRobot->EnableWindow(TRUE);
  } else {
    AddRobot->EnableWindow(FALSE);
  }

  // enable/disable delete robot button
  if (   Segments ()[current->segment]->special == SEGMENT_IS_ROBOTMAKER
		&& matcen >=0
		&& matcen < GameInfo ().matcen_howmany
		&& UsedRobotsList->GetSelIndex() >= 0) {
    DelRobot->EnableWindow(TRUE);
  } else {
    DelRobot->EnableWindow(FALSE);
  }
}

//------------------------------------------------------------------------
// TCubeDialog - User Message
//------------------------------------------------------------------------
LRESULT TCubeDialog::User( WPARAM, LPARAM ) {
  RefreshData();
  return (LRESULT)0;
}

//------------------------------------------------------------------------
// TCubeDialog - Add Cube
//------------------------------------------------------------------------
void TCubeDialog::AddCubeMsg() {
  mine->add_segment();
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - Edit Point
//------------------------------------------------------------------------
void TCubeDialog::EditPointMsg() {
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_USER+0x101,17,0);
}

//------------------------------------------------------------------------
// TCubeDialog - Delete Cube
//------------------------------------------------------------------------
void TCubeDialog::DeleteCubeMsg() {
  mine->delete_segment(current->segment);
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - SpecialMsg
//------------------------------------------------------------------------
void TCubeDialog::SpecialMsg() {
  UINT8 new_special,old_special;
  int n_matcen,del_matcen,n_fuelcen;
  int i;
  bool ok_to_change = TRUE;

  old_special = Segments ()[current->segment]->special;
  new_special = SpecialCombo->GetSelIndex();
  n_matcen = (int) GameInfo ().matcen_howmany;

  if (old_special != new_special) {
    switch(new_special) {
      // check to see if we are adding a robot maker
      case SEGMENT_IS_ROBOTMAKER:
#if 0
       // add matcen
       if (n_matcen < MAX_NUM_MATCENS) {
	 robot_centers[n_matcen].objFlags = 8;
	 robot_centers[n_matcen].robot_flags2 = 0;
	 robot_centers[n_matcen].hit_points = 0;
	 robot_centers[n_matcen].interval = 0;
	 robot_centers[n_matcen].segnum = current->segment;
	 robot_centers[n_matcen].fuelcen_num = n_matcen;
	 Segments ()[current->segment]->value   = n_matcen;
	 GameInfo ().matcen_howmany++;
       } else {
	 ErrorMsg("Maximum number of robot makers reached");
	 ok_to_change = FALSE;
       }
#else
       ErrorMsg("You must use the ""Insert/Robot Maker Cube"" menu to add a robot maker.");
       ok_to_change = FALSE;
#endif
       break;
     // check to see if we are adding a fuel center
     case SEGMENT_IS_FUELCEN:
	// count number of fuel centers
	n_fuelcen = FuelCenterCount ();
	if (n_fuelcen >= MAX_NUM_FUELCENS) {
	  ErrorMsg("Maximum number of fuel centers reached");
	  ok_to_change = FALSE;
	}
      break;
    }

    switch(old_special) {
      // check to see if we are removing a robot maker
      case SEGMENT_IS_ROBOTMAKER:
	// remove matcen
	if (n_matcen > 0) {
	  // fill in deleted matcen
	  del_matcen = Segments ()[current->segment]->value;
	  for (i=del_matcen;i<n_matcen-1;i++) {
	    memcpy((void *)&robot_centers[i],(void *)&robot_centers[i+1],sizeof(matcen_info));
	  }
	  GameInfo ().matcen_howmany--;
	}
      break;
    }

    // update "special"
    if (ok_to_change) {
      Segments ()[current->segment]->special = new_special;
      if (new_special == SEGMENT_IS_NOTHING) {
	Segments ()[current->segment]->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
      } else {
	Segments ()[current->segment]->child_bitmask |=  (1 << MAX_SIDES_PER_SEGMENT);
      }
    }

    LastSeg = -1; // force cube dialog refresh
    ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
  }
  auto_link_exit_to_reactor();
}

//------------------------------------------------------------------------
// renumber_matcen()
//
// ACTION - Numbers segment->matcen and segment->value parameters.
//
//          matcen = 0..M where M is the number of robot maker Segments ()
//          value = 0..N where N is the number of special Segments ()
//------------------------------------------------------------------------
void renumber_matcen() {
  int i,matcen_num,value,segnum;

  // number "matcen"
  matcen_num = 0;
  for (i=0;i<GameInfo ().matcen_howmany;i++) {
    segnum = robot_centers[i].segnum;
    if (segnum >=0) {
      Segments ()[segnum]->value = i;
      if (Segments ()[segnum]->special == SEGMENT_IS_ROBOTMAKER) {
	Segments ()[segnum]->matcen_num = matcen_num++;
      }
    }
  }

  // number "value"
  value = 0;
  for (i=0;i<num_segments;i++) {
    if (Segments ()[i]->special == SEGMENT_IS_NOTHING) {
      Segments ()[i]->value = 0;
    } else {
      Segments ()[i]->value = value++;
    }
  }
}
//------------------------------------------------------------------------
// TCubeDialog - Cube Number Message
//------------------------------------------------------------------------
void TCubeDialog::CubeNumberMsg() {
  current->segment = CubeNumberCombo->GetSelIndex();
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - Side Number Message
//------------------------------------------------------------------------
void TCubeDialog::SideNumberMsg() {
  current->side = SideNumberCombo->GetSelIndex();
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - Point Number Message
//------------------------------------------------------------------------
void TCubeDialog::PointNumberMsg() {
  current->point = PointNumberCombo->GetSelIndex();
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - LightMsg
//------------------------------------------------------------------------
void TCubeDialog::LightMsg() {
  double percentage;
  LightEdit->GetText(message,sizeof(message));
  percentage = atof(message);
  if (percentage <= 100.0 && percentage >= 0) {
    Segments ()[current->segment]->static_light = percentage*6*4*327.68;
  }
}

//------------------------------------------------------------------------
// TCubeDialog - UsedRobotMsg
//------------------------------------------------------------------------
void TCubeDialog::UsedRobotMsg() {
  DelMsg();
  GrayButtons();
}

//------------------------------------------------------------------------
// TCubeDialog - UnusedRobotMsg
//------------------------------------------------------------------------
void TCubeDialog::UnusedRobotMsg() {
  AddMsg();
  GrayButtons();
}

//------------------------------------------------------------------------
// TCubeDialog - AddMsg
//------------------------------------------------------------------------
void TCubeDialog::AddMsg() {
	int i;
	int matcen;

	matcen = Segments ()[current->segment]->matcen_num;
	if (   Segments ()[current->segment]->special == SEGMENT_IS_ROBOTMAKER
		&& matcen >=0
		&& matcen < GameInfo ().matcen_howmany) {

		UnusedRobotsList->GetSelString(message,sizeof(message));
		for (i=0;i<MAX_ROBOT_IDS_TOTAL;i++) {
			LoadString(GetModule()->GetInstance(),ROBOT_STRING_TABLE+i,string,sizeof(string));
			if (strcmp(message,string)==0) break;
		}
		if (i<MAX_ROBOT_IDS_TOTAL) {
			if (i<32) {
				robot_centers[matcen].objFlags |= (1L<<i);
			} else {
				if (i<64) {
					robot_centers[matcen].robot_flags2 |= (1L<<(i-32));
				}
			}
			int selected = UnusedRobotsList->GetSelIndex();
			UnusedRobotsList->DeleteString(selected);
			UnusedRobotsList->SetSelIndex(selected);
			UsedRobotsList->AddString(string);
		}
	}
	::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - DelMsg
//------------------------------------------------------------------------
void TCubeDialog::DelMsg() {
#if 1
	int i;
	int matcen;

	matcen = Segments ()[current->segment]->matcen_num;
	if (   Segments ()[current->segment]->special == SEGMENT_IS_ROBOTMAKER
		&& matcen >=0
		&& matcen < GameInfo ().matcen_howmany) {

		UsedRobotsList->GetSelString(message,sizeof(message));
		for (i=0;i<MAX_ROBOT_IDS_TOTAL;i++) {
			LoadString(GetModule()->GetInstance(),ROBOT_STRING_TABLE+i,string,sizeof(string));
			if (strcmp(message,string)==0) break;
		}
		if (i<MAX_ROBOT_IDS_TOTAL) {
			if (i<32) {
				robot_centers[matcen].objFlags &= ~(1L<<i);
			} else {
				if (i<64) {
					robot_centers[matcen].robot_flags2 &= ~(1L<<(i-32));
				}
			}
			int selected = UsedRobotsList->GetSelIndex();
			UsedRobotsList->DeleteString(selected);
			UsedRobotsList->SetSelIndex(selected);
			UnusedRobotsList->AddString(string);
		}
	}
	::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
//  DefaultProcessing();
#else
  HCURSOR hcurSave;
  unsigned char *data;
// Set the cursor to the hourglass and save the previous cursor
  hcurSave = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
  int i,j,k,l;
  FILE *file;
//  file = fopen("Triggers ().txt","w");
//  fprintf(file,"(level,trigger)\ttype\tflags\tvalue\ttime\tlink_num\n");
  file = fopen("Objects ().txt","w");
  fprintf(file,"type\tid\trtype\n");
  for (i=1;i<=27;i++) {
    sprintf(message,"c:\\c\\descent\\level%02d.rdl",i);
    free_texture_handles();
    mine->load(message);
//    for (j=0;j<GameInfo ().triggers_howmany;j++) {
//      fprintf(file,"(%02d,%02d)\t",i,j);
//      fprintf(file,"%02x\t",(UINT8)Triggers ()[j].type);
//      fprintf(file,"%04x\t",(UINT16)Triggers ()[j].flags);
//      fprintf(file,"%08lx\t",Triggers ()[j].value);
//      fprintf(file,"%08lx\t",Triggers ()[j].time);
//      fprintf(file,"%02x\n",(UINT8)Triggers ()[j].link_num);
//    }
    for (j=0;j<GameInfo ().object_howmany;j++) {
      if (Objects ()[j]->type == OBJ_ROBOT) {
	fprintf(file,"(%02d,%02d)\t",i,j);
	fprintf(file,"%02d\t",Objects ()[j]->type);
	fprintf(file,"%02d\t",Objects ()[j]->id);
	fprintf(file,"%02x\t",0xff&Objects ()[j]->ctype.ai_info.behavior);
	for (int k=0;k<11;k++) {
	  fprintf(file,"%02x,",0xff&Objects ()[j]->ctype.ai_info.flags[k]);
	}
	fprintf(file,"\n");
      }
//    fprintf(file,"%04ld\t",Objects ()[j]->size);
//    fprintf(file,"%04ld\t",Objects ()[j]->shields);
//      data = (unsigned char *) Objects ()[j];
//      data += 81; // mtype offset
//      for (k=0;k<10;k++) {
//        sprintf(message,"%02x",(UINT)*data++);
//        l = strlen(message);
//        fprintf(file,"%c%c",message[l-2],message[l-1]);
//      }
//      fprintf(file," ");
//      data = (unsigned char *) Objects ()[j];
//      data += 91; // ctype offset
//      for (k=0;k<84;k++) {
//        sprintf(message,"%02x",(UINT)*data++);
//	l = strlen(message);
//        fprintf(file,"%c%c",message[l-2],message[l-1]);
//      }
//    fprintf(file," ");
//      data = (unsigned char *) Objects ()[j];
//      data += 175; // rtype offset
//      for (k=0;k<73;k++) {
//        sprintf(message,"%02x",(UINT)*data++);
//        l = strlen(message);
//        fprintf(file,"%c%c",message[l-2],message[l-1]);
//      }
//      fprintf(file,"\n");
    }
  }
  fclose(file);
// Restore the previous cursor
  ::SetCursor(hcurSave);

#endif
}

//------------------------------------------------------------------------
// TCubeDialog - OtherCubeMsg
//------------------------------------------------------------------------
void TCubeDialog::OtherCubeMsg() {
  if (current == &current1) {
    current = &current2;
  } else {
    current = &current1;
  }
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_PAINT,0,0L);
}

//------------------------------------------------------------------------
// TCubeDialog - WallButtonMsg
//------------------------------------------------------------------------
void TCubeDialog::WallButtonMsg() {
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_USER+0x101,11,0);
}

//------------------------------------------------------------------------
// TCubeDialog - TriggerButtonMsg
//------------------------------------------------------------------------
void TCubeDialog::TriggerButtonMsg() {
  ::SendMessage(Parent->HWindow,WM_FIRST + WM_USER+0x101,10,0);
}

DEFINE_RESPONSE_TABLE1( TPointDialog, TDialog )
    EV_COMMAND( 107, ApplyMsg ),
    EV_MESSAGE(1280, User),
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1( TCubeDialog, TDialog )
    EV_COMMAND( 11, AddCubeMsg ),
    EV_COMMAND( 12, DeleteCubeMsg ),
    EV_LBN_DBLCLK( 13, UsedRobotMsg ),
    EV_COMMAND( 14, EditPointMsg ),
    EV_LBN_DBLCLK( 17, UnusedRobotMsg ),
    EV_LBN_SELCHANGE( 13, GrayButtons ),
    EV_LBN_SELCHANGE( 17, GrayButtons ),
    EV_COMMAND( 18, AddMsg ),
    EV_COMMAND( 19, DelMsg ),
    EV_COMMAND( 21, WallButtonMsg ),
    EV_COMMAND( 23, TriggerButtonMsg ),
    EV_COMMAND( 29, OtherCubeMsg ),
    EV_COMMAND( 30, EndOfExitTunnelMsg ),
    EV_LBN_SELCHANGE( 140, SpecialMsg ),
    EV_LBN_SELCHANGE( 141, CubeNumberMsg ),
    EV_LBN_SELCHANGE( 142, SideNumberMsg ),
    EV_LBN_SELCHANGE( 143, PointNumberMsg ),
    EV_EN_CHANGE( 170, LightMsg ),
    EV_MESSAGE(1280, User),
END_RESPONSE_TABLE;
