/* $Id: cfile.c,v 1.23 2003/11/27 00:36:14 btb Exp $ */
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
 * Functions for accessing compressed files.
 *
 * Old Log:
 * Revision 1.7  1995/10/27  15:18:20  allender
 * get back to descent directory before trying to read a hog file
 *
 * Revision 1.6  1995/10/21  23:48:24  allender
 * hogfile(s) are now in :Data: folder
 *
 * Revision 1.5  1995/08/14  09:27:31  allender
 * added byteswap header
 *
 * Revision 1.4  1995/05/12  11:54:33  allender
 * changed memory stuff again
 *
 * Revision 1.3  1995/05/04  20:03:38  allender
 * added code that was missing...use NewPtr instead of d_malloc
 *
 * Revision 1.2  1995/04/03  09:59:49  allender
 * *** empty log message ***
 *
 * Revision 1.1  1995/03/30  10:25:02  allender
 * Initial revision
 *
 *
 * --- PC RCS Information ---
 * Revision 1.24  1995/03/15  14:20:27  john
 * Added critical error checker.
 *
 * Revision 1.23  1995/03/13  15:16:53  john
 * Added alternate directory stuff.
 *
 * Revision 1.22  1995/02/09  23:08:47  matt
 * Increased the max number of files in hogfile to 250
 *
 * Revision 1.21  1995/02/01  20:56:47  john
 * Added CFExist function
 *
 * Revision 1.20  1995/01/21  17:53:48  john
 * Added alternate pig file thing.
 *
 * Revision 1.19  1994/12/29  15:10:02  john
 * Increased hogfile max files to 200.
 *
 * Revision 1.18  1994/12/12  13:20:57  john
 * Made cfile work with fiellentth.
 *
 * Revision 1.17  1994/12/12  13:14:25  john
 * Made cfiles prefer non-hog files.
 *
 * Revision 1.16  1994/12/09  18:53:26  john
 * *** empty log message ***
 *
 * Revision 1.15  1994/12/09  18:52:56  john
 * Took out mem, error checking.
 *
 * Revision 1.14  1994/12/09  18:10:31  john
 * Speed up CFGetS, which was slowing down the reading of
 * bitmaps.tbl, which was making POF loading look slow.
 *
 * Revision 1.13  1994/12/09  17:53:51  john
 * Added error checking to number of hogfiles..
 *
 * Revision 1.12  1994/12/08  19:02:55  john
 * Added CFGetS.
 *
 * Revision 1.11  1994/12/07  21:57:48  john
 * Took out data dir.
 *
 * Revision 1.10  1994/12/07  21:38:02  john
 * Made cfile not return error..
 *
 * Revision 1.9  1994/12/07  21:35:34  john
 * Made it read from data directory.
 *
 * Revision 1.8  1994/12/07  21:33:55  john
 * Stripped out compression stuff...
 *
 * Revision 1.7  1994/04/13  23:44:59  matt
 * When file cannot be opened, d_free up the buffer for that file.
 *
 * Revision 1.6  1994/02/18  12:38:20  john
 * Optimized a bit
 *
 * Revision 1.5  1994/02/15  18:13:20  john
 * Fixed more bugs.
 *
 * Revision 1.4  1994/02/15  13:27:58  john
 * Works ok...
 *
 * Revision 1.3  1994/02/15  12:51:57  john
 * Crappy inbetween version
 *
 * Revision 1.2  1994/02/14  20:12:29  john
 * First version working with new cfile stuff.
 *
 * Revision 1.1  1994/02/14  15:51:33  john
 * Initial revision
 *
 * Revision 1.1  1994/02/10  15:45:12  john
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <string.h>
#if defined (_WIN32_WCE) || defined (_WIN32)
# include <windows.h>
#else
#include <sys/stat.h>
#endif

#include "pstypes.h"
#include "u_mem.h"
#include "strutil.h"
#include "d_io.h"
#include "error.h"
#include "cfile.h"
#include "byteswap.h"
#include "inferno.h"
#include "mission.h"
#include "console.h"
#include "findfile.h"

int nCFileError = 0;

tGameHogFiles gameHogFiles;
tGameFolders gameFolders;

// ----------------------------------------------------------------------------

int GetAppFolder (char *szRootDir, char *szFolder, char *szName, char *szFilter)
{
	FFS	ffs;
	char	szDir [FILENAME_LEN];
	int	i;

if (!(szName && *szName))
	return 1;
LogErr ("GetAppFolder ('%s', '%s', '%s', '%s')\n", szRootDir, szFolder, szName, szFilter);
sprintf (szDir, "%s%s%s%s%s", szRootDir, *szRootDir ? "/" : "", szName, *szFilter ? "/" : "", szFilter);
if (!(i = FFF (szDir, &ffs, *szFilter == '\0')))
	sprintf (szFolder, "%s%s%s", szRootDir, *szRootDir ? "/" : "", szName);
else if (*szRootDir)
	strcpy (szFolder, szRootDir);
LogErr ("GetAppFolder (%s) = '%s' (%d)\n", szName, szFolder, i);
FFC (&ffs);
return i;
}

// ----------------------------------------------------------------------------

void CFUseAltHogDir (char * path) 
{
gameFolders.bAltHogDirInited = 
	(strcmp (path, gameFolders.szDataDir) != 0) && (GetAppFolder ("", gameFolders.szAltHogDir, path, "descent2.hog") == 0);
}

// ----------------------------------------------------------------------------
//in case no one installs one
int default_error_counter=0;

//ptr to counter of how many critical errors
int *critical_error_counter_ptr=&default_error_counter;

// ----------------------------------------------------------------------------
//tell cfile about your critical error counter
void CFSetCriticalErrorCounterPtr(int *ptr)
{
	critical_error_counter_ptr = ptr;
}

// ----------------------------------------------------------------------------

FILE * cfile_get_filehandle (char *filename, char *folder, char *mode) 
{
	FILE	*fp;
	char	fn [FILENAME_LEN], *pfn;
  
  if (!*filename)
    return NULL;
  if ((*filename != '/') && (strstr (filename, "./") != filename) && *folder) {
    sprintf (fn, "%s/%s", folder, filename);
    pfn = fn;
	}
  else
    pfn = filename;
  
  *critical_error_counter_ptr = 0;
  fp = fopen (pfn, mode);
  if (fp && *critical_error_counter_ptr) {
    fclose(fp);
    fp = NULL;
	}
  
  if (!fp && gameFolders.bAltHogDirInited && strcmp (folder, gameFolders.szAltHogDir)) {
    sprintf (fn, "%s/%s", gameFolders.szAltHogDir, filename);
    pfn = fn;
    *critical_error_counter_ptr = 0;
    fp = fopen (pfn, mode);
    if (fp && *critical_error_counter_ptr) {
      fclose(fp);
      fp = NULL;
		}
	}
  //if (!fp) LogErr ("cfile_get_filehandle(): error opening %s\n", pfn);
  return fp;
}

// ----------------------------------------------------------------------------
//returns 1 if file loaded with no errors
int CFInitHogFile (char *fname, char *folder, hogfile *hog_files, int *nfiles) 
{
	char	id[4];
	FILE	*fp;
	int	i, len;
	char	fn [FILENAME_LEN];
	char  *psz;

	if (*folder) {
		sprintf (fn, "%s/%s", folder, fname);
		fname = fn;
		}

	*nfiles = 0;
	fp = cfile_get_filehandle (fname, "", "rb");
	if (fp == NULL) 
		return 0;

	if ((psz = strstr (fname, ".rdl")) || (psz = strstr (fname, ".rl2"))) {
		while ((psz >= fname) && (*psz != '\\') && (*psz != '/') && (*psz != ':'))
			psz--;
		*nfiles = 1;
		strncpy (hog_files [0].name, psz + 1, 13);
		hog_files [0].offset = 0;
		hog_files [0].length = -1;
		return 1;
		}

	fread (id, 3, 1, fp);
	if (strncmp (id, "DHF", 3)) {
		fclose(fp);
		return 0;
	}

	while (1) 
	{
		if (*nfiles >= MAX_HOGFILES) {
			fclose(fp);
			Error ("HOGFILE is limited to %d files.\n",  MAX_HOGFILES);
		}
		i = (int) fread (hog_files[*nfiles].name, 13, 1, fp);
		if (i != 1) 	{		//eof here is ok
			fclose(fp);
			return 1;
		}
		i = (int) fread (&len, 4, 1, fp);
		if (i != 1) 	{
			fclose(fp);
			return 0;
		}
		hog_files[*nfiles].length = INTEL_INT(len);
		hog_files[*nfiles].offset = ftell (fp);
		*nfiles = (*nfiles) + 1;
		// Skip over
		i = fseek (fp, INTEL_INT(len), SEEK_CUR);
	}
}

// ----------------------------------------------------------------------------

int CFUseHogFile (tHogFileList *hog, char *name, char *folder)
{
if (hog->bInitialized)
	return 1;
if (name) {
	strcpy (hog->szName, name);
	hog->bInitialized = 
		*name && 
		CFInitHogFile (hog->szName, folder, hog->files, &hog->nFiles);
	if (*(hog->szName))
		LogErr ("   found hog file '%s'\n", hog->szName);
	return hog->bInitialized && (hog->nFiles > 0);
	} 
return 0;
}

// ----------------------------------------------------------------------------

int CFUseAltHogFile (char * name) 
{
gameHogFiles.AltHogFiles.bInitialized = 0;
return CFUseHogFile (&gameHogFiles.AltHogFiles, name, "");
}

// ----------------------------------------------------------------------------

int cfile_use_D2X_hogfile (char * name) 
{
return CFUseHogFile (&gameHogFiles.D2XHogFiles, name, gameFolders.szMissionDir);
}

// ----------------------------------------------------------------------------

int cfile_use_XL_hogfile (char * name) 
{
return CFUseHogFile (&gameHogFiles.XLHogFiles, name, gameFolders.szDataDir);
}

// ----------------------------------------------------------------------------

int cfile_use_Extra_hogfile (char * name) 
{
return gameStates.app.bHaveExtraData = 
	!gameStates.app.bNostalgia &&
	CFUseHogFile (&gameHogFiles.ExtraHogFiles, name, gameFolders.szDataDir);
}

// ----------------------------------------------------------------------------

int CFUseD1HogFile (char * name) 
{
return CFUseHogFile (&gameHogFiles.D1HogFiles, name, gameFolders.szDataDir);
}

// ----------------------------------------------------------------------------
//Specify the name of the hogfile.  Returns 1 if hogfile found & had files
int CFileInit (char *hogname, char *folder)
{
if (!*hogname) {
	memset (&gameHogFiles, 0, sizeof (gameHogFiles));
	memset (&gameFolders, 0, sizeof (gameFolders));
	return 1;
	}
Assert(gameHogFiles.D2HogFiles.bInitialized == 0);
if (CFInitHogFile (hogname, folder, gameHogFiles.D2HogFiles.files, &gameHogFiles.D2HogFiles.nFiles)) {
	strcpy (gameHogFiles.D2HogFiles.szName, hogname);
	gameHogFiles.D2HogFiles.bInitialized = 1;
	cfile_use_D2X_hogfile ("d2x.hog");
	cfile_use_XL_hogfile ("d2x-xl.hog");
	cfile_use_Extra_hogfile ("extra.hog");
	CFUseD1HogFile ("descent.hog");
	return 1;
	}
return 0;	//not loaded!
}

// ----------------------------------------------------------------------------

int CFSize(char *hogname, char *folder, int bUseD1Hog)
{
	CFILE *fp;
//	char fn [FILENAME_LEN];
#if !(defined (_WIN32_WCE) || defined (_WIN32))
	struct stat statbuf;

//	sprintf (fn, "%s/%s", folder, hogname);
	fp = CFOpen(hogname, gameFolders.szDataDir, "rb", bUseD1Hog);
	if (fp == NULL)
		return -1;
	fstat (fileno(fp->file), &statbuf);
	CFClose(fp);
	return statbuf.st_size;
#else
	DWORD size;

	//sprintf (fn, "%s%s%s", folder, *folder ? "/" : "", hogname);
	fp = CFOpen(hogname, gameFolders.szDataDir, "rb", bUseD1Hog);
	if (fp == NULL)
		return -1;
	if (fseek (fp->file, 0, SEEK_END))
		return -1;
	size = ftell (fp->file);
	CFClose(fp);
	return size;
#endif
}

// ----------------------------------------------------------------------------
/*
 * return handle for file called "name", embedded in one of the hogfiles
 */

FILE *CFFindHogFile (tHogFileList *hog, char *folder, char *name, int *length)
{
	FILE		*fp;
	int		i;
	hogfile	*phf;
	char		*hogFilename = hog->szName;
  
if (!(hog->bInitialized && *hogFilename))
	return NULL;
if (*folder) {
	char fn [FILENAME_LEN];

	sprintf (fn, "%s/%s", folder, hog->szName);
	hogFilename = fn;
	}

for (i = hog->nFiles, phf = hog->files; i; i--, phf++) {
	if (stricmp (phf->name, name))
		continue;
	if (!(fp = cfile_get_filehandle (hogFilename, "", "rb")))
		break;
	fseek (fp, phf->offset, SEEK_SET);
	if (length)
		*length = phf->length;
	return fp;
	}
//LogErr ("CFFindHogFile(): '%s:%s' not found\n", hogFilename, name);
return NULL;
}

// ----------------------------------------------------------------------------

FILE* CFFindLibFile (char *name, int *length, int bUseD1Hog)
{
	FILE* fp;
  
if (fp = CFFindHogFile (&gameHogFiles.AltHogFiles, "", name, length))
	return fp;
if (fp = CFFindHogFile (&gameHogFiles.XLHogFiles, gameFolders.szDataDir, name, length))
	return fp;
if (fp = CFFindHogFile (&gameHogFiles.ExtraHogFiles, gameFolders.szDataDir, name, length))
	return fp;
if (bUseD1Hog) {
	if (fp = CFFindHogFile (&gameHogFiles.D1HogFiles, gameFolders.szDataDir, name, length))
		return fp;
	}
if (!bUseD1Hog) {
	if (fp = CFFindHogFile (&gameHogFiles.D2XHogFiles, gameFolders.szMissionDir, name, length))
		return fp;
	if (fp = CFFindHogFile (&gameHogFiles.D2HogFiles, gameFolders.szDataDir, name, length))
		return fp;
	}
//LogErr ("File '%s' not found\n", name);
return NULL;
}

// ----------------------------------------------------------------------------
// CFEoF() Tests for end-of-file on a stream
//
// returns a nonzero value after the first read operation that attempts to read
// past the end of the file. It returns 0 if the current position is not end of file.
// There is no error return.

int CFEoF(CFILE *cfile)
{
Assert(cfile != NULL);
Assert(cfile->file != NULL);
return (cfile->raw_position >= cfile->size);
}

// ----------------------------------------------------------------------------

int CFError(CFILE *cfile)
{
	return ferror(cfile->file);
}

// ----------------------------------------------------------------------------

int CFExist (char *filename, char *folder, int bUseD1Hog) 
{
	int length;
	FILE *fp;

	if (filename[0] != '\x01')
		fp = cfile_get_filehandle (filename, folder, "rb");		// Check for non-hog file first...
	else {
		fp = NULL;		//don't look in dir, only in hogfile
		filename++;
	}
	if (fp) 	{
		fclose(fp);
		return 1;
	}
	fp = CFFindLibFile (filename, &length, bUseD1Hog);
	if (fp) 	{
		fclose(fp);
		return 2;		// file found in hog
	}
	return 0;		// Couldn't find it.
}

// ----------------------------------------------------------------------------
// Deletes a file.
int CFDelete(char *filename, char*folder)
{
	char	fn [FILENAME_LEN];

sprintf (fn, "%s%s%s", folder, *folder ? "/" : "", filename);
#ifndef _WIN32_WCE
	return remove(fn);
#else
	return !DeleteFile(fn);
#endif
}

// ----------------------------------------------------------------------------
// Rename a file.
int CFRename(char *oldname, char *newname, char *folder)
{
	char	fno [FILENAME_LEN], fnn [FILENAME_LEN];

sprintf (fno, "%s%s%s", folder, *folder ? "/" : "", oldname);
sprintf (fnn, "%s%s%s", folder, *folder ? "/" : "", newname);
#ifndef _WIN32_WCE
	return rename(fno, fnn);
#else
	return !MoveFile(fno, fnn);
#endif
}

// ----------------------------------------------------------------------------
// Make a directory.
int CFMkDir(char *pathname)
{
#if defined (_WIN32_WCE) || defined (_WIN32)
	return !CreateDirectory(pathname, NULL);
#else
	return mkdir(pathname, 0755);
#endif
}

// ----------------------------------------------------------------------------

CFILE * CFOpen (char *filename, char *folder, char *mode, int bUseD1Hog) 
{
	int	length = -1;
	FILE	*fp;
	CFILE *cfile = NULL;
	char	*pszHogExt, *pszFileExt;

if (! (filename && *filename))
	return NULL;
if ((*filename != '\x01') /*&& !bUseD1Hog*/) {
	fp = cfile_get_filehandle (filename, folder, mode);		// Check for non-hog file first...
	if (!fp && 
		 ((pszFileExt = strstr (filename, ".rdl")) || (pszFileExt = strstr (filename, ".rl2"))) &&
		 (pszHogExt = strchr (gameHogFiles.szAltHogFile, '.')) &&
		 !stricmp (pszFileExt, pszHogExt))
		fp = cfile_get_filehandle (gameHogFiles.szAltHogFile, folder, mode);		// Check for non-hog file first...
	}
else {
	fp = NULL;		//don't look in dir, only in hogfile
	if (*filename == '\x01')
		filename++;
	}

if (!fp) {
	if (fp = CFFindLibFile (filename, &length, bUseD1Hog)) 
		if (stricmp (mode, "rb"))
			Error ("Cannot read hog file\n(wrong file io mode).\n");
	}
if (fp) {
	if (!(cfile = d_malloc (sizeof (CFILE))))
		fclose (fp);
	else {
		cfile->file = fp;
		cfile->raw_position = 0;
		cfile->size = (length < 0) ? ffilelength (fp) : length;
		cfile->lib_offset = (length < 0) ? 0 : ftell (fp);
		}
	}
//if (!cfile) LogErr ("CFOpen(): error opening %s\n", filename);
return cfile;
}

// ----------------------------------------------------------------------------

int CFLength (CFILE *fp, int bUseD1Hog) 
{
return fp ? fp->size : 0;
}

// ----------------------------------------------------------------------------
// CFWrite () writes to the file
//
// returns:   number of full elements actually written
//
//
int CFWrite (void *buf, int elsize, int nelem, CFILE *cfile)
{
	int items_written;

	Assert(cfile != NULL);
	Assert(buf != NULL);
	Assert(elsize > 0);

	Assert(cfile->file != NULL);
	Assert(cfile->lib_offset == 0);

	items_written = (int) fwrite(buf, elsize, nelem, cfile->file);
	cfile->raw_position = ftell(cfile->file);

	return items_written;
}

// ----------------------------------------------------------------------------
// CFPutC() writes a character to a file
//
// returns:   success ==> returns character written
//            error   ==> EOF
//
int CFPutC(int c, CFILE *cfile)
{
	int char_written;

	Assert(cfile != NULL);

	Assert(cfile->file != NULL);
	Assert(cfile->lib_offset == 0);

	char_written = fputc(c, cfile->file);
	cfile->raw_position = ftell(cfile->file);

	return char_written;
}

// ----------------------------------------------------------------------------

int CFGetC (CFILE * fp) 
{
	int c;

if (fp->raw_position >= fp->size) 
	return EOF;
c = getc (fp->file);
if (c != EOF)
	fp->raw_position = ftell (fp->file) - fp->lib_offset;
return c;
}

// ----------------------------------------------------------------------------
// CFPutS() writes a string to a file
//
// returns:   success ==> non-negative value
//            error   ==> EOF
//
int CFPutS(char *str, CFILE *cfile)
{
	int ret;

	Assert(cfile != NULL);
	Assert(str != NULL);

	Assert(cfile->file != NULL);

	ret = fputs(str, cfile->file);
	cfile->raw_position = ftell(cfile->file);

	return ret;
}

// ----------------------------------------------------------------------------

char * CFGetS (char * buf, size_t n, CFILE * fp) 
{
	char * t = buf;
	size_t i;
	int c;

#if 0 // don't use the standard fgets, because it will only handle the native line-ending style
	if (fp->lib_offset == 0) // This is not an archived file
	{
		t = fgets(buf, n, fp->file);
		fp->raw_position = ftell(fp->file);
		return t;
	}
#endif

	for (i=0; i<n-1; i++) {
		do {
			if (fp->raw_position >= fp->size) {
				*buf = 0;
				return NULL;
			}
			c = CFGetC(fp);
			if (c == 0 || c == 10)       // Unix line ending
				break;
			if (c == 13) {      // Mac or DOS line ending
				int c1;

				c1 = CFGetC(fp);
				CFSeek(fp, -1, SEEK_CUR);
				if (c1 == 10) // DOS line ending
					continue;
				else            // Mac line ending
					break;
			}
		} while (c == 13);
 		if (c == 13)  // because cr-lf is a bad thing on the mac
 			c = '\n';   // and anyway -- 0xod is CR on mac, not 0x0a
		*buf++ = c;
		if (c=='\n') break;
	}
	*buf++ = 0;
	return  t;
}

// ----------------------------------------------------------------------------

size_t CFRead (void * buf, size_t elsize, size_t nelem, CFILE * fp) 
{
	unsigned int i, size = (int) (elsize * nelem);
	if (!fp || (size < 1))
		return 0;

	i = (int) fread (buf, 1, size, fp->file);
	nCFileError = (i != size);
	fp->raw_position += i;
	return i/elsize;
}


// ----------------------------------------------------------------------------

int CFTell (CFILE *fp) 	
{
return fp ? fp->raw_position : -1;
}

// ----------------------------------------------------------------------------

int CFSeek (CFILE *fp, long int offset, int where) 
{
	int c, goal_position;

	switch (where) 	{
	case SEEK_SET:
		goal_position = offset;
		break;
	case SEEK_CUR:
		goal_position = fp->raw_position+offset;
		break;
	case SEEK_END:
		goal_position = fp->size+offset;
		break;
	default:
		return 1;
	}	
	c = fseek (fp->file, fp->lib_offset + goal_position, SEEK_SET);
	fp->raw_position = ftell(fp->file)-fp->lib_offset;
	return c;
}

// ----------------------------------------------------------------------------

int CFClose(CFILE *fp)
{
	int result;

if (!fp)
	return nCFileError;
result = fclose (fp->file);
d_free (fp);
return result;
}

// ----------------------------------------------------------------------------
// routines to read basic data types from CFILE's.  Put here to
// simplify mac/pc reading from cfiles.

int CFReadInt (CFILE *file)
{
	int32_t i;

if (CFRead (&i, sizeof (i), 1, file) != 1)
	return nCFileError;
//Error ("Error reading int in CFReadInt()");
return INTEL_INT (i);
}

// ----------------------------------------------------------------------------

short CFReadShort (CFILE *file)
{
	int16_t s;

if (CFRead (&s, sizeof (s), 1, file) != 1)
	return nCFileError;
//Error ("Error reading short in CFReadShort()");
return INTEL_SHORT (s);
}

// ----------------------------------------------------------------------------

sbyte CFReadByte (CFILE *file)
{
	sbyte b;

if (CFRead (&b, sizeof (b), 1, file) != 1)
	return nCFileError;
//Error ("Error reading byte in CFReadByte()");
return b;
}

// ----------------------------------------------------------------------------

float CFReadFloat (CFILE *file)
{
	float f;

if (CFRead (&f, sizeof (f), 1, file) != 1)
	return (float) nCFileError;
//Error ("Error reading float in CFReadFloat()");
return INTEL_FLOAT (f);
}

// ----------------------------------------------------------------------------
//Read and return a double (64 bits)
//Throws an exception of type (nCFileError *) if the OS returns an error on read
double CFReadDouble (CFILE *file)
{
	double d;

if (CFRead (&d, sizeof (d), 1, file) != 1)
	return nCFileError;
return INTEL_DOUBLE (d);
}

// ----------------------------------------------------------------------------

fix CFReadFix (CFILE *file)
{
	fix f;

if (CFRead (&f, sizeof (f), 1, file) != 1)
	return nCFileError;
//Error ("Error reading fix in CFReadFix()");
return (fix) INTEL_INT ((int) f);
return f;
}

// ----------------------------------------------------------------------------

fixang CFReadFixAng (CFILE *file)
{
	fixang f;

if (CFRead (&f, 2, 1, file) != 1)
	return nCFileError;
//Error("Error reading fixang in CFReadFixAng()");
return (fixang) INTEL_SHORT ((int) f);
}

// ----------------------------------------------------------------------------

void CFReadVector (vms_vector *v, CFILE *file)
{
v->x = CFReadFix (file);
v->y = CFReadFix (file);
v->z = CFReadFix (file);
}

// ----------------------------------------------------------------------------

void CFReadAngVec(vms_angvec *v, CFILE *file)
{
v->p = CFReadFixAng (file);
v->b = CFReadFixAng (file);
v->h = CFReadFixAng (file);
}

// ----------------------------------------------------------------------------

void CFReadMatrix(vms_matrix *m,CFILE *file)
{
CFReadVector (&m->rvec,file);
CFReadVector (&m->uvec,file);
CFReadVector (&m->fvec,file);
}


// ----------------------------------------------------------------------------

void CFReadString (char *buf, int n, CFILE *file)
{
	char c;

do {
	c = (char) CFReadByte (file);
	if (n > 0) {
		*buf++ = c;
		n--;
		}
	} while (c != 0);
}

// ----------------------------------------------------------------------------
// equivalent write functions of above read functions follow

int CFWriteInt(int i, CFILE *file)
{
i = INTEL_INT(i);
return CFWrite (&i, sizeof (i), 1, file);
}

// ----------------------------------------------------------------------------

int CFWriteShort(short s, CFILE *file)
{
s = INTEL_SHORT(s);
return CFWrite (&s, sizeof (s), 1, file);
}

// ----------------------------------------------------------------------------

int CFWriteByte (sbyte b, CFILE *file)
{
return CFWrite (&b, sizeof (b), 1, file);
}

// ----------------------------------------------------------------------------

int cfile_write_float (float f, CFILE *file)
{
f = INTEL_FLOAT (f);
return CFWrite (&f, sizeof (f), 1, file);
}

// ----------------------------------------------------------------------------
//Read and return a double (64 bits)
//Throws an exception of type (nCFileError *) if the OS returns an error on read
int cfile_write_double (double d, CFILE *file)
{
d = INTEL_DOUBLE (d);
return CFWrite (&d, sizeof (d), 1, file);
}

// ----------------------------------------------------------------------------

int CFWriteFix (fix x, CFILE *file)
{
x = INTEL_INT (x);
return CFWrite (&x, sizeof (x), 1, file);
}

// ----------------------------------------------------------------------------

int CFWriteFixAng (fixang a, CFILE *file)
{
a = INTEL_SHORT (a);
return CFWrite (&a, sizeof (a), 1, file);
}

// ----------------------------------------------------------------------------

void CFWriteVector (vms_vector *v, CFILE *file)
{
CFWriteFix (v->x, file);
CFWriteFix (v->y, file);
CFWriteFix (v->z, file);
}

// ----------------------------------------------------------------------------

void CFWriteAngVec (vms_angvec *v, CFILE *file)
{
CFWriteFixAng (v->p, file);
CFWriteFixAng (v->b, file);
CFWriteFixAng (v->h, file);
}

// ----------------------------------------------------------------------------

void CFWriteMatrix (vms_matrix *m,CFILE *file)
{
CFWriteVector (&m->rvec, file);
CFWriteVector (&m->uvec, file);
CFWriteVector (&m->fvec, file);
}


// ----------------------------------------------------------------------------

int CFWriteString (char *buf, CFILE *file)
{
if (buf && *buf && CFWrite (buf, (int) strlen (buf), 1, file))
	return (int) CFWriteByte (0, file);   // write out NULL termination
return 0;
}

// ----------------------------------------------------------------------------

int CFExtract (char *filename, char *folder, int bUseD1Hog, char *szDestName)
{
	CFILE		*fp = CFOpen (filename, folder, "rb", bUseD1Hog);
	FILE		*f;
	char		szDest [FILENAME_LEN], fn [FILENAME_LEN];
	static	char buf [4096];
	int		h, l;

if (!fp)
	return 0;
strcpy (fn, filename);
if (*szDestName) {
	if (*szDestName == '.') {
		char *psz = strchr (fn, '.');
		if (psz)
			strcpy (psz, szDestName);
		else
			strcat (fn, szDestName);
		}
	else
		strcpy (fn, szDestName);
	}
sprintf (szDest, "%s%s%s", folder, *folder ? "/" : "", fn);
f = fopen (szDest, "wb");
if (!f) {
	CFClose (fp);
	return 0;
	}
for (h = sizeof (buf), l = fp->size; l; l -= h) {
	if (h > l)
		h = l;
	CFRead (buf, h, 1, fp);
	fwrite (buf, h, 1, f);
	}
CFClose (fp);
fclose (f);
return 1;
}

// ----------------------------------------------------------------------------

char *CFReadData (char *filename, char *folder, int bUseD1Hog)
{
	CFILE		*fp = CFOpen (filename, folder, "rb", bUseD1Hog);
	char		*pData = NULL;
	size_t	nSize;

if (!fp)
	return NULL;
nSize = CFLength (fp, bUseD1Hog);
if (!(pData = (char *) d_malloc (nSize)))
	return NULL;
if (!CFRead (pData, nSize, 1, fp)) {
	d_free (pData);
	pData = NULL;
	}
CFClose (fp);
return pData;
}

// ----------------------------------------------------------------------------

