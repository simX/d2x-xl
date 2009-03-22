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

#include "menu.h"
#include "descent.h"
#include "ipx.h"
#include "key.h"
#include "iff.h"
#include "u_mem.h"
#include "error.h"
#include "screens.h"
#include "joy.h"
#include "slew.h"
#include "args.h"
#include "hogfile.h"
#include "newdemo.h"
#include "timer.h"
#include "text.h"
#include "gamefont.h"
#include "menu.h"
#include "network.h"
#include "network_lib.h"
#include "netmenu.h"
#include "scores.h"
#include "joydefs.h"
#include "playsave.h"
#include "kconfig.h"
#include "credits.h"
#include "texmap.h"
#include "state.h"
#include "movie.h"
#include "gamepal.h"
#include "cockpit.h"
#include "strutil.h"
#include "reorder.h"
#include "rendermine.h"
#include "light.h"
#include "lightmap.h"
#include "autodl.h"
#include "tracker.h"
#include "omega.h"
#include "lightning.h"
#include "vers_id.h"
#include "input.h"
#include "collide.h"
#include "objrender.h"
#include "sparkeffect.h"
#include "renderthreads.h"
#include "soundthreads.h"
#include "menubackground.h"

//------------------------------------------------------------------------------

static int	optWindowSize, optWindowZoom, optWindowPos, optWindowAlign,
				optTextGauges, optWeaponIcons, bShowWeaponIcons, 
				optIconAlpha, optTgtInd, optDmgInd, optHitInd, optMslLockInd;

static int nWindowPos, nWindowAlign;

static const char *szWindowSize [4];
static const char *szWindowPos [2];
static const char *szWindowAlign [3];

//------------------------------------------------------------------------------

#if !SIMPLE_MENUS

int TgtIndOptionsCallback (CMenu& menu, int& key, int nCurItem, int nState)
{
if (nState)
	return nCurItem;

	CMenuItem	*m;
	int			v, j;

m = menu + optTgtInd;
v = m->m_value;
if (v != (extraGameInfo [0].bTargetIndicators == 0)) {
	for (j = 0; j < 3; j++)
		if (m [optTgtInd + j].m_value) {
			extraGameInfo [0].bTargetIndicators = j;
			break;
			}
	key = -2;
	return nCurItem;
	}
m = menu + optDmgInd;
v = m->m_value;
if (v != extraGameInfo [0].bDamageIndicators) {
	extraGameInfo [0].bDamageIndicators = v;
	key = -2;
	return nCurItem;
	}
m = menu + optMslLockInd;
v = m->m_value;
if (v != extraGameInfo [0].bMslLockIndicators) {
	extraGameInfo [0].bMslLockIndicators = v;
	key = -2;
	return nCurItem;
	}
return nCurItem;
}

//------------------------------------------------------------------------------

void TgtIndOptionsMenu (void)
{
	CMenu	m;
	int	i, j, choice = 0;
	int	optCloakedInd, optRotateInd;

do {
	m.Destroy ();
	m.Create (15);

	optTgtInd = m.AddRadio (TXT_TGTIND_NONE, 0, KEY_A, HTX_CPIT_TGTIND);
	m.AddRadio (TXT_TGTIND_SQUARE, 0, KEY_R, HTX_CPIT_TGTIND);
	m.AddRadio (TXT_TGTIND_TRIANGLE, 0, KEY_T, HTX_CPIT_TGTIND);
	m [optTgtInd + extraGameInfo [0].bTargetIndicators].m_value = 1;
	if (extraGameInfo [0].bTargetIndicators)
		optCloakedInd = m.AddCheck (TXT_CLOAKED_INDICATOR, extraGameInfo [0].bCloakedIndicators, KEY_C, HTX_CLOAKED_INDICATOR);
	else
		optCloakedInd = -1;
	optDmgInd = m.AddCheck (TXT_DMG_INDICATOR, extraGameInfo [0].bDamageIndicators, KEY_D, HTX_CPIT_DMGIND);
	if (extraGameInfo [0].bTargetIndicators || extraGameInfo [0].bDamageIndicators)
		optHitInd = m.AddCheck (TXT_HIT_INDICATOR, extraGameInfo [0].bTagOnlyHitObjs, KEY_T, HTX_HIT_INDICATOR);
	else
		optHitInd = -1;
	optMslLockInd = m.AddCheck (TXT_MSLLOCK_INDICATOR, extraGameInfo [0].bMslLockIndicators, KEY_M, HTX_CPIT_MSLLOCKIND);
	if (extraGameInfo [0].bMslLockIndicators)
		optRotateInd = m.AddCheck (TXT_ROTATE_MSLLOCKIND, gameOpts->render.cockpit.bRotateMslLockInd, KEY_R, HTX_ROTATE_MSLLOCKIND);
	else
		optRotateInd = -1;
	do {
		i = m.Menu (NULL, TXT_TGTIND_MENUTITLE, &TgtIndOptionsCallback, &choice);
	} while (i >= 0);
	if (optTgtInd >= 0) {
		for (j = 0; j < 3; j++)
			if (m [optTgtInd + j].m_value) {
				extraGameInfo [0].bTargetIndicators = j;
				break;
				}
		GET_VAL (extraGameInfo [0].bCloakedIndicators, optCloakedInd);
		}
	GET_VAL (extraGameInfo [0].bDamageIndicators, optDmgInd);
	GET_VAL (extraGameInfo [0].bMslLockIndicators, optMslLockInd);
	GET_VAL (gameOpts->render.cockpit.bRotateMslLockInd, optRotateInd);
	GET_VAL (extraGameInfo [0].bTagOnlyHitObjs, optHitInd);
	} while (i == -2);
}

#endif //SIMPLE_MENUS

//------------------------------------------------------------------------------

#if !SIMPLE_MENUS

int GaugeOptionsCallback (CMenu& menu, int& key, int nCurItem, int nState)
{
if (nState)
	return nCurItem;

	CMenuItem	*m;
	int			v;

m = menu + optTextGauges;
v = !m->m_value;
if (v != gameOpts->render.cockpit.bTextGauges) {
	gameOpts->render.cockpit.bTextGauges = v;
	key = -2;
	return nCurItem;
	}
return nCurItem;
}

//------------------------------------------------------------------------------

void GaugeOptionsMenu (void)
{
	CMenu m;
	int	i, choice = 0;
	int	optScaleGauges, optFlashGauges, optShieldWarn, optObjectTally, optPlayerStats;

do {
	m.Destroy ();
	m.Create (10);
	optTextGauges = m.AddCheck (TXT_SHOW_GFXGAUGES, !gameOpts->render.cockpit.bTextGauges, KEY_P, HTX_CPIT_GFXGAUGES);
	if (!gameOpts->render.cockpit.bTextGauges && gameOpts->app.bExpertMode) {
		optScaleGauges = m.AddCheck (TXT_SCALE_GAUGES, gameOpts->render.cockpit.bScaleGauges, KEY_C, HTX_CPIT_SCALEGAUGES);
		optFlashGauges = m.AddCheck (TXT_FLASH_GAUGES, gameOpts->render.cockpit.bFlashGauges, KEY_F, HTX_CPIT_FLASHGAUGES);
		}
	else
		optScaleGauges =
		optFlashGauges = -1;
	optShieldWarn = m.AddCheck (TXT_SHIELD_WARNING, gameOpts->gameplay.bShieldWarning, KEY_W, HTX_CPIT_SHIELDWARN);
	optObjectTally = m.AddCheck (TXT_OBJECT_TALLY, gameOpts->render.cockpit.bObjectTally, KEY_T, HTX_CPIT_OBJTALLY);
	optPlayerStats = m.AddCheck (TXT_PLAYER_STATS, gameOpts->render.cockpit.bPlayerStats, KEY_S, HTX_CPIT_PLAYERSTATS);
	do {
		i = m.Menu (NULL, TXT_GAUGES_MENUTITLE, &GaugeOptionsCallback, &choice);
	} while (i >= 0);
	GET_VAL (gameOpts->gameplay.bShieldWarning, optShieldWarn);
	GET_VAL (gameOpts->render.cockpit.bObjectTally, optObjectTally);
	GET_VAL (gameOpts->render.cockpit.bPlayerStats, optPlayerStats);
	if (!(gameOpts->render.cockpit.bTextGauges = !m [optTextGauges].m_value)) {
		if (gameOpts->app.bExpertMode) {
			GET_VAL (gameOpts->render.cockpit.bScaleGauges, optScaleGauges);
			GET_VAL (gameOpts->render.cockpit.bFlashGauges, optFlashGauges);
			}
		else {
#if EXPMODE_DEFAULTS
			gameOpts->render.cockpit.bScaleGauges = 1;
			gameOpts->render.cockpit.bFlashGauges = 1;
			gameOpts->gameplay.bShieldWarning = 0;
			gameOpts->render.cockpit.bObjectTally = 0;
			gameOpts->render.cockpit.bPlayerStats = 0;
#endif
			}
		}
	} while (i == -2);
}

#endif


//------------------------------------------------------------------------------

int CockpitOptionsCallback (CMenu& menu, int& key, int nCurItem, int nState)
{
if (nState)
	return nCurItem;

	CMenuItem*	m;
	int			v;

#if SIMPLE_MENUS

m = menu + optWeaponIcons;
v = m->m_value;
if (v != bShowWeaponIcons) {
	bShowWeaponIcons = v;
	key = -2;
	return nCurItem;
	}

#endif

m = menu + optWindowSize;
v = m->m_value;
if (gameOpts->render.cockpit.nWindowSize != v) {
	gameOpts->render.cockpit.nWindowSize = v;
	m->SetText (szWindowSize [v]);
	sprintf (m->m_text, TXT_AUXWIN_SIZE, szWindowSize [v]);
	m->m_bRebuild = 1;
	}

m = menu + optWindowZoom;
v = m->m_value;
if (gameOpts->render.cockpit.nWindowZoom != v) {
	gameOpts->render.cockpit.nWindowZoom = v;
	sprintf (m->m_text, TXT_AUXWIN_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
	m->m_bRebuild = 1;
	}

m = menu + optWindowPos;
v = m->m_value;
if (nWindowPos != v) {
	nWindowPos = v;
	sprintf (m->m_text, TXT_AUXWIN_POSITION, szWindowPos [v]);
	m->m_bRebuild = 1;
	}

m = menu + optWindowAlign;
v = m->m_value;
if (nWindowAlign != v) {
	nWindowAlign = v;
	sprintf (m->m_text, TXT_AUXWIN_ALIGNMENT, szWindowAlign [v]);
	m->m_bRebuild = 1;
	}

return nCurItem;
}

//------------------------------------------------------------------------------

void DefaultCockpitSettings (void);

void CockpitOptionsMenu (void)
{
	static int choice = 0;

	CMenu m;
	int	i;
	int	optTextGauges, optHUD, optPosition, optAlignment, optTgtInd;

	char	szSlider [40];

szWindowSize [0] = TXT_AUXWIN_SMALL;
szWindowSize [1] = TXT_AUXWIN_MEDIUM;
szWindowSize [2] = TXT_AUXWIN_LARGE;
szWindowSize [3] = TXT_AUXWIN_HUGE;

szWindowPos [0] = TXT_POS_BOTTOM;
szWindowPos [1] = TXT_POS_TOP;

szWindowAlign [0] = TXT_ALIGN_CORNERS;
szWindowAlign [1] = TXT_ALIGN_MIDDLE;
szWindowAlign [2] = TXT_ALIGN_CENTER;

nWindowPos = gameOpts->render.cockpit.nWindowPos / 3;
nWindowAlign = gameOpts->render.cockpit.nWindowPos % 3;

optPosition = optAlignment = optWindowSize = optWindowZoom = optTextGauges = optWeaponIcons = -1;
bShowWeaponIcons = (extraGameInfo [0].nWeaponIcons != 0);

#if SIMPLE_MENUS

	int	optIconPos;

do {
	m.Destroy ();
	m.Create (15);

	optHUD = m.AddCheck (TXT_SHOW_HUD, gameOpts->render.cockpit.bHUD, KEY_U, HTX_CPIT_SHOWHUD);
	optTgtInd = m.AddCheck (TXT_TARGET_INDICATORS, extraGameInfo [0].bTargetIndicators, KEY_T, HTX_CPIT_TGTIND);
	optTextGauges = m.AddCheck (TXT_SHOW_GFXGAUGES, !gameOpts->render.cockpit.bTextGauges, KEY_P, HTX_CPIT_GFXGAUGES);
	m.AddText ("", 0);

	optWeaponIcons = m.AddCheck (TXT_SHOW_WEAPONICONS, bShowWeaponIcons, KEY_W, HTX_CPIT_WPNICONS);
	if (bShowWeaponIcons) {
		optIconPos = m.AddRadio (TXT_WPNICONS_TOP, 0, KEY_I, HTX_CPIT_ICONPOS);
		m.AddRadio (TXT_WPNICONS_BTM, 0, KEY_I, HTX_CPIT_ICONPOS);
		m.AddRadio (TXT_WPNICONS_LRB, 0, KEY_I, HTX_CPIT_ICONPOS);
		m.AddRadio (TXT_WPNICONS_LRT, 0, KEY_I, HTX_CPIT_ICONPOS);
		m [optIconPos + NMCLAMP (extraGameInfo [0].nWeaponIcons - 1, 0, 3)].m_value = 1;
		}
	else
		optIconPos = -1;
	m.AddText ("", 0);

	//if (gameOpts->app.bExpertMode) 
		{
		m.AddText (TXT_COCKPIT_WINDOWS, 0);
		sprintf (szSlider, TXT_AUXWIN_SIZE, szWindowSize [gameOpts->render.cockpit.nWindowSize]);
		optWindowSize = m.AddSlider (szSlider, gameOpts->render.cockpit.nWindowSize, 0, 3, KEY_S, HTX_CPIT_WINSIZE);

		sprintf (szSlider, TXT_AUXWIN_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
		optWindowZoom = m.AddSlider (szSlider, gameOpts->render.cockpit.nWindowZoom, 0, 3, KEY_Z, HTX_CPIT_WINZOOM);

		sprintf (szSlider, TXT_AUXWIN_POSITION, szWindowPos [nWindowPos]);
		optWindowPos = m.AddSlider (szSlider, nWindowPos, 0, 1, KEY_P, HTX_AUXWIN_POSITION);

		sprintf (szSlider, TXT_AUXWIN_ALIGNMENT, szWindowAlign [nWindowAlign]);
		optWindowAlign = m.AddSlider (szSlider, nWindowAlign, 0, 2, KEY_A, HTX_AUXWIN_ALIGNMENT);
		m.AddText ("", 0);
		}

	do {
		i = m.Menu (NULL, TXT_COCKPIT_OPTS, &CockpitOptionsCallback, &choice);
		if (i < 0)
			break;
		if (bShowWeaponIcons && (optIconPos >= 0)) {
			for (int j = 0; j < 4; j++)
				if (m [optIconPos + j].m_value) {
					extraGameInfo [0].nWeaponIcons = j + 1;
					break;
					}
				}
	} while (i >= 0);

	GET_VAL (gameOpts->render.cockpit.bHUD, optHUD);
	GET_VAL (extraGameInfo [0].bTargetIndicators, optTgtInd);
	gameOpts->render.cockpit.bTextGauges = !m [optTextGauges].m_value;
	gameOpts->render.cockpit.nWindowPos = nWindowPos * 3 + nWindowAlign;
	if (bShowWeaponIcons) {
		if (optIconPos >= 0) {
			for (int j = 0; j < 4; j++)
				if (m [optIconPos + j].m_value) {
					extraGameInfo [0].nWeaponIcons = j + 1;
					break;
					}
				}
		GET_VAL (gameOpts->render.weaponIcons.alpha, optIconAlpha);
		}
	} while (i == -2);

DefaultCockpitSettings ();

#else

	int	optGauges, optReticle, optGuided, optWideDisplays,
			optMissileView, optMouseInd, optSplitMsgs, optHUDMsgs;
do {
	m.Destroy ();
	m.Create (30);

	if (gameOpts->app.bExpertMode) {
		optWindowSize = m.AddSlider (szWindowSize [gameOpts->render.cockpit.nWindowSize], gameOpts->render.cockpit.nWindowSize, 0, 3, KEY_S, HTX_CPIT_WINSIZE);
		sprintf (szSlider, TXT_CW_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
		optWindowZoom = m.AddSlider (szSlider, gameOpts->render.cockpit.nWindowZoom, 0, 3, KEY_Z, HTX_CPIT_WINZOOM);
		m.AddText ("", 0);
		m.AddText (TXT_AUXWIN_POSITION, 0);
		optPosition = m.AddRadio (TXT_POS_BOTTOM, nWindowPos == 0, KEY_B, HTX_AUXWIN_POSITION);
		m.AddRadio (TXT_POS_TOP, nWindowPos == 1, KEY_T, HTX_AUXWIN_POSITION);
		m.AddText ("", 0);
		m.AddText (TXT_AUXWIN_ALIGNMENT, 0);
		optAlignment = m.AddRadio (TXT_ALIGN_CORNERS, nWindowAlign == 0, KEY_O, HTX_AUXWIN_ALIGNMENT);
		m.AddRadio (TXT_ALIGN_MIDDLE, nWindowAlign == 1, KEY_I, HTX_AUXWIN_ALIGNMENT);
		m.AddRadio (TXT_ALIGN_CENTER, nWindowAlign == 2, KEY_E, HTX_AUXWIN_ALIGNMENT);
		m.AddText ("", 0);
		optHUD = m.AddCheck (TXT_SHOW_HUD, gameOpts->render.cockpit.bHUD, KEY_U, HTX_CPIT_SHOWHUD);
		optWideDisplays = m.AddCheck (TXT_CPIT_WIDE_DISPLAYS, gameOpts->render.cockpit.bWideDisplays, KEY_D, HTX_CPIT_WIDE_DISPLAYS);
		optHUDMsgs = m.AddCheck (TXT_SHOW_HUDMSGS, gameOpts->render.cockpit.bHUDMsgs, KEY_M, HTX_CPIT_SHOWHUDMSGS);
		optReticle = m.AddCheck (TXT_SHOW_RETICLE, gameOpts->render.cockpit.bReticle, KEY_R, HTX_CPIT_SHOWRETICLE);
		if (gameOpts->input.mouse.bJoystick)
			optMouseInd = m.AddCheck (TXT_SHOW_MOUSEIND, gameOpts->render.cockpit.bMouseIndicator, KEY_O, HTX_CPIT_MOUSEIND);
		else
			optMouseInd = -1;
		}
	else
		optHUD =
		optWideDisplays =
		optHUDMsgs =
		optMouseInd = 
		optReticle = -1;
	optSplitMsgs = m.AddCheck (TXT_EXTRA_PLRMSGS, gameOpts->render.cockpit.bSplitHUDMsgs, KEY_P, HTX_CPIT_SPLITMSGS);
	optMissileView = m.AddCheck (TXT_MISSILE_VIEW, gameOpts->render.cockpit.bMissileView, KEY_I, HTX_CPITMSLVIEW);
	optGuided = m.AddCheck (TXT_GUIDED_MAINVIEW, gameOpts->render.cockpit.bGuidedInMainView, KEY_F, HTX_CPIT_GUIDEDVIEW);
	m.AddText ("", 0);
	optTgtInd = m.AddMenu (TXT_TGTIND_MENUCALL, KEY_T, "");
	optWeaponIcons = m.AddMenu (TXT_WPNICON_MENUCALL, KEY_W, "");
	optGauges = m.AddMenu (TXT_GAUGES_MENUCALL, KEY_G, "");
	do {
		i = m.Menu (NULL, TXT_COCKPIT_OPTS, &CockpitOptionsCallback, &choice);
		if (i < 0)
			break;
		if ((optTgtInd >= 0) && (i == optTgtInd))
			TgtIndOptionsMenu ();
		else if ((optWeaponIcons >= 0) && (i == optWeaponIcons))
			WeaponIconOptionsMenu ();
		else if ((optGauges >= 0) && (i == optGauges))
			GaugeOptionsMenu ();
	} while (i >= 0);
	GET_VAL (gameOpts->render.cockpit.bReticle, optReticle);
	GET_VAL (gameOpts->render.cockpit.bMissileView, optMissileView);
	GET_VAL (gameOpts->render.cockpit.bGuidedInMainView, optGuided);
	GET_VAL (gameOpts->render.cockpit.bMouseIndicator, optMouseInd);
	GET_VAL (gameOpts->render.cockpit.bHUD, optHUD);
	GET_VAL (gameOpts->render.cockpit.bWideDisplays, optWideDisplays);
	GET_VAL (gameOpts->render.cockpit.bHUDMsgs, optHUDMsgs);
	GET_VAL (gameOpts->render.cockpit.bSplitHUDMsgs, optSplitMsgs);
	if ((optAlignment >= 0) && (optPosition >= 0)) {
		for (nWindowPos = 0; nWindowPos < 2; nWindowPos++)
			if (m [optPosition + nWindowPos].m_value)
				break;
		for (nWindowAlign = 0; nWindowAlign < 3; nWindowAlign++)
			if (m [optAlignment + nWindowAlign].m_value)
				break;
		gameOpts->render.cockpit.nWindowPos = nWindowPos * 3 + nWindowAlign;
		}
	} while (i == -2);

#endif

}

//------------------------------------------------------------------------------
//eof
