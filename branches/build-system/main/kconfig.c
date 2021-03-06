/* $Id: KConfig.c,v 1.27 2003/12/18 11:24:04 btb Exp $ */
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
COPYRIGHT 1993-1999 PARALLAX SOFTWARE EVE.  ALL RIGHTS RESERVED.
*/

/*
 *
 * Routines to configure keyboard, joystick, etc..
 *
 * Old Log:
 * Revision 1.18  1995/10/29  20:14:10  allender
 * don't read mouse 30x/sec.  Still causes problems -- left with
 * exposure at > 60 frame/s
 *
 * Revision 1.17  1995/10/27  14:16:35  allender
 * don't set lastreadtime when doing mouse stuff if we didn't
 * read mouse this frame
 *
 * Revision 1.16  1995/10/24  18:10:22  allender
 * get mouse stuff working right this time?
 *
 * Revision 1.15  1995/10/23  14:50:50  allender
 * corrected values for control type in KCSetControls
 *
 * Revision 1.14  1995/10/21  16:36:54  allender
 * fix up mouse read time
 *
 * Revision 1.13  1995/10/20  00:46:53  allender
 * fix up mouse reading problem
 *
 * Revision 1.12  1995/10/19  13:36:38  allender
 * mouse support in KConfig screens
 *
 * Revision 1.11  1995/10/18  21:06:06  allender
 * removed Int3 in cruise stuff -- was in there for debugging and
 * now not needed
 *
 * Revision 1.10  1995/10/17  13:12:47  allender
 * fixed config menus so buttons don't get configured
 *
 * Revision 1.9  1995/10/15  23:07:55  allender
 * added return key as second button for primary fire
 *
 * Revision 1.8  1995/09/05  08:49:47  allender
 * change 'PADRTN' label to 'ENTER'
 *
 * Revision 1.7  1995/09/01  15:38:22  allender
 * took out cap of reading controls max 25 times/sec
 *
 * Revision 1.6  1995/09/01  13:33:59  allender
 * erase all old text
 *
 * Revision 1.5  1995/08/18  10:20:55  allender
 * keep controls reading to 25 times/s max so fast
 * frame rates don't mess up control reading
 *
 * Revision 1.4  1995/07/28  15:43:13  allender
 * make mousebutton control primary fire
 *
 * Revision 1.3  1995/07/26  17:04:32  allender
 * new defaults and make joystick main button work correctly
 *
 * Revision 1.2  1995/07/17  08:51:03  allender
 * fixed up configuration menus to look right
 *
 * Revision 1.1  1995/05/16  15:26:56  allender
 * Initial revision
 *
 * Revision 2.11  1995/08/23  16:08:04  john
 * Added version 2 of external controls that passes the ship
 * position and orientation the drivers.
 *
 * Revision 2.10  1995/07/07  16:48:01  john
 * Fixed bug with new interface.
 *
 * Revision 2.9  1995/07/03  15:02:32  john
 * Added new version of external controls for Cybermouse absolute position.
 *
 * Revision 2.8  1995/06/30  12:30:28  john
 * Added -Xname command line.
 *
 * Revision 2.7  1995/03/30  16:36:56  mike
 * text localization.
 *
 * Revision 2.6  1995/03/21  14:39:31  john
 * Ifdef'd out the NETWORK code.
 *
 * Revision 2.5  1995/03/16  10:53:07  john
 * Move VFX center to Shift+Z instead of Enter because
 * it conflicted with toggling HUD on/off.
 *
 * Revision 2.4  1995/03/10  13:47:24  john
 * Added head tracking sensitivity.
 *
 * Revision 2.3  1995/03/09  18:07:06  john
 * Fixed bug with iglasses tracking not "centering" right.
 * Made VFX have bright headlight lighting.
 *
 * Revision 2.2  1995/03/08  15:32:39  john
 * Made VictorMaxx head tracking use Greenleaf code.
 *
 * Revision 2.1  1995/03/06  15:23:31  john
 * New screen techniques.
 *
 * Revision 2.0  1995/02/27  11:29:26  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.105  1995/02/22  14:11:58  allender
 * remove anonymous unions from object structure
 *
 * Revision 1.104  1995/02/13  12:01:56  john
 * Fixed bug with buggin not mmaking player faster.
 *
 * Revision 1.103  1995/02/09  22:00:46  john
 * Added i-glasses tracking.
 *
 * Revision 1.102  1995/01/24  21:25:47  john
 * Fixed bug with slide/bank on not working with
 * Cyberman heading.,
 *
 * Revision 1.101  1995/01/24  16:09:56  john
 * Fixed bug with Wingman extreme customize text overwriting title.
 *
 * Revision 1.100  1995/01/24  12:37:46  john
 * Made Esc exit key define menu.
 *
 * Revision 1.99  1995/01/23  23:54:43  matt
 * Made keypad enter work
 *
 * Revision 1.98  1995/01/23  16:42:00  john
 * Made the external controls always turn banking off, leveling off
 * and passed automap state thru to the tsr.
 *
 * Revision 1.97  1995/01/12  11:41:33  john
 * Added external control reading.
 *
 * Revision 1.96  1995/01/05  10:43:58  mike
 * Handle case when TimerGetFixedSeconds () goes negative.  Happens at 9.1
 * hours.  Previously, joystick would stop functioning.  Now will work.
 *
 * Revision 1.95  1994/12/29  11:17:38  john
 * Took out some warnings and con_printf.
 *
 * Revision 1.94  1994/12/29  11:07:41  john
 * Fixed Thrustmaster and Logitech Wingman extreme
 * Hat by reading the y2 axis during the center stage
 * of the calibration, and using 75, 50, 27, and 3 %
 * as values for the 4 positions.
 *
 * Revision 1.93  1994/12/27  12:16:20  john
 * Fixed bug with slide on not working with joystick or mouse buttons.
 *
 * Revision 1.92  1994/12/20  10:34:15  john
 * Made sensitivity work for mouse & joystick and made
 * it only affect, pitch, heading, and roll.
 *
 * Revision 1.91  1994/12/16  00:11:23  matt
 * Made delete key act normally when debug out
 *
 * Revision 1.90  1994/12/14  17:41:15  john
 * Added more buttons so that  Yoke would work.
 *
 * Revision 1.89  1994/12/13  17:25:35  allender
 * Added Assert for bogus time for joystick reading.
 *
 * Revision 1.88  1994/12/13  14:48:01  john
 * Took out some debugging con_printf's
 *
 *
 * Revision 1.87  1994/12/13  14:43:02  john
 * Took out the code in KConfig to build direction array.
 * Called KCSetControls after selecting a new control type.
 *
 * Revision 1.86  1994/12/13  01:11:32  john
 * Fixed bug with message clearing overwriting
 * right border.
 *
 * Revision 1.85  1994/12/12  00:35:58  john
 * Added or thing for keys.
 *
 * Revision 1.84  1994/12/09  17:08:06  john
 * Made mouse a bit less sensitive.
 *
 * Revision 1.83  1994/12/09  16:04:00  john
 * Increased mouse sensitivity.
 *
 * Revision 1.82  1994/12/09  00:41:26  mike
 * fix hang in automap print screen
 *
 * Revision 1.81  1994/12/08  11:50:37  john
 * Made strcpy only copy corect number of chars,.
 *
 * Revision 1.80  1994/12/07  16:16:06  john
 * Added command to check to see if a joystick axes has been used.
 *
 * Revision 1.79  1994/12/07  14:52:28  yuan
 * Localization 492
 *
 * Revision 1.78  1994/12/07  13:37:40  john
 * Made the joystick thrust work in reverse.
 *
 * Revision 1.77  1994/12/07  11:28:24  matt
 * Did a little localization support
 *
 * Revision 1.76  1994/12/04  12:30:03  john
 * Made the Thrustmaster stick read every frame, not every 10 frames,
 * because it uses analog axis as buttons.
 *
 * Revision 1.75  1994/12/03  22:35:25  yuan
 * Localization 412
 *
 * Revision 1.74  1994/12/03  15:39:24  john
 * Made numeric keypad move in conifg.
 *
 * Revision 1.73  1994/12/01  16:23:39  john
 * Fixed include mistake.
 *
 * Revision 1.72  1994/12/01  16:07:57  john
 * Fixed bug that disabled joystick in automap because it used gametime, which is
 * paused during automap. Fixed be used timer_Get_fixed_seconds instead of gameData.time.xGame.
 *
 * Revision 1.71  1994/12/01  12:30:49  john
 * Made Ctrl+D delete, not Ctrl+E
 *
 * Revision 1.70  1994/12/01  11:52:52  john
 * Added default values for GamePad.
 *
 * Revision 1.69  1994/11/30  00:59:12  mike
 * optimizations.
 *
 * Revision 1.68  1994/11/29  03:45:50  john
 * Added joystick sensitivity; Added sound channels to detail menu.  Removed -maxchannels
 * command line arg.
 *
 * Revision 1.67  1994/11/27  23:13:44  matt
 * Made changes for new con_printf calling convention
 *
 * Revision 1.66  1994/11/27  19:52:12  matt
 * Made screen shots work in a few more places
 *
 * Revision 1.65  1994/11/22  16:54:50  mike
 * autorepeat on missiles.
 *
 * Revision 1.64  1994/11/21  11:16:17  rob
 * Changed calls to GameLoop to calls to MultiMenuPoll and changed
 * conditions under which they are called.
 *
 * Revision 1.63  1994/11/19  15:14:48  mike
 * remove unused code and data
 *
 * Revision 1.62  1994/11/18  23:37:56  john
 * Changed some shorts to ints.
 *
 * Revision 1.61  1994/11/17  13:36:35  rob
 * Added better network hook in KConfig menu.
 *
 * Revision 1.60  1994/11/14  20:09:13  john
 * Made Tab be default for automap.
 *
 * Revision 1.59  1994/11/13  16:34:07  matt
 * Fixed victormaxx angle conversions
 *
 * Revision 1.58  1994/11/12  14:47:05  john
 * Added support for victor head tracking.
 *
 * Revision 1.57  1994/11/08  15:14:55  john
 * Added more calls so net doesn't die in net game.
 *
 * Revision 1.56  1994/11/07  14:01:07  john
 * Changed the gamma correction sequencing.
 *
 * Revision 1.55  1994/11/01  16:40:08  john
 * Added Gamma correction.
 *
 * Revision 1.54  1994/10/25  23:09:26  john
 * Made the automap key configurable.
 *
 * Revision 1.53  1994/10/25  13:11:59  john
 * Made keys the way Adam speced 'em for final game.
 *
 * Revision 1.52  1994/10/24  17:44:22  john
 * Added stereo channel reversing.
 *
 * Revision 1.51  1994/10/22  13:23:18  john
 * Made default Rear View key be R.
 *
 * Revision 1.50  1994/10/22  13:20:09  john
 * Took out toggle primary/secondary weapons.  Fixed black
 * background for 'axes' and 'buttons' text.
 *
 * Revision 1.49  1994/10/21  15:20:15  john
 * Made PrtScr do screen dump, not F2.
 *
 * Revision 1.48  1994/10/21  13:41:36  john
 * Allowed F2 to screen dump.
 *
 * Revision 1.47  1994/10/17  13:07:05  john
 * Moved the descent.cfg info into the player config file.
 *
 * Revision 1.46  1994/10/14  15:30:22  john
 * Added Cyberman default positions.
 *
 * Revision 1.45  1994/10/14  15:24:54  john
 * Made Cyberman work with config.
 *
 * Revision 1.44  1994/10/14  12:46:04  john
 * Added the ability to reset all to default.
 *
 * Revision 1.43  1994/10/14  12:18:31  john
 * Made mouse invert axis always be 0 or 1.
 *
 * Revision 1.42  1994/10/14  12:16:03  john
 * Changed code so that by doing DEL+F12 saves the current KConfig
 * values as default. Added support for drop_bomb key.  Took out
 * unused slots for keyboard.  Made keyboard use control_type of 0
 * save slots.
 *
 * Revision 1.41  1994/10/13  21:27:02  john
 * Made axis invert value always be 0 or 1.
 *
 * Revision 1.40  1994/10/13  20:18:15  john
 * Added some more system keys, such as F? and CAPSLOCK.
 *
 * Revision 1.39  1994/10/13  19:22:29  john
 * Added separate config saves for different devices.
 * Made all the devices work together better, such as mice won't
 * get read when you're playing with the joystick.
 *
 * Revision 1.38  1994/10/13  15:41:57  mike
 * Remove afterburner.
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid [] = "$Id: KConfig.c,v 1.27 2003/12/18 11:24:04 btb Exp $";
#endif

#ifdef WINDOWS
#include "desw.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "pa_enabl.h"                   //$$POLY_ACC
#include "error.h"
#include "inferno.h"
#include "gr.h"
#include "mono.h"
#include "key.h"
#include "palette.h"
#include "game.h"
#include "gamefont.h"
#include "iff.h"
#include "u_mem.h"
#include "event.h"
#include "joy.h"
#include "mouse.h"
#include "kconfig.h"
#include "gauges.h"
#include "joydefs.h"
#include "songs.h"
#include "render.h"
#include "digi.h"
#include "newmenu.h"
#include "endlevel.h"
#include "multi.h"
#include "timer.h"
#include "text.h"
#include "player.h"
#include "menu.h"
#include "automap.h"
#include "args.h"
#include "lighting.h"
#include "ai.h"
#include "cntrlcen.h"
#include "network.h"
#include "hudmsg.h"
#include "ogl_init.h"
#include "object.h"
#include "inferno.h"
#include "input.h"
#if defined (TACTILE)
 #include "tactile.h"
#endif

#if defined (POLY_ACC)
#include "poly_acc.h"
#endif

#include "collide.h"

#ifdef USE_LINUX_JOY
#include "joystick.h"
#endif

#ifdef D2X_KEYS
//added/removed by Victor Rachels for adding rebindable keys for these
// KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0
ubyte system_keys [] = { (ubyte) KEY_ESC, (ubyte) KEY_F1, (ubyte) KEY_F2, (ubyte) KEY_F3, (ubyte) KEY_F4, (ubyte) KEY_F5, (ubyte) KEY_F6, (ubyte) KEY_F7, (ubyte) KEY_F8, (ubyte) KEY_F9, (ubyte) KEY_F10, (ubyte) KEY_F11, (ubyte) KEY_F12, (ubyte) KEY_MINUS, (ubyte) KEY_EQUAL, (ubyte) KEY_ALTED+KEY_F9 };
#else
ubyte system_keys [] = { (ubyte) KEY_ESC, (ubyte) KEY_F1, (ubyte) KEY_F2, (ubyte) KEY_F3, (ubyte) KEY_F4, (ubyte) KEY_F5, (ubyte) KEY_F6, (ubyte) KEY_F7, (ubyte) KEY_F8, (ubyte) KEY_F9, (ubyte) KEY_F10, (ubyte) KEY_F11, (ubyte) KEY_F12, (ubyte) KEY_0, (ubyte) KEY_1, (ubyte) KEY_2, (ubyte) KEY_3, (ubyte) KEY_4, (ubyte) KEY_5, (ubyte) KEY_6, (ubyte) KEY_7, (ubyte) KEY_8, (ubyte) KEY_9, (ubyte) KEY_0, (ubyte) KEY_MINUS, (ubyte) KEY_EQUAL, (ubyte) KEY_ALTED+KEY_F9 };
#endif

#define TABLE_CREATION 1

// Array used to 'blink' the cursor while waiting for a keypress.
sbyte fades [64] = { 1,1,1,2,2,3,4,4,5,6,8,9,10,12,13,15,16,17,19,20,22,23,24,26,27,28,28,29,30,30,31,31,31,31,31,30,30,29,28,28,27,26,24,23,22,20,19,17,16,15,13,12,10,9,8,6,5,4,4,3,2,2,1,1 };

//char * invert_text [2] = { "N", "Y" };
//char * joybutton_text [28] = { "TRIG", "BTN 1", "BTN 2", "BTN 3", "BTN 4", "", "LEFT", "HAT ", "RIGHT", "", "", "HAT ", "MID", "", "", "HAT ", "", "", "", "HAT ", "TRIG", "LEFT", "RIGHT", "", "UP","DOWN","LEFT", "RIGHT" };
//char * JOYAXIS_TEXT [4] = { "X1", "Y1", "X2", "Y2" };
//char * mouseaxis_text [2] = { "L/R", "F/B" };
//char * mousebutton_text [3] = { "Left", "Right", "Mid" };

int invert_text [2] = { TNUM_N, TNUM_Y };

#ifndef USE_LINUX_JOY
#ifdef WINDOWS
	int joybutton_text [28] = 
	{ TNUM_BTN_1, TNUM_BTN_2, TNUM_BTN_3, TNUM_BTN_4,
	  -1, -1, -1, -1,
	  -1, -1, -1, -1,
	  -1, -1, -1, -1,
	  TNUM_HAT_L, TNUM_HAT_R, TNUM_HAT_U, TNUM_HAT_D,
	  -1, -1, -1, -1,
	  -1, -1, -1, -1
	};
	int joyaxis_text [MAX_AXES_PER_JOYSTICK] = { TNUM_X1, TNUM_Y1, TNUM_Z1, TNUM_R1, TNUM_P1, TNUM_R1, TNUM_YA1 };
#else
	int joybutton_text [28] = 
	{ TNUM_BTN_1, TNUM_BTN_2, TNUM_BTN_3, TNUM_BTN_4,
	  -1, TNUM_TRIG, TNUM_LEFT, TNUM_HAT_L,
	 TNUM_RIGHT, -1, TNUM_HAT2_D, TNUM_HAT_R,
	 TNUM_MID, -1, TNUM_HAT2_R, TNUM_HAT_U,
	 TNUM_HAT2_L, -1, TNUM_HAT2_U, TNUM_HAT_D,
	 TNUM_TRIG, TNUM_LEFT, TNUM_RIGHT, -1, 
	 TNUM_UP, TNUM_DOWN, TNUM_LEFT, TNUM_RIGHT };

	int joyaxis_text [7] = { TNUM_X1, TNUM_Y1, TNUM_Z1, TNUM_R1, TNUM_P1,TNUM_R1,TNUM_YA1 };
//	int JOYAXIS_TEXT [4] = { TNUM_X1, TNUM_Y1, TNUM_X2, TNUM_Y2 };
#endif
#endif

#define JOYAXIS_TEXT (v)		joyaxis_text [ (v) % MAX_AXES_PER_JOYSTICK]
#define JOYBUTTON_TEXT (v)	joybutton_text [ (v) % MAX_BUTTONS_PER_JOYSTICK]

int mouseaxis_text [3] = { TNUM_L_R, TNUM_F_B, TNUM_Z1 };
int mousebutton_text [3] = { TNUM_LEFT, TNUM_RIGHT, TNUM_MID };
char * mousebutton_textra [13] = { "MW UP", "MW DN", "M6", "M7", "M8", "M9", "M10","M11","M12","M13","M14","M15","M16" };//text for buttons above 3. -MPM

#if !defined OGL && !defined SDL_INPUT
char * key_text [256] = {         \
"","ESC","1","2","3","4","5","6","7","8","9","0","-", 			\
"=","BSPC","TAB","Q","W","E","R","T","Y","U","I","O",				\
"P"," [","]","","LCTRL","A","S","D","F",        \
"G","H","J","K","L",";","'","`",        \
"LSHFT","\\","Z","X","C","V","B","N","M",",",      \
".","/","RSHFT","PAD*","LALT","SPC",      \
"CPSLK","F1","F2","F3","F4","F5","F6","F7","F8","F9",        \
"F10","NMLCK","SCLK","PAD7","PAD8","PAD9","PAD-",   \
"PAD4","PAD5","PAD6","PAD+","PAD1","PAD2","PAD3","PAD0", \
"PAD.","","","","F11","F12","","","","","","","","","",         \
"","","","","","","","","","","","","","","","","","","","",     \
"","","","","","","","","","","","","","","","","","","","",     \
"","","","","","","","","","","","","","","","","","",           \
"PAD","RCTRL","","","","","","","","","","","","","", \
"","","","","","","","","","","PAD/","","","RALT","",      \
"","","","","","","","","","","","","","HOME","","PGUP",     \
"","","","","","END","","PGDN","INS",       \
"DEL","","","","","","","","","","","","","","","","","",     \
"","","","","","","","","","","","","","","","","","","","",     \
"","","","","","","" };
#endif /* OGL */

// macros for drawing lo/hi res KConfig screens (see scores.c as well)

#define LHX(x)      (gameStates.menus.bHires?2* (x):x)
#define LHY(y)      (gameStates.menus.bHires? (24* (y))/10:y)

char *btype_text [] = { "BT_KEY", "BT_MOUSE_BUTTON", "BT_MOUSE_AXIS", "BT_JOY_BUTTON", "BT_JOY_AXIS", "BT_INVERT" };

#define INFO_Y 28

int Num_items=28;
kc_item *All_items;

//----------- WARNING!!!!!!! -------------------------------------------
// THESE NEXT FOUR BLOCKS OF DATA ARE GENERATED BY PRESSING DEL+F12 WHEN
// IN THE KEYBOARD CONFIG SCREEN.  BASICALLY, THAT PROCEDURE MODIFIES THE
// U,D,L,R FIELDS OF THE ARRAYS AND DUMPS THE NEW ARRAYS INTO KCONFIG.COD
//-------------------------------------------------------------------------

tControlSettings controlSettings = {
	{
	{0xc8,0x48,0xd0,0x50,0xcb,0x4b,0xcd,0x4d,0x38,0xff,0xff,0x4f,0xff,0x51,0xff,0x4a,0xff,0x4e,0xff,0xff,0x10,0x47,0x12,0x49,0x1d,0x9d,0x39,0xff,0x21,0xff,0x1e,0xff,0x2c,0xff,0x30,0xff,0x13,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0x1f,0xff,0x33,0xff,0x34,0xff,0x23,0xff,0x14,0xff,0xff,0xff,0x0,0x0},
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	{0x5,0xc,0xff,0xff,0xff,0xff,0x7,0xf,0x13,0xb,0xff,0x6,0x8,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0},
	{0x0,0x1,0xff,0xff,0x2,0xff,0x7,0xf,0x13,0xb,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x3,0x0,0x1,0x2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x1,0xff,0xff,0x2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	#ifdef WINDOWS
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	#endif
	},
	{
	{0xc8,0x48,0xd0,0x50,0xcb,0x4b,0xcd,0x4d,0x38,0xff,0xff,0x4f,0xff,0x51,0xff,0x4a,0xff,0x4e,0xff,0xff,0x10,0x47,0x12,0x49,0x1d,0x9d,0x39,0xff,0x21,0xff,0x1e,0xff,0x2c,0xff,0x30,0xff,0x13,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0x1f,0xff,0x33,0xff,0x34,0xff,0x23,0xff,0x14,0xff,0xff,0xff,0x0,0x0},
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	{0x5,0xc,0xff,0xff,0xff,0xff,0x7,0xf,0x13,0xb,0xff,0x6,0x8,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0},
	{0x0,0x1,0xff,0xff,0x2,0xff,0x7,0xf,0x13,0xb,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x3,0x0,0x1,0x2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x1,0xff,0xff,0x2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	#ifdef WINDOWS
	{0x0,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0},
	#endif
	},
	{
	0x2 ,0xff,0x3 ,0xff,0x4 ,0xff,0x5 ,0xff,0x6 ,0xff,0x7 ,0xff,0x8 ,0xff,0x9 ,
	0xff,0xa ,0xff,0xb ,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff 
	},
	{
	0x2 ,0xff,0x3 ,0xff,0x4 ,0xff,0x5 ,0xff,0x6 ,0xff,0x7 ,0xff,0x8 ,0xff,0x9 ,
	0xff,0xa ,0xff,0xb ,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff 
	}
};

kc_item kc_keyboard [NUM_KEY_CONTROLS] = {
	{  0, 15, 49, 71, 26, 62,  2, 63,  1,"Pitch forward", 270, BT_KEY, 255 },
	{  1, 15, 49,100, 26, 63,  3,  0, 24,"Pitch forward", 270, BT_KEY, 255 },
	{  2, 15, 57, 71, 26,  0,  4, 25,  3,"Pitch backward", 271, BT_KEY, 255 },
	{  3, 15, 57,100, 26,  1,  5,  2, 26,"Pitch backward", 271, BT_KEY, 255 },
	{  4, 15, 65, 71, 26,  2,  6, 27,  5,"Turn left", 272, BT_KEY, 255 },
	{  5, 15, 65,100, 26,  3,  7,  4, 28,"Turn left", 272, BT_KEY, 255 },
	{  6, 15, 73, 71, 26,  4,  8, 29,  7,"Turn right", 273, BT_KEY, 255 },
	{  7, 15, 73,100, 26,  5,  9,  6, 34,"Turn right", 273, BT_KEY, 255 },
	{  8, 15, 84, 71, 26,  6, 10, 35,  9,"Slide on", 274, BT_KEY, 255 },
	{  9, 15, 84,100, 26,  7, 11,  8, 36,"Slide on", 274, BT_KEY, 255 },
	{ 10, 15, 92, 71, 26,  8, 12, 37, 11,"Slide left", 275, BT_KEY, 255 },
	{ 11, 15, 92,100, 26,  9, 13, 10, 44,"Slide left", 275, BT_KEY, 255 },
	{ 12, 15,100, 71, 26, 10, 14, 45, 13,"Slide right", 276, BT_KEY, 255 },
	{ 13, 15,100,100, 26, 11, 15, 12, 30,"Slide right", 276, BT_KEY, 255 },
	{ 14, 15,108, 71, 26, 12, 16, 31, 15,"Slide up", 277, BT_KEY, 255 },
	{ 15, 15,108,100, 26, 13, 17, 14, 32,"Slide up", 277, BT_KEY, 255 },
	{ 16, 15,116, 71, 26, 14, 18, 33, 17,"Slide down", 278, BT_KEY, 255 },
	{ 17, 15,116,100, 26, 15, 19, 16, 46,"Slide down", 278, BT_KEY, 255 },
	{ 18, 15,127, 71, 26, 16, 20, 47, 19,"Bank on", 279, BT_KEY, 255 },
	{ 19, 15,127,100, 26, 17, 21, 18, 38,"Bank on", 279, BT_KEY, 255 },
	{ 20, 15,135, 71, 26, 18, 22, 39, 21,"Bank left", 280, BT_KEY, 255 },
	{ 21, 15,135,100, 26, 19, 23, 20, 40,"Bank left", 280, BT_KEY, 255 },
	{ 22, 15,143, 71, 26, 20, 48, 41, 23,"Bank right", 281, BT_KEY, 255 },
	{ 23, 15,143,100, 26, 21, 49, 22, 42,"Bank right", 281, BT_KEY, 255 },
	{ 24,158, 49, 83, 26, 59, 26,  1, 25,"Fire primary", 282, BT_KEY, 255 },
	{ 25,158, 49,112, 26, 57, 27, 24,  2,"Fire primary", 282, BT_KEY, 255 },
	{ 26,158, 57, 83, 26, 24, 28,  3, 27,"Fire secondary", 283,BT_KEY, 255 },
	{ 27,158, 57,112, 26, 25, 29, 26,  4,"Fire secondary", 283,BT_KEY, 255 },
	{ 28,158, 65, 83, 26, 26, 34,  5, 29,"Fire flare", 284,BT_KEY, 255 },
	{ 29,158, 65,112, 26, 27, 35, 28,  6,"Fire flare", 284,BT_KEY, 255 },
	{ 30,158,103, 83, 26, 44, 32, 13, 31,"Accelerate", 285,BT_KEY, 255 },
	{ 31,158,103,112, 26, 45, 33, 30, 14,"Accelerate", 285,BT_KEY, 255 },
	{ 32,158,111, 83, 26, 30, 46, 15, 33,"reverse", 286,BT_KEY, 255 },
	{ 33,158,111,112, 26, 31, 47, 32, 16,"reverse", 286,BT_KEY, 255 },
	{ 34,158, 73, 83, 26, 28, 36,  7, 35,"Drop Bomb", 287,BT_KEY, 255 },
	{ 35,158, 73,112, 26, 29, 37, 34,  8,"Drop Bomb", 287,BT_KEY, 255 },
	{ 36,158, 84, 83, 26, 34, 44,  9, 37,"Rear View", 288,BT_KEY, 255 },
	{ 37,158, 84, 112, 26, 35, 45, 36, 10,"Rear View", 288,BT_KEY, 255 },
	{ 38,158,130, 83, 26, 46, 40, 19, 39,"Cruise Faster", 289,BT_KEY, 255 },
	{ 39,158,130,112, 26, 47, 41, 38, 20,"Cruise Faster", 289,BT_KEY, 255 },
	{ 40,158,138, 83, 26, 38, 42, 21, 41,"Cruise Slower", 290,BT_KEY, 255 },
	{ 41,158,138,112, 26, 39, 43, 40, 22,"Cruise Slower", 290,BT_KEY, 255 },
	{ 42,158,146, 83, 26, 40, 54, 23, 43,"Cruise Off", 291,BT_KEY, 255 },
	{ 43,158,146,112, 26, 41, 55, 42, 48,"Cruise Off", 291,BT_KEY, 255 },
	{ 44,158, 92, 83, 26, 36, 30, 11, 45,"Automap", 292,BT_KEY, 255 },
	{ 45,158, 92,112, 26, 37, 31, 44, 12,"Automap", 292,BT_KEY, 255 },
	{ 46,158,119, 83, 26, 32, 38, 17, 47,"Afterburner", 293,BT_KEY, 255 },
	{ 47,158,119,112, 26, 33, 39, 46, 18,"Afterburner", 293,BT_KEY, 255 },
	{ 48, 15,154, 71, 26, 22, 50, 43, 49,"Cycle Primary", 294,BT_KEY, 255 },
	{ 49, 15,154,100, 26, 23, 51, 48, 54,"Cycle Primary", 294,BT_KEY, 255 },
	{ 50, 15,162, 71, 26, 48, 52, 55, 51,"Cycle Second", 295,BT_KEY, 255 },
	{ 51, 15,162,100, 26, 49, 53, 50, 56,"Cycle Second", 295,BT_KEY, 255 },
	{ 52, 15,170, 71, 26, 50, 60, 57, 53,"Zoom In", 296,BT_KEY, 255 },
	{ 53, 15,170,100, 26, 51, 61, 52, 58,"Zoom In", 296,BT_KEY, 255 },
	{ 54,158,157, 83, 26, 42, 56, 49, 55,"Headlight", 297,BT_KEY, 255 },
	{ 55,158,157,112, 26, 43, 57, 54, 50,"Headlight", 297,BT_KEY, 255 },
	{ 56,158,165, 83, 26, 54, 58, 51, 57,"Energy->Shield", 298,BT_KEY, 255 },
	{ 57,158,165,112, 26, 55, 25, 56, 52,"Energy->Shield", 298,BT_KEY, 255 },
   { 58,158,173, 83, 26, 56, 59, 53, 60,"Toggle Bomb", 299, BT_KEY,255},
   { 59,158,181, 83, 26, 58, 24, 61, 62,"Toggle Icons", 653, BT_KEY,255},
	{ 60, 15,181, 71, 26, 52, 62, 58, 61,"Use Cloak", 751,BT_KEY, 255 },
	{ 61, 15,181,100, 26, 53, 63, 60, 59,"Use Cloak", 751,BT_KEY, 255 },
	{ 62, 15,189, 71, 26, 60,  0, 59, 63,"Use Invul", 752,BT_KEY, 255 },
	{ 63, 15,189,100, 26, 61,  1, 62,  0,"Use Invul", 752,BT_KEY, 255 }
};

ubyte kc_kbd_flags [NUM_KEY_CONTROLS];

kc_item kc_joystick [NUM_JOY_CONTROLS] = {
	{  0, 25, 46, 85, 26, 15,  1, 24,  5,"Fire primary", 282, BT_JOY_BUTTON, 255 },
	{  1, 25, 54, 85, 26,  0,  4,  5,  6,"Fire secondary", 283, BT_JOY_BUTTON, 255 },
	{  2, 25, 94, 85, 26, 35,  3, 10, 11,"Accelerate", 285, BT_JOY_BUTTON, 255 },
	{  3, 25,102, 85, 26,  2, 25, 11, 12,"reverse", 286, BT_JOY_BUTTON, 255 },
	{  4, 25, 62, 85, 26,  1, 26,  6,  7,"Fire flare", 284, BT_JOY_BUTTON, 255 },
	{  5,180, 46, 79, 26, 23,  6,  0,  1,"Slide on", 274, BT_JOY_BUTTON, 255 },
	{  6,180, 54, 79, 26,  5,  7,  1,  4,"Slide left", 275, BT_JOY_BUTTON, 255 },
	{  7,180, 62, 79, 26,  6,  8,  4, 26,"Slide right", 276, BT_JOY_BUTTON, 255 },
	{  8,180, 70, 79, 26,  7,  9, 26, 34,"Slide up", 277, BT_JOY_BUTTON, 255 },
	{  9,180, 78, 79, 26,  8, 10, 34, 35,"Slide down", 278, BT_JOY_BUTTON, 255 },
	{ 10,180, 86, 79, 26,  9, 11, 35,  2,"Bank on", 279, BT_JOY_BUTTON, 255 },
	{ 11,180, 94, 79, 26, 10, 12,  2,  3,"Bank left", 280, BT_JOY_BUTTON, 255 },
	{ 12,180,102, 79, 26, 11, 28,  3, 25,"Bank right", 281, BT_JOY_BUTTON, 255 },
	{ 13, 22,162, 51, 26, 33, 15, 32, 14,"Pitch U/D", 300, BT_JOY_AXIS, 255 },
	{ 14, 22,162, 99,  8, 33, 16, 13, 17,"Pitch U/D", 300, BT_INVERT, 255 },
	{ 15, 22,170, 51, 26, 13,  0, 18, 16,"Turn L/R", 301, BT_JOY_AXIS, 255 },
	{ 16, 22,170, 99,  8, 14, 17, 15, 19,"Turn L/R", 301, BT_INVERT, 255 },
	{ 17,164,162, 58, 26, 32, 19, 14, 18,"Slide L/R", 302, BT_JOY_AXIS, 255 },
	{ 18,164,162,106,  8, 32, 20, 17, 15,"Slide L/R", 302, BT_INVERT, 255 },
	{ 19,164,170, 58, 26, 17, 21, 16, 20,"Slide U/D", 303, BT_JOY_AXIS, 255 },
	{ 20,164,170,106,  8, 18, 22, 19, 21,"Slide U/D", 303, BT_INVERT, 255 },
	{ 21,164,178, 58, 26, 19, 23, 20, 22,"Bank L/R", 304, BT_JOY_AXIS, 255 },
	{ 22,164,178,106,  8, 20, 24, 21, 23,"Bank L/R", 304, BT_INVERT, 255 },
	{ 23,164,186, 58, 26, 21,  5, 22, 24,"throttle", 305, BT_JOY_AXIS, 255 },
	{ 24,164,186,106,  8, 22, 13, 23,  0,"throttle", 305, BT_INVERT, 255 },
	{ 25, 25,110, 85, 26,  3, 27, 12, 28,"Rear View", 288, BT_JOY_BUTTON, 255 },
	{ 26, 25, 70, 85, 26,  4, 34,  7,  8,"Drop Bomb", 287, BT_JOY_BUTTON, 255 },
	{ 27, 25,118, 85, 26, 25, 30, 28, 29,"Afterburner", 293, BT_JOY_BUTTON, 255 },
	{ 28,180,110, 79, 26, 12, 29, 25, 27,"Cycle Primary", 294, BT_JOY_BUTTON, 255 },
	{ 29,180,118, 79, 26, 28, 31, 27, 30,"Cycle Secondary", 295, BT_JOY_BUTTON, 255 },
	{ 30, 25,126, 85, 26, 27, 33, 29, 31,"Headlight", 297, BT_JOY_BUTTON, 255 },
	{ 31,180,126, 79, 26, 29, 32, 30, 33,"Toggle Bomb", 299, BT_JOY_BUTTON, 255 },
	{ 32,180,134, 79, 26, 31, 18, 33, 13,"Toggle Icons", 653, BT_JOY_BUTTON, 255 },
	{ 33, 25,134, 85, 26, 30, 14, 31, 32,"Automap", 292, BT_JOY_BUTTON, 255 },
	{ 34, 25, 78, 85, 26, 26, 35, 8,   9,"Use Cloak", 751, BT_JOY_BUTTON, 255 },
	{ 35, 25, 86, 85, 26, 34,  2, 9,  10,"Use Invul", 752, BT_JOY_BUTTON, 255 }
};

kc_item kc_mouse [NUM_MOUSE_CONTROLS] = {
	{  0, 25, 46, 85, 26, 23,  1, 24,  5,"Fire primary", 282, BT_MOUSE_BUTTON, 255 },
	{  1, 25, 54, 85, 26,  0,  4,  5,  6,"Fire secondary", 283, BT_MOUSE_BUTTON, 255 },
	{  2, 25, 78, 85, 26, 26,  3,  8,  9,"Accelerate", 285, BT_MOUSE_BUTTON, 255 },
	{  3, 25, 86, 85, 26,  2, 27,  9, 10,"reverse", 286, BT_MOUSE_BUTTON, 255 },
	{  4, 25, 62, 85, 26,  1, 26,  6,  7,"Fire flare", 284, BT_MOUSE_BUTTON, 255 },
	{  5,180, 46, 59, 26, 24,  6,  0,  1,"Slide on", 274, BT_MOUSE_BUTTON, 255 },
	{  6,180, 54, 59, 26,  5,  7,  1,  4,"Slide left", 275, BT_MOUSE_BUTTON, 255 },
	{  7,180, 62, 59, 26,  6,  8,  4, 26,"Slide right", 276, BT_MOUSE_BUTTON, 255 },
	{  8,180, 70, 59, 26,  7,  9, 26,  2,"Slide up", 277, BT_MOUSE_BUTTON, 255 },
	{  9,180, 78, 59, 26,  8, 10,  2,  3,"Slide down", 278, BT_MOUSE_BUTTON, 255 },
	{ 10,180, 86, 59, 26,  9, 11,  3, 27,"Bank on", 279, BT_MOUSE_BUTTON, 255 },
	{ 11,180, 94, 59, 26, 10, 12, 27, 25,"Bank left", 280, BT_MOUSE_BUTTON, 255 },
	{ 12,180,102, 59, 26, 11, 30, 25, 28,"Bank right", 281, BT_MOUSE_BUTTON, 255 },

	{ 13,103,146, 58, 26, 29, 15, 30, 14,"Pitch U/D", 300, BT_MOUSE_AXIS, 255 },
	{ 14,103,146,106,  8, 23, 16, 13, 15,"Pitch U/D", 300, BT_INVERT, 255 },
	{ 15,103,154, 58, 26, 13, 17, 14, 16,"Turn L/R", 301, BT_MOUSE_AXIS, 255 },
	{ 16,103,154,106,  8, 14, 18, 15, 17,"Turn L/R", 301, BT_INVERT, 255 },
	{ 17,103,162, 58, 26, 15, 19, 16, 18,"Slide L/R", 302, BT_MOUSE_AXIS, 255 },
	{ 18,103,162,106,  8, 16, 20, 17, 19,"Slide L/R", 302, BT_INVERT, 255 },
	{ 19,103,170, 58, 26, 17, 21, 18, 20,"Slide U/D", 303, BT_MOUSE_AXIS, 255 },
	{ 20,103,170,106,  8, 18, 22, 19, 21,"Slide U/D", 303, BT_INVERT, 255 },
	{ 21,103,178, 58, 26, 19, 23, 20, 22,"Bank L/R", 304, BT_MOUSE_AXIS, 255 },
	{ 22,103,178,106,  8, 20, 24, 21, 23,"Bank L/R", 304, BT_INVERT, 255 },
	{ 23,103,186, 58, 26, 21,  0, 22, 24,"Throttle", 305, BT_MOUSE_AXIS, 255 },
	{ 24,103,186,106,  8, 22,  5, 23,  0,"Throttle", 305, BT_INVERT, 255 },

	{ 25, 25,102, 85, 26, 27, 28, 11, 12,"Rear View", 288, BT_MOUSE_BUTTON, 255 },
	{ 26, 25, 70, 85, 26,  4,  2,  7,  8,"Drop Bomb", 287, BT_MOUSE_BUTTON, 255 },
	{ 27, 25, 94, 85, 26,  3, 25, 10, 11,"Afterburner", 293, BT_MOUSE_BUTTON, 255 },
	{ 28, 25,110, 85, 26, 25, 29, 12, 29,"Cycle Primary", 294, BT_MOUSE_BUTTON, 255 },
	{ 29, 25,118, 85, 26, 28, 13, 28, 30,"Cycle Second", 295, BT_MOUSE_BUTTON, 255 },
	{ 30,180,118, 59, 26, 12, 14, 29, 13,"Zoom in", 296, BT_MOUSE_BUTTON, 255 }
};

kc_item kc_superjoy [NUM_JOY_CONTROLS] = {
	{  0, 25, 46, 85, 26, 15,  1, 24,  5,"Fire primary", 282, BT_JOY_BUTTON, 255 },
	{  1, 25, 54, 85, 26,  0,  4,  5,  6,"Fire secondary", 283, BT_JOY_BUTTON, 255 },
	{  2, 25, 85, 85, 26, 26,  3,  9, 10,"Accelerate", 285, BT_JOY_BUTTON, 255 },
	{  3, 25, 93, 85, 26,  2, 25, 10, 11,"reverse", 286, BT_JOY_BUTTON, 255 },
	{  4, 25, 62, 85, 26,  1, 26,  6,  7,"Fire flare", 284, BT_JOY_BUTTON, 255 },
	{  5,180, 46, 79, 26, 23,  6,  0,  1,"Slide on", 274, BT_JOY_BUTTON, 255 },
	{  6,180, 54, 79, 26,  5,  7,  1,  4,"Slide left", 275, BT_JOY_BUTTON, 255 },
	{  7,180, 62, 79, 26,  6,  8,  4, 26,"Slide right", 276, BT_JOY_BUTTON, 255 },
	{  8,180, 70, 79, 26,  7,  9, 26,  9,"Slide up", 277, BT_JOY_BUTTON, 255 },
	{  9,180, 78, 79, 26,  8, 10,  8,  2,"Slide down", 278, BT_JOY_BUTTON, 255 },
	{ 10,180, 86, 79, 26,  9, 11,  2,  3,"Bank on", 279, BT_JOY_BUTTON, 255 },
	{ 11,180, 94, 79, 26, 10, 12,  3, 12,"Bank left", 280, BT_JOY_BUTTON, 255 },
	{ 12,180,102, 79, 26, 11, 28, 11, 25,"Bank right", 281, BT_JOY_BUTTON, 255 },
	{ 13, 22,162, 51, 26, 33, 15, 32, 14,"Pitch U/D", 300, BT_JOY_AXIS, 255 },
	{ 14, 22,162, 99,  8, 33, 16, 13, 17,"Pitch U/D", 300, BT_INVERT, 255 },
	{ 15, 22,170, 51, 26, 13,  0, 18, 16,"Turn L/R", 301, BT_JOY_AXIS, 255 },
	{ 16, 22,170, 99,  8, 14, 17, 15, 19,"Turn L/R", 301, BT_INVERT, 255 },
	{ 17,164,162, 58, 26, 32, 19, 14, 18,"Slide L/R", 302, BT_JOY_AXIS, 255 },
	{ 18,164,162,106,  8, 32, 20, 17, 15,"Slide L/R", 302, BT_INVERT, 255 },
	{ 19,164,170, 58, 26, 17, 21, 16, 20,"Slide U/D", 303, BT_JOY_AXIS, 255 },
	{ 20,164,170,106,  8, 18, 22, 19, 21,"Slide U/D", 303, BT_INVERT, 255 },
	{ 21,164,178, 58, 26, 19, 23, 20, 22,"Bank L/R", 304, BT_JOY_AXIS, 255 },
	{ 22,164,178,106,  8, 20, 24, 21, 23,"Bank L/R", 304, BT_INVERT, 255 },
	{ 23,164,186, 58, 26, 21,  5, 22, 24,"throttle", 305, BT_JOY_AXIS, 255 },
	{ 24,164,186,106,  8, 22, 13, 23,  0,"throttle", 305, BT_INVERT, 255 },
	{ 25, 25,110, 85, 26,  3, 27, 12, 28,"Rear View", 288, BT_JOY_BUTTON, 255 },
	{ 26, 25, 70, 85, 26,  4,  2,  7,  8,"Drop Bomb", 287, BT_JOY_BUTTON, 255 },
	{ 27, 25,118, 85, 26, 25, 30, 28, 29,"Afterburner", 293, BT_JOY_BUTTON, 255 },
	{ 28,180,110, 79, 26, 12, 29, 25, 27,"Cycle Primary", 294, BT_JOY_BUTTON, 255 },
	{ 29,180,118, 79, 26, 28, 31, 27, 30,"Cycle Secondary", 295, BT_JOY_BUTTON, 255 },
	{ 30, 25,126, 85, 26, 27, 33, 29, 31,"Headlight", 297, BT_JOY_BUTTON, 255 },
	{ 31,180,126, 79, 26, 29, 32, 30, 33,"Toggle Bomb", 299, BT_JOY_BUTTON, 255 },
	{ 32,180,134, 79, 26, 31, 18, 33, 13,"Toggle Icons", 653, BT_JOY_BUTTON, 255 },
	{ 33, 25,134, 85, 26, 30, 14, 31, 32,"Automap", 292, BT_JOY_BUTTON, 255 },
};

#ifdef D2X_KEYS
//added on 2/4/99 by Victor Rachels to add d1x new keys
kc_item kc_d2x [NUM_D2X_CONTROLS] = {
//        id,x,y,w1,w2,u,d,l,r,text_num1,type,value
	{  0, 15, 49, 71, 26, 27,  2, 27,  1, "WEAPON 1", 306, BT_KEY, 255 },
	{  1, 15, 49,100, 26, 26,  3,  0,  2, "WEAPON 1", 306, BT_JOY_BUTTON, 255 },
	{  2, 15, 57, 71, 26,  0,  4,  1,  3, "WEAPON 2", 307, BT_KEY, 255 },
	{  3, 15, 57,100, 26,  1,  5,  2,  4, "WEAPON 2", 307, BT_JOY_BUTTON, 255 },
	{  4, 15, 65, 71, 26,  2,  6,  3,  5, "WEAPON 3", 308, BT_KEY, 255 },
	{  5, 15, 65,100, 26,  3,  7,  4,  6, "WEAPON 3", 308, BT_JOY_BUTTON, 255 },
	{  6, 15, 73, 71, 26,  4,  8,  5,  7, "WEAPON 4", 309, BT_KEY, 255 },
	{  7, 15, 73,100, 26,  5,  9,  6,  8, "WEAPON 4", 309, BT_JOY_BUTTON, 255 },
	{  8, 15, 81, 71, 26,  6, 10,  7,  9, "WEAPON 5", 310, BT_KEY, 255 },
	{  9, 15, 81,100, 26,  7, 11,  8, 10, "WEAPON 5", 310, BT_JOY_BUTTON, 255 },

	{ 10, 15, 89, 71, 26,  8, 12,  9, 11, "WEAPON 6", 311, BT_KEY, 255 },
	{ 11, 15, 89,100, 26,  9, 13, 10, 12, "WEAPON 6", 311, BT_JOY_BUTTON, 255 },
	{ 12, 15, 97, 71, 26, 10, 14, 11, 13, "WEAPON 7", 312, BT_KEY, 255 },
	{ 13, 15, 97,100, 26, 11, 15, 12, 14, "WEAPON 7", 312, BT_JOY_BUTTON, 255 },
	{ 14, 15,105, 71, 26, 12, 16, 13, 15, "WEAPON 8", 313, BT_KEY, 255 },
	{ 15, 15,105,100, 26, 13, 17, 14, 16, "WEAPON 8", 313, BT_JOY_BUTTON, 255 },
	{ 16, 15,113, 71, 26, 14, 18, 15, 17, "WEAPON 9", 314, BT_KEY, 255 },
	{ 17, 15,113,100, 26, 15, 19, 16, 18, "WEAPON 9", 314, BT_JOY_BUTTON, 255 },
	{ 18, 15,121, 71, 26, 16, 20, 17, 19, "WEAPON 10", 315, BT_KEY, 255 },
	{ 19, 15,121,100, 26, 17, 21, 18, 20, "WEAPON 10", 315, BT_JOY_BUTTON, 255 },

	//{ 20, 15,131, 71, 26, 18, 22, 19, 21, "CYC PRIMARY", BT_KEY, 255 },
	//{ 21, 15,131,100, 26, 19, 23, 20, 22, "CYC PRIMARY", BT_JOY_BUTTON, 255 },
	//{ 22, 15,139, 71, 26, 20, 24, 21, 23, "CYC SECONDARY", BT_KEY, 255 },
	//{ 23, 15,139,100, 26, 21, 25, 22, 24, "CYC SECONDARY", BT_JOY_BUTTON, 255 },
	//{ 24,  8,147, 78, 26, 22, 26, 23, 25, "TOGGLE_PRIM AUTO", BT_KEY, 255 },
	//{ 25,  8,147,107, 26, 23, 27, 24, 26, "TOGGLE_PRIM_AUTO", BT_JOY_BUTTON, 255 },
	//{ 26,  8,155, 78, 26, 24,  1, 25, 27, "TOGGLE SEC AUTO", BT_KEY, 255 },
	//{ 27,  8,155,107, 26, 25,  0, 26,  0, "TOGGLE SEC AUTO", BT_JOY_BUTTON, 255 },
};
//end this section addition - VR
#endif

static int xOffs = 0, yOffs = 0;

static int start_axis [JOY_MAX_AXES];

vms_vector ExtForceVec;
vms_matrix ExtApplyForceMatrix;

int ExtJoltInfo [3]={0,0,0};
int ExtXVibrateInfo [2]={0,0};
int ExtYVibrateInfo [2]={0,0};
ubyte ExtXVibrateClear=0;
ubyte ExtYVibrateClear=0;

//------------------------------------------------------------------------------

void KCDrawItemExt (kc_item *item, int is_current, int bRedraw);
int KCChangeInvert (kc_item * item);
void ControlsReadFCS (int raw_axis);
void KCSetFCSButton (int btn, int button);
void KCReadExternalControls (void);

//------------------------------------------------------------------------------

inline void KCDrawItem (kc_item *item, int is_current)
{
KCDrawItemExt (item, is_current, gameOpts->menus.nStyle);
}

//------------------------------------------------------------------------------

int KCIsAxisUsed (int axis)
{
	int i;

for (i = 0; i < NUM_JOY_CONTROLS; i++) {
	if ((kc_joystick [i].type == BT_JOY_AXIS) && (kc_joystick [i].value == axis))
		return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

#ifdef TABLE_CREATION
int find_item_at (kc_item * items, int nitems, int x, int y)
{
	int i;
	
	for (i=0; i<nitems; i++)	{
		if (((items [i].x+items [i].w1)==x) && (items [i].y==y))
			return i;
	}
	return -1;
}

//------------------------------------------------------------------------------

int FindNextItemUp (kc_item * items, int nitems, int citem)
{
	int x, y, i;

	y = items [citem].y;
	x = items [citem].x+items [citem].w1;
	
do {	
	if (--y < 0) {
		y = grdCurCanv->cv_bitmap.bm_props.h-1;
		if (--x < 0) 
			x = grdCurCanv->cv_bitmap.bm_props.w-1;
		}
	i = find_item_at (items, nitems, x, y);
} while (i < 0);
return i;
}

//------------------------------------------------------------------------------

int FindNextItemDown (kc_item * items, int nitems, int citem)
{
	int x, y, i;

	y = items [citem].y;
	x = items [citem].x+items [citem].w1;
	
do {	
	if (++y > grdCurCanv->cv_bitmap.bm_props.h-1) {
		y = 0;
		if (++x > grdCurCanv->cv_bitmap.bm_props.w-1)
			x = 0;
		}
	i = find_item_at (items, nitems, x, y);
} while (i < 0);
return i;
}

//------------------------------------------------------------------------------

int FindNextItemRight (kc_item * items, int nitems, int citem)
{
	int x, y, i;

	y = items [citem].y;
	x = items [citem].x+items [citem].w1;
	
do {	
	if (++x > grdCurCanv->cv_bitmap.bm_props.w-1) {
		x = 0;
		y++;
		if (++y > grdCurCanv->cv_bitmap.bm_props.h-1)
			y = 0;
		}
	i = find_item_at (items, nitems, x, y);
} while (i < 0);
return i;
}

//------------------------------------------------------------------------------

int FindNextItemLeft (kc_item * items, int nitems, int citem)
{
	int x, y, i;

	y = items [citem].y;
	x = items [citem].x+items [citem].w1;
	
do {	
	if (--x < 0) {
		x = grdCurCanv->cv_bitmap.bm_props.w-1;
		if (--y < 0) 
			y = grdCurCanv->cv_bitmap.bm_props.h-1;
		}
	i = find_item_at (items, nitems, x, y);
} while (i < 0);
return i;
}
#endif

//------------------------------------------------------------------------------

inline char *MouseTextString (i)
{
return (i < 3)? baseGameTexts [mousebutton_text [i]] : mousebutton_textra [i - 3];
}

//------------------------------------------------------------------------------

#ifdef NEWMENU_MOUSE
int KCGetItemHeight (kc_item *item)
{
	int w, h, aw;
	char btext [10];

	if (item->value==255) {
		strcpy (btext, "");
	} else {
		switch (item->type)	{
			case BT_KEY:
				strncpy (btext, key_text [item->value], 10); 
				break;
			case BT_MOUSE_BUTTON:
				strncpy (btext, MouseTextString (item->value), 10); 
				break;
			case BT_MOUSE_AXIS:
				strncpy (btext, baseGameTexts [mouseaxis_text [item->value]], 10); 
				break;
			case BT_JOY_BUTTON:
#if defined (USE_LINUX_JOY)
				sprintf (btext, "J%d B%d", j_button [item->value].joydev, 
						  j_Get_joydev_button_number (item->value);
#elif 1//defined (_WIN32)
				{
					int	nStick = item->value / MAX_BUTTONS_PER_JOYSTICK;
					int	nBtn = item->value % MAX_BUTTONS_PER_JOYSTICK;
					int	nHat = SDL_Joysticks [nStick].n_buttons;
					//static char szHatDirs [4] = {'U', 'L', 'D', 'R'};
					static char cHatDirs [4] = { (char) 130, (char) 127, (char) 128, (char) 129};

				if (nBtn < nHat)
					sprintf (btext, "J%d B%d", nStick + 1, nBtn + 1);
				else
					sprintf (btext, "HAT%d%c", nStick + 1, cHatDirs [nBtn - nHat]);
				}
#else
				if (JOYBUTTON_TEXT (item->value) !=-1)
					strncpy (btext, baseGameTexts [JOYBUTTON_TEXT (item->value)], 10);
				else
					sprintf (btext, "BTN%d", item->value);
#endif
				break;
			case BT_JOY_AXIS:
#if defined (USE_LINUX_JOY)
				sprintf (btext, "J%d A%d", j_axis [item->value].joydev, 
						  j_Get_joydev_axis_number (item->value);
#elif 1//defined (_WIN32)
				{
					int	nStick = item->value / MAX_AXES_PER_JOYSTICK;
					int	nAxis = item->value % MAX_AXES_PER_JOYSTICK;
					static char	cAxis [4] = {'X', 'Y', 'Z', 'R'};

				if (nAxis < 4)
					sprintf (btext, "J%d %c", nStick + 1, cAxis [nAxis]);
				else
					sprintf (btext, "J%d A%d", nStick + 1, nAxis + 1);
				}
#else
				strncpy (btext, baseGameTexts [JOYAXIS_TEXT (item->value)], 10);
#endif
				break;
			case BT_INVERT:
				strncpy (btext, baseGameTexts [invert_text [item->value]], 10); 
				break;
		}
	}
	GrGetStringSize (btext, &w, &h, &aw);

	return h;
}
#endif

//------------------------------------------------------------------------------

#define kc_gr_scanline(_x1,_x2,_y)	gr_scanline ((_x1), (_x2), (_y))
#define kc_gr_pixel(_x,_y)	gr_pixel ((_x), (_y))
#define KC_LHX(_x) (LHX (_x)+xOffs)
#define KC_LHY(_y) (LHY (_y)+yOffs)

void KCDrawTitle (char *title)
{
char *p = strchr (title, '\n');

grdCurCanv->cv_font = MEDIUM3_FONT;
if (p) 
	*p = 32;
GrString (0x8000, KC_LHY (8), title);
if (p)
	*p = '\n';
}

//------------------------------------------------------------------------------

void KCDrawHeader (kc_item *items)
{
grdCurCanv->cv_font = GAME_FONT;
GrSetFontColorRGBi (RGBA_PAL2 (28, 28, 28), 1, 0, 0);

GrString (0x8000, KC_LHY (20), TXT_KCONFIG_STRING_1);
GrSetFontColorRGBi (RGBA_PAL2 (28, 28, 28), 1, 0, 0);
if (items == kc_keyboard)	{
	GrSetFontColorRGBi (RGBA_PAL2 (31, 27, 6), 1, 0, 0);
	GrSetColorRGBi (RGBA_PAL2 (31, 27, 6));
	kc_gr_scanline (KC_LHX (98), KC_LHX (106), KC_LHY (42));
	kc_gr_scanline (KC_LHX (120), KC_LHX (128), KC_LHY (42));
	kc_gr_pixel (KC_LHX (98), KC_LHY (43));						
	kc_gr_pixel (KC_LHX (98), KC_LHY (44));						
	kc_gr_pixel (KC_LHX (128), KC_LHY (43));						
	kc_gr_pixel (KC_LHX (128), KC_LHY (44));						
	
	GrString (KC_LHX (109), KC_LHY (40), "OR");

	kc_gr_scanline (KC_LHX (253), KC_LHX (261), KC_LHY (42));
	kc_gr_scanline (KC_LHX (274), KC_LHX (283), KC_LHY (42));
	kc_gr_pixel (KC_LHX (253), KC_LHY (43));						
	kc_gr_pixel (KC_LHX (253), KC_LHY (44));						
	kc_gr_pixel (KC_LHX (283), KC_LHY (43));						
	kc_gr_pixel (KC_LHX (283), KC_LHY (44));						

	GrString (KC_LHX (264), KC_LHY (40), "OR");

}
if (items == kc_joystick)	{
	GrSetFontColorRGBi (RGBA_PAL2 (31,27,6), 1, 0, 0);
	GrSetColorRGBi (RGBA_PAL2 (31, 27, 6));
	kc_gr_scanline (KC_LHX (18), KC_LHX (135), KC_LHY (37));
	kc_gr_scanline (KC_LHX (181), KC_LHX (294), KC_LHY (37));
	kc_gr_scanline (KC_LHX (18), KC_LHX (144), KC_LHY (119+18));
	kc_gr_scanline (KC_LHX (174), KC_LHX (294), KC_LHY (119+18));
	GrString (0x8000, KC_LHY (35), TXT_BUTTONS_HATS);
	GrString (0x8000,KC_LHY (125+18), TXT_AXES);
	GrSetFontColorRGBi (RGBA_PAL2 (28,28,28), 1, 0, 0);
	GrString (KC_LHX (81), KC_LHY (145+8), TXT_AXIS);
	GrString (KC_LHX (111), KC_LHY (145+8), TXT_INVERT);
	GrString (KC_LHX (222), KC_LHY (145+8), TXT_AXIS);
	GrString (KC_LHX (252), KC_LHY (145+8), TXT_INVERT);
} else if (items == kc_mouse)	{
	GrSetFontColorRGBi (RGBA_PAL2 (31,27,6), 1, 0, 0);
	GrSetColorRGBi (RGBA_PAL2 (31,27,6));
	kc_gr_scanline (KC_LHX (18), KC_LHX (135), KC_LHY (37));
	kc_gr_scanline (KC_LHX (181), KC_LHX (294), KC_LHY (37));
	kc_gr_scanline (KC_LHX (18), KC_LHX (144), KC_LHY (119+5));
	kc_gr_scanline (KC_LHX (174), KC_LHX (294), KC_LHY (119+5));
	GrString (0x8000, KC_LHY (35), TXT_BUTTONS);
	GrString (0x8000,KC_LHY (125+5), TXT_AXES);
	GrSetFontColorRGBi (RGBA_PAL2 (28,28,28), 1, 0, 0);
	GrString (KC_LHX (169), KC_LHY (137), TXT_AXIS);
	GrString (KC_LHX (199), KC_LHY (137), TXT_INVERT);
}
#ifdef D2X_KEYS
else if (items == kc_d2x)
{
	GrSetFontColorRGBi (RGBA_PAL2 (31,27,6), 1, 0, 0);
	GrSetColorRGBi (RGBA_PAL2 (31, 27, 6));

	GrString (KC_LHX (94), KC_LHY (40), "KB");
	GrString (KC_LHX (121), KC_LHY (40), "JOY");
}
#endif
}

//------------------------------------------------------------------------------

void KCDrawTable (kc_item *items, int nitems, int citem)
{
	int	i;

for (i = 0; i < nitems; i++)
	KCDrawItemExt (items + i, 0, 0);
KCDrawItemExt (items + citem, 1, 0);
}

//------------------------------------------------------------------------------

void KCQuitMenu (grs_canvas *save_canvas, grs_font *save_font, bkg *bg, int time_stopped)
{
grdCurCanv->cv_font	= save_font;
WIN (DEFINE_SCREEN (old_bg_pcx));
WINDOS (DDGrFreeSubCanvas (bg->menu_canvas), 
		  GrFreeSubCanvas (bg->menu_canvas));
WINDOS (
	DDGrSetCurrentCanvas (save_canvas),
	GrSetCurrentCanvas (save_canvas)
	);			
GameFlushInputs ();
NMRemoveBackground (bg);
newmenu_hide_cursor ();
if (time_stopped)
	StartTime ();
gameStates.menus.nInMenu--;
}

//------------------------------------------------------------------------------

inline int KCAssignControl (kc_item *item, int type, ubyte code)
{
	int	i, n;

if (code == 255)
	return type;

for (i = 0, n = (int) (item - All_items); i < Num_items; i++)	{
	if ((i != n) && (All_items [i].type == type) && (All_items [i].value == code)) {
		All_items [i].value = 255;
		if (curDrawBuffer == GL_FRONT)
			KCDrawItem (All_items + i, 0);
		}
	}
item->value = code;					 
if (curDrawBuffer == GL_FRONT) {
	KCDrawItem (item, 1);
	NMRestoreBackground (0, KC_LHY (INFO_Y), xOffs, yOffs, KC_LHX (310), grdCurCanv->cv_font->ft_h);
	}
GameFlushInputs ();
WIN (DDGRLOCK (dd_grd_curcanv));
GrSetFontColorRGBi (RGBA_PAL2 (28,28,28), 1, 0, 1);
WIN (DDGRUNLOCK (dd_grd_curcanv));
return BT_NONE;
}

//------------------------------------------------------------------------------

void KCDrawQuestion (kc_item *item)
{
	static int looper=0;

	int x, w, h, aw;

WIN (DDGRLOCK (dd_grd_curcanv));	
  // PA_DFX (pa_set_frontbuffer_current ();

	GrGetStringSize ("?", &w, &h, &aw);
	//@@GrSetColor (grFadeTable [fades [looper]*256+c]);
	GrSetColorRGBi (RGBA_PAL2 (21*fades [looper]/31, 0, 24*fades [looper]/31));
	if (++looper>63) 
		looper=0;
	GrURect (KC_LHX (item->w1+item->x), KC_LHY (item->y-1), 
				KC_LHX (item->w1+item->x+item->w2), KC_LHY (item->y)+h);
	GrSetFontColorRGBi (RGBA_PAL2 (28,28,28), 1, 0, 0);
	x = LHX (item->w1+item->x)+ ((LHX (item->w2)-w)/2)+xOffs;
	GrString (x, KC_LHY (item->y), "?");
//	PA_DFX (pa_set_backbuffer_current ();
WIN (DDGRUNLOCK (dd_grd_curcanv));
if (curDrawBuffer != GL_BACK)
	GrUpdate (0);
}

//------------------------------------------------------------------------------

typedef ubyte kc_ctrlfunc_type (void);
typedef kc_ctrlfunc_type *kc_ctrlfunc_ptr;

//------------------------------------------------------------------------------

ubyte KCKeyCtrlFunc (void)
{
	int	i, n, f;

for (i = 0; i < 256; i++)	{
	if (keyd_pressed [i] && strlen (key_text [i]))	{
		f = 0;
		for (n = 0; n < sizeof (system_keys); n++)
			if (system_keys [n] == i)
				f = 1;
		if (!f)	
			return (ubyte) i;
		}
	}
return 255;
}

//------------------------------------------------------------------------------

ubyte KCJoyBtnCtrlFunc (void)
{
	int i;
	ubyte code = 255;

WIN (code = joydefsw_do_button ());
#ifndef WINDOWS
if (gameStates.input.nJoyType == CONTROL_THRUSTMASTER_FCS) {
	int axis [JOY_MAX_AXES];
	joystick_read_raw_axis (JOY_ALL_AXIS, axis);
	ControlsReadFCS (axis [3]);
	if (joy_get_button_state (19)) 
		code = 19;
	else if (joy_get_button_state (15)) 
		code = 15;
	else if (joy_get_button_state (11)) 
		code = 11;
	else if (joy_get_button_state (7)) 
		code = 7;
	for (i = 0; i < 4; i++)
		if (joy_get_button_state (i))
			return (ubyte) i;
	}
else if (gameStates.input.nJoyType == CONTROL_FLIGHTSTICK_PRO) {
	for (i = 4; i < 20; i++) {
		if (joy_get_button_state (i))
			return (ubyte) i;
		}
	}
else {
	for (i = 0; i < JOY_MAX_BUTTONS; i++) {
		if (joy_get_button_state (i))
			return (ubyte) i;
		}
	}
#endif
return code;
}

//------------------------------------------------------------------------------

ubyte KCMouseBtnCtrlFunc (void)
{
int i, b = MouseGetButtons ();
for (i = 0; i < 16; i++)
	if (b & (1 << i))	
		return (ubyte) i;
return 255;
}

//------------------------------------------------------------------------------

ubyte KCJoyAxisCtrlFunc (void)
{
	int cur_axis [JOY_MAX_AXES];
	int i, hd, dd;
	int bLinJoySensSave = gameOpts->input.bLinearJoySens;
	ubyte code = 255;

memset (cur_axis, 0, sizeof (cur_axis));
gameOpts->input.bLinearJoySens = 1;
gameStates.input.kcFrameTime = 128;
ControlsReadJoystick (cur_axis);
gameOpts->input.bLinearJoySens = bLinJoySensSave;
for (i = dd = 0; i < JOY_MAX_AXES; i++) {
	hd = abs (cur_axis [i] - start_axis [i]);
  	if ((hd > (128 * 3 / 4)) && (hd > dd)) {
		dd = hd;
		code = i;
		start_axis [i] = cur_axis [i];
		}
	}
return code;
}

//------------------------------------------------------------------------------

ubyte KCMouseAxisCtrlFunc (void)
{
	int dx, dy;
#ifdef SDL_INPUT
	int dz;
#endif
	ubyte code = 255;

#ifdef SDL_INPUT
MouseGetDeltaZ (&dx, &dy, &dz);
#else
MouseGetDelta (&dx, &dy);
#endif
con_printf (CON_VERBOSE, "mouse: %3d %3d\n", dx, dy);
dx = abs (dx);
dy = abs (dy);
if (max (dx, dy) > 20) {
	code = dy > dx;
	}
#ifdef SDL_INPUT
dz = abs (dz);
if ((dz > 20) && (dz > code ? dy : dx))
	code = 2;
#endif
return code;
}

//------------------------------------------------------------------------------

int KCChangeControl (kc_item *item, int type, kc_ctrlfunc_ptr ctrlfunc, char *pszMsg)
{
	int k = 255;
	ubyte code = 255;

WIN (DDGRLOCK (dd_grd_curcanv));
	GrSetFontColorRGBi (RGBA_PAL2 (28,28,28), 1, 0, 0);
	GrString (0x8000, KC_LHY (INFO_Y), pszMsg);
WIN (DDGRUNLOCK (dd_grd_curcanv));	
{				
#ifdef WINDOWS
	{
		MSG msg;

		DoMessageStuff (&msg);
		DDGRRESTORE;
	}
#endif

#ifdef NETWORK
	if ((gameData.app.nGameMode & GM_MULTI) && (gameStates.app.nFunctionMode == FMODE_GAME) && (!gameStates.app.bEndLevelSequence))
		MultiMenuPoll ();
#endif
//		if (gameData.app.nGameMode & GM_MULTI)
//			GameLoop (0, 0);				// Continue
	k = KeyInKey ();
	if (k == KEY_ESC)
		return KCAssignControl (item, BT_NONE, 255);
	if (k == KEY_ALTED+KEY_F9) {
		SaveScreenShot (NULL, 0);
		return KCAssignControl (item, BT_NONE, 255);
		}
	if (curDrawBuffer == GL_FRONT)
		timer_delay (f0_1 / 10);
	KCDrawQuestion (item);
	}
return KCAssignControl (item, type, ctrlfunc ());
}

//------------------------------------------------------------------------------

inline int KCChangeKey (kc_item *item)
{
return KCChangeControl (item, BT_KEY, KCKeyCtrlFunc, TXT_PRESS_NEW_KEY);
}

//------------------------------------------------------------------------------

inline int KCChangeJoyButton (kc_item *item)
{
return KCChangeControl (item, BT_JOY_BUTTON, KCJoyBtnCtrlFunc, TXT_PRESS_NEW_JBUTTON);
}

//------------------------------------------------------------------------------

inline int KCChangeMouseButton (kc_item * item)
{
return KCChangeControl (item, BT_MOUSE_BUTTON, KCMouseBtnCtrlFunc, TXT_PRESS_NEW_MBUTTON);
}

//------------------------------------------------------------------------------

inline int KCChangeJoyAxis (kc_item *item)
{
return KCChangeControl (item, BT_JOY_AXIS, KCJoyAxisCtrlFunc, TXT_MOVE_NEW_JOY_AXIS);
}

//------------------------------------------------------------------------------

inline int KCChangeMouseAxis (kc_item * item)
{
return KCChangeControl (item, BT_MOUSE_AXIS, KCMouseAxisCtrlFunc, TXT_MOVE_NEW_MSE_AXIS);
}

//------------------------------------------------------------------------------

int KCChangeInvert (kc_item * item)
{
GameFlushInputs ();
item->value = !item->value;
if (curDrawBuffer == GL_FRONT) 
	KCDrawItem (item, 1);
return BT_NONE;
}

//------------------------------------------------------------------------------

void KConfigSub (kc_item * items, int nitems, char * title)
{
WINDOS (
	dd_grs_canvas * save_canvas,
	grs_canvas * save_canvas
);
	grs_font * save_font;
#ifdef NEWMENU_MOUSE
	int	mouse_state, omouse_state, mx, my, x1, x2, y1, y2;
	int	close_x, close_y, close_size;
#endif

	int	i,k,ocitem,citem;
	int	time_stopped = 0;
	int	bRedraw = 0;
	int	nChangeMode = BT_NONE, nPrevMode = BT_NONE;
	bkg	bg;

WIN (char *old_bg_pcx);
WIN (old_bg_pcx = _SCRContext.bkg_filename);
WIN (DEFINE_SCREEN (NULL));

	All_items = items;
	Num_items = nitems;
	memset (&bg, 0, sizeof (bg));
	bg.bIgnoreBg = 1;
	gameStates.menus.nInMenu++;
	memset (start_axis, 0, sizeof (start_axis));

	if (! ((gameData.app.nGameMode & GM_MULTI) && (gameStates.app.nFunctionMode == FMODE_GAME) && (!gameStates.app.bEndLevelSequence)))
	{
		time_stopped = 1;
		StopTime ();
	}

//	if (gameConfig.nControlType == CONTROL_WINJOYSTICK) {
//		WINDOS (
//			joydefsw_win_joyselect (title2); title = title2,
//			Int3 ()
//		);												// Get Samir...
//	}

WINDOS (
	save_canvas = dd_grd_curcanv,
	save_canvas = grdCurCanv
);


WINDOS (
	DDGrSetCurrentCanvas (NULL),
	GrSetCurrentCanvas (NULL)
);		
	save_font = grdCurCanv->cv_font;

#ifdef WINDOWS
KConfigPaint:
#endif
	FlushInput ();
	NMDrawBackground (&bg, xOffs, yOffs, 
		xOffs + 639 /*grdCurCanv->cv_bitmap.bm_props.w - 1*/, 
		yOffs + 479 /*grdCurCanv->cv_bitmap.bm_props.h - 1*/, 0);
   GrPaletteStepLoad (NULL);

citem = 0;
newmenu_show_cursor ();
#ifdef NEWMENU_MOUSE
	mouse_state = omouse_state = 0;
#endif
	while (1) {
	//	Windows addendum to allow for KConfig input.
	#if defined (WINDOWS)
		{
			MSG msg;
			DoMessageStuff (&msg);
			if (_RedrawScreen) {
				_RedrawScreen = FALSE;
				DDGrSetCurrentCanvas (NULL);	
				goto KConfigPaint;
			}
			DDGRRESTORE;
	 	}
	#endif
	do {
		if (gameOpts->menus.nStyle || !bRedraw) {
			bRedraw = 1;
			WIN (DDGRLOCK (dd_grd_curcanv));	
			if (gameStates.app.bGameRunning)
				GameRenderFrame ();
			NMDrawBackground (&bg, xOffs, yOffs, xOffs + 639, yOffs + 479, 1);
			KCDrawTitle (title);
			#ifdef NEWMENU_MOUSE
				close_x = close_y = gameStates.menus.bHires ? 15 : 7;
				close_x += xOffs;
				close_y += yOffs;
				close_size = gameStates.menus.bHires?10:5;
				GrSetColorRGB (0, 0, 0, 255);
				GrRect (close_x, close_y, close_x + close_size, close_y + close_size);
				GrSetColorRGBi (RGBA_PAL2 (21, 21, 21));
				GrRect (close_x + LHX (1), close_y + LHX (1), close_x + close_size - LHX (1), close_y + close_size - LHX (1));
			#endif
			KCDrawHeader (items);
			WIN (DDGRUNLOCK (dd_grd_curcanv));	
			KCDrawTable (items, nitems, citem);
			}
		newmenu_hide_cursor ();
		switch (nChangeMode) {
			case BT_KEY:
				nChangeMode = KCChangeKey (items + citem);
				break;
			case BT_MOUSE_BUTTON:
				nChangeMode = KCChangeMouseButton (items + citem);
				break;
			case BT_MOUSE_AXIS:
				nChangeMode = KCChangeMouseAxis (items + citem);
				break;
			case BT_JOY_BUTTON:
				nChangeMode = KCChangeJoyButton (items + citem);
				break;
			case BT_JOY_AXIS:
				if (nChangeMode != nPrevMode)
					ControlsReadJoystick (start_axis);
				nChangeMode = KCChangeJoyAxis (items + citem);
				break;
			case BT_INVERT:
				nChangeMode = KCChangeInvert (items + citem);
				break;
			default:
				nChangeMode = BT_NONE;
			}
		nPrevMode = nChangeMode;
		newmenu_show_cursor ();
		GrUpdate (0);
		} while (nChangeMode != BT_NONE);
		//see if redbook song needs to be restarted
		songs_check_redbook_repeat ();


		k = KeyInKey ();
		MultiDoFrame();
#ifdef NEWMENU_MOUSE
		omouse_state = mouse_state;
		mouse_state = MouseButtonState (0);
#endif

		if (!time_stopped) {
			#ifdef NETWORK
			if (MultiMenuPoll () == -1)
				k = -2;
			#endif
		}
		ocitem = citem;
		switch (k)	{
		case KEY_BACKSP:
			Int3 ();
			break;
		case KEY_COMMAND+KEY_SHIFTED+KEY_P:
		case KEY_ALTED+KEY_F9:
			SaveScreenShot (NULL, 0);
			break;							
		case KEY_CTRLED+KEY_D:
			items [citem].value = 255;
			KCDrawItem (items + citem, 1);
			break;
		case KEY_CTRLED+KEY_R:	
			if (items==kc_keyboard)	{
				for (i=0; i<NUM_KEY_CONTROLS; i++) {
					items [i].value=controlSettings.defaults [0][i];
					KCDrawItem (items + i, 0);
					}
#ifdef D2X_KEYS
				}
			else if (items == kc_d2x) {
				for (i=0;i<NUM_D2X_CONTROLS;i++) {
					items [i].value=controlSettings.d2xDefaults [i];
					KCDrawItem (items + i, 0);
					}
				}
#endif
			else if (items == kc_mouse) {
				for (i = 0; i < NUM_MOUSE_CONTROLS; i++) {
					items [i].value = controlSettings.defaults [gameConfig.nControlType][i];
					KCDrawItem (items + i, 0);
					}
				}
			else {
				for (i=0; i < NUM_JOY_CONTROLS; i++)	{
					items [i].value = controlSettings.defaults [gameConfig.nControlType][i];
					KCDrawItem (items + i, 0);
					}
				}
			KCDrawItem (items + citem, 1);
			break;
		case KEY_DELETE:
			items [citem].value=255;
			KCDrawItem (items + citem, 1);
			break;
		case KEY_UP: 		
		case KEY_PAD8:
#ifdef TABLE_CREATION
			if (items [citem].u==-1) 
				items [citem].u=FindNextItemUp (items,nitems, citem);
#endif
			citem = items [citem].u; 
			break;
		
		case KEY_DOWN: 	
		case KEY_PAD2:
#ifdef TABLE_CREATION
			if (items [citem].d==-1) 
				items [citem].d=FindNextItemDown (items,nitems, citem);
#endif
			citem = items [citem].d; 
			break;
		case KEY_LEFT: 	
		case KEY_PAD4:
#ifdef TABLE_CREATION
			if (items [citem].l==-1) 
				items [citem].l=FindNextItemLeft (items,nitems, citem);
#endif
			citem = items [citem].l; 
			break;
		case KEY_RIGHT: 	
		case KEY_PAD6:
#ifdef TABLE_CREATION
			if (items [citem].r==-1) 
				items [citem].r=FindNextItemRight (items,nitems, citem);
#endif
			citem = items [citem].r; 
			break;
		case KEY_ENTER:	
		case KEY_PADENTER:	
			nChangeMode = items [citem].type;
			GameFlushInputs ();
			break;
		case -2:	
		case KEY_ESC:
			KCQuitMenu (save_canvas, save_font, &bg, time_stopped);
			return;
#ifdef TABLE_CREATION
		case KEY_DEBUGGED+KEY_F12:	{
			FILE * fp;
#if TRACE		
			con_printf (CON_DEBUG, "start table creation\n");
#endif
			for (i=0; i < NUM_KEY_CONTROLS; i++)	{
				kc_keyboard [i].u = FindNextItemUp (kc_keyboard,NUM_KEY_CONTROLS, i);
				kc_keyboard [i].d = FindNextItemDown (kc_keyboard,NUM_KEY_CONTROLS, i);
				kc_keyboard [i].l = FindNextItemLeft (kc_keyboard,NUM_KEY_CONTROLS, i);
				kc_keyboard [i].r = FindNextItemRight (kc_keyboard,NUM_KEY_CONTROLS, i);
				}
			for (i=0; i < NUM_JOY_CONTROLS; i++)	{
				kc_joystick [i].u = FindNextItemUp (kc_joystick,NUM_JOY_CONTROLS, i);
				kc_joystick [i].d = FindNextItemDown (kc_joystick,NUM_JOY_CONTROLS, i);
				kc_joystick [i].l = FindNextItemLeft (kc_joystick,NUM_JOY_CONTROLS, i);
				kc_joystick [i].r = FindNextItemRight (kc_joystick,NUM_JOY_CONTROLS, i);
				}
			for (i=0; i < NUM_MOUSE_CONTROLS; i++)	{
				kc_mouse [i].u = FindNextItemUp (kc_mouse,NUM_MOUSE_CONTROLS, i);
				kc_mouse [i].d = FindNextItemDown (kc_mouse,NUM_MOUSE_CONTROLS, i);
				kc_mouse [i].l = FindNextItemLeft (kc_mouse,NUM_MOUSE_CONTROLS, i);
				kc_mouse [i].r = FindNextItemRight (kc_mouse,NUM_MOUSE_CONTROLS, i);
				}
			fp = fopen ("KConfig.cod", "wt");
			fprintf (fp, "ubyte controlSettings.defaults [CONTROL_MAX_TYPES][MAX_CONTROLS] = {\n");
			for (i=0; i<CONTROL_MAX_TYPES; i++)	{
				int j;
				fprintf (fp, "{0x%x", controlSettings.custom [i][0]);
				for (j=1; j<MAX_CONTROLS; j++)
					fprintf (fp, ",0x%x", controlSettings.custom [i][j]);
				fprintf (fp, "},\n");
				}
			fprintf (fp, "};\n");

			fprintf (fp, "\nkc_item kc_keyboard [NUM_KEY_CONTROLS] = {\n");
			for (i=0; i<NUM_KEY_CONTROLS; i++)	{
				fprintf (fp, "\t{ %2d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%c%s%c, %s, 255 },\n", 
					kc_keyboard [i].id, kc_keyboard [i].x, kc_keyboard [i].y, kc_keyboard [i].w1, kc_keyboard [i].w2,
					kc_keyboard [i].u, kc_keyboard [i].d, kc_keyboard [i].l, kc_keyboard [i].r,
													 34, kc_keyboard [i].text, 34, btype_text [kc_keyboard [i].type]);
				}
			fprintf (fp, "};");

			fprintf (fp, "\nkc_item kc_joystick [NUM_JOY_CONTROLS] = {\n");
			for (i=0; i<NUM_JOY_CONTROLS; i++)	{
				if (kc_joystick [i].type == BT_JOY_BUTTON)
					fprintf (fp, "\t{ %2d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%c%s%c, %s, 255 },\n", 
						kc_joystick [i].id, kc_joystick [i].x, kc_joystick [i].y, kc_joystick [i].w1, kc_joystick [i].w2,
						kc_joystick [i].u, kc_joystick [i].d, kc_joystick [i].l, kc_joystick [i].r,
														 34, kc_joystick [i].text, 34, btype_text [kc_joystick [i].type]);
				else
					fprintf (fp, "\t{ %2d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%c%s%c, %s, 255 },\n", 
						kc_joystick [i].id, kc_joystick [i].x, kc_joystick [i].y, kc_joystick [i].w1, kc_joystick [i].w2,
						kc_joystick [i].u, kc_joystick [i].d, kc_joystick [i].l, kc_joystick [i].r,
														 34, kc_joystick [i].text, 34, btype_text [kc_joystick [i].type]);
				}
			fprintf (fp, "};");

			fprintf (fp, "\nkc_item kc_mouse [NUM_MOUSE_CONTROLS] = {\n");
			for (i=0; i<NUM_MOUSE_CONTROLS; i++)	{
				fprintf (fp, "\t{ %2d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%c%s%c, %s, 255 },\n", 
					kc_mouse [i].id, kc_mouse [i].x, kc_mouse [i].y, kc_mouse [i].w1, kc_mouse [i].w2,
					kc_mouse [i].u, kc_mouse [i].d, kc_mouse [i].l, kc_mouse [i].r,
													 34, kc_mouse [i].text, 34, btype_text [kc_mouse [i].type]);
				}
			fprintf (fp, "};");
			fclose (fp);
#if TRACE		
			con_printf (CON_DEBUG, "end table creation\n");
#endif
			}
		break;
#endif
		}

#ifdef NEWMENU_MOUSE
		if ((mouse_state && !omouse_state) || (mouse_state && omouse_state)) {
			int item_height;
#if 0
			int b = gameOpts->bInput;
			gameOpts->legacy.bInput = 1;
		   event_poll (SDL_MOUSEEVENTMASK);	//polled in main/KConfig.c:read_bm_all ()
			gameOpts->legacy.bInput = b;
#endif
			mouse_get_pos (&mx, &my);
			mx -= xOffs;
			my -= yOffs;
//			my = (my * 12) / 10;	//y mouse pos is off here, no clue why
			for (i = 0; i < nitems; i++)	{
				item_height = KCGetItemHeight (items + i);
				x1 = grdCurCanv->cv_bitmap.bm_props.x + LHX (items [i].x) + LHX (items [i].w1);
				x2 = x1 + LHX (items [i].w2);
				y1 = grdCurCanv->cv_bitmap.bm_props.y + LHY (items [i].y);
				y2 = y1 + /*LHY*/ (item_height);
				if (((mx > x1) && (mx < x2)) && ((my > y1) && (my < y2))) {
					citem = i;
					break;
				}
			}
		}
		else if (!mouse_state && omouse_state) {
			int item_height;
			
			mouse_get_pos (&mx, &my);
			mx -= xOffs;
			my -= yOffs;
			my = (my * 12) / 10;	//y mouse pos is off here, no clue why
			item_height = KCGetItemHeight (items + citem);
			x1 = grdCurCanv->cv_bitmap.bm_props.x + LHX (items [citem].x) + LHX (items [citem].w1);
			x2 = x1 + LHX (items [citem].w2);
			y1 = grdCurCanv->cv_bitmap.bm_props.y + LHY (items [citem].y);
			y2 = y1 + /*LHY*/ (item_height);
			if (((mx > x1) && (mx < x2)) && ((my > y1) && (my < y2))) {
				nChangeMode = items [citem].type;
				GameFlushInputs ();
			} else {
				x1 = grdCurCanv->cv_bitmap.bm_props.x + close_x + LHX (1);
				x2 = x1 + close_size - LHX (1);
				y1 = grdCurCanv->cv_bitmap.bm_props.y + close_y + LHX (1);
				y2 = y1 + close_size - LHY (1);
				if (((mx > x1) && (mx < x2)) && ((my > y1) && (my < y2))) {
					KCQuitMenu (save_canvas, save_font, &bg, time_stopped);
					return;
				}
			}
		}
#endif // NEWMENU_MOUSE

		if (ocitem!=citem)	{
			newmenu_hide_cursor ();
			KCDrawItem (items + ocitem, 0);
			KCDrawItem (items + citem, 1);
			newmenu_show_cursor ();
		}
	}
KCQuitMenu (save_canvas, save_font, &bg, time_stopped);
}

//------------------------------------------------------------------------------

void KCDrawItemExt (kc_item *item, int is_current, int bRedraw)
{
	int x, w, h, aw;
	char btext [64];
//	PA_DFX (pa_set_frontbuffer_current ();

if (bRedraw && gameOpts->menus.nStyle)
	return;
WIN (DDGRLOCK (dd_grd_curcanv));

	if (is_current)
		GrSetFontColorRGBi (RGBA_PAL2 (20,20,29), 1, 0, 0);
	else
		GrSetFontColorRGBi (RGBA_PAL2 (15,15,24), 1, 0, 0);
   GrString (KC_LHX (item->x), KC_LHY (item->y), item->textId ? GT (item->textId) : item->text);
WIN (DDGRUNLOCK (dd_grd_curcanv));

	*btext = '\0';
	if (item->value != 255) {
		switch (item->type)	{
			case BT_KEY:
				strncat (btext, key_text [item->value], 10); 
				break;

			case BT_MOUSE_BUTTON:
				//strncpy (btext, baseGameTexts [mousebutton_text [item->value]], 10); break;
				strncpy (btext, MouseTextString (item->value), 10); 
				break;

			case BT_MOUSE_AXIS:
				strncpy (btext, baseGameTexts [mouseaxis_text [item->value]], 10); 
				break;

			case BT_JOY_BUTTON:
#ifdef USE_LINUX_JOY
				sprintf (btext, "J%d B%d", 
						  j_button [item->value].joydev, j_Get_joydev_button_number (item->value);
#elif defined (WINDOWS)
				if (joybutton_text [item->value % MAX_BUTTONS_PER_JOYSTICK] != -1) 
					strncpy (btext, baseGameTexts [joybutton_text [item->value % MAX_BUTTONS_PER_JOYSTICK]], 10);
				else 
					sprintf (btext, "BTN%2d", item->value+1);
#elif 1//defined (_WIN32)
				{
					int	nStick = item->value / MAX_BUTTONS_PER_JOYSTICK;
					int	nBtn = item->value % MAX_BUTTONS_PER_JOYSTICK;
					int	nHat = SDL_Joysticks [nStick].n_buttons;
					//static char szHatDirs [4] = {'U', 'L', 'D', 'R'};
					static char cHatDirs [4] = { (char) 130, (char) 127, (char) 128, (char) 129};

				if (nBtn < nHat)
					sprintf (btext, "J%d B%d", nStick + 1, nBtn + 1);
				else
					sprintf (btext, "HAT%d%c", nStick + 1, cHatDirs [nBtn - nHat]);
				}
#else
				if (JOYBUTTON_TEXT (item->value) !=-1)
					strncpy (btext, baseGameTexts [JOYBUTTON_TEXT (item->value)], 10);
				else
					sprintf (btext, "BTN%d", item->value);
#endif
				break;

			case BT_JOY_AXIS:
#if defined (USE_LINUX_JOY)
				sprintf (btext, "J%d A%d", j_axis [item->value].joydev, j_Get_joydev_axis_number (item->value));
#elif 1//defined (_WIN32)
				{
					int	nStick = item->value / MAX_AXES_PER_JOYSTICK;
					int	nAxis = item->value % MAX_AXES_PER_JOYSTICK;
					static char	cAxis [4] = {'X', 'Y', 'Z', 'R'};

				if (nAxis < 4)
					sprintf (btext, "J%d %c", nStick + 1, cAxis [nAxis]);
				else
					sprintf (btext, "J%d A%d", nStick + 1, nAxis + 1);
				}
#else
				strncpy (btext, baseGameTexts [JOYAXIS_TEXT (item->value)], 10);
#endif
				break;

			case BT_INVERT:
				strncpy (btext, baseGameTexts [invert_text [item->value]], 10); 
				break;
		}
	}
	if (item->w1) {
	WIN (DDGRLOCK (dd_grd_curcanv));
		GrGetStringSize (btext, &w, &h, &aw);

		if (is_current)
			GrSetColorRGBi (RGBA_PAL2 (21, 0, 24));
		else
			GrSetColorRGBi (RGBA_PAL2 (16, 0, 19));
		GrURect (KC_LHX (item->w1+item->x), KC_LHY (item->y-1), 
					KC_LHX (item->w1+item->x+item->w2), KC_LHY (item->y)+h);
		GrSetFontColorRGBi (RGBA_PAL2 (28, 28, 28), 1, 0, 0);
		x = LHX (item->w1+item->x)+ ((LHX (item->w2)-w)/2)+xOffs;
		GrString (x, KC_LHY (item->y), btext);
//		PA_DFX (pa_set_backbuffer_current ();

	WIN (DDGRUNLOCK (dd_grd_curcanv));
	}
}

//------------------------------------------------------------------------------

#include "screens.h"

void KConfig (int n, char * title)
{
	int i;
	grs_bitmap *bmSave;
	int	b = gameOpts->legacy.bInput;

	xOffs = (grdCurCanv->cv_bitmap.bm_props.w - 640) / 2;
	yOffs = (grdCurCanv->cv_bitmap.bm_props.h - 480) / 2;
	if (xOffs < 0)
		xOffs = 0;
	if (yOffs < 0)
		yOffs = 0;

	gameOpts->legacy.bInput = 1;
	SetScreenMode (SCREEN_MENU);
	KCSetControls ();
	//save screen
	WIN (mouse_set_mode (0));
	WIN (DDGrSetCurrentCanvas (NULL));
#if defined (POLY_ACC)
	bmSave = GrCreateBitmap2 (grdCurCanv->cv_bitmap.bm_props.w, grdCurCanv->cv_bitmap.bm_props.h, 
										grdCurCanv->cv_bitmap.bm_props.type, NULL);
#else
	bmSave = GrCreateBitmap (grdCurCanv->cv_bitmap.bm_props.w, grdCurCanv->cv_bitmap.bm_props.h, 0);
#endif
	Assert (bmSave != NULL);
	bmSave->bm_palette = gameData.render.ogl.palette;
	WIN (DDGRLOCK (dd_grd_curcanv));
	GrBmBitBlt (grdCurCanv->cv_bitmap.bm_props.w, grdCurCanv->cv_bitmap.bm_props.w, 
					 0, 0, 0, 0, &grdCurCanv->cv_bitmap, bmSave);
	WIN (DDGRUNLOCK (dd_grd_curcanv));
	switch (n)	{
	case 0:
		KConfigSub (kc_keyboard, NUM_KEY_CONTROLS, title);
		break;
	case 1:
		KConfigSub (kc_joystick, NUM_JOY_CONTROLS, title);
		break;
	case 2:
		KConfigSub (kc_mouse, NUM_MOUSE_CONTROLS, title); 
		break;
#if 0
	case 3:
		KConfigSub (kc_superjoy, NUM_JOY_CONTROLS, title); 
		break;
#endif
#ifdef D2X_KEYS
	//added on 2/4/99 by Victor Rachels for new keys menu
	case 4:
		KConfigSub (kc_d2x, NUM_D2X_CONTROLS, title); 
		break;
	//end this section addition - VR
#endif
 	default:
		Int3 ();
		gameOpts->legacy.bInput = b;
		return;
	}

	//restore screen
	WIN (mouse_set_mode (1));
	WIN (DDGrSetCurrentCanvas (NULL));
	WIN (DDGRLOCK (dd_grd_curcanv));
	GrBitmap (xOffs, yOffs, bmSave);
	WIN (DDGRUNLOCK (dd_grd_curcanv));
	GrFreeBitmap (bmSave);
	ResetCockpit ();		//force cockpit redraw next time
	// Update save values...
	for (i = 0; i < NUM_KEY_CONTROLS; i++)	
		controlSettings.custom [0][i] = kc_keyboard [i].value;
	if (gameOpts->input.bUseJoystick) { 
		for (i = 0; i < NUM_JOY_CONTROLS; i++)	
			controlSettings.custom [gameStates.input.nJoyType][i] = kc_joystick [i].value;
		}
	if (gameOpts->input.bUseMouse) {
		for (i = 0; i < NUM_MOUSE_CONTROLS; i++)	
			controlSettings.custom [gameStates.input.nMouseType][i] = kc_mouse [i].value;
		}
	if (gameConfig.nControlType == CONTROL_WINJOYSTICK) {
		for (i = 0; i < NUM_JOY_CONTROLS; i++)	
			controlSettings.custom [gameConfig.nControlType][i] = kc_superjoy [i].value;
	}

#ifdef D2X_KEYS
	for (i=0; i < NUM_D2X_CONTROLS; i++)
		controlSettings.d2xCustom [i] = kc_d2x [i].value;
#endif
gameOpts->legacy.bInput = b;
}

//------------------------------------------------------------------------------

fix Last_angles_p = 0;
fix Last_angles_b = 0;
fix Last_angles_h = 0;
ubyte Last_angles_read = 0;

extern int			VR_sensitivity;
						
int VR_sense_range [3] = { 25, 50, 75 };

#if 0
read_head_tracker ()
{
#ifndef WINDOWS

	fix yaw, pitch, roll;
	int buttons;

//------ read vfx1 helmet --------
	if (vfx1_installed) {
		vfx_get_data (&yaw,&pitch,&roll,&buttons);
	} else if (iglasses_headset_installed)	{
		iglasses_read_headset (&yaw, &pitch, &roll);
	} else if (Victor_headset_installed)   {
		victor_read_headset_filtered (&yaw, &pitch, &roll);
	} else {
		return;
	}

	bUsePlayerHeadAngles = 0;
	if (Last_angles_read)	{
		fix yaw1 = yaw;
		
		yaw1 = yaw;
		if ((Last_angles_h < (F1_0/4)) && (yaw > ((F1_0*3)/4)))	
			yaw1 -= F1_0;
		else if ((yaw < (F1_0/4)) && (Last_angles_h > ((F1_0*3)/4)))	
			yaw1 += F1_0;
	
		Controls.pitch_time	+= FixMul ((pitch- Last_angles_p)*VR_sense_range [VR_sensitivity],gameData.time.xFrame);
		Controls.heading_time+= FixMul ((yaw1 -  Last_angles_h)*VR_sense_range [VR_sensitivity],gameData.time.xFrame);
		Controls.bank_time	+= FixMul ((roll - Last_angles_b)*VR_sense_range [VR_sensitivity],gameData.time.xFrame);
	}
	Last_angles_read = 1;
	Last_angles_p = pitch;
	Last_angles_h = yaw;
	Last_angles_b = roll;
#endif
}
#endif

//------------------------------------------------------------------------------

ubyte 			kc_use_external_control = 0;
ubyte				kc_enable_external_control = 0;
ubyte 			kc_external_intno = 0;
ext_control_info	*kc_external_control = NULL;
ubyte				*kc_external_name = NULL;
ubyte				kc_external_version = 0;

void KCInitExternalControls (int intno, int address)
{
	int i;
	kc_external_intno = intno;
	kc_external_control	= (ext_control_info *) (size_t) address;
	kc_use_external_control = 1;
	kc_enable_external_control  = 1;

	i = FindArg ("-xname");
	if (i)	
		kc_external_name = Args [i+1];
	else
		kc_external_name = "External Controller";

   for (i=0;i<(int) strlen (kc_external_name);i++)
    if (kc_external_name [i]=='_')
	  kc_external_name [i]=' '; 

	i = FindArg ("-xver");
	if (i)
		kc_external_version = atoi (Args [i+1]);
}

/*void KCReadExternalControls ()
{
	union REGS r;

	if (!kc_enable_external_control && !gameStates.input.bCybermouseActive) 
		return;

	if (kc_external_version == 0) 
		memset (kc_external_control, 0, sizeof (control_info);
	else if (kc_external_version > 0) 	{
		memset (kc_external_control, 0, sizeof (control_info)+sizeof (vms_angvec) + 64);
		if (kc_external_version > 1) {
			// Write ship pos and angles to external controls...
			ubyte *temp_ptr = (ubyte *)kc_external_control;
			vms_vector *ship_pos;
			vms_matrix *ship_orient;
			memset (kc_external_control, 0, sizeof (control_info)+sizeof (vms_angvec) + 64 + sizeof (vms_vector)+sizeof (vms_matrix);
			temp_ptr += sizeof (control_info)+sizeof (vms_angvec) + 64;
			ship_pos = (vms_vector *)temp_ptr;
			temp_ptr += sizeof (vms_vector);
			ship_orient = (vms_matrix *)temp_ptr;
			// Fill in ship postion...
			*ship_pos = gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].pos;
			// Fill in ship orientation...
			*ship_orient = gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient;
		}
	}

        if (gameStates.app.bAutoMap)                    // (If in automap...)
		kc_external_control->automap_state = 1;
	memset (&r,0,sizeof (r);

#ifndef WINDOWS
  
   if (!gameStates.input.bCybermouseActive)
   	int386 (kc_external_intno, &r, &r);		// Read external info...
//	else
  //		ReadOWL (kc_external_control);

#endif

	if (gameData.multi.nLocalPlayer > -1)	{
		gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].mtype.phys_info.flags &= (~PF_TURNROLL);	// Turn off roll when turning
		gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].mtype.phys_info.flags &= (~PF_LEVELLING);	// Turn off leveling to nearest side.
		gameOpts->gameplay.bAutoLeveling = 0;

		if (kc_external_version > 0) {		
			vms_matrix tempm, ViewMatrix;
			vms_angvec * Kconfig_abs_movement;
			char * oem_message;
	
			Kconfig_abs_movement = (vms_angvec *) ((uint)kc_external_control + sizeof (control_info);
	
			if (Kconfig_abs_movement->p || Kconfig_abs_movement->b || Kconfig_abs_movement->h)	{
				VmAngles2Matrix (&tempm,Kconfig_abs_movement);
				VmMatMul (&ViewMatrix,&gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient,&tempm);
				gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient = ViewMatrix;		
			}
			oem_message = (char *) ((uint)Kconfig_abs_movement + sizeof (vms_angvec);
			if (oem_message [0] != '\0')
				HUDInitMessage (oem_message);
		}
	}

	Controls.pitch_time += FixMul (kc_external_control->pitch_time,gameData.time.xFrame);						
	Controls.vertical_thrust_time += FixMul (kc_external_control->vertical_thrust_time,gameData.time.xFrame);
	Controls.heading_time += FixMul (kc_external_control->heading_time,gameData.time.xFrame);
	Controls.sideways_thrust_time += FixMul (kc_external_control->sideways_thrust_time ,gameData.time.xFrame);
	Controls.bank_time += FixMul (kc_external_control->bank_time ,gameData.time.xFrame);
	Controls.forward_thrust_time += FixMul (kc_external_control->forward_thrust_time ,gameData.time.xFrame);
	Controls.rear_viewDownCount += kc_external_control->rear_viewDownCount;	
	Controls.rear_view_down_state |= kc_external_control->rear_view_down_state;	
	Controls.fire_primaryDownCount += kc_external_control->fire_primaryDownCount;
	Controls.fire_primary_state |= kc_external_control->fire_primary_state;
	Controls.fire_secondary_state |= kc_external_control->fire_secondary_state;
	Controls.fire_secondaryDownCount += kc_external_control->fire_secondaryDownCount;
	Controls.fire_flareDownCount += kc_external_control->fire_flareDownCount;
	Controls.drop_bombDownCount += kc_external_control->drop_bombDownCount;	
	Controls.automapDownCount += kc_external_control->automapDownCount;
	Controls.automap_state |= kc_external_control->automap_state;
} */

//------------------------------------------------------------------------------

void KCReadExternalControls ()
{
	//union REGS r;
   int i;

	if (!kc_enable_external_control) return;

	if (kc_external_version == 0) 
		memset (kc_external_control, 0, sizeof (ext_control_info));
	else if (kc_external_version > 0) 	{
    	
		if (kc_external_version>=4)
			memset (kc_external_control, 0, sizeof (advanced_ext_control_info));
      else if (kc_external_version>0)     
			memset (kc_external_control, 0, sizeof (ext_control_info)+sizeof (vms_angvec) + 64);
		else if (kc_external_version>2)
			memset (kc_external_control, 0, sizeof (ext_control_info)+sizeof (vms_angvec) + 64 + sizeof (vms_vector) + sizeof (vms_matrix) +4);

		if (kc_external_version > 1) {
			// Write ship pos and angles to external controls...
			ubyte *temp_ptr = (ubyte *)kc_external_control;
			vms_vector *ship_pos;
			vms_matrix *ship_orient;
			memset (kc_external_control, 0, sizeof (ext_control_info)+sizeof (vms_angvec) + 64 + sizeof (vms_vector)+sizeof (vms_matrix));
			temp_ptr += sizeof (ext_control_info) + sizeof (vms_angvec) + 64;
			ship_pos = (vms_vector *)temp_ptr;
			temp_ptr += sizeof (vms_vector);
			ship_orient = (vms_matrix *)temp_ptr;
			// Fill in ship postion...
			*ship_pos = gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].pos;
			// Fill in ship orientation...
			*ship_orient = gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient;
		}
    if (kc_external_version>=4)
	  {
	   advanced_ext_control_info *temp_ptr= (advanced_ext_control_info *)kc_external_control;
 
      temp_ptr->headlight_state= (gameData.multi.players [gameData.multi.nLocalPlayer].flags & PLAYER_FLAGS_HEADLIGHT_ON);
		temp_ptr->primary_weapon_flags=gameData.multi.players [gameData.multi.nLocalPlayer].primary_weapon_flags;
		temp_ptr->secondary_weapon_flags=gameData.multi.players [gameData.multi.nLocalPlayer].secondary_weapon_flags;
      temp_ptr->current_primary_weapon=gameData.weapons.nPrimary;
      temp_ptr->current_secondary_weapon=gameData.weapons.nSecondary;

      temp_ptr->current_guidebot_command=gameData.escort.nGoalObject;

	   temp_ptr->force_vector=ExtForceVec;
		temp_ptr->force_matrix=ExtApplyForceMatrix;
	   for (i=0;i<3;i++)
       temp_ptr->joltinfo [i]=ExtJoltInfo [i];  
      for (i=0;i<2;i++)
		   temp_ptr->x_vibrate_info [i]=ExtXVibrateInfo [i];
		temp_ptr->x_vibrate_clear=ExtXVibrateClear;
 	   temp_ptr->game_status=gameStates.app.nExtGameStatus;
   
      memset ((void *)&ExtForceVec,0,sizeof (vms_vector));
      memset ((void *)&ExtApplyForceMatrix,0,sizeof (vms_matrix));
      
      for (i=0;i<3;i++)
		 ExtJoltInfo [i]=0;
      for (i=0;i<2;i++)
		 ExtXVibrateInfo [i]=0;
      ExtXVibrateClear=0;
     }
	}

	if (gameStates.app.bAutoMap)			// (If in automap...)
		kc_external_control->automap_state = 1;
	//memset (&r,0,sizeof (r);

  #if 0
 
	int386 (kc_external_intno, &r, &r);		// Read external info...

  #endif 

	if (gameData.multi.nLocalPlayer > -1)	{
		gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].mtype.phys_info.flags &= (~PF_TURNROLL);	// Turn off roll when turning
		gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].mtype.phys_info.flags &= (~PF_LEVELLING);	// Turn off leveling to nearest side.
		gameOpts->gameplay.bAutoLeveling = 0;

		if (kc_external_version > 0) {		
			vms_matrix tempm, ViewMatrix;
			vms_angvec * Kconfig_abs_movement;
			char * oem_message;
	
			Kconfig_abs_movement = (vms_angvec *) (size_t) ((size_t) kc_external_control + sizeof (ext_control_info));
	
			if (Kconfig_abs_movement->p || Kconfig_abs_movement->b || Kconfig_abs_movement->h)	{
				VmAngles2Matrix (&tempm,Kconfig_abs_movement);
				VmMatMul (&ViewMatrix,&gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient,&tempm);
				gameData.objs.objects [gameData.multi.players [gameData.multi.nLocalPlayer].objnum].orient = ViewMatrix;		
			}
			oem_message = (char *) (size_t) ((size_t)Kconfig_abs_movement + sizeof (vms_angvec));
			if (oem_message [0] != '\0')
				HUDInitMessage (oem_message);
		}
	}

	Controls.pitch_time += FixMul (kc_external_control->pitch_time,gameData.time.xFrame);						
	Controls.vertical_thrust_time += FixMul (kc_external_control->vertical_thrust_time,gameData.time.xFrame);
	Controls.heading_time += FixMul (kc_external_control->heading_time,gameData.time.xFrame);
	Controls.sideways_thrust_time += FixMul (kc_external_control->sideways_thrust_time ,gameData.time.xFrame);
	Controls.bank_time += FixMul (kc_external_control->bank_time ,gameData.time.xFrame);
	Controls.forward_thrust_time += FixMul (kc_external_control->forward_thrust_time ,gameData.time.xFrame);
	Controls.rear_viewDownCount += kc_external_control->rear_viewDownCount;	
	Controls.rear_view_down_state |= kc_external_control->rear_view_down_state;	
	Controls.fire_primaryDownCount += kc_external_control->fire_primaryDownCount;
	Controls.fire_primary_state |= kc_external_control->fire_primary_state;
	Controls.fire_secondary_state |= kc_external_control->fire_secondary_state;
	Controls.fire_secondaryDownCount += kc_external_control->fire_secondaryDownCount;
	Controls.fire_flareDownCount += kc_external_control->fire_flareDownCount;
	Controls.drop_bombDownCount += kc_external_control->drop_bombDownCount;	
	Controls.automapDownCount += kc_external_control->automapDownCount;
	Controls.automap_state |= kc_external_control->automap_state;
	
   if (kc_external_version>=3)
	 {
		ubyte *temp_ptr = (ubyte *)kc_external_control;
		temp_ptr += (sizeof (ext_control_info) + sizeof (vms_angvec) + 64 + sizeof (vms_vector) + sizeof (vms_matrix));
  
	   if (* (temp_ptr))
		 Controls.cycle_primary_count= (* (temp_ptr));
	   if (* (temp_ptr+1))
		 Controls.cycle_secondary_count= (* (temp_ptr+1));

		if (* (temp_ptr+2))
		 Controls.afterburner_state= (* (temp_ptr+2));
		if (* (temp_ptr+3))
		 Controls.headlight_count= (* (temp_ptr+3));
  	 }
   if (kc_external_version>=4)
	 {
     int i;
	  advanced_ext_control_info *temp_ptr= (advanced_ext_control_info *)kc_external_control;
     
     for (i=0;i<128;i++)
	   if (temp_ptr->keyboard [i])
			key_putkey ((char) i);

     if (temp_ptr->Reactor_blown)
      {
       if (gameData.app.nGameMode & GM_MULTI)
		    NetDestroyReactor (ObjFindFirstOfType (OBJ_CNTRLCEN));
		 else
			 DoReactorDestroyedStuff (ObjFindFirstOfType (OBJ_CNTRLCEN));
	   }
    }
  
}

//------------------------------------------------------------------------------

void KCSetControls ()
{
	int i;

SetControlType ();
for (i=0; i < NUM_KEY_CONTROLS; i++)	
	kc_keyboard [i].value = controlSettings.custom [0][i];

//if ((gameConfig.nControlType > 0) && (gameConfig.nControlType < 5)) {
if (gameOpts->input.bUseJoystick) {
	for (i = 0; i < NUM_JOY_CONTROLS; i++) {
		kc_joystick [i].value = controlSettings.custom [gameStates.input.nJoyType][i];
		if (kc_joystick [i].type == BT_INVERT)	{
			if (kc_joystick [i].value != 1)
				kc_joystick [i].value = 0;
			controlSettings.custom [gameStates.input.nJoyType][i] = kc_joystick [i].value;
			}
		}
	}
//else if (gameConfig.nControlType > 4 && gameConfig.nControlType < CONTROL_WINJOYSTICK) {
if (gameOpts->input.bUseMouse) {
	for (i=0; i < NUM_MOUSE_CONTROLS; i++)	{
		kc_mouse [i].value = controlSettings.custom [gameStates.input.nMouseType][i];
		if (kc_mouse [i].type == BT_INVERT)	{
			if (kc_mouse [i].value != 1)
				kc_mouse [i].value = 0;
			controlSettings.custom [gameStates.input.nMouseType][i] = kc_mouse [i].value;
			}
		}
	}
//else 
if (gameConfig.nControlType == CONTROL_WINJOYSTICK) {
	for (i=0; i<NUM_JOY_CONTROLS; i++) {
		kc_superjoy [i].value = controlSettings.custom [gameConfig.nControlType][i];
		if (kc_superjoy [i].type == BT_INVERT)	{
			if (kc_superjoy [i].value!=1)
				kc_superjoy [i].value	= 0;
			controlSettings.custom [gameConfig.nControlType][i] = kc_superjoy [i].value;
		}
	}
}

#ifdef D2X_KEYS
	for (i=0; i<NUM_D2X_CONTROLS; i++)
		kc_d2x [i].value = controlSettings.d2xCustom [i];
#endif
}

//------------------------------------------------------------------------------
//eof
