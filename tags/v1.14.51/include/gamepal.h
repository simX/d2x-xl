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

#ifndef _GAMEPAL_H
#define _GAMEPAL_H

#include "descent.h"
#include "cfile.h"

#define D2_DEFAULT_PALETTE "default.256"
#define MENU_PALETTE			"default.256"

// load a palette by name. returns 1 if new palette loaded, else 0
// if used_forLevel is set, load pig, etc.
// if no_change_screen is set, the current screen does not get
// remapped, and the hardware palette does not get changed
ubyte *LoadPalette (const char *pszPaletteName, const char *pszLevelName, int nUsedForLevel, 
						  int bNoScreenChange, int bForce);

// remap color font & menu background into the current palette
extern ubyte *gamePalette;

#endif /* _GAMEPAL_H */
