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

#ifndef _GAUGES_H
#define _GAUGES_H

#include "fix.h"
#include "gr.h"
#include "piggy.h"
#include "object.h"
#include "hudmsg.h"
#include "ogl_defs.h"
#include "ogl_bitmap.h"
#include "ogl_hudstuff.h"
//from gauges.c

// Flags for gauges/hud stuff
extern ubyte Reticle_on;

void InitGaugeCanvases();
void CloseGaugeCanvases();

void ShowScore ();
void ShowScoreAdded ();
void AddPointsToScore (int);
void AddBonusPointsToScore (int);

void RenderGauges(void);
void InitGauges(void);
void check_erase_message(void);

void HUDRenderMessageFrame();
void HUDClearMessages();

// Call to flash a message on the HUD.  Returns true if message drawn.
// (message might not be drawn if previous message was same)
#define gauge_message HUDInitMessage

void DrawHUD();     // draw all the HUD stuff

void PlayerDeadMessage(void);
//void say_afterburner_status(void);

// fills in the coords of the hostage video window
void get_hostage_window_coords(int *x, int *y, int *w, int *h);

// from testgaug.c

void gauge_frame(void);
void UpdateLaserWeaponInfo(void);
void PlayHomingWarning(void);

extern tRgbColorb playerColors [];

#define WBU_WEAPON			0       // the weapons display
#define WBUMSL					1       // the missile view
#define WBU_ESCORT			2       // the "buddy bot"
#define WBU_REAR				3       // the rear view
#define WBU_COOP				4       // coop or team member view
#define WBU_GUIDED			5       // the guided missile
#define WBU_MARKER			6       // a dropped marker
#define WBU_STATIC			7       // playing static after missile hits
#define WBU_RADAR_TOPDOWN	8
#define WBU_RADAR_HEADSUP	9

// draws a 3d view into one of the cockpit windows.  win is 0 for
// left, 1 for right.  viewer is CObject.  NULL CObject means give up
// window user is one of the WBU_ constants.  If rearViewFlag is
// set, show a rear view.  If label is non-NULL, print the label at
// the top of the window.
void HUDRenderWindow(int win, CObject *viewer, int rearViewFlag, int user, const char *label);
void FreeInventoryIcons (void);
void FreeObjTallyIcons (void);
void HUDShowIcons (void);
int CanSeeObject(int nObject, int bCheckObjs);
void ShowFrameRate (void);
void ToggleCockpit ();

#define SHOW_COCKPIT	((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) || (gameStates.render.cockpit.nMode == CM_STATUS_BAR))
#define SHOW_HUD		(!gameStates.app.bEndLevelSequence && (!gameStates.app.bNostalgia || gameOpts->render.cockpit.bHUD || !SHOW_COCKPIT))
#define HIDE_HUD		(gameStates.app.bEndLevelSequence || (!(gameStates.app.bNostalgia || gameOpts->render.cockpit.bHUD) && (gameStates.render.cockpit.nMode >= CM_FULL_SCREEN)))

extern double cmScaleX, cmScaleY;
extern int nHUDLineSpacing;

//	-----------------------------------------------------------------------------

#define HUD_SCALE(v, s)	(int (float (v) * (s) /*+ 0.5*/))
#define HUD_SCALE_X(v)	HUD_SCALE (v, m_info.xScale)
#define HUD_SCALE_Y(v)	HUD_SCALE (v, m_info.yScale)
#define HUD_LHX(x)      (gameStates.menus.bHires ? 2 * (x) : x)
#define HUD_LHY(y)      (gameStates.menus.bHires? (24 * (y)) / 10 : y)
#define HUD_ASPECT		((double) screen.Height () / (double) screen.Width () / 0.75)

//	-----------------------------------------------------------------------------

CBitmap* HUDBitBlt (int nGauge, int x, int y, bool bScalePos = true, bool bScaleSize = true, int scale = I2X (1), int orient = 0, CBitmap* bmP = NULL);

//	-----------------------------------------------------------------------------

#endif /* _GAUGES_H */
