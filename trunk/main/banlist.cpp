
#include <string.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif 

#include "inferno.h"
#include "strutil.h"
#include "u_mem.h"
#include "cfile.h"
#include "banlist.h"

//-----------------------------------------------------------------------------

typedef char tBanListEntry [CALLSIGN_LEN + 1];
typedef tBanListEntry *pBanListEntry;

pBanListEntry	pBanList;
int nBanListSize = 0;

//-----------------------------------------------------------------------------

int AddPlayerToBanList (char *szPlayer)
{
if (!*szPlayer)
	return 1;
if (!pBanList) {
	pBanList = (pBanListEntry) D2_ALLOC (100 * sizeof (tBanListEntry));
	if (!pBanList)
		return 0;
	nBanListSize = 1;
	}
else if (!(++nBanListSize % 100))
	pBanList = (pBanListEntry) D2_REALLOC (pBanList, nBanListSize + 100 * sizeof (tBanListEntry));
if (!pBanList) {
	nBanListSize = 0;
	return 0;
	}
memcpy (pBanList [nBanListSize - 1], szPlayer, sizeof (tBanListEntry));
return 1;
}

//-----------------------------------------------------------------------------

int LoadBanList (void)
{
	CFile	cf;
	tBanListEntry	szPlayer;
	uint i;

if (!cf.Open ("banlist.txt", gameFolders.szDataDir, "rt", 0))
	return 0;
while (!feof (cf.File ())) {
	fgets (reinterpret_cast<char*> (szPlayer), sizeof (szPlayer), cf.File ());
	for (i = 0; i < sizeof (tBanListEntry); i++)
		if (szPlayer [i] == '\n')
			szPlayer [i] = '\0';
	szPlayer [CALLSIGN_LEN] = '\0';
	if (!AddPlayerToBanList (szPlayer))
		break;
	}
cf.Close ();
return 1;
}

//-----------------------------------------------------------------------------

int SaveBanList (void)
{
if (pBanList && nBanListSize) {
	CFile	cf;
	int i;

	if (!cf.Open ("banlist.txt", gameFolders.szDataDir, "wt", 0))
		return 0;
	for (i = 0; i < nBanListSize; i++)
		fputs (reinterpret_cast<const char*> (pBanList [i]), cf.File ());
	cf.Close ();
	}
return 1;
}
//-----------------------------------------------------------------------------

int FindPlayerInBanList (char *szPlayer)
{
	int	i;

for (i = 0; i < nBanListSize; i++)
	if (!strnicmp (reinterpret_cast<char*> (pBanList [i]), szPlayer, sizeof (tBanListEntry)))
		return 1;
return 0;
}

//-----------------------------------------------------------------------------

void FreeBanList (void)
{
if (pBanList) {
	D2_FREE (pBanList);
	pBanList = NULL;
	nBanListSize = 0;
	}
}

//-----------------------------------------------------------------------------
