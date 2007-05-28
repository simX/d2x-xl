/* $Id: menu.c, v 1.34 2003/11/18 01:08:07 btb Exp $ */
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
#include "inferno.h"
#include "game.h"
#include "ipx.h"
#include "gr.h"
#include "key.h"
#include "iff.h"
#include "u_mem.h"
#include "error.h"
#include "bm.h"
#include "screens.h"
#include "mono.h"
#include "joy.h"
#include "vecmat.h"
#include "effects.h"
#include "slew.h"
#include "gamemine.h"
#include "gamesave.h"
#include "palette.h"
#include "args.h"
#include "newdemo.h"
#include "timer.h"
#include "sounds.h"
#include "gameseq.h"
#include "text.h"
#include "gamefont.h"
#include "newmenu.h"
#include "piggy.h"
#include "network.h"
#include "netmenu.h"
#include "ipx.h"
#include "multi.h"
#include "scores.h"
#include "joydefs.h"
#include "modem.h"
#include "playsave.h"
#include "kconfig.h"
#include "titles.h"
#include "credits.h"
#include "texmap.h"
#include "polyobj.h"
#include "state.h"
#include "mission.h"
#include "songs.h"
#include "config.h"
#include "movie.h"
#include "gamepal.h"
#include "gauges.h"
#include "powerup.h"
#include "strutil.h"
#include "reorder.h"
#include "digi.h"
#include "ai.h"
#include "ogl_init.h"
#include "cameras.h"
#include "texmerge.h"
#include "render.h"
#include "ipx_udp.h"
#include "lightmap.h"
#include "autodl.h"
#include "tracker.h"
#include "particles.h"
#include "vers_id.h"
#include "input.h"
#include "collide.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

//------------------------------------------------------------------------------

//char *menu_difficulty_text[] = {"Trainee", "Rookie", "Hotshot", "Ace", "Insane"};
//char *menu_detail_text[] = {"Lowest", "Low", "Medium", "High", "Highest", "", "Custom..."};

#define MENU_NEW_GAME                   0
#define MENU_NEW_SP_GAME                1
#define MENU_GAME                       2 
#define MENU_LOAD_GAME                  3
#define MENU_SAVE_GAME                  4
#define MENU_MULTIPLAYER                5
#define MENU_CONFIG                     6
#define MENU_NEW_PLAYER                 7
#define MENU_DEMO_PLAY                  8
#define MENU_VIEW_SCORES                9
#define MENU_ORDER_INFO                 10
#define MENU_PLAY_MOVIE                 11
#define MENU_PLAY_SONG                  12
#define MENU_SHOW_CREDITS               13
#define MENU_QUIT                       14
#define MENU_EDITOR                     15
#define MENU_D2_MISSIONS					 16
#define MENU_D1_MISSIONS					 17
#define MENU_LOAD_LEVEL                 18
#define MENU_START_IPX_NETGAME          20
#define MENU_JOIN_IPX_NETGAME           21
#define MENU_REJOIN_NETGAME             22
#define MENU_DIFFICULTY                 23
#define MENU_START_SERIAL               24
#define MENU_HELP                       25
#define MENU_STOP_MODEM                 26
//#define MENU_START_TCP_NETGAME          26 // TCP/IP support was planned in Descent II, 
//#define MENU_JOIN_TCP_NETGAME           27 // but never realized.
#define MENU_START_APPLETALK_NETGAME    28
#define MENU_JOIN_APPLETALK_NETGAME     29
#define MENU_START_UDP_NETGAME          30 // UDP/IP support copied from d1x
#define MENU_JOIN_UDP_NETGAME           31
#define MENU_START_KALI_NETGAME         32 // Kali support copied from d1x
#define MENU_JOIN_KALI_NETGAME          33
#define MENU_START_MCAST4_NETGAME       34 // UDP/IP over multicast networks
#define MENU_JOIN_MCAST4_NETGAME        35
#define MENU_START_UDP_TRACKER_NETGAME  36 // UDP/IP support copied from d1x
#define MENU_JOIN_UDP_TRACKER_NETGAME   37

#define D2X_MENU_GAP 0

//------------------------------------------------------------------------------

static int	nFPSopt, nRSDopt, 
				nDiffOpt, nTranspOpt, nSBoostOpt, nCamFpsOpt, nPlrSmokeOpt,
				nFusionRampOpt, nLightRangeOpt, nRendQualOpt, nTexQualOpt, nGunColorOpt,
				nCamSpeedOpt, nSmokeDensOpt [4], nSmokeSizeOpt [4], nSmokeLifeOpt [4], 
				nUseSmokeOpt, nUseCamOpt, nMslTurnSpeedOpt, nSlowmoSpeedupOpt,
				nLightingMethodOpt, nShadowsOpt, nMaxLightsOpt, nOglMaxLightsOpt, 
				nShadowReachOpt, 
				nSyncSmokeSizes, nSmokeGrenadeOpt, nMaxSmokeGrenOpt;
#if DBG_SHADOWS
static int	optZPass, optShadowVolume, nShadowTestOpt;
#endif

static int fpsTable [16] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 125, 150, 175, 200, 250};

static char *pszTexQual [4];
static char *pszRendQual [5];
static char *pszAmount [5];
static char *pszSize [4];
static char *pszLife [3];

#if DBG_SHADOWS
extern int	bZPass, bFrontCap, bRearCap, bFrontFaces, bBackFaces, bShadowVolume, bShadowTest, 
				bWallShadows, bSWCulling;
#endif

//------------------------------------------------------------------------------

//ADD_MENU ("Start netgame...", MENU_START_NETGAME, -1);
//ADD_MENU ("Send net message...", MENU_SEND_NET_MESSAGE, -1);

//unused - extern int last_joyTime;               //last time the joystick was used
extern void SetFunctionMode (int);
extern int UDPGetMyAddress ();
extern unsigned char ipx_ServerAddress [10];

void SoundMenu ();
void MiscellaneousMenu ();

// Function Prototypes added after LINTING
void ExecMenuOption (int select);
void CustomDetailsMenu (void);
void NewGameMenu (void);
void MultiplayerMenu (void);
void IpxSetDriver (int ipx_driver);

//returns the number of demo files on the disk
int NDCountDemos ();

//------------------------------------------------------------------------------

dmi displayModeInfo [NUM_DISPLAY_MODES + 1] = {
	{SM (320,  200),  320,  200, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (640,  400),  640,  400, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (640,  480),  640,  480, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (800,  600),  800,  600, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (1024,  768), 1024,  768, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 	
	{SM (1152,  864), 1152,  864, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 	
	{SM (1280,  960), 1280,  960, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 	
	{SM (1280, 1024), 1280, 1024, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (1600, 1200), 1600, 1200, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	{SM (2048, 1536), 2048, 1536, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	//test>>>
	{SM (4096, 3072), 4096, 3072, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0}, 
	//<<<test
	{SM (720,  480), 1280,  768, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1280,  768), 1280,  768, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1280,  800), 1280,  800, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1280,  854), 1280,  854, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1360,  768), 1360,  768, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1400, 1050), 1400, 1050, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1440,  900), 1440,  900, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1440,  960), 1440,  960, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1680, 1050), 1680, 1050, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0}, 
	{SM (1920, 1200), 1920, 1200, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 1, 0},
	//placeholder for custom resolutions
	{              0,    0,    0, VR_NONE, VRF_COMPATIBLE_MENUS+VRF_ALLOW_COCKPIT, 0, 0} 
	};

WIN (extern int DD_Emulation);

// ------------------------------------------------------------------------

void AutoDemoMenuCheck (int nitems, tMenuItem * items, int *last_key, int citem)
{
	int curtime;

PrintVersionInfo ();
// Don't allow them to hit ESC in the main menu.
if (*last_key==KEY_ESC) 
	*last_key = 0;

if (gameStates.app.bAutoDemos) {
	curtime = TimerGetApproxSeconds ();
	if (((keydTime_when_last_pressed + i2f (/*2*/5)) < curtime)
#ifdef _DEBUG	
		&& !gameData.speedtest.bOn
#endif		
		) {
		int n_demos;

		n_demos = NDCountDemos ();

try_again:;

		if ((d_rand () % (n_demos+1)) == 0) {
				gameStates.video.nScreenMode = -1;
				PlayIntroMovie ();
				SongsPlaySong (SONG_TITLE, 1);
				*last_key = -3; //exit menu to force rebuild even if not going to game mode. -3 tells menu system not to restore
				SetScreenMode (SCREEN_MENU);
			}
		else {
			WIN (HideCursorW ());
			keydTime_when_last_pressed = curtime;                  // Reset timer so that disk won't thrash if no demos.
			NDStartPlayback (NULL);           // Randomly pick a file
			if (gameData.demo.nState == ND_STATE_PLAYBACK) {
				SetFunctionMode (FMODE_GAME);
				*last_key = -3; //exit menu to get into game mode. -3 tells menu system not to restore
				}
			else
				goto try_again;	//keep trying until we get a demo that works
			}
		}
	}
}

//------------------------------------------------------------------------------
//static int FirstTime = 1;
static int main_menu_choice = 0;

static int nD1Opt = -1, nD2Opt = -1;

//      Create the main menu.
void CreateMainMenu (tMenuItem *m, int *nMenuChoice, int *nCallerOptions)
{
	int     opt = 0;

#ifndef DEMO_ONLY
SetScreenMode (SCREEN_MENU);
#if 1
ADD_TEXT (opt, "", 0);
m [opt++].noscroll = 1;
ADD_TEXT (opt, "", 0);
m [opt++].noscroll = 1;
ADD_TEXT (opt, "", 0);
m [opt++].noscroll = 1;
#endif
ADD_MENU (opt, TXT_NEW_GAME1, KEY_N, HTX_MAIN_NEW);
nMenuChoice [opt++] = MENU_NEW_GAME;
if (!gameStates.app.bNostalgia) {
	ADD_MENU (opt, TXT_NEW_SPGAME, KEY_I, HTX_MAIN_SINGLE);
	nMenuChoice [opt++] = MENU_NEW_SP_GAME;
	}
ADD_MENU (opt, TXT_LOAD_GAME, KEY_L, HTX_MAIN_LOAD);
nMenuChoice [opt++] = MENU_LOAD_GAME;
ADD_MENU (opt, TXT_MULTIPLAYER_, KEY_M, HTX_MAIN_MULTI);
nMenuChoice [opt++] = MENU_MULTIPLAYER;
if (gameStates.app.bNostalgia)
	ADD_MENU (opt, TXT_OPTIONS_, KEY_O, HTX_MAIN_CONF);
else
	ADD_MENU (opt, TXT_CONFIGURE, KEY_O, HTX_MAIN_CONF);
nMenuChoice [opt++] = MENU_CONFIG;
ADD_MENU (opt, TXT_CHANGE_PILOTS, KEY_P, HTX_MAIN_PILOT);
nMenuChoice [opt++] = MENU_NEW_PLAYER;
ADD_MENU (opt, TXT_VIEW_DEMO, KEY_D, HTX_MAIN_DEMO);
nMenuChoice [opt++] = MENU_DEMO_PLAY;
ADD_MENU (opt, TXT_VIEW_SCORES, KEY_H, HTX_MAIN_SCORES);
nMenuChoice [opt++] = MENU_VIEW_SCORES;
if (CFExist ("orderd2.pcx", gameFolders.szDataDir, 0)) { // SHAREWARE
	ADD_MENU (opt, TXT_ORDERING_INFO, -1, NULL);
	nMenuChoice [opt++] = MENU_ORDER_INFO;
	}
ADD_MENU (opt, TXT_PLAY_MOVIES, KEY_V, HTX_MAIN_MOVIES);
nMenuChoice [opt++] = MENU_PLAY_MOVIE;
if (!gameStates.app.bNostalgia) {
	ADD_MENU (opt, TXT_PLAY_SONGS, KEY_S, HTX_MAIN_SONGS);
	nMenuChoice [opt++] = MENU_PLAY_SONG;
	}
ADD_MENU (opt, TXT_CREDITS, KEY_C, HTX_MAIN_CREDITS);
nMenuChoice [opt++] = MENU_SHOW_CREDITS;
#endif
ADD_MENU (opt, TXT_QUIT, KEY_Q, HTX_MAIN_QUIT);
nMenuChoice [opt++] = MENU_QUIT;
#ifdef _DEBUG
if (!(gameData.app.nGameMode & GM_MULTI))   {
	//m [opt].nType=NM_TYPE_TEXT;
	//m [opt++].text=" Debug options:";

//		ADD_MENU ("  Load level...", MENU_LOAD_LEVEL , KEY_N);
	#ifdef EDITOR
	ADD_MENU (opt, "  Editor", KEY_E, NULL);
	nMenuChoice [opt++] = MENU_EDITOR;
	#endif
}

//ADD_MENU ("  Play song", MENU_PLAY_SONG, -1);
#endif
#if 0
if (!gameStates.app.bNostalgia) {
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_CHECK (opt, TXT_PLAY_D2MISSIONS, (gameOpts->app.nVersionFilter & 2) != 0, KEY_2, HTX_MAIN_D2);
	nMenuChoice [nD2Opt = opt++] = MENU_D2_MISSIONS;
	ADD_CHECK (opt, TXT_PLAY_D1MISSIONS, (gameOpts->app.nVersionFilter & 1) != 0, KEY_1, HTX_MAIN_D1);
	nMenuChoice [nD1Opt = opt++] = MENU_D1_MISSIONS;
	}
#endif
*nCallerOptions = opt;
}

//------------------------------------------------------------------------------

extern unsigned char ipx_ServerAddress[10];
extern unsigned char ipx_LocalAddress[10];

//------------------------------------------------------------------------------
//returns number of item chosen
int CallMenu () 
{
	int nMenuChoice[25];
	tMenuItem m [25];
	int i, num_options = 0;

IpxClose ();
memset (nMenuChoice, 0, sizeof (nMenuChoice));
memset (m, 0, sizeof (m));
//LoadPalette (MENU_PALETTE, NULL, 0, 1, 0);		//get correct palette

if (!LOCALPLAYER.callsign[0]) {
	SelectPlayer ();
	return 0;
}

if ((gameData.app.nGameMode & GM_SERIAL) || (gameData.app.nGameMode & GM_MODEM)) {
	ExecMenuOption (MENU_START_SERIAL);
	return 0;
}

if (gameData.multiplayer.autoNG.bValid) {
	ExecMenuOption (MENU_MULTIPLAYER);
	return 0;
	}
LogErr ("launching main menu\n");
do {
	CreateMainMenu (m, nMenuChoice, &num_options); // may have to change, eg, maybe selected pilot and no save games.
	keydTime_when_last_pressed = TimerGetFixedSeconds ();                // .. 20 seconds from now!
	if (main_menu_choice < 0)
		main_menu_choice = 0;
	gameStates.menus.bDrawCopyright = 1;
	i = ExecMenu2 ("", NULL, num_options, m, AutoDemoMenuCheck, &main_menu_choice, Menu_pcx_name);
#if 0
	gameOpts->app.nVersionFilter = 0;
	if (gameStates.app.bNostalgia)
		gameOpts->app.nVersionFilter = 3;
	else {
		if (m [nD2Opt].value)
			gameOpts->app.nVersionFilter |= 2;
		if (m [nD1Opt].value)
			gameOpts->app.nVersionFilter |= 1;
		}
#endif
	WritePlayerFile ();
	if ((i > -1) && (nMenuChoice[main_menu_choice] <= MENU_QUIT))
		ExecMenuOption (nMenuChoice[main_menu_choice]);
} while (gameStates.app.nFunctionMode == FMODE_MENU);
if (gameStates.app.nFunctionMode == FMODE_GAME)
	GrPaletteFadeOut (NULL, 32, 0);
FlushInput ();
StopPlayerMovement ();
return main_menu_choice;
}

//------------------------------------------------------------------------------

static void PlayMenuMovie (void)
{
	int h, i, j;
	char **m, *ps;

i = GetNumMovieLibs ();
for (h = j = 0; j < i; j++)
	if (j != 2)	//skip robot movies
		h += GetNumMovies (j);
if (!h)
	return;
if (!(m = (char **) D2_ALLOC (h * sizeof (char **))))
	return;
for (i = j = 0; i < h; i++)
	if ((ps = CycleThroughMovies (i == 0, 0))) {
		if (j && !strcmp (ps, m [0]))
			break;
		m [j++] = ps;
		}
i = ExecMenuListBox (TXT_SELECT_MOVIE, j, m, 1, NULL);
if (i > -1) {
	SDL_ShowCursor (0);
	if (strstr (m [i], "intro"))
		InitSubTitles ("intro.tex");	
	else if (strstr (m [i], ENDMOVIE))
		InitSubTitles (ENDMOVIE ".tex");	
	PlayMovie (m [i], 1, 1, gameOpts->movies.bResize);
	SDL_ShowCursor (1);
	}
D2_FREE (m);
SongsPlayCurrentSong (1);
}

//------------------------------------------------------------------------------

static void PlayMenuSong (void)
{
	int h, i, j = 0;
	char * m [MAX_NUM_SONGS + 2];
	CFILE *fp;
	char	szSongTitles [2][14] = {"- Descent 2 -", "- Descent 1 -"};

m [j++] = szSongTitles [0];
for (i = 0; i < gameData.songs.nSongs; i++) {
	if ((fp = CFOpen ((char *) gameData.songs.info [i].filename, gameFolders.szDataDir, "rb", i >= gameData.songs.nD2Songs))) {
		CFClose (fp);
		if (i == gameData.songs.nD2Songs)
			m [j++] = szSongTitles [1];
		m [j++] = gameData.songs.info [i].filename;
		}
	}
for (;;) {
	h = ExecMenuListBox (TXT_SELECT_SONG, j, m, 1, NULL);
	if (h < 0)
		return;
	if (!strstr (m [h], ".hmp"))
		continue;
	for (i = 0; i < gameData.songs.nSongs; i++)
		if (gameData.songs.info [i].filename == m [h]) {
			SongsPlaySong (i, 0);
			return;
			}
	}
}

//------------------------------------------------------------------------------

extern void show_order_form (void);      // John didn't want this in inferno.h so I just externed it.

//returns flag, true means quit menu
void ExecMenuOption (int select) 
{
	int bGotGame = 0;

gameStates.multi.bUseTracker = 0;
switch (select) {
	case MENU_NEW_GAME:
		gameOpts->app.bSinglePlayer = 0;
		NewGameMenu ();
		break;
	case MENU_NEW_SP_GAME:
		gameOpts->app.bSinglePlayer = 1;
		NewGameMenu ();
		break;
	case MENU_GAME:
		break;
	case MENU_DEMO_PLAY:	{
		char demoPath [FILENAME_LEN], demoFile[FILENAME_LEN];
		sprintf (demoPath, "%s%s*.dem", gameFolders.szDemoDir, *gameFolders.szDemoDir ? "/" : ""); 
		if (ExecMenuFileSelector (TXT_SELECT_DEMO, demoPath, demoFile, 1))
			NDStartPlayback (demoFile);
		break;
	}
	case MENU_LOAD_GAME:
		if (!StateRestoreAll (0, 0, NULL))
			SetFunctionMode (FMODE_MENU);
		break;
	#ifdef EDITOR
	case MENU_EDITOR:
		SetFunctionMode (FMODE_EDITOR);
		InitCockpit ();
		break;
	#endif
	case MENU_VIEW_SCORES:
		GrPaletteFadeOut (NULL, 32, 0);
		scoresView (-1);
		break;
#if 0
	case MENU_ORDER_INFO:
		show_order_form ();
		break;
#endif
	case MENU_QUIT:
		#ifdef EDITOR
		if (!SafetyCheck ()) break;
		#endif
		GrPaletteFadeOut (NULL, 32, 0);
		SetFunctionMode (FMODE_EXIT);
		break;
	case MENU_NEW_PLAYER:
		gameStates.gfx.bOverride = 0;
		SelectPlayer ();               //1 == allow escape out of menu
		break;

	case MENU_PLAY_MOVIE:
		PlayMenuMovie ();
		break;

	case MENU_HELP:
		DoShowHelp ();
		break;

	case MENU_PLAY_SONG:    
		PlayMenuSong ();
		break;

#ifdef _DEBUG
	case MENU_LOAD_LEVEL: {
		tMenuItem m;
		char text[10]="";
		int nNewLevel;

		m.nType=NM_TYPE_INPUT; m.text_len = 10; m.text = text;
		ExecMenu (NULL, "Enter level to load", 1, &m, NULL, NULL);
		nNewLevel = atoi (m.text);
		if (nNewLevel!=0 && nNewLevel>=gameData.missions.nLastSecretLevel && nNewLevel<=gameData.missions.nLastLevel)  {
			GrPaletteFadeOut (NULL, 32, 0);
			StartNewGame (nNewLevel);
		}

		break;
	}
#endif //ifndef RELEASE

	//case MENU_START_TCP_NETGAME:
	//case MENU_JOIN_TCP_NETGAME:
	case MENU_START_UDP_TRACKER_NETGAME:
	case MENU_JOIN_UDP_TRACKER_NETGAME:
		if (gameStates.app.bNostalgia < 2)
			gameStates.multi.bUseTracker = 1;
	case MENU_START_UDP_NETGAME:
	case MENU_JOIN_UDP_NETGAME:
		if (gameStates.app.bNostalgia > 1)
			break;
	case MENU_START_IPX_NETGAME:
	case MENU_JOIN_IPX_NETGAME:
	case MENU_START_KALI_NETGAME:
	case MENU_JOIN_KALI_NETGAME:
	case MENU_START_MCAST4_NETGAME:
	case MENU_JOIN_MCAST4_NETGAME:
		gameOpts->app.bSinglePlayer = 0;
		LoadMission (gameData.missions.nLastMission);
//			WIN (ipx_create_read_thread ();
		if ((select == MENU_JOIN_UDP_NETGAME) ||
			 (select == MENU_JOIN_UDP_TRACKER_NETGAME)) {
			if (!(InitAutoNetGame () || NetworkGetIpAddr ())) {
				//ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_INVALID_IP);
				return;
				}
			}
		gameStates.multi.bServer = (select & 1) == 0;
		gameStates.app.bHaveExtraGameInfo [1] = gameStates.multi.bServer;
		switch (select & ~0x1) {
			case MENU_START_IPX_NETGAME: 
				gameStates.multi.nGameType = IPX_GAME;
				IpxSetDriver (IPX_DRIVER_IPX); 
				break;
			case MENU_START_UDP_TRACKER_NETGAME: 
			case MENU_START_UDP_NETGAME: 
				gameStates.multi.nGameType = UDP_GAME;
				IpxSetDriver (IPX_DRIVER_UDP); 
				if (select == MENU_START_UDP_TRACKER_NETGAME) {
					int n = ActiveTrackerCount (1);
					if (n < -2) {
						if (n == -4)
							ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_NO_TRACKERS);
						gameStates.multi.bUseTracker = 0;
						return;
						}
					}
				break;
			case MENU_START_KALI_NETGAME: 
				gameStates.multi.nGameType = IPX_GAME;
				IpxSetDriver (IPX_DRIVER_KALI); 
				break;
			case MENU_START_MCAST4_NETGAME: 
				gameStates.multi.nGameType = IPX_GAME;
				IpxSetDriver (IPX_DRIVER_MCAST4); 
				break;
			default: 
				Int3 ();
			}

		if ((select & 0x1) == 0) // MENU_START_*_NETGAME
			bGotGame = NetworkStartGame ();
		else // MENU_JOIN_*_NETGAME
			bGotGame = NetworkBrowseGames ();
		if (!bGotGame)
			IpxClose ();
		break;

#if 0
	case MENU_START_TCP_NETGAME:
	case MENU_JOIN_TCP_NETGAME:
		ExecMessageBox (TXT_SORRY, 1, TXT_OK, "Not available in shareware version!");
		// DoNewIPAddress ();
		break;
#endif //NETWORK

	case MENU_START_SERIAL:
		com_main_menu ();
		break;
	case MENU_MULTIPLAYER:
		MultiplayerMenu ();
		break;
	case MENU_CONFIG:
		ConfigMenu ();
		break;
	case MENU_SHOW_CREDITS:
		GrPaletteFadeOut (NULL, 32, 0);
		SongsStopAll ();
		ShowCredits (NULL); 
		break;
	default:
		Error ("Unknown option %d in ExecMenuOption", select);
		break;
	}
}

//------------------------------------------------------------------------------

int DifficultyMenu ()
{
	int i, choice = gameStates.app.nDifficultyLevel;
	tMenuItem m [5];

memset (m, 0, sizeof (m));
for (i = 0; i < 5; i++)
	ADD_MENU (i, MENU_DIFFICULTY_TEXT (i), 0, "");
i = ExecMenu1 (NULL, TXT_DIFFICULTY_LEVEL, NDL, m, NULL, &choice);
if (i <= -1)
	return 0;
if (choice != gameStates.app.nDifficultyLevel) {       
	gameOpts->gameplay.nPlayerDifficultyLevel = choice;
	WritePlayerFile ();
	}
gameStates.app.nDifficultyLevel = gameOpts->gameplay.nPlayerDifficultyLevel;
return 1;
}

//------------------------------------------------------------------------------

ubyte   Render_depths[NUM_DETAIL_LEVELS-1] =                        {15, 31, 63, 127, 255};
sbyte   Max_perspective_depths[NUM_DETAIL_LEVELS-1] =               { 1,  2,  3,  5,  8};
sbyte   Max_linear_depths[NUM_DETAIL_LEVELS-1] =                    { 3,  5,  7, 10, 50};
sbyte   Max_linear_depthsObjects[NUM_DETAIL_LEVELS-1] =            { 1,  2,  3,  7, 20};
sbyte   Max_debrisObjects_list[NUM_DETAIL_LEVELS-1] =              { 2,  4,  7, 10, 15};
sbyte   MaxObjects_onscreen_detailed_list[NUM_DETAIL_LEVELS-1] =   { 2,  4,  7, 10, 15};
sbyte   Smts_list[NUM_DETAIL_LEVELS-1] =                            { 2,  4,  8, 16, 50};   //      threshold for models to go to lower detail model, gets multiplied by objP->size
sbyte   MaxSound_channels[NUM_DETAIL_LEVELS-1] =                   { 2,  4,  8, 12, 16};

//      -----------------------------------------------------------------------------
//      Set detail level based stuff.
//      Note: Highest detail level (gameStates.render.detail.nLevel == NUM_DETAIL_LEVELS-1) is custom detail level.
void InitDetailLevels (int nDetailLevel)
{
	Assert ((nDetailLevel >= 0) && (nDetailLevel < NUM_DETAIL_LEVELS));

if (nDetailLevel < NUM_DETAIL_LEVELS-1) {
	gameStates.render.detail.nRenderDepth = Render_depths[nDetailLevel];
	gameStates.render.detail.nMaxPerspectiveDepth = Max_perspective_depths[nDetailLevel];
	gameStates.render.detail.nMaxLinearDepth = Max_linear_depths[nDetailLevel];
	gameStates.render.detail.nMaxLinearDepthObjects = Max_linear_depthsObjects[nDetailLevel];
	gameStates.render.detail.nMaxDebrisObjects = Max_debrisObjects_list[nDetailLevel];
	gameStates.render.detail.nMaxObjectsOnScreenDetailed = MaxObjects_onscreen_detailed_list[nDetailLevel];
	gameData.models.nSimpleModelThresholdScale = Smts_list[nDetailLevel];
	DigiSetMaxChannels (MaxSound_channels[ nDetailLevel ]);
	//      Set custom menu defaults.
	gameStates.render.detail.nObjectComplexity = nDetailLevel;
	gameStates.render.detail.nWallRenderDepth = nDetailLevel;
	gameStates.render.detail.nObjectDetail = nDetailLevel;
	gameStates.render.detail.nWallDetail = nDetailLevel;
	gameStates.render.detail.nDebrisAmount = nDetailLevel;
	gameStates.sound.nMaxSoundChannels = nDetailLevel;
	gameStates.render.detail.nLevel = nDetailLevel;
	}
}

//      -----------------------------------------------------------------------------
void DetailLevelMenu (void)
{
	int i, choice = gameStates.app.nDetailLevel;
	tMenuItem m [8];

#if 1
	char szMenuDetails [5][20];

memset (m, 0, sizeof (m));
for (i = 0; i < 5; i++) {
	sprintf (szMenuDetails [i], "%d. %s", i + 1, MENU_DETAIL_TEXT (i));
	ADD_MENU (i, szMenuDetails [i], 0, HTX_ONLINE_MANUAL);
	}
#else
memset (m, 0, sizeof (m));
for (i = 0; i < 5; i++)
	ADD_MENU (i, MENU_DETAIL_TEXT (i), 0, HTX_ONLINE_MANUAL);
#endif
ADD_TEXT (5, "", 0);
ADD_MENU (6, MENU_DETAIL_TEXT (5), KEY_C, HTX_ONLINE_MANUAL);
ADD_CHECK (7, TXT_HIRES_MOVIES, gameOpts->movies.bHires, KEY_S, HTX_ONLINE_MANUAL);
i = ExecMenu1 (NULL, TXT_DETAIL_LEVEL , NDL+3, m, NULL, &choice);
if (i > -1) {
	switch (choice) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gameStates.app.nDetailLevel = choice;
			InitDetailLevels (gameStates.app.nDetailLevel);
			break;
		case 6:
			gameStates.app.nDetailLevel = 5;
			CustomDetailsMenu ();
			break;
		}
	}
gameOpts->movies.bHires = m [7].value;
}

//      -----------------------------------------------------------------------------

void CustomDetailsCallback (int nitems, tMenuItem * items, int *last_key, int citem)
{
	nitems = nitems;
	*last_key = *last_key;
	citem = citem;

gameStates.render.detail.nObjectComplexity = items[0].value;
gameStates.render.detail.nObjectDetail = items[1].value;
gameStates.render.detail.nWallDetail = items[2].value;
gameStates.render.detail.nWallRenderDepth = items[3].value;
gameStates.render.detail.nDebrisAmount = items[4].value;
gameStates.sound.nMaxSoundChannels = items[5].value;
}

// -----------------------------------------------------------------------------

void InitCustomDetails (void)
{
gameStates.render.detail.nRenderDepth = Render_depths[gameStates.render.detail.nWallRenderDepth];
gameStates.render.detail.nMaxPerspectiveDepth = Max_perspective_depths[gameStates.render.detail.nWallDetail];
gameStates.render.detail.nMaxLinearDepth = Max_linear_depths[gameStates.render.detail.nWallDetail];
gameStates.render.detail.nMaxDebrisObjects = Max_debrisObjects_list[gameStates.render.detail.nDebrisAmount];
gameStates.render.detail.nMaxObjectsOnScreenDetailed = MaxObjects_onscreen_detailed_list[gameStates.render.detail.nObjectComplexity];
gameData.models.nSimpleModelThresholdScale = Smts_list[gameStates.render.detail.nObjectComplexity];
gameStates.render.detail.nMaxLinearDepthObjects = Max_linear_depthsObjects[gameStates.render.detail.nObjectDetail];
DigiSetMaxChannels (MaxSound_channels[gameStates.sound.nMaxSoundChannels ]);
}

#define	DL_MAX	10

// -----------------------------------------------------------------------------

void CustomDetailsMenu (void)
{
	int	opt;
	int	i, choice = 0;
	tMenuItem m [DL_MAX];

do {
	opt = 0;
	memset (m, 0, sizeof (m));
	ADD_SLIDER (opt, TXT_OBJ_COMPLEXITY, gameStates.render.detail.nObjectComplexity, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_SLIDER (opt, TXT_OBJ_DETAIL, gameStates.render.detail.nObjectDetail, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_SLIDER (opt, TXT_WALL_DETAIL, gameStates.render.detail.nWallDetail, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_SLIDER (opt, TXT_WALL_RENDER_DEPTH, gameStates.render.detail.nWallRenderDepth, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_SLIDER (opt, TXT_DEBRIS_AMOUNT, gameStates.render.detail.nDebrisAmount, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_SLIDER (opt, TXT_SOUND_CHANNELS, gameStates.sound.nMaxSoundChannels, 0, NDL-1, 0, HTX_ONLINE_MANUAL);
	opt++;
	ADD_TEXT (opt, TXT_LO_HI, 0);
	opt++;

	Assert (opt <= sizeof (m) / sizeof (m [0]));

	i = ExecMenu1 (NULL, TXT_DETAIL_CUSTOM, opt, m, CustomDetailsCallback, &choice);
} while (i > -1);
InitCustomDetails ();
}

//------------------------------------------------------------------------------

int SetCustomDisplayMode (int w, int h)
{
displayModeInfo [NUM_DISPLAY_MODES].VGA_mode = SM (w, h);
displayModeInfo [NUM_DISPLAY_MODES].w = w;
displayModeInfo [NUM_DISPLAY_MODES].h = h;
if (!(displayModeInfo [NUM_DISPLAY_MODES].isAvailable = 
	   GrVideoModeOK (displayModeInfo [NUM_DISPLAY_MODES].VGA_mode)))
	return 0;
SetDisplayMode (NUM_DISPLAY_MODES, 0);
return 1;
}

//------------------------------------------------------------------------------

int GetDisplayMode (int mode)
{
	int h, i;

for (i = 0, h = NUM_DISPLAY_MODES; i < h; i++)
	if (mode == displayModeInfo [i].VGA_mode)
		return i;
return -1;
}

//------------------------------------------------------------------------------

#if VR_NONE
#   undef VR_NONE			//undef if != 0
#endif
#ifndef VR_NONE
#   define VR_NONE 0		//make sure VR_NONE is defined and 0 here
#endif

void SetDisplayMode (int mode, int bOverride)
{
	dmi *dmi;

if ((gameStates.video.nDisplayMode == -1) || (gameStates.render.vr.nRenderMode != VR_NONE))	//special VR mode
	return;								//...don't change
if (bOverride && gameStates.gfx.bOverride)
	mode = gameStates.gfx.nStartScrSize;
else
	gameStates.gfx.bOverride = 0;
if (!gameStates.menus.bHiresAvailable && (mode != 1))
	mode = 0;
if (!GrVideoModeOK (displayModeInfo [mode].VGA_mode))		//can't do mode
	mode = 0;
gameStates.video.nDisplayMode = mode;
dmi = displayModeInfo + mode;
if (gameStates.video.nDisplayMode != -1) {
	GameInitRenderBuffers (dmi->VGA_mode, dmi->w, dmi->h, dmi->render_method, dmi->flags);
	gameStates.video.nDefaultDisplayMode = gameStates.video.nDisplayMode;
	}
gameStates.video.nScreenMode = -1;		//force screen reset
}

//------------------------------------------------------------------------------

static int wideScreenOpt, optCustRes, nDisplayMode;

static int ScreenResMenuItemToMode (int menuItem)
{
	int j;

if ((wideScreenOpt >= 0) && (menuItem > wideScreenOpt))
	menuItem--;
for (j = 0; j < NUM_DISPLAY_MODES; j++)
	if (displayModeInfo[j].isAvailable)
		if (--menuItem < 0)
			break;
return j;
}

//------------------------------------------------------------------------------

static int ScreenResModeToMenuItem(int mode)
{
	int j;
	int item = 0;

	for (j = 0; j < mode; j++)
		if (displayModeInfo[j].isAvailable)
			item++;

	if ((wideScreenOpt >= 0) && (mode >= wideScreenOpt))
		item++;

	return item;
}

//------------------------------------------------------------------------------

void ScreenResCallback (int nItems, tMenuItem *m, int *last_key, int citem)
{
	int	i, j;

if (m [optCustRes].value != (nDisplayMode == NUM_DISPLAY_MODES)) 
	*last_key = -2;
for (i = 0; i < optCustRes; i++)
	if (m [i].value) {
		j = ScreenResMenuItemToMode(i);
		if ((j < NUM_DISPLAY_MODES) && (j != nDisplayMode)) {
			SetDisplayMode (j, 0);
			nDisplayMode = gameStates.video.nDisplayMode;
			*last_key = -2;
			}
		break;
		}
}

//------------------------------------------------------------------------------

int SwitchDisplayMode (int dir)
{
	int	i, h = NUM_DISPLAY_MODES;

for (i = 0; i < h; i++)
	displayModeInfo [i].isAvailable =
		 ((i < 2) || gameStates.menus.bHiresAvailable) && GrVideoModeOK (displayModeInfo [i].VGA_mode);
i = gameStates.video.nDisplayMode;
do {
	i += dir;
	if (i < 0)
		i = 0;
	else if (i >= h)
		i = h - 1;
	if (displayModeInfo [i].isAvailable) {
		SetDisplayMode (i, 0);
		return 1;
		}
	} while (i && (i < h - 1) && (i != gameStates.video.nDisplayMode));
return 0;
}

//------------------------------------------------------------------------------

#if VR_NONE
#   undef VR_NONE			//undef if != 0
#endif
#ifndef VR_NONE
#   define VR_NONE 0		//make sure VR_NONE is defined and 0 here
#endif

#define ADD_RES_OPT(_t) {ADD_RADIO (opt, _t, 0, -1, 0, NULL); opt++;}
//{m [opt].nType = NM_TYPE_RADIO; m [opt].text = (_t); m [opt].key = -1; m [opt].value = 0; opt++;}

extern int nCurrentVGAMode;

void ScreenResMenu ()
{
#	define N_SCREENRES_ITEMS (NUM_DISPLAY_MODES + 4)

	tMenuItem	m [N_SCREENRES_ITEMS];
	int				choice;
	int				i, j, key, opt = 0, nCustWOpt, nCustHOpt, nCustW, nCustH, bStdRes;
	char				szMode [NUM_DISPLAY_MODES][20];
	char				cShortCut, szCustX [5], szCustY [5];

if ((gameStates.video.nDisplayMode == -1) || (gameStates.render.vr.nRenderMode != VR_NONE)) {				//special VR mode
	ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, 
			"You may not change screen\nresolution when VR modes enabled.");
	return;
	}
nDisplayMode = gameStates.video.nDisplayMode;
do {
	wideScreenOpt = -1;
	cShortCut = '1';
	opt = 0;
	memset (m, 0, sizeof (m));
	for (i = 0, j = NUM_DISPLAY_MODES; i < j; i++) {
		if (!(displayModeInfo [i].isAvailable = 
				 ((i < 2) || gameStates.menus.bHiresAvailable) && GrVideoModeOK (displayModeInfo [i].VGA_mode)))
				continue;
		if (displayModeInfo [i].isWideScreen && !displayModeInfo [i-1].isWideScreen) {
			ADD_TEXT (opt, TXT_WIDESCREEN_RES, 0);
			if (wideScreenOpt < 0)
				wideScreenOpt = opt;
			opt++;
			}
		sprintf (szMode [i], "%c. %dx%d", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		if (cShortCut == '9')
			cShortCut = 'A';
		else
			cShortCut++;
		ADD_RES_OPT (szMode [i]);
		}
	ADD_RADIO (opt, TXT_CUSTOM_SCRRES, 0, -1, 0, HTX_CUSTOM_SCRRES);
	optCustRes = opt++;
	*szCustX = *szCustY = '\0';
	if (displayModeInfo [NUM_DISPLAY_MODES].w)
		sprintf (szCustX, "%d", displayModeInfo [NUM_DISPLAY_MODES].w);
	else
		*szCustX = '\0';
	if (displayModeInfo [NUM_DISPLAY_MODES].h)
		sprintf (szCustY, "%d", displayModeInfo [NUM_DISPLAY_MODES].h);
	else
		*szCustY = '\0';
	//if (nDisplayMode == NUM_DISPLAY_MODES) 
		{
		ADD_INPUT (opt, szCustX, 4, NULL);
		nCustWOpt = opt++;
		ADD_INPUT (opt, szCustY, 4, NULL);
		nCustHOpt = opt++;
		}
/*
	else
		nCustWOpt =
		nCustHOpt = -1;
*/
	choice = ScreenResModeToMenuItem(nDisplayMode);
	m [choice].value = 1;

	key = ExecMenu1 (NULL, TXT_SELECT_SCRMODE, opt, m, ScreenResCallback, &choice);
	if (key == -1)
		return;
	bStdRes = 0;
	if (m [optCustRes].value) {
		key = -2;
		nDisplayMode = NUM_DISPLAY_MODES;
		if ((nCustWOpt > 0) && (nCustHOpt > 0) &&
			 (0 < (nCustW = atoi (szCustX))) && (0 < (nCustH = atoi (szCustY)))) {
			i = NUM_DISPLAY_MODES;
			if (SetCustomDisplayMode (nCustW, nCustH))
				key = 0;
			else
				ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_ERROR_SCRMODE);
			}	
		else
			continue;
		}
	else {
		for (i = 0; i <= optCustRes; i++)
			if (m [i].value) {
				bStdRes = 1;
				i = ScreenResMenuItemToMode(i);
				break;
				}
			if (!bStdRes)
				continue;
		}
	if (((i > 1) && !gameStates.menus.bHiresAvailable) || !GrVideoModeOK (displayModeInfo [i].VGA_mode)) {
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_ERROR_SCRMODE);
		return;
		}
	if (i == gameStates.video.nDisplayMode) {
		SetDisplayMode (i, 0);
		SetScreenMode (SCREEN_MENU);
		if (bStdRes)
			return;
		}
	}
	while (key == -2);
}

//------------------------------------------------------------------------------

int SelectAndLoadMission (int bMulti, int *bAnarchyOnly)
{
	int	i, nMissions, nDefaultMission, nNewMission = -1;
	char	*szMsnNames [MAX_MISSIONS];

	static char* menuTitles [4];
	
	menuTitles [0] = TXT_NEW_GAME;
	menuTitles [1] = TXT_NEW_D1GAME;
	menuTitles [2] = TXT_NEW_D2GAME;
	menuTitles [3] = TXT_NEW_GAME;

if (bAnarchyOnly)
	*bAnarchyOnly = 0;
do {
	nMissions = BuildMissionList (1, nNewMission);
	if (nMissions < 1)
		return -1;
	nDefaultMission = 0;
	for (i = 0; i < nMissions; i++) {
		szMsnNames [i] = gameData.missions.list [i].szMissionName;
		if (!stricmp (szMsnNames [i], gameConfig.szLastMission))
			nDefaultMission = i;
		}
	gameStates.app.nExtGameStatus = bMulti ? GAMESTAT_START_MULTIPLAYER_MISSION : GAMESTAT_SELECT_MISSION;
	nNewMission = ExecMenuListBox1 (bMulti ? TXT_MULTI_MISSION : menuTitles [gameOpts->app.nVersionFilter], 
											  nMissions, szMsnNames, 1, nDefaultMission, NULL);
	GameFlushInputs ();
	if (nNewMission == -1)
		return -1;      //abort!
	} while (!gameData.missions.list [nNewMission].nDescentVersion);
strcpy (gameConfig.szLastMission, szMsnNames [nNewMission]);
if (!LoadMission (nNewMission)) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_MISSION_ERROR);
	return -1;
	}
gameStates.app.bD1Mission = (gameData.missions.list [nNewMission].nDescentVersion == 1);
gameData.missions.nLastMission = nNewMission;
if (bAnarchyOnly)
	*bAnarchyOnly = gameData.missions.list [nNewMission].bAnarchyOnly;
return nNewMission;
}

//------------------------------------------------------------------------------

void LegacyNewGameMenu (void)
{
	int			nNewLevel, nHighestPlayerLevel;
	int			nMissions;
	char			*m [MAX_MISSIONS];
	int			i, choice = 0, nFolder = -1, nDefaultMission = 0;
	static int	nMission = -1;
	static char	*menuTitles [4];
	
menuTitles [0] = TXT_NEW_GAME;
menuTitles [1] = TXT_NEW_D1GAME;
menuTitles [2] = TXT_NEW_D2GAME;
menuTitles [3] = TXT_NEW_GAME;

gameStates.app.bD1Mission = 0;
gameStates.app.bD1Data = 0;
SetDataVersion (-1);
if ((nMission < 0) || gameOpts->app.bSinglePlayer)
	gameFolders.szMsnSubFolder [0] = '\0';
CFUseAltHogFile ("");
do {
	nMissions = BuildMissionList (0, nFolder);
	if (nMissions < 1)
		return;
	for (i = 0; i < nMissions; i++) {
		m [i] = gameData.missions.list[i].szMissionName;
		if (!stricmp (m [i], gameConfig.szLastMission))
			nDefaultMission= i;
		}
	nMission = ExecMenuListBox1 (menuTitles [gameOpts->app.nVersionFilter], nMissions, m, 1, nDefaultMission, NULL);
	GameFlushInputs ();
	if (nMission == -1)
		return;         //abort!
	nFolder = nMission;
	}
while (!gameData.missions.list [nMission].nDescentVersion);
strcpy (gameConfig.szLastMission, m [nMission]);
if (!LoadMission (nMission)) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_ERROR_MSNFILE); 
	return;
}
gameStates.app.bD1Mission = (gameData.missions.list [nMission].nDescentVersion == 1);
gameData.missions.nLastMission = nMission;
nNewLevel = 1;

LogErr ("   getting highest level allowed to play\n");
nHighestPlayerLevel = GetHighestLevel ();

if (nHighestPlayerLevel > gameData.missions.nLastLevel)
	nHighestPlayerLevel = gameData.missions.nLastLevel;

if (nHighestPlayerLevel > 1) {
	tMenuItem m [4];
	char szInfo [80];
	char szNumber [10];
	int nItems;

try_again:
	sprintf (szInfo, "%s %d", TXT_START_ANY_LEVEL, nHighestPlayerLevel);

	memset (m, 0, sizeof (m));
	ADD_TEXT (0, szInfo, 0);
	ADD_INPUT (1, szNumber, 10, "");
	nItems = 2;

	strcpy (szNumber, "1");
	choice = ExecMenu (NULL, TXT_SELECT_START_LEV, nItems, m, NULL, NULL);
	if ((choice == -1) || !m [1].text [0])
		return;
	nNewLevel = atoi (m [1].text);
	if ((nNewLevel <= 0) || (nNewLevel > nHighestPlayerLevel)) {
		m [0].text = TXT_ENTER_TO_CONT;
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_INVALID_LEVEL); 
		goto try_again;
	}
}

gameStates.app.nDifficultyLevel = gameOpts->gameplay.nPlayerDifficultyLevel;
WritePlayerFile ();
if (!DifficultyMenu ())
	return;
GrPaletteFadeOut (NULL, 32, 0);
if (!StartNewGame (nNewLevel))
	SetFunctionMode (FMODE_MENU);
}

//------------------------------------------------------------------------------

static int nOptVerFilter = -1;

void NewGameMenuCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			i, v;

m = menus + nDiffOpt;
v = m->value;
if (gameStates.app.nDifficultyLevel != v) {
	gameStates.app.nDifficultyLevel = 
	gameOpts->gameplay.nPlayerDifficultyLevel = v;
	InitGateIntervals ();
	sprintf (m->text, TXT_DIFFICULTY2, MENU_DIFFICULTY_TEXT (gameOpts->gameplay.nPlayerDifficultyLevel));
	m->rebuild = 1;
	}
for (i = 0; i < 3; i++)
	if (menus [nOptVerFilter + i].value) {
		gameOpts->app.nVersionFilter = i + 1;
		break;
		}
}

//------------------------------------------------------------------------------

void NewGameMenu ()
{
	tMenuItem		m [15];
	int				opt, optSelMsn, optMsnName, optLevelText, optLevel, optLaunch;
	int				nMission = gameData.missions.nLastMission, bMsnLoaded = 0;
	int				i, choice = 0;
	char				szDifficulty [50];
	char				szLevelText [32];
	char				szLevel [5];

	static int		nPlayerMaxLevel = 1;
	static int		nLevel = 1;
	
if (gameStates.app.bNostalgia) {
	LegacyNewGameMenu ();
	return;
	}
gameStates.app.bD1Mission = 0;
gameStates.app.bD1Data = 0;
SetDataVersion (-1);
if ((nMission < 0) || gameOpts->app.bSinglePlayer)
	gameFolders.szMsnSubFolder [0] = '\0';
CFUseAltHogFile ("");
for (;;) {
	memset (m, 0, sizeof (m));
	opt = 0;

	ADD_MENU (opt, TXT_SEL_MISSION, KEY_I, HTX_MULTI_MISSION);
	optSelMsn = opt++;
	ADD_TEXT (opt, (nMission < 0) ? TXT_NONE_SELECTED : gameData.missions.list [nMission].szMissionName, 0);	
	optMsnName = opt++;
	if ((nMission >= 0) && (nPlayerMaxLevel > 1)) {
#if 0
		ADD_TEXT (opt, "", 0);
		opt++;
#endif
		sprintf (szLevelText, "%s (1-%d)", TXT_LEVEL_, nPlayerMaxLevel);
		Assert (strlen (szLevelText) < 32);
		ADD_TEXT (opt, szLevelText, 0); 
		m [opt].rebuild = 1;
		optLevelText = opt++;
		sprintf (szLevel, "%d", nLevel);
		ADD_INPUT (opt, szLevel, 4, HTX_MULTI_LEVEL);
		optLevel = opt++;
		}
	else
		optLevel = -1;
	ADD_TEXT (opt, "                              ", 0);
	opt++;
	sprintf (szDifficulty + 1, TXT_DIFFICULTY2, MENU_DIFFICULTY_TEXT (gameStates.app.nDifficultyLevel));
	*szDifficulty = *(TXT_DIFFICULTY2 - 1);
	ADD_SLIDER (opt, szDifficulty + 1, gameStates.app.nDifficultyLevel, 0, 4, KEY_D, HTX_GPLAY_DIFFICULTY);
	nDiffOpt = opt++;
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_RADIO (opt, TXT_PLAY_D1MISSIONS, 0, KEY_1, 1, HTX_LEVEL_VERSION_FILTER);
	nOptVerFilter = opt++;
	ADD_RADIO (opt, TXT_PLAY_D2MISSIONS, 0, KEY_2, 1, HTX_LEVEL_VERSION_FILTER);
	opt++;
	ADD_RADIO (opt, TXT_PLAY_ALL_MISSIONS, 0, KEY_A, 1, HTX_LEVEL_VERSION_FILTER);
	opt++;
	m [nOptVerFilter + gameOpts->app.nVersionFilter - 1].value = 1;
	if (nMission >= 0) {
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_MENU (opt, TXT_LAUNCH_GAME, KEY_L, "");
		m [opt].centered = 1;
		optLaunch = opt++;
		}
	else
		optLaunch = -1;

	Assert (opt <= sizeofa (m));
	i = ExecMenu1 (NULL, TXT_NEWGAME_MENUTITLE, opt, m, &NewGameMenuCallback, &choice);
	if (i < 0) {
		SetFunctionMode (FMODE_MENU);
		return;
		}
	if (choice == optSelMsn) {
		i = SelectAndLoadMission (0, NULL);
		if (i >= 0) {
			bMsnLoaded = 1;
			nMission = i;
			nLevel = 1;
			LogErr ("   getting highest level allowed to play\n");
			nPlayerMaxLevel = GetHighestLevel ();
			if (nPlayerMaxLevel > gameData.missions.nLastLevel)
				nPlayerMaxLevel = gameData.missions.nLastLevel;
			}
		}
	else if (choice == optLevel) {
		i = atoi (m [optLevel].text);
		if ((i <= 0) || (i > nPlayerMaxLevel))
			ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_INVALID_LEVEL); 
		else if (nLevel == i)
			break;
		else
			nLevel = i;
		}
	else if (nMission >= 0)
		break;
	}

i = m [nDiffOpt].value;
if (gameOpts->gameplay.nPlayerDifficultyLevel != i) {
	gameOpts->gameplay.nPlayerDifficultyLevel = i;
	gameStates.app.nDifficultyLevel = i;
	InitGateIntervals ();
	}
WritePlayerFile ();
if (optLevel > 0)
	nLevel = atoi (m [optLevel].text);
GrPaletteFadeOut (NULL, 32, 0);
if (!bMsnLoaded)
	LoadMission (nMission);
if (!StartNewGame (nLevel))
	SetFunctionMode (FMODE_MENU);
}

//------------------------------------------------------------------------------

extern void GameLoop (int, int);

static int optBrightness = -1;
static int optContrast = -1;

//------------------------------------------------------------------------------

void options_menuset (int nitems, tMenuItem * items, int *last_key, int citem)
{
if (gameStates.app.bNostalgia) {
	if (citem == optBrightness)
		GrSetPaletteGamma (items [optBrightness].value);
	}
nitems++;		//kill warning
last_key++;		//kill warning
}

//------------------------------------------------------------------------------

static int	nCWSopt, nCWZopt, optTextGauges, optWeaponIcons, bShowWeaponIcons, 
				optIconAlpha, optTgtInd, optDmgInd, optHitInd, optMslLockInd;

static char *szCWS [4];

//------------------------------------------------------------------------------

void TgtIndOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v, j;

m = menus + optTgtInd;
v = m->value;
if (v != (extraGameInfo [0].bTargetIndicators == 0)) {
	for (j = 0; j < 3; j++)
		if (m [optTgtInd + j].value) {
			extraGameInfo [0].bTargetIndicators = j;
			break;
			}
	*key = -2;
	return;
	}
m = menus + optDmgInd;
v = m->value;
if (v != extraGameInfo [0].bDamageIndicators) {
	extraGameInfo [0].bDamageIndicators = v;
	*key = -2;
	return;
	}
m = menus + optMslLockInd;
v = m->value;
if (v != extraGameInfo [0].bMslLockIndicators) {
	extraGameInfo [0].bMslLockIndicators = v;
	*key = -2;
	return;
	}
}

//------------------------------------------------------------------------------

void TgtIndOptionsMenu ()
{
	tMenuItem m [15];
	int	i, j, opt, choice = 0;
	int	optCloakedInd, optRotateInd;

do {
	memset (m, 0, sizeof (m));
	opt = 0;

	ADD_RADIO (opt, TXT_TGTIND_NONE, 0, KEY_A, 1, HTX_CPIT_TGTIND);
	optTgtInd = opt++;
	ADD_RADIO (opt, TXT_TGTIND_SQUARE, 0, KEY_R, 1, HTX_CPIT_TGTIND);
	opt++;
	ADD_RADIO (opt, TXT_TGTIND_TRIANGLE, 0, KEY_T, 1, HTX_CPIT_TGTIND);
	opt++;
	m [optTgtInd + extraGameInfo [0].bTargetIndicators].value = 1;
	if (extraGameInfo [0].bTargetIndicators) {
		ADD_CHECK (opt, TXT_CLOAKED_INDICATOR, extraGameInfo [0].bCloakedIndicators, KEY_C, HTX_CLOAKED_INDICATOR);
		optCloakedInd = opt++;
		}
	else
		optCloakedInd = -1;
	ADD_CHECK (opt, TXT_DMG_INDICATOR, extraGameInfo [0].bDamageIndicators, KEY_D, HTX_CPIT_DMGIND);
	optDmgInd = opt++;
	if (extraGameInfo [0].bTargetIndicators || extraGameInfo [0].bDamageIndicators) {
		ADD_CHECK (opt, TXT_HIT_INDICATOR, extraGameInfo [0].bTagOnlyHitObjs, KEY_T, HTX_HIT_INDICATOR);
		optHitInd = opt++;
		}
	else
		optHitInd = -1;
	ADD_CHECK (opt, TXT_MSLLOCK_INDICATOR, extraGameInfo [0].bMslLockIndicators, KEY_M, HTX_CPIT_MSLLOCKIND);
	optMslLockInd = opt++;
	if (extraGameInfo [0].bMslLockIndicators) {
		ADD_CHECK (opt, TXT_ROTATE_MSLLOCKIND, gameOpts->render.cockpit.bRotateMslLockInd, KEY_R, HTX_ROTATE_MSLLOCKIND);
		optRotateInd = opt++;
		}
	else
		optRotateInd = -1;
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, TXT_TGTIND_MENUTITLE, opt, m, &TgtIndOptionsCallback, &choice);
	} while (i >= 0);
	if (optTgtInd >= 0) {
		for (j = 0; j < 3; j++)
			if (m [optTgtInd + j].value) {
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

//------------------------------------------------------------------------------

void WeaponIconOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + optWeaponIcons;
v = m->value;
if (v != bShowWeaponIcons) {
	bShowWeaponIcons = v;
	*key = -2;
	return;
	}
}

//------------------------------------------------------------------------------

void WeaponIconOptionsMenu (void)
{
	tMenuItem m [35];
	int	i, j, opt, choice = 0;
	int	optSmallIcons, optIconSort, optIconAmmo, optIconPos, optEquipIcons;

bShowWeaponIcons = (extraGameInfo [0].nWeaponIcons != 0);
do {
	memset (m, 0, sizeof (m));
	opt = 0;

	ADD_CHECK (opt, TXT_SHOW_WEAPONICONS, bShowWeaponIcons, KEY_W, HTX_CPIT_WPNICONS);
	optWeaponIcons = opt++;
	if (bShowWeaponIcons && gameOpts->app.bExpertMode) {
		ADD_CHECK (opt, TXT_SHOW_EQUIPICONS, gameOpts->render.weaponIcons.bEquipment, KEY_Q, HTX_CPIT_EQUIPICONS);
		optEquipIcons = opt++;
		ADD_CHECK (opt, TXT_SMALL_WPNICONS, gameOpts->render.weaponIcons.bSmall, KEY_I, HTX_CPIT_SMALLICONS);
		optSmallIcons = opt++;
		ADD_CHECK (opt, TXT_SORT_WPNICONS, gameOpts->render.weaponIcons.nSort, KEY_T, HTX_CPIT_SORTICONS);
		optIconSort = opt++;
		ADD_CHECK (opt, TXT_AMMO_WPNICONS, gameOpts->render.weaponIcons.bShowAmmo, KEY_A, HTX_CPIT_ICONAMMO);
		optIconAmmo = opt++;
		optIconPos = opt;
		ADD_RADIO (opt, TXT_WPNICONS_TOP, 0, KEY_I, 3, HTX_CPIT_ICONPOS);
		opt++;
		ADD_RADIO (opt, TXT_WPNICONS_BTM, 0, KEY_I, 3, HTX_CPIT_ICONPOS);
		opt++;
		ADD_RADIO (opt, TXT_WPNICONS_LRB, 0, KEY_I, 3, HTX_CPIT_ICONPOS);
		opt++;
		ADD_RADIO (opt, TXT_WPNICONS_LRT, 0, KEY_I, 3, HTX_CPIT_ICONPOS);
		opt++;
		m [optIconPos + NMCLAMP (extraGameInfo [0].nWeaponIcons - 1, 0, 3)].value = 1;
		ADD_SLIDER (opt, TXT_ICON_DIM, gameOpts->render.weaponIcons.alpha, 0, 8, KEY_D, HTX_CPIT_ICONDIM);
		optIconAlpha = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	else
		optEquipIcons =
		optSmallIcons =
		optIconSort =
		optIconPos =
		optIconAmmo = 
		optIconAlpha = -1;
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, TXT_WPNICON_MENUTITLE, opt, m, &WeaponIconOptionsCallback, &choice);
	} while (i >= 0);
	if (bShowWeaponIcons) {
		if (gameOpts->app.bExpertMode) {
			GET_VAL (gameOpts->render.weaponIcons.bEquipment, optEquipIcons);
			GET_VAL (gameOpts->render.weaponIcons.bSmall, optSmallIcons);
			GET_VAL (gameOpts->render.weaponIcons.nSort, optIconSort);
			GET_VAL (gameOpts->render.weaponIcons.bShowAmmo, optIconAmmo);
			if (optIconPos >= 0)
				for (j = 0; j < 4; j++)
					if (m [optIconPos + j].value) {
						extraGameInfo [0].nWeaponIcons = j + 1;
						break;
						}
			GET_VAL (gameOpts->render.weaponIcons.alpha, optIconAlpha);
			}
		else {
#if EXPMODE_DEFAULTS
			gameOpts->render.weaponIcons.bEquipment = 1;
			gameOpts->render.weaponIcons.bSmall = 1;
			gameOpts->render.weaponIcons.nSort = 1;
			gameOpts->render.weaponIcons.bShowAmmo = 1;
			gameOpts->render.weaponIcons.alpha = 3;
#endif
			}
		}
	else
		extraGameInfo [0].nWeaponIcons = 0;
	} while (i == -2);
}

//------------------------------------------------------------------------------

void GaugeOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + optTextGauges;
v = !m->value;
if (v != gameOpts->render.cockpit.bTextGauges) {
	gameOpts->render.cockpit.bTextGauges = v;
	*key = -2;
	return;
	}
}

//------------------------------------------------------------------------------

void GaugeOptionsMenu (void)
{
	tMenuItem m [10];
	int	i, opt, choice = 0;
	int	optScaleGauges, optFlashGauges, optShieldWarn, optObjectTally, optPlayerStats;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_SHOW_GFXGAUGES, !gameOpts->render.cockpit.bTextGauges, KEY_P, HTX_CPIT_GFXGAUGES);
	optTextGauges = opt++;
	if (!gameOpts->render.cockpit.bTextGauges && gameOpts->app.bExpertMode) {
		ADD_CHECK (opt, TXT_SCALE_GAUGES, gameOpts->render.cockpit.bScaleGauges, KEY_C, HTX_CPIT_SCALEGAUGES);
		optScaleGauges = opt++;
		ADD_CHECK (opt, TXT_FLASH_GAUGES, gameOpts->render.cockpit.bFlashGauges, KEY_F, HTX_CPIT_FLASHGAUGES);
		optFlashGauges = opt++;
		}
	else
		optScaleGauges =
		optFlashGauges = -1;
	ADD_CHECK (opt, TXT_SHIELD_WARNING, gameOpts->gameplay.bShieldWarning, KEY_W, HTX_CPIT_SHIELDWARN);
	optShieldWarn = opt++;
	ADD_CHECK (opt, TXT_OBJECT_TALLY, gameOpts->render.cockpit.bObjectTally, KEY_T, HTX_CPIT_OBJTALLY);
	optObjectTally = opt++;
	ADD_CHECK (opt, TXT_PLAYER_STATS, gameOpts->render.cockpit.bPlayerStats, KEY_S, HTX_CPIT_PLAYERSTATS);
	optPlayerStats = opt++;
	do {
		i = ExecMenu1 (NULL, TXT_GAUGES_MENUTITLE, opt, m, &GaugeOptionsCallback, &choice);
	} while (i >= 0);
	if (!(gameOpts->render.cockpit.bTextGauges = !m [optTextGauges].value)) {
		if (gameOpts->app.bExpertMode) {
			GET_VAL (gameOpts->render.cockpit.bScaleGauges, optScaleGauges);
			GET_VAL (gameOpts->render.cockpit.bFlashGauges, optFlashGauges);
			GET_VAL (gameOpts->gameplay.bShieldWarning, optShieldWarn);
			GET_VAL (gameOpts->render.cockpit.bObjectTally, optObjectTally);
			GET_VAL (gameOpts->render.cockpit.bPlayerStats, optPlayerStats);
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

//------------------------------------------------------------------------------

void CockpitOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

if (gameOpts->app.bExpertMode) {
	m = menus + nCWSopt;
	v = m->value;
	if (gameOpts->render.cockpit.nWindowSize != v) {
		gameOpts->render.cockpit.nWindowSize = v;
		m->text = szCWS [v];
		m->rebuild = 1;
		}

	m = menus + nCWZopt;
	v = m->value;
	if (gameOpts->render.cockpit.nWindowZoom != v) {
		gameOpts->render.cockpit.nWindowZoom = v;
		sprintf (m->text, TXT_CW_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
		m->rebuild = 1;
		}
	}
}

//------------------------------------------------------------------------------

void CockpitOptionsMenu (void)
{
	tMenuItem m [25];
	int	i, opt, choice = 0;
	int	optGauges, optHUD, optReticle, optGuided, 
			optMissileView, optMouseInd, optSplitMsgs, optHUDMsgs, optTgtInd, optWeaponIcons;

	char szCockpitWindowZoom [40];

szCWS [0] = TXT_CWS_SMALL;
szCWS [1] = TXT_CWS_MEDIUM;
szCWS [2] = TXT_CWS_LARGE;
szCWS [3] = TXT_CWS_HUGE;
nCWSopt = nCWZopt = optTextGauges = optWeaponIcons = optIconAlpha = -1;
bShowWeaponIcons = (extraGameInfo [0].nWeaponIcons != 0);
do {
	memset (m, 0, sizeof (m));
	opt = 0;

	if (gameOpts->app.bExpertMode) {
		ADD_SLIDER (opt, szCWS [gameOpts->render.cockpit.nWindowSize], gameOpts->render.cockpit.nWindowSize, 0, 3, KEY_S, HTX_CPIT_WINSIZE);
		nCWSopt = opt++;
		sprintf (szCockpitWindowZoom, TXT_CW_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
		ADD_SLIDER (opt, szCockpitWindowZoom, gameOpts->render.cockpit.nWindowZoom, 0, 3, KEY_Z, HTX_CPIT_WINZOOM);
		nCWZopt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_SHOW_HUD, gameOpts->render.cockpit.bHUD, KEY_U, HTX_CPIT_SHOWHUD);
		optHUD = opt++;
		ADD_CHECK (opt, TXT_SHOW_HUDMSGS, gameOpts->render.cockpit.bHUDMsgs, KEY_M, HTX_CPIT_SHOWHUDMSGS);
		optHUDMsgs = opt++;
		ADD_CHECK (opt, TXT_SHOW_RETICLE, gameOpts->render.cockpit.bReticle, KEY_R, HTX_CPIT_SHOWRETICLE);
		optReticle = opt++;
		if (gameOpts->input.mouse.bJoystick) {
			ADD_CHECK (opt, TXT_SHOW_MOUSEIND, gameOpts->render.cockpit.bMouseIndicator, KEY_O, HTX_CPIT_MOUSEIND);
			optMouseInd = opt++;
			}
		else
			optMouseInd = -1;
		}
	else
		optHUD =
		optHUDMsgs =
		optMouseInd = 
		optReticle = -1;
	ADD_CHECK (opt, TXT_EXTRA_PLRMSGS, gameOpts->render.cockpit.bSplitHUDMsgs, KEY_P, HTX_CPIT_SPLITMSGS);
	optSplitMsgs = opt++;
	ADD_CHECK (opt, TXT_MISSILE_VIEW, gameOpts->render.cockpit.bMissileView, KEY_I, HTX_CPITMSLVIEW);
	optMissileView = opt++;
	ADD_CHECK (opt, TXT_GUIDED_MAINVIEW, gameOpts->render.cockpit.bGuidedInMainView, KEY_F, HTX_CPIT_GUIDEDVIEW);
	optGuided = opt++;
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_MENU (opt, TXT_TGTIND_MENUCALL, KEY_T, "");
	optTgtInd = opt++;
	ADD_MENU (opt, TXT_WPNICON_MENUCALL, KEY_W, "");
	optWeaponIcons = opt++;
	ADD_MENU (opt, TXT_GAUGES_MENUCALL, KEY_G, "");
	optGauges = opt++;
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, TXT_COCKPIT_OPTS, opt, m, &CockpitOptionsCallback, &choice);
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
	GET_VAL (gameOpts->render.cockpit.bHUDMsgs, optHUDMsgs);
	GET_VAL (gameOpts->render.cockpit.bSplitHUDMsgs, optSplitMsgs);
	} while (i == -2);
}

//------------------------------------------------------------------------------

static inline char *ContrastText (void)
{
return (gameStates.ogl.nContrast == 8) ? TXT_STANDARD : 
		 (gameStates.ogl.nContrast < 8) ? TXT_LOW : 
		 TXT_HIGH;
}

//------------------------------------------------------------------------------

int FindTableFps (int nFps)
{
	int	i, j = 0, d, dMin = 0x7fffffff;

for (i = 0; i < 16; i++) {
	d = abs (nFps - fpsTable [i]);
	if (d < dMin) {
		j = i;
		dMin = d;
		}
	}
return j;
}

//------------------------------------------------------------------------------

int nCoronaIntOpt = -1, nCoronasOpt = -1, nObjCoronasOpt = -1;

char *pszCoronaInt [3];

void EffectOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + nCoronasOpt;
v = m->value;
if (gameOpts->render.bCoronas != v) {
	gameOpts->render.bCoronas = v;
	*key = -2;
	}	
m = menus + nObjCoronasOpt;
v = m->value;
if (gameOpts->render.bObjectCoronas != v) {
	gameOpts->render.bObjectCoronas = v;
	*key = -2;
	}	
if (nCoronaIntOpt >= 0) {
	m = menus + nCoronaIntOpt;
	v = m->value;
	if (gameOpts->render.nCoronaIntensity != v) {
		gameOpts->render.nCoronaIntensity = v;
		sprintf (m->text, TXT_CORONA_INTENSITY, pszCoronaInt [v]);
		m->rebuild = -1;
		}
	}
}

//------------------------------------------------------------------------------

void EffectOptionsMenu ()
{
	tMenuItem m [20];
	int	i, choice = 0;
	int	opt;
	int	optTranspExpl, optThrustFlame, optRenderShields, optDmgExpl, optAutoTransp, optLightTrails, 
			optTracers, optShockwaves;
	char	szCoronaInt [50];

pszCoronaInt [0] = TXT_LOW;
pszCoronaInt [1] = TXT_MEDIUM;
pszCoronaInt [2] = TXT_HIGH;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_RENDER_CORONAS, gameOpts->render.bCoronas, KEY_C, HTX_ADVRND_CORONAS);
	nCoronasOpt = opt++;
	ADD_CHECK (opt, TXT_RENDER_OBJCORONAS, gameOpts->render.bObjectCoronas, KEY_O, HTX_ADVRND_OBJCORONAS);
	nObjCoronasOpt = opt++;
	if (gameOpts->render.bCoronas || gameOpts->render.bObjectCoronas) {
		sprintf (szCoronaInt + 1, TXT_CORONA_INTENSITY, pszCoronaInt [gameOpts->render.nCoronaIntensity]);
		*szCoronaInt = *(TXT_CORONA_INTENSITY - 1);
		ADD_SLIDER (opt, szCoronaInt + 1, gameOpts->render.nCoronaIntensity, 0, 2, KEY_I, HTX_CORONA_INTENSITY);
		nCoronaIntOpt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	else	
		nCoronaIntOpt = -1;
	ADD_CHECK (opt, TXT_TRANSP_EFFECTS, gameOpts->render.bTransparentEffects, KEY_E, HTX_ADVRND_TRANSPFX);
	optTranspExpl = opt++;
	ADD_CHECK (opt, TXT_RENDER_SHKWAVES, extraGameInfo [0].bShockwaves, KEY_S, HTX_RENDER_SHKWAVES);
	optShockwaves = opt++;
	ADD_CHECK (opt, TXT_RENDER_LGTTRAILS, extraGameInfo [0].bLightTrails, KEY_L, HTX_RENDER_LGTTRAILS);
	optLightTrails = opt++;
	ADD_CHECK (opt, TXT_RENDER_TRACERS, extraGameInfo [0].bTracers, KEY_T, HTX_RENDER_TRACERS);
	optTracers = opt++;
	ADD_CHECK (opt, TXT_AUTO_TRANSPARENCY, gameOpts->render.bAutoTransparency, KEY_A, HTX_RENDER_AUTOTRANSP);
	optAutoTransp = opt++;
	ADD_CHECK (opt, TXT_DMG_EXPL, extraGameInfo [0].bDamageExplosions, KEY_X, HTX_RENDER_DMGEXPL);
	optDmgExpl = opt++;
	ADD_CHECK (opt, TXT_THRUSTER_FLAME, extraGameInfo [0].bThrusterFlames, KEY_F, HTX_RENDER_THRUSTER);
	optThrustFlame = opt++;
	ADD_CHECK (opt, TXT_RENDER_SHIELDS, extraGameInfo [0].bRenderShield, KEY_P, HTX_RENDER_SHIELDS);
	optRenderShields = opt++;
	Assert (opt <= sizeofa (m));
	for (;;) {
		i = ExecMenu1 (NULL, TXT_EFFECT_MENUTITLE, opt, m, EffectOptionsCallback, &choice);
		if (i < 0)
			break;
		} 
	gameOpts->render.bTransparentEffects = m [optTranspExpl].value;
	gameOpts->render.bAutoTransparency = m [optAutoTransp].value;
	gameOpts->render.bCoronas = m [nCoronasOpt].value;
	gameOpts->render.bObjectCoronas = m [nObjCoronasOpt].value;
	extraGameInfo [0].bLightTrails = m [optLightTrails].value;
	extraGameInfo [0].bTracers = m [optTracers].value;
	extraGameInfo [0].bShockwaves = m [optShockwaves].value;
	extraGameInfo [0].bDamageExplosions = m [optDmgExpl].value;
	extraGameInfo [0].bThrusterFlames = m [optThrustFlame].value;
	extraGameInfo [0].bRenderShield = m [optRenderShields].value;
#if EXPMODE_DEFAULTS
	if (!gameOpts->app.bExpertMode) {
		gameOpts->render.bTransparentEffects = 1;
	gameOpts->render.bAutoTransparency = 1;
	gameOpts->render.bCoronas = 0;
	gameOpts->render.bObjectCoronas = 0;
	extraGameInfo [0].bLightTrails = 1;
	extraGameInfo [0].bTracers = 1;
	extraGameInfo [0].bShockwaves = 1;
	extraGameInfo [0].bDamageExplosions = 1;
	extraGameInfo [0].bThrusterFlames = 1;
	extraGameInfo [0].bRenderShield = 1;
		}
#endif
	} while (i == -2);
SetDebrisCollisions ();
}

//------------------------------------------------------------------------------

static int nOptTextured, nOptRadar, nOptRadarRange;
static char *pszRadarRange [3];

void AutomapOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem * m;
	int			v;

m = menus + nOptTextured;
v = m->value;
if (v != gameOpts->render.automap.bTextured) {
	gameOpts->render.automap.bTextured = v;
	*key = -2;
	return;
	}
if (!m [nOptRadar + extraGameInfo [0].nRadar].value) {
	*key = -2;
	return;
	}
if (nOptRadarRange >= 0) {
	m = menus + nOptRadarRange;
	v = m->value;
	if (v != gameOpts->render.automap.nRange) {
		gameOpts->render.automap.nRange = v;
		sprintf (m->text, TXT_RADAR_RANGE, pszRadarRange [v]);
		m->rebuild = 1;
		}
	}
}

//------------------------------------------------------------------------------

void AutomapOptionsMenu ()
{
	tMenuItem m [20];
	int	i, j, choice = 0;
	int	opt;
	int	optBright, optShowRobots, optShowPowerups, optCoronas, optSmoke, optColor, optSkybox;
	char	szRadarRange [50];

pszRadarRange [0] = TXT_SHORT;
pszRadarRange [1] = TXT_MEDIUM;
pszRadarRange [2] = TXT_FAR;
*szRadarRange = '\0';
do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_AUTOMAP_TEXTURED, gameOpts->render.automap.bTextured, KEY_T, HTX_AUTOMAP_TEXTURED);
	nOptTextured = opt++;
	if (gameOpts->render.automap.bTextured) {
		ADD_CHECK (opt, TXT_AUTOMAP_BRIGHT, gameOpts->render.automap.bBright, KEY_B, HTX_AUTOMAP_BRIGHT);
		optBright = opt++;
		ADD_CHECK (opt, TXT_AUTOMAP_CORONAS, gameOpts->render.automap.bCoronas, KEY_C, HTX_AUTOMAP_CORONAS);
		optCoronas = opt++;
		ADD_CHECK (opt, TXT_AUTOMAP_SMOKE, gameOpts->render.automap.bSmoke, KEY_S, HTX_AUTOMAP_SMOKE);
		optSmoke = opt++;
		ADD_CHECK (opt, TXT_AUTOMAP_SKYBOX, gameOpts->render.automap.bSkybox, KEY_B, HTX_AUTOMAP_SKYBOX);
		optSkybox = opt++;
		}
	else
		optSmoke =
		optCoronas =
		optSkybox =
		optBright = -1;
	ADD_CHECK (opt, TXT_AUTOMAP_ROBOTS, extraGameInfo [0].bRobotsOnRadar, KEY_R, HTX_AUTOMAP_ROBOTS);
	optShowRobots = opt++;
	ADD_RADIO (opt, TXT_AUTOMAP_NO_POWERUPS, 0, KEY_D, 3, HTX_AUTOMAP_POWERUPS);
	optShowPowerups = opt++;
	ADD_RADIO (opt, TXT_AUTOMAP_POWERUPS, 0, KEY_P, 3, HTX_AUTOMAP_POWERUPS);
	opt++;
	if (extraGameInfo [0].nRadar) {
		ADD_RADIO (opt, TXT_RADAR_POWERUPS, 0, KEY_A, 3, HTX_AUTOMAP_POWERUPS);
		opt++;
		}
	m [optShowPowerups + extraGameInfo [0].bPowerupsOnRadar].value = 1;
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_RADIO (opt, TXT_RADAR_OFF, 0, KEY_R, 1, HTX_AUTOMAP_RADAR);
	nOptRadar = opt++;
	ADD_RADIO (opt, TXT_RADAR_TOP, 0, KEY_T, 1, HTX_AUTOMAP_RADAR);
	opt++;
	ADD_RADIO (opt, TXT_RADAR_BOTTOM, 0, KEY_O, 1, HTX_AUTOMAP_RADAR);
	opt++;
	if (extraGameInfo [0].nRadar) {
		ADD_TEXT (opt, "", 0);
		opt++;
		sprintf (szRadarRange + 1, TXT_RADAR_RANGE, pszRadarRange [gameOpts->render.automap.nRange]);
		*szRadarRange = *(TXT_RADAR_RANGE - 1);
		ADD_SLIDER (opt, szRadarRange + 1, gameOpts->render.automap.nRange, 0, 2, KEY_A, HTX_RADAR_RANGE);
		nOptRadarRange = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_RADIO (opt, TXT_RADAR_WHITE, 0, KEY_W, 2, NULL);
		optColor = opt++;
		ADD_RADIO (opt, TXT_RADAR_BLACK, 0, KEY_L, 2, NULL);
		opt++;
		}
	else
		nOptRadarRange =
		optColor = -1;
	m [optColor + gameOpts->render.automap.nColor].value = 1;
	m [nOptRadar + extraGameInfo [0].nRadar].value = 1;
	Assert (opt <= sizeofa (m));
	for (;;) {
		i = ExecMenu1 (NULL, TXT_AUTOMAP_MENUTITLE, opt, m, AutomapOptionsCallback, &choice);
		if (i < 0)
			break;
		} 
	gameOpts->render.automap.bTextured = m [nOptTextured].value;
	GET_VAL (gameOpts->render.automap.bBright, optBright);
	GET_VAL (gameOpts->render.automap.bCoronas, optCoronas);
	GET_VAL (gameOpts->render.automap.bSmoke, optSmoke);
	GET_VAL (gameOpts->render.automap.bSkybox, optSkybox);
	if (nOptRadarRange >= 0)
		gameOpts->render.automap.nRange = m [nOptRadarRange].value;
	extraGameInfo [0].bPowerupsOnRadar = m [optShowPowerups].value;
	extraGameInfo [0].bRobotsOnRadar = m [optShowRobots].value;
	for (j = 0; j < 2 + extraGameInfo [0].nRadar; j++)
		if (m [optShowPowerups + j].value) {
			extraGameInfo [0].bPowerupsOnRadar = j;
			break;
			}
	for (j = 0; j < 3; j++)
		if (m [nOptRadar + j].value) {
			extraGameInfo [0].nRadar = j;
			break;
			}
	if (optColor >= 0) {
		for (j = 0; j < 2; j++)
			if (m [optColor + j].value) {
				gameOpts->render.automap.nColor = j;
				break;
				}
		}
	} while (i == -2);
}

//------------------------------------------------------------------------------

static int nOpt3D;

void PowerupOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem * m;
	int			v;

m = menus + nOpt3D;
v = m->value;
if (v != gameOpts->render.powerups.b3D) {
	gameOpts->render.powerups.b3D = v;
	*key = -2;
	return;
	}
}

//------------------------------------------------------------------------------

void PowerupOptionsMenu ()
{
	tMenuItem m [10];
	int	i, j, choice = 0;
	int	opt;
	int	optSpin;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_3D_POWERUPS, gameOpts->render.powerups.b3D, KEY_D, HTX_3D_POWERUPS);
	nOpt3D = opt++;
	if (!gameOpts->render.powerups.b3D)
		optSpin = -1;
	else {
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_RADIO (opt, TXT_SPIN_OFF, 0, KEY_O, 1, NULL);
		optSpin = opt++;
		ADD_RADIO (opt, TXT_SPIN_SLOW, 0, KEY_S, 1, NULL);
		opt++;
		ADD_RADIO (opt, TXT_SPIN_MEDIUM, 0, KEY_M, 1, NULL);
		opt++;
		ADD_RADIO (opt, TXT_SPIN_FAST, 0, KEY_F, 1, NULL);
		opt++;
		m [optSpin + gameOpts->render.powerups.nSpin].value = 1;
		}
	for (;;) {
		i = ExecMenu1 (NULL, TXT_POWERUP_MENUTITLE, opt, m, PowerupOptionsCallback, &choice);
		if (i < 0)
			break;
		} 
	if (gameOpts->render.powerups.b3D && (optSpin >= 0))
		for (j = 0; j < 4; j++)
			if (m [optSpin + j].value) {
				gameOpts->render.powerups.nSpin = j;
				break;
			}
	} while (i == -2);
}

//------------------------------------------------------------------------------

#if SHADOWS

static char *pszReach [4];
static char *pszClip [4];

void ShadowOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + nShadowsOpt;
v = m->value;
if (v != extraGameInfo [0].bShadows) {
	extraGameInfo [0].bShadows = v;
	*key = -2;
	return;
	}
if (extraGameInfo [0].bShadows) {
	m = menus + nMaxLightsOpt;
	v = m->value + 1;
	if (gameOpts->render.shadows.nLights != v) {
		gameOpts->render.shadows.nLights = v;
		sprintf (m->text, TXT_MAX_LIGHTS, gameOpts->render.shadows.nLights);
		m->rebuild = 1;
		}
	m = menus + nShadowReachOpt;
	v = m->value;
	if (gameOpts->render.shadows.nReach != v) {
		gameOpts->render.shadows.nReach = v;
		sprintf (m->text, TXT_SHADOW_REACH, pszReach [gameOpts->render.shadows.nReach]);
		m->rebuild = 1;
		}
#if DBG_SHADOWS
	if (nShadowTestOpt >= 0) {
		m = menus + nShadowTestOpt;
		v = m->value;
		if (bShadowTest != v) {
			bShadowTest = v;
			sprintf (m->text, "Test mode: %d", bShadowTest);
			m->rebuild = 1;
			}
		m = menus + optZPass;
		v = m->value;
		if (bZPass != v) {
			bZPass = v;
			m->rebuild = 1;
			*key = -2;
			return;
			}
		m = menus + optShadowVolume;
		v = m->value;
		if (bShadowVolume != v) {
			bShadowVolume = v;
			m->rebuild = 1;
			*key = -2;
			return;
			}
		}
#endif
	}
}

//------------------------------------------------------------------------------

void ShadowOptionsMenu ()
{
	tMenuItem m [30];
	int	i, j, choice = 0;
	int	opt;
	int	optClipShadows, optPlayerShadows, optRobotShadows, optMissileShadows, optReactorShadows;
	char	szMaxLights [50], szReach [50];
#if DBG_SHADOWS
	char	szShadowTest [50];
	int	optFrontCap, optRearCap, optFrontFaces, optBackFaces, optSWCulling, optWallShadows,
			optFastShadows;
#endif

pszReach [0] = TXT_PRECISE;
pszReach [1] = TXT_SHORT;
pszReach [2] = TXT_MEDIUM;
pszReach [3] = TXT_LONG;

pszClip [0] = TXT_OFF;
pszClip [1] = TXT_FAST;
pszClip [2] = TXT_MEDIUM;
pszClip [3] = TXT_PRECISE;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	if (extraGameInfo [0].bShadows) {
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	ADD_CHECK (opt, TXT_RENDER_SHADOWS, extraGameInfo [0].bShadows, KEY_W, HTX_ADVRND_SHADOWS);
	nShadowsOpt = opt++;
	optClipShadows =
	optPlayerShadows =
	optRobotShadows =
	optMissileShadows =
	optReactorShadows = -1;
#if DBG_SHADOWS
	optZPass =
	optFrontCap =
	optRearCap =
	optShadowVolume =
	optFrontFaces =
	optBackFaces =
	optSWCulling =
	optWallShadows =
	optFastShadows =
	nShadowTestOpt = -1;
#endif
	if (extraGameInfo [0].bShadows) {
		sprintf (szMaxLights + 1, TXT_MAX_LIGHTS, gameOpts->render.shadows.nLights);
		*szMaxLights = *(TXT_MAX_LIGHTS - 1);
		ADD_SLIDER (opt, szMaxLights + 1, gameOpts->render.shadows.nLights - 1, 0, MAX_SHADOW_LIGHTS, KEY_S, HTX_ADVRND_MAXLIGHTS);
		nMaxLightsOpt = opt++;
		sprintf (szReach + 1, TXT_SHADOW_REACH, pszReach [gameOpts->render.shadows.nReach]);
		*szReach = *(TXT_SHADOW_REACH - 1);
		ADD_SLIDER (opt, szReach + 1, gameOpts->render.shadows.nReach, 0, 3, KEY_R, HTX_RENDER_SHADOWREACH);
		nShadowReachOpt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_TEXT (opt, TXT_CLIP_SHADOWS, 0);
		optClipShadows = ++opt;
		for (j = 0; j < 4; j++) {
			ADD_RADIO (opt, pszClip [j], gameOpts->render.shadows.nClip == j, 0, 1, HTX_CLIP_SHADOWS);
			opt++;
			}
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_PLAYER_SHADOWS, gameOpts->render.shadows.bPlayers, KEY_R, HTX_PLAYER_SHADOWS);
		optPlayerShadows = opt++;
		ADD_CHECK (opt, TXT_ROBOT_SHADOWS, gameOpts->render.shadows.bRobots, KEY_R, HTX_ROBOT_SHADOWS);
		optRobotShadows = opt++;
		ADD_CHECK (opt, TXT_MISSILE_SHADOWS, gameOpts->render.shadows.bMissiles, KEY_R, HTX_MISSILE_SHADOWS);
		optMissileShadows = opt++;
		ADD_CHECK (opt, TXT_REACTOR_SHADOWS, gameOpts->render.shadows.bReactors, KEY_R, HTX_REACTOR_SHADOWS);
		optReactorShadows = opt++;
#if DBG_SHADOWS
		ADD_CHECK (opt, TXT_FAST_SHADOWS, gameOpts->render.shadows.bFast, KEY_F, HTX_FAST_SHADOWS);
		optFastShadows = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, "use Z-Pass algorithm", bZPass, 0, NULL);
		optZPass = opt++;
		if (!bZPass) {
			ADD_CHECK (opt, "render front cap", bFrontCap, 0, NULL);
			optFrontCap = opt++;
			ADD_CHECK (opt, "render rear cap", bRearCap, 0, NULL);
			optRearCap = opt++;
			}
		ADD_CHECK (opt, "render shadow volume", bShadowVolume, 0, NULL);
		optShadowVolume = opt++;
		if (bShadowVolume) {
			ADD_CHECK (opt, "render front faces", bFrontFaces, 0, NULL);
			optFrontFaces = opt++;
			ADD_CHECK (opt, "render back faces", bBackFaces, 0, NULL);
			optBackFaces = opt++;
			}
		ADD_CHECK (opt, "render tWall shadows", bWallShadows, 0, NULL);
		optWallShadows = opt++;
		ADD_CHECK (opt, "software culling", bSWCulling, 0, NULL);
		optSWCulling = opt++;
		sprintf (szShadowTest, "test method: %d", bShadowTest);
		ADD_SLIDER (opt, szShadowTest, bShadowTest, 0, 6, KEY_S, NULL);
		nShadowTestOpt = opt++;
#endif
		}
	for (;;) {
		i = ExecMenu1 (NULL, TXT_SHADOW_MENUTITLE, opt, m, &ShadowOptionsCallback, &choice);
		if (i < 0)
			break;
		} 
	for (j = 0; j < 4; j++)
		if (m [optClipShadows + j].value) {
			gameOpts->render.shadows.nClip = j;
			break;
			}
	GET_VAL (gameOpts->render.shadows.bPlayers, optPlayerShadows);
	GET_VAL (gameOpts->render.shadows.bRobots, optRobotShadows);
	GET_VAL (gameOpts->render.shadows.bMissiles, optMissileShadows);
	GET_VAL (gameOpts->render.shadows.bReactors, optReactorShadows);
#if DBG_SHADOWS
	if (extraGameInfo [0].bShadows) {
		GET_VAL (gameOpts->render.shadows.bFast, optFastShadows);
		GET_VAL (bZPass, optZPass);
		GET_VAL (bFrontCap, optFrontCap);
		GET_VAL (bRearCap, optRearCap);
		GET_VAL (bFrontFaces, optFrontFaces);
		GET_VAL (bBackFaces, optBackFaces);
		GET_VAL (bWallShadows, optWallShadows);
		GET_VAL (bSWCulling, optSWCulling);
		GET_VAL (bShadowVolume, optShadowVolume);
		}
#endif
	} while (i == -2);
}

#endif

//------------------------------------------------------------------------------

void CameraOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + nUseCamOpt;
v = m->value;
if (v != extraGameInfo [0].bUseCameras) {
	extraGameInfo [0].bUseCameras = v;
	*key = -2;
	return;
	}
if (extraGameInfo [0].bUseCameras) {
	if (nCamFpsOpt >= 0) {
		m = menus + nCamFpsOpt;
		v = m->value * 5;
		if (gameOpts->render.cameras.nFPS != v) {
			gameOpts->render.cameras.nFPS = v;
			sprintf (m->text, TXT_CAM_REFRESH, gameOpts->render.cameras.nFPS);
			m->rebuild = 1;
			}
		}	
	if (gameOpts->app.bExpertMode && (nCamSpeedOpt >= 0)) {
		m = menus + nCamSpeedOpt;
		v = (m->value + 1) * 1000;
		if (gameOpts->render.cameras.nSpeed != v) {
			gameOpts->render.cameras.nSpeed = v;
			sprintf (m->text, TXT_CAM_SPEED, v / 1000);
			m->rebuild = 1;
			}
		}
	}
}

//------------------------------------------------------------------------------

void CameraOptionsMenu ()
{
	tMenuItem m [10];
	int	i, choice = 0;
	int	opt;
	int	bFSCameras = gameOpts->render.cameras.bFitToWall;
	int	optFSCameras, optTeleCams;
#if 0
	int checks;
#endif

	char szCameraFps [50];
	char szCameraSpeed [50];

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_USE_CAMS, extraGameInfo [0].bUseCameras, KEY_C, HTX_ADVRND_USECAMS);
	nUseCamOpt = opt++;
	if (extraGameInfo [0].bUseCameras && gameOpts->app.bExpertMode) {
		ADD_CHECK (opt, TXT_TELEPORTER_CAMS, extraGameInfo [0].bTeleporterCams, KEY_U, HTX_TELEPORTER_CAMS);
		optTeleCams = opt++;
		ADD_CHECK (opt, TXT_ADJUST_CAMS, gameOpts->render.cameras.bFitToWall, KEY_U, HTX_ADVRND_ADJUSTCAMS);
		optFSCameras = opt++;
		sprintf (szCameraFps + 1, TXT_CAM_REFRESH, gameOpts->render.cameras.nFPS);
		*szCameraFps = *(TXT_CAM_REFRESH - 1);
		ADD_SLIDER (opt, szCameraFps + 1, gameOpts->render.cameras.nFPS / 5, 0, 6, KEY_A, HTX_ADVRND_CAMREFRESH);
		nCamFpsOpt = opt++;
		sprintf (szCameraSpeed + 1, TXT_CAM_SPEED, gameOpts->render.cameras.nSpeed / 1000);
		*szCameraSpeed = *(TXT_CAM_SPEED - 1);
		ADD_SLIDER (opt, szCameraSpeed + 1, (gameOpts->render.cameras.nSpeed / 1000) - 1, 0, 9, KEY_D, HTX_ADVRND_CAMSPEED);
		nCamSpeedOpt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	else {
		optTeleCams = -1;
		optFSCameras = -1;
		nCamFpsOpt = -1;
		nCamSpeedOpt = -1;
		}

	do {
		i = ExecMenu1 (NULL, TXT_CAMERA_MENUTITLE, opt, m, &CameraOptionsCallback, &choice);
	} while (i >= 0);

	if ((extraGameInfo [0].bUseCameras = m [nUseCamOpt].value)) {
		GET_VAL (extraGameInfo [0].bTeleporterCams, optTeleCams);
		GET_VAL (gameOpts->render.cameras.bFitToWall, optFSCameras);
		}
	if (bFSCameras != gameOpts->render.cameras.bFitToWall) {
		DestroyCameras ();
		CreateCameras ();
		}
	} while (i == -2);
}

//------------------------------------------------------------------------------

void SmokeOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem * m;
	int				i, v;

m = menus + nUseSmokeOpt;
v = m->value;
if (v != extraGameInfo [0].bUseSmoke) {
	extraGameInfo [0].bUseSmoke = v;
	*key = -2;
	return;
	}
if (extraGameInfo [0].bUseSmoke) {
	m = menus + nSyncSmokeSizes;
	v = m->value;
	if (v != gameOpts->render.smoke.bSyncSizes) {
		gameOpts->render.smoke.bSyncSizes = v;
		*key = -2;
		return;
		}
	m = menus + nPlrSmokeOpt;
	v = m->value;
	if (gameOpts->render.smoke.bPlayers != v) {
		gameOpts->render.smoke.bPlayers = v;
		*key = -2;
		}
	if (gameOpts->render.smoke.bSyncSizes) {
		m = menus + nSmokeDensOpt [0];
		v = m->value;
		if (gameOpts->render.smoke.nDens [0] != v) {
			gameOpts->render.smoke.nDens [0] = v;
			sprintf (m->text, TXT_SMOKE_DENS, pszAmount [gameOpts->render.smoke.nDens [0]]);
			m->rebuild = 1;
			}
		m = menus + nSmokeSizeOpt [0];
		v = m->value;
		if (gameOpts->render.smoke.nSize [0] != v) {
			gameOpts->render.smoke.nSize [0] = v;
			sprintf (m->text, TXT_SMOKE_SIZE, pszSize [gameOpts->render.smoke.nSize [0]]);
			m->rebuild = 1;
			}
		}
	else {
		for (i = 1; i < 4; i++) {
			if (nSmokeDensOpt [i] >= 0) {
				m = menus + nSmokeDensOpt [i];
				v = m->value;
				if (gameOpts->render.smoke.nDens [i] != v) {
					gameOpts->render.smoke.nDens [i] = v;
					sprintf (m->text, TXT_SMOKE_DENS, pszAmount [gameOpts->render.smoke.nDens [i]]);
					m->rebuild = 1;
					}
				}
			if (nSmokeSizeOpt [i] >= 0) {
				m = menus + nSmokeSizeOpt [i];
				v = m->value;
				if (gameOpts->render.smoke.nSize [i] != v) {
					gameOpts->render.smoke.nSize [i] = v;
					sprintf (m->text, TXT_SMOKE_SIZE, pszSize [gameOpts->render.smoke.nSize [i]]);
					m->rebuild = 1;
					}
				}
			if (nSmokeLifeOpt [i] >= 0) {
				m = menus + nSmokeLifeOpt [i];
				v = m->value;
				if (gameOpts->render.smoke.nLife [i] != v) {
					gameOpts->render.smoke.nLife [i] = v;
					sprintf (m->text, TXT_SMOKE_LIFE, pszLife [gameOpts->render.smoke.nLife [i]]);
					m->rebuild = 1;
					}
				}
			}	
		}
	}
else
	DestroyAllSmoke ();
}

//------------------------------------------------------------------------------

static char szSmokeDens [4][50];
static char szSmokeSize [4][50];
static char szSmokeLife [4][50];

int AddSmokeSliders (tMenuItem *m, int opt, int i)
{
sprintf (szSmokeDens [i] + 1, TXT_SMOKE_DENS, pszAmount [NMCLAMP (gameOpts->render.smoke.nDens [i], 0, 4)]);
*szSmokeDens [i] = *(TXT_SMOKE_DENS - 1);
ADD_SLIDER (opt, szSmokeDens [i] + 1, gameOpts->render.smoke.nDens [i], 0, 4, KEY_P, HTX_ADVRND_SMOKEDENS);
nSmokeDensOpt [i] = opt++;
sprintf (szSmokeSize [i] + 1, TXT_SMOKE_SIZE, pszSize [NMCLAMP (gameOpts->render.smoke.nSize [i], 0, 3)]);
*szSmokeSize [i] = *(TXT_SMOKE_SIZE - 1);
ADD_SLIDER (opt, szSmokeSize [i] + 1, gameOpts->render.smoke.nSize [i], 0, 3, KEY_Z, HTX_ADVRND_PARTSIZE);
nSmokeSizeOpt [i] = opt++;
if (i < 3)
	nSmokeLifeOpt [i] = -1;
else {
	sprintf (szSmokeLife [i] + 1, TXT_SMOKE_LIFE, pszLife [NMCLAMP (gameOpts->render.smoke.nLife [i], 0, 3)]);
	*szSmokeLife [i] = *(TXT_SMOKE_LIFE - 1);
	ADD_SLIDER (opt, szSmokeLife [i] + 1, gameOpts->render.smoke.nLife [i], 0, 2, KEY_L, HTX_SMOKE_LIFE);
	nSmokeLifeOpt [i] = opt++;
	}
return opt;
}

//------------------------------------------------------------------------------

void SmokeOptionsMenu ()
{
	tMenuItem m [30];
	int	i, j, choice = 0;
	int	opt;
	int	nOptSmokeLag, optBotSmoke, optMissSmoke, optDebrisSmoke, 
			optStaticSmoke, optSmokeColl, optSmokeDisp;

	pszSize [0] = TXT_SMALL;
	pszSize [1] = TXT_MEDIUM;
	pszSize [2] = TXT_LARGE;
	pszSize [3] = TXT_VERY_LARGE;

	pszAmount [0] = TXT_QUALITY_LOW;
	pszAmount [1] = TXT_QUALITY_MED;
	pszAmount [2] = TXT_QUALITY_HIGH;
	pszAmount [3] = TXT_VERY_HIGH;
	pszAmount [4] = TXT_EXTREME;

	pszLife [0] = TXT_SHORT;
	pszLife [1] = TXT_MEDIUM;
	pszLife [2] = TXT_LONG;
do {
	memset (m, 0, sizeof (m));
	opt = 0;
	nOptSmokeLag = optBotSmoke = optMissSmoke = optDebrisSmoke = 
	optStaticSmoke = optSmokeColl = optSmokeDisp = -1;

	ADD_CHECK (opt, TXT_USE_SMOKE, extraGameInfo [0].bUseSmoke, KEY_U, HTX_ADVRND_USESMOKE);
	nUseSmokeOpt = opt++;
	for (j = 1; j < 4; j++)
		nSmokeSizeOpt [j] =
		nSmokeDensOpt [j] = -1;
	if (extraGameInfo [0].bUseSmoke) {
		if (gameOpts->app.bExpertMode) {
			if (!gameOpts->render.smoke.bSyncSizes && gameOpts->render.smoke.bPlayers) {
				ADD_TEXT (opt, "", 0);
				opt++;
				}
			ADD_CHECK (opt, TXT_SMOKE_PLAYERS, gameOpts->render.smoke.bPlayers, KEY_Y, HTX_ADVRND_PLRSMOKE);
			nPlrSmokeOpt = opt++;
			if (gameOpts->render.smoke.bPlayers) {
				if (!gameOpts->render.smoke.bSyncSizes) {
					opt = AddSmokeSliders (m, opt, 1);
					}
				ADD_CHECK (opt, TXT_SMOKE_DECREASE_LAG, gameOpts->render.smoke.bDecreaseLag, KEY_R, HTX_ADVREND_DECSMOKELAG);
				nOptSmokeLag = opt++;
				}
			else
				nOptSmokeLag = -1;
			if (!gameOpts->render.smoke.bSyncSizes && gameOpts->render.smoke.bPlayers) {
				ADD_TEXT (opt, "", 0);
				opt++;
				}
			ADD_CHECK (opt, TXT_SMOKE_ROBOTS, gameOpts->render.smoke.bRobots, KEY_O, HTX_ADVRND_BOTSMOKE);
			optBotSmoke = opt++;
			if (gameOpts->render.smoke.bRobots) {
				if (!gameOpts->render.smoke.bSyncSizes) {
					opt = AddSmokeSliders (m, opt, 2);
					}
				}
			if (!gameOpts->render.smoke.bSyncSizes && gameOpts->render.smoke.bRobots) {
				ADD_TEXT (opt, "", 0);
				opt++;
				}
			ADD_CHECK (opt, TXT_SMOKE_MISSILES, gameOpts->render.smoke.bMissiles, KEY_M, HTX_ADVRND_MSLSMOKE);
			optMissSmoke = opt++;
			if (gameOpts->render.smoke.bMissiles) {
				if (!gameOpts->render.smoke.bSyncSizes) {
					opt = AddSmokeSliders (m, opt, 3);
					}
				}
			if (!gameOpts->render.smoke.bSyncSizes && gameOpts->render.smoke.bMissiles) {
				ADD_TEXT (opt, "", 0);
				opt++;
				}
			ADD_CHECK (opt, TXT_SMOKE_DEBRIS, gameOpts->render.smoke.bDebris, KEY_D, HTX_ADVRND_DEBRISSMOKE);
			optDebrisSmoke = opt++;
			ADD_CHECK (opt, TXT_SMOKE_STATIC, gameOpts->render.smoke.bStatic, KEY_S, HTX_ADVRND_STATICSMOKE);
			optStaticSmoke = opt++;
			ADD_CHECK (opt, TXT_SMOKE_COLLISION, gameOpts->render.smoke.bCollisions, KEY_I, HTX_ADVRND_SMOKECOLL);
			optSmokeColl = opt++;
			ADD_CHECK (opt, TXT_SMOKE_DISPERSE, gameOpts->render.smoke.bDisperse, KEY_D, HTX_ADVRND_SMOKEDISP);
			optSmokeDisp = opt++;
			ADD_TEXT (opt, "", 0);
			opt++;
			ADD_CHECK (opt, TXT_SYNC_SIZES, gameOpts->render.smoke.bSyncSizes, KEY_M, HTX_ADVRND_SYNCSIZES);
			nSyncSmokeSizes = opt++;
			if (gameOpts->render.smoke.bSyncSizes) {
				opt = AddSmokeSliders (m, opt, 0);
				for (j = 1; j < 4; j++) {
					gameOpts->render.smoke.nSize [j] = gameOpts->render.smoke.nSize [0];
					gameOpts->render.smoke.nDens [j] = gameOpts->render.smoke.nDens [0];
					}
				}
			else {
				nSmokeDensOpt [0] =
				nSmokeSizeOpt [0] = -1;
				}	
			}
		}
	else
		nOptSmokeLag =
		nPlrSmokeOpt =
		optBotSmoke =
		optMissSmoke =
		optDebrisSmoke =
		optStaticSmoke =
		optSmokeColl =
		optSmokeDisp = -1;

	Assert (opt <= sizeof (m) / sizeof (m [0]));
	do {
		i = ExecMenu1 (NULL, TXT_SMOKE_MENUTITLE, opt, m, &SmokeOptionsCallback, &choice);
		} while (i >= 0);
	if ((extraGameInfo [0].bUseSmoke = m [nUseSmokeOpt].value)) {
		GET_VAL (gameOpts->render.smoke.bPlayers, nPlrSmokeOpt);
		GET_VAL (gameOpts->render.smoke.bRobots, optBotSmoke);
		GET_VAL (gameOpts->render.smoke.bMissiles, optMissSmoke);
		GET_VAL (gameOpts->render.smoke.bDebris, optDebrisSmoke);
		GET_VAL (gameOpts->render.smoke.bStatic, optStaticSmoke);
		GET_VAL (gameOpts->render.smoke.bCollisions, optSmokeColl);
		GET_VAL (gameOpts->render.smoke.bDisperse, optSmokeDisp);
		GET_VAL (gameOpts->render.smoke.bDecreaseLag, nOptSmokeLag);
		//GET_VAL (gameOpts->render.smoke.bSyncSizes, nSyncSmokeSizes);
		if (gameOpts->render.smoke.bSyncSizes) {
			for (j = 1; j < 4; j++) {
				gameOpts->render.smoke.nSize [j] = gameOpts->render.smoke.nSize [0];
				gameOpts->render.smoke.nDens [j] = gameOpts->render.smoke.nDens [0];
				}
			}
		}	
	} while (i == -2);
}

//------------------------------------------------------------------------------

void AdvancedRenderOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

if (optContrast >= 0) {
	m = menus + optContrast;
	v = m->value;
	if (v != gameStates.ogl.nContrast) {
		gameStates.ogl.nContrast = v;
		sprintf (m->text, TXT_CONTRAST, ContrastText ());
		m->rebuild = 1;
		}
	}
if (gameOpts->app.bExpertMode) {
	m = menus + nRendQualOpt;
	v = m->value;
	if (gameOpts->render.nQuality != v) {
		gameOpts->render.nQuality = v;
		sprintf (m->text, TXT_RENDQUAL, pszRendQual [gameOpts->render.nQuality]);
		m->rebuild = 1;
		}
	if (nTexQualOpt > 0) {
		m = menus + nTexQualOpt;
		v = m->value;
		if (gameOpts->render.textures.nQuality != v) {
			gameOpts->render.textures.nQuality = v;
			sprintf (m->text, TXT_TEXQUAL, pszTexQual [gameOpts->render.textures.nQuality]);
			m->rebuild = 1;
			}
		}
	m = menus + nTranspOpt;
	v = (GR_ACTUAL_FADE_LEVELS * m->value + 5) / 10;
	if (extraGameInfo [0].grWallTransparency != v) {
		extraGameInfo [0].grWallTransparency = v;
		sprintf (m->text, TXT_WALL_TRANSP, m->value * 10, '%');
		m->rebuild = 1;
		}
	}
}

//------------------------------------------------------------------------------

static	char *pszLightRange [3];

void LightingOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

if (nLightingMethodOpt >= 0) {
	v = menus [nLightingMethodOpt + 1].value;
	if (v != gameOpts->render.bDynLighting) {
		gameOpts->render.bDynLighting = v;
		gameOpts->render.color.bUseLightMaps = menus [nLightingMethodOpt + 1].value;
		*key = -2;
		return;
		}
	if (gameStates.render.color.bLightMapsOk) {
		v = menus [nLightingMethodOpt + 2].value;
		if (v != gameOpts->render.color.bUseLightMaps) {
			gameOpts->render.color.bUseLightMaps = v;
			gameOpts->render.bDynLighting = menus [nLightingMethodOpt + 2].value;
			*key = -2;
			return;
			}
		}
	}
m = menus + nGunColorOpt;
v = m->value;
if (v != gameOpts->render.color.bGunLight) {
	gameOpts->render.color.bGunLight = v;
	*key = -2;
	return;
	}
if (nOglMaxLightsOpt >= 0) {
	m = menus + nOglMaxLightsOpt;
	v = m->value + 4;
	if (v != gameOpts->ogl.nMaxLights) {
		gameOpts->ogl.nMaxLights = v;
		sprintf (m->text, TXT_OGL_MAXLIGHTS, nMaxNearestLights [gameOpts->ogl.nMaxLights]);
		m->rebuild = 1;
		return;
		}
	}
if (nLightRangeOpt >= 0) {
	m = menus + nLightRangeOpt;
	v = m->value;
	if (gameStates.render.color.bLightMapsOk && gameOpts->render.color.bUseLightMaps) {
		if (gameOpts->render.color.nLightMapRange != v) {
			gameOpts->render.color.nLightMapRange = v;
			sprintf (m->text, TXT_LMAP_RANGE, 50 + gameOpts->render.color.nLightMapRange * 10, '%');
			m->rebuild = 1;
			}
		}
	else {
		if (extraGameInfo [0].nLightRange != v) {
			extraGameInfo [0].nLightRange = v;
			sprintf (m->text, TXT_LIGHT_RANGE, pszLightRange [v]);
			m->rebuild = 1;
			}
		}
	}
}

//------------------------------------------------------------------------------

void LightingOptionsMenu ()
{
	tMenuItem m [15];
	int	i, choice = 0;
	int	opt;
	int	optColoredLight, optObjectLight, optMixColors, optPowerupLights, optFlickerLights;
#if 0
	int checks;
#endif

	char szMaxLights [50];
	char szLightRange [50];
	int bLightMaps = gameOpts->render.color.bUseLightMaps;

	pszLightRange [0] = TXT_STANDARD;
	pszLightRange [1] = TXT_MEDIUM;
	pszLightRange [2] = TXT_FAR;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	nLightingMethodOpt =
	nLightRangeOpt =
	nOglMaxLightsOpt = 
	optObjectLight = -1;
	if (!gameStates.app.bGameRunning) {
		ADD_RADIO (opt, TXT_STD_LIGHTING, !(gameOpts->render.color.bUseLightMaps || gameOpts->render.bDynLighting), KEY_S, 1, NULL);
		nLightingMethodOpt = opt++;
		ADD_RADIO (opt, TXT_OGL_LIGHTING, gameOpts->render.bDynLighting, KEY_G, 1, HTX_OGL_LIGHTING);
		opt++;
		if (gameStates.render.color.bLightMapsOk) {
			ADD_RADIO (opt, TXT_USE_LMAPS, gameOpts->render.color.bUseLightMaps, KEY_M, 1, HTX_RENDER_LIGHTMAPS);
			opt++;
			}
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	if (gameStates.render.color.bLightMapsOk && gameOpts->render.color.bUseLightMaps) {
		sprintf (szLightRange + 1, TXT_LMAP_RANGE, 50 + gameOpts->render.color.nLightMapRange * 10, '%');
		*szLightRange = *(TXT_LMAP_RANGE - 1);
		ADD_SLIDER (opt, szLightRange + 1, gameOpts->render.color.nLightMapRange, 0, 10, KEY_R, HTX_ADVRND_LMAPRANGE);
		nLightRangeOpt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	if (!gameStates.app.bGameRunning && (nLightingMethodOpt >= 0)) {
		if (gameOpts->render.bDynLighting) {
			sprintf (szLightRange + 1, TXT_LIGHT_RANGE, pszLightRange [extraGameInfo [0].nLightRange], ' ');
			*szLightRange = *(TXT_LIGHT_RANGE - 1);
			ADD_SLIDER (opt, szLightRange + 1, extraGameInfo [0].nLightRange, 0, 2, KEY_R, HTX_ADVRND_LIGHTRANGE);
			nLightRangeOpt = opt++;
#if 0
			ADD_TEXT (opt, "", 0);
			opt++;
#endif
			ADD_CHECK (opt, TXT_OBJECT_LIGHTING, gameOpts->ogl.bLightObjects, KEY_O, HTX_OBJECT_LIGHTING);
			optObjectLight = opt++;
			sprintf (szMaxLights + 1, TXT_OGL_MAXLIGHTS, nMaxNearestLights [gameOpts->ogl.nMaxLights]);
			*szMaxLights = *(TXT_OGL_MAXLIGHTS - 1);
			ADD_SLIDER (opt, szMaxLights + 1, gameOpts->ogl.nMaxLights - 4, 0, sizeofa (nMaxNearestLights) - 5, KEY_I, HTX_OGL_MAXLIGHTS);
			nOglMaxLightsOpt = opt++;
			ADD_TEXT (opt, "", 0);
			opt++;
			}
		}
	ADD_CHECK (opt, TXT_USE_COLOR, gameOpts->render.color.bAmbientLight, KEY_C, HTX_RENDER_AMBICOLOR);
	optColoredLight = opt++;
	ADD_CHECK (opt, TXT_USE_WPNCOLOR, gameOpts->render.color.bGunLight, KEY_W, HTX_RENDER_WPNCOLOR);
	nGunColorOpt = opt++;
	optMixColors = 
	optPowerupLights = -1;
	if (gameOpts->app.bExpertMode) {
		if (gameOpts->render.color.bGunLight) {
			ADD_CHECK (opt, TXT_MIX_COLOR, gameOpts->render.color.bMix, KEY_X, HTX_ADVRND_MIXCOLOR);
			optMixColors = opt++;
			}
		ADD_CHECK (opt, TXT_POWERUPLIGHTS, !extraGameInfo [0].bPowerupLights, KEY_P, HTX_POWERUPLIGHTS);
		optPowerupLights = opt++;
		}
	ADD_CHECK (opt, TXT_FLICKERLIGHTS, extraGameInfo [0].bFlickerLights, KEY_F, HTX_FLICKERLIGHTS);
	optFlickerLights = opt++;
	for (;;) {
		i = ExecMenu1 (NULL, TXT_LIGHTING_MENUTITLE, opt, m, &LightingOptionsCallback, &choice);
		if (i < 0)
			break;
		} 
	if (gameStates.render.color.bLightMapsOk) {
		// (re-) calc lightmaps if either lightmaps turned on and no lightmaps yet computed
		// or lightmap range has changed
		if (gameStates.app.bGameRunning && gameOpts->render.color.bUseLightMaps && 
			 !bLightMaps && !HaveLightMaps ())
			CreateLightMaps ();
		}
	if (gameOpts->render.bDynLighting && (optObjectLight >= 0))
		gameOpts->ogl.bLightObjects = m [optObjectLight].value;
	if (optColoredLight >= 0)
		gameOpts->render.color.bAmbientLight = m [optColoredLight].value;
	if (nGunColorOpt >= 0)
		gameOpts->render.color.bGunLight = m [nGunColorOpt].value;
	if (gameOpts->app.bExpertMode) {
		if (gameStates.render.color.bLightMapsOk && gameOpts->render.color.bUseLightMaps)
			gameStates.ogl.nContrast = 8;
		if (gameOpts->render.color.bGunLight)
			GET_VAL (gameOpts->render.color.bMix, optMixColors);
#if EXPMODE_DEFAULTS
			else
				gameOpts->render.color.bMix = 1;
#endif
		if (optPowerupLights >= 0)
			extraGameInfo [0].bPowerupLights = !m [optPowerupLights].value;
		}
	extraGameInfo [0].bFlickerLights = m [optFlickerLights].value;
	} while (i == -2);
}

//------------------------------------------------------------------------------

void MovieOptionsMenu ()
{
	tMenuItem m [5];
	int	i, choice = 0;
	int	opt;
	int	optMovieQual, optMovieSize, optSubTitles;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_CHECK (opt, TXT_MOVIE_SUBTTL, gameOpts->movies.bSubTitles, KEY_O, HTX_RENDER_SUBTTL);
	optSubTitles = opt++;
	if (gameOpts->app.bExpertMode) {
		ADD_CHECK (opt, TXT_MOVIE_QUAL, gameOpts->movies.nQuality, KEY_Q, HTX_RENDER_MOVIEQUAL);
		optMovieQual = opt++;
		ADD_CHECK (opt, TXT_MOVIE_FULLSCR, gameOpts->movies.bResize, KEY_U, HTX_RENDER_MOVIEFULL);
		optMovieSize = opt++;
		}
	else
		optMovieQual = 
		optMovieSize = -1;

	for (;;) {
		i = ExecMenu1 (NULL, TXT_MOVIE_OPTIONS, opt, m, NULL, &choice);
		if (i < 0)
			break;
		} 
	gameOpts->movies.bSubTitles = m [optSubTitles].value;
	if (gameOpts->app.bExpertMode) {
		gameOpts->movies.nQuality = m [optMovieQual].value;
		gameOpts->movies.bResize = m [optMovieSize].value;
		}
	} while (i == -2);
}

//------------------------------------------------------------------------------

void RenderOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

if (!gameStates.app.bNostalgia) {
	m = menus + optBrightness;
	v = m->value;
	if (v != GrGetPaletteGamma ())
		GrSetPaletteGamma (v);
	}
m = menus + nFPSopt;
v = fpsTable [m->value];
if (gameOpts->render.nMaxFPS != (v ? v : 1)) {
	gameOpts->render.nMaxFPS = v ? v : 1;
	if (v)
		sprintf (m->text, TXT_FRAMECAP, gameOpts->render.nMaxFPS);
	else
		sprintf (m->text, TXT_NO_FRAMECAP);
	m->rebuild = 1;
	}
if (gameOpts->app.bExpertMode) {
	if (optContrast >= 0) {
		m = menus + optContrast;
		v = m->value;
		if (v != gameStates.ogl.nContrast) {
			gameStates.ogl.nContrast = v;
			sprintf (m->text, TXT_CONTRAST, ContrastText ());
			m->rebuild = 1;
			}
		}
	m = menus + nRendQualOpt;
	v = m->value;
	if (gameOpts->render.nQuality != v) {
		gameOpts->render.nQuality = v;
		sprintf (m->text, TXT_RENDQUAL, pszRendQual [gameOpts->render.nQuality]);
		m->rebuild = 1;
		}
	if (nTexQualOpt > 0) {
		m = menus + nTexQualOpt;
		v = m->value;
		if (gameOpts->render.textures.nQuality != v) {
			gameOpts->render.textures.nQuality = v;
			sprintf (m->text, TXT_TEXQUAL, pszTexQual [gameOpts->render.textures.nQuality]);
			m->rebuild = 1;
			}
		}
	m = menus + nTranspOpt;
	v = (GR_ACTUAL_FADE_LEVELS * m->value + 5) / 10;
	if (extraGameInfo [0].grWallTransparency != v) {
		extraGameInfo [0].grWallTransparency = v;
		sprintf (m->text, TXT_WALL_TRANSP, m->value * 10, '%');
		m->rebuild = 1;
		}
	}
}

//------------------------------------------------------------------------------

void RenderOptionsMenu ()
{
	tMenuItem m [40];
	int	h, i, choice = 0;
	int	opt;
	int	optSmokeOpts, optShadowOpts, optCameraOpts, optLightingOpts, optMovieOpts,	
			optAdvOpts, optEffectOpts, optPowerupOpts, optAutomapOpts;
	int	optUseGamma, optColoredWalls;
#ifdef _DEBUG
	int	optWireFrame, optTextures, optObjects, optWalls, optDynLight;
#endif

	char szMaxFps [50];
	char szWallTransp [50];
	char szRendQual [50];
	char szTexQual [50];
	char szContrast [50];

	int nRendQualSave = gameOpts->render.nQuality;

	pszRendQual [0] = TXT_QUALITY_LOW;
	pszRendQual [1] = TXT_QUALITY_MED;
	pszRendQual [2] = TXT_QUALITY_HIGH;
	pszRendQual [3] = TXT_VERY_HIGH;
	pszRendQual [4] = TXT_QUALITY_MAX;

	pszTexQual [0] = TXT_QUALITY_LOW;
	pszTexQual [1] = TXT_QUALITY_MED;
	pszTexQual [2] = TXT_QUALITY_HIGH;
	pszTexQual [3] = TXT_QUALITY_MAX;

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	optPowerupOpts = optAutomapOpts = -1;
	if (!gameStates.app.bNostalgia) {
		ADD_SLIDER (opt, TXT_BRIGHTNESS, GrGetPaletteGamma (), 0, 16, KEY_B, HTX_RENDER_BRIGHTNESS);
		optBrightness = opt++;
		}
	if (gameOpts->render.nMaxFPS > 1)
		sprintf (szMaxFps + 1, TXT_FRAMECAP, gameOpts->render.nMaxFPS);
	else
		sprintf (szMaxFps + 1, TXT_NO_FRAMECAP);
	*szMaxFps = *(TXT_FRAMECAP - 1);
	ADD_SLIDER (opt, szMaxFps + 1, FindTableFps (gameOpts->render.nMaxFPS), 0, 15, KEY_F, HTX_RENDER_FRAMECAP);
	nFPSopt = opt++;

	if (gameOpts->app.bExpertMode) {
		if (!(gameStates.render.color.bLightMapsOk && gameOpts->render.color.bUseLightMaps)) {
			sprintf (szContrast, TXT_CONTRAST, ContrastText ());
			ADD_SLIDER (opt, szContrast, gameStates.ogl.nContrast, 0, 16, KEY_C, HTX_ADVRND_CONTRAST);
			optContrast = opt++;
			}
		sprintf (szRendQual + 1, TXT_RENDQUAL, pszRendQual [gameOpts->render.nQuality]);
		*szRendQual = *(TXT_RENDQUAL - 1);
		ADD_SLIDER (opt, szRendQual + 1, gameOpts->render.nQuality, 0, 4, KEY_Q, HTX_ADVRND_RENDQUAL);
		nRendQualOpt = opt++;
		if (gameStates.app.bGameRunning)
			nTexQualOpt = -1;
		else {
			sprintf (szTexQual + 1, TXT_TEXQUAL, pszTexQual [gameOpts->render.textures.nQuality]);
			*szTexQual = *(TXT_TEXQUAL + 1);
			ADD_SLIDER (opt, szTexQual + 1, gameOpts->render.textures.nQuality, 0, 3, KEY_U, HTX_ADVRND_TEXQUAL);
			nTexQualOpt = opt++;
			}
		ADD_TEXT (opt, "", 0);
		opt++;
		h = extraGameInfo [0].grWallTransparency * 10 / GR_ACTUAL_FADE_LEVELS;
		sprintf (szWallTransp + 1, TXT_WALL_TRANSP, h * 10, '%');
		*szWallTransp = *(TXT_WALL_TRANSP - 1);
		ADD_SLIDER (opt, szWallTransp + 1, h, 0, 10, KEY_T, HTX_ADVRND_WALLTRANSP);
		nTranspOpt = opt++;
		ADD_CHECK (opt, TXT_COLOR_WALLS, gameOpts->render.color.bWalls, KEY_W, HTX_ADVRND_COLORWALLS);
		optColoredWalls = opt++;
#if 0
		ADD_CHECK (opt, TXT_GAMMA_BRIGHT, gameOpts->ogl.bSetGammaRamp, KEY_V, HTX_ADVRND_GAMMA);
		optUseGamma = opt++;
#else
		optUseGamma = -1;
#endif
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_MENU (opt, TXT_LIGHTING_OPTIONS, KEY_L, HTX_RENDER_LIGHTINGOPTS);
		optLightingOpts = opt++;
		ADD_MENU (opt, TXT_SMOKE_OPTIONS, KEY_S, HTX_RENDER_SMOKEOPTS);
		optSmokeOpts = opt++;
		if (!(gameStates.app.bEnableShadows && gameStates.render.bHaveStencilBuffer))
			optShadowOpts = -1;
		else {
			ADD_MENU (opt, TXT_SHADOW_OPTIONS, KEY_A, HTX_RENDER_SHADOWOPTS);
			optShadowOpts = opt++;
			}
		ADD_MENU (opt, TXT_EFFECT_OPTIONS, KEY_E, HTX_RENDER_EFFECTOPTS);
		optEffectOpts = opt++;
		ADD_MENU (opt, TXT_CAMERA_OPTIONS, KEY_C, HTX_RENDER_CAMERAOPTS);
		optCameraOpts = opt++;
		ADD_MENU (opt, TXT_POWERUP_OPTIONS, KEY_P, HTX_RENDER_PRUPOPTS);
		optPowerupOpts = opt++;
		ADD_MENU (opt, TXT_AUTOMAP_OPTIONS, KEY_M, HTX_RENDER_AUTOMAPOPTS);
		optAutomapOpts = opt++;
		ADD_MENU (opt, TXT_MOVIE_OPTIONS, KEY_M, HTX_RENDER_MOVIEOPTS);
		optMovieOpts = opt++;
		}
	else
		nRendQualOpt =
		nTexQualOpt =
		nTranspOpt = 
		optColoredWalls =
		optContrast =
		optLightingOpts =
		optSmokeOpts =
		optShadowOpts =
		optEffectOpts =
		optCameraOpts = 
		optMovieOpts = 
		optAdvOpts = -1;

#ifdef _DEBUG
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_CHECK (opt, "Draw wire frame", gameOpts->render.bWireFrame, 0, NULL);
	optWireFrame = opt++;
	ADD_CHECK (opt, "Draw textures", gameOpts->render.bTextures, 0, NULL);
	optTextures = opt++;
	ADD_CHECK (opt, "Draw walls", gameOpts->render.bWalls, 0, NULL);
	optWalls = opt++;
	ADD_CHECK (opt, "Draw objects", gameOpts->render.bObjects, 0, NULL);
	optObjects = opt++;
	ADD_CHECK (opt, "Dynamic Light", gameOpts->render.bDynamicLight, 0, NULL);
	optDynLight = opt++;
#endif

	Assert (sizeofa (m) >= opt);
	for (;;) {
		i = ExecMenu1 (NULL, TXT_RENDER_OPTS, opt, m, &RenderOptionsCallback, &choice);
		if (i < 0)
			break;
		if (gameOpts->app.bExpertMode) {
			if ((optLightingOpts >= 0) && (i == optLightingOpts))
				LightingOptionsMenu ();
			else if ((optSmokeOpts >= 0) && (i == optSmokeOpts))
				SmokeOptionsMenu ();
			else if ((optShadowOpts >= 0) && (i == optShadowOpts))
				ShadowOptionsMenu ();
			else if ((optEffectOpts >= 0) && (i == optEffectOpts))
				EffectOptionsMenu ();
			else if ((optCameraOpts >= 0) && (i == optCameraOpts))
				CameraOptionsMenu ();
			else if ((optPowerupOpts >= 0) && (i == optPowerupOpts))
				PowerupOptionsMenu ();
			else if ((optAutomapOpts >= 0) && (i == optAutomapOpts))
				AutomapOptionsMenu ();
			else if ((optMovieOpts >= 0) && (i == optMovieOpts))
				MovieOptionsMenu ();
			}
		} 
	if (!gameStates.app.bNostalgia)
		GrSetPaletteGamma (m [optBrightness].value);
	if (gameOpts->app.bExpertMode) {
		gameOpts->render.color.bWalls = m [optColoredWalls].value;
		GET_VAL (gameOpts->ogl.bSetGammaRamp, optUseGamma);
		if (gameStates.render.color.bLightMapsOk && gameOpts->render.color.bUseLightMaps)
			gameStates.ogl.nContrast = 8;
		else if (optContrast >= 0)
			gameStates.ogl.nContrast = m [optContrast].value;
		if (nRendQualSave != gameOpts->render.nQuality)
			SetRenderQuality ();
		}
#if EXPMODE_DEFAULTS
	else {
		gameOpts->render.nMaxFPS = 250;
		gameOpts->render.color.nLightMapRange = 5;
		gameOpts->render.color.bMix = 1;
		gameOpts->render.nQuality = 3;
		gameOpts->render.color.bWalls = 1;
		gameOpts->render.bTransparentEffects = 1;
		gameOpts->render.smoke.bPlayers = 0;
		gameOpts->render.smoke.bRobots =
		gameOpts->render.smoke.bMissiles = 1;
		gameOpts->render.smoke.bCollisions = 0;
		gameOpts->render.smoke.bDisperse = 0;
		gameOpts->render.smoke.nDens = 2;
		gameOpts->render.smoke.nSize = 3;
		gameOpts->render.cameras.bFitToWall = 0;
		gameOpts->render.cameras.nSpeed = 5000;
		gameOpts->render.cameras.nFPS = 0;
		gameOpts->movies.nQuality = 0;
		gameOpts->movies.bResize = 1;
		gameStates.ogl.nContrast = 8;
		gameOpts->ogl.bSetGammaRamp = 0;
		}
#endif
#ifdef _DEBUG
	gameOpts->render.bWireFrame = m [optWireFrame].value;
	gameOpts->render.bTextures = m [optTextures].value;
	gameOpts->render.bObjects = m [optObjects].value;
	gameOpts->render.bWalls = m [optWalls].value;
	gameOpts->render.bDynamicLight = m [optDynLight].value;
#endif
	} while (i == -2);
}

//------------------------------------------------------------------------------

static char *pszMslTurnSpeeds [3];

void GameplayOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

m = menus + nDiffOpt;
v = m->value;
if (gameOpts->gameplay.nPlayerDifficultyLevel != v) {
	gameOpts->gameplay.nPlayerDifficultyLevel = v;
	if (!IsMultiGame) {
		gameStates.app.nDifficultyLevel = v;
		InitGateIntervals ();
		}
	sprintf (m->text, TXT_DIFFICULTY2, MENU_DIFFICULTY_TEXT (gameOpts->gameplay.nPlayerDifficultyLevel));
	m->rebuild = 1;
	}

if (gameOpts->app.bExpertMode) {
	m = menus + nRSDopt;
	v = (m->value - 1) * 5;
	if (extraGameInfo [0].nSpawnDelay != v * 1000) {
		extraGameInfo [0].nSpawnDelay = v * 1000;
		sprintf (m->text, TXT_RESPAWN_DELAY, (v < 0) ? -1 : v);
		m->rebuild = 1;
		}

	m = menus + nSmokeGrenadeOpt;
	v = m->value;
	if (extraGameInfo [0].bSmokeGrenades != v) {
		extraGameInfo [0].bSmokeGrenades = v;
		*key = -2;
		return;
		}

	if (nMaxSmokeGrenOpt >= 0) {
		m = menus + nMaxSmokeGrenOpt;
		v = m->value + 1;
		if (extraGameInfo [0].nMaxSmokeGrenades != v) {
			extraGameInfo [0].nMaxSmokeGrenades = v;
			sprintf (m->text, TXT_MAX_SMOKEGRENS, extraGameInfo [0].nMaxSmokeGrenades);
			m->rebuild = 1;
			}
		}
	}
}

//------------------------------------------------------------------------------

#define D2X_MENU_GAP 0

void GameplayOptionsMenu ()
{
	tMenuItem m [35];
	int	i, j, opt = 0, choice = 0;
	int	optFixedSpawn = -1, optSnipeMode = -1, optAutoSel = -1, optInventory = -1, optHeadlight = -1,
			optDualMiss = -1, optDropAll = -1, optImmortal = -1, optMultiBosses = -1, optTripleFusion = -1,
			optSmartWeaponSwitch = -1, optWeaponDrop = -1, optIdleAnims = -1, optAwareness = -1;
	char	szRespawnDelay [60];
	char	szDifficulty [50], szMaxSmokeGrens [50];

do {
	memset (&m, 0, sizeof (m));
	opt = 0;
	sprintf (szDifficulty + 1, TXT_DIFFICULTY2, MENU_DIFFICULTY_TEXT (gameStates.app.nDifficultyLevel));
	*szDifficulty = *(TXT_DIFFICULTY2 - 1);
	ADD_SLIDER (opt, szDifficulty + 1, gameStates.app.nDifficultyLevel, 0, 4, KEY_D, HTX_GPLAY_DIFFICULTY);
	nDiffOpt = opt++;
	if (gameOpts->app.bExpertMode) {
		sprintf (szRespawnDelay + 1, TXT_RESPAWN_DELAY, (extraGameInfo [0].nSpawnDelay < 0) ? -1 : extraGameInfo [0].nSpawnDelay / 1000);
		*szRespawnDelay = *(TXT_RESPAWN_DELAY - 1);
		ADD_SLIDER (opt, szRespawnDelay + 1, extraGameInfo [0].nSpawnDelay / 5000 + 1, 0, 13, KEY_R, HTX_GPLAY_SPAWNDELAY);
		nRSDopt = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_HEADLIGHT_ON, gameOpts->gameplay.bHeadlightOn, KEY_H, HTX_MISC_HEADLIGHT);
		optHeadlight = opt++;
		ADD_CHECK (opt, TXT_USE_INVENTORY, gameOpts->gameplay.bInventory, KEY_V, HTX_GPLAY_INVENTORY);
		optInventory = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_MULTI_BOSSES, extraGameInfo [0].bMultiBosses, KEY_M, HTX_GPLAY_MULTIBOSS);
		optMultiBosses = opt++;
		ADD_CHECK (opt, TXT_IDLE_ANIMS, gameOpts->gameplay.bIdleAnims, KEY_D, HTX_GPLAY_IDLEANIMS);
		optIdleAnims = opt++;
		ADD_CHECK (opt, TXT_AI_AWARENESS, gameOpts->gameplay.nAIAwareness, KEY_I, HTX_GPLAY_AWARENESS);
		optAwareness = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_ALWAYS_RESPAWN, extraGameInfo [0].bImmortalPowerups, KEY_P, HTX_GPLAY_ALWAYSRESP);
		optImmortal = opt++;
		ADD_CHECK (opt, TXT_FIXED_SPAWN, extraGameInfo [0].bFixedRespawns, KEY_F, HTX_GPLAY_FIXEDSPAWN);
		optFixedSpawn = opt++;
		ADD_CHECK (opt, TXT_DROP_ALL, extraGameInfo [0].bDropAllMissiles, KEY_A, HTX_GPLAY_DROPALL);
		optDropAll = opt++;
		ADD_CHECK (opt, TXT_DROP_QUADSUPER, extraGameInfo [0].nWeaponDropMode, KEY_Q, HTX_GPLAY_DROPQUAD);
		optWeaponDrop = opt++;
		ADD_CHECK (opt, TXT_DUAL_LAUNCH, extraGameInfo [0].bDualMissileLaunch, KEY_U, HTX_GPLAY_DUALLAUNCH);
		optDualMiss = opt++;
		ADD_CHECK (opt, TXT_TRIPLE_FUSION, extraGameInfo [0].bTripleFusion, KEY_U, HTX_GPLAY_TRIFUSION);
		optTripleFusion = opt++;
		if (extraGameInfo [0].bSmokeGrenades) {
			ADD_TEXT (opt, "", 0);
			opt++;
			}
		ADD_CHECK (opt, TXT_GPLAY_SMOKEGRENADES, extraGameInfo [0].bSmokeGrenades, KEY_S, HTX_GPLAY_SMOKEGRENADES);
		nSmokeGrenadeOpt = opt++;
		if (extraGameInfo [0].bSmokeGrenades) {
			sprintf (szMaxSmokeGrens + 1, TXT_MAX_SMOKEGRENS, extraGameInfo [0].nMaxSmokeGrenades);
			*szMaxSmokeGrens = *(TXT_MAX_SMOKEGRENS - 1);
			ADD_SLIDER (opt, szMaxSmokeGrens + 1, extraGameInfo [0].nMaxSmokeGrenades - 1, 0, 3, KEY_X, HTX_GPLAY_MAXGRENADES);
			nMaxSmokeGrenOpt = opt++;
			}
		else
			nMaxSmokeGrenOpt = -1;
		}
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_CHECK (opt, TXT_SMART_WPNSWITCH, extraGameInfo [0].bSmartWeaponSwitch, KEY_W, HTX_GPLAY_SMARTSWITCH);
	optSmartWeaponSwitch = opt++;
	optAutoSel = opt;
	ADD_RADIO (opt, TXT_WPNSEL_NEVER, 0, KEY_N, 2, HTX_GPLAY_WSELNEVER);
	opt++;
	ADD_RADIO (opt, TXT_WPNSEL_EMPTY, 0, KEY_Y, 2, HTX_GPLAY_WSELEMPTY);
	opt++;
	ADD_RADIO (opt, TXT_WPNSEL_ALWAYS, 0, KEY_T, 2, HTX_GPLAY_WSELALWAYS);
	opt++;
	ADD_TEXT (opt, "", 0);
	opt++;
	optSnipeMode = opt;
	ADD_RADIO (opt, TXT_ZOOM_OFF, 0, KEY_D, 3, HTX_GPLAY_ZOOMOFF);
	opt++;
	ADD_RADIO (opt, TXT_ZOOM_FIXED, 0, KEY_X, 3, HTX_GPLAY_ZOOMFIXED);
	opt++;
	ADD_RADIO (opt, TXT_ZOOM_SMOOTH, 0, KEY_Z, 3, HTX_GPLAY_ZOOMSMOOTH);
	opt++;
	m [optAutoSel + NMCLAMP (gameOpts->gameplay.nAutoSelectWeapon, 0, 2)].value = 1;
	m [optSnipeMode + NMCLAMP (extraGameInfo [0].nZoomMode, 0, 2)].value = 1;
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, TXT_GAMEPLAY_OPTS, opt, m, &GameplayOptionsCallback, &choice);
		} while (i >= 0);
	} while (i == -2);
if (gameOpts->app.bExpertMode) {
	extraGameInfo [0].bFixedRespawns = m [optFixedSpawn].value;
	extraGameInfo [0].bSmokeGrenades = m [nSmokeGrenadeOpt].value;
	extraGameInfo [0].bDualMissileLaunch = m [optDualMiss].value;
	extraGameInfo [0].bDropAllMissiles = m [optDropAll].value;
	extraGameInfo [0].bImmortalPowerups = m [optImmortal].value;
	extraGameInfo [0].bMultiBosses = m [optMultiBosses].value;
	extraGameInfo [0].bSmartWeaponSwitch = m [optSmartWeaponSwitch].value;
	extraGameInfo [0].bTripleFusion = m [optTripleFusion].value;
	extraGameInfo [0].nWeaponDropMode = m [optWeaponDrop].value;
	GET_VAL (gameOpts->gameplay.bInventory, optInventory);
	GET_VAL (gameOpts->gameplay.bIdleAnims, optIdleAnims);
	GET_VAL (gameOpts->gameplay.nAIAwareness, optAwareness);
	GET_VAL (gameOpts->gameplay.bHeadlightOn, optHeadlight);
	}
else {
#if EXPMODE_DEFAULTS
	extraGameInfo [0].bFixedRespawns = 0;
	extraGameInfo [0].bDualMissileLaunch = 0;
	extraGameInfo [0].bDropAllMissiles = 0;
	extraGameInfo [0].bImmortalPowerups = 0;
	extraGameInfo [0].bMultiBosses = 1;
	extraGameInfo [0].bSmartWeaponSwitch = 0;
	extraGameInfo [0].nWeaponDropMode = 1;
	gameOpts->gameplay.bInventory = 0;
	gameOpts->gameplay.bIdleAnims = 0;
	gameOpts->gameplay.nAIAwareness = 0;
	gameOpts->gameplay.bHeadlightOn = 0;
#endif
	}
for (j = 0; j < 3; j++)
	if (m [optAutoSel + j].value) {
		gameOpts->gameplay.nAutoSelectWeapon = j;
		break;
		}
for (j = 0; j < 3; j++)
	if (m [optSnipeMode + j].value) {
		extraGameInfo [0].nZoomMode = j;
		break;
		}
if (!COMPETITION && EGI_FLAG (bSmokeGrenades, 0, 0, 0))
	LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX] = 4;
if (IsMultiGame)
	NetworkSendExtraGameInfo (NULL);
}

//------------------------------------------------------------------------------

static int nOptDebrisLife;

static char *pszMslTurnSpeeds [3];

void PhysicsOptionsCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem	*m;
	int			v;

if (gameOpts->app.bExpertMode) {
	m = menus + nSBoostOpt;
	v = m->value;
	if (extraGameInfo [0].nSpeedBoost != v) {
		extraGameInfo [0].nSpeedBoost = v;
		sprintf (m->text, TXT_SPEEDBOOST, extraGameInfo [0].nSpeedBoost * 10, '%');
		m->rebuild = 1;
		}

	m = menus + nFusionRampOpt;
	v = m->value + 2;
	if (extraGameInfo [0].nFusionPowerMod != v) {
		extraGameInfo [0].nFusionPowerMod = v;
		sprintf (m->text, TXT_FUSION_PWR, extraGameInfo [0].nFusionPowerMod * 50, '%');
		m->rebuild = 1;
		}

	m = menus + nMslTurnSpeedOpt;
	v = m->value;
	if (extraGameInfo [0].nMslTurnSpeed != v) {
		extraGameInfo [0].nMslTurnSpeed = v;
		sprintf (m->text, TXT_MSL_TURNSPEED, pszMslTurnSpeeds [v]);
		*key = -2;
		return;
		}

	m = menus + nSlowmoSpeedupOpt;
	v = m->value + 4;
	if (gameOpts->gameplay.nSlowMotionSpeedup != v) {
		gameOpts->gameplay.nSlowMotionSpeedup = v;
		sprintf (m->text, TXT_SLOWMOTION_SPEEDUP, (float) v / 2);
		m->rebuild = 1;
		return;
		}

	m = menus + nOptDebrisLife;
	v = m->value;
	if (gameOpts->render.nDebrisLife != v) {
		gameOpts->render.nDebrisLife = v;
		sprintf (m->text, TXT_DEBRIS_LIFE, nDebrisLife [v]);
		m->rebuild = -1;
		}
	}
}

//------------------------------------------------------------------------------

#define D2X_MENU_GAP 0

void PhysicsOptionsMenu ()
{
	tMenuItem m [20];
	int	i, opt = 0, choice = 0;
	int	optRobHits = -1, optWiggle = -1, optAutoLevel = -1,
			optFluidPhysics = -1, optHitAngles = -1, optShootMissiles = -1, optHitboxes = -1;
	char	szSpeedBoost [50], szMslTurnSpeed [50], szSlowmoSpeedup [50], szFusionPower [50], szDebrisLife [50];

pszMslTurnSpeeds [0] = TXT_SLOW;
pszMslTurnSpeeds [1] = TXT_MEDIUM;
pszMslTurnSpeeds [2] = TXT_STANDARD;

do {
	memset (&m, 0, sizeof (m));
	opt = 0;
	if (gameOpts->app.bExpertMode) {
		sprintf (szSpeedBoost + 1, TXT_SPEEDBOOST, extraGameInfo [0].nSpeedBoost * 10, '%');
		*szSpeedBoost = *(TXT_SPEEDBOOST - 1);
		ADD_SLIDER (opt, szSpeedBoost + 1, extraGameInfo [0].nSpeedBoost, 0, 10, KEY_B, HTX_GPLAY_SPEEDBOOST);
		nSBoostOpt = opt++;
		sprintf (szFusionPower + 1, TXT_FUSION_PWR, extraGameInfo [0].nFusionPowerMod * 50, '%');
		*szFusionPower = *(TXT_FUSION_PWR - 1);
		ADD_SLIDER (opt, szFusionPower + 1, extraGameInfo [0].nFusionPowerMod - 2, 0, 6, KEY_W, HTX_GPLAY_FUSIONPOWER);
		nFusionRampOpt = opt++;
		sprintf (szMslTurnSpeed + 1, TXT_MSL_TURNSPEED, pszMslTurnSpeeds [extraGameInfo [0].nMslTurnSpeed]);
		*szMslTurnSpeed = *(TXT_MSL_TURNSPEED - 1);
		ADD_SLIDER (opt, szMslTurnSpeed + 1, extraGameInfo [0].nMslTurnSpeed, 0, 2, KEY_T, HTX_GPLAY_MSL_TURNSPEED);
		nMslTurnSpeedOpt = opt++;
		sprintf (szSlowmoSpeedup + 1, TXT_SLOWMOTION_SPEEDUP, (float) gameOpts->gameplay.nSlowMotionSpeedup / 2);
		*szSlowmoSpeedup = *(TXT_SLOWMOTION_SPEEDUP - 1);
		ADD_SLIDER (opt, szSlowmoSpeedup + 1, gameOpts->gameplay.nSlowMotionSpeedup - 4, 0, 4, KEY_M, HTX_SLOWMOTION_SPEEDUP);
		nSlowmoSpeedupOpt = opt++;
		sprintf (szDebrisLife + 1, TXT_DEBRIS_LIFE, nDebrisLife [gameOpts->render.nDebrisLife]);
		*szDebrisLife = *(TXT_DEBRIS_LIFE - 1);
		ADD_SLIDER (opt, szDebrisLife, gameOpts->render.nDebrisLife, 0, 6, KEY_D, HTX_DEBRIS_LIFE);
		nOptDebrisLife = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_CHECK (opt, TXT_AUTO_LEVEL, gameOpts->gameplay.bAutoLeveling, KEY_L, HTX_MISC_AUTOLEVEL);
		optAutoLevel = opt++;
		ADD_CHECK (opt, TXT_WIGGLE_SHIP, extraGameInfo [0].bWiggle, KEY_W, HTX_MISC_WIGGLE);
		optWiggle = opt++;
		ADD_CHECK (opt, TXT_BOTS_HIT_BOTS, extraGameInfo [0].bRobotsHitRobots, KEY_O, HTX_GPLAY_BOTSHITBOTS);
		optRobHits = opt++;
		ADD_CHECK (opt, TXT_FLUID_PHYS, extraGameInfo [0].bFluidPhysics, KEY_Y, HTX_GPLAY_FLUIDPHYS);
		optFluidPhysics = opt++;
		ADD_CHECK (opt, TXT_USE_HITANGLES, extraGameInfo [0].bUseHitAngles, KEY_H, HTX_GPLAY_HITANGLES);
		optHitAngles = opt++;
		ADD_CHECK (opt, TXT_SHOOT_MISSILES, extraGameInfo [0].bShootMissiles, KEY_S, HTX_GPLAY_SHOOTMISSILES);
		optShootMissiles = opt++;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_RADIO (opt, TXT_HIT_SPHERES, 0, KEY_W, 1, HTX_GPLAY_HITBOXES);
		optHitboxes = opt++;
		ADD_RADIO (opt, TXT_SIMPLE_HITBOXES, 0, KEY_W, 1, HTX_GPLAY_HITBOXES);
		opt++;
		ADD_RADIO (opt, TXT_COMPLEX_HITBOXES, 0, KEY_W, 1, HTX_GPLAY_HITBOXES);
		opt++;
		m [optHitboxes + NMCLAMP (extraGameInfo [0].nHitboxes, 0, 2)].value = 1;
		}
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, TXT_PHYSICS_MENUTITLE, opt, m, &PhysicsOptionsCallback, &choice);
		} while (i >= 0);
	} while (i == -2);
if (gameOpts->app.bExpertMode) {
	gameOpts->gameplay.bAutoLeveling = m [0].value;
	extraGameInfo [0].bRobotsHitRobots = m [optRobHits].value;
	extraGameInfo [0].bShootMissiles = m [optShootMissiles].value;
	extraGameInfo [0].bFluidPhysics = m [optFluidPhysics].value;
	extraGameInfo [0].bUseHitAngles = m [optHitAngles].value;
	extraGameInfo [0].bWiggle = m [optWiggle].value;
	for (i = 0; i < 3; i++)
		if (m [optHitboxes + i].value) {
			extraGameInfo [0].nHitboxes = i;
			break;
			}
	for (i = 0; i < 2; i++)
		if (gameStates.gameplay.slowmo [i].fSpeed > (float) gameOpts->gameplay.nSlowMotionSpeedup / 2)
			gameStates.gameplay.slowmo [i].fSpeed = (float) gameOpts->gameplay.nSlowMotionSpeedup / 2;
		else if ((gameStates.gameplay.slowmo [i].fSpeed > 1) && 
					(gameStates.gameplay.slowmo [i].fSpeed < (float) gameOpts->gameplay.nSlowMotionSpeedup / 2))
			gameStates.gameplay.slowmo [i].fSpeed = (float) gameOpts->gameplay.nSlowMotionSpeedup / 2;
	}
else {
#if EXPMODE_DEFAULTS
	extraGameInfo [0].bRobotsHitRobots = 0;
	extraGameInfo [0].bShootMissiles = 0;
	extraGameInfo [0].bFluidPhysics = 1;
	extraGameInfo [0].nHitboxes = 0;
	extraGameInfo [0].bWiggle = 0;
#endif
	}
if (IsMultiGame)
	NetworkSendExtraGameInfo (NULL);
}

//------------------------------------------------------------------------------

void ConfigMenu ()
{
	tMenuItem m [20];
	int i, opt, choice = 0;
	int optSound, optConfig, optJoyCal, optDetails, optScrRes, optReorderPrim, optReorderSec, 
		 optToggles, optRender, optGameplay, optCockpit, optPhysics;

do {
	memset (m, 0, sizeof (m));
	optRender = optGameplay = optCockpit = -1;
	opt = 0;
	ADD_MENU (opt, TXT_SOUND_MUSIC, KEY_M, HTX_OPTIONS_SOUND);
	optSound = opt++;
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_MENU (opt, strupr (TXT_CONTROLS_), KEY_O, HTX_OPTIONS_CONFIG);
	optConfig = opt++;
#if defined (_WIN32) || defined (__linux__)
	optJoyCal = -1;
#else
	ADD_MENU (opt, TXT_CAL_JOYSTICK, KEY_J, HTX_OPTIONS_CALSTICK);
	optJoyCal = opt++;
#endif
	ADD_TEXT (opt, "", 0);
	opt++;
	if (gameStates.app.bNostalgia) {
		ADD_SLIDER (opt, TXT_BRIGHTNESS, GrGetPaletteGamma (), 0, 16, KEY_B, HTX_RENDER_BRIGHTNESS);
		optBrightness = opt++;
		}
	ADD_MENU (opt, TXT_DETAIL_LEVELS, KEY_D, HTX_OPTIONS_DETAIL);
	optDetails = opt++;
	ADD_MENU (opt, TXT_SCREEN_RES, KEY_S, HTX_OPTIONS_SCRRES);
	optScrRes = opt++;
	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_MENU (opt, TXT_PRIMARY_PRIO, KEY_P, HTX_OPTIONS_PRIMPRIO);
	optReorderPrim = opt++;
	ADD_MENU (opt, TXT_SECONDARY_PRIO, KEY_E, HTX_OPTIONS_SECPRIO);
	optReorderSec = opt++;
	ADD_MENU (opt, gameStates.app.bNostalgia ? TXT_TOGGLES : TXT_MISCELLANEOUS, gameStates.app.bNostalgia ? KEY_T : KEY_I, HTX_OPTIONS_MISC);
	optToggles = opt++;
	if (!gameStates.app.bNostalgia) {
		ADD_MENU (opt, TXT_COCKPIT_OPTS2, KEY_C, HTX_OPTIONS_COCKPIT);
		optCockpit = opt++;
		ADD_MENU (opt, TXT_RENDER_OPTS2, KEY_R, HTX_OPTIONS_RENDER);
		optRender = opt++;
		if (IsMultiGame && !IsCoopGame) 
			optPhysics =
			optGameplay = -1;
		else {
			ADD_MENU (opt, TXT_GAMEPLAY_OPTS2, KEY_G, HTX_OPTIONS_GAMEPLAY);
			optGameplay = opt++;
			ADD_MENU (opt, TXT_PHYSICS_MENUCALL, KEY_Y, HTX_OPTIONS_PHYSICS);
			optPhysics = opt++;
			}
		}

	i = ExecMenu1 (NULL, TXT_OPTIONS, opt, m, options_menuset, &choice);
	if (i >= 0) {
		if (i == optSound)
			SoundMenu ();			
		else if (i == optConfig)
			InputDeviceConfig ();			
		else if (i == optJoyCal)
			joydefs_calibrate ();		
		else if (i == optDetails)
			DetailLevelMenu (); 	
		else if (i == optScrRes)
			ScreenResMenu ();		
		else if (i == optReorderPrim)
			ReorderPrimary ();			
		else if (i == optReorderSec)
			ReorderSecondary ();		
		else if (i == optToggles)
			MiscellaneousMenu ();			
		else if (!gameStates.app.bNostalgia) {
			if (i == optCockpit)
				CockpitOptionsMenu ();			
			else if (i == optRender)
				RenderOptionsMenu ();			
			else if ((optGameplay >= 0) && (i == optGameplay))
				GameplayOptionsMenu ();        
			else if ((optPhysics >= 0) && (i == optPhysics))
				PhysicsOptionsMenu ();        
			}
		}
	} while (i > -1);
WritePlayerFile ();
}

//------------------------------------------------------------------------------

void SetRedbookVolume (int volume);

WIN (extern int RBCDROM_State);
WIN (static BOOL windigi_driver_off=FALSE);

static int optDigiVol, optMusicVol, optRedbook;

void SoundMenuCallback (int nitems, tMenuItem * items, int *last_key, int citem)
{
	nitems=nitems;          
	*last_key = *last_key;

if (gameConfig.nDigiVolume != items [optDigiVol].value)     {
	gameConfig.nDigiVolume = items [optDigiVol].value;
	DigiSetFxVolume ((gameConfig.nDigiVolume*32768)/8);
	DigiPlaySampleOnce (SOUND_DROP_BOMB, F1_0);
	}

if (items [optRedbook].value != gameStates.sound.bRedbookEnabled) {
	if (items[optRedbook].value && !gameOpts->sound.bUseRedbook) {
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_REDBOOK_DISABLED);
		items[optRedbook].value = 0;
		items[optRedbook].rebuild = 1;
		}
	else if ((gameStates.sound.bRedbookEnabled = items [optRedbook].value)) {
		if (gameStates.app.nFunctionMode == FMODE_MENU)
			SongsPlaySong (SONG_TITLE, 1);
		else if (gameStates.app.nFunctionMode == FMODE_GAME)
			PlayLevelSong (gameData.missions.nCurrentLevel);
		else
			Int3 ();

		if (items[optRedbook].value && !gameStates.sound.bRedbookPlaying) {
			gameStates.sound.bRedbookEnabled = 0;
			gameStates.menus.nInMenu = 0;
			ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_MUSIC_NOCD);
			gameStates.menus.nInMenu = 1;
			items [optRedbook].value = 0;
			items [optRedbook].rebuild = 1;
			}
		}
	items [optMusicVol].text = gameStates.sound.bRedbookEnabled ? TXT_CD_VOLUME : TXT_MIDI_VOLUME;
	items [optMusicVol].rebuild = 1;
	}

if (gameStates.sound.bRedbookEnabled) {
	if (gameConfig.nRedbookVolume != items [optMusicVol].value)   {
		gameConfig.nRedbookVolume = items[optMusicVol].value;
		SetRedbookVolume (gameConfig.nRedbookVolume);
		}
	}
else 
	{
	if (gameConfig.nMidiVolume != items [optMusicVol].value) {
		int bSongPlaying = (gameConfig.nMidiVolume > 0);

 		gameConfig.nMidiVolume = items [optMusicVol].value;
		DigiSetMidiVolume ((gameConfig.nMidiVolume*128)/8);
		if (gameConfig.nMidiVolume < 1)
			DigiPlayMidiSong (NULL, NULL, NULL, 1, 0);
		else if (!bSongPlaying) {
			DigiStopAllChannels ();
			if (gameStates.app.bGameRunning)
				PlayLevelSong (gameData.missions.nCurrentLevel);
			else
				SongsPlaySong (gameStates.sound.nCurrentSong, 1);
			}
		}
	}
// don't enable redbook for a non-apple demo version of the shareware demo
citem++;		//kill warning
}

//------------------------------------------------------------------------------

void SoundMenu ()
{
   tMenuItem m [6];
	int	i, opt, choice = 0, 
			optReverse,
			bSongPlaying = (gameConfig.nMidiVolume > 0);

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_SLIDER (opt, TXT_FX_VOLUME, gameConfig.nDigiVolume, 0, 8, KEY_F, HTX_ONLINE_MANUAL);
	optDigiVol = opt++;
	ADD_SLIDER (opt, gameStates.sound.bRedbookEnabled ? TXT_CD_VOLUME : TXT_MIDI_VOLUME, 
					gameStates.sound.bRedbookEnabled ? gameConfig.nRedbookVolume : gameConfig.nMidiVolume, 
					0, 8, KEY_M, HTX_ONLINE_MANUAL);
	optMusicVol = opt++;

	ADD_TEXT (opt, "", 0);
	opt++;
	ADD_CHECK (opt, TXT_REDBOOK_ENABLED, gameStates.sound.bRedbookPlaying, KEY_C, HTX_ONLINE_MANUAL);
	optRedbook = opt++;
	ADD_CHECK (opt, TXT_REVERSE_STEREO, gameConfig.bReverseChannels, KEY_R, HTX_ONLINE_MANUAL);
	optReverse = opt++;
	i = ExecMenu1 (NULL, TXT_SOUND_OPTS, opt, m, SoundMenuCallback, &choice);
	gameStates.sound.bRedbookEnabled = m [optRedbook].value;
	gameConfig.bReverseChannels = m [optReverse].value;
} while (i > -1);

if (gameConfig.nMidiVolume < 1)   {
		DigiPlayMidiSong (NULL, NULL, NULL, 0, 0);
	}
else if (!bSongPlaying)
	SongsPlaySong (gameStates.sound.nCurrentSong, 1);
}

//------------------------------------------------------------------------------

static int nDlTimeoutOpt, nAutoDlOpt, nExpModeOpt, nUseDefOpt, nCompSpeedOpt, nScreenShotOpt;
static char *pszCompSpeeds [5];
extern int screenShotIntervals [];

void MiscellaneousCallback (int nitems, tMenuItem * menus, int * key, int citem)
{
	tMenuItem * m;
	int				v;

if (!gameStates.app.bNostalgia) {
	m = menus + nUseDefOpt;
	v = m->value;
	if (gameStates.app.bUseDefaults != v) {
		gameStates.app.bUseDefaults = v;
		*key = -2;
		return;
		}
	if (gameStates.app.bUseDefaults) {
		m = menus + nCompSpeedOpt;
		v = m->value;
		if (gameStates.app.nCompSpeed != v)
			sprintf (m->text, TXT_COMP_SPEED, pszCompSpeeds [v]);
		m->rebuild = 1;
		}
	m = menus + nScreenShotOpt;
	v = m->value;
	if (gameOpts->app.nScreenShotInterval != v) {
		gameOpts->app.nScreenShotInterval = v;
		if (v)
			sprintf (m->text, TXT_SCREENSHOTS, screenShotIntervals [v]);
		else
			strcpy (m->text, TXT_NO_SCREENSHOTS);
		m->rebuild = 1;
		*key = -2;
		return;
		}
	}
m = menus + nExpModeOpt;
v = m->value;
if (gameOpts->app.bExpertMode != v) {
	gameOpts->app.bExpertMode = v;
	*key = -2;
	return;
	}
m = menus + nAutoDlOpt;
v = m->value;
if (extraGameInfo [0].bAutoDownload != v) {
	extraGameInfo [0].bAutoDownload = v;
	*key = -2;
	return;
	}
if (gameOpts->app.bExpertMode) {
	if (extraGameInfo [0].bAutoDownload) {
		m = menus + nDlTimeoutOpt;
		v = m->value;
		if (GetDlTimeout () != v) {
			v = SetDlTimeout (v);
			sprintf (m->text, TXT_AUTODL_TO, GetDlTimeoutSecs ());
			m->rebuild = 1;
			}
		}
	}
else
	SetDlTimeout (15);
}

//------------------------------------------------------------------------------

void MiscellaneousMenu ()
{
	tMenuItem m [20];
	int	i, opt, choice,
#if 0
			optFastResp, 
#endif
			optHeadlight, optEscort, optUseMacros,	optAutoLevel,
			optReticle, optMissileView, optGuided, optSmartSearch, optLevelVer, optDemoFmt;
#if UDP_SAFEMODE
	int	optSafeUDP;
#endif
	char  szDlTimeout [50];
	char  szCompSpeed [50];
	char	szScreenShots [50];

pszCompSpeeds [0] = TXT_VERY_SLOW;
pszCompSpeeds [1] = TXT_SLOW;
pszCompSpeeds [2] = TXT_MEDIUM;
pszCompSpeeds [3] = TXT_FAST;
pszCompSpeeds [4] = TXT_VERY_FAST;
do {
	i = opt = 0;
	memset (m, 0, sizeof (m));
	optReticle = optMissileView = optGuided = optSmartSearch = optLevelVer = optDemoFmt = -1;
	if (gameStates.app.bNostalgia) {
		ADD_CHECK (0, TXT_AUTO_LEVEL, gameOpts->gameplay.bAutoLeveling, KEY_L, HTX_MISC_AUTOLEVEL);
		optAutoLevel = opt++;
		ADD_CHECK (opt, TXT_SHOW_RETICLE, gameOpts->render.cockpit.bReticle, KEY_R, HTX_CPIT_SHOWRETICLE);
		optReticle = opt++;
		ADD_CHECK (opt, TXT_MISSILE_VIEW, gameOpts->render.cockpit.bMissileView, KEY_I, HTX_CPITMSLVIEW);
		optMissileView = opt++;
		ADD_CHECK (opt, TXT_GUIDED_MAINVIEW, gameOpts->render.cockpit.bGuidedInMainView, KEY_G, HTX_CPIT_GUIDEDVIEW);
		optGuided = opt++;
		ADD_CHECK (opt, TXT_HEADLIGHT_ON, gameOpts->gameplay.bHeadlightOn, KEY_H, HTX_MISC_HEADLIGHT);
		optHeadlight = opt++;
		}
	ADD_CHECK (opt, TXT_ESCORT_KEYS, gameOpts->gameplay.bEscortHotKeys, KEY_K, HTX_MISC_ESCORTKEYS);
	optEscort = opt++;
#if 0
	ADD_CHECK (opt, TXT_FAST_RESPAWN, gameOpts->gameplay.bFastRespawn, KEY_F, HTX_MISC_FASTRESPAWN);
	optFastResp = opt++;
#endif
	ADD_CHECK (opt, TXT_USE_MACROS, gameOpts->multi.bUseMacros, KEY_M, HTX_MISC_USEMACROS);
	optUseMacros = opt++;
	if (!(gameStates.app.bNostalgia || gameStates.app.bGameRunning)) {
#if UDP_SAFEMODE
		ADD_CHECK (opt, TXT_UDP_QUAL, extraGameInfo [0].bSafeUDP, KEY_Q, HTX_MISC_UDPQUAL);
		optSafeUDP=opt++;
#endif
		}
	if (!gameStates.app.bNostalgia) {
		if (gameOpts->app.bExpertMode) {
			ADD_CHECK (opt, TXT_SMART_SEARCH, gameOpts->menus.bSmartFileSearch, KEY_S, HTX_MISC_SMARTSEARCH);
			optSmartSearch = opt++;
			ADD_CHECK (opt, TXT_SHOW_LVL_VERSION, gameOpts->menus.bShowLevelVersion, KEY_V, HTX_MISC_SHOWLVLVER);
			optLevelVer = opt++;
			}
		else
			optSmartSearch =
			optLevelVer = -1;
		ADD_CHECK (opt, TXT_EXPERT_MODE, gameOpts->app.bExpertMode, KEY_X, HTX_MISC_EXPMODE);
		nExpModeOpt = opt++;
		ADD_CHECK (opt, TXT_OLD_DEMO_FORMAT, gameOpts->demo.bOldFormat, KEY_C, HTX_OLD_DEMO_FORMAT);
		optDemoFmt = opt++;
		}
	if (gameStates.app.bNostalgia < 2) {
		if (extraGameInfo [0].bAutoDownload && gameOpts->app.bExpertMode) {
			ADD_TEXT (opt, "", 0);
			opt++;
			}
		ADD_CHECK (opt, TXT_AUTODL_ENABLE, extraGameInfo [0].bAutoDownload, KEY_A, HTX_MISC_AUTODL);
		nAutoDlOpt = opt++;
		if (extraGameInfo [0].bAutoDownload && gameOpts->app.bExpertMode) {
			sprintf (szDlTimeout + 1, TXT_AUTODL_TO, GetDlTimeoutSecs ());
			*szDlTimeout = *(TXT_AUTODL_TO - 1);
			ADD_SLIDER (opt, szDlTimeout + 1, GetDlTimeout (), 0, MaxDlTimeout (), KEY_T, HTX_MISC_AUTODLTO);  
			nDlTimeoutOpt = opt++;
			}
		ADD_TEXT (opt, "", 0);
		opt++;
		if (gameOpts->app.nScreenShotInterval)
			sprintf (szScreenShots + 1, TXT_SCREENSHOTS, screenShotIntervals [gameOpts->app.nScreenShotInterval]);
		else
			strcpy (szScreenShots + 1, TXT_NO_SCREENSHOTS);
		*szScreenShots = *(TXT_SCREENSHOTS - 1);
		ADD_SLIDER (opt, szScreenShots + 1, gameOpts->app.nScreenShotInterval, 0, 7, KEY_S, HTX_MISC_SCREENSHOTS);  
		nScreenShotOpt = opt++;
		}
	if (!gameStates.app.bNostalgia) {
		if (gameStates.app.bUseDefaults || (extraGameInfo [0].bAutoDownload && gameOpts->app.bExpertMode)) {
			ADD_TEXT (opt, "", 0);
			opt++;
			}
		ADD_CHECK (opt, TXT_USE_DEFAULTS, gameStates.app.bUseDefaults, KEY_U, HTX_MISC_DEFAULTS);
		nUseDefOpt = opt++;
		if (gameStates.app.bUseDefaults) {
			sprintf (szCompSpeed + 1, TXT_COMP_SPEED, pszCompSpeeds [gameStates.app.nCompSpeed]);
			*szCompSpeed = *(TXT_COMP_SPEED - 1);
			ADD_SLIDER (opt, szCompSpeed + 1, gameStates.app.nCompSpeed, 0, 4, KEY_C, HTX_MISC_COMPSPEED);
			nCompSpeedOpt = opt++;
			}
		}
	Assert (sizeofa (m) >= opt);
	do {
		i = ExecMenu1 (NULL, gameStates.app.bNostalgia ? TXT_TOGGLES : TXT_MISC_TITLE, opt, m, MiscellaneousCallback, &choice);
	} while (i >= 0);
	if (gameStates.app.bNostalgia) {
		gameOpts->gameplay.bAutoLeveling = m [optAutoLevel].value;
		gameOpts->render.cockpit.bReticle = m [optReticle].value;
		gameOpts->render.cockpit.bMissileView = m [optMissileView].value;
		gameOpts->render.cockpit.bGuidedInMainView = m [optGuided].value;
		gameOpts->gameplay.bHeadlightOn = m [optHeadlight].value;
		}
	gameOpts->gameplay.bEscortHotKeys = m [optEscort].value;
	gameOpts->multi.bUseMacros = m [optUseMacros].value;
	if (!gameStates.app.bNostalgia) {
		gameOpts->app.bExpertMode = m [nExpModeOpt].value;
		gameOpts->demo.bOldFormat = m [optDemoFmt].value;
		if (gameOpts->app.bExpertMode) {
#if UDP_SAFEMODE
			if (!gameStates.app.bGameRunning)
				GET_VAL (extraGameInfo [0].bSafeUDP, optSafeUDP);
#endif
#if 0
			GET_VAL (gameOpts->gameplay.bFastRespawn, optFastResp);
#endif
			GET_VAL (gameOpts->menus.bSmartFileSearch, optSmartSearch);
			GET_VAL (gameOpts->menus.bShowLevelVersion, optLevelVer);
			}
		else {
#if EXPMODE_DEFAULTS
			extraGameInfo [0].bWiggle = 1;
#if 0
			gameOpts->gameplay.bFastRespawn = 0;
#endif
			gameOpts->menus.bSmartFileSearch = 1;
			gameOpts->menus.bShowLevelVersion = 1;
#endif
			}
		}
	if (gameStates.app.bNostalgia > 1)
		extraGameInfo [0].bAutoDownload = 0;
	else
		extraGameInfo [0].bAutoDownload = m [nAutoDlOpt].value;
	} while (i == -2);
if (!gameStates.app.bNostalgia && gameStates.app.bUseDefaults) {
	gameOpts->render.nMaxFPS = 60;
	gameOpts->render.bTransparentEffects = 1;
	gameOpts->render.color.nLightMapRange = 5;
	gameOpts->render.color.bMix = 1;
	gameOpts->render.color.bWalls = 1;
	gameOpts->render.cameras.bFitToWall = 0;
	gameOpts->render.cameras.nSpeed = 5000;
	gameOpts->ogl.bSetGammaRamp = 0;
	gameStates.ogl.nContrast = 8;
	switch (gameStates.app.nCompSpeed = m [nCompSpeedOpt].value) {
		case 0:
			gameOpts->render.color.bUseLightMaps = 0;
			gameOpts->render.nQuality = 1;
			gameOpts->render.cockpit.bTextGauges = 1;
			gameOpts->render.bDynLighting = 0;
			gameOpts->ogl.bLightObjects = 0;
			gameOpts->movies.nQuality = 0;
			gameOpts->movies.bResize = 0;
			gameOpts->render.shadows.nClip = 0;
			gameOpts->render.shadows.nReach = 0;
			extraGameInfo [0].bShadows = 0;
			extraGameInfo [0].bUseSmoke = 0;
			extraGameInfo [0].bUseCameras = 0;
			extraGameInfo [0].bRenderShield = 0;
			extraGameInfo [0].bThrusterFlames = 0;
			extraGameInfo [0].bDamageExplosions = 0;
		break;
		case 1:
			gameOpts->render.nQuality = 2;
			extraGameInfo [0].bUseSmoke = 1;
			gameOpts->render.smoke.bPlayers = 0;
			gameOpts->render.smoke.bRobots = 1;
			gameOpts->render.smoke.bMissiles = 1;
			gameOpts->render.smoke.bCollisions = 0;
			gameOpts->render.smoke.nDens [0] =
			gameOpts->render.smoke.nDens [1] =
			gameOpts->render.smoke.nDens [2] =
			gameOpts->render.smoke.nDens [3] = 0;
			gameOpts->render.smoke.nSize [0] =
			gameOpts->render.smoke.nSize [1] =
			gameOpts->render.smoke.nSize [2] =
			gameOpts->render.smoke.nSize [3] = 3;
			gameOpts->render.smoke.nLife [0] =
			gameOpts->render.smoke.nLife [1] =
			gameOpts->render.smoke.nLife [2] = 0;
			gameOpts->render.smoke.nLife [3] = 1;
			gameOpts->render.cockpit.bTextGauges = 1;
			gameOpts->render.bDynLighting = 0;
			gameOpts->ogl.bLightObjects = 0;
			extraGameInfo [0].bUseCameras = 1;
			gameOpts->render.cameras.nFPS = 5;
			gameOpts->movies.nQuality = 0;
			gameOpts->movies.bResize = 1;
			gameOpts->render.shadows.nClip = 0;
			gameOpts->render.shadows.nReach = 0;
			extraGameInfo [0].bShadows = 1;
			extraGameInfo [0].bRenderShield = 0;
			extraGameInfo [0].bThrusterFlames = 1;
			extraGameInfo [0].bDamageExplosions = 0;
			break;
		case 2:
			gameOpts->render.nQuality = 2;
			extraGameInfo [0].bUseSmoke = 1;
			gameOpts->render.smoke.bPlayers = 0;
			gameOpts->render.smoke.bRobots = 1;
			gameOpts->render.smoke.bMissiles = 1;
			gameOpts->render.smoke.bCollisions = 0;
			gameOpts->render.smoke.nDens [0] =
			gameOpts->render.smoke.nDens [1] =
			gameOpts->render.smoke.nDens [2] =
			gameOpts->render.smoke.nDens [3] = 1;
			gameOpts->render.smoke.nSize [0] =
			gameOpts->render.smoke.nSize [1] =
			gameOpts->render.smoke.nSize [2] =
			gameOpts->render.smoke.nSize [3] = 3;
			gameOpts->render.smoke.nLife [0] =
			gameOpts->render.smoke.nLife [1] =
			gameOpts->render.smoke.nLife [2] = 0;
			gameOpts->render.smoke.nLife [3] = 1;
			gameOpts->render.cockpit.bTextGauges = 0;
			gameOpts->render.bDynLighting = 1;
			gameOpts->ogl.bLightObjects = 0;
			gameOpts->ogl.nMaxLights = MAX_NEAREST_LIGHTS / 2;
			extraGameInfo [0].bUseCameras = 1;
			gameOpts->render.cameras.nFPS = 0;
			gameOpts->movies.nQuality = 0;
			gameOpts->movies.bResize = 1;
			gameOpts->render.shadows.nClip = 1;
			gameOpts->render.shadows.nReach = 1;
			extraGameInfo [0].bShadows = 1;
			extraGameInfo [0].bRenderShield = 1;
			extraGameInfo [0].bThrusterFlames = 1;
			extraGameInfo [0].bDamageExplosions = 1;
			break;
		case 3:
			gameOpts->render.nQuality = 3;
			extraGameInfo [0].bUseSmoke = 1;
			gameOpts->render.smoke.bPlayers = 1;
			gameOpts->render.smoke.bRobots = 1;
			gameOpts->render.smoke.bMissiles = 1;
			gameOpts->render.smoke.bCollisions = 0;
			gameOpts->render.smoke.nDens [0] =
			gameOpts->render.smoke.nDens [1] =
			gameOpts->render.smoke.nDens [2] =
			gameOpts->render.smoke.nDens [3] = 2;
			gameOpts->render.smoke.nSize [0] =
			gameOpts->render.smoke.nSize [1] =
			gameOpts->render.smoke.nSize [2] =
			gameOpts->render.smoke.nSize [3] = 3;
			gameOpts->render.smoke.nLife [0] =
			gameOpts->render.smoke.nLife [1] =
			gameOpts->render.smoke.nLife [2] = 0;
			gameOpts->render.smoke.nLife [3] = 2;
			gameOpts->render.cockpit.bTextGauges = 0;
			gameOpts->render.bDynLighting = 1;
			gameOpts->ogl.bLightObjects = 0;
			gameOpts->ogl.nMaxLights = MAX_NEAREST_LIGHTS * 3 / 4;
			extraGameInfo [0].bUseCameras = 1;
			gameOpts->render.cameras.nFPS = 0;
			gameOpts->movies.nQuality = 1;
			gameOpts->movies.bResize = 1;
			gameOpts->render.shadows.nClip = 1;
			gameOpts->render.shadows.nReach = 1;
			extraGameInfo [0].bShadows = 1;
			extraGameInfo [0].bRenderShield = 1;
			extraGameInfo [0].bThrusterFlames = 1;
			extraGameInfo [0].bDamageExplosions = 1;
			break;
		case 4:
			gameOpts->render.nQuality = 4;
			extraGameInfo [0].bUseSmoke = 1;
			gameOpts->render.smoke.bPlayers = 1;
			gameOpts->render.smoke.bRobots = 1;
			gameOpts->render.smoke.bMissiles = 1;
			gameOpts->render.smoke.bCollisions = 1;
			gameOpts->render.smoke.nDens [0] =
			gameOpts->render.smoke.nDens [1] =
			gameOpts->render.smoke.nDens [2] =
			gameOpts->render.smoke.nDens [3] = 3;
			gameOpts->render.smoke.nSize [0] =
			gameOpts->render.smoke.nSize [1] =
			gameOpts->render.smoke.nSize [2] =
			gameOpts->render.smoke.nSize [3] = 3;
			gameOpts->render.smoke.nLife [0] =
			gameOpts->render.smoke.nLife [1] =
			gameOpts->render.smoke.nLife [2] = 0;
			gameOpts->render.smoke.nLife [3] = 2;
			gameOpts->render.bDynLighting = 1;
			gameOpts->ogl.bLightObjects = 1;
			gameOpts->ogl.nMaxLights = MAX_NEAREST_LIGHTS;
			extraGameInfo [0].bUseCameras = 1;
			gameOpts->render.cockpit.bTextGauges = 0;
			gameOpts->render.cameras.nFPS = 0;
			gameOpts->movies.nQuality = 1;
			gameOpts->movies.bResize = 1;
			gameOpts->render.shadows.nClip = 1;
			gameOpts->render.shadows.nReach = 1;
			extraGameInfo [0].bShadows = 1;
			extraGameInfo [0].bRenderShield = 1;
			extraGameInfo [0].bThrusterFlames = 1;
			extraGameInfo [0].bDamageExplosions = 1;
			break;
		}
	}
}

//------------------------------------------------------------------------------

int QuitSaveLoadMenu (void)
{
	tMenuItem m [5];
	int	i, choice = 0, opt, optQuit, optOptions, optLoad, optSave;

memset (m, 0, sizeof (m));
opt = 0;
ADD_MENU (opt, TXT_QUIT_GAME, KEY_Q, HTX_QUIT_GAME);
optQuit = opt++;
ADD_MENU (opt, TXT_GAME_OPTIONS, KEY_O, HTX_MAIN_CONF);
optOptions = opt++;
ADD_MENU (opt, TXT_LOAD_GAME2, KEY_L, HTX_LOAD_GAME);
optLoad = opt++;
ADD_MENU (opt, TXT_SAVE_GAME2, KEY_S, HTX_SAVE_GAME);
optSave = opt++;
i = ExecMenu1 (NULL, TXT_ABORT_GAME, opt, m, NULL, &choice);
if (!i)
	return 0;
if (i == optOptions)
	ConfigMenu ();
else if (i == optLoad)
	StateRestoreAll (1, 0, NULL);
else if (i == optSave)
	StateSaveAll (0, 0, NULL);
return 1;
}

//------------------------------------------------------------------------------

void MultiplayerMenu ()
{
	int nMenuChoice [11];
	tMenuItem m [11];
	int choice = 0, opt = 0, i, optCreate, optJoin, optConn, nConnections;
	int old_game_mode;
	static int choiceMap [5][2] = {
		{MENU_START_IPX_NETGAME, MENU_JOIN_IPX_NETGAME},
		{MENU_START_UDP_NETGAME, MENU_JOIN_UDP_NETGAME},
		{MENU_START_UDP_TRACKER_NETGAME, MENU_JOIN_UDP_TRACKER_NETGAME},
		{MENU_START_MCAST4_NETGAME, MENU_JOIN_MCAST4_NETGAME},
		{MENU_START_KALI_NETGAME, MENU_JOIN_KALI_NETGAME}
	};

if (gameData.multiplayer.autoNG.bValid)
	ExecMenuOption (choiceMap [gameData.multiplayer.autoNG.uConnect][!gameData.multiplayer.autoNG.bHost]);
else {
	do {
		old_game_mode = gameData.app.nGameMode;
		memset (m, 0, sizeof (m));
		opt = 0;
		if (gameStates.app.bNostalgia < 2) {
			ADD_MENU (opt, TXT_CREATE_GAME, KEY_S, HTX_NETWORK_SERVER);
			optCreate = opt++;
			ADD_MENU (opt, TXT_JOIN_GAME, KEY_J, HTX_NETWORK_CLIENT);
			optJoin = opt++;
			ADD_TEXT (opt, "", 0);
			opt++;
			ADD_RADIO (opt, TXT_NGTYPE_IPX, 0, KEY_I, 0, HTX_NETWORK_IPX);
			optConn = opt++;
			ADD_RADIO (opt, TXT_NGTYPE_UDP, 0, KEY_U, 0, HTX_NETWORK_UDP);
			opt++;
			ADD_RADIO (opt, TXT_NGTYPE_TRACKER, 0, KEY_T, 0, HTX_NETWORK_TRACKER);
			opt++;
			ADD_RADIO (opt, TXT_NGTYPE_MCAST4, 0, KEY_M, 0, HTX_NETWORK_MCAST);
			opt++;
#ifdef KALINIX
			ADD_RADIO (opt, TXT_NGTYPE_KALI, 0, KEY_K, 0, HTX_NETWORK_KALI);
			opt++;
#endif
			nConnections = opt;
			m [optConn + NMCLAMP (gameStates.multi.nConnection, 0, nConnections - optConn)].value = 1;
			}
		else {
#ifdef NATIVE_IPX
			ADD_MENU (opt, TXT_START_IPX_NET_GAME,  -1, HTX_NETWORK_IPX);
			nMenuChoice [opt++] = MENU_START_IPX_NETGAME;
			ADD_MENU (opt, TXT_JOIN_IPX_NET_GAME, -1, HTX_NETWORK_IPX);
			nMenuChoice [opt++] = MENU_JOIN_IPX_NETGAME;
#endif //NATIVE_IPX
			//ADD_MENU (TXT_START_TCP_NET_GAME, MENU_START_TCP_NETGAME, -1);
			//ADD_MENU (TXT_JOIN_TCP_NET_GAME, MENU_JOIN_TCP_NETGAME, -1);
			ADD_MENU (opt, TXT_MULTICAST_START, KEY_M, HTX_NETWORK_MCAST);
			nMenuChoice [opt++] = MENU_START_MCAST4_NETGAME;
			ADD_MENU (opt, TXT_MULTICAST_JOIN, KEY_N, HTX_NETWORK_MCAST);
			nMenuChoice [opt++] = MENU_JOIN_MCAST4_NETGAME;
#ifdef KALINIX
			ADD_MENU (opt, TXT_KALI_START, KEY_K, HTX_NETWORK_KALI);
			nMenuChoice [opt++] = MENU_START_KALI_NETGAME;
			ADD_MENU (opt, TXT_KALI_JOIN, KEY_I, HTX_NETWORK_KALI);
			nMenuChoice [opt++] = MENU_JOIN_KALI_NETGAME;
#endif // KALINIX
			if (gameStates.app.bNostalgia > 2) {
				ADD_MENU (opt, TXT_MODEM_GAME2, KEY_G, HTX_NETWORK_MODEM);
				nMenuChoice [opt++] = MENU_START_SERIAL;
				}
			}
		i = ExecMenu1 (NULL, TXT_MULTIPLAYER, opt, m, NULL, &choice);
		if (i > -1) {      
			if (gameStates.app.bNostalgia > 1)
				i = nMenuChoice [choice];
			else {
				for (gameStates.multi.nConnection = 0; 
					  gameStates.multi.nConnection < nConnections; 
					  gameStates.multi.nConnection++)
					if (m [optConn + gameStates.multi.nConnection].value)
						break;
				i = choiceMap [gameStates.multi.nConnection][choice == optJoin];
				}
			ExecMenuOption (i);
			}
		if (old_game_mode != gameData.app.nGameMode)
			break;          // leave menu
		} while (i > -1);
	}	
}

//------------------------------------------------------------------------------
/*
 * IpxSetDriver was called do_network_init and located in main/inferno
 * before the change which allows the user to choose the network driver
 * from the game menu instead of having to supply command line args.
 */
void IpxSetDriver (int ipx_driver)
{
	IpxClose ();

if (!FindArg ("-nonetwork")) {
	int nIpxError;
	int socket = 0, t;

	if ((t = FindArg ("-socket")))
		socket = atoi (Args [t + 1]);
	ArchIpxSetDriver (ipx_driver);
	if ((nIpxError = IpxInit (IPX_DEFAULT_SOCKET + socket)) == IPX_INIT_OK) {
		networkData.bActive = 1;
		} 
	else {
#if 1 //TRACE
	switch (nIpxError) {
		case IPX_NOT_INSTALLED: 
			con_printf (CON_VERBOSE, "%s\n", TXT_NO_NETWORK); 
			break;
		case IPX_SOCKET_TABLE_FULL: 
			con_printf (CON_VERBOSE, "%s 0x%x.\n", TXT_SOCKET_ERROR, IPX_DEFAULT_SOCKET + socket); 
			break;
		case IPX_NO_LOW_DOS_MEM: 
			con_printf (CON_VERBOSE, "%s\n", TXT_MEMORY_IPX); 
			break;
		default: 
			con_printf (CON_VERBOSE, "%s %d", TXT_ERROR_IPX, nIpxError);
		}
		con_printf (CON_VERBOSE, "%s\n", TXT_NETWORK_DISABLED);
#endif
		networkData.bActive = 0;		// Assume no network
	}
	IpxReadUserFile ("descent.usr");
	IpxReadNetworkFile ("descent.net");
	} 
else {
#if 1 //TRACE
	con_printf (CON_VERBOSE, "%s\n", TXT_NETWORK_DISABLED);
#endif
	networkData.bActive = 0;		// Assume no network
	}
}

//------------------------------------------------------------------------------

void DoNewIPAddress ()
 {
  tMenuItem m [4];
  char IPText[30];
  int choice;

  	memset (m, 0, sizeof (m));
	m [0].nType=NM_TYPE_TEXT; 
  m [0].text = "Enter an address or hostname:";
  m [1].nType=NM_TYPE_INPUT; 
  m [1].text_len = 50; 
  m [1].text = IPText;
  IPText[0]=0;

  choice = ExecMenu (NULL, TXT_JOIN_TCP, 2, m, NULL, NULL);

  if (choice==-1 || m [1].text[0]==0)
   return;

  ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_INV_ADDRESS);
 }

//------------------------------------------------------------------------------
//eof
