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
#include <ctype.h>
#ifndef _WIN32
#	include <unistd.h>
#endif
#include <time.h>

#include "inferno.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "console.h"
#include "game.h"
#include "player.h"
#include "key.h"
#include "object.h"
#include "objrender.h"
#include "objsmoke.h"
#include "lightning.h"
#include "physics.h"
#include "error.h"
#include "joy.h"
#include "mono.h"
#include "iff.h"
#include "pcx.h"
#include "timer.h"
#include "cameras.h"
#include "render.h"
#include "laser.h"
#include "omega.h"
#include "screens.h"
#include "textures.h"
#include "slew.h"
#include "gauges.h"
#include "texmap.h"
#include "3d.h"
#include "effects.h"
#include "menu.h"
#include "gameseg.h"
#include "wall.h"
#include "ai.h"
#include "fuelcen.h"
#include "switch.h"
#include "digi.h"
#include "gamesave.h"
#include "scores.h"
#include "ibitblt.h"
#include "u_mem.h"
#include "palette.h"
#include "morph.h"
#include "light.h"
#include "dynlight.h"
#include "newdemo.h"
#include "briefings.h"
#include "collide.h"
#include "weapon.h"
#include "sounds.h"
#include "args.h"
#include "segment.h"
#include "loadgame.h"
#include "gamefont.h"
#include "newmenu.h"
#include "endlevel.h"
#include "interp.h"
#include "multi.h"
#include "network.h"
#include "netmisc.h"
#include "modem.h"
#include "playsave.h"
#include "ctype.h"
#include "fireball.h"
#include "kconfig.h"
#include "config.h"
#include "robot.h"
#include "automap.h"
#include "reactor.h"
#include "powerup.h"
#include "text.h"
#include "cfile.h"
#include "hogfile.h"
#include "piggy.h"
#include "textdata.h"
#include "texmerge.h"
#include "paging.h"
#include "mission.h"
#include "state.h"
#include "songs.h"
#include "gamepal.h"
#include "movie.h"
#include "controls.h"
#include "credits.h"
#include "gamemine.h"
#include "lightmap.h"
#include "polyobj.h"
#include "movie.h"
#include "particles.h"
#include "interp.h"
#include "sphere.h"
#include "hiresmodels.h"
#include "entropy.h"
#include "monsterball.h"
#include "sparkeffect.h"
#include "transprender.h"
#include "slowmotion.h"
#include "soundthreads.h"
#include "menubackground.h"

#if defined (TACTILE)
 #include "tactile.h"
#endif

#ifdef EDITOR
#include "editor/editor.h"
#endif

#include "strutil.h"
#include "rle.h"
#include "input.h"

#define SPAWN_MIN_DIST	I2X (15 * 20)

//------------------------------------------------------------------------------

void ShowLevelIntro (int nLevel);
int StartNewLevelSecret (int nLevel, int bPageInTextures);
void InitPlayerPosition (int bRandom);
void ReturningToLevelMessage (void);
void AdvancingToLevelMessage (void);
void DoEndGame (void);
void AdvanceLevel (int bSecret, int bFromSecret);
void FilterObjectsFromLevel (void);

// From allender -- you'll find these defines in state.c and cntrlcen.c
// since I couldn't think of a good place to put them and i wanted to
// fix this stuff fast!  Sorry about that...

#define SECRETB_FILENAME	"secret.sgb"
#define SECRETC_FILENAME	"secret.sgc"

//gameData.missions.nCurrentLevel starts at 1 for the first level
//-1,-2,-3 are secret levels
//0 means not a real level loaded
// Global variables telling what sort of game we have

//	Extra prototypes declared for the sake of LINT
void InitPlayerStatsNewShip (void);
void CopyDefaultsToRobotsAll (void);

//	HUDClearMessages external, declared in gauges.h
#ifndef _GAUGES_H
void HUDClearMessages (); // From hud.c
#endif

void SetFunctionMode (int);
void InitHoardData ();

extern int nLastLevelPathCreated;
extern int nTimeLastMoved;
extern int nDescentCriticalError;
extern int nLastMsgYCrd;

//--------------------------------------------------------------------

void VerifyConsoleObject (void)
{
Assert (gameData.multiplayer.nLocalPlayer > -1);
Assert (LOCALPLAYER.nObject > -1);
gameData.objs.consoleP = OBJECTS + LOCALPLAYER.nObject;
Assert (gameData.objs.consoleP->info.nType == OBJ_PLAYER);
Assert (gameData.objs.consoleP->info.nId == gameData.multiplayer.nLocalPlayer);
}

//------------------------------------------------------------------------------

int CountRobotsInLevel (void)
{
	int robotCount = 0;
	//int 		i;
	CObject	*objP;

FORALL_ROBOT_OBJS (objP, i)
	robotCount++;
return robotCount;
}

//------------------------------------------------------------------------------

int CountHostagesInLevel (void)
{
	int 		count = 0;
	//int 		i;
	CObject	*objP;

FORALL_STATIC_OBJS (objP, i)
	if (objP->info.nType == OBJ_HOSTAGE)
		count++;
return count;
}

//------------------------------------------------------------------------------
//added 10/12/95: delete buddy bot if coop game.  Probably doesn't really belong here. -MT
void GameStartInitNetworkPlayers (void)
{
	int		i, j, t, bCoop = IsCoopGame,
				segNum, segType,
				playerObjs [MAX_PLAYERS], startSegs [MAX_PLAYERS],
				nPlayers, nMaxPlayers = bCoop ? MAX_COOP_PLAYERS : MAX_PLAYERS;
	CObject	*objP, *nextObjP;

	// Initialize network CPlayerData start locations and CObject numbers

memset (gameStates.multi.bPlayerIsTyping, 0, sizeof (gameStates.multi.bPlayerIsTyping));
//VerifyConsoleObject ();
nPlayers = 0;
j = 0;
for (objP = gameData.objs.lists.all.head; objP; objP = nextObjP) {
	nextObjP = objP->Links (0).next;
	t = objP->info.nType;
	if ((t == OBJ_PLAYER) || (t == OBJ_GHOST) || (t == OBJ_COOP)) {
		i = objP->Index ();
		if ((nPlayers >= nMaxPlayers) || (bCoop ? (j && (t != OBJ_COOP)) : (t == OBJ_COOP)))
			ReleaseObject ((short) i);
		else {
			playerObjs [nPlayers] = i;
			startSegs [nPlayers] = objP->info.nSegment;
			nPlayers++;
			}
		j++;
		}
	else if (t == OBJ_ROBOT) {
		if (ROBOTINFO (objP->info.nId).companion && IsMultiGame)
			ReleaseObject ((short) i);		//kill the buddy in netgames
		}
	}

// the following code takes care of team players being assigned the proper start locations
// in enhanced CTF
for (i = 0; i < nPlayers; i++) {
// find a CPlayerData CObject that resides in a CSegment of proper nType for the current
// CPlayerData start info
	for (j = 0; j < nPlayers; j++) {
		segNum = startSegs [j];
		if (segNum < 0)
			continue;
		segType = bCoop ? SEGMENTS [segNum].m_nType : SEGMENT_IS_NOTHING;
#if 0
		switch (segType) {
			case SEGMENT_IS_GOAL_RED:
			case SEGMENT_IS_TEAM_RED:
				if (i < nPlayers / 2) // (GetTeam (i) != TEAM_RED)
					continue;
				SEGMENTS [segNum].m_nType = SEGMENT_IS_NOTHING;
				break;
			case SEGMENT_IS_GOAL_BLUE:
			case SEGMENT_IS_TEAM_BLUE:
				if (i >= nPlayers / 2) //GetTeam (i) != TEAM_BLUE)
					continue;
				SEGMENTS [segNum].m_nType = SEGMENT_IS_NOTHING;
				break;
			default:
				break;
			}
#endif
		objP = OBJECTS + playerObjs [j];
		objP->SetType (OBJ_PLAYER);
		gameData.multiplayer.playerInit [i].position = objP->info.position;
		gameData.multiplayer.playerInit [i].nSegment = objP->info.nSegment;
		gameData.multiplayer.playerInit [i].nSegType = segType;
		gameData.multiplayer.players [i].nObject = playerObjs [j];
		objP->info.nId = i;
		startSegs [j] = -1;
		break;
		}
	}
gameData.objs.viewerP = gameData.objs.consoleP = OBJECTS.Buffer (); // + LOCALPLAYER.nObject;
gameData.multiplayer.nPlayerPositions = nPlayers;

#if DBG
if (gameData.multiplayer.nPlayerPositions != (bCoop ? 4 : 8)) {
#if TRACE
	//console.printf (CON_VERBOSE, "--NOT ENOUGH MULTIPLAYER POSITIONS IN THIS MINE!--\n");
#endif
	//Int3 (); // Not enough positions!!
}
#endif
if (IS_D2_OEM && IsMultiGame && (gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission) && (gameData.missions.nCurrentLevel == 8)) {
	for (i = 0; i < nPlayers; i++)
		if (gameData.multiplayer.players [i].connected && !(netPlayers.players [i].versionMinor & 0xF0)) {
			ExecMessageBox ("Warning!", NULL, 1, TXT_OK,
								 "This special version of Descent II\nwill disconnect after this level.\nPlease purchase the full version\nto experience all the levels!");
			return;
			}
	}
if (IS_MAC_SHARE && IsMultiGame && (gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission) && (gameData.missions.nCurrentLevel == 4)) {
	for (i = 0; i < nPlayers; i++)
		if (gameData.multiplayer.players [i].connected && !(netPlayers.players [i].versionMinor & 0xF0)) {
			ExecMessageBox ("Warning!", NULL, 1 , TXT_OK,
								 "This shareware version of Descent II\nwill disconnect after this level.\nPlease purchase the full version\nto experience all the levels!");
			return;
			}
	}
}

//------------------------------------------------------------------------------

void GameStartRemoveUnusedPlayers (void)
{
	int i;

	// 'Remove' the unused players

if (IsMultiGame) {
	for (i = 0; i < gameData.multiplayer.nPlayerPositions; i++) {
		if (!gameData.multiplayer.players [i].connected || (i >= gameData.multiplayer.nPlayers))
			MultiMakePlayerGhost (i);
		}
	}
else {		// Note link to above if!!!
	for (i = 1; i < gameData.multiplayer.nPlayerPositions; i++)
		ReleaseObject ((short) gameData.multiplayer.players [i].nObject);
	}
}

//------------------------------------------------------------------------------

// Setup CPlayerData for new game
void InitPlayerStatsGame (void)
{
LOCALPLAYER.score = 0;
LOCALPLAYER.lastScore = 0;
LOCALPLAYER.lives = INITIAL_LIVES;
LOCALPLAYER.level = 1;
LOCALPLAYER.timeLevel = 0;
LOCALPLAYER.timeTotal = 0;
LOCALPLAYER.hoursLevel = 0;
LOCALPLAYER.hoursTotal = 0;
LOCALPLAYER.energy = INITIAL_ENERGY;
LOCALPLAYER.shields = gameStates.gameplay.xStartingShields;
LOCALPLAYER.nKillerObj = -1;
LOCALPLAYER.netKilledTotal = 0;
LOCALPLAYER.netKillsTotal = 0;
LOCALPLAYER.numKillsLevel = 0;
LOCALPLAYER.numKillsTotal = 0;
LOCALPLAYER.numRobotsLevel = 0;
LOCALPLAYER.numRobotsTotal = 0;
LOCALPLAYER.nKillGoalCount = 0;
LOCALPLAYER.hostages.nRescued = 0;
LOCALPLAYER.hostages.nLevel = 0;
LOCALPLAYER.hostages.nTotal = 0;
LOCALPLAYER.laserLevel = 0;
LOCALPLAYER.flags = 0;
LOCALPLAYER.nCloaks =
LOCALPLAYER.nInvuls = 0;
InitPlayerStatsNewShip ();
gameStates.app.bFirstSecretVisit = 1;
}

//------------------------------------------------------------------------------

#define TEAMKEY(_p)	((GetTeam (_p) == TEAM_RED) ? KEY_RED : KEY_BLUE)

void InitAmmoAndEnergy (void)
{
if (LOCALPLAYER.energy < INITIAL_ENERGY)
	LOCALPLAYER.energy = INITIAL_ENERGY;
if (LOCALPLAYER.shields < gameStates.gameplay.xStartingShields)
	LOCALPLAYER.shields = gameStates.gameplay.xStartingShields;
if (LOCALPLAYER.primaryWeaponFlags & (1 << OMEGA_INDEX))
	SetMaxOmegaCharge ();
if (LOCALPLAYER.secondaryAmmo [0] < 2 + NDL - gameStates.app.nDifficultyLevel)
	LOCALPLAYER.secondaryAmmo [0] = 2 + NDL - gameStates.app.nDifficultyLevel;
}

//------------------------------------------------------------------------------

// Setup CPlayerData for new level (After completion of previous level)
void InitPlayerStatsLevel (int bSecret)
{
LOCALPLAYER.lastScore = LOCALPLAYER.score;
LOCALPLAYER.level = gameData.missions.nCurrentLevel;
if (!networkData.nJoinState) {
	LOCALPLAYER.timeLevel = 0;
	LOCALPLAYER.hoursLevel = 0;
	}
LOCALPLAYER.nKillerObj = -1;
LOCALPLAYER.numKillsLevel = 0;
LOCALPLAYER.numRobotsLevel = CountRobotsInLevel ();
LOCALPLAYER.numRobotsTotal += LOCALPLAYER.numRobotsLevel;
LOCALPLAYER.hostages.nLevel = CountHostagesInLevel ();
LOCALPLAYER.hostages.nTotal += LOCALPLAYER.hostages.nLevel;
LOCALPLAYER.hostages.nOnBoard = 0;
if (!bSecret) {
	InitAmmoAndEnergy ();
	LOCALPLAYER.flags &=
		~(PLAYER_FLAGS_INVULNERABLE | PLAYER_FLAGS_CLOAKED | PLAYER_FLAGS_FULLMAP | KEY_BLUE | KEY_RED | KEY_GOLD);
	LOCALPLAYER.cloakTime = 0;
	LOCALPLAYER.invulnerableTime = 0;
	if (IsMultiGame && !IsCoopGame) {
		if (IsTeamGame && gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bTeamDoors)
			LOCALPLAYER.flags |= KEY_GOLD | TEAMKEY (gameData.multiplayer.nLocalPlayer);
		else
			LOCALPLAYER.flags |= (KEY_BLUE | KEY_RED | KEY_GOLD);
		}
	}
else if (gameStates.app.bD1Mission)
	InitAmmoAndEnergy ();
gameStates.app.bPlayerIsDead = 0; // Added by RH
LOCALPLAYER.homingObjectDist = -F1_0; // Added by RH
gameData.laser.xLastFiredTime =
gameData.laser.xNextFireTime =
gameData.missiles.xLastFiredTime =
gameData.missiles.xNextFireTime = gameData.time.xGame; // added by RH, solved demo playback bug
Controls [0].afterburnerState = 0;
gameStates.gameplay.bLastAfterburnerState = 0;
DigiKillSoundLinkedToObject (LOCALPLAYER.nObject);
InitGauges ();
#ifdef TACTILE
if (TactileStick)
	tactile_set_button_jolt ();
#endif
gameData.objs.missileViewerP = NULL;
}

//------------------------------------------------------------------------------

void InitAIForShip (void);

//------------------------------------------------------------------------------

void AddPlayerLoadout (void)
{
if (gameStates.app.bHaveExtraGameInfo [IsMultiGame])
	{
	LOCALPLAYER.primaryWeaponFlags |= extraGameInfo [IsMultiGame].loadout.nGuns;
	if (gameStates.app.bD1Mission)
	   LOCALPLAYER.primaryWeaponFlags &= ~(HAS_FLAG (HELIX_INDEX) | HAS_FLAG (GAUSS_INDEX) | HAS_FLAG (PHOENIX_INDEX) | HAS_FLAG (OMEGA_INDEX));
	if (!gameStates.app.bD1Mission && (extraGameInfo [IsMultiGame].loadout.nGuns & HAS_FLAG (SUPER_LASER_INDEX)))
		LOCALPLAYER.laserLevel = MAX_LASER_LEVEL + 2;
	else if (extraGameInfo [IsMultiGame].loadout.nGuns & HAS_FLAG (LASER_INDEX))
		LOCALPLAYER.laserLevel = MAX_LASER_LEVEL;
	if (extraGameInfo [IsMultiGame].loadout.nGuns & (HAS_FLAG (VULCAN_INDEX) | HAS_FLAG (GAUSS_INDEX)))
		LOCALPLAYER.primaryAmmo [1] = GAUSS_WEAPON_AMMO_AMOUNT;
	LOCALPLAYER.flags |= extraGameInfo [IsMultiGame].loadout.nDevices;
	if (extraGameInfo [1].bDarkness)
		LOCALPLAYER.flags |= PLAYER_FLAGS_HEADLIGHT;
	if (gameStates.app.bD1Mission)
		{
	   LOCALPLAYER.primaryWeaponFlags &= ~(HAS_FLAG (HELIX_INDEX) | HAS_FLAG (GAUSS_INDEX) | HAS_FLAG (PHOENIX_INDEX) | HAS_FLAG (OMEGA_INDEX));
	   LOCALPLAYER.flags &= ~(PLAYER_FLAGS_FULLMAP | PLAYER_FLAGS_AMMO_RACK | PLAYER_FLAGS_CONVERTER | PLAYER_FLAGS_AFTERBURNER | PLAYER_FLAGS_HEADLIGHT);
	   }
	}
}

//------------------------------------------------------------------------------

// Setup CPlayerData for a brand-new ship
void InitPlayerStatsNewShip (void)
{
	int	i;

if (gameData.demo.nState == ND_STATE_RECORDING) {
	NDRecordLaserLevel (LOCALPLAYER.laserLevel, 0);
	NDRecordPlayerWeapon (0, 0);
	NDRecordPlayerWeapon (1, 0);
	}

LOCALPLAYER.energy = INITIAL_ENERGY;
LOCALPLAYER.shields = gameStates.gameplay.xStartingShields;
LOCALPLAYER.laserLevel = 0;
LOCALPLAYER.nKillerObj = -1;
LOCALPLAYER.hostages.nOnBoard = 0;

gameData.physics.xAfterburnerCharge = 0;

for (i = 0; i < MAX_PRIMARY_WEAPONS; i++) {
	LOCALPLAYER.primaryAmmo [i] = 0;
	bLastPrimaryWasSuper [i] = 0;
	}
for (i = 1; i < MAX_SECONDARY_WEAPONS; i++) {
	LOCALPLAYER.secondaryAmmo [i] = 0;
	bLastSecondaryWasSuper [i] = 0;
	}
LOCALPLAYER.secondaryAmmo [0] = 2 + NDL - gameStates.app.nDifficultyLevel;
LOCALPLAYER.primaryWeaponFlags = HAS_LASER_FLAG;
LOCALPLAYER.secondaryWeaponFlags = HAS_CONCUSSION_FLAG;
gameData.weapons.nOverridden = 0;
gameData.weapons.nPrimary = 0;
gameData.weapons.nSecondary = 0;
LOCALPLAYER.flags &= ~
	(PLAYER_FLAGS_QUAD_LASERS |
	 PLAYER_FLAGS_AFTERBURNER |
	 PLAYER_FLAGS_CLOAKED |
	 PLAYER_FLAGS_INVULNERABLE |
	 PLAYER_FLAGS_FULLMAP |
	 PLAYER_FLAGS_CONVERTER |
	 PLAYER_FLAGS_AMMO_RACK |
	 PLAYER_FLAGS_HEADLIGHT |
	 PLAYER_FLAGS_HEADLIGHT_ON |
	 PLAYER_FLAGS_FLAG);
AddPlayerLoadout ();
LOCALPLAYER.cloakTime = 0;
LOCALPLAYER.invulnerableTime = 0;
gameStates.app.bPlayerIsDead = 0;		//CPlayerData no longer dead
LOCALPLAYER.homingObjectDist = -F1_0; // Added by RH
Controls [0].afterburnerState = 0;
gameStates.gameplay.bLastAfterburnerState = 0;
DigiKillSoundLinkedToObject (LOCALPLAYER.nObject);
gameData.objs.missileViewerP = NULL;		///reset missile camera if out there
#ifdef TACTILE
	if (TactileStick)
	{
	tactile_set_button_jolt ();
	}
#endif
InitAIForShip ();
}

//------------------------------------------------------------------------------

void InitStuckObjects (void);

#ifdef EDITOR

extern int gameData.segs.bHaveSlideSegs;

//reset stuff so game is semi-Normal when playing from editor
void editor_reset_stuff_onLevel ()
{
	GameStartInitNetworkPlayers ();
	InitPlayerStatsLevel (0);
	gameData.objs.viewerP = gameData.objs.consoleP;
	gameData.objs.consoleP = gameData.objs.viewerP = OBJECTS + LOCALPLAYER.nObject;
	gameData.objs.consoleP->id=gameData.multiplayer.nLocalPlayer;
	gameData.objs.consoleP->controlType = CT_FLYING;
	gameData.objs.consoleP->movementType = MT_PHYSICS;
	gameStates.app.bGameSuspended = 0;
	VerifyConsoleObject ();
	gameData.reactor.bDestroyed = 0;
	if (gameData.demo.nState != ND_STATE_PLAYBACK)
		GameStartRemoveUnusedPlayers ();
	InitCockpit ();
	InitRobotsForLevel ();
	InitAIObjects ();
	MorphInit ();
	InitAllMatCens ();
	InitPlayerStatsNewShip ();
	InitReactorForLevel (0);
	automap.ClearVisited ();
	InitStuckObjects ();
	InitThiefForLevel ();

	gameData.segs.bHaveSlideSegs = 0;
}
#endif

//------------------------------------------------------------------------------

//do whatever needs to be done when a CPlayerData dies in multiplayer
void DoGameOver (void)
{
//	ExecMessageBox (TXT_GAME_OVER, 1, TXT_OK, "");
if (gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission)
	MaybeAddPlayerScore (0);
SetFunctionMode (FMODE_MENU);
gameData.app.nGameMode = GM_GAME_OVER;
longjmp (gameExitPoint, 0);		// Exit out of game loop

}

//------------------------------------------------------------------------------

//update various information about the CPlayerData
void UpdatePlayerStats (void)
{
LOCALPLAYER.timeLevel += gameData.time.xFrame;	//the never-ending march of time...
if (LOCALPLAYER.timeLevel > I2X (3600))	{
	LOCALPLAYER.timeLevel -= I2X (3600);
	LOCALPLAYER.hoursLevel++;
	}
LOCALPLAYER.timeTotal += gameData.time.xFrame;	//the never-ending march of time...
if (LOCALPLAYER.timeTotal > I2X (3600))	{
	LOCALPLAYER.timeTotal -= I2X (3600);
	LOCALPLAYER.hoursTotal++;
	}
}

//------------------------------------------------------------------------------

//go through this level and start any effect sounds
void SetSoundSources (void)
{
	short			nSegment, nSide, nConnSeg, nConnSide, nSound;
	CSegment*	segP, * connSegP;
	CObject*		objP;
	int			nOvlTex, nEffect;
	//int			i;

gameStates.sound.bD1Sound = gameStates.app.bD1Mission && gameStates.app.bHaveD1Data && gameOpts->sound.bUseD1Sounds && !gameOpts->sound.bHires;
DigiInitSounds ();		//clear old sounds
gameStates.sound.bDontStartObjects = 1;
for (segP = SEGMENTS.Buffer (), nSegment = 0; nSegment <= gameData.segs.nLastSegment; segP++, nSegment++)
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (!(segP->IsDoorWay (nSide, NULL) & WID_RENDER_FLAG))
			continue;
		nEffect = (nOvlTex = segP->m_sides [nSide].m_nOvlTex) ? gameData.pig.tex.tMapInfoP [nOvlTex].nEffectClip : -1;
		if (nEffect < 0)
			nEffect = gameData.pig.tex.tMapInfoP [segP->m_sides [nSide].m_nBaseTex].nEffectClip;
		if (nEffect < 0)
			continue;
		if ((nSound = gameData.eff.effectP [nEffect].nSound) == -1)
			continue;
		nConnSeg = segP->m_children [nSide];

		//check for sound on other CSide of CWall.  Don't add on
		//both walls if sound travels through CWall.  If sound
		//does travel through CWall, add sound for lower-numbered
		//CSegment.

		if (IS_CHILD (nConnSeg) && (nConnSeg < nSegment) &&
			 (segP->IsDoorWay (nSide, NULL) & (WID_FLY_FLAG | WID_RENDPAST_FLAG))) {
			connSegP = SEGMENTS + segP->m_children [nSide];
			nConnSide = segP->ConnectedSide (connSegP);
			if (connSegP->m_sides [nConnSide].m_nOvlTex == segP->m_sides [nSide].m_nOvlTex)
				continue;		//skip this one
			}
		DigiLinkSoundToPos (nSound, nSegment, nSide, segP->SideCenter (nSide), 1, F1_0 / 2);
		}

if (0 <= (nSound = DigiGetSoundByName ("explode2"))) {
	FORALL_STATIC_OBJS (objP, i)
		if (objP->info.nType == OBJ_EXPLOSION) {
			objP->info.renderType = RT_POWERUP;
			objP->rType.vClipInfo.nClipIndex = objP->info.nId;
			DigiSetObjectSound (objP->Index (), nSound, NULL);
			}
	}
//gameStates.sound.bD1Sound = 0;
gameStates.sound.bDontStartObjects = 0;
}

//	------------------------------------------------------------------------------

void SetVertigoRobotFlags (void)
{
	CObject	*objP;
	//int		i;

gameData.objs.nVertigoBotFlags = 0;
FORALL_ROBOT_OBJS (objP, i)
	if ((objP->info.nId >= 66) && !IS_BOSS (objP))
		gameData.objs.nVertigoBotFlags |= (1 << (objP->info.nId - 64));
}

//------------------------------------------------------------------------------

char *LevelName (int nLevel)
{
return gameStates.app.bAutoRunMission ? szAutoMission : (nLevel < 0) ?
		 gameData.missions.szSecretLevelNames [-nLevel-1] :
		 gameData.missions.szLevelNames [nLevel-1];
}

//------------------------------------------------------------------------------

char *MakeLevelFilename (int nLevel, char *pszFilename, const char *pszFileExt)
{
CFile::ChangeFilenameExtension (pszFilename, strlwr (LevelName (nLevel)), pszFileExt);
return pszFilename;
}

//------------------------------------------------------------------------------

char *LevelSongName (int nLevel)
{
return gameStates.app.bAutoRunMission ? 0 :
			(nLevel < 0) ?
			gameData.missions.szSongNames [-nLevel-1] :
			gameData.missions.szSongNames [nLevel-1];
}

//------------------------------------------------------------------------------
//load a level off disk. level numbers start at 1.  Secret levels are -1,-2,-3

extern char szAutoMission [255];

int LoadLevel (int nLevel, int bPageInTextures, int bRestore)
{
	char		*pszLevelName;
	char		szHogName [FILENAME_LEN];
	CPlayerData	save_player;
	int		nRooms, bRetry = 0, nLoadRes, nCurrentLevel = gameData.missions.nCurrentLevel;

/*---*/PrintLog ("Loading level...\n");
lightmapManager.Destroy ();
gameStates.app.bBetweenLevels = 1;
gameStates.app.bFreeCam = 0;
gameStates.app.bGameRunning = 0;
gameStates.app.bPlayerExploded = 0;
gameData.physics.side.nSegment = -1;
gameData.physics.side.nSide = -1;
memset (&gameData.marker, 0, sizeof (gameData.marker));
gameData.marker.nLast = -1;
gameData.songs.tPos =
gameData.songs.tSlowDown = 0;
gameStates.gameplay.bKillBossCheat = 0;
gameStates.render.nFlashScale = F1_0;
gameOpts->app.nScreenShotInterval = 0;	//better reset this every time a level is loaded
automap.m_bFull = 0;
gameData.render.ogl.nHeadlights = -1;
gameData.render.nColoredFaces = 0;
gameData.app.nFrameCount = 0;
gameData.app.nMineRenderCount = 0;
memset (&gameData.objs.lists, 0, sizeof (gameData.objs.lists));
memset (gameData.app.semaphores, 0, sizeof (gameData.app.semaphores));
transpItems.nMinOffs = ITEM_DEPTHBUFFER_SIZE;
transpItems.nMaxOffs = 0;
#if PROFILING
memset (&gameData.profiler, 0, sizeof (gameData.profiler));
#endif
DigiKillSoundLinkedToObject (LOCALPLAYER.nObject);
memset (gameData.stats.player, 0, sizeof (tPlayerStats));
memset (gameData.render.mine.bObjectRendered, 0xff, sizeof (gameData.render.mine.bObjectRendered));
memset (gameData.render.mine.bRenderSegment, 0xff, sizeof (gameData.render.mine.bRenderSegment));
memset (gameData.render.mine.bCalcVertexColor, 0, sizeof (gameData.render.mine.bCalcVertexColor));
memset (gameData.multiplayer.weaponStates, 0xff, sizeof (gameData.multiplayer.weaponStates));
memset (gameData.multiplayer.bWasHit, 0, sizeof (gameData.multiplayer.bWasHit));
memset (gameData.multiplayer.nLastHitTime, 0, sizeof (gameData.multiplayer.nLastHitTime));
memset (gameData.weapons.firing, 0, sizeof (gameData.weapons.firing));
gameData.objs.objects.Clear ();
gameData.objs.lightObjs.Clear (0xff);
gameData.render.faceIndex [0].roots.Clear (0xff);
gameData.render.faceIndex [1].roots.Clear (0xff);
gameData.render.faceIndex [0].tails.Clear (0xff);
gameData.render.faceIndex [1].tails.Clear (0xff);
memset (&gameData.render.lights.dynamic.shader.index, 0, sizeof (gameData.render.lights.dynamic.shader.index));
memset (gameData.objs.bWantEffect, 0, sizeof (gameData.objs.bWantEffect));
memset (gameData.objs.guidedMissile, 0, sizeof (gameData.objs.guidedMissile));
gameData.render.faceIndex [0].nUsedFaces = 0;
gameData.render.faceIndex [0].nUsedKeys = 0;
gameData.render.faceIndex [1].nUsedFaces = MAX_FACES;
gameData.render.faceIndex [1].nUsedKeys = 0;
omegaLightnings.Init ();
gameData.multiplayer.bMoving = -1;
#if 1
/*---*/PrintLog ("   stopping music\n");
SongsStopAll ();
/*---*/PrintLog ("   stopping sounds\n");
DigiStopAllChannels ();
/*---*/PrintLog ("   reconfiguring audio\n");
gameData.missions.nCurrentLevel = nLevel;
if (!bRestore) {
	gameStates.gameplay.slowmo [0].fSpeed =
	gameStates.gameplay.slowmo [1].fSpeed = 1;
	gameStates.gameplay.slowmo [0].nState =
	gameStates.gameplay.slowmo [1].nState = 0;
	SpeedupSound ();
	}
/*---*/PrintLog ("   unloading textures\n");
PiggyBitmapPageOutAll (0);
/*---*/PrintLog ("   unloading custom sounds\n");
FreeSoundReplacements ();
/*---*/PrintLog ("   unloading hardware lights\n");
RemoveDynLights ();
/*---*/PrintLog ("   unloading hires models\n");
FreeHiresModels (1);
/*---*/PrintLog ("   unloading cambot\n");
UnloadCamBot ();
/*---*/PrintLog ("   unloading additional models\n");
BMFreeExtraModels ();
/*---*/PrintLog ("   unloading additional model textures\n");
BMFreeExtraObjBitmaps ();
/*---*/PrintLog ("   unloading additional model textures\n");
PiggyFreeHiresAnimations ();
/*---*/PrintLog ("   freeing spark effect buffers\n");
sparkManager.Destroy ();
/*---*/PrintLog ("   freeing sound buffers\n");
DigiFreeSoundBufs ();
/*---*/PrintLog ("   freeing auxiliary poly model data\n");
G3FreeAllPolyModelItems ();
/*---*/PrintLog ("   restoring default robot settings\n");
RestoreDefaultRobots ();
if (gameData.bots.bReplacementsLoaded) {
	/*---*/PrintLog ("   loading default robot settings\n");
	ReadHamFile ();		//load original data
	gameData.bots.bReplacementsLoaded = 0;
	}
if (gameData.missions.nEnhancedMission) {
	char t [FILENAME_LEN];

	sprintf (t,"%s.ham", gameStates.app.szCurrentMissionFile);
	/*---*/PrintLog ("   reading additional robots\n");
	switch (BMReadExtraRobots (t, gameFolders.szMissionDirs [0], gameData.missions.nEnhancedMission)) {
		case -1:
			gameStates.app.bBetweenLevels = 0;
			gameData.missions.nCurrentLevel = nCurrentLevel;
			return 0;
		case 1:
			break;
		default:
			if (0 > BMReadExtraRobots ("d2x.ham", gameFolders.szMissionDir, gameData.missions.nEnhancedMission)) {
				gameStates.app.bBetweenLevels = 0;
				gameData.missions.nCurrentLevel = nCurrentLevel;
				return 0;
				}
		}
	strncpy (t, gameStates.app.szCurrentMissionFile, 6);
	strcat (t, "-l.mvl");
	/*---*/PrintLog ("   initializing additional robot movies\n");
	InitExtraRobotMovie (t);
	}
#endif
/*---*/PrintLog ("   Destroying camera objects\n");
cameraManager.Destroy ();
/*---*/PrintLog ("   Destroying particle data\n");
particleManager.Shutdown ();
/*---*/PrintLog ("   Destroying lightning data\n");
omegaLightnings.Destroy (-1);
lightningManager.Shutdown (1);
/*---*/PrintLog ("   Initializing smoke manager\n");
InitObjectSmoke ();
gameData.pig.tex.bitmapColors.Clear ();
memset (gameData.models.thrusters, 0, sizeof (gameData.models.thrusters));
gameData.render.lights.flicker.nLights = 0;
save_player = LOCALPLAYER;
#if 0
Assert (gameStates.app.bAutoRunMission ||
		  ((nLevel <= gameData.missions.nLastLevel) &&
		   (nLevel >= gameData.missions.nLastSecretLevel) &&
			(nLevel != 0)));
#endif
if (!gameStates.app.bAutoRunMission &&
	 (!nLevel || (nLevel > gameData.missions.nLastLevel) || (nLevel < gameData.missions.nLastSecretLevel))) {
	gameStates.app.bBetweenLevels = 0;
	gameData.missions.nCurrentLevel = nCurrentLevel;
	Warning ("Invalid level number!");
	return 0;
	}
strlwr (pszLevelName = LevelName (nLevel));
/*---*/PrintLog ("   loading level '%s'\n", pszLevelName);
#if 0
CCanvas::SetCurrent (NULL);
GrClearCanvas (BLACK_RGBA);		//so palette switching is less obvious
#endif
nLastMsgYCrd = -1;		//so we don't restore backgound under msg
/*---*/PrintLog ("   loading palette\n");
paletteManager.LoadEffect  ();
 //paletteManager.Load ("groupa.256", NULL, 0, 0, 1);		//don't change screen
//if (!gameOpts->menus.nStyle)
//	NMLoadBackground (NULL, NULL, 0);
ShowBoxedMessage (TXT_LOADING);
/*---*/PrintLog ("   loading level data\n");
gameStates.app.bD1Mission = gameStates.app.bAutoRunMission ? (strstr (szAutoMission, "rdl") != NULL) :
									 (gameData.missions.list [gameData.missions.nCurrentMission].nDescentVersion == 1);
SEGMENTS.Clear (0xff);
/*---*/PrintLog ("   loading texture brightness info\n");
SetDataVersion (-1);

memcpy (gameData.pig.tex.brightness.Buffer (),
		  gameData.pig.tex.defaultBrightness [gameStates.app.bD1Mission].Buffer (),
		  gameData.pig.tex.brightness. Size ());
LoadTextureBrightness (pszLevelName, NULL);
gameData.render.color.textures = gameData.render.color.defaultTextures [gameStates.app.bD1Mission];
LoadTextureColors (pszLevelName, NULL);
InitTexColors ();
if (gameStates.app.bD1Mission)
	LoadD1BitmapReplacements ();

for (;;) {
	if (!(nLoadRes = LoadLevelSub (pszLevelName, nLevel)))
		break;	//actually load the data from disk!
	nLoadRes = 1;
	if (bRetry)
		break;
	if (strstr (hogFileManager.AltFiles ().szName, ".hog"))
		break;
	sprintf (szHogName, "%s%s%s%s",
				gameFolders.szMissionDir, *gameFolders.szMissionDir ? "/" : "",
				gameFolders.szMsnSubDir, pszLevelName);
	if (!hogFileManager.UseAlt (szHogName))
		break;
	bRetry = 1;
	};
if (nLoadRes) {
	/*---*/PrintLog ("Couldn't load '%s' (%d)\n", pszLevelName, nLoadRes);
	gameStates.app.bBetweenLevels = 0;
	gameData.missions.nCurrentLevel = nCurrentLevel;
	Warning (TXT_LOAD_ERROR, pszLevelName);
	return 0;
	}

paletteManager.SetGame (paletteManager.Load (szCurrentLevelPalette, pszLevelName, 1, 1, 1));		//don't change screen
InitGaugeCanvases ();
ResetPogEffects ();
if (gameStates.app.bD1Mission) {
	/*---*/PrintLog ("   loading Descent 1 textures\n");
	LoadD1BitmapReplacements ();
	if (bPageInTextures)
		PiggyLoadLevelData ();
	}
else {
	if (bPageInTextures)
		PiggyLoadLevelData ();
	LoadTextData (pszLevelName, ".msg", gameData.messages);
	LoadTextData (pszLevelName, ".snd", &gameData.sounds);
	LoadBitmapReplacements (pszLevelName);
	LoadSoundReplacements (pszLevelName);
	}
/*---*/PrintLog ("   loading endlevel data\n");
LoadEndLevelData (nLevel);
/*---*/PrintLog ("   loading cambot\n");
gameData.bots.nCamBotId = (LoadRobotReplacements ("cambot.hxm", 1, 0) > 0) ? gameData.bots.nTypes [0] - 1 : -1;
gameData.bots.nCamBotModel = gameData.models.nPolyModels - 1;
/*---*/PrintLog ("   loading replacement robots\n");
if (0 > LoadRobotReplacements (pszLevelName, 0, 0)) {
	gameStates.app.bBetweenLevels = 0;
	gameData.missions.nCurrentLevel = nCurrentLevel;
	return 0;
	}
LoadHiresModels (1);
/*---*/PrintLog ("   initializing cambot\n");
InitCamBots (0);
networkData.nSegmentCheckSum = CalcSegmentCheckSum ();
ResetNetworkObjects ();
ResetChildObjects ();
externalView.Reset (-1, -1);
ResetPlayerPaths ();
FixObjectSizes ();
/*---*/PrintLog ("   counting entropy rooms\n");
nRooms = CountRooms ();
if (gameData.app.nGameMode & GM_ENTROPY) {
	if (!nRooms) {
		Warning (TXT_NO_ENTROPY);
		gameData.app.nGameMode &= ~GM_ENTROPY;
		gameData.app.nGameMode |= GM_TEAM;
		}
	}
else if ((gameData.app.nGameMode & (GM_CAPTURE | GM_HOARD)) ||
			((gameData.app.nGameMode & GM_MONSTERBALL) == GM_MONSTERBALL)) {
/*---*/PrintLog ("   gathering CTF+ flag goals\n");
	if (GatherFlagGoals () != 3) {
		Warning (TXT_NO_CTF);
		gameData.app.nGameMode &= ~GM_CAPTURE;
		gameData.app.nGameMode |= GM_TEAM;
		}
	}
gameData.render.lights.segDeltas.Clear ();
/*---*/PrintLog ("   initializing door animations\n");
InitDoorAnims ();
LOCALPLAYER = save_player;
gameData.hoard.nMonsterballSeg = -1;
/*---*/PrintLog ("   initializing sound sources\n");
SetSoundSources ();
if (!IsMultiGame)
	InitEntropySettings (0);	//required for repair centers
PlayLevelSong (gameData.missions.nCurrentLevel, 1);
ClearBoxedMessage ();		//remove message before new palette loaded
paletteManager.LoadEffect  ();		//actually load the palette
/*---*/PrintLog ("   rebuilding OpenGL texture data\n");
/*---*/PrintLog ("      rebuilding effects\n");
if (!bRestore) {
	RebuildRenderContext (1);
	SetRenderQuality ();
	}
ResetPingStats ();
gameStates.gameplay.nDirSteps = 0;
gameStates.gameplay.bMineMineCheat = 0;
gameStates.render.bAllVisited = 0;
gameStates.render.bViewDist = 1;
gameStates.render.bHaveSkyBox = -1;
gameStates.app.cheats.nUnlockLevel = 0;
gameStates.render.nFrameFlipFlop = 0;
gameStates.app.bUsingConverter = 0;
/*---*/PrintLog ("   resetting color information\n");
gameData.render.color.vertices.Clear ();
gameData.render.color.segments.Clear ();
/*---*/PrintLog ("   resetting speed boost information\n");
gameData.objs.speedBoost.Clear ();
if (!gameStates.render.bHaveStencilBuffer)
	extraGameInfo [0].bShadows = 0;
D2SetCaption ();
if (!bRestore) {
	gameData.render.lights.bInitDynColoring = 1;
	gameData.omega.xCharge [IsMultiGame] = MAX_OMEGA_CHARGE;
	SetMaxOmegaCharge ();
	ConvertObjects ();
	ComputeStaticDynLighting (nLevel);
	SetEquipGenStates ();
	SetupEffects ();
	gameData.time.nPaused = 0;
	}
LoadExtraImages ();
CreateShieldSphere ();
PrintLog ("   initializing energy spark render data\n");
sparkManager.Setup ();
PrintLog ("   setting robot generator vertigo robot flags\n");
SetVertigoRobotFlags ();
PrintLog ("   initializing debris collision handlers\n");
SetDebrisCollisions ();
PrintLog ("   building sky box segment list\n");
BuildSkyBoxSegList ();
if (RENDERPATH)
	gameOpts->render.bDepthSort = 1;
gameStates.app.bBetweenLevels = 0;
return 1;
}

//------------------------------------------------------------------------------

//sets up gameData.multiplayer.nLocalPlayer & gameData.objs.consoleP
void InitMultiPlayerObject (void)
{
Assert ((gameData.multiplayer.nLocalPlayer >= 0) && (gameData.multiplayer.nLocalPlayer < MAX_PLAYERS));
if (gameData.multiplayer.nLocalPlayer != 0)	{
	gameData.multiplayer.players [0] = LOCALPLAYER;
	gameData.multiplayer.nLocalPlayer = 0;
	}
LOCALPLAYER.nObject = 0;
LOCALPLAYER.nInvuls =
LOCALPLAYER.nCloaks = 0;
gameData.objs.consoleP = OBJECTS + LOCALPLAYER.nObject;
gameData.objs.consoleP->SetType (OBJ_PLAYER);
gameData.objs.consoleP->info.nId = gameData.multiplayer.nLocalPlayer;
gameData.objs.consoleP->info.controlType	= CT_FLYING;
gameData.objs.consoleP->info.movementType = MT_PHYSICS;
gameStates.entropy.nTimeLastMoved = -1;
}

//------------------------------------------------------------------------------

//starts a new game on the given level
int StartNewGame (int nStartLevel)
{
	int result;

gameData.app.nGameMode = GM_NORMAL;
SetFunctionMode (FMODE_GAME);
gameData.missions.nNextLevel = 0;
InitMultiPlayerObject ();				//make sure CPlayerData's CObject set up
InitPlayerStatsGame ();		//clear all stats
gameData.multiplayer.nPlayers = 1;
gameData.objs.nLastObject [0] = 0;
networkData.bNewGame = 0;
if (nStartLevel < 0)
	result = StartNewLevelSecret (nStartLevel, 0);
else
	result = StartNewLevel (nStartLevel, 0);
if (result) {
	LOCALPLAYER.startingLevel = nStartLevel;		// Mark where they started
	GameDisableCheats ();
	InitSeismicDisturbances ();
	}
return result;
}

//------------------------------------------------------------------------------

#ifndef _NETWORK_H
extern int NetworkEndLevelPoll2 (int nitems, tMenuItem * menus, int * key, int nCurItem); // network.c
#endif

//	Does the bonus scoring.
//	Call with deadFlag = 1 if CPlayerData died, but deserves some portion of bonus (only skill points), anyway.
void DoEndLevelScoreGlitz (int network)
{
	#define N_GLITZITEMS 11

	int			nLevelPoints, nSkillPoints, nEnergyPoints, nShieldPoints, nHostagePoints, nAllHostagePoints, nEndGamePoints;
	char			szAllHostages [64];
	char			szEndGame [64];
	char			szMenu [N_GLITZITEMS+1][40];
	tMenuItem	m [N_GLITZITEMS+1];
	int			i, c;
	char			szTitle [128];
	int			bIsLastLevel = 0;
	int			nMineLevel = 0;

DigiKillSoundLinkedToObject (LOCALPLAYER.nObject);
DigiStopAllChannels ();
SetScreenMode (SCREEN_MENU);		//go into menu mode
if (gameStates.app.bHaveExtraData)

#ifdef TACTILE
if (TactileStick)
	ClearForces ();
#endif

	//	Compute level CPlayerData is on, deal with secret levels (negative numbers)
nMineLevel = LOCALPLAYER.level;
if (nMineLevel < 0)
	nMineLevel *= - (gameData.missions.nLastLevel / gameData.missions.nSecretLevels);
else if (nMineLevel == 0)
	nMineLevel = 1;
nEndGamePoints =
nSkillPoints =
nShieldPoints =
nEnergyPoints =
nHostagePoints =
nAllHostagePoints = 0;
bIsLastLevel = 0;
nLevelPoints = LOCALPLAYER.score - LOCALPLAYER.lastScore;
szAllHostages [0] = 0;
szEndGame [0] = 0;
if (!gameStates.app.cheats.bEnabled) {
	if (gameStates.app.nDifficultyLevel > 1) {
		nSkillPoints = nLevelPoints * (gameStates.app.nDifficultyLevel) / 4;
		nSkillPoints -= nSkillPoints % 100;
		}
	else
		nSkillPoints = 0;
	nShieldPoints = X2I (LOCALPLAYER.shields) * 5 * nMineLevel;
	nEnergyPoints = X2I (LOCALPLAYER.energy) * 2 * nMineLevel;
	nHostagePoints = LOCALPLAYER.hostages.nOnBoard * 500 * (gameStates.app.nDifficultyLevel+1);
	nShieldPoints -= nShieldPoints % 50;
	nEnergyPoints -= nEnergyPoints % 50;
	if ((LOCALPLAYER.hostages.nOnBoard > 0) && (LOCALPLAYER.hostages.nOnBoard == LOCALPLAYER.hostages.nLevel)) {
		nAllHostagePoints = LOCALPLAYER.hostages.nOnBoard * 1000 * (gameStates.app.nDifficultyLevel+1);
		sprintf (szAllHostages, "%s%i", TXT_FULL_RESCUE_BONUS, nAllHostagePoints);
		}
	if (!IsMultiGame && LOCALPLAYER.lives && (gameData.missions.nCurrentLevel == gameData.missions.nLastLevel)) {		//CPlayerData has finished the game!
		nEndGamePoints = LOCALPLAYER.lives * 10000;
		sprintf (szEndGame, "%s%i  ", TXT_SHIP_BONUS, nEndGamePoints);
		bIsLastLevel = 1;
		}
	AddBonusPointsToScore (nSkillPoints + nEnergyPoints + nShieldPoints + nHostagePoints + nAllHostagePoints + nEndGamePoints);
	}
c = 0;
sprintf (szMenu [c++], "%s%i  ", TXT_SHIELD_BONUS, nShieldPoints);		// Return at start to lower menu...
sprintf (szMenu [c++], "%s%i  ", TXT_ENERGY_BONUS, nEnergyPoints);
sprintf (szMenu [c++], "%s%i  ", TXT_HOSTAGE_BONUS, nHostagePoints);
sprintf (szMenu [c++], "%s%i  ", TXT_SKILL_BONUS, nSkillPoints);
if (*szAllHostages)
	sprintf (szMenu [c++], "%s  ", szAllHostages);
if (bIsLastLevel)
	sprintf (szMenu [c++], "%s  ", szEndGame);
sprintf (szMenu [c++], "%s%i  ", TXT_TOTAL_BONUS,
			nShieldPoints + nEnergyPoints + nHostagePoints + nSkillPoints + nAllHostagePoints + nEndGamePoints);
sprintf (szMenu [c++], "%s%i  ", TXT_TOTAL_SCORE, LOCALPLAYER.score);
memset (m, 0, sizeof (m));
for (i = 0; i < c; i++) {
	m [i].nType = NM_TYPE_TEXT;
	m [i].text = szMenu [i];
	}
sprintf (szTitle,
			"%s%s %d %s\n%s %s",
			gameOpts->menus.nStyle ? "" : bIsLastLevel ? "\n\n\n":"\n",
			 (gameData.missions.nCurrentLevel < 0) ? TXT_SECRET_LEVEL : TXT_LEVEL,
			 (gameData.missions.nCurrentLevel < 0) ? -gameData.missions.nCurrentLevel : gameData.missions.nCurrentLevel,
			TXT_COMPLETE,
			gameData.missions.szCurrentLevel,
			TXT_DESTROYED);
Assert (c <= N_GLITZITEMS);
paletteManager.DisableEffect ();
if (network && (gameData.app.nGameMode & GM_NETWORK))
	ExecMenu2 (NULL, szTitle, c, m, NetworkEndLevelPoll2, 0, reinterpret_cast<char*> (STARS_BACKGROUND));
else
// NOTE LINK TO ABOVE!!!
gameStates.app.bGameRunning = 0;
ExecMenu2 (NULL, szTitle, c, m, NULL, 0, reinterpret_cast<char*> (STARS_BACKGROUND));
}

//	-----------------------------------------------------------------------------------------------------

//give the CPlayerData the opportunity to save his game
void DoEndlevelMenu ()
{
//No between level saves......!!!	StateSaveAll (1);
}

//	-----------------------------------------------------------------------------------------------------
//called when the CPlayerData is starting a level (new game or new ship)
void StartSecretLevel (void)
{
Assert (!gameStates.app.bPlayerIsDead);
InitPlayerPosition (0);
VerifyConsoleObject ();
gameData.objs.consoleP->info.controlType = CT_FLYING;
gameData.objs.consoleP->info.movementType = MT_PHYSICS;
// -- WHY? -- DisableMatCens ();
ClearTransientObjects (0);		//0 means leave proximity bombs
// gameData.objs.consoleP->CreateAppearanceEffect ();
gameStates.render.bDoAppearanceEffect = 1;
AIResetAllPaths ();
ResetTime ();
ResetRearView ();
gameData.fusion.xAutoFireTime = 0;
gameData.fusion.xCharge = 0;
gameStates.app.cheats.bRobotsFiring = 1;
gameStates.sound.bD1Sound = gameStates.app.bD1Mission && gameStates.app.bHaveD1Data && gameOpts->sound.bUseD1Sounds && !gameOpts->sound.bHires;
if (gameStates.app.bD1Mission) {
	if (LOCALPLAYER.energy < INITIAL_ENERGY)
		LOCALPLAYER.energy = INITIAL_ENERGY;
	if (LOCALPLAYER.shields < INITIAL_SHIELDS)
		LOCALPLAYER.shields = INITIAL_SHIELDS;
	}
}

//------------------------------------------------------------------------------

//	Returns true if secret level has been destroyed.
int PSecretLevelDestroyed (void)
{
if (gameStates.app.bFirstSecretVisit)
	return 0;		//	Never been there, can't have been destroyed.
if (CFile::Exist (SECRETC_FILENAME, gameFolders.szSaveDir, 0))
	return 0;
return 1;
}

//	-----------------------------------------------------------------------------------------------------

void DoSecretMessage (const char *msg)
{
	int fMode = gameStates.app.nFunctionMode;

StopTime ();
SetFunctionMode (FMODE_MENU);
ExecMessageBox (NULL, reinterpret_cast<char*> (STARS_BACKGROUND), 1, TXT_OK, msg);
SetFunctionMode (fMode);
StartTime (0);
}

//	-----------------------------------------------------------------------------------------------------

void InitSecretLevel (int nLevel)
{
Assert (gameData.missions.nCurrentLevel == nLevel);	//make sure level set right
Assert (gameStates.app.nFunctionMode == FMODE_GAME);
GameStartInitNetworkPlayers (); // Initialize the gameData.multiplayer.players array for this level
HUDClearMessages ();
automap.ClearVisited ();
InitPlayerStatsLevel (1);
gameData.objs.viewerP = OBJECTS + LOCALPLAYER.nObject;
GameStartRemoveUnusedPlayers ();
gameStates.app.bGameSuspended = 0;
gameData.reactor.bDestroyed = 0;
InitCockpit ();
paletteManager.ResetEffect ();
}

//	-----------------------------------------------------------------------------------------------------
// called when the CPlayerData is starting a new level for Normal game mode and restore state
//	Need to deal with whether this is the first time coming to this level or not.  If not the
//	first time, instead of initializing various things, need to do a game restore for all the
//	robots, powerups, walls, doors, etc.
int StartNewLevelSecret (int nLevel, int bPageInTextures)
{
	tMenuItem	m [1];
  //int i;

gameStates.app.xThisLevelTime=0;

m [0].nType = NM_TYPE_TEXT;
m [0].text = reinterpret_cast<char*> (" ");

gameStates.render.cockpit.nLastDrawn [0] = -1;
gameStates.render.cockpit.nLastDrawn [1] = -1;

if (gameData.demo.nState == ND_STATE_PAUSED)
	gameData.demo.nState = ND_STATE_RECORDING;

if (gameData.demo.nState == ND_STATE_RECORDING) {
	NDSetNewLevel (nLevel);
	NDRecordStartFrame (gameData.app.nFrameCount, gameData.time.xFrame);
	}
else if (gameData.demo.nState != ND_STATE_PLAYBACK) {
	paletteManager.DisableEffect ();
	SetScreenMode (SCREEN_MENU);		//go into menu mode
	if (gameStates.app.bFirstSecretVisit)
		DoSecretMessage (gameStates.app.bD1Mission ? TXT_ALTERNATE_EXIT : TXT_SECRET_EXIT);
	else if (CFile::Exist (SECRETC_FILENAME,gameFolders.szSaveDir,0))
		DoSecretMessage (gameStates.app.bD1Mission ? TXT_ALTERNATE_EXIT : TXT_SECRET_EXIT);
	else {
		char	text_str [128];

		sprintf (text_str, TXT_ADVANCE_LVL, gameData.missions.nCurrentLevel+1);
		DoSecretMessage (text_str);
		}
	}

if (gameStates.app.bFirstSecretVisit || (gameData.demo.nState == ND_STATE_PLAYBACK)) {
	if (!LoadLevel (nLevel, bPageInTextures, 0))
		return 0;
	InitSecretLevel (nLevel);
	if (!gameStates.app.bAutoRunMission && gameStates.app.bD1Mission)
		ShowLevelIntro (nLevel);
	PlayLevelSong (gameData.missions.nCurrentLevel, 0);
	InitRobotsForLevel ();
	InitAIObjects ();
	InitShakerDetonates ();
	MorphInit ();
	InitAllMatCens ();
	ResetSpecialEffects ();
	StartSecretLevel ();
	LOCALPLAYER.flags &= ~PLAYER_FLAGS_ALL_KEYS;
	}
else {
	if (CFile::Exist (SECRETC_FILENAME, gameFolders.szSaveDir, 0)) {
		int	pw_save, sw_save, nCurrentLevel;

		pw_save = gameData.weapons.nPrimary;
		sw_save = gameData.weapons.nSecondary;
		nCurrentLevel = gameData.missions.nCurrentLevel;
		saveGameHandler.Load (1, 1, 0, SECRETC_FILENAME);
		gameData.missions.nEnteredFromLevel = nCurrentLevel;
		gameData.weapons.nPrimary = pw_save;
		gameData.weapons.nSecondary = sw_save;
		ResetSpecialEffects ();
		StartSecretLevel ();
		}
	else {
		char	text_str [128];

		sprintf (text_str, TXT_ADVANCE_LVL, gameData.missions.nCurrentLevel+1);
		DoSecretMessage (text_str);
		if (!LoadLevel (nLevel, bPageInTextures, 0))
			return 0;
		InitSecretLevel (nLevel);
		return 1;

		// -- //	If file doesn't exist, it's because reactor was destroyed.
		// -- // -- StartNewLevel (gameData.missions.secretLevelTable [-gameData.missions.nCurrentLevel-1]+1, 0);
		// -- StartNewLevel (gameData.missions.secretLevelTable [-gameData.missions.nCurrentLevel-1]+1, 0);
		// -- return;
		}
	}

if (gameStates.app.bFirstSecretVisit)
	CopyDefaultsToRobotsAll ();
TurnCheatsOff ();
InitReactorForLevel (0);
//	Say CPlayerData can use FLASH cheat to mark path to exit.
nLastLevelPathCreated = -1;
gameStates.app.bFirstSecretVisit = 0;
return 1;
}

//------------------------------------------------------------------------------

//	Called from switch.c when CPlayerData is on a secret level and hits exit to return to base level.
void ExitSecretLevel (void)
{
if (gameData.demo.nState == ND_STATE_PLAYBACK)
	return;
if (!(gameStates.app.bD1Mission || gameData.reactor.bDestroyed))
	saveGameHandler.Save (0, 2, 0, SECRETC_FILENAME);
if (!gameStates.app.bD1Mission && CFile::Exist (SECRETB_FILENAME, gameFolders.szSaveDir, 0)) {
	int pw_save = gameData.weapons.nPrimary;
	int sw_save = gameData.weapons.nSecondary;

	ReturningToLevelMessage ();
	saveGameHandler.Load (1, 1, 0, SECRETB_FILENAME);
	gameStates.sound.bD1Sound = gameStates.app.bD1Mission && gameStates.app.bHaveD1Data && gameOpts->sound.bUseD1Sounds && !gameOpts->sound.bHires;
	SetDataVersion (-1);
	gameData.weapons.nPrimary = pw_save;
	gameData.weapons.nSecondary = sw_save;
	}
else {
	// File doesn't exist, so can't return to base level.  Advance to next one.
	if (gameData.missions.nEnteredFromLevel == gameData.missions.nLastLevel)
		DoEndGame ();
	else {
		if (!gameStates.app.bD1Mission)
			AdvancingToLevelMessage ();
		DoEndLevelScoreGlitz (0);
		StartNewLevel (gameData.missions.nEnteredFromLevel + 1, 0);
		}
	}
}

//------------------------------------------------------------------------------
//	Set invulnerableTime and cloakTime in CPlayerData struct to preserve amount of time left to
//	be invulnerable or cloaked.
void DoCloakInvulSecretStuff (fix xOldGameTime)
{
if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
		fix	time_used;

	time_used = xOldGameTime - LOCALPLAYER.invulnerableTime;
	LOCALPLAYER.invulnerableTime = gameData.time.xGame - time_used;
	}

if (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) {
		fix	time_used;

	time_used = xOldGameTime - LOCALPLAYER.cloakTime;
	LOCALPLAYER.cloakTime = gameData.time.xGame - time_used;
	}
}

//------------------------------------------------------------------------------
//	Called from switch.c when CPlayerData passes through secret exit.  That means he was on a non-secret level and he
//	is passing to the secret level.
//	Do a savegame.
void EnterSecretLevel (void)
{
	fix	xOldGameTime;
	int	i;

Assert (!IsMultiGame);
gameData.missions.nEnteredFromLevel = gameData.missions.nCurrentLevel;
if (gameData.reactor.bDestroyed)
	DoEndLevelScoreGlitz (0);
if (gameData.demo.nState != ND_STATE_PLAYBACK)
	saveGameHandler.Save (0, 1, 0, NULL);	//	Not between levels (ie, save all), IS a secret level, NO filename override
//	Find secret level number to go to, stuff in gameData.missions.nNextLevel.
for (i = 0; i < -gameData.missions.nLastSecretLevel; i++)
	if (gameData.missions.secretLevelTable [i] == gameData.missions.nCurrentLevel) {
		gameData.missions.nNextLevel = -i - 1;
		break;
		}
	else if (gameData.missions.secretLevelTable [i] > gameData.missions.nCurrentLevel) {	//	Allows multiple exits in same group.
		gameData.missions.nNextLevel = -i;
		break;
		}
if (i >= -gameData.missions.nLastSecretLevel)		//didn't find level, so must be last
	gameData.missions.nNextLevel = gameData.missions.nLastSecretLevel;
xOldGameTime = gameData.time.xGame;
StartNewLevelSecret (gameData.missions.nNextLevel, 1);
// do_cloak_invul_stuff ();
}

//------------------------------------------------------------------------------
//called when the CPlayerData has finished a level
void PlayerFinishedLevel (int bSecret)
{
	Assert (!bSecret);

	//credit the CPlayerData for hostages
LOCALPLAYER.hostages.nRescued += LOCALPLAYER.hostages.nOnBoard;
if (gameData.app.nGameMode & GM_NETWORK)
	LOCALPLAYER.connected = 2; // Finished but did not die
gameStates.render.cockpit.nLastDrawn [0] = -1;
gameStates.render.cockpit.nLastDrawn [1] = -1;
if (gameData.missions.nCurrentLevel < 0)
	ExitSecretLevel ();
else
	AdvanceLevel (0, 0);				//now go on to the next one (if one)
}

//------------------------------------------------------------------------------

void PlayLevelMovie (const char *pszExt, int nLevel)
{
	char szFilename [FILENAME_LEN];

PlayMovie (MakeLevelFilename (nLevel, szFilename, pszExt), MOVIE_OPTIONAL, 0, gameOpts->movies.bResize);
}

//------------------------------------------------------------------------------

void PlayLevelIntroMovie (int nLevel)
{
PlayLevelMovie (".mvi", nLevel);
}

//------------------------------------------------------------------------------

void PlayLevelExtroMovie (int nLevel)
{
PlayLevelMovie (".mvx", nLevel);
}

//------------------------------------------------------------------------------

#define MOVIE_REQUIRED 1

//called when the CPlayerData has finished the last level
void DoEndGame (void)
{
SetFunctionMode (FMODE_MENU);
if ((gameData.demo.nState == ND_STATE_RECORDING) || (gameData.demo.nState == ND_STATE_PAUSED))
	NDStopRecording ();
SetScreenMode (SCREEN_MENU);
CCanvas::SetCurrent (NULL);
KeyFlush ();
if (!IsMultiGame) {
	if (gameData.missions.nCurrentMission == (gameStates.app.bD1Mission ? gameData.missions.nD1BuiltinMission : gameData.missions.nBuiltinMission)) {
		int bPlayed = MOVIE_NOT_PLAYED;	//default is not bPlayed

		if (!gameStates.app.bD1Mission) {
			InitSubTitles (ENDMOVIE ".tex");
			bPlayed = PlayMovie (ENDMOVIE, MOVIE_REQUIRED, 0, gameOpts->movies.bResize);
			CloseSubTitles ();
			}
		else if (movieManager.bHaveExtras) {
			//InitSubTitles (ENDMOVIE ".tex");	//ingore errors
			bPlayed = PlayMovie (D1_ENDMOVIE, MOVIE_REQUIRED, 0, gameOpts->movies.bResize);
			CloseSubTitles ();
			}
		if (!bPlayed) {
			if (IS_D2_OEM) {
				SongsPlaySong (SONG_TITLE, 0);
				DoBriefingScreens (reinterpret_cast<char*> ("end2oem.tex"), 1);
				}
			else {
				SongsPlaySong (SONG_ENDGAME, 0);
				DoBriefingScreens (gameStates.app.bD1Mission ? reinterpret_cast<char*> ("endreg.tex") : reinterpret_cast<char*> ("ending2.tex"), 
										 gameStates.app.bD1Mission ? 0x7e : 1);
				}
			}
		}
	else {    //not multi
		char szBriefing [FILENAME_LEN];
		PlayLevelExtroMovie (gameData.missions.nCurrentLevel);
		sprintf (szBriefing, "%s.tex", gameStates.app.szCurrentMissionFile);
		DoBriefingScreens (szBriefing, gameData.missions.nLastLevel + 1);   //level past last is endgame breifing

		//try doing special credits
		sprintf (szBriefing,"%s.ctb",gameStates.app.szCurrentMissionFile);
		creditsManager.Show (szBriefing);
		}
	}
KeyFlush ();
if (IsMultiGame)
	MultiEndLevelScore ();
else
	// NOTE LINK TO ABOVE
	DoEndLevelScoreGlitz (0);

if ((gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission) &&
	 !(gameData.app.nGameMode & (GM_MULTI | GM_MULTI_COOP))) {
	CCanvas::SetCurrent (NULL);
	CCanvas::Current ()->Clear (BLACK_RGBA);
	paletteManager.ClearEffect ();
	//paletteManager.Load (D2_DEFAULT_PALETTE, NULL, 0, 1, 0);
	MaybeAddPlayerScore (0);
	}
SetFunctionMode (FMODE_MENU);
if ((gameData.app.nGameMode & GM_SERIAL) || (gameData.app.nGameMode & GM_MODEM))
	gameData.app.nGameMode |= GM_GAME_OVER;		//preserve modem setting so go back into modem menu
else
	gameData.app.nGameMode = GM_GAME_OVER;
longjmp (gameExitPoint, 0);		// Exit out of game loop
}

//------------------------------------------------------------------------------
//from which level each do you get to each secret level
//called to go to the next level (if there is one)
//if bSecret is true, advance to secret level, else next Normal one
//	Return true if game over.
void AdvanceLevel (int bSecret, int bFromSecret)
{
	int result;

gameStates.app.bBetweenLevels = 1;
Assert (!bSecret);
if ((!bFromSecret/* && gameStates.app.bD1Mission*/) &&
	 ((gameData.missions.nCurrentLevel != gameData.missions.nLastLevel) ||
	  extraGameInfo [IsMultiGame].bRotateLevels)) {
	if (IsMultiGame)
		MultiEndLevelScore ();
	else {
		PlayLevelExtroMovie (gameData.missions.nCurrentLevel);
		DoEndLevelScoreGlitz (0);		//give bonuses
		}
	}
gameData.reactor.bDestroyed = 0;
#ifdef EDITOR
if (gameData.missions.nCurrentLevel == 0)
	return;		//not a real level
#endif
if (IsMultiGame) {
	if ((result = MultiEndLevel (&bSecret))) { // Wait for other players to reach this point
		gameStates.app.bBetweenLevels = 0;
		if (gameData.missions.nCurrentLevel != gameData.missions.nLastLevel)		//CPlayerData has finished the game!
			return;
		longjmp (gameExitPoint, 0);		// Exit out of game loop
		}
	}
if ((gameData.missions.nCurrentLevel == gameData.missions.nLastLevel) &&
	!extraGameInfo [IsMultiGame].bRotateLevels) //CPlayerData has finished the game!
	DoEndGame ();
else {
	gameData.missions.nNextLevel = gameData.missions.nCurrentLevel + 1;		//assume go to next Normal level
	if (gameData.missions.nNextLevel > gameData.missions.nLastLevel) {
		if (extraGameInfo [IsMultiGame].bRotateLevels)
			gameData.missions.nNextLevel = 1;
		else
			gameData.missions.nNextLevel = gameData.missions.nLastLevel;
		}
	if (!IsMultiGame)
		DoEndlevelMenu (); // Let user save their game
	StartNewLevel (gameData.missions.nNextLevel, 0);
	}
gameStates.app.bBetweenLevels = 0;
}

//------------------------------------------------------------------------------

void DiedInMineMessage (void)
{
	// Tell the CPlayerData he died in the mine, explain why
	int old_fmode;

if (gameData.app.nGameMode & GM_MULTI)
	return;
paletteManager.DisableEffect ();
SetScreenMode (SCREEN_MENU);		//go into menu mode
CCanvas::SetCurrent (NULL);
old_fmode = gameStates.app.nFunctionMode;
SetFunctionMode (FMODE_MENU);
ExecMessageBox (NULL, reinterpret_cast<char*> (STARS_BACKGROUND), 1, TXT_OK, TXT_DIED_IN_MINE);
SetFunctionMode (old_fmode);
}

//------------------------------------------------------------------------------
//	Called when CPlayerData dies on secret level.
void ReturningToLevelMessage (void)
{
	char	msg [128];

	int old_fmode;

if (gameData.app.nGameMode & GM_MULTI)
	return;
StopTime ();
paletteManager.DisableEffect ();
SetScreenMode (SCREEN_MENU);		//go into menu mode
CCanvas::SetCurrent (NULL);
old_fmode = gameStates.app.nFunctionMode;
SetFunctionMode (FMODE_MENU);
if (gameData.missions.nEnteredFromLevel < 0)
	sprintf (msg, TXT_SECRET_LEVEL_RETURN);
else
	sprintf (msg, TXT_RETURN_LVL, gameData.missions.nEnteredFromLevel);
ExecMessageBox (NULL, reinterpret_cast<char*> (STARS_BACKGROUND), 1, TXT_OK, msg);
SetFunctionMode (old_fmode);
StartTime (0);
}

//------------------------------------------------------------------------------
//	Called when CPlayerData dies on secret level.
void AdvancingToLevelMessage (void)
{
	char	msg [128];

	int old_fmode;

	//	Only supposed to come here from a secret level.
Assert (gameData.missions.nCurrentLevel < 0);
if (IsMultiGame)
	return;
paletteManager.DisableEffect ();
SetScreenMode (SCREEN_MENU);		//go into menu mode
CCanvas::SetCurrent (NULL);
old_fmode = gameStates.app.nFunctionMode;
SetFunctionMode (FMODE_MENU);
sprintf (msg, "Base level destroyed.\nAdvancing to level %i", gameData.missions.nEnteredFromLevel + 1);
ExecMessageBox (NULL, reinterpret_cast<char*> (STARS_BACKGROUND), 1, TXT_OK, msg);
SetFunctionMode (old_fmode);
}

//	-----------------------------------------------------------------------------------
//	Set the CPlayerData's position from the globals gameData.segs.secret.nReturnSegment and gameData.segs.secret.returnOrient.
void SetPosFromReturnSegment (int bRelink)
{
	int	nPlayerObj = LOCALPLAYER.nObject;

OBJECTS [nPlayerObj].info.position.vPos = SEGMENTS [gameData.segs.secret.nReturnSegment].Center ();
if (bRelink)
	OBJECTS [nPlayerObj].RelinkToSeg (gameData.segs.secret.nReturnSegment);
ResetPlayerObject ();
OBJECTS [nPlayerObj].info.position.mOrient = gameData.segs.secret.returnOrient;
}

//------------------------------------------------------------------------------

void DoPlayerDead (void)
{
	int bSecret = (gameData.missions.nCurrentLevel < 0);

gameStates.app.bGameRunning = 0;
paletteManager.ResetEffect ();
paletteManager.LoadEffect  ();
DigiStopDigiSounds ();		//kill any continuing sounds (eg. forcefield hum)
DeadPlayerEnd ();		//terminate death sequence (if playing)
if (IsCoopGame && gameStates.app.bHaveExtraGameInfo [1])
	LOCALPLAYER.score =
	(LOCALPLAYER.score * (100 - nCoopPenalties [(int) extraGameInfo [1].nCoopPenalty])) / 100;
if (gameStates.multi.bPlayerIsTyping [gameData.multiplayer.nLocalPlayer] && (gameData.app.nGameMode & GM_MULTI))
	MultiSendMsgQuit ();
gameStates.entropy.bConquering = 0;
#ifdef EDITOR
if (gameData.app.nGameMode == GM_EDITOR) {			//test mine, not real level
	CObject * playerobj = OBJECTS + LOCALPLAYER.nObject;
	//ExecMessageBox ("You're Dead!", 1, "Continue", "Not a real game, though.");
	LoadLevelSub ("gamesave.lvl");
	InitPlayerStatsNewShip ();
	playerobjP->info.nFlags &= ~OF_SHOULD_BE_DEAD;
	StartLevel (0);
	return;
}
#endif

if (gameData.app.nGameMode & GM_MULTI)
	MultiDoDeath (LOCALPLAYER.nObject);
else {				//Note link to above else!
	if (!--LOCALPLAYER.lives) {
		DoGameOver ();
		return;
		}
	}
if (gameData.reactor.bDestroyed) {
	//clear out stuff so no bonus
	LOCALPLAYER.hostages.nOnBoard = 0;
	LOCALPLAYER.energy = 0;
	LOCALPLAYER.shields = 0;
	LOCALPLAYER.connected = 3;
	DiedInMineMessage (); // Give them some indication of what happened
	}
if (bSecret && !gameStates.app.bD1Mission) {
	ExitSecretLevel ();
	SetPosFromReturnSegment (1);
	LOCALPLAYER.lives--;	//	re-lose the life, LOCALPLAYER.lives got written over in restore.
	InitPlayerStatsNewShip ();
	gameStates.render.cockpit.nLastDrawn [0] =
	gameStates.render.cockpit.nLastDrawn [1] = -1;
	}
else {
	if (gameData.reactor.bDestroyed) {
		//AdvancingToLevelMessage ();
		AdvanceLevel (0, bSecret);
		//StartNewLevel (gameData.missions.nCurrentLevel + 1, 0);
		InitPlayerStatsNewShip ();
		}
	else if (!gameStates.entropy.bExitSequence) {
		InitPlayerStatsNewShip ();
		StartLevel (1);
		}
	}
SetSoundSources ();
DigiSyncSounds ();
}

//------------------------------------------------------------------------------

//called when the CPlayerData is starting a new level for Normal game mode and restore state
//	bSecret set if came from a secret level
int StartNewLevelSub (int nLevel, int bPageInTextures, int bSecret, int bRestore)
{
	int funcRes;

	gameStates.multi.bTryAutoDL = 1;

reloadLevel:

if (!IsMultiGame) {
	gameStates.render.cockpit.nLastDrawn [0] =
	gameStates.render.cockpit.nLastDrawn [1] = -1;
	}
gameStates.render.cockpit.bBigWindowSwitch = 0;
if (gameData.demo.nState == ND_STATE_PAUSED)
	gameData.demo.nState = ND_STATE_RECORDING;
if (gameData.demo.nState == ND_STATE_RECORDING) {
	NDSetNewLevel (nLevel);
	NDRecordStartFrame (gameData.app.nFrameCount, gameData.time.xFrame);
	}
if (IsMultiGame)
	SetFunctionMode (FMODE_MENU); // Cheap fix to prevent problems with errror dialogs in loadlevel.
SetWarnFunc (ShowInGameWarning);
funcRes = LoadLevel (nLevel, bPageInTextures, bRestore);
ClearWarnFunc (ShowInGameWarning);
if (!funcRes)
	return 0;
Assert (gameStates.app.bAutoRunMission || (gameData.missions.nCurrentLevel == nLevel));	//make sure level set right
GameStartInitNetworkPlayers (); // Initialize the gameData.multiplayer.players array for
#if DBG										  // this level
InitHoardData ();
SetMonsterballForces ();
#endif
//	gameData.objs.viewerP = OBJECTS + LOCALPLAYER.nObject;
if (gameData.multiplayer.nPlayers > gameData.multiplayer.nPlayerPositions) {
	ExecMessageBox (NULL, NULL, 1, TXT_OK, "Too many players for this level.");
	return 0;
	}
Assert (gameData.multiplayer.nPlayers <= gameData.multiplayer.nPlayerPositions);
	//If this assert fails, there's not enough start positions

if (gameData.app.nGameMode & GM_NETWORK) {
	switch (NetworkLevelSync ()) { // After calling this, gameData.multiplayer.nLocalPlayer is set
		case -1:
			return 0;
		case 0:
			if (gameData.multiplayer.nLocalPlayer < 0)
				return 0;
			fontManager.Remap ();
			break;
		case 1:
			networkData.nStatus = 2;
			goto reloadLevel;
		}
	}
Assert (gameStates.app.nFunctionMode == FMODE_GAME);
HUDClearMessages ();
automap.ClearVisited ();
if (networkData.bNewGame == 1) {
	networkData.bNewGame = 0;
	InitPlayerStatsNewShip ();
	}
InitPlayerStatsLevel (bSecret);
if (IsCoopGame && networkData.nJoinState) {
	for (int i = 0; i < gameData.multiplayer.nPlayers; i++)
		gameData.multiplayer.players [i].flags |= netGame.playerFlags [i];
	}
if (IsMultiGame)
	MultiPrepLevel (); // Removes robots from level if necessary
else
	FindMonsterball (); //will simply remove all Monsterballs
GameStartRemoveUnusedPlayers ();
gameStates.app.bGameSuspended = 0;
gameData.reactor.bDestroyed = 0;
gameStates.render.glFOV = DEFAULT_FOV;
SetScreenMode (SCREEN_GAME);
InitCockpit ();
InitRobotsForLevel ();
InitShakerDetonates ();
MorphInit ();
InitAllMatCens ();
paletteManager.ResetEffect ();
InitThiefForLevel ();
InitStuckObjects ();
GameFlushInputs ();		// clear out the keyboard
if (!IsMultiGame)
	FilterObjectsFromLevel ();
TurnCheatsOff ();
if (!(IsMultiGame || gameStates.app.cheats.bEnabled)) {
	SetHighestLevel (gameData.missions.nCurrentLevel);
	}
else
	ReadPlayerFile (1);		//get window sizes
ResetSpecialEffects ();
if (networkData.nJoinState == 1){
	networkData.nJoinState = 0;
	StartLevel (1);
	}
else {
	StartLevel (0);		// Note link to above if!
	}
CopyDefaultsToRobotsAll ();
if (!bRestore)
	InitReactorForLevel (0);
InitAIObjects ();
BuildObjectModels ();
#if 0
LOCALPLAYER.nInvuls =
LOCALPLAYER.nCloaks = 0;
#endif
//	Say CPlayerData can use FLASH cheat to mark path to exit.
nLastLevelPathCreated = -1;
return 1;
}

//------------------------------------------------------------------------------

void BashToShield (int i, const char *s)
{
	CObject *objP = OBJECTS + i;
	int id = objP->info.nId;

gameData.multiplayer.powerupsInMine [id] =
gameData.multiplayer.maxPowerupsAllowed [id] = 0;
objP->SetType (OBJ_POWERUP);
objP->info.nId = POW_SHIELD_BOOST;
objP->info.renderType = RT_POWERUP;
objP->info.controlType = CT_POWERUP;
objP->info.xSize = gameData.objs.pwrUp.info [POW_SHIELD_BOOST].size;
objP->rType.vClipInfo.nClipIndex = gameData.objs.pwrUp.info [POW_SHIELD_BOOST].nClipIndex;
objP->rType.vClipInfo.xFrameTime = gameData.eff.vClips [0][objP->rType.vClipInfo.nClipIndex].xFrameTime;
}

//------------------------------------------------------------------------------

void BashToEnergy (int i, const char *s)
{
	CObject *objP = OBJECTS + i;
	int id = objP->info.nId;

gameData.multiplayer.powerupsInMine [id] =
gameData.multiplayer.maxPowerupsAllowed [id] = 0;
objP->SetType (OBJ_POWERUP);
objP->info.nId = POW_ENERGY;
objP->info.renderType = RT_POWERUP;
objP->info.controlType = CT_POWERUP;
objP->info.xSize = gameData.objs.pwrUp.info [POW_ENERGY].size;
objP->rType.vClipInfo.nClipIndex = gameData.objs.pwrUp.info [POW_ENERGY].nClipIndex;
objP->rType.vClipInfo.xFrameTime = gameData.eff.vClips [0][objP->rType.vClipInfo.nClipIndex].xFrameTime;
}

//------------------------------------------------------------------------------

void FilterObjectsFromLevel (void)
{
  //int 		i;
	CObject	*objP;

FORALL_POWERUP_OBJS (objP, i) {
	if ((objP->info.nId == POW_REDFLAG) || (objP->info.nId == POW_BLUEFLAG))
		BashToShield (objP->Index (), "Flag!!!!");
  }
}

//------------------------------------------------------------------------------

struct tIntroMovieInfo {
	int	nLevel;
	char	szMovieName [FILENAME_LEN];
} szIntroMovies [] = {{ 1,"pla"},
							 { 5,"plb"},
							 { 9,"plc"},
							 {13,"pld"},
							 {17,"ple"},
							 {21,"plf"},
							 {24,"plg"}};

#define NUM_INTRO_MOVIES (sizeof (szIntroMovies) / sizeof (*szIntroMovies))

void ShowLevelIntro (int nLevel)
{
//if shareware, show a briefing?
if (!IsMultiGame) {
	uint i, bPlayed = 0;

	PlayLevelIntroMovie (nLevel);
	if (!gameStates.app.bD1Mission && (gameData.missions.nCurrentMission == gameData.missions.nBuiltinMission)) {
		if (IS_SHAREWARE) {
			if (nLevel == 1)
				DoBriefingScreens (reinterpret_cast<char*> ("brief2.tex"), 1);
			}
		else if (IS_D2_OEM) {
			if ((nLevel == 1) && !gameStates.movies.bIntroPlayed)
				DoBriefingScreens (reinterpret_cast<char*> ("brief2o.tex"), 1);
			}
		else { // full version
			if (movieManager.bHaveExtras && (nLevel == 1)) {
				PlayIntroMovie ();
				}
			for (i = 0; i < NUM_INTRO_MOVIES; i++) {
				if (szIntroMovies [i].nLevel == nLevel) {
					gameStates.video.nScreenMode = -1;
					PlayMovie (szIntroMovies [i].szMovieName, MOVIE_REQUIRED, 0, gameOpts->movies.bResize);
					bPlayed=1;
					break;
					}
				}
			if (gameStates.movies.nRobots) {
				int hires_save = gameStates.menus.bHiresAvailable;
				if (gameStates.movies.nRobots == 1) {		//lowres only
					gameStates.menus.bHiresAvailable = 0;		//pretend we can't do highres
					if (hires_save != gameStates.menus.bHiresAvailable)
						gameStates.video.nScreenMode = -1;		//force reset
					}
				DoBriefingScreens (reinterpret_cast<char*> ("robot.tex"), nLevel);
				gameStates.menus.bHiresAvailable = hires_save;
				}
			}
		}
	else {	//not the built-in mission.  check for add-on briefing
		if ((gameData.missions.list [gameData.missions.nCurrentMission].nDescentVersion == 1) &&
			 (gameData.missions.nCurrentMission == gameData.missions.nD1BuiltinMission)) {
			if (movieManager.bHaveExtras && (nLevel == 1)) {
				if (PlayMovie ("briefa.mve", MOVIE_REQUIRED, 0, gameOpts->movies.bResize) != MOVIE_ABORTED)
					 PlayMovie ("briefb.mve", MOVIE_REQUIRED, 0, gameOpts->movies.bResize);
				}
			DoBriefingScreens (gameData.missions.szBriefingFilename, nLevel);
			}
		else {
			char szBriefing [FILENAME_LEN];
			sprintf (szBriefing, "%s.tex", gameStates.app.szCurrentMissionFile);
			DoBriefingScreens (szBriefing, nLevel);
			}
		}
	}
}

//	---------------------------------------------------------------------------
//	If starting a level which appears in the gameData.missions.secretLevelTable, then set gameStates.app.bFirstSecretVisit.
//	Reason: On this level, if CPlayerData goes to a secret level, he will be going to a different
//	secret level than he's ever been to before.
//	Sets the global gameStates.app.bFirstSecretVisit if necessary.  Otherwise leaves it unchanged.
void MaybeSetFirstSecretVisit (int nLevel)
{
	int	i;

for (i = 0; i < gameData.missions.nSecretLevels; i++) {
	if (gameData.missions.secretLevelTable [i] == nLevel) {
		gameStates.app.bFirstSecretVisit = 1;
		}
	}
}

//------------------------------------------------------------------------------
//called when the CPlayerData is starting a new level for Normal game model
//	bSecret if came from a secret level
int StartNewLevel (int nLevel, int bSecret)
{
	gameStates.app.xThisLevelTime = 0;

if ((nLevel > 0) && !bSecret)
	MaybeSetFirstSecretVisit (nLevel);
if (!gameStates.app.bAutoRunMission)
	ShowLevelIntro (nLevel);
return StartNewLevelSub (nLevel, 1, bSecret, 0);
}

//------------------------------------------------------------------------------

typedef struct tSpawnMap {
	int	i;
	fix	xDist;
	} tSpawnMap;

void SortSpawnMap (tSpawnMap *spawnMap, int left, int right)
{
	int	l = left,
			r = right;
	fix	m = spawnMap [(l + r) / 2].xDist;

do {
	while (spawnMap [l].xDist > m)
		l++;
	while (spawnMap [r].xDist < m)
		r--;
	if (l <= r) {
		if (l < r) {
			tSpawnMap h = spawnMap [l];
			spawnMap [l] = spawnMap [r];
			spawnMap [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortSpawnMap (spawnMap, l, right);
if (left < r)
	SortSpawnMap (spawnMap, left, r);
}

//------------------------------------------------------------------------------

int GetRandomPlayerPosition (void)
{
	CObject		*objP;
	tSpawnMap	spawnMap [MAX_NUM_NET_PLAYERS];
	int			nSpawnPos = 0;
	int			nSpawnSegs = 0;
	int			i, j, bRandom;
	fix			xDist;

// find the smallest distance between each spawn point and any player in the mine
for (i = 0; i < gameData.multiplayer.nPlayerPositions; i++) {
	spawnMap [i].i = i;
	spawnMap [i].xDist = 0x7fffffff;
	for (j = 0; j < gameData.multiplayer.nPlayers; j++) {
		if (j != gameData.multiplayer.nLocalPlayer) {
			objP = OBJECTS + gameData.multiplayer.players [j].nObject;
			if ((objP->info.nType == OBJ_PLAYER))	{
				xDist = FindConnectedDistance (objP->info.position.vPos,
														 objP->info.nSegment,
														 gameData.multiplayer.playerInit [i].position.vPos,
														 gameData.multiplayer.playerInit [i].nSegment,
														 10, WID_FLY_FLAG, 0);	//	Used to be 5, search up to 10 segments
				if (xDist < 0)
					continue;
				if (spawnMap [i].xDist > xDist)
					spawnMap [i].xDist = xDist;
				}
			}
		}
	}
nSpawnSegs = gameData.multiplayer.nPlayerPositions;
SortSpawnMap (spawnMap, 0, nSpawnSegs - 1);
bRandom = (spawnMap [0].xDist >= SPAWN_MIN_DIST);

d_srand (SDL_GetTicks ());
j = 0;
for (;;) {
	i = bRandom ? d_rand () % nSpawnSegs : j++;
	nSpawnPos = spawnMap [i].i;
	if (IsTeamGame) {
		switch (gameData.multiplayer.playerInit [nSpawnPos].nSegType) {
			case SEGMENT_IS_GOAL_RED:
			case SEGMENT_IS_TEAM_RED:
				if (GetTeam (gameData.multiplayer.nLocalPlayer) != TEAM_RED)
					continue;
				break;
			case SEGMENT_IS_GOAL_BLUE:
			case SEGMENT_IS_TEAM_BLUE:
				if (GetTeam (gameData.multiplayer.nLocalPlayer) != TEAM_BLUE)
					continue;
				break;
			default:
				break;
			}
		}
	if (!bRandom || (spawnMap [i].xDist > SPAWN_MIN_DIST))
		break;
	if (i < --nSpawnSegs)
		memcpy (spawnMap + i, spawnMap + i + 1, nSpawnSegs - i);
	}
return nSpawnPos;
}

//------------------------------------------------------------------------------
//initialize the CPlayerData CObject position & orientation (at start of game, or new ship)
void InitPlayerPosition (int bRandom)
{
	int nSpawnPos = 0;

if (!(gameData.app.nGameMode & (GM_MULTI | GM_MULTI_COOP))) // If not deathmatch
	nSpawnPos = gameData.multiplayer.nLocalPlayer;
else if (bRandom == 1) {
	nSpawnPos = GetRandomPlayerPosition ();
	}
else {
	goto done; // If deathmatch and not Random, positions were already determined by sync packet
	}
Assert (nSpawnPos >= 0);
Assert (nSpawnPos < gameData.multiplayer.nPlayerPositions);

GetPlayerSpawn (nSpawnPos, gameData.objs.consoleP);

done:

ResetPlayerObject ();
ResetCruise ();
}

//------------------------------------------------------------------------------

fix RobotDefaultShields (CObject *objP)
{
	tRobotInfo	*botInfoP;
	int			objId, i;
	fix			shields;

Assert (objP->info.nType == OBJ_ROBOT);
objId = objP->info.nId;
//Assert (objId < gameData.bots.nTypes [0]);
i = gameStates.app.bD1Mission && (objId < gameData.bots.nTypes [1]);
botInfoP = gameData.bots.info [i] + objId;
//	Boost shield for Thief and Buddy based on level.
shields = botInfoP->strength;
if (botInfoP->thief || botInfoP->companion) {
	shields = (shields * (abs (gameData.missions.nCurrentLevel) + 7)) / 8;
	if (botInfoP->companion) {
		//	Now, scale guide-bot hits by skill level
		switch (gameStates.app.nDifficultyLevel) {
			case 0:
				shields = I2X (20000);
				break;		//	Trainee, basically unkillable
			case 1:
				shields *= 3;
				break;		//	Rookie, pretty dang hard
			case 2:
				shields *= 2;
				break;		//	Hotshot, a bit tough
			default:
				break;
			}
		}
	}
else if (botInfoP->bossFlag) {	//	MK, 01/16/95, make boss shields lower on lower diff levels.
	shields = shields / (NDL + 3) * (gameStates.app.nDifficultyLevel + 4);
//	Additional wimpification of bosses at Trainee
	if (gameStates.app.nDifficultyLevel == 0)
		shields /= 2;
	}
return shields;
}

//------------------------------------------------------------------------------
//	Initialize default parameters for one robot, copying from gameData.bots.infoP to *objP.
//	What about setting size!?  Where does that come from?
void CopyDefaultsToRobot (CObject *objP)
{
objP->info.xShields = RobotDefaultShields (objP);
}

//------------------------------------------------------------------------------
//	Copy all values from the robot info structure to all instances of robots.
//	This allows us to change bitmaps.tbl and have these changes manifested in existing robots.
//	This function should be called at level load time.
void CopyDefaultsToRobotsAll ()
{
	//int		i;
	CObject	*objP;

FORALL_ROBOT_OBJS (objP, i)
	CopyDefaultsToRobot (objP);

}

extern void ClearStuckObjects (void);

//------------------------------------------------------------------------------
//called when the CPlayerData is starting a level (new game or new ship)
void StartLevel (int bRandom)
{
Assert (!gameStates.app.bPlayerIsDead);
VerifyConsoleObject ();
InitPlayerPosition (bRandom);
gameData.objs.consoleP->info.controlType = CT_FLYING;
gameData.objs.consoleP->info.movementType = MT_PHYSICS;
MultiSendShields ();
DisableMatCens ();
ClearTransientObjects (0);		//0 means leave proximity bombs
// gameData.objs.consoleP->CreateAppearanceEffect ();
gameStates.render.bDoAppearanceEffect = 1;
if (IsMultiGame) {
	if (IsCoopGame)
		MultiSendScore ();
	MultiSendPosition (LOCALPLAYER.nObject);
	MultiSendReappear ();
	}
if (gameData.app.nGameMode & GM_NETWORK)
	NetworkDoFrame (1, 1);
AIResetAllPaths ();
AIInitBossForShip ();
ClearStuckObjects ();
#ifdef EDITOR
//	Note, this is only done if editor builtin.  Calling this from here
//	will cause it to be called after the CPlayerData dies, resetting the
//	hits for the buddy and thief.  This is ok, since it will work ok
//	in a shipped version.
InitAIObjects ();
#endif
ResetTime ();
ResetRearView ();
gameData.fusion.xAutoFireTime = 0;
gameData.fusion.xCharge = 0;
gameStates.app.cheats.bRobotsFiring = 1;
gameStates.app.cheats.bD1CheatsEnabled = 0;
gameStates.sound.bD1Sound = gameStates.app.bD1Mission && gameStates.app.bHaveD1Data && gameOpts->sound.bUseD1Sounds && !gameOpts->sound.bHires;
SetDataVersion (-1);
}

//------------------------------------------------------------------------------
//eof
