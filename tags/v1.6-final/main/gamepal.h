/* $Id: gamepal.h,v 1.4 2003/10/10 09:36:35 btb Exp $ */
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
 * Header for gamepal.c
 *
 */

#ifndef _GAMEPAL_H
#define _GAMEPAL_H

#include "inferno.h"
#include "cfile.h"

#define D2_DEFAULT_PALETTE "default.256"
#define MENU_PALETTE			"default.256"

extern char szCurrentLevelPalette[SHORT_FILENAME_LEN];

// load a palette by name. returns 1 if new palette loaded, else 0
// if used_for_level is set, load pig, etc.
// if no_change_screen is set, the current screen does not get
// remapped, and the hardware palette does not get changed
ubyte *LoadPalette (char *pszPaletteName, char *pszLevelName, int nUsedForLevel, 
						  int bNoScreenChange, int bForce);

// remap color font & menu background into the current palette
void RemapFontsAndMenus(int do_fadetable_hack);

extern ubyte *gamePalette;

#endif /* _GAMEPAL_H */
