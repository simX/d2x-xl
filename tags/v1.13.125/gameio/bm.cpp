/* $Id: bm.c,v 1.41 2003/11/03 12:03:43 btb Exp $ */
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
#include <stdlib.h>
#include <string.h>

#include "inferno.h"
#include "pstypes.h"
#include "strutil.h"
#include "text.h"
#include "gr.h"
#include "ogl_defs.h"
#include "bm.h"
#include "u_mem.h"
#include "mono.h"
#include "error.h"
#include "object.h"
#include "vclip.h"
#include "effects.h"
#include "polyobj.h"
#include "wall.h"
#include "textures.h"
#include "game.h"
#include "multi.h"
#include "iff.h"
#include "cfile.h"
#include "powerup.h"
#include "sounds.h"
#include "piggy.h"
#include "aistruct.h"
#include "robot.h"
#include "weapon.h"
#include "gauges.h"
#include "player.h"
#include "endlevel.h"
#include "reactor.h"
#include "makesig.h"
#include "interp.h"
#include "light.h"
#include "byteswap.h"

ubyte Sounds [2][MAX_SOUNDS];
ubyte AltSounds [2][MAX_SOUNDS];

#ifdef EDITOR
int gameData.objs.types.nCount;
byte	gameData.objs.types.nType [MAX_OBJTYPE];
byte	gameData.objs.types.nType.nId [MAX_OBJTYPE];
fix	gameData.objs.types.nType.nStrength [MAX_OBJTYPE];
#endif

//right now there's only one tPlayer ship, but we can have another by
//adding an array and setting the pointer to the active ship.

//---------------- Variables for tWall textures ------------------

int ComputeAvgPixel (grsBitmap *newBm)
{
	int	row, column, color, size;
	char	*pptr;
	int	total_red, total_green, total_blue;
	ubyte	*palette;

pptr = (char *)newBm->bmTexBuf;
total_red = 0;
total_green = 0;
total_blue = 0;
palette = newBm->bmPalette;
for (row = 0; row < newBm->bmProps.h; row++)
	for (column = 0; column < newBm->bmProps.w; column++) {
		color = *pptr++;
		total_red += palette [color*3];
		total_green += palette [color*3+1];
		total_blue += palette [color*3+2];
		}
size = newBm->bmProps.h * newBm->bmProps.w * 2;
total_red /= size;
total_green /= size;
total_blue /= size;
return GrFindClosestColor (palette, total_red, total_green, total_blue);
}

//---------------- Variables for tObject textures ----------------

#if 0//def FAST_FILE_IO /*disabled for a reason!*/
#define ReadTMapInfoN(ti, n, fp) CFRead (ti, sizeof (tTexMapInfo), n, fp)
#else
/*
 * reads n tTexMapInfo structs from a CFILE
 */
int ReadTMapInfoN (tTexMapInfo *ti, int n, CFILE *fp)
{
	int i;

for (i = 0;i < n;i++) {
	ti [i].flags = CFReadByte (fp);
	ti [i].pad [0] = CFReadByte (fp);
	ti [i].pad [1] = CFReadByte (fp);
	ti [i].pad [2] = CFReadByte (fp);
	ti [i].lighting = CFReadFix (fp);
	ti [i].damage = CFReadFix (fp);
	ti [i].nEffectClip = CFReadShort (fp);
	ti [i].destroyed = CFReadShort (fp);
	ti [i].slide_u = CFReadShort (fp);
	ti [i].slide_v = CFReadShort (fp);
	}
return i;
}
#endif

//------------------------------------------------------------------------------

int ReadTMapInfoND1 (tTexMapInfo *ti, int n, CFILE *fp)
{
	int i;

for (i = 0;i < n;i++) {
	CFSeek (fp, 13, SEEK_CUR);// skip filename
	ti [i].flags = CFReadByte (fp);
	ti [i].lighting = CFReadFix (fp);
	ti [i].damage = CFReadFix (fp);
	ti [i].nEffectClip = CFReadInt (fp);
	}
return i;
}

//-----------------------------------------------------------------
// Read data from piggy.
// This is called when the editor is OUT.
// If editor is in, bm_init_use_table () is called.
int BMInit ()
{
	int	i;

for (i = 0; i < MAX_OBJECTS; i++)
	gameData.weapons.color [i].red =
	gameData.weapons.color [i].green =
	gameData.weapons.color [i].blue = 1.0;
InitPolygonModels ();
if (!PiggyInit ())				// This calls BMReadAll
	Error ("Cannot open pig and/or ham file");
/*---*/PrintLog ("   Loading sound data\n");
PiggyReadSounds ();
/*---*/PrintLog ("   Initializing endlevel data\n");
InitEndLevel ();		//this is in bm_init_use_tbl (), so I gues it goes here
return 0;
}

//------------------------------------------------------------------------------

void BMSetAfterburnerSizes (void)
{
	sbyte	nSize = gameData.weapons.info [MERCURYMSL_ID].afterburner_size;

//gameData.weapons.info [VULCAN_ID].afterburner_size = 
//gameData.weapons.info [GAUSS_ID].afterburner_size = nSize / 8;
gameData.weapons.info [CONCUSSION_ID].afterburner_size =
gameData.weapons.info [HOMINGMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_CONCUSSION_ID].afterburner_size =
gameData.weapons.info [FLASHMSL_ID].afterburner_size =
gameData.weapons.info [GUIDEDMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_FLASHMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_MEGA_FLASHMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_MERCURYMSL_ID].afterburner_size = nSize;
gameData.weapons.info [ROBOT_HOMINGMSL_ID].afterburner_size =
gameData.weapons.info [SMARTMSL_ID].afterburner_size = 2 * nSize;
gameData.weapons.info [MEGAMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_MEGAMSL_ID].afterburner_size =
gameData.weapons.info [ROBOT_SHAKER_MEGA_ID].afterburner_size =
gameData.weapons.info [EARTHSHAKER_MEGA_ID].afterburner_size = 3 * nSize;
gameData.weapons.info [EARTHSHAKER_ID].afterburner_size =
gameData.weapons.info [ROBOT_EARTHSHAKER_ID].afterburner_size = 4 * nSize;
}

//------------------------------------------------------------------------------

void QSortTextureIndex (short *pti, short left, short right)
{
	short	l = left,
			r = right,
			m = pti [(left + right) / 2],
			h;

do {
	while (pti [l] < m)
		l++;
	while (pti [r] > m)
		r--;
	if (l <= r) {
		if (l < r) {
			h = pti [l];
			pti [l] = pti [r];
			pti [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortTextureIndex (pti, l, right);
if (left < r)
	QSortTextureIndex (pti, left, r);
}

//------------------------------------------------------------------------------

void BuildTextureIndex (int i, int n)
{
	short				*pti = gameData.pig.tex.textureIndex [i];
	tBitmapIndex	*pbi = gameData.pig.tex.bmIndex [i];

memset (pti, 0xff, sizeof (gameData.pig.tex.textureIndex [i]));
for (i = 0; i < n; i++)
	pti [pbi [i].index] = i;
//QSortTextureIndex (pti, 0, n - 1);
}

//------------------------------------------------------------------------------

void BMReadAll (CFILE * fp)
{
	int i, t;

gameData.pig.tex.nTextures [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d texture indices\n", gameData.pig.tex.nTextures [0]);
BitmapIndexReadN (gameData.pig.tex.bmIndex [0], gameData.pig.tex.nTextures [0], fp);
BuildTextureIndex (0, gameData.pig.tex.nTextures [0]);
ReadTMapInfoN (gameData.pig.tex.tMapInfo [0], gameData.pig.tex.nTextures [0], fp);

t = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d sound indices\n", t);
CFRead (Sounds [0], sizeof (ubyte), t, fp);
CFRead (AltSounds [0], sizeof (ubyte), t, fp);

gameData.eff.nClips [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d animation clips\n", gameData.eff.nClips [0]);
VClipReadN (gameData.eff.vClips [0], gameData.eff.nClips [0], fp);

gameData.eff.nEffects [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d animation descriptions\n", gameData.eff.nEffects [0]);
EClipReadN (gameData.eff.effects [0], gameData.eff.nEffects [0], fp);
// red glow texture animates way too fast
gameData.eff.effects [0][32].vc.xTotalTime *= 10;
gameData.eff.effects [0][32].vc.xFrameTime *= 10;
gameData.walls.nAnims [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d tWall animations\n", gameData.walls.nAnims [0]);
WClipReadN (gameData.walls.anims [0], gameData.walls.nAnims [0], fp);

gameData.bots.nTypes [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d robot descriptions\n", gameData.bots.nTypes [0]);
RobotInfoReadN (gameData.bots.info [0], gameData.bots.nTypes [0], fp);
gameData.bots.nDefaultTypes = gameData.bots.nTypes [0];
memcpy (gameData.bots.defaultInfo, gameData.bots.info [0], gameData.bots.nTypes [0] * sizeof (*gameData.bots.defaultInfo));

gameData.bots.nJoints = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d robot joint descriptions\n", gameData.bots.nJoints);
JointPosReadN (gameData.bots.joints, gameData.bots.nJoints, fp);
gameData.bots.nDefaultJoints = gameData.bots.nJoints;
memcpy (gameData.bots.defaultJoints, gameData.bots.joints, gameData.bots.nJoints * sizeof (*gameData.bots.defaultJoints));

gameData.weapons.nTypes [0] = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d weapon descriptions\n", gameData.weapons.nTypes [0]);
WeaponInfoReadN (gameData.weapons.info, gameData.weapons.nTypes [0], fp, gameData.pig.tex.nHamFileVersion);
BMSetAfterburnerSizes ();

gameData.objs.pwrUp.nTypes = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d powerup descriptions\n", gameData.objs.pwrUp.nTypes);
PowerupTypeInfoReadN (gameData.objs.pwrUp.info, gameData.objs.pwrUp.nTypes, fp);

gameData.models.nPolyModels = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d tPolyModel descriptions\n", gameData.models.nPolyModels);
PolyModelReadN (gameData.models.polyModels, gameData.models.nPolyModels, fp);
gameData.models.nDefPolyModels = gameData.models.nPolyModels;
memcpy (gameData.models.defPolyModels, gameData.models.polyModels, gameData.models.nPolyModels * sizeof (*gameData.models.defPolyModels));

/*---*/PrintLog ("      Loading poly model data\n");
for (i = 0; i < gameData.models.nPolyModels; i++) {
	gameData.models.polyModels [i].modelData = 
	gameData.models.defPolyModels [i].modelData = NULL;
	PolyModelDataRead (gameData.models.polyModels + i, i, gameData.models.defPolyModels + i, fp);
	}

for (i = 0; i < gameData.models.nPolyModels; i++)
	gameData.models.nDyingModels [i] = CFReadInt (fp);
for (i = 0; i < gameData.models.nPolyModels; i++)
	gameData.models.nDeadModels [i] = CFReadInt (fp);

t = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d cockpit gauges\n", t);
BitmapIndexReadN (gameData.cockpit.gauges [1], t, fp);
BitmapIndexReadN (gameData.cockpit.gauges [0], t, fp);

gameData.pig.tex.nObjBitmaps = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d tObject bitmap indices\n", gameData.pig.tex.nObjBitmaps);
BitmapIndexReadN (gameData.pig.tex.objBmIndex, gameData.pig.tex.nObjBitmaps, fp);
for (i = 0; i < gameData.pig.tex.nObjBitmaps; i++)
	gameData.pig.tex.pObjBmIndex [i] = CFReadShort (fp);

/*---*/PrintLog ("      Loading tPlayer ship description\n");
PlayerShipRead (&gameData.pig.ship.only, fp);

gameData.models.nCockpits = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d cockpit bitmaps\n", gameData.models.nCockpits);
BitmapIndexReadN (gameData.pig.tex.cockpitBmIndex, gameData.models.nCockpits, fp);
gameData.pig.tex.nFirstMultiBitmap = CFReadInt (fp);

gameData.reactor.nReactors = CFReadInt (fp);
/*---*/PrintLog ("      Loading %d reactor descriptions\n", gameData.reactor.nReactors);
ReactorReadN (gameData.reactor.props, gameData.reactor.nReactors, fp);

gameData.models.nMarkerModel = CFReadInt (fp);
if (gameData.pig.tex.nHamFileVersion < 3) {
	gameData.endLevel.exit.nModel = CFReadInt (fp);
	gameData.endLevel.exit.nDestroyedModel = CFReadInt (fp);
	}
else
	gameData.endLevel.exit.nModel = 
	gameData.endLevel.exit.nDestroyedModel = gameData.models.nPolyModels;
}

//------------------------------------------------------------------------------

#if 0

static int	nTMaps [2] = {0,0};

#define LINEBUF_SIZE 600

#define REMOVE_EOL (s)		remove_char ((s),'\n')
#define REMOVE_COMMENTS (s)	remove_char ((s),';')
#define REMOVE_DOTS (s)  	remove_char ((s),'.')


inline void remove_char (char * s, char c)
{
	char *p;

if (p = strchr (s, c)) 
	*p = '\0';
}


void BMReadInfoFile (void)
{
	int i, bHaveBinTbl = 0;
	char	szInput [LINEBUF_SIZE];
	CFILE	infoFile;

if (!(CFOpen (&infoFile, "BITMAPS.TBL", gameFolders.szDataDir, "rb", 0) || 
	   CFOpen (&infoFile, "BITMAPS.BIN", gameFolders.szDataDir, "rb", 0)))
	Error ("Missing BITMAPS.TBL and BITMAPS.BIN file\n");
	bHaveBinTbl = 1;
	}
linenum = 0;

CFSeek (&infoFile, 0L, SEEK_SET);

while (CFGetS (szInput, LINEBUF_SIZE, &infoFile)) {
	int l;
	char *temp_ptr;
	linenum++;
	if (szInput [0]==' ' || szInput [0]=='\t') {
		char *t;
		for (t=szInput;*t && *t!='\n';t++)
			if (!(*t==' ' || *t=='\t')) {
#if TRACE
				con_printf (1,"Suspicious: line %d of BITMAPS.TBL starts with whitespace\n",linenum);
#endif
				break;
			}
	}

	if (bHaveBinTbl) {				// is this a binary tbl file
		for (i = 0; i < strlen (szInput) - 1; i++) {
			szInput [i] = EncodeRotateLeft (EncodeRotateLeft (szInput + i) ^ BITMAP_TBL_XOR);
		}
	} else {
		while (szInput [ (l=strlen (szInput))-2]=='\\') {
			if (!isspace (szInput [l-3])) {		//if not space before backslash...
				szInput [l-2] = ' ';				//add one
				l++;
			}
			CFGetS (szInput+l-2,LINEBUF_SIZE- (l-2), &infoFile);
			linenum++;
		}
	}

	REMOVE_EOL (szInput);
	if (strchr (szInput, ';')!=NULL) REMOVE_COMMENTS (szInput);

	if (strlen (szInput) == LINEBUF_SIZE-1)
		Warning ("Possible line truncation in BITMAPS.TBL on line %d\n",linenum);

	arg = strtok (szInput, space);
	if (arg [0] == '@') {
		arg++;
		Registered_only = 1;
	} else
		Registered_only = 0;

	while (arg != NULL)
		{
		// Check all possible flags and defines.
		if (*arg == '$') bmFlag = BM_NONE; // reset to no flags as default.

		IFTOK ("$COCKPIT") 			bmFlag = BM_COCKPIT;
		else IFTOK ("$GAUGES")		{bmFlag = BM_GAUGES;   clipCount = 0;}
		else IFTOK ("$GAUGES_HIRES"){bmFlag = BM_GAUGES_HIRES; clipCount = 0;}
		else IFTOK ("$SOUND") 		bm_readSound ();
		else IFTOK ("$DOOR_ANIMS")	bmFlag = BM_WALL_ANIMS;
		else IFTOK ("$WALL_ANIMS")	bmFlag = BM_WALL_ANIMS;
		else IFTOK ("$TEXTURES") 	bmFlag = BM_TEXTURES;
		else IFTOK ("$VCLIP")			{bmFlag = BM_VCLIP;		vlighting = 0;	clipCount = 0;}
		else IFTOK ("$ECLIP")			{bmFlag = BM_ECLIP;		vlighting = 0;	clipCount = 0; obj_eclip=0; dest_bm=NULL; dest_vclip=-1; dest_eclip=-1; dest_size=-1; crit_clip=-1; critFlag=0; nSound=-1;}
		else IFTOK ("$WCLIP")			{bmFlag = BM_WCLIP;		vlighting = 0;	clipCount = 0; wall_explodes = wall_blastable = 0; wall_openSound=wall_closeSound=-1; tmap1Flag=0; wall_hidden=0;}

		else IFTOK ("$EFFECTS")		{bmFlag = BM_EFFECTS;	nClip = 0;}
		else IFTOK ("$ALIAS")			bm_read_alias ();

		else IFTOK ("lighting") 			gameData.pig.tex.pTMapInfo [textureCount-1].lighting = F2X (get_float ();
		else IFTOK ("damage") 			gameData.pig.tex.pTMapInfo [textureCount-1].damage = F2X (get_float ();
		else IFTOK ("volatile") 			gameData.pig.tex.pTMapInfo [textureCount-1].flags |= TMI_VOLATILE;
		else IFTOK ("goal_blue")			gameData.pig.tex.pTMapInfo [textureCount-1].flags |= TMI_GOAL_BLUE;
		else IFTOK ("goal_red")			gameData.pig.tex.pTMapInfo [textureCount-1].flags |= TMI_GOAL_RED;
		else IFTOK ("water")	 			gameData.pig.tex.pTMapInfo [textureCount-1].flags |= TMI_WATER;
		else IFTOK ("force_field")		gameData.pig.tex.pTMapInfo [textureCount-1].flags |= TMI_FORCE_FIELD;
		else IFTOK ("slide")	 			{gameData.pig.tex.pTMapInfo [textureCount-1].slide_u = F2X (get_float ())>>8; gameData.pig.tex.pTMapInfo [textureCount-1].slide_v = F2X (get_float ())>>8;}
		else IFTOK ("destroyed")	 		{int t=textureCount-1; gameData.pig.tex.pTMapInfo [t].destroyed = get_texture (strtok (NULL, space);}
		//else IFTOK ("gameData.eff.nEffects")		gameData.eff.nEffects = get_int ();
		else IFTOK ("gameData.walls.nAnims")	gameData.walls.nAnims = get_int ();
		else IFTOK ("nClip")			nClip = get_int ();
		else IFTOK ("dest_bm")			dest_bm = strtok (NULL, space);
		else IFTOK ("dest_vclip")		dest_vclip = get_int ();
		else IFTOK ("dest_eclip")		dest_eclip = get_int ();
		else IFTOK ("dest_size")			dest_size = F2X (get_float ();
		else IFTOK ("crit_clip")			crit_clip = get_int ();
		else IFTOK ("critFlag")			critFlag = get_int ();
		else IFTOK ("nSound") 		nSound = get_int ();
		else IFTOK ("frames") 			frames = get_int ();
		else IFTOK ("time") 				time = get_float ();
		else IFTOK ("obj_eclip")			obj_eclip = get_int ();
		else IFTOK ("hitSound") 		hitSound = get_int ();
		else IFTOK ("abmFlag")			abmFlag = get_int ();
		else IFTOK ("tmap1Flag")		tmap1Flag = get_int ();
		else IFTOK ("vlighting")			vlighting = get_float ();
		else IFTOK ("rodFlag")			rodFlag = get_int ();
		else IFTOK ("superx") 			get_int ();
		else IFTOK ("openSound") 		wall_openSound = get_int ();
		else IFTOK ("closeSound") 		wall_closeSound = get_int ();
		else IFTOK ("explodes")	 		wall_explodes = get_int ();
		else IFTOK ("blastable")	 		wall_blastable = get_int ();
		else IFTOK ("hidden")	 			wall_hidden = get_int ();
		else IFTOK ("$ROBOT_AI") 		bm_readRobot_ai ();

		else IFTOK ("$POWERUP")			{bm_read_powerup (0);		continue;}
		else IFTOK ("$POWERUP_UNUSED")	{bm_read_powerup (1);		continue;}
		else IFTOK ("$HOSTAGE")			{bm_read_hostage ();		continue;}
		else IFTOK ("$ROBOT")				{bm_readRobot ();			continue;}
		else IFTOK ("$WEAPON")			{bm_read_weapon (0);		continue;}
		else IFTOK ("$WEAPON_UNUSED")	{bm_read_weapon (1);		continue;}
		else IFTOK ("$REACTOR")			{bm_read_reactor ();		continue;}
		else IFTOK ("$MARKER")			{bm_read_marker ();		continue;}
		else IFTOK ("$PLAYER_SHIP")		{bm_read_player_ship ();	continue;}
		else IFTOK ("$EXIT") {
				bm_read_exitmodel ();
			continue;
		}
		else	{		//not a special token, must be a bitmap!

			// Remove any illegal/unwanted spaces and tabs at this point.
			while ((*arg=='\t') || (*arg==' ')) arg++;
			if (*arg == '\0') { break; }

			//check for '=' in token, indicating error
			if (strchr (arg,'='))
				Warning ("Unknown token <'%s'> on line %d of BITMAPS.TBL",arg,linenum);

			// Otherwise, 'arg' is apparently a bitmap filename.
			// Load bitmap and process it below:
			bm_read_some_file ();

		}

		arg = strtok (NULL, equal_space);
		continue;
   }
}

gameData.pig.tex.nTextures = textureCount;
nTMaps [gameStates.app.bD1Data] = tmapCount;

gameData.pig.tex.pBmIndex [gameData.pig.tex.nTextures++].index = 0;		//entry for bogus tmap

CFClose (&infoFile);
}

#endif

void BMReadWeaponInfoD1N (CFILE * fp, int i)
{
	tD1WeaponInfo	*wiP = gameData.weapons.infoD1 + i;

wiP->renderType = CFReadByte (fp);
wiP->nModel = CFReadByte (fp);
wiP->nInnerModel = CFReadByte (fp);
wiP->persistent = CFReadByte (fp);
wiP->nFlashVClip = CFReadByte (fp);
wiP->flashSound = CFReadShort (fp);
wiP->robot_hit_vclip = CFReadByte (fp);
wiP->robot_hitSound = CFReadShort (fp);
wiP->wall_hit_vclip = CFReadByte (fp);
wiP->wall_hitSound = CFReadShort (fp);
wiP->fireCount = CFReadByte (fp);
wiP->ammo_usage = CFReadByte (fp);
wiP->nVClipIndex = CFReadByte (fp);
wiP->destroyable = CFReadByte (fp);
wiP->matter = CFReadByte (fp);
wiP->bounce = CFReadByte (fp);
wiP->homingFlag = CFReadByte (fp);
wiP->dum1 = CFReadByte (fp); 
wiP->dum2 = CFReadByte (fp);
wiP->dum3 = CFReadByte (fp);
wiP->energy_usage = CFReadFix (fp);
wiP->fire_wait = CFReadFix (fp);
wiP->bitmap.index = CFReadShort (fp);
wiP->blob_size = CFReadFix (fp);
wiP->xFlashSize = CFReadFix (fp);
wiP->impact_size = CFReadFix (fp);
for (i = 0; i < NDL; i++)
	wiP->strength [i] = CFReadFix (fp);
for (i = 0; i < NDL; i++)
	wiP->speed [i] = CFReadFix (fp);
wiP->mass = CFReadFix (fp);
wiP->drag = CFReadFix (fp);
wiP->thrust = CFReadFix (fp);
wiP->po_len_to_width_ratio = CFReadFix (fp);
wiP->light = CFReadFix (fp);
wiP->lifetime = CFReadFix (fp);
wiP->damage_radius = CFReadFix (fp);
wiP->picture.index = CFReadShort (fp);

}

//------------------------------------------------------------------------------
// the following values are needed for compiler settings causing struct members 
// not to be byte aligned. If D2X-XL is compiled with such settings, the size of
// the Descent data structures in memory is bigger than on disk. This needs to
// be compensated when reading such data structures from disk, or needing to skip
// them in the disk files.

#define D1_ROBOT_INFO_SIZE			486
#define D1_WEAPON_INFO_SIZE		115
#define JOINTPOS_SIZE				8
#define JOINTLIST_SIZE				4
#define POWERUP_TYPE_INFO_SIZE	16
#define POLYMODEL_SIZE				734
#define PLAYER_SHIP_SIZE			132
#define MODEL_DATA_SIZE_OFFS		4

void BMReadGameDataD1 (CFILE *cfp)
{
	int				h, i, j;
#if 1
	tD1WallClip		w;
	D1_tmap_info	t;
	//D1Robot_info	r;
#endif
	tWallClip				*pw;
	tTexMapInfo		*pt;
	tRobotInfo		*pr;
	tPolyModel		pm;
	ubyte				tmpSounds [D1_MAX_SOUNDS];

CFSeek (cfp, sizeof (int), SEEK_CUR);
CFRead (&gameData.pig.tex.nTextures [1], sizeof (int), 1, cfp);
j = (gameData.pig.tex.nTextures [1] == 70) ? 70 : D1_MAX_TEXTURES;
/*---*/PrintLog ("         Loading %d texture indices\n", j);
//CFRead (gameData.pig.tex.bmIndex [1], sizeof (tBitmapIndex), D1_MAX_TEXTURES, cfp);
BitmapIndexReadN (gameData.pig.tex.bmIndex [1], D1_MAX_TEXTURES, cfp);
BuildTextureIndex (1, D1_MAX_TEXTURES);
/*---*/PrintLog ("         Loading %d texture descriptions\n", j);
for (i = 0, pt = &gameData.pig.tex.tMapInfo [1][0]; i < j; i++, pt++) {
	CFSeek (cfp, sizeof (t.filename), SEEK_CUR);
	pt->flags = (ubyte) CFReadByte (cfp);
	pt->lighting = CFReadFix (cfp);
	pt->damage = CFReadFix (cfp);
	pt->nEffectClip = CFReadInt (cfp);
	pt->slide_u = 
	pt->slide_v = 0;
	pt->destroyed = -1;
	}
CFRead (Sounds [1], sizeof (ubyte), D1_MAX_SOUNDS, cfp);
CFRead (AltSounds [1], sizeof (ubyte), D1_MAX_SOUNDS, cfp);
/*---*/PrintLog ("         Initializing %d sounds\n", D1_MAX_SOUNDS);
if (gameOpts->sound.bUseD1Sounds) {
memcpy (Sounds [1] + D1_MAX_SOUNDS, Sounds [0] + D1_MAX_SOUNDS, MAX_SOUNDS - D1_MAX_SOUNDS);
	memcpy (AltSounds [1] + D1_MAX_SOUNDS, AltSounds [0] + D1_MAX_SOUNDS, MAX_SOUNDS - D1_MAX_SOUNDS);
	}
else {
	memcpy (Sounds [1], Sounds [0], MAX_SOUNDS);
	memcpy (AltSounds [1], AltSounds [0], MAX_SOUNDS);
	}
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (Sounds [1][i] == 255)
		Sounds [1][i] = Sounds [0][i];
	if (AltSounds [1][i] == 255)
		AltSounds [1][i] = AltSounds [0][i];
	}
gameData.eff.nClips [1] = CFReadInt (cfp);
/*---*/PrintLog ("         Loading %d animation clips\n", gameData.eff.nClips [1]);
VClipReadN (gameData.eff.vClips [1], D1_VCLIP_MAXNUM, cfp);
gameData.eff.nEffects [1] = CFReadInt (cfp);
/*---*/PrintLog ("         Loading %d animation descriptions\n", gameData.eff.nClips [1]);
EClipReadN (gameData.eff.effects [1], D1_MAX_EFFECTS, cfp);
gameData.walls.nAnims [1] = CFReadInt (cfp);
/*---*/PrintLog ("         Loading %d tWall animations\n", gameData.walls.nAnims [1]);
for (i = 0, pw = &gameData.walls.anims [1][0]; i < D1_MAX_WALL_ANIMS; i++, pw++) {
	//CFRead (&w, sizeof (w), 1, cfp);
	pw->xTotalTime = CFReadFix (cfp);
	pw->nFrameCount = CFReadShort (cfp);
	for (j = 0; j < D1_MAX_CLIP_FRAMES; j++)
		pw->frames [j] = CFReadShort (cfp);
	pw->openSound = CFReadShort (cfp);
	pw->closeSound = CFReadShort (cfp);
	pw->flags = CFReadShort (cfp);
	CFRead (pw->filename, sizeof (w.filename), 1, cfp);
	pw->pad = (char) CFReadByte (cfp);
	}
CFRead (&gameData.bots.nTypes [1], sizeof (int), 1, cfp);
/*---*/PrintLog ("         Loading %d robot descriptions\n", gameData.bots.nTypes [1]);
memcpy (gameData.bots.info [1], gameData.bots.info [0], MAX_ROBOT_TYPES * sizeof (tRobotInfo));
if (!gameOpts->sound.bUseD1Sounds)
	return;
#if 1
for (i = 0, pr = &gameData.bots.info [1][0]; i < D1_MAX_ROBOT_TYPES; i++, pr++) {
	//CFRead (&r, sizeof (r), 1, cfp);
	CFSeek (cfp,
		sizeof (int) * 3 + 
		 (sizeof (vmsVector) + sizeof (ubyte)) * MAX_GUNS + 
		sizeof (short) * 5 +
		sizeof (sbyte) * 7 +
		sizeof (fix) * 4 +
		sizeof (fix) * 7 * NDL +
		sizeof (sbyte) * 2 * NDL,
		SEEK_CUR);

	pr->seeSound = (ubyte) CFReadByte (cfp);
	pr->attackSound = (ubyte) CFReadByte (cfp);
	pr->clawSound = (ubyte) CFReadByte (cfp);
	CFSeek (cfp,
		JOINTLIST_SIZE * (MAX_GUNS + 1) * N_ANIM_STATES +
		sizeof (int),
		SEEK_CUR);
#if 0
	pr->tauntSound = 0;
	pr->nModel = r.nModel;
	memcpy (pr->gunPoints, r.gunPoints, sizeof (r.gunPoints));
	memcpy (pr->gunSubModels, r.gunSubModels, sizeof (r.gunSubModels));
	pr->nExp1VClip = r.nExp1VClip;
	pr->nExp1Sound = r.nExp1Sound;
	pr->nExp2VClip = r.nExp2VClip;
	pr->nExp2Sound = r.nExp2Sound;
	pr->nWeaponType = r.nWeaponType;
	pr->nSecWeaponType = 0;
	pr->nGuns = r.nGuns;
	pr->containsId = r.containsId;
	pr->containsCount = r.containsCount;
	pr->containsProb = r.containsProb;
	pr->containsType = r.containsType;
	pr->kamikaze = 0;
	pr->scoreValue = r.scoreValue;
	pr->badass = 0;
	pr->energyDrain = 0;
	pr->lighting = r.lighting;
	pr->strength = r.strength;
	pr->mass = r.mass;
	pr->drag = r.drag;
	memcpy (pr->fieldOfView, r.fieldOfView, sizeof (r.fieldOfView));
	memcpy (pr->primaryFiringWait, r.primaryFiringWait, sizeof (r.primaryFiringWait));
	memset (pr->secondaryFiringWait, 0, sizeof (pr->secondaryFiringWait));
	memcpy (pr->turnTime, r.turnTime, sizeof (r.turnTime));
	memcpy (pr->xMaxSpeed, r.xMaxSpeed, sizeof (r.xMaxSpeed));
	memcpy (pr->circleDistance, r.circleDistance, sizeof (r.circleDistance));
	memcpy (pr->nRapidFireCount, r.nRapidFireCount, sizeof (r.nRapidFireCount));
	memcpy (pr->evadeSpeed, r.evadeSpeed, sizeof (r.evadeSpeed));
	pr->cloakType = r.cloakType;
	pr->attackType = r.attackType;
	pr->bossFlag = r.bossFlag;
	pr->companion = 0;
	pr->smartBlobs = 0;
	pr->energyBlobs = 0;
	pr->thief = 0;
	pr->pursuit = 0;
	pr->lightcast = 0;
	pr->bDeathRoll = r.bDeathRoll;
	pr->flags = r.flags;
	pr->deathrollSound = r.deathrollSound;
	pr->glow = r.glow;
	pr->behavior = r.behavior;
	pr->aim = r.aim;
	memcpy (pr->animStates, r.animStates, sizeof (r.animStates));
#endif
	pr->always_0xabcd = 0xabcd;   
	}         
#endif
#if 1
CFSeek (cfp, 
	sizeof (int) +
	JOINTPOS_SIZE * D1_MAX_ROBOT_JOINTS +
	sizeof (int) +
	D1_WEAPON_INFO_SIZE * D1_MAX_WEAPON_TYPES +
	sizeof (int) +
	POWERUP_TYPE_INFO_SIZE * MAX_POWERUP_TYPES_D1,
	SEEK_CUR);
i = CFReadInt (cfp);
/*---*/PrintLog ("         Acquiring model data size of %d polymodels\n", i);
for (h = 0; i; i--) {
	CFSeek (cfp, MODEL_DATA_SIZE_OFFS, SEEK_CUR);
	pm.nDataSize = CFReadInt (cfp);
	h += pm.nDataSize;
	CFSeek (cfp, POLYMODEL_SIZE - MODEL_DATA_SIZE_OFFS - sizeof (int), SEEK_CUR);
	}
CFSeek (cfp, 
	h +
	sizeof (tBitmapIndex) * D1_MAX_GAUGE_BMS +
	sizeof (int) * 2 * D1_MAX_POLYGON_MODELS +
	sizeof (tBitmapIndex) * D1_MAX_OBJ_BITMAPS +
	sizeof (ushort) * D1_MAX_OBJ_BITMAPS +
	PLAYER_SHIP_SIZE +
	sizeof (int) +
	sizeof (tBitmapIndex) * D1_N_COCKPIT_BITMAPS,
	SEEK_CUR);
/*---*/PrintLog ("         Loading sound data\n", i);
CFRead (tmpSounds, sizeof (ubyte), D1_MAX_SOUNDS, cfp);
//for (i = 0, pr = &gameData.bots.info [1][0]; i < gameData.bots.nTypes [1]; i++, pr++) 
pr = gameData.bots.info [1] + 17;
/*---*/PrintLog ("         Initializing sound data\n", i);
for (i = 0; i < D1_MAX_SOUNDS; i++)	{
	if (Sounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (Sounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (Sounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
pr = gameData.bots.info [1] + 23;
for (i = 0; i < D1_MAX_SOUNDS; i++)	{
	if (Sounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (Sounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (Sounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
CFRead (tmpSounds, sizeof (ubyte), D1_MAX_SOUNDS, cfp);
//	for (i = 0, pr = &gameData.bots.info [1][0]; i < gameData.bots.nTypes [1]; i++, pr++) {
pr = gameData.bots.info [1] + 17;
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (AltSounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
pr = gameData.bots.info [1] + 23;
for (i = 0; i < D1_MAX_SOUNDS; i++)	{
	if (AltSounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
#else
CFSeek (cfp, 
	sizeof (int) +
	D1_ROBOT_INFO_SIZE * D1_MAX_ROBOT_TYPES +
	sizeof (int) +
	JOINTPOS_SIZE * D1_MAX_ROBOT_JOINTS,
	SEEK_CUR);
gameData.weapons.nTypes [1] = CFReadInt (cfp);
/*---*/PrintLog ("         Loading %d weapon descriptions\n", gameData.weapons.nTypes [1]);
for (i = 0; i < gameData.weapons.nTypes [1]; i++) 
	BMReadWeaponInfoD1N (cfp, i);
#endif
}

//------------------------------------------------------------------------------

void BMReadWeaponInfoD1 (CFILE * fp)
{
	int	i;

	CFSeek (
		fp, 
		sizeof (int) +
		sizeof (int) +
		sizeof (tBitmapIndex) * D1_MAX_TEXTURES +
		sizeof (D1_tmap_info) * D1_MAX_TEXTURES +
		sizeof (ubyte) * D1_MAX_SOUNDS +
		sizeof (ubyte) * D1_MAX_SOUNDS +
		sizeof (int) +
		sizeof (tVideoClip) * D1_VCLIP_MAXNUM +
		sizeof (int) +
		sizeof (D1_eclip) * D1_MAX_EFFECTS +
		sizeof (int) +
		sizeof (tD1WallClip) * D1_MAX_WALL_ANIMS +
		sizeof (int) +
		sizeof (D1Robot_info) * D1_MAX_ROBOT_TYPES +
		sizeof (int) +
		sizeof (tJointPos) * D1_MAX_ROBOT_JOINTS,
		SEEK_CUR);
	gameData.weapons.nTypes [1] = CFReadInt (fp);
	for (i = 0; i < gameData.weapons.nTypes [1]; i++)
		BMReadWeaponInfoD1N (fp, i);
#if 0	//write the important data to a C source file for hard coding the values into D2X-W32
	{
	int	i, j;
	FILE	*f;

	f = fopen ("d:/projekte/d2x-w32/d2d1-weaponinfo.c", "wt");
	fprintf (f, "D2D1_weapon_info weaponInfoD2D1 [D1_MAX_WEAPON_TYPES] = {\r\n");
	for (i = 0;i < D1_MAX_WEAPON_TYPES;i++) {
		fprintf (f, "{%d,%d,%d,%d,%d,%d,%d,%d,%d,{",
			gameData.weapons.infoD1 [i].persistent,
			gameData.weapons.infoD1 [i].fireCount,
			gameData.weapons.infoD1 [i].ammo_usage,
			gameData.weapons.infoD1 [i].destroyable,
			gameData.weapons.infoD1 [i].matter,	
			gameData.weapons.infoD1 [i].bounce,	
			gameData.weapons.infoD1 [i].homingFlag,
			gameData.weapons.infoD1 [i].energy_usage,
			gameData.weapons.infoD1 [i].fire_wait);
		for (j = 0;j < NDL;j++)
			fprintf (f, "%s%d", j ? "," : "", 
				gameData.weapons.infoD1 [i].strength [j]);
		fprintf (f, "},{");
		for (j = 0;j < NDL;j++)
			fprintf (f, "%s%d", j ? "," : "", 
				gameData.weapons.infoD1 [i].speed [j]);
		fprintf (f, "},%d,%d,%d,%d,%d,%d}%s",
			gameData.weapons.infoD1 [i].mass,	
			gameData.weapons.infoD1 [i].drag,	
			gameData.weapons.infoD1 [i].thrust,
			gameData.weapons.infoD1 [i].light,
			gameData.weapons.infoD1 [i].lifetime,
			gameData.weapons.infoD1 [i].damage_radius,
			 (i < D1_MAX_WEAPON_TYPES - 1) ? ",\r\n" : "};\r\n");
		}
	fclose (f);
	}
#endif
#if 0
	CFRead (&gameData.pig.tex.nTextures, sizeof (int), 1, fp);
	CFRead (gameData.pig.tex.bmIndex, sizeof (tBitmapIndex), MAX_TEXTURES, fp);
	CFRead (gameData.pig.tex.tMapInfo, sizeof (tTexMapInfo), MAX_TEXTURES, fp);

	CFRead (Sounds [0], sizeof (ubyte), MAX_SOUNDS, fp);
	CFRead (AltSounds, sizeof (ubyte), MAX_SOUNDS, fp);

	CFRead (&gameData.eff.nClips, sizeof (int), 1, fp);
	CFRead (gameData.eff.vClips, sizeof (tVideoClip), VCLIP_MAXNUM, fp);

	CFRead (&gameData.eff.nEffects, sizeof (int), 1, fp);
	CFRead (gameData.eff.effects, sizeof (tEffectClip), MAX_EFFECTS, fp);

	CFRead (&gameData.walls.nAnims, sizeof (int), 1, fp);
	CFRead (gameData.walls.anims, sizeof (tWallClip), MAX_WALL_ANIMS, fp);

	CFRead (&gameData.bots.nTypes, sizeof (int), 1, fp);
	CFRead (gameData.bots.info [0], sizeof (tRobotInfo), MAX_ROBOT_TYPES, fp);

	CFRead (&gameData.bots.nJoints, sizeof (int), 1, fp);
	CFRead (gameData.bots.joints, sizeof (tJointPos), MAX_ROBOT_JOINTS, fp);

	CFRead (&gameData.weapons.nTypes [0], sizeof (int), 1, fp);
	CFRead (gameData.weapons.info, sizeof (tWeaponInfo), MAX_WEAPON_TYPES, fp);

	CFRead (&gameData.objs.pwrUp.nTypes, sizeof (int), 1, fp);
	CFRead (gameData.objs.pwrUp.info, sizeof (tPowerupTypeInfo), MAX_POWERUP_TYPES, fp);

	CFRead (&gameData.models.nPolyModels, sizeof (int), 1, fp);
	CFRead (gameData.models.polyModels, sizeof (tPolyModel), gameData.models.nPolyModels, fp);

	for (i=0;i<gameData.models.nPolyModels;i++)	{
		gameData.models.polyModels [i].modelData = D2_ALLOC (gameData.models.polyModels [i].nDataSize);
		Assert (gameData.models.polyModels [i].modelData != NULL);
		CFRead (gameData.models.polyModels [i].modelData, sizeof (ubyte), gameData.models.polyModels [i].nDataSize, fp);
	}

	CFRead (gameData.cockpit.gauges [1], sizeof (tBitmapIndex), MAX_GAUGE_BMS, fp);

	CFRead (gameData.models.nDyingModels, sizeof (int), MAX_POLYGON_MODELS, fp);
	CFRead (gameData.models.nDeadModels, sizeof (int), MAX_POLYGON_MODELS, fp);

	CFRead (gameData.pig.tex.objBmIndex, sizeof (tBitmapIndex), MAX_OBJ_BITMAPS, fp);
	CFRead (gameData.pig.tex.pObjBmIndex, sizeof (ushort), MAX_OBJ_BITMAPS, fp);

	CFRead (&gameData.pig.ship.only, sizeof (tPlayerShip), 1, fp);

	CFRead (&gameData.models.nCockpits, sizeof (int), 1, fp);
	CFRead (gameData.pig.tex.cockpitBmIndex, sizeof (tBitmapIndex), N_COCKPIT_BITMAPS, fp);

	CFRead (Sounds [0], sizeof (ubyte), MAX_SOUNDS, fp);
	CFRead (AltSounds, sizeof (ubyte), MAX_SOUNDS, fp);

	CFRead (&gameData.objs.types.nCount, sizeof (int), 1, fp);
	CFRead (gameData.objs.types.nType, sizeof (byte), MAX_OBJTYPE, fp);
	CFRead (gameData.objs.types.nType.nId, sizeof (byte), MAX_OBJTYPE, fp);
	CFRead (gameData.objs.types.nType.nStrength, sizeof (fix), MAX_OBJTYPE, fp);

	CFRead (&gameData.pig.tex.nFirstMultiBitmap, sizeof (int), 1, fp);

	CFRead (&N_controlcen_guns, sizeof (int), 1, fp);
	CFRead (controlcen_gun_points, sizeof (vmsVector), MAX_CONTROLCEN_GUNS, fp);
	CFRead (controlcen_gun_dirs, sizeof (vmsVector), MAX_CONTROLCEN_GUNS, fp);
	CFRead (&gameData.endLevel.exit.nModel, sizeof (int), 1, fp);
	CFRead (&gameData.endLevel.exit.nDestroyedModel, sizeof (int), 1, fp);
#endif
}

// the following is old code for reading descent 1 textures.
#if 0

#define D1_MAX_TEXTURES 800
#define D1_MAX_SOUNDS 250
#define D1_MAX_VCLIPS 70
#define D1_MAX_EFFECTS 60
#define D1_MAX_WALL_ANIMS 30
#define D1_MAX_ROBOT_TYPES 30
#define D1_MAX_ROBOT_JOINTS 600
#define D1_MAX_WEAPON_TYPES 30
#define MAX_POWERUP_TYPES_D1 29
#define D1_MAX_GAUGE_BMS 80
#define D1_MAX_OBJ_BITMAPS 210
#define D1_MAX_COCKPIT_BITMAPS 4
#define D1_MAX_OBJTYPE 100
#define D1_MAX_POLYGON_MODELS 85

#define D1_TMAP_INFO_SIZE 26
#define D1_VCLIP_SIZE 66
#define D1_ROBOT_INFO_SIZE 486
#define D1_WEAPON_INFO_SIZE 115

#define D1_LAST_STATIC_TMAP_NUM 324

// store the gameData.pig.tex.bmIndex [0][] array as read from the descent 2 pig.
short *d2_Textures_backup = NULL;

void undo_bm_read_all_d1 () 
{
if (d2_Textures_backup) {
	int i;
	for (i = 0;i < D1_LAST_STATIC_TMAP_NUM;i++)
		gameData.pig.tex.bmIndex [0][i].index = d2_Textures_backup [i];
	D2_FREE (d2_Textures_backup);
	d2_Textures_backup = NULL;
	}
}

//------------------------------------------------------------------------------
/*
 * used by piggy_d1_init to read in descent 1 pigfile
 */
void BMReadAllD1 (CFILE * fp)
{
	int i;

	atexit (undo_bm_read_all_d1);

	/*gameData.pig.tex.nTextures = */ CFReadInt (fp);
	//BitmapIndexReadN (gameData.pig.tex.bmIndex, D1_MAX_TEXTURES, fp);
	//for (i = 0;i < D1_MAX_TEXTURES;i++)
	//	gameData.pig.tex.bmIndex [0][i].index = CFReadShort (fp) + 600;
	//CFSeek (fp, D1_MAX_TEXTURES * sizeof (short), SEEK_CUR);
	MALLOC (d2_Textures_backup, short, D1_LAST_STATIC_TMAP_NUM);
	for (i = 0;i < D1_LAST_STATIC_TMAP_NUM;i++) {
		d2_Textures_backup [i] = gameData.pig.tex.bmIndex [0][i].index;
		gameData.pig.tex.bmIndex [0][i].index = CFReadShort (fp) + 521;
	}
	CFSeek (fp, (D1_MAX_TEXTURES - D1_LAST_STATIC_TMAP_NUM) * sizeof (short), SEEK_CUR);

	//ReadTMapInfoND1 (gameData.pig.tex.tMapInfo, D1_MAX_TEXTURES, fp);
	CFSeek (fp, D1_MAX_TEXTURES * D1_TMAP_INFO_SIZE, SEEK_CUR);

	/*
	CFRead (Sounds [0], sizeof (ubyte), D1_MAX_SOUNDS, fp);
	CFRead (AltSounds, sizeof (ubyte), D1_MAX_SOUNDS, fp);
	*/CFSeek (fp, D1_MAX_SOUNDS * 2, SEEK_CUR);

	/*gameData.eff.nClips = */ CFReadInt (fp);
	//VClipReadN (gameData.eff.vClips, D1_MAX_VCLIPS, fp);
	CFSeek (fp, D1_MAX_VCLIPS * D1_VCLIP_SIZE, SEEK_CUR);

	gameData.eff.nEffects [1] = CFReadInt (fp);
	EClipReadN (gameData.eff.effects [1], D1_MAX_EFFECTS, fp);

	/*
	gameData.walls.nAnims = CFReadInt (fp);
	wclip_read_n_d1 (gameData.walls.anims, D1_MAX_WALL_ANIMS, fp);
	*/

	/*
	gameData.bots.nTypes = CFReadInt (fp);
	//RobotInfoReadN (gameData.bots.info [0], D1_MAX_ROBOT_TYPES, fp);
	CFSeek (fp, D1_MAX_ROBOT_TYPES * D1_ROBOT_INFO_SIZE, SEEK_CUR);

	gameData.bots.nJoints = CFReadInt (fp);
	JointPosReadN (gameData.bots.joints, D1_MAX_ROBOT_JOINTS, fp);

	gameData.weapons.nTypes [0] = CFReadInt (fp);
	//WeaponInfoReadN (gameData.weapons.info, D1_MAX_WEAPON_TYPES, fp, gameData.pig.tex.nHamFileVersion);
	CFSeek (fp, D1_MAX_WEAPON_TYPES * D1_WEAPON_INFO_SIZE, SEEK_CUR);

	gameData.objs.pwrUp.nTypes = CFReadInt (fp);
	PowerupTypeInfoReadN (gameData.objs.pwrUp.info, MAX_POWERUP_TYPES_D1, fp);
	*/

	/* in the following code are bugs, solved by hack
	gameData.models.nPolyModels = CFReadInt (fp);
	PolyModelReadN (gameData.models.polyModels, gameData.models.nPolyModels, fp);
	for (i=0;i<gameData.models.nPolyModels;i++)
		PolyModelDataRead (&gameData.models.polyModels [i], fp);
	*/CFSeek (fp, 521490-160, SEEK_SET);// OK, I admit, this is a dirty hack
	//BitmapIndexReadN (gameData.cockpit.gauges [1], D1_MAX_GAUGE_BMS, fp);
	CFSeek (fp, D1_MAX_GAUGE_BMS * sizeof (tBitmapIndex), SEEK_CUR);

	/*
	for (i = 0;i < D1_MAX_POLYGON_MODELS;i++)
		gameData.models.nDyingModels [i] = CFReadInt (fp);
	for (i = 0;i < D1_MAX_POLYGON_MODELS;i++)
		gameData.models.nDeadModels [i] = CFReadInt (fp);
	*/ CFSeek (fp, D1_MAX_POLYGON_MODELS * 8, SEEK_CUR);

	//BitmapIndexReadN (gameData.pig.tex.objBmIndex, D1_MAX_OBJ_BITMAPS, fp);
	CFSeek (fp, D1_MAX_OBJ_BITMAPS * sizeof (tBitmapIndex), SEEK_CUR);
	for (i = 0;i < D1_MAX_OBJ_BITMAPS;i++)
		CFSeek (fp, 2, SEEK_CUR);//gameData.pig.tex.pObjBmIndex [i] = CFReadShort (fp);

	//PlayerShipRead (&gameData.pig.ship.only, fp);
	CFSeek (fp, sizeof (tPlayerShip), SEEK_CUR);

	/*gameData.models.nCockpits = */ CFReadInt (fp);
	//BitmapIndexReadN (gameData.pig.tex.cockpitBmIndex, D1_MAX_COCKPIT_BITMAPS, fp);
	CFSeek (fp, D1_MAX_COCKPIT_BITMAPS * sizeof (tBitmapIndex), SEEK_CUR);

	/*
	CFRead (Sounds [0], sizeof (ubyte), D1_MAX_SOUNDS, fp);
	CFRead (AltSounds, sizeof (ubyte), D1_MAX_SOUNDS, fp);
	*/CFSeek (fp, D1_MAX_SOUNDS * 2, SEEK_CUR);

	/*gameData.objs.types.nCount = */ CFReadInt (fp);
	/*
	CFRead (gameData.objs.types.nType, sizeof (byte), D1_MAX_OBJTYPE, fp);
	CFRead (gameData.objs.types.nType.nId, sizeof (byte), D1_MAX_OBJTYPE, fp);
	for (i=0;i<D1_MAX_OBJTYPE;i++)
		gameData.objs.types.nType.nStrength [i] = CFReadInt (fp);
	*/ CFSeek (fp, D1_MAX_OBJTYPE * 6, SEEK_CUR);

	/*gameData.pig.tex.nFirstMultiBitmap =*/ CFReadInt (fp);
	/*gameData.reactor.reactors [0].nGuns = */ CFReadInt (fp);
	/*for (i=0;i<4;i++)
		CFReadVector (& (gameData.reactor.reactors [0].gunPoints [i]), fp);
	for (i=0;i<4;i++)
		CFReadVector (& (gameData.reactor.reactors [0].gun_dirs [i]), fp);
	*/CFSeek (fp, 8 * 12, SEEK_CUR);

	/*gameData.endLevel.exit.nModel = */ CFReadInt (fp);
	/*gameData.endLevel.exit.nDestroyedModel = */ CFReadInt (fp);
}

#endif // if 0, old code for reading descent 1 textures

//------------------------------------------------------------------------------
//these values are the number of each item in the release of d2
//extra items added after the release get written in an additional hamfile
#define N_D2_ROBOT_TYPES		66
#define N_D2_ROBOT_JOINTS		1145
#define N_D2_POLYGON_MODELS   166
#define N_D2_OBJBITMAPS			422
#define N_D2_OBJBITMAPPTRS		502
#define N_D2_WEAPON_TYPES		62

void _CDECL_ BMFreeExtraObjBitmaps (void)
{
	int			i;
	grsBitmap	*bmP;

PrintLog ("unloading extra bitmaps\n");
if (!gameData.pig.tex.nExtraBitmaps)
	gameData.pig.tex.nExtraBitmaps = gameData.pig.tex.nBitmaps [0];
for (i = gameData.pig.tex.nBitmaps [0], bmP = gameData.pig.tex.bitmaps [0] + i; 
	  i < gameData.pig.tex.nExtraBitmaps; i++, bmP++) {
	gameData.pig.tex.nObjBitmaps--;
	OglFreeBmTexture (bmP);
	if (bmP->bmTexBuf) {
		D2_FREE (bmP->bmTexBuf);
		UseBitmapCache (bmP, (int) -bmP->bmProps.h * (int) bmP->bmProps.rowSize);
		}
	}
gameData.pig.tex.nExtraBitmaps = gameData.pig.tex.nBitmaps [0];
}

//------------------------------------------------------------------------------

void BMFreeExtraModels ()
{
	//return;
PrintLog ("unloading extra poly models\n");
while (gameData.models.nPolyModels > N_D2_POLYGON_MODELS) {
	FreeModel (gameData.models.polyModels + --gameData.models.nPolyModels);
	FreeModel (gameData.models.defPolyModels + gameData.models.nPolyModels);
	}
if (!gameStates.app.bDemoData)
	while (gameData.models.nPolyModels > gameData.endLevel.exit.nModel) {
		FreeModel (gameData.models.polyModels + --gameData.models.nPolyModels);
		FreeModel (gameData.models.defPolyModels + gameData.models.nPolyModels);
		}
}

//------------------------------------------------------------------------------

//nType==1 means 1.1, nType==2 means 1.2 (with weapons)
int BMReadExtraRobots (const char *fname, char *folder, int nType)
{
	CFILE cf;
	int t,i,j;
	int version, bVertigoData;

	//strlwr (fname);
bVertigoData = !strcmp (fname, "d2x.ham");
if (!CFOpen (&cf, fname, folder, "rb", 0))
	return 0;

//if (bVertigoData) 
	{
	BMFreeExtraModels ();
	BMFreeExtraObjBitmaps ();
	}

if (nType > 1) {
	int sig;

	sig = CFReadInt (&cf);
	if (sig != MAKE_SIG ('X','H','A','M'))
		return 0;
	version = CFReadInt (&cf);
}
else
	version = 0;

//read extra weapons

t = CFReadInt (&cf);
gameData.weapons.nTypes [0] = N_D2_WEAPON_TYPES+t;
if (gameData.weapons.nTypes [0] >= MAX_WEAPON_TYPES) {
	Warning ("Too many weapons (%d) in <%s>.  Max is %d.",t,fname,MAX_WEAPON_TYPES-N_D2_WEAPON_TYPES);
	return -1;
	}
WeaponInfoReadN (gameData.weapons.info + N_D2_WEAPON_TYPES, t, &cf, 3);

//now read robot info

t = CFReadInt (&cf);
gameData.bots.nTypes [0] = N_D2_ROBOT_TYPES + t;
if (gameData.bots.nTypes [0] >= MAX_ROBOT_TYPES) {
	Warning ("Too many robots (%d) in <%s>.  Max is %d.",t,fname,MAX_ROBOT_TYPES-N_D2_ROBOT_TYPES);
	return -1;
	}
RobotInfoReadN (gameData.bots.info [0] + N_D2_ROBOT_TYPES, t, &cf);
if (bVertigoData) {
	gameData.bots.nDefaultTypes = gameData.bots.nTypes [0];
	memcpy (gameData.bots.defaultInfo + N_D2_ROBOT_TYPES, gameData.bots.info [0] + N_D2_ROBOT_TYPES, sizeof (*gameData.bots.info [0]) * t);
	}

t = CFReadInt (&cf);
gameData.bots.nJoints = N_D2_ROBOT_JOINTS + t;
if (gameData.bots.nJoints >= MAX_ROBOT_JOINTS) {
	Warning ("Too many robot joints (%d) in <%s>.  Max is %d.",t,fname,MAX_ROBOT_JOINTS-N_D2_ROBOT_JOINTS);
	return -1;
	}
JointPosReadN (gameData.bots.joints + N_D2_ROBOT_JOINTS, t, &cf);
if (bVertigoData) {
	gameData.bots.nDefaultJoints = gameData.bots.nJoints;
	memcpy (gameData.bots.defaultJoints + N_D2_ROBOT_TYPES, gameData.bots.joints + N_D2_ROBOT_TYPES, sizeof (*gameData.bots.joints) * t);
	}

t = CFReadInt (&cf);
j = N_D2_POLYGON_MODELS; //gameData.models.nPolyModels;
gameData.models.nPolyModels += t;
if (gameData.models.nPolyModels >= MAX_POLYGON_MODELS) {
	Warning ("Too many polygon models (%d)\nin <%s>.\nMax is %d.",
				t,fname, MAX_POLYGON_MODELS - N_D2_POLYGON_MODELS);
	return -1;
	}
PolyModelReadN (gameData.models.polyModels + j, t, &cf);
if (bVertigoData) {
	gameData.models.nDefPolyModels = gameData.models.nPolyModels;
	memcpy (gameData.models.defPolyModels + j, gameData.models.polyModels + j, sizeof (*gameData.models.polyModels) * t);
	}
for (i = j; i < gameData.models.nPolyModels; i++) {
	gameData.models.defPolyModels [i].modelData =
	gameData.models.polyModels [i].modelData = NULL;
	PolyModelDataRead (gameData.models.polyModels + i, i, bVertigoData ? gameData.models.defPolyModels + i : NULL, &cf);
	}
for (i = j; i < gameData.models.nPolyModels; i++)
	gameData.models.nDyingModels [i] = CFReadInt (&cf);
for (i = j; i < gameData.models.nPolyModels; i++)
	gameData.models.nDeadModels [i] = CFReadInt (&cf);

t = CFReadInt (&cf);
if (N_D2_OBJBITMAPS + t >= MAX_OBJ_BITMAPS) {
	Warning ("Too many tObject bitmaps (%d) in <%s>.  Max is %d.",t,fname,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPS);
	return -1;
	}
BitmapIndexReadN (&gameData.pig.tex.objBmIndex [N_D2_OBJBITMAPS], t, &cf);

t = CFReadInt (&cf);
if (N_D2_OBJBITMAPPTRS + t >= MAX_OBJ_BITMAPS) {
	Warning ("Too many tObject bitmap pointers (%d) in <%s>.  Max is %d.",t,fname,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPPTRS);
	return -1;
	}
for (i = N_D2_OBJBITMAPPTRS; i < (N_D2_OBJBITMAPPTRS + t); i++)
	gameData.pig.tex.pObjBmIndex [i] = CFReadShort (&cf);
CFClose (&cf);
return 1;
}

//------------------------------------------------------------------------------

int LoadRobotReplacements (const char *szLevelName, int bAddBots, int bAltModels)
{
	CFILE			cf;
	tPolyModel	*pm;
	int			t, i, j;
	int			nBotTypeSave = gameData.bots.nTypes [0], 
					nBotJointSave = gameData.bots.nJoints, 
					nPolyModelSave = gameData.models.nPolyModels;
	tRobotInfo	botInfoSave;
	char			szFilename [FILENAME_LEN];

ChangeFilenameExtension (szFilename, szLevelName, ".hxm");
if (!CFOpen (&cf, szFilename, gameFolders.szDataDir, "rb", 0))		//no robot replacement file
	return 0;
t = CFReadInt (&cf);			//read id "HXM!"
if (t!= MAKE_SIG ('!','X','M','H'))
	Warning (TXT_HXM_ID);
t = CFReadInt (&cf);			//read version
if (t < 1)
	Warning (TXT_HXM_VERSION, t);
t = CFReadInt (&cf);			//read number of robots
for (j = 0; j < t; j++) {
	i = CFReadInt (&cf);		//read robot number
	if (bAddBots) {
		if (gameData.bots.nTypes [0] >= MAX_ROBOT_TYPES) {
			Warning (TXT_ROBOT_NO, szLevelName, i, MAX_ROBOT_TYPES);
			return -1;
			}
		i = gameData.bots.nTypes [0]++;
		}
	else if (i < 0 || i >= gameData.bots.nTypes [0]) {
		Warning (TXT_ROBOT_NO, szLevelName, i, gameData.bots.nTypes [0] - 1);
		gameData.bots.nTypes [0] = nBotTypeSave;
		gameData.bots.nJoints = nBotJointSave;
		gameData.models.nPolyModels = nPolyModelSave;
		return -1;
		}
	if (bAltModels)
		CFSeek (&cf, sizeof (tRobotInfo), SEEK_CUR);
	else {
		botInfoSave = gameData.bots.info [0][i];
		RobotInfoReadN (gameData.bots.info [0] + i, 1, &cf);
		}
	}
t = CFReadInt (&cf);			//read number of joints
for (j = 0; j < t; j++) {
	i = CFReadInt (&cf);		//read joint number
	if (bAddBots) {
		if (gameData.bots.nJoints >= MAX_ROBOT_JOINTS) {
			Warning ("%s: Robots joint (%d) out of range (valid range = 0 - %d).",
						szLevelName, i, MAX_ROBOT_JOINTS - 1);
			gameData.bots.nTypes [0] = nBotTypeSave;
			gameData.bots.nJoints = nBotJointSave;
			gameData.models.nPolyModels = nPolyModelSave;
			return -1;
			}
		else
			i = gameData.bots.nJoints++;
		}
	else if ((i < 0) || (i >= gameData.bots.nJoints)) {
		Warning ("%s: Robots joint (%d) out of range (valid range = 0 - %d).",
					szLevelName, i, gameData.bots.nJoints - 1);
		gameData.bots.nTypes [0] = nBotTypeSave;
		gameData.bots.nJoints = nBotJointSave;
		gameData.models.nPolyModels = nPolyModelSave;
		return -1;
		}
	JointPosReadN (gameData.bots.joints + i, 1, &cf);
	}
t = CFReadInt (&cf);			//read number of polygon models
for (j = 0; j < t; j++) {
	i = CFReadInt (&cf);		//read model number
	if (bAddBots) {
		if (gameData.models.nPolyModels >= MAX_POLYGON_MODELS) {
			Warning ("%s: Polygon model (%d) out of range (valid range = 0 - %d).",
						szLevelName, i, gameData.models.nPolyModels - 1);
			gameData.bots.nTypes [0] = nBotTypeSave;
			gameData.bots.nJoints = nBotJointSave;
			gameData.models.nPolyModels = nPolyModelSave;
			return -1;
			}
		else
			i = gameData.models.nPolyModels++;
		}
	else if ((i < 0) || (i >= gameData.models.nPolyModels)) {
		if (bAltModels) {
			if (i < MAX_POLYGON_MODELS) 
				gameData.models.nPolyModels = i + 1;
			else {
				Warning ("%s: Polygon model (%d) out of range (valid range = 0 - %d).",
							szLevelName, i, gameData.models.nPolyModels - 1);
				gameData.bots.nTypes [0] = nBotTypeSave;
				gameData.bots.nJoints = nBotJointSave;
				gameData.models.nPolyModels = nPolyModelSave;
				return -1;
				}
			}
		else {
			Warning ("%s: Polygon model (%d) out of range (valid range = 0 - %d).",
						szLevelName, i, gameData.models.nPolyModels - 1);
			gameData.bots.nTypes [0] = nBotTypeSave;
			gameData.bots.nJoints = nBotJointSave;
			gameData.models.nPolyModels = nPolyModelSave;
			return -1;
			}
		}
	pm = bAltModels ? gameData.models.altPolyModels + i : gameData.models.polyModels + i;
	FreeModel (pm);
	if (!PolyModelRead (pm, &cf, 0))
		return -1;
	PolyModelDataRead (pm, i, NULL, &cf);
	pm->nType = bAltModels ? 1 : -1;
	pm->rad = G3PolyModelSize (pm, i);
	if (bAltModels) {
#if 0
		ubyte	*p = gameData.models.defPolyModels [i].modelData;
		gameData.models.defPolyModels [i] = gameData.models.polyModels [i];
		gameData.models.defPolyModels [i].modelData = p;
#else
		CFReadInt (&cf);
		CFReadInt (&cf);
#endif
		}
	else {
		gameData.models.nDyingModels [i] = CFReadInt (&cf);
		gameData.models.nDeadModels [i] = CFReadInt (&cf);
		}
	}

t = CFReadInt (&cf);			//read number of objbitmaps
for (j = 0; j < t; j++) {
	i = CFReadInt (&cf);		//read objbitmap number
	if (bAddBots) {
		}
	else if ((i < 0) || (i >= MAX_OBJ_BITMAPS)) {
		Warning ("%s: Object bitmap number (%d) out of range (valid range = 0 - %d).",
					szLevelName, i, MAX_OBJ_BITMAPS - 1);
		gameData.bots.nTypes [0] = nBotTypeSave;
		gameData.bots.nJoints = nBotJointSave;
		gameData.models.nPolyModels = nPolyModelSave;
		return -1;
		}
	BitmapIndexRead (gameData.pig.tex.objBmIndex + i, &cf);
	}
t = CFReadInt (&cf);			//read number of objbitmapptrs
for (j = 0; j < t; j++) {
	i = CFReadInt (&cf);		//read objbitmapptr number
	if ((i < 0) || (i >= MAX_OBJ_BITMAPS)) {
		Warning ("%s: Object bitmap pointer (%d) out of range (valid range = 0 - %d).",
					szLevelName, i, MAX_OBJ_BITMAPS - 1);
		gameData.bots.nTypes [0] = nBotTypeSave;
		gameData.bots.nJoints = nBotJointSave;
		gameData.models.nPolyModels = nPolyModelSave;
		return -1;
		}
	gameData.pig.tex.pObjBmIndex [i] = CFReadShort (&cf);
	}
CFClose (&cf);
return 1;
}

//------------------------------------------------------------------------------

/*
 * Routines for loading exit models
 *
 * Used by d1 levels (including some add-ons), and by d2 shareware.
 * Could potentially be used by d2 add-on levels, but only if they
 * don't use "extra" robots...
 */

// formerly exitmodel_bm_load_sub
tBitmapIndex ReadExtraBitmapIFF (const char * filename)
{
	tBitmapIndex bitmap_num;
	grsBitmap * newBm = gameData.pig.tex.bitmaps [0] + gameData.pig.tex.nExtraBitmaps;
	int iff_error;		//reference parm to avoid warning message

	bitmap_num.index = 0;
	//MALLOC (newBm, grsBitmap, 1);
	iff_error = iff_read_bitmap (filename,newBm,BM_LINEAR);
	//newBm->bmHandle=0;
	if (iff_error != IFF_NO_ERROR)		{
#if TRACE
		con_printf (CONDBG, 
			"Error loading exit model bitmap <%s> - IFF error: %s\n", 
			filename, iff_errormsg (iff_error));
#endif		
		return bitmap_num;
	}
	if (iff_has_transparency)
		GrRemapBitmapGood (newBm, NULL, iff_transparent_color, 254);
	else
		GrRemapBitmapGood (newBm, NULL, -1, 254);
	newBm->bmAvgColor = ComputeAvgPixel (newBm);
	bitmap_num.index = gameData.pig.tex.nExtraBitmaps;
	gameData.pig.tex.pBitmaps [gameData.pig.tex.nExtraBitmaps++] = *newBm;
	//D2_FREE (new);
	return bitmap_num;
}

//------------------------------------------------------------------------------
extern int GrAvgColor (grsBitmap *bm);
// formerly load_exit_model_bitmap
grsBitmap *BMLoadExtraBitmap (const char *name)
{
	int i;
	tBitmapIndex	*bip = gameData.pig.tex.objBmIndex + gameData.pig.tex.nObjBitmaps;
	Assert (gameData.pig.tex.nObjBitmaps < MAX_OBJ_BITMAPS);

*bip = ReadExtraBitmapIFF (name);
if (!bip->index) {
	char *name2 = D2_STRDUP (name);
	*strrchr (name2, '.') = '\0';
	*bip = ReadExtraBitmapD1Pig (name2);
	D2_FREE (name2);
	}
if (!(i = bip->index))
	return NULL;
//if (gameData.pig.tex.bitmaps [0][i].bmProps.w != 64 || gameData.pig.tex.bitmaps [0][i].bmProps.h != 64)
//	Error ("Bitmap <%s> is not 64x64", name);
gameData.pig.tex.pObjBmIndex [gameData.pig.tex.nObjBitmaps] = gameData.pig.tex.nObjBitmaps;
gameData.pig.tex.nObjBitmaps++;
Assert (gameData.pig.tex.nObjBitmaps < MAX_OBJ_BITMAPS);
return gameData.pig.tex.bitmaps [0] + i;
}

//------------------------------------------------------------------------------

void OglCachePolyModelTextures (int nModel);

int LoadExitModels (void)
{
	CFILE cfExitHAM;
	int start_num, i;
	static const char* szExitBm [] = {
		"steel1.bbm", 
		"rbot061.bbm", 
		"rbot062.bbm", 
		"steel1.bbm", 
		"rbot061.bbm", 
		"rbot063.bbm", 
		NULL};

	BMFreeExtraModels ();
	BMFreeExtraObjBitmaps ();

	start_num = gameData.pig.tex.nObjBitmaps;
	for (i = 0; szExitBm [i]; i++) 
		if (!BMLoadExtraBitmap (szExitBm [i]))
		{
#if TRACE
		con_printf (CON_NORMAL, "Can't load exit models!\n");
#endif
		return 0;
		}

	if (CFOpen (&cfExitHAM, "exit.ham", gameFolders.szDataDir, "rb", 0)) {
		gameData.endLevel.exit.nModel = gameData.models.nPolyModels++;
		gameData.endLevel.exit.nDestroyedModel = gameData.models.nPolyModels++;
		if (!PolyModelRead (gameData.models.polyModels + gameData.endLevel.exit.nModel, &cfExitHAM, 0))
			return 0;
		if (!PolyModelRead (gameData.models.polyModels + gameData.endLevel.exit.nDestroyedModel, &cfExitHAM, 0))
			return 0;
		gameData.models.polyModels [gameData.endLevel.exit.nModel].nFirstTexture = start_num;
		gameData.models.polyModels [gameData.endLevel.exit.nDestroyedModel].nFirstTexture = start_num + 3;
		gameData.models.polyModels [gameData.endLevel.exit.nModel].modelData = NULL;
		gameData.models.polyModels [gameData.endLevel.exit.nDestroyedModel].modelData = NULL;
		PolyModelDataRead (gameData.models.polyModels + gameData.endLevel.exit.nModel, gameData.endLevel.exit.nModel, NULL, &cfExitHAM);
		PolyModelDataRead (gameData.models.polyModels + gameData.endLevel.exit.nDestroyedModel, gameData.endLevel.exit.nDestroyedModel, NULL, &cfExitHAM);
		CFClose (&cfExitHAM);
		}
	else if (CFExist ("exit01.pof", gameFolders.szDataDir, 0) && 
				CFExist ("exit01d.pof", gameFolders.szDataDir, 0)) {
		gameData.endLevel.exit.nModel = LoadPolygonModel ("exit01.pof", 3, start_num, NULL);
		gameData.endLevel.exit.nDestroyedModel = LoadPolygonModel ("exit01d.pof", 3, start_num + 3, NULL);
		OglCachePolyModelTextures (gameData.endLevel.exit.nModel);
		OglCachePolyModelTextures (gameData.endLevel.exit.nDestroyedModel);
	}
	else if (CFExist (D1_PIGFILE,gameFolders.szDataDir,0)) {
		int offset, offset2;

		CFOpen (&cfExitHAM, D1_PIGFILE, gameFolders.szDataDir, "rb",0);
		switch (CFLength (&cfExitHAM,0)) { //total hack for loading models
		case D1_PIGSIZE:
			offset = 91848;/* and 92582  */
			offset2 = 383390;/* and 394022 */
			break;
		default:
		case D1_SHARE_BIG_PIGSIZE:
		case D1_SHARE_10_PIGSIZE:
		case D1_SHARE_PIGSIZE:
		case D1_10_BIG_PIGSIZE:
		case D1_10_PIGSIZE:
			Int3 ();/* exit models should be in .pofs */
		case D1_OEM_PIGSIZE:
		case D1_MAC_PIGSIZE:
		case D1_MAC_SHARE_PIGSIZE:
#if TRACE
			con_printf (CON_NORMAL, "Can't load exit models!\n");
#endif
			return 0;
		}
		CFSeek (&cfExitHAM, offset, SEEK_SET);
		gameData.endLevel.exit.nModel = gameData.models.nPolyModels++;
		gameData.endLevel.exit.nDestroyedModel = gameData.models.nPolyModels++;
		if (!PolyModelRead (gameData.models.polyModels + gameData.endLevel.exit.nModel, &cfExitHAM, 0))
			return 0;
		if (!PolyModelRead (gameData.models.polyModels + gameData.endLevel.exit.nDestroyedModel, &cfExitHAM, 0))
			return 0;
		gameData.models.polyModels [gameData.endLevel.exit.nModel].nFirstTexture = start_num;
		gameData.models.polyModels [gameData.endLevel.exit.nDestroyedModel].nFirstTexture = start_num+3;
		CFSeek (&cfExitHAM, offset2, SEEK_SET);
		gameData.models.polyModels [gameData.endLevel.exit.nModel].modelData = NULL;
		gameData.models.polyModels [gameData.endLevel.exit.nDestroyedModel].modelData = NULL;
		PolyModelDataRead (gameData.models.polyModels + gameData.endLevel.exit.nModel, gameData.endLevel.exit.nModel, NULL, &cfExitHAM);
		PolyModelDataRead (gameData.models.polyModels + gameData.endLevel.exit.nDestroyedModel, gameData.endLevel.exit.nDestroyedModel, NULL, &cfExitHAM);
		CFClose (&cfExitHAM);
	} else {
#if TRACE
		con_printf (CON_NORMAL, "Can't load exit models!\n");
#endif
		return 0;
	}
	atexit (BMFreeExtraObjBitmaps);
#if 1
	OglCachePolyModelTextures (gameData.endLevel.exit.nModel);
	OglCachePolyModelTextures (gameData.endLevel.exit.nDestroyedModel);
#endif
	return 1;
}

//------------------------------------------------------------------------------

void RestoreDefaultRobots (void)
{
	int	i;
	ubyte	*p;

memcpy (gameData.bots.info [0], gameData.bots.defaultInfo, gameData.bots.nDefaultTypes * sizeof (*gameData.bots.defaultInfo));
memcpy (gameData.bots.joints, gameData.bots.defaultJoints, gameData.bots.nDefaultJoints * sizeof (*gameData.bots.defaultJoints));
for (i = 0; i < gameData.models.nDefPolyModels; i++) {
	p = gameData.models.polyModels [i].modelData;
	if (gameData.models.defPolyModels [i].nDataSize != gameData.models.polyModels [i].nDataSize) {
		D2_FREE (p);
		p = NULL;
		}
	memcpy (gameData.models.polyModels + i, gameData.models.defPolyModels + i, sizeof (*gameData.models.defPolyModels));
	if (gameData.models.defPolyModels [i].modelData) {
		if (!p)
			p = (ubyte *) D2_ALLOC (gameData.models.defPolyModels [i].nDataSize);
		Assert (p != NULL);
		memcpy (p, gameData.models.defPolyModels [i].modelData, gameData.models.defPolyModels [i].nDataSize);
		gameData.models.polyModels [i].modelData = p;
		}
	else if (p) {
		D2_FREE (p);
		gameData.models.polyModels [i].modelData = NULL;
		}
	}
for (;i < gameData.models.nPolyModels; i++)
	FreeModel (gameData.models.polyModels + i);
gameData.bots.nTypes [0] = gameData.bots.nDefaultTypes;
gameData.bots.nJoints = gameData.bots.nDefaultJoints;
}

//------------------------------------------------------------------------------

void LoadTextureBrightness (const char *pszLevel, int *brightnessP)
{
	CFILE		cf;
	char		szFile [FILENAME_LEN];
	int		i, *pb;

if (!brightnessP)
	brightnessP = gameData.pig.tex.brightness;
ChangeFilenameExtension (szFile, pszLevel, ".lgt");
if (CFOpen (&cf, szFile, gameFolders.szDataDir, "rb", 0) &&
	 (CFRead (brightnessP, sizeof (*brightnessP) * MAX_WALL_TEXTURES, 1, &cf) == 1)) {
	for (i = MAX_WALL_TEXTURES, pb = gameData.pig.tex.brightness; i; i--, pb++)
		*pb = INTEL_INT (*pb);
	CFClose (&cf);
	}
}

//------------------------------------------------------------------------------

#define MODEL_DATA_VERSION 1

typedef struct tModelDataHeader {
	int					nVersion;
} tModelDataHeader;

int LoadModelData (void)
{
	CFILE					cf;
	tModelDataHeader	mdh;
	int					bOk;

if (!gameStates.app.bCacheModelData)
	return 0;
if (!CFOpen (&cf, "modeldata.d2x", gameFolders.szCacheDir, "rb", 0))
	return 0;
bOk = (CFRead (&mdh, sizeof (mdh), 1, &cf) == 1);
if (bOk)
	bOk = (mdh.nVersion == MODEL_DATA_VERSION);
if (bOk)
	bOk = (CFRead (gameData.models.spheres, sizeof (gameData.models.spheres), 1, &cf) == 1);
if (!bOk)
	memset (gameData.models.spheres, 0, sizeof (gameData.models.spheres));
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------

int SaveModelData (void)
{
	CFILE					cf;
	tModelDataHeader	mdh = {MODEL_DATA_VERSION};
	int					bOk;

if (!gameStates.app.bCacheModelData)
	return 0;
if (!CFOpen (&cf, "modeldata.d2x", gameFolders.szCacheDir, "wb", 0))
	return 0;
bOk = (CFWrite (&mdh, sizeof (mdh), 1, &cf) == 1) &&
		(CFWrite (gameData.models.spheres, sizeof (gameData.models.spheres), 1, &cf) == 1) &&
CFClose (&cf);
return bOk;
}

//------------------------------------------------------------------------------
//eof
