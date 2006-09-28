/* $Id: titles.c,v 1.29 2003/11/27 09:16:58 btb Exp $ */
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

/*
 *
 * Routines to display title screens...
 *
 * Old Log:
 * Revision 1.13  1995/11/03  12:52:23  allender
 * shareware changes
 *
 * Revision 1.12  1995/10/31  10:17:56  allender
 * shareware stuff
 *
 * Revision 1.11  1995/10/24  18:12:02  allender
 * don't do special processing on do_appl_quit anymore
 *
 * Revision 1.10  1995/10/21  22:25:02  allender
 * added bald guy cheat
 *
 * Revision 1.9  1995/10/17  13:14:30  allender
 * mouse will now move through title stuff
 *
 * Revision 1.8  1995/10/15  23:00:34  allender
 * made mouse move through screens and do event processing
 * at title screens
 *
 * Revision 1.7  1995/10/10  11:52:10  allender
 * use appropriate end01 file for registered
 *
 * Revision 1.6  1995/09/24  10:53:09  allender
 * added cmd-q to quit during titles screens and briefing screens
 *
 * Revision 1.5  1995/08/25  15:39:37  allender
 * save and resotre interpolation method during briefing screens
 *
 * Revision 1.4  1995/08/24  16:10:57  allender
 * endgame screen changes, and fixups to other stuff
 *
 * Revision 1.3  1995/08/14  14:40:37  allender
 * fixed up briefing screens to look correct.
 * made robot canvas smaller for now for speec
 *
 * Revision 1.2  1995/06/13  13:07:05  allender
 * do a bitblt during tight loops to get spinning robots and cursor flashing
 *
 * Revision 1.1  1995/05/16  15:31:52  allender
 * Initial revision
 *
 * Revision 2.10  1995/06/15  12:14:16  john
 * Made end game, win game and title sequences all go
 * on after 5 minutes automatically.
 *
 * Revision 2.9  1995/06/14  17:25:48  john
 * Fixed bug with VFX palette not getting loaded for credits, titles.
 *
 * Revision 2.8  1995/05/26  16:16:30  john
 * Split SATURN into define's for requiring cd, using cd, etc.
 * Also started adding all the Rockwell stuff.
 *
 * Revision 2.7  1995/03/24  13:11:36  john
 * Added save game during briefing screens.
 *
 * Revision 2.6  1995/03/21  14:41:17  john
 * Ifdef'd out the NETWORK code.
 *
 * Revision 2.5  1995/03/15  14:33:45  john
 * Added code to force the Descent CD-rom in the drive.
 *
 * Revision 2.4  1995/03/14  18:24:50  john
 * Force Destination Saturn to use CD-ROM drive.
 *
 * Revision 2.3  1995/03/10  13:05:52  john
 * Added code so that palette is correct for VFX1 helmets.
 *
 * Revision 2.2  1995/03/07  14:19:28  mike
 * More destination saturn stuff.
 *
 * Revision 2.1  1995/03/06  15:24:16  john
 * New screen techniques.
 *
 * Revision 2.0  1995/02/27  11:32:50  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.95  1995/02/12  04:07:19  matt
 * After freeing Robot_canv, set ptr to NULL
 *
 * Revision 1.94  1995/02/11  12:41:57  john
 * Added new song method, with FM bank switching..
 *
 * Revision 1.93  1995/02/11  09:51:59  john
 * Stripped out all SVGA references.
 *
 * Revision 1.92  1995/02/07  09:56:10  john
 * Fixed bug with wrong title screen drawing during save btwn lvls.
 *
 * Revision 1.91  1995/02/04  13:53:16  john
 * Added code to display the correct titles between levels
 * that have robot maps..
 *
 * Revision 1.90  1995/02/02  16:36:33  adam
 * *** empty log message ***
 *
 * Revision 1.89  1995/02/02  12:23:30  john
 * Made between level saves have picture.
 *
 * Revision 1.88  1995/02/02  01:50:16  adam
 * rearranged robot briefings.
 *
 * Revision 1.87  1995/02/01  22:28:54  mike
 * *** empty log message ***
 *
 * Revision 1.86  1995/02/01  17:12:21  mike
 * Make score come after endgame screens.
 *
 * Revision 1.85  1995/02/01  14:04:17  adam
 * mucked with screens
 *
 * Revision 1.84  1995/01/28  16:59:30  adam
 * added hook for special first briefing music
 *
 * Revision 1.83  1995/01/21  17:04:35  mike
 * fix endgame text bogosity.
 *
 * Revision 1.82  1995/01/21  16:26:29  matt
 * Made endlevel briefing work with missions
 *
 * Revision 1.81  1995/01/21  13:15:20  adam
 * added new robot briefings.
 *
 * Revision 1.80  1995/01/20  22:47:33  matt
 * Mission system implemented, though imcompletely
 *
 * Revision 1.79  1995/01/20  14:57:52  mike
 * support arbitrary number of screens/level.
 *
 * Revision 1.78  1995/01/15  14:29:12  john
 * Made the exit door page in.
 *
 * Revision 1.77  1995/01/15  14:27:09  adam
 * messed with endgame stuff
 *
 * Revision 1.76  1995/01/14  15:42:49  mike
 * Make endgame for registered support 3 screens.
 *
 * Revision 1.75  1995/01/14  14:00:59  adam
 * *** empty log message ***
 *
 * Revision 1.74  1995/01/09  11:28:53  mike
 * Support new $O thingy for bitmaps which cycle and are not the exit door.
 *
 * Revision 1.73  1995/01/02  12:39:13  mike
 * fix secret level sequencing.
 *
 * Revision 1.72  1994/12/12  19:44:14  rob
 * Added kill matrix to end of game sequence.
 *
 * Revision 1.71  1994/12/09  22:32:57  adam
 * messed with text positioning
 *
 * Revision 1.70  1994/12/09  14:08:37  mike
 * make briefing text work with hog files.
 *
 * Revision 1.69  1994/12/09  00:41:47  mike
 * fix hang in automap print screen.
 *
 * Revision 1.68  1994/12/08  20:56:36  john
 * More cfile stuff.
 *
 * Revision 1.67  1994/12/08  17:19:11  yuan
 * Cfiling stuff.
 *
 * Revision 1.66  1994/12/07  11:28:00  matt
 * Did a localization suppport
 *
 * Revision 1.65  1994/12/06  17:10:23  yuan
 * Fixed missing)
 *
 * Revision 1.64  1994/12/06  17:00:45  rob
 * Fixed problem with finishing the game in modem/serial mode.
 *
 * Revision 1.63  1994/12/06  16:58:37  matt
 * Killed warnings
 *
 * Revision 1.62  1994/12/06  15:54:28  mike
 * fix guy at end...
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef WINDOWS
#include "desw.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pa_enabl.h"                   //$$POLY_ACC
#include "inferno.h"
#include "timer.h"
#include "key.h"
#include "gr.h"
#include "palette.h"
#include "iff.h"
#include "pcx.h"
#include "u_mem.h"
#include "joy.h"
#include "mono.h"
#include "gamefont.h"
#include "cfile.h"
#include "error.h"
#include "polyobj.h"
#include "textures.h"
#include "screens.h"
#include "multi.h"
#include "player.h"
#include "digi.h"
#include "compbit.h"
#include "text.h"
#include "kmatrix.h"
#include "piggy.h"
#include "songs.h"
#include "newmenu.h"
#include "state.h"
#include "movie.h"
#include "menu.h"
#include "mouse.h"
#include "gamepal.h"
#include "cfile.h"
#include "globvars.h"
#include "strutil.h"

#include "ogl_init.h"
#ifdef __macosx__
# include <OpenGL/glu.h>
# undef GL_ARB_multitexture // hack!
#else
# include <GL/glu.h>
#endif

#if defined (POLY_ACC)
#include "poly_acc.h"
#endif

#define KEY_DELAY_DEFAULT       ((F1_0*20)/1000)

typedef struct {
	char    bs_name[14];                //  filename, eg merc01.  Assumes .lbm suffix.
	sbyte   level_num;
	sbyte   message_num;
	short   text_ulx, text_uly;         //  upper left x,y of text window
	short   text_width, text_height;    //  width and height of text window
} briefing_screen;


void DoBriefingColorStuff ();
int get_new_message_num (char **message);
int DefineBriefingBox (char **buf, briefing_screen *pBriefBuf);

extern unsigned RobSX,RobSY,RobDX,RobDY; // Robot movie coords

//static ubyte brief_palette [256*3];
//static ubyte robot_palette [256*3];
static int	brief_palette_254_bash;
static int  bInitRobot;

char curBriefScreenName[15] = "";
char	* Briefing_text = NULL;
int briefingTextLen;
char bRobotPlaying=0;

//Begin D1X modification
#define MAX_BRIEFING_COLORS     7
//End D1X modification

// Descent 1 briefings
char szEndingTextFilename[13] = "endreg.txt";
char szBriefingTextFilename[13] = "briefing.txt";

#define	SHAREWARE_ENDING_FILENAME	"ending.tex"

//	Can be set by -noscreens command line option.  Causes bypassing of all briefing screens.
int	briefFgColorIndex[MAX_BRIEFING_COLORS], 
		briefBgColorIndex[MAX_BRIEFING_COLORS];
int	Current_color = 0;
unsigned int nEraseColor = BLACK_RGBA;

grs_rgba briefFgColors [2][7] = {
	{{0,160,0,255},{160,132,140,255},{32,124,216,255},{0,0,216,255},{56,56,56,255},{216,216,0,255},{0,216,216,255}},
	{{0,216,0,255},{168,152,128,255},{252,0,0,255},{0,0,216,255},{56,56,56,255},{216,216,0,255},{0,216,216,255}}
	};

grs_rgba briefBgColors [2][7] = {
	{{0,24,255},{20,20,20,255},{4,16,28,255},{0,0,76,255},{0,0,0,255},{76,76,0,255},{0,76,76,255}},
	{{0,76,0,255},{56,56,56,255},{124,0,0,255},{0,0,76,255},{0,0,0,255},{76,76,0,255},{0,76,76,255}}
	};

grs_rgba eraseColorRgb = {0,0,0,255};

typedef struct tD1ExtraBotSound {
	char	*pszName;
	short nLevel;
	short	nBot;
} tD1ExtraBotSound;

tD1ExtraBotSound extraBotSounds [] = {
	{"enemy01.wav", 1, 0},
	{"enemy02.wav", 1, 1},
	{"enemy03.wav", 1, 2},
	{"enemy04.wav", 1, 3},
	{"enemy05.wav", 1, 4},
	{"enemy06.wav", 1, 5},
	{"enemy07.wav", 6, 0},
	{"enemy08.wav", 8, 0},
	{"enemy09.wav", 8, 1},
	{"enemy10.wav", 8, 2},
	{"enemy11.wav", 10, 0},
	{"enemy12.wav", 13, 0},
	{"enemy13.wav", 16, 0},
	{"enemy14.wav", 17, 0}
};

//-----------------------------------------------------------------------------

int StartBriefingSound (int c, short nSound, fix nVolume, char *pszWAV)
{
if (c < 0) {
	int b = gameOpts->sound.bD1Sound;
	gameOpts->sound.bD1Sound = 0;
	c = DigiStartSound (DigiXlatSound (nSound), nVolume, 0xFFFF / 2, 1, -1, -1, -1, F1_0, pszWAV);
	gameOpts->sound.bD1Sound = b;
	}
return c;
}

//-----------------------------------------------------------------------------

void StopBriefingSound (int *c)
{
if (*c > -1) {
	DigiStopSound (*c);
	*c = -1;
	}
}

//-----------------------------------------------------------------------------

int StartBriefingHum (int nChannel, int nLevel, int nScreen, int bExtraSounds)
{
if (bExtraSounds && (nScreen > 3)) {	//only play for the mission directives on a planet background
	char	szWAV [10];

	if (nLevel < 0)
		nLevel = 27 - nLevel;
 	sprintf (szWAV, "brf%02d.wav", nLevel);
	return StartBriefingSound (nChannel, SOUND_BRIEFING_HUM, F1_0 * 4, szWAV);
	}
return StartBriefingSound (nChannel, SOUND_BRIEFING_HUM, F1_0 / 2, NULL);
}

//-----------------------------------------------------------------------------

tD1ExtraBotSound *FindExtraBotSound (short nLevel, short nBot)
{
	tD1ExtraBotSound	*p = extraBotSounds;
	int					i = sizeof (extraBotSounds) / sizeof (tD1ExtraBotSound);

if (!gameStates.app.bD1Mission)
	return NULL;
for (; i; i--, p++)
	if ((p->nLevel == nLevel) && (p->nBot == nBot))
		return p;
return NULL;
}

//-----------------------------------------------------------------------------

int StartExtraBotSound (int nChannel, short nLevel, short nBot)
{
	tD1ExtraBotSound	*p;

StopBriefingSound (&nChannel);
if (!gameStates.app.bHaveExtraData)
	return -1;
if (! (p = FindExtraBotSound (nLevel, nBot)))
	return -1;
return StartBriefingSound (nChannel, -1, 8 * F1_0, p->pszName);
}

//-----------------------------------------------------------------------------

extern int NMCheckButtonPress ();

// added by Jan Bobrowski for variable-size menu screen
static int rescale_x (int x)
{
	return x * GWIDTH / 320;
}

//-----------------------------------------------------------------------------

static int rescale_y (int y)
{
	return y * GHEIGHT / 200;
}

//-----------------------------------------------------------------------------

int local_key_inkey (void)
{
	int	rval;

#ifdef WINDOWS
	MSG msg;

	DoMessageStuff (&msg);
#endif

	rval = KeyInKey ();

	if (rval == KEY_PRINT_SCREEN) {
#ifdef POLY_ACC
		if (bRobotPlaying) {
			GrPaletteRead (grPalette);
			GrCopyPalette (grPalette,grPalette,0);	//reset color lookup cache
		}
#endif
		SaveScreenShot (NULL, 0);
		return 0;				//say no key pressed
	}

	if (NMCheckButtonPress ())		//joystick or mouse button pressed?
		rval = KEY_SPACEBAR;
	else if (MouseButtonState (0))
		rval = KEY_SPACEBAR;

	return rval;
}

//-----------------------------------------------------------------------------

int LoadBriefImg (char *pszImg, grs_bitmap *bmP, int bFullScr)
{
	char	*ps, c;
	char	szImg [FILENAME_LEN+1];
	int	pcxErr;

strcpy (szImg, pszImg);
strlwr (szImg);
if (! (ps = strstr (szImg, "b.pcx")) && (ps = strstr (szImg, ".pcx"))) {
	c = *--ps;
	strcpy (--ps, "0b.pcx");
	*ps = c;
	}

for (;;) {
	pcxErr = bFullScr ?
		pcx_read_fullscr (szImg, gameStates.app.bD1Mission) :
		pcx_read_bitmap (szImg, bmP, BM_LINEAR, gameStates.app.bD1Mission);
	if (pcxErr == PCX_ERROR_NONE)
		break;
	if (!ps)
		break;
	strcpy (ps, "00.pcx");
	*++ps = c;
	ps = NULL;
	} 
return pcxErr;
}

//-----------------------------------------------------------------------------

int show_title_screen (char * filename, int allow_keys, int from_hog_only)
{
	fix timer;
	int pcx_error;
	grs_bitmap title_bm;
	char new_filename [FILENAME_LEN+1] = "";

	#ifdef RELEASE
	if (from_hog_only)
		strcpy (new_filename,"\x01");	//only read from hog file
	#endif

	strcat (new_filename,filename);
	filename = new_filename;

	title_bm.bm_texBuf=NULL;
	if ((pcx_error = LoadBriefImg (filename, &title_bm, 0)) != PCX_ERROR_NONE) {
#if TRACE
		con_printf (CON_DEBUG, "File '%s', PCX load error: %s (%i)\n  (No big deal, just no title screen.)\n",filename, pcx_errormsg (pcx_error), pcx_error);
#endif
		Error ("Error loading briefing screen <%s>, PCX load error: %s (%i)\n",filename, pcx_errormsg (pcx_error), pcx_error);
	}

#if defined (POLY_ACC)
    pa_save_clut ();
    pa_update_clut (title_bm.bm_palette, 0, 256, 0);
#endif

	//vfx_set_palette_sub (brief_palette);
#ifdef OGL
	GrPaletteStepLoad (NULL);
#else
	GrPaletteStepClear ();
#endif

	WINDOS (
		DDGrSetCurrentCanvas (NULL),
		GrSetCurrentCanvas (NULL)
	);
	WIN (DDGRLOCK (dd_grd_curcanv));
	show_fullscr (&title_bm);
	WIN (DDGRUNLOCK (dd_grd_curcanv));

	WIN (DDGRRESTORE);

#if defined (POLY_ACC)
    pa_restore_clut ();
#endif

	if (GrPaletteFadeIn (NULL, 32, allow_keys))
		return 1;

	GrPaletteStepLoad (NULL);
	timer	= TimerGetFixedSeconds () + i2f (3);
	while (1) {
		if (local_key_inkey () && allow_keys) break;
		if (TimerGetFixedSeconds () > timer) break;
	}
	if (GrPaletteFadeOut (NULL, 32, allow_keys))
		return 1;
	d_free (title_bm.bm_texBuf);
	return 0;
}

//-----------------------------------------------------------------------------

#define BRIEFING_SECRET_NUM 31          //  This must correspond to the first secret level which must come at the end of the list.
#define BRIEFING_OFFSET_NUM 4           // This must correspond to the first level screen (ie, past the bald guy briefing screens)

#define	SHAREWARE_ENDING_LEVEL_NUM  0x7f
#define	REGISTERED_ENDING_LEVEL_NUM 0x7e

#ifdef SHAREWARE
#define ENDING_LEVEL_NUM 	SHAREWARE_ENDING_LEVEL_NUM
#else
#define ENDING_LEVEL_NUM 	REGISTERED_ENDING_LEVEL_NUM
#endif

#define MAX_BRIEFING_SCREENS 60

#if 0
briefing_screen Briefing_screens[MAX_BRIEFING_SCREENS]=
 {{"brief03.pcx",0,3,8,8,257,177}}; // default=0!!!
#else
briefing_screen Briefing_screens[] = {
	{ "brief01.pcx",   0,  1,  13, 140, 290,  59 },
	{ "brief02.pcx",   0,  2,  27,  34, 257, 177 },
	{ "brief03.pcx",   0,  3,  20,  22, 257, 177 },
	{ "brief02.pcx",   0,  4,  27,  34, 257, 177 },

	{ "moon01.pcx",    1,  5,  10,  10, 300, 170 }, // level 1
	{ "moon01.pcx",    2,  6,  10,  10, 300, 170 }, // level 2
	{ "moon01.pcx",    3,  7,  10,  10, 300, 170 }, // level 3

	{ "venus01.pcx",   4,  8,  15, 15, 300,  200 }, // level 4
	{ "venus01.pcx",   5,  9,  15, 15, 300,  200 }, // level 5

	{ "brief03.pcx",   6, 10,  20,  22, 257, 177 },
	{ "merc01.pcx",    6, 11,  10, 15, 300, 200 },  // level 6
	{ "merc01.pcx",    7, 12,  10, 15, 300, 200 },  // level 7

#ifndef SHAREWARE
	{ "brief03.pcx",   8, 13,  20,  22, 257, 177 },
	{ "mars01.pcx",    8, 14,  10, 100, 300,  200 }, // level 8
	{ "mars01.pcx",    9, 15,  10, 100, 300,  200 }, // level 9
	{ "brief03.pcx",  10, 16,  20,  22, 257, 177 },
	{ "mars01.pcx",   10, 17,  10, 100, 300,  200 }, // level 10

	{ "jup01.pcx",    11, 18,  10, 40, 300,  200 }, // level 11
	{ "jup01.pcx",    12, 19,  10, 40, 300,  200 }, // level 12
	{ "brief03.pcx",  13, 20,  20,  22, 257, 177 },
	{ "jup01.pcx",    13, 21,  10, 40, 300,  200 }, // level 13
	{ "jup01.pcx",    14, 22,  10, 40, 300,  200 }, // level 14

	{ "saturn01.pcx", 15, 23,  10, 40, 300,  200 }, // level 15
	{ "brief03.pcx",  16, 24,  20,  22, 257, 177 },
	{ "saturn01.pcx", 16, 25,  10, 40, 300,  200 }, // level 16
	{ "brief03.pcx",  17, 26,  20,  22, 257, 177 },
	{ "saturn01.pcx", 17, 27,  10, 40, 300,  200 }, // level 17

	{ "uranus01.pcx", 18, 28,  100, 100, 300,  200 }, // level 18
	{ "uranus01.pcx", 19, 29,  100, 100, 300,  200 }, // level 19
	{ "uranus01.pcx", 20, 30,  100, 100, 300,  200 }, // level 20
	{ "uranus01.pcx", 21, 31,  100, 100, 300,  200 }, // level 21

	{ "neptun01.pcx", 22, 32,  10, 20, 300,  200 }, // level 22
	{ "neptun01.pcx", 23, 33,  10, 20, 300,  200 }, // level 23
	{ "neptun01.pcx", 24, 34,  10, 20, 300,  200 }, // level 24

	{ "pluto01.pcx",  25, 35,  10, 20, 300,  200 }, // level 25
	{ "pluto01.pcx",  26, 36,  10, 20, 300,  200 }, // level 26
	{ "pluto01.pcx",  27, 37,  10, 20, 300,  200 }, // level 27

	{ "aster01.pcx",  -1, 38,  10, 90, 300,  200 }, // secret level -1
	{ "aster01.pcx",  -2, 39,  10, 90, 300,  200 }, // secret level -2
	{ "aster01.pcx",  -3, 40,  10, 90, 300,  200 }, // secret level -3
#endif

	{ "end01.pcx",   SHAREWARE_ENDING_LEVEL_NUM,  1,  23, 40, 320, 200 },   // shareware end
#ifndef SHAREWARE
	{ "end02.pcx",   REGISTERED_ENDING_LEVEL_NUM,  1,  5, 5, 300, 200 },    // registered end
	{ "end01.pcx",   REGISTERED_ENDING_LEVEL_NUM,  2,  23, 40, 320, 200 },  // registered end
	{ "end03.pcx",   REGISTERED_ENDING_LEVEL_NUM,  3,  5, 5, 300, 200 },    // registered end
#endif

};

#endif

int	Briefing_text_x, Briefing_text_y;

grs_canvas	*Robot_canv = NULL;
vms_angvec	Robot_angles;

char    Bitmap_name[32] = "";
#define EXIT_DOOR_MAX   14
#define OTHER_THING_MAX 10      //  Adam: This is the number of frames in your new animating thing.
#define DOOR_DIV_INIT   6
sbyte   Door_dir=1, Door_div_count=0, Animating_bitmap_type=0;

//-----------------------------------------------------------------------------

void init_char_pos (briefing_screen *bsp, int bRescale)
{
if (bRescale) {
	bsp->text_ulx = rescale_x (bsp->text_ulx);
	bsp->text_uly = rescale_y (bsp->text_uly);
	bsp->text_width = rescale_x (bsp->text_width);
	bsp->text_height = rescale_y (bsp->text_height);
	}
Briefing_text_x = bsp->text_ulx;
Briefing_text_y = gameStates.app.bD1Mission ? bsp->text_uly : bsp->text_uly - (8 * (1 + gameStates.menus.bHires));
}

//-----------------------------------------------------------------------------

void ShowBitmapFrame (int bRedraw)
{
#ifdef WINDOWS
	dd_grs_canvas *curcanv_save, *bitmap_canv=0;
#else
	grs_canvas *curcanv_save, *bitmap_canv=0;
#endif

	grs_bitmap *bitmap_ptr;
	int x = rescale_x (138);
	int y = rescale_y (55);
	int w = rescale_x (166);
	int h = rescale_y (138);

	//	Only plot every nth frame.
	if (!bRedraw && Door_div_count) {
		Door_div_count--;
		if (!gameOpts->menus.nStyle)
			return;
	}

	if (Bitmap_name[0] != 0) {
		char		*pound_signp;
		int		num, dig1, dig2;

		//	Set supertransparency color to black
		switch (Animating_bitmap_type) {
			case 0: 
				WINDOS (
					bitmap_canv = DDGrCreateSubCanvas (dd_grd_curcanv, x, y, w, h);/*rescale_x (220), rescale_x (45), 64, 64);*/	break,
					bitmap_canv = GrCreateSubCanvas (grdCurCanv, x, y, w, h);/*rescale_x (220), rescale_x (45), 64, 64);*/	break
					);
			case 1:
				WINDOS (
					bitmap_canv = DDGrCreateSubCanvas (dd_grd_curcanv, x, y, w, h);/*rescale_x (220), rescale_x (45), 64, 64);*/	break,
					bitmap_canv = GrCreateSubCanvas (grdCurCanv, x, y, w, h);/*rescale_x (220), rescale_x (45), 64, 64);*/	break
					);

			// Adam: Change here for your new animating bitmap thing. 94, 94 are bitmap size.
			default:
				Int3 (); // Impossible, illegal value for Animating_bitmap_type
			}

		WINDOS (
			curcanv_save = dd_grd_curcanv; dd_grd_curcanv = bitmap_canv,
			curcanv_save = grdCurCanv; grdCurCanv = bitmap_canv
		);
		if (!bRedraw) {	//extract current bitmap number from bitmap name (<name>#<number>)
			pound_signp = strchr (Bitmap_name, '#');
			Assert (pound_signp != NULL);

			dig1 = * (pound_signp+1);
			dig2 = * (pound_signp+2);
			if (dig2 == 0)
				num = dig1-'0';
			else
				num = (dig1-'0')*10 + (dig2-'0');

			switch (Animating_bitmap_type) {
				case 0:
					if (!Door_div_count) {
						num += Door_dir;
						if (num > EXIT_DOOR_MAX) {
							num = EXIT_DOOR_MAX;
							Door_dir = -1;
						} else if (num < 0) {
							num = 0;
							Door_dir = 1;
						}
					}
					break;
				case 1:
					if (!Door_div_count)
						num++;
					if (num > OTHER_THING_MAX)
						num = 0;
					break;
				}

			Assert (num < 100);
			if (num >= 10) {
				* (pound_signp+1) = (num / 10) + '0';
				* (pound_signp+2) = (num % 10) + '0';
				* (pound_signp+3) = 0;
				} 
			else {
				* (pound_signp+1) = (num % 10) + '0';
				* (pound_signp+2) = 0;
				}
			}

		LoadPalette ("", "", 0, 0, 1);
		{
		bitmap_index bi;
		bi = piggy_find_bitmap (Bitmap_name, 0);
		bitmap_ptr = gameData.pig.tex.bitmaps [0] + bi.index;
		PIGGY_PAGE_IN (bi, 0);
//			OglLoadBmTexture (gameData.pig.tex.bitmaps + bi.index);
		}

		WIN (DDGRLOCK (dd_grd_curcanv));
		//GrBitmapM (0, 0, bitmap_ptr);
		//show_fullscr (bitmap_ptr);
		{
		#define DEFAULT_VIEW_DIST 0x60000
		vms_vector	temp_pos=ZERO_VECTOR;
		vms_matrix	temp_orient = IDENTITY_MATRIX;
		GrClearCanvas (0);
		G3StartFrame (0,0);
		G3SetViewMatrix (&temp_pos,&temp_orient,0);
		memcpy (&viewInfo.view, &temp_orient, sizeof (viewInfo.view));
		VmsMatToFloat (&viewInfo.viewf, &viewInfo.view);
		temp_pos.z = w * F1_0;
		G3DrawBitMap (&temp_pos, w * F1_0, h * F1_0, bitmap_ptr, 0, 1.0, 0, 0);
		G3EndFrame ();
		if (!gameOpts->menus.nStyle)
			GrUpdate (0);
		}
		GrPaletteStepLoad (NULL);
		WIN (DDGRUNLOCK (dd_grd_curcanv));
		WINDOS (
			dd_grd_curcanv = curcanv_save,
			grdCurCanv = curcanv_save
		);
		d_free (bitmap_canv);
		if (! (bRedraw || Door_div_count)) {
#if 1
		Door_div_count = DOOR_DIV_INIT;
#else
		switch (Animating_bitmap_type) {
			case 0:
				if (num == EXIT_DOOR_MAX) {
					Door_dir = -1;
					Door_div_count = DOOR_DIV_INIT;
					} 
				else if (num == 0) {
					Door_dir = 1;
					Door_div_count = DOOR_DIV_INIT;
				}
				break;
			case 1:
				break;
			}
#endif
		}
	}
}

//-----------------------------------------------------------------------------

void show_briefing_bitmap (grs_bitmap *bmp)
{
#ifdef WINDOWS
  	dd_grs_canvas *bitmap_canv, *curcanv_save;

	bitmap_canv = DDGrCreateSubCanvas (dd_grd_curcanv, 220, 45, bmp->bm_props.w, bmp->bm_props.h);
	curcanv_save = dd_grd_curcanv;
	DDGrSetCurrentCanvas (bitmap_canv);
	DDGRLOCK (dd_grd_curcanv);
	GrBitmapM (0,0,bmp);
	DDGRUNLOCK (dd_grd_curcanv);
	DDGrSetCurrentCanvas (curcanv_save);
#else
	grs_canvas	*curcanv_save, *bitmap_canv;

	bitmap_canv = GrCreateSubCanvas (grdCurCanv, 220, 45, bmp->bm_props.w, bmp->bm_props.h);
	curcanv_save = grdCurCanv;
	GrSetCurrentCanvas (bitmap_canv);
	GrBitmapM (0, 0, bmp);
	GrSetCurrentCanvas (curcanv_save);
#endif

	d_free (bitmap_canv);
}

#ifndef WINDOWS

//-----------------------------------------------------------------------------

void ShowSpinningRobotFrame (int robot_num)
{
	grs_canvas	*curcanv_save;

	if (robot_num != -1) {
		Robot_angles.h += 150;

		curcanv_save = grdCurCanv;
		grdCurCanv = Robot_canv;
		Assert (gameData.bots.pInfo[robot_num].model_num != -1);
		if (bInitRobot) 
			{
//			GrPaletteStepLoad (robot_palette);
			LoadPalette ("", "", 0, 0, 1);
			OglCachePolyModelTextures (gameData.bots.pInfo[robot_num].model_num);
			GrPaletteStepLoad (NULL);
			bInitRobot = 0;
			}
 		DrawModelPicture (gameData.bots.pInfo[robot_num].model_num, &Robot_angles);
		grdCurCanv = curcanv_save;
	}

}

//-----------------------------------------------------------------------------

void RotateBriefingRobot (void)
{
	grs_canvas	*curcanv_save = grdCurCanv;
	grdCurCanv = Robot_canv;
	RotateRobot ();
	grdCurCanv = curcanv_save;

}

//-----------------------------------------------------------------------------

void InitSpinningRobot (void) // (int x,int y,int w,int h)
{
	//Robot_angles.p += 0;
	//Robot_angles.b += 0;
	//Robot_angles.h += 0;

	int x = rescale_x (138);
	int y = rescale_y (55);
	int w = rescale_x (163);
	int h = rescale_y (136);

	Robot_canv = GrCreateSubCanvas (grdCurCanv, x, y, w, h);
	bInitRobot = 1;
	// 138, 55, 166, 138
}
#endif

//---------------------------------------------------------------------------
// Returns char width.
// If show_robot_flag set, then show a frame of the spinning robot.
int show_char_delay (char the_char, int delay, int robot_num, int cursor_flag, int bRedraw)
{
	int w, h, aw;
	char message[2];
	static fix	t, t0, StartTime=0;

	message[0] = the_char;
	message[1] = 0;

	if (StartTime==0 && TimerGetFixedSeconds ()<0)
		StartTime=TimerGetFixedSeconds ();

GrGetStringSize (message, &w, &h, &aw);

Assert ((Current_color >= 0) && (Current_color < MAX_BRIEFING_COLORS));

//	Draw cursor if there is some delay and caller says to draw cursor
if (cursor_flag && !bRedraw) {
	WIN (DDGRLOCK (dd_grd_curcanv));
	GrSetFontColorRGB (briefFgColors [gameStates.app.bD1Mission] + Current_color, NULL);
	GrPrintF (Briefing_text_x+1, Briefing_text_y, "_");
	WIN (DDGRUNLOCK (dd_grd_curcanv));
	if (!gameOpts->menus.nStyle)
		GrUpdate (0);
}

if (delay > 0)
	delay=fixdiv (F1_0,i2f (15));

if ((delay > 0) && !bRedraw) {
	if (Bitmap_name[0] != 0)
		ShowBitmapFrame (0);
	if (bRobotPlaying)
		RotateBriefingRobot ();
	t0 = StartTime;
	while ((t = TimerGetFixedSeconds ()) < (StartTime + delay)) {
		if (bRobotPlaying)
			RotateBriefingRobot ();
		if (t >= t0 + KEY_DELAY_DEFAULT/2) {
			if ((Bitmap_name[0] != 0) && (delay != 0))
				ShowBitmapFrame (0);
			if (robot_num != -1)
				ShowSpinningRobotFrame (robot_num);
				t0 = t;
			}
		}
	}

StartTime = TimerGetFixedSeconds ();

WIN (DDGRLOCK (dd_grd_curcanv));
//	Erase cursor
if (cursor_flag && (delay > 0) && !bRedraw) {
	GrSetFontColorRGBi (nEraseColor, 1, 0, 0);
	GrPrintF (Briefing_text_x+1, Briefing_text_y, "_");
	//	erase the character
	GrSetFontColorRGB (briefBgColors [gameStates.app.bD1Mission] + Current_color, NULL);
	GrPrintF (Briefing_text_x, Briefing_text_y, message);
}
//draw the character
GrSetFontColorRGB (briefFgColors [gameStates.app.bD1Mission] + Current_color, NULL);
GrPrintF (Briefing_text_x+1, Briefing_text_y, message);
WIN (DDGRUNLOCK (dd_grd_curcanv));

if (! (bRedraw || gameOpts->menus.nStyle)) 
	GrUpdate (0);

//	if (the_char != ' ')
//		if (!DigiIsSoundPlaying (SOUND_MARKER_HIT))
//			DigiPlaySample (SOUND_MARKER_HIT, F1_0);

return w;
}

//-----------------------------------------------------------------------------

int load_briefing_screen (int nScreen)
{
	int	pcx_error;
	char *szBriefScreen;

if (gameStates.app.bD1Mission)
	szBriefScreen = Briefing_screens[nScreen].bs_name;
else
	szBriefScreen = curBriefScreenName;
if (*szBriefScreen) {
	glClear (GL_COLOR_BUFFER_BIT);
	WIN (DDGRLOCK (dd_grd_curcanv));
	if ((pcx_error = pcx_read_fullscr (szBriefScreen, gameStates.app.bD1Mission)) != PCX_ERROR_NONE) {
		WIN (DDGRUNLOCK (dd_grd_curcanv));
#ifdef _DEBUG
		Error ("Error loading briefing screen <%s>,\nPCX load error: %s (%i)\n", szBriefScreen, pcx_errormsg (pcx_error), pcx_error);
#endif
		}
	WIN (DDGRUNLOCK (dd_grd_curcanv));
	WIN (DDGRRESTORE);
	}
return 0;
}

//-----------------------------------------------------------------------------

int LoadNewBriefingScreen (char *szBriefScreen, int bRedraw)
{
	int pcx_error;

#if TRACE
	con_printf (CON_DEBUG,"Loading new briefing <%s>\n",szBriefScreen);
#endif
	strcpy (curBriefScreenName,szBriefScreen);
	//WIN (DEFINE_SCREEN (curBriefScreenName);
	if (GrPaletteFadeOut (NULL, 32, 0))
		return 0;
	WIN (DDGRLOCK (dd_grd_curcanv));
	if ((pcx_error = LoadBriefImg (szBriefScreen, NULL, 1)) != PCX_ERROR_NONE) {
		WIN (DDGRUNLOCK (dd_grd_curcanv));
#ifdef _DEBUG
		Error ("Error loading briefing screen <%s>,\nPCX load error: %s (%i)\n",
				szBriefScreen, pcx_errormsg (pcx_error), pcx_error);
#endif
	}
	WIN (DDGRUNLOCK (dd_grd_curcanv));
	WIN (DDGRRESTORE);
	//GrUpdate (0);
	if (GrPaletteFadeIn (NULL, 32, 0))
		return 0;
	DoBriefingColorStuff ();
	//GrUpdate (1);
	return 1;
}

//-----------------------------------------------------------------------------

int get_message_num (char **message)
{
	int	num=0;
	char	*psz = *message;

while (*psz && (*psz == ' '))
	psz++;
while ((*psz >= '0') && (*psz <= '9')) {
	num = 10*num + *psz-'0';
	psz++;
	}
while (*psz && (*psz++ != 10))		//	Get and drop eoln
	;
*message = psz;
return num;
}

//-----------------------------------------------------------------------------

void get_message_name (char **message, char *result)
{
	char	*psz = *message;

while (*psz == ' ')
	 (*message)++;

while ((*psz != ' ') && (*psz != 10)) {
	if (*psz != '\n')
		*result++ = *psz;
	psz++;
	}
if (*psz != 10)
	while (*psz++ != 10)		//	Get and drop eoln
		;
*message = psz;
*result = 0;
}

//-----------------------------------------------------------------------------

void flash_cursor (int cursor_flag)
{
	if (cursor_flag == 0)
		return;

WIN (DDGRLOCK (dd_grd_curcanv));
	if ((TimerGetFixedSeconds () % (F1_0/2)) > (F1_0/4))
		GrSetFontColorRGB (briefFgColors [gameStates.app.bD1Mission] + Current_color, NULL);
	else
		GrSetFontColorRGB (&eraseColorRgb, NULL);

	GrPrintF (Briefing_text_x+1, Briefing_text_y, "_");
WIN (DDGRUNLOCK (dd_grd_curcanv));
if (curDrawBuffer == GL_FRONT)
	GrUpdate (0);
}

extern int InitMovieBriefing ();

//-----------------------------------------------------------------------------

char *NextPage (char *message)
{
	char	*pNextPage = strstr (message, "$P");
	char	*pNextBrief = strstr (message, "$S");

if (pNextPage && pNextBrief)
	return ((pNextPage < pNextBrief) ? pNextPage : pNextBrief);
return (pNextPage ? pNextPage : pNextBrief);
return NULL;
}

//-----------------------------------------------------------------------------

int PageHasRobot (char *message)
{
	char	*pEnd = NextPage (message);
	char	*pBot = strstr (message, "$R");

return pBot && (!pEnd || (pBot < pEnd));
}

//-----------------------------------------------------------------------------

char *SkipPage (char *message, briefing_screen *pBriefBuf, int *px, int *py, int *pnScreen)
{
	char	ch, *pEnd = NextPage (message);
	int	nScreen = *pnScreen, x = *px, y = *py;

while (*message && (!pEnd || (message < pEnd))) {
	ch = *message++;
	if (ch == '$') {
		ch = *message++;
		if (ch=='D') {
			nScreen = DefineBriefingBox (&message, pBriefBuf);
			x = Briefing_text_x;
			y = Briefing_text_y;
			}
		else if (ch=='U') {
			nScreen = get_message_num (&message);
			*pBriefBuf = Briefing_screens [nScreen];
			init_char_pos (pBriefBuf, 1);
			x = Briefing_text_x;
			y = Briefing_text_y;
			}
		}
	}
*pnScreen = nScreen;
*px = x;
*py = y;
if (!message)
	return NULL;
ch = *message;
if (ch == '$') {
	ch = *++message;
	if (ch == 'S')
		return NULL;
	do {
		message++;
		} while (isdigit (*message) || isspace (*message));
	}
return message;
}

//-----------------------------------------------------------------------------
// Return true if message got aborted by user (pressed ESC), else return false.
int ShowBriefingMessage (int nScreen, char *message, int nLevel)
{
	static int tab_stop=0;

	briefing_screen	*bsp, briefBuf;
	int			prev_ch=-1;
	int			ch, done=0,i;
	int			delay_count = KEY_DELAY_DEFAULT;
	int			key_check;
	int			robot_num=-1;
	int			rval=0;
	int			flashing_cursor=0;
	int			new_page=0,GotZ=0;
	int			hum_channel = -1, printing_channel = -1, bot_channel = -1;
	int			LineAdjustment=1;
	int			bHaveScreen = 0;
	char			*pi, *pj;
	int			x, y, bRedraw, bPageDone;
	char			spinRobotName[]="rba.mve",kludge;  // matt don't change this!
	char			szBriefScreen[15], szBriefScreenB[15];
	char			DumbAdjust=0;
	char			chattering=0;
	short 		nBot = 0;
	char			*pEnd = strstr (message, "$S");
	int			bOnlyRobots, bExtraSounds;
	WIN (int wpage_done=0);

Bitmap_name[0] = 0;
Current_color = 0;
bRobotPlaying=0;

InitMovieBriefing ();

bExtraSounds = gameStates.app.bHaveExtraData && gameStates.app.bD1Mission && 
				  (gameData.missions.nCurrentMission == gameData.missions.nD1BuiltinMission);
bOnlyRobots = gameStates.app.bHaveExtraMovies && bExtraSounds && (nLevel == 1) && (nScreen < 4);

#ifndef SHAREWARE
hum_channel = StartBriefingHum (hum_channel, nLevel, nScreen, bExtraSounds);
#endif

GrSetCurFont (GAME_FONT);

briefBuf = Briefing_screens [nScreen];
bsp = &briefBuf;
if (gameStates.app.bD1Mission)
	GotZ = 1;
init_char_pos (bsp, gameStates.app.bD1Mission);

x = Briefing_text_x;
y = Briefing_text_y;
*szBriefScreen = *szBriefScreenB = '\0';
if (bOnlyRobots) {
	while (message && !PageHasRobot (message))
		message = SkipPage (message, &briefBuf, &x, &y, &nScreen);
	if (!message)
		goto done;
	}
pi = pj = message;

redrawPage:

while (!done) {
	pj = message;
	if (curDrawBuffer == GL_FRONT)
		pi = message;
	else {
		GrUpdate (0);
		//StopBriefingSound (&printing_channel);
		message = pi;
		Briefing_text_x = x;
		Briefing_text_y = y;
		//if (bHaveScreen)
		load_briefing_screen (nScreen);
		briefBuf = Briefing_screens [nScreen];
		bsp = &briefBuf;
		init_char_pos (bsp, 1);
		}

	bRedraw = 1;
	bPageDone = 0;
	new_page = 0;
	while (bRedraw) {
		ch = *message++;
		if (ch == '$') {
			ch = *message++;
			if (ch=='D') {
				nScreen=DefineBriefingBox (&message, &briefBuf);
		    	//LoadNewBriefingScreen (Briefing_screens[nScreen].bs_name);

				bsp = &briefBuf;
				x = Briefing_text_x;
				y = Briefing_text_y;
				LineAdjustment=0;
				prev_ch = 10;                                   // read to eoln
				}
			else if (ch=='U') {
				nScreen=get_message_num (&message);
				briefBuf = Briefing_screens [nScreen];
				bsp = &briefBuf;
				init_char_pos (bsp, 1);
				x = Briefing_text_x;
				y = Briefing_text_y;
				prev_ch = 10;                                   // read to eoln
				}
			else if (ch == 'C') {
				Current_color = get_message_num (&message)-1;
				Assert ((Current_color >= 0) && (Current_color < MAX_BRIEFING_COLORS));
				prev_ch = 10;
				}
			else if (ch == 'F') {     // toggle flashing cursor
				flashing_cursor = !flashing_cursor;
				prev_ch = 10;
				while (*message++ != 10)
					;
				}
			else if (ch == 'T') {
				tab_stop = get_message_num (&message);
				tab_stop*= (1+gameStates.menus.bHires);
				prev_ch = 10;							//	read to eoln
				}
			else if (ch == 'R') {
				if (message > pj) {
					if (Robot_canv != NULL) {
						d_free (Robot_canv);
						Robot_canv=NULL;
						}
					if (bRobotPlaying) {
						DeInitRobotMovie ();
						bRobotPlaying=0;
						}
					InitSpinningRobot ();
					StopBriefingSound (&hum_channel);
					bot_channel = StartExtraBotSound (bot_channel, (short) nLevel, nBot++);
					}
				if (gameStates.app.bD1Mission) {
					robot_num = get_message_num (&message);
					while (*message++ != 10)
						;
					}
				else {
					kludge=*message++;
					spinRobotName[2]=kludge; // ugly but proud
					if (message > pj) {
						grs_canvas	*curcanv_save = grdCurCanv;
						grdCurCanv = Robot_canv;
						bRobotPlaying=InitRobotMovie (spinRobotName);
						grdCurCanv = curcanv_save;
						// GrRemapBitmapGood (&grdCurCanv->cv_bitmap, pal, -1, -1);
						if (bRobotPlaying) {
							RotateBriefingRobot ();
							DoBriefingColorStuff ();
						}
					}
				}
				prev_ch = 10;                           // read to eoln
				}
			else if (ch == 'N') {
				if (message > pj) {
				//--grs_bitmap *bitmap_ptr;
					if (Robot_canv != NULL) {
						d_free (Robot_canv);
						Robot_canv=NULL;
						}
					StopBriefingSound (&bot_channel);
					get_message_name (&message, Bitmap_name);
					strcat (Bitmap_name, "#0");
					Animating_bitmap_type = 0;
					}
				prev_ch = 10;
				}
			else if (ch == 'O') {
				if (message > pj) {
					if (Robot_canv != NULL) {
						d_free (Robot_canv);
						Robot_canv=NULL;
						}
					get_message_name (&message, Bitmap_name);
					strcat (Bitmap_name, "#0");
					Animating_bitmap_type = 1;
					}
				prev_ch = 10;
				}
			else if (ch=='A') {
				LineAdjustment=1-LineAdjustment;
				}
			else if (ch=='Z') {
				GotZ=1;
#if 1 //defined (D2_OEM) || defined (COMPILATION)
				DumbAdjust=1;
#else
				DumbAdjust= (LineAdjustment==1) ? 1 : 2;
#endif
				i = 0;
				while ((szBriefScreen [i] = *message) != '\n') {
					i++;
					message++;
					}
				szBriefScreen [i] = 0;
				if (*message != 10)
					while (*message++ != 10)    //  Get and drop eoln
						;
				for (i = 0; szBriefScreen [i] != '.'; i++)
					szBriefScreenB[i] = szBriefScreen[i];
				memcpy (szBriefScreenB + i, "b.pcx", sizeof ("b.pcx"));
				i += sizeof ("b.pcx");
				if ((gameStates.menus.bHires && CFExist (szBriefScreenB,gameFolders.szDataDir,0)) || 
					 !CFExist (szBriefScreen,gameFolders.szDataDir,0))
					LoadNewBriefingScreen (szBriefScreenB, message <= pj);
				else
					LoadNewBriefingScreen (szBriefScreen, message <= pj);
				bHaveScreen = 1;

				//LoadNewBriefingScreen (gameStates.menus.bHires?"end01b.pcx":"end01.pcx");

				}
			else if (ch == 'B') {
				char        bitmap_name[32];
				grs_bitmap  guy_bitmap;
				int         iff_error;

				if (message > pj) {
					if (Robot_canv != NULL) {
						d_free (Robot_canv);
						Robot_canv=NULL;
						}
					}
				get_message_name (&message, bitmap_name);
				strcat (bitmap_name, ".bbm");
				memset (&guy_bitmap, 0, sizeof (guy_bitmap));
				iff_error = iff_read_bitmap (bitmap_name, &guy_bitmap, BM_LINEAR);
				Assert (iff_error == IFF_NO_ERROR);

				show_briefing_bitmap (&guy_bitmap);
				d_free (guy_bitmap.bm_texBuf);
				prev_ch = 10;
//			} else if (ch==EOF) {
//				done=1;
//			} else if (ch == 'B') {
//				if (Robot_canv != NULL)	{
//					d_free (Robot_canv);
//					Robot_canv=NULL;
//				}
//
//				bitmap_num = get_message_num (&message);
//				if (bitmap_num != -1)
//					show_briefing_bitmap (gameData.pig.tex.pBmIndex[bitmap_num]);
//				prev_ch = 10;                           // read to eoln
				}
			else if (ch == 'S') {
				int keypress = 0;
				fix StartTime;

				chattering=0;
				StopBriefingSound (&printing_channel);

#ifdef WINDOWS
				if (!wpage_done) {
					DDGRRESTORE;
					wpage_done =1;
				}
#endif

				StartTime = TimerGetFixedSeconds ();
				do {		//	Wait for a key
#ifdef WINDOWS
					if (_RedrawScreen) {
						_RedrawScreen = FALSE;
						if (bot_channel < 0)
							hum_channel = StartBriefingHum (hum_channel, nLevel, nScreen, bExtraSounds);
						keypress = KEY_ESC;
						break;
						}
#endif
					while (TimerGetFixedSeconds () < StartTime + KEY_DELAY_DEFAULT/2)
						;
					flash_cursor (flashing_cursor);
					if (bRobotPlaying)
						RotateBriefingRobot ();
					if (robot_num != -1)
						ShowSpinningRobotFrame (robot_num);
					if (Bitmap_name[0] != 0)
						ShowBitmapFrame (0);
					StartTime += KEY_DELAY_DEFAULT/2;
					//GrUpdate (0);
					keypress = local_key_inkey ();
					if (curDrawBuffer == GL_BACK)
						break;
					} while (!keypress);
				if (keypress) {
#ifndef NDEBUG
					if (keypress == KEY_BACKSP)
						Int3 ();
					else
#endif
						{
						if (keypress == KEY_ESC)
							rval = 1;
						flashing_cursor = 0;
						WIN (wpage_done = 0);
						goto done;
						}
					}
				else
					goto redrawPage;
				}
			else if (ch == 'P') {		//	New page.
				if (!GotZ) {
					Int3 (); // Hey ryan!!!! You gotta load a screen before you start
					        // printing to it! You know, $Z !!!
					//if (message > pj)
		  			LoadNewBriefingScreen (gameStates.menus.bHires?"end01b.pcx":"end01.pcx", message <= pj);
					bHaveScreen = 1;
					}
				new_page = 1;
				while (*message != 10)
					message++;	//	drop carriage return after special escape sequence
				message++;
				prev_ch = 10;
				//GrUpdate (0);
				}
			}
		else if (ch == '\t') {		//	Tab
			if (Briefing_text_x - bsp->text_ulx < tab_stop)
				Briefing_text_x = bsp->text_ulx + tab_stop;
			}
		else if ((ch == ';') && (prev_ch == 10)) {
			while (*message++ != 10)
				;
			prev_ch = 10;
			}
		else if (ch == '\\') {
			prev_ch = ch;
			}
		else if (ch == 10) {
			if (prev_ch != '\\') {
				prev_ch = ch;
				if (DumbAdjust==0)
					Briefing_text_y += (8* (gameStates.menus.bHires+1));
				else
					DumbAdjust--;
				Briefing_text_x = bsp->text_ulx;
				if (Briefing_text_y > bsp->text_uly + bsp->text_height) {
					load_briefing_screen (nScreen);
					Briefing_text_x = bsp->text_ulx;
					Briefing_text_y = bsp->text_uly;
				}
			} else {
				if (*message == 13)		//Can this happen? Above says ch==10
					message++;
				prev_ch = ch;
			}

			}
		else {
			if (!GotZ) {
				Int3 (); // Hey ryan!!!! You gotta load a screen before you start
				        // printing to it! You know, $Z !!!
				//if (message > pj)
					LoadNewBriefingScreen (gameStates.menus.bHires?"end01b.pcx":"end01.pcx", message <= pj);
				}
			prev_ch = ch;
			WIN (if (GRMODEINFO (emul)) delay_count = 0);
			bRedraw = !delay_count || (message <= pj);
			if (!bRedraw) {
		 		printing_channel = StartBriefingSound (printing_channel, SOUND_BRIEFING_PRINTING, F1_0, NULL);
				chattering=1;
			}
			Briefing_text_x += show_char_delay ((char) ch, delay_count, robot_num, flashing_cursor, bRedraw);
//			if (! (bRedraw || bRobotPlaying || *Bitmap_name))
//				GrUpdate (0);
		}

		//	Check for Esc -> abort.
		if (!bRedraw) {
			if (delay_count)
				key_check=local_key_inkey ();
			else
				key_check=0;
#ifdef WINDOWS
			if (_RedrawScreen) {
				_RedrawScreen = FALSE;
				if (bot_channel < 0)
					hum_channel = StartBriefingHum (hum_channel, nLevel, nScreen, bExtraSounds);
				key_check = KEY_ESC;
				}
#endif
			if (key_check == KEY_ESC) {
				rval = 1;
				done = 1;
				goto done;
				}
			if ((key_check == KEY_SPACEBAR) || (key_check == KEY_ENTER)) {
				StopBriefingSound (&bot_channel);
				delay_count = 0;
				bRedraw = 1;
				}
#ifdef GR_SUPPORTS_FULLSCREEN_TOGGLE
			if ((key_check == KEY_ALTED+KEY_ENTER) ||
				 (key_check == KEY_ALTED+KEY_PADENTER))
				GrToggleFullScreen ();
#endif
			}
		if (Briefing_text_x > bsp->text_ulx + bsp->text_width) {
			Briefing_text_x = bsp->text_ulx;
			Briefing_text_y += bsp->text_uly;
		}

		if (new_page || (Briefing_text_y > bsp->text_uly + bsp->text_height)) {
			fix	StartTime = 0;
			int	keypress = 0;

			new_page = 0;
			StopBriefingSound (&printing_channel);
			chattering=0;
#ifdef WINDOWS
			if (!wpage_done) {
				DDGRRESTORE;
				wpage_done =1;
			}
#endif
			{
				StartTime = TimerGetFixedSeconds ();
				do {		//	Wait for a key
#ifdef WINDOWS
					if (_RedrawScreen) {
						_RedrawScreen = FALSE;
						if (bot_channel < 0)
							hum_channel = StartBriefingHum (hum_channel, nLevel, nScreen, bExtraSounds);
						keypress = KEY_ESC;
						break;
						}
#endif
					while (TimerGetFixedSeconds () < StartTime + KEY_DELAY_DEFAULT/2)
						;
					flash_cursor (flashing_cursor);
					if (bRobotPlaying)
						RotateBriefingRobot ();
					if (robot_num != -1)
						ShowSpinningRobotFrame (robot_num);
					if (Bitmap_name[0] != 0)
						ShowBitmapFrame (0);
					StartTime += KEY_DELAY_DEFAULT/2;
					keypress = local_key_inkey ();
					if (curDrawBuffer == GL_BACK)
						break;
 					GrUpdate (0);
				} while (!keypress);
				if (keypress) {
					if (bRobotPlaying)
						DeInitRobotMovie ();
					bRobotPlaying=0;
					robot_num = -1;
#ifndef NDEBUG
					if (keypress == KEY_BACKSP)
						Int3 ();
#endif
					if (keypress == KEY_ESC) {
						rval = 1;
						goto done;
						}
					else {
						if (bOnlyRobots) {
							*szBriefScreen = *szBriefScreenB = '\0';
							while (message && !PageHasRobot (message))
								message = SkipPage (message, &briefBuf, &x, &y, &nScreen);
							if (!message)
								goto done;
							}
 						pi = message;
						if (curDrawBuffer == GL_FRONT) {
							load_briefing_screen (nScreen);
							GrUpdate (0);
							}
						Briefing_text_x = bsp->text_ulx;
						Briefing_text_y = bsp->text_uly;
						delay_count = KEY_DELAY_DEFAULT;
						WIN (wpage_done = 0);
						goto redrawPage;
						}
					}
				else
					goto redrawPage;
				}
		}
	}
}

done:

if (bRobotPlaying) {
	DeInitRobotMovie ();
	bRobotPlaying=0;
	}

if (Robot_canv != NULL)
	{d_free (Robot_canv); Robot_canv=NULL;}

StopBriefingSound (&hum_channel);
StopBriefingSound (&printing_channel);
StopBriefingSound (&bot_channel);
return rval;
}

//-----------------------------------------------------------------------------
// Return a pointer to the start of text for screen #nScreen.
char * get_briefing_message (int nScreen)
{
	char *tptr = Briefing_text;
	int	cur_screen=0;
	int	ch, i;

Assert (nScreen >= 0);

for (i = 0; *tptr && (i < briefingTextLen); i++) {
	ch = *tptr++;
	if (ch != '$')
		continue;
	ch = *tptr++;
	if (ch != 'S')
		continue;
	cur_screen = get_message_num (&tptr);
	if (cur_screen == nScreen)
		return tptr;
	}
return (NULL);
}

//-----------------------------------------------------------------------------
//	Load Descent briefing text.
int load_screen_text (char *filename, char **buf)
{
	CFILE *tfile;
	CFILE *ifile;
	int	len, i;
	int	have_binary = 0;
	char	*bufP;

if ((tfile = CFOpen (filename, gameFolders.szDataDir, "rb", gameStates.app.bD1Mission)) == NULL) {
	char nfilename[30], *ptr;

	strcpy (nfilename, filename);
	if ((ptr = strrchr (nfilename, '.')))
		*ptr = '\0';
	strcat (nfilename, ".txb");
	if ((ifile = CFOpen (nfilename, gameFolders.szDataDir, "rb", gameStates.app.bD1Mission)) == NULL) {
#if TRACE
		con_printf (CON_DEBUG,"can't open %s!\n",nfilename);
#endif
		return (0);
		}
	have_binary = 1;
	len = CFLength (ifile,0);
	MALLOC (bufP, char, len);
	*buf = bufP;
	for (i=0; i < len; i++,bufP++) {
		CFRead (bufP, 1, 1, ifile);
			if (*bufP == 13)
				bufP--;
		}
	CFClose (ifile);
	}
else {
	len = CFLength (tfile,0);
	MALLOC (bufP, char, len+500);
	*buf = bufP;
	for (i=0; i < len; i++, bufP++) {
		CFRead (bufP, 1, 1, tfile);
			if (*bufP == 13)
				bufP--;
		}
	*bufP = 0;
	//CFRead (*buf, 1, len, tfile);
	CFClose (tfile);
	}
if (have_binary) {
	for (i = 0, bufP = *buf; i < len; i++, bufP++)
		if (*bufP != '\n')
			*bufP = EncodeRotateLeft ((char) (EncodeRotateLeft (*bufP) ^ BITMAP_TBL_XOR));
	//*bufP = '\0';
	}
briefingTextLen = (int) (bufP - *buf);
return (1);
}

//-----------------------------------------------------------------------------
// Return true if message got aborted, else return false.
int ShowBriefingText (int nScreen, short nLevel)
{
	char	*message_ptr;
	int i;

message_ptr = (gameStates.app.bD1Mission) ?
				  get_briefing_message (Briefing_screens[nScreen].message_num) :
				  get_briefing_message (nScreen);
if (!message_ptr)
	return (0);
DoBriefingColorStuff ();
glEnable (GL_TEXTURE_2D);
i = ShowBriefingMessage (nScreen, message_ptr, nLevel);
glDisable (GL_TEXTURE_2D);
return i;
}

//-----------------------------------------------------------------------------

void DoBriefingColorStuff ()
{
GrPaletteStepLoad (NULL);

if (gameStates.app.bD1Mission) {
  //green
	briefFgColorIndex[0] = GrFindClosestColorCurrent (0, 54, 0);
	briefBgColorIndex[0] = GrFindClosestColorCurrent (0, 19, 0);
  //white
	briefFgColorIndex[1] = GrFindClosestColorCurrent (42, 38, 32);
	briefBgColorIndex[1] = GrFindClosestColorCurrent (14, 14, 14);
	//Begin D1X addition
	//red
	briefFgColorIndex[2] = GrFindClosestColorCurrent (63, 0, 0);
	briefBgColorIndex[2] = GrFindClosestColorCurrent (31, 0, 0);
	}
else {
	briefFgColorIndex[0] = GrFindClosestColorCurrent (0, 40, 0);
	briefBgColorIndex[0] = GrFindClosestColorCurrent (0, 6, 0);
	briefFgColorIndex[1] = GrFindClosestColorCurrent (40, 33, 35);
	briefBgColorIndex[1] = GrFindClosestColorCurrent (5, 5, 5);
	briefFgColorIndex[2] = GrFindClosestColorCurrent (8, 31, 54);
	briefBgColorIndex[2] = GrFindClosestColorCurrent (1, 4, 7);
	}
//blue
briefFgColorIndex[3] = GrFindClosestColorCurrent (0, 0, 54);
briefBgColorIndex[3] = GrFindClosestColorCurrent (0, 0, 19);
//gray
briefFgColorIndex[4] = GrFindClosestColorCurrent (14, 14, 14);
briefBgColorIndex[4] = GrFindClosestColorCurrent (0, 0, 0);
//yellow
briefFgColorIndex[5] = GrFindClosestColorCurrent (54, 54, 0);
briefBgColorIndex[5] = GrFindClosestColorCurrent (19, 19, 0);
//purple
briefFgColorIndex[6] = GrFindClosestColorCurrent (0, 54, 54);
briefBgColorIndex[6] = GrFindClosestColorCurrent (0, 19, 19);
//End D1X addition
}

//-----------------------------------------------------------------------------
// Return true if screen got aborted by user, else return false.
int ShowBriefingScreen (int nScreen, int allow_keys, short nLevel)
{
brief_palette_254_bash = 0;
if (gameOpts->gameplay.bSkipBriefingScreens) {
	con_printf (CON_DEBUG, "Skipping briefing screen [%s]\n", &Briefing_screens[nScreen].bs_name);
	return 0;
	}
if (gameStates.app.bD1Mission) {
	int pcx_error;
#if 1
	grs_bitmap bmBriefing;

	GrInitBitmapData (&bmBriefing);
	if ((pcx_error=LoadBriefImg (Briefing_screens[nScreen].bs_name, &bmBriefing, 0))!=PCX_ERROR_NONE) {
#else
	if ((pcx_error=pcx_read_fullscr (Briefing_screens[nScreen].bs_name, 1))!=PCX_ERROR_NONE) {
#endif
		con_printf (CON_DEBUG, "File '%s', PCX load error: %s (%i)\n  (It's a briefing screen.  Does this cause you pain?)\n", Briefing_screens[nScreen].bs_name, pcx_errormsg (pcx_error), pcx_error);
		Int3 ();
		return 0;
		}
#if 1
#ifdef OGL
	GrPaletteStepLoad (bmBriefing.bm_palette);
#else
	GrPaletteStepClear ();
#endif
	GrSetCurrentCanvas (NULL);
	show_fullscr (&bmBriefing);
#endif
	GrUpdate (0);
#if 1
	GrFreeBitmapData (&bmBriefing);
#endif
	if (GrPaletteFadeIn (NULL, 32, allow_keys))
		return 1;
	}
if (!ShowBriefingText (nScreen, nLevel))
	return 0;
#ifndef WINDOWS
if (GrPaletteFadeOut (NULL, 32, allow_keys))
	return 1;
#else
DEFINE_SCREEN (NULL);
WIN (DDGRLOCK (dd_grd_curcanv);
GrClearCanvas (0);
WIN (DDGRUNLOCK (dd_grd_curcanv);
if (GrPaletteFadeOut (NULL, 32, allow_keys))
	return 1;
#endif
return 1;
}

//-----------------------------------------------------------------------------

void DoBriefingScreens (char *filename, int level_num)
{
	int	abort_briefing_screens = 0;
	int	cur_briefing_screen = 0;
	int	bEnding = (strstr (filename, "endreg") != NULL);

if (gameOpts->gameplay.bSkipBriefingScreens) {
	con_printf (CON_DEBUG, "Skipping all briefing screens.\n");
	return;
	}
con_printf (CON_DEBUG,"Trying briefing screen <%s>\n",filename);
LogErr ("Looking for briefing screen '%s'\n", filename);
if (!filename)
	return;
if (gameStates.app.bD1Mission && (gameData.missions.nCurrentMission != gameData.missions.nD1BuiltinMission)) {
	FILE	*fp;
	int	i;
	char	fn [FILENAME_LEN], *psz;
	for (i = 0; i < 2; i++) {
		if (psz = strchr (fn, '.'))
			*psz = '\0';
		strcat (fn, i ? ".txb" : ".tex");
		if	 (fp = CFFindHogFile (&gameHogFiles.AltHogFiles, "", fn, NULL)) {
			fclose (fp);
			break;
			}
		}
	if (i == 2)
		return;
	}
if (!load_screen_text (filename, &Briefing_text))
	return;
#ifndef SHAREWARE
songs_play_song (SONG_BRIEFING, 1);
#else
SongsStopAll ();
#endif
SetScreenMode (SCREEN_MENU);
WINDOS (
	DDGrSetCurrentCanvas (NULL),
	GrSetCurrentCanvas (NULL)
	);
con_printf (CON_DEBUG,"Playing briefing screen <%s>, level %d\n",filename,level_num);
KeyFlush ();
if (gameStates.app.bD1Mission) {
	gamePalette = LoadPalette (NULL,NULL,1,1,1);
	LoadD1BitmapReplacements ();
	if (level_num == 1) {
		while ((!abort_briefing_screens) && (Briefing_screens[cur_briefing_screen].level_num == ((gameStates.app.bD1Mission && bEnding) ? level_num : 0))) {
			abort_briefing_screens = ShowBriefingScreen (cur_briefing_screen, 0, (short) level_num);
			cur_briefing_screen++;
			if (gameStates.app.bD1Mission && bEnding)
				break;
			}
		}
	if (!abort_briefing_screens) {
		for (cur_briefing_screen = 0; cur_briefing_screen < MAX_BRIEFING_SCREENS; cur_briefing_screen++)
			if (Briefing_screens[cur_briefing_screen].level_num == level_num)
				if (ShowBriefingScreen (cur_briefing_screen, 0, (short) level_num))
					break;
		}
	}
else
	ShowBriefingScreen (level_num, 0, (short) level_num);
d_free (Briefing_text);
Briefing_text = NULL;
KeyFlush ();
return;
}

//-----------------------------------------------------------------------------

int DefineBriefingBox (char **buf, briefing_screen *pBriefBuf)
{
	int n,i=0;
	char name[20];

	n=get_new_message_num (buf);

	Assert (n < MAX_BRIEFING_SCREENS);

	while (**buf!=' ') {
		name[i++]=**buf;
		 (*buf)++;
		}
	name[i]='\0';   // slap a delimiter on this guy
	*pBriefBuf = Briefing_screens [n];
	strcpy (pBriefBuf->bs_name,name);
	pBriefBuf->level_num=get_new_message_num (buf);
	pBriefBuf->message_num=get_new_message_num (buf);
	pBriefBuf->text_ulx=get_new_message_num (buf);
	pBriefBuf->text_uly=get_new_message_num (buf);
	pBriefBuf->text_width=get_new_message_num (buf);
	pBriefBuf->text_height=get_message_num (buf);  // NOTICE!!!
	init_char_pos (pBriefBuf, 1);
	return (n);
}

//-----------------------------------------------------------------------------

int get_new_message_num (char **message)
{
	int	num=0;

while (**message == ' ')
	 (*message)++;
while ((**message >= '0') && (**message <= '9')) {
	num = 10*num + **message-'0';
	 (*message)++;
	}
 (*message)++;
return num;
}

//-----------------------------------------------------------------------------

