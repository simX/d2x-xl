/* $Id: paging.c,v 1.3 2003/10/04 03:14:47 btb Exp $ */
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

//#define PSX_BUILD_TOOLS

#ifdef RCS
static char rcsid [] = "$Id: paging.c,v 1.3 2003/10/04 03:14:47 btb Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pstypes.h"
#include "mono.h"
#include "inferno.h"
#include "segment.h"
#include "textures.h"
#include "wall.h"
#include "object.h"
#include "gamemine.h"
#include "error.h"
#include "gameseg.h"
#include "game.h"
#include "piggy.h"
#include "texmerge.h"
#include "polyobj.h"
#include "vclip.h"
#include "effects.h"
#include "fireball.h"
#include "weapon.h"
#include "palette.h"
#include "timer.h"
#include "text.h"
#include "cntrlcen.h"
#include "gauges.h"
#include "powerup.h"
#include "fuelcen.h"
#include "mission.h"
#include "menu.h"
#include "newmenu.h"
#include "gamesave.h"
#include "gamepal.h"

//------------------------------------------------------------------------------

void PagingTouchVClip (tVideoClip * vc, int bD1)
{
	int i;

	for (i=0; i<vc->nFrameCount; i++)	{
		PIGGY_PAGE_IN (vc->frames [i], bD1);
	}
}

//------------------------------------------------------------------------------

void PagingTouchWallEffects (int nTexture)
{
	int	i;
	tEffectClip *ecP = gameData.eff.pEffects;

for (i = gameData.eff.nEffects [gameStates.app.bD1Data]; i; i--, ecP++) {
	if (ecP->changingWallTexture == nTexture) {
		PagingTouchVClip (&ecP->vc, gameStates.app.bD1Data);
		if (ecP->nDestBm > -1)
			PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [ecP->nDestBm], gameStates.app.bD1Data);	//use this bitmap when monitor destroyed
		if (ecP->dest_vclip > -1)
			PagingTouchVClip (&gameData.eff.pVClips [ecP->dest_vclip], gameStates.app.bD1Data);		  //what tVideoClip to play when exploding
		if (ecP->dest_eclip > -1)
			PagingTouchVClip (&gameData.eff.pEffects [ecP->dest_eclip].vc, gameStates.app.bD1Data); //what tEffectClip to play when exploding
		if (ecP->crit_clip > -1)
			PagingTouchVClip (&gameData.eff.pEffects [ecP->crit_clip].vc, gameStates.app.bD1Data); //what tEffectClip to play when mine critical
		}
	}
}

//------------------------------------------------------------------------------

void PagingTouchObjectEffects (int nTexture)
{
	int	i;
	tEffectClip *ecP = gameData.eff.pEffects;

for (i = gameData.eff.nEffects [gameStates.app.bD1Data]; i; i--, ecP++)
	if (ecP->changingObjectTexture == nTexture)
		PagingTouchVClip (&ecP->vc, 0);
}

//------------------------------------------------------------------------------

void PagingTouchModel (int modelnum)
{
	int		i, j;
	ushort	*pi;
	tPolyModel *pm = gameData.models.polyModels + modelnum;

for (i = pm->nTextures, pi = gameData.pig.tex.pObjBmIndex + pm->nFirstTexture; i; i--, pi++) {
	j = *pi;
	PIGGY_PAGE_IN (gameData.pig.tex.objBmIndex [j], 0);
	PagingTouchObjectEffects (j);
	}
}

//------------------------------------------------------------------------------

void PagingTouchWeapon (int nWeaponType)
{
	// Page in the robot's weapons.
	
	if ((nWeaponType < 0) || (nWeaponType > gameData.weapons.nTypes [0])) return;

	if (gameData.weapons.info [nWeaponType].picture.index)	{
		PIGGY_PAGE_IN (gameData.weapons.info [nWeaponType].picture,0);
	}		
	
	if (gameData.weapons.info [nWeaponType].flash_vclip > -1)
		PagingTouchVClip (&gameData.eff.vClips [0][gameData.weapons.info [nWeaponType].flash_vclip], 0);
	if (gameData.weapons.info [nWeaponType].wall_hit_vclip > -1)
		PagingTouchVClip (&gameData.eff.vClips [0][gameData.weapons.info [nWeaponType].wall_hit_vclip], 0);
	if (WI_damage_radius (nWeaponType))	{
		// Robot_hit_vclips are actually badass_vclips
		if (gameData.weapons.info [nWeaponType].robot_hit_vclip > -1)
			PagingTouchVClip (&gameData.eff.vClips [0][gameData.weapons.info [nWeaponType].robot_hit_vclip], 0);
	}

	switch (gameData.weapons.info [nWeaponType].renderType)	{
	case WEAPON_RENDER_VCLIP:
		if (gameData.weapons.info [nWeaponType].nVClipIndex > -1)
			PagingTouchVClip (&gameData.eff.vClips [0][gameData.weapons.info [nWeaponType].nVClipIndex], 0);
		break;
	case WEAPON_RENDER_NONE:
		break;
	case WEAPON_RENDER_POLYMODEL:
		PagingTouchModel (gameData.weapons.info [nWeaponType].nModel);
		break;
	case WEAPON_RENDER_BLOB:
		PIGGY_PAGE_IN (gameData.weapons.info [nWeaponType].bitmap, 0);
		break;
	}
}

//------------------------------------------------------------------------------

sbyte super_boss_gateType_list [13] = {0, 1, 8, 9, 10, 11, 12, 15, 16, 18, 19, 20, 22 };

void PagingTouchRobot (int robot_index)
{
	int i;

	// Page in robot_index
	PagingTouchModel (ROBOTINFO (robot_index).nModel);
	if (ROBOTINFO (robot_index).nExp1VClip>-1)
		PagingTouchVClip (&gameData.eff.vClips [0][ROBOTINFO (robot_index).nExp1VClip], 0);
	if (ROBOTINFO (robot_index).nExp2VClip>-1)
		PagingTouchVClip (&gameData.eff.vClips [0][ROBOTINFO (robot_index).nExp2VClip], 0);

	// Page in his weapons
	PagingTouchWeapon (ROBOTINFO (robot_index).nWeaponType);

	// A super-boss can gate in robots...
	if (ROBOTINFO (robot_index).bossFlag==2)	{
		for (i=0; i<13; i++)
			PagingTouchRobot (super_boss_gateType_list [i]);

		PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_MORPHING_ROBOT], 0);
	}
}

//------------------------------------------------------------------------------

void PagingTouchObject (tObject *objP)
{
	int v;

	switch (objP->renderType) {

		case RT_NONE:	break;		//doesn't render, like the tPlayer

		case RT_POLYOBJ:
			if (objP->rType.polyObjInfo.nTexOverride == -1)
				PagingTouchModel (objP->rType.polyObjInfo.nModel);
			else
				PIGGY_PAGE_IN (gameData.pig.tex.bmIndex [0][objP->rType.polyObjInfo.nTexOverride], 0);
			break;

		case RT_POWERUP:
			if ((objP->rType.vClipInfo.nClipIndex > -1) && 
				 (objP->rType.vClipInfo.nClipIndex < gameData.eff.nClips [0])) {
				}
			else
				objP->rType.vClipInfo.nClipIndex = -1;
			break;

		case RT_MORPH:	break;
		case RT_FIREBALL: break;
		case RT_THRUSTER: break;
		case RT_WEAPON_VCLIP: break;

		case RT_HOSTAGE:
			PagingTouchVClip (&gameData.eff.vClips [0][objP->rType.vClipInfo.nClipIndex], 0);
			break;

		case RT_LASER: break;
 	}

	switch (objP->nType) {	
	case OBJ_PLAYER:	
		v = GetExplosionVClip (objP, 0);
		if (v > -1)
			PagingTouchVClip (&gameData.eff.vClips [0][v], 0);
		break;
	case OBJ_ROBOT:
		PagingTouchRobot (objP->id);
		break;
	case OBJ_CNTRLCEN:
		PagingTouchWeapon (CONTROLCEN_WEAPON_NUM);
		if (gameData.models.nDeadModels [objP->rType.polyObjInfo.nModel] != -1)	{
			PagingTouchModel (gameData.models.nDeadModels [objP->rType.polyObjInfo.nModel]);
		}
		break;
	}
}

//------------------------------------------------------------------------------

void PagingTouchSide (tSegment * segp, short nSide)
{
	int tmap1, tmap2;

	if ((segp - gameData.segs.segments == 208) && (nSide == 4))
		tmap1 = segp->sides [nSide].nBaseTex;
	if (!(WALL_IS_DOORWAY (segp,nSide, NULL) & WID_RENDER_FLAG))
		return;
	
	tmap1 = segp->sides [nSide].nBaseTex;
	PagingTouchWallEffects (tmap1);
	tmap2 = segp->sides [nSide].nOvlTex;
	if (tmap2) {
		PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [tmap2], gameStates.app.bD1Data);
		PagingTouchWallEffects (tmap2);
		}
	else
		PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [tmap1], gameStates.app.bD1Data);

	// PSX STUFF
	#ifdef PSX_BUILD_TOOLS
		// If there is water on the level, then force the water splash into memory
		if (!(gameData.pig.tex.pTMapInfo [tmap1].flags & TMI_VOLATILE) && (gameData.pig.tex.pTMapInfo [tmap1].flags & TMI_WATER)) {
			tBitmapIndex Splash;
			Splash.index = 1098;
			PIGGY_PAGE_IN (Splash);
			Splash.index = 1099;
			PIGGY_PAGE_IN (Splash);
			Splash.index = 1100;
			PIGGY_PAGE_IN (Splash);
			Splash.index = 1101;
			PIGGY_PAGE_IN (Splash);
			Splash.index = 1102;
			PIGGY_PAGE_IN (Splash);
		}
	#endif

}

//------------------------------------------------------------------------------

void PagingTouchRobotMaker (tSegment * segp)
{
	tSegment2	*seg2p = &gameData.segs.segment2s [SEG_IDX (segp)];

	if (seg2p->special == SEGMENT_IS_ROBOTMAKER)	{
		PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_MORPHING_ROBOT], 0);
		if (gameData.matCens.botGens [seg2p->nMatCen].objFlags != 0) {
			int	i;
			uint	flags;
			int	robot_index;

			for (i=0;i<2;i++) {
				robot_index = i*32;
				flags = gameData.matCens.botGens [seg2p->nMatCen].objFlags [i];
				while (flags) {
					if (flags & 1)	{
						// Page in robot_index
						PagingTouchRobot (robot_index);
					}
					flags >>= 1;
					robot_index++;
				}
			}
		}
	}
}


//------------------------------------------------------------------------------

void PagingTouchSegment (tSegment * segp)
{
	short sn, nObject;
	tSegment2	*seg2p = &gameData.segs.segment2s [SEG_IDX (segp)];

	if (seg2p->special == SEGMENT_IS_ROBOTMAKER)	
		PagingTouchRobotMaker (segp);

//	paging_draw_orb ();
	for (sn=0;sn<MAX_SIDES_PER_SEGMENT;sn++) {
//		paging_draw_orb ();
		PagingTouchSide (segp, sn);
	}

	for (nObject=segp->objects;nObject!=-1;nObject=gameData.objs.objects [nObject].next)	{
//		paging_draw_orb ();
		PagingTouchObject (&gameData.objs.objects [nObject]);
	}
}

//------------------------------------------------------------------------------

void PagingTouchWall (tWall *wallP)
{
	int	j;
	tWallClip *anim;

if (wallP->nClip > -1)	{
	anim = gameData.walls.pAnims + wallP->nClip;
	for (j=0; j < anim->nFrameCount; j++)
		PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [anim->frames [j]], gameStates.app.bD1Data);
	}
}

//------------------------------------------------------------------------------

void PagingTouchWalls ()
{
	int i;

for (i=0;i<gameData.walls.nWalls;i++)
	PagingTouchWall (gameData.walls.walls + i);
}

//------------------------------------------------------------------------------

void PagingTouchSegments (void)
{
	int	s;

for (s=0; s < gameData.segs.nSegments; s++)
	PagingTouchSegment (gameData.segs.segments + s);
}

//------------------------------------------------------------------------------

void PagingTouchPowerups (void)
{
	int	s;

for (s=0; s < gameData.objs.pwrUp.nTypes; s++)
	if (gameData.objs.pwrUp.info [s].nClipIndex > -1)	
		PagingTouchVClip (&gameData.eff.vClips [0][gameData.objs.pwrUp.info [s].nClipIndex], 0);
}

//------------------------------------------------------------------------------

void PagingTouchWeapons (void)
{
	int	s;

for (s = 0; s < gameData.weapons.nTypes [0]; s++)
	PagingTouchWeapon (s);
}

//------------------------------------------------------------------------------

void PagingTouchGauges (void)
{
	int	s;

for (s = 0; s < MAX_GAUGE_BMS; s++)
	if (gameData.cockpit.gauges [1] [s].index)
		PIGGY_PAGE_IN (gameData.cockpit.gauges [1] [s], 0);
}

//------------------------------------------------------------------------------

void PagingTouchAllSub ()
{
	int 			bBlackScreen;
	
	StopTime ();
	bBlackScreen = gameStates.render.bPaletteFadedOut;
	if (gameStates.render.bPaletteFadedOut)	{
		GrClearCanvas (BLACK_RGBA);
		GrPaletteStepLoad (NULL);
	}
	
//	ShowBoxedMessage (TXT_LOADING);

#if TRACE				
	con_printf (CON_VERBOSE, "Loading all textures in mine...");
#endif
	PagingTouchSegments ();
	PagingTouchWalls ();
	PagingTouchPowerups ();
	PagingTouchWeapons ();
	PagingTouchPowerups ();
	PagingTouchGauges ();
	PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_PLAYER_APPEARANCE], 0);
	PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_POWERUP_DISAPPEARANCE], 0);


#ifdef PSX_BUILD_TOOLS

	//PSX STUFF
	PagingTouchWalls ();
	for (s=0; s<=gameData.objs.nLastObject; s++) {
		PagingTouchObject (gameData.objs.objects + s);
	}


	{
		char * p;
		extern int gameData.missions.nCurrentLevel;
		extern ushort gameData.pig.tex.bitmapXlat [MAX_BITMAP_FILES];
		short bmUsed [MAX_BITMAP_FILES];
		FILE * fp;
		char fname [128];
		int i, bPageIn;
		grsBitmap *bmP;

		if (gameData.missions.nCurrentLevel<0)                //secret level
			strcpy (fname, gameData.missions.szSecretLevelNames [-gameData.missions.nCurrentLevel-1]);
		else                                    //normal level
			strcpy (fname, gameData.missions.szLevelNames [gameData.missions.nCurrentLevel-1]);
		p = strchr (fname, '.');
		if (p) *p = 0;
		strcat (fname, ".pag");

		fp = fopen (fname, "wt");
		for (i=0; i<MAX_BITMAP_FILES;i++)      {
			bmUsed [i] = 0;
		}
		for (i=0; i<MAX_BITMAP_FILES;i++)      {
			bmUsed [gameData.pig.tex.bitmapXlat [i]]++;
		}

		//cmp added so that .damage bitmaps are included for paged-in lights of the current level
		for (i=0; i<MAX_TEXTURES;i++) {
			if (gameData.pig.tex.pBmIndex [i].index > 0 && gameData.pig.tex.pBmIndex [i].index < MAX_BITMAP_FILES &&
				bmUsed [gameData.pig.tex.pBmIndex [i].index] > 0 &&
				gameData.pig.tex.pTMapInfo [i].destroyed > 0 && gameData.pig.tex.pTMapInfo [i].destroyed < MAX_BITMAP_FILES) {
				bmUsed [gameData.pig.tex.pBmIndex [gameData.pig.tex.pTMapInfo [i].destroyed].index] += 1;
				PIGGY_PAGE_IN (gameData.pig.tex.pBmIndex [gameData.pig.tex.pTMapInfo [i].destroyed]);

			}
		}

		//	Force cockpit to be paged in.
		{
			tBitmapIndex bonk;
			bonk.index = 109;
			PIGGY_PAGE_IN (bonk);
		}

		// Force in the frames for markers
		{
			tBitmapIndex bonk2;
			bonk2.index = 2014;
			PIGGY_PAGE_IN (bonk2);
			bonk2.index = 2015;
			PIGGY_PAGE_IN (bonk2);
			bonk2.index = 2016;
			PIGGY_PAGE_IN (bonk2);
			bonk2.index = 2017;
			PIGGY_PAGE_IN (bonk2);
			bonk2.index = 2018;
			PIGGY_PAGE_IN (bonk2);
		}

		for (i = 0, bmP = gameData.pig.tex.pBitmaps; i < MAX_BITMAP_FILES; i++, bmP++) {
			bPageIn = 1;
			// cmp debug
			//piggy_get_bitmap_name (i,fname);

			if (!bmP->bm_texBuf || (bmP->bm_props.flags & BM_FLAG_PAGED_OUT))
				bPageIn = 0;
//                      if (gameData.pig.tex.bitmapXlat [i]!=i)
//                              bPageIn = 0;

			if (!bmUsed [i])
				bPageIn = 0;
			if ((i==47) || (i==48))               // Mark red mplayer ship textures as paged in.
				bPageIn = 1;
			if (!bPageIn)
				fprintf (fp, "0,\t// Bitmap %d (%s)\n", i, "test\0"); // cmp debug fname);
			else
				fprintf (fp, "1,\t// Bitmap %d (%s)\n", i, "test\0"); // cmp debug fname);
		}

		fclose (fp);
	}
#endif

#if TRACE				
	con_printf (CON_VERBOSE, "... loading all textures in mine done\n");
#endif
//@@	ClearBoxedMessage ();

	if (bBlackScreen)	{
		GrPaletteStepClear ();
		GrClearCanvas (BLACK_RGBA);
	}
	StartTime ();
	ResetCockpit ();		//force cockpit redraw next time

}

//------------------------------------------------------------------------------

int PagingGaugeSize (void)
{
return PROGRESS_STEPS (gameData.segs.nSegments) + 
		 PROGRESS_STEPS (gameFileInfo.walls.count) +
		 PROGRESS_STEPS (gameData.objs.pwrUp.nTypes) * 2 +
		 PROGRESS_STEPS (gameData.weapons.nTypes [0]) + 
		 PROGRESS_STEPS (MAX_GAUGE_BMS);
}

//------------------------------------------------------------------------------

static int nTouchSeg = 0;
static int nTouchWall = 0;
static int nTouchWeapon = 0;
static int nTouchPowerup1 = 0;
static int nTouchPowerup2 = 0;
static int nTouchGauge = 0;

static void PagingTouchPoll (int nItems, tMenuItem *m, int *key, int cItem)
{
	int	i;

GrPaletteStepLoad (NULL);
if (nTouchSeg < gameData.segs.nSegments) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchSeg < gameData.segs.nSegments); i++)
		PagingTouchSegment (gameData.segs.segments + nTouchSeg++);
	}
else if (nTouchWall < gameData.walls.nWalls) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchWall < gameData.walls.nWalls); i++)
		PagingTouchWall (gameData.walls.walls + nTouchWall++);
	}
else if (nTouchPowerup1 < gameData.objs.pwrUp.nTypes) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchPowerup1 < gameData.objs.pwrUp.nTypes); i++, nTouchPowerup1++)
		if (gameData.objs.pwrUp.info [nTouchPowerup1].nClipIndex > -1)	
			PagingTouchVClip (&gameData.eff.vClips [0][gameData.objs.pwrUp.info [nTouchPowerup1].nClipIndex], 0);
	}
else if (nTouchWeapon < gameData.weapons.nTypes [0]) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchWeapon < gameData.weapons.nTypes [0]); i++)
		PagingTouchWeapon (nTouchWeapon++);
	}
else if (nTouchPowerup2 < gameData.objs.pwrUp.nTypes) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchPowerup2 < gameData.objs.pwrUp.nTypes); i++, nTouchPowerup2++)
		if (gameData.objs.pwrUp.info [nTouchPowerup2].nClipIndex > -1)	
			PagingTouchVClip (&gameData.eff.vClips [0][gameData.objs.pwrUp.info [nTouchPowerup2].nClipIndex], 0);
	}
else if (nTouchGauge < MAX_GAUGE_BMS) {
	for (i = 0; (i < PROGRESS_INCR) && (nTouchGauge < MAX_GAUGE_BMS); i++, nTouchGauge++)
		if (gameData.cockpit.gauges [1] [nTouchGauge].index)
			PIGGY_PAGE_IN (gameData.cockpit.gauges [1] [nTouchGauge], 0);
	}
else {
	PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_PLAYER_APPEARANCE], 0);
	PagingTouchVClip (&gameData.eff.vClips [0][VCLIP_POWERUP_DISAPPEARANCE], 0);
	*key = -2;
	GrPaletteStepLoad (NULL);
	return;
	}
m [0].value++;
m [0].rebuild = 1;
*key = 0;
GrPaletteStepLoad (NULL);
return;
}

//------------------------------------------------------------------------------

void PagingTouchAll ()
{
if (gameStates.app.bProgressBars && gameOpts->menus.nStyle) {
		int	i = LoadMineGaugeSize ();

	nTouchSeg = 0;
	nTouchWall = 0;
	nTouchWeapon = 0;
	nTouchPowerup1 = 0;
	nTouchPowerup2 = 0;
	nTouchGauge = 0;
	NMProgressBar (TXT_PREP_DESCENT, i, i + PagingGaugeSize () + SortLightsGaugeSize (), PagingTouchPoll); 
	}
else
	PagingTouchAllSub ();
}

//------------------------------------------------------------------------------
