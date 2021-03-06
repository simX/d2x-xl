/* $Id: gauges.c, v 1.10 2003/10/11 09:28:38 btb Exp $ */
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

#include "hudmsg.h"

#include "inferno.h"
#include "error.h"
#include "text.h"
#include "gamefont.h"
#include "input.h"
#include "object.h"
#include "ogl_defs.h"
#include "ogl_bitmap.h"
#include "ogl_hudstuff.h"
#include "slowmotion.h"
#include "network.h"
#include "gauges.h"
#include "hudicons.h"

//	-----------------------------------------------------------------------------

#define NUM_INV_ITEMS			10
#define INV_ITEM_HEADLIGHT		2
#define INV_ITEM_QUADLASERS	5
#define INV_ITEM_CLOAK			6
#define INV_ITEM_INVUL			7
#define INV_ITEM_SLOWMOTION	8
#define INV_ITEM_BULLETTIME	9

grsBitmap	*bmpInventory = NULL;
grsBitmap	bmInvItems [NUM_INV_ITEMS];
grsBitmap	bmObjTally [2];

int bHaveInvBms = -1;
int bHaveObjTallyBms = -1;

//	-----------------------------------------------------------------------------

char *pszObjTallyIcons [] = {"louguard.tga", "shldorb.tga"};

int LoadObjTallyIcons (void)
{
	int	i;

if (bHaveObjTallyBms > -1)
	return bHaveObjTallyBms;
memset (bmObjTally, 0, sizeof (bmObjTally));
for (i = 0; i < 2; i++)
	if (!ReadTGA (pszObjTallyIcons [i], gameFolders.szDataDir, bmObjTally + i, -1, 1.0, 0, 0)) {
		while (i)
			GrFreeBitmapData (bmObjTally + --i);
		return bHaveObjTallyBms = 0;
		}
return bHaveObjTallyBms = 1;
}

//	-----------------------------------------------------------------------------

void FreeObjTallyIcons (void)
{
	int	i;

if (bHaveObjTallyBms > 0) {
	for (i = 0; i < 2; i++)
		GrFreeBitmapData (bmObjTally + i);
	memset (bmObjTally, 0, sizeof (bmObjTally));
	bHaveObjTallyBms = -1;
	}
}

//	-----------------------------------------------------------------------------

void HUDShowObjTally (void)
{
	static int		objCounts [2] = {0, 0};
	static time_t	t0 = -1;
	static int		nIdTally [2] = {0, 0};
	time_t			t;

if (!gameOpts->render.cockpit.bObjectTally)
	return;
if (HIDE_HUD)
	return;
if (!IsMultiGame || IsCoopGame) {
	int	x, x0 = 0, y = 0, w, h, aw, i, bmW, bmH;
	char	szInfo [20];

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
		y = 3 * nHUDLineSpacing;
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		y = 2 * nHUDLineSpacing;
	else {//if (!SHOW_COCKPIT) {
		y = 2 * nHUDLineSpacing;
		if (gameStates.render.fonts.bHires)
			y += nHUDLineSpacing;
		}
	if (gameOpts->render.cockpit.bPlayerStats)
		y += 2 * nHUDLineSpacing;

	x0 = grdCurCanv->cv_w;
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	t = gameStates.app.nSDLTicks;
	if (t - t0 > 333) {	//update 3 times per second
		t0 = t;
		for (i = 0; i < 2; i++) 
			objCounts [i] = ObjectCount (i ? OBJ_POWERUP : OBJ_ROBOT);
		}
	if (!gameOpts->render.cockpit.bTextGauges && (LoadObjTallyIcons () > 0)) {
		for (i = 0; i < 2; i++) {
			bmH = bmObjTally [i].bmProps.h / 2;
			bmW = bmObjTally [i].bmProps.w / 2;
			x = x0 - bmW - HUD_LHX (2);
			OglUBitMapMC (x, y, bmW, bmH, bmObjTally + i, NULL, F1_0, 0);
			sprintf (szInfo, "%d", objCounts [i]);
			GrGetStringSize (szInfo, &w, &h, &aw);
			x -= w + HUD_LHY (2);
			nIdTally [i] = GrPrintF (nIdTally + i, x, y + (bmH - h) / 2, szInfo);
			y += bmH;
			}
		}
	else {
		y = 6 + 3 * nHUDLineSpacing;
		for (i = 0; i < 2; i++) {
			sprintf (szInfo, "%s: %5d", i ? "Powerups" : "Robots", objCounts [i]);
			GrGetStringSize (szInfo, &w, &h, &aw);
			nIdTally [i] = GrPrintF (nIdTally + i, grdCurCanv->cv_w - w - HUD_LHX (2), y, szInfo);
			y += nHUDLineSpacing;
			}
		}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowPlayerStats (void)
{
	int		h, w, aw, y;
	double	p [3], s [3];
	char		szStats [50];

	static int nIdStats = 0;

if (!gameOpts->render.cockpit.bPlayerStats)
	return;
if (HIDE_HUD)
	return;
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
	y = 3 * nHUDLineSpacing;
else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	y = 2 * nHUDLineSpacing;
else {//if (!SHOW_COCKPIT) {
	y = 2 * nHUDLineSpacing;
	if (gameStates.render.fonts.bHires)
		y += nHUDLineSpacing;
	}
GrSetFontColorRGBi (ORANGE_RGBA, 1, 0, 0);
y = 6 + 2 * nHUDLineSpacing;
h = (gameData.stats.nDisplayMode - 1) / 2;
if ((gameData.stats.nDisplayMode - 1) % 2 == 0) {
	sprintf (szStats, "%s%d-%d %d-%d %d-%d", 
				h ? "T:" : "", 
				gameData.stats.player [h].nHits [0],
				gameData.stats.player [h].nMisses [0],
				gameData.stats.player [h].nHits [1],
				gameData.stats.player [h].nMisses [1],
				gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nHits [1],
				gameData.stats.player [h].nMisses [0] + gameData.stats.player [h].nMisses [1]);
	}
else {
	s [0] = gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nMisses [0];
	s [1] = gameData.stats.player [h].nHits [1] + gameData.stats.player [h].nMisses [1];
	s [2] = s [0] + s [1];
	p [0] = s [0] ? (gameData.stats.player [h].nHits [0] / s [0]) * 100 : 0;
	p [1] = s [1] ? (gameData.stats.player [h].nHits [1] / s [1]) * 100 : 0;
	p [2] = s [2] ? ((gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nHits [1]) / s [2]) * 100 : 0;
	sprintf (szStats, "%s%1.1f%c %1.1f%c %1.1f%c", h ? "T:" : "", p [0], '%', p [1], '%', p [2], '%');
	}
GrGetStringSize (szStats, &w, &h, &aw);
nIdStats = GrString (grdCurCanv->cv_w - w - HUD_LHX (2), y, szStats, &nIdStats);
}

//	-----------------------------------------------------------------------------

void HUDToggleWeaponIcons (void)
{
	int	i;

for (i = 0; i < Controls [0].toggleIconsCount; i++)
	if (gameStates.app.bNostalgia)
		extraGameInfo [0].nWeaponIcons = 0;
	else {
		extraGameInfo [0].nWeaponIcons = (extraGameInfo [0].nWeaponIcons + 1) % 5;
		if (!gameOpts->render.weaponIcons.bEquipment && (extraGameInfo [0].nWeaponIcons == 3))
			extraGameInfo [0].nWeaponIcons = 4;
		}
}

//	-----------------------------------------------------------------------------

#define ICON_SCALE	3

void HUDShowWeaponIcons (void)
{
	grsBitmap	*bmP;
	int	nWeaponIcons = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? 3 : extraGameInfo [0].nWeaponIcons;
	int	nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nMode != CM_FULL_SCREEN)) ? 4 : 3;
	int	nIconPos = nWeaponIcons - 1;
	int	nMaxAutoSelect;
	int	fw, fh, faw, 
			i, j, ll, n, 
			ox = 6, 
			oy = 6, 
			x, dx, y = 0, dy = 0;
	ubyte	alpha = gameOpts->render.weaponIcons.alpha;
	unsigned int nAmmoColor;
	char	szAmmo [10];
	int	nLvlMap [2][10] = {{9, 4, 8, 3, 7, 2, 6, 1, 5, 0}, {4, 3, 2, 1, 0, 4, 3, 2, 1, 0}};
	static int	wIcon = 0, 
					hIcon = 0;
	static int	w = -1, 
					h = -1;
	static ubyte ammoType [2][10] = {{0, 1, 0, 0, 0, 0, 1, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
	static int bInitIcons = 1;
	static int nIdIcons [2][10] = {{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};

ll = LOCALPLAYER.laserLevel;
if (gameOpts->render.weaponIcons.bShowAmmo) {
	GrSetCurFont (SMALL_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	}
dx = (int) (10 * cmScaleX);
if (nWeaponIcons < 3) {
#if 0
	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
#endif
		dy = (grdCurScreen->scHeight - grdCurCanv->cvBitmap.bmProps.h);
		y = nIconPos ? grdCurScreen->scHeight - dy - oy : oy + hIcon + 12;
#if 0
		}
	else {
		y = (2 - gameStates.app.bD1Mission) * (oy + hIcon) + 12;
		nIconPos = 1;
		}
#endif
	}
for (i = 0; i < 2; i++) {
	n = (gameStates.app.bD1Mission) ? 5 : 10;
	nMaxAutoSelect = 255;
	if (nWeaponIcons > 2) {
		int h;
		if (gameStates.render.cockpit.nMode != CM_STATUS_BAR)
			h = 0;
		else {
#ifdef _DEBUG
			h = gameStates.render.cockpit.nMode + (gameStates.video.nDisplayMode ? gameData.models.nCockpits / 2 : 0);
			h = gameData.pig.tex.cockpitBmIndex [h].index;
			h = gameData.pig.tex.bitmaps [0][h].bmProps.h;
#else
			h = gameData.pig.tex.bitmaps [0][gameData.pig.tex.cockpitBmIndex [gameStates.render.cockpit.nMode + (gameStates.video.nDisplayMode ? gameData.models.nCockpits / 2 : 0)].index].bmProps.h;
#endif
			}
		y = (grdCurCanv->cvBitmap.bmProps.h - h - n * (hIcon + oy)) / 2 + hIcon;
		x = i ? grdCurScreen->scWidth - wIcon - ox : ox;
		}
	else {
		x = grdCurScreen->scWidth / 2;
		if (i)
			x += dx;
		else
			x -= dx + wIcon;
		}
	for (j = 0; j < n; j++) {
		int bArmed, bHave, bLoaded, l, m;

		if (gameOpts->render.weaponIcons.nSort && !gameStates.app.bD1Mission) {
			l = nWeaponOrder [i][j];
			if (l == 255)
				nMaxAutoSelect = j;
			l = nWeaponOrder [i][j + (j >= nMaxAutoSelect)];
			}
		else
			l = nLvlMap [gameStates.app.bD1Mission][j];
		m = i ? secondaryWeaponToWeaponInfo [l] : primaryWeaponToWeaponInfo [l];
		if ((gameData.pig.tex.nHamFileVersion >= 3) && gameStates.video.nDisplayMode) {
			bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].hires_picture.index;
			PIGGY_PAGE_IN (gameData.weapons.info [m].hires_picture.index, 0);
			}
		else {
			bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].picture.index;
			PIGGY_PAGE_IN (gameData.weapons.info [m].picture.index, 0);
			}
		Assert (bmP != NULL);
		if (w < bmP->bmProps.w)
			w = bmP->bmProps.w;
		if (h < bmP->bmProps.h)
			h = bmP->bmProps.h;
		wIcon = (int) ((w + nIconScale - 1) / nIconScale * cmScaleX);
		hIcon = (int) ((h + nIconScale - 1) / nIconScale * cmScaleY);
		if (bInitIcons)
			continue;
		if (i)
			bHave = (LOCALPLAYER.secondaryWeaponFlags & (1 << l));
		else if (!l) {
			bHave = (ll <= MAX_LASER_LEVEL);
			if (!bHave)
				continue;
			}
		else if (l == 5) {
			bHave = (ll > MAX_LASER_LEVEL);
			if (!bHave)
				continue;
			}
		else {
			bHave = (LOCALPLAYER.primaryWeaponFlags & (1 << l));
			if (bHave && extraGameInfo [0].bSmartWeaponSwitch && ((l == 1) || (l == 2)) &&
				 LOCALPLAYER.primaryWeaponFlags & (1 << (l + 5)))
				continue;
			}
		HUDBitBlt (nIconScale * - (x + (w - bmP->bmProps.w) / (2 * nIconScale)), 
					  nIconScale * - (y - hIcon), bmP, nIconScale * F1_0, 0);
		*szAmmo = '\0';
		nAmmoColor = GREEN_RGBA;
		if (ammoType [i][l]) {
			int nAmmo = (i ? LOCALPLAYER.secondaryAmmo [l] : LOCALPLAYER.primaryAmmo [(l == 6) ? 1 : l]);
			bLoaded = (nAmmo > 0);
			if (bHave) {
				if (bLoaded && gameOpts->render.weaponIcons.bShowAmmo) {
					if (!i && (l % 5 == 1)) {//Gauss/Vulcan
						nAmmo = f2i (nAmmo * (unsigned) VULCAN_AMMO_SCALE);
#if 0
						sprintf (szAmmo, "%d.%d", nAmmo / 1000, (nAmmo % 1000) / 100);
#else
						if (nAmmo && (nAmmo < 1000)) {
							sprintf (szAmmo, ".%d", nAmmo / 100);
							nAmmoColor = RED_RGBA;
							}
						else
							sprintf (szAmmo, "%d", nAmmo / 1000);
#endif
						}
					else
						sprintf (szAmmo, "%d", nAmmo);
					GrGetStringSize (szAmmo, &fw, &fh, &faw);
					}
				}
			}
		else {
			bLoaded = (LOCALPLAYER.energy > gameData.weapons.info [l].energy_usage);
			if (l == 0) {//Lasers
				sprintf (szAmmo, "%d", (ll > MAX_LASER_LEVEL) ? MAX_LASER_LEVEL + 1 : ll + 1);
				GrGetStringSize (szAmmo, &fw, &fh, &faw);
				}
			else if ((l == 5) && (ll > MAX_LASER_LEVEL)) {
				sprintf (szAmmo, "%d", ll - MAX_LASER_LEVEL);
				GrGetStringSize (szAmmo, &fw, &fh, &faw);
				}
			}
		if (i && !bLoaded)
			bHave = 0;
		if (bHave) {
			//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 3;
			if (bLoaded)
				GrSetColorRGB (128, 128, 0, (ubyte) (alpha * 16));
			else
				GrSetColorRGB (128, 0, 0, (ubyte) (alpha * 16));
			}
		else {
			//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 7;
			GrSetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
			}
		GrURect (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
		if (i) {
			if (j < 8)
				bArmed = (l == gameData.weapons.nSecondary);
			else
				bArmed = (j == 8) == (bLastSecondaryWasSuper [PROXMINE_INDEX] != 0);
			}
		else {
			if (l == 5)
				bArmed = (bHave && (0 == gameData.weapons.nPrimary));
			else if (l)
				bArmed = (l == gameData.weapons.nPrimary);
			else
				bArmed = (bHave && (l == gameData.weapons.nPrimary));
			}
		if (bArmed)
			if (bLoaded)
				GrSetColorRGB (255, 192, 0, 255);
			else
				GrSetColorRGB (160, 0, 0, 255);
		else if (bHave)
			if (bLoaded)
				GrSetColorRGB (0, 160, 0, 255);
			else
				GrSetColorRGB (96, 0, 0, 255);
		else
			GrSetColorRGB (64, 64, 64, 255);
		GrUBox (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
		if (*szAmmo) {
			GrSetFontColorRGBi (nAmmoColor, 1, 0, 0);
			nIdIcons [i][j] = GrString (x + wIcon + 2 - fw, y - fh, szAmmo, nIdIcons [i] + j);
			GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
			}
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
		if (nWeaponIcons > 2)
			y += hIcon + oy;
		else {
			if (i)
				x += wIcon + ox;
			else
				x -= wIcon + ox;
			}
		}
	}
bInitIcons = 0;
}

//	-----------------------------------------------------------------------------

int LoadInventoryIcons (void)
{
	int	h, i;

if (!((bmpInventory = PiggyLoadBitmap ("inventry.bmp")) ||
	   (bmpInventory = PiggyLoadBitmap ("inventory.bmp"))))
	return bHaveInvBms = 0;
memset (bmInvItems, 0, sizeof (bmInvItems));
h = bmpInventory->bmProps.w * bmpInventory->bmProps.w;
for (i = 0; i < NUM_INV_ITEMS; i++) {
	bmInvItems [i] = *bmpInventory;
	bmInvItems [i].bmProps.h = bmInvItems [i].bmProps.w;
	bmInvItems [i].bmTexBuf += h * i;
	bmInvItems [i].bmPalette = gamePalette;
	}
return bHaveInvBms = 1;
}

//	-----------------------------------------------------------------------------

void FreeInventoryIcons (void)
{
if (bmpInventory) {
	GrFreeBitmap (bmpInventory);
	bmpInventory = NULL;
	bHaveInvBms = -1;
	}
}

//	-----------------------------------------------------------------------------

int HUDEquipmentActive (int bFlag)
{
switch (bFlag) {
	case PLAYER_FLAGS_AFTERBURNER:
		return (gameData.physics.xAfterburnerCharge && Controls [0].afterburnerState);
	case PLAYER_FLAGS_CONVERTER:
		return gameStates.app.bUsingConverter;
	case PLAYER_FLAGS_HEADLIGHT:
		return HeadLightIsOn (-1) != 0;
	case PLAYER_FLAGS_FULLMAP:
		return 0;
	case PLAYER_FLAGS_AMMO_RACK:
		return 0;
	case PLAYER_FLAGS_QUAD_LASERS:
		return 0;
	case PLAYER_FLAGS_CLOAKED:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) != 0;
	case PLAYER_FLAGS_INVULNERABLE:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) != 0;
	case PLAYER_FLAGS_SLOWMOTION:
		return SlowMotionActive ();
	case PLAYER_FLAGS_BULLETTIME:
		return BulletTimeActive ();
	}
return 0;
}

//	-----------------------------------------------------------------------------

void HUDShowInventoryIcons (void)
{
	grsBitmap	*bmP;
	char	szCount [4];
	int nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nMode != CM_FULL_SCREEN)) ? 3 : 2;
	int nIconPos = extraGameInfo [0].nWeaponIcons & 1;
	int	fw, fh, faw;
	int	j, n, firstItem, 
			oy = 6, 
			ox = 6, 
			x, y, dy;
	int	w = bmpInventory->bmProps.w, 
			h = bmpInventory->bmProps.w;
	int	wIcon = (int) ((w + nIconScale - 1) / nIconScale * cmScaleX), 
			hIcon = (int) ((h + nIconScale - 1) / nIconScale * cmScaleY);
	ubyte	alpha = gameOpts->render.weaponIcons.alpha;
	static int nInvFlags [NUM_INV_ITEMS] = {
		PLAYER_FLAGS_AFTERBURNER, 
		PLAYER_FLAGS_CONVERTER, 
		PLAYER_FLAGS_HEADLIGHT, 
		PLAYER_FLAGS_FULLMAP, 
		PLAYER_FLAGS_AMMO_RACK, 
		PLAYER_FLAGS_QUAD_LASERS, 
		PLAYER_FLAGS_CLOAKED, 
		PLAYER_FLAGS_INVULNERABLE,
		PLAYER_FLAGS_SLOWMOTION,
		PLAYER_FLAGS_BULLETTIME
		};
	static int nEnergyType [NUM_INV_ITEMS] = {F1_0, 100 * F1_0, 0, F1_0, 0, F1_0, 0, 0, F1_0, F1_0};
	static int nIdItems [NUM_INV_ITEMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

dy = (grdCurScreen->scHeight - grdCurCanv->cvBitmap.bmProps.h);
if (gameStates.render.cockpit.nMode != CM_STATUS_BAR) //(!SHOW_COCKPIT)
	y = nIconPos ? grdCurScreen->scHeight - dy - oy : oy + hIcon + 12;
else
	y = oy + hIcon + 12;
n = (gameOpts->gameplay.bInventory && (!IsMultiGame || IsCoopGame)) ? NUM_INV_ITEMS : NUM_INV_ITEMS - 2;
firstItem = gameStates.app.bD1Mission ? INV_ITEM_QUADLASERS : 0;
x = (grdCurScreen->scWidth - (n - firstItem) * wIcon - (n - 1 - firstItem) * ox) / 2;
for (j = firstItem; j < n; j++) {
	int bHave, bArmed, bActive = HUDEquipmentActive (nInvFlags [j]);
	bmP = bmInvItems + j;
	HUDBitBlt (nIconScale * - (x + (w - bmP->bmProps.w) / (2 * nIconScale)), nIconScale * - (y - hIcon), bmP, nIconScale * F1_0, 0);
	//m = 9 - j;
	*szCount = '\0';
	if (j == INV_ITEM_HEADLIGHT)
		bHave = PlayerHasHeadLight (-1);
	else if (j == INV_ITEM_INVUL) {
		if ((bHave = (LOCALPLAYER.nInvuls > 0)))
			sprintf (szCount, "%d", LOCALPLAYER.nInvuls);
		else
			bHave = LOCALPLAYER.flags & nInvFlags [j];
		}
	else if (j == INV_ITEM_CLOAK) {
		if ((bHave = (LOCALPLAYER.nCloaks > 0)))
			sprintf (szCount, "%d", LOCALPLAYER.nCloaks);
		else
			bHave = LOCALPLAYER.flags & nInvFlags [j];
		}
	else
		bHave = LOCALPLAYER.flags & nInvFlags [j];
	bArmed = (LOCALPLAYER.energy > nEnergyType [j]);
	if (bHave) {
		//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 3;
		if (bArmed)
			if (bActive)
				GrSetColorRGB (255, 208, 0, (ubyte) (alpha * 16));
			else
				GrSetColorRGB (128, 128, 0, (ubyte) (alpha * 16));
		else
			GrSetColorRGB (128, 0, 0, (ubyte) (alpha * 16));
		}
	else {
		//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 7;
		GrSetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
		}
	GrURect (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
	if (bHave)
		if (bArmed)
			if (bActive)
				GrSetColorRGB (255, 208, 0, 255);
			else
				GrSetColorRGB (0, 160, 0, 255);
		else
			GrSetColorRGB (96, 0, 0, 255);
	else
		GrSetColorRGB (64, 64, 64, 255);
	GrUBox (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
	if (*szCount) {
		GrGetStringSize (szCount, &fw, &fh, &faw);
		nIdItems [j] = GrString (x + wIcon + 2 - fw, y - fh, szCount, nIdItems + j);
		}
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	x += wIcon + ox;
	}
}

//	-----------------------------------------------------------------------------

void HUDShowIcons (void)
{
if (gameStates.app.bNostalgia)
	return;
if (gameStates.app.bEndLevelSequence)
	return;
HUDToggleWeaponIcons ();
if (gameOpts->render.cockpit.bHUD || SHOW_COCKPIT) {
	HUDShowPlayerStats ();
	HUDShowObjTally ();
	if (!gameStates.app.bDemoData && EGI_FLAG (nWeaponIcons, 0, 1, 0)) {
		cmScaleX *= HUD_ASPECT;
		HUDShowWeaponIcons ();
		if (gameOpts->render.weaponIcons.bEquipment) {
			if (bHaveInvBms < 0)
				LoadInventoryIcons ();
			if (bHaveInvBms > 0)
				HUDShowInventoryIcons ();
			}
		cmScaleX /= HUD_ASPECT;
		}
	}
}

//	-----------------------------------------------------------------------------
