/* $Id: network.c, v 1.24 2003/10/12 09:38:48 btb Exp $ */
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

#ifdef RCS
static char rcsid [] = "$Id: network.c, v 1.24 2003/10/12 09:38:48 btb Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#	include <winsock.h>
#else
#	include <sys/socket.h>
#endif
#ifndef _WIN32
#	include <arpa/inet.h>
#	include <netinet/in.h> /* for htons & co. */
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
#include "objsmoke.h"
#include "error.h"
#include "laser.h"
#include "gamesave.h"
#include "gamemine.h"
#include "player.h"
#include "loadgame.h"
#include "fireball.h"
#include "network.h"
#include "network_lib.h"
#include "game.h"
#include "multi.h"
#include "endlevel.h"
#include "palette.h"
#include "reactor.h"
#include "powerup.h"
#include "menu.h"
#include "sounds.h"
#include "text.h"
#include "highscores.h"
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
#include "autodl.h"
#include "tracker.h"
#include "newmenu.h"
#include "gamefont.h"
#include "gameseg.h"
#include "hudmsg.h"
#include "vers_id.h"
#include "netmenu.h"
#include "banlist.h"
#include "collide.h"
#include "ipx.h"
#ifdef _WIN32
#	include "win32/include/ipx_udp.h"
#	include "win32/include/ipx_drv.h"
#else
#	include "linux/include/ipx_udp.h"
#	include "linux/include/ipx_drv.h"
#endif

//------------------------------------------------------------------------------

void NetworkProcessMonitorVector (int vector)
{
	int		i, j;
	int		tm, ec, bm;
	int		count = 0;
	tSegment	*segP = gameData.segs.segments ;
	tSide		*sideP;

for (i = 0; i <= gameData.segs.nLastSegment; i++, segP++) {
	for (j = 0, sideP = segP->sides; j < 6; j++, sideP++) {
		if (((tm = sideP->nOvlTex) != 0) &&
				((ec = gameData.pig.tex.pTMapInfo [tm].nEffectClip) != -1) &&
				((bm = gameData.eff.pEffects [ec].nDestBm) != -1)) {
			if (vector & (1 << count))
				sideP->nOvlTex = bm;
			count++;
			Assert (count < 32);
			}
		}
	}
}

//------------------------------------------------------------------------------

void NetworkProcessGameInfo (ubyte *dataP)
{
	int i;
	tNetgameInfo *newGame = (tNetgameInfo *) dataP;

#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	tNetgameInfo tmp_info;

if (gameStates.multi.nGameType >= IPX_GAME) {
	ReceiveNetGamePacket (dataP, &tmp_info, 0); // get correctly aligned structure
	newGame = &tmp_info;
	}
#endif
networkData.bWaitingForPlayerInfo = 0;
#if SECURITY_CHECK
if (newGame->nSecurity != tmpPlayersInfo->nSecurity) {
	Int3 ();     // Get Jason
   return;     // If this first half doesn't go with the second half
   }
#endif
Assert (tmpPlayersInfo != NULL);
i = FindActiveNetGame (newGame->szGameName, newGame->nSecurity);
if (i == MAX_ACTIVE_NETGAMES) {
#if 1
	con_printf (CONDBG, "Too many netgames.\n");
#endif	
	return;
	}
if (i == networkData.nActiveGames) {
	if (newGame->nNumPlayers == 0)
		return;
	networkData.nActiveGames++;
	}
networkData.bGamesChanged = 1;
// MWA  memcpy (&activeNetGames [i], dataP, sizeof (tNetgameInfo);
nLastNetGameUpdate [i] = SDL_GetTicks ();
memcpy (activeNetGames + i, (ubyte *) newGame, sizeof (tNetgameInfo));
memcpy (activeNetPlayers + i, tmpPlayersInfo, sizeof (tAllNetPlayersInfo));
if (networkData.nSecurityCheck)
#if SECURITY_CHECK
	if (activeNetGames [i].nSecurity == networkData.nSecurityCheck)
#endif
		networkData.nSecurityCheck = -1;
if (i == networkData.nActiveGames)
if (activeNetGames [i].nNumPlayers == 0) {	// Delete this game
	DeleteActiveNetGame (i);
	networkData.nSecurityCheck = 0;
	}
}

//------------------------------------------------------------------------------

void NetworkProcessLiteInfo (ubyte *dataP)
{
	int				i;
	tNetgameInfo	*actGameP;
	tLiteInfo		*newInfo = (tLiteInfo *)dataP;
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	tLiteInfo		tmp_info;

if (gameStates.multi.nGameType >= IPX_GAME) {
	ReceiveNetGamePacket (dataP, (tNetgameInfo *)&tmp_info, 1);
	newInfo = &tmp_info;
	}
#endif

networkData.bGamesChanged = 1;
i = FindActiveNetGame (((tNetgameInfo *) newInfo)->szGameName, ((tNetgameInfo *) newInfo)->nSecurity);
if (i == MAX_ACTIVE_NETGAMES)
	return;
if (i == networkData.nActiveGames) {
	if (newInfo->nNumPlayers == 0)
		return;
	networkData.nActiveGames++;
	}
actGameP = activeNetGames + i;
memcpy (actGameP, (ubyte *) newInfo, sizeof (tLiteInfo));
nLastNetGameUpdate [i] = SDL_GetTicks ();
// See if this is really a Hoard/Entropy/Monsterball game
// If so, adjust all the dataP accordingly
if (HoardEquipped ()) {
	if (actGameP->gameFlags & (NETGAME_FLAG_HOARD | NETGAME_FLAG_ENTROPY | NETGAME_FLAG_MONSTERBALL)) {
		if ((actGameP->gameFlags & NETGAME_FLAG_MONSTERBALL) == NETGAME_FLAG_MONSTERBALL)
			actGameP->gameMode = NETGAME_MONSTERBALL; 
		else if (actGameP->gameFlags & NETGAME_FLAG_HOARD)
			actGameP->gameMode = NETGAME_HOARD;					  
		else if (actGameP->gameFlags & NETGAME_FLAG_ENTROPY)
			actGameP->gameMode = NETGAME_ENTROPY;					  
		actGameP->gameStatus = NETSTAT_PLAYING;
		if (actGameP->gameFlags & NETGAME_FLAG_TEAM_HOARD)
			actGameP->gameMode = NETGAME_TEAM_HOARD;					  
		if (actGameP->gameFlags & NETGAME_FLAG_REALLY_ENDLEVEL)
			actGameP->gameStatus = NETSTAT_ENDLEVEL;
		if (actGameP->gameFlags & NETGAME_FLAG_REALLY_FORMING)
			actGameP->gameStatus = NETSTAT_STARTING;
		}
	}
if (actGameP->nNumPlayers == 0)
	DeleteActiveNetGame (i);
}

//------------------------------------------------------------------------------

void NetworkProcessExtraGameInfo (ubyte *dataP)
{
	int	i;

ReceiveExtraGameInfoPacket (dataP, extraGameInfo + 1);
SetMonsterballForces ();
LogExtraGameInfo ();
gameStates.app.bHaveExtraGameInfo [1] = 1;
i = FindActiveNetGame (extraGameInfo [1].szGameName, extraGameInfo [1].nSecurity);
if (i < networkData.nActiveGames)
	activeExtraGameInfo [i] = extraGameInfo [1];
else
	memset (activeExtraGameInfo + i, 0, sizeof (activeExtraGameInfo [i]));
}

//------------------------------------------------------------------------------

void NetworkProcessDump (tSequencePacket *their)
{
	// Our request for join was denied.  Tell the user why.

	char temp [40];
	int i;

if (their->player.connected != 7)
	ExecMessageBox (NULL, NULL, 1, TXT_OK, NET_DUMP_STRINGS (their->player.connected));
else {
	for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
		if (!stricmp (their->player.callsign, gameData.multiplayer.players [i].callsign)) {
			if (i!=NetworkWhoIsMaster ()) 
				HUDInitMessage (TXT_KICK_ATTEMPT, their->player.callsign);
			else {
				sprintf (temp, TXT_KICKED_YOU, their->player.callsign);
				ExecMessageBox (NULL, NULL, 1, TXT_OK, &temp);
				if (networkData.nStatus == NETSTAT_PLAYING) {
					gameStates.multi.bIWasKicked=1;
					MultiLeaveGame ();     
					}
				else
					networkData.nStatus = NETSTAT_MENU;
		      }
		   }
 		}
	}
}

//------------------------------------------------------------------------------

void NetworkProcessRequest (tSequencePacket *their)
{
	// Player is ready to receieve a sync packet
	int i;

for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
	if (!CmpNetPlayers (their->player.callsign, netPlayers.players [i].callsign, 
								&their->player.network, &netPlayers.players [i].network)) {
		gameData.multiplayer.players [i].connected = 1;
		break;
		}
	}                       
}

//------------------------------------------------------------------------------

void NetworkProcessPData (char *dataP)
{
Assert (gameData.app.nGameMode & GM_NETWORK);
if (netGame.bShortPackets)
	NetworkReadPDataShortPacket ((tFrameInfoShort *)dataP);
else
	NetworkReadPDataPacket ((tFrameInfo *)dataP);
}

//------------------------------------------------------------------------------

void NetworkProcessNakedPData (char *dataP, int len)
 {
   int nPlayer = dataP [1]; 
   Assert (dataP [0] == PID_NAKED_PDATA);

if (nPlayer < 0) {
#if 1			
   con_printf (CONDBG, "Naked packet is bad!\n");
#endif
	Int3 (); // This packet is bogus!!
	return;
	}

if (!gameData.multigame.bQuitGame && (nPlayer >= gameData.multiplayer.nPlayers)) {
	if (networkData.nStatus != NETSTAT_WAITING) {
		Int3 (); // We missed an important packet!
		NetworkConsistencyError ();
		}
	return;
	}
if (gameStates.app.bEndLevelSequence || (networkData.nStatus == NETSTAT_ENDLEVEL)) {
	int old_Endlevel_sequence = gameStates.app.bEndLevelSequence;
	gameStates.app.bEndLevelSequence = 1;
	MultiProcessBigData ((char *) dataP+2, len-2);
	gameStates.app.bEndLevelSequence = old_Endlevel_sequence;
	return;
	}
MultiProcessBigData ((char *) dataP+2, len-2);
 }

//------------------------------------------------------------------------------

void NetworkProcessNamesReturn (char *dataP)
 {
	tMenuItem m [15];
   char mtext [15][50], temp [50];
	int i, l, nInMenu, gnum, num = 0, count = 5, nPlayers;
   
if (networkData.nNamesInfoSecurity != (*(int *) (dataP+1))) {
#if 1			
  con_printf (CONDBG, "Bad security on names return!\n");
  con_printf (CONDBG, "NIS=%d dataP=%d\n", networkData.nNamesInfoSecurity, (*(int *) (dataP+1)));
#endif
	return;
	}
nPlayers = dataP [count++]; 
if (nPlayers == 255) {
	gameStates.multi.bSurfingNet = 0;
	networkData.nNamesInfoSecurity = -1;
	ExecMessageBox (NULL, NULL, 1, "OK", "That game is refusing\nname requests.\n");
	gameStates.multi.bSurfingNet=1;
	return;
	}
Assert ((nPlayers > 0) && (nPlayers < MAX_NUM_NET_PLAYERS));
memset (m, 0, sizeof (m));
for (i = 0; i < 12; i++) {
	m [i].text = (char *)(mtext + i);
	m [i].nType = NM_TYPE_TEXT;	
	}
#if SECURITY_CHECK
for (gnum = -1, i = 0; i < networkData.nActiveGames; i++) {
	if (networkData.nNamesInfoSecurity == activeNetGames [i].nSecurity) {
		gnum = i;
		break;
		}
	}
if (gnum == -1) {
	gameStates.multi.bSurfingNet = 0;
	networkData.nNamesInfoSecurity = -1;
	ExecMessageBox (NULL, NULL, 1, TXT_OK, TXT_GAME_GONE);
	gameStates.multi.bSurfingNet = 1;
	return;
	}
#else
gnum = 0;
#endif
sprintf (mtext [num], TXT_GAME_PLRS, activeNetGames [gnum].szGameName); 
num++;
for (i = 0; i < nPlayers; i++) {
	l = dataP [count++];
	memcpy (temp, dataP + count, CALLSIGN_LEN + 1);
	count += CALLSIGN_LEN + 1;
	if (gameOpts->multi.bNoRankings)
		sprintf (mtext [num], "%s", temp);
	else
		sprintf (mtext [num], "%s%s", pszRankStrings [l], temp);
	num++;
	}
if (dataP [count] == 99) {
	sprintf (mtext [num++], " ");
	sprintf (mtext [num++], TXT_SHORT_PACKETS2, dataP [count+1] ? TXT_ON : TXT_OFF);
	sprintf (mtext [num++], TXT_PPS2, dataP [count+2]);
	}
bAlreadyShowingInfo = 1;
nInMenu = gameStates.menus.nInMenu;
gameStates.menus.nInMenu = 0;
ExecMenutiny2 (NULL, NULL, num, m, NULL);
gameStates.menus.nInMenu = nInMenu;
bAlreadyShowingInfo = 0;
}

//------------------------------------------------------------------------------

void NetworkProcessMissingObjFrames (char *dataP)
{
	tMissingObjFrames	missingObjFrames;

ReceiveMissingObjFramesPacket (dataP, &missingObjFrames);
if (!networkData.playerRejoining.player.connected)
	networkData.playerRejoining.player.connected = missingObjFrames.nPlayer;
if (missingObjFrames.nPlayer == networkData.playerRejoining.player.connected) {
	networkData.missingObjFrames = missingObjFrames;
	networkData.nSentObjs = -1;				
	networkData.nSyncState = 3;
	}
else
	networkData.missingObjFrames.nFrame = 0;
}

//------------------------------------------------------------------------------

