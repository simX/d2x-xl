/* $Id: hostage.h,v 1.2 2003/10/10 09:36:35 btb Exp $ */
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
 * Header for hostage.c
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:57:57  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:27:50  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.23  1995/01/15  19:42:06  matt
 * Ripped out hostage faces for registered version
 *
 * Revision 1.22  1994/12/19  16:35:07  john
 * Made hoastage playback end when ship dies.
 *
 * Revision 1.21  1994/11/30  17:22:12  matt
 * Ripped out hostage faces in shareware version
 *
 * Revision 1.20  1994/11/19  19:53:43  matt
 * Added code to full support different hostage head clip & message for
 * each hostage.
 *
 * Revision 1.19  1994/11/19  16:35:28  matt
 * Got rid of unused code, & made an array smaller
 *
 * Revision 1.18  1994/10/23  02:10:57  matt
 * Got rid of obsolete hostage_info stuff
 *
 * Revision 1.17  1994/09/20  00:10:37  matt
 * Finished gauges for Status Bar, including hostage video display.
 *
 * Revision 1.16  1994/09/01  10:41:24  matt
 * Sizes for powerups now specified in bitmaps.tbl; blob bitmaps now plot
 * correctly if width & height of bitmap are different.
 *
 * Revision 1.15  1994/08/14  23:15:14  matt
 * Added animating bitmap hostages, and cleaned up vclips a bit
 *
 * Revision 1.14  1994/07/14  22:06:51  john
 * Fix radar/hostage tVideoClip conflict.
 *
 * Revision 1.13  1994/07/14  22:05:41  john
 * Made radar display not conflict with hostage
 * tVideoClip talking.
 *
 * Revision 1.12  1994/07/06  13:41:46  matt
 * Added support for bitmap hostages
 *
 * Revision 1.11  1994/07/06  12:43:36  john
 * Made generic messages for hostages.
 *
 * Revision 1.10  1994/07/06  10:55:09  john
 * New structures for hostages.
 *
 * Revision 1.9  1994/07/05  12:48:47  john
 * Put functionality of New Hostage spec into code.
 *
 * Revision 1.8  1994/07/01  17:55:14  john
 * First version of not-working hostage system.
 *
 * Revision 1.7  1994/06/15  14:57:10  john
 * Added triggers to demo recording.
 *
 * Revision 1.6  1994/06/08  18:15:49  john
 * Bunch of new stuff that basically takes constants out of the code
 * and puts them into bitmaps.tbl.
 *
 * Revision 1.5  1994/05/02  12:43:06  yuan
 * Fixed warnings.
 *
 * Revision 1.4  1994/04/06  14:42:46  yuan
 * Adding new powerups.
 *
 * Revision 1.3  1994/04/01  11:15:14  yuan
 * Added multiple bitmap functionality to all objects...
 * (hostages, powerups, lasers, etc.)
 * Hostages and powerups are implemented in the tObject system,
 * just need to finish function call to "affect" player.
 *
 * Revision 1.2  1994/03/31  12:04:35  matt
 * Added prototypes for 2 funcs
 *
 * Revision 1.1  1994/03/31  11:39:24  matt
 * Initial revision
 *
 *
 */


#ifndef _HOSTAGE_H
#define _HOSTAGE_H

//@@#include "vclip.h"

#define HOSTAGE_SIZE        i2f(5)  // 3d size of a hostage

#define MAX_HOSTAGE_TYPES   1       //only one hostage bitmap

extern int N_hostageTypes;

extern int Hostage_vclip_num[MAX_HOSTAGE_TYPES];    // for each nType of hostage

void DrawHostage(tObject *obj);
void hostage_rescue(int hostage_num);

#endif /* _HOSTAGE_H */
