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

#ifndef _GENERICCOCKPIT_H
#define _GENERICCOCKPIT_H

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

typedef struct tCockpitHistory {
	int	score;
	int	energy;
	int	shields;
	int	flags;
	int	oldCloak;
	int	lives;
	fix	afterburner;
	int	bombCount;
	int	laserLevel;
	int	weapon [0]; 
	int	weapon [2];
	int	ammo [2];
	int	omegaCharge;
} tCockpitHistory;

class CCockpitInfo {
	public:
		tCockpitHistory	history [2];

		int	nCloakFadeState;
		int	bLastHomingWarningShown [2];
		int	scoreDisplay [2];
		fix	scoreTime;
		fix	lastWarningBeepTime [2];
		int	bHaveGaugeCanvases;
		int	nInvulnerableFrame;
		int	weaponBoxStates [2];
		fix	weaponBoxFadeValues [2];
		int	weaponBoxUser [2];
		int	nLineSpacing;
		int	nMode;
		float	xScale;
		float	yScale;
		float	xGaugeScale;
		float	yGaugeScale;
		int	fontWidth;
		int	fontHeight;
		int	nCockpit;

	public:
		CCockpitInfo () { Init (); }
		void Init (void);
};

//	-----------------------------------------------------------------------------

class CGenericCockpit {
	protected:
		CCockpitInfo	m_info;

	public:
		CBitmap* BitBlt (int nGauge, int x, int y, bool bScalePos = true, bool bScaleSize = true, int scale = I2X (1), int orient = 0, CBitmap* bmP = NULL);
		int _CDECL_ PrintF (int *idP, int x, int y, const char *pszFmt, ...);
		char* ftoa (char *pszVal, fix f);

		void DrawSlowMotion (void);
		void DrawTime (void);
		void DrawTimerCount (void);
		void PlayHomingWarning (void);
		void DrawBombCount (int x, int y, int bgColor, int bShowAlways);
		void DrawAmmoInfo (int x, int y, int ammoCount, int bPrimary);
		void CheckForExtraLife (int nPrevScore);
		void AddPointsToScore (int points);
		void AddBonusPointsToScore (int points);
		void DrawPlayerShip (int nCloakState, int nOldCloakState, int x, int y);
		void DrawWeaponInfo (int info_index, tGaugeBox *box, int xPic, int yPic, const char *pszName, int xText, int yText, int orient);
		void DrawWeaponInfo (int nWeaponType, int nWeaponId, int laserLevel);
		int DrawWeaponDisplay (int nWeaponType, int nWeaponId);
		void DrawStatic (int nWindow);
		void DrawWeaponDisplays (void);
		void DrawReticle (int bForceBig);
		int CanSeeObject (int nObject, int bCheckObjs);
		void DrawPlayerNames (void);
		void Render (void);

		virtual void GetHostageWindowCoords (int& x, int& y, int& w, int& h) = 0;
		virtual void DrawScore (void) = 0;
		virtual void DrawScoreAdded (void) = 0;
		virtual void DrawHomingWarning (void) = 0;
		virtual void DrawKeys (void) = 0;
		virtual void DrawOrbs (void) = 0;
		virtual void DrawFlag (void) = 0;
		virtual void DrawEnergy (void) = 0;
		virtual void DrawEnergyBar (void) = 0;
		virtual void DrawAfterburner (void) = 0;
		virtual void DrawAfterburnerBar (void) = 0;
		virtual void DrawBombCount (int& nIdBombCount, int y, int x, char* pszBombCount) = 0;
		virtual void DrawPrimaryAmmoInfo (int ammoCount) = 0;
		virtual void DrawSecondaryAmmoInfo (int ammoCount) = 0;
		virtual void DrawCloak (void) = 0;
		virtual void DrawInvul (void) = 0;
		virtual void DrawShield (void) = 0;
		virtual void DrawShieldBar (void) = 0;
		virtual void DrawLives (void) = 0;
		virtual void DrawPlayerShip (int nCloakState, int nOldCloakState, int y, int x) = 0;
		virtual void ClearBombCount (void) = 0;
		virtual int DrawBombCount (int* nId, int x, int y, char* pszBombCount) = 0;
		virtual void DrawKillList (void) = 0;
		virtual void DrawStatic (int nWindow) = 0;
		virtual void DrawWeapons (void);
		virtual void DrawCockpit (bool bAlphaTest = false);

		void DrawOrbs (int x, int y);
		void DrawFlag (int x, int y);
		void DrawKillList (int x, int y);
		void DrawStatic (int nWindow, int nIndex);
		void DrawCockpit (int nCockpit, int y, bool bAlphaTest = false);

		inline int Mode (void) { return m_info.mode; }
		inline void SetMode (int mode) { m_info.mode = mode; }
};

//	-----------------------------------------------------------------------------

class CHUD : public CGenericCockpit {
	public:
		virtual void GetHostageWindowCoords (int& x, int& y, int& w, int& h);
		virtual void DrawScore (void);
		virtual void DrawScoreAdded (void);
		virtual void DrawHomingWarning (void);
		virtual void DrawKeys (void);
		virtual void DrawOrbs (void);
		virtual void DrawFlag (void);
		virtual void DrawEnergy (void);
		virtual void DrawEnergyBar (void);
		virtual void DrawAfterburner (void);
		virtual void DrawAfterburnerBar (void);
		virtual void DrawBombCount (int& nIdBombCount, int y, int x, char* pszBombCount);
		virtual void DrawPrimaryAmmoInfo (int ammoCount);
		virtual void DrawSecondaryAmmoInfo (int ammoCount);
		virtual void DrawWeapons (void);
		virtual void DrawCloak (void);
		virtual void DrawInvul (void);
		virtual void DrawShield (void);
		virtual void DrawShieldBar (void);
		virtual void DrawLives (void);
		virtual void DrawPlayerShip (int nCloakState, int nOldCloakState, int y, int x);
		virtual void DrawCockpit (void);

		virtual void DrawStatic (int nWindow);
		virtual void DrawWeapons (void);
		virtual void DrawKillList (void);
		virtual void ClearBombCount (void);
		virtual int DrawBombCount (int* nId, int x, int y, char* pszBombCount);
		virtual void DrawCockpit (bool bAlphaTest = false);

	private:
		int FlashGauge (int h, int *bFlash, int tToggle);
	};

//	-----------------------------------------------------------------------------

class CStatusBar : public CGenericCockpit {
	public:
		CBitmap* StretchBlt (int nGauge, int x, int y, double xScale, double yScale, int scale, int orient, CBitmap);

		virtual void GetHostageWindowCoords (int& x, int& y, int& w, int& h);
		virtual void DrawScore (void);
		virtual void DrawScoreAdded (void);
		virtual void DrawHomingWarning (void);
		virtual void DrawKeys (void);
		virtual void DrawOrbs (void);
		virtual void DrawFlag (void);
		virtual void DrawEnergy (void);
		virtual void DrawEnergyBar (void);
		virtual void DrawAfterburner (void);
		virtual void DrawAfterburnerBar (void);
		virtual void DrawBombCount (int& nIdBombCount, int y, int x, char* pszBombCount);
		virtual void DrawPrimaryAmmoInfo (int ammoCount);
		virtual void DrawSecondaryAmmoInfo (int ammoCount);
		virtual void DrawWeapons (void);
		virtual void DrawCloak (void) {}
		virtual void DrawInvul (void) {}
		virtual void DrawShield (void);
		virtual void DrawShieldBar (void);
		virtual void DrawLives (void);
		virtual void DrawPlayerShip (int nCloakState, int nOldCloakState, int y, int x);

		virtual void DrawStatic (int nWindow);
		virtual void DrawWeapons (void);
		virtual void DrawKillList (void);
		virtual void ClearBombCount (void);
		virtual int DrawBombCount (int* nId, int x, int y, char* pszBombCount);
		virtual void DrawCockpit (bool bAlphaTest = false);
	};

//	-----------------------------------------------------------------------------

class CCockpit : public CGenericCockpit {
	public:
		virtual void GetHostageWindowCoords (int& x, int& y, int& w, int& h);
		virtual void DrawScore (void);
		virtual void DrawScoreAdded (void);
		virtual void DrawHomingWarning (void);
		virtual void DrawKeys (void);
		virtual void DrawOrbs (void);
		virtual void DrawFlag (void);
		virtual void DrawEnergy (void);
		virtual void DrawEnergyBar (void);
		virtual void DrawAfterburner (void);
		virtual void DrawAfterburnerBar (void);
		virtual void DrawBombCount (int& nIdBombCount, int y, int x, char* pszBombCount);
		virtual void DrawPrimaryAmmoInfo (int ammoCount);
		virtual void DrawSecondaryAmmoInfo (int ammoCount);
		virtual void DrawWeapons (void);
		virtual void DrawCloak (void) {}
		virtual void DrawInvul (void) {}
		virtual void DrawShield (void);
		virtual void DrawShieldBar (void);
		virtual void DrawLives (void);
		virtual void DrawPlayerShip (int nCloakState, int nOldCloakState, int y, int x);

		virtual void DrawStatic (int nWindow);
		virtual void DrawWeapons (void);
		virtual void DrawKillList (void);
		virtual void ClearBombCount (void);
		virtual int DrawBombCount (int* nId, int x, int y, char* pszBombCount);
		virtual void DrawCockpit (bool bAlphaTest = false);
	};

//	-----------------------------------------------------------------------------

class CRearView : public CGenericCockpit {
	public:
		virtual void GetHostageWindowCoords (int& x, int& y, int& w, int& h) { x = y = w = h = -1; }
		virtual void DrawScore (void) {}
		virtual void DrawScoreAdded (void) {}
		virtual void DrawHomingWarning (void) {}
		virtual void DrawKeys (void) {}
		virtual void DrawOrbs (void) {}
		virtual void DrawFlag (void) {}
		virtual void DrawEnergy (void) {}
		virtual void DrawEnergyBar (void) {}
		virtual void DrawAfterburner (void) {}
		virtual void DrawAfterburnerBar (void) {}
		virtual void DrawBombCount (int& nIdBombCount, int y, int x, char* pszBombCount) {}
		virtual void DrawPrimaryAmmoInfo (int ammoCount) {}
		virtual void DrawSecondaryAmmoInfo (int ammoCount) {}
		virtual void DrawWeapons (void) {}
		virtual void DrawCloak (void) {}
		virtual void DrawInvul (void) {}
		virtual void DrawShield (void) {}
		virtual void DrawShieldBar (void) {}
		virtual void DrawLives (void) {}
		virtual void DrawPlayerShip (int nCloakState, int nOldCloakState, int y, int x) {}
		virtual void DrawCockpit (void) {}

		virtual void DrawStatic (int nWindow) {}
		virtual void DrawWeapons (void) {}
		virtual void DrawKillList (void) {}
		virtual void ClearBombCount (void) {}
		virtual int DrawBombCount (int* nId, int x, int y, char* pszBombCount) {}
		virtual void DrawCockpit (bool bAlphaTest = false) { DrawCockpit (gameStates.render.cockpit.nMode + nCockpit, 0, bAlphaTest); }

	private:
		int FlashGauge (int h, int *bFlash, int tToggle);
	};

//	-----------------------------------------------------------------------------

#endif /* _GENERICCOCKPIT_H */
