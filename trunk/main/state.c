/* $Id: state.c,v 1.12 2003/11/27 00:21:04 btb Exp $ */
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifndef _WIN32_WCE
#include <errno.h>
#endif

#ifdef _MSC_VER
#  include <windows.h>
#endif
#ifdef __macosx__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "pstypes.h"
#include "mono.h"
#include "inferno.h"
#include "segment.h"
#include "textures.h"
#include "wall.h"
#include "object.h"
#include "digi.h"
#include "gamemine.h"
#include "error.h"
#include "gameseg.h"
#include "menu.h"
#include "switch.h"
#include "game.h"
#include "screens.h"
#include "newmenu.h"
#include "gamepal.h"
#include "cfile.h"
#include "fuelcen.h"
#include "hash.h"
#include "key.h"
#include "piggy.h"
#include "player.h"
#include "cntrlcen.h"
#include "morph.h"
#include "weapon.h"
#include "render.h"
#include "gameseq.h"
#include "gauges.h"
#include "newdemo.h"
#include "automap.h"
#include "piggy.h"
#include "paging.h"
#include "titles.h"
#include "text.h"
#include "mission.h"
#include "pcx.h"
#include "u_mem.h"
#include "network.h"
#include "args.h"
#include "ai.h"
#include "fireball.h"
#include "controls.h"
#include "laser.h"
#include "multibot.h"
#include "ogl_init.h"
#include "state.h"
#include "strutil.h"
#include "ipx.h"
#include "gr.h"

#define STATE_VERSION				28
#define STATE_COMPATIBLE_VERSION 20
// 0 - Put DGSS (Descent Game State Save) id at tof.
// 1 - Added Difficulty level save
// 2 - Added gameStates.app.cheats.bEnabled flag
// 3 - Added between levels save.
// 4 - Added mission support
// 5 - Mike changed ai and tObject structure.
// 6 - Added buggin' cheat save
// 7 - Added other cheat saves and game_id.
// 8 - Added AI stuff for escort and thief.
// 9 - Save palette with screen shot
// 12- Saved last_was_super array
// 13- Saved palette flash stuff
// 14- Save cloaking wall stuff
// 15- Save additional ai info
// 16- Save gameData.render.lights.subtracted
// 17- New marker save
// 18- Took out saving of old cheat status
// 19- Saved cheats_enabled flag
// 20- gameStates.app.bFirstSecretVisit
// 22- xOmegaCharge

#define NUM_SAVES		9
#define THUMBNAIL_W	100
#define THUMBNAIL_H	50
#define THUMBNAIL_LW 200
#define THUMBNAIL_LH 120
#define DESC_LENGTH	20

void SetFunctionMode (int);
void InitPlayerStatsNewShip (void);
void ShowLevelIntro (int level_num);
void DoCloakInvulSecretStuff (fix xOldGameTime);
void copy_defaults_toRobot (tObject *objP);
void MultiInitiateSaveGame ();
void MultiInitiateRestoreGame ();
void ApplyAllChangedLight (void);
void DoLunacyOn (void);
void DoLunacyOff (void);

int sc_last_item= 0;
grsBitmap *sc_bmp [NUM_SAVES+1];

char dgss_id [4] = "DGSS";

int state_default_item = 0;

void ComputeAllStaticLight (void);

static char szDesc [NUM_SAVES + 1] [DESC_LENGTH + 16];
static char szTime [NUM_SAVES + 1] [DESC_LENGTH + 16];

void GameRenderFrame (void);

//-------------------------------------------------------------------

#define NM_IMG_SPACE	6

static int bShowTime = 1;

void state_callback (int nitems, tMenuItem *items, int *last_key, int citem)
{
	int x, y, i = citem - NM_IMG_SPACE;

//if ((citem == 1) && (items [1].value != bShowTime))
//	*last_key = -2;
if (citem < 2)
	return;
/*
for (citem = -1; i; i--)
	if (items [i].nType == NM_TYPE_MENU)
		citem++;
*/
if (!items [NM_IMG_SPACE - 1].text || strcmp (items [NM_IMG_SPACE - 1].text, szTime [i])) {
	items [NM_IMG_SPACE - 1].text = szTime [i];
	items [NM_IMG_SPACE - 1].rebuild = 1;
	}
if (!sc_bmp [i])
	return;
if (gameStates.menus.bHires) {
#if 0
	WINDOS (
		dd_grs_canvas *save_canv = dd_grd_curcanv,
		grs_canvas *save_canv = grdCurCanv
		);
	grs_canvas *temp_canv = GrCreateCanvas (THUMBNAIL_W*2, (THUMBNAIL_H*24/10));
	grs_point vertbuf [3] = {{0,0}, {0,0}, {i2f (THUMBNAIL_W*2),i2f (THUMBNAIL_H*24/10)} };
	GrSetCurrentCanvas (temp_canv);
	scale_bitmap (sc_bmp [citem], vertbuf, 0);
	WINDOS (
		DDGrSetCurrentCanvas (save_canv),
		GrSetCurrentCanvas (save_canv)
		);
#endif
	WIN (DDGRLOCK (dd_grd_curcanv));
	x = (grdCurCanv->cv_bitmap.bm_props.w - sc_bmp [i]->bm_props.w/*THUMBNAIL_W*2*/)/2;
	y = items [0].y - 16;
//	GrBitmap (x, y, &temp_canv->cv_bitmap);
	if (gameStates.app.bGameRunning)
		GrPaletteStepLoad (NULL);
	GrBitmap (x, y, sc_bmp [i]);
	//if (gameStates.app.bGameRunning)
	//	GrPaletteStepLoad (gamePalette);
	if (gameOpts->menus.nStyle) {
		GrSetColorRGBi (RGBA_PAL (0, 0, 32));
		GrUBox (x - 1, y - 1, 
					x + sc_bmp [i]->bm_props.w/*THUMBNAIL_W*2*/ + 1, y + sc_bmp [i]->bm_props.h/*THUMBNAIL_H*24/10*/ + 1);
		}
	WIN (DDGRUNLOCK (dd_grd_curcanv));
//	GrFreeCanvas (temp_canv);
	}
else {
	GrBitmap ((grdCurCanv->cv_bitmap.bm_props.w-THUMBNAIL_W)/2,items [0].y-5, sc_bmp [citem-1]);
	}
}

//------------------------------------------------------------------------------

void rpad_string (char * string, int max_chars)
{
	int i, end_found;

	end_found = 0;
	for (i=0; i<max_chars; i++)	{
		if (*string == 0)
			end_found = 1;
		if (end_found)
			*string = ' ';
		string++;
	}
	*string = 0;		// NULL terminate
}

//------------------------------------------------------------------------------

int StateGetSaveFile (char * fname, char * dsc, int bMulti)
{
	CFILE *fp;
	int i, menuRes, choice, sgVersion;
	tMenuItem m [NUM_SAVES+2];
	char filename [NUM_SAVES+1] [30];
	char id [5];
	int valid=0;
	
memset (m, 0, sizeof (m));
for (i = 0; i < NUM_SAVES; i++)	{
	sc_bmp [i] = NULL;
	if (!bMulti)
		sprintf (filename [i], "%s.sg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, i);
	else
		sprintf (filename [i], "%s.mg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, i);
	valid = 0;
	fp = CFOpen (filename [i], gameFolders.szSaveDir, "rb", 0);
	if (fp) {
		//Read id
		CFRead (id, sizeof (char)*4, 1, fp);
		if (!memcmp (id, dgss_id, 4)) {
			//Read sgVersion
			CFRead (&sgVersion, sizeof (int), 1, fp);
			if (sgVersion >= STATE_COMPATIBLE_VERSION)	{
				// Read description
				CFRead (szDesc [i], sizeof (char)*DESC_LENGTH, 1, fp);
				valid = 1;
				}
			}
			CFClose (fp);
		}
	if (!valid)
		strcpy (szDesc [i], TXT_EMPTY);
	m [i].nType = NM_TYPE_INPUT_MENU; 
	m [i].text = szDesc [i]; 
	m [i].text_len = DESC_LENGTH-1;
	}

sc_last_item = -1;
choice = state_default_item;
menuRes = ExecMenu1 (NULL, TXT_SAVE_GAME_MENU, NUM_SAVES, m, NULL, &choice);

for (i = 0; i < NUM_SAVES; i++)	{
	if (sc_bmp [i])
		GrFreeBitmap (sc_bmp [i]);
	}
if (menuRes < 0)
	return 0;
strcpy (fname, filename [choice]);
strcpy (dsc, szDesc [choice]);
state_default_item = choice;
return choice + 1;
}

//------------------------------------------------------------------------------

int RestoringMenu=0;

int StateGetRestoreFile (char * fname, int bMulti)
{
	CFILE				*fp;
	int				i, j, choice = -1, sgVersion, nsaves;
	tMenuItem	m [NUM_SAVES + NM_IMG_SPACE + 1];
	char				filename [NUM_SAVES+1] [30];
	char				id [5];
	int				valid;

	nsaves=0;
	memset (m, 0, sizeof (m));
	for (i = 0; i < NM_IMG_SPACE; i++) {
		m [i].nType = NM_TYPE_TEXT; 
		m [i].text = "";	
		m [i].noscroll = 1;
		}
	if (gameStates.app.bGameRunning) {
		GrPaletteStepLoad (NULL);
		}
	for (i = 0, j = 0; i < NUM_SAVES + 1; i++, j++) {
		sc_bmp [i] = NULL;
		if (!bMulti)
			sprintf (filename [i], "%s.sg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, i);
		else
			sprintf (filename [i], "%s.mg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, i);
		valid = 0;
		fp = CFOpen (filename [i], gameFolders.szSaveDir, "rb", 0);
		if (fp) {
			//Read id
			CFRead (id, sizeof (char)*4, 1, fp);
			if (!memcmp (id, dgss_id, 4)) {
				//Read sgVersion
				CFRead (&sgVersion, sizeof (int), 1, fp);
				if (sgVersion >= STATE_COMPATIBLE_VERSION)	{
					// Read description
					CFRead (szDesc [j], sizeof (char)*DESC_LENGTH, 1, fp);
					//rpad_string (szDesc [i], DESC_LENGTH-1);
					m [j+NM_IMG_SPACE].nType = NM_TYPE_MENU; 
					m [j+NM_IMG_SPACE].text = szDesc [j];
					// Read thumbnail
					if (sgVersion < 26) {
						sc_bmp [i] = GrCreateBitmap (THUMBNAIL_W,THUMBNAIL_H, 0);
						CFRead (sc_bmp [i]->bm_texBuf, THUMBNAIL_W * THUMBNAIL_H, 1, fp);
						}
					else {
						sc_bmp [i] = GrCreateBitmap (THUMBNAIL_LW,THUMBNAIL_LH, 0);
						CFRead (sc_bmp [i]->bm_texBuf, THUMBNAIL_LW * THUMBNAIL_LH, 1, fp);
						}
					if (sgVersion >= 9) {
						ubyte pal [256*3];
						CFRead (pal, 3, 256, fp);
						GrRemapBitmapGood (sc_bmp [i], pal, -1, -1);
					}
					nsaves++;
					valid = 1;
				}
			}
			CFClose (fp);
		}
		if (valid) {
			if (bShowTime) {
				struct tm	*t;
				int			h;
#ifdef _WIN32
				char			fn [FILENAME_LEN];

				struct _stat statBuf;
				sprintf (fn, "%s/%s", gameFolders.szSaveDir, filename [i]);
				h = _stat (fn, &statBuf);
#else
				struct stat statBuf;
				h = stat (filename [i], &statBuf);
#endif
				if (!h && (t = localtime (&statBuf.st_mtime)))
					sprintf (szTime [j], " [%d-%d-%d %d:%02d:%02d]",
						t->tm_mon + 1, t->tm_mday, t->tm_year + 1900,
						t->tm_hour, t->tm_min, t->tm_sec);
				}
			}
		else {
			strcpy (szDesc [j], TXT_EMPTY);
			//rpad_string (szDesc [i], DESC_LENGTH-1);
			m [j+NM_IMG_SPACE].nType = NM_TYPE_MENU; 
			m [j+NM_IMG_SPACE].text = szDesc [j];
		}
	}
	if (gameStates.app.bGameRunning) {
		GrPaletteStepLoad (NULL);
		}

	if (nsaves < 1)	{
		ExecMessageBox (NULL, NULL, 1, "Ok", TXT_NO_SAVEGAMES);
		return 0;
	}

	if (gameStates.video.nDisplayMode == 1)	//restore menu won't fit on 640x400
		VR_screenFlags ^= VRF_COMPATIBLE_MENUS;

	sc_last_item = -1;

   RestoringMenu=1;
	choice = state_default_item + NM_IMG_SPACE;
	i = ExecMenu3 (NULL, TXT_LOAD_GAME_MENU, j + NM_IMG_SPACE, m, state_callback, 
					   &choice, NULL, 190, -1);
	if (i < 0)
		return 0;
   RestoringMenu=0;
	choice -= NM_IMG_SPACE;

	for (i=0; i<NUM_SAVES+1; i++)	{
		if (sc_bmp [i])
			GrFreeBitmap (sc_bmp [i]);
	}

	if (choice >= 0) {
		strcpy (fname, filename [choice]);
		if (choice != NUM_SAVES+1)		//no new default when restore from autosave
			state_default_item = choice;
		return choice + 1;
	}
	return 0;
}

#define	DESC_OFFSET	8

#define	CF_BUF_SIZE	1024

#ifdef _WIN32_WCE
# define errno -1
# define strerror (x) "Unknown Error"
#endif


//	-----------------------------------------------------------------------------------
//	Imagine if C had a function to copy a file...
int copy_file (char *old_file, char *new_file)
{
	sbyte  buf [CF_BUF_SIZE];
	CFILE   *in_file, *out_file;

	out_file = CFOpen (new_file, gameFolders.szSaveDir, "wb", 0);
	if (out_file == NULL)
		return -1;
	in_file = CFOpen (old_file, gameFolders.szSaveDir, "rb", 0);
	if (in_file == NULL)
		return -2;
	while (!CFEoF (in_file))
	{
		int bytes_read = (int) CFRead (buf, 1, CF_BUF_SIZE, in_file);
		if (CFError (in_file))
			Error (TXT_FILEREAD_ERROR, old_file, strerror (errno));
		Assert (bytes_read == CF_BUF_SIZE || CFEoF (in_file));
		CFWrite (buf, 1, bytes_read, out_file);
		if (CFError (out_file))
			Error (TXT_FILEWRITE_ERROR, new_file, strerror (errno));
	}

	if (CFClose (in_file)) {
		CFClose (out_file);
		return -3;
	}

	if (CFClose (out_file))
		return -4;

	return 0;
}

#define SECRETB_FILENAME	"secret.sgb"
#define SECRETC_FILENAME	"secret.sgc"

//	-----------------------------------------------------------------------------------
//	blind_save means don't prompt user for any info.

int StateSaveAll (int bBetweenLevels, int bSecretSave, char *pszFilenameOverride)
{
	int	rval, filenum = -1;
	char	filename [128], szDesc [DESC_LENGTH+1];

Assert (bBetweenLevels == 0);	//between levels save ripped out
if (gameData.app.nGameMode & GM_MULTI)	{
	MultiInitiateSaveGame ();
	return 0;
	}
if (!(bSecretSave || gameOpts->gameplay.bSecretSave || gameStates.app.bD1Mission) && 
	  (gameData.missions.nCurrentLevel < 0)) {
	HUDInitMessage (TXT_SECRET_SAVE_ERROR);
	return 0;
	}
if (gameStates.gameplay.bFinalBossIsDead)		//don't allow save while final boss is dying
	return 0;
//	If this is a secret save and the control center has been destroyed, don't allow
//	return to the base level.
if (bSecretSave && gameData.reactor.bDestroyed) {
	CFDelete (SECRETB_FILENAME, gameFolders.szSaveDir);
	return 0;
	}
StopTime ();
gameData.app.bGamePaused = 1;
if (bSecretSave == 1) {
	pszFilenameOverride = filename;
	sprintf (pszFilenameOverride, SECRETB_FILENAME);
	} 
else if (bSecretSave == 2) {
	pszFilenameOverride = filename;
	sprintf (pszFilenameOverride, SECRETC_FILENAME);
	} 
else {
	if (pszFilenameOverride) {
		strcpy (filename, pszFilenameOverride);
		sprintf (szDesc, " [autosave backup]");
		}
	else if (!(filenum = StateGetSaveFile (filename, szDesc, 0))) {
		gameData.app.bGamePaused = 0;
		StartTime ();
		return 0;
		}
	}
//	MK, 1/1/96
//	If not in multiplayer, do special secret level stuff.
//	If secret.sgc exists, then copy it to Nsecret.sgc (where N = filenum).
//	If it doesn't exist, then delete Nsecret.sgc
if (!bSecretSave && ! (gameData.app.nGameMode & GM_MULTI)) {
	int	rval;
	char	temp_fname [32], fc;

	if (filenum != -1) {
		if (filenum >= 10)
			fc = (filenum-10) + 'a';
		else
			fc = '0' + filenum;
		sprintf (temp_fname, "%csecret.sgc", fc);
		if (CFExist (temp_fname,gameFolders.szSaveDir,0)) {
			rval = CFDelete (temp_fname, gameFolders.szSaveDir);
			Assert (rval == 0);	//	Oops, error deleting file in temp_fname.
			}
		if (CFExist (SECRETC_FILENAME,gameFolders.szSaveDir,0)) {
			rval = copy_file (SECRETC_FILENAME, temp_fname);
			Assert (rval == 0);	//	Oops, error copying temp_fname to secret.sgc!
			}
		}
	}

	//	Save file we're going to save over in last slot and call " [autosave backup]"
if (!pszFilenameOverride) {
	CFILE *tfp = CFOpen (filename, gameFolders.szSaveDir, "rb",0);

	if (tfp) {
		char	newname [128];

		sprintf (newname, "%s.sg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, NUM_SAVES);
		CFSeek (tfp, DESC_OFFSET, SEEK_SET);
		CFWrite (" [autosave backup]", sizeof (char)*DESC_LENGTH, 1, tfp);
		CFClose (tfp);
		CFDelete (newname, gameFolders.szSaveDir);
		CFRename (filename, newname, gameFolders.szSaveDir);
		}
	}
rval = StateSaveAllSub (filename, szDesc, bBetweenLevels);
gameData.app.bGamePaused = 0;
StartTime ();
return rval;
}

//------------------------------------------------------------------------------

void StateSaveBinGameData (CFILE *fp, int bBetweenLevels)
{
	int		i, j;
	ushort	nWall, nTexture;
// Save the Between levels flag...
CFWrite (&bBetweenLevels, sizeof (int), 1, fp);
// Save the mission info...
CFWrite (gameData.missions.list + gameData.missions.nCurrentMission, sizeof (char), 9, fp);
//Save level info
CFWrite (&gameData.missions.nCurrentLevel, sizeof (int), 1, fp);
CFWrite (&gameData.missions.nNextLevel, sizeof (int), 1, fp);
//Save gameData.time.xGame
CFWrite (&gameData.time.xGame, sizeof (fix), 1, fp);
// If coop save, save all
if (gameData.app.nGameMode & GM_MULTI_COOP) {
	CFWrite (&gameData.app.nStateGameId, sizeof (int), 1, fp);
	CFWrite (&netGame, sizeof (tNetgameInfo), 1, fp);
	CFWrite (&netPlayers, sizeof (tAllNetPlayersInfo), 1, fp);
	CFWrite (&gameData.multi.nPlayers, sizeof (int), 1, fp);
	CFWrite (&gameData.multi.nLocalPlayer, sizeof (int), 1, fp);
	for (i = 0; i < gameData.multi.nPlayers; i++)
		CFWrite (&gameData.multi.players [i], sizeof (tPlayer), 1, fp);
	}
//Save tPlayer info
CFWrite (&gameData.multi.players [gameData.multi.nLocalPlayer], sizeof (tPlayer), 1, fp);
// Save the current weapon info
CFWrite (&gameData.weapons.nPrimary, sizeof (sbyte), 1, fp);
CFWrite (&gameData.weapons.nSecondary, sizeof (sbyte), 1, fp);
// Save the difficulty level
CFWrite (&gameStates.app.nDifficultyLevel, sizeof (int), 1, fp);
// Save cheats enabled
CFWrite (&gameStates.app.cheats.bEnabled, sizeof (int), 1, fp);
if (!bBetweenLevels)	{
//Finish all morph gameData.objs.objects
	for (i = 0; i <= gameData.objs.nLastObject; i++) {
		if (gameData.objs.objects [i].nType == OBJ_NONE) 
			continue;
		if (gameData.objs.objects [i].nType == OBJ_CAMERA)
			gameData.objs.objects [i].position.mOrient = cameras [gameData.objs.cameraRef [i]].orient;
		else if (gameData.objs.objects [i].renderType==RT_MORPH) {
			tMorphInfo *md = MorphFindData (gameData.objs.objects + i);
			if (md) {
				tObject *objP = md->objP;
				objP->controlType = md->saveControlType;
				objP->movementType = md->saveMovementType;
				objP->renderType = RT_POLYOBJ;
				objP->mType.physInfo = md->savePhysInfo;
				md->objP = NULL;
				} 
			else {						//maybe loaded half-morphed from disk
				gameData.objs.objects [i].flags |= OF_SHOULD_BE_DEAD;
				gameData.objs.objects [i].renderType = RT_POLYOBJ;
				gameData.objs.objects [i].controlType = CT_NONE;
				gameData.objs.objects [i].movementType = MT_NONE;
				}
			}
		}
//Save tObject info
	i = gameData.objs.nLastObject + 1;
	CFWrite (&i, sizeof (int), 1, fp);
	CFWrite (gameData.objs.objects, sizeof (tObject), i, fp);
//Save wall info
	i = gameData.walls.nWalls;
	CFWrite (&i, sizeof (int), 1, fp);
	CFWrite (gameData.walls.walls, sizeof (wall), i, fp);
//Save exploding wall info
	i = MAX_EXPLODING_WALLS;
	CFWrite (&i, sizeof (int), 1, fp);
	CFWrite (gameData.walls.explWalls, sizeof (*gameData.walls.explWalls), i, fp);
//Save door info
	i = gameData.walls.nOpenDoors;
	CFWrite (&i, sizeof (int), 1, fp);
	CFWrite (gameData.walls.activeDoors, sizeof (tActiveDoor), i, fp);
//Save cloaking wall info
	i = gameData.walls.nCloaking;
	CFWrite (&i, sizeof (int), 1, fp);
	CFWrite (gameData.walls.cloaking, sizeof (tCloakingWall), i, fp);
//Save tTrigger info
	CFWrite (&gameData.trigs.nTriggers, sizeof (int), 1, fp);
	CFWrite (gameData.trigs.triggers, sizeof (tTrigger), gameData.trigs.nTriggers, fp);
	CFWrite (&gameData.trigs.nObjTriggers, sizeof (int), 1, fp);
	CFWrite (gameData.trigs.objTriggers, sizeof (tTrigger), gameData.trigs.nObjTriggers, fp);
	CFWrite (gameData.trigs.objTriggerRefs, sizeof (tObjTriggerRef), gameData.trigs.nObjTriggers, fp);
	CFWrite (gameData.trigs.firstObjTrigger, sizeof (short), MAX_OBJECTS_D2X, fp);
//Save tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++) {
		for (j = 0; j < 6; j++)	{
			nWall = WallNumI ((short) i, (short) j);
			CFWrite (&nWall, sizeof (short), 1, fp);
			CFWrite (&gameData.segs.segments [i].sides [j].nBaseTex, sizeof (short), 1, fp);
			nTexture = gameData.segs.segments [i].sides [j].nOvlTex | (gameData.segs.segments [i].sides [j].nOvlOrient << 14);
			CFWrite (&nTexture, sizeof (short), 1, fp);
		}
	}
// Save the fuelcen info
	CFWrite (&gameData.reactor.bDestroyed, sizeof (int), 1, fp);
	CFWrite (&gameData.reactor.countdown.nTimer, sizeof (int), 1, fp);
	CFWrite (&gameData.matCens.nRobotCenters, sizeof (int), 1, fp);
	CFWrite (gameData.matCens.robotCenters, sizeof (tMatCenInfo), gameData.matCens.nRobotCenters, fp);
	CFWrite (&gameData.reactor.triggers, sizeof (tReactorTriggers), 1, fp);
	CFWrite (&gameData.matCens.nFuelCenters, sizeof (int), 1, fp);
	CFWrite (gameData.matCens.fuelCenters, sizeof (tFuelCenInfo), gameData.matCens.nFuelCenters, fp);
// Save the control cen info
	CFWrite (&gameData.reactor.bHit, sizeof (int), 1, fp);
	CFWrite (&gameData.reactor.bSeenPlayer, sizeof (int), 1, fp);
	CFWrite (&gameData.reactor.nNextFireTime, sizeof (int), 1, fp);
	CFWrite (&gameData.reactor.bPresent, sizeof (int), 1, fp);
	CFWrite (&gameData.reactor.nDeadObj, sizeof (int), 1, fp);
// Save the AI state
	AISaveBinState (fp);

// Save the automap visited info
	CFWrite (bAutomapVisited, sizeof (ubyte) * MAX_SEGMENTS, 1, fp);

}
CFWrite (&gameData.app.nStateGameId, sizeof (uint), 1, fp);
CFWrite (&gameStates.app.cheats.bLaserRapidFire, sizeof (int), 1, fp);
CFWrite (&gameStates.app.bLunacy, sizeof (int), 1, fp);		//	Yes, writing this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
CFWrite (&gameStates.app.bLunacy, sizeof (int), 1, fp);
// Save automap marker info
CFWrite (gameData.marker.objects, sizeof (gameData.marker.objects), 1, fp);
CFWrite (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1, fp);
CFWrite (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1, fp);
CFWrite (&xAfterburnerCharge, sizeof (fix), 1, fp);
//save last was super information
CFWrite (&bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1, fp);
CFWrite (&bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1, fp);
//	Save flash effect stuff
CFWrite (&gameData.render.xFlashEffect, sizeof (int), 1, fp);
CFWrite (&gameData.render.xTimeFlashLastPlayed, sizeof (int), 1, fp);
CFWrite (&gameStates.ogl.palAdd.red, sizeof (int), 1, fp);
CFWrite (&gameStates.ogl.palAdd.green, sizeof (int), 1, fp);
CFWrite (&gameStates.ogl.palAdd.blue, sizeof (int), 1, fp);
CFWrite (gameData.render.lights.subtracted, sizeof (gameData.render.lights.subtracted [0]), MAX_SEGMENTS, fp);
CFWrite (&gameStates.app.bFirstSecretVisit, sizeof (gameStates.app.bFirstSecretVisit), 1, fp);
CFWrite (&xOmegaCharge, sizeof (xOmegaCharge), 1, fp);
}

//------------------------------------------------------------------------------

void StateSaveNetGame (CFILE *fp)
{
	int	i, j;

CFWriteByte (netGame.nType, fp);
CFWriteInt (netGame.Security, fp);
CFWrite (netGame.game_name, 1, NETGAME_NAME_LEN + 1, fp);
CFWrite (netGame.mission_title, 1, MISSION_NAME_LEN + 1, fp);
CFWrite (netGame.mission_name, 1, 9, fp);
CFWriteInt (netGame.levelnum, fp);
CFWriteByte ((sbyte) netGame.gamemode, fp);
CFWriteByte ((sbyte) netGame.RefusePlayers, fp);
CFWriteByte ((sbyte) netGame.difficulty, fp);
CFWriteByte ((sbyte) netGame.game_status, fp);
CFWriteByte ((sbyte) netGame.numplayers, fp);
CFWriteByte ((sbyte) netGame.max_numplayers, fp);
CFWriteByte ((sbyte) netGame.numconnected, fp);
CFWriteByte ((sbyte) netGame.gameFlags, fp);
CFWriteByte ((sbyte) netGame.protocol_version, fp);
CFWriteByte ((sbyte) netGame.version_major, fp);
CFWriteByte ((sbyte) netGame.version_minor, fp);
CFWriteByte ((sbyte) netGame.team_vector, fp);
CFWriteByte ((sbyte) netGame.DoMegas, fp);
CFWriteByte ((sbyte) netGame.DoSmarts, fp);
CFWriteByte ((sbyte) netGame.DoFusions, fp);
CFWriteByte ((sbyte) netGame.DoHelix, fp);
CFWriteByte ((sbyte) netGame.DoPhoenix, fp);
CFWriteByte ((sbyte) netGame.DoAfterburner, fp);
CFWriteByte ((sbyte) netGame.DoInvulnerability, fp);
CFWriteByte ((sbyte) netGame.DoCloak, fp);
CFWriteByte ((sbyte) netGame.DoGauss, fp);
CFWriteByte ((sbyte) netGame.DoVulcan, fp);
CFWriteByte ((sbyte) netGame.DoPlasma, fp);
CFWriteByte ((sbyte) netGame.DoOmega, fp);
CFWriteByte ((sbyte) netGame.DoSuperLaser, fp);
CFWriteByte ((sbyte) netGame.DoProximity, fp);
CFWriteByte ((sbyte) netGame.DoSpread, fp);
CFWriteByte ((sbyte) netGame.DoSmartMine, fp);
CFWriteByte ((sbyte) netGame.DoFlash, fp);
CFWriteByte ((sbyte) netGame.DoGuided, fp);
CFWriteByte ((sbyte) netGame.DoEarthShaker, fp);
CFWriteByte ((sbyte) netGame.DoMercury, fp);
CFWriteByte ((sbyte) netGame.bAllowMarkerView, fp);
CFWriteByte ((sbyte) netGame.AlwaysLighting, fp);
CFWriteByte ((sbyte) netGame.DoAmmoRack, fp);
CFWriteByte ((sbyte) netGame.DoConverter, fp);
CFWriteByte ((sbyte) netGame.DoHeadlight, fp);
CFWriteByte ((sbyte) netGame.DoHoming, fp);
CFWriteByte ((sbyte) netGame.DoLaserUpgrade, fp);
CFWriteByte ((sbyte) netGame.DoQuadLasers, fp);
CFWriteByte ((sbyte) netGame.ShowAllNames, fp);
CFWriteByte ((sbyte) netGame.BrightPlayers, fp);
CFWriteByte ((sbyte) netGame.invul, fp);
CFWriteByte ((sbyte) netGame.FriendlyFireOff, fp);
for (i = 0; i < 2; i++)
	CFWrite (netGame.team_name [i], 1, CALLSIGN_LEN + 1, fp);		// 18 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	CFWriteInt (netGame.locations [i], fp);
for (i = 0; i < MAX_PLAYERS; i++)
	for (j = 0; j < MAX_PLAYERS; j++)
		CFWriteShort (netGame.kills [i] [j], fp);			// 128 bytes
CFWriteShort (netGame.segments_checksum, fp);				// 2 bytes
for (i = 0; i < 2; i++)
	CFWriteShort (netGame.teamKills [i], fp);				// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	CFWriteShort (netGame.killed [i], fp);					// 16 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	CFWriteShort (netGame.playerKills [i], fp);			// 16 bytes
CFWriteInt (netGame.KillGoal, fp);							// 4 bytes
CFWriteFix (netGame.PlayTimeAllowed, fp);					// 4 bytes
CFWriteFix (netGame.levelTime, fp);							// 4 bytes
CFWriteInt (netGame.control_invulTime, fp);				// 4 bytes
CFWriteInt (netGame.monitor_vector, fp);					// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	CFWriteInt (netGame.player_score [i], fp);				// 32 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	CFWriteByte ((sbyte) netGame.playerFlags [i], fp);	// 8 bytes
CFWriteShort (netGame.nPacketsPerSec, fp);					// 2 bytes
CFWriteByte ((sbyte) netGame.bShortPackets, fp);			// 1 bytes
// 279 bytes
// 355 bytes total
CFWrite (netGame.AuxData, NETGAME_AUX_SIZE, 1, fp);  // Storage for protocol-specific data (e.g., multicast session and port)
}

//------------------------------------------------------------------------------

void StateSaveNetPlayers (CFILE *fp)
{
	int	i;

CFWriteByte ((sbyte) netPlayers.nType, fp);
CFWriteInt (netPlayers.Security, fp);
for (i = 0; i < MAX_PLAYERS + 4; i++) {
	CFWrite (netPlayers.players [i].callsign, 1, CALLSIGN_LEN + 1, fp);
	CFWrite (netPlayers.players [i].network.ipx.server, 1, 4, fp);
	CFWrite (netPlayers.players [i].network.ipx.node, 1, 6, fp);
	CFWriteByte ((sbyte) netPlayers.players [i].version_major, fp);
	CFWriteByte ((sbyte) netPlayers.players [i].version_minor, fp);
	CFWriteByte ((sbyte) netPlayers.players [i].computerType, fp);
	CFWriteByte (netPlayers.players [i].connected, fp);
	CFWriteShort ((short) netPlayers.players [i].socket, fp);
	CFWriteByte ((sbyte) netPlayers.players [i].rank, fp);
	}
}

//------------------------------------------------------------------------------

void StateSavePlayer (tPlayer *playerP, CFILE *fp)
{
	int	i;

CFWrite (playerP->callsign, 1, CALLSIGN_LEN + 1, fp); // The callsign of this tPlayer, for net purposes.
CFWrite (playerP->netAddress, 1, 6, fp);					// The network address of the player.
CFWriteByte (playerP->connected, fp);            // Is the tPlayer connected or not?
CFWriteInt (playerP->nObject, fp);                // What tObject number this tPlayer is. (made an int by mk because it's very often referenced)
CFWriteInt (playerP->nPacketsGot, fp);         // How many packets we got from them
CFWriteInt (playerP->nPacketsSent, fp);        // How many packets we sent to them
CFWriteInt ((int) playerP->flags, fp);           // Powerup flags, see below...
CFWriteFix (playerP->energy, fp);                // Amount of energy remaining.
CFWriteFix (playerP->shields, fp);               // shields remaining (protection)
CFWriteByte (playerP->lives, fp);                // Lives remaining, 0 = game over.
CFWriteByte (playerP->level, fp);                // Current level tPlayer is playing. (must be signed for secret levels)
CFWriteByte ((sbyte) playerP->laserLevel, fp);  // Current level of the laser.
CFWriteByte (playerP->startingLevel, fp);       // What level the tPlayer started on.
CFWriteShort (playerP->nKillerObj, fp);       // Who killed me.... (-1 if no one)
CFWriteShort ((short) playerP->primaryWeaponFlags, fp);   // bit set indicates the tPlayer has this weapon.
CFWriteShort ((short) playerP->secondaryWeaponFlags, fp); // bit set indicates the tPlayer has this weapon.
for (i = 0; i < MAX_PRIMARY_WEAPONS; i++)
	CFWriteShort ((short) playerP->primaryAmmo [i], fp); // How much ammo of each nType.
for (i = 0; i < MAX_SECONDARY_WEAPONS; i++)
	CFWriteShort ((short) playerP->secondaryAmmo [i], fp); // How much ammo of each nType.
#if 1 //for inventory system
CFWriteByte ((sbyte) playerP->nInvuls, fp);
CFWriteByte ((sbyte) playerP->nCloaks, fp);
#endif
CFWriteInt (playerP->last_score, fp);             // Score at beginning of current level.
CFWriteInt (playerP->score, fp);                  // Current score.
CFWriteFix (playerP->timeLevel, fp);             // Level time played
CFWriteFix (playerP->timeTotal, fp);             // Game time played (high word = seconds)
CFWriteFix (playerP->cloakTime - gameData.time.xGame, fp);             // Time cloaked
if (playerP->cloakTime == 0x7fffffff)		// invul cheat active
	CFWriteFix (playerP->cloakTime, fp);      // Time invulnerable
else
	CFWriteFix (playerP->cloakTime - gameData.time.xGame, fp);      // Time invulnerable
if (playerP->invulnerableTime == 0x7fffffff)		// invul cheat active
	CFWriteFix (playerP->invulnerableTime, fp);      // Time invulnerable
else
	CFWriteFix (playerP->invulnerableTime - gameData.time.xGame, fp);      // Time invulnerable
CFWriteShort (playerP->nKillGoalCount, fp);          // Num of players killed this level
CFWriteShort (playerP->netKilledTotal, fp);       // Number of times killed total
CFWriteShort (playerP->netKillsTotal, fp);        // Number of net kills total
CFWriteShort (playerP->numKillsLevel, fp);        // Number of kills this level
CFWriteShort (playerP->numKillsTotal, fp);        // Number of kills total
CFWriteShort (playerP->numRobotsLevel, fp);       // Number of initial robots this level
CFWriteShort (playerP->numRobotsTotal, fp);       // Number of robots total
CFWriteShort ((short) playerP->hostages_rescuedTotal, fp); // Total number of hostages rescued.
CFWriteShort ((short) playerP->hostagesTotal, fp);         // Total number of hostages.
CFWriteByte ((sbyte) playerP->hostages_on_board, fp);      // Number of hostages on ship.
CFWriteByte ((sbyte) playerP->hostagesLevel, fp);         // Number of hostages on this level.
CFWriteFix (playerP->homingObjectDist, fp);     // Distance of nearest homing tObject.
CFWriteByte (playerP->hoursLevel, fp);            // Hours played (since timeTotal can only go up to 9 hours)
CFWriteByte (playerP->hoursTotal, fp);            // Hours played (since timeTotal can only go up to 9 hours)
}

//------------------------------------------------------------------------------

void StateSaveObject (tObject *objP, CFILE *fp)
{
CFWriteInt (objP->nSignature, fp);      
CFWriteByte ((sbyte) objP->nType, fp); 
CFWriteByte ((sbyte) objP->id, fp);
CFWriteShort (objP->next, fp);
CFWriteShort (objP->prev, fp);
CFWriteByte ((sbyte) objP->controlType, fp);
CFWriteByte ((sbyte) objP->movementType, fp);
CFWriteByte ((sbyte) objP->renderType, fp);
CFWriteByte ((sbyte) objP->flags, fp);
CFWriteShort (objP->nSegment, fp);
CFWriteShort (objP->attachedObj, fp);
CFWriteVector (&objP->position.vPos, fp);     
CFWriteMatrix (&objP->position.mOrient, fp);  
CFWriteFix (objP->size, fp); 
CFWriteFix (objP->shields, fp);
CFWriteVector (&objP->last_pos, fp);  
CFWriteByte (objP->containsType, fp); 
CFWriteByte (objP->containsId, fp);   
CFWriteByte (objP->containsCount, fp);
CFWriteByte (objP->matCenCreator, fp);
CFWriteFix (objP->lifeleft, fp);   
if (objP->movementType == MT_PHYSICS) {
	CFWriteVector (&objP->mType.physInfo.velocity, fp);   
	CFWriteVector (&objP->mType.physInfo.thrust, fp);     
	CFWriteFix (objP->mType.physInfo.mass, fp);       
	CFWriteFix (objP->mType.physInfo.drag, fp);       
	CFWriteFix (objP->mType.physInfo.brakes, fp);     
	CFWriteVector (&objP->mType.physInfo.rotVel, fp);     
	CFWriteVector (&objP->mType.physInfo.rotThrust, fp);  
	CFWriteFixAng (objP->mType.physInfo.turnRoll, fp);   
	CFWriteShort ((short) objP->mType.physInfo.flags, fp);      
	}
else if (objP->movementType == MT_SPINNING) {
	CFWriteVector (&objP->mType.spinRate, fp);  
	}
switch (objP->controlType) {
	case CT_WEAPON:
		CFWriteShort (objP->cType.laserInfo.parentType, fp);
		CFWriteShort (objP->cType.laserInfo.nParentObj, fp);
		CFWriteInt (objP->cType.laserInfo.nParentSig, fp);
		CFWriteFix (objP->cType.laserInfo.creationTime, fp);
		CFWriteShort (objP->cType.laserInfo.nLastHitObj, fp);
		CFWriteShort (objP->cType.laserInfo.nTrackGoal, fp);
		CFWriteFix (objP->cType.laserInfo.multiplier, fp);
		break;

	case CT_EXPLOSION:
		CFWriteFix (objP->cType.explInfo.nSpawnTime, fp);
		CFWriteFix (objP->cType.explInfo.nDeleteTime, fp);
		CFWriteShort (objP->cType.explInfo.nDeleteObj, fp);
		CFWriteShort (objP->cType.explInfo.nAttachParent, fp);
		CFWriteShort (objP->cType.explInfo.nPrevAttach, fp);
		CFWriteShort (objP->cType.explInfo.nNextAttach, fp);
		break;

	case CT_AI:
		CFWriteByte ((sbyte) objP->cType.aiInfo.behavior, fp);
		CFWrite (objP->cType.aiInfo.flags, 1, MAX_AI_FLAGS, fp);
		CFWriteShort (objP->cType.aiInfo.nHideSegment, fp);
		CFWriteShort (objP->cType.aiInfo.nHideIndex, fp);
		CFWriteShort (objP->cType.aiInfo.nPathLength, fp);
		CFWriteByte (objP->cType.aiInfo.nCurPathIndex, fp);
		CFWriteByte (objP->cType.aiInfo.bDyingSoundPlaying, fp);
		CFWriteShort (objP->cType.aiInfo.nDangerLaser, fp);
		CFWriteInt (objP->cType.aiInfo.nDangerLaserSig, fp);
		CFWriteFix (objP->cType.aiInfo.xDyingStartTime, fp);
		break;

	case CT_LIGHT:
		CFWriteFix (objP->cType.lightInfo.intensity, fp);
		break;

	case CT_POWERUP:
		CFWriteInt (objP->cType.powerupInfo.count, fp);
		CFWriteFix (objP->cType.powerupInfo.creationTime, fp);
		CFWriteInt (objP->cType.powerupInfo.flags, fp);
		break;
	}
switch (objP->renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		CFWriteInt (objP->rType.polyObjInfo.nModel, fp);
		for (i = 0; i < MAX_SUBMODELS; i++)
			CFWriteAngVec (objP->rType.polyObjInfo.animAngles + i, fp);
		CFWriteInt (objP->rType.polyObjInfo.nSubObjFlags, fp);
		CFWriteInt (objP->rType.polyObjInfo.nTexOverride, fp);
		CFWriteInt (objP->rType.polyObjInfo.nAltTextures, fp);
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		CFWriteInt (objP->rType.vClipInfo.nClipIndex, fp);
		CFWriteFix (objP->rType.vClipInfo.xFrameTime, fp);
		CFWriteByte (objP->rType.vClipInfo.nCurFrame, fp);
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------

void StateSaveWall (wall *wallP, CFILE *fp)
{
CFWriteInt (wallP->nSegment, fp);
CFWriteInt (wallP->nSide, fp);
CFWriteFix (wallP->hps, fp);    
CFWriteInt (wallP->nLinkedWall, fp);
CFWriteByte ((sbyte) wallP->nType, fp);       
CFWriteByte ((sbyte) wallP->flags, fp);      
CFWriteByte ((sbyte) wallP->state, fp);      
CFWriteByte ((sbyte) wallP->nTrigger, fp);    
CFWriteByte (wallP->nClip, fp);   
CFWriteByte ((sbyte) wallP->keys, fp);       
CFWriteByte (wallP->controllingTrigger, fp);
CFWriteByte (wallP->cloakValue, fp); 
}

//------------------------------------------------------------------------------

void StateSaveExplWall (tExplWall *wallP, CFILE *fp)
{
CFWriteInt (wallP->nSegment, fp);
CFWriteInt (wallP->nSide, fp);
CFWriteFix (wallP->time, fp);    
}

//------------------------------------------------------------------------------

void StateSaveCloakingWall (tCloakingWall *wallP, CFILE *fp)
{
	int	i;

CFWriteShort (wallP->nFrontWall, fp);
CFWriteShort (wallP->nBackWall, fp); 
for (i = 0; i < 4; i++) {
	CFWriteFix (wallP->front_ls [i], fp); 
	CFWriteFix (wallP->back_ls [i], fp);
	}
CFWriteFix (wallP->time, fp);    
}

//------------------------------------------------------------------------------

void StateSaveActiveDoor (tActiveDoor *doorP, CFILE *fp)
{
	int	i;

CFWriteInt (doorP->nPartCount, fp);
for (i = 0; i < 2; i++) {
	CFWriteShort (doorP->nFrontWall [i], fp);
	CFWriteShort (doorP->nBackWall [i], fp);
	}
CFWriteFix (doorP->time, fp);    
}

//------------------------------------------------------------------------------

void StateSaveTrigger (tTrigger *triggerP, CFILE *fp)
{
	int	i;

CFWriteByte ((sbyte) triggerP->nType, fp); 
CFWriteByte ((sbyte) triggerP->flags, fp); 
CFWriteByte (triggerP->nLinks, fp);
CFWriteFix (triggerP->value, fp);
CFWriteFix (triggerP->time, fp);
for (i = 0; i < MAX_WALLS_PER_LINK; i++) {
	CFWriteShort (triggerP->nSegment [i], fp);
	CFWriteShort (triggerP->nSide [i], fp);
	}
}

//------------------------------------------------------------------------------

void StateSaveObjTriggerRef (tObjTriggerRef *refP, CFILE *fp)
{
CFWriteShort (refP->prev, fp);
CFWriteShort (refP->next, fp);
CFWriteShort (refP->nObject, fp);
}

//------------------------------------------------------------------------------

void StateSaveMatCen (tMatCenInfo *matcenP, CFILE *fp)
{
	int	i;

for (i = 0; i < 2; i++)
	CFWriteInt (matcenP->robotFlags [i], fp);
CFWriteFix (matcenP->hit_points, fp);
CFWriteFix (matcenP->interval, fp);
CFWriteShort (matcenP->nSegment, fp);
CFWriteShort (matcenP->fuelcen_num, fp);
}

//------------------------------------------------------------------------------

void StateSaveFuelCen (tFuelCenInfo *fuelcenP, CFILE *fp)
{
CFWriteInt (fuelcenP->Type, fp);
CFWriteInt (fuelcenP->nSegment, fp);
CFWriteByte (fuelcenP->Flag, fp);
CFWriteByte (fuelcenP->Enabled, fp);
CFWriteByte (fuelcenP->Lives, fp);
CFWriteFix (fuelcenP->Capacity, fp);
CFWriteFix (fuelcenP->MaxCapacity, fp);
CFWriteFix (fuelcenP->Timer, fp);
CFWriteFix (fuelcenP->DisableTime, fp);
CFWriteVector (&fuelcenP->Center, fp);
}

//------------------------------------------------------------------------------

void StateSaveReactorTrigger (tReactorTriggers *triggerP, CFILE *fp)
{
	int	i;

CFWriteShort (triggerP->nLinks, fp);
for (i = 0; i < MAX_CONTROLCEN_LINKS; i++) {
	CFWriteShort (triggerP->nSegment [i], fp);
	CFWriteShort (triggerP->nSide [i], fp);
	}
}

//------------------------------------------------------------------------------

void StateSaveUniGameData (CFILE *fp, int bBetweenLevels)
{
	int	i, j, fpos;
// Save the Between levels flag...
CFWriteInt (bBetweenLevels, fp);
// Save the mission info...
CFWrite (gameData.missions.list + gameData.missions.nCurrentMission, sizeof (char), 9, fp);
//Save level info
CFWriteInt (gameData.missions.nCurrentLevel, fp);
CFWriteInt (gameData.missions.nNextLevel, fp);
//Save gameData.time.xGame
CFWriteFix (gameData.time.xGame, fp);
// If coop save, save all
if (gameData.app.nGameMode & GM_MULTI_COOP) {
	CFWriteInt (gameData.app.nStateGameId, fp);
	StateSaveNetGame (fp);
	fpos = CFTell (fp);
	StateSaveNetPlayers (fp);
	fpos = CFTell (fp);
	CFWriteInt (gameData.multi.nPlayers, fp);
	CFWriteInt (gameData.multi.nLocalPlayer, fp);
	for (i = 0; i < gameData.multi.nPlayers; i++)
		StateSavePlayer (gameData.multi.players + i, fp);
	fpos = CFTell (fp);
	}
//Save tPlayer info
StateSavePlayer (gameData.multi.players + gameData.multi.nLocalPlayer, fp);
// Save the current weapon info
CFWriteByte (gameData.weapons.nPrimary, fp);
CFWriteByte (gameData.weapons.nSecondary, fp);
// Save the difficulty level
CFWriteInt (gameStates.app.nDifficultyLevel, fp);
// Save cheats enabled
CFWriteInt (gameStates.app.cheats.bEnabled, fp);
if (!bBetweenLevels)	{
//Finish all morph gameData.objs.objects
	for (i = 0; i <= gameData.objs.nLastObject; i++) {
		if (gameData.objs.objects [i].nType == OBJ_NONE) 
			continue;
		if (gameData.objs.objects [i].nType == OBJ_CAMERA)
			gameData.objs.objects [i].position.mOrient = cameras [gameData.objs.cameraRef [i]].orient;
		else if (gameData.objs.objects [i].renderType == RT_MORPH) {
			tMorphInfo *md = MorphFindData (gameData.objs.objects + i);
			if (md) {
				tObject *objP = md->objP;
				objP->controlType = md->saveControlType;
				objP->movementType = md->saveMovementType;
				objP->renderType = RT_POLYOBJ;
				objP->mType.physInfo = md->savePhysInfo;
				md->objP = NULL;
				} 
			else {						//maybe loaded half-morphed from disk
				gameData.objs.objects [i].flags |= OF_SHOULD_BE_DEAD;
				gameData.objs.objects [i].renderType = RT_POLYOBJ;
				gameData.objs.objects [i].controlType = CT_NONE;
				gameData.objs.objects [i].movementType = MT_NONE;
				}
			}
		}
	fpos = CFTell (fp);
//Save tObject info
	i = gameData.objs.nLastObject + 1;
	CFWriteInt (i, fp);
	for (j = 0; j < i; j++)
		StateSaveObject (gameData.objs.objects + j, fp);
	fpos = CFTell (fp);
//Save wall info
	i = gameData.walls.nWalls;
	CFWriteInt (i, fp);
	for (j = 0; j < i; j++)
		StateSaveWall (gameData.walls.walls + j, fp);
	fpos = CFTell (fp);
//Save exploding wall info
	i = MAX_EXPLODING_WALLS;
	CFWriteInt (i, fp);
	for (j = 0; j < i; j++)
		StateSaveExplWall (gameData.walls.explWalls + j, fp);
	fpos = CFTell (fp);
//Save door info
	i = gameData.walls.nOpenDoors;
	CFWriteInt (i, fp);
	for (j = 0; j < i; j++)
		StateSaveActiveDoor (gameData.walls.activeDoors + j, fp);
	fpos = CFTell (fp);
//Save cloaking wall info
	i = gameData.walls.nCloaking;
	CFWriteInt (i, fp);
	for (j = 0; j < i; j++)
		StateSaveCloakingWall (gameData.walls.cloaking + j, fp);
	fpos = CFTell (fp);
//Save tTrigger info
	CFWriteInt (gameData.trigs.nTriggers, fp);
	for (i = 0; i < gameData.trigs.nTriggers; i++)
		StateSaveTrigger (gameData.trigs.triggers + i, fp);
	CFWriteInt (gameData.trigs.nObjTriggers, fp);
	for (i = 0; i < gameData.trigs.nObjTriggers; i++)
		StateSaveTrigger (gameData.trigs.objTriggers + i, fp);
	for (i = 0; i < gameData.trigs.nObjTriggers; i++)
		StateSaveObjTriggerRef (gameData.trigs.objTriggerRefs + i, fp);
	for (i = 0; i < MAX_OBJECTS_D2X; i++)
		CFWriteShort (gameData.trigs.firstObjTrigger [i], fp);
	fpos = CFTell (fp);
//Save tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++) {
		for (j = 0; j < 6; j++)	{
			ushort nWall = WallNumI ((short) i, (short) j);
			CFWriteShort (nWall, fp);
			CFWriteShort (gameData.segs.segments [i].sides [j].nBaseTex, fp);
			CFWriteShort (gameData.segs.segments [i].sides [j].nOvlTex | (gameData.segs.segments [i].sides [j].nOvlOrient << 14), fp);
			}
		}
	fpos = CFTell (fp);
// Save the fuelcen info
	CFWriteInt (gameData.reactor.bDestroyed, fp);
	CFWriteFix (gameData.reactor.countdown.nTimer, fp);
	CFWriteInt (gameData.matCens.nRobotCenters, fp);
	for (i = 0; i < gameData.matCens.nRobotCenters; i++)
		StateSaveMatCen (gameData.matCens.robotCenters + i, fp);
	StateSaveReactorTrigger (&gameData.reactor.triggers, fp);
	CFWriteInt (gameData.matCens.nFuelCenters, fp);
	for (i = 0; i < gameData.matCens.nFuelCenters; i++)
		StateSaveFuelCen (gameData.matCens.fuelCenters + i, fp);
	fpos = CFTell (fp);
// Save the control cen info
	CFWriteInt (gameData.reactor.bHit, fp);
	CFWriteInt (gameData.reactor.bSeenPlayer, fp);
	CFWriteInt (gameData.reactor.nNextFireTime, fp);
	CFWriteInt (gameData.reactor.bPresent, fp);
	CFWriteInt (gameData.reactor.nDeadObj, fp);
	fpos = CFTell (fp);
// Save the AI state
	AISaveUniState (fp);

	fpos = CFTell (fp);
// Save the automap visited info
	CFWrite (bAutomapVisited, sizeof (ubyte), MAX_SEGMENTS, fp);
	fpos = CFTell (fp);
	}
CFWriteInt ((int) gameData.app.nStateGameId, fp);
CFWriteInt (gameStates.app.cheats.bLaserRapidFire, fp);
CFWriteInt (gameStates.app.bLunacy, fp);		//	Yes, writing this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
CFWriteInt (gameStates.app.bLunacy, fp);
// Save automap marker info
for (i = 0; i < NUM_MARKERS; i++)
	CFWriteShort (gameData.marker.objects [i], fp);
CFWrite (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1, fp);
CFWrite (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1, fp);
CFWriteFix (xAfterburnerCharge, fp);
//save last was super information
CFWrite (bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1, fp);
CFWrite (bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1, fp);
//	Save flash effect stuff
CFWriteFix (gameData.render.xFlashEffect, fp);
CFWriteFix (gameData.render.xTimeFlashLastPlayed, fp);
CFWriteShort (gameStates.ogl.palAdd.red, fp);
CFWriteShort (gameStates.ogl.palAdd.green, fp);
CFWriteShort (gameStates.ogl.palAdd.blue, fp);
CFWrite (gameData.render.lights.subtracted, sizeof (gameData.render.lights.subtracted [0]), MAX_SEGMENTS, fp);
CFWriteInt (gameStates.app.bFirstSecretVisit, fp);
CFWriteFix (xOmegaCharge, fp);
CFWriteShort (gameData.missions.nEnteredFromLevel, fp);

}

//------------------------------------------------------------------------------

int StateSaveAllSub (char *filename, char *szDesc, int bBetweenLevels)
{
	int			i;
	CFILE			*fp;
	grs_canvas	*cnv;

	Assert (bBetweenLevels == 0);	//between levels save ripped out
	StopTime ();
/*	if (gameData.app.nGameMode & GM_MULTI)	{
		{
		StartTime ();
		return 0;
		}
	}*/

	fp = CFOpen (filename, gameFolders.szSaveDir, "wb", 0);
	if (!fp) {
		if (! (gameData.app.nGameMode & GM_MULTI))
			ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_SAVE_ERROR2);
		StartTime ();
		return 0;
	}

//Save id
	CFWrite (dgss_id, sizeof (char)*4, 1, fp);

//Save sgVersion
	i = STATE_VERSION;
	CFWrite (&i, sizeof (int), 1, fp);

//Save description
	CFWrite (szDesc, sizeof (char) * DESC_LENGTH, 1, fp);
	
// Save the current screen shot...

	cnv = GrCreateCanvas (THUMBNAIL_LW, THUMBNAIL_LH);
	if (cnv)	{
		ubyte			*buf;
		grsBitmap	tmp;
		int			k, x, y;
		grs_canvas * cnv_save;
		cnv_save = grdCurCanv;
		
		GrSetCurrentCanvas (cnv);
		PA_DFX (pa_set_backbuffer_current ());
		RenderFrame (0, 0);
		PA_DFX (pa_alpha_always ());
			
		//buf = d_malloc (THUMBNAIL_LW * THUMBNAIL_LH * 3);
		if (curDrawBuffer == GL_BACK)
			GameRenderFrame ();
		glReadBuffer (GL_FRONT);
		buf = d_malloc (grdCurScreen->sc_w * grdCurScreen->sc_h * 3);
		glReadPixels (0, 0, grdCurScreen->sc_w, grdCurScreen->sc_h, GL_RGB, GL_UNSIGNED_BYTE, buf);
		tmp.bm_props.w = grdCurScreen->sc_w;
		tmp.bm_props.h = grdCurScreen->sc_h;
		tmp.bm_texBuf = buf;
		// do a nice, half-way smart (by merging pixel groups using their average color) image resize
		ShrinkTGA (&tmp, grdCurScreen->sc_w / THUMBNAIL_LW, grdCurScreen->sc_h / THUMBNAIL_LH, 0, 3);
		GrPaletteStepLoad (NULL);
		// convert the resized TGA to bmp
		for (y = 0; y < THUMBNAIL_LH; y++) {
			i = y * (grdCurScreen->sc_w / (grdCurScreen->sc_w / THUMBNAIL_LW)) * 3;
			k = (THUMBNAIL_LH - y - 1) * THUMBNAIL_LW;
			for (x = 0; x < THUMBNAIL_LW; x++, k++, i += 3)
				cnv->cv_bitmap.bm_texBuf [k] =
					GrFindClosestColor (gamePalette, buf [i] / 4, buf [i+1] / 4, buf [i+2] / 4);
				}
		GrPaletteStepLoad (NULL);
		d_free (buf);
		CFWrite (cnv->cv_bitmap.bm_texBuf, THUMBNAIL_LW * THUMBNAIL_LH, 1, fp);
	WINDOS (
		DDGrSetCurrentCanvas (cnv_save),
		GrSetCurrentCanvas (cnv_save)
		);
	GrFreeCanvas (cnv);
	CFWrite (gamePalette, 3, 256, fp);
	}
else	{
 	ubyte color = 0;
 	for (i=0; i<THUMBNAIL_LW*THUMBNAIL_LH; i++)
		CFWrite (&color, sizeof (ubyte), 1, fp);
	}
StateSaveUniGameData (fp, bBetweenLevels);
if (CFError (fp)) {
	if (! (gameData.app.nGameMode & GM_MULTI)) {
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_SAVE_ERROR);
		CFClose (fp);
		CFDelete (filename, gameFolders.szSaveDir);
		}
	}
else  {
	CFClose (fp);
	}
StartTime ();
return 1;
}

//	-----------------------------------------------------------------------------------
//	Set the tPlayer's position from the globals gameData.segs.secret.nReturnSegment and gameData.segs.secret.returnOrient.
void SetPosFromReturnSegment (void)
{
	int	plobjnum = gameData.multi.players [gameData.multi.nLocalPlayer].nObject;

COMPUTE_SEGMENT_CENTER_I (&gameData.objs.objects [plobjnum].position.vPos, 
							     gameData.segs.secret.nReturnSegment);
RelinkObject (plobjnum, gameData.segs.secret.nReturnSegment);
ResetPlayerObject ();
gameData.objs.objects [plobjnum].position.mOrient = gameData.segs.secret.returnOrient;
}

//	-----------------------------------------------------------------------------------

int StateRestoreAll (int bInGame, int bSecretRestore, char *pszFilenameOverride)
{
	char filename [128];
	int	i, filenum = -1;

if (gameData.app.nGameMode & GM_MULTI)	{
#	ifdef MULTI_SAVE
	MultiInitiateRestoreGame ();
#	endif
	return 0;
	}
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDStopRecording ();
if (gameData.demo.nState != ND_STATE_NORMAL)
	return 0;
StopTime ();
gameData.app.bGamePaused = 1;
if (pszFilenameOverride) {
	strcpy (filename, pszFilenameOverride);
	filenum = NUM_SAVES+1;		//	So we don't tTrigger autosave
	}
else if (! (filenum = StateGetRestoreFile (filename, 0)))	{
	gameData.app.bGamePaused = 0;
	StartTime ();
	return 0;
	}
//	MK, 1/1/96
//	If not in multiplayer, do special secret level stuff.
//	If Nsecret.sgc (where N = filenum) exists, then copy it to secret.sgc.
//	If it doesn't exist, then delete secret.sgc

if (!bSecretRestore && !(gameData.app.nGameMode & GM_MULTI)) {
	int	rval;
	char	temp_fname [32], fc;

	if (filenum != -1) {
		if (filenum >= 10)
			fc = (filenum-10) + 'a';
		else
			fc = '0' + filenum;
		sprintf (temp_fname, "%csecret.sgc", fc);
		if (CFExist (temp_fname,gameFolders.szSaveDir,0)) {
			rval = copy_file (temp_fname, SECRETC_FILENAME);
			Assert (rval == 0);	//	Oops, error copying temp_fname to secret.sgc!
			}
		else
			CFDelete (SECRETC_FILENAME, gameFolders.szSaveDir);
		}
	}
	//	Changed, 11/15/95, MK, don't to autosave if restoring from main menu.
if ((filenum != (NUM_SAVES + 1)) && bInGame) {
	char	temp_filename [128];
	sprintf (temp_filename, "%s.sg%x", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, NUM_SAVES);
	StateSaveAll (!bInGame, bSecretRestore, temp_filename);
	}
if (!bSecretRestore && bInGame) {
	int choice = ExecMessageBox (NULL, NULL, 2, TXT_YES, TXT_NO, TXT_CONFIRM_LOAD);
	if (choice != 0) {
		gameData.app.bGamePaused = 0;
		StartTime ();
		return 0;
		}
	}
gameStates.app.bGameRunning = 0;
i = StateRestoreAllSub (filename, 0, bSecretRestore);
gameData.app.bGamePaused = 0;
StartTime ();
return i;
}

//------------------------------------------------------------------------------

int CFReadBoundedInt (int nMax, int *nVal, CFILE *fp)
{
	int	i;

i = CFReadInt (fp);
if ((i < 0) || (i > nMax)) {
	Warning (TXT_SAVE_CORRUPT);
	CFClose (fp);
	return 1;
	}
*nVal = i;
return 0;
}

//------------------------------------------------------------------------------

int StateLoadMission (CFILE *fp)
{
	char	szMission [16];

CFRead (szMission, sizeof (char), 9, fp);
if (LoadMissionByName (szMission, -1))
	return 1;
ExecMessageBox (NULL, NULL, 1, "Ok", TXT_MSN_LOAD_ERROR, szMission);
//CFClose (fp);
return 0;
}

//------------------------------------------------------------------------------

void StateRestoreMultiGame (char *pszOrgCallSign, int bMulti, int bSecretRestore)
{
if (bMulti)
	strcpy (pszOrgCallSign, gameData.multi.players [gameData.multi.nLocalPlayer].callsign);
else {
	gameData.app.nGameMode = GM_NORMAL;
	SetFunctionMode (FMODE_GAME);
	ChangePlayerNumTo (0);
	strcpy (pszOrgCallSign, gameData.multi.players [0].callsign);
	gameData.multi.nPlayers = 1;
	if (!bSecretRestore) {
		InitMultiPlayerObject ();	//make sure tPlayer's tObject set up
		InitPlayerStatsGame ();		//clear all stats
		}
	}
}

//------------------------------------------------------------------------------

int StateSetServerPlayer (tPlayer *restore_players, int nPlayers, char *pszServerCallSign,
								  int *pnOtherObjNum, int *pnServerObjNum)
{
	int	i,
			nServerPlayer = -1,
			nOtherObjNum = -1, 
			nServerObjNum = -1;

if (gameStates.multi.nGameType >= IPX_GAME) {
	if (gameStates.multi.bServer || NetworkIAmMaster ())
		nServerPlayer = gameData.multi.nLocalPlayer;
	else {
		nServerPlayer = -1;
		for (i = 0; i < nPlayers; i++)
			if (!strcmp (pszServerCallSign, netPlayers.players [i].callsign)) {
				nServerPlayer = i;
				break;
				}
		}
	if (nServerPlayer > 0) {
		nOtherObjNum = restore_players [0].nObject;
		nServerObjNum = restore_players [nServerPlayer].nObject;
		{
		tNetPlayerInfo h = netPlayers.players [0];
		netPlayers.players [0] = netPlayers.players [nServerPlayer];
		netPlayers.players [nServerPlayer] = h;
		}
		{
		tPlayer h = restore_players [0];
		restore_players [0] = restore_players [nServerPlayer];
		restore_players [nServerPlayer] = h;
		}
		if (gameStates.multi.bServer || NetworkIAmMaster ())
			gameData.multi.nLocalPlayer = 0;
		else if (!gameData.multi.nLocalPlayer)
			gameData.multi.nLocalPlayer = nServerPlayer;
		}
	memcpy (netPlayers.players [gameData.multi.nLocalPlayer].network.ipx.node, 
			 IpxGetMyLocalAddress (), 6);
	* ((ushort *) (netPlayers.players [gameData.multi.nLocalPlayer].network.ipx.node + 4)) = 
		htons (* ((ushort *) (netPlayers.players [gameData.multi.nLocalPlayer].network.ipx.node + 4)));
	}
*pnOtherObjNum = nOtherObjNum;
*pnServerObjNum = nServerObjNum;
return nServerPlayer;
}

//------------------------------------------------------------------------------

void StateGetConnectedPlayers (tPlayer *restore_players, int nPlayers)
{
	int	i, j;

for (i = 0; i < nPlayers; i++) {
	for (j = 0; j < nPlayers; j++) {
      if ((!stricmp (restore_players [i].callsign, gameData.multi.players [j].callsign)) && 
			 gameData.multi.players [j].connected) {
			restore_players [i].connected = 1;
			break;
			}
		}
	}
memcpy (gameData.multi.players, restore_players, sizeof (tPlayer) * nPlayers);
gameData.multi.nPlayers = nPlayers;
if (NetworkIAmMaster ()) {
	for (i = 0; i < gameData.multi.nPlayers; i++) {
		if (i == gameData.multi.nLocalPlayer)
			continue;
   	gameData.multi.players [i].connected = 0;
		}
	}
}

//------------------------------------------------------------------------------

void StateFixNetworkObjects (int nServerPlayer, int nOtherObjNum, int nServerObjNum)
{
if (IsMultiGame && (gameStates.multi.nGameType >= IPX_GAME) && (nServerPlayer > 0)) {
	tObject h = gameData.objs.objects [nServerObjNum];
	gameData.objs.objects [nServerObjNum] = gameData.objs.objects [nOtherObjNum];
	gameData.objs.objects [nOtherObjNum] = h;
	gameData.objs.objects [nServerObjNum].id = nServerObjNum;
	gameData.objs.objects [nOtherObjNum].id = 0;
	if (gameData.multi.nLocalPlayer == nServerObjNum) {
		gameData.objs.objects [nServerObjNum].controlType = CT_FLYING;
		gameData.objs.objects [nOtherObjNum].controlType = CT_REMOTE;
		}
	else if (gameData.multi.nLocalPlayer == nOtherObjNum) {
		gameData.objs.objects [nServerObjNum].controlType = CT_REMOTE;
		gameData.objs.objects [nOtherObjNum].controlType = CT_FLYING;
		}
	}
}

//------------------------------------------------------------------------------

void StateFixObjects (void)
{
	tObject	*objP = gameData.objs.objects;
	int		i, nSegment;
	
gameData.objs.nNextSignature = 0;
for (i = 0; i <= gameData.objs.nLastObject; i++, objP++) {
	objP->rType.polyObjInfo.nAltTextures = -1;
	nSegment = objP->nSegment;
	// hack for a bug I haven't yet been able to fix
	if (objP->shields < 0)
		objP->nType = OBJ_NONE;
	objP->next = objP->prev = objP->nSegment = -1;
	if (objP->nType != OBJ_NONE) {
		LinkObject (i,nSegment);
		if (objP->nSignature > gameData.objs.nNextSignature)
			gameData.objs.nNextSignature = objP->nSignature;
		}
	//look for, and fix, boss with bogus shields
	if (objP->nType == OBJ_ROBOT && gameData.bots.pInfo [objP->id].bossFlag) {
		fix save_shields = objP->shields;
		copy_defaults_toRobot (objP);		//calculate starting shields
		//if in valid range, use loaded shield value
		if (save_shields > 0 && (save_shields <= objP->shields))
			objP->shields = save_shields;
		else
			objP->shields /= 2;  //give tPlayer a break
		}
	}	
}

//------------------------------------------------------------------------------

void StateAwardReturningPlayer (tPlayer *retPlayerP, fix xOldGameTime)
{
tPlayer *playerP = gameData.multi.players + gameData.multi.nLocalPlayer;
playerP->level = retPlayerP->level;
playerP->last_score = retPlayerP->last_score;
playerP->timeLevel = retPlayerP->timeLevel;
playerP->flags |= (retPlayerP->flags & PLAYER_FLAGS_ALL_KEYS);
playerP->numRobotsLevel = retPlayerP->numRobotsLevel;
playerP->numRobotsTotal = retPlayerP->numRobotsTotal;
playerP->hostages_rescuedTotal = retPlayerP->hostages_rescuedTotal;
playerP->hostagesTotal = retPlayerP->hostagesTotal;
playerP->hostages_on_board = retPlayerP->hostages_on_board;
playerP->hostagesLevel = retPlayerP->hostagesLevel;
playerP->homingObjectDist = retPlayerP->homingObjectDist;
playerP->hoursLevel = retPlayerP->hoursLevel;
playerP->hoursTotal = retPlayerP->hoursTotal;
//playerP->nCloaks = retPlayerP->nCloaks;
//playerP->nInvuls = retPlayerP->nInvuls;
DoCloakInvulSecretStuff (xOldGameTime);
}

//------------------------------------------------------------------------------

void StateRestoreNetGame (CFILE *fp)
{
	int	i, j;

netGame.nType = CFReadByte (fp);
netGame.Security = CFReadInt (fp);
CFRead (netGame.game_name, 1, NETGAME_NAME_LEN + 1, fp);
CFRead (netGame.mission_title, 1, MISSION_NAME_LEN + 1, fp);
CFRead (netGame.mission_name, 1, 9, fp);
netGame.levelnum = CFReadInt (fp);
netGame.gamemode = (ubyte) CFReadByte (fp);
netGame.RefusePlayers = (ubyte) CFReadByte (fp);
netGame.difficulty = (ubyte) CFReadByte (fp);
netGame.game_status = (ubyte) CFReadByte (fp);
netGame.numplayers = (ubyte) CFReadByte (fp);
netGame.max_numplayers = (ubyte) CFReadByte (fp);
netGame.numconnected = (ubyte) CFReadByte (fp);
netGame.gameFlags = (ubyte) CFReadByte (fp);
netGame.protocol_version = (ubyte) CFReadByte (fp);
netGame.version_major = (ubyte) CFReadByte (fp);
netGame.version_minor = (ubyte) CFReadByte (fp);
netGame.team_vector = (ubyte) CFReadByte (fp);
netGame.DoMegas = (ubyte) CFReadByte (fp);
netGame.DoSmarts = (ubyte) CFReadByte (fp);
netGame.DoFusions = (ubyte) CFReadByte (fp);
netGame.DoHelix = (ubyte) CFReadByte (fp);
netGame.DoPhoenix = (ubyte) CFReadByte (fp);
netGame.DoAfterburner = (ubyte) CFReadByte (fp);
netGame.DoInvulnerability = (ubyte) CFReadByte (fp);
netGame.DoCloak = (ubyte) CFReadByte (fp);
netGame.DoGauss = (ubyte) CFReadByte (fp);
netGame.DoVulcan = (ubyte) CFReadByte (fp);
netGame.DoPlasma = (ubyte) CFReadByte (fp);
netGame.DoOmega = (ubyte) CFReadByte (fp);
netGame.DoSuperLaser = (ubyte) CFReadByte (fp);
netGame.DoProximity = (ubyte) CFReadByte (fp);
netGame.DoSpread = (ubyte) CFReadByte (fp);
netGame.DoSmartMine = (ubyte) CFReadByte (fp);
netGame.DoFlash = (ubyte) CFReadByte (fp);
netGame.DoGuided = (ubyte) CFReadByte (fp);
netGame.DoEarthShaker = (ubyte) CFReadByte (fp);
netGame.DoMercury = (ubyte) CFReadByte (fp);
netGame.bAllowMarkerView = (ubyte) CFReadByte (fp);
netGame.AlwaysLighting = (ubyte) CFReadByte (fp);
netGame.DoAmmoRack = (ubyte) CFReadByte (fp);
netGame.DoConverter = (ubyte) CFReadByte (fp);
netGame.DoHeadlight = (ubyte) CFReadByte (fp);
netGame.DoHoming = (ubyte) CFReadByte (fp);
netGame.DoLaserUpgrade = (ubyte) CFReadByte (fp);
netGame.DoQuadLasers = (ubyte) CFReadByte (fp);
netGame.ShowAllNames = (ubyte) CFReadByte (fp);
netGame.BrightPlayers = (ubyte) CFReadByte (fp);
netGame.invul = (ubyte) CFReadByte (fp);
netGame.FriendlyFireOff = (ubyte) CFReadByte (fp);
for (i = 0; i < 2; i++)
	CFRead (netGame.team_name [i], 1, CALLSIGN_LEN + 1, fp);		// 18 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.locations [i] = CFReadInt (fp);
for (i = 0; i < MAX_PLAYERS; i++)
	for (j = 0; j < MAX_PLAYERS; j++)
		netGame.kills [i] [j] = CFReadShort (fp);			// 128 bytes
netGame.segments_checksum = CFReadShort (fp);				// 2 bytes
for (i = 0; i < 2; i++)
	netGame.teamKills [i] = CFReadShort (fp);				// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.killed [i] = CFReadShort (fp);					// 16 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.playerKills [i] = CFReadShort (fp);			// 16 bytes
netGame.KillGoal = CFReadInt (fp);							// 4 bytes
netGame.PlayTimeAllowed = CFReadFix (fp);					// 4 bytes
netGame.levelTime = CFReadFix (fp);							// 4 bytes
netGame.control_invulTime = CFReadInt (fp);				// 4 bytes
netGame.monitor_vector = CFReadInt (fp);					// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.player_score [i] = CFReadInt (fp);				// 32 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.playerFlags [i] = (ubyte) CFReadByte (fp);	// 8 bytes
netGame.nPacketsPerSec = CFReadShort (fp);					// 2 bytes
netGame.bShortPackets = (ubyte) CFReadByte (fp);			// 1 bytes
// 279 bytes
// 355 bytes total
CFRead (netGame.AuxData, NETGAME_AUX_SIZE, 1, fp);  // Storage for protocol-specific data (e.g., multicast session and port)
}

//------------------------------------------------------------------------------

void StateRestoreNetPlayers (CFILE *fp)
{
	int	i;

netPlayers.nType = (ubyte) CFReadByte (fp);
netPlayers.Security = CFReadInt (fp);
for (i = 0; i < MAX_PLAYERS + 4; i++) {
	CFRead (netPlayers.players [i].callsign, 1, CALLSIGN_LEN + 1, fp);
	CFRead (netPlayers.players [i].network.ipx.server, 1, 4, fp);
	CFRead (netPlayers.players [i].network.ipx.node, 1, 6, fp);
	netPlayers.players [i].version_major = (ubyte) CFReadByte (fp);
	netPlayers.players [i].version_minor = (ubyte) CFReadByte (fp);
	netPlayers.players [i].computerType = (ubyte) CFReadByte (fp);
	netPlayers.players [i].connected = CFReadByte (fp);
	netPlayers.players [i].socket = (ushort) CFReadShort (fp);
	netPlayers.players [i].rank = (ubyte) CFReadByte (fp);
	}
}

//------------------------------------------------------------------------------

void StateRestorePlayer (tPlayer *playerP, CFILE *fp)
{
	int	i;

CFRead (playerP->callsign, 1, CALLSIGN_LEN + 1, fp); // The callsign of this tPlayer, for net purposes.
CFRead (playerP->netAddress, 1, 6, fp);					// The network address of the player.
playerP->connected = CFReadByte (fp);            // Is the tPlayer connected or not?
playerP->nObject = CFReadInt (fp);                // What tObject number this tPlayer is. (made an int by mk because it's very often referenced)
playerP->nPacketsGot = CFReadInt (fp);         // How many packets we got from them
playerP->nPacketsSent = CFReadInt (fp);        // How many packets we sent to them
playerP->flags = (uint) CFReadInt (fp);           // Powerup flags, see below...
playerP->energy = CFReadFix (fp);                // Amount of energy remaining.
playerP->shields = CFReadFix (fp);               // shields remaining (protection)
playerP->lives = CFReadByte (fp);                // Lives remaining, 0 = game over.
playerP->level = CFReadByte (fp);                // Current level tPlayer is playing. (must be signed for secret levels)
playerP->laserLevel = (ubyte) CFReadByte (fp);  // Current level of the laser.
playerP->startingLevel = CFReadByte (fp);       // What level the tPlayer started on.
playerP->nKillerObj = CFReadShort (fp);       // Who killed me.... (-1 if no one)
playerP->primaryWeaponFlags = (ushort) CFReadShort (fp);   // bit set indicates the tPlayer has this weapon.
playerP->secondaryWeaponFlags = (ushort) CFReadShort (fp); // bit set indicates the tPlayer has this weapon.
for (i = 0; i < MAX_PRIMARY_WEAPONS; i++)
	playerP->primaryAmmo [i] = (ushort) CFReadShort (fp); // How much ammo of each nType.
for (i = 0; i < MAX_SECONDARY_WEAPONS; i++)
	playerP->secondaryAmmo [i] = (ushort) CFReadShort (fp); // How much ammo of each nType.
#if 1 //for inventory system
playerP->nInvuls = (ubyte) CFReadByte (fp);
playerP->nCloaks = (ubyte) CFReadByte (fp);
#endif
playerP->last_score = CFReadInt (fp);           // Score at beginning of current level.
playerP->score = CFReadInt (fp);                // Current score.
playerP->timeLevel = CFReadFix (fp);            // Level time played
playerP->timeTotal = CFReadFix (fp);				// Game time played (high word = seconds)
playerP->cloakTime = CFReadFix (fp);					// Time cloaked
if (playerP->cloakTime != 0x7fffffff)
	playerP->cloakTime += gameData.time.xGame;
playerP->invulnerableTime = CFReadFix (fp);      // Time invulnerable
if (playerP->invulnerableTime != 0x7fffffff)
	playerP->invulnerableTime += gameData.time.xGame;
playerP->nKillGoalCount = CFReadShort (fp);          // Num of players killed this level
playerP->netKilledTotal = CFReadShort (fp);       // Number of times killed total
playerP->netKillsTotal = CFReadShort (fp);        // Number of net kills total
playerP->numKillsLevel = CFReadShort (fp);        // Number of kills this level
playerP->numKillsTotal = CFReadShort (fp);        // Number of kills total
playerP->numRobotsLevel = CFReadShort (fp);       // Number of initial robots this level
playerP->numRobotsTotal = CFReadShort (fp);       // Number of robots total
playerP->hostages_rescuedTotal = (ushort) CFReadShort (fp); // Total number of hostages rescued.
playerP->hostagesTotal = (ushort) CFReadShort (fp);         // Total number of hostages.
playerP->hostages_on_board = (ubyte) CFReadByte (fp);      // Number of hostages on ship.
playerP->hostagesLevel = (ubyte) CFReadByte (fp);         // Number of hostages on this level.
playerP->homingObjectDist = CFReadFix (fp);     // Distance of nearest homing tObject.
playerP->hoursLevel = CFReadByte (fp);            // Hours played (since timeTotal can only go up to 9 hours)
playerP->hoursTotal = CFReadByte (fp);            // Hours played (since timeTotal can only go up to 9 hours)
}

//------------------------------------------------------------------------------

void StateRestoreObject (tObject *objP, CFILE *fp)
{
objP->nSignature = CFReadInt (fp);      
objP->nType = (ubyte) CFReadByte (fp); 
objP->id = (ubyte) CFReadByte (fp);
objP->next = CFReadShort (fp);
objP->prev = CFReadShort (fp);
objP->controlType = (ubyte) CFReadByte (fp);
objP->movementType = (ubyte) CFReadByte (fp);
objP->renderType = (ubyte) CFReadByte (fp);
objP->flags = (ubyte) CFReadByte (fp);
objP->nSegment = CFReadShort (fp);
objP->attachedObj = CFReadShort (fp);
CFReadVector (&objP->position.vPos, fp);     
CFReadMatrix (&objP->position.mOrient, fp);  
objP->size = CFReadFix (fp); 
objP->shields = CFReadFix (fp);
CFReadVector (&objP->last_pos, fp);  
objP->containsType = CFReadByte (fp); 
objP->containsId = CFReadByte (fp);   
objP->containsCount = CFReadByte (fp);
objP->matCenCreator = CFReadByte (fp);
objP->lifeleft = CFReadFix (fp);   
if (objP->movementType == MT_PHYSICS) {
	CFReadVector (&objP->mType.physInfo.velocity, fp);   
	CFReadVector (&objP->mType.physInfo.thrust, fp);     
	objP->mType.physInfo.mass = CFReadFix (fp);       
	objP->mType.physInfo.drag = CFReadFix (fp);       
	objP->mType.physInfo.brakes = CFReadFix (fp);     
	CFReadVector (&objP->mType.physInfo.rotVel, fp);     
	CFReadVector (&objP->mType.physInfo.rotThrust, fp);  
	objP->mType.physInfo.turnRoll = CFReadFixAng (fp);   
	objP->mType.physInfo.flags = (ushort) CFReadShort (fp);      
	}
else if (objP->movementType == MT_SPINNING) {
	CFReadVector (&objP->mType.spinRate, fp);  
	}
switch (objP->controlType) {
	case CT_WEAPON:
		objP->cType.laserInfo.parentType = CFReadShort (fp);
		objP->cType.laserInfo.nParentObj = CFReadShort (fp);
		objP->cType.laserInfo.nParentSig = CFReadInt (fp);
		objP->cType.laserInfo.creationTime = CFReadFix (fp);
		objP->cType.laserInfo.nLastHitObj = CFReadShort (fp);
		objP->cType.laserInfo.nTrackGoal = CFReadShort (fp);
		objP->cType.laserInfo.multiplier = CFReadFix (fp);
		break;

	case CT_EXPLOSION:
		objP->cType.explInfo.nSpawnTime = CFReadFix (fp);
		objP->cType.explInfo.nDeleteTime = CFReadFix (fp);
		objP->cType.explInfo.nDeleteObj = CFReadShort (fp);
		objP->cType.explInfo.nAttachParent = CFReadShort (fp);
		objP->cType.explInfo.nPrevAttach = CFReadShort (fp);
		objP->cType.explInfo.nNextAttach = CFReadShort (fp);
		break;

	case CT_AI:
		objP->cType.aiInfo.behavior = (ubyte) CFReadByte (fp);
		CFRead (objP->cType.aiInfo.flags, 1, MAX_AI_FLAGS, fp);
		objP->cType.aiInfo.nHideSegment = CFReadShort (fp);
		objP->cType.aiInfo.nHideIndex = CFReadShort (fp);
		objP->cType.aiInfo.nPathLength = CFReadShort (fp);
		objP->cType.aiInfo.nCurPathIndex = CFReadByte (fp);
		objP->cType.aiInfo.bDyingSoundPlaying = CFReadByte (fp);
		objP->cType.aiInfo.nDangerLaser = CFReadShort (fp);
		objP->cType.aiInfo.nDangerLaserSig = CFReadInt (fp);
		objP->cType.aiInfo.xDyingStartTime = CFReadFix (fp);
		break;

	case CT_LIGHT:
		objP->cType.lightInfo.intensity = CFReadFix (fp);
		break;

	case CT_POWERUP:
		objP->cType.powerupInfo.count = CFReadInt (fp);
		objP->cType.powerupInfo.creationTime = CFReadFix (fp);
		objP->cType.powerupInfo.flags = CFReadInt (fp);
		break;
	}
switch (objP->renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		objP->rType.polyObjInfo.nModel = CFReadInt (fp);
		for (i = 0; i < MAX_SUBMODELS; i++)
			CFReadAngVec (objP->rType.polyObjInfo.animAngles + i, fp);
		objP->rType.polyObjInfo.nSubObjFlags = CFReadInt (fp);
		objP->rType.polyObjInfo.nTexOverride = CFReadInt (fp);
		objP->rType.polyObjInfo.nAltTextures = CFReadInt (fp);
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		objP->rType.vClipInfo.nClipIndex = CFReadInt (fp);
		objP->rType.vClipInfo.xFrameTime = CFReadFix (fp);
		objP->rType.vClipInfo.nCurFrame = CFReadByte (fp);
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------

void StateRestoreWall (wall *wallP, CFILE *fp)
{
wallP->nSegment = CFReadInt (fp);
wallP->nSide = CFReadInt (fp);
wallP->hps = CFReadFix (fp);    
wallP->nLinkedWall = CFReadInt (fp);
wallP->nType = (ubyte) CFReadByte (fp);       
wallP->flags = (ubyte) CFReadByte (fp);      
wallP->state = (ubyte) CFReadByte (fp);      
wallP->nTrigger = (ubyte) CFReadByte (fp);    
wallP->nClip = CFReadByte (fp);   
wallP->keys = (ubyte) CFReadByte (fp);       
wallP->controllingTrigger = CFReadByte (fp);
wallP->cloakValue = CFReadByte (fp); 
}

//------------------------------------------------------------------------------

void StateRestoreExplWall (tExplWall *wallP, CFILE *fp)
{
wallP->nSegment = CFReadInt (fp);
wallP->nSide = CFReadInt (fp);
wallP->time = CFReadFix (fp);    
}

//------------------------------------------------------------------------------

void StateRestoreCloakingWall (tCloakingWall *wallP, CFILE *fp)
{
	int	i;

wallP->nFrontWall = CFReadShort (fp);
wallP->nBackWall = CFReadShort (fp); 
for (i = 0; i < 4; i++) {
	wallP->front_ls [i] = CFReadFix (fp); 
	wallP->back_ls [i] = CFReadFix (fp);
	}
wallP->time = CFReadFix (fp);    
}

//------------------------------------------------------------------------------

void StateRestoreActiveDoor (tActiveDoor *doorP, CFILE *fp)
{
	int	i;

doorP->nPartCount = CFReadInt (fp);
for (i = 0; i < 2; i++) {
	doorP->nFrontWall [i] = CFReadShort (fp);
	doorP->nBackWall [i] = CFReadShort (fp);
	}
doorP->time = CFReadFix (fp);    
}

//------------------------------------------------------------------------------

void StateRestoreTrigger (tTrigger *triggerP, CFILE *fp)
{
	int	i;

triggerP->nType = (ubyte) CFReadByte (fp); 
triggerP->flags = (ubyte) CFReadByte (fp); 
triggerP->nLinks = CFReadByte (fp);
triggerP->value = CFReadFix (fp);
triggerP->time = CFReadFix (fp);
for (i = 0; i < MAX_WALLS_PER_LINK; i++) {
	triggerP->nSegment [i] = CFReadShort (fp);
	triggerP->nSide [i] = CFReadShort (fp);
	}
}

//------------------------------------------------------------------------------

void StateRestoreObjTriggerRef (tObjTriggerRef *refP, CFILE *fp)
{
refP->prev = CFReadShort (fp);
refP->next = CFReadShort (fp);
refP->nObject = CFReadShort (fp);
}

//------------------------------------------------------------------------------

void StateRestoreMatCen (tMatCenInfo *matcenP, CFILE *fp)
{
	int	i;

for (i = 0; i < 2; i++)
	matcenP->robotFlags [i] = CFReadInt (fp);
matcenP->hit_points = CFReadFix (fp);
matcenP->interval = CFReadFix (fp);
matcenP->nSegment = CFReadShort (fp);
matcenP->fuelcen_num = CFReadShort (fp);
}

//------------------------------------------------------------------------------

void StateRestoreFuelCen (tFuelCenInfo *fuelcenP, CFILE *fp)
{
fuelcenP->Type = CFReadInt (fp);
fuelcenP->nSegment = CFReadInt (fp);
fuelcenP->Flag = CFReadByte (fp);
fuelcenP->Enabled = CFReadByte (fp);
fuelcenP->Lives = CFReadByte (fp);
fuelcenP->Capacity = CFReadFix (fp);
fuelcenP->MaxCapacity = CFReadFix (fp);
fuelcenP->Timer = CFReadFix (fp);
fuelcenP->DisableTime = CFReadFix (fp);
CFReadVector (&fuelcenP->Center, fp);
}

//------------------------------------------------------------------------------

void StateRestoreReactorTrigger (tReactorTriggers *triggerP, CFILE *fp)
{
	int	i;

triggerP->nLinks = CFReadShort (fp);
for (i = 0; i < MAX_CONTROLCEN_LINKS; i++) {
	triggerP->nSegment [i] = CFReadShort (fp);
	triggerP->nSide [i] = CFReadShort (fp);
	}
}

//------------------------------------------------------------------------------

int StateRestoreUniGameData (CFILE *fp, int sgVersion, int bMulti, int bSecretRestore, fix xOldGameTime)
{
	tPlayer	restore_players [MAX_PLAYERS];
	int		nPlayers, nServerPlayer = -1;
	int		nOtherObjNum = -1, nServerObjNum = -1, nLocalObjNum = -1, nSavedLocalPlayer = -1;
	int		bBetweenLevels;
	int		nCurrentLevel, nNextLevel;
	wall		*wallP;
	char		szOrgCallSign [CALLSIGN_LEN+16];
	int		h, i, j, fpos;
	short		nTexture;

bBetweenLevels = CFReadInt (fp);
Assert (bBetweenLevels == 0);	//between levels save ripped out
// Read the mission info...
if (!StateLoadMission (fp))
	return 0;
//Read level info
nCurrentLevel = CFReadInt (fp);
nNextLevel = CFReadInt (fp);
//Restore gameData.time.xGame
gameData.time.xGame = CFReadFix (fp);
// Start new game....
StateRestoreMultiGame (szOrgCallSign, bMulti, bSecretRestore);
if (gameData.app.nGameMode & GM_MULTI) {
		char szServerCallSign [CALLSIGN_LEN + 1];

	strcpy (szServerCallSign, netPlayers.players [0].callsign);
	gameData.app.nStateGameId = CFReadInt (fp);
	StateRestoreNetGame (fp);
	fpos = CFTell (fp);
	StateRestoreNetPlayers (fp);
	fpos = CFTell (fp);
	nPlayers = CFReadInt (fp);
	for (i = 0; i < nPlayers; i++)
		nSavedLocalPlayer = gameData.multi.nLocalPlayer;
	for (i = 0; i < nPlayers; i++)
		StateRestorePlayer (restore_players + i, fp);
	fpos = CFTell (fp);
	// make sure the current game host is in tPlayer slot #0
	nServerPlayer = StateSetServerPlayer (restore_players, nPlayers, szServerCallSign, &nOtherObjNum, &nServerObjNum);
	StateGetConnectedPlayers (restore_players, nPlayers);
	}
//Read tPlayer info
if (!StartNewLevelSub (nCurrentLevel, 1, bSecretRestore)) {
	CFClose (fp);
	return 0;
	}
nLocalObjNum = gameData.multi.players [gameData.multi.nLocalPlayer].nObject;
if (bSecretRestore != 1)	//either no secret restore, or tPlayer died in scret level
	StateRestorePlayer (gameData.multi.players + gameData.multi.nLocalPlayer, fp);
else {
	tPlayer	retPlayer;
	StateRestorePlayer (&retPlayer, fp);
	StateAwardReturningPlayer (&retPlayer, xOldGameTime);
	}
gameData.multi.players [gameData.multi.nLocalPlayer].nObject = nLocalObjNum;
strcpy (gameData.multi.players [gameData.multi.nLocalPlayer].callsign, szOrgCallSign);
// Set the right level
if (bBetweenLevels)
	gameData.multi.players [gameData.multi.nLocalPlayer].level = nNextLevel;
// Restore the weapon states
gameData.weapons.nPrimary = CFReadByte (fp);
gameData.weapons.nSecondary = CFReadByte (fp);
SelectWeapon (gameData.weapons.nPrimary, 0, 0, 0);
SelectWeapon (gameData.weapons.nSecondary, 1, 0, 0);
// Restore the difficulty level
gameStates.app.nDifficultyLevel = CFReadInt (fp);
gameStates.app.nDifficultyLevel = gameOpts->gameplay.nPlayerDifficultyLevel;
// Restore the cheats enabled flag
gameStates.app.cheats.bEnabled = CFReadInt (fp);
if (!bBetweenLevels)	{
	gameStates.render.bDoAppearanceEffect = 0;			// Don't do this for middle o' game stuff.
	//Clear out all the gameData.objs.objects from the lvl file
	for (i = 0; i <= gameData.segs.nLastSegment; i++)
		gameData.segs.segments [i].objects = -1;
	ResetObjects (1);

	//Read objects, and pop 'em into their respective segments.
	h = CFReadInt (fp);
	gameData.objs.nLastObject = h - 1;
	for (i = 0; i < h; i++)
		StateRestoreObject (gameData.objs.objects + i, fp);
	fpos = CFTell (fp);
	StateFixNetworkObjects (nServerPlayer, nOtherObjNum, nServerObjNum);
	gameData.objs.nNextSignature = 0;
	StateFixObjects ();
	SpecialResetObjects ();
	InitCamBots (1);
	gameData.objs.nNextSignature++;
	//	1 = Didn't die on secret level.
	//	2 = Died on secret level.
	if (bSecretRestore && (gameData.missions.nCurrentLevel >= 0)) {
		SetPosFromReturnSegment ();
		if (bSecretRestore == 2)
			InitPlayerStatsNewShip ();
		}
	//Restore wall info
	if (CFReadBoundedInt (MAX_WALLS, &gameData.walls.nWalls, fp))
		return 0;
	for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; i++, wallP++) {
		StateRestoreWall (wallP, fp);
		if (wallP->nType == WALL_OPEN)
			DigiKillSoundLinkedToSegment ((short) wallP->nSegment, (short) wallP->nSide, -1);	//-1 means kill any sound
		}
	fpos = CFTell (fp);
	//Restore exploding wall info
	if (CFReadBoundedInt (MAX_EXPLODING_WALLS, &h, fp))
		return 0;
	for (i = 0; i < h; i++)
		StateRestoreExplWall (gameData.walls.explWalls + i, fp);
	fpos = CFTell (fp);
	//Restore door info
	if (CFReadBoundedInt (MAX_DOORS, &gameData.walls.nOpenDoors, fp))
		return 0;
	for (i = 0; i < gameData.walls.nOpenDoors; i++)
		StateRestoreActiveDoor (gameData.walls.activeDoors + i, fp);
	fpos = CFTell (fp);
	if (CFReadBoundedInt (MAX_WALLS, &gameData.walls.nCloaking, fp))
		return 0;
	for (i = 0; i < gameData.walls.nCloaking; i++)
		StateRestoreCloakingWall (gameData.walls.cloaking + i, fp);
	fpos = CFTell (fp);
	//Restore tTrigger info
	if (CFReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nTriggers, fp))
		return 0;
	for (i = 0; i < gameData.trigs.nTriggers; i++)
		StateRestoreTrigger (gameData.trigs.triggers + i, fp);
	fpos = CFTell (fp);
	//Restore tObject tTrigger info
	if (CFReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nObjTriggers, fp))
		return 0;
	if (gameData.trigs.nObjTriggers > 0) {
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			StateRestoreTrigger (gameData.trigs.objTriggers + i, fp);
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			StateRestoreObjTriggerRef (gameData.trigs.objTriggerRefs + i, fp);
		for (i = 0; i < MAX_OBJECTS_D2X; i++)
			gameData.trigs.firstObjTrigger [i] = CFReadShort (fp);
		}
	else
		CFSeek (fp, MAX_OBJECTS_D2X * sizeof (short), SEEK_CUR);
	fpos = CFTell (fp);
	//Restore tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++)	{
		for (j = 0; j < 6; j++)	{
			gameData.segs.segments [i].sides [j].nWall = CFReadShort (fp);
			gameData.segs.segments [i].sides [j].nBaseTex = CFReadShort (fp);
			nTexture = CFReadShort (fp);
			gameData.segs.segments [i].sides [j].nOvlTex = nTexture & 0x3fff;
			gameData.segs.segments [i].sides [j].nOvlOrient = (nTexture >> 14) & 3;
			}
		}
	fpos = CFTell (fp);
	//Restore the fuelcen info
	gameData.reactor.bDestroyed = CFReadInt (fp);
	gameData.reactor.countdown.nTimer = CFReadFix (fp);
	if (CFReadBoundedInt (MAX_ROBOT_CENTERS, &gameData.matCens.nRobotCenters, fp))
		return 0;
	for (i = 0; i < gameData.matCens.nRobotCenters; i++)
		StateRestoreMatCen (gameData.matCens.robotCenters + i, fp);
	StateRestoreReactorTrigger (&gameData.reactor.triggers, fp);
	if (CFReadBoundedInt (MAX_FUEL_CENTERS, &gameData.matCens.nFuelCenters, fp))
		return 0;
	for (i = 0; i < gameData.matCens.nFuelCenters; i++)
		StateRestoreFuelCen (gameData.matCens.fuelCenters + i, fp);
	fpos = CFTell (fp);
	// Restore the control cen info
	gameData.reactor.bHit = CFReadInt (fp);
	gameData.reactor.bSeenPlayer = CFReadInt (fp);
	gameData.reactor.nNextFireTime = CFReadInt (fp);
	gameData.reactor.bPresent = CFReadInt (fp);
	gameData.reactor.nDeadObj = CFReadInt (fp);
	fpos = CFTell (fp);
	// Restore the AI state
	AIRestoreUniState (fp, sgVersion);
	// Restore the automap visited info
	fpos = CFTell (fp);
	CFRead (bAutomapVisited, sizeof (ubyte), (sgVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2, fp);
	fpos = CFTell (fp);
	//	Restore hacked up weapon system stuff.
	gameData.app.fusion.xNextSoundTime = gameData.time.xGame;
	gameData.app.fusion.xAutoFireTime = 0;
	xNextLaserFireTime = gameData.time.xGame;
	xNextMissileFireTime = gameData.time.xGame;
	Last_laser_firedTime = gameData.time.xGame;
	}
gameData.app.nStateGameId = 0;
gameData.app.nStateGameId = (uint) CFReadInt (fp);
gameStates.app.cheats.bLaserRapidFire = CFReadInt (fp);
gameStates.app.bLunacy = CFReadInt (fp);		//	Yes, reading this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
gameStates.app.bLunacy = CFReadInt (fp);
if (gameStates.app.bLunacy)
	DoLunacyOn ();

CFRead (gameData.marker.objects, sizeof (gameData.marker.objects), 1, fp);
CFRead (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1, fp);
CFRead (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1, fp);

if (bSecretRestore != 1)
	xAfterburnerCharge = CFReadFix (fp);
else {
	CFReadFix (fp);
	}
//read last was super information
CFRead (&bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1, fp);
CFRead (&bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1, fp);
gameData.render.xFlashEffect = CFReadFix (fp);
gameData.render.xTimeFlashLastPlayed = CFReadFix (fp);
gameStates.ogl.palAdd.red = CFReadShort (fp);
gameStates.ogl.palAdd.green = CFReadShort (fp);
gameStates.ogl.palAdd.blue = CFReadShort (fp);
CFRead (gameData.render.lights.subtracted, 
		  sizeof (gameData.render.lights.subtracted [0]), 
		  (sgVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2, fp);
ApplyAllChangedLight ();
if (bSecretRestore) 
	gameStates.app.bFirstSecretVisit = 0;
else
	gameStates.app.bFirstSecretVisit = CFReadInt (fp);

if (bSecretRestore != 1)
	xOmegaCharge = CFReadFix (fp);
else
	CFReadFix (fp);
if (sgVersion > 27)
	gameData.missions.nEnteredFromLevel = CFReadShort (fp);
return 1;
}

//------------------------------------------------------------------------------

int StateRestoreBinGameData (CFILE *fp, int sgVersion, int bMulti, int bSecretRestore, fix xOldGameTime)
{
	tPlayer	restore_players [MAX_PLAYERS];
	int		nPlayers, nServerPlayer = -1;
	int		nOtherObjNum = -1, nServerObjNum = -1, nLocalObjNum = -1, nSavedLocalPlayer = -1;
	int		bBetweenLevels;
	int		nCurrentLevel, nNextLevel;
	wall		*wallP;
	char		szOrgCallSign [CALLSIGN_LEN+16];
	int		i, j;
	short		nTexture;

CFRead (&bBetweenLevels, sizeof (int), 1, fp);
Assert (bBetweenLevels == 0);	//between levels save ripped out
// Read the mission info...
if (!StateLoadMission (fp))
	return 0;
//Read level info
CFRead (&nCurrentLevel, sizeof (int), 1, fp);
CFRead (&nNextLevel, sizeof (int), 1, fp);
//Restore gameData.time.xGame
CFRead (&gameData.time.xGame, sizeof (fix), 1, fp);
// Start new game....
StateRestoreMultiGame (szOrgCallSign, bMulti, bSecretRestore);
if (gameData.app.nGameMode & GM_MULTI) {
		char szServerCallSign [CALLSIGN_LEN + 1];

	strcpy (szServerCallSign, netPlayers.players [0].callsign);
	CFRead (&gameData.app.nStateGameId, sizeof (int), 1, fp);
	CFRead (&netGame, sizeof (tNetgameInfo), 1, fp);
	CFRead (&netPlayers, sizeof (tAllNetPlayersInfo), 1, fp);
	CFRead (&nPlayers, sizeof (gameData.multi.nPlayers), 1, fp);
	CFRead (&gameData.multi.nLocalPlayer, sizeof (gameData.multi.nLocalPlayer), 1, fp);
	nSavedLocalPlayer = gameData.multi.nLocalPlayer;
	for (i = 0; i < nPlayers; i++)
		CFRead (restore_players + i, sizeof (tPlayer), 1, fp);
	nServerPlayer = StateSetServerPlayer (restore_players, nPlayers, szServerCallSign, &nOtherObjNum, &nServerObjNum);
	StateGetConnectedPlayers (restore_players, nPlayers);
	}

//Read tPlayer info
if (!StartNewLevelSub (nCurrentLevel, 1, bSecretRestore)) {
	CFClose (fp);
	return 0;
	}
nLocalObjNum = gameData.multi.players [gameData.multi.nLocalPlayer].nObject;
if (bSecretRestore != 1)	//either no secret restore, or tPlayer died in scret level
	CFRead (gameData.multi.players + gameData.multi.nLocalPlayer, sizeof (tPlayer), 1, fp);
else {
	tPlayer	retPlayer;
	CFRead (&retPlayer, sizeof (tPlayer), 1, fp);
	StateAwardReturningPlayer (&retPlayer, xOldGameTime);
	}
gameData.multi.players [gameData.multi.nLocalPlayer].nObject = nLocalObjNum;
strcpy (gameData.multi.players [gameData.multi.nLocalPlayer].callsign, szOrgCallSign);
// Set the right level
if (bBetweenLevels)
	gameData.multi.players [gameData.multi.nLocalPlayer].level = nNextLevel;
// Restore the weapon states
CFRead (&gameData.weapons.nPrimary, sizeof (sbyte), 1, fp);
CFRead (&gameData.weapons.nSecondary, sizeof (sbyte), 1, fp);
SelectWeapon (gameData.weapons.nPrimary, 0, 0, 0);
SelectWeapon (gameData.weapons.nSecondary, 1, 0, 0);
// Restore the difficulty level
CFRead (&gameStates.app.nDifficultyLevel, sizeof (int), 1, fp);
gameStates.app.nDifficultyLevel = gameOpts->gameplay.nPlayerDifficultyLevel;
// Restore the cheats enabled flag
CFRead (&gameStates.app.cheats.bEnabled, sizeof (int), 1, fp);
if (!bBetweenLevels)	{
	gameStates.render.bDoAppearanceEffect = 0;			// Don't do this for middle o' game stuff.
	//Clear out all the gameData.objs.objects from the lvl file
	for (i = 0; i <= gameData.segs.nLastSegment; i++)
		gameData.segs.segments [i].objects = -1;
	ResetObjects (1);
	//Read objects, and pop 'em into their respective segments.
	CFRead (&i, sizeof (int), 1, fp);
	gameData.objs.nLastObject = i - 1;
	CFRead (gameData.objs.objects, sizeof (tObject), i, fp);
	StateFixNetworkObjects (nServerPlayer, nOtherObjNum, nServerObjNum);
	StateFixObjects ();
	SpecialResetObjects ();
	InitCamBots (1);
	gameData.objs.nNextSignature++;
	//	1 = Didn't die on secret level.
	//	2 = Died on secret level.
	if (bSecretRestore && (gameData.missions.nCurrentLevel >= 0)) {
		SetPosFromReturnSegment ();
		if (bSecretRestore == 2)
			InitPlayerStatsNewShip ();
		}
	//Restore wall info
	if (CFReadBoundedInt (MAX_WALLS, &gameData.walls.nWalls, fp))
		return 0;
	CFRead (gameData.walls.walls, sizeof (wall), gameData.walls.nWalls, fp);
	//now that we have the walls, check if any sounds are linked to
	//walls that are now open
	for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; i++, wallP++)
		if (wallP->nType == WALL_OPEN)
			DigiKillSoundLinkedToSegment ((short) wallP->nSegment, (short) wallP->nSide, -1);	//-1 means kill any sound
	//Restore exploding wall info
	if (sgVersion >= 10) {
		CFRead (&i, sizeof (int), 1, fp);
		CFRead (gameData.walls.explWalls, sizeof (*gameData.walls.explWalls), i, fp);
		}
	//Restore door info
	if (CFReadBoundedInt (MAX_DOORS, &gameData.walls.nOpenDoors, fp))
		return 0;
	CFRead (gameData.walls.activeDoors, sizeof (tActiveDoor), gameData.walls.nOpenDoors, fp);
	if (sgVersion >= 14) {		//Restore cloaking wall info
		if (CFReadBoundedInt (MAX_WALLS, &gameData.walls.nCloaking, fp))
			return 0;
		CFRead (gameData.walls.cloaking, sizeof (tCloakingWall), gameData.walls.nCloaking, fp);
		}
	//Restore tTrigger info
	if (CFReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nTriggers, fp))
		return 0;
	CFRead (gameData.trigs.triggers, sizeof (tTrigger), gameData.trigs.nTriggers, fp);
	if (sgVersion >= 26) {
		//Restore tObject tTrigger info

		CFRead (&gameData.trigs.nObjTriggers, sizeof (gameData.trigs.nObjTriggers), 1, fp);
		if (gameData.trigs.nObjTriggers > 0) {
			CFRead (gameData.trigs.objTriggers, sizeof (tTrigger), gameData.trigs.nObjTriggers, fp);
			CFRead (gameData.trigs.objTriggerRefs, sizeof (tObjTriggerRef), gameData.trigs.nObjTriggers, fp);
			CFRead (gameData.trigs.firstObjTrigger, sizeof (short), MAX_OBJECTS_D2X, fp);
			}
		else
			CFSeek (fp, MAX_OBJECTS_D2X * sizeof (short), SEEK_CUR);
		}
	//Restore tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++)	{
		for (j = 0; j < 6; j++)	{
			gameData.segs.segments [i].sides [j].nWall = CFReadShort (fp);
			gameData.segs.segments [i].sides [j].nBaseTex = CFReadShort (fp);
			nTexture = CFReadShort (fp);
			gameData.segs.segments [i].sides [j].nOvlTex = nTexture & 0x3fff;
			gameData.segs.segments [i].sides [j].nOvlOrient = (nTexture >> 14) & 3;
			}
		}
//Restore the fuelcen info
	gameData.reactor.bDestroyed = CFReadInt (fp);
	gameData.reactor.countdown.nTimer = CFReadFix (fp);
	if (CFReadBoundedInt (MAX_ROBOT_CENTERS, &gameData.matCens.nRobotCenters, fp))
		return 0;
	CFRead (gameData.matCens.robotCenters, sizeof (tMatCenInfo), gameData.matCens.nRobotCenters, fp);
	CFRead (&gameData.reactor.triggers, sizeof (tReactorTriggers), 1, fp);
	if (CFReadBoundedInt (MAX_FUEL_CENTERS, &gameData.matCens.nFuelCenters, fp))
		return 0;
	CFRead (gameData.matCens.fuelCenters, sizeof (tFuelCenInfo), gameData.matCens.nFuelCenters, fp);

	// Restore the control cen info
	gameData.reactor.bHit = CFReadInt (fp);
	gameData.reactor.bSeenPlayer = CFReadInt (fp);
	gameData.reactor.nNextFireTime = CFReadInt (fp);
	gameData.reactor.bPresent = CFReadInt (fp);
	gameData.reactor.nDeadObj = CFReadInt (fp);
	// Restore the AI state
	AIRestoreBinState (fp, sgVersion);
	// Restore the automap visited info
	CFRead (bAutomapVisited, sizeof (ubyte), (sgVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2, fp);

	//	Restore hacked up weapon system stuff.
	gameData.app.fusion.xNextSoundTime = gameData.time.xGame;
	gameData.app.fusion.xAutoFireTime = 0;
	xNextLaserFireTime = gameData.time.xGame;
	xNextMissileFireTime = gameData.time.xGame;
	Last_laser_firedTime = gameData.time.xGame;

}
gameData.app.nStateGameId = 0;

if (sgVersion >= 7)	{
	CFRead (&gameData.app.nStateGameId, sizeof (uint), 1, fp);
	CFRead (&gameStates.app.cheats.bLaserRapidFire, sizeof (int), 1, fp);
	CFRead (&gameStates.app.bLunacy, sizeof (int), 1, fp);		//	Yes, writing this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
	CFRead (&gameStates.app.bLunacy, sizeof (int), 1, fp);
	if (gameStates.app.bLunacy)
		DoLunacyOn ();
}

if (sgVersion >= 17) {
	CFRead (gameData.marker.objects, sizeof (gameData.marker.objects), 1, fp);
	CFRead (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1, fp);
	CFRead (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1, fp);
}
else {
	int num,dummy;

	// skip dummy info
	CFRead (&num, sizeof (int), 1, fp);       //was NumOfMarkers
	CFRead (&dummy, sizeof (int), 1, fp);     //was CurMarker
	CFSeek (fp, num * (sizeof (vmsVector) + 40), SEEK_CUR);
	for (num = 0; num < NUM_MARKERS; num++)
		gameData.marker.objects [num] = -1;
}

if (sgVersion >= 11) {
	if (bSecretRestore != 1)
		CFRead (&xAfterburnerCharge, sizeof (fix), 1, fp);
	else {
		fix	dummy_fix;
		CFRead (&dummy_fix, sizeof (fix), 1, fp);
	}
}
if (sgVersion >= 12) {
	//read last was super information
	CFRead (&bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1, fp);
	CFRead (&bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1, fp);
	CFRead (&gameData.render.xFlashEffect, sizeof (int), 1, fp);
	CFRead (&gameData.render.xTimeFlashLastPlayed, sizeof (int), 1, fp);
	CFRead (&gameStates.ogl.palAdd.red, sizeof (int), 1, fp);
	CFRead (&gameStates.ogl.palAdd.green, sizeof (int), 1, fp);
	CFRead (&gameStates.ogl.palAdd.blue, sizeof (int), 1, fp);
} else {
	gameData.render.xFlashEffect = 0;
	gameData.render.xTimeFlashLastPlayed = 0;
	gameStates.ogl.palAdd.red = 0;
	gameStates.ogl.palAdd.green = 0;
	gameStates.ogl.palAdd.blue = 0;
	}

//	Load gameData.render.lights.subtracted
if (sgVersion >= 16) {
	CFRead (gameData.render.lights.subtracted, sizeof (gameData.render.lights.subtracted [0]), (sgVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2, fp);
	ApplyAllChangedLight ();
	//ComputeAllStaticLight ();	//	set static_light field in tSegment struct.  See note at that function.
	}
else
	memset (gameData.render.lights.subtracted, 0, sizeof (gameData.render.lights.subtracted));

if (bSecretRestore) 
	gameStates.app.bFirstSecretVisit = 0;
else if (sgVersion >= 20)
	CFRead (&gameStates.app.bFirstSecretVisit, sizeof (gameStates.app.bFirstSecretVisit), 1, fp);
else
	gameStates.app.bFirstSecretVisit = 1;

if (sgVersion >= 22) {
	if (bSecretRestore != 1)
		CFRead (&xOmegaCharge, sizeof (fix), 1, fp);
	else {
		fix	dummy_fix;
		CFRead (&dummy_fix, sizeof (fix), 1, fp);
		}
	}
return 1;
}

//------------------------------------------------------------------------------

int StateRestoreAllSub (char *filename, int bMulti, int bSecretRestore)
{
	CFILE			*fp;
	char			szDesc [DESC_LENGTH + 1];
	char			id [5];
	int			sgVersion, i;
	fix			xOldGameTime = gameData.time.xGame;

if (! (fp = CFOpen (filename, gameFolders.szSaveDir, "rb", 0)))
	return 0;
StopTime ();
//Read id
CFRead (id, sizeof (char)*4, 1, fp);
if (memcmp (id, dgss_id, 4)) {
	CFClose (fp);
	StartTime ();
	return 0;
	}
//Read sgVersion
CFRead (&sgVersion, sizeof (int), 1, fp);
if (sgVersion < STATE_COMPATIBLE_VERSION)	{
	CFClose (fp);
	StartTime ();
	return 0;
	}
// Read description
CFRead (szDesc, sizeof (char) * DESC_LENGTH, 1, fp);
// Skip the current screen shot...
CFSeek (fp, (sgVersion < 26) ? THUMBNAIL_W*THUMBNAIL_H : THUMBNAIL_LW * THUMBNAIL_LH, SEEK_CUR);
// And now...skip the goddamn palette stuff that somebody forgot to add
CFSeek (fp, 768, SEEK_CUR);
if (sgVersion < 27)
	StateRestoreBinGameData (fp, sgVersion, bMulti, bSecretRestore, xOldGameTime);
else
	StateRestoreUniGameData (fp, sgVersion, bMulti, bSecretRestore, xOldGameTime);
CFClose (fp);
FixObjectSegs ();
SetLastSuperWeaponStates ();
 // Get rid of ships that aren't connected in the restored game
if (gameData.app.nGameMode & GM_MULTI) {
	for (i = 0; i < gameData.multi.nPlayers; i++) {
	  if (gameData.multi.players [i].connected != 1) {
			NetworkDisconnectPlayer (i);
  			CreatePlayerAppearanceEffect (gameData.objs.objects + gameData.multi.players [i].nObject);
	      }
		}
	}
gameData.objs.viewer = 
gameData.objs.console = 
	gameData.objs.objects + gameData.multi.players [gameData.multi.nLocalPlayer].nObject;
StartTime ();
return 1;
}

//------------------------------------------------------------------------------
//	When loading a saved game, segp->static_light is bogus.
//	This is because ApplyAllChangedLight, which is supposed to properly update this value,
//	cannot do so because it needs the original light cast from a light which is no longer there.
//	That is, a light has been blown out, so the texture remaining casts 0 light, but the static light
//	which is present in the static_light field contains the light cast from that light.
void ComputeAllStaticLight (void)
{
	int		h, i, j, k;
	tSegment	*segP;
	tSide		*sideP;
	fix		total_light;

	for (i=0, segP = gameData.segs.segments; i<=gameData.segs.nLastSegment; i++, segP++) {
		total_light = 0;
		for (h = j = 0, sideP = segP->sides; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
			if ((segP->children [j] < 0) || IS_WALL (sideP->nWall)) {
				h++;
				for (k = 0; k < 4; k++)
					total_light += sideP->uvls [k].l;
			}
		}
		gameData.segs.segment2s [i].static_light = h ? total_light / (h * 4) : 0;
	}
}

//------------------------------------------------------------------------------

int StateGetGameId (char *filename)
{
	int sgVersion;
	CFILE *fp;
	int bBetweenLevels;
	char mission [16];
	char szDesc [DESC_LENGTH+1];
	char id [5];
	int dumbint;

	fp = CFOpen (filename, gameFolders.szSaveDir, "rb", 0);
	if (!fp) return 0;

//Read id
	CFRead (id, sizeof (char)*4, 1, fp);
	if (memcmp (id, dgss_id, 4)) {
		CFClose (fp);
		return 0;
	}

//Read sgVersion
	CFRead (&sgVersion, sizeof (int), 1, fp);
	if (sgVersion < STATE_COMPATIBLE_VERSION)	{
		CFClose (fp);
		return 0;
	}

// Read description
	CFRead (szDesc, sizeof (char)*DESC_LENGTH, 1, fp);

// Skip the current screen shot...
	CFSeek (fp, (sgVersion < 26) ? THUMBNAIL_W*THUMBNAIL_H : THUMBNAIL_LW*THUMBNAIL_LH, SEEK_CUR);

// And now...skip the palette stuff that somebody forgot to add
	CFSeek (fp, 768, SEEK_CUR);

// Read the Between levels flag...
	CFRead (&bBetweenLevels, sizeof (int), 1, fp);

	Assert (bBetweenLevels == 0);	//between levels save ripped out

// Read the mission info...
	CFRead (mission, sizeof (char), 9, fp);
//Read level info
	CFRead (&dumbint, sizeof (int), 1, fp);
	CFRead (&dumbint, sizeof (int), 1, fp);

//Restore gameData.time.xGame
	CFRead (&dumbint, sizeof (fix), 1, fp);

	CFRead (&gameData.app.nStateGameId, sizeof (int), 1, fp);

	return (gameData.app.nStateGameId);
 }

//------------------------------------------------------------------------------
//eof
