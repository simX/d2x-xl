/* $Id: args.c,v 1.10 2003/11/26 12:26:33 btb Exp $ */
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
 COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
 */

/*
 *
 * Functions for accessing arguments.
 *
 * Old Log:
 * Revision 2.0  1995/02/27  11:31:22  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.9  1994/11/29  01:07:57  john
 * Took out some unused vars.
 *
 * Revision 1.8  1994/11/29  01:04:30  john
 * Took out descent.ini stuff.
 *
 * Revision 1.7  1994/09/20  19:29:15  matt
 * Made args require exact (not substring), though still case insensitive.
 *
 * Revision 1.6  1994/07/25  12:33:11  john
 * Network "pinging" in.
 *
 * Revision 1.5  1994/06/17  18:07:50  matt
 * Took out //printf
 *
 * Revision 1.4  1994/05/11  19:45:33  john
 * *** empty log message ***
 *
 * Revision 1.3  1994/05/11  18:42:11  john
 * Added Descent.ini config file.
 *
 * Revision 1.2  1994/05/09  17:03:30  john
 * Split command pszLine parameters into arg.c and arg.h.
 * Also added /dma, /port, /irq to digi.c
 *
 * Revision 1.1  1994/05/09  16:49:11  john
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid [] = "$Id: args.c,v 1.10 2003/11/26 12:26:33 btb Exp $";
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "cfile.h"
#include "u_mem.h"
#include "strio.h"
#include "strutil.h"
#include "console.h"
#include "error.h"
#include "findfile.h"

int Num_args = 0;

#define MAX_ARGS	1000

char * Args [MAX_ARGS], *ps;

//------------------------------------------------------------------------------

int FindArg (char * s)	
{
	int i;
  
  for (i=0; i<Num_args; i++)
    if (Args [i] && *Args [i] && !stricmp (Args [i], s))
      return i;
  return 0;
}

//------------------------------------------------------------------------------

void _CDECL_ args_exit (void)
{
	int i;

LogErr ("unloading program arguments\n");
for (i = 0; i < Num_args; i++)
	if (Args [i])
		d_free (Args [i]);
memset (Args, 0, sizeof (Args));
Num_args = 0;
}

//------------------------------------------------------------------------------

void InitArgs (int argc, char **argv)
{
	int 		i, j;
	CFILE 	*f;
	char 		*pszLine, *pszToken, fnIni [FILENAME_LEN];
	static	char **pszArgs = NULL;
	static	int  nArgs = 0;

if (argv) {
	pszArgs = argv;
	nArgs = argc;
	}
else if (pszArgs) {
	argv = pszArgs;
	argc = nArgs;
	}
else
	return;
LogErr ("Loading program arguments\n");
args_exit ();
for (i = 0; i < argc; i++)
	Args [Num_args++] = d_strdup (argv [i]);

for (i = 0; i < Num_args; i++)
	if (Args [i] [0] == '-')
		strlwr (Args [i]);  // Convert all args to lowercase

// look for the ini file
// for unix, allow both ~/.d2x-xl and <config dir>/d2x.ini
if (i = FindArg ("-ini"))
	strncpy (fnIni, Args [i + 1], sizeof (fnIni) - 1);
else {
#if defined(__unix__)
FFS		ffs;
	strcpy (fnIni, gameFolders.szHomeDir);
#	ifndef RELEASE
	strcat (fnIni, "/.d2x-xl-dbg");
#	else
	strcat (fnIni, "/.d2x-xl");
#	endif
	if (FFF (fnIni, &ffs, 0) <= 0) {
#endif

	strcpy (fnIni, gameFolders.szConfigDir);
	if (*fnIni)
		strcat (fnIni, "/");
#ifdef _DEBUG
	strcat (fnIni, "d2xdebug.ini");
#else
	strcat (fnIni, "d2x.ini");
#endif

#if defined(__unix__)
   }
#endif //!__unix__
	}
  
f = CFOpen (fnIni, "", "rt", 0);
if (f) {
	while (!CFEoF (f)) {
		pszLine = fsplitword (f, '\n');
		if (*pszLine && (*pszLine != ';')) {
			pszToken = splitword (pszLine, ' ');
			if (Num_args >= MAX_ARGS)
				break;
			Args [Num_args++] = pszToken;
			if (pszLine) {
				if (Num_args >= MAX_ARGS) {
					LogErr ("too many program arguments\n");
					break;
					}
				Args [Num_args++] = *pszLine ? d_strdup (pszLine) : NULL;
				}
			}
		d_free (pszLine); 
		}
	CFClose (f);
	}
LogErr ("   ");
for (i = j = 0; i < Num_args; i++, j++) {
	if (!Args [i]) 
		continue;
	if ((Args [i][0] == '-') && (isalpha (Args [i][1]) || (j == 2))) {
		LogErr ("\n   ");
		j = 0;
		}
	LogErr (Args [i]);
	LogErr (" ");
	}
LogErr ("\n");
atexit (args_exit);
}

// ----------------------------------------------------------------------------

int NumArg (int t, int nDefault)
{
	char *psz = Args [t+1];

if (!psz)
	return nDefault;
if (psz && isdigit (*psz))
	return atoi (psz);
if (((*psz == '-') || (*psz == '+')) && isdigit (*psz + 1))
	return atoi (psz);
return nDefault;
}

//------------------------------------------------------------------------------
//eof
