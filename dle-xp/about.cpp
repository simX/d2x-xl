// Copyright (C) 1997 Bryan Aamot
#include "myowl.h"
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dir.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "about.h"
#include "mine.h"
#include "global.h"
#include "proto.h"

#define ENABLE_PASSWORD 0

char *welcome_string1 =
  "Welcome to the Descent Level Editor XP\n\n"
  "In order to enable texture mapping,\n"
  "you need to select the Descent PIG files.\n"
  "Use the Browse buttons in the File/Preferences dialog\n"
  "to make your selection.\n\n"
  "Hint: You can choose any one of the listed PIG\n"
  "files for the Descent2 PIG.";
char *welcome_strint2 =
  "Now select 'View/Texture Mapped' or press F9\n"
  "to turn on texture mapping.\n\n"
  "Then hold the Ctrl key down as you move the mouse\n"
  "to move the level around on the screen.\n\n"
  "Hint: Shift+Ctrl+Mouse rotates the level.";

//************************************************************************
// DIALOG - TAboutDialog (constructor)
//************************************************************************
TAboutDialog::TAboutDialog(TWindow * AParent, LPSTR name):TDialog(AParent, name) {
  AboutButton    = new TButton(this,602);
  WarningStatic  = new TStatic(this,101,80);
  VersionStatic  = new TStatic(this,102,80);
  PasswordEdit	 = new TEdit(this,103,80);
  got_button_down = 0;
}

//************************************************************************
//
//************************************************************************
void TAboutDialog::EvLButtonDown( UINT, TPoint& ) {
  got_button_down = 1;
}

//************************************************************************
//
//************************************************************************
void TAboutDialog::EvLButtonUp( UINT, TPoint& ) {
  if (got_button_down) {
    CmOk();
  }
}

//************************************************************************
// TAboutDialog - ~TAboutDialog
//************************************************************************
TAboutDialog::~TAboutDialog() {
}

//************************************************************************
// TAboutDialog - cmOK
//************************************************************************
void TAboutDialog::CmOk() {
#if ENABLE_PASSWORD
  TMessage Msg = __GetTMessage();

  INT16 fwKeys = Msg.WParam;  // key flags
  if (fwKeys & MK_SHIFT && fwKeys & MK_CONTROL) {
    char name[21];
    if (strncmp(testers_name,"TESTERS",7)==0) {
      strcpy(name,"Interplay");
    } else {
      for (int i=0;i<20;i++) {
	name[i] = testers_name[i] ^ 0xBA;
      }
      name[20] = NULL;
    }
    sprintf(message,"User '%s'\nSerial Number %d",name,serial_number);
    InfoMsg(message);
  } else {
    // save password
    PasswordEdit->GetText(message,sizeof(message));
    WritePrivateProfileString("dle-xp", "Password", message, "dlc2.ini");

    // calculate hardware specific string
//    UINT16 v5 = _8087;
    // read drive c: label

    // enable saves if password is good
    PasswordEdit->GetText(testers_name,sizeof(testers_name));
    testers_name[19] = NULL; // make sure it is null terminated

    // figure sum of xor of password rotated left 3 bits
    UINT8 sum = 0;
    int i;
    for (i=0;i<strlen(testers_name);i++) {
      sum ^= message[i];
    }
    if (sum == 0 && strlen(testers_name) > 10) {
	InfoMsg("Saves have been enabled.");
	serial_number = atoi(&testers_name[strlen(testers_name)-5]);;
	testers_name[strlen(testers_name)-5] = NULL;
	disable_saves = FALSE;
    }
  }
#else
//  disable_saves = FALSE;
#endif

  TDialog::CmOk();
}

//************************************************************************
// TAboutDialog - SetupWindow
//************************************************************************
void TAboutDialog::SetupWindow() {
  RECT main_rect;
//  RECT rect,main_rect,client_rect;

  TDialog::SetupWindow();

  // set timer which automatically closes the window in 10 seconds
//  ::SetTimer(HWindow, 1, 10000U, NULL);

  // center window
  ::GetWindowRect(::GetDesktopWindow(), &main_rect);
//  ::GetWindowRect(HWindow, &rect);
//  ::GetClientRect(Parent->HWindow, &client_rect);
  ::SetWindowPos(HWindow,HWND_TOPMOST,
    (main_rect.right  - AboutButton->Attr.W)/2,
    (main_rect.bottom - AboutButton->Attr.H)/2,
//    abs(abs(main_rect.right-main_rect.left)/2 - abs(rect.right-rect.left)/2)- client_rect.left,
//    abs(abs(main_rect.bottom-main_rect.top)/2 - abs(rect.bottom-rect.top)/2)- client_rect.top,
#if ENABLE_PASSWORD
    abs(rect.right-rect.left),abs(rect.top-rect.bottom),
#else
    AboutButton->Attr.W + 8, AboutButton->Attr.H + 8,
//    553+8,300+8, // bitmap size plus border of 4 on all sides
#endif
    SWP_NOZORDER | SWP_NOACTIVATE);


#if !ENABLE_PASSWORD
  // hide static text and edit box
  ::ShowWindow(GetItemHandle(101),SW_HIDE);
  ::ShowWindow(GetItemHandle(102),SW_HIDE);
  ::ShowWindow(GetItemHandle(103),SW_HIDE);
#endif



  sprintf(message,"Version %s",dlc_version);
  VersionStatic->SetText(message);

  // display password
  GetPrivateProfileString("dle-xp", "Password", "", message, sizeof(message), "dle-xp.ini");
  PasswordEdit->SetText(message);

#if 0
  // display key
  // read c: drive label
  struct ffblk ffblk;
  findfirst("c:\*.*",&ffblk,FA_LABEL);
  sprintf(message,"%02x%02x%02x%02x%02x%02x%02x%02x",
	ffblk.ff_name[0],ffblk.ff_name[1],ffblk.ff_name[2],ffblk.ff_name[3],
	ffblk.ff_name[4],ffblk.ff_name[5],ffblk.ff_name[6],ffblk.ff_name[7]);
  WarningStatic->SetText(message);
#endif
  WarningStatic->SetText("This software is to be used for beta testing only.\n"
			 "Please, do not distribute for any reason.  Thanks.");
}

//==========================================================================
// TAboutDialog - Timer
//==========================================================================
void TAboutDialog::EvTimer( UINT ) {
//  CloseWindow();
}

//************************************************************************
// TTextureDialog - WMDrawItem
//************************************************************************
void TAboutDialog::EvDrawItem( UINT, DRAWITEMSTRUCT FAR & ) {
}



DEFINE_RESPONSE_TABLE1( TAboutDialog, TDialog )
    EV_WM_DRAWITEM,
    EV_WM_TIMER,
    EV_WM_LBUTTONUP,
    EV_WM_LBUTTONDOWN,
    EV_COMMAND (IDOK, CmOk),
END_RESPONSE_TABLE;
