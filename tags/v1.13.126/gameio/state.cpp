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
#ifndef _WIN32
#include <unistd.h>
#endif
#ifndef _WIN32_WCE
#include <errno.h>
#endif

#ifdef _WIN32
#  include <windows.h>
#endif
#ifdef __macosx__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#ifndef _WIN32
#	include <arpa/inet.h>
#	include <netinet/in.h> /* for htons & co. */
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
#include "reactor.h"
#include "morph.h"
#include "weapon.h"
#include "render.h"
#include "loadgame.h"
#include "gauges.h"
#include "newdemo.h"
#include "automap.h"
#include "piggy.h"
#include "paging.h"
#include "briefings.h"
#include "text.h"
#include "mission.h"
#include "pcx.h"
#include "u_mem.h"
#include "network.h"
#include "objeffects.h"
#include "args.h"
#include "ai.h"
#include "fireball.h"
#include "controls.h"
#include "laser.h"
#include "omega.h"
#include "multibot.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "state.h"
#include "strutil.h"
#include "light.h"
#include "dynlight.h"
#include "ipx.h"
#include "gr.h"

#undef DBG
#if DBG
#	define DBG(_expr)	_expr
#else
#	define DBG(_expr)
#endif

#define STATE_VERSION				39
#define STATE_COMPATIBLE_VERSION 20
// 0 - Put DGSS (Descent Game State Save) nId at tof.
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
// 14- Save cloaking tWall stuff
// 15- Save additional ai info
// 16- Save gameData.render.lights.subtracted
// 17- New marker save
// 18- Took out saving of old cheat status
// 19- Saved cheats_enabled flag
// 20- gameStates.app.bFirstSecretVisit
// 22- gameData.omega.xCharge

void SetFunctionMode (int);
void InitPlayerStatsNewShip (void);
void ShowLevelIntro (int level_num);
void DoCloakInvulSecretStuff (fix xOldGameTime);
void CopyDefaultsToRobot (tObject *objP);
void MultiInitiateSaveGame ();
void MultiInitiateRestoreGame ();
void ApplyAllChangedLight (void);
void DoLunacyOn (void);
void DoLunacyOff (void);

int m_nLastSlot= 0;

char dgss_id [4] = {'D', 'G', 'S', 'S'};

void ComputeAllStaticLight (void);

void GameRenderFrame (void);

#define	DESC_OFFSET	8

#ifdef _WIN32_WCE
# define errno -1
# define strerror (x) "Unknown Error"
#endif

#define SECRETB_FILENAME	"secret.sgb"
#define SECRETC_FILENAME	"secret.sgc"

CSaveGameHandler saveGameHandler;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

typedef struct tSaveGameInfo {
	char			szLabel [DESC_LENGTH + 16];
	char			szTime [DESC_LENGTH + 16];
	grsBitmap	*image;
} tSaveGameInfo;

class CSaveGameInfo {
	private:
		tSaveGameInfo	m_info;
	public:
		CSaveGameInfo () { Init (); }
		~CSaveGameInfo () {};
		void Init (void);
		inline char* Label (void) { return m_info.szLabel; }
		inline char* Time (void) { return m_info.szTime; }
		inline grsBitmap* Image (void) { return m_info.image; }
		bool Load (char *filename, int nSlot);
		void Destroy (void);
};


//------------------------------------------------------------------------------

void CSaveGameInfo::Init (void)
{
memset (&m_info, 0, sizeof (m_info));
strcpy (m_info.szLabel, TXT_EMPTY);
}

//------------------------------------------------------------------------------

void CSaveGameInfo::Destroy (void)
{
if (m_info.image) {
	GrFreeBitmap (m_info.image);
	m_info.image = NULL;
	}
}

//------------------------------------------------------------------------------

bool CSaveGameInfo::Load (char *filename, int nSlot)
{
	CFile	cf;
	int	nId, nVersion;

Init ();
if (!cf.Open (filename, gameFolders.szSaveDir, "rb", 0))
	return false;
	//Read nId
cf.Read (&nId, sizeof (char) * 4, 1);
if (memcmp (&nId, dgss_id, 4)) {
	cf.Close ();
	return false;
	}
cf.Read (&nVersion, sizeof (int), 1);
if (nVersion < STATE_COMPATIBLE_VERSION) {
	cf.Close ();
	return false;
	}
if (nSlot < 0)
	cf.Read (m_info.szLabel, DESC_LENGTH, 1);
else {
	if (nSlot < NUM_SAVES)
		sprintf (m_info.szLabel, "%d. ", nSlot + 1);
	else
		strcpy (m_info.szLabel, "   ");
	cf.Read (m_info.szLabel + 3, DESC_LENGTH, 1);
	if (nVersion < 26) {
		m_info.image = GrCreateBitmap (THUMBNAIL_W, THUMBNAIL_H, 1);
		cf.Read (m_info.image->bmTexBuf, THUMBNAIL_W * THUMBNAIL_H, 1);
		}
	else {
		m_info.image = GrCreateBitmap (THUMBNAIL_LW, THUMBNAIL_LH, 1);
		cf.Read (m_info.image->bmTexBuf, THUMBNAIL_LW * THUMBNAIL_LH, 1);
		}
	if (nVersion >= 9) {
		ubyte palette [3 * 356];
		cf.Read (palette, 3, 256);
		GrRemapBitmapGood (m_info.image, palette, -1, -1);
		}
	struct tm	*t;
	int			h;
#ifdef _WIN32
	char	fn [FILENAME_LEN];

	struct _stat statBuf;
	sprintf (fn, "%s/%s", gameFolders.szSaveDir, filename);
	h = _stat (fn, &statBuf);
#else
	struct stat statBuf;
	h = stat (filename, &statBuf);
#endif
	if (!h && (t = localtime (&statBuf.st_mtime)))
		sprintf (m_info.szTime, " [%d-%d-%d %d:%02d:%02d]",
			t->tm_mon + 1, t->tm_mday, t->tm_year + 1900,
			t->tm_hour, t->tm_min, t->tm_sec);
	}
cf.Close ();
return true;
}

CSaveGameInfo saveGameInfo [NUM_SAVES + 1];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define NM_IMG_SPACE	6

static int bShowTime = 1;

int SaveStateMenuCallback (int nitems, tMenuItem *items, int *lastKey, int nCurItem)
{
	int			x, y, i = nCurItem - NM_IMG_SPACE;
	char			c = KeyToASCII (*lastKey);
	grsBitmap	*image = saveGameInfo [i].Image ();

if (nCurItem < 2)
	return nCurItem;
if ((c >= '1') && (c <= '9')) {
	for (i = 0; i < NUM_SAVES; i++)
		if (items [i + NM_IMG_SPACE].text [0] == c) {
			*lastKey = -2;
			return -(i + NM_IMG_SPACE) - 1;
			}
	}
if (!items [NM_IMG_SPACE - 1].text || strcmp (items [NM_IMG_SPACE - 1].text, saveGameInfo [i].Time ())) {
	items [NM_IMG_SPACE - 1].text = saveGameInfo [i].Time ();
	items [NM_IMG_SPACE - 1].rebuild = 1;
	}
if (!image)
	return nCurItem;
if (gameStates.menus.bHires) {
	x = (grdCurCanv->cvBitmap.bmProps.w - image->bmProps.w) / 2;
	y = items [0].y - 16;
	if (gameStates.app.bGameRunning)
		GrPaletteStepLoad (NULL);
	GrBitmap (x, y, image);
	if (gameOpts->menus.nStyle) {
		GrSetColorRGBi (RGBA_PAL (0, 0, 32));
		GrUBox (x - 1, y - 1, x + image->bmProps.w + 1, y + image->bmProps.h + 1);
		}
	}
else {
	GrBitmap ((grdCurCanv->cvBitmap.bmProps.w-THUMBNAIL_W) / 2, items [0].y - 5, saveGameInfo [nCurItem - 1].Image ());
	}
return nCurItem;
}

//------------------------------------------------------------------------------

int LoadStateMenuCallback (int nitems, tMenuItem *items, int *lastKey, int nCurItem)
{
	int	i = nCurItem - NM_IMG_SPACE;
	char	c = KeyToASCII (*lastKey);

if (nCurItem < 2)
	return nCurItem;
if ((c >= '1') && (c <= '9')) {
	for (i = 0; i < NUM_SAVES; i++)
		if (items [i].text [0] == c) {
			*lastKey = -2;
			return -i - 1;
			}
	}
return nCurItem;
}

//------------------------------------------------------------------------------

char *strrpad (char * str, int len)
{
str += len;
*str = '\0';
for (; len; len--)
	if (*(--str))
		*str = ' ';
	else {
		*str = ' ';
		break;
		}
return str;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CSaveGameHandler::Init (void)
{
m_nDefaultSlot = 0;
m_nLastSlot = 0;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::GetSaveFile (int bMulti)
{
	int			i, menuRes, choice;
	tMenuItem	m [NUM_SAVES + 2];
	char			filename [NUM_SAVES + 1][30];

memset (m, 0, sizeof (m));
for (i = 0; i < NUM_SAVES; i++)	{
	sprintf (filename [i], bMulti ? "%s.mg%x" : "%s.sg%x", LOCALPLAYER.callsign, i);
	saveGameInfo [i].Load (filename [i], -1);
	ADD_INPUT_MENU (i, saveGameInfo [i].Label (), DESC_LENGTH - 1, -1, NULL);
	}

m_nLastSlot = -1;
choice = m_nDefaultSlot;
menuRes = ExecMenu1 (NULL, TXT_SAVE_GAME_MENU, NUM_SAVES, m, NULL, &choice);

if (menuRes >= 0) {
	strcpy (m_filename, filename [choice]);
	strcpy (m_description, saveGameInfo [choice].Label());
	}
for (i = 0; i < NUM_SAVES; i++)
	saveGameInfo [i].Destroy ();
m_nDefaultSlot = choice;
return (menuRes < 0) ? 0 : choice + 1;
}

//------------------------------------------------------------------------------

int bRestoringMenu = 0;

int CSaveGameHandler::GetLoadFile (int bMulti)
{
	int			i, choice = -1, nSaves;
	tMenuItem	m [NUM_SAVES + NM_IMG_SPACE + 1];
	char			filename [NUM_SAVES + 1][30];

nSaves = 0;
memset (m, 0, sizeof (m));
for (i = 0; i < NM_IMG_SPACE; i++) {
	m [i].nType = NM_TYPE_TEXT; 
	m [i].text = (char *) "";
	m [i].noscroll = 1;
	}
if (gameStates.app.bGameRunning) {
	GrPaletteStepLoad (NULL);
	}
for (i = 0; i < NUM_SAVES + 1; i++) {
	sprintf (filename [i], bMulti ? "%s.mg%x" : "%s.sg%x", LOCALPLAYER.callsign, i);
	if (saveGameInfo [i].Load (filename [i], i)) {
		ADD_MENU (i + NM_IMG_SPACE, saveGameInfo [i].Label (), (i < NUM_SAVES) ? -1 : 0, NULL);
		nSaves++;
		}
	else {
		m [i + NM_IMG_SPACE].nType = NM_TYPE_MENU; 
		m [i + NM_IMG_SPACE].text = saveGameInfo [i].Label ();
		}
	}
if (gameStates.app.bGameRunning) 
	GrPaletteStepLoad (NULL);
if (nSaves < 1) {
	ExecMessageBox (NULL, NULL, 1, "Ok", TXT_NO_SAVEGAMES);
	return 0;
	}
if (gameStates.video.nDisplayMode == 1)	//restore menu won't fit on 640x400
	gameStates.render.vr.nScreenFlags ^= VRF_COMPATIBLE_MENUS;
m_nLastSlot = -1;
bRestoringMenu = 1;
choice = m_nDefaultSlot + NM_IMG_SPACE;
i = ExecMenu3 (NULL, TXT_LOAD_GAME_MENU, i + NM_IMG_SPACE, m, SaveStateMenuCallback, 
				   &choice, NULL, 190, -1);
bRestoringMenu = 0;
if (i < 0)
	return 0;
choice -= NM_IMG_SPACE;
for (i = 0; i < NUM_SAVES + 1; i++)
	saveGameInfo [i].Destroy ();
if (choice >= 0) {
	strcpy (m_filename, filename [choice]);
	if (choice != NUM_SAVES + 1)		//no new default when restore from autosave
		m_nDefaultSlot = choice;
	return choice + 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------------
//	Save file we're going to save over in last slot and call " [autosave backup]"

void CSaveGameHandler::Backup (void)
{
if (!m_override) {
	CFile cf;
	
	if (cf.Open (m_filename, gameFolders.szSaveDir, "rb", 0)) {
		char	newname [FILENAME_LEN];

		sprintf (newname, "%s.sg%x", LOCALPLAYER.callsign, NUM_SAVES);
		cf.Seek (DESC_OFFSET, SEEK_SET);
		cf.Write ((char *) " [autosave backup]", sizeof (char) * DESC_LENGTH, 1);
		cf.Close ();
		cf.Delete (newname, gameFolders.szSaveDir);
		cf.Rename (m_filename, newname, gameFolders.szSaveDir);
		}
	}
}

//	-----------------------------------------------------------------------------------
//	If not in multiplayer, do special secret level stuff.
//	If secret.sgc exists, then copy it to Nsecret.sgc (where N = nSaveSlot).
//	If it doesn't exist, then delete Nsecret.sgc

void CSaveGameHandler::PushSecretSave (int nSaveSlot)
{
if ((nSaveSlot != -1) && !(m_bSecret || IsMultiGame)) {
	int	rval;
	char	tempname [32], fc;
	CFile m_cf;

	if (nSaveSlot >= 10)
		fc = (nSaveSlot-10) + 'a';
	else
		fc = '0' + nSaveSlot;
	sprintf (tempname, "%csecret.sgc", fc);
	if (m_cf.Exist (tempname, gameFolders.szSaveDir, 0)) {
		rval = m_cf.Delete (tempname, gameFolders.szSaveDir);
		Assert (rval == 0);	//	Oops, error deleting file in tempname.
		}
	if (m_cf.Exist (SECRETC_FILENAME, gameFolders.szSaveDir, 0)) {
		rval = m_cf.Copy (SECRETC_FILENAME, tempname);
		Assert (rval == 0);	//	Oops, error copying tempname to secret.sgc!
		}
	}
}

//	-----------------------------------------------------------------------------------
//	blind_save means don't prompt user for any info.

int CSaveGameHandler::Save (int bBetweenLevels, int bSecret, int bQuick, const char *pszFilenameOverride)
{
	int	rval, nSaveSlot = -1;

m_override = pszFilenameOverride;
m_bBetweenLevels = bBetweenLevels;
m_bQuick = bQuick;
m_cf.Init ();
if (IsMultiGame) {
	MultiInitiateSaveGame ();
	return 0;
	}
if (!(m_bSecret || gameOpts->gameplay.bSecretSave || gameStates.app.bD1Mission) && 
	  (gameData.missions.nCurrentLevel < 0)) {
	HUDInitMessage (TXT_SECRET_SAVE_ERROR);
	return 0;
	}
if (gameStates.gameplay.bFinalBossIsDead)		//don't allow save while final boss is dying
	return 0;
//	If this is a secret save and the control center has been destroyed, don't allow
//	return to the base level.
if (m_bSecret && gameData.reactor.bDestroyed) {
	CFile::Delete (SECRETB_FILENAME, gameFolders.szSaveDir);
	return 0;
	}
StopTime ();
gameData.app.bGamePaused = 1;
if (m_bQuick)
	sprintf (m_filename, "%s.quick", LOCALPLAYER.callsign);
else {
	if (m_bSecret == 1) {
		m_override = m_filename;
		sprintf (m_filename, SECRETB_FILENAME);
		} 
	else if (m_bSecret == 2) {
		m_override = m_filename;
		sprintf (m_filename, SECRETC_FILENAME);
		} 
	else {
		if (m_override) {
			strcpy (m_filename, m_override);
			sprintf (m_description, " [autosave backup]");
			}
		else if (!(nSaveSlot = GetSaveFile (0))) {
			gameData.app.bGamePaused = 0;
			StartTime (1);
			return 0;
			}
		}
	PushSecretSave (nSaveSlot);
	Backup ();
	}
if ((rval = SaveState (bSecret)))
	if (bQuick)
		HUDInitMessage (TXT_QUICKSAVE);
gameData.app.bGamePaused = 0;
StartTime (1);
return rval;
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveNetGame (void)
{
	int	i, j;

m_cf.WriteByte (netGame.nType);
m_cf.WriteInt (netGame.nSecurity);
m_cf.Write (netGame.szGameName, 1, NETGAME_NAME_LEN + 1);
m_cf.Write (netGame.szMissionTitle, 1, MISSION_NAME_LEN + 1);
m_cf.Write (netGame.szMissionName, 1, 9);
m_cf.WriteInt (netGame.nLevel);
m_cf.WriteByte ((sbyte) netGame.gameMode);
m_cf.WriteByte ((sbyte) netGame.bRefusePlayers);
m_cf.WriteByte ((sbyte) netGame.difficulty);
m_cf.WriteByte ((sbyte) netGame.gameStatus);
m_cf.WriteByte ((sbyte) netGame.nNumPlayers);
m_cf.WriteByte ((sbyte) netGame.nMaxPlayers);
m_cf.WriteByte ((sbyte) netGame.nConnected);
m_cf.WriteByte ((sbyte) netGame.gameFlags);
m_cf.WriteByte ((sbyte) netGame.protocolVersion);
m_cf.WriteByte ((sbyte) netGame.versionMajor);
m_cf.WriteByte ((sbyte) netGame.versionMinor);
m_cf.WriteByte ((sbyte) netGame.teamVector);
m_cf.WriteByte ((sbyte) netGame.DoMegas);
m_cf.WriteByte ((sbyte) netGame.DoSmarts);
m_cf.WriteByte ((sbyte) netGame.DoFusions);
m_cf.WriteByte ((sbyte) netGame.DoHelix);
m_cf.WriteByte ((sbyte) netGame.DoPhoenix);
m_cf.WriteByte ((sbyte) netGame.DoAfterburner);
m_cf.WriteByte ((sbyte) netGame.DoInvulnerability);
m_cf.WriteByte ((sbyte) netGame.DoCloak);
m_cf.WriteByte ((sbyte) netGame.DoGauss);
m_cf.WriteByte ((sbyte) netGame.DoVulcan);
m_cf.WriteByte ((sbyte) netGame.DoPlasma);
m_cf.WriteByte ((sbyte) netGame.DoOmega);
m_cf.WriteByte ((sbyte) netGame.DoSuperLaser);
m_cf.WriteByte ((sbyte) netGame.DoProximity);
m_cf.WriteByte ((sbyte) netGame.DoSpread);
m_cf.WriteByte ((sbyte) netGame.DoSmartMine);
m_cf.WriteByte ((sbyte) netGame.DoFlash);
m_cf.WriteByte ((sbyte) netGame.DoGuided);
m_cf.WriteByte ((sbyte) netGame.DoEarthShaker);
m_cf.WriteByte ((sbyte) netGame.DoMercury);
m_cf.WriteByte ((sbyte) netGame.bAllowMarkerView);
m_cf.WriteByte ((sbyte) netGame.bIndestructibleLights);
m_cf.WriteByte ((sbyte) netGame.DoAmmoRack);
m_cf.WriteByte ((sbyte) netGame.DoConverter);
m_cf.WriteByte ((sbyte) netGame.DoHeadlight);
m_cf.WriteByte ((sbyte) netGame.DoHoming);
m_cf.WriteByte ((sbyte) netGame.DoLaserUpgrade);
m_cf.WriteByte ((sbyte) netGame.DoQuadLasers);
m_cf.WriteByte ((sbyte) netGame.bShowAllNames);
m_cf.WriteByte ((sbyte) netGame.BrightPlayers);
m_cf.WriteByte ((sbyte) netGame.invul);
m_cf.WriteByte ((sbyte) netGame.FriendlyFireOff);
for (i = 0; i < 2; i++)
	m_cf.Write (netGame.szTeamName [i], 1, CALLSIGN_LEN + 1);		// 18 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteInt (netGame.locations [i]);
for (i = 0; i < MAX_PLAYERS; i++)
	for (j = 0; j < MAX_PLAYERS; j++)
		m_cf.WriteShort (netGame.kills [i][j]);			// 128 bytes
m_cf.WriteShort (netGame.nSegmentCheckSum);				// 2 bytes
for (i = 0; i < 2; i++)
	m_cf.WriteShort (netGame.teamKills [i]);				// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteShort (netGame.killed [i]);					// 16 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteShort (netGame.playerKills [i]);			// 16 bytes
m_cf.WriteInt (netGame.KillGoal);							// 4 bytes
m_cf.WriteFix (netGame.xPlayTimeAllowed);					// 4 bytes
m_cf.WriteFix (netGame.xLevelTime);							// 4 bytes
m_cf.WriteInt (netGame.controlInvulTime);				// 4 bytes
m_cf.WriteInt (netGame.monitorVector);					// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteInt (netGame.playerScore [i]);				// 32 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteByte ((sbyte) netGame.playerFlags [i]);	// 8 bytes
m_cf.WriteShort (PacketsPerSec ());					// 2 bytes
m_cf.WriteByte ((sbyte) netGame.bShortPackets);			// 1 bytes
// 279 bytes
// 355 bytes total
m_cf.Write (netGame.AuxData, NETGAME_AUX_SIZE, 1);  // Storage for protocol-specific data (e.g., multicast session and port)
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveNetPlayers (void)
{
	int	i;

m_cf.WriteByte ((sbyte) netPlayers.nType);
m_cf.WriteInt (netPlayers.nSecurity);
for (i = 0; i < MAX_PLAYERS + 4; i++) {
	m_cf.Write (netPlayers.players [i].callsign, 1, CALLSIGN_LEN + 1);
	m_cf.Write (netPlayers.players [i].network.ipx.server, 1, 4);
	m_cf.Write (netPlayers.players [i].network.ipx.node, 1, 6);
	m_cf.WriteByte ((sbyte) netPlayers.players [i].versionMajor);
	m_cf.WriteByte ((sbyte) netPlayers.players [i].versionMinor);
	m_cf.WriteByte ((sbyte) netPlayers.players [i].computerType);
	m_cf.WriteByte (netPlayers.players [i].connected);
	m_cf.WriteShort ((short) netPlayers.players [i].socket);
	m_cf.WriteByte ((sbyte) netPlayers.players [i].rank);
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SavePlayer (tPlayer *playerP)
{
	int	i;

m_cf.Write (playerP->callsign, 1, CALLSIGN_LEN + 1); // The callsign of this tPlayer, for net purposes.
m_cf.Write (playerP->netAddress, 1, 6);					// The network address of the player.
m_cf.WriteByte (playerP->connected);            // Is the tPlayer connected or not?
m_cf.WriteInt (playerP->nObject);                // What tObject number this tPlayer is. (made an int by mk because it's very often referenced)
m_cf.WriteInt (playerP->nPacketsGot);         // How many packets we got from them
m_cf.WriteInt (playerP->nPacketsSent);        // How many packets we sent to them
m_cf.WriteInt ((int) playerP->flags);           // Powerup flags, see below...
m_cf.WriteFix (playerP->energy);                // Amount of energy remaining.
m_cf.WriteFix (playerP->shields);               // shields remaining (protection)
m_cf.WriteByte (playerP->lives);                // Lives remaining, 0 = game over.
m_cf.WriteByte (playerP->level);                // Current level tPlayer is playing. (must be signed for secret levels)
m_cf.WriteByte ((sbyte) playerP->laserLevel);  // Current level of the laser.
m_cf.WriteByte (playerP->startingLevel);       // What level the tPlayer started on.
m_cf.WriteShort (playerP->nKillerObj);       // Who killed me.... (-1 if no one)
m_cf.WriteShort ((short) playerP->primaryWeaponFlags);   // bit set indicates the tPlayer has this weapon.
m_cf.WriteShort ((short) playerP->secondaryWeaponFlags); // bit set indicates the tPlayer has this weapon.
for (i = 0; i < MAX_PRIMARY_WEAPONS; i++)
	m_cf.WriteShort ((short) playerP->primaryAmmo [i]); // How much ammo of each nType.
for (i = 0; i < MAX_SECONDARY_WEAPONS; i++)
	m_cf.WriteShort ((short) playerP->secondaryAmmo [i]); // How much ammo of each nType.
#if 1 //for inventory system
m_cf.WriteByte ((sbyte) playerP->nInvuls);
m_cf.WriteByte ((sbyte) playerP->nCloaks);
#endif
m_cf.WriteInt (playerP->lastScore);             // Score at beginning of current level.
m_cf.WriteInt (playerP->score);                  // Current score.
m_cf.WriteFix (playerP->timeLevel);             // Level time played
m_cf.WriteFix (playerP->timeTotal);             // Game time played (high word = seconds)
if (playerP->cloakTime == 0x7fffffff)				// cloak cheat active
	m_cf.WriteFix (playerP->cloakTime);			// Time invulnerable
else
	m_cf.WriteFix (playerP->cloakTime - gameData.time.xGame);      // Time invulnerable
if (playerP->invulnerableTime == 0x7fffffff)		// invul cheat active
	m_cf.WriteFix (playerP->invulnerableTime);      // Time invulnerable
else
	m_cf.WriteFix (playerP->invulnerableTime - gameData.time.xGame);      // Time invulnerable
m_cf.WriteShort (playerP->nKillGoalCount);          // Num of players killed this level
m_cf.WriteShort (playerP->netKilledTotal);       // Number of times killed total
m_cf.WriteShort (playerP->netKillsTotal);        // Number of net kills total
m_cf.WriteShort (playerP->numKillsLevel);        // Number of kills this level
m_cf.WriteShort (playerP->numKillsTotal);        // Number of kills total
m_cf.WriteShort (playerP->numRobotsLevel);       // Number of initial robots this level
m_cf.WriteShort (playerP->numRobotsTotal);       // Number of robots total
m_cf.WriteShort ((short) playerP->hostages.nRescued); // Total number of hostages rescued.
m_cf.WriteShort ((short) playerP->hostages.nTotal);         // Total number of hostages.
m_cf.WriteByte ((sbyte) playerP->hostages.nOnBoard);      // Number of hostages on ship.
m_cf.WriteByte ((sbyte) playerP->hostages.nLevel);         // Number of hostages on this level.
m_cf.WriteFix (playerP->homingObjectDist);     // Distance of nearest homing tObject.
m_cf.WriteByte (playerP->hoursLevel);            // Hours played (since timeTotal can only go up to 9 hours)
m_cf.WriteByte (playerP->hoursTotal);            // Hours played (since timeTotal can only go up to 9 hours)
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveObject (tObject *objP)
{
m_cf.WriteInt (objP->info.nSignature);      
m_cf.WriteByte ((sbyte) objP->info.nType); 
m_cf.WriteByte ((sbyte) objP->info.nId);
m_cf.WriteShort (objP->info.nNextInSeg);
m_cf.WriteShort (objP->info.nPrevInSeg);
m_cf.WriteByte ((sbyte) objP->info.controlType);
m_cf.WriteByte ((sbyte) objP->info.movementType);
m_cf.WriteByte ((sbyte) objP->info.renderType);
m_cf.WriteByte ((sbyte) objP->info.nFlags);
m_cf.WriteShort (objP->info.nSegment);
m_cf.WriteShort (objP->info.nAttachedObj);
m_cf.WriteVector (OBJPOS (objP)->vPos);     
m_cf.WriteMatrix (OBJPOS (objP)->mOrient);  
m_cf.WriteFix (objP->info.xSize); 
m_cf.WriteFix (objP->info.xShields);
m_cf.WriteVector (objP->info.vLastPos);  
m_cf.WriteByte (objP->info.contains.nType); 
m_cf.WriteByte (objP->info.contains.nId);   
m_cf.WriteByte (objP->info.contains.nCount);
m_cf.WriteByte (objP->info.nCreator);
m_cf.WriteFix (objP->info.xLifeLeft);   
if (objP->info.movementType == MT_PHYSICS) {
	m_cf.WriteVector (objP->mType.physInfo.velocity);   
	m_cf.WriteVector (objP->mType.physInfo.thrust);     
	m_cf.WriteFix (objP->mType.physInfo.mass);       
	m_cf.WriteFix (objP->mType.physInfo.drag);       
	m_cf.WriteFix (objP->mType.physInfo.brakes);     
	m_cf.WriteVector (objP->mType.physInfo.rotVel);     
	m_cf.WriteVector (objP->mType.physInfo.rotThrust);  
	m_cf.WriteFixAng (objP->mType.physInfo.turnRoll);   
	m_cf.WriteShort ((short) objP->mType.physInfo.flags);      
	}
else if (objP->info.movementType == MT_SPINNING) {
	m_cf.WriteVector(objP->mType.spinRate);  
	}
switch (objP->info.controlType) {
	case CT_WEAPON:
		m_cf.WriteShort (objP->cType.laserInfo.parent.nType);
		m_cf.WriteShort (objP->cType.laserInfo.parent.nObject);
		m_cf.WriteInt (objP->cType.laserInfo.parent.nSignature);
		m_cf.WriteFix (objP->cType.laserInfo.xCreationTime);
		if (objP->cType.laserInfo.nLastHitObj)
			m_cf.WriteShort (gameData.objs.nHitObjects [OBJ_IDX (objP) * MAX_HIT_OBJECTS + objP->cType.laserInfo.nLastHitObj - 1]);
		else
			m_cf.WriteShort (-1);
		m_cf.WriteShort (objP->cType.laserInfo.nHomingTarget);
		m_cf.WriteFix (objP->cType.laserInfo.xScale);
		break;

	case CT_EXPLOSION:
		m_cf.WriteFix (objP->cType.explInfo.nSpawnTime);
		m_cf.WriteFix (objP->cType.explInfo.nDeleteTime);
		m_cf.WriteShort (objP->cType.explInfo.nDeleteObj);
		m_cf.WriteShort (objP->cType.explInfo.attached.nParent);
		m_cf.WriteShort (objP->cType.explInfo.attached.nPrev);
		m_cf.WriteShort (objP->cType.explInfo.attached.nNext);
		break;

	case CT_AI:
		m_cf.WriteByte ((sbyte) objP->cType.aiInfo.behavior);
		m_cf.Write (objP->cType.aiInfo.flags, 1, MAX_AI_FLAGS);
		m_cf.WriteShort (objP->cType.aiInfo.nHideSegment);
		m_cf.WriteShort (objP->cType.aiInfo.nHideIndex);
		m_cf.WriteShort (objP->cType.aiInfo.nPathLength);
		m_cf.WriteByte (objP->cType.aiInfo.nCurPathIndex);
		m_cf.WriteByte (objP->cType.aiInfo.bDyingSoundPlaying);
		m_cf.WriteShort (objP->cType.aiInfo.nDangerLaser);
		m_cf.WriteInt (objP->cType.aiInfo.nDangerLaserSig);
		m_cf.WriteFix (objP->cType.aiInfo.xDyingStartTime);
		break;

	case CT_LIGHT:
		m_cf.WriteFix (objP->cType.lightInfo.intensity);
		break;

	case CT_POWERUP:
		m_cf.WriteInt (objP->cType.powerupInfo.nCount);
		m_cf.WriteFix (objP->cType.powerupInfo.xCreationTime);
		m_cf.WriteInt (objP->cType.powerupInfo.nFlags);
		break;
	}
switch (objP->info.renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		m_cf.WriteInt (objP->rType.polyObjInfo.nModel);
		for (i = 0; i < MAX_SUBMODELS; i++)
			m_cf.WriteAngVec (objP->rType.polyObjInfo.animAngles [i]);
		m_cf.WriteInt (objP->rType.polyObjInfo.nSubObjFlags);
		m_cf.WriteInt (objP->rType.polyObjInfo.nTexOverride);
		m_cf.WriteInt (objP->rType.polyObjInfo.nAltTextures);
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		m_cf.WriteInt (objP->rType.vClipInfo.nClipIndex);
		m_cf.WriteFix (objP->rType.vClipInfo.xFrameTime);
		m_cf.WriteByte (objP->rType.vClipInfo.nCurFrame);
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveWall (tWall *wallP)
{
m_cf.WriteInt (wallP->nSegment);
m_cf.WriteInt (wallP->nSide);
m_cf.WriteFix (wallP->hps);    
m_cf.WriteInt (wallP->nLinkedWall);
m_cf.WriteByte ((sbyte) wallP->nType);       
m_cf.WriteByte ((sbyte) wallP->flags);      
m_cf.WriteByte ((sbyte) wallP->state);      
m_cf.WriteByte ((sbyte) wallP->nTrigger);    
m_cf.WriteByte (wallP->nClip);   
m_cf.WriteByte ((sbyte) wallP->keys);       
m_cf.WriteByte (wallP->controllingTrigger);
m_cf.WriteByte (wallP->cloakValue); 
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveExplWall (tExplWall *wallP)
{
m_cf.WriteInt (wallP->nSegment);
m_cf.WriteInt (wallP->nSide);
m_cf.WriteFix (wallP->time);    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveCloakingWall (tCloakingWall *wallP)
{
	int	i;

m_cf.WriteShort (wallP->nFrontWall);
m_cf.WriteShort (wallP->nBackWall); 
for (i = 0; i < 4; i++) {
	m_cf.WriteFix (wallP->front_ls [i]); 
	m_cf.WriteFix (wallP->back_ls [i]);
	}
m_cf.WriteFix (wallP->time);    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveActiveDoor (tActiveDoor *doorP)
{
	int	i;

m_cf.WriteInt (doorP->nPartCount);
for (i = 0; i < 2; i++) {
	m_cf.WriteShort (doorP->nFrontWall [i]);
	m_cf.WriteShort (doorP->nBackWall [i]);
	}
m_cf.WriteFix (doorP->time);    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveTrigger (tTrigger *triggerP)
{
	int	i;

m_cf.WriteByte ((sbyte) triggerP->nType); 
m_cf.WriteByte ((sbyte) triggerP->flags); 
m_cf.WriteByte (triggerP->nLinks);
m_cf.WriteFix (triggerP->value);
m_cf.WriteFix (triggerP->time);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++) {
	m_cf.WriteShort (triggerP->nSegment [i]);
	m_cf.WriteShort (triggerP->nSide [i]);
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveObjTriggerRef (tObjTriggerRef *refP)
{
m_cf.WriteShort (refP->prev);
m_cf.WriteShort (refP->next);
m_cf.WriteShort (refP->nObject);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveMatCen (tMatCenInfo *matcenP)
{
	int	i;

for (i = 0; i < 2; i++)
	m_cf.WriteInt (matcenP->objFlags [i]);
m_cf.WriteFix (matcenP->xHitPoints);
m_cf.WriteFix (matcenP->xInterval);
m_cf.WriteShort (matcenP->nSegment);
m_cf.WriteShort (matcenP->nFuelCen);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveFuelCen (tFuelCenInfo *fuelcenP)
{
m_cf.WriteInt (fuelcenP->nType);
m_cf.WriteInt (fuelcenP->nSegment);
m_cf.WriteByte (fuelcenP->bFlag);
m_cf.WriteByte (fuelcenP->bEnabled);
m_cf.WriteByte (fuelcenP->nLives);
m_cf.WriteFix (fuelcenP->xCapacity);
m_cf.WriteFix (fuelcenP->xMaxCapacity);
m_cf.WriteFix (fuelcenP->xTimer);
m_cf.WriteFix (fuelcenP->xDisableTime);
m_cf.WriteVector(fuelcenP->vCenter);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveReactorTrigger (tReactorTriggers *triggerP)
{
	int	i;

m_cf.WriteShort (triggerP->nLinks);
for (i = 0; i < MAX_CONTROLCEN_LINKS; i++) {
	m_cf.WriteShort (triggerP->nSegment [i]);
	m_cf.WriteShort (triggerP->nSide [i]);
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveReactorState (tReactorStates *stateP)
{
m_cf.WriteInt (stateP->nObject);
m_cf.WriteInt (stateP->bHit);
m_cf.WriteInt (stateP->bSeenPlayer);
m_cf.WriteInt (stateP->nNextFireTime);
m_cf.WriteInt (stateP->nDeadObj);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveSpawnPoint (int i)
{
#if DBG
i = m_cf.Tell;
#endif
m_cf.WriteVector (gameData.multiplayer.playerInit [i].position.vPos);     
m_cf.WriteMatrix (gameData.multiplayer.playerInit [i].position.mOrient);  
m_cf.WriteShort (gameData.multiplayer.playerInit [i].nSegment);
m_cf.WriteShort (gameData.multiplayer.playerInit [i].nSegType);
}

//------------------------------------------------------------------------------

DBG (static int fPos);

void CSaveGameHandler::SaveGameData (void)
{
	int		i, j;
	short		nObjsWithTrigger, nObject, nFirstTrigger;
	tObject	*objP;

m_cf.WriteInt (gameData.segs.nMaxSegments);
// Save the Between levels flag...
m_cf.WriteInt (m_bBetweenLevels);
// Save the mission info...
m_cf.Write (gameData.missions.list + gameData.missions.nCurrentMission, sizeof (char), 9);
//Save level info
m_cf.WriteInt (gameData.missions.nCurrentLevel);
m_cf.WriteInt (gameData.missions.nNextLevel);
//Save gameData.time.xGame
m_cf.WriteFix (gameData.time.xGame);
// If coop save, save all
if (IsCoopGame) {
	m_cf.WriteInt (gameData.app.nStateGameId);
	SaveNetGame ();
	DBG (fPos = m_cf.Tell ());
	SaveNetPlayers ();
	DBG (fPos = m_cf.Tell ());
	m_cf.WriteInt (gameData.multiplayer.nPlayers);
	m_cf.WriteInt (gameData.multiplayer.nLocalPlayer);
	for (i = 0; i < gameData.multiplayer.nPlayers; i++)
		SavePlayer (gameData.multiplayer.players + i);
	DBG (fPos = m_cf.Tell ());
	}
//Save tPlayer info
SavePlayer (gameData.multiplayer.players + gameData.multiplayer.nLocalPlayer);
// Save the current weapon info
m_cf.WriteByte (gameData.weapons.nPrimary);
m_cf.WriteByte (gameData.weapons.nSecondary);
// Save the difficulty level
m_cf.WriteInt (gameStates.app.nDifficultyLevel);
// Save cheats enabled
m_cf.WriteInt (gameStates.app.cheats.bEnabled);
for (i = 0; i < 2; i++) {
	m_cf.WriteInt (F2X (gameStates.gameplay.slowmo [i].fSpeed));
	m_cf.WriteInt (gameStates.gameplay.slowmo [i].nState);
	}
for (i = 0; i < MAX_PLAYERS; i++)
	m_cf.WriteInt (gameData.multiplayer.weaponStates [i].bTripleFusion);
if (!m_bBetweenLevels)	{
//Finish all morph OBJECTS
	FORALL_OBJS (objP, i) {
	if (objP->info.nType == OBJ_NONE) 
			continue;
		if (objP->info.nType == OBJ_CAMERA)
			objP->info.position.mOrient = cameraManager.Camera (objP)->Orient ();
		else if (objP->info.renderType == RT_MORPH) {
			tMorphInfo *md = MorphFindData (objP);
			if (md) {
				tObject *mdObjP = md->objP;
				mdObjP->info.controlType = md->saveControlType;
				mdObjP->info.movementType = md->saveMovementType;
				mdObjP->info.renderType = RT_POLYOBJ;
				mdObjP->mType.physInfo = md->savePhysInfo;
				md->objP = NULL;
				} 
			else {						//maybe loaded half-morphed from disk
				KillObject (objP);
				objP->info.renderType = RT_POLYOBJ;
				objP->info.controlType = CT_NONE;
				objP->info.movementType = MT_NONE;
				}
			}
		}
	DBG (fPos = m_cf.Tell ());
//Save tObject info
	i = gameData.objs.nLastObject [0] + 1;
	m_cf.WriteInt (i);
	for (j = 0; j < i; j++)
		SaveObject (OBJECTS + j);
	DBG (fPos = m_cf.Tell ());
//Save tWall info
	i = gameData.walls.nWalls;
	m_cf.WriteInt (i);
	for (j = 0; j < i; j++)
		SaveWall (gameData.walls.walls + j);
	DBG (fPos = m_cf.Tell ());
//Save exploding wall info
	i = MAX_EXPLODING_WALLS;
	m_cf.WriteInt (i);
	for (j = 0; j < i; j++)
		SaveExplWall (gameData.walls.explWalls + j);
	DBG (fPos = m_cf.Tell ());
//Save door info
	i = gameData.walls.nOpenDoors;
	m_cf.WriteInt (i);
	for (j = 0; j < i; j++)
		SaveActiveDoor (gameData.walls.activeDoors + j);
	DBG (fPos = m_cf.Tell ());
//Save cloaking tWall info
	i = gameData.walls.nCloaking;
	m_cf.WriteInt (i);
	for (j = 0; j < i; j++)
		SaveCloakingWall (gameData.walls.cloaking + j);
	DBG (fPos = m_cf.Tell ());
//Save tTrigger info
	m_cf.WriteInt (gameData.trigs.nTriggers);
	for (i = 0; i < gameData.trigs.nTriggers; i++)
		SaveTrigger (gameData.trigs.triggers + i);
	DBG (fPos = m_cf.Tell ());
	m_cf.WriteInt (gameData.trigs.nObjTriggers);
	if (!gameData.trigs.nObjTriggers)
		m_cf.WriteShort (0);
	else {
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			SaveTrigger (gameData.trigs.objTriggers + i);
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			SaveObjTriggerRef (gameData.trigs.objTriggerRefs + i);
		nObjsWithTrigger = 0;
		FORALL_OBJS (objP, nObject) {
			nObject = OBJ_IDX (objP);
			nFirstTrigger = gameData.trigs.firstObjTrigger [nObject];
			if ((nFirstTrigger >= 0) && (nFirstTrigger < gameData.trigs.nObjTriggers))
				nObjsWithTrigger++;
			}
		m_cf.WriteShort (nObjsWithTrigger);
		FORALL_OBJS (objP, nObject) {
			nObject = OBJ_IDX (objP);
			nFirstTrigger = gameData.trigs.firstObjTrigger [nObject];
			if ((nFirstTrigger >= 0) && (nFirstTrigger < gameData.trigs.nObjTriggers)) {
				m_cf.WriteShort (nObject);
				m_cf.WriteShort (nFirstTrigger);
				}
			}
		}
	DBG (fPos = m_cf.Tell ());
//Save tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++) {
		for (j = 0; j < 6; j++)	{
			ushort nWall = WallNumI ((short) i, (short) j);
			m_cf.WriteShort (nWall);
			m_cf.WriteShort (gameData.segs.segments [i].sides [j].nBaseTex);
			m_cf.WriteShort (gameData.segs.segments [i].sides [j].nOvlTex | (gameData.segs.segments [i].sides [j].nOvlOrient << 14));
			}
		}
	DBG (fPos = m_cf.Tell ());
// Save the fuelcen info
	m_cf.WriteInt (gameData.reactor.bDestroyed);
	m_cf.WriteFix (gameData.reactor.countdown.nTimer);
	DBG (fPos = m_cf.Tell ());
	m_cf.WriteInt (gameData.matCens.nBotCenters);
	for (i = 0; i < gameData.matCens.nBotCenters; i++)
		SaveMatCen (gameData.matCens.botGens + i);
	m_cf.WriteInt (gameData.matCens.nEquipCenters);
	for (i = 0; i < gameData.matCens.nEquipCenters; i++)
		SaveMatCen (gameData.matCens.equipGens + i);
	SaveReactorTrigger (&gameData.reactor.triggers);
	m_cf.WriteInt (gameData.matCens.nFuelCenters);
	for (i = 0; i < gameData.matCens.nFuelCenters; i++)
		SaveFuelCen (gameData.matCens.fuelCenters + i);
	DBG (fPos = m_cf.Tell ());
// Save the control cen info
	m_cf.WriteInt (gameData.reactor.bPresent);
	for (i = 0; i < MAX_BOSS_COUNT; i++)
		SaveReactorState (gameData.reactor.states + i);
	DBG (fPos = m_cf.Tell ());
// Save the AI state
	SaveAI ();

	DBG (fPos = m_cf.Tell ());
// Save the automap visited info
	for (i = 0; i < MAX_SEGMENTS; i++)
		m_cf.WriteShort (gameData.render.mine.bAutomapVisited [i]);
	DBG (fPos = m_cf.Tell ());
	}
m_cf.WriteInt ((int) gameData.app.nStateGameId);
m_cf.WriteInt (gameStates.app.cheats.bLaserRapidFire);
m_cf.WriteInt (gameStates.app.bLunacy);		//	Yes, writing this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
m_cf.WriteInt (gameStates.app.bLunacy);
// Save automap marker info
for (i = 0; i < NUM_MARKERS; i++)
	m_cf.WriteShort (gameData.marker.objects [i]);
m_cf.Write (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1);
m_cf.Write (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1);
m_cf.WriteFix (gameData.physics.xAfterburnerCharge);
//save last was super information
m_cf.Write (bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1);
m_cf.Write (bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1);
//	Save flash effect stuff
m_cf.WriteFix (gameData.render.xFlashEffect);
m_cf.WriteFix (gameData.render.xTimeFlashLastPlayed);
m_cf.WriteShort (gameStates.ogl.palAdd.red);
m_cf.WriteShort (gameStates.ogl.palAdd.green);
m_cf.WriteShort (gameStates.ogl.palAdd.blue);
m_cf.Write (gameData.render.lights.subtracted, sizeof (gameData.render.lights.subtracted [0]), MAX_SEGMENTS);
m_cf.WriteInt (gameStates.app.bFirstSecretVisit);
m_cf.WriteFix (gameData.omega.xCharge [0]);
m_cf.WriteShort (gameData.missions.nEnteredFromLevel);
for (i = 0; i < MAX_PLAYERS; i++)
	SaveSpawnPoint (i);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::SaveImage (void)
{
	gsrCanvas	*thumbCanv = GrCreateCanvas (THUMBNAIL_LW, THUMBNAIL_LH);

if (thumbCanv) {
		grsBitmap	bm;
		int			i, k, x, y;
		gsrCanvas*	saveCanv = grdCurCanv;
	
	GrSetCurrentCanvas (thumbCanv);
	gameStates.render.nFrameFlipFlop = !gameStates.render.nFrameFlipFlop;
	if (gameStates.ogl.nDrawBuffer == GL_BACK) {
		gameStates.render.nFrameFlipFlop = !gameStates.render.nFrameFlipFlop;
		GameRenderFrame ();
		}
	else
		RenderFrame (0, 0);
	bm.bmProps.w = (grdCurScreen->scWidth / THUMBNAIL_LW) * THUMBNAIL_LW;
	bm.bmProps.h = bm.bmProps.w * 3 / 5;	//force 5:3 aspect ratio
	if (bm.bmProps.h > grdCurScreen->scHeight) {
		bm.bmProps.h = (grdCurScreen->scHeight / THUMBNAIL_LH) * THUMBNAIL_LH;
		bm.bmProps.w = bm.bmProps.h * 5 / 3;
		}
	x = (grdCurScreen->scWidth - bm.bmProps.w) / 2;
	y = (grdCurScreen->scHeight - bm.bmProps.h) / 2;
	bm.bmBPP = 3;
	bm.bmProps.rowSize = bm.bmProps.w * bm.bmBPP;
	bm.bmTexBuf = (ubyte *) D2_ALLOC (bm.bmProps.w * bm.bmProps.h * bm.bmBPP);
	//glDisable (GL_TEXTURE_2D);
	OglSetReadBuffer (GL_FRONT, 1);
	glReadPixels (x, y, bm.bmProps.w, bm.bmProps.h, GL_RGB, GL_UNSIGNED_BYTE, bm.bmTexBuf);
	// do a nice, half-way smart (by merging pixel groups using their average color) image resize
	ShrinkTGA (&bm, bm.bmProps.w / THUMBNAIL_LW, bm.bmProps.h / THUMBNAIL_LH, 0);
	GrPaletteStepLoad (NULL);
	// convert the resized TGA to bmp
	for (y = 0; y < THUMBNAIL_LH; y++) {
		i = y * THUMBNAIL_LW * 3;
		k = (THUMBNAIL_LH - y - 1) * THUMBNAIL_LW;
		for (x = 0; x < THUMBNAIL_LW; x++, k++, i += 3)
			thumbCanv->cvBitmap.bmTexBuf [k] = GrFindClosestColor (gamePalette, bm.bmTexBuf [i] / 4, bm.bmTexBuf [i+1] / 4, bm.bmTexBuf [i+2] / 4);
			}
	GrPaletteStepLoad (NULL);
	D2_FREE (bm.bmTexBuf);
	m_cf.Write (thumbCanv->cvBitmap.bmTexBuf, THUMBNAIL_LW * THUMBNAIL_LH, 1);
	GrSetCurrentCanvas (saveCanv);
	GrFreeCanvas (thumbCanv);
	m_cf.Write (gamePalette, 3, 256);
	}
else	{
 	ubyte color = 0;
 	for (int i = 0; i < THUMBNAIL_LW * THUMBNAIL_LH; i++)
		m_cf.Write (&color, sizeof (ubyte), 1);
	}
}

//------------------------------------------------------------------------------

int CSaveGameHandler::SaveState (int bSecret, char *filename, char *description)
{
	int			i;

StopTime ();
if (filename)
	strcpy (m_filename, filename);
if (description)
	strcpy (m_description, description);
m_bSecret = bSecret;
if (!m_cf.Open (m_filename, gameFolders.szSaveDir, "wb", 0)) {
	if (!IsMultiGame)
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_SAVE_ERROR2);
	StartTime (1);
	return 0;
	}

//Save nId
m_cf.Write (dgss_id, sizeof (char) * 4, 1);
//Save m_nVersion
i = STATE_VERSION;
m_cf.Write (&i, sizeof (int), 1);
//Save description
m_cf.Write (m_description, sizeof (char) * DESC_LENGTH, 1);
// Save the current screen shot...
SaveImage ();
SaveGameData ();
if (m_cf.Error ()) {
	if (!IsMultiGame) {
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_SAVE_ERROR);
		m_cf.Close ();
		m_cf.Delete (m_filename, gameFolders.szSaveDir);
		}
	}
else 
	m_cf.Close ();
StartTime (1);
return 1;
}

//	-----------------------------------------------------------------------------------

void CSaveGameHandler::AutoSave (int nSaveSlot)
{
if ((nSaveSlot != (NUM_SAVES + 1)) && m_bInGame) {
	char	filename [FILENAME_LEN];
	const char *pszOverride = m_override;
	strcpy (filename, m_filename);
	sprintf (m_filename, "%s.sg%x", LOCALPLAYER.callsign, NUM_SAVES);
	Save (0, m_bSecret, 0, m_filename);
	m_override = pszOverride;
	strcpy (m_filename, filename);
	}
}

//	-----------------------------------------------------------------------------------

void CSaveGameHandler::PopSecretSave (int nSaveSlot)
{
if ((nSaveSlot != -1) && !(m_bSecret || IsMultiGame)) {
	int	rval;
	char	tempname [32], fc;

	if (nSaveSlot >= 10)
		fc = (nSaveSlot-10) + 'a';
	else
		fc = '0' + nSaveSlot;
	sprintf (tempname, "%csecret.sgc", fc);
	if (m_cf.Exist (tempname, gameFolders.szSaveDir, 0)) {
		rval = m_cf.Copy (tempname, SECRETC_FILENAME);
		Assert (rval == 0);	//	Oops, error copying tempname to secret.sgc!
		}
	else
		m_cf.Delete (SECRETC_FILENAME, gameFolders.szSaveDir);
	}
}

//	-----------------------------------------------------------------------------------

int CSaveGameHandler::Load (int bInGame, int bSecret, int bQuick, const char *pszFilenameOverride)
{
	int	i, nSaveSlot = -1;

m_bInGame = bInGame;
m_bQuick = bQuick;
m_override = pszFilenameOverride;
m_bBetweenLevels = 0;
if (IsMultiGame) {
	MultiInitiateRestoreGame ();
	return 0;
	}
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDStopRecording ();
if (gameData.demo.nState != ND_STATE_NORMAL)
	return 0;
StopTime ();
gameData.app.bGamePaused = 1;
if (m_bQuick) {
	sprintf (m_filename, "%s.quick", LOCALPLAYER.callsign);
	if (!CFile::Exist (m_filename, gameFolders.szSaveDir, 0))
		m_bQuick = 0;
	}
if (!m_bQuick) {
	if (m_override) {
		strcpy (m_filename, m_override);
		nSaveSlot = NUM_SAVES + 1;		//	So we don't tTrigger autosave
		}
	else if (!(nSaveSlot = GetLoadFile (0))) {
		gameData.app.bGamePaused = 0;
		StartTime (1);
		return 0;
		}
	PopSecretSave (nSaveSlot);
	AutoSave (nSaveSlot);
	if (!m_bSecret && bInGame) {
		int choice = ExecMessageBox (NULL, NULL, 2, TXT_YES, TXT_NO, TXT_CONFIRM_LOAD);
		if (choice != 0) {
			gameData.app.bGamePaused = 0;
			StartTime (1);
			return 0;
			}
		}
	}
gameStates.app.bGameRunning = 0;
i = LoadState (0, bSecret);
gameData.app.bGamePaused = 0;
/*---*/PrintLog ("   rebuilding OpenGL texture data\n");
/*---*/PrintLog ("      rebuilding effects\n");
if (i) {
	RebuildRenderContext (1);
	if (bQuick)
		HUDInitMessage (TXT_QUICKLOAD);
	}
StartTime (1);
return i;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::ReadBoundedInt (int nMax, int *nVal)
{
	int	i;

i = m_cf.ReadInt ();
if ((i < 0) || (i > nMax)) {
	Warning (TXT_SAVE_CORRUPT);
	//m_cf.Close ();
	return 1;
	}
*nVal = i;
return 0;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::LoadMission (void)
{
	char	szMission [16];
	int	i, nVersionFilter = gameOpts->app.nVersionFilter;

m_cf.Read (szMission, sizeof (char), 9);
szMission [9] = '\0';
gameOpts->app.nVersionFilter = 3;
i = LoadMissionByName (szMission, -1);
gameOpts->app.nVersionFilter = nVersionFilter;
if (i)
	return 1;
ExecMessageBox (NULL, NULL, 1, "Ok", TXT_MSN_LOAD_ERROR, szMission);
return 0;
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadMulti (char *pszOrgCallSign, int bMulti)
{
if (bMulti)
	strcpy (pszOrgCallSign, LOCALPLAYER.callsign);
else {
	gameData.app.nGameMode = GM_NORMAL;
	SetFunctionMode (FMODE_GAME);
	ChangePlayerNumTo (0);
	strcpy (pszOrgCallSign, gameData.multiplayer.players [0].callsign);
	gameData.multiplayer.nPlayers = 1;
	if (!m_bSecret) {
		InitMultiPlayerObject ();	//make sure tPlayer's tObject set up
		InitPlayerStatsGame ();		//clear all stats
		}
	}
}

//------------------------------------------------------------------------------

int CSaveGameHandler::SetServerPlayer (
	tPlayer *restoredPlayers, int nPlayers, const char *pszServerCallSign, int *pnOtherObjNum, int *pnServerObjNum)
{
	int	i,
			nServerPlayer = -1,
			nOtherObjNum = -1, 
			nServerObjNum = -1;

if (gameStates.multi.nGameType >= IPX_GAME) {
	if (gameStates.multi.bServer || NetworkIAmMaster ())
		nServerPlayer = gameData.multiplayer.nLocalPlayer;
	else {
		nServerPlayer = -1;
		for (i = 0; i < nPlayers; i++)
			if (!strcmp (pszServerCallSign, netPlayers.players [i].callsign)) {
				nServerPlayer = i;
				break;
				}
		}
	if (nServerPlayer > 0) {
		nOtherObjNum = restoredPlayers [0].nObject;
		nServerObjNum = restoredPlayers [nServerPlayer].nObject;
		{
		tNetPlayerInfo h = netPlayers.players [0];
		netPlayers.players [0] = netPlayers.players [nServerPlayer];
		netPlayers.players [nServerPlayer] = h;
		}
		{
		tPlayer h = restoredPlayers [0];
		restoredPlayers [0] = restoredPlayers [nServerPlayer];
		restoredPlayers [nServerPlayer] = h;
		}
		if (gameStates.multi.bServer || NetworkIAmMaster ())
			gameData.multiplayer.nLocalPlayer = 0;
		else if (!gameData.multiplayer.nLocalPlayer)
			gameData.multiplayer.nLocalPlayer = nServerPlayer;
		}
#if 0
	memcpy (netPlayers.players [gameData.multiplayer.nLocalPlayer].network.ipx.node, 
			 IpxGetMyLocalAddress (), 6);
	*((ushort *) (netPlayers.players [gameData.multiplayer.nLocalPlayer].network.ipx.node + 4)) = 
		htons (*((ushort *) (netPlayers.players [gameData.multiplayer.nLocalPlayer].network.ipx.node + 4)));
#endif
	}
*pnOtherObjNum = nOtherObjNum;
*pnServerObjNum = nServerObjNum;
return nServerPlayer;
}

//------------------------------------------------------------------------------

void CSaveGameHandler::GetConnectedPlayers (tPlayer *restoredPlayers, int nPlayers)
{
	int	i, j;

for (i = 0; i < nPlayers; i++) {
	for (j = 0; j < nPlayers; j++) {
		if (!stricmp (restoredPlayers [i].callsign, gameData.multiplayer.players [j].callsign)) {
			if (gameData.multiplayer.players [j].connected) {
				if (gameStates.multi.nGameType == UDP_GAME) {
					memcpy (restoredPlayers [i].netAddress, gameData.multiplayer.players [j].netAddress, 
							  sizeof (gameData.multiplayer.players [j].netAddress));
					memcpy (netPlayers.players [i].network.ipx.node, gameData.multiplayer.players [j].netAddress, 
							  sizeof (gameData.multiplayer.players [j].netAddress));
					}
				restoredPlayers [i].connected = 1;
				break;
				}
			}
		}
	}
for (i = 0; i < MAX_PLAYERS; i++) {
	if (!restoredPlayers [i].connected) {
		memset (restoredPlayers [i].netAddress, 0xFF, sizeof (restoredPlayers [i].netAddress));
		memset (netPlayers.players [i].network.ipx.node, 0xFF, sizeof (netPlayers.players [i].network.ipx.node));
		}
	}
memcpy (gameData.multiplayer.players, restoredPlayers, sizeof (tPlayer) * nPlayers);
gameData.multiplayer.nPlayers = nPlayers;
if (NetworkIAmMaster ()) {
	for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
		if (i == gameData.multiplayer.nLocalPlayer)
			continue;
   	gameData.multiplayer.players [i].connected = 0;
		}
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::FixNetworkObjects (int nServerPlayer, int nOtherObjNum, int nServerObjNum)
{
if (IsMultiGame && (gameStates.multi.nGameType >= IPX_GAME) && (nServerPlayer > 0)) {
	tObject h = OBJECTS [nServerObjNum];
	OBJECTS [nServerObjNum] = OBJECTS [nOtherObjNum];
	OBJECTS [nOtherObjNum] = h;
	OBJECTS [nServerObjNum].info.nId = nServerObjNum;
	OBJECTS [nOtherObjNum].info.nId = 0;
	if (gameData.multiplayer.nLocalPlayer == nServerObjNum) {
		OBJECTS [nServerObjNum].info.controlType = CT_FLYING;
		OBJECTS [nOtherObjNum].info.controlType = CT_REMOTE;
		}
	else if (gameData.multiplayer.nLocalPlayer == nOtherObjNum) {
		OBJECTS [nServerObjNum].info.controlType = CT_REMOTE;
		OBJECTS [nOtherObjNum].info.controlType = CT_FLYING;
		}
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::FixObjects (void)
{
	tObject	*objP = OBJECTS;
	int		i, j, nSegment;

ConvertObjects ();
gameData.objs.nNextSignature = 0;
for (i = 0; i <= gameData.objs.nLastObject [0]; i++, objP++) {
	objP->rType.polyObjInfo.nAltTextures = -1;
	nSegment = objP->info.nSegment;
	// hack for a bug I haven't yet been able to fix 
	if ((objP->info.nType != OBJ_REACTOR) && (objP->info.xShields < 0)) {
		j = FindBoss (i);
		if ((j < 0) || (gameData.boss [j].nDying != i))
			objP->info.nType = OBJ_NONE;
		}
	objP->info.nNextInSeg = objP->info.nPrevInSeg = objP->info.nSegment = -1;
	if (objP->info.nType != OBJ_NONE) {
		LinkObjToSeg (i, nSegment);
		if (objP->info.nSignature > gameData.objs.nNextSignature)
			gameData.objs.nNextSignature = objP->info.nSignature;
		}
	//look for, and fix, boss with bogus shields
	if ((objP->info.nType == OBJ_ROBOT) && ROBOTINFO (objP->info.nId).bossFlag) {
		fix xShieldSave = objP->info.xShields;
		CopyDefaultsToRobot (objP);		//calculate starting shields
		//if in valid range, use loaded shield value
		if (xShieldSave > 0 && (xShieldSave <= objP->info.xShields))
			objP->info.xShields = xShieldSave;
		else
			objP->info.xShields /= 2;  //give tPlayer a break
		}
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::AwardReturningPlayer (tPlayer *retPlayerP, fix xOldGameTime)
{
tPlayer *playerP = gameData.multiplayer.players + gameData.multiplayer.nLocalPlayer;
playerP->level = retPlayerP->level;
playerP->lastScore = retPlayerP->lastScore;
playerP->timeLevel = retPlayerP->timeLevel;
playerP->flags |= (retPlayerP->flags & PLAYER_FLAGS_ALL_KEYS);
playerP->numRobotsLevel = retPlayerP->numRobotsLevel;
playerP->numRobotsTotal = retPlayerP->numRobotsTotal;
playerP->hostages.nRescued = retPlayerP->hostages.nRescued;
playerP->hostages.nTotal = retPlayerP->hostages.nTotal;
playerP->hostages.nOnBoard = retPlayerP->hostages.nOnBoard;
playerP->hostages.nLevel = retPlayerP->hostages.nLevel;
playerP->homingObjectDist = retPlayerP->homingObjectDist;
playerP->hoursLevel = retPlayerP->hoursLevel;
playerP->hoursTotal = retPlayerP->hoursTotal;
//playerP->nCloaks = retPlayerP->nCloaks;
//playerP->nInvuls = retPlayerP->nInvuls;
DoCloakInvulSecretStuff (xOldGameTime);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadNetGame (void)
{
	int	i, j;

netGame.nType = m_cf.ReadByte ();
netGame.nSecurity = m_cf.ReadInt ();
m_cf.Read (netGame.szGameName, 1, NETGAME_NAME_LEN + 1);
m_cf.Read (netGame.szMissionTitle, 1, MISSION_NAME_LEN + 1);
m_cf.Read (netGame.szMissionName, 1, 9);
netGame.nLevel = m_cf.ReadInt ();
netGame.gameMode = (ubyte) m_cf.ReadByte ();
netGame.bRefusePlayers = (ubyte) m_cf.ReadByte ();
netGame.difficulty = (ubyte) m_cf.ReadByte ();
netGame.gameStatus = (ubyte) m_cf.ReadByte ();
netGame.nNumPlayers = (ubyte) m_cf.ReadByte ();
netGame.nMaxPlayers = (ubyte) m_cf.ReadByte ();
netGame.nConnected = (ubyte) m_cf.ReadByte ();
netGame.gameFlags = (ubyte) m_cf.ReadByte ();
netGame.protocolVersion = (ubyte) m_cf.ReadByte ();
netGame.versionMajor = (ubyte) m_cf.ReadByte ();
netGame.versionMinor = (ubyte) m_cf.ReadByte ();
netGame.teamVector = (ubyte) m_cf.ReadByte ();
netGame.DoMegas = (ubyte) m_cf.ReadByte ();
netGame.DoSmarts = (ubyte) m_cf.ReadByte ();
netGame.DoFusions = (ubyte) m_cf.ReadByte ();
netGame.DoHelix = (ubyte) m_cf.ReadByte ();
netGame.DoPhoenix = (ubyte) m_cf.ReadByte ();
netGame.DoAfterburner = (ubyte) m_cf.ReadByte ();
netGame.DoInvulnerability = (ubyte) m_cf.ReadByte ();
netGame.DoCloak = (ubyte) m_cf.ReadByte ();
netGame.DoGauss = (ubyte) m_cf.ReadByte ();
netGame.DoVulcan = (ubyte) m_cf.ReadByte ();
netGame.DoPlasma = (ubyte) m_cf.ReadByte ();
netGame.DoOmega = (ubyte) m_cf.ReadByte ();
netGame.DoSuperLaser = (ubyte) m_cf.ReadByte ();
netGame.DoProximity = (ubyte) m_cf.ReadByte ();
netGame.DoSpread = (ubyte) m_cf.ReadByte ();
netGame.DoSmartMine = (ubyte) m_cf.ReadByte ();
netGame.DoFlash = (ubyte) m_cf.ReadByte ();
netGame.DoGuided = (ubyte) m_cf.ReadByte ();
netGame.DoEarthShaker = (ubyte) m_cf.ReadByte ();
netGame.DoMercury = (ubyte) m_cf.ReadByte ();
netGame.bAllowMarkerView = (ubyte) m_cf.ReadByte ();
netGame.bIndestructibleLights = (ubyte) m_cf.ReadByte ();
netGame.DoAmmoRack = (ubyte) m_cf.ReadByte ();
netGame.DoConverter = (ubyte) m_cf.ReadByte ();
netGame.DoHeadlight = (ubyte) m_cf.ReadByte ();
netGame.DoHoming = (ubyte) m_cf.ReadByte ();
netGame.DoLaserUpgrade = (ubyte) m_cf.ReadByte ();
netGame.DoQuadLasers = (ubyte) m_cf.ReadByte ();
netGame.bShowAllNames = (ubyte) m_cf.ReadByte ();
netGame.BrightPlayers = (ubyte) m_cf.ReadByte ();
netGame.invul = (ubyte) m_cf.ReadByte ();
netGame.FriendlyFireOff = (ubyte) m_cf.ReadByte ();
for (i = 0; i < 2; i++)
	m_cf.Read (netGame.szTeamName [i], 1, CALLSIGN_LEN + 1);		// 18 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.locations [i] = m_cf.ReadInt ();
for (i = 0; i < MAX_PLAYERS; i++)
	for (j = 0; j < MAX_PLAYERS; j++)
		netGame.kills [i][j] = m_cf.ReadShort ();			// 128 bytes
netGame.nSegmentCheckSum = m_cf.ReadShort ();				// 2 bytes
for (i = 0; i < 2; i++)
	netGame.teamKills [i] = m_cf.ReadShort ();				// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.killed [i] = m_cf.ReadShort ();					// 16 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.playerKills [i] = m_cf.ReadShort ();			// 16 bytes
netGame.KillGoal = m_cf.ReadInt ();							// 4 bytes
netGame.xPlayTimeAllowed = m_cf.ReadFix ();					// 4 bytes
netGame.xLevelTime = m_cf.ReadFix ();							// 4 bytes
netGame.controlInvulTime = m_cf.ReadInt ();				// 4 bytes
netGame.monitorVector = m_cf.ReadInt ();					// 4 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.playerScore [i] = m_cf.ReadInt ();				// 32 bytes
for (i = 0; i < MAX_PLAYERS; i++)
	netGame.playerFlags [i] = (ubyte) m_cf.ReadByte ();	// 8 bytes
netGame.nPacketsPerSec = m_cf.ReadShort ();					// 2 bytes
netGame.bShortPackets = (ubyte) m_cf.ReadByte ();			// 1 bytes
// 279 bytes
// 355 bytes total
m_cf.Read (netGame.AuxData, NETGAME_AUX_SIZE, 1);  // Storage for protocol-specific data (e.g., multicast session and port)
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadNetPlayers (void)
{
	int	i;

netPlayers.nType = (ubyte) m_cf.ReadByte ();
netPlayers.nSecurity = m_cf.ReadInt ();
for (i = 0; i < MAX_PLAYERS + 4; i++) {
	m_cf.Read (netPlayers.players [i].callsign, 1, CALLSIGN_LEN + 1);
	m_cf.Read (netPlayers.players [i].network.ipx.server, 1, 4);
	m_cf.Read (netPlayers.players [i].network.ipx.node, 1, 6);
	netPlayers.players [i].versionMajor = (ubyte) m_cf.ReadByte ();
	netPlayers.players [i].versionMinor = (ubyte) m_cf.ReadByte ();
	netPlayers.players [i].computerType = (enum compType) m_cf.ReadByte ();
	netPlayers.players [i].connected = m_cf.ReadByte ();
	netPlayers.players [i].socket = (ushort) m_cf.ReadShort ();
	netPlayers.players [i].rank = (ubyte) m_cf.ReadByte ();
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadPlayer (tPlayer *playerP)
{
	int	i;

m_cf.Read (playerP->callsign, 1, CALLSIGN_LEN + 1); // The callsign of this tPlayer, for net purposes.
m_cf.Read (playerP->netAddress, 1, 6);					// The network address of the player.
playerP->connected = m_cf.ReadByte ();            // Is the tPlayer connected or not?
playerP->nObject = m_cf.ReadInt ();                // What tObject number this tPlayer is. (made an int by mk because it's very often referenced)
playerP->nPacketsGot = m_cf.ReadInt ();         // How many packets we got from them
playerP->nPacketsSent = m_cf.ReadInt ();        // How many packets we sent to them
playerP->flags = (uint) m_cf.ReadInt ();           // Powerup flags, see below...
playerP->energy = m_cf.ReadFix ();                // Amount of energy remaining.
playerP->shields = m_cf.ReadFix ();               // shields remaining (protection)
playerP->lives = m_cf.ReadByte ();                // Lives remaining, 0 = game over.
playerP->level = m_cf.ReadByte ();                // Current level tPlayer is playing. (must be signed for secret levels)
playerP->laserLevel = (ubyte) m_cf.ReadByte ();  // Current level of the laser.
playerP->startingLevel = m_cf.ReadByte ();       // What level the tPlayer started on.
playerP->nKillerObj = m_cf.ReadShort ();       // Who killed me.... (-1 if no one)
playerP->primaryWeaponFlags = (ushort) m_cf.ReadShort ();   // bit set indicates the tPlayer has this weapon.
playerP->secondaryWeaponFlags = (ushort) m_cf.ReadShort (); // bit set indicates the tPlayer has this weapon.
for (i = 0; i < MAX_PRIMARY_WEAPONS; i++)
	playerP->primaryAmmo [i] = (ushort) m_cf.ReadShort (); // How much ammo of each nType.
for (i = 0; i < MAX_SECONDARY_WEAPONS; i++)
	playerP->secondaryAmmo [i] = (ushort) m_cf.ReadShort (); // How much ammo of each nType.
#if 1 //for inventory system
playerP->nInvuls = (ubyte) m_cf.ReadByte ();
playerP->nCloaks = (ubyte) m_cf.ReadByte ();
#endif
playerP->lastScore = m_cf.ReadInt ();           // Score at beginning of current level.
playerP->score = m_cf.ReadInt ();                // Current score.
playerP->timeLevel = m_cf.ReadFix ();            // Level time played
playerP->timeTotal = m_cf.ReadFix ();				// Game time played (high word = seconds)
playerP->cloakTime = m_cf.ReadFix ();					// Time cloaked
if (playerP->cloakTime != 0x7fffffff)
	playerP->cloakTime += gameData.time.xGame;
playerP->invulnerableTime = m_cf.ReadFix ();      // Time invulnerable
if (playerP->invulnerableTime != 0x7fffffff)
	playerP->invulnerableTime += gameData.time.xGame;
playerP->nKillGoalCount = m_cf.ReadShort ();          // Num of players killed this level
playerP->netKilledTotal = m_cf.ReadShort ();       // Number of times killed total
playerP->netKillsTotal = m_cf.ReadShort ();        // Number of net kills total
playerP->numKillsLevel = m_cf.ReadShort ();        // Number of kills this level
playerP->numKillsTotal = m_cf.ReadShort ();        // Number of kills total
playerP->numRobotsLevel = m_cf.ReadShort ();       // Number of initial robots this level
playerP->numRobotsTotal = m_cf.ReadShort ();       // Number of robots total
playerP->hostages.nRescued = (ushort) m_cf.ReadShort (); // Total number of hostages rescued.
playerP->hostages.nTotal = (ushort) m_cf.ReadShort ();         // Total number of hostages.
playerP->hostages.nOnBoard = (ubyte) m_cf.ReadByte ();      // Number of hostages on ship.
playerP->hostages.nLevel = (ubyte) m_cf.ReadByte ();         // Number of hostages on this level.
playerP->homingObjectDist = m_cf.ReadFix ();     // Distance of nearest homing tObject.
playerP->hoursLevel = m_cf.ReadByte ();            // Hours played (since timeTotal can only go up to 9 hours)
playerP->hoursTotal = m_cf.ReadByte ();            // Hours played (since timeTotal can only go up to 9 hours)
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadObject (tObject *objP)
{
objP->info.nSignature = m_cf.ReadInt ();      
objP->info.nType = (ubyte) m_cf.ReadByte (); 
#if DBG
if (objP->info.nType == OBJ_REACTOR)
	objP->info.nType = objP->info.nType;
else 
#endif
if ((m_nVersion < 32) && IS_BOSS (objP))
	gameData.boss [(int) extraGameInfo [0].nBossCount++].nObject = OBJ_IDX (objP);
objP->info.nId = (ubyte) m_cf.ReadByte ();
objP->info.nNextInSeg = m_cf.ReadShort ();
objP->info.nPrevInSeg = m_cf.ReadShort ();
objP->info.controlType = (ubyte) m_cf.ReadByte ();
objP->info.movementType = (ubyte) m_cf.ReadByte ();
objP->info.renderType = (ubyte) m_cf.ReadByte ();
objP->info.nFlags = (ubyte) m_cf.ReadByte ();
objP->info.nSegment = m_cf.ReadShort ();
objP->info.nAttachedObj = m_cf.ReadShort ();
m_cf.ReadVector (objP->info.position.vPos);     
m_cf.ReadMatrix (objP->info.position.mOrient);  
objP->info.xSize = m_cf.ReadFix (); 
objP->info.xShields = m_cf.ReadFix ();
m_cf.ReadVector (objP->info.vLastPos);  
objP->info.contains.nType = m_cf.ReadByte (); 
objP->info.contains.nId = m_cf.ReadByte ();   
objP->info.contains.nCount = m_cf.ReadByte ();
objP->info.nCreator = m_cf.ReadByte ();
objP->info.xLifeLeft = m_cf.ReadFix ();   
if (objP->info.movementType == MT_PHYSICS) {
	m_cf.ReadVector (objP->mType.physInfo.velocity);   
	m_cf.ReadVector (objP->mType.physInfo.thrust);     
	objP->mType.physInfo.mass = m_cf.ReadFix ();       
	objP->mType.physInfo.drag = m_cf.ReadFix ();       
	objP->mType.physInfo.brakes = m_cf.ReadFix ();     
	m_cf.ReadVector (objP->mType.physInfo.rotVel);     
	m_cf.ReadVector (objP->mType.physInfo.rotThrust);  
	objP->mType.physInfo.turnRoll = m_cf.ReadFixAng ();   
	objP->mType.physInfo.flags = (ushort) m_cf.ReadShort ();      
	}
else if (objP->info.movementType == MT_SPINNING) {
	m_cf.ReadVector (objP->mType.spinRate);  
	}
switch (objP->info.controlType) {
	case CT_WEAPON:
		objP->cType.laserInfo.parent.nType = m_cf.ReadShort ();
		objP->cType.laserInfo.parent.nObject = m_cf.ReadShort ();
		objP->cType.laserInfo.parent.nSignature = m_cf.ReadInt ();
		objP->cType.laserInfo.xCreationTime = m_cf.ReadFix ();
		objP->cType.laserInfo.nLastHitObj = m_cf.ReadShort ();
		if (objP->cType.laserInfo.nLastHitObj < 0)
			objP->cType.laserInfo.nLastHitObj = 0;
		else {
			gameData.objs.nHitObjects [OBJ_IDX (objP) * MAX_HIT_OBJECTS] = objP->cType.laserInfo.nLastHitObj;
			objP->cType.laserInfo.nLastHitObj = 1;
			}
		objP->cType.laserInfo.nHomingTarget = m_cf.ReadShort ();
		objP->cType.laserInfo.xScale = m_cf.ReadFix ();
		break;

	case CT_EXPLOSION:
		objP->cType.explInfo.nSpawnTime = m_cf.ReadFix ();
		objP->cType.explInfo.nDeleteTime = m_cf.ReadFix ();
		objP->cType.explInfo.nDeleteObj = m_cf.ReadShort ();
		objP->cType.explInfo.attached.nParent = m_cf.ReadShort ();
		objP->cType.explInfo.attached.nPrev = m_cf.ReadShort ();
		objP->cType.explInfo.attached.nNext = m_cf.ReadShort ();
		break;

	case CT_AI:
		objP->cType.aiInfo.behavior = (ubyte) m_cf.ReadByte ();
		m_cf.Read (objP->cType.aiInfo.flags, 1, MAX_AI_FLAGS);
		objP->cType.aiInfo.nHideSegment = m_cf.ReadShort ();
		objP->cType.aiInfo.nHideIndex = m_cf.ReadShort ();
		objP->cType.aiInfo.nPathLength = m_cf.ReadShort ();
		objP->cType.aiInfo.nCurPathIndex = m_cf.ReadByte ();
		objP->cType.aiInfo.bDyingSoundPlaying = m_cf.ReadByte ();
		objP->cType.aiInfo.nDangerLaser = m_cf.ReadShort ();
		objP->cType.aiInfo.nDangerLaserSig = m_cf.ReadInt ();
		objP->cType.aiInfo.xDyingStartTime = m_cf.ReadFix ();
		break;

	case CT_LIGHT:
		objP->cType.lightInfo.intensity = m_cf.ReadFix ();
		break;

	case CT_POWERUP:
		objP->cType.powerupInfo.nCount = m_cf.ReadInt ();
		objP->cType.powerupInfo.xCreationTime = m_cf.ReadFix ();
		objP->cType.powerupInfo.nFlags = m_cf.ReadInt ();
		break;
	}
switch (objP->info.renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		objP->rType.polyObjInfo.nModel = m_cf.ReadInt ();
		for (i = 0; i < MAX_SUBMODELS; i++)
			m_cf.ReadAngVec (objP->rType.polyObjInfo.animAngles [i]);
		objP->rType.polyObjInfo.nSubObjFlags = m_cf.ReadInt ();
		objP->rType.polyObjInfo.nTexOverride = m_cf.ReadInt ();
		objP->rType.polyObjInfo.nAltTextures = m_cf.ReadInt ();
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		objP->rType.vClipInfo.nClipIndex = m_cf.ReadInt ();
		objP->rType.vClipInfo.xFrameTime = m_cf.ReadFix ();
		objP->rType.vClipInfo.nCurFrame = m_cf.ReadByte ();
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadWall (tWall *wallP)
{
wallP->nSegment = m_cf.ReadInt ();
wallP->nSide = m_cf.ReadInt ();
wallP->hps = m_cf.ReadFix ();    
wallP->nLinkedWall = m_cf.ReadInt ();
wallP->nType = (ubyte) m_cf.ReadByte ();       
wallP->flags = (ubyte) m_cf.ReadByte ();      
wallP->state = (ubyte) m_cf.ReadByte ();      
wallP->nTrigger = (ubyte) m_cf.ReadByte ();    
wallP->nClip = m_cf.ReadByte ();   
wallP->keys = (ubyte) m_cf.ReadByte ();       
wallP->controllingTrigger = m_cf.ReadByte ();
wallP->cloakValue = m_cf.ReadByte (); 
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadExplWall (tExplWall *wallP)
{
wallP->nSegment = m_cf.ReadInt ();
wallP->nSide = m_cf.ReadInt ();
wallP->time = m_cf.ReadFix ();    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadCloakingWall (tCloakingWall *wallP)
{
	int	i;

wallP->nFrontWall = m_cf.ReadShort ();
wallP->nBackWall = m_cf.ReadShort (); 
for (i = 0; i < 4; i++) {
	wallP->front_ls [i] = m_cf.ReadFix (); 
	wallP->back_ls [i] = m_cf.ReadFix ();
	}
wallP->time = m_cf.ReadFix ();    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadActiveDoor (tActiveDoor *doorP)
{
	int	i;

doorP->nPartCount = m_cf.ReadInt ();
for (i = 0; i < 2; i++) {
	doorP->nFrontWall [i] = m_cf.ReadShort ();
	doorP->nBackWall [i] = m_cf.ReadShort ();
	}
doorP->time = m_cf.ReadFix ();    
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadTrigger (tTrigger *triggerP)
{
	int	i;

i = m_cf.Tell ();
triggerP->nType = (ubyte) m_cf.ReadByte (); 
triggerP->flags = (ubyte) m_cf.ReadByte (); 
triggerP->nLinks = m_cf.ReadByte ();
triggerP->value = m_cf.ReadFix ();
triggerP->time = m_cf.ReadFix ();
for (i = 0; i < MAX_TRIGGER_TARGETS; i++) {
	triggerP->nSegment [i] = m_cf.ReadShort ();
	triggerP->nSide [i] = m_cf.ReadShort ();
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadObjTriggerRef (tObjTriggerRef *refP)
{
refP->prev = m_cf.ReadShort ();
refP->next = m_cf.ReadShort ();
refP->nObject = m_cf.ReadShort ();
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadMatCen (tMatCenInfo *matcenP)
{
	int	i;

for (i = 0; i < 2; i++)
	matcenP->objFlags [i] = m_cf.ReadInt ();
matcenP->xHitPoints = m_cf.ReadFix ();
matcenP->xInterval = m_cf.ReadFix ();
matcenP->nSegment = m_cf.ReadShort ();
matcenP->nFuelCen = m_cf.ReadShort ();
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadFuelCen (tFuelCenInfo *fuelcenP)
{
fuelcenP->nType = m_cf.ReadInt ();
fuelcenP->nSegment = m_cf.ReadInt ();
fuelcenP->bFlag = m_cf.ReadByte ();
fuelcenP->bEnabled = m_cf.ReadByte ();
fuelcenP->nLives = m_cf.ReadByte ();
fuelcenP->xCapacity = m_cf.ReadFix ();
fuelcenP->xMaxCapacity = m_cf.ReadFix ();
fuelcenP->xTimer = m_cf.ReadFix ();
fuelcenP->xDisableTime = m_cf.ReadFix ();
m_cf.ReadVector (fuelcenP->vCenter);
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadReactorTrigger (tReactorTriggers *triggerP)
{
	int	i;

triggerP->nLinks = m_cf.ReadShort ();
for (i = 0; i < MAX_CONTROLCEN_LINKS; i++) {
	triggerP->nSegment [i] = m_cf.ReadShort ();
	triggerP->nSide [i] = m_cf.ReadShort ();
	}
}

//------------------------------------------------------------------------------

void CSaveGameHandler::LoadReactorState (tReactorStates *stateP)
{
stateP->nObject = m_cf.ReadInt ();
stateP->bHit = m_cf.ReadInt ();
stateP->bSeenPlayer = m_cf.ReadInt ();
stateP->nNextFireTime = m_cf.ReadInt ();
stateP->nDeadObj = m_cf.ReadInt ();
}

//------------------------------------------------------------------------------

int CSaveGameHandler::LoadSpawnPoint (int i)
{
DBG (i = m_cf.Tell ());
m_cf.ReadVector (gameData.multiplayer.playerInit [i].position.vPos);     
m_cf.ReadMatrix (gameData.multiplayer.playerInit [i].position.mOrient);  
gameData.multiplayer.playerInit [i].nSegment = m_cf.ReadShort ();
gameData.multiplayer.playerInit [i].nSegType = m_cf.ReadShort ();
return (gameData.multiplayer.playerInit [i].nSegment >= 0) &&
		 (gameData.multiplayer.playerInit [i].nSegment < gameData.segs.nSegments) &&
		 (gameData.multiplayer.playerInit [i].nSegment ==
		  FindSegByPos (gameData.multiplayer.playerInit [i].position.vPos, 
							 gameData.multiplayer.playerInit [i].nSegment, 1, 0));

}

//------------------------------------------------------------------------------

int CSaveGameHandler::LoadUniFormat (int bMulti, fix xOldGameTime, int *nLevel)
{
	tPlayer	restoredPlayers [MAX_PLAYERS];
	int		nPlayers, nServerPlayer = -1;
	int		nOtherObjNum = -1, nServerObjNum = -1, nLocalObjNum = -1, nSavedLocalPlayer = -1;
	int		nCurrentLevel, nNextLevel;
	tWall		*wallP;
	char		szOrgCallSign [CALLSIGN_LEN+16];
	int		h, i, j;
	short		nTexture;

if (m_nVersion >= 39) {
	h = m_cf.ReadInt ();
	if (h != gameData.segs.nMaxSegments) {
		Warning (TXT_MAX_SEGS_WARNING, h);
		return 0;
		}
	}
m_bBetweenLevels = m_cf.ReadInt ();
Assert (m_bBetweenLevels == 0);	//between levels save ripped out
// Read the mission info...
if (!LoadMission ())
	return 0;
//Read level info
nCurrentLevel = m_cf.ReadInt ();
nNextLevel = m_cf.ReadInt ();
//Restore gameData.time.xGame
gameData.time.xGame = m_cf.ReadFix ();
// Start new game....
CSaveGameHandler::LoadMulti (szOrgCallSign, bMulti);
if (IsMultiGame) {
		char szServerCallSign [CALLSIGN_LEN + 1];

	strcpy (szServerCallSign, netPlayers.players [0].callsign);
	gameData.app.nStateGameId = m_cf.ReadInt ();
	CSaveGameHandler::LoadNetGame ();
	DBG (fPos = m_cf.Tell ());
	CSaveGameHandler::LoadNetPlayers ();
	DBG (fPos = m_cf.Tell ());
	nPlayers = m_cf.ReadInt ();
	nSavedLocalPlayer = gameData.multiplayer.nLocalPlayer;
	gameData.multiplayer.nLocalPlayer = m_cf.ReadInt ();
	for (i = 0; i < nPlayers; i++) {
		CSaveGameHandler::LoadPlayer (restoredPlayers + i);
		restoredPlayers [i].connected = 0;
		}
	DBG (fPos = m_cf.Tell ());
	// make sure the current game host is in tPlayer slot #0
	nServerPlayer = SetServerPlayer (restoredPlayers, nPlayers, szServerCallSign, &nOtherObjNum, &nServerObjNum);
	GetConnectedPlayers (restoredPlayers, nPlayers);
	}
//Read tPlayer info
if (!StartNewLevelSub (nCurrentLevel, 1, m_bSecret, 1)) {
	m_cf.Close ();
	return 0;
	}
nLocalObjNum = LOCALPLAYER.nObject;
if (m_bSecret != 1)	//either no secret restore, or tPlayer died in scret level
	CSaveGameHandler::LoadPlayer (gameData.multiplayer.players + gameData.multiplayer.nLocalPlayer);
else {
	tPlayer	retPlayer;
	CSaveGameHandler::LoadPlayer (&retPlayer);
	AwardReturningPlayer (&retPlayer, xOldGameTime);
	}
LOCALPLAYER.nObject = nLocalObjNum;
strcpy (LOCALPLAYER.callsign, szOrgCallSign);
// Set the right level
if (m_bBetweenLevels)
	LOCALPLAYER.level = nNextLevel;
// Restore the weapon states
gameData.weapons.nPrimary = m_cf.ReadByte ();
gameData.weapons.nSecondary = m_cf.ReadByte ();
SelectWeapon (gameData.weapons.nPrimary, 0, 0, 0);
SelectWeapon (gameData.weapons.nSecondary, 1, 0, 0);
// Restore the difficulty level
gameStates.app.nDifficultyLevel = m_cf.ReadInt ();
// Restore the cheats enabled flag
gameStates.app.cheats.bEnabled = m_cf.ReadInt ();
for (i = 0; i < 2; i++) {
	if (m_nVersion < 33) {
		gameStates.gameplay.slowmo [i].fSpeed = 1;
		gameStates.gameplay.slowmo [i].nState = 0;
		}
	else {
		gameStates.gameplay.slowmo [i].fSpeed = X2F (m_cf.ReadInt ());
		gameStates.gameplay.slowmo [i].nState = m_cf.ReadInt ();
		}
	}
if (m_nVersion > 33) {
	for (i = 0; i < MAX_PLAYERS; i++)
	   if (i != gameData.multiplayer.nLocalPlayer)
		   gameData.multiplayer.weaponStates [i].bTripleFusion = m_cf.ReadInt ();
   	else {
   	   gameData.weapons.bTripleFusion = m_cf.ReadInt ();
		   gameData.multiplayer.weaponStates [i].bTripleFusion = !gameData.weapons.bTripleFusion;  //force MultiSendWeapons
		   }
	}
if (!m_bBetweenLevels)	{
	gameStates.render.bDoAppearanceEffect = 0;			// Don't do this for middle o' game stuff.
	//Clear out all the objects from the lvl file
	ResetSegObjLists ();
	ResetObjects (1);

	//Read objects, and pop 'em into their respective segments.
	DBG (fPos = m_cf.Tell ());
	h = m_cf.ReadInt ();
	gameData.objs.nLastObject [0] = h - 1;
	extraGameInfo [0].nBossCount = 0;
	for (i = 0; i < h; i++)
		LoadObject (OBJECTS + i);
	DBG (fPos = m_cf.Tell ());
	FixNetworkObjects (nServerPlayer, nOtherObjNum, nServerObjNum);
	gameData.objs.nNextSignature = 0;
	InitCamBots (1);
	gameData.objs.nNextSignature++;
	//	1 = Didn't die on secret level.
	//	2 = Died on secret level.
	if (m_bSecret && (gameData.missions.nCurrentLevel >= 0)) {
		SetPosFromReturnSegment (0);
		if (m_bSecret == 2)
			InitPlayerStatsNewShip ();
		}
	//Restore tWall info
	if (ReadBoundedInt (MAX_WALLS, &gameData.walls.nWalls))
		return 0;
	for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; i++, wallP++) {
		CSaveGameHandler::LoadWall (wallP);
		if (wallP->nType == WALL_OPEN)
			DigiKillSoundLinkedToSegment ((short) wallP->nSegment, (short) wallP->nSide, -1);	//-1 means kill any sound
		}
	DBG (fPos = m_cf.Tell ());
	//Restore exploding wall info
	if (ReadBoundedInt (MAX_EXPLODING_WALLS, &h))
		return 0;
	for (i = 0; i < h; i++)
		CSaveGameHandler::LoadExplWall (gameData.walls.explWalls + i);
	DBG (fPos = m_cf.Tell ());
	//Restore door info
	if (ReadBoundedInt (MAX_DOORS, &gameData.walls.nOpenDoors))
		return 0;
	for (i = 0; i < gameData.walls.nOpenDoors; i++)
		CSaveGameHandler::LoadActiveDoor (gameData.walls.activeDoors + i);
	DBG (fPos = m_cf.Tell ());
	if (ReadBoundedInt (MAX_WALLS, &gameData.walls.nCloaking))
		return 0;
	for (i = 0; i < gameData.walls.nCloaking; i++)
		CSaveGameHandler::LoadCloakingWall (gameData.walls.cloaking + i);
	DBG (fPos = m_cf.Tell ());
	//Restore tTrigger info
	if (ReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nTriggers))
		return 0;
	for (i = 0; i < gameData.trigs.nTriggers; i++)
		CSaveGameHandler::LoadTrigger (gameData.trigs.triggers + i);
	DBG (fPos = m_cf.Tell ());
	//Restore tObject tTrigger info
	if (ReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nObjTriggers))
		return 0;
	if (gameData.trigs.nObjTriggers > 0) {
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			CSaveGameHandler::LoadTrigger (gameData.trigs.objTriggers + i);
		for (i = 0; i < gameData.trigs.nObjTriggers; i++)
			CSaveGameHandler::LoadObjTriggerRef (gameData.trigs.objTriggerRefs + i);
		if (m_nVersion < 36) {
			j = (m_nVersion < 35) ? 700 : MAX_OBJECTS_D2X;
			for (i = 0; i < j; i++)
				gameData.trigs.firstObjTrigger [i] = m_cf.ReadShort ();
			}
		else {
			memset (gameData.trigs.firstObjTrigger, 0xff, sizeof (short) * MAX_OBJECTS_D2X);
			for (i = m_cf.ReadShort (); i; i--) {
				j = m_cf.ReadShort ();
				gameData.trigs.firstObjTrigger [j] = m_cf.ReadShort ();
				}
			}
		}
	else if (m_nVersion < 36)
		m_cf.Seek (((m_nVersion < 35) ? 700 : MAX_OBJECTS_D2X) * sizeof (short), SEEK_CUR);
	else
		m_cf.ReadShort ();
	DBG (fPos = m_cf.Tell ());
	//Restore tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++)	{
		for (j = 0; j < 6; j++)	{
			gameData.segs.segments [i].sides [j].nWall = m_cf.ReadShort ();
			gameData.segs.segments [i].sides [j].nBaseTex = m_cf.ReadShort ();
			nTexture = m_cf.ReadShort ();
			gameData.segs.segments [i].sides [j].nOvlTex = nTexture & 0x3fff;
			gameData.segs.segments [i].sides [j].nOvlOrient = (nTexture >> 14) & 3;
			}
		}
	DBG (fPos = m_cf.Tell ());
	//Restore the fuelcen info
	for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; i++, wallP++) {
		if ((wallP->nType == WALL_DOOR) && (wallP->flags & WALL_DOOR_OPENED))
			AnimateOpeningDoor (SEGMENTS + wallP->nSegment, wallP->nSide, -1);
		else if ((wallP->nType == WALL_BLASTABLE) && (wallP->flags & WALL_BLASTED))
			BlastBlastableWall (SEGMENTS + wallP->nSegment, wallP->nSide);
		}
	gameData.reactor.bDestroyed = m_cf.ReadInt ();
	gameData.reactor.countdown.nTimer = m_cf.ReadFix ();
	DBG (fPos = m_cf.Tell ());
	if (ReadBoundedInt (MAX_ROBOT_CENTERS, &gameData.matCens.nBotCenters))
		return 0;
	for (i = 0; i < gameData.matCens.nBotCenters; i++)
		CSaveGameHandler::LoadMatCen (gameData.matCens.botGens + i);
	if (m_nVersion >= 30) {
		if (ReadBoundedInt (MAX_EQUIP_CENTERS, &gameData.matCens.nEquipCenters))
			return 0;
		for (i = 0; i < gameData.matCens.nEquipCenters; i++)
			CSaveGameHandler::LoadMatCen (gameData.matCens.equipGens + i);
		}
	else {
		gameData.matCens.nBotCenters = 0;
		memset (gameData.matCens.botGens, 0, sizeof (gameData.matCens.botGens));
		}
	CSaveGameHandler::LoadReactorTrigger (&gameData.reactor.triggers);
	if (ReadBoundedInt (MAX_FUEL_CENTERS, &gameData.matCens.nFuelCenters))
		return 0;
	for (i = 0; i < gameData.matCens.nFuelCenters; i++)
		CSaveGameHandler::LoadFuelCen (gameData.matCens.fuelCenters + i);
	DBG (fPos = m_cf.Tell ());
	// Restore the control cen info
	if (m_nVersion < 31) {
		gameData.reactor.states [0].bHit = m_cf.ReadInt ();
		gameData.reactor.states [0].bSeenPlayer = m_cf.ReadInt ();
		gameData.reactor.states [0].nNextFireTime = m_cf.ReadInt ();
		gameData.reactor.bPresent = m_cf.ReadInt ();
		gameData.reactor.states [0].nDeadObj = m_cf.ReadInt ();
		}
	else {
		int	i;

		gameData.reactor.bPresent = m_cf.ReadInt ();
		for (i = 0; i < MAX_BOSS_COUNT; i++)
			LoadReactorState (gameData.reactor.states + i);
		}
	DBG (fPos = m_cf.Tell ());
	// Restore the AI state
	LoadAIUniFormat ();
	// Restore the automap visited info
	DBG (fPos = m_cf.Tell ());
	FixObjects ();
	SpecialResetObjects ();
	if (m_nVersion > 37) {
		for (i = 0; i < MAX_SEGMENTS; i++)
			gameData.render.mine.bAutomapVisited [i] = (ushort) m_cf.ReadShort ();
		}
	else {
		int	i, j = (m_nVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2;
		for (i = 0; i < j; i++)
			gameData.render.mine.bAutomapVisited [i] = (ushort) m_cf.ReadByte ();
		}
	DBG (fPos = m_cf.Tell ());
	//	Restore hacked up weapon system stuff.
	gameData.fusion.xNextSoundTime = gameData.time.xGame;
	gameData.fusion.xAutoFireTime = 0;
	gameData.laser.xNextFireTime = 
	gameData.missiles.xNextFireTime = gameData.time.xGame;
	gameData.laser.xLastFiredTime = 
	gameData.missiles.xLastFiredTime = gameData.time.xGame;
	}
gameData.app.nStateGameId = 0;
gameData.app.nStateGameId = (uint) m_cf.ReadInt ();
gameStates.app.cheats.bLaserRapidFire = m_cf.ReadInt ();
gameStates.app.bLunacy = m_cf.ReadInt ();		//	Yes, reading this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
gameStates.app.bLunacy = m_cf.ReadInt ();
if (gameStates.app.bLunacy)
	DoLunacyOn ();

DBG (fPos = m_cf.Tell ());
for (i = 0; i < NUM_MARKERS; i++)
	gameData.marker.objects [i] = m_cf.ReadShort ();
m_cf.Read (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1);
m_cf.Read (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1);

if (m_bSecret != 1)
	gameData.physics.xAfterburnerCharge = m_cf.ReadFix ();
else {
	m_cf.ReadFix ();
	}
//read last was super information
m_cf.Read (&bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1);
m_cf.Read (&bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1);
gameData.render.xFlashEffect = m_cf.ReadFix ();
gameData.render.xTimeFlashLastPlayed = m_cf.ReadFix ();
gameStates.ogl.palAdd.red = m_cf.ReadShort ();
gameStates.ogl.palAdd.green = m_cf.ReadShort ();
gameStates.ogl.palAdd.blue = m_cf.ReadShort ();
m_cf.Read (gameData.render.lights.subtracted, 
		  sizeof (gameData.render.lights.subtracted [0]), 
		  (m_nVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2);
ApplyAllChangedLight ();
gameStates.app.bFirstSecretVisit = m_cf.ReadInt ();
if (m_bSecret) 
	gameStates.app.bFirstSecretVisit = 0;

if (m_bSecret != 1)
	gameData.omega.xCharge [0] = 
	gameData.omega.xCharge [1] = m_cf.ReadFix ();
else
	m_cf.ReadFix ();
if (m_nVersion > 27)
	gameData.missions.nEnteredFromLevel = m_cf.ReadShort ();
*nLevel = nCurrentLevel;
if (m_nVersion >= 37) {
	tObjPosition playerInitSave [MAX_PLAYERS];

	memcpy (playerInitSave, gameData.multiplayer.playerInit, sizeof (playerInitSave));
	for (h = 1, i = 0; i < MAX_PLAYERS; i++)
		if (!CSaveGameHandler::LoadSpawnPoint (i))
			h = 0;
	if (!h)
		memcpy (gameData.multiplayer.playerInit, playerInitSave, sizeof (playerInitSave));
	}
return 1;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::LoadBinFormat (int bMulti, fix xOldGameTime, int *nLevel)
{
	tPlayer	restoredPlayers [MAX_PLAYERS];
	int		nPlayers, nServerPlayer = -1;
	int		nOtherObjNum = -1, nServerObjNum = -1, nLocalObjNum = -1, nSavedLocalPlayer = -1;
	int		nCurrentLevel, nNextLevel;
	tWall		*wallP;
	char		szOrgCallSign [CALLSIGN_LEN+16];
	int		i, j;
	short		nTexture;

m_cf.Read (&m_bBetweenLevels, sizeof (int), 1);
Assert (m_bBetweenLevels == 0);	//between levels save ripped out
// Read the mission info...
if (!LoadMission ())
	return 0;
//Read level info
m_cf.Read (&nCurrentLevel, sizeof (int), 1);
m_cf.Read (&nNextLevel, sizeof (int), 1);
//Restore gameData.time.xGame
m_cf.Read (&gameData.time.xGame, sizeof (fix), 1);
// Start new game....
CSaveGameHandler::LoadMulti (szOrgCallSign, bMulti);
if (gameData.app.nGameMode & GM_MULTI) {
		char szServerCallSign [CALLSIGN_LEN + 1];

	strcpy (szServerCallSign, netPlayers.players [0].callsign);
	m_cf.Read (&gameData.app.nStateGameId, sizeof (int), 1);
	m_cf.Read (&netGame, sizeof (tNetgameInfo), 1);
	m_cf.Read (&netPlayers, sizeof (tAllNetPlayersInfo), 1);
	m_cf.Read (&nPlayers, sizeof (gameData.multiplayer.nPlayers), 1);
	m_cf.Read (&gameData.multiplayer.nLocalPlayer, sizeof (gameData.multiplayer.nLocalPlayer), 1);
	nSavedLocalPlayer = gameData.multiplayer.nLocalPlayer;
	for (i = 0; i < nPlayers; i++)
		m_cf.Read (restoredPlayers + i, sizeof (tPlayer), 1);
	nServerPlayer = SetServerPlayer (restoredPlayers, nPlayers, szServerCallSign, &nOtherObjNum, &nServerObjNum);
	GetConnectedPlayers (restoredPlayers, nPlayers);
	}

//Read tPlayer info
if (!StartNewLevelSub (nCurrentLevel, 1, m_bSecret, 1)) {
	m_cf.Close ();
	return 0;
	}
nLocalObjNum = LOCALPLAYER.nObject;
if (m_bSecret != 1)	//either no secret restore, or tPlayer died in scret level
	m_cf.Read (gameData.multiplayer.players + gameData.multiplayer.nLocalPlayer, sizeof (tPlayer), 1);
else {
	tPlayer	retPlayer;
	m_cf.Read (&retPlayer, sizeof (tPlayer), 1);
	AwardReturningPlayer (&retPlayer, xOldGameTime);
	}
LOCALPLAYER.nObject = nLocalObjNum;
strcpy (LOCALPLAYER.callsign, szOrgCallSign);
// Set the right level
if (m_bBetweenLevels)
	LOCALPLAYER.level = nNextLevel;
// Restore the weapon states
m_cf.Read (&gameData.weapons.nPrimary, sizeof (sbyte), 1);
m_cf.Read (&gameData.weapons.nSecondary, sizeof (sbyte), 1);
SelectWeapon (gameData.weapons.nPrimary, 0, 0, 0);
SelectWeapon (gameData.weapons.nSecondary, 1, 0, 0);
// Restore the difficulty level
m_cf.Read (&gameStates.app.nDifficultyLevel, sizeof (int), 1);
// Restore the cheats enabled flag
m_cf.Read (&gameStates.app.cheats.bEnabled, sizeof (int), 1);
if (!m_bBetweenLevels)	{
	gameStates.render.bDoAppearanceEffect = 0;			// Don't do this for middle o' game stuff.
	//Clear out all the OBJECTS from the lvl file
	ResetSegObjLists ();
	ResetObjects (1);
	//Read objects, and pop 'em into their respective segments.
	m_cf.Read (&i, sizeof (int), 1);
	gameData.objs.nLastObject [0] = i - 1;
	m_cf.Read (OBJECTS, sizeof (tObject), i);
	FixNetworkObjects (nServerPlayer, nOtherObjNum, nServerObjNum);
	FixObjects ();
	SpecialResetObjects ();
	InitCamBots (1);
	gameData.objs.nNextSignature++;
	//	1 = Didn't die on secret level.
	//	2 = Died on secret level.
	if (m_bSecret && (gameData.missions.nCurrentLevel >= 0)) {
		SetPosFromReturnSegment (0);
		if (m_bSecret == 2)
			InitPlayerStatsNewShip ();
		}
	//Restore tWall info
	if (ReadBoundedInt (MAX_WALLS, &gameData.walls.nWalls))
		return 0;
	m_cf.Read (gameData.walls.walls, sizeof (tWall), gameData.walls.nWalls);
	//now that we have the walls, check if any sounds are linked to
	//walls that are now open
	for (i = 0, wallP = gameData.walls.walls; i < gameData.walls.nWalls; i++, wallP++)
		if (wallP->nType == WALL_OPEN)
			DigiKillSoundLinkedToSegment ((short) wallP->nSegment, (short) wallP->nSide, -1);	//-1 means kill any sound
	//Restore exploding wall info
	if (m_nVersion >= 10) {
		m_cf.Read (&i, sizeof (int), 1);
		m_cf.Read (gameData.walls.explWalls, sizeof (*gameData.walls.explWalls), i);
		}
	//Restore door info
	if (ReadBoundedInt (MAX_DOORS, &gameData.walls.nOpenDoors))
		return 0;
	m_cf.Read (gameData.walls.activeDoors, sizeof (tActiveDoor), gameData.walls.nOpenDoors);
	if (m_nVersion >= 14) {		//Restore cloaking tWall info
		if (ReadBoundedInt (MAX_WALLS, &gameData.walls.nCloaking))
			return 0;
		m_cf.Read (gameData.walls.cloaking, sizeof (tCloakingWall), gameData.walls.nCloaking);
		}
	//Restore tTrigger info
	if (ReadBoundedInt (MAX_TRIGGERS, &gameData.trigs.nTriggers))
		return 0;
	m_cf.Read (gameData.trigs.triggers, sizeof (tTrigger), gameData.trigs.nTriggers);
	if (m_nVersion >= 26) {
		//Restore tObject tTrigger info

		m_cf.Read (&gameData.trigs.nObjTriggers, sizeof (gameData.trigs.nObjTriggers), 1);
		if (gameData.trigs.nObjTriggers > 0) {
			m_cf.Read (gameData.trigs.objTriggers, sizeof (tTrigger), gameData.trigs.nObjTriggers);
			m_cf.Read (gameData.trigs.objTriggerRefs, sizeof (tObjTriggerRef), gameData.trigs.nObjTriggers);
			m_cf.Read (gameData.trigs.firstObjTrigger, sizeof (short), 700);
			}
		else
			m_cf.Seek ((m_nVersion < 35) ? 700 : MAX_OBJECTS_D2X * sizeof (short), SEEK_CUR);
		}
	//Restore tmap info
	for (i = 0; i <= gameData.segs.nLastSegment; i++)	{
		for (j = 0; j < 6; j++)	{
			gameData.segs.segments [i].sides [j].nWall = m_cf.ReadShort ();
			gameData.segs.segments [i].sides [j].nBaseTex = m_cf.ReadShort ();
			nTexture = m_cf.ReadShort ();
			gameData.segs.segments [i].sides [j].nOvlTex = nTexture & 0x3fff;
			gameData.segs.segments [i].sides [j].nOvlOrient = (nTexture >> 14) & 3;
			}
		}
//Restore the fuelcen info
	gameData.reactor.bDestroyed = m_cf.ReadInt ();
	gameData.reactor.countdown.nTimer = m_cf.ReadFix ();
	if (ReadBoundedInt (MAX_ROBOT_CENTERS, &gameData.matCens.nBotCenters))
		return 0;
	for (i = 0; i < gameData.matCens.nBotCenters; i++) {
		m_cf.Read (gameData.matCens.botGens [i].objFlags, sizeof (int), 2);
		m_cf.Read (&gameData.matCens.botGens [i].xHitPoints, sizeof (tMatCenInfo) - ((char *) &gameData.matCens.botGens [i].xHitPoints - (char *) &gameData.matCens.botGens [i]), 1);
		}
	m_cf.Read (&gameData.reactor.triggers, sizeof (tReactorTriggers), 1);
	if (ReadBoundedInt (MAX_FUEL_CENTERS, &gameData.matCens.nFuelCenters))
		return 0;
	m_cf.Read (gameData.matCens.fuelCenters, sizeof (tFuelCenInfo), gameData.matCens.nFuelCenters);

	// Restore the control cen info
	gameData.reactor.states [0].bHit = m_cf.ReadInt ();
	gameData.reactor.states [0].bSeenPlayer = m_cf.ReadInt ();
	gameData.reactor.states [0].nNextFireTime = m_cf.ReadInt ();
	gameData.reactor.bPresent = m_cf.ReadInt ();
	gameData.reactor.states [0].nDeadObj = m_cf.ReadInt ();
	// Restore the AI state
	LoadAIBinFormat ();
	// Restore the automap visited info
	if (m_nVersion > 37)
		m_cf.Read (gameData.render.mine.bAutomapVisited, sizeof (ushort), MAX_SEGMENTS);
	else {
		int	i, j = (m_nVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2;
		for (i = 0; i < j; i++)
			gameData.render.mine.bAutomapVisited [i] = (ushort) m_cf.ReadByte ();
		}
	
	m_cf.Read (gameData.render.mine.bAutomapVisited, sizeof (ubyte), (m_nVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2);

	//	Restore hacked up weapon system stuff.
	gameData.fusion.xNextSoundTime = gameData.time.xGame;
	gameData.fusion.xAutoFireTime = 0;
	gameData.laser.xNextFireTime = 
	gameData.missiles.xNextFireTime = gameData.time.xGame;
	gameData.laser.xLastFiredTime = 
	gameData.missiles.xLastFiredTime = gameData.time.xGame;
}
gameData.app.nStateGameId = 0;

if (m_nVersion >= 7)	{
	m_cf.Read (&gameData.app.nStateGameId, sizeof (uint), 1);
	m_cf.Read (&gameStates.app.cheats.bLaserRapidFire, sizeof (int), 1);
	m_cf.Read (&gameStates.app.bLunacy, sizeof (int), 1);		//	Yes, writing this twice.  Removed the Ugly robot system, but didn't want to change savegame format.
	m_cf.Read (&gameStates.app.bLunacy, sizeof (int), 1);
	if (gameStates.app.bLunacy)
		DoLunacyOn ();
}

if (m_nVersion >= 17) {
	m_cf.Read (gameData.marker.objects, sizeof (gameData.marker.objects), 1);
	m_cf.Read (gameData.marker.nOwner, sizeof (gameData.marker.nOwner), 1);
	m_cf.Read (gameData.marker.szMessage, sizeof (gameData.marker.szMessage), 1);
}
else {
	int num,dummy;

	// skip dummy info
	m_cf.Read (&num, sizeof (int), 1);       //was NumOfMarkers
	m_cf.Read (&dummy, sizeof (int), 1);     //was CurMarker
	m_cf.Seek (num * (sizeof (vmsVector) + 40), SEEK_CUR);
	for (num = 0; num < NUM_MARKERS; num++)
		gameData.marker.objects [num] = -1;
}

if (m_nVersion >= 11) {
	if (m_bSecret != 1)
		m_cf.Read (&gameData.physics.xAfterburnerCharge, sizeof (fix), 1);
	else {
		fix	dummy_fix;
		m_cf.Read (&dummy_fix, sizeof (fix), 1);
	}
}
if (m_nVersion >= 12) {
	//read last was super information
	m_cf.Read (&bLastPrimaryWasSuper, sizeof (bLastPrimaryWasSuper), 1);
	m_cf.Read (&bLastSecondaryWasSuper, sizeof (bLastSecondaryWasSuper), 1);
	m_cf.Read (&gameData.render.xFlashEffect, sizeof (int), 1);
	m_cf.Read (&gameData.render.xTimeFlashLastPlayed, sizeof (int), 1);
	m_cf.Read (&gameStates.ogl.palAdd.red, sizeof (int), 1);
	m_cf.Read (&gameStates.ogl.palAdd.green, sizeof (int), 1);
	m_cf.Read (&gameStates.ogl.palAdd.blue, sizeof (int), 1);
	}
else {
	ResetPaletteAdd ();
	}

//	Load gameData.render.lights.subtracted
if (m_nVersion >= 16) {
	m_cf.Read (gameData.render.lights.subtracted, sizeof (gameData.render.lights.subtracted [0]), (m_nVersion > 22) ? MAX_SEGMENTS : MAX_SEGMENTS_D2);
	ApplyAllChangedLight ();
	//ComputeAllStaticLight ();	//	set xAvgSegLight field in tSegment struct.  See note at that function.
	}
else
	memset (gameData.render.lights.subtracted, 0, sizeof (gameData.render.lights.subtracted));

if (m_bSecret) 
	gameStates.app.bFirstSecretVisit = 0;
else if (m_nVersion >= 20)
	m_cf.Read (&gameStates.app.bFirstSecretVisit, sizeof (gameStates.app.bFirstSecretVisit), 1);
else
	gameStates.app.bFirstSecretVisit = 1;

if (m_nVersion >= 22) {
	if (m_bSecret != 1)
		m_cf.Read (&gameData.omega.xCharge, sizeof (fix), 1);
	else {
		fix	dummy_fix;
		m_cf.Read (&dummy_fix, sizeof (fix), 1);
		}
	}
*nLevel = nCurrentLevel;
return 1;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::LoadState (int bMulti, int bSecret, char *filename)
{
	char		szDescription [DESC_LENGTH + 1];
	char		nId [5];
	int		nLevel, i;
	fix		xOldGameTime = gameData.time.xGame;

StopTime ();
if (filename)
	strcpy (m_filename, filename);
if (!m_cf.Open (m_filename, gameFolders.szSaveDir, "rb", 0)) {
	StartTime (1);
	return 0;
	}
m_bSecret = bSecret;
//Read nId
m_cf.Read (nId, sizeof (char)*4, 1);
if (memcmp (nId, dgss_id, 4)) {
	m_cf.Close ();
	StartTime (1);
	return 0;
	}
//Read m_nVersion
m_cf.Read (&m_nVersion, sizeof (int), 1);
if (m_nVersion < STATE_COMPATIBLE_VERSION)	{
	m_cf.Close ();
	StartTime (1);
	return 0;
	}
// Read description
m_cf.Read (szDescription, sizeof (char) * DESC_LENGTH, 1);
// Skip the current screen shot...
m_cf.Seek ((m_nVersion < 26) ? THUMBNAIL_W * THUMBNAIL_H : THUMBNAIL_LW * THUMBNAIL_LH, SEEK_CUR);
// skip the palette
m_cf.Seek (768, SEEK_CUR);
if (m_nVersion < 27)
	i = LoadBinFormat (bMulti, xOldGameTime, &nLevel);
else
	i = LoadUniFormat (bMulti, xOldGameTime, &nLevel);
m_cf.Close ();
if (!i) {
	StartTime (1);
	return 0;
	}
FixObjectSegs ();
FixObjectSizes ();
//ComputeNearestLights (nLevel);
ComputeStaticDynLighting (nLevel);
SetupEffects ();
InitReactorForLevel (1);
AddPlayerLoadout ();
SetMaxOmegaCharge ();
SetEquipGenStates ();
if (!IsMultiGame)
	InitEntropySettings (0);	//required for repair centers
else {
	for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
	  if (!gameData.multiplayer.players [i].connected) {
			NetworkDisconnectPlayer (i);
  			CreatePlayerAppearanceEffect (OBJECTS + gameData.multiplayer.players [i].nObject);
	      }
		}
	}
gameData.objs.viewerP = 
gameData.objs.consoleP = OBJECTS + LOCALPLAYER.nObject;
StartTime (1);
if (!extraGameInfo [0].nBossCount && (!IsMultiGame || IsCoopGame) && OpenExits ())
	InitCountdown (NULL, 1, -1);
return 1;
}

//------------------------------------------------------------------------------

int CSaveGameHandler::GetGameId (char *filename)
{
	int	nId;

if (!m_cf.Open (filename, gameFolders.szSaveDir, "rb", 0))
	return 0;
//Read nId
m_cf.Read (&nId, sizeof (char) * 4, 1);
if (memcmp (&nId, dgss_id, 4)) {
	m_cf.Close ();
	return 0;
	}
//Read m_nVersion
m_nVersion = m_cf.ReadInt ();
if (m_nVersion < STATE_COMPATIBLE_VERSION) {
	m_cf.Close ();
	return 0;
	}
// skip the description, current image, palette, mission name, between levels flag and mission info
m_cf.Seek (DESC_LENGTH + ImageSize () + 768 + 9 * sizeof (char) + 5 * sizeof (int), SEEK_CUR);
m_nGameId = m_cf.ReadInt ();
m_cf.Close ();
return m_nGameId;
}

//------------------------------------------------------------------------------
//eof
