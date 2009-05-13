// Copyright (C) 1997 Bryan Aamot
#include "myowl.h"
#include <stdio.h>
#include <dir.h>
#include <ctype.h>
#include <commdlg.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "global.h"
#include "prefs.h"


// external function prototypes
void free_texture_handles();
int QueryMsg(LPCSTR lpszText); // dlc.cpp
BOOL has_custom_textures(); // file.cpp

#define MAX_PREFS 3

//************************************************************************
// DIALOG - TPreferencesDialog (constructor)
//************************************************************************
TPreferencesDialog::TPreferencesDialog(TWindow * AParent, LPSTR name)
                   :TDialog(AParent, name) {

  GameDirectory   = new TEdit(this,101,sizeof(descent_path));
  GameDirectory2  = new TEdit(this,107,sizeof(descent2_path));
  LevelsDirectory = new TEdit(this,102,sizeof(levels_path));
  DepthComboBox   = new TComboBox(this,103,20);
  Grid            = new TEdit(this,104,20);
  MoveRate        = new TEdit(this,105,20);
  MineName        = new TEdit(this,106,sizeof(current_level_name));
  Rotate1         = new TCheckBox(this,110,NULL);
  Rotate2         = new TCheckBox(this,111,NULL);
  Rotate3         = new TCheckBox(this,112,NULL);
  Rotate4         = new TCheckBox(this,113,NULL);
  for (int i=0;i<MAX_PREFS;i++) {
	PrefCheck[i]  = new TCheckBox(this,200+i,NULL);
  }
}

//************************************************************************
// TPreferencesDialog - SetupWindow
//************************************************************************
void TPreferencesDialog::SetupWindow() {
  TDialog::SetupWindow();
  MineName->SetText(current_level_name);
  GameDirectory->SetText(descent_path);
  GameDirectory2->SetText(descent2_path);
  LevelsDirectory->SetText(levels_path);
  sprintf(message,"%f",(double)((double)grid/(double)0x10000L) );
  Grid->SetText(message);
  sprintf(message,"%f",(double)((double)move_rate/(double)0x10000L) );
  MoveRate->SetText(message);
  Rotate1->Uncheck();
  Rotate2->Uncheck();
  Rotate3->Uncheck();
  Rotate4->Uncheck();
  if (angle_rate>(double)PI*(3.0/16.0)) {
    Rotate1->Check();
  } else {
	if (angle_rate>(double)PI*(3.0/32.0)) {
      Rotate2->Check();
    } else {
	  if (angle_rate>(double)PI*(3.0/64.0)) {
	Rotate3->Check();
      } else {
        Rotate4->Check();
      }
    }
  }
  DepthComboBox->AddString("High");
  DepthComboBox->AddString("Medium");
  DepthComboBox->AddString("Low");
  DepthComboBox->AddString("Off");
  if (depth_perception <= 50) {
    DepthComboBox->SetSelString("High",-1);
  } else {
    if (depth_perception <= 100) {
      DepthComboBox->SetSelString("Medium",-1);
    } else {
      if (depth_perception <= 200) {
        DepthComboBox->SetSelString("Low",-1);
      } else {
	DepthComboBox->SetSelString("Off",-1);
      }
    }
  }
  for (int i=0;i<MAX_PREFS;i++) {
	if (preferences & (1<<i)) {
	PrefCheck[i]->Check();
	} else {
	PrefCheck[i]->Uncheck();
    }
  }
}

//************************************************************************
// TPreferencesDialog - Rotate 1
//************************************************************************
void TPreferencesDialog::Rotate1Msg( UINT )
{

  Rotate1->Check();
  Rotate2->Uncheck();
  Rotate3->Uncheck();
  Rotate4->Uncheck();
}

//************************************************************************
// TPreferencesDialog - Rotate 2
//************************************************************************
void TPreferencesDialog::Rotate2Msg( UINT )
{

  Rotate1->Uncheck();
  Rotate2->Check();
  Rotate3->Uncheck();
  Rotate4->Uncheck();
}

//************************************************************************
// TPreferencesDialog - Rotate 3
//************************************************************************
void TPreferencesDialog::Rotate3Msg( UINT )
{

  Rotate1->Uncheck();
  Rotate2->Uncheck();
  Rotate3->Check();
  Rotate4->Uncheck();
}

//************************************************************************
// TPreferencesDialog - Rotate 4
//************************************************************************
void TPreferencesDialog::Rotate4Msg( UINT )
{

  Rotate1->Uncheck();
  Rotate2->Uncheck();
  Rotate3->Uncheck();
  Rotate4->Check();
}

//************************************************************************
// TPreferencesDialog - Browse 1
//************************************************************************
void TPreferencesDialog::Browse1Msg( UINT )
{

  OPENFILENAME ofn;
  char szFile[80] = "\0";

  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "Descent Pig\0descent.pig\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "pig";
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  if (GetOpenFileName(&ofn)) {
    strlwr(ofn.lpstrFile);
    GameDirectory->SetText(ofn.lpstrFile);
  }
}

//************************************************************************
// TPreferencesDialog - Browse 2
//************************************************************************
void TPreferencesDialog::Browse2Msg( UINT )
{

  OPENFILENAME ofn;
  char szFile[80] = "\0";

  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "Descent 2 Pig\0*.pig\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "pig";
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  if (GetOpenFileName(&ofn)) {
	strlwr(ofn.lpstrFile);
    GameDirectory2->SetText(ofn.lpstrFile);
  }
}

//************************************************************************
// TPreferencesDialog - Ok
//************************************************************************
void TPreferencesDialog::Ok( UINT ) {
	MineName->GetText(current_level_name,sizeof(current_level_name));

	GameDirectory->GetText(message,sizeof(message));
	if (strcmp(message,descent_path) != 0) {
		strcpy(descent_path,message);
		WritePrivateProfileString("dlc", "DescentDirectory", descent_path, "dlc2.ini");
		if (file_type == RDL_FILE) {
			free_texture_handles();
		}
	}
	GameDirectory2->GetText(message,sizeof(message));
	if (strcmp(message,descent2_path) != 0) {
		int ok_to_change_pig = 1;
		if (has_custom_textures()) {
			if (QueryMsg(
				"Changing the pig file will delete the custom textures\n"
				"in this level because of the change in palette.\n\n"
				"Are you sure you want to do this?") != IDOK) {

				ok_to_change_pig = 0;
			}
		}
		if (ok_to_change_pig) {
			strcpy(descent2_path,message);
			WritePrivateProfileString("dlc", "DescentDirectory2", descent2_path, "dlc2.ini");
			if (file_type != RDL_FILE) {
				free_texture_handles();
			}
		}

		LevelsDirectory->GetText(levels_path,sizeof(levels_path));
		WritePrivateProfileString("dlc", "LevelsDirectory",  levels_path,  "dlc2.ini");
		// set current directory to levels directory
		chdir(levels_path);
		if (levels_path[1] == ':') {
			if (isalpha(levels_path[0])) {
				setdisk(toupper(levels_path[0]) - 'A');
			}
		}
	}

//		DepthComboBox->AddString("High");
//  	DepthComboBox->AddString("Medium");
//  	DepthComboBox->AddString("Low");
//  	DepthComboBox->AddString("Off");
	if (DepthComboBox->GetSelString(message,10) != -1) {
		switch (message[0]) {
		case 'H':
			depth_perception = 50;
		break;
		case 'M':
			depth_perception = 100;
		break;
		case 'L':
			depth_perception = 200;
		break;
		case 'O':
			depth_perception = 1000;
		break;
		}
	}
	sprintf(message,"%f",depth_perception);
	WritePrivateProfileString("dlc", "DepthPerception", message, "dlc2.ini");


	Grid->GetText(message,sizeof(message));
	grid = (FIX)(atof(message)*0x10000L);
	if (grid<=0) grid = 0x1L;

	// get move rate
	MoveRate->GetText(message,sizeof(message));
	move_rate = (FIX)(atof(message)*0x10000L);
	WritePrivateProfileString("dlc", "MoveRate",message, "dlc2.ini");

	if (Rotate1->GetCheck() == BF_CHECKED) {
		angle_rate = PI/4;
	} else {
		if (Rotate2->GetCheck() == BF_CHECKED) {
			angle_rate = PI/8;
		} else {
			if (Rotate3->GetCheck() == BF_CHECKED) {
				angle_rate = PI/16;
			} else {
				if (Rotate4->GetCheck() == BF_CHECKED) {
					angle_rate = PI/32;
				} else {
					angle_rate = PI/16;
				}
			}
		}
	}
	sprintf(message,"%f",angle_rate);
	WritePrivateProfileString("dlc", "AngleRate", message, "dlc2.ini");

	preferences = 0;
	for (int i=0;i<MAX_PREFS;i++) {
		if (PrefCheck[i]->GetCheck() == BF_CHECKED) {
			preferences |= 1<<i;
		}
	}
	sprintf(message,"%ld",preferences);
	WritePrivateProfileString("dlc", "Prefs", message, "dlc2.ini");

	TDialog::CmOk();
}


DEFINE_RESPONSE_TABLE1( TPreferencesDialog, TDialog )
	EV_CHILD_NOTIFY_ALL_CODES( 1, Ok ),
	EV_CHILD_NOTIFY_ALL_CODES( 108, Browse1Msg ),
	EV_CHILD_NOTIFY_ALL_CODES( 109, Browse2Msg ),
	EV_CHILD_NOTIFY_ALL_CODES( 110, Rotate1Msg ),
	EV_CHILD_NOTIFY_ALL_CODES( 111, Rotate2Msg ),
	EV_CHILD_NOTIFY_ALL_CODES( 112, Rotate3Msg ),
	EV_CHILD_NOTIFY_ALL_CODES( 113, Rotate4Msg ),
END_RESPONSE_TABLE;
