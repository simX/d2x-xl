#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#ifdef RCS
static char rcsid [] = "$Id: network.c, v 1.24 2003/10/12 09:38:48 btb Exp $";
#endif

#define PATCH12

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#	include <winsock.h>
#else
#	include <sys/socket.h>
#endif

#include "inferno.h"
#include "strutil.h"
#include "args.h"
#include "timer.h"
#include "mono.h"
#include "ipx.h"
#include "newmenu.h"
#include "key.h"
#include "gauges.h"
#include "object.h"
#include "error.h"
#include "laser.h"
#include "gamesave.h"
#include "gamemine.h"
#include "player.h"
#include "gameseq.h"
#include "fireball.h"
#include "network.h"
#include "game.h"
#include "multi.h"
#include "endlevel.h"
#include "palette.h"
#include "cntrlcen.h"
#include "powerup.h"
#include "menu.h"
#include "sounds.h"
#include "text.h"
#include "kmatrix.h"
#include "newdemo.h"
#include "multibot.h"
#include "wall.h"
#include "bm.h"
#include "effects.h"
#include "physics.h"
#include "switch.h"
#include "automap.h"
#include "byteswap.h"
#include "netmisc.h"
#include "kconfig.h"
#include "playsave.h"
#include "cfile.h"
#include "ipx.h"
#include "ipx_udp.h"
#include "ipx_drv.h"
#include "autodl.h"
#include "tracker.h"
#include "newmenu.h"
#include "gamefont.h"
#include "gameseg.h"
#include "multi.h"
#include "collide.h"
#include "hudmsg.h"
#include "vers_id.h"

#define LHX(x)      (gameStates.menus.bHires?2* (x):x)
#define LHY(y)      (gameStates.menus.bHires? (24* (y))/10:y)

/* the following are the possible packet identificators.
 * they are stored in the "type" field of the packet structs.
 * they are offset 4 bytes from the beginning of the raw IPX data
 * because of the "driver's" ipx_packetnum (see linuxnet.c).
 */


char szNMTextBuffer [MAX_ACTIVE_NETGAMES + 5][100];

extern struct ipx_recv_data ipx_udpSrc;

extern void SetFunctionMode (int);
extern unsigned char ipx_MyAddress [10];

//------------------------------------------------------------------------------

void NetworkSetWeaponsAllowed (void)
 {
  int opt = 0, choice, opt_primary, opt_second, opt_power;
  newmenu_item m [40];
  
  opt_primary = opt;
  memset (m, 0, sizeof (m));
  ADD_CHECK (opt, TXT_WA_LASER, netGame.DoLaserUpgrade, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_SLASER, netGame.DoSuperLaser, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_QLASER, netGame.DoQuadLasers, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_VULCAN, netGame.DoVulcan, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_SPREAD, netGame.DoSpread, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_PLASMA, netGame.DoPlasma, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_FUSION, netGame.DoFusions, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_GAUSS, netGame.DoGauss, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_HELIX, netGame.DoHelix, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_PHOENIX, netGame.DoPhoenix, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_OMEGA, netGame.DoOmega, 0, NULL); opt++;
  
  opt_second = opt;   
  ADD_CHECK (opt, TXT_WA_HOMING_MSL, netGame.DoHoming, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_PROXBOMB, netGame.DoProximity, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_SMART_MSL, netGame.DoSmarts, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_MEGA_MSL, netGame.DoMegas, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_FLASH_MSL, netGame.DoFlash, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_GUIDED_MSL, netGame.DoGuided, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_SMARTMINE, netGame.DoSmartMine, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_MERC_MSL, netGame.DoMercury, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_SHAKER_MSL, netGame.DoEarthShaker, 0, NULL); opt++;

  opt_power = opt;
  ADD_CHECK (opt, TXT_WA_INVUL, netGame.DoInvulnerability, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_CLOAK, netGame.DoCloak, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_BURNER, netGame.DoAfterburner, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_AMMORACK, netGame.DoAmmoRack, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_CONVERTER, netGame.DoConverter, 0, NULL); opt++;
  ADD_CHECK (opt, TXT_WA_HEADLIGHT, netGame.DoHeadlight, 0, NULL); opt++;
  
  choice = ExecMenu (NULL, TXT_WA_OBJECTS, opt, m, NULL, NULL);

  netGame.DoLaserUpgrade = m [opt_primary].value; 
  netGame.DoSuperLaser = m [opt_primary+1].value;
  netGame.DoQuadLasers = m [opt_primary+2].value;  
  netGame.DoVulcan = m [opt_primary+3].value;
  netGame.DoSpread = m [opt_primary+4].value;
  netGame.DoPlasma = m [opt_primary+5].value;
  netGame.DoFusions = m [opt_primary+6].value;
  netGame.DoGauss = m [opt_primary+7].value;
  netGame.DoHelix = m [opt_primary+8].value;
  netGame.DoPhoenix = m [opt_primary+9].value;
  netGame.DoOmega = m [opt_primary+10].value;
  
  netGame.DoHoming = m [opt_second].value;
  netGame.DoProximity = m [opt_second+1].value;
  netGame.DoSmarts = m [opt_second+2].value;
  netGame.DoMegas = m [opt_second+3].value;
  netGame.DoFlash = m [opt_second+4].value;
  netGame.DoGuided = m [opt_second+5].value;
  netGame.DoSmartMine = m [opt_second+6].value;
  netGame.DoMercury = m [opt_second+7].value;
  netGame.DoEarthShaker = m [opt_second+8].value;

  netGame.DoInvulnerability = m [opt_power].value;
  netGame.DoCloak = m [opt_power+1].value;
  netGame.DoAfterburner = m [opt_power+2].value;
  netGame.DoAmmoRack = m [opt_power+3].value;
  netGame.DoConverter = m [opt_power+4].value;     
  netGame.DoHeadlight = m [opt_power+5].value;     
  
 }

//------------------------------------------------------------------------------

static int 
	optCapVirLim, optCapTimLim, optMaxVirCap, optBumpVirCap, optBashVirCap, optVirGenTim, 
	optVirLife, optVirStab, optRevRooms, optEnergyFill, optShieldFill, optShieldDmg, 
	optOvrTex, optBrRooms, optFF, optSuicide, optPlrHand, optTogglesMenu, optTextureMenu;

int opt_cinvul, opt_team_anarchy, opt_coop, optPlayersOnMap, opt_closed, opt_maxnet, 
	 opt_entropy, opt_monsterball, opt_entopts, opt_mballopts;
int last_cinvul = 0, last_maxnet, opt_hoard, opt_team_hoard;
int opt_refuse, opt_capture, optEnhancedCTF;

#define	WF(_f)	 ((short) (( (nFilter) & (1 << (_f))) != 0))

void SetAllAllowablesTo (int nFilter)
{
	last_cinvul = 0;   //default to zero
   
netGame.DoMegas = WF (0);
netGame.DoSmarts = WF (1);
netGame.DoFusions = WF (2);
netGame.DoHelix = WF (3);
netGame.DoPhoenix = WF (4);
netGame.DoCloak = WF (5);
netGame.DoInvulnerability = WF (6);
netGame.DoAfterburner = WF (7);
netGame.DoGauss = WF (8);
netGame.DoVulcan = WF (9);
netGame.DoPlasma = WF (10);
netGame.DoOmega = WF (11);
netGame.DoSuperLaser = WF (12);
netGame.DoProximity = WF (13);
netGame.DoSpread = WF (14);
netGame.DoMercury = WF (15);
netGame.DoSmartMine = WF (16);
netGame.DoFlash = WF (17);
netGame.DoGuided = WF (18);
netGame.DoEarthShaker = WF (19);
netGame.DoConverter = WF (20);
netGame.DoAmmoRack = WF (21);
netGame.DoHeadlight = WF (22);
netGame.DoHoming = WF (23);
netGame.DoLaserUpgrade = WF (24);
netGame.DoQuadLasers = WF (25);
netGame.BrightPlayers = !WF (26);
netGame.invul = WF (27);
}

#undef WF

//------------------------------------------------------------------------------

#define	WF(_w, _f)	if (_w) mpParams.nWeaponFilter |= (1 << (_f))

void GetAllAllowables (void)
{
mpParams.nWeaponFilter = 0;
WF (netGame.DoMegas, 0);
WF (netGame.DoSmarts, 1);
WF (netGame.DoFusions, 2);
WF (netGame.DoHelix, 3);
WF (netGame.DoPhoenix, 4);
WF (netGame.DoCloak, 5);
WF (netGame.DoInvulnerability, 6);
WF (netGame.DoAfterburner, 7);
WF (netGame.DoGauss, 8);
WF (netGame.DoVulcan, 9);
WF (netGame.DoPlasma, 10);
WF (netGame.DoOmega, 11);
WF (netGame.DoSuperLaser, 12);
WF (netGame.DoProximity, 13);
WF (netGame.DoSpread, 14);
WF (netGame.DoMercury, 15);
WF (netGame.DoSmartMine, 16);
WF (netGame.DoFlash, 17);
WF (netGame.DoGuided, 18);
WF (netGame.DoEarthShaker, 19);
WF (netGame.DoConverter, 20);
WF (netGame.DoAmmoRack, 21);
WF (netGame.DoHeadlight, 22);
WF (netGame.DoHoming, 23);
WF (netGame.DoLaserUpgrade, 24);
WF (netGame.DoQuadLasers, 25);
WF (!netGame.BrightPlayers, 26);
WF (netGame.invul, 27);
}

#undef WF

//------------------------------------------------------------------------------

#define ENDLEVEL_SEND_INTERVAL  (F1_0*2)
#define ENDLEVEL_IDLE_TIME      (F1_0*20)

void NetworkEndLevelPoll2 (int nitems, newmenu_item * menus, int * key, int citem)
{
	// Polling loop for End-of-level menu

	static fix t1 = 0;
	int i = 0;
	int num_ready = 0;
	int goto_secret = 0;

	// Send our endlevel packet at regular intervals
if (TimerGetApproxSeconds () > (t1+ENDLEVEL_SEND_INTERVAL)) {
	NetworkSendEndLevelPacket ();
	t1 = TimerGetApproxSeconds ();
	}
NetworkListen ();
for (i = 0; i < gameData.multi.nPlayers; i++) {
	if ((gameData.multi.players [i].connected != 1) && 
		 (gameData.multi.players [i].connected != 5) && 
		 (gameData.multi.players [i].connected != 6))
		num_ready++;
	if (gameData.multi.players [i].connected == 4)
		goto_secret = 1;                                        
	}
if (num_ready == gameData.multi.nPlayers) {// All players have checked in or are disconnected
	if (goto_secret)
		*key = -3;
	else
		*key = -2;
	}
}


//------------------------------------------------------------------------------

void NetworkEndLevelPoll3 (int nitems, newmenu_item * menus, int * key, int citem)
{
	// Polling loop for End-of-level menu
   int num_ready = 0, i;
 
if (TimerGetApproxSeconds () > (gameData.multi.xStartAbortMenuTime+ (F1_0 * 8)))
	*key = -2;
NetworkListen ();
for (i = 0; i < gameData.multi.nPlayers; i++)
	if ((gameData.multi.players [i].connected != 1) && (gameData.multi.players [i].connected != 5) && (gameData.multi.players [i].connected != 6))
		num_ready++;
if (num_ready == gameData.multi.nPlayers) // All players have checked in or are disconnected
	*key = -2;
}

//------------------------------------------------------------------------------

void NetworkStartPoll (int nitems, newmenu_item * menus, int * key, int citem)
{
	int i, n, nm;

	key = key;
	citem = citem;

Assert (networkData.nStatus == NETSTAT_STARTING);
if (!menus [0].value) {
	menus [0].value = 1;
	menus [0].rebuild = 1;
	}
for (i = 1; i < nitems; i++) {
	if ((i >= gameData.multi.nPlayers) && menus [i].value) {
		menus [i].value = 0;
		menus [i].rebuild = 1;
		}
	}
nm = 0;
for (i = 0; i < nitems; i++) {
	if (menus [i].value) {
		if (++nm > gameData.multi.nPlayers)   {
			menus [i].value = 0;
			menus [i].rebuild = 1;
			}
		}
	}
if (nm > gameData.multi.nMaxPlayers) {
	ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, "%s %d %s", TXT_SORRY_ONLY, gameData.multi.nMaxPlayers, TXT_NETPLAYERS_IN);
	// Turn off the last player highlighted
	for (i = gameData.multi.nPlayers; i > 0; i--)
		if (menus [i].value == 1) {
			menus [i].value = 0;
			menus [i].rebuild = 1;
			break;
			}
	}

//       if (nitems > MAX_PLAYERS) return; 

n = netGame.numplayers;
NetworkListen ();

if (n < netGame.numplayers) {
	DigiPlaySample (SOUND_HUD_MESSAGE, F1_0);
	if (gameOpts->multi.bNoRankings)
	   sprintf (menus [gameData.multi.nPlayers - 1].text, "%d. %-20s", gameData.multi.nPlayers, netPlayers.players [gameData.multi.nPlayers-1].callsign);
	else
	   sprintf (menus [gameData.multi.nPlayers - 1].text, "%d. %s%-20s", gameData.multi.nPlayers, pszRankStrings [netPlayers.players [gameData.multi.nPlayers-1].rank], netPlayers.players [gameData.multi.nPlayers-1].callsign);
	menus [gameData.multi.nPlayers - 1].rebuild = 1;
	if (gameData.multi.nPlayers <= gameData.multi.nMaxPlayers)
		menus [gameData.multi.nPlayers - 1].value = 1;
	} 
else if (n > netGame.numplayers) {
	// One got removed...
   DigiPlaySample (SOUND_HUD_KILL, F1_0);
	for (i = 0; i < gameData.multi.nPlayers; i++) {
		if (gameOpts->multi.bNoRankings)	
			sprintf (menus [i].text, "%d. %-20s", i+1, netPlayers.players [i].callsign);
		else
			sprintf (menus [i].text, "%d. %s%-20s", i+1, pszRankStrings [netPlayers.players [i].rank], netPlayers.players [i].callsign);
		menus [i].value = (i < gameData.multi.nMaxPlayers);
		menus [i].rebuild = 1;
		}
	for (i = gameData.multi.nPlayers; i<n; i++)  {
		sprintf (menus [i].text, "%d. ", i+1);          // Clear out the deleted entries...
		menus [i].value = 0;
		menus [i].rebuild = 1;
		}
   }
}

//------------------------------------------------------------------------------

void NetworkGameParamPoll (int nitems, newmenu_item * menus, int * key, int citem)
{
	static int oldmaxnet = 0;

if ((menus [opt_entropy].value == (opt_entopts < 0)) ||
	 (menus [opt_monsterball].value == (opt_mballopts < 0)))
	*key = -2;
//force restricted game for team games
//obsolete with D2X-W32 as it can assign players to teams automatically
//even in a match and progress, and allows players to switch teams
if (menus [opt_coop].value) {
	oldmaxnet = 1;
	if (menus [opt_maxnet].value>2)  {
		menus [opt_maxnet].value = 2;
		menus [opt_maxnet].redraw = 1;
		}
	if (menus [opt_maxnet].max_value>2) {
		menus [opt_maxnet].max_value = 2;
		menus [opt_maxnet].redraw = 1;
		}
	if (!(netGame.game_flags & NETGAME_FLAG_SHOW_MAP))
		netGame.game_flags |= NETGAME_FLAG_SHOW_MAP;
	if (netGame.PlayTimeAllowed || netGame.KillGoal) {
		netGame.PlayTimeAllowed = 0;
		netGame.KillGoal = 0;
		}
	}
else {// if !Coop game
	if (oldmaxnet) {
		oldmaxnet = 0;
		menus [opt_maxnet].value = 6;
		menus [opt_maxnet].max_value = 6;
		}
	}         
if (last_maxnet != menus [opt_maxnet].value)  {
	sprintf (menus [opt_maxnet].text, TXT_MAX_PLAYERS, menus [opt_maxnet].value+2);
	last_maxnet = menus [opt_maxnet].value;
	menus [opt_maxnet].rebuild = 1;
	}               
 }

//------------------------------------------------------------------------------

fix LastPTA;
int LastKillGoal;

// Jeez -- mac compiler can't handle all of these on the same decl line.
int optSetPower, optPlayTime, optKillGoal, optSocket, optMarkerView, optLight, optPlayersOnMap;
int optDifficulty, optPPS, optShortPkts, optBrightPlayers, optStartInvul;
int optDarkness, optTeamDoors, optMultiCheats, optTgtInd, optDmgInd, optFriendlyInd;
int optHeadlights, optPowerupLights, optSpotSize;
int optShowNames, optEnhancedCTF, optAutoTeams, optDualMiss, optRotateLevels, optDisableReactor;
int optMouseLook, optFastPitch, optSafeUDP, optTowFlags;

//------------------------------------------------------------------------------

void NetworkMoreOptionsPoll (int nitems, newmenu_item * menus, int * key, int citem)
{
	int	v, j;

v = menus [optDarkness].value;
if (v != extraGameInfo [1].bDarkness) {
	extraGameInfo [1].bDarkness = v;
	*key = -2;
	return;
	}
v = menus [optTgtInd].value;
if (v != (extraGameInfo [1].bTargetIndicators == 0)) {
	for (j = 0; j < 3; j++)
		if (menus [optTgtInd + j].value) {
			extraGameInfo [1].bTargetIndicators = j;
			break;
			}
	*key = -2;
	return;
	}
if (optHeadlights >= 0) {
	v = menus [optHeadlights].value;
	if (v == extraGameInfo [1].bHeadLights) {
		extraGameInfo [1].bHeadLights = !v;
		*key = -2;
		return;
		}
	}
if (optSpotSize >= 0) {
	v = menus [optSpotSize].value;
	if (v != extraGameInfo [1].nSpotSize) {
		extraGameInfo [1].nSpotSize =
		extraGameInfo [1].nSpotStrength = v;
		sprintf (menus [optSpotSize].text, TXT_SPOTSIZE, GT (664 + v));
		menus [optSpotSize].rebuild = 1;
		return;
		}
	}

if (last_cinvul != menus [opt_cinvul].value)   {
	sprintf (menus [opt_cinvul].text, "%s: %d %s", TXT_REACTOR_LIFE, menus [opt_cinvul].value*5, TXT_MINUTES_ABBREV);
	last_cinvul = menus [opt_cinvul].value;
	menus [opt_cinvul].rebuild = 1;
   }
  
if (menus [optPlayTime].value != LastPTA) {
#ifdef SHAREWARE
   LastPTA = 0;
   ExecMessageBox ("Sorry", 1, TXT_OK, "Registered version only!");
   menus [optPlayTime].value = 0;
   menus [optPlayTime].rebuild = 1;
   return;
#endif  

if (gameData.app.nGameMode & GM_MULTI_COOP) {
	LastPTA = 0;
	ExecMessageBox ("Sorry", NULL, 1, TXT_OK, TXT_COOP_ERROR);
	menus [optPlayTime].value = 0;
	menus [optPlayTime].rebuild = 1;
	return;
	}

	mpParams.nMaxTime = netGame.PlayTimeAllowed = menus [optPlayTime].value;
	sprintf (menus [optPlayTime].text, TXT_MAXTIME, netGame.PlayTimeAllowed*5, TXT_MINUTES_ABBREV);
	LastPTA = netGame.PlayTimeAllowed;
	menus [optPlayTime].rebuild = 1;
	}
if (menus [optKillGoal].value!= LastKillGoal) {
	#ifdef SHAREWARE
	ExecMessageBox ("Sorry", 1, TXT_OK, "Registered version only!");
	menus [optKillGoal].value = 0;
	menus [optKillGoal].rebuild = 1;
	LastKillGoal = 0;
	return;
	#endif         


if (gameData.app.nGameMode & GM_MULTI_COOP) {
	ExecMessageBox ("Sorry", NULL, 1, TXT_OK, TXT_COOP_ERROR);
	menus [optKillGoal].value = 0;
	menus [optKillGoal].rebuild = 1;
	LastKillGoal = 0;
	return;
	}

mpParams.nKillGoal = netGame.KillGoal = menus [optKillGoal].value;
	sprintf (menus [optKillGoal].text, TXT_KILLGOAL, netGame.KillGoal*5);
	LastKillGoal = netGame.KillGoal;
	menus [optKillGoal].rebuild = 1;
	}
}

//------------------------------------------------------------------------------

void NetworkMoreGameOptions ()
 {
  int		opt = 0, i, j;
  char	szPlayTime [80], szKillGoal [80], szInvul [50], szSpotSize [50],
			socket_string [6], packstring [6];
  newmenu_item m [40];

do {
	memset (m, 0, sizeof (m));
	opt = 0;
	ADD_SLIDER (opt, TXT_DIFFICULTY, mpParams.nDifficulty, 0, NDL - 1, KEY_D, HTX_GPLAY_DIFFICULTY); 
	optDifficulty = opt++;
	sprintf (szInvul + 1, "%s: %d %s", TXT_REACTOR_LIFE, mpParams.nReactorLife * 5, TXT_MINUTES_ABBREV);
	strupr (szInvul + 1);
	*szInvul = * (TXT_REACTOR_LIFE - 1);
	ADD_SLIDER (opt, szInvul + 1, mpParams.nReactorLife, 0, 10, KEY_R, HTX_MULTI2_REACTOR); 
	opt_cinvul = opt++;
	sprintf (szPlayTime + 1, TXT_MAXTIME, netGame.PlayTimeAllowed*5, TXT_MINUTES_ABBREV);
	*szPlayTime = * (TXT_MAXTIME - 1);
	ADD_SLIDER (opt, szPlayTime + 1, mpParams.nMaxTime, 0, 10, KEY_T, HTX_MULTI2_LVLTIME); 
	optPlayTime = opt++;
	sprintf (szKillGoal + 1, TXT_KILLGOAL, netGame.KillGoal*5);
	*szKillGoal = * (TXT_KILLGOAL - 1);
	ADD_SLIDER (opt, szKillGoal + 1, mpParams.nKillGoal, 0, 10, KEY_K, HTX_MULTI2_KILLGOAL);
	optKillGoal = opt++;
	ADD_CHECK (opt, TXT_INVUL_RESPAWN, mpParams.bInvul, KEY_I, HTX_MULTI2_INVUL);
	optStartInvul = opt++;
	ADD_CHECK (opt, TXT_MARKER_CAMS, mpParams.bMarkerView, KEY_C, HTX_MULTI2_MARKERCAMS);
	optMarkerView = opt++;
	ADD_CHECK (opt, TXT_KEEP_LIGHTS, mpParams.bAlwaysBright, KEY_L, HTX_MULTI2_KEEPLIGHTS);
	optLight = opt++;
	ADD_CHECK (opt, TXT_BRIGHT_SHIPS, mpParams.bBrightPlayers ? 0 : 1, KEY_S, HTX_MULTI2_BRIGHTSHIP);
	optBrightPlayers = opt++;
	ADD_CHECK (opt, TXT_SHOW_NAMES, mpParams.bShowAllNames, KEY_E, HTX_MULTI2_SHOWNAMES);
	optShowNames = opt++;
	ADD_CHECK (opt, TXT_SHOW_PLAYERS, mpParams.bShowPlayersOnAutomap, KEY_A, HTX_MULTI2_SHOWPLRS);
	optPlayersOnMap = opt++;
	if (!gameStates.app.bNostalgia) {
		ADD_CHECK (opt, TXT_FRIENDLY_FIRE, extraGameInfo [0].bFriendlyFire, KEY_F, HTX_MULTI2_FFIRE);
		optFF = opt++;
		ADD_CHECK (opt, TXT_NO_SUICIDE, extraGameInfo [0].bInhibitSuicide, KEY_U, HTX_MULTI2_SUICIDE);
		optSuicide = opt++;
		ADD_CHECK (opt, TXT_MOUSELOOK, extraGameInfo [1].bMouseLook, KEY_O, HTX_MULTI2_MOUSELOOK);
		optMouseLook = opt++;
		ADD_CHECK (opt, TXT_FASTPITCH, (extraGameInfo [1].bFastPitch == 1) ? 1 : 0, KEY_P, HTX_MULTI2_FASTPITCH);
		optFastPitch = opt++;
		ADD_CHECK (opt, TXT_DUAL_LAUNCH, extraGameInfo [1].bDualMissileLaunch, KEY_M, HTX_GPLAY_DUALLAUNCH);
		optDualMiss = opt++;
		ADD_CHECK (opt, TXT_AUTOBALANCE, extraGameInfo [0].bAutoBalanceTeams, KEY_B, HTX_MULTI2_BALANCE);
		optAutoTeams = opt++;
		ADD_CHECK (opt, TXT_TEAMDOORS, mpParams.bTeamDoors, KEY_T, HTX_TEAMDOORS);
		optTeamDoors = opt++;
		ADD_CHECK (opt, TXT_TOW_FLAGS, extraGameInfo [1].bFriendlyIndicators, KEY_F, HTX_TOW_FLAGS);
		optTowFlags = opt++;
		ADD_CHECK (opt, TXT_MULTICHEATS, mpParams.bEnableCheats, KEY_T, HTX_MULTICHEATS);
		optMultiCheats = opt++;
		ADD_CHECK (opt, TXT_MSN_CYCLE, extraGameInfo [1].bRotateLevels, KEY_Y, HTX_MULTI2_MSNCYCLE); 
		optRotateLevels = opt++;
	#if 0
		ADD_CHECK (opt, TXT_NO_REACTOR, extraGameInfo [1].bDisableReactor, KEY_R, HTX_MULTI2_NOREACTOR); 
		optDisableReactor = opt++;
	#endif
	#if UDP_SAFEMODE
		ADD_CHECK (opt, TXT_UDP_QUAL, extraGameInfo [0].bSafeUDP, KEY_Q, HTX_MISC_UDPQUAL);
		optSafeUDP = opt++;
	#endif
		}
	ADD_CHECK (opt, TXT_SHORT_PACKETS, mpParams.bShortPackets, KEY_H, HTX_MULTI2_SHORTPKTS);
	optShortPkts = opt++;
	if (gameStates.app.bNostalgia) 
		optDarkness =
		optTgtInd = -1;
	else {
		if (extraGameInfo [1].bDarkness) {
			ADD_TEXT (opt, "", 0);
			opt++;
			}	
		ADD_CHECK (opt, TXT_DARKNESS, extraGameInfo [1].bDarkness, KEY_D, HTX_DARKNESS);
		optDarkness = opt++;
		if (extraGameInfo [1].bDarkness) {
			ADD_CHECK (opt, TXT_POWERUPLIGHTS, !extraGameInfo [1].bPowerupLights, KEY_P, HTX_POWERUPLIGHTS);
			optPowerupLights = opt++;
			ADD_CHECK (opt, TXT_HEADLIGHTS, !extraGameInfo [1].bHeadLights, KEY_H, HTX_HEADLIGHTS);
			optHeadlights = opt++;
			if (extraGameInfo [1].bHeadLights) {
				sprintf (szSpotSize + 1, TXT_SPOTSIZE, GT (664 + extraGameInfo [1].nSpotSize));
				strupr (szSpotSize + 1);
				*szSpotSize = *(TXT_SPOTSIZE - 1);
				ADD_SLIDER (opt, szSpotSize + 1, extraGameInfo [1].nSpotSize, 0, 2, KEY_O, HTX_SPOTSIZE); 
				optSpotSize = opt++;
				}
			else
				optSpotSize = -1;
			}
		else
			optHeadlights =
			optPowerupLights =
			optSpotSize = -1;
		ADD_TEXT (opt, "", 0);
		opt++;
		ADD_RADIO (opt, TXT_TGTIND_NONE, 0, KEY_A, 1, HTX_CPIT_TGTIND);
		optTgtInd = opt++;
		ADD_RADIO (opt, TXT_TGTIND_SQUARE, 0, KEY_R, 1, HTX_CPIT_TGTIND);
		opt++;
		ADD_RADIO (opt, TXT_TGTIND_TRIANGLE, 0, KEY_T, 1, HTX_CPIT_TGTIND);
		opt++;
		m [optTgtInd + extraGameInfo [1].bTargetIndicators].value = 1;
		if (extraGameInfo [1].bTargetIndicators) {
			ADD_CHECK (opt, TXT_DMG_INDICATOR, extraGameInfo [1].bDamageIndicators, KEY_D, HTX_CPIT_DMGIND);
			optDmgInd = opt++;
			ADD_CHECK (opt, TXT_FRIENDLY_INDICATOR, extraGameInfo [1].bFriendlyIndicators, KEY_F, HTX_FRIENDLY_INDICATOR);
			optFriendlyInd = opt++;
			}
		else
			optDmgInd =
			optFriendlyInd = -1;
		ADD_TEXT (opt, "", 0);
		opt++;
		}
	ADD_MENU (opt, TXT_WAOBJECTS_MENU, KEY_O, HTX_MULTI2_OBJECTS);
	optSetPower = opt++;

	sprintf (socket_string, "%d", (gameStates.multi.nGameType == UDP_GAME) ? udpBasePort [1] + networkData.nSocket : networkData.nSocket);
	if (gameStates.multi.nGameType >= IPX_GAME) {
		ADD_TEXT (opt, TXT_SOCKET2, KEY_N);
		opt++;
		ADD_INPUT (opt, socket_string, 5, HTX_MULTI2_SOCKET);
		optSocket = opt++;
		}

	sprintf (packstring, "%d", mpParams.nPPS);
	ADD_TEXT (opt, TXT_PPS, KEY_P);
	opt++;
	ADD_INPUT (opt, packstring, 2, HTX_MULTI2_PPS);
	optPPS = opt++;

	LastKillGoal = netGame.KillGoal;
	LastPTA = mpParams.nMaxTime;

	do_menu:

	gameStates.app.nExtGameStatus = GAMESTAT_MORE_NETGAME_OPTIONS; 
	Assert (sizeofa (m) >= opt);
	i = ExecMenu1 (NULL, TXT_MORE_MPOPTIONS, opt, m, NetworkMoreOptionsPoll, 0);
	} while (i == -2);

   //mpParams.nReactorLife = atoi (szInvul)*60*F1_0;
mpParams.nReactorLife = m [opt_cinvul].value;
netGame.control_invul_time = mpParams.nReactorLife * 5 * F1_0 * 60;

if (i == optSetPower) {
	NetworkSetWeaponsAllowed ();
	GetAllAllowables ();
	goto do_menu;
	}
mpParams.nPPS = atoi (packstring);
if (mpParams.nPPS>20) {
	mpParams.nPPS = 20;
	ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_PPS_HIGH_ERROR);
}
else if (mpParams.nPPS<2) {
	ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_PPS_HIGH_ERROR);
	mpParams.nPPS = 2;      
}
netGame.nPacketsPerSec = mpParams.nPPS;
if (gameStates.multi.nGameType >= IPX_GAME) { 
	int newSocket = atoi (socket_string);
	if ((newSocket < -0xFFFF) || (newSocket > 0xFFFF))
		ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, 
							TXT_INV_SOCKET, 
							(gameStates.multi.nGameType == UDP_GAME) ? udpBasePort [1] : networkData.nSocket);
	else if (newSocket != networkData.nSocket) {
		networkData.nSocket = (gameStates.multi.nGameType == UDP_GAME) ? newSocket - UDP_BASEPORT : newSocket;
		IpxChangeDefaultSocket ((ushort) (IPX_DEFAULT_SOCKET + networkData.nSocket));
		}
	}

netGame.invul = m [optStartInvul].value;	
mpParams.bInvul = (ubyte) netGame.invul;
netGame.BrightPlayers = m [optBrightPlayers].value ? 0 : 1;
mpParams.bBrightPlayers = (ubyte) netGame.BrightPlayers;
extraGameInfo [1].bDarkness = (ubyte) m [optDarkness].value;
if (optDarkness >= 0) {
	if (mpParams.bDarkness = extraGameInfo [1].bDarkness) {
		extraGameInfo [1].bHeadLights = !m [optHeadlights].value;
		extraGameInfo [1].bPowerupLights = !m [optPowerupLights].value;
		}
	}
extraGameInfo [1].bTowFlags = (ubyte) m [optTowFlags].value;
extraGameInfo [1].bTeamDoors = (ubyte) m [optTeamDoors].value;
mpParams.bTeamDoors = extraGameInfo [1].bTeamDoors;
extraGameInfo [1].bEnableCheats = (ubyte) m [optMultiCheats].value;
mpParams.bEnableCheats = extraGameInfo [1].bEnableCheats;
mpParams.bShortPackets = netGame.bShortPackets = m [optShortPkts].value;
netGame.ShowAllNames = m [optShowNames].value;
mpParams.bShowAllNames = (ubyte) netGame.ShowAllNames;
NetworkAdjustMaxDataSize ();
//  extraGameInfo [0].bEnhancedCTF = (m [optEnhancedCTF].value != 0);

netGame.Allow_marker_view = m [optMarkerView].value;
mpParams.bMarkerView = (ubyte) netGame.Allow_marker_view;
netGame.AlwaysLighting = m [optLight].value; 
mpParams.bAlwaysBright = (ubyte) netGame.AlwaysLighting;
mpParams.nDifficulty = gameStates.app.nDifficultyLevel = m [optDifficulty].value;
if (mpParams.bShowPlayersOnAutomap = m [optPlayersOnMap].value)
	netGame.game_flags |= NETGAME_FLAG_SHOW_MAP;
else
	netGame.game_flags &= ~NETGAME_FLAG_SHOW_MAP;
if (!gameStates.app.bNostalgia) {
	extraGameInfo [0].bFriendlyFire = m [optFF].value;
	extraGameInfo [0].bInhibitSuicide = m [optSuicide].value;
	extraGameInfo [0].bAutoBalanceTeams = (m [optAutoTeams].value != 0);
#if UDP_SAFEMODE
	extraGameInfo [0].bSafeUDP = (m [optSafeUDP].value != 0);
#endif
	extraGameInfo [1].bMouseLook = m [optMouseLook].value;
	extraGameInfo [1].bFastPitch = m [optFastPitch].value ? 1 : 2;
	extraGameInfo [1].bDualMissileLaunch = m [optDualMiss].value;
	extraGameInfo [1].bRotateLevels = m [optRotateLevels].value;
	extraGameInfo [1].bDisableReactor = m [optDisableReactor].value;
	if (optTgtInd >= 0) {
		for (j = 0; j < 3; j++)
			if (m [optTgtInd + j].value) {
				extraGameInfo [1].bTargetIndicators = j;
				break;
				}
		GET_VAL (extraGameInfo [1].bDamageIndicators, optDmgInd);
		GET_VAL (extraGameInfo [1].bFriendlyIndicators, optFriendlyInd);
		}
	}
}

//------------------------------------------------------------------------------

#define SetTextOpt(_text) \
	m [opt].type = NM_TYPE_TEXT; \
	m [opt++].text = _text

#define SetInputOpt(_label, _text, _value, _len) \
	SetTextOpt (_label); \
	m [opt].type = NM_TYPE_INPUT; \
	sprintf (_text, "%d", _value); \
	m [opt].text = _text; \
	m [opt].value = _value; \
	m [opt].text_len = _len; \
	m [opt].szHelp = HTX_ONLINE_MANUAL

#define SetRadioOpt(_text, _group, _key) \
	m [opt].type = NM_TYPE_RADIO; \
	m [opt].text = _text; \
	m [opt].value = 0; \
	m [opt].group = _group; \
	m [opt++].key = _key; \
	m [opt].szHelp = HTX_ONLINE_MANUAL

#define SetCheckOpt(_text, _value, _key) \
	m [opt].type = NM_TYPE_CHECK; \
	m [opt].text = _text; \
	m [opt].value = _value; \
	m [opt].key = _key; \
	m [opt].szHelp = HTX_ONLINE_MANUAL

//------------------------------------------------------------------------------

void NetworkDummyCallback (int nitems, newmenu_item * menus, int * key, int citem) {}
  
void NetworkEntropyToggleOptions ()
{
	newmenu_item	m [12];
	int				opt = 0;

memset (m, 0, sizeof (m));

ADD_CHECK (opt, TXT_ENT_HANDICAP, extraGameInfo [0].entropy.bPlayerHandicap, KEY_H, HTX_ONLINE_MANUAL);
optPlrHand = opt++;
ADD_CHECK (opt, TXT_ENT_CONQWARN, extraGameInfo [0].entropy.bDoConquerWarning, KEY_W, HTX_ONLINE_MANUAL);
optRevRooms = opt++;
ADD_CHECK (opt, TXT_ENT_REVERT, extraGameInfo [0].entropy.bRevertRooms, KEY_R, HTX_ONLINE_MANUAL);
optRevRooms = opt++;
SetTextOpt ("");
SetTextOpt (TXT_ENT_VIRSTAB);
optVirStab = opt;
SetRadioOpt (TXT_ENT_VIRSTAB_DROP, 2, KEY_D);
SetRadioOpt (TXT_ENT_VIRSTAB_ENEMY, 2, KEY_T);
SetRadioOpt (TXT_ENT_VIRSTAB_TOUCH, 2, KEY_L);
SetRadioOpt (TXT_ENT_VIRSTAB_NEVER, 2, KEY_N);
m [optVirStab + extraGameInfo [0].entropy.nVirusStability].value = 1;

Assert (sizeofa (m) >= opt);
ExecMenu1 (NULL, TXT_ENT_TOGGLES, opt, m, NetworkDummyCallback, 0);

extraGameInfo [0].entropy.bRevertRooms = m [optRevRooms].value;
extraGameInfo [0].entropy.bPlayerHandicap = m [optPlrHand].value;
for (extraGameInfo [0].entropy.nVirusStability = 0; 
	  extraGameInfo [0].entropy.nVirusStability < 3; 
	  extraGameInfo [0].entropy.nVirusStability++)
	if (m [optVirStab + extraGameInfo [0].entropy.nVirusStability].value)
		break;
}

//------------------------------------------------------------------------------

void NetworkEntropyTextureOptions ()
{
	newmenu_item	m [7];
	int				opt = 0;

memset (m, 0, sizeof (m));

optOvrTex = opt;
SetRadioOpt (TXT_ENT_TEX_KEEP, 1, KEY_K);
SetRadioOpt (TXT_ENT_TEX_OVERRIDE, 1, KEY_O);
SetRadioOpt (TXT_ENT_TEX_COLOR, 1, KEY_C);
m [optOvrTex + extraGameInfo [0].entropy.nOverrideTextures].value = 1;
SetTextOpt ("");
ADD_CHECK (opt, TXT_ENT_TEX_BRIGHTEN, extraGameInfo [0].entropy.bBrightenRooms, KEY_B, HTX_ONLINE_MANUAL);
optBrRooms = opt++;

Assert (sizeofa (m) >= opt);
ExecMenu1 (NULL, TXT_ENT_TEXTURES, opt, m, NetworkDummyCallback, 0);

extraGameInfo [0].entropy.bBrightenRooms = m [optBrRooms].value;
for (extraGameInfo [0].entropy.nOverrideTextures = 0; 
	  extraGameInfo [0].entropy.nOverrideTextures < 3; 
	  extraGameInfo [0].entropy.nOverrideTextures++)
	if (m [optOvrTex + extraGameInfo [0].entropy.nOverrideTextures].value)
		break;
}

//------------------------------------------------------------------------------

void NetworkEntropyOptions (void)
{
	newmenu_item	m [25];
	int				i, opt = 0;
	char				szCapVirLim [10], szCapTimLim [10], szMaxVirCap [10], szBumpVirCap [10], 
						szBashVirCap [10], szVirGenTim [10], szVirLife [10], 
						szEnergyFill [10], szShieldFill [10], szShieldDmg [10];

memset (m, 0, sizeof (m));

SetInputOpt (TXT_ENT_VIRLIM, szCapVirLim, extraGameInfo [0].entropy.nCaptureVirusLimit, 3);
optCapVirLim = opt++;
SetInputOpt (TXT_ENT_CAPTIME, szCapTimLim, extraGameInfo [0].entropy.nCaptureTimeLimit, 3);
optCapTimLim = opt++;
SetInputOpt (TXT_ENT_MAXCAP, szMaxVirCap, extraGameInfo [0].entropy.nMaxVirusCapacity, 6);
optMaxVirCap = opt++;
SetInputOpt (TXT_ENT_BUMPCAP, szBumpVirCap, extraGameInfo [0].entropy.nBumpVirusCapacity, 3);
optBumpVirCap = opt++;
SetInputOpt (TXT_ENT_BASHCAP, szBashVirCap, extraGameInfo [0].entropy.nBashVirusCapacity, 3);
optBashVirCap = opt++;
SetInputOpt (TXT_ENT_GENTIME, szVirGenTim, extraGameInfo [0].entropy.nVirusGenTime, 3);
optVirGenTim = opt++;
SetInputOpt (TXT_ENT_VIRLIFE, szVirLife, extraGameInfo [0].entropy.nVirusLifespan, 3);
optVirLife = opt++;
SetInputOpt (TXT_ENT_EFILL, szEnergyFill, extraGameInfo [0].entropy.nEnergyFillRate, 3);
optEnergyFill = opt++;
SetInputOpt (TXT_ENT_SFILL, szShieldFill, extraGameInfo [0].entropy.nShieldFillRate, 3);
optShieldFill = opt++;
SetInputOpt (TXT_ENT_DMGRATE, szShieldDmg, extraGameInfo [0].entropy.nShieldDamageRate, 3);
optShieldDmg = opt++;

SetTextOpt ("");
optTogglesMenu = opt;
m [opt].type = NM_TYPE_MENU;  
m [opt].text = TXT_ENT_TGLMENU; 
m [opt++].key = KEY_E;
optTextureMenu = opt;
m [opt].type = NM_TYPE_MENU;  
m [opt].text = TXT_ENT_TEXMENU; 
m [opt++].key = KEY_T;
Assert (sizeofa (m) >= opt);

for (;;) {
	i = ExecMenu1 (NULL, "Entropy Options", opt, m, NetworkDummyCallback, 0);
	if (i == optTogglesMenu)
		NetworkEntropyToggleOptions ();
	else if (i == optTextureMenu)
		NetworkEntropyTextureOptions ();
	else
		break;
	}
extraGameInfo [0].entropy.nCaptureVirusLimit = (char) atol (m [optCapVirLim].text);
extraGameInfo [0].entropy.nCaptureTimeLimit = (char) atol (m [optCapTimLim].text);
extraGameInfo [0].entropy.nMaxVirusCapacity = (ushort) atol (m [optMaxVirCap].text);
extraGameInfo [0].entropy.nBumpVirusCapacity = (char) atol (m [optBumpVirCap].text);
extraGameInfo [0].entropy.nBashVirusCapacity = (char) atol (m [optBashVirCap].text);
extraGameInfo [0].entropy.nVirusGenTime = (char) atol (m [optVirGenTim].text);
extraGameInfo [0].entropy.nVirusLifespan = (char) atol (m [optVirLife].text);
extraGameInfo [0].entropy.nEnergyFillRate = (ushort) atol (m [optEnergyFill].text);
extraGameInfo [0].entropy.nShieldFillRate = (ushort) atol (m [optShieldFill].text);
extraGameInfo [0].entropy.nShieldDamageRate = (ushort) atol (m [optShieldDmg].text);
}

//------------------------------------------------------------------------------

static int nBonusOpt, nSizeModOpt, nPyroForceOpt;

void MonsterballMenuCallback (int nitems, newmenu_item * menus, int * key, int citem)
{
	newmenu_item * m;
	int				v;

m = menus + nPyroForceOpt;
v = m->value + 1;
if (v != extraGameInfo [0].monsterball.forces [24].nForce) {
	extraGameInfo [0].monsterball.forces [24].nForce = v;
	sprintf (m->text, TXT_MBALL_PYROFORCE, v);
	m->rebuild = 1;
	//*key = -2;
	return;
	}
m = menus + nBonusOpt;
v = m->value + 1;
if (v != extraGameInfo [0].monsterball.nBonus) {
	extraGameInfo [0].monsterball.nBonus = v;
	sprintf (m->text, TXT_GOAL_BONUS, v);
	m->rebuild = 1;
	//*key = -2;
	return;
	}
m = menus + nSizeModOpt;
v = m->value + 2;
if (v != extraGameInfo [0].monsterball.nSizeMod) {
	extraGameInfo [0].monsterball.nSizeMod = v;
	sprintf (m->text, TXT_MBALL_SIZE, v / 2, (v & 1) ? 5 : 0);
	m->rebuild = 1;
	//*key = -2;
	return;
	}
}

//------------------------------------------------------------------------------

static int optionToWeaponId [] = {
	LASER_ID, 
	LASER_ID + 1, 
	LASER_ID + 2, 
	LASER_ID + 3, 
	SPREADFIRE_ID, 
	VULCAN_ID, 
	PLASMA_ID, 
	FUSION_ID, 
	SUPER_LASER_ID, 
	SUPER_LASER_ID + 1, 
	HELIX_ID, 
	GAUSS_ID, 
	PHOENIX_ID, 
	OMEGA_ID, 
	FLARE_ID, 
	CONCUSSION_ID, 
	HOMING_ID, 
	SMART_ID, 
	MEGA_ID, 
	FLASH_ID, 
	GUIDEDMISS_ID, 
	MERCURY_ID, 
	EARTHSHAKER_ID, 
	EARTHSHAKER_MEGA_ID
	};

ubyte nOptionToForce [] = {
	5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 
	60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 175, 200, 250
	};

static char *szWeaponTexts [] = {
	"Laser 1", 
	"Laser 2", 
	"Laser 3", 
	"Laser 4", 
	"Spreadfire", 
	"Vulcan", 
	"Plasma", 
	"Fusion", 
	"Superlaser 1", 
	"Superlaser 2", 
	"Helix", 
	"Gauss", 
	"Phoenix", 
	"Omega", 
	"Flare", 
	"Concussion", 
	"Homing", 
	"Smart", 
	"Mega", 
	"Flash", 
	"Guided", 
	"Mercury", 
	"Earthshaker", 
	"Shaker Bomblet"
	};

static inline int ForceToOption (double dForce)
{
	int	i, h = sizeofa (nOptionToForce);

for (i = 0; i < h - 1; i++)
	if ((dForce >= nOptionToForce [i]) && (dForce < nOptionToForce [i + 1]))
		break;
return i;
}

extern short nMonsterballForces [];
extern short nMonsterballPyroForce;

void NetworkMonsterballOptions (void)
{
	newmenu_item		m [35];
	int					h, i, j, opt = 0, optDefaultForces;
	char					szBonus [60], szSize [60], szPyroForce [60];
	tMonsterballForce	*pf = extraGameInfo [0].monsterball.forces;

h = sizeofa (optionToWeaponId);
j = sizeofa (nOptionToForce);
memset (m, 0, sizeof (m));
for (i = opt = 0; i < h; i++, opt++, pf++) {
	ADD_SLIDER (opt, szWeaponTexts [i], ForceToOption (pf->nForce), 
				   0, j - 1, 0, NULL);
	if (pf->nWeaponId == FLARE_ID) {
		opt++;
		ADD_TEXT (opt, "", 0);
		}
	}
ADD_TEXT (opt, "", 0);
opt++;
sprintf (szPyroForce + 1, TXT_MBALL_PYROFORCE, pf->nForce);
*szPyroForce = *(TXT_MBALL_PYROFORCE - 1);
ADD_SLIDER (opt, szPyroForce + 1, pf->nForce - 1, 0, 9, 0, NULL);
nPyroForceOpt = opt++;
ADD_TEXT (opt, "", 0);
opt++;
sprintf (szBonus + 1, TXT_GOAL_BONUS, extraGameInfo [0].monsterball.nBonus);
*szBonus = *(TXT_GOAL_BONUS - 1);
ADD_SLIDER (opt, szBonus + 1, extraGameInfo [0].monsterball.nBonus - 1, 0, 9, 0, HTX_GOAL_BONUS);
nBonusOpt = opt++;
i = extraGameInfo [0].monsterball.nSizeMod;
sprintf (szSize + 1, TXT_MBALL_SIZE, i / 2, (i & 1) ? 5 : 0);
*szSize = *(TXT_MBALL_SIZE - 1);
ADD_SLIDER (opt, szSize + 1, extraGameInfo [0].monsterball.nSizeMod - 2, 0, 8, 0, HTX_MBALL_SIZE);
nSizeModOpt = opt++;
ADD_TEXT (opt, "", 0);
opt++;
ADD_MENU (opt, "Set default values", 0, NULL);
optDefaultForces = opt++;
Assert (sizeofa (m) >= opt);

for (;;) {
	i = ExecMenu1 (NULL, "Monsterball Impact Forces", opt, m, MonsterballMenuCallback, 0);
	if (i == -1)
		break;
	if (i != optDefaultForces)
		break;
	InitMonsterballSettings (&extraGameInfo [0].monsterball);
	pf = extraGameInfo [0].monsterball.forces;
	for (i = 0; i < h + 1; i++, pf++) {
		m [i].value = ForceToOption (pf->nForce);
		if (pf->nWeaponId == FLARE_ID)
			i++;
		}
	m [nPyroForceOpt].value = NMCLAMP (pf->nForce - 1, 0, 9);
	m [nSizeModOpt].value = extraGameInfo [0].monsterball.nSizeMod - 2;
	}
pf = extraGameInfo [0].monsterball.forces;
for (i = 0; i < h; i++, pf++)
	pf->nForce = nOptionToForce [m [i].value];
pf->nForce = m [nPyroForceOpt].value + 1;
extraGameInfo [0].monsterball.nBonus = m [nBonusOpt].value + 1;
extraGameInfo [0].monsterball.nSizeMod = m [nSizeModOpt].value + 2;
}

//------------------------------------------------------------------------------

int NetworkGetGameParams (int bAutoRun)
{
	int i, key, choice = 1;
	int opt, opt_name, opt_level, opt_level_num, opt_mode, opt_moreopts, 
		 opt_access, opt_mission, opt_mission_name;
	newmenu_item m [35];
	char name [NETGAME_NAME_LEN+1];
	char level_text [32];
	char szMaxNet [50];
	char szIpAddr [80];
	char szLevel [5];

	int nNewMission = gameData.missions.nLastMission;
	int bAnarchyOnly = 0;

	SetAllAllowablesTo (mpParams.nWeaponFilter);
	networkData.nNamesInfoSecurity = -1;

	if (nNewMission >= 0)
		bAnarchyOnly = gameData.missions.list[nNewMission].anarchy_only_flag;
	for (i = 0;i<MAX_PLAYERS;i++)
		if (i!= gameData.multi.nLocalPlayer)
			gameData.multi.players [i].callsign [0] = 0;

	gameData.multi.nMaxPlayers = MAX_NUM_NET_PLAYERS;
	//netGame.KillGoal = 0;
	//netGame.PlayTimeAllowed = 0;
	//netGame.Allow_marker_view = 1;
#if 0 // can be called via menu option now so you don't need to chose a level if you have one already
	nNewMission = MultiChooseMission (&bAnarchyOnly);
	if (nNewMission < 0)
		return -1;
#endif
	if (!(FindArg ("-pps") && FindArg ("-shortpackets")))
		if (!NetworkChooseConnect ())
			return -1;

	sprintf (name, "%s%s", gameData.multi.players [gameData.multi.nLocalPlayer].callsign, TXT_S_GAME);	
	if (bAutoRun)
		return 1;

build_menu:
	sprintf (szLevel, "%d", mpParams.nLevel);

	memset (m, 0, sizeof (m));
	opt = 0;
	if (gameStates.multi.nGameType == UDP_GAME) {
		if (UDPGetMyAddress () < 0) 
			strcpy (szIpAddr, TXT_IP_FAIL);
		else {
			sprintf (szIpAddr, "Game Host: %d.%d.%d.%d:%d", 
						ipx_MyAddress [4], 
						ipx_MyAddress [5], 
						ipx_MyAddress [6], 
						ipx_MyAddress [7], 
						udpBasePort [1]);
			}
		}
	ADD_TEXT (opt, TXT_DESCRIPTION, 0); 
	opt++;
	ADD_INPUT (opt, name, NETGAME_NAME_LEN, HTX_MULTI_NAME); 
	opt_name = opt++;
	ADD_MENU (opt, TXT_SEL_MISSION, KEY_I, HTX_MULTI_MISSION);
	opt_mission = opt++;
	ADD_TEXT (opt, "", 0);
	m [opt].rebuild = 1; 
	opt_mission_name = opt++;

//      if (gameData.missions.nLastSecretLevel < -1)
//              sprintf (level_text+strlen (level_text)-1, ", S1-S%d)", -gameData.missions.nLastSecretLevel);
//      else if (gameData.missions.nLastSecretLevel == -1)
//              sprintf (level_text+strlen (level_text)-1, ", S1)");

	if ((nNewMission >= 0) && (gameData.missions.nLastLevel > 1)) {
		ADD_TEXT (opt, level_text, 0); 
		opt_level_num = opt++;
		ADD_INPUT (opt, szLevel, 4, HTX_MULTI_LEVEL);
		opt_level = opt++;
		}
//	m [opt].type = NM_TYPE_TEXT; m [opt].text = TXT_OPTIONS; opt++;
	
	ADD_RADIO (opt, TXT_ANARCHY, 0, KEY_A, 0, HTX_MULTI_ANARCHY);
	opt_mode = opt++;
	ADD_RADIO (opt, TXT_TEAM_ANARCHY, 0, KEY_T, 0, HTX_MULTI_TEAMANA);
	opt_team_anarchy = opt++;
	ADD_RADIO (opt, TXT_ANARCHY_W_ROBOTS, 0, KEY_R, 0, HTX_MULTI_BOTANA);
	opt++;
	ADD_RADIO (opt, TXT_COOP, 0, KEY_P, 0, HTX_MULTI_COOP);
	opt_coop = opt++;
	ADD_RADIO (opt, TXT_CTF, 0, KEY_F, 0, HTX_MULTI_CTF);
	opt_capture = opt++;
	if (!gameStates.app.bNostalgia) {
		ADD_RADIO (opt, TXT_CTF_PLUS, 0, KEY_T, 0, HTX_MULTI_CTFPLUS);
		optEnhancedCTF = opt++;
		}
   
	opt_entropy =
	opt_monsterball = -1;
	if (HoardEquipped ()) {
		ADD_RADIO (opt, TXT_HOARD, 0, KEY_H, 0, HTX_MULTI_HOARD);
		opt_hoard = opt++;
		ADD_RADIO (opt, TXT_TEAM_HOARD, 0, KEY_H, 0, HTX_MULTI_TEAMHOARD);
		opt_team_hoard = opt++;
		if (!gameStates.app.bNostalgia) {
			ADD_RADIO (opt, TXT_ENTROPY, 0, KEY_Y, 0, HTX_MULTI_ENTROPY);
			opt_entropy = opt++;
			ADD_RADIO (opt, TXT_MONSTERBALL, 0, KEY_B, 0, HTX_MULTI_MONSTERBALL);
			opt_monsterball = opt++;
			}
		} 
	ADD_TEXT (opt, "", 0); 
	opt++; 

	m [opt_mode + NMCLAMP (mpParams.nGameType, 0, opt - opt_mode - 1)].value = 1;
	 
	ADD_RADIO (opt, TXT_OPEN_GAME, 0, KEY_O, 1, HTX_MULTI_OPENGAME);
	opt_access = opt++;
	ADD_RADIO (opt, TXT_CLOSED_GAME, 0, KEY_C, 1, HTX_MULTI_CLOSEDGAME);
	opt_closed = opt++;
   ADD_RADIO (opt, TXT_RESTR_GAME, 0, KEY_R, 1, HTX_MULTI_RESTRGAME);
   opt_refuse = opt++;

	m [opt_access + NMCLAMP (mpParams.nGameAccess, 0, 2)].value = 1;

//      m [opt].type = NM_TYPE_CHECK; m [opt].text = TXT_SHOW_IDS; m [opt].value = 0; opt++;

	ADD_TEXT (opt, "", 0);
	opt++; 
   sprintf (szMaxNet + 1, TXT_MAX_PLAYERS, gameData.multi.nMaxPlayers);
	*szMaxNet = * (TXT_MAX_PLAYERS - 1);
   last_maxnet = gameData.multi.nMaxPlayers - 2;
   ADD_SLIDER (opt, szMaxNet + 1, last_maxnet, 0, last_maxnet, KEY_X, HTX_MULTI_MAXPLRS); 
   opt_maxnet = opt++;
	ADD_TEXT (opt, "", 0);
	opt++; 
   ADD_MENU (opt, TXT_MORE_OPTS, KEY_M, HTX_MULTI_MOREOPTS);
   opt_moreopts = opt++;
	opt_entopts =
	opt_mballopts = -1;
	if (!gameStates.app.bNostalgia) {
		if (m [opt_entropy].value) {
			ADD_MENU (opt, TXT_ENTROPY_OPTS, KEY_E, HTX_MULTI_ENTOPTS);
			opt_entopts = opt++;
			}
		if (m [opt_monsterball].value) {
		   ADD_MENU (opt, TXT_MONSTERBALL_OPTS, KEY_O, HTX_MULTI_MBALLOPTS);
			opt_mballopts = opt++;
			}
		}

do_menu:
	if (m [opt_mission_name].rebuild) {
		strncpy (netGame.mission_name, 
					 (nNewMission < 0) ? "" : gameData.missions.list [nNewMission].filename, 
					sizeof (netGame.mission_name) - 1);
		m [opt_mission_name].text = 
			 (nNewMission < 0) ? 
			TXT_NONE_SELECTED : 
			gameData.missions.list [nNewMission].mission_name;
		if ((nNewMission >= 0) && (gameData.missions.nLastLevel > 1)) {
			sprintf (level_text, "%s (1-%d)", TXT_LEVEL_, gameData.missions.nLastLevel);
			Assert (strlen (level_text) < 32);
			m [opt_level_num].rebuild = 1;
			}
		mpParams.nLevel = 1;
		}

   gameStates.app.nExtGameStatus = GAMESTAT_NETGAME_OPTIONS; 
	Assert (sizeofa (m) >= opt);
	key = ExecMenu1 (NULL, (gameStates.multi.nGameType == UDP_GAME) ? szIpAddr : NULL, 
						  opt, m, NetworkGameParamPoll, &choice);
									//TXT_NETGAME_SETUP
	if (key == -1)
		return -1;
   else if (choice == opt_moreopts) {
		if (m [opt_mode+3].value)
			gameData.app.nGameMode = GM_MULTI_COOP;
			NetworkMoreGameOptions ();
			gameData.app.nGameMode = 0;
			if (gameStates.multi.nGameType == UDP_GAME) {
				sprintf (szIpAddr, "Game Host: %d.%d.%d.%d:%d", 
				ipx_MyAddress [4], 
				ipx_MyAddress [5], 
				ipx_MyAddress [6], 
				ipx_MyAddress [7], 
				udpBasePort [1]);
				}
		  goto do_menu;
		}
	else if (!gameStates.app.bNostalgia && (opt_entopts >= 0) && (choice == opt_entopts)) {
		NetworkEntropyOptions ();
		goto do_menu;
		}
	else if (!gameStates.app.bNostalgia && (opt_mballopts >= 0) && (choice == opt_mballopts)) {
		NetworkMonsterballOptions ();
		goto do_menu;
		}
	else if (choice == opt_mission) {
		int h = MultiChooseMission (&bAnarchyOnly);
		if (h < 0)
			goto do_menu;
		gameData.missions.nLastMission = nNewMission = h;
		m [opt_mission_name].rebuild = 1;
		goto build_menu;
		}
  netGame.RefusePlayers = m [opt_refuse].value;

	if (key != -1) {
		int j;
		      
   gameData.multi.nMaxPlayers = m [opt_maxnet].value+2;
   netGame.max_numplayers = gameData.multi.nMaxPlayers;
				
	for (j = 0; j < networkData.nActiveGames; j++)
		if (!stricmp (activeNetGames [j].game_name, name)) {
			ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_DUPLICATE_NAME);
			goto do_menu;
		}
	strncpy (mpParams.szGameName, name, sizeof (mpParams.szGameName));
	mpParams.nLevel = atoi (szLevel);
	if ((gameData.missions.nLastLevel > 0) && ((mpParams.nLevel < 1) || (mpParams.nLevel > gameData.missions.nLastLevel))) {
		ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_LEVEL_OUT_RANGE);
		sprintf (szLevel, "1");
		goto do_menu;
	}

	for (i = opt_mode; i < opt_access; i++)
		if (m [i].value) {
			mpParams.nGameType = i - opt_mode;
			break;
			}

	for (i = opt_access; i < opt_maxnet; i++)
		if (m [i].value) {
			mpParams.nGameAccess = i - opt_access;
			break;
			}

	if (m [opt_mode].value)
		mpParams.nGameMode = NETGAME_ANARCHY;

#ifdef SHAREWARE
	else {
		ExecMessageBox (TXT_SORRY, 1, TXT_OK, TXT_REGISTERED_ONLY);
		m [opt_mode+1].value = 0;
		m [opt_mode+2].value = 0;
		m [opt_mode+3].value = 0;
		if (HoardEquipped ())
			m [opt_mode+4].value = 0;

		m [opt_mode].value = 1;
		goto do_menu;
		}
#else
	else if (m [opt_mode+1].value) {
		mpParams.nGameMode = NETGAME_TEAM_ANARCHY;
		}
	else if (m [opt_capture].value) {
		mpParams.nGameMode = NETGAME_CAPTURE_FLAG;
		extraGameInfo [0].bEnhancedCTF = 0;
		}
	else if (m [optEnhancedCTF].value) {
		mpParams.nGameMode = NETGAME_CAPTURE_FLAG;
		extraGameInfo [0].bEnhancedCTF = 1;
		}
	else if (HoardEquipped () && m [opt_hoard].value)
		mpParams.nGameMode = NETGAME_HOARD;
	else if (HoardEquipped () && m [opt_team_hoard].value)
		mpParams.nGameMode = NETGAME_TEAM_HOARD;
	else if (HoardEquipped () && m [opt_entropy].value)
		mpParams.nGameMode = NETGAME_ENTROPY;
	else if (HoardEquipped () && m [opt_monsterball].value)
		mpParams.nGameMode = NETGAME_MONSTERBALL;
	else if (bAnarchyOnly) {
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_ANARCHY_ONLY_MISSION);
		m [opt_mode+2].value = 0;
		m [opt_mode+3].value = 0;
		m [opt_mode].value = 1;
		goto do_menu;
		}               
	else if (m [opt_mode+2].value) 
		mpParams.nGameMode = NETGAME_ROBOT_ANARCHY;
	else if (m [opt_mode+3].value) 
		mpParams.nGameMode = NETGAME_COOPERATIVE;
	else Int3 (); // Invalid mode -- see Rob
#endif
	if (m [opt_closed].value)
		netGame.game_flags |= NETGAME_FLAG_CLOSED;
	}
	if (key == -2)
		goto build_menu;

	if (nNewMission < 0) {
		ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, "Please chose a mission");
		goto do_menu;
		}

	netGame.mission_name [sizeof (netGame.mission_name) - 1] = '\0';
	strcpy (netGame.mission_title, gameData.missions.list [nNewMission].mission_name + (gameOpts->menus.bShowLevelVersion ? 4 : 0));
	netGame.control_invul_time = mpParams.nReactorLife * 5 * F1_0 * 60;
	IpxChangeDefaultSocket ((ushort) (IPX_DEFAULT_SOCKET + networkData.nSocket));
	return key;
}

//------------------------------------------------------------------------------

static time_t	nQueryTimeout;

static void QueryPoll (int nItems, newmenu_item *m, int *key, int cItem)
{
	time_t t;

if (NetworkListen () && (networkData.nActiveGames >= MAX_ACTIVE_NETGAMES))
	*key = -2;
else if (*key == KEY_ESC)
	*key = -3;
else if ((t = SDL_GetTicks () - nQueryTimeout) > 5000)
	*key = -4;
else {
	int v = (int) (t / 5);
	if (m [0].value != v) {
		m [0].value = v;
		m [0].rebuild = 1;
		}
	*key = 0;
	}
return;
}

//------------------------------------------------------------------------------

int NetworkFindGame (void)
{
	newmenu_item	m [3];
	int i;

if (gameStates.multi.nGameType > IPX_GAME)
	return 0;

networkData.nStatus = NETSTAT_BROWSING;
networkData.nActiveGames = 0;
NetworkSendGameListRequest ();

memset (m, 0, sizeof (m));
ADD_GAUGE (0, "                    ", 0, 1000); 
ADD_TEXT (1, "", 0);
ADD_TEXT (2, TXT_PRESS_ESC, 0);
m [2].centered = 1;
nQueryTimeout = SDL_GetTicks ();
do {
	i = ExecMenu2 (NULL, TXT_NET_SEARCH, 3, m, (void (*)) QueryPoll, 0, NULL);
	} while (i >= 0);
return (networkData.nActiveGames >= MAX_ACTIVE_NETGAMES);
}

//------------------------------------------------------------------------------

int NetworkSelectTeams (void)
{
#ifndef SHAREWARE
	newmenu_item m [MAX_PLAYERS+4];
	int choice, opt, opt_team_b;
	ubyte team_vector = 0;
	char team_names [2] [CALLSIGN_LEN+1];
	int i;
	int pnums [MAX_PLAYERS+2];

	// One-time initialization

	for (i = gameData.multi.nPlayers/2; i < gameData.multi.nPlayers; i++) // Put first half of players on team A
	{
		team_vector |= (1 << i);
	}

	sprintf (team_names [0], "%s", TXT_BLUE);
	sprintf (team_names [1], "%s", TXT_RED);

	// Here comes da menu
do_menu:

	memset (m, 0, sizeof (m));

	m [0].type = NM_TYPE_INPUT; 
	m [0].text = team_names [0]; 
	m [0].text_len = CALLSIGN_LEN; 

	opt = 1;
	for (i = 0; i < gameData.multi.nPlayers; i++)
	{
		if (!(team_vector & (1 << i)))
		{
			m [opt].type = NM_TYPE_MENU; 
			m [opt].text = netPlayers.players [i].callsign; 
			pnums [opt] = i; 
			opt++;
		}
	}
	opt_team_b = opt;
	m [opt].type = NM_TYPE_INPUT; 
	m [opt].text = team_names [1]; 
	m [opt].text_len = CALLSIGN_LEN; 
	opt++;
	for (i = 0; i < gameData.multi.nPlayers; i++)
	{
		if (team_vector & (1 << i))
		{
			m [opt].type = NM_TYPE_MENU; 
			m [opt].text = netPlayers.players [i].callsign; 
			pnums [opt] = i; 
			opt++;
		}
	}
	m [opt].type = NM_TYPE_TEXT; 
	m [opt].text = ""; 
	opt++;
	m [opt].type = NM_TYPE_MENU; 
	m [opt].text = TXT_ACCEPT; 
	m [opt].key = KEY_A;
	opt++;

	Assert (opt <= MAX_PLAYERS+4);
	
	choice = ExecMenu (NULL, TXT_TEAM_SELECTION, opt, m, NULL, NULL);

	if (choice == opt-1)
	{
		if ((opt-2-opt_team_b < 2) || (opt_team_b == 1)) 
		{
			ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_TEAM_MUST_ONE);
#if 0//def RELEASE
				goto do_menu;
#endif
		}
		
		netGame.team_vector = team_vector;
		strcpy (netGame.team_name [0], team_names [0]);
		strcpy (netGame.team_name [1], team_names [1]);
		return 1;
	}

	else if ((choice > 0) && (choice < opt_team_b)) {
		team_vector |= (1 << pnums [choice]);
	}
	else if ((choice > opt_team_b) && (choice < opt-2)) {
		team_vector &= ~ (1 << pnums [choice]);
	}
	else if (choice == -1)
		return 0;
	goto do_menu;
#else
	return 0;
#endif
}

//------------------------------------------------------------------------------

int NetworkSelectPlayers (int bAutoRun)
{
	int i, j, choice = 1;
   newmenu_item m [MAX_PLAYERS+4];
   char text [MAX_PLAYERS+4] [45];
	char title [50];
	int save_nplayers;              //how may people would like to join

NetworkAddPlayer (&networkData.mySeq);
if (bAutoRun)
	return 1;
	
memset (m, 0, sizeof (m));
for (i = 0; i< MAX_PLAYERS+4; i++) {
	sprintf (text [i], "%d.  %-20s", i+1, "");
	m [i].type = NM_TYPE_CHECK; 
	m [i].text = text [i]; 
	m [i].value = 0;
	}
m [0].value = 1;                         // Assume server will play...
if (gameOpts->multi.bNoRankings)
	sprintf (text [0], "%d. %-20s", 1, gameData.multi.players [gameData.multi.nLocalPlayer].callsign);
else
	sprintf (text [0], "%d. %s%-20s", 1, pszRankStrings [netPlayers.players [gameData.multi.nLocalPlayer].rank], gameData.multi.players [gameData.multi.nLocalPlayer].callsign);
sprintf (title, "%s %d %s", TXT_TEAM_SELECT, gameData.multi.nMaxPlayers, TXT_TEAM_PRESS_ENTER);

GetPlayersAgain:

gameStates.app.nExtGameStatus = GAMESTAT_NETGAME_PLAYER_SELECT;
Assert (sizeofa (m) >= MAX_PLAYERS + 4);
j = ExecMenu1 (NULL, title, MAX_PLAYERS + 4, m, NetworkStartPoll, &choice);
save_nplayers = gameData.multi.nPlayers;
if (j < 0) {
	// Aborted!                                     
	// Dump all players and go back to menu mode

abort:
	for (i = 1; i<save_nplayers; i++) {
		if (gameStates.multi.nGameType >= IPX_GAME)
			NetworkDumpPlayer (
				netPlayers.players [i].network.ipx.server, 
				netPlayers.players [i].network.ipx.node, 
				DUMP_ABORTED);
		}

	netGame.numplayers = 0;
	NetworkSendGameInfo (0); // Tell everyone we're bailing
	ipx_handle_leave_game (); // Tell the network driver we're bailing too
	networkData.nStatus = NETSTAT_MENU;
	return 0;
	}
// Count number of players chosen
gameData.multi.nPlayers = 0;
for (i = 0; i<save_nplayers; i++) {
	if (m [i].value) 
		gameData.multi.nPlayers++;
	}
if (gameData.multi.nPlayers > netGame.max_numplayers) {
	ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, "%s %d %s", TXT_SORRY_ONLY, gameData.multi.nMaxPlayers, TXT_NETPLAYERS_IN);
	gameData.multi.nPlayers = save_nplayers;
	goto GetPlayersAgain;
	}
#ifdef RELEASE
if (gameData.multi.nPlayers < 2) {
	ExecMessageBox (TXT_WARNING, NULL, 1, TXT_OK, TXT_TEAM_ATLEAST_TWO);
#	if 0
	gameData.multi.nPlayers = save_nplayers;
	goto GetPlayersAgain;
#	endif
	}
#endif

#ifdef RELEASE
if ((netGame.gamemode == NETGAME_TEAM_ANARCHY ||
		netGame.gamemode == NETGAME_CAPTURE_FLAG || 
		netGame.gamemode == NETGAME_TEAM_HOARD) && 
		(gameData.multi.nPlayers < 2)) {
	ExecMessageBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_NEED_2PLAYERS);
	gameData.multi.nPlayers = save_nplayers;
#if 0		
	goto GetPlayersAgain;
#endif		
	}
#endif

// Remove players that aren't marked.
gameData.multi.nPlayers = 0;
for (i = 0; i<save_nplayers; i++) {
	if (m [i].value) {
		if (i > gameData.multi.nPlayers) {
			if (gameStates.multi.nGameType >= IPX_GAME) {
				memcpy (netPlayers.players [gameData.multi.nPlayers].network.ipx.node, 
				netPlayers.players [i].network.ipx.node, 6);
				memcpy (netPlayers.players [gameData.multi.nPlayers].network.ipx.server, 
				netPlayers.players [i].network.ipx.server, 4);
				}
			else {
				netPlayers.players [gameData.multi.nPlayers].network.appletalk.node = netPlayers.players [i].network.appletalk.node;
				netPlayers.players [gameData.multi.nPlayers].network.appletalk.net = netPlayers.players [i].network.appletalk.net;
				netPlayers.players [gameData.multi.nPlayers].network.appletalk.socket = netPlayers.players [i].network.appletalk.socket;
				}
			memcpy (
				netPlayers.players [gameData.multi.nPlayers].callsign, 
				netPlayers.players [i].callsign, CALLSIGN_LEN+1);
			netPlayers.players [gameData.multi.nPlayers].version_major = netPlayers.players [i].version_major;
			netPlayers.players [gameData.multi.nPlayers].version_minor = netPlayers.players [i].version_minor;
			netPlayers.players [gameData.multi.nPlayers].rank = netPlayers.players [i].rank;
			ClipRank (&netPlayers.players [gameData.multi.nPlayers].rank);
			NetworkCheckForOldVersion ((char)i);
			}
		gameData.multi.players [gameData.multi.nPlayers].connected = 1;
		gameData.multi.nPlayers++;
		}
	else {
		if (gameStates.multi.nGameType >= IPX_GAME)
			NetworkDumpPlayer (netPlayers.players [i].network.ipx.server, netPlayers.players [i].network.ipx.node, DUMP_DORK);
		}
	}

for (i = gameData.multi.nPlayers; i < MAX_NUM_NET_PLAYERS; i++) {
	if (gameStates.multi.nGameType >= IPX_GAME) {
		memset (netPlayers.players [i].network.ipx.node, 0, 6);
		memset (netPlayers.players [i].network.ipx.server, 0, 4);
	   }
	else {
		netPlayers.players [i].network.appletalk.node = 0;
		netPlayers.players [i].network.appletalk.net = 0;
		netPlayers.players [i].network.appletalk.socket = 0;
	   }
	memset (netPlayers.players [i].callsign, 0, CALLSIGN_LEN+1);
	netPlayers.players [i].version_major = 0;
	netPlayers.players [i].version_minor = 0;
	netPlayers.players [i].rank = 0;
	}
#if 1				
con_printf (CON_DEBUG, "Select teams: Game mode is %d\n", netGame.gamemode);
#endif
if (netGame.gamemode == NETGAME_TEAM_ANARCHY ||
	 netGame.gamemode == NETGAME_CAPTURE_FLAG ||
	 netGame.gamemode == NETGAME_TEAM_HOARD ||
	 netGame.gamemode == NETGAME_ENTROPY ||
	 netGame.gamemode == NETGAME_MONSTERBALL)
	if (!NetworkSelectTeams ())
		goto abort;
return 1; 
}

//------------------------------------------------------------------------------

void InitNetgameMenuOption (newmenu_item *m, int j)
{
m += j;
if (!m->text) {
	m->text = szNMTextBuffer [j];
	m->type = NM_TYPE_MENU;
	}
sprintf (m->text, "%2d.                                                     ", 
			j - 1 - gameStates.multi.bUseTracker);
m->value = 0;
m->rebuild = 1;
}

//------------------------------------------------------------------------------

void InitNetgameMenu (newmenu_item *m, int i)
{
	int j;

for (j = i + 2 + gameStates.multi.bUseTracker; i < MAX_ACTIVE_NETGAMES; i++, j++)
	InitNetgameMenuOption (m, j);
}

//------------------------------------------------------------------------------

extern int nTabs [];

char *PruneText (char *pszDest, char *pszSrc, int nSize, int nPos, int nVersion)
{
	int		lDots, lMax, l, tx, ty, ta;
	char		*psz;
	grs_font	*curFont = grdCurCanv->cv_font;

if (gameOpts->menus.bShowLevelVersion && (nVersion >= 0)) {
	if (nVersion)
		sprintf (pszDest, "[%d] ", nVersion);
	else
		strcpy (pszDest, "[?] ");
	strncat (pszDest + 4, pszSrc, nSize - 4);
	}
else
	strncpy (pszDest, pszSrc, nSize);
pszDest [nSize - 1] = '\0';
if (psz = strchr (pszDest, '\t'))
	*psz = '\0';
grdCurCanv->cv_font = SMALL_FONT;
GrGetStringSize ("... ", &lDots, &ty, &ta);
GrGetStringSize (pszDest, &tx, &ty, &ta);
l = (int) strlen (pszDest);
lMax = LHX (nTabs [nPos]) - LHX (nTabs [nPos - 1]);
if (tx > lMax) {
	lMax -= lDots;
	do {
		pszDest [--l] = '\0';
		GrGetStringSize (pszDest, &tx, &ty, &ta);
	} while (tx > lMax);
	strcat (pszDest, "...");
	}
grdCurCanv->cv_font = curFont; 
return pszDest;
}

//------------------------------------------------------------------------------

char *szModeLetters []  = 
	{"ANRCHY", 
	 "TEAM", 
	 "ROBO", 
	 "COOP", 
	 "FLAG", 
	 "HOARD", 
	 "TMHOARD", 
	 "ENTROPY",
	 "MONSTER"};

void NetworkJoinPoll (int nitems, newmenu_item * menus, int * key, int citem)
{
	// Polling loop for Join Game menu
	static fix t1 = 0;
	int	i, h = 2 + gameStates.multi.bUseTracker, t, osocket, nJoinStatus, bPlaySound = 0;
	char	*psz;
	char	szOption [200];
	char	szTrackers [100];

if (gameStates.multi.bUseTracker) {
	i = ActiveTrackerCount (0);
	menus [1].color = ORANGE_RGBA;
	sprintf (szTrackers, TXT_TRACKERS_FOUND, i, (i == 1) ? "" : "s");
	if (strcmp (menus [1].text, szTrackers)) {
		strcpy (menus [1].text, szTrackers);
//			menus [1].x = (short) 0x8000;
		menus [1].rebuild = 1;
		}
	}

if ((gameStates.multi.nGameType >= IPX_GAME) && networkData.bAllowSocketChanges) {
	osocket = networkData.nSocket;
	if (*key == KEY_PAGEDOWN) { 
		networkData.nSocket--; 
		*key = 0; 
		}
	else if (*key == KEY_PAGEUP) { 
		networkData.nSocket++; 
		*key = 0; 
		}
	if (networkData.nSocket > 99)
		networkData.nSocket = -99;
	else if (networkData.nSocket < -99)
		networkData.nSocket = 99;
	if (networkData.nSocket + IPX_DEFAULT_SOCKET > 0x8000)
		networkData.nSocket = 0x8000 - IPX_DEFAULT_SOCKET;
	if (networkData.nSocket + IPX_DEFAULT_SOCKET < 0)
		networkData.nSocket = IPX_DEFAULT_SOCKET;
	if (networkData.nSocket != osocket) {
		sprintf (menus [0].text, TXT_CURR_SOCK, 
					 (gameStates.multi.nGameType == IPX_GAME) ? "IPX" : "UDP", 
					 networkData.nSocket);
		menus [0].rebuild = 1;
#if 1				
		con_printf (0, TXT_CHANGE_SOCK, networkData.nSocket);
#endif
		NetworkListen ();
		IpxChangeDefaultSocket ((ushort) (IPX_DEFAULT_SOCKET + networkData.nSocket));
		RestartNetSearching (menus);
		NetworkSendGameListRequest ();
		return;
		}
	}
	// send a request for game info every 3 seconds
#if 0//def _DEBUG
if (!networkData.nActiveGames)
#endif
	if (gameStates.multi.nGameType >= IPX_GAME) {
		if (TimerGetApproxSeconds () > t1 + F1_0 * 3) {
			if (!NetworkSendGameListRequest ())
				return;
			t1 = TimerGetApproxSeconds ();
			}
		}
NetworkListen ();
DeleteTimedOutNetGames ();
t = SDL_GetTicks ();
if (networkData.bGamesChanged || (networkData.nActiveGames != networkData.nLastActiveGames)) {
	networkData.bGamesChanged = 0;
	networkData.nLastActiveGames = networkData.nActiveGames;
#if 1				
	con_printf (CON_DEBUG, "Found %d netgames.\n", networkData.nActiveGames);
#endif
	// Copy the active games data into the menu options
	for (i = 0; i < networkData.nActiveGames; i++, h++) {
			int game_status = activeNetGames [i].game_status;
			int nplayers = 0;
			char szLevelName [20], szMissionName [50], szGameName [50];
			int nLevelVersion = gameOpts->menus.bShowLevelVersion ? FindMissionByName (activeNetGames [i].mission_name, -1) : -1;

		// These next two loops protect against menu skewing
		// if missiontitle or gamename contain a tab

		PruneText (szMissionName, activeNetGames [i].mission_title, sizeof (szMissionName), 4, nLevelVersion);
		PruneText (szGameName, activeNetGames [i].game_name, sizeof (szGameName), 1, -1);
		nplayers = activeNetGames [i].numconnected;
		if (activeNetGames [i].levelnum < 0)
			sprintf (szLevelName, "S%d", -activeNetGames [i].levelnum);
		else
			sprintf (szLevelName, "%d", activeNetGames [i].levelnum);
		if (game_status == NETSTAT_STARTING)
			psz = "Forming";
		else if (game_status == NETSTAT_PLAYING) {
			nJoinStatus = CanJoinNetgame (activeNetGames + i, NULL);

			if (nJoinStatus == 1)
				psz = "Open";
			else if (nJoinStatus == 2)
				psz = "Full";
			else if (nJoinStatus == 3)
				psz = "Restrict";
			else
				psz = "Closed";
			}
		else 
			psz = "Between";
		sprintf (szOption, "%2d.\t%s\t%s\t%d/%d\t%s\t%s\t%s", 
					i + 1, szGameName, szModeLetters [activeNetGames [i].gamemode], nplayers, 
					activeNetGames [i].max_numplayers, szMissionName, szLevelName, psz);
		Assert (strlen (szOption) < 100);
		if (strcmp (szOption, menus [h].text)) {
			memcpy (menus [h].text, szOption, 100);
			menus [h].rebuild = 1;
			menus [h].unavailable = (nLevelVersion == 0);
			bPlaySound = 1;
			}
		}
	for (i = 3 + networkData.nActiveGames; i < MAX_ACTIVE_NETGAMES; i++, h++)
		InitNetgameMenuOption (menus, h);
	}
#if 0
else
	for (i = 3; i < networkData.nActiveGames; i++, h++)
		menus [h].value = SDL_GetTicks ();
for (i = 3 + networkData.nActiveGames; i < MAX_ACTIVE_NETGAMES; i++, h++)
	if (menus [h].value && (t - menus [h].value > 10000)) 
		{
		InitNetgameMenuOption (menus, h);
		bPlaySound = 1;
		}
#endif
if (bPlaySound)
	DigiPlaySample (SOUND_HUD_MESSAGE, F1_0);
}


//------------------------------------------------------------------------------

int NetworkBrowseGames (void)
{
	int choice, i, bAutoRun = gameData.multi.autoNG.bValid;
	bkg bg;
	newmenu_item m [MAX_ACTIVE_NETGAMES + 5];

//LogErr ("launching netgame browser\n");
memset (&bg, 0, sizeof (bg));
bg.bIgnoreBg = 1;
if (gameStates.multi.nGameType >= IPX_GAME) {
	if (!networkData.bActive) {
		ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_IPX_NOT_FOUND);
		return 0;
		}
	}
//LogErr ("   NetworkInit\n");
NetworkInit ();
gameData.multi.nPlayers = 0;
setjmp (gameExitPoint);
networkData.bSendObjects = 0; 
networkData.bSendingExtras = 0;
networkData.bRejoined = 0;
networkData.nStatus = NETSTAT_BROWSING; // We are looking at a game menu
IpxChangeDefaultSocket ((ushort) (IPX_DEFAULT_SOCKET + networkData.nSocket));
//LogErr ("   NetworkFlush\n");
NetworkFlush ();
//LogErr ("   NetworkListen\n");
NetworkListen ();  // Throw out old info
//LogErr ("   NetworkSendGameListRequest\n");
NetworkSendGameListRequest (); // broadcast a request for lists
networkData.nActiveGames = 0;
networkData.nLastActiveGames = 0;
memset (activeNetGames, 0, sizeof (activeNetGames));
memset (activeNetPlayers, 0, sizeof (activeNetPlayers));
if (!bAutoRun) {
	GrSetFontColorRGBi (RGBA_PAL (15, 15, 23), 1, 0, 0);
	memset (m, 0, sizeof (m));
	m [0].text = szNMTextBuffer [0];
	m [0].type = NM_TYPE_TEXT;
	m [0].noscroll = 1;
	m [0].x = (short) 0x8000;
	//m [0].x = (short) 0x8000;
	if (gameStates.multi.nGameType >= IPX_GAME)
		if (networkData.bAllowSocketChanges)
			sprintf (m [0].text, TXT_CURR_SOCK, 
										 (gameStates.multi.nGameType == IPX_GAME) ? "IPX" : "UDP", networkData.nSocket);
		else
			*m [0].text = '\0';
	i = 1;
	if (gameStates.multi.bUseTracker) {
		m [i].type = NM_TYPE_TEXT;
		m [i].text = szNMTextBuffer [i];
		strcpy (m [i].text, TXT_0TRACKERS);
		m [i].x = (short) 0x8000;
		m [i].noscroll = 1;
		i++;
		}
	m [i].text = szNMTextBuffer [i];
	m [i].type = NM_TYPE_TEXT;
	strcpy (m [i].text, TXT_GAME_BROWSER);
	m [i].noscroll = 1;
	InitNetgameMenu (m, 0);
	}
networkData.bGamesChanged = 1;    

do_menu:

gameStates.app.nExtGameStatus = GAMESTAT_JOIN_NETGAME;
if (bAutoRun) {
	fix t0 = 0;
	do {
		if (TimerGetApproxSeconds () > t0 + F1_0) {
			t0 = TimerGetApproxSeconds ();
			NetworkSendGameListRequest ();
			}
		NetworkListen ();
		if (KeyInKey () == KEY_ESC)
			return 0;
		} while (!networkData.nActiveGames);
	choice = 0;
	}
else {
	gameStates.multi.bSurfingNet = 1;
	NMLoadBackground (Menu_pcx_name, &bg, 0);             //load this here so if we abort after loading level, we restore the palette
	GrPaletteStepLoad (NULL);
	choice = ExecMenutiny (TXT_NETGAMES, NULL, MAX_ACTIVE_NETGAMES + 2 + gameStates.multi.bUseTracker, m, NetworkJoinPoll);
	NMRemoveBackground (&bg);
	gameStates.multi.bSurfingNet = 0;
	}
if (choice == -1) {
	networkData.nStatus = NETSTAT_MENU;
	return 0; // they cancelled               
	}               
choice -= (2 + gameStates.multi.bUseTracker);
if (choice >= networkData.nActiveGames) {
	ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_INVALID_CHOICE);
	goto do_menu;
	}

// Choice has been made and looks legit
if (activeNetGames [choice].game_status == NETSTAT_ENDLEVEL) {
	ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_NET_GAME_BETWEEN2);
	goto do_menu;
	}
if (activeNetGames [choice].protocol_version != MULTI_PROTO_VERSION) {
	if (activeNetGames [choice].protocol_version == 3) {
#ifndef SHAREWARE
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_INCOMPAT1);
#endif
		}
	else if (activeNetGames [choice].protocol_version == 4) {
#ifdef SHAREWARE
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_INCOMPAT2);
#endif
		}
	else {
		char	szFmt [200], szError [200];

		sprintf (szFmt, "%s%s", TXT_VERSION_MISMATCH, TXT_NETGAME_VERSIONS);
		sprintf (szError, szFmt, MULTI_PROTO_VERSION, activeNetGames [choice].protocol_version);
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, szError);
		}
	goto do_menu;
	}

if (gameStates.multi.bUseTracker) {
	//LogErr ("   getting server lists from trackers\n");
	GetServerFromList (choice);
	}
#ifndef SHAREWARE
// Check for valid mission name
#if 1				
con_printf (CON_DEBUG, TXT_LOADING_MSN, activeNetGames [choice].mission_name);
#endif
if (!(LoadMissionByName (activeNetGames [choice].mission_name, -1) ||
		 (DownloadMission (activeNetGames [choice].mission_name) &&
		 LoadMissionByName (activeNetGames [choice].mission_name, -1)))) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_MISSION_NOT_FOUND);
	goto do_menu;
	}
#endif
if (IS_D2_OEM && (activeNetGames [choice].levelnum > 8)) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_OEM_ONLY8);
	goto do_menu;
	}
if (IS_MAC_SHARE && (activeNetGames [choice].levelnum > 4)) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_SHARE_ONLY4);
	goto do_menu;
	}
if (!NetworkWaitForAllInfo (choice)) {
	ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_JOIN_ERROR);
	networkData.nStatus = NETSTAT_BROWSING; // We are looking at a game menu
	goto do_menu;
	}       

networkData.nStatus = NETSTAT_BROWSING; // We are looking at a game menu
  if (!CanJoinNetgame (activeNetGames + choice, activeNetPlayers + choice)) {
	if (activeNetGames [choice].numplayers == activeNetGames [choice].max_numplayers)
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_GAME_FULL);
	else
		ExecMessageBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_IN_PROGRESS);
	goto do_menu;
	}
// Choice is valid, prepare to join in
memcpy (&netGame, activeNetGames + choice, sizeof (netgame_info));
memcpy (&netPlayers, activeNetPlayers + choice, sizeof (allNetPlayers_info));
gameStates.app.nDifficultyLevel = netGame.difficulty;
gameData.multi.nMaxPlayers = netGame.max_numplayers;
ChangePlayerNumTo (1);
// Handle the extra data for the network driver
// For the mcast4 driver, this is the game's multicast address, to
// which the driver subscribes.
if (IpxHandleNetGameAuxData (netGame.AuxData) < 0) {
	networkData.nStatus = NETSTAT_BROWSING;
	goto do_menu;
	}
NetworkSetGameMode (netGame.gamemode);
NetworkAdjustMaxDataSize ();
//LogErr ("loading level\n");
StartNewLevel (netGame.levelnum, 0);
//LogErr ("exiting netgame browser\n");
NMRemoveBackground (&bg);
return 1;         // look ma, we're in a game!!!
}

//------------------------------------------------------------------------------

int ConnectionPacketLevel [] = {0, 1, 1, 1};
int ConnectionSecLevel [] = {12, 3, 5, 7};

int AppletalkConnectionPacketLevel [] = {0, 1, 0};
int AppletalkConnectionSecLevel [] = {10, 3, 8};

#ifdef RELEASE
int NetworkChooseConnect ()
{
return 1;
}
#else
int NetworkChooseConnect ()
{
#if 0
newmenu_item m [16];
int choice, opt = 0;
#endif
if (gameStates.multi.nGameType >= IPX_GAME) {  
#if 0
	memset (m, 0, sizeof (m));
	m [opt].type = NM_TYPE_MENU;  
	m [opt].text = "Local Subnet"; 
	opt++;
	m [opt].type = NM_TYPE_MENU;  
	m [opt].text = "14.4 modem over Internet"; 
	opt++;
	m [opt].type = NM_TYPE_MENU;  
	m [opt].text = "28.8 modem over Internet"; 
	opt++;
	m [opt].type = NM_TYPE_MENU;  
	m [opt].text = "ISDN or T1 over Internet"; 
	opt++;

	Assert (sizeofa (m) >= opt);
	choice = ExecMenu1 (NULL, "Choose connection type", opt, m, NULL, 0);

	if (choice<0)
		return (NULL);

	Assert (choice> = 0 && choice< = 3);

	netGame.bShortPackets = ConnectionPacketLevel [choice];
	netGame.nPacketsPerSec = ConnectionSecLevel [choice];
#endif
   return 1;
	}
return 0;	
}
#endif

//------------------------------------------------------------------------------

int stoport (char *szPort, int *pPort, int *pSign)
{
	int	h, sign;

if (*szPort == '+')
	sign = 1;
else if (*szPort == '-')
	sign = -1;
else
	sign = 0;
if (sign && !pSign)
	return 0;
h = atol (szPort + (pSign && *pSign != 0));
*pPort = sign ? UDP_BASEPORT + sign * h : h;
if (pSign)
	*pSign = sign;
return 1;
}

//------------------------------------------------------------------------------

int stoip (char *szServerIpAddr, unsigned char *pIpAddr)
{
	char	*pi, *pj, *pFields [5], tmp [22];
	int	h, i, j;

memset (pFields, 0, sizeof (pFields));
memcpy (tmp, szServerIpAddr, sizeof (tmp));

for (pi = pj = tmp, i = 0;;) {
	if (!*pi) {
		if (i < 3)
			return 0;
		pFields [i++] = pj;
		break;
		}
	else if (*pi == '.') {
		if (i > 3)
			return 0;
		pFields [i++] = pj;
		}
	else if ((*pi == ':') || (*pi == '-')) {
		if (i != 3)
			return 0;
		pFields [i++] = pj;
		}
	else {
		pi++;
		continue;
		}
	* (pi++) = '\0';
	pj = pi;
	}
if (i < 3)
	return 0;
for (j = 0; j < i; j++) {
	if (!pFields [j])
		return 0;
	if (j == 4)
		return stoport (pFields [j], udpBasePort, NULL); 
	else {
		h = atol (pFields [j]);
		if ((h < 0) || (h > 255))
			return 0;
		pIpAddr [j] = (unsigned char) h;
		}
	}
return 1;
}

//------------------------------------------------------------------------------

void IpAddrMenuCallBack (int nitems, newmenu_item * menus, int * key, int citem)
{
}

//------------------------------------------------------------------------------

int NetworkGetIpAddr (void)
{
	newmenu_item m [9];
	int i, choice = 0;
	int opt = 0, optServer = -1, optPort = -1;
	int commands;

#ifdef _DEBUG
	static char szLocalIpAddr [16] = {'0', '.', '0', '.', '0', '.', '0', '\0'};
#else
#endif
	static char szClientPort [7] = {'\0'};
	static int nClientPortSign = 0;

if (!gameStates.multi.bUseTracker) {
	if (!*mpParams.szServerIpAddr) {
		ArchIpxSetDriver (IPX_DRIVER_UDP);
		if (IpxInit (-1) != IPX_INIT_OK) {
			strcpy (mpParams.szServerIpAddr, "0.0.0.0");
			}
		else {
			sprintf (mpParams.szServerIpAddr, "%d.%d.%d.%d", 
						ipx_ServerAddress [4], ipx_ServerAddress [5], ipx_ServerAddress [6], ipx_ServerAddress [7]);
			sprintf (szClientPort, "%s%d", 
						 (nClientPortSign < 0) ? "-" : (nClientPortSign > 0) ? "+" : "", mpParams.udpClientPort);
			IpxClose ();
			}
	//	strcpy (szLocalIpAddr, szServerIpAddr);
		}
	}
memset (m, 0, sizeof (m));
if (!gameStates.multi.bUseTracker) {
	ADD_TEXT (opt, TXT_HOST_IP, 0);
	//IP [\":\"<port> | {{\"+\" | \"-\"} <offset>]\n\n (e.g. 127.0.0.1 for an arbitrary port, \nor 127.0.0.1:28342 for a fixed port, \nor 127.0.0.1:+1 for host port + offset)\n";       
	//m [opt].text = "Enter the game host' IP address.\nYou can specify a port to use\nby entering a colon followed by the port number.\nYou can use an 
	opt++;
	ADD_INPUT (opt, mpParams.szServerIpAddr, sizeof (mpParams.szServerIpAddr) - 1, HTX_GETIP_SERVER);
	optServer = opt++;
	/*
	m [opt].type = NM_TYPE_TEXT;  
	m [opt].text = "\nServer IP address:";
	opt++;
	m [opt].type = NM_TYPE_INPUT; 
	m [opt].text = szLocalIpAddr; 
	m [opt].text_len = sizeof (szLocalIpAddr)-1;         
	opt++;
	*/
	ADD_TEXT (opt, TXT_CLIENT_PORT, 0);
	opt++;
	}
if ((mpParams.udpClientPort < 0) || (mpParams.udpClientPort > 65535))
	mpParams.udpClientPort = 0;
sprintf (szClientPort, "%u", mpParams.udpClientPort);
ADD_INPUT (opt, szClientPort, sizeof (szClientPort) - 1, HTX_GETIP_CLIENT);
optPort = opt++;
ADD_TEXT (opt, TXT_PORT_HELP1, 0);
ADD_TEXT (opt, TXT_PORT_HELP2, 0);
opt++;
commands = opt;
Assert (sizeofa (m) >= opt);
for (;;) {
	i = ExecMenu1 (NULL, gameStates.multi.bUseTracker ? TXT_CLIENT_PORT + 1 : TXT_IP_HEADER, 
						opt, m, &IpAddrMenuCallBack, &choice);
	if (i < 0)
		break;
	if (i < commands) {
		if ((i == optServer) || (i == optPort)) {
			if (gameStates.multi.bUseTracker || stoip (mpParams.szServerIpAddr, ipx_ServerAddress + 4)) {
				stoport (szClientPort, &mpParams.udpClientPort, &nClientPortSign);
				return 1;
				}
			ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_IP_INVALID);
			}
		}
	}
return 0;
} 

//------------------------------------------------------------------------------

