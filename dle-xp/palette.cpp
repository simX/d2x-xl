// palette.cpp

#include "stdafx.h"
#include "dlcres.h"

#include <string.h>

#include "define.h"
#include "types.h"
#include "mine.h"
#include "global.h"
#include "io.h"
#include "palette.h"

UINT8 *pCustomPalette = NULL;
HGLOBAL hPalette;

//------------------------------------------------------------------------

int Luminance (int r, int g, int b)
{
	int minColor, maxColor;

if (r < g) {
	minColor = (r < b) ? r : b;
	maxColor = (g > b) ? g : b;
	}
else {
	minColor = (g < b) ? g : b;
	maxColor = (r > b) ? r : b;
	}
return (minColor + maxColor) / 2;
}

//------------------------------------------------------------------------

UINT8 FadeValue (UINT8 c, int f)
{
return (UINT8) (((int) c * f) / 34);
}

//------------------------------------------------------------------------

int HasCustomPalette (void)
{
return pCustomPalette != NULL;
}

//------------------------------------------------------------------------

void FreeCustomPalette (void)
{
if (pCustomPalette) {
	free (pCustomPalette);
	pCustomPalette = NULL;
	}
}

//------------------------------------------------------------------------

int ReadCustomPalette (FILE *fp, long fSize)
{
FreeCustomPalette ();

UINT8 *pCustomPalette = (UINT8 *) malloc (37 * 256);

if (!pCustomPalette)
	return 0;

int h = fread (pCustomPalette, 37 * 256, 1, fp);
if (h == 37 * 256)
	return 1;

if (h != 3 * 256) {
	free (pCustomPalette);
	return 0;
	}

UINT8 *pFade = pCustomPalette + 3 * 256;

int i, j;
for (i = 0; i < 256; i++) {
	UINT8	c = pCustomPalette [i];
	for (j = 0; j < 34; j++)
		pFade [j * 256 + i] = FadeValue (c, j + 1);
	}
return 1;
}

//------------------------------------------------------------------------

int WriteCustomPalette (FILE *fp)
{
return fwrite (pCustomPalette, 37 * 256, 1, fp) == 37 * 256;
}

//------------------------------------------------------------------------

UINT8 * PalettePtr (void)
{
if (pCustomPalette)
	return pCustomPalette;
HINSTANCE hInst = AfxGetInstanceHandle();
HRSRC hResource = FindResource (hInst, PaletteResource (), "RC_DATA");
if (!hResource)
	return NULL;
hPalette = LoadResource (hInst, hResource);
if (!hPalette)
	return NULL;
return (UINT8 *) LockResource (hPalette);
}

//------------------------------------------------------------------------

void FreePaletteResource ()
{
if (!pCustomPalette && hPalette) {
	FreeResource (hPalette);
	hPalette = 0;
	}
}

//------------------------------------------------------------------------
// PaletteResource()
//
// Action - returns the name of the palette resource.  Neat part about
//          this function is that the strings are automatically stored
//          in the local heap so the string is static.
//
//------------------------------------------------------------------------

LPCTSTR PaletteResource () 
{
	typedef struct tPalExt {
		char	szFile [13];
		int	nIdPal;
	} tPalExt;

	static tPalExt palExt [] = {
		{"groupa", IDR_GROUPA_256}, 
		{"alien1", IDR_ALIEN1_256}, 
		{"alien2", IDR_ALIEN2_256}, 
		{"fire", IDR_FIRE_256}, 
		{"water", IDR_WATER_256}, 
		{"ice", IDR_ICE_256},
		{"", 0}
	};
	tPalExt	*ppe;
	char		szFile [13];

int id = IDR_GROUPA_256;
if (file_type == RDL_FILE)
	return MAKEINTRESOURCE (IDR_PALETTE_256);
FSplit (descent2_path, NULL, szFile, NULL);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
/*
LPCTSTR	pszRsc;
int len = strlen(descent2_path);
char *str;
str = "ALIEN1_256";
if (_strnicmp(&descent2_path[len-strlen(str)],str,6)==0) id = IDR_ALIEN1_256;
str = "ALIEN2_256";
if (_strnicmp(&descent2_path[len-strlen(str)],str,6)==0) id = IDR_ALIEN2_256;
str = "FIRE_256";
if (_strnicmp(&descent2_path[len-strlen(str)],str,4)==0) id = IDR_FIRE_256;
str = "WATER_256";
if (_strnicmp(&descent2_path[len-strlen(str)],str,5)==0) id = IDR_WATER_256;
str = "ICE_256";
if (_strnicmp(&descent2_path[len-strlen(str)],str,3)==0) id = IDR_FIRE_256;
}
pszRsc = MAKEINTRESOURCE(id);
return pszRsc;
*/
}
