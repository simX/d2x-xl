/* $Id: joydefs.c,v 1.2 2003/10/10 09:36:35 btb Exp $ */
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
 * Joystick Settings Stuff
 *
 * Old Log:
 * Revision 1.10  1995/10/18  22:21:21  allender
 * fixed bug with gravis mousestick and call KeyFlush when
 * calibrating joystick since it used keystrokes for the tTrigger
 * (at least most of them do)
 *
 * Revision 1.9  1995/10/17  13:12:32  allender
 * fixed up controller support for mac world
 *
 * Revision 1.8  1995/10/15  19:27:04  allender
 * new Dave Denhart controller code
 *
 * Revision 1.7  1995/10/15  16:14:59  allender
 * fixed axis value for Thrustmaster rudders
 *
 * Revision 1.6  1995/09/13  11:39:39  allender
 * made joystick cal menu all text so buttons will work properly
 *
 * Revision 1.5  1995/09/01  13:13:13  allender
 * added close box on controls menu
 *
 * Revision 1.4  1995/08/18  10:22:47  allender
 * if thrustmaster choosen, set joystick at thrustmaster nType
 * for proper reading in joyc.c
 *
 * Revision 1.3  1995/07/26  17:00:34  allender
 * put back in code to make joysticks work
 *
 * Revision 1.2  1995/07/17  08:52:21  allender
 * put back in code that was taken out previously
 *
 * Revision 1.1  1995/05/16  15:26:48  allender
 * Initial revision
 *
 * Revision 2.2  1995/06/30  12:30:22  john
 * Added -Xname command line.
 *
 * Revision 2.1  1995/04/06  12:13:20  john
 * Made so you can calibrate Gravis Gamepad.
 *
 * Revision 2.0  1995/02/27  11:30:27  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.71  1995/02/12  02:06:10  john
 * Fixed bug with joystick incorrectly asking for
 * calibration.
 *
 * Revision 1.70  1995/01/28  15:58:07  john
 * Made joystick calibration be only when wrong detected in
 * menu or joystick axis changed.
 *
 * Revision 1.69  1995/01/25  14:37:55  john
 * Made joystick only prompt for calibration once...
 *
 * Revision 1.68  1995/01/24  16:34:29  john
 * Made so that if you reconfigure joystick and
 * add or subtract an axis, it asks for a recalibration
 * upon leaving.
 *
 * Revision 1.67  1994/12/29  11:08:51  john
 * Fixed Thrustmaster and Logitech Wingman extreme
 * Hat by reading the y2 axis during the center stage
 * of the calibration, and using 75, 50, 27, and 3 %
 * as values for the 4 positions.
 *
 * Revision 1.66  1994/12/15  18:17:39  john
 * Fixed warning with previous.
 *
 * Revision 1.65  1994/12/15  18:15:48  john
 * Made the joy cal only write the .cfg file, not
 * the tPlayer file.
 *
 * Revision 1.64  1994/12/13  14:43:35  john
 * Took out the code in KConfig to build direction array.
 * Called KCSetControls after selecting a new control nType.
 *
 * Revision 1.63  1994/12/10  12:08:47  john
 * Changed some delays to use TICKER instead of TimerGetFixedSeconds.
 *
 * Revision 1.62  1994/12/09  11:01:07  mike
 * force calibration of joystick on joystick selection from Controls [0]... menu.
 *
 * Revision 1.61  1994/12/07  21:50:27  john
 * Put stop/start time around joystick delay.
 *
 * Revision 1.60  1994/12/07  19:34:39  john
 * Added delay.
 *
 * Revision 1.59  1994/12/07  18:12:14  john
 * NEatened up joy cal.,
 *
 * Revision 1.58  1994/12/07  17:07:51  john
 * Fixed up joy cal.
 *
 * Revision 1.57  1994/12/07  16:48:53  yuan
 * localization
 *
 * Revision 1.56  1994/12/07  16:05:55  john
 * Changed the way joystick calibration works.
 *
 * Revision 1.55  1994/12/06  20:15:22  john
 * Took out code that unpauses songs that were never paused.
 *
 * Revision 1.54  1994/12/06  15:14:09  yuan
 * Localization
 *
 * Revision 1.53  1994/12/05  16:29:16  john
 * Took out music pause around the cheat menu.
 *
 * Revision 1.52  1994/12/04  12:39:10  john
 * MAde so that FCS calibration doesn't ask for axis #2.
 *
 * Revision 1.51  1994/12/03  15:14:59  john
 * Took out the delay mentioned previosuly cause it would
 * cause bigger problems than it helps, especially with netgames.
 *
 * Revision 1.50  1994/12/03  14:16:14  john
 * Put a delay between screens in joy cal to keep Yuan from
 * double hitting.
 *
 * Revision 1.49  1994/12/03  11:04:06  john
 * Changed newmenu code a bit to fix bug with bogus
 * backgrounds occcasionally.
 *
 * Revision 1.48  1994/12/02  11:03:44  yuan
 * Localization.
 *
 * Revision 1.47  1994/12/02  10:50:33  yuan
 * Localization
 *
 * Revision 1.46  1994/12/01  12:21:59  john
 * Added code to calibrate 2 joysticks separately.
 *
 * Revision 1.45  1994/12/01  11:52:31  john
 * Added message when you select FCS to say that if
 * you have WCS, see manuel.
 *
 * Revision 1.44  1994/11/29  02:26:28  john
 * Made the prompts for upper-left, lower right for joy
 * calibration more obvious.
 *
 * Revision 1.43  1994/11/26  13:13:59  matt
 * Changed "none" option to "keyboard only"
 *
 * Revision 1.42  1994/11/21  19:35:13  john
 * Replaced calls to joy_init with if (joy_present)
 *
 * Revision 1.41  1994/11/21  19:28:34  john
 * Changed warning for no joystick to use ExecMessageBox..
 *
 * Revision 1.40  1994/11/21  19:06:25  john
 * Made it so that it only stops sound when your in game mode.
 *
 * Revision 1.39  1994/11/21  11:47:18  john
 * Made sound pause during joystick calibration.
 *
 * Revision 1.38  1994/11/10  20:34:18  rob
 * Removed menu-specific network mode support in favor in new stuff
 * in newmenu.c
 *
 * Revision 1.37  1994/11/08  21:21:38  john
 * Made Esc exit joystick calibration.
 *
 * Revision 1.36  1994/11/08  15:14:42  john
 * Added more calls so net doesn't die in net game.
 *
 * Revision 1.35  1994/11/08  14:59:12  john
 * Added code to respond to network while in menus.
 *
 * Revision 1.34  1994/10/24  19:56:32  john
 * Made the new user setup prompt for config options.
 *
 * Revision 1.33  1994/10/22  14:11:52  mike
 * Suppress compiler warning message.
 *
 * Revision 1.32  1994/10/19  12:44:24  john
 * Added hours field to tPlayer structure.
 *
 * Revision 1.31  1994/10/17  13:07:13  john
 * Moved the descent.cfg info into the tPlayer config file.
 *
 * Revision 1.30  1994/10/13  21:41:12  john
 * MAde Esc exit out of joystick calibration.
 *
 * Revision 1.29  1994/10/13  19:22:27  john
 * Added separate config saves for different devices.
 * Made all the devices work together better, such as mice won't
 * get read when you're playing with the joystick.
 *
 * Revision 1.28  1994/10/13  11:40:18  john
 * Took out warnings.
 *
 * Revision 1.27  1994/10/13  11:35:23  john
 * Made Thrustmaster FCS Hat work.  Put a background behind the
 * keyboard configure.  Took out turn_sensitivity.  Changed sound/config
 * menu to new menu. Made F6 be calibrate joystick.
 *
 * Revision 1.26  1994/10/11  21:29:03  matt
 * Made a bunch of menus have good initial selected values
 *
 * Revision 1.25  1994/10/11  17:08:39  john
 * Added sliders for volume controls.
 *
 * Revision 1.24  1994/10/10  17:59:21  john
 * Neatend previous.
 *
 * Revision 1.23  1994/10/10  17:57:59  john
 * Neatend previous.
 *
 * Revision 1.22  1994/10/10  17:56:11  john
 * Added messagebox that tells that config has been saved.
 *
 * Revision 1.21  1994/09/30  12:37:26  john
 * Added midi,digi volume to configuration.
 *
 * Revision 1.20  1994/09/22  16:14:14  john
 * Redid intro sequecing.
 *
 * Revision 1.19  1994/09/19  18:50:15  john
 * Added switch to disable joystick.
 *
 * Revision 1.18  1994/09/12  11:47:36  john
 * Made stupid cruise work better.  Make KConfig values get
 * read/written to disk.
 *
 * Revision 1.17  1994/09/10  15:46:47  john
 * First version of new keyboard configuration.
 *
 * Revision 1.16  1994/09/06  19:35:44  john
 * Fixed bug that didn';t load new size .cal file.
 *
 * Revision 1.15  1994/09/06  14:51:58  john
 * Added sensitivity adjustment, fixed bug with joystick button not
 * staying down.
 *
 * Revision 1.14  1994/09/02  16:13:47  john
 * Made keys fill in position.
 *
 * Revision 1.13  1994/08/31  17:58:50  john
 * Made a bit simpler.
 *
 * Revision 1.12  1994/08/31  14:17:54  john
 * *** empty log message ***
 *
 * Revision 1.11  1994/08/31  14:10:56  john
 * Made keys not work when KEY_DELETE pressed.
 *
 * Revision 1.10  1994/08/31  13:40:47  mike
 * Change constant
 *
 * Revision 1.9  1994/08/31  12:56:27  john
 * *** empty log message ***
 *
 * Revision 1.8  1994/08/30  20:38:29  john
 * Add more config stuff..
 *
 * Revision 1.7  1994/08/30  16:37:25  john
 * Added menu options to set controls.
 *
 * Revision 1.6  1994/08/30  09:27:18  john
 * *** empty log message ***
 *
 * Revision 1.5  1994/08/30  09:12:01  john
 * *** empty log message ***
 *
 * Revision 1.4  1994/08/29  21:18:32  john
 * First version of new keyboard/oystick remapping stuff.
 *
 * Revision 1.3  1994/08/24  19:00:29  john
 * Changed KeyDownTime to return fixed seconds instead of
 * milliseconds.
 *
 * Revision 1.2  1994/08/17  16:50:37  john
 * Added damaging fireballs, missiles.
 *
 * Revision 1.1  1994/08/17  10:07:12  john
 * Initial revision
 *
 *
 */

#ifdef RCS
static char rcsid[] = "$Id: joydefs.c,v 1.2 2003/10/10 09:36:35 btb Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if 0
#include <dos.h>
#endif

#include "mono.h"
#include "key.h"
#include "joy.h"
#include "timer.h"
#include "error.h"

#include "inferno.h"
#include "game.h"
#include "object.h"
#include "player.h"

#include "controls.h"
#include "joydefs.h"
#if 0
#include "victor.h"
#endif
#include "render.h"
#include "palette.h"
#include "newmenu.h"
#include "args.h"
#include "text.h"
#include "kconfig.h"
#include "digi.h"
#include "playsave.h"

int joydefs_calibrateFlag = 0;

#ifdef MACINTOSH
ubyte joydefs_calibrating = 0;		// stupid hack all because of silly mouse cursor emulation
#endif

void joy_delay()
{
	StopTime();
	timer_delay(.25);
	//delay(250);				// changed by allender because	1) more portable
							//								2) was totally broken on PC
	JoyFlush();
	StartTime();
}


int joycal_message( char * title, char * text )
{
	int i;
	tMenuItem	m[2];
	MAC(joydefs_calibrating = 1;)
	memset (m, 0, sizeof (m));
	m[0].nType = NM_TYPE_TEXT; 
	m[0].text = text;
	m[1].nType = NM_TYPE_MENU; 
	m[1].text = TXT_OK;
	i = ExecMenu( title, NULL, 2, m, NULL );
	MAC(joydefs_calibrating = 0;)
	if ( i < 0 )
		return 1;
	return 0;
}

extern int WriteConfigFile();

void JoyDefsCalibrate(void)
{
	#ifndef MACINTOSH
	if ( (gameConfig.nControlType!=CONTROL_JOYSTICK) && (gameConfig.nControlType!=CONTROL_FLIGHTSTICK_PRO) && (gameConfig.nControlType!=CONTROL_THRUSTMASTER_FCS) )
		return;
	#else
	if ( (gameConfig.nControlType == CONTROL_NONE) || (gameConfig.nControlType == CONTROL_MOUSE) )
		return;
	#endif

	PaletteSave();
	ApplyModifiedPalette();
	ResetPaletteAdd();

	GrPaletteStepLoad (NULL);
#if 0
	joydefs_calibrate2();
#endif

	ResetCockpit();

	PaletteRestore();

}

#if 0
#ifndef MACINTOSH
void joydefs_calibrate2()
{
	ubyte masks;
	int org_axis_min[4];
	int org_axis_center[4];
	int org_axis_max[4];

	int axis_min[4] = { 0, 0, 0, 0 };
	int axis_cen[4] = { 0, 0, 0, 0 };
	int axis_max[4] = { 0, 0, 0, 0 };

	int tempValues[4];
	char title[50];
	char text[50];
	int nsticks = 0;

	joydefs_calibrateFlag = 0;

	JoyGetCalVals(org_axis_min, org_axis_center, org_axis_max);

	joy_set_cen();
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );

	if (!joy_present)	{
		ExecMessageBox( NULL, 1, TXT_OK, TXT_NO_JOYSTICK );
		return;
	}

	masks = JoyGetPresentMask();

	if ( masks == JOY_ALL_AXIS )
		nsticks = 2;
	else
		nsticks = 1;

	if (gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS)
		nsticks = 1;		//ignore for now the Sidewinder Pro X2 axis

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
	}

	if (joycal_message( title, text )) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_min[0] = tempValues[0];
	axis_min[1] = tempValues[1];
	joy_delay();

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
	}
	if (joycal_message( title, text)) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_max[0] = tempValues[0];
	axis_max[1] = tempValues[1];
	joy_delay();

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_CENTER);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_CENTER);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
	}
	if (joycal_message( title, text)) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_cen[0] = tempValues[0];
	axis_cen[1] = tempValues[1];
	axis_cen[2] = tempValues[2];
	joy_delay();

	// The fcs uses axes 3 for hat, so don't calibrate it.
	if ( gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS )	{
		//set Y2 axis, which is hat
		axis_min[3] = 0;
		axis_cen[3] = tempValues[3]/2;
		axis_max[3] = tempValues[3];
		joy_delay();

		//if X2 exists, calibrate it (for Sidewinder Pro)
		if ( kconfig_is_axes_used(2) && (masks & JOY_2_X_AXIS) )	{
			sprintf( title, "Joystick X2 axis\nLEFT");
			sprintf( text, "Move joystick X2 axis\nall the way left");
			if (joycal_message( title, text )) {
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_min[2] = tempValues[2];
			axis_min[3] = tempValues[3];
			joy_delay();

			sprintf( title, "Joystick X2 axis\nRIGHT");
			sprintf( text, "Move joystick X2 axis\nall the way right");
			if (joycal_message( title, text ))	{
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_max[2] = tempValues[2];
			axis_max[3] = tempValues[3];
			joy_delay();
		}
	} else {
		masks = JoyGetPresentMask();

		if ( nsticks == 2 )	{
			if ( kconfig_is_axes_used(2) || kconfig_is_axes_used(3) )	{
				sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
				sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
				if (joycal_message( title, text )) {
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_min[2] = tempValues[2];
				axis_min[3] = tempValues[3];
				joy_delay();

				sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
				sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
				if (joycal_message( title, text ))	{
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_max[2] = tempValues[2];
				axis_max[3] = tempValues[3];
				joy_delay();

				sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_CENTER);
				sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
				if (joycal_message( title, text ))	{
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_cen[2] = tempValues[2];
				axis_cen[3] = tempValues[3];
				joy_delay();
			}
		}
		else if ( (!(masks & JOY_2_X_AXIS)) && (masks & JOY_2_Y_AXIS) )	{
			if ( kconfig_is_axes_used(3) )	{
				// A throttle axis!!!!!
				sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_FORWARD);
				if (joycal_message( title, TXT_MOVE_THROTTLE_F))	{
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_min[3] = tempValues[3];
				joy_delay();

				sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_REVERSE);
				if (joycal_message( title, TXT_MOVE_THROTTLE_R)) {
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_max[3] = tempValues[3];
				joy_delay();

				sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_CENTER);
				if (joycal_message( title, TXT_MOVE_THROTTLE_C)) {
					JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
					return;
				}
				JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
				axis_cen[3] = tempValues[3];
				joy_delay();
			}
		}
	}
	JoySetCalVals(axis_min, axis_cen, axis_max);

	WriteConfigFile();
}
#else
void joydefs_calibrate2()
{
	ubyte masks;
	int org_axis_min[4];
	int org_axis_center[4];
	int org_axis_max[4];

	int axis_min[4] = { 0, 0, 0, 0 };
	int axis_cen[4] = { 0, 0, 0, 0 };
	int axis_max[4] = { 0, 0, 0, 0 };

	int tempValues[4];
	char title[50];
	char text[50];
	int i, nsticks = 0;

	joydefs_calibrateFlag = 0;

	if ( gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS ) {
		axis_cen[0] = axis_cen[1] = axis_cen[2] = 0;
		axis_min[0] = axis_min[1] = axis_min[2] = -127;
		axis_max[0] = axis_max[1] = axis_max[2] = 127;
		axis_min[3] = 0;
		axis_max[3] = 255;
		axis_cen[3] = 128;
		JoySetCalVals(axis_min, axis_cen, axis_max);
		return;
	}

	if ( gameConfig.nControlType == CONTROL_FLIGHTSTICK_PRO ) 		// no calibration needed
		return;

	JoyGetCalVals(org_axis_min, org_axis_center, org_axis_max);

	joy_set_cen();
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );

	if (!joy_present)	{
		ExecMessageBox( NULL, 1, TXT_OK, TXT_NO_JOYSTICK );
		return;
	}

	masks = JoyGetPresentMask();

	if ( masks == JOY_ALL_AXIS )
		nsticks = 2;
	else
		nsticks = 1;

	if ( (gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS) || (gameConfig.nControlType == CONTROL_FLIGHTSTICK_PRO) )
		nsticks = 1;		//ignore for now the Sidewinder Pro X2 axis

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
	}

	if (joycal_message( title, text )) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_min[0] = tempValues[0];
	axis_min[1] = tempValues[1];
	joy_delay();

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
	}
	if (joycal_message( title, text)) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_max[0] = tempValues[0];
	axis_max[1] = tempValues[1];
	joy_delay();

	if ( nsticks == 2 )	{
		sprintf( title, "%s #1\n%s", TXT_JOYSTICK, TXT_CENTER);
		sprintf( text, "%s #1 %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
	} else {
		sprintf( title, "%s\n%s", TXT_JOYSTICK, TXT_CENTER);
		sprintf( text, "%s %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
	}
	if (joycal_message( title, text)) {
		JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
		return;
	}
	JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
	axis_cen[0] = tempValues[0];
	axis_cen[1] = tempValues[1];
	axis_cen[2] = tempValues[2];
	joy_delay();

	masks = JoyGetPresentMask();

	if ( nsticks == 2 )	{
		if ( kconfig_is_axes_used(2) || kconfig_is_axes_used(3) )	{
			sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_UPPER_LEFT);
			sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_UL);
			if (joycal_message( title, text )) {
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_min[2] = tempValues[2];
			axis_min[3] = tempValues[3];
			joy_delay();

			sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_LOWER_RIGHT);
			sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_LR);
			if (joycal_message( title, text ))	{
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_max[2] = tempValues[2];
			axis_max[3] = tempValues[3];
			joy_delay();

			sprintf( title, "%s #2\n%s", TXT_JOYSTICK, TXT_CENTER);
			sprintf( text, "%s #2 %s", TXT_MOVE_JOYSTICK, TXT_TO_C);
			if (joycal_message( title, text ))	{
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_cen[2] = tempValues[2];
			axis_cen[3] = tempValues[3];
			joy_delay();
		}
	} else {
		if ( kconfig_is_axes_used(3) )	{
			// A throttle axis!!!!!
			sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_FORWARD);
			if (joycal_message( title, TXT_MOVE_THROTTLE_F))	{
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_min[3] = tempValues[3];
			joy_delay();

			sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_REVERSE);
			if (joycal_message( title, TXT_MOVE_THROTTLE_R)) {
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_max[3] = tempValues[3];
			joy_delay();

			sprintf( title, "%s\n%s", TXT_THROTTLE, TXT_CENTER);
			if (joycal_message( title, TXT_MOVE_THROTTLE_C)) {
				JoySetCalVals(org_axis_min, org_axis_center, org_axis_max);
				return;
			}
			JoyReadRawAxis( JOY_ALL_AXIS, tempValues );
			axis_cen[3] = tempValues[3];
			joy_delay();
		}
	}
	JoySetCalVals(axis_min, axis_cen, axis_max);

	WriteConfigFile();
}
#endif
#endif // 0

//char *control_text[CONTROL_MAX_TYPES] = { "Keyboard only", "Joystick", "Flightstick Pro", "Thrustmaster FCS", "Gravis Gamepad", "Mouse", "Cyberman" };

#ifndef MACINTOSH		// mac will have own verion of this function

#define CONTROL_MAX_TYPES_DOS	(CONTROL_MAX_TYPES-1)	//last item is windows only

void joydef_menuset_1(int nitems, tMenuItem * items, int *last_key, int citem )
{
	int i;
	int ocType = gameConfig.nControlType;

	nitems = nitems;
	last_key = last_key;
	citem = citem;

	for (i=0; i<CONTROL_MAX_TYPES_DOS; i++ )
		if (items[i].value) gameConfig.nControlType = i;

	if ( (ocType != gameConfig.nControlType) && (gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS ) )	{
		ExecMessageBox( TXT_IMPORTANT_NOTE, 1, TXT_OK, TXT_FCS );
	}

	if (ocType != gameConfig.nControlType) {
		switch (gameConfig.nControlType) {
	//		case	CONTROL_NONE:
			case	CONTROL_JOYSTICK:
			case	CONTROL_FLIGHTSTICK_PRO:
			case	CONTROL_THRUSTMASTER_FCS:
	//		case	CONTROL_GRAVIS_GAMEPAD:
	//		case	CONTROL_MOUSE:
	//		case	CONTROL_CYBERMAN:
				joydefs_calibrateFlag = 1;
		}
		KCSetControls (0);
	}

}

#else		// ifndef MACINTOSH

#define MAX_MAC_CONTROL_TYPES	6

char *ch_warning = "Choosing this option will\noverride any settings for Descent II\nin the CH control panels.  This\noption provides direct programming\nonly.  See the readme for details.";
char *tm_warning = "Choosing this option might\ncause settings in the Thrustmaster\ncontrol panels to be overridden\ndepending on the direct inhibition\nsetting in that panel.\nSee the readme for details.";
char *ms_warning = "When using a Mousestick II,\nbe sure that there is not a stick\nset active for Descent II.\nHaving a stick set active might cause\nundesirable joystick and\nkeyboard behavior.  See\nthe readme for detals.";
char *joy_warning = "Please use your joystick's\ncontrol panel to customize the\nbuttons and axis for Descent II.\nSee the joystick's manual for\ninstructions.";

void joydef_menuset_1(int nitems, tMenuItem * items, int *last_key, int citem )
{
	int i;
	int ocType = gameConfig.nControlType;
	char *warning_text = NULL;

	nitems = nitems;
	last_key = last_key;
	citem = citem;

	for (i=0; i<MAX_MAC_CONTROL_TYPES; i++ )
		if (items[i].value) gameConfig.nControlType = i;

	if ( (ocType != gameConfig.nControlType) && (gameConfig.nControlType == CONTROL_FLIGHTSTICK_PRO ) )	{
		warning_text = ch_warning;
	}

	if ( (ocType != gameConfig.nControlType) && (gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS ) )	{
		warning_text = tm_warning;
	}

	if ( (ocType != gameConfig.nControlType) && (gameConfig.nControlType == CONTROL_GRAVIS_GAMEPAD ) )	{
		warning_text = ms_warning;
	}

	if (warning_text) {
		hide_cursor();
		ExecMessageBox( TXT_IMPORTANT_NOTE, 1, TXT_OK, warning_text );
		show_cursor();
	}

	if (ocType != gameConfig.nControlType) {
		switch (gameConfig.nControlType) {
			case	CONTROL_JOYSTICK:
			case	CONTROL_FLIGHTSTICK_PRO:
			case	CONTROL_THRUSTMASTER_FCS:
			case	CONTROL_GRAVIS_GAMEPAD:		// this really means a firebird or mousestick
				joydefs_calibrateFlag = 1;
		}
		KCSetControls (0);
		joydefs_setType( gameConfig.nControlType );
	}

}

#endif

extern ubyte kc_use_external_control;
extern ubyte kc_enable_external_control;
extern ubyte *kc_external_name;

#ifdef MACINTOSH

//NOTE: MAC VERSION
void InputDeviceConfig()
{
	int i, old_masks, masks,nitems;
	tMenuItem m[14];
	int i1=11;
	char xtext[128];

	do {
		memset (m, 0, sizeof (m));
		m[0].nType = NM_TYPE_RADIO; m[0].text = CONTROL_TEXT(0); m[0].value = 0; m[0].group = 0;
		m[1].nType = NM_TYPE_RADIO; m[1].text = CONTROL_TEXT(1); m[1].value = 0; m[1].group = 0;
		m[2].nType = NM_TYPE_RADIO; m[2].text = CONTROL_TEXT(2); m[2].value = 0; m[2].group = 0;
		m[3].nType = NM_TYPE_RADIO; m[3].text = CONTROL_TEXT(3); m[3].value = 0; m[3].group = 0;
// change the text for the thrustmaster
		m[3].text = "Thrustmaster";
		m[4].nType = NM_TYPE_RADIO; m[4].text = CONTROL_TEXT(4); m[4].value = 0; m[4].group = 0;
// change the text of the gravis gamepad to be the mac gravis sticks
		m[4].text = "Gravis Firebird/MouseStick II";
		m[5].nType = NM_TYPE_RADIO; m[5].text = CONTROL_TEXT(5); m[5].value = 0; m[5].group = 0;

		m[6].nType = NM_TYPE_MENU;   m[6].text=TXT_CUST_ABOVE;
		m[7].nType = NM_TYPE_TEXT;   m[7].text="";
		m[8].nType = NM_TYPE_SLIDER; m[8].text=TXT_JOYS_SENSITIVITY; m[8].value=gameOpts->input.joystick.sensitivity; m[8].minValue =0; m[8].maxValue = 8;
		m[9].nType = NM_TYPE_TEXT;   m[9].text="";
		m[10].nType = NM_TYPE_MENU;  m[10].text=TXT_CUST_KEYBOARD;
		nitems=11;

		m[gameConfig.nControlType].value = 1;

		i1 = ExecMenu1( NULL, TXT_CONTROLS, nitems, m, joydef_menuset_1, i1 );
		gameOpts->input.mouse.sensitivity =
		gameOpts->input.joystick.sensitivity = m[8].value;

		switch(i1)	{
		case 6: {
				old_masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						old_masks |= (1<<i);
				}
				if ( gameConfig.nControlType==0 )
					// nothing...
					gameConfig.nControlType=0;
				else if ( gameConfig.nControlType == 1)  // this is just a joystick
					ExecMessageBox( TXT_IMPORTANT_NOTE, 1, TXT_OK, joy_warning );
				else if ( gameConfig.nControlType<5 ) {
					char title[64];

					if (gameConfig.nControlType == 3)
						strcpy(title, "Thrustmaster");
					else if (gameConfig.nControlType == 4)
						strcpy(title, "Gravis Firebird/Mousestick II");
					else
						strcpy(title, CONTROL_TEXT(gameConfig.nControlType) );


					KConfig(1, title );
				} else
					KConfig(2, CONTROL_TEXT(gameConfig.nControlType) );

				masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						masks |= (1<<i);
				}

				switch (gameConfig.nControlType) {
				case	CONTROL_JOYSTICK:
				case	CONTROL_FLIGHTSTICK_PRO:
				case	CONTROL_THRUSTMASTER_FCS:
				case	CONTROL_GRAVIS_GAMEPAD:
					{
						for (i=0; i<4; i++ )	{
							if ( (masks&(1<<i)) && (!(old_masks&(1<<i))))
								joydefs_calibrateFlag = 1;
						}
					}
					break;
				}
			}
			break;
		case 10:
			KConfig(0, TXT_KEYBOARD);
			break;
		}

	} while(i1>-1);

	switch (gameConfig.nControlType) {
	case	CONTROL_JOYSTICK:
//	case	CONTROL_FLIGHTSTICK_PRO:
	case	CONTROL_THRUSTMASTER_FCS:
	case	CONTROL_GRAVIS_GAMEPAD:
		if ( joydefs_calibrateFlag )
			JoyDefsCalibrate();
		break;
	}

}

// silly routine to tell the joystick handler which nType of control
// we are using
void joydefs_setType(ubyte nType)
{
	ubyte joyType;

	switch (nType)
	{
		case	CONTROL_NONE:				joyType = JOY_AS_NONE;					break;
		case	CONTROL_JOYSTICK:			joyType = JOY_AS_MOUSE;				break;
		case	CONTROL_FLIGHTSTICK_PRO:	joyType = JOY_AS_JOYMANAGER;			break;
		case	CONTROL_THRUSTMASTER_FCS:	joyType = JOY_AS_THRUSTMASTER;			break;
		case	CONTROL_GRAVIS_GAMEPAD:		joyType = JOY_AS_MOUSESTICK;			break;
		case	CONTROL_MOUSE:				joyType = JOY_AS_MOUSE;				break;
	}
	joy_setType(joyType);
}

#else		// #ifdef MACINTOSH

//NOTE: UNIX/DOS VERSION
void InputDeviceConfig()
{
	int i, old_masks, masks,nitems;
	tMenuItem m[14];
	int i1=11;
	char xtext[128];

	do {
		nitems=12;
		memset (m, 0, sizeof (m));
		m[0].nType = NM_TYPE_RADIO; m[0].text = CONTROL_TEXT(0); m[0].value = 0; m[0].group = 0;
		m[1].nType = NM_TYPE_RADIO; m[1].text = CONTROL_TEXT(1); m[1].value = 0; m[1].group = 0;
		m[2].nType = NM_TYPE_RADIO; m[2].text = CONTROL_TEXT(2); m[2].value = 0; m[2].group = 0;
		m[3].nType = NM_TYPE_RADIO; m[3].text = CONTROL_TEXT(3); m[3].value = 0; m[3].group = 0;
		m[4].nType = NM_TYPE_RADIO; m[4].text = CONTROL_TEXT(4); m[4].value = 0; m[4].group = 0;
		m[5].nType = NM_TYPE_RADIO; m[5].text = CONTROL_TEXT(5); m[5].value = 0; m[5].group = 0;
		m[6].nType = NM_TYPE_RADIO; m[6].text = CONTROL_TEXT(6); m[6].value = 0; m[6].group = 0;

		m[ 7].nType = NM_TYPE_MENU;		m[ 7].text=TXT_CUST_ABOVE;
		m[ 8].nType = NM_TYPE_TEXT;		m[ 8].text="";
		m[ 9].nType = NM_TYPE_SLIDER;	m[ 9].text=TXT_JOYS_SENSITIVITY; m[9].value=gameOpts->input.joystick.sensitivity; m[9].minValue =0; m[9].maxValue = 8;
		m[10].nType = NM_TYPE_TEXT;		m[10].text="";
		m[11].nType = NM_TYPE_MENU;		m[11].text=TXT_CUST_KEYBOARD;

		m[gameConfig.nControlType].value = 1;

		if ( kc_use_external_control )	{
			sprintf( xtext, "Enable %s", kc_external_name );
			m[12].nType = NM_TYPE_CHECK; m[12].text = xtext; m[12].value = kc_enable_external_control;
			nitems++;
		}

		m[nitems].nType = NM_TYPE_MENU; m[nitems].text="CUSTOMIZE D2X KEYS"; nitems++;

		i1 = ExecMenu1( NULL, TXT_CONTROLS, nitems, m, joydef_menuset_1, i1 );
		gameOpts->input.mouse.sensitivity =
		gameOpts->input.joystick.sensitivity = m[9].value;

		switch(i1)	{
		case 7: {
				old_masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						old_masks |= (1<<i);
				}
				if ( gameConfig.nControlType==0 )
					KConfig(0, TXT_KEYBOARD);
				else if ( gameConfig.nControlType<5 )
					KConfig(1, CONTROL_TEXT(gameConfig.nControlType) );
				else
					KConfig(2, CONTROL_TEXT(gameConfig.nControlType) );

				masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						masks |= (1<<i);
				}

				switch (gameConfig.nControlType) {
				case	CONTROL_JOYSTICK:
				case	CONTROL_FLIGHTSTICK_PRO:
				case	CONTROL_THRUSTMASTER_FCS:
					{
						for (i=0; i<4; i++ )	{
							if ( (masks&(1<<i)) && (!(old_masks&(1<<i))))
								joydefs_calibrateFlag = 1;
						}
					}
					break;
				}
			}
			break;
		case 11:
			KConfig(0, TXT_KEYBOARD);
			break;
		case 12:
			KConfig(4, "D2X KEYS");
			break;
		}

		if ( kc_use_external_control )	{
			kc_enable_external_control = m[12].value;
		}

	} while(i1>-1);

	switch (gameConfig.nControlType) {
	case	CONTROL_JOYSTICK:
	case	CONTROL_FLIGHTSTICK_PRO:
	case	CONTROL_THRUSTMASTER_FCS:
		if ( joydefs_calibrateFlag )
			JoyDefsCalibrate();
		break;
	}

}

#endif	// ifdef MACINTOSH

void joydef_menuset_win(int nitems, tMenuItem * items, int *last_key, int citem )
{
	int i;
	int ocType = gameConfig.nControlType;

	Int3();	//need to make this code work for windows

	nitems = nitems;
	last_key = last_key;
	citem = citem;

	for (i=0; i<CONTROL_MAX_TYPES; i++ )
		if (items[i].value) gameConfig.nControlType = i;

	if ( (ocType != gameConfig.nControlType) && (gameConfig.nControlType == CONTROL_THRUSTMASTER_FCS ) )	{
		ExecMessageBox( TXT_IMPORTANT_NOTE, 1, TXT_OK, TXT_FCS );
	}

	if (ocType != gameConfig.nControlType) {
		switch (gameConfig.nControlType) {
	//		case	CONTROL_NONE:
			case	CONTROL_JOYSTICK:
			case	CONTROL_FLIGHTSTICK_PRO:
			case	CONTROL_THRUSTMASTER_FCS:
	//		case	CONTROL_GRAVIS_GAMEPAD:
	//		case	CONTROL_MOUSE:
	//		case	CONTROL_CYBERMAN:
				joydefs_calibrateFlag = 1;
		}
		KCSetControls (0);
	}

}

//NOTE: WINDOWS VERSION
void InputDeviceConfig()
{
	int i, old_masks, masks,nitems;
	tMenuItem m[14];
	int i1=11;
	char xtext[128];

	Int3();		//this routine really needs to be cleaned up

	do {
		nitems=13;
		memset (m, 0, sizeof (m));
		m[0].nType = NM_TYPE_RADIO; m[0].text = CONTROL_TEXT(0); m[0].value = 0; m[0].group = 0;
		m[1].nType = NM_TYPE_RADIO; m[1].text = CONTROL_TEXT(1); m[1].value = 0; m[1].group = 0;
		m[2].nType = NM_TYPE_RADIO; m[2].text = CONTROL_TEXT(2); m[2].value = 0; m[2].group = 0;
		m[3].nType = NM_TYPE_RADIO; m[3].text = CONTROL_TEXT(3); m[3].value = 0; m[3].group = 0;
		m[4].nType = NM_TYPE_RADIO; m[4].text = CONTROL_TEXT(4); m[4].value = 0; m[4].group = 0;
		m[5].nType = NM_TYPE_RADIO; m[5].text = CONTROL_TEXT(5); m[5].value = 0; m[5].group = 0;
		m[6].nType = NM_TYPE_RADIO; m[6].text = CONTROL_TEXT(6); m[6].value = 0; m[6].group = 0;
		m[7].nType = NM_TYPE_RADIO; m[7].text = CONTROL_TEXT(7); m[7].value = 0; m[7].group = 0;

		m[8].nType = NM_TYPE_MENU; m[8].text=TXT_CUST_ABOVE;
		m[9].nType = NM_TYPE_TEXT;   m[9].text="";
		m[10].nType = NM_TYPE_SLIDER; m[10].text=TXT_JOYS_SENSITIVITY; m[10].value=gameOpts->input.joystick.sensitivity; m[10].minValue =0; m[10].maxValue = 8;
		m[11].nType = NM_TYPE_TEXT;   m[11].text="";
		m[12].nType = NM_TYPE_MENU; m[12].text=TXT_CUST_KEYBOARD;

		m[gameConfig.nControlType].value = 1;

		if ( kc_use_external_control )	{
			sprintf( xtext, "Enable %s", kc_external_name );
			m[13].nType = NM_TYPE_CHECK; m[13].text = xtext; m[13].value = kc_enable_external_control;
			nitems++;
		}

		i1 = ExecMenu1( NULL, TXT_CONTROLS, nitems, m, joydef_menuset_win, i1 );
		gameOpts->input.mouse.sensitivity =
		gameOpts->input.joystick.sensitivity = m[10].value;

		switch(i1)	{
		case 8: {
				old_masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						old_masks |= (1<<i);
				}
				if ( gameConfig.nControlType==0 )
					// nothing...
					gameConfig.nControlType=0;
				else if ( gameConfig.nControlType<5 )
					KConfig(1, CONTROL_TEXT(gameConfig.nControlType) );
				else
					KConfig(2, CONTROL_TEXT(gameConfig.nControlType) );

				masks = 0;
				for (i=0; i<4; i++ )		{
					if (kconfig_is_axes_used(i))
						masks |= (1<<i);
				}

				switch (gameConfig.nControlType) {
				case	CONTROL_JOYSTICK:
				case	CONTROL_FLIGHTSTICK_PRO:
				case	CONTROL_THRUSTMASTER_FCS:
					{
						for (i=0; i<4; i++ )	{
							if ( (masks&(1<<i)) && (!(old_masks&(1<<i))))
								joydefs_calibrateFlag = 1;
						}
					}
					break;
				}
			}
			break;
		case 12:
			KConfig(0, TXT_KEYBOARD);
			break;
		}

		if ( kc_use_external_control )	{
			kc_enable_external_control = m[13].value;
		}

	} while(i1>-1);

	switch (gameConfig.nControlType) {
	case	CONTROL_JOYSTICK:
	case	CONTROL_FLIGHTSTICK_PRO:
	case	CONTROL_THRUSTMASTER_FCS:
		if ( joydefs_calibrateFlag )
			JoyDefsCalibrate();
		break;
	}

}

