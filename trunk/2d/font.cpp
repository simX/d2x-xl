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
 * Graphical routines for drawing fonts.
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef _WIN32
#	include <fcntl.h>
#	include <unistd.h>
#endif

#include "u_mem.h"

#include "inferno.h"
#include "text.h"
#include "font.h"
#include "grdef.h"
#include "error.h"

#include "cfile.h"
#include "mono.h"
#include "byteswap.h"
#include "bitmap.h"
#include "gamefont.h"

#include "makesig.h"

CFontManager fontManager;

#define LHX(x)	 (gameStates.menus.bHires ? 2 * (x) : x)

//------------------------------------------------------------------------------

ubyte *CFont::FindKernEntry (ubyte first, ubyte second)
{
	ubyte *p = m_info.kernData;

while (*p != 255)
	if ((*p == first) && (*(p + 1) == second))
		return p;
	else p += 3;
return NULL;
}

//------------------------------------------------------------------------------
//takes the character AFTER being offset into font
//takes the character BEFORE being offset into current font

void CFont::GetCharWidth (ubyte c, ubyte c2, int& width, int& spacing)
{
	int letter = c - m_info.minChar;

if (!InFont (letter)) {				//not in font, draw as space
	width = 0;
	if (m_info.flags & FT_PROPORTIONAL)
		spacing = m_info.width / 2;
	else
		spacing = m_info.width;
	return;
	}

if (m_info.flags & FT_PROPORTIONAL)
	width = m_info.widths [letter];
else
	width = m_info.width;
spacing = width;

if (m_info.flags & FT_KERNED) {
	if ((c2 != 0) && (c2 != '\n')) {
		int letter2 = c2 - m_info.minChar;
		if (InFont (letter2)) {
				ubyte *p = FindKernEntry ((ubyte) letter, (ubyte) letter2);
			if (p)
				spacing = p [2];
			}
		}
	}
}

//------------------------------------------------------------------------------

int CFont::GetLineWidth (const char *s)
{
	int w, w2, s2;

if (!s)
	return 0;
for (w = 0; *s && (*s != '\n'); s++) {
	if (*s <= 0x06) {
		if (*s <= 0x03)
			s++;
		continue;//skip color codes.
		}
	GetCharWidth (s[0], s[1], w2, s2);
	w += s2;
	}
return w;
}

//------------------------------------------------------------------------------

int CFont::GetCenteredX (const char *s)
{
return ((CCanvas::Current ()->Width () - GetLineWidth (s)) / 2);
}

//------------------------------------------------------------------------------
//hack to allow color codes to be embedded in strings -MPM
//note we subtract one from color, since 255 is "transparent" so it'll never be used, and 0 would otherwise end the string.
//function must already have orig_color var set (or they could be passed as args...)
//perhaps some sort of recursive orig_color nType thing would be better, but that would be way too much trouble for little gain
int grMsgColorLevel = 1;

inline char *CheckEmbeddedColors (char *textP, char c, int orig_color)
{
if ((c >= 1) && (c <= 3)) {
	if (*++textP) {
		if (grMsgColorLevel >= c) {
			FG_COLOR.rgb = 1;
			FG_COLOR.color.red = textP [0];
			FG_COLOR.color.green = textP [1];
			FG_COLOR.color.blue = textP [2];
			FG_COLOR.color.alpha = 0;
			}
		textP += 3;
		}
	}
else if ((c >= 4) && (c <= 6)) {
	if (grMsgColorLevel >= *textP - 3) {
		FG_COLOR.index = orig_color;
		FG_COLOR.rgb = 0;
		}
	textP++;
	}
return textP;
}

#define CHECK_EMBEDDED_COLORS () \
	if ((c >= 1) && (c <= 3)) { \
		if (*++textP) { \
			if (grMsgColorLevel >= c) { \
				FG_COLOR.rgb = 1; \
				FG_COLOR.color.red = textP [0] - 128; \
				FG_COLOR.color.green = textP [1] - 128; \
				FG_COLOR.color.blue = textP [2] - 128; \
				FG_COLOR.color.alpha = 0; \
				} \
			textP += 3; \
			} \
		} \
	else if ((c >= 4) && (c <= 6)) { \
		if (grMsgColorLevel >= *textP - 3) { \
			FG_COLOR.index = orig_color; \
			FG_COLOR.rgb = 0; \
			} \
		textP++; \
		}

//------------------------------------------------------------------------------

#include "ogl_defs.h"
#include "ogl_bitmap.h"
#include "args.h"
//font handling routines for OpenGL - Added 9/25/99 Matthew Mueller - they are here instead of in arch/ogl because they use all these defines

int Pow2ize (int x);//from ogl.c

//------------------------------------------------------------------------------

int CFont::TotalWidth (void)
{
if (m_info.flags & FT_PROPORTIONAL){
	int i, w = 0, c = m_info.minChar;
	for (i = 0; c <= m_info.maxChar; i++, c++) {
		if (m_info.widths [i] < 0)
			Error (TXT_FONT_WIDTH);
		w += m_info.widths [i];
		}
	return w;
	}
return m_info.width * Range ();
}

//------------------------------------------------------------------------------

void CFont::ChooseSize (int gap, int& rw, int& rh)
{
	int nChars = Range ();
	int r, x, y, nc = 0, smallest = 999999, smallr = -1, tries;
	int smallprop = 10000;
	int h, w;

for (h = 32; h <= 256; h *= 2) {
	if (m_info.height > h)
		continue;
	r= (h / (m_info.height + gap));
	w = Pow2ize ((TotalWidth () + (nChars - r) * gap) / r);
	tries = 0;
	do {
		if (tries)
			w = Pow2ize (w+1);
		if (tries > 3) 
			break;
		nc = 0;
		y = 0;
		while (y + m_info.height <= h){
			x=0;
			while (x < w) {
				if (nc == nChars)
					break;
				if (m_info.flags & FT_PROPORTIONAL){
					if (x + m_info.widths [nc] + gap > w)
						break;
					x += m_info.widths [nc++]+gap;
					}
				else {
					if (x + m_info.width + gap > w)
						break;
					x += m_info.width + gap;
					nc++;
					}
				}
			if (nc == nChars)
				break;
			y += m_info.height + gap;
			}
		tries++;
		} while (nc != nChars);
	if (nc != nChars)
		continue;

	if (w * h == smallest) {//this gives squarer sizes priority (ie, 128x128 would be better than 512*32)
		if (w >= h){
			if (w / h < smallprop) {
				smallprop = w / h;
				smallest++;
				}
			}
		else {
			if (h / w < smallprop) {
				smallprop = h / w;
				smallest++;//hack
				}
			}
		}
	if (w * h < smallest) {
		smallr = 1;
		smallest = w * h;
		rw = w;
		rh = h;
		}
	}
if (smallr <= 0)
	Error ("couldn't fit font?\n");
}

//------------------------------------------------------------------------------

void CFont::Create (const char *fontname)
{
	ubyte		*fp;
	CPalette *palette;
	int		nChars = Range ();
	int		i, w, h, tw, th, x, y, curx = 0, cury = 0;
	int		white;
	int		gap = 0; //having a gap just wastes ram, since we don't filter text textures at all.

ChooseSize (gap, tw, th);
palette = m_info.parentBitmap.Palette ();
m_info.parentBitmap.Setup (BM_LINEAR, tw, th, 1, fontname, NULL);
m_info.parentBitmap.SetPalette (palette);
if (!(m_info.flags & FT_COLOR))
	m_info.parentBitmap.SetTexture (textureManager.Get (&m_info.parentBitmap));
m_info.bitmaps = new CBitmap [nChars]; //(CBitmap*) D2_ALLOC (nChars * sizeof (CBitmap));
memset (m_info.bitmaps, 0, nChars * sizeof (CBitmap));
h = m_info.height;

white = palette->ClosestColor (63, 63, 63);

for (i = 0; i < nChars; i++) {
	if (m_info.flags & FT_PROPORTIONAL)
		w = m_info.widths [i];
	else
		w = m_info.width;
	if (w < 1 || w > 256)
		continue;
	if (curx + w + gap > tw) {
		cury += h + gap;
		curx = 0;
		}
	if (cury + h > th)
		Error (TXT_FONT_SIZE, i, nChars);
	if (m_info.flags & FT_COLOR) {
		if (m_info.flags & FT_PROPORTIONAL)
			fp = m_info.chars[i];
		else
			fp = m_info.data + i * w*h;
		for (y = 0; y < h; y++)
#if 1
			memcpy (m_info.parentBitmap.Buffer () + curx + (cury + y) * tw, fp + y * w, w);
#else
			for (x = 0; x < w; x++)
				m_info.parentBitmap [curx + x + (cury + y) * tw] = fp [x + y * w];
#endif
		}
	else {
		int mask, bits = 0;
		//			if (w*h>sizeof (data))
		//				Error ("OglInitFont: toobig\n");
		if (m_info.flags & FT_PROPORTIONAL)
			fp = m_info.chars [i];
		else
			fp = m_info.data + i * BITS_TO_BYTES (w) * h;
		for (y = 0; y < h; y++) {
			mask = 0;
			for (x = 0; x < w; x++) {
				if (mask == 0) {
					bits = *fp++;
					mask = 0x80;
					}
				m_info.parentBitmap [curx + x + (cury + y) * tw] = (bits & mask) ? white : 255;
				mask >>= 1;
				}
			}
		}
	m_info.bitmaps [i].InitChild (&m_info.parentBitmap, curx, cury, w, h);
	curx += w + gap;
	}
if (!(m_info.flags & FT_COLOR)) {
	m_info.parentBitmap.PrepareTexture (0, 2, 0, NULL);
	//use GL_INTENSITY instead of GL_RGB
	if (gameStates.ogl.bIntensity4) {
		m_info.parentBitmap.Texture ()->SetInternalFormat (1);
		m_info.parentBitmap.Texture ()->SetFormat (GL_LUMINANCE);
		}
	else if (gameStates.ogl.bLuminance4Alpha4){
		m_info.parentBitmap.Texture ()->SetInternalFormat (1);
		m_info.parentBitmap.Texture ()->SetFormat (GL_LUMINANCE_ALPHA);
		}
	else {
		m_info.parentBitmap.Texture ()->SetInternalFormat (gameStates.ogl.bpp / 8);
		m_info.parentBitmap.Texture ()->SetFormat (gameStates.ogl.nRGBAFormat);
		}
	}
}

//------------------------------------------------------------------------------

//remap a font by re-reading its data & palette
void CFont::Setup (const char *fontname, ubyte* fontData, CPalette& palette)
{
	int	i;
	int	nChars;
	ubyte	*ptr;
	int	freq [256];

if (m_info.chars) {
	delete [] m_info.chars;
	m_info.chars = NULL;
	}
if (m_info.bitmaps) {
	delete [] m_info.bitmaps;
	m_info.bitmaps = NULL;
	}
m_info.parentBitmap.Destroy ();

// make these offsets relative to font data
m_info.data = (ubyte *) ((size_t) m_info.data - GRS_FONT_SIZE);
m_info.widths = (short *) ((size_t) m_info.widths - GRS_FONT_SIZE);
m_info.kernData = (ubyte *) ((size_t) m_info.kernData - GRS_FONT_SIZE);

nChars = m_info.maxChar - m_info.minChar + 1;

if (m_info.flags & FT_PROPORTIONAL) {
	m_info.widths = (short *) (fontData + (size_t) m_info.widths);
	m_info.data = (ubyte *) (fontData + (size_t) m_info.data);
	m_info.chars = new ubyte* [nChars];
	ptr = m_info.data;
	for (i = 0; i < nChars; i++) {
		m_info.widths [i] = INTEL_SHORT (m_info.widths [i]);
		m_info.chars [i] = ptr;
		if (m_info.flags & FT_COLOR)
			ptr += m_info.widths [i] * m_info.height;
		else
			ptr += BITS_TO_BYTES (m_info.widths [i]) * m_info.height;
		}
	}
else  {
	m_info.data = (ubyte *) fontData;
	m_info.chars = NULL;
	m_info.widths = NULL;
	ptr = m_info.data + (nChars * m_info.width * m_info.height);
	}
if (m_info.flags & FT_KERNED)
	m_info.kernData = (ubyte *) (fontData + (size_t)m_info.kernData);
m_info.parentBitmap.Destroy ();
if (m_info.flags & FT_COLOR) {		//remap palette
#ifdef SWAP_0_255			// swap the first and last palette entries (black and white)
	palette.SwapTransparency ();

//  we also need to swap the data entries as well.  black is white and white is black

	for (i = 0; i < ptr-m_info.data; i++) {
		if (m_info.data [i] == 0)
			m_info.data [i] = 255;
		else if (m_info.data [i] == 255)
			m_info.data [i] = 0;
		}
	}
#endif
	memset (freq, 0, sizeof (freq));
	CountColors (m_info.data, (int) (ptr - m_info.data), freq);
	m_info.parentBitmap.SetPalette (&palette, TRANSPARENCY_COLOR, -1, freq);
	}
else
	m_info.parentBitmap.SetPalette (paletteManager.Default (), TRANSPARENCY_COLOR, -1, freq);

Create (fontname);
}

//------------------------------------------------------------------------------

ubyte* CFont::Load (const char *fontname, ubyte* fontData)
{
	static int bFirstTime = 1;
	
	CFile 	cf;
	CPalette	palette;
	char 		fileId [4];
	int 		dataSize;	//size up to (but not including) palette

	//find D2_FREE font slot
if (!cf.Open (fontname, gameFolders.szDataDir, "rb", 0)) {
#if TRACE
	con_printf (CON_VERBOSE, "Can't open font file %s\n", fontname);
#endif
	return NULL;
	}

cf.Read (fileId, 4, 1);
if (!strncmp (fileId, "NFSP", 4)) {
#if TRACE
	con_printf (CON_NORMAL, "File %s is not a font file\n", fontname);
#endif
	return NULL;
	}

dataSize = cf.ReadInt ();
dataSize -= GRS_FONT_SIZE; // subtract the size of the header.
Read (cf);
if (!(fontData || (fontData = new ubyte [dataSize]))) {
	cf.Close ();
	return NULL;
	}
cf.Read (fontData, 1, dataSize);
if (m_info.flags & FT_COLOR)
	palette.Read (cf);
cf.Close ();

Setup (fontname, fontData, palette);
return fontData;
}

//------------------------------------------------------------------------------

void CFont::Destroy (void)
{
if (m_info.chars) {
	delete m_info.chars;
	m_info.chars = NULL;
	}
if (m_info.bitmaps) {
	delete[] m_info.bitmaps; //D2_FREE (m_info.bitmaps);
	m_info.bitmaps = NULL;
	}
m_info.parentBitmap.Destroy ();
}

//------------------------------------------------------------------------------

void CFont::Read (CFile& cf)
{
m_info.width = cf.ReadShort ();
m_info.height = cf.ReadShort ();
m_info.flags = cf.ReadShort ();
m_info.baseLine = cf.ReadShort ();
m_info.minChar = cf.ReadByte ();
m_info.maxChar = cf.ReadByte ();
m_info.byteWidth = cf.ReadShort ();
m_info.data = (ubyte *) (size_t) cf.ReadInt ();
m_info.chars = (ubyte **) (size_t) cf.ReadInt ();
m_info.widths = (short *) (size_t) cf.ReadInt ();
m_info.kernData = (ubyte *) (size_t) cf.ReadInt ();
}

//------------------------------------------------------------------------------

void CFont::StringSize (const char *s, int& stringWidth, int& stringHeight, int& averageWidth)
{
	int i = 0, longestWidth = 0, nTab = 0;
	int width, spacing;

stringHeight = m_info.height;
stringWidth = 0;
averageWidth = m_info.width;

if (!(s && *s))
	return;

while (*s) {
	while (*s == '\n') {
		s++;
		stringHeight += m_info.height + 2;
		stringWidth = 0;
		nTab = 0;
		}

	if (*s == 0) 
		break;

	//	1 = next byte specifies color, so skip the 1 and the color value
	if (*s == CC_COLOR)
		s += 2;
	else if (*s == CC_LSPACING) {
		stringHeight += * (s+1)-'0';
		s += 2;
		}
	else {
		GetCharWidth (s[0], s[1], width, spacing);
		stringWidth += spacing;
		if (stringWidth > longestWidth)
			longestWidth = stringWidth;
		i++;
		s++;
		}
	}
stringWidth = longestWidth;
}

//------------------------------------------------------------------------------

void CFont::StringSizeTabbed (const char *s, int& stringWidth, int& stringHeight, int& averageWidth, int *nTabs, int nMaxWidth)
{
stringHeight = m_info.height;
stringWidth = 0;
averageWidth = m_info.width;

if (!(s && *s))
	return;

	char	*pi, *pj;
	int	w = 0, nTab = 0;
	static char	hs [10000];

strncpy (hs, s, sizeof (hs));
pi = hs;
do {
	pj = strchr (pi, '\t');
	if (pj)
		*pj = '\0';
	FONT->StringSize (pi, w, stringHeight, averageWidth);
	if (nTab && nTabs) {
		stringWidth = LHX (nTabs [nTab - 1]);
		if (gameStates.multi.bSurfingNet)
			stringWidth += w;
		else
			stringWidth += nMaxWidth;
		}
	else
		stringWidth += w;
	if (pj) {
		nTab++;
		pi = pj + 1;
		}
	} while (pj);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

const char* gameFontFilenames [] = { 
	"font1-1.fnt",      // Font 0
	"font1-1h.fnt",     // Font 0 High-res
	"font2-1.fnt",      // Font 1
	"font2-1h.fnt",     // Font 1 High-res
	"font2-2.fnt",      // Font 2
	"font2-2h.fnt",     // Font 2 High-res
	"font2-3.fnt",      // Font 3
	"font2-3h.fnt",     // Font 3 High-res
	"font3-1.fnt",      // Font 4
	"font3-1h.fnt",     // Font 4 High-res
	};

void CFontManager::Init (void)
{
memset (m_fonts, 0, sizeof (m_fonts));

if (!gameStates.render.fonts.bInstalled) {
		int i;

	gameStates.render.fonts.bInstalled = 1;
	gameStates.render.fonts.bHiresAvailable = 1;
	// load lores fonts
	for (i = 0; i < MAX_FONTS; i += 2)
		m_gameFonts [i] = Load (gameFontFilenames [i]);
	// load hires fonts
	for (i = 1; i < MAX_FONTS; i += 2)
		if (!(m_gameFonts [i] = Load (gameFontFilenames [i])))
			gameStates.render.fonts.bHiresAvailable = 0;
	}
}

//------------------------------------------------------------------------------

void CFontManager::Destroy (void)
{
	int i;

for (i = 0; (i < MAX_OPEN_FONTS); i++)
	if (m_fonts [i].data)
		Close (&m_fonts [i].font);
}

//------------------------------------------------------------------------------

CFont *CFontManager::Load (const char* fontname)
{
	int i;

for (i = 0; (i < MAX_OPEN_FONTS) && m_fonts [i].data; i++)
	;
if (i >= MAX_OPEN_FONTS)
	return NULL;
strncpy (m_fonts [i].filename, fontname, SHORT_FILENAME_LEN);
m_fonts [i].data = m_fonts [i].font.Load (fontname);
CCanvas::Current ()->SetFont (&m_fonts [i].font);
FG_COLOR.index = 0;
BG_COLOR.index = 0;
return &m_fonts [i].font;
}

//------------------------------------------------------------------------------

void CFontManager::Close (CFont* font)
{
if (!font)
	return;

	int	i;

		//find font in list
for (i = 0; (i < MAX_OPEN_FONTS) && (&m_fonts [i].font != font); i++)
		;
if (i >= MAX_OPEN_FONTS)
	return;
delete m_fonts [i].data;
m_fonts [i].data = NULL;
m_fonts [i].font.Destroy ();
}

//------------------------------------------------------------------------------

void CFontManager::SetColor (int fg, int bg)
{
if (fg >= 0)
	FG_COLOR.index = fg;
FG_COLOR.rgb = 0;
if (bg >= 0)
	BG_COLOR.index = bg;
BG_COLOR.rgb = 0;
}

//------------------------------------------------------------------------------

void CFontManager::SetColorRGB (tRgbaColorb *fg, tRgbaColorb *bg)
{
if (fg) {
	FG_COLOR.rgb = 1;
	FG_COLOR.color = *fg;
	}
if (bg) {
	BG_COLOR.rgb = 1;
	BG_COLOR.color = *fg;
	}
}

//------------------------------------------------------------------------------

void CFontManager::SetColorRGBi (unsigned int fg, int bSetFG, unsigned int bg, int bSetBG)
{
if (bSetFG) {
	FG_COLOR.rgb = 1;
	FG_COLOR.color.red = RGBA_RED (fg);
	FG_COLOR.color.green = RGBA_GREEN (fg);
	FG_COLOR.color.blue = RGBA_BLUE (fg);
	FG_COLOR.color.alpha = RGBA_ALPHA (fg);
	}
if (bSetBG) {
	BG_COLOR.rgb = 1;
	BG_COLOR.color.red = RGBA_RED (bg);
	BG_COLOR.color.green = RGBA_GREEN (bg);
	BG_COLOR.color.blue = RGBA_BLUE (bg);
	BG_COLOR.color.alpha = RGBA_ALPHA (bg);
	}
}

//------------------------------------------------------------------------------

void CFontManager::RemapMono (void)
{
for (int i = 0; i < MAX_OPEN_FONTS; i++)
	if (m_fonts [i].data && !(m_fonts [i].font.Flags () & FT_COLOR))
		m_fonts [i].font.Load (m_fonts [i].filename, m_fonts [i].data);
}

//------------------------------------------------------------------------------
//remap (by re-reading) all the color fonts
void CFontManager::RemapColor (void)
{
for (int i = 0; i < MAX_OPEN_FONTS; i++)
	if (m_fonts [i].data && (m_fonts [i].font.Flags () & FT_COLOR))
		m_fonts [i].font.Load (m_fonts [i].filename, m_fonts [i].data);
}

//------------------------------------------------------------------------------
//eof
