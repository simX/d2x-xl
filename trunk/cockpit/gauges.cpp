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

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "inferno.h"
#include "screens.h"
#include "error.h"
#include "newdemo.h"
#include "gamefont.h"
#include "text.h"
#include "network.h"
#include "network_lib.h"
#include "render.h"
#include "transprender.h"
#include "gamepal.h"
#include "ogl_lib.h"
#include "ogl_bitmap.h"
#include "ogl_hudstuff.h"
#include "playsave.h"
#include "gauges.h"
#include "hud_defs.h"
#include "statusbar.h"
#include "slowmotion.h"

#define SHOW_PLAYER_IP		0

void DrawGuidedCrosshair (void);
void DoAutomap (int key_code, int bRadar);

double cmScaleX, cmScaleY;
int nHUDLineSpacing;

gsrCanvas *Canv_LeftEnergyGauge;
gsrCanvas *Canv_AfterburnerGauge;
gsrCanvas *Canv_RightEnergyGauge;
gsrCanvas *Canv_NumericalGauge;

//Flags for gauges/hud stuff
//bitmap numbers for gauges


//change MAX_GAUGE_BMS when adding gauges

//Coordinats for gauges

int scoreDisplay [2];
fix scoreTime;
fix	lastWarningBeepTime [2] = {0, 0};		//	Time we last played homing missile warning beep.

int bHaveGaugeCanvases = 0;
int nInvulnerableFrame = 0;
int nCloakFadeState;		//0=steady, -1 fading out, 1 fading in

#define WS_SET				0		//in correct state
#define WS_FADING_OUT	1
#define WS_FADING_IN		2

int weaponBoxUser [2] = {WBU_WEAPON, WBU_WEAPON};		//see WBU_ constants in gauges.h
int weaponBoxStates [2];
fix weaponBoxFadeValues [2];

#define FADE_SCALE	 (2*I2X (GR_ACTUAL_FADE_LEVELS)/REARM_TIME)		// fade out and back in REARM_TIME, in fade levels per seconds (int)

//store delta x values from left of box
tSpan weaponWindowLeft [] = {		//first tSpan 67, 151
		{8, 51},
		{6, 53},
		{5, 54},
		{4-1, 53+2},
		{4-1, 53+3},
		{4-1, 53+3},
		{4-2, 53+3},
		{4-2, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-1, 53+3},
		{3-2, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{2-1, 53+3},
		{1-1, 53+3},
		{1-1, 53+2},
		{1-1, 53+2},
		{1-1, 53+2},
		{1-1, 53+2},
		{1-1, 53+2},
		{1-1, 53+2},
		{1-1, 53+2},
		{0, 53+2},
		{0, 53+2},
		{0, 53+2},
		{0, 53+2},
		{0, 52+3},
		{1-1, 52+2},
		{2-2, 51+3},
		{3-2, 51+2},
		{4-2, 50+2},
		{5-2, 50},
		{5-2+2, 50-2},
	};


//store delta x values from left of box
tSpan weaponWindowRight [] = {		//first tSpan 207, 154
		{208-202, 255-202},
		{206-202, 257-202},
		{205-202, 258-202},
		{204-202, 259-202},
		{203-202, 260-202},
		{203-202, 260-202},
		{203-202, 260-202},
		{203-202, 260-202},
		{203-202, 260-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 261-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{203-202, 262-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{204-202, 263-202},
		{205-202, 263-202},
		{206-202, 262-202},
		{207-202, 261-202},
		{208-202, 260-202},
		{211-202, 255-202},
	};

//store delta x values from left of box
tSpan weaponWindowLeftHires [] = {		//first tSpan 67, 154
	{20, 110},
	{18, 113},
	{16, 114},
	{15, 116},
	{14, 117},
	{13, 118},
	{12, 119},
	{11, 119},
	{10, 120},
	{10, 120},
	{9, 121},
	{8, 121},
	{8, 121},
	{8, 122},
	{7, 122},
	{7, 122},
	{7, 122},
	{7, 122},
	{7, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{6, 122},
	{5, 122},
	{5, 122},
	{5, 122},
	{5, 122},
	{5, 121},
	{5, 121},
	{5, 121},
	{5, 121},
	{5, 121},
	{5, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{4, 121},
	{3, 121},
	{3, 121},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{3, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{2, 120},
	{1, 120},
	{1, 120},
	{1, 119},
	{1, 119},
	{1, 119},
	{1, 119},
	{1, 119},
	{1, 119},
	{1, 119},
	{1, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 118},
	{0, 118},
	{0, 118},
	{0, 117},
	{0, 117},
	{0, 117},
	{1, 116},
	{1, 116},
	{2, 115},
	{2, 114},
	{3, 113},
	{4, 112},
	{5, 111},
	{5, 110},
	{7, 109},
	{9, 107},
	{10, 105},
	{12, 102},
};


//store delta x values from left of box
tSpan weaponWindowRightHires [] = {		//first tSpan 207, 154
	{12, 105},
	{9, 107},
	{8, 109},
	{6, 110},
	{5, 111},
	{4, 112},
	{3, 113},
	{3, 114},
	{2, 115},
	{2, 115},
	{1, 116},
	{1, 117},
	{1, 117},
	{0, 117},
	{0, 118},
	{0, 118},
	{0, 118},
	{0, 118},
	{0, 118},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 119},
	{0, 120},
	{0, 120},
	{0, 120},
	{0, 120},
	{1, 120},
	{1, 120},
	{1, 120},
	{1, 120},
	{1, 120},
	{1, 120},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 121},
	{1, 122},
	{1, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 122},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 123},
	{2, 124},
	{2, 124},
	{3, 124},
	{3, 124},
	{3, 124},
	{3, 124},
	{3, 124},
	{3, 124},
	{3, 124},
	{3, 125},
	{3, 125},
	{3, 125},
	{3, 125},
	{3, 125},
	{3, 125},
	{3, 125},
	{3, 125},
	{4, 125},
	{4, 125},
	{4, 125},
	{5, 125},
	{5, 125},
	{5, 125},
	{6, 125},
	{6, 124},
	{7, 123},
	{8, 123},
	{9, 122},
	{10, 121},
	{11, 120},
	{12, 120},
	{13, 118},
	{15, 117},
	{18, 115},
	{20, 114},
};


tGaugeBox gaugeBoxes [] = {
// primary left/right low res
	{PRIMARY_W_BOX_LEFT_L, PRIMARY_W_BOX_TOP_L, PRIMARY_W_BOX_RIGHT_L, PRIMARY_W_BOX_BOT_L, weaponWindowLeft},
	{SECONDARY_W_BOX_LEFT_L, SECONDARY_W_BOX_TOP_L, SECONDARY_W_BOX_RIGHT_L, SECONDARY_W_BOX_BOT_L, weaponWindowRight},
//sb left/right low res
	{SB_PRIMARY_W_BOX_LEFT_L, SB_PRIMARY_W_BOX_TOP_L, SB_PRIMARY_W_BOX_RIGHT_L, SB_PRIMARY_W_BOX_BOT_L, NULL},
	{SB_SECONDARY_W_BOX_LEFT_L, SB_SECONDARY_W_BOX_TOP_L, SB_SECONDARY_W_BOX_RIGHT_L, SB_SECONDARY_W_BOX_BOT_L, NULL},
// primary left/right hires
	{PRIMARY_W_BOX_LEFT_H, PRIMARY_W_BOX_TOP_H, PRIMARY_W_BOX_RIGHT_H, PRIMARY_W_BOX_BOT_H, weaponWindowLeftHires},
	{SECONDARY_W_BOX_LEFT_H, SECONDARY_W_BOX_TOP_H, SECONDARY_W_BOX_RIGHT_H, SECONDARY_W_BOX_BOT_H, weaponWindowRightHires},
// sb left/right hires
	{SB_PRIMARY_W_BOX_LEFT_H, SB_PRIMARY_W_BOX_TOP_H, SB_PRIMARY_W_BOX_RIGHT_H, SB_PRIMARY_W_BOX_BOT_H, NULL},
	{SB_SECONDARY_W_BOX_LEFT_H, SB_SECONDARY_W_BOX_TOP_H, SB_SECONDARY_W_BOX_RIGHT_H, SB_SECONDARY_W_BOX_BOT_H, NULL},
	};

// these macros refer to arrays above

int oldScore [2]			= {-1, -1};
int oldEnergy [2]			= {-1, -1};
int oldShields [2]		= {-1, -1};
uint oldFlags [2]			= {(uint) -1, (uint) -1};
int oldWeapon [2][2]		= {{-1, -1}, {-1, -1}};
int oldAmmoCount [2][2]	= {{-1, -1}, {-1, -1}};
int xOldOmegaCharge [2]	= {-1, -1};
int oldLaserLevel [2]	= {-1, -1};
int bOldCloak [2]			= {0, 0};
int oldLives [2]			= {-1, -1};
fix oldAfterburner [2]	= {-1, -1};
int oldBombcount [2]		= {0, 0};

#define COCKPIT_PRIMARY_BOX		 (!gameStates.video.nDisplayMode?0:4)
#define COCKPIT_SECONDARY_BOX		 (!gameStates.video.nDisplayMode?1:5)
#define SB_PRIMARY_BOX				 (!gameStates.video.nDisplayMode?2:6)
#define SB_SECONDARY_BOX			 (!gameStates.video.nDisplayMode?3:7)

static double xScale, yScale;

//	-----------------------------------------------------------------------------

inline void HUDUScanLine (int left, int right, int y)
{
gr_uscanline (HUD_SCALE_X (left), HUD_SCALE_X (right), HUD_SCALE_Y (y));
}

//	-----------------------------------------------------------------------------

int _CDECL_ HUDPrintF (int *idP, int x, int y, const char *pszFmt, ...)
{
	static char szBuf [1000];
	va_list args;

va_start (args, pszFmt);
vsprintf (szBuf, pszFmt, args);
return GrString (HUD_SCALE_X (x), HUD_SCALE_Y (y), szBuf, idP);
}

//	-----------------------------------------------------------------------------

//copy a box from the off-screen buffer to the visible page

void CopyGaugeBox (tGaugeBox *box, grsBitmap *bmP)
{
#if 0
if (box->spanlist) {
	int n_spans = box->bot-box->top+1;
	int cnt, y;

for (cnt=0, y=box->top;cnt<n_spans;cnt++, y++) {
	GrBmUBitBlt (box->spanlist [cnt].r-box->spanlist [cnt].l+1, 1,
				box->left+box->spanlist [cnt].l, y, box->left+box->spanlist [cnt].l, y, bmP, &grdCurCanv->cvBitmap);
	 	}
 	}
else {
	GrBmUBitBlt (box->right-box->left+1, box->bot-box->top+1,
					box->left, box->top, box->left, box->top,
					bmP, &grdCurCanv->cvBitmap);
	}
#endif
}

//	-----------------------------------------------------------------------------
//fills in the coords of the hostage video window
void GetHostageWindowCoords (int *x, int *y, int *w, int *h)
{
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBGetHostageWindowCoords (x, y, w, h);
else {
	*x = SECONDARY_W_BOX_LEFT;
	*y = SECONDARY_W_BOX_TOP;
	*w = SECONDARY_W_BOX_RIGHT - SECONDARY_W_BOX_LEFT + 1;
	*h = SECONDARY_W_BOX_BOT - SECONDARY_W_BOX_TOP + 1;
	}
}

//	-----------------------------------------------------------------------------
//these should be in gr.h
#define cv_w  cvBitmap.bmProps.w
#define cv_h  cvBitmap.bmProps.h

void HUDShowScore (void)
{
	char	szScore [40];
	int	w, h, aw;

if (HIDE_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) &&
	 (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;
GrSetCurFont (GAME_FONT);
szScore [0] = (char) 1;
szScore [1] = (char) (127 + 128);
szScore [2] = (char) (127 + 128);
szScore [3] = (char) (0 + 128);
#if !DBG
if (!SlowMotionActive ())
	strcpy (szScore + 4, "          ");
else
#endif
	sprintf (szScore + 4, "M%1.1f S%1.1f ",
				gameStates.gameplay.slowmo [0].fSpeed,
				gameStates.gameplay.slowmo [1].fSpeed);
szScore [14] = (char) 1;
szScore [15] = (char) (0 + 128);
szScore [16] = (char) (127 + 128);
szScore [17] = (char) (0 + 128);
if ((IsMultiGame && !IsCoopGame))
	sprintf (szScore + 18, "   %s: %5d", TXT_KILLS, LOCALPLAYER.netKillsTotal);
else
	sprintf (szScore + 18, "   %s: %5d", TXT_SCORE, LOCALPLAYER.score);
GrGetStringSize (szScore, &w, &h, &aw);
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
GrPrintF (NULL, grdCurCanv->cv_w-w-HUD_LHX (2), 3, szScore);
}

//	-----------------------------------------------------------------------------

void HUDShowTimerCount (void)
 {
	char	szScore [20];
	int	w, h, aw, i;
	fix timevar = 0;

	static int nIdTimer = 0;

if (HIDE_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) && (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;

if ((gameData.app.nGameMode & GM_NETWORK) && netGame.xPlayTimeAllowed) {
	timevar = I2X (netGame.xPlayTimeAllowed*5*60);
	i = X2I (timevar-gameStates.app.xThisLevelTime);
	i++;
	sprintf (szScore, "T - %5d", i);
	GrGetStringSize (szScore, &w, &h, &aw);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	if ((i >= 0) && !gameData.reactor.bDestroyed)
		nIdTimer = GrPrintF (&nIdTimer, grdCurCanv->cv_w-w-HUD_LHX (10), HUD_LHX (11), szScore);
	}
}

//	-----------------------------------------------------------------------------
//y offset between lines on HUD

 void HUDShowScoreAdded ()
{
	int	color;
	int	w, h, aw;
	char	szScore [20];

	static int nIdTotalScore = 0;

if (HIDE_HUD)
	return;
if (IsMultiGame && !IsCoopGame)
	return;
if (scoreDisplay [0] == 0)
	return;
GrSetCurFont (GAME_FONT);
scoreTime -= gameData.time.xFrame;
if (scoreTime > 0) {
	color = X2I (scoreTime * 20) + 12;
	if (color < 10)
		color = 12;
	else if (color > 31)
		color = 30;
	color = color - (color % 4);	//	Only allowing colors 12, 16, 20, 24, 28 speeds up gr_getcolor, improves caching
	if (gameStates.app.cheats.bEnabled)
		sprintf (szScore, "%s", TXT_CHEATER);
	else
		sprintf (szScore, "%5d", scoreDisplay [0]);
	GrGetStringSize (szScore, &w, &h, &aw);
	GrSetFontColorRGBi (RGBA_PAL2 (0, color, 0), 1, 0, 0);
	nIdTotalScore = GrPrintF (&nIdTotalScore, grdCurCanv->cv_w-w-HUD_LHX (2+10), nHUDLineSpacing+4, szScore);
	}
else {
	scoreTime = 0;
	scoreDisplay [0] = 0;
	}
}

//	-----------------------------------------------------------------------------

void PlayHomingWarning (void)
{
	fix	xBeepDelay;

if (gameStates.app.bEndLevelSequence || gameStates.app.bPlayerIsDead)
	return;
if (LOCALPLAYER.homingObjectDist >= 0) {
	xBeepDelay = LOCALPLAYER.homingObjectDist/128;
	if (xBeepDelay > F1_0)
		xBeepDelay = F1_0;
	else if (xBeepDelay < F1_0/8)
		xBeepDelay = F1_0/8;
	if (lastWarningBeepTime [gameStates.render.vr.nCurrentPage] > gameData.time.xGame)
		lastWarningBeepTime [gameStates.render.vr.nCurrentPage] = 0;
	if (gameData.time.xGame - lastWarningBeepTime [gameStates.render.vr.nCurrentPage] > xBeepDelay/2) {
		DigiPlaySample (SOUND_HOMING_WARNING, F1_0);
		lastWarningBeepTime [gameStates.render.vr.nCurrentPage] = gameData.time.xGame;
		}
	}
}

int	bLastHomingWarningShown [2]={-1, -1};

//	-----------------------------------------------------------------------------

void ShowHomingWarning (void)
{
if ((gameStates.render.cockpit.nMode == CM_STATUS_BAR) || gameStates.app.bEndLevelSequence)
	SBShowHomingWarning ();
else {
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	if (LOCALPLAYER.homingObjectDist >= 0) {
		if (gameData.time.xGame & 0x4000) {
			PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_ON);
			HUDBitBlt (
				HOMING_WARNING_X, HOMING_WARNING_Y,
				gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_ON), F1_0, 0);
				bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 1;
			}
		else {
			PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_OFF);
			HUDBitBlt (
				HOMING_WARNING_X, HOMING_WARNING_Y,
				gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_OFF), F1_0, 0);
			bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 0;
			}
		}
	else {
		PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_OFF);
		HUDBitBlt (
			HOMING_WARNING_X, HOMING_WARNING_Y,
			gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_OFF), F1_0, 0);
		bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 0;
		}
	}
}

//	-----------------------------------------------------------------------------

extern int SW_y [2];

void HUDShowHomingWarning (void)
{
	static int nIdLock = 0;

if (HIDE_HUD)
	return;
if (LOCALPLAYER.homingObjectDist >= 0) {
	if (gameData.time.xGame & 0x4000) {
		int x = 0x8000, y = grdCurCanv->cv_h-nHUDLineSpacing;
		if ((weaponBoxUser [0] != WBU_WEAPON) || (weaponBoxUser [1] != WBU_WEAPON)) {
			int wy = (weaponBoxUser [0] != WBU_WEAPON) ? SW_y [0] : SW_y [1];
			y = min (y, (wy - nHUDLineSpacing - gameData.render.window.y));
			}
		GrSetCurFont (GAME_FONT);
		GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
		nIdLock = GrPrintF (&nIdLock, x, y, TXT_LOCK);
		}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowKeys (void)
{
	int y = 3 * nHUDLineSpacing;
	int dx = GAME_FONT->ftWidth + GAME_FONT->ftWidth / 2;

if (HIDE_HUD)
	return;
if (IsMultiGame && !IsCoopGame)
	return;
if (LOCALPLAYER.flags & PLAYER_FLAGS_BLUE_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_BLUE);
	GrUBitmapM (2, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_BLUE)]);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_GOLD_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_YELLOW);
	GrUBitmapM (2+dx, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_YELLOW)]);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_RED_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_RED);
	GrUBitmapM (2+2*dx, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_RED)]);
	}
}

//	-----------------------------------------------------------------------------

void HUDShowOrbs (void)
{
	static int nIdOrbs = 0, nIdEntropy [2]= {0, 0};

if (HIDE_HUD)
	return;
if (gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)) {
	int x = 0, y = 0;
	grsBitmap *bmP = NULL;

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
		y = 2*nHUDLineSpacing;
		x = 4*GAME_FONT->ftWidth;
		}
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
		y = nHUDLineSpacing;
		x = GAME_FONT->ftWidth;
		}
	else if ((gameStates.render.cockpit.nMode == CM_FULL_SCREEN) ||
				(gameStates.render.cockpit.nMode == CM_LETTERBOX)) {
//			y = 5*nHUDLineSpacing;
		y = nHUDLineSpacing;
		x = GAME_FONT->ftWidth;
		if (gameStates.render.fonts.bHires)
			y += nHUDLineSpacing;
		}
	else
		Int3 ();		//what sort of cockpit?

	bmP = &gameData.hoard.icon [gameStates.render.fonts.bHires].bm;
	GrUBitmapM (x, y, bmP);

	x+=bmP->bmProps.w+bmP->bmProps.w/2;
	y+= (gameStates.render.fonts.bHires?2:1);
	if (gameData.app.nGameMode & GM_ENTROPY) {
		char	szInfo [20];
		int	w, h, aw;
		if (LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX] >= extraGameInfo [1].entropy.nCaptureVirusLimit)
			GrSetFontColorRGBi (ORANGE_RGBA, 1, 0, 0);
		else
			GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
		sprintf (szInfo,
			"x %d [%d]",
			LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX],
			LOCALPLAYER.secondaryAmmo [SMARTMINE_INDEX]);
		nIdEntropy [0] = GrPrintF (nIdEntropy, x, y, szInfo);
		if (gameStates.entropy.bConquering) {
			int t = (extraGameInfo [1].entropy.nCaptureTimeLimit * 1000) -
					   (gameStates.app.nSDLTicks - gameStates.entropy.nTimeLastMoved);

			if (t < 0)
				t = 0;
			GrGetStringSize (szInfo, &w, &h, &aw);
			x += w;
			GrSetFontColorRGBi (RED_RGBA, 1, 0, 0);
			sprintf (szInfo, " %d.%d", t / 1000, (t % 1000) / 100);
			nIdEntropy [1] = GrPrintF (nIdEntropy + 1, x, y, szInfo);
			}
		}
	else
		nIdOrbs = GrPrintF (&nIdOrbs, x, y, "x %d", LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX]);
	}
}

//	-----------------------------------------------------------------------------

void HUDShowFlag (void)
{
if (HIDE_HUD)
	return;
if ((gameData.app.nGameMode & GM_CAPTURE) && (LOCALPLAYER.flags & PLAYER_FLAGS_FLAG)) {
	int x = 0, y = 0, icon;

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
		y = 2*nHUDLineSpacing;
		x = 4*GAME_FONT->ftWidth;
		}
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
		y = nHUDLineSpacing;
		x = GAME_FONT->ftWidth;
		}
	else if ((gameStates.render.cockpit.nMode == CM_FULL_SCREEN) ||
				 (gameStates.render.cockpit.nMode == CM_LETTERBOX)) {
		y = 5*nHUDLineSpacing;
		x = GAME_FONT->ftWidth;
		if (gameStates.render.fonts.bHires)
			y += nHUDLineSpacing;
		}
	else
		Int3 ();		//what sort of cockpit?
	icon = (GetTeam (gameData.multiplayer.nLocalPlayer) == TEAM_BLUE) ? FLAG_ICON_RED : FLAG_ICON_BLUE;
	PAGE_IN_GAUGE (icon);
	GrUBitmapM (x, y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (icon));
	}
}

//	-----------------------------------------------------------------------------

inline int HUDShowFlashGauge (int h, int *bFlash, int tToggle)
{
	time_t t = gameStates.app.nSDLTicks;
	int b = *bFlash;

if (gameStates.app.bPlayerIsDead || gameStates.app.bPlayerExploded)
	b = 0;
else if (b == 2) {
	if (h > 20)
		b = 0;
	else if (h > 10)
		b = 1;
	}
else if (b == 1) {
	if (h > 20)
		b = 0;
	else if (h <= 10)
		b = 2;
	}
else {
	if (h <= 10)
		b = 2;
	else if (h <= 20)
		b = 1;
	else
		b = 0;
	tToggle = -1;
	}
*bFlash = b;
return (int) ((b && (tToggle <= t)) ? t + 300 / b : 0);
}

//	-----------------------------------------------------------------------------

void HUDShowEnergy (void)
{
	int h, y;

	static int nIdEnergy = 0;

	//GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
if (HIDE_HUD)
	return;
h = LOCALPLAYER.energy ? X2IR (LOCALPLAYER.energy) : 0;
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - (IsMultiGame ? 5 : 1) * nHUDLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	nIdEnergy = GrPrintF (&nIdEnergy, 2, y, "%s: %i", TXT_ENERGY, h);
	}
else {
	static int		bFlash = 0, bShow = 1;
	static time_t	tToggle;
	time_t			t;
	ubyte				c;

	if ((t = HUDShowFlashGauge (h, &bFlash, (int) tToggle))) {
		tToggle = t;
		bShow = !bShow;
		}
	y = grdCurCanv->cv_h - (int) (((IsMultiGame ? 5 : 1) * nHUDLineSpacing - 1) * yScale);
	GrSetColorRGB (255, 255, (ubyte) ((h > 100) ? 255 : 0), 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	if (bFlash) {
		if (!bShow)
			goto skipGauge;
		h = 100;
		}
	else
		bShow = 1;
	c = (h > 100) ? 224 : 224;
	GrSetColorRGB (c, c, (ubyte) ((h > 100) ? c : 0), 128);
	GrURect (6, y, 6 + (int) (((h > 100) ? h - 100 : h) * xScale), y + (int) (9 * yScale));
	}

skipGauge:

if (gameData.demo.nState == ND_STATE_RECORDING) {
	int energy = X2IR (LOCALPLAYER.energy);

	if (energy != oldEnergy [gameStates.render.vr.nCurrentPage]) {
		NDRecordPlayerEnergy (oldEnergy [gameStates.render.vr.nCurrentPage], energy);
		oldEnergy [gameStates.render.vr.nCurrentPage] = energy;
	 	}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowAfterburner (void)
{
	int h, y;

	static int nIdAfterBurner = 0;
if (HIDE_HUD)
	return;
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER))
	return;		//don't draw if don't have
h = FixMul (gameData.physics.xAfterburnerCharge, 100);
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - ((gameData.app.nGameMode & GM_MULTI) ? 8 : 3) * nHUDLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	nIdAfterBurner = GrPrintF (&nIdAfterBurner, 2, y, TXT_HUD_BURN, h);
	}
else {
	y = grdCurCanv->cv_h - (int) ((((gameData.app.nGameMode & GM_MULTI) ? 8 : 3) * nHUDLineSpacing - 1) * yScale);
	GrSetColorRGB (255, 0, 0, 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	GrSetColorRGB (224, 0, 0, 128);
	GrURect (6, y, 6 + (int) (h * xScale), y + (int) (9 * yScale));
	}
if (gameData.demo.nState==ND_STATE_RECORDING) {
	if (gameData.physics.xAfterburnerCharge != oldAfterburner [gameStates.render.vr.nCurrentPage]) {
		NDRecordPlayerAfterburner (oldAfterburner [gameStates.render.vr.nCurrentPage], gameData.physics.xAfterburnerCharge);
		oldAfterburner [gameStates.render.vr.nCurrentPage] = gameData.physics.xAfterburnerCharge;
	 	}
	}
}
#if 0
char *d2_very_shortSecondary_weapon_names [] =
		{"Flash", "Guided", "SmrtMine", "Mercury", "Shaker"};
#endif
#define SECONDARY_WEAPON_NAMES_VERY_SHORT(nWeaponNum) 				\
	 ((nWeaponNum <= MEGA_INDEX)?GAMETEXT (541 + nWeaponNum):	\
	 GT (636+nWeaponNum-FLASHMSL_INDEX))

//return which bomb will be dropped next time the bomb key is pressed
extern int ArmedBomb ();

//	-----------------------------------------------------------------------------

void ShowBombCount (int x, int y, int bg_color, int bShowAlways)
{
	int bomb, count, countx;
	char txt [5], *t;

	static int nIdBombCount = 0;

if (gameData.app.nGameMode & GM_ENTROPY)
	return;
bomb = ArmedBomb ();
if (!bomb)
	return;
if ((gameData.app.nGameMode & GM_HOARD) && (bomb == PROXMINE_INDEX))
	return;
count = LOCALPLAYER.secondaryAmmo [bomb];
#if DBG
count = min (count, 99);	//only have room for 2 digits - cheating give 200
#endif
countx = (bomb == PROXMINE_INDEX) ? count : -count;
if (bShowAlways && count == 0)		//no bombs, draw nothing on HUD
	return;
if (!bShowAlways && countx == oldBombcount [gameStates.render.vr.nCurrentPage])
	return;
// I hate doing this off of hard coded coords!!!!
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {		//draw background
	GrSetColorRGBi (bg_color);
	if (!gameStates.video.nDisplayMode) {
		HUDRect (169, 189, 189, 196);
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrScanLine (168, 189, 189);
		}
	else {
		GrRect (HUD_SCALE_X (338), HUD_SCALE_Y (453), HUD_SCALE_X (378), HUD_SCALE_Y (470));
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrScanLine (HUD_SCALE_X (336), HUD_SCALE_X (378), HUD_SCALE_Y (453));
		}
	}
if (count)
	GrSetFontColorRGBi (
		(bomb == PROXMINE_INDEX) ? RGBA_PAL2 (55, 0, 0) : RGBA_PAL2 (59, 59, 21), 1,
		bg_color, bg_color != -1);
else if (bg_color != -1)
	GrSetFontColorRGBi (bg_color, 1, bg_color, 1);	//erase by drawing in background color
sprintf (txt, "B:%02d", count);
while ((t = strchr (txt, '1')))
	*t = '\x84';	//convert to wide '1'
nIdBombCount = SHOW_COCKPIT ? HUDPrintF (&nIdBombCount, x, y, txt, nIdBombCount) : GrString (x, y, txt, &nIdBombCount);
oldBombcount [gameStates.render.vr.nCurrentPage] = countx;
}

//	-----------------------------------------------------------------------------

void DrawPrimaryAmmoInfo (int ammoCount)
{
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBDrawPrimaryAmmoInfo (ammoCount);
else
	DrawAmmoInfo (PRIMARY_AMMO_X, PRIMARY_AMMO_Y, ammoCount, 1);
}

//	-----------------------------------------------------------------------------

//convert '1' characters to special wide ones
#define convert_1s(s) {char *p=s; while ((p = strchr (p, '1'))) *p = (char)132;}

void HUDShowWeapons (void)
{
	int	w, h, aw;
	int	y;
	const char	*pszWeapon;
	char	szWeapon [32];

	static int nIdWeapons [2] = {0, 0};

if (HIDE_HUD)
	return;
#if 0
HUDShowIcons ();
#endif
//	GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
GrSetCurFont (GAME_FONT);
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
y = grdCurCanv->cv_h;
if (IsMultiGame)
	y -= 4*nHUDLineSpacing;
pszWeapon = PRIMARY_WEAPON_NAMES_SHORT (gameData.weapons.nPrimary);
switch (gameData.weapons.nPrimary) {
	case LASER_INDEX:
		if (LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS)
			sprintf (szWeapon, "%s %s %i", TXT_QUAD, pszWeapon, LOCALPLAYER.laserLevel+1);
		else
			sprintf (szWeapon, "%s %i", pszWeapon, LOCALPLAYER.laserLevel+1);
		break;

	case SUPER_LASER_INDEX:
		Int3 ();
		break;	//no such thing as super laser

	case VULCAN_INDEX:
	case GAUSS_INDEX:
		sprintf (szWeapon, "%s: %i", pszWeapon, X2I ((unsigned) LOCALPLAYER.primaryAmmo [VULCAN_INDEX] * (unsigned) VULCAN_AMMO_SCALE));
		convert_1s (szWeapon);
		break;

	case SPREADFIRE_INDEX:
	case PLASMA_INDEX:
	case FUSION_INDEX:
	case HELIX_INDEX:
	case PHOENIX_INDEX:
		strcpy (szWeapon, pszWeapon);
		break;

	case OMEGA_INDEX:
		sprintf (szWeapon, "%s: %03i", pszWeapon, gameData.omega.xCharge [IsMultiGame] * 100 / MAX_OMEGA_CHARGE);
		convert_1s (szWeapon);
		break;

	default:
		Int3 ();
		szWeapon [0] = 0;
		break;
	}

GrGetStringSize (szWeapon, &w, &h, &aw);
nIdWeapons [0] = GrPrintF (nIdWeapons + 0, grdCurCanv->cvBitmap.bmProps.w - 5 - w, y-2*nHUDLineSpacing, szWeapon);

if (gameData.weapons.nPrimary == VULCAN_INDEX) {
	if (LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary] != oldAmmoCount [0][gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordPrimaryAmmo (oldAmmoCount [0][gameStates.render.vr.nCurrentPage], LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary]);
		oldAmmoCount [0][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary];
		}
	}

if (gameData.weapons.nPrimary == OMEGA_INDEX) {
	if (gameData.omega.xCharge [IsMultiGame] != xOldOmegaCharge [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordPrimaryAmmo (xOldOmegaCharge [gameStates.render.vr.nCurrentPage], gameData.omega.xCharge [IsMultiGame]);
		xOldOmegaCharge [gameStates.render.vr.nCurrentPage] = gameData.omega.xCharge [IsMultiGame];
		}
	}

pszWeapon = SECONDARY_WEAPON_NAMES_VERY_SHORT (gameData.weapons.nSecondary);
sprintf (szWeapon, "%s %d", pszWeapon, LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
GrGetStringSize (szWeapon, &w, &h, &aw);
nIdWeapons [1] = GrPrintF (nIdWeapons + 1, grdCurCanv->cvBitmap.bmProps.w-5-w, y-nHUDLineSpacing, szWeapon);

if (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary] != oldAmmoCount [1][gameStates.render.vr.nCurrentPage]) {
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordSecondaryAmmo (oldAmmoCount [1][gameStates.render.vr.nCurrentPage], LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
	oldAmmoCount [1][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary];
	}
ShowBombCount (grdCurCanv->cvBitmap.bmProps.w- (3*GAME_FONT->ftWidth+ (gameStates.render.fonts.bHires?0:2)), y-3*nHUDLineSpacing, -1, 1);
}

//	-----------------------------------------------------------------------------

void HUDShowCloakInvul (void)
{

	static int nIdCloak = 0, nIdInvul = 0;

if (HIDE_HUD)
	return;
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);

if (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) {
	int	y = grdCurCanv->cv_h;

	if (IsMultiGame)
		y -= 7*nHUDLineSpacing;
	else
		y -= 4*nHUDLineSpacing;
	if ((LOCALPLAYER.cloakTime+CLOAK_TIME_MAX - gameData.time.xGame > F1_0*3) || (gameData.time.xGame & 0x8000))
		nIdCloak = GrPrintF (&nIdCloak, 2, y, "%s", TXT_CLOAKED);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
	int	y = grdCurCanv->cv_h;

	if (IsMultiGame)
		y -= 10*nHUDLineSpacing;
	else
		y -= 5*nHUDLineSpacing;
	if (((LOCALPLAYER.invulnerableTime + INVULNERABLE_TIME_MAX - gameData.time.xGame) > F1_0*4) || (gameData.time.xGame & 0x8000))
		nIdInvul = GrPrintF (&nIdInvul, 2, y, "%s", TXT_INVULNERABLE);
	}
}

//	-----------------------------------------------------------------------------

void HUDShowShield (void)
{
	static int		bShow = 1;
	static time_t	tToggle = 0, nBeep = -1;
	time_t			t = gameStates.app.nSDLTicks;
	int				bLastFlash = gameStates.render.cockpit.nShieldFlash;
	int				h, y;

	static int nIdShield = 0;

if (HIDE_HUD)
	return;
//	GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
h = (LOCALPLAYER.shields >= 0) ? X2IR (LOCALPLAYER.shields) : 0;
if ((t = HUDShowFlashGauge (h, &gameStates.render.cockpit.nShieldFlash, (int) tToggle))) {
	tToggle = t;
	bShow = !bShow;
	}
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - (IsMultiGame ? 6 : 2) * nHUDLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	nIdShield = GrPrintF (&nIdShield, 2, y, "%s: %i", TXT_SHIELD, h);
	}
else {
	y = grdCurCanv->cv_h - (int) (((IsMultiGame ? 6 : 2) * nHUDLineSpacing - 1) * yScale);
	GrSetColorRGB (0, (ubyte) ((h > 100) ? 255 : 64), 255, 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	if (bShow) {
		GrSetColorRGB (0, (ubyte) ((h > 100) ? 224 : 64), 224, 128);
		GrURect (6, y, 6 + (int) (((h > 100) ? h - 100 : h) * xScale), y + (int) (9 * yScale));
		}
	}
if (gameStates.render.cockpit.nShieldFlash) {
	if (gameOpts->gameplay.bShieldWarning && gameOpts->sound.bUseSDLMixer) {
		if ((nBeep < 0) || (bLastFlash != gameStates.render.cockpit.nShieldFlash)) {
			if (nBeep >= 0)
				DigiStopSound ((int) nBeep);
			nBeep = DigiStartSound (-1, F1_0 * 2 / 3, 0xFFFF / 2, -1, -1, -1, -1, F1_0,
											AddonSoundName ((gameStates.render.cockpit.nShieldFlash == 1) ?
											SND_ADDON_LOW_SHIELDS1 : SND_ADDON_LOW_SHIELDS2), NULL, 0);
			}
		}
	else if (nBeep >= 0) {
		DigiStopSound ((int) nBeep);
		nBeep = -1;
		}
	if (!bShow)
		goto skipGauge;
	h = 100;
	}
else {
	bShow = 1;
	if (nBeep >= 0) {
		DigiStopSound ((int) nBeep);
		nBeep = -1;
		}
	}

skipGauge:

if (gameData.demo.nState==ND_STATE_RECORDING) {
	int shields = X2IR (LOCALPLAYER.shields);

	if (shields != oldShields [gameStates.render.vr.nCurrentPage]) {		// Draw the shield gauge
		NDRecordPlayerShields (oldShields [gameStates.render.vr.nCurrentPage], shields);
		oldShields [gameStates.render.vr.nCurrentPage] = shields;
		}
	}
}

//	-----------------------------------------------------------------------------

//draw the icons for number of lives
void HUDShowLives ()
{
	static int nIdLives = 0;

if (HIDE_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) && (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;
if (IsMultiGame) {
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	nIdLives = GrPrintF (&nIdLives, 10, 3, "%s: %d", TXT_DEATHS, LOCALPLAYER.netKilledTotal);
	}
else if (LOCALPLAYER.lives > 1)  {
	grsBitmap *bmP;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
	PAGE_IN_GAUGE (GAUGE_LIVES);
	bmP = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_LIVES);
	GrUBitmapM (10, 3, bmP);
	nIdLives = GrPrintF (&nIdLives, 10 + bmP->bmProps.w + bmP->bmProps.w / 2, 4, "x %d", LOCALPLAYER.lives - 1);
	}
}

//	-----------------------------------------------------------------------------

void ShowTime (void)
{
if (gameStates.render.bShowTime) {
		int secs = X2I (LOCALPLAYER.timeLevel) % 60;
		int mins = X2I (LOCALPLAYER.timeLevel) / 60;

		static int nIdTime = 0;

	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	nIdTime = GrPrintF (&nIdTime, grdCurCanv->cv_w - 4 * GAME_FONT->ftWidth,
							  grdCurCanv->cv_h - 4 * nHUDLineSpacing,
							  "%d:%02d", mins, secs);
	}
}

//	-----------------------------------------------------------------------------

#define EXTRA_SHIP_SCORE	50000		//get new ship every this many points

void AddPointsToScore (int points)
{
	int prev_score;

	scoreTime += f1_0*2;
	scoreDisplay [0] += points;
	scoreDisplay [1] += points;
	if (scoreTime > f1_0*4) scoreTime = f1_0*4;
	if (points == 0 || gameStates.app.cheats.bEnabled)
		return;
	if (IsMultiGame && !IsCoopGame)
		return;
	prev_score=LOCALPLAYER.score;
	LOCALPLAYER.score += points;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordPlayerScore (points);
	if (gameData.app.nGameMode & GM_MULTI_COOP)
		MultiSendScore ();
	if (IsMultiGame)
		return;
	if (LOCALPLAYER.score/EXTRA_SHIP_SCORE != prev_score/EXTRA_SHIP_SCORE) {
		short snd;
		LOCALPLAYER.lives += LOCALPLAYER.score/EXTRA_SHIP_SCORE - prev_score/EXTRA_SHIP_SCORE;
		PowerupBasic (20, 20, 20, 0, TXT_EXTRA_LIFE);
		if ((snd=gameData.objs.pwrUp.info [POW_EXTRA_LIFE].hitSound) > -1)
			DigiPlaySample (snd, F1_0);
	}
}

//	-----------------------------------------------------------------------------

void AddBonusPointsToScore (int points)
{
	int prev_score;

	if (points == 0 || gameStates.app.cheats.bEnabled)
		return;

	prev_score=LOCALPLAYER.score;

	LOCALPLAYER.score += points;


	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordPlayerScore (points);

	if (IsMultiGame)
		return;

	if (LOCALPLAYER.score/EXTRA_SHIP_SCORE != prev_score/EXTRA_SHIP_SCORE) {
		short snd;
		LOCALPLAYER.lives += LOCALPLAYER.score/EXTRA_SHIP_SCORE - prev_score/EXTRA_SHIP_SCORE;
		if ((snd=gameData.objs.pwrUp.info [POW_EXTRA_LIFE].hitSound) > -1)
			DigiPlaySample (snd, F1_0);
	}
}

//	-----------------------------------------------------------------------------

#include "key.h"

void InitGaugeCanvases ()
{
if (!bHaveGaugeCanvases && gamePalette) {
	PAGE_IN_GAUGE (SB_GAUGE_ENERGY);
	PAGE_IN_GAUGE (GAUGE_AFTERBURNER);
	Canv_LeftEnergyGauge = GrCreateCanvas (LEFT_ENERGY_GAUGE_W, LEFT_ENERGY_GAUGE_H);
	Canv_RightEnergyGauge = GrCreateCanvas (RIGHT_ENERGY_GAUGE_W, RIGHT_ENERGY_GAUGE_H);
	Canv_NumericalGauge = GrCreateCanvas (NUMERICAL_GAUGE_W, NUMERICAL_GAUGE_H);
	Canv_AfterburnerGauge = GrCreateCanvas (AFTERBURNER_GAUGE_W, AFTERBURNER_GAUGE_H);
	SBInitGaugeCanvases ();
	bHaveGaugeCanvases = 1;
	}
}

//	-----------------------------------------------------------------------------

void CloseGaugeCanvases (void)
{
if (bHaveGaugeCanvases) {
	GrFreeCanvas (Canv_LeftEnergyGauge);
	GrFreeCanvas (Canv_RightEnergyGauge);
	GrFreeCanvas (Canv_NumericalGauge);
	GrFreeCanvas (Canv_AfterburnerGauge);
	SBCloseGaugeCanvases ();
	bHaveGaugeCanvases = 0;
	}
}

//	-----------------------------------------------------------------------------

void InitGauges (void)
{
	int i;

	//draw_gauges_on 	= 1;

for (i=0; i<2; i++)	{
	if ((IsMultiGame && !IsCoopGame) || ((gameData.demo.nState == ND_STATE_PLAYBACK) && (gameData.demo.nGameMode & GM_MULTI) && !(gameData.demo.nGameMode & GM_MULTI_COOP)))
		oldScore [i] = -99;
	else
		oldScore [i]		= -1;
	oldEnergy [i]			= -1;
	oldShields [i]		= -1;
	oldFlags [i]			= -1;
	bOldCloak [i]			= -1;
	oldLives [i]			= -1;
	oldAfterburner [i]	= -1;
	oldBombcount [i]		= 0;
	oldLaserLevel [i]	= 0;

	oldWeapon [0][i] = oldWeapon [1][i] = -1;
	oldAmmoCount [0][i] = oldAmmoCount [1][i] = -1;
	xOldOmegaCharge [i] = -1;
	}
nCloakFadeState = 0;
weaponBoxUser [0] = weaponBoxUser [1] = WBU_WEAPON;
}

//	-----------------------------------------------------------------------------

void DrawEnergyBar (int energy)
{
	grsBitmap *bmP;
	int energy0;
	int x1, x2, y, yMax, i;
	int h0 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_H - 1);
	int h1 = HUD_SCALE_Y (LEFT_ENERGY_GAUGE_H / 4);
	int h2 = HUD_SCALE_Y ((LEFT_ENERGY_GAUGE_H * 3) / 4);
	int w1 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 1);
	int w2 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 2);
	int w3 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 3);
	double eBarScale = (100.0 - (double) energy) * cmScaleX * 0.15 / (double) HUD_SCALE_Y (LEFT_ENERGY_GAUGE_H);

// Draw left energy bar
PAGE_IN_GAUGE (GAUGE_ENERGY_LEFT);
bmP = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_ENERGY_LEFT);
HUDBitBlt (LEFT_ENERGY_GAUGE_X, LEFT_ENERGY_GAUGE_Y, bmP, F1_0, 0);
#if DBG
	GrSetColorRGBi (RGBA_PAL (255, 255, 255));
#else
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
#endif
	//energy0 = (gameStates.video.nDisplayMode ? 125 - (energy * 125) / 100 : 61 - (energy * 61) / 100);
	energy0 = HUD_SCALE_X (112);
	energy0 = energy0 - (energy * energy0) / 100;
	//energy0 = HUD_SCALE_X (energy0);
	if (energy < 100) {
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
		for (i = 0; i < LEFT_ENERGY_GAUGE_H; i++) {
			yMax = HUD_SCALE_Y (i + 1);
			for (y = i; y <= yMax; y++) {
				x1 = h0 - y;
				x2 = x1 + energy0 + (int) ((double) y * eBarScale);
				if (y < h1) {
					if (x2 > w1)
						x2 = w1;
					}
				else if (y < h2) {
					if (x2 > w2)
						x2 = w2;
					}
				else {
					if (x2 > w3)
						x2 = w3;
					}
				if (x2 > x1)
					gr_uscanline (
						HUD_SCALE_X (LEFT_ENERGY_GAUGE_X) + x1,
						HUD_SCALE_X (LEFT_ENERGY_GAUGE_X) + x2,
						HUD_SCALE_Y (LEFT_ENERGY_GAUGE_Y) + y);
				}
			}
		}
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	// Draw right energy bar
//	GrSetCurrentCanvas (Canv_RightEnergyGauge);
	PAGE_IN_GAUGE (GAUGE_ENERGY_RIGHT);
	bmP = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_ENERGY_RIGHT);
//	GrUBitmapM (0, 0, bmP);
//	GrUBitmapM (RIGHT_ENERGY_GAUGE_X, RIGHT_ENERGY_GAUGE_Y, bmP);
	HUDBitBlt (RIGHT_ENERGY_GAUGE_X, RIGHT_ENERGY_GAUGE_Y, bmP, F1_0, 0);
#if DBG
	GrSetColorRGBi (RGBA_PAL (255, 255, 255));
#else
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
#endif
	h0 = HUD_SCALE_X (RIGHT_ENERGY_GAUGE_W - RIGHT_ENERGY_GAUGE_H);
	w1 = HUD_SCALE_X (1);
	w2 = HUD_SCALE_X (2);
	if (energy < 100) {
		yMax = HUD_SCALE_Y (RIGHT_ENERGY_GAUGE_H);
		for (i = 0; i < RIGHT_ENERGY_GAUGE_H; i++) {
			yMax = HUD_SCALE_Y (i + 1);
			for (y = i; y <= yMax; y++) {
				x2 = h0 + y;
				x1 = x2 - energy0 - (int) ((double) y * eBarScale);
				if (y < h1) {
					if (x1 < 0)
						x1 = 0;
					}
				else if (y < h2) {
					if (x1 < w1)
						x1 = w1;
					}
				else {
					if (x1 < w2)
						x1 = w2;
					}
				if (x2 > x1)
					gr_uscanline (
						HUD_SCALE_X (RIGHT_ENERGY_GAUGE_X) + x1,
						HUD_SCALE_X (RIGHT_ENERGY_GAUGE_X) + x2,
						HUD_SCALE_Y (RIGHT_ENERGY_GAUGE_Y) + y);
				}
			}
		}
GrSetCurrentCanvas (GetCurrentGameScreen ());
}

//	-----------------------------------------------------------------------------

ubyte afterburner_bar_table [AFTERBURNER_GAUGE_H_L*2] = {
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			2, 11,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			1, 10,
			1, 10,
			1, 10,
			1, 9,
			1, 9,
			1, 9,
			1, 9,
			0, 9,
			0, 9,
			0, 8,
			0, 8,
			0, 8,
			0, 8,
			1, 8,
			2, 8,
			3, 8,
			4, 8,
			5, 8,
			6, 7,
};

ubyte afterburner_bar_table_hires [AFTERBURNER_GAUGE_H_H*2] = {
	5, 20,
	5, 20,
	5, 19,
	5, 19,
	5, 19,
	5, 19,
	4, 19,
	4, 19,
	4, 19,
	4, 19,

	4, 19,
	4, 18,
	4, 18,
	4, 18,
	4, 18,
	3, 18,
	3, 18,
	3, 18,
	3, 18,
	3, 18,

	3, 18,
	3, 17,
	3, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,

	2, 17,
	2, 16,
	2, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,

	1, 16,
	1, 15,
	1, 15,
	1, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,

	0, 14,
	0, 14,
	0, 14,
	1, 14,
	2, 14,
	3, 14,
	4, 14,
	5, 14,
	6, 13,
	7, 13,

	8, 13,
	9, 13,
	10, 13,
	11, 13,
	12, 13
};


//	-----------------------------------------------------------------------------

void DrawAfterburnerBar (int afterburner)
{
	int not_afterburner;
	int i, j, y, yMax;
	grsBitmap *bmP;
	ubyte *pabt = gameStates.video.nDisplayMode ? afterburner_bar_table_hires : afterburner_bar_table;

// Draw afterburner bar
//GrSetCurrentCanvas (Canv_AfterburnerGauge);
PAGE_IN_GAUGE (GAUGE_AFTERBURNER);
bmP =  gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_AFTERBURNER);
//	GrUBitmapM (0, 0, bmP);
//	GrUBitmapM (AFTERBURNER_GAUGE_X, AFTERBURNER_GAUGE_Y, bmP);
HUDBitBlt (AFTERBURNER_GAUGE_X, AFTERBURNER_GAUGE_Y, bmP, F1_0, 0);
GrSetColorRGB (0, 0, 0, 255);
not_afterburner = FixMul (f1_0 - afterburner, AFTERBURNER_GAUGE_H);
gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
yMax = HUD_SCALE_Y (not_afterburner);
for (y = 0; y < not_afterburner; y++) {
	for (i = HUD_SCALE_Y (y), j = HUD_SCALE_Y (y + 1); i < j; i++) {
		gr_uscanline (
			HUD_SCALE_X (AFTERBURNER_GAUGE_X + pabt [y * 2]),
			HUD_SCALE_X (AFTERBURNER_GAUGE_X + pabt [y * 2 + 1] + 1),
			HUD_SCALE_Y (AFTERBURNER_GAUGE_Y) + i);
		}
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
}

//	-----------------------------------------------------------------------------

void DrawShieldBar (int shield)
{
	int bm_num = shield>=100?9: (shield / 10);

PAGE_IN_GAUGE (GAUGE_SHIELDS+9-bm_num	);
HUDBitBlt (SHIELD_GAUGE_X, SHIELD_GAUGE_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIELDS+9-bm_num), F1_0, 0);
}

#define CLOAK_FADE_WAIT_TIME  0x400

//	-----------------------------------------------------------------------------

void DrawPlayerShip (int nCloakState, int nOldCloakState, int x, int y)
{
	static fix xCloakFadeTimer = 0;
	static int nCloakFadeValue = GR_ACTUAL_FADE_LEVELS - 1;
	static int refade = 0;
	grsBitmap *bmP = NULL;

if (IsTeamGame) {
	PAGE_IN_GAUGE (GAUGE_SHIPS + GetTeam (gameData.multiplayer.nLocalPlayer));
	bmP = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIPS + GetTeam (gameData.multiplayer.nLocalPlayer));
	}
else {
	PAGE_IN_GAUGE (GAUGE_SHIPS + gameData.multiplayer.nLocalPlayer);
	bmP = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIPS + gameData.multiplayer.nLocalPlayer);
	}
if ((nOldCloakState == -1) && nCloakState)
	nCloakFadeValue = 0;
if (!nCloakState) {
	nCloakFadeValue = GR_ACTUAL_FADE_LEVELS - 1;
	nCloakFadeState = 0;
	}
if ((nCloakState == 1) && !nOldCloakState)
	nCloakFadeState = -1;
if (nCloakState == nOldCloakState)		//doing "about-to-uncloak" effect
	if (!nCloakFadeState)
		nCloakFadeState = 2;
if (nCloakState && (gameData.time.xGame > LOCALPLAYER.cloakTime + CLOAK_TIME_MAX - I2X (3)))		//doing "about-to-uncloak" effect
	if (!nCloakFadeState)
		nCloakFadeState = 2;
if (nCloakFadeState)
	xCloakFadeTimer -= gameData.time.xFrame;
while (nCloakFadeState && (xCloakFadeTimer < 0)) {
	xCloakFadeTimer += CLOAK_FADE_WAIT_TIME;
	nCloakFadeValue += nCloakFadeState;
	if (nCloakFadeValue >= GR_ACTUAL_FADE_LEVELS - 1) {
		nCloakFadeValue = GR_ACTUAL_FADE_LEVELS - 1;
		if (nCloakFadeState == 2 && nCloakState)
			nCloakFadeState = -2;
		else
			nCloakFadeState = 0;
		}
	else if (nCloakFadeValue <= 0) {
		nCloakFadeValue = 0;
		if (nCloakFadeState == -2)
			nCloakFadeState = 2;
		else
			nCloakFadeState = 0;
		}
	}

//	To fade out both pages in a paged mode.
if (refade)
	refade = 0;
else if (nCloakState && nOldCloakState && !nCloakFadeState && !refade) {
	nCloakFadeState = -1;
	refade = 1;
	}
if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
	GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
	}
//if (nCloakFadeValue >= GR_ACTUAL_FADE_LEVELS - 1)
	{
	gameStates.render.grAlpha = (float) nCloakFadeValue / (float) GR_ACTUAL_FADE_LEVELS;
	HUDBitBlt (x, y, bmP, F1_0, 0);
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
//		gameStates.render.grAlpha = nCloakFadeValue;
//		GrRect (x, y, x+bmP->bmProps.w-1, y+bmP->bmProps.h-1);
//		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT)
	GrSetCurrentCanvas (GetCurrentGameScreen ());
#if 0
GrBmUBitBltM (
	(int) (bmP->bmProps.w * cmScaleX), (int) (bmP->bmProps.h * cmScaleY),
	(int) (x * cmScaleX), (int) (y * cmScaleY), x, y,
	&gameStates.render.vr.buffers.render [0].cvBitmap, &grdCurCanv->cvBitmap, 1);
#endif
}

#define INV_FRAME_TIME	 (f1_0/10)		//how long for each frame

//	-----------------------------------------------------------------------------

inline int NumDispX (int val)
{
int x = ((val > 99) ? 7 : (val > 9) ? 11 : 15);
if (!gameStates.video.nDisplayMode)
	x /= 2;
return x + NUMERICAL_GAUGE_X;
}

void DrawNumericalDisplay (int shield, int energy)
{
	int dx = NUMERICAL_GAUGE_X,
		 dy = NUMERICAL_GAUGE_Y;

	static int nIdShields = 0, nIdEnergy = 0;

	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT)
		GrSetCurrentCanvas (Canv_NumericalGauge);
	PAGE_IN_GAUGE (GAUGE_NUMERICAL);
//	gr_ubitmap (dx, dy, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_NUMERICAL));
	HUDBitBlt (dx, dy, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_NUMERICAL), F1_0, 0);
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (RGBA_PAL2 (14, 14, 23), 1, 0, 0);
	nIdShields = HUDPrintF (&nIdShields, NumDispX (shield), dy + (gameStates.video.nDisplayMode ? 36 : 16), "%d", shield);
	GrSetFontColorRGBi (RGBA_PAL2 (25, 18, 6), 1, 0, 0);
	nIdEnergy = HUDPrintF (&nIdEnergy, NumDispX (energy), dy + (gameStates.video.nDisplayMode ? 5 : 2), "%d", energy);

	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
		GrSetCurrentCanvas (GetCurrentGameScreen ());
		HUDBitBlt (NUMERICAL_GAUGE_X, NUMERICAL_GAUGE_Y, &Canv_NumericalGauge->cvBitmap, F1_0, 0);
	}
}


//	-----------------------------------------------------------------------------

void DrawKeys ()
{
GrSetCurrentCanvas (GetCurrentGameScreen ());

if (LOCALPLAYER.flags & PLAYER_FLAGS_BLUE_KEY)	{
	PAGE_IN_GAUGE (GAUGE_BLUE_KEY);
	HUDBitBlt (GAUGE_BLUE_KEY_X, GAUGE_BLUE_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_BLUE_KEY), F1_0, 0);
	}
else {
	PAGE_IN_GAUGE (GAUGE_BLUE_KEY_OFF);
	HUDBitBlt (GAUGE_BLUE_KEY_X, GAUGE_BLUE_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_BLUE_KEY_OFF), F1_0, 0);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_GOLD_KEY)	{
	PAGE_IN_GAUGE (GAUGE_GOLD_KEY);
	HUDBitBlt (GAUGE_GOLD_KEY_X, GAUGE_GOLD_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_GOLD_KEY), F1_0, 0);
	}
else {
	PAGE_IN_GAUGE (GAUGE_GOLD_KEY_OFF);
	HUDBitBlt (GAUGE_GOLD_KEY_X, GAUGE_GOLD_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_GOLD_KEY_OFF), F1_0, 0);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_RED_KEY)	{
	PAGE_IN_GAUGE (GAUGE_RED_KEY);
	HUDBitBlt (GAUGE_RED_KEY_X,  GAUGE_RED_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_RED_KEY), F1_0, 0);
	}
else {
	PAGE_IN_GAUGE (GAUGE_RED_KEY_OFF);
	HUDBitBlt (GAUGE_RED_KEY_X,  GAUGE_RED_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_RED_KEY_OFF), F1_0, 0);
	}
}

//	-----------------------------------------------------------------------------

void DrawWeaponInfoSub (int info_index, tGaugeBox *box, int pic_x, int pic_y, const char *pszName, int text_x,
								int text_y, int orient)
{
	grsBitmap	*bmP;
	char			szName [100], *p;
	int			l;

	static int nIdWeapon [3] = {0, 0, 0}, nIdLaser [2] = {0, 0};

	//clear the window
#if 0
	if (gameStates.render.cockpit.nMode != CM_FULL_SCREEN) {
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrRect ((int) (box->left * cmScaleX), (int) (box->top * cmScaleY),
				  (int) (box->right * cmScaleX), (int) (box->bot * cmScaleY));
		}
#endif
	if ((gameData.pig.tex.nHamFileVersion >= 3) && gameStates.video.nDisplayMode) {
		bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [info_index].hires_picture.index;
		PIGGY_PAGE_IN (gameData.weapons.info [info_index].hires_picture.index, 0);
		}
	else {
		bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [info_index].picture.index;
		PIGGY_PAGE_IN (gameData.weapons.info [info_index].picture.index, 0);
		}
	Assert (bmP != NULL);

	HUDBitBlt (pic_x, pic_y, bmP, (gameStates.render.cockpit.nMode == CM_FULL_SCREEN) ? 2 * F1_0 : F1_0, orient);
	if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		return;
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	if ((p = (char *) strchr (pszName, '\n'))) {
		memcpy (szName, pszName, l = p - pszName);
		szName [l + 1] = '\0';
		nIdWeapon [0] = HUDPrintF (&nIdWeapon [0], text_x, text_y, szName);
		nIdWeapon [1] = HUDPrintF (&nIdWeapon [1], text_x, text_y + grdCurCanv->cvFont->ftHeight + 1, p + 1);
		}
	else {
		nIdWeapon [2] = HUDPrintF (&nIdWeapon [2], text_x, text_y, pszName);
	}

	//	For laser, show level and quadness
	if (info_index == LASER_ID || info_index == SUPERLASER_ID) {
		sprintf (szName, "%s: 0", TXT_LVL);
		szName [5] = LOCALPLAYER.laserLevel + 1 + '0';
		nIdLaser [0] = HUDPrintF (&nIdLaser [0], text_x, text_y + nHUDLineSpacing, szName);
		if (LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) {
			strcpy (szName, TXT_QUAD);
			nIdLaser [1] = HUDPrintF (&nIdLaser [1], text_x, text_y + 2 * nHUDLineSpacing, szName);
		}
	}
}

//	-----------------------------------------------------------------------------

void DrawWeaponInfo (int nWeaponType, int nWeaponNum, int laserLevel)
{
	int info_index;

if (nWeaponType == 0) {
	info_index = primaryWeaponToWeaponInfo [nWeaponNum];

	if (info_index == LASER_ID && laserLevel > MAX_LASER_LEVEL)
		info_index = SUPERLASER_ID;

	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + SB_PRIMARY_BOX,
			SB_PRIMARY_W_PIC_X, SB_PRIMARY_W_PIC_Y,
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum),
			SB_PRIMARY_W_TEXT_X, SB_PRIMARY_W_TEXT_Y, 0);
#if 0
	else if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + SB_PRIMARY_BOX,
			SB_PRIMARY_W_PIC_X, SB_PRIMARY_W_PIC_Y,
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum),
			SB_PRIMARY_W_TEXT_X, SB_PRIMARY_W_TEXT_Y, 3);
#endif
	else
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + COCKPIT_PRIMARY_BOX,
			PRIMARY_W_PIC_X, PRIMARY_W_PIC_Y,
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum),
			PRIMARY_W_TEXT_X, PRIMARY_W_TEXT_Y, 0);
		}
else {
	info_index = secondaryWeaponToWeaponInfo [nWeaponNum];

	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + SB_SECONDARY_BOX,
			SB_SECONDARY_W_PIC_X, SB_SECONDARY_W_PIC_Y,
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum),
			SB_SECONDARY_W_TEXT_X, SB_SECONDARY_W_TEXT_Y, 0);
#if 0
	else if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + COCKPIT_SECONDARY_BOX,
			SECONDARY_W_PIC_X, SECONDARY_W_PIC_Y,
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum),
			SECONDARY_W_TEXT_X, SECONDARY_W_TEXT_Y, 1);
#endif
	else
		DrawWeaponInfoSub (info_index,
			gaugeBoxes + COCKPIT_SECONDARY_BOX,
			SECONDARY_W_PIC_X, SECONDARY_W_PIC_Y,
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum),
			SECONDARY_W_TEXT_X, SECONDARY_W_TEXT_Y, 0);
	}
}

//	-----------------------------------------------------------------------------

void DrawAmmoInfo (int x, int y, int ammoCount, int bPrimary)
{
	int w;
	char str [16];

	static int nIdAmmo [2][2] = {{0, 0},{0, 0}};

w = (grdCurCanv->cvFont->ftWidth * (bPrimary ? 7 : 5)) / 2;
GrSetColorRGBi (RGBA_PAL (0, 0, 0));
GrRect (HUD_SCALE_X (x), HUD_SCALE_Y (y), HUD_SCALE_X (x+w), HUD_SCALE_Y (y + grdCurCanv->cvFont->ftHeight));
GrSetFontColorRGBi (RED_RGBA, 1, 0, 0);
sprintf (str, "%03d", ammoCount);
convert_1s (str);
nIdAmmo [bPrimary][0] = HUDPrintF (&nIdAmmo [bPrimary][0], x, y, str);
GrRect (HUD_SCALE_X (x), HUD_SCALE_Y (y), HUD_SCALE_X (x+w), HUD_SCALE_Y (y + grdCurCanv->cvFont->ftHeight));
nIdAmmo [bPrimary][1] = HUDPrintF (&nIdAmmo [bPrimary][1], x, y, str);
}

//	-----------------------------------------------------------------------------

void DrawSecondaryAmmoInfo (int ammoCount)
{
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBDrawSecondaryAmmoInfo (ammoCount);
else
	DrawAmmoInfo (SECONDARY_AMMO_X, SECONDARY_AMMO_Y, ammoCount, 0);
}

//	-----------------------------------------------------------------------------

//returns true if drew picture
int DrawWeaponBox (int nWeaponType, int nWeaponNum)
{
	int bDrew = 0;
	int bLaserLevelChanged;

GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
GrSetCurFont (GAME_FONT);
bLaserLevelChanged = ((nWeaponType == 0) &&
								(nWeaponNum == LASER_INDEX) &&
								((LOCALPLAYER.laserLevel != oldLaserLevel [gameStates.render.vr.nCurrentPage])));

if ((nWeaponNum != oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage] || bLaserLevelChanged) && weaponBoxStates [nWeaponType] == WS_SET) {
	weaponBoxStates [nWeaponType] = WS_FADING_OUT;
	weaponBoxFadeValues [nWeaponType]=I2X (GR_ACTUAL_FADE_LEVELS-1);
	}

if ((oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage] == -1) || 1/* (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)*/) {
	DrawWeaponInfo (nWeaponType, nWeaponNum, LOCALPLAYER.laserLevel);
	oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage] = nWeaponNum;
	oldAmmoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
	xOldOmegaCharge [gameStates.render.vr.nCurrentPage] = -1;
	oldLaserLevel [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
	bDrew = 1;
	weaponBoxStates [nWeaponType] = WS_SET;
	}

if (weaponBoxStates [nWeaponType] == WS_FADING_OUT) {
	DrawWeaponInfo (nWeaponType, oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage], oldLaserLevel [gameStates.render.vr.nCurrentPage]);
	oldAmmoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
	xOldOmegaCharge [gameStates.render.vr.nCurrentPage] = -1;
	bDrew = 1;
	weaponBoxFadeValues [nWeaponType] -= gameData.time.xFrame * FADE_SCALE;
	if (weaponBoxFadeValues [nWeaponType] <= 0) {
		weaponBoxStates [nWeaponType] = WS_FADING_IN;
		oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage] = nWeaponNum;
		oldWeapon [nWeaponType][!gameStates.render.vr.nCurrentPage] = nWeaponNum;
		oldLaserLevel [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
		oldLaserLevel [!gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
		weaponBoxFadeValues [nWeaponType] = 0;
		}
	}
else if (weaponBoxStates [nWeaponType] == WS_FADING_IN) {
	if (nWeaponNum != oldWeapon [nWeaponType][gameStates.render.vr.nCurrentPage]) {
		weaponBoxStates [nWeaponType] = WS_FADING_OUT;
		}
	else {
		DrawWeaponInfo (nWeaponType, nWeaponNum, LOCALPLAYER.laserLevel);
		oldAmmoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
		xOldOmegaCharge [gameStates.render.vr.nCurrentPage] = -1;
		bDrew=1;
		weaponBoxFadeValues [nWeaponType] += gameData.time.xFrame * FADE_SCALE;
		if (weaponBoxFadeValues [nWeaponType] >= I2X (GR_ACTUAL_FADE_LEVELS-1)) {
			weaponBoxStates [nWeaponType] = WS_SET;
			oldWeapon [nWeaponType][!gameStates.render.vr.nCurrentPage] = -1;		//force redraw (at full fade-in) of other page
			}
		}
	}

if (weaponBoxStates [nWeaponType] != WS_SET) {		//fade gauge
	int fadeValue = X2I (weaponBoxFadeValues [nWeaponType]);
	int boxofs = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? SB_PRIMARY_BOX : COCKPIT_PRIMARY_BOX;
	gameStates.render.grAlpha = (float) fadeValue;
	GrRect (gaugeBoxes [boxofs + nWeaponType].left,
			  gaugeBoxes [boxofs + nWeaponType].top,
			  gaugeBoxes [boxofs + nWeaponType].right,
			  gaugeBoxes [boxofs + nWeaponType].bot);
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
return bDrew;
}

//	-----------------------------------------------------------------------------

fix staticTime [2];

void DrawStatic (int win)
{
	tVideoClip *vc = gameData.eff.vClips [0] + VCLIP_MONITOR_STATIC;
	grsBitmap *bmp;
	int framenum;
	int boxofs = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? SB_PRIMARY_BOX : COCKPIT_PRIMARY_BOX;
	int x, y;

	staticTime [win] += gameData.time.xFrame;
	if (staticTime [win] >= vc->xTotalTime) {
		weaponBoxUser [win] = WBU_WEAPON;
		return;
		}
	framenum = staticTime [win] * vc->nFrameCount / vc->xTotalTime;
	PIGGY_PAGE_IN (vc->frames [framenum].index, 0);
	bmp = gameData.pig.tex.bitmaps [0] + vc->frames [framenum].index;
	GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
	for (x=gaugeBoxes [boxofs+win].left;x<gaugeBoxes [boxofs+win].right;x+=bmp->bmProps.w)
		for (y=gaugeBoxes [boxofs+win].top;y<gaugeBoxes [boxofs+win].bot;y+=bmp->bmProps.h)
			/*GrBitmap*/HUDBitBlt (x, y, bmp, F1_0, 0);
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	CopyGaugeBox (&gaugeBoxes [boxofs+win], &gameStates.render.vr.buffers.render [0].cvBitmap);
}

//	-----------------------------------------------------------------------------

void DrawWeaponBoxes ()
{
	int boxofs = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? SB_PRIMARY_BOX : COCKPIT_PRIMARY_BOX;
	int bDrawn;

if (weaponBoxUser [0] == WBU_WEAPON) {
	bDrawn = DrawWeaponBox (0, gameData.weapons.nPrimary);
	if (bDrawn)
		CopyGaugeBox (gaugeBoxes+boxofs, &gameStates.render.vr.buffers.render [0].cvBitmap);

	if (weaponBoxStates [0] == WS_SET) {
		if ((gameData.weapons.nPrimary == VULCAN_INDEX) || (gameData.weapons.nPrimary == GAUSS_INDEX)) {
			if (LOCALPLAYER.primaryAmmo [VULCAN_INDEX] != oldAmmoCount [0][gameStates.render.vr.nCurrentPage]) {
				if (gameData.demo.nState == ND_STATE_RECORDING)
					NDRecordPrimaryAmmo (oldAmmoCount [0][gameStates.render.vr.nCurrentPage], LOCALPLAYER.primaryAmmo [VULCAN_INDEX]);
				DrawPrimaryAmmoInfo (X2I ((unsigned) VULCAN_AMMO_SCALE * (unsigned) LOCALPLAYER.primaryAmmo [VULCAN_INDEX]));
				oldAmmoCount [0][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.primaryAmmo [VULCAN_INDEX];
				}
			}
		if (gameData.weapons.nPrimary == OMEGA_INDEX) {
			if (gameData.omega.xCharge [IsMultiGame] != xOldOmegaCharge [gameStates.render.vr.nCurrentPage]) {
				if (gameData.demo.nState == ND_STATE_RECORDING)
					NDRecordPrimaryAmmo (xOldOmegaCharge [gameStates.render.vr.nCurrentPage], gameData.omega.xCharge [IsMultiGame]);
				DrawPrimaryAmmoInfo (gameData.omega.xCharge [IsMultiGame] * 100 / MAX_OMEGA_CHARGE);
				xOldOmegaCharge [gameStates.render.vr.nCurrentPage] = gameData.omega.xCharge [IsMultiGame];
				}
			}
		}
	}
else if (weaponBoxUser [0] == WBU_STATIC)
	DrawStatic (0);

if (weaponBoxUser [1] == WBU_WEAPON) {
	bDrawn = DrawWeaponBox (1, gameData.weapons.nSecondary);
	if (bDrawn)
		CopyGaugeBox (&gaugeBoxes [boxofs+1], &gameStates.render.vr.buffers.render [0].cvBitmap);

	if (weaponBoxStates [1] == WS_SET)
		if (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary] != oldAmmoCount [1][gameStates.render.vr.nCurrentPage]) {
			oldBombcount [gameStates.render.vr.nCurrentPage] = 0x7fff;	//force redraw
			if (gameData.demo.nState == ND_STATE_RECORDING)
				NDRecordSecondaryAmmo (oldAmmoCount [1][gameStates.render.vr.nCurrentPage], LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
			DrawSecondaryAmmoInfo (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
			oldAmmoCount [1][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary];
			}
	}
else if (weaponBoxUser [1] == WBU_STATIC)
	DrawStatic (1);
}

//	-----------------------------------------------------------------------------

//	Draws invulnerable ship, or maybe the flashing ship, depending on invulnerability time left.
void DrawInvulnerableShip (void)
{
	static fix time = 0;
	fix tInvul = LOCALPLAYER.invulnerableTime + INVULNERABLE_TIME_MAX - gameData.time.xGame;

GrSetCurrentCanvas (GetCurrentGameScreen ());
if (tInvul > 0) {
	if ((tInvul > F1_0 * 4) || (gameData.time.xGame & 0x8000)) {
		if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)	{
			PAGE_IN_GAUGE (GAUGE_INVULNERABLE + nInvulnerableFrame);
			HUDBitBlt (SB_SHIELD_GAUGE_X, SB_SHIELD_GAUGE_Y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (GAUGE_INVULNERABLE + nInvulnerableFrame)], F1_0, 0);
			}
		else {
			PAGE_IN_GAUGE (GAUGE_INVULNERABLE + nInvulnerableFrame);
			HUDBitBlt (SHIELD_GAUGE_X, SHIELD_GAUGE_Y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (GAUGE_INVULNERABLE + nInvulnerableFrame)], F1_0, 0);
			}
		time += gameData.time.xFrame;
		while (time > INV_FRAME_TIME) {
			time -= INV_FRAME_TIME;
			if (++nInvulnerableFrame == N_INVULNERABLE_FRAMES)
				nInvulnerableFrame = 0;
			}
		}
	}
else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBDrawShieldBar (X2IR (LOCALPLAYER.shields));
else
	DrawShieldBar (X2IR (LOCALPLAYER.shields));
}

//	-----------------------------------------------------------------------------

rgb playerColors [] = {
	{15, 15, 23},
	{27, 0, 0},
	{0, 23, 0},
	{30, 11, 31},
	{31, 16, 0},
	{24, 17, 6},
	{14, 21, 12},
	{29, 29, 0}};

typedef struct {
	sbyte x, y;
} xy;

//offsets for reticle parts: high-big  high-sml  low-big  low-sml
xy cross_offsets [4] = 		{{-8, -5}, 	{-4, -2}, 	{-4, -2}, {-2, -1}};
xy primary_offsets [4] = 	{{-30, 14}, {-16, 6}, 	{-15, 6}, {-8, 2}};
xy secondary_offsets [4] =	{{-24, 2}, 	{-12, 0}, {-12, 1}, {-6, -2}};

void OglDrawMouseIndicator (void);

//draw the reticle
void ShowReticle (int bForceBig)
{
	int x, y;
	int bLaserReady, bMissileReady, bLaserAmmo, bMissileAmmo;
	int nCrossBm, nPrimaryBm, nSecondaryBm;
	int bHiresReticle, bSmallReticle, ofs, nGaugeIndex;

if ((gameData.demo.nState==ND_STATE_PLAYBACK) && gameData.demo.bFlyingGuided) {
	DrawGuidedCrosshair ();
	return;
	}

x = grdCurCanv->cv_w / 2;
y = grdCurCanv->cv_h / ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 : 2);
bLaserReady = AllowedToFireLaser ();
bMissileReady = AllowedToFireMissile (-1, 1);
bLaserAmmo = PlayerHasWeapon (gameData.weapons.nPrimary, 0, -1, 1);
bMissileAmmo = PlayerHasWeapon (gameData.weapons.nSecondary, 1, -1, 1);
nPrimaryBm = bLaserReady && (bLaserAmmo == HAS_ALL);
nSecondaryBm = bMissileReady && (bMissileAmmo == HAS_ALL);
if (nPrimaryBm && (gameData.weapons.nPrimary == LASER_INDEX) &&
	 (LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS))
	nPrimaryBm++;

if (secondaryWeaponToGunNum [gameData.weapons.nSecondary] == 7)
	nSecondaryBm += 3;		//now value is 0, 1 or 3, 4
else if (nSecondaryBm && !(gameData.laser.nMissileGun & 1))
		nSecondaryBm++;

nCrossBm = ((nPrimaryBm > 0) || (nSecondaryBm > 0));

Assert (nPrimaryBm <= 2);
Assert (nSecondaryBm <= 4);
Assert (nCrossBm <= 1);
#if DBG
if (gameStates.render.bExternalView)
#else
if (gameStates.render.bExternalView && (!IsMultiGame || EGI_FLAG (bEnableCheats, 0, 0, 0)))
#endif
	return;
cmScaleX *= HUD_ASPECT;
if ((gameStates.ogl.nReticle == 2) || (gameStates.ogl.nReticle && grdCurCanv->cvBitmap.bmProps.w > 320))
   OglDrawReticle (nCrossBm, nPrimaryBm, nSecondaryBm);
else {
	bHiresReticle = (gameStates.render.fonts.bHires != 0);
	bSmallReticle = !bForceBig && (grdCurCanv->cvBitmap.bmProps.w * 3 <= gameData.render.window.wMax * 2);
	ofs = (bHiresReticle ? 0 : 2) + bSmallReticle;

	nGaugeIndex = (bSmallReticle ? SML_RETICLE_CROSS : RETICLE_CROSS) + nCrossBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (cross_offsets [ofs].x)),
		- (y + HUD_SCALE_Y (cross_offsets [ofs].y)),
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
	nGaugeIndex = (bSmallReticle ? SML_RETICLE_PRIMARY : RETICLE_PRIMARY) + nPrimaryBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (primary_offsets [ofs].x)),
		- (y + HUD_SCALE_Y (primary_offsets [ofs].y)),
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
	nGaugeIndex = (bSmallReticle ? SML_RETICLE_SECONDARY : RETICLE_SECONDARY) + nSecondaryBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (secondary_offsets [ofs].x)),
		- (y + HUD_SCALE_Y (secondary_offsets [ofs].y)),
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
  }
if (!gameStates.app.bNostalgia && gameOpts->input.mouse.bJoystick && gameOpts->render.cockpit.bMouseIndicator)
	OglDrawMouseIndicator ();
cmScaleX /= HUD_ASPECT;
}

//	-----------------------------------------------------------------------------

void HUDShowKillList ()
{
	int n_players, player_list [MAX_NUM_NET_PLAYERS];
	int n_left, i, xo = 0, x0, x1, y, save_y, fth, l;
	int t = gameStates.app.nSDLTicks;
	int bGetPing = gameStates.render.cockpit.bShowPingStats && (!networkData.tLastPingStat || (t - networkData.tLastPingStat >= 1000));
	static int faw = -1;

	static int nIdKillList [2][MAX_PLAYERS] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

if (HIDE_HUD)
	return;
// ugly hack since placement of netgame players and kills is based off of
// menuhires (which is always 1 for mac).  This throws off placement of
// players in pixel double mode.
if (bGetPing)
	networkData.tLastPingStat = t;
if (gameData.multigame.kills.xShowListTimer > 0) {
	gameData.multigame.kills.xShowListTimer -= gameData.time.xFrame;
	if (gameData.multigame.kills.xShowListTimer < 0)
		gameData.multigame.kills.bShowList = 0;
	}
GrSetCurFont (GAME_FONT);
n_players = (gameData.multigame.kills.bShowList == 3) ? 2 : MultiGetKillList (player_list);
n_left = (n_players <= 4) ? n_players : (n_players+1)/2;
//If font size changes, this code might not work right anymore
//Assert (GAME_FONT->ftHeight==5 && GAME_FONT->ftWidth==7);
fth = GAME_FONT->ftHeight;
x0 = HUD_LHX (1);
x1 = (IsCoopGame) ? HUD_LHX (43) : HUD_LHX (43);
save_y = y = grdCurCanv->cv_h - n_left* (fth+1);
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
	save_y = y -= HUD_LHX (6);
	x1 = (IsCoopGame) ? HUD_LHX (43) : HUD_LHX (43);
	}
if (gameStates.render.cockpit.bShowPingStats) {
	if (faw < 0)
		faw = get_fontTotal_width (GAME_FONT) / (GAME_FONT->ftMaxChar - GAME_FONT->ftMinChar + 1);
		if (gameData.multigame.kills.bShowList == 2)
			xo = faw * 24;//was +25;
		else if (IsCoopGame)
			xo = faw * 14;//was +30;
		else
			xo = faw * 8; //was +20;
	}
for (i = 0; i < n_players; i++) {
	int nPlayer;
	char name [80], teamInd [2] = {127, 0};
	int sw, sh, aw, indent =0;

	if (i>=n_left) {
		x0 = grdCurCanv->cv_w - ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? HUD_LHX (53) : HUD_LHX (60));
		x1 = grdCurCanv->cv_w - ((gameData.app.nGameMode & GM_MULTI_COOP) ? HUD_LHX (27) : HUD_LHX (15));
		if (gameStates.render.cockpit.bShowPingStats) {
			x0 -= xo + 6 * faw;
			x1 -= xo + 6 * faw;
			}
		if (i==n_left)
			y = save_y;
		if (netGame.KillGoal || netGame.xPlayTimeAllowed)
			x1-=HUD_LHX (18);
		}
	else if (netGame.KillGoal || netGame.xPlayTimeAllowed)
		 x1 = HUD_LHX (43) - HUD_LHX (18);
	nPlayer = (gameData.multigame.kills.bShowList == 3) ? i : player_list [i];
	if ((gameData.multigame.kills.bShowList == 1) || (gameData.multigame.kills.bShowList == 2)) {
		int color;

		if (gameData.multiplayer.players [nPlayer].connected != 1)
			GrSetFontColorRGBi (RGBA_PAL2 (12, 12, 12), 1, 0, 0);
		else {
			if (IsTeamGame)
				color = GetTeam (nPlayer);
			else
				color = nPlayer;
			GrSetFontColorRGBi (RGBA_PAL2 (playerColors [color].r, playerColors [color].g, playerColors [color].b), 1, 0, 0);
			}
		}
	else
		GrSetFontColorRGBi (RGBA_PAL2 (playerColors [nPlayer].r, playerColors [nPlayer].g, playerColors [nPlayer].b), 1, 0, 0);
	if (gameData.multigame.kills.bShowList == 3) {
		if (GetTeam (gameData.multiplayer.nLocalPlayer) == i) {
#if 0//def _DEBUG
			sprintf (name, "%c%-8s %d.%d.%d.%d:%d",
						teamInd [0], netGame.szTeamName [i],
						netPlayers.players [i].network.ipx.node [0],
						netPlayers.players [i].network.ipx.node [1],
						netPlayers.players [i].network.ipx.node [2],
						netPlayers.players [i].network.ipx.node [3],
						netPlayers.players [i].network.ipx.node [5] +
						 (unsigned) netPlayers.players [i].network.ipx.node [4] * 256);
#else
			sprintf (name, "%c%s", teamInd [0], netGame.szTeamName [i]);
#endif
			indent = 0;
			}
		else {
#if SHOW_PLAYER_IP
			sprintf (name, "%-8s %d.%d.%d.%d:%d",
						netGame.szTeamName [i],
						netPlayers.players [i].network.ipx.node [0],
						netPlayers.players [i].network.ipx.node [1],
						netPlayers.players [i].network.ipx.node [2],
						netPlayers.players [i].network.ipx.node [3],
						netPlayers.players [i].network.ipx.node [5] +
						 (unsigned) netPlayers.players [i].network.ipx.node [4] * 256);
#else
			strcpy (name, netGame.szTeamName [i]);
#endif
			GrGetStringSize (teamInd, &indent, &sh, &aw);
			}
		}
	else
#if 0//def _DEBUG
		sprintf (name, "%-8s %d.%d.%d.%d:%d",
					gameData.multiplayer.players [nPlayer].callsign,
					netPlayers.players [nPlayer].network.ipx.node [0],
					netPlayers.players [nPlayer].network.ipx.node [1],
					netPlayers.players [nPlayer].network.ipx.node [2],
					netPlayers.players [nPlayer].network.ipx.node [3],
					netPlayers.players [nPlayer].network.ipx.node [5] +
					 (unsigned) netPlayers.players [nPlayer].network.ipx.node [4] * 256);
#else
		strcpy (name, gameData.multiplayer.players [nPlayer].callsign);	// Note link to above if!!
#endif
#if 0//def _DEBUG
	x1 += HUD_LHX (100);
#endif
	for (l = (int) strlen (name); l;) {
		GrGetStringSize (name, &sw, &sh, &aw);
		if (sw <= x1 - x0 - HUD_LHX (2))
			break;
		name [--l] = '\0';
		}
	nIdKillList [0][i] = GrPrintF (nIdKillList [0] + i, x0 + indent, y, "%s", name);

	if (gameData.multigame.kills.bShowList == 2) {
		if (gameData.multiplayer.players [nPlayer].netKilledTotal + gameData.multiplayer.players [nPlayer].netKillsTotal <= 0)
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, TXT_NOT_AVAIL);
		else
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%d%%",
				 (int) ((double) gameData.multiplayer.players [nPlayer].netKillsTotal /
						 ((double) gameData.multiplayer.players [nPlayer].netKilledTotal +
						  (double) gameData.multiplayer.players [nPlayer].netKillsTotal) * 100.0));
		}
	else if (gameData.multigame.kills.bShowList == 3) {
		if (gameData.app.nGameMode & GM_ENTROPY)
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%3d [%d/%d]",
						 gameData.multigame.kills.nTeam [i], gameData.entropy.nTeamRooms [i + 1],
						 gameData.entropy.nTotalRooms);
		else
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%3d", gameData.multigame.kills.nTeam [i]);
		}
	else if (IsCoopGame)
		nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%-6d", gameData.multiplayer.players [nPlayer].score);
   else if (netGame.xPlayTimeAllowed || netGame.KillGoal)
      nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%3d (%d)",
					 gameData.multiplayer.players [nPlayer].netKillsTotal,
					 gameData.multiplayer.players [nPlayer].nKillGoalCount);
   else
		nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1, y, "%3d", gameData.multiplayer.players [nPlayer].netKillsTotal);
	if (gameStates.render.cockpit.bShowPingStats && (nPlayer != gameData.multiplayer.nLocalPlayer)) {
		if (bGetPing)
			PingPlayer (nPlayer);
		if (pingStats [nPlayer].sent) {
#if 0//def _DEBUG
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1 + xo, y, "%lu %d %d",
						  pingStats [nPlayer].ping,
						  pingStats [nPlayer].sent,
						  pingStats [nPlayer].received);
#else
			nIdKillList [1][i] = GrPrintF (nIdKillList [1] + i, x1 + xo, y, "%lu %i%%", pingStats [nPlayer].ping,
						 100 - ((pingStats [nPlayer].received * 100) / pingStats [nPlayer].sent));
#endif
			}
		}
	y += fth+1;
	}
}

//	-----------------------------------------------------------------------------

#if DBG
extern int bSavingMovieFrames;
#else
#define bSavingMovieFrames 0
#endif

//returns true if viewerP can see tObject
//	-----------------------------------------------------------------------------

int CanSeeObject (int nObject, int bCheckObjs)
{
	tFVIQuery fq;
	int nHitType;
	tFVIData hit_data;

	//see if we can see this tPlayer

fq.p0 = &gameData.objs.viewerP->info.position.vPos;
fq.p1 = &OBJECTS [nObject].info.position.vPos;
fq.radP0 =
fq.radP1 = 0;
fq.thisObjNum = gameStates.render.cameras.bActive ? -1 : OBJ_IDX (gameData.objs.viewerP);
fq.flags = bCheckObjs ? FQ_CHECK_OBJS | FQ_TRANSWALL : FQ_TRANSWALL;
fq.startSeg = gameData.objs.viewerP->info.nSegment;
fq.ignoreObjList = NULL;
nHitType = FindVectorIntersection (&fq, &hit_data);
return bCheckObjs ? (nHitType == HIT_OBJECT) && (hit_data.hit.nObject == nObject) : (nHitType != HIT_WALL);
}

//	-----------------------------------------------------------------------------

//show names of teammates & players carrying flags
void ShowHUDNames ()
{
	int bHasFlag, bShowName, bShowTeamNames, bShowAllNames, bShowFlags, nObject, nTeam;
	int p;
	rgb *colorP;
	static int nCurColor = 1, tColorChange = 0;
	static rgb typingColors [2] = {
		{63, 0, 0},
		{63, 63, 0}
	};
	char s [CALLSIGN_LEN+10];
	int w, h, aw;
	int x1, y1;
	int nColor;
	static int nIdNames [2][MAX_PLAYERS] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

bShowAllNames = ((gameData.demo.nState == ND_STATE_PLAYBACK) ||
						 (netGame.bShowAllNames && gameData.multigame.bShowReticleName));
bShowTeamNames = (gameData.multigame.bShowReticleName && (IsCoopGame || IsTeamGame));
bShowFlags = (gameData.app.nGameMode & (GM_CAPTURE | GM_HOARD | GM_ENTROPY));

nTeam = GetTeam (gameData.multiplayer.nLocalPlayer);
for (p = 0; p < gameData.multiplayer.nPlayers; p++) {	//check all players

	bShowName = (gameStates.multi.bPlayerIsTyping [p] || (bShowAllNames && !(gameData.multiplayer.players [p].flags & PLAYER_FLAGS_CLOAKED)) || (bShowTeamNames && GetTeam (p)==nTeam));
	bHasFlag = (gameData.multiplayer.players [p].connected && gameData.multiplayer.players [p].flags & PLAYER_FLAGS_FLAG);

	if (gameData.demo.nState != ND_STATE_PLAYBACK)
		nObject = gameData.multiplayer.players [p].nObject;
	else {
		//if this is a demo, the nObject in the tPlayer struct is wrong,
		//so we search the tObject list for the nObject
		tObject *objP;
		FORALL_OBJS (objP, nObject)
			if ((objP->info.nType == OBJ_PLAYER) && (objP->info.nId == p))
				break;
		if (IS_OBJECT (objP, nObject))		//not in list, thus not visible
			bShowName = !bHasFlag;				//..so don't show name
		}

	if ((bShowName || bHasFlag) && CanSeeObject (nObject, 1)) {
		g3sPoint		vPlayerPos;
		vmsVector	vPos;

		vPos = OBJECTS [nObject].info.position.vPos;
		vPos[Y] += 2 * F1_0;
		G3TransformAndEncodePoint(&vPlayerPos, vPos);
		if (vPlayerPos.p3_codes == 0) {	//on screen
			G3ProjectPoint (&vPlayerPos);
			if (!(vPlayerPos.p3_flags & PF_OVERFLOW)) {
				fix x = vPlayerPos.p3_screen.x;
				fix y = vPlayerPos.p3_screen.y;
				if (bShowName) {				// Draw callsign on HUD
					if (gameStates.multi.bPlayerIsTyping [p]) {
						int t = gameStates.app.nSDLTicks;

						if (t - tColorChange > 333) {
							tColorChange = t;
							nCurColor = !nCurColor;
							}
						colorP = typingColors + nCurColor;
						}
					else {
						nColor = IsTeamGame ? GetTeam (p) : p;
						colorP = playerColors + nColor;
						}

					sprintf (s, "%s", gameStates.multi.bPlayerIsTyping [p] ? TXT_TYPING : gameData.multiplayer.players [p].callsign);
					GrGetStringSize (s, &w, &h, &aw);
					GrSetFontColorRGBi (RGBA_PAL2 (colorP->r, colorP->g, colorP->b), 1, 0, 0);
					x1 = x - w / 2;
					y1 = y - h / 2;
					//glBlendFunc (GL_ONE, GL_ONE);
					nIdNames [nCurColor][p] = GrString (x1, y1, s, nIdNames [nCurColor] + p);
					GrSetColorRGBi (RGBA_PAL2 (colorP->r, colorP->g, colorP->b));
					glLineWidth ((GLfloat) 2.0f);
					GrUBox (x1 - 4, y1 - 3, x1 + w + 2, y1 + h + 3);
					glLineWidth (1);
					//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					}

				if (bHasFlag && (gameStates.app.bNostalgia || !(EGI_FLAG (bTargetIndicators, 0, 1, 0) || EGI_FLAG (bTowFlags, 0, 1, 0)))) {// Draw box on HUD
					fix dy = -FixMulDiv (OBJECTS [nObject].info.xSize, I2X (grdCurCanv->cv_h)/2, vPlayerPos.p3_vec[Z]);
//					fix dy = -FixMulDiv (FixMul (OBJECTS [nObject].size, viewInfo.scale.y), I2X (grdCurCanv->cv_h)/2, vPlayerPos.p3_z);
					fix dx = FixMul (dy, grdCurScreen->scAspect);
					fix w = dx/4;
					fix h = dy/4;
					if (gameData.app.nGameMode & (GM_CAPTURE | GM_ENTROPY))
						GrSetColorRGBi ((GetTeam (p) == TEAM_BLUE) ? MEDRED_RGBA :  MEDBLUE_RGBA);
					else if (gameData.app.nGameMode & GM_HOARD) {
						if (gameData.app.nGameMode & GM_TEAM)
							GrSetColorRGBi ((GetTeam (p) == TEAM_RED) ? MEDRED_RGBA :  MEDBLUE_RGBA);
						else
							GrSetColorRGBi (MEDGREEN_RGBA);
						}
					GrLine (x+dx-w, y-dy, x+dx, y-dy);
					GrLine (x+dx, y-dy, x+dx, y-dy+h);
					GrLine (x-dx, y-dy, x-dx+w, y-dy);
					GrLine (x-dx, y-dy, x-dx, y-dy+h);
					GrLine (x+dx-w, y+dy, x+dx, y+dy);
					GrLine (x+dx, y+dy, x+dx, y+dy-h);
					GrLine (x-dx, y+dy, x-dx+w, y+dy);
					GrLine (x-dx, y+dy, x-dx, y+dy-h);
					}
				}
			}
		}
	}
}

//	-----------------------------------------------------------------------------

inline void SetCMScales (void)
{
cmScaleX = (grdCurScreen->scWidth <= 640) ? 1 : (double) grdCurScreen->scWidth / 640.0;
cmScaleY = (grdCurScreen->scHeight <= 480) ? 1 : (double) grdCurScreen->scHeight / 480.0;
}

//	-----------------------------------------------------------------------------
//draw all the things on the HUD
void DrawHUD (void)
{
if (HIDE_HUD)
	return;
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
	gameStates.render.cockpit.nLastDrawn [0] =
	gameStates.render.cockpit.nLastDrawn [1] = -1;
	InitGauges ();
//	vr_reset_display ();
  }
SetCMScales ();
nHUDLineSpacing = GAME_FONT->ftHeight + GAME_FONT->ftHeight/4;

	//	Show score so long as not in rearview
if (!(gameStates.render.bRearView || bSavingMovieFrames ||
	 (gameStates.render.cockpit.nMode == CM_REAR_VIEW) ||
	 (gameStates.render.cockpit.nMode == CM_STATUS_BAR))) {
	HUDShowScore ();
	if (scoreTime)
		HUDShowScoreAdded ();
	}

if (!(gameStates.render.bRearView || bSavingMovieFrames || (gameStates.render.cockpit.nMode == CM_REAR_VIEW)))
	HUDShowTimerCount ();

//	Show other stuff if not in rearview or letterbox.
if (!gameStates.render.bRearView && (gameStates.render.cockpit.nMode != CM_REAR_VIEW)) {
	if ((gameStates.render.cockpit.nMode == CM_STATUS_BAR) || (gameStates.render.cockpit.nMode == CM_FULL_SCREEN))
		HUDShowHomingWarning ();

	if ((gameStates.render.cockpit.nMode == CM_FULL_SCREEN) || (gameStates.render.cockpit.nMode == CM_LETTERBOX)) {
		if (!gameOpts->render.cockpit.bTextGauges) {
			if (gameOpts->render.cockpit.bScaleGauges) {
				xScale = (double) grdCurCanv->cv_h / 480.0;
				yScale = (double) grdCurCanv->cv_h / 640.0;
				}
			else
				xScale = yScale = 1;
			}
		HUDShowEnergy ();
		HUDShowShield ();
		HUDShowAfterburner ();
		HUDShowWeapons ();
		if (!bSavingMovieFrames)
			HUDShowKeys ();
		HUDShowCloakInvul ();

		if ((gameData.demo.nState==ND_STATE_RECORDING) && (LOCALPLAYER.flags != oldFlags [gameStates.render.vr.nCurrentPage])) {
			NDRecordPlayerFlags (oldFlags [gameStates.render.vr.nCurrentPage], LOCALPLAYER.flags);
			oldFlags [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.flags;
			}
		}
#if 1//def _DEBUG
	if (!(IsMultiGame && gameData.multigame.kills.bShowList) && !bSavingMovieFrames)
		ShowTime ();
#endif
	if (gameOpts->render.cockpit.bReticle && !gameStates.app.bPlayerIsDead && !viewInfo.bUsePlayerHeadAngles)
		ShowReticle (0);

	ShowHUDNames ();
	if (gameStates.render.cockpit.nMode != CM_REAR_VIEW) {
		HUDShowFlag ();
		HUDShowOrbs ();
		}
	if (!bSavingMovieFrames)
		HUDRenderMessageFrame ();

	if (gameStates.render.cockpit.nMode!=CM_STATUS_BAR && !bSavingMovieFrames)
		HUDShowLives ();

	if (gameData.app.nGameMode&GM_MULTI && gameData.multigame.kills.bShowList)
		HUDShowKillList ();
#if 0
	DrawWeaponBoxes ();
#endif
	return;
	}

if (gameStates.render.bRearView && gameStates.render.cockpit.nMode!=CM_REAR_VIEW) {
	HUDRenderMessageFrame ();
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (NULL, 0x8000, grdCurCanv->cv_h - ((gameData.demo.nState == ND_STATE_PLAYBACK) ? 14 : 10), TXT_REAR_VIEW);
	}
}

//	-----------------------------------------------------------------------------

//print out some tPlayer statistics
void RenderGauges (void)
{
	static int old_display_mode = 0;
	int nEnergy = X2IR (LOCALPLAYER.energy);
	int nShields = X2IR (LOCALPLAYER.shields);
	int bCloak = ((LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) != 0);

if (HIDE_HUD)
	return;
SetCMScales ();
Assert ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) || (gameStates.render.cockpit.nMode == CM_STATUS_BAR));
// check to see if our display mode has changed since last render time --
// if so, then we need to make new gauge canvases.
if (old_display_mode != gameStates.video.nDisplayMode) {
	CloseGaugeCanvases ();
	InitGaugeCanvases ();
	old_display_mode = gameStates.video.nDisplayMode;
	}
if (nShields < 0)
	nShields = 0;
GrSetCurrentCanvas (GetCurrentGameScreen ());
GrSetCurFont (GAME_FONT);
if (gameData.demo.nState == ND_STATE_RECORDING)
	if (LOCALPLAYER.homingObjectDist >= 0)
		NDRecordHomingDistance (LOCALPLAYER.homingObjectDist);

DrawWeaponBoxes ();
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBRenderGauges ();
else if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
	if (nEnergy != oldEnergy [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING) {
			NDRecordPlayerEnergy (oldEnergy [gameStates.render.vr.nCurrentPage], nEnergy);
			}
		}
	DrawEnergyBar (nEnergy);
	DrawNumericalDisplay (nShields, nEnergy);
	oldEnergy [gameStates.render.vr.nCurrentPage] = nEnergy;

	if (gameData.physics.xAfterburnerCharge != oldAfterburner [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING) {
			NDRecordPlayerAfterburner (oldAfterburner [gameStates.render.vr.nCurrentPage], gameData.physics.xAfterburnerCharge);
			}
		}
	DrawAfterburnerBar (gameData.physics.xAfterburnerCharge);
	oldAfterburner [gameStates.render.vr.nCurrentPage] = gameData.physics.xAfterburnerCharge;

	if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
		DrawNumericalDisplay (nShields, nEnergy);
		DrawInvulnerableShip ();
		oldShields [gameStates.render.vr.nCurrentPage] = nShields ^ 1;
		}
	else {
		if (nShields != oldShields [gameStates.render.vr.nCurrentPage]) {		// Draw the shield gauge
			if (gameData.demo.nState == ND_STATE_RECORDING) {
				NDRecordPlayerShields (oldShields [gameStates.render.vr.nCurrentPage], nShields);
				}
			}
		DrawShieldBar (nShields);
		DrawNumericalDisplay (nShields, nEnergy);
		oldShields [gameStates.render.vr.nCurrentPage] = nShields;
		}

	if (LOCALPLAYER.flags != oldFlags [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState==ND_STATE_RECORDING)
			NDRecordPlayerFlags (oldFlags [gameStates.render.vr.nCurrentPage], LOCALPLAYER.flags);
		oldFlags [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.flags;
		}
	DrawKeys ();
	ShowHomingWarning ();
	ShowBombCount (BOMB_COUNT_X, BOMB_COUNT_Y, BLACK_RGBA, gameStates.render.cockpit.nMode == CM_FULL_COCKPIT);
	DrawPlayerShip (bCloak, bOldCloak [gameStates.render.vr.nCurrentPage], SHIP_GAUGE_X, SHIP_GAUGE_Y);
	}
bOldCloak [gameStates.render.vr.nCurrentPage] = bCloak;
}

//	---------------------------------------------------------------------------------------------------------
//	Call when picked up a laser powerup.
//	If laser is active, set oldWeapon [0] to -1 to force redraw.
void UpdateLaserWeaponInfo (void)
{
	if (oldWeapon [0][gameStates.render.vr.nCurrentPage] == 0)
		if (!(LOCALPLAYER.laserLevel > MAX_LASER_LEVEL && oldLaserLevel [gameStates.render.vr.nCurrentPage] <= MAX_LASER_LEVEL))
			oldWeapon [0][gameStates.render.vr.nCurrentPage] = -1;
}

void FillBackground (void);

int SW_drawn [2], SW_x [2], SW_y [2], SW_w [2], SW_h [2];

//	---------------------------------------------------------------------------------------------------------
//draws a 3d view into one of the cockpit windows.  win is 0 for left,
//1 for right.  viewerP is tObject.  NULL tObject means give up window
//nUser is one of the WBU_ constants.  If bRearView is set, show a
//rear view.  If label is non-NULL, print the label at the top of the
//window.

int cockpitWindowScale [4] = {6, 5, 4, 3};

void DoCockpitWindowView (int nWindow, tObject *viewerP, int bRearView, int nUser, const char *pszLabel)
{
	gsrCanvas windowCanv;
	static gsrCanvas overlap_canv;

	tObject *viewerSave = gameData.objs.viewerP;
	gsrCanvas *save_canv = grdCurCanv;
	static int bOverlapDirty [2]={0, 0};
	int nBox;
	static int window_x, window_y;
	tGaugeBox *boxP;
	int bRearViewSave = gameStates.render.bRearView;
	int w, h, dx, nZoomSave;

if (HIDE_HUD)
	return;
boxP = NULL;
if (!viewerP) {								//this nUser is done
	Assert (nUser == WBU_WEAPON || nUser == WBU_STATIC);
	if ((nUser == WBU_STATIC) && (weaponBoxUser [nWindow] != WBU_STATIC))
		staticTime [nWindow] = 0;
	if (weaponBoxUser [nWindow] == WBU_WEAPON || weaponBoxUser [nWindow] == WBU_STATIC)
		return;		//already set
	weaponBoxUser [nWindow] = nUser;
	if (bOverlapDirty [nWindow]) {
		GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [gameStates.render.vr.nCurrentPage]);
		FillBackground ();
		bOverlapDirty [nWindow] = 0;
		}
	return;
	}
UpdateRenderedData (nWindow+1, viewerP, bRearView, nUser);
weaponBoxUser [nWindow] = nUser;						//say who's using window
gameData.objs.viewerP = viewerP;
gameStates.render.bRearView = bRearView;

if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN) {
	w = (int) (gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / cockpitWindowScale [gameOpts->render.cockpit.nWindowSize] * HUD_ASPECT);			// hmm.  I could probably do the sub_buffer assigment for all macines, but I aint gonna chance it
	h = I2X (w) / grdCurScreen->scAspect;
	dx = (nWindow==0)?- (w+ (w/10)): (w/10);
	switch (gameOpts->render.cockpit.nWindowPos) {
		case 0:
			window_x = nWindow ?
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w - w - h / 10 :
				h / 10;
			window_y = gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.h - h - h / 10;
			break;
		case 1:
			window_x = nWindow ?
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 3 * 2 - w / 3 :
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 3 - 2 * w / 3;
			window_y = gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.h - h - h / 10;
			break;
		case 2:	// only makes sense if there's only one cockpit window
			window_x = gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 2 - w / 2;
			window_y = gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.h - h - h / 10;
			break;
		case 3:
			window_x = nWindow ?
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w - w - h / 10 :
				h / 10;
			window_y = h / 10;
			break;
		case 4:
			window_x = nWindow ?
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 3 * 2 - w / 3 :
				gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 3 - 2 * w / 3;
			window_y = h / 10;
			break;
		case 5:	// only makes sense if there's only one cockpit window
			window_x = gameStates.render.vr.buffers.render [0].cvBitmap.bmProps.w / 2 - w / 2;
			window_y = h / 10;
			break;
		}
	if ((gameOpts->render.cockpit.nWindowPos < 3) &&
			extraGameInfo [0].nWeaponIcons &&
			(extraGameInfo [0].nWeaponIcons - gameOpts->render.weaponIcons.bEquipment < 3))
			window_y -= (int) ((gameOpts->render.weaponIcons.bSmall ? 20.0 : 30.0) * (double) grdCurCanv->cv_h / 480.0);


	//copy these vars so stereo code can get at them
	SW_drawn [nWindow] = 1;
	SW_x [nWindow] = window_x;
	SW_y [nWindow] = window_y;
	SW_w [nWindow] = w;
	SW_h [nWindow] = h;

	GrInitSubCanvas (&windowCanv, &gameStates.render.vr.buffers.render [0], window_x, window_y, w, h);
	}
else {
	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
		nBox = (COCKPIT_PRIMARY_BOX) + nWindow;
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		nBox = (SB_PRIMARY_BOX) + nWindow;
	else
		goto abort;
	boxP = gaugeBoxes + nBox;
	GrInitSubCanvas (
		&windowCanv, gameStates.render.vr.buffers.render,
		HUD_SCALE_X (boxP->left),
		HUD_SCALE_Y (boxP->top),
		HUD_SCALE_X (boxP->right - boxP->left+1),
		HUD_SCALE_Y (boxP->bot - boxP->top+1));
	}

GrSetCurrentCanvas (&windowCanv);
G3PushMatrix ();
nZoomSave = gameStates.render.nZoomFactor;
gameStates.render.nZoomFactor = F1_0 * (gameOpts->render.cockpit.nWindowZoom + 1);					//the tPlayer's zoom factor
if ((nUser == WBU_RADAR_TOPDOWN) || (nUser == WBU_RADAR_HEADSUP)) {
	gameStates.render.bTopDownRadar = (nUser == WBU_RADAR_TOPDOWN);
	if (!IsMultiGame || (netGame.gameFlags & NETGAME_FLAG_SHOW_MAP))
		DoAutomap (0, 1);
	else
		RenderFrame (0, nWindow+1);
	}
else
	RenderFrame (0, nWindow+1);
gameStates.render.nZoomFactor = nZoomSave;
G3PopMatrix ();
//	HACK!If guided missile, wake up robots as necessary.
if (viewerP->info.nType == OBJ_WEAPON) {
	// -- Used to require to be GUIDED -- if (viewerP->id == GUIDEDMSL_ID)
	WakeupRenderedObjects (viewerP, nWindow+1);
	}
if (pszLabel) {
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (NULL, 0x8000, 2, pszLabel);
	}
if (nUser == WBU_GUIDED) {
	DrawGuidedCrosshair ();
	}
if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN) {
	int small_window_bottom, big_window_bottom, extra_part_h;

	GrSetColorRGBi (RGBA_PAL (0, 0, 32));
	GrUBox (0, 0, grdCurCanv->cvBitmap.bmProps.w-1, grdCurCanv->cvBitmap.bmProps.h);

	//if the window only partially overlaps the big 3d window, copy
	//the extra part to the visible screen
	big_window_bottom = gameData.render.window.y + gameData.render.window.h - 1;
	if (window_y > big_window_bottom) {
		//the small window is completely outside the big 3d window, so
		//copy it to the visible screen
		if (gameStates.render.vr.nScreenFlags & VRF_USE_PAGING)
			GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [!gameStates.render.vr.nCurrentPage]);
		else
			GrSetCurrentCanvas (GetCurrentGameScreen ());
		GrBitmap (window_x, window_y, &windowCanv.cvBitmap);
		bOverlapDirty [nWindow] = 1;
		}
	else {
		small_window_bottom = window_y + windowCanv.cvBitmap.bmProps.h - 1;
		extra_part_h = small_window_bottom - big_window_bottom;
		if (extra_part_h > 0) {
			GrInitSubCanvas (&overlap_canv, &windowCanv, 0, windowCanv.cvBitmap.bmProps.h-extra_part_h, windowCanv.cvBitmap.bmProps.w, extra_part_h);
			if (gameStates.render.vr.nScreenFlags & VRF_USE_PAGING)
				GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [!gameStates.render.vr.nCurrentPage]);
			else
				GrSetCurrentCanvas (GetCurrentGameScreen ());
				GrBitmap (window_x, big_window_bottom+1, &overlap_canv.cvBitmap);
			bOverlapDirty [nWindow] = 1;
			}
		}
	}
else {
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	CopyGaugeBox (boxP, &gameStates.render.vr.buffers.render [0].cvBitmap);
	}
//force redraw when done
oldWeapon [nWindow][gameStates.render.vr.nCurrentPage] = oldAmmoCount [nWindow][gameStates.render.vr.nCurrentPage] = -1;

abort:;

gameData.objs.viewerP = viewerSave;
GrSetCurrentCanvas (save_canv);
gameStates.render.bRearView = bRearViewSave;
}

//------------------------------------------------------------------------------

char *ftoa (char *pszVal, fix f)
{
	int decimal, fractional;

decimal = X2I (f);
fractional = ((f & 0xffff) * 100) / 65536;
if (fractional < 0)
	fractional = -fractional;
if (fractional > 99)
	fractional = 99;
sprintf (pszVal, "%d.%02d", decimal, fractional);
return pszVal;
}

//------------------------------------------------------------------------------

fix frameTimeList [8] = {0, 0, 0, 0, 0, 0, 0, 0};
fix frameTimeTotal = 0;
int frameTimeCounter = 0;

//	-----------------------------------------------------------------------------

void ShowFrameRate (void)
{
	static int nIdFrameRate = 0;

if (gameStates.render.bShowFrameRate) {
		static time_t t, t0 = -1;

		char	szItem [50];
		int	x = 11, y; // position measured from lower right corner

	frameTimeTotal += gameData.time.xRealFrame - frameTimeList [frameTimeCounter];
	frameTimeList [frameTimeCounter] = gameData.time.xRealFrame;
	frameTimeCounter = (frameTimeCounter + 1) % 8;
	if (gameStates.render.bShowFrameRate == 1) {
		static fix xRate = 0;

		char szRate [20];
		t = SDL_GetTicks ();
		if ((t0 < 0) || (t - t0 >= 500)) {
			t0 = t;
			xRate = frameTimeTotal ? FixDiv (f1_0 * 8, frameTimeTotal) : 0;
			}
		sprintf (szItem, "FPS: %s", ftoa (szRate, xRate));
		}
	else if (gameStates.render.bShowFrameRate == 2) {
		sprintf (szItem, "Faces: %d ", gameData.render.nTotalFaces);
		}
	else if (gameStates.render.bShowFrameRate == 3) {
		sprintf (szItem, "Transp: %d ", renderItems.nItems);
		}
	else if (gameStates.render.bShowFrameRate == 4) {
		sprintf (szItem, "Objects: %d/%d ", gameData.render.nTotalObjects, gameData.render.nTotalSprites);
		}
	else if (gameStates.render.bShowFrameRate == 5) {
		sprintf (szItem, "States: %d/%d ", gameData.render.nStateChanges, gameData.render.nShaderChanges);
		}
	else if (gameStates.render.bShowFrameRate == 6) {
		sprintf (szItem, "Lights/Face: %1.2f (%d)",
					(float) gameData.render.nTotalLights / (float) gameData.render.nTotalFaces,
					gameData.render.nMaxLights);
		x = 19;
		}
	if (gameStates.render.automap.bDisplay)
		y = 2;
	else if (IsMultiGame)
		y = 7;
	else
		y = 6;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (ORANGE_RGBA, 1, 0, 0);
	nIdFrameRate = GrPrintF (&nIdFrameRate,
									 grdCurCanv->cv_w - (x * GAME_FONT->ftWidth),
									 grdCurCanv->cv_h - y * (GAME_FONT->ftHeight + GAME_FONT->ftHeight / 4),
									 szItem);
	}
}

//------------------------------------------------------------------------------

void ToggleCockpit ()
{
	int nNewMode;

switch (gameStates.render.cockpit.nMode) {
	case CM_FULL_COCKPIT:
		nNewMode = CM_STATUS_BAR;
		break;

	case CM_STATUS_BAR:
		if (gameStates.render.bRearView)
			return;
		nNewMode = (gameStates.render.cockpit.nNextMode < 0) ? CM_FULL_SCREEN : CM_FULL_COCKPIT;
		break;

	case CM_FULL_SCREEN:
		if (gameStates.render.bRearView)
			return;
		nNewMode = CM_LETTERBOX;
		break;

	case CM_LETTERBOX:
		nNewMode = CM_FULL_COCKPIT;
		break;

	case CM_REAR_VIEW:
   default:
		return;			//do nothing
		break;
	}
gameStates.render.cockpit.nNextMode = -1;
SelectCockpit (nNewMode);
HUDClearMessages ();
WritePlayerFile ();
}

//	-----------------------------------------------------------------------------
