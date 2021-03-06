#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "inferno.h"
#include "u_mem.h"
#include "error.h"

#if 1//ndef FAST_FILE_IO /*permanently enabled for a reason!*/

//------------------------------------------------------------------------------
// reads a tSegment2 structure from a CFile
 
void ReadSegment2 (tSegment2 *s2, CFile& cf)
{
	s2->special = cf.ReadByte ();
	s2->nMatCen = cf.ReadByte ();
	s2->value = cf.ReadByte ();
	s2->s2Flags = cf.ReadByte ();
	s2->xAvgSegLight = cf.ReadFix ();
}

//------------------------------------------------------------------------------
// reads a tLightDelta structure from a CFile

void ReadlightDelta (tLightDelta *dlP, CFile& cf)
{
dlP->nSegment = cf.ReadShort ();
dlP->nSide = cf.ReadByte ();
cf.ReadByte ();
if (!(dlP->bValid = (dlP->nSegment >= 0) && (dlP->nSegment < gameData.segs.nSegments) && (dlP->nSide >= 0) && (dlP->nSide < 6)))
	PrintLog ("Invalid delta light data %d (%d,%d)\n", dlP - gameData.render.lights.deltas, dlP->nSegment, dlP->nSide);
cf.Read (dlP->vertLight, sizeof (dlP->vertLight [0]), sizeofa (dlP->vertLight));
}


//------------------------------------------------------------------------------
// reads a tLightDeltaIndex structure from a CFile

void ReadlightDeltaIndex (tLightDeltaIndex *di, CFile& cf)
{
if (gameStates.render.bD2XLights) {
	short	i, j;
	di->d2x.nSegment = cf.ReadShort ();
	i = (short) cf.ReadByte ();
	j = (short) cf.ReadByte ();
	di->d2x.nSide = i;
	di->d2x.count = (j << 5) + ((i >> 3) & 63);
	di->d2x.index = cf.ReadShort ();
	}
else {
	di->d2.nSegment = cf.ReadShort ();
	di->d2.nSide = cf.ReadByte ();
	di->d2.count = cf.ReadByte ();
	di->d2.index = cf.ReadShort ();
	}
}
#endif

//------------------------------------------------------------------------------

int CountSkyBoxSegments (void)
{
	tSegment2	*seg2P;
	int			i, nSegments;

for (i = gameData.segs.nSegments, nSegments = 0, seg2P = SEGMENT2S; i; i--, seg2P++)
	if (seg2P->special == SEGMENT_IS_SKYBOX)
		nSegments++;
return nSegments;
}

//------------------------------------------------------------------------------

void FreeSkyBoxSegList (void)
{
D2_FREE (gameData.segs.skybox.segments);
}

//------------------------------------------------------------------------------

int BuildSkyBoxSegList (void)
{
FreeSkyBoxSegList ();
if ((gameData.segs.skybox.nSegments = CountSkyBoxSegments ())) {
	tSegment2	*seg2P;
	short			*segP;
	int			h, i;

if (!(gameData.segs.skybox.segments = (short *) D2_ALLOC (gameData.segs.nSegments * sizeof (short))))
	return 0;
segP = gameData.segs.skybox.segments;
for (h = gameData.segs.nSegments, i = 0, seg2P = SEGMENT2S; i < h; i++, seg2P++)
	if (seg2P->special == SEGMENT_IS_SKYBOX)
		*segP++ = i;
	}
gameStates.render.bHaveSkyBox = (gameData.segs.skybox.nSegments > 0);
return gameData.segs.skybox.nSegments;
}

//------------------------------------------------------------------------------
//eof
