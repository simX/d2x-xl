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

#define DEBUG_LEVEL CON_NORMAL

#include <string.h>
#ifndef _WIN32_WCE
#	include <sys/types.h>
#endif
#ifndef _WIN32
#	include <unistd.h>
#endif
#include <ctype.h>

#include "inferno.h"
#include "movie.h"
#include "key.h"
#include "strutil.h"
#include "error.h"
#include "u_mem.h"
#include "byteswap.h"
#include "ogl_bitmap.h"
#include "gamefont.h"
#include "libmve.h"
#include "text.h"
#include "screens.h"

extern char CDROM_dir [];

// Subtitle data
typedef struct {
	short first_frame, last_frame;
	char *msg;
} subtitle;

#define MAX_SUBTITLES 500
#define MAX_ACTIVE_SUBTITLES 3

typedef struct tSubTitles {
	subtitle captions [MAX_SUBTITLES];
	int		nCaptions;
	ubyte		*rawDataP;
} tSubTitles;

tSubTitles subTitles;

// Movielib data
typedef struct {
	char name [FILENAME_LEN];
	int offset, len;
} ml_entry;

typedef struct {
	char     name [100]; // [FILENAME_LEN];
	int      n_movies;
	ubyte    flags;
	ubyte		pad [3];
	ml_entry *movies;
	int		bLittleEndian;
} tMovieLib;

char pszMovieLibs [][FILENAME_LEN] = {
	"intro-l.mvl", 
	"other-l.mvl", 
	"robots-l.mvl", 
	"d2x-l.mvl", 
	"extra1-l.mvl", 
	"extra2-l.mvl", 
	"extra3-l.mvl", 
	"extra4-l.mvl", 
	"extra5-l.mvl"};

#define MLF_ON_CD						1
#define MAX_MOVIES_PER_LIB			50    //determines size of D2_ALLOC

#define	FIRST_EXTRA_MOVIE_LIB	4
#define N_EXTRA_MOVIE_LIBS			5
#define N_BUILTIN_MOVIE_LIBS		(sizeof (pszMovieLibs) / sizeof (*pszMovieLibs))
#define N_MOVIE_LIBS					(N_BUILTIN_MOVIE_LIBS+1)
#define EXTRA_ROBOT_LIB				N_BUILTIN_MOVIE_LIBS

typedef struct tRobotMovie {
	CFile		cf;
	int		nFilePos;
	int		bLittleEndian;
} tRobotMovie;

typedef struct tMovieData {
	tMovieLib	*libs [N_MOVIE_LIBS];
	CPalette		*palette;
	tRobotMovie	robot;
} tMovieData;

tMovieData	movies;

// Function Prototypes
int RunMovie (char *filename, int highresFlag, int allow_abort, int dx, int dy);

int OpenMovieFile (CFile& cf, char *filename, int bRequired);
int ResetMovieFile (CFile& cf);

void DecodeTextLine (char *p);
void DrawSubTitles (int nFrame);
tMovieLib *FindMovieLib (const char *pszTargetMovie);

// ----------------------------------------------------------------------

void *MPlayAlloc (unsigned size)
{
return D2_ALLOC (size);
}

// ----------------------------------------------------------------------

void MPlayFree (void *p)
{
D2_FREE (p);
}


//-----------------------------------------------------------------------

uint FileRead (void *handle, void *buf, uint count)
{
uint numread = (uint) (reinterpret_cast<CFile*> (handle))->Read (buf, 1, count);
return numread == count;
}


//-----------------------------------------------------------------------

//filename will actually get modified to be either low-res or high-res
//returns status.  see values in movie.h

int PlayMovie (const char *filename, int bRequired, int bForce, int bFullScreen)
{
	char name [FILENAME_LEN], *p;
	int c, ret;

#if 1//ndef _DEBUG
if (!bForce && (gameOpts->movies.nLevel < 2))
	return MOVIE_NOT_PLAYED;
#endif
strcpy (name, filename);
if (!(p = strchr (name, '.')))		//add extension, if missing
	strcat (name, ".mve");
//check for escape already pressed & abort if so
while ((c = KeyInKey ()))
	if (c == KEY_ESC)
		return MOVIE_ABORTED;
// Stop all digital sounds currently playing.
DigiExit ();
// Start sound
MVE_sndInit (gameStates.app.bUseSound ? 1 : -1);
gameOpts->movies.bFullScreen = bFullScreen;
ret = RunMovie (name, gameOpts->movies.bHires, bRequired, -1, -1);
DigiInit (1);
gameStates.video.nScreenMode = -1;		//force screen reset
return ret;
}

//-----------------------------------------------------------------------

void MovieShowFrame (ubyte *buf, uint bufw, uint bufh, uint sx, uint sy, 
							uint w, uint h, uint dstx, uint dsty)
{
	CBitmap bmFrame;

Assert (bufw == w && bufh == h);

memset (&bmFrame, 0, sizeof (bmFrame));
bmFrame.Init (BM_LINEAR, 0, 0, bufw, bufh, 1, buf);
bmFrame.SetPalette (movies.palette);

TRANSPARENCY_COLOR = -1;
if (gameOpts->menus.nStyle) {
	//memset (grPalette, 0, 768);
	//paletteManager.LoadEffect  ();
	}
if (gameOpts->movies.bFullScreen) {
	double r = (double) bufh / (double) bufw;
	int dh = (int) (CCanvas::Current ()->Width () * r);
	int yOffs = (CCanvas::Current ()->Bitmap ().Height () - dh) / 2;

	glDisable (GL_BLEND);
	OglUBitBltI (CCanvas::Current ()->Width (), dh, 0, yOffs, 
					 bufw, bufh, sx, sy, 
					 &bmFrame, &CCanvas::Current ()->Bitmap (), 
					 gameOpts->movies.nQuality, 1, 1.0f);
	glEnable (GL_BLEND);
	}
else {
	int xOffs = (CCanvas::Current ()->Width () - 640) / 2;
	int yOffs = (CCanvas::Current ()->Bitmap ().Height () - 480) / 2;

	if (xOffs < 0)
		xOffs = 0;
	if (yOffs < 0)
		yOffs = 0;
	dstx += xOffs;
	dsty += yOffs;
	if ((CCanvas::Current ()->Width () > 640) || (CCanvas::Current ()->Bitmap ().Height () > 480)) {
		CCanvas::Current ()->SetColorRGBi (RGBA_PAL (0, 0, 32));
		GrUBox (dstx-1, dsty, dstx+w, dsty+h+1);
		}
	GrBmUBitBlt (bufw, bufh, dstx, dsty, sx, sy, &bmFrame, &CCanvas::Current ()->Bitmap (), 1);
	}
TRANSPARENCY_COLOR = DEFAULT_TRANSPARENCY_COLOR;
}

//-----------------------------------------------------------------------
//our routine to set the palette, called from the movie code
void MovieSetPalette (ubyte *p, unsigned start, unsigned count)
{
	CPalette	palette;

if (count == 0)
	return;

//paletteManager.LoadEffect  ();
//Color 0 should be black, and we get color 255
//movie libs palette into our array
Assert (start>=0 && start+count<=PALETTE_SIZE);
if (start + count > PALETTE_SIZE)
	count = PALETTE_SIZE - start;
memcpy (palette.Color () + start, p + start * 3, count * 3);
//Set color 0 to be black
palette.SetBlack (0, 0, 0);
//Set color 255 to be our subtitle color
palette.SetTransparency (50, 50, 50);
//finally set the palette in the hardware
movies.palette = paletteManager.Add (palette);
paletteManager.LoadEffect  ();
}

//-----------------------------------------------------------------------

#define BOX_BORDER (gameStates.menus.bHires ? 40 : 20)

void ShowPauseMessage (const char *msg)
{
	int w, h, aw;
	int x, y;

CCanvas::SetCurrent (NULL);
fontManager.SetCurrent (SMALL_FONT);
FONT->StringSize (msg, w, h, aw);
x = (screen.Width () - w) / 2;
y = (screen.Height () - h) / 2;
#if 0
if (movie_bg.bmp) {
	GrFreeBitmap (movie_bg.bmp);
	movie_bg.bmp = NULL;
	}
// Save the background of the display
movie_bg.x=x; 
movie_bg.y=y; 
movie_bg.w=w; 
movie_bg.h=h;
movie_bg.bmp = GrCreateBitmap (w+BOX_BORDER, h+BOX_BORDER, 1);
GrBmUBitBlt (w+BOX_BORDER, h+BOX_BORDER, 0, 0, x-BOX_BORDER/2, y-BOX_BORDER/2, & (CCanvas::Current ()->Bitmap ()), movie_bg.bmp);
#endif
CCanvas::Current ()->SetColorRGB (0, 0, 0, 255);
GrRect (x-BOX_BORDER/2, y-BOX_BORDER/2, x+w+BOX_BORDER/2-1, y+h+BOX_BORDER/2-1);
fontManager.SetColor (255, -1);
GrUString (0x8000, y, msg);
GrUpdate (0);
}

//-----------------------------------------------------------------------

void ClearPauseMessage ()
{
#if 0
if (movie_bg.bmp) {
	GrBitmap (movie_bg.x-BOX_BORDER/2, movie_bg.y-BOX_BORDER/2, movie_bg.bmp);
	GrFreeBitmap (movie_bg.bmp);
	movie_bg.bmp = NULL;
	}
#endif
}

//-----------------------------------------------------------------------
//returns status.  see movie.h
int RunMovie (char *filename, int hiresFlag, int bRequired, int dx, int dy)
{
	CFile			cf;
	int			result=1, aborted=0;
	int			track = 0;
	int			nFrame;
	int			key;
	tMovieLib	*libP = FindMovieLib (filename);

result = 1;
// Open Movie file.  If it doesn't exist, no movie, just return.
if (!(cf.Open (filename, gameFolders.szDataDir, "rb", 0) || OpenMovieFile (cf, filename, bRequired))) {
	if (bRequired) {
#if TRACE
		con_printf (CON_NORMAL, "movie: RunMovie: Cannot open movie <%s>\n", filename);
#endif
		}
	return MOVIE_NOT_PLAYED;
	}
MVE_memCallbacks (MPlayAlloc, MPlayFree);
MVE_ioCallbacks (FileRead);
SetScreenMode (SCREEN_MENU);
paletteManager.LoadEffect  ();
MVE_sfCallbacks (MovieShowFrame);
MVE_palCallbacks (MovieSetPalette);
if (MVE_rmPrepMovie (reinterpret_cast<void*> (&cf), dx, dy, track, libP ? libP->bLittleEndian : 1)) {
	Int3 ();
	return MOVIE_NOT_PLAYED;
	}
nFrame = 0;
gameStates.render.fonts.bHires = gameStates.render.fonts.bHiresAvailable && hiresFlag;
while ((result = MVE_rmStepMovie ()) == 0) {
	DrawSubTitles (nFrame);
	paletteManager.LoadEffect  (); // moved this here because of flashing
	GrUpdate (1);
	key = KeyInKey ();
	// If ESCAPE pressed, then quit movie.
	if (key == KEY_ESC) {
		result = aborted = 1;
		break;
		}
	// If PAUSE pressed, then pause movie
	if (key == KEY_PAUSE) {
		MVE_rmHoldMovie ();
		ShowPauseMessage (TXT_PAUSE);
		while (!KeyInKey ()) 
			;
		ClearPauseMessage ();
		}
	if ((key == KEY_ALTED+KEY_ENTER) || (key == KEY_ALTED+KEY_PADENTER))
		GrToggleFullScreen ();
	nFrame++;
	}
Assert (aborted || result == MVE_ERR_EOF);	 ///movie should be over
MVE_rmEndMovie ();
cf.Close ();                           // Close Movie File
// Restore old graphic state
gameStates.video.nScreenMode = -1;  //force reset of screen mode
paletteManager.LoadEffect  ();
return (aborted ? MOVIE_ABORTED : MOVIE_PLAYED_FULL);
}

//-----------------------------------------------------------------------

int InitMovieBriefing ()
{
return 1;
}

//-----------------------------------------------------------------------
//returns 1 if frame updated ok
int RotateRobot ()
{
	int err;

gameOpts->movies.bFullScreen = 1;
if (gameStates.ogl.nDrawBuffer == GL_BACK)
	paletteManager.LoadEffect  ();
err = MVE_rmStepMovie ();
paletteManager.LoadEffect  ();
if (err == MVE_ERR_EOF) {   //end of movie, so reset
	ResetMovieFile (movies.robot.cf);
	if (MVE_rmPrepMovie (reinterpret_cast<void*> (&movies.robot.cf), 
								gameStates.menus.bHires ? 280 : 140, 
								gameStates.menus.bHires ? 200 : 80, 0,
								movies.robot.bLittleEndian)) {
		Int3 ();
		return 0;
		}
	}
else if (err) {
	Int3 ();
	return 0;
	}
return 1;
}

//-----------------------------------------------------------------------

void DeInitRobotMovie (void)
{
MVE_rmEndMovie ();
movies.robot.cf.Close ();                           // Close Movie File
}

//-----------------------------------------------------------------------

int InitRobotMovie (char *filename)
{
	tMovieLib	*libP = FindMovieLib (filename);

if (gameOpts->movies.nLevel < 1)
	return 0;

#if TRACE
con_printf (DEBUG_LEVEL, "movies.robot.cf=%s\n", filename);
#endif
MVE_sndInit (-1);        //tell movies to play no sound for robots
if (!OpenMovieFile (movies.robot.cf, filename, 1)) {
#if DBG
	Warning (TXT_MOVIE_ROBOT, filename);
#endif
	return MOVIE_NOT_PLAYED;
	}
gameOpts->movies.bFullScreen = 1;
movies.robot.bLittleEndian = libP ? libP->bLittleEndian : 1;
MVE_memCallbacks (MPlayAlloc, MPlayFree);
MVE_ioCallbacks (FileRead);
MVE_sfCallbacks (MovieShowFrame);
MVE_palCallbacks (MovieSetPalette);
if (MVE_rmPrepMovie (reinterpret_cast<void*> (&movies.robot.cf), 
							gameStates.menus.bHires ? 280 : 140, 
							gameStates.menus.bHires ? 200 : 80, 0,
							movies.robot.bLittleEndian)) {
	Int3 ();
	return 0;
	}
movies.robot.nFilePos = movies.robot.cf.Seek (0L, SEEK_CUR);
#if TRACE
con_printf (DEBUG_LEVEL, "movies.robot.nFilePos=%d!\n", movies.robot.nFilePos);
#endif
return 1;
}

//-----------------------------------------------------------------------
/*
 *		Subtitle system code
 */

//search for next field following whitespace 
ubyte *next_field (ubyte *p)
{
	while (*p && !::isspace (*p))
	p++;
if (!*p)
	return NULL;
while (*p && ::isspace (*p))
	p++;
if (!*p)
	return NULL;
return p;
}

//-----------------------------------------------------------------------

int InitSubTitles (const char *filename)
{
	CFile cf;
	int 	size, readCount;
	ubyte	*p;
	int 	bHaveBinary = 0;

subTitles.nCaptions = 0;
if (!gameOpts->movies.bSubTitles)
	return 0;
if (!cf.Open (filename, gameFolders.szDataDir, "rb", 0)) { // first try text version
	char filename2 [FILENAME_LEN];	//no text version, try binary version
	CFile::ChangeFilenameExtension (filename2, filename, ".txb");
	if (!cf.Open (filename2, gameFolders.szDataDir, "rb", 0))
		return 0;
	bHaveBinary = 1;
	}

size = cf.Length ();
MALLOC (subTitles.rawDataP, ubyte, size+1);
readCount = (int) cf.Read (subTitles.rawDataP, 1, size);
cf.Close ();
subTitles.rawDataP [size] = 0;
if (readCount != size) {
	D2_FREE (subTitles.rawDataP);
	return 0;
	}
p = subTitles.rawDataP;
while (p && (p < subTitles.rawDataP + size)) {
	char *endp = strchr (reinterpret_cast<char*> (p), '\n'); 

	if (endp) {
		if (endp [-1] == '\r')
			endp [-1] = 0;		//handle 0d0a pair
		*endp = 0;			//string termintor
		}
	if (bHaveBinary)
		DecodeTextLine (reinterpret_cast<char*> (p));
	if (*p != ';') {
		subTitles.captions [subTitles.nCaptions].first_frame = atoi (reinterpret_cast<char*> (p));
		if (!(p = next_field (p))) 
			continue;
		subTitles.captions [subTitles.nCaptions].last_frame = atoi (reinterpret_cast<char*> (p));
		if (!(p = next_field (p)))
			continue;
		subTitles.captions [subTitles.nCaptions].msg = reinterpret_cast<char*> (p);
		Assert (subTitles.nCaptions==0 || subTitles.captions [subTitles.nCaptions].first_frame >= subTitles.captions [subTitles.nCaptions-1].first_frame);
		Assert (subTitles.captions [subTitles.nCaptions].last_frame >= subTitles.captions [subTitles.nCaptions].first_frame);
		subTitles.nCaptions++;
		}
	p = reinterpret_cast<ubyte*> (endp + 1);
	}
return 1;
}

//-----------------------------------------------------------------------

void CloseSubTitles (void)
{
if (subTitles.rawDataP)
	delete[] subTitles.rawDataP;
subTitles.rawDataP = NULL;
subTitles.nCaptions = 0;
}

//-----------------------------------------------------------------------
//draw the subtitles for this frame
void DrawSubTitles (int nFrame)
{
	static int activeSubTitleList [MAX_ACTIVE_SUBTITLES];
	static int nActiveSubTitles, nNextSubTitle, nLineSpacing;
	int t, y;
	int bMustErase = 0;

if (nFrame == 0) {
	nActiveSubTitles = 0;
	nNextSubTitle = 0;
	fontManager.SetCurrent (GAME_FONT);
	nLineSpacing = CCanvas::Current ()->Font ()->Height () + (CCanvas::Current ()->Font ()->Height () / 4);
	fontManager.SetColor (255, -1);
	}

//get rid of any subtitles that have expired
for (t = 0; t <nActiveSubTitles; )
	if (nFrame > subTitles.captions [activeSubTitleList [t]].last_frame) {
		int t2;
		for (t2 = t; t2 < nActiveSubTitles - 1; t2++)
			activeSubTitleList [t2] = activeSubTitleList [t2+1];
		nActiveSubTitles--;
		bMustErase = 1;
	}
	else
		t++;

//get any subtitles new for this frame 
while (nNextSubTitle < subTitles.nCaptions && nFrame >= subTitles.captions [nNextSubTitle].first_frame) {
	if (nActiveSubTitles >= MAX_ACTIVE_SUBTITLES)
		Error ("Too many active subtitles!");
	activeSubTitleList [nActiveSubTitles++] = nNextSubTitle;
	nNextSubTitle++;
	}

//find y coordinate for first line of subtitles
y = CCanvas::Current ()->Bitmap ().Height () - ((nLineSpacing+1)*MAX_ACTIVE_SUBTITLES+2);

//erase old subtitles if necessary
if (bMustErase) {
	CCanvas::Current ()->SetColorRGB (0, 0, 0, 255);
	GrRect (0, y, CCanvas::Current ()->Width ()-1, CCanvas::Current ()->Bitmap ().Height ()-1);
	}
//now draw the current subtitles
for (t=0;t<nActiveSubTitles;t++)
	if (activeSubTitleList [t] != -1) {
		GrString (0x8000, y, subTitles.captions [activeSubTitleList [t]].msg, NULL);
		y += nLineSpacing+1;
	}
}

//-----------------------------------------------------------------------

tMovieLib *InitNewMovieLib (const char *filename, CFile& cf)
{
	int		nFiles, offset;
	int		i, n, len, bLittleEndian = gameStates.app.bLittleEndian;
	tMovieLib *table;

	//read movie file header

nFiles = cf.ReadInt ();        //get number of files
if (nFiles > 255) {
	gameStates.app.bLittleEndian = 0;
	nFiles = SWAPINT (nFiles);
	}
//table = D2_ALLOC (sizeof (*table) + sizeof (ml_entry)*nFiles);
MALLOC (table, tMovieLib, 1);
MALLOC (table->movies, ml_entry, nFiles);
strcpy (table->name, filename);
table->bLittleEndian = gameStates.app.bLittleEndian;
table->n_movies = nFiles;
offset = 4 + 4 + nFiles * (13 + 4);	//id + nFiles + nFiles * (filename + size)
for (i = 0; i < nFiles; i++) {
	n = (int) cf.Read (table->movies [i].name, 13, 1);
	if (n != 1)
		break;		//end of file (probably)
	len = cf.ReadInt ();
	table->movies [i].len = len;
	table->movies [i].offset = offset;
	offset += table->movies [i].len;
	}
cf.Close ();
table->flags = 0;
gameStates.app.bLittleEndian = bLittleEndian;
return table;
}

//-----------------------------------------------------------------------

tMovieLib *InitOldMovieLib (const char *filename, CFile& cf)
{
	int nFiles, size;
	int i, len;
	tMovieLib *table, *table2;

	nFiles = 0;

	//allocate big table
table = reinterpret_cast<tMovieLib*> (D2_ALLOC (sizeof (*table) + sizeof (ml_entry) * MAX_MOVIES_PER_LIB));
while (1) {
	i = (int) cf.Read (table->movies [nFiles].name, 13, 1);
	if (i != 1)
		break;		//end of file (probably)
	i = (int) cf.Read (&len, 4, 1);
	if (i != 1) {
		Error ("error reading movie library <%s>", filename);
		return NULL;
		}
	table->movies [nFiles].len = INTEL_INT (len);
	table->movies [nFiles].offset = cf.Tell ();
	cf.Seek (INTEL_INT (len), SEEK_CUR);       //skip data
	nFiles++;
	}
	//allocate correct-sized table
size = sizeof (*table) + sizeof (ml_entry) * nFiles;
table2 = reinterpret_cast<tMovieLib*> (D2_ALLOC (size));
memcpy (table2, table, size);
D2_FREE (table);
table = table2;
strcpy (table->name, filename);
table->n_movies = nFiles;
cf.Close ();
table->flags = 0;
return table;
}

//-----------------------------------------------------------------------

//find the specified movie library, and read in list of movies in it
tMovieLib *InitMovieLib (const char *filename)
{
	//note: this based on CFInitHogFile ()

	char id [4];
	CFile cf;

if (!cf.Open (filename, gameFolders.szMovieDir, "rb", 0))
	return NULL;
cf.Read (id, 4, 1);
if (!strncmp (id, "DMVL", 4))
	return InitNewMovieLib (filename, cf);
else if (!strncmp (id, "DHF", 3)) {
	cf.Seek (-1, SEEK_CUR);		//old file had 3 char id
	return InitOldMovieLib (filename, cf);
	}
else {
	cf.Close ();
	return NULL;
	}
}

//-----------------------------------------------------------------------

//ask user to put the D2 CD in.
//returns -1 if ESC pressed, 0 if OK chosen
//CD may not have been inserted
int RequestCD (void)
{
#if 0
	ubyte save_pal [256*3];
	CCanvas *tcanv, canvP = CCanvas::Current ();
	int ret, was_faded=gameStates.render.bPaletteFadedOut;

	GrPaletteStepClear ();

	CCanvas::Push ();
	tcanv = GrCreateCanvas (CCanvas::Current ()->bm.Width (), CCanvas::Current ()->bm.Height ());

	CCanvas::SetCurrent (tcanv);
	gr_ubitmap (0, 0, &canvP->Bitmap ());
	CCanvas::Pop ();

	GrClearCanvas (0);

 try_again:;

	ret = ExecMessageBox ("CD ERROR", 1, "Ok", "Please insert your Descent II CD");

	if (ret == -1) {
		int ret2;

		ret2 = ExecMessageBox ("CD ERROR", 2, "Try Again", "Leave Game", "You must insert your\nDescent II CD to Continue");

		if (ret2 == -1 || ret2 == 0)
			goto try_again;
	}
	force_rb_register = 1;  //disc has changed; force register new CD
	GrPaletteStepClear ();
	gr_ubitmap (0, 0, &tcanv->Bitmap ());
	if (!was_faded)
		paletteManager.LoadEffect  ();
	GrFreeCanvas (tcanv);
	return ret;
#else
#if TRACE
	con_printf (DEBUG_LEVEL, "STUB: movie: RequestCD\n");
#endif
	return 0;
#endif
}

//-----------------------------------------------------------------------

void InitMovie (const char *pszFilename, int nLibrary, int bIsRobots, int bRequired)
{
	int bHighRes, nTries;
	char filename [FILENAME_LEN];

strcpy (filename, pszFilename);
	
	char *res = strchr (filename, '.') - 1; // 'h' == high resolution, 'l' == low

#if 0//DBG
	if (FindArg ("-nomovies")) {
		movies.libs [nLibrary] = NULL;
		return;
	}
#endif

	//for robots, load highres versions if highres menus set
	bHighRes = bIsRobots ? gameStates.menus.bHiresAvailable : gameOpts->movies.bHires;
	if (bHighRes)
		*res = 'h';
	for (nTries = 0; (movies.libs [nLibrary] = InitMovieLib (filename)) == NULL; nTries++) {
		char name2 [100];

		strcpy (name2, CDROM_dir);
		strcat (name2, filename);
		movies.libs [nLibrary] = InitMovieLib (name2);

		if (movies.libs [nLibrary]) {
			movies.libs [nLibrary]->flags |= MLF_ON_CD;
			break; // we found our movie on the CD
			}
		else {
			if (nTries == 0) { // first nTries
				if (*res == 'h') { // nTries low res instead
					*res = 'l';
					bHighRes = 0;
					}
				else if (*res == 'l') { // nTries high
					*res = 'h';
					bHighRes = 1;
					}
				else {
#if DBG
					if (bRequired)
						Warning (TXT_MOVIE_FILE, filename);
#endif
					break;
					}
				}
			else { // nTries == 1
				if (bRequired) {
					*res = '*';
#if DBG
					//Warning (TXT_MOVIE_ANY, filename);
#endif
				}
				break;
			}
		}
	}

	if (bIsRobots && movies.libs [nLibrary] != NULL)
		gameStates.movies.nRobots = bHighRes ? 2 : 1;
}

//-----------------------------------------------------------------------

void close_movie (int i)
{
if (movies.libs [i]) {
	D2_FREE (movies.libs [i]->movies);
	D2_FREE (movies.libs [i]);
	}
}

//-----------------------------------------------------------------------

void _CDECL_ CloseMovies (void)
{
	uint i;

PrintLog ("unloading movies\n");
for (i = 0; i < N_MOVIE_LIBS; i++)
	close_movie (i);
}

//-----------------------------------------------------------------------

static int bMoviesInited = 0;
//find and initialize the movie libraries
void InitMovies ()
{
	uint i, j;
	int bIsRobots;

	j = (gameStates.app.bHaveExtraMovies = !gameStates.app.bNostalgia) ? 
		 N_BUILTIN_MOVIE_LIBS : FIRST_EXTRA_MOVIE_LIB;
	for (i = 0; i < N_BUILTIN_MOVIE_LIBS; i++) {
		bIsRobots = !strnicmp (pszMovieLibs[i], "robot", 5);
		InitMovie (pszMovieLibs[i], i, bIsRobots, 1);
		if (movies.libs [i])
			PrintLog ("   found movie lib '%s'\n", pszMovieLibs[i]);
		else if ((i >= FIRST_EXTRA_MOVIE_LIB) && 
			 (i < FIRST_EXTRA_MOVIE_LIB + N_EXTRA_MOVIE_LIBS))
			gameStates.app.bHaveExtraMovies = 0;
		}

	movies.libs [EXTRA_ROBOT_LIB] = NULL;
	bMoviesInited = 1;
	atexit (CloseMovies);
}

//-----------------------------------------------------------------------

void InitExtraRobotMovie (char *filename)
{
	close_movie (EXTRA_ROBOT_LIB);
	InitMovie (filename, EXTRA_ROBOT_LIB, 1, 0);
}

//-----------------------------------------------------------------------

CFile cfMovies;
int nMovieStart = -1;

//looks through a movie library for a movie file
//returns filehandle, with fileposition at movie, or -1 if can't find
int SearchMovieLib (CFile& cf, tMovieLib *lib, char *filename, int bRequired)
{
	int i, bFromCD;

if (!lib)
	return 0;
for (i = 0; i < lib->n_movies; i++)
	if (!stricmp (filename, lib->movies [i].name)) {	//found the movie in a library 
		if ((bFromCD = (lib->flags & MLF_ON_CD)))
			SongsStopRedbook ();		//ready to read from CD
		do {		//keep trying until we get the file handle
			cf.Open (lib->name, gameFolders.szMovieDir, "rb", 0);
			if (bRequired && bFromCD && !cf.File ()) {   //didn't get file!
				if (RequestCD () == -1)		//ESC from requester
					break;						//bail from here. will get error later
				}
			} while (bRequired && bFromCD && !cf.File ());
		if (cf.File ())
			cf.Seek (nMovieStart = lib->movies [i].offset, SEEK_SET);
		return 1;
	}
return 0;
}

//-----------------------------------------------------------------------
//returns file handle
int OpenMovieFile (CFile& cf, char *filename, int bRequired)
{
	uint i;

for (i = 0; i < N_MOVIE_LIBS; i++)
	if (SearchMovieLib (cf, movies.libs [i], filename, bRequired))
		return 1;
return 0;    //couldn't find it
}

//-----------------------------------------------------------------------
//sets the file position to the start of this already-open file
int ResetMovieFile (CFile& cf)
{
cf.Seek (nMovieStart, SEEK_SET);
return 0;       //everything is cool
}

//-----------------------------------------------------------------------

int GetNumMovieLibs (void)
{
	uint	i;

for (i = 0; i < N_MOVIE_LIBS; i++)
	if (!movies.libs [i])
		return i;
return N_MOVIE_LIBS;
}

//-----------------------------------------------------------------------

int GetNumMovies (int nLib)
{
return ((nLib < (int) N_MOVIE_LIBS) && movies.libs [nLib]) ? movies.libs [nLib]->n_movies : 0;
}

//-----------------------------------------------------------------------

char *GetMovieName (int nLib, int nMovie)
{
return (nLib < (int) N_MOVIE_LIBS) && (nMovie < movies.libs [nLib]->n_movies) ? movies.libs [nLib]->movies [nMovie].name : NULL;
}

//-----------------------------------------------------------------------

char *CycleThroughMovies (int bRestart, int bPlayMovie)
{
	static int nMovieLibs = -1;
	static int iMovieLib = -1;
	static int nMovies = -1;
	static int iMovie = -1;
	char *pszMovieName;

if (bRestart)
	iMovieLib =
	nMovies = 
	iMovie = -1;

if (nMovieLibs < 0) {
	if (!bMoviesInited)
		InitMovies ();
	nMovieLibs = GetNumMovieLibs ();
	}
if (nMovieLibs) {
	if (++iMovie >= nMovies) {
		iMovieLib = (iMovieLib + 1) % nMovieLibs;
		if (iMovieLib == 2)	//skip the robot movies
			iMovieLib = (iMovieLib + 1) % nMovieLibs;
		nMovies = GetNumMovies (iMovieLib);
		iMovie = 0;
		}
	//InitSubTitles ("intro.tex");
	if ((pszMovieName = GetMovieName (iMovieLib, iMovie))) {
		gameStates.video.nScreenMode = -1;
		if (bPlayMovie)
			PlayMovie (pszMovieName, 1, 1, gameOpts->movies.bResize);
		}
	return pszMovieName;
	//CloseSubTitles ();
	}
return NULL;
}

//-----------------------------------------------------------------------

tMovieLib *FindMovieLib (const char *pszTargetMovie)
{

	static int nMovieLibs = -1;

	int iMovieLib, iMovie, nMovies;
	char *pszMovieName;

if (nMovieLibs < 0) {
	if (!bMoviesInited)
		InitMovies ();
	nMovieLibs = GetNumMovieLibs ();
	}
if (nMovieLibs) {
	for (iMovieLib = 0; iMovieLib < nMovieLibs; iMovieLib++) {
		nMovies = GetNumMovies (iMovieLib);
		for (iMovie = 0; iMovie < nMovies; iMovie++) {
			if (!(pszMovieName = GetMovieName (iMovieLib, iMovie)))
				continue;
			if (!strcmp (pszMovieName, pszTargetMovie))
				return movies.libs [iMovieLib];
			}
		}
	}
return NULL;
}

//-----------------------------------------------------------------------

void PlayIntroMovie (void)
{
	static	int bHaveIntroMovie = 1;

if (bHaveIntroMovie) {
	InitSubTitles ("intro.tex");
	if (PlayMovie ("intro.mve", MOVIE_REQUIRED, 0, gameOpts->movies.bResize) == MOVIE_NOT_PLAYED)
		bHaveIntroMovie = 0;
	CloseSubTitles ();
	}
}

//-----------------------------------------------------------------------
