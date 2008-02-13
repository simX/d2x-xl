/* $Id: credits.c,v 1.8 2003/10/10 09:36:34 btb Exp $ */
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
static char rcsid[] = "$Id: credits.c,v 1.8 2003/10/10 09:36:34 btb Exp $";
#endif

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>
#endif
#ifdef __macosx__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "error.h"
#include "inferno.h"
#include "gr.h"
#include "mono.h"
#include "key.h"
#include "palette.h"
#include "game.h"
#include "gamepal.h"
#include "timer.h"

#include "newmenu.h"
#include "gamefont.h"
#include "network.h"
#include "iff.h"
#include "pcx.h"
#include "u_mem.h"
#include "mouse.h"
#include "joy.h"
#include "screens.h"
#include "digi.h"

#include "cfile.h"
#include "compbit.h"
#include "songs.h"
#include "menu.h"			// for gameStates.menus.bHires

#define LHX(x)      (gameStates.menus.bHires?2*(x):x)
#define LHY(y)      (gameStates.menus.bHires?(24*(y))/10:y)

#define ROW_SPACING (gameStates.menus.bHires?26:11)
#define NUM_LINES_HIRES 21
#define NUM_LINES (gameStates.menus.bHires?NUM_LINES_HIRES:20)

ubyte fadeValues[200] = { 1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,8,9,9,10,10,
11,11,12,12,12,13,13,14,14,15,15,15,16,16,17,17,17,18,18,19,19,19,20,20,
20,21,21,22,22,22,23,23,23,24,24,24,24,25,25,25,26,26,26,26,27,27,27,27,
28,28,28,28,28,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,31,31,31,31,
31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,30,30,
30,30,30,30,29,29,29,29,29,29,28,28,28,28,28,27,27,27,27,26,26,26,26,25,
25,25,24,24,24,24,23,23,23,22,22,22,21,21,20,20,20,19,19,19,18,18,17,17,
17,16,16,15,15,15,14,14,13,13,12,12,12,11,11,10,10,9,9,8,8,8,7,7,6,6,5,
5,4,4,3,3,2,2,1 };

ubyte fadeValues_hires[480] = { 1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,
5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,9,9,9,9,9,10,10,10,10,10,10,11,11,11,11,11,12,12,12,12,12,12,
13,13,13,13,13,14,14,14,14,14,14,15,15,15,15,15,15,16,16,16,16,16,17,17,17,17,17,17,18,18,
18,18,18,18,18,19,19,19,19,19,19,20,20,20,20,20,20,20,21,21,21,21,21,21,22,22,22,22,22,22,
22,22,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,26,26,26,26,
26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,
29,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,
30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,
31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,
30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,29,29,29,29,29,29,29,29,29,29,29,29,
29,29,28,28,28,28,28,28,28,28,28,28,28,28,27,27,27,27,27,27,27,27,27,27,26,26,26,26,26,26,
26,26,26,25,25,25,25,25,25,25,25,25,24,24,24,24,24,24,24,24,23,23,23,23,23,23,23,22,22,22,
22,22,22,22,22,21,21,21,21,21,21,20,20,20,20,20,20,20,19,19,19,19,19,19,18,18,18,18,18,18,
18,17,17,17,17,17,17,16,16,16,16,16,15,15,15,15,15,15,14,14,14,14,14,14,13,13,13,13,13,12,
12,12,12,12,12,11,11,11,11,11,10,10,10,10,10,10,9,9,9,9,9,8,8,8,8,8,7,7,7,7,7,6,6,6,6,6,5,5,5,5,
5,5,4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1};

ubyte *creditsPalette = NULL;

extern ubyte *grBitBltFadeTable;

grsFont * header_font;
grsFont * title_font;
grsFont * names_font;

#define ALLOWED_CHAR 'R'

#ifndef _DEBUG
#	define CREDITS_BACKGROUND_FILENAME (gameStates.menus.bHires ? "\x01starsb.pcx" : "\x01stars.pcx")	//only read from hog file
#else
#	define CREDITS_BACKGROUND_FILENAME (gameStates.menus.bHires ? "starsb.pcx" : "stars.pcx")
#endif

typedef struct box {
	int left, top, width, height;
} box;

#define CREDITS_FILE    \
		  (CFExist("mcredits.tex",gameFolders.szDataDir,0)?"mcredits.tex":\
			CFExist("ocredits.tex",gameFolders.szDataDir,0)?"ocredits.tex":"credits.tex")

#define cr_gr_printf(x,y,s)	GrPrintF (NULL, (x) == 0x8000 ? (x) : (x), (y), s)

//if filename passed is NULL, show normal credits
void ShowCredits(char *credits_filename)
{
	int i, j, l, done;
	CFILE cf;
	char buffer[NUM_LINES_HIRES][80];
	grsBitmap bmBackdrop;
	int pcx_error;
	int buffer_line = 0;
	fix lastTime;
//	fix time_delay = 4180;			// ~ F1_0 / 12.9
//	fix time_delay = 1784;
	fix time_delay = 2800;
	int first_line_offset,extra_inc=0;
	int have_bin_file = 0;
	char * tempp;
	char filename[32];
	int xOffs, yOffs;

	box dirty_box[NUM_LINES_HIRES];
	gsrCanvas *CreditsOffscreenBuf=NULL;
	gsrCanvas *save_canv = grdCurCanv;

	// Clear out all tex buffer lines.
	for (i=0; i<NUM_LINES; i++)
		buffer[i][0] = 0;
	memset (dirty_box, 0, sizeof (dirty_box));

	sprintf(filename, "%s", CREDITS_FILE);
	have_bin_file = 0;
	if (credits_filename) {
		strcpy(filename,credits_filename);
		have_bin_file = 1;
	}
	if (!CFOpen(&cf, filename, gameFolders.szDataDir, "rb", 0)) {
		char nfile[32];
	
		if (credits_filename)
			return;		//ok to not find special filename

		tempp = strchr(filename, '.');
		*tempp = '\0';
		sprintf(nfile, "%s.txb", filename);
		if (!CFOpen(&cf, nfile, gameFolders.szDataDir, "rb", 0))
			Error("Missing CREDITS.TEX and CREDITS.TXB &cf\n");
		have_bin_file = 1;
	}

	SetScreenMode(SCREEN_MENU);

	xOffs = (grdCurCanv->cvBitmap.bmProps.w - 640) / 2;
	yOffs = (grdCurCanv->cvBitmap.bmProps.h - 480) / 2;

	if (xOffs < 0)
		xOffs = 0;
	if (yOffs < 0)
		yOffs = 0;

	creditsPalette = GrUsePaletteTable("credits.256", NULL);
	GrPaletteStepLoad (NULL);
	header_font = GrInitFont(gameStates.menus.bHires?"font1-1h.fnt":"font1-1.fnt");
	title_font = GrInitFont(gameStates.menus.bHires?"font2-3h.fnt":"font2-3.fnt");
	names_font = GrInitFont(gameStates.menus.bHires?"font2-2h.fnt":"font2-2.fnt");
	bmBackdrop.bmTexBuf = NULL;
	bmBackdrop.bmPalette = NULL;

//MWA  Made bmBackdrop bitmap linear since it should always be.  the current canvas may not
//MWA  be linear, so we can't rely on grdCurCanv->cvBitmap->bmProps.nType.

	pcx_error = PCXReadBitmap (CREDITS_BACKGROUND_FILENAME, &bmBackdrop, BM_LINEAR, 0);
	if (pcx_error != PCX_ERROR_NONE) {
		CFClose(&cf);
		return;
	}
	SongsPlaySong(SONG_CREDITS, 1);
	GrRemapBitmapGood(&bmBackdrop, NULL, -1, -1);

if (!gameOpts->menus.nStyle) {
	GrSetCurrentCanvas(NULL);
	GrBitmap(xOffs,yOffs,&bmBackdrop);
	if ((grdCurCanv->cvBitmap.bmProps.w > 640) || (grdCurCanv->cvBitmap.bmProps.h > 480)) {
		GrSetColorRGBi (RGBA_PAL (0,0,32));
		GrUBox(xOffs,yOffs,xOffs+bmBackdrop.bmProps.w+1,yOffs+bmBackdrop.bmProps.h+1);
		}
	}
   //GrUpdate (0);
	GrPaletteFadeIn(NULL, 32, 0);

//	Create a new offscreen buffer for the credits screen
//MWA  Let's be a little smarter about this and check the VR_offscreen buffer
//MWA  for size to determine if we can use that buffer.  If the game size
//MWA  matches what we need, then lets save memory.

//if (!gameOpts->menus.nStyle) 
{
	if (gameStates.menus.bHires && !gameOpts->menus.nStyle && gameStates.render.vr.buffers.offscreen->cv_w == 640)
		CreditsOffscreenBuf = gameStates.render.vr.buffers.offscreen;
	else if (gameStates.menus.bHires)
		CreditsOffscreenBuf = GrCreateCanvas(640,480);
	else
		CreditsOffscreenBuf = GrCreateCanvas(320,200);
	if (!CreditsOffscreenBuf)
		Error("Not enough memory to allocate Credits Buffer.");
	}
CreditsOffscreenBuf->cvBitmap.bmPalette = grdCurCanv->cvBitmap.bmPalette;
if (gameOpts->menus.nStyle)
	CreditsOffscreenBuf->cvBitmap.bmProps.flags |= BM_FLAG_TRANSPARENT;
KeyFlush();

	{
		lastTime = TimerGetFixedSeconds();
		done = 0;
		first_line_offset = 0;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gameStates.menus.nInMenu = 1;
	while(1)	{
		int k;

		do {
			buffer_line = (buffer_line+1) % NUM_LINES;
get_line:;
			if (CFGetS(buffer[buffer_line], 80, &cf))	{
				char *p = buffer [buffer_line];
				if (have_bin_file) {				// is this a binary tbl &cf
					for (i = (int) strlen(buffer[buffer_line]) - 1; i; i--, p++) {
						*p = EncodeRotateLeft ((char) (EncodeRotateLeft (*p) ^ BITMAP_TBL_XOR));
					}
				}
				p = buffer[buffer_line];
				if (p[0] == ';')
					goto get_line;
				if (p[0] == '%') {
					if (p[1] == ALLOWED_CHAR)
						strcpy(p,p+2);
					else
						goto get_line;
					}
				p = strchr(&buffer[buffer_line][0],'\n');
				if (p) *p = '\0';
				} 
			else{
				//fseek(&cf, 0, SEEK_SET);
				buffer[buffer_line][0] = 0;
				done++;
			}
		} while (extra_inc--);
		extra_inc = 0;

for (i=0; i<ROW_SPACING; i += (gameStates.menus.bHires ? 2 : 1)) {
	int y;

	if (gameOpts->menus.nStyle) {
		GrSetCurrentCanvas(NULL);
		ShowFullscreenImage (&bmBackdrop);
//			GrUpdate (0);
#if 0
		if ((grdCurCanv->cvBitmap.bmProps.w > 640) || (grdCurCanv->cvBitmap.bmProps.h > 480)) {
			GrSetColorRGBi (RGBA_PAL (0,0,32));
			GrUBox (xOffs, yOffs, xOffs + bmBackdrop. bmProps.w + 1, yOffs + bmBackdrop.bmProps.h + 1);
			}
#endif
		}
	y = first_line_offset - i;
	//if (!gameOpts->menus.nStyle) 
		{
		GrSetCurrentCanvas(CreditsOffscreenBuf);
		if (gameOpts->menus.nStyle)
			GrClearCanvas (0);	
		else
			GrBitmap(0,0,&bmBackdrop);
		}
	for (j=0; j<NUM_LINES; j++)	{
		char *s;

		l = (buffer_line + j + 1) %  NUM_LINES;
		s = buffer[l];
		if (s[0] == '!') 
			s++;
		else if (s[0] == '$')	{
			grdCurCanv->cvFont = header_font;
			s++;
			} 
		else if (s[0] == '*')	{
			grdCurCanv->cvFont = title_font;
			s++;
			} 
		else
			grdCurCanv->cvFont = names_font;
		grBitBltFadeTable = (gameStates.menus.bHires ? fadeValues_hires : fadeValues);
		tempp = strchr (s, '\t');
		if (tempp)	{
		//	Wacky Credits thing
			int w, h, aw, w2, x1, x2;

			*tempp = 0;
			GrGetStringSize(s, &w, &h, &aw);
			x1 = ((gameStates.menus.bHires?320:160)-w)/2;
			cr_gr_printf(x1 , y, s);
			GrGetStringSize(&tempp[1], &w2, &h, &aw);
			x2 = (gameStates.menus.bHires?320:160)+(((gameStates.menus.bHires?320:160)-w2)/2);
			cr_gr_printf(x2, y, &tempp[1]);
			dirty_box[j].left = ((gameStates.menus.bHires?320:160)-w)/2;
			dirty_box[j].top = y;
			dirty_box[j].width =(x2+w2)-x1;
			dirty_box[j].height = h;
			*tempp = '\t';
			} 
		else {
		// Wacky Fast Credits thing
			int w, h, aw;

			GrGetStringSize(s, &w, &h, &aw);
			dirty_box[j].width = w;
        	dirty_box[j].height = h;
        	dirty_box[j].top = y;
        	dirty_box[j].left = ((gameStates.menus.bHires?640:320) - w) / 2;
			cr_gr_printf(0x8000, y, s);
		}
		grBitBltFadeTable = NULL;
		if (buffer[l][0] == '!')
			y += ROW_SPACING/2;
		else
			y += ROW_SPACING;
	}

	if (gameOpts->menus.nStyle) 
			GrSetCurrentCanvas(NULL);

		{	// Wacky Fast Credits Thing
		box	*new_box;
		grsBitmap *tempbmp;

		for (j=0; j<NUM_LINES; j++) {
			new_box = dirty_box + j;
			tempbmp = &(CreditsOffscreenBuf->cvBitmap);

			GrBmBitBlt (new_box->width + 1, new_box->height +4,
							new_box->left + xOffs, new_box->top + yOffs, 
							new_box->left, new_box->top,
							tempbmp, &(grdCurScreen->scCanvas.cvBitmap));
		}
	}
	GrUpdate (0);

			while(TimerGetFixedSeconds() < lastTime+time_delay);
			lastTime = TimerGetFixedSeconds();
			//see if redbook song needs to be restarted
			SongsCheckRedbookRepeat();
			k = KeyInKey();
#ifdef _DEBUG
			if (k == KEY_BACKSP) {
				Int3();
				k=0;
			}
#endif

			if ((k == KEY_PRINT_SCREEN) || (k == KEY_ALTED+KEY_F9)){
				bSaveScreenShot = 1;
				SaveScreenShot (NULL, 0);
				k = 0;
				}

			else if ((k == KEY_ESC)||(done>NUM_LINES))	{
					GrCloseFont(header_font);
					GrCloseFont(title_font);
					GrCloseFont(names_font);
					GrPaletteFadeOut(NULL, 32, 0);
					GrUsePaletteTable(D2_DEFAULT_PALETTE, NULL);
					D2_FREE(bmBackdrop.bmTexBuf);
					CFClose(&cf);
					GrSetCurrentCanvas(save_canv);
					SongsPlaySong(SONG_TITLE, 1);

				if (CreditsOffscreenBuf != gameStates.render.vr.buffers.offscreen)
					GrFreeCanvas(CreditsOffscreenBuf);
				glDisable (GL_BLEND);
				gameStates.menus.nInMenu = 0;
				return;
			}
		}

		if (buffer[(buffer_line + 1) %  NUM_LINES][0] == '!') {
			first_line_offset -= ROW_SPACING-ROW_SPACING/2;
			if (first_line_offset <= -ROW_SPACING) {
				first_line_offset += ROW_SPACING;
				extra_inc++;
			}
		}
	}

}
