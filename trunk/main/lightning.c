/* $Id: tObject.c, v 1.9 2003/10/04 03:14:47 btb Exp $ */
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

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inferno.h"
#include "perlin.h"
#include "game.h"
#include "gr.h"
#include "stdlib.h"
#include "bm.h"
//#include "error.h"
#include "mono.h"
#include "3d.h"
#include "segment.h"
#include "texmap.h"
#include "laser.h"
#include "key.h"
#include "gameseg.h"
#include "textures.h"

#include "object.h"
#include "objsmoke.h"
#include "objrender.h"
#include "lightning.h"
#include "render.h"
#include "error.h"
#include "pa_enabl.h"
#include "timer.h"

#include "sounds.h"
#include "collide.h"

#include "lighting.h"
#include "interp.h"
#include "player.h"
#include "weapon.h"
#include "network.h"
#include "newmenu.h"
#include "multi.h"
#include "menu.h"
#include "args.h"
#include "text.h"
#include "vecmat.h"
#include "particles.h"
#include "hudmsg.h"
#include "oof.h"
#include "sphere.h"
#include "globvars.h"
#ifdef TACTILE
#include "tactile.h"
#endif
#include "ogl_init.h"
#include "input.h"
#include "automap.h"
#include "u_mem.h"

#ifdef EDITOR
#include "editor/editor.h"
#endif

#ifdef _3DFX
#include "3dfx_des.h"
#endif

#define RENDER_TARGET_LIGHTNING 0
#define RENDER_LIGHTNING_PLASMA 1
#define RENDER_LIGHTING_SEGMENTS 0
#define RENDER_LIGHTNING_OUTLINE 0
#define UPDATE_LIGHTINGS 1

#define STYLE(_pl)	((((_pl)->nStyle < 0) || (gameOpts->render.lightnings.nStyle < (_pl)->nStyle)) ? \
							gameOpts->render.lightnings.nStyle : \
							(_pl)->nStyle)

void CreateLightningPath (tLightning *pl, int bSeed, int nDepth);

//------------------------------------------------------------------------------

inline double f_rand (void)
{
return (double) rand () / (double) RAND_MAX;
}

//	-----------------------------------------------------------------------------

void InitLightnings (void)
{
	int i, j;

for (i = 0, j = 1; j < MAX_LIGHTNINGS; i++, j++) 
	gameData.lightnings.buffer [i].nNext = j;
gameData.lightnings.buffer [i].nNext = -1;
gameData.lightnings.iFree = 0;
gameData.lightnings.iUsed = -1;
gameData.lightnings.bDestroy = 0;
memset (gameData.lightnings.objects, 0xff, MAX_OBJECTS * sizeof (*gameData.lightnings.objects));
}

//------------------------------------------------------------------------------

vmsVector *VmRandomVector (vmsVector *vRand)
{
do {
	vRand->p.x = F1_0 / 4 - d_rand ();
	vRand->p.y = F1_0 / 4 - d_rand ();
	vRand->p.z = F1_0 / 4 - d_rand ();
	} while (!(vRand->p.x && vRand->p.y && vRand->p.z));
VmVecNormalize (vRand);
return vRand;
}

//------------------------------------------------------------------------------

#define SIGN(_i)	(((_i) < 0) ? -1 : 1)

#define VECSIGN(_v)	(SIGN ((_v).p.x) * SIGN ((_v).p.y) * SIGN ((_v).p.z))

//------------------------------------------------------------------------------

vmsVector *DirectedRandomVector (vmsVector *vRand, vmsVector *vDir, int nMinDot, int nMaxDot)
{
	vmsVector	vr, vd, vSign;
	int			nDot, nSign;

VmVecCopyNormalize (&vd, vDir);
vSign.p.x = vd.p.x ? vd.p.x / abs (vd.p.x) : 0;
vSign.p.y = vd.p.y ? vd.p.y / abs (vd.p.y) : 0;
vSign.p.z = vd.p.z ? vd.p.z / abs (vd.p.z) : 0;
nSign = VECSIGN (vd);
do {
	vr.p.x = /*vSign.p.x ? vSign.p.x * rand () :*/ F1_0 / 4 - rand ();
	vr.p.y = /*vSign.p.y ? vSign.p.y * rand () :*/ F1_0 / 4 - rand ();
	vr.p.z = /*vSign.p.z ? vSign.p.z * rand () :*/ F1_0 / 4 - rand ();
	VmVecNormalize (&vr);
#if 0
	if (nSign != VECSIGN (vr)) {
		int i;
		do {
			i = rand () % 3;
			} while (!vr.v [i]);
		vr.v [i] = -vr.v [i];
		}
#endif
	nDot = VmVecDot (&vr, &vd);
	} while ((nDot > nMaxDot) || (nDot < nMinDot));
*vRand = vr;
return vRand;
}

//------------------------------------------------------------------------------

int ComputeChildEnd (vmsVector *vPos, vmsVector *vEnd, vmsVector *vDir, vmsVector *vParentDir, int nLength)
{
nLength = 3 * nLength / 4 + (int) (f_rand () * nLength / 4);
DirectedRandomVector (vDir, vParentDir, 3 * F1_0 / 4, 9 * F1_0 / 10);
VmVecScaleAdd (vEnd, vPos, vDir, nLength);
return nLength;
}

//------------------------------------------------------------------------------

void SetupLightning (tLightning *pl, int bInit)
{
	tLightning		*pp;
	tLightningNode	*pln;
	vmsVector		vPos, vDir, vRefDir, vDelta [2], v;
	int				i, l;

pl->vPos = pl->vBase;
if (pl->bRandom) {
	if (!pl->nAngle)
		VmRandomVector (&vDir);
	else {
		int nMinDot = F1_0 - pl->nAngle * F1_0 / 90;
		VmVecSub (&vRefDir, &pl->vRefEnd, &pl->vPos);
		VmVecNormalize (&vRefDir);
		do {
			VmRandomVector (&vDir);
			} while (VmVecDot (&vRefDir, &vDir) < nMinDot);
		}
	VmVecScaleAdd (&pl->vEnd, &pl->vPos, &vDir, pl->nLength);
	}
else {
	VmVecSub (&vDir, &pl->vEnd, &pl->vPos);
	VmVecNormalize (&vDir);
	}
pl->vDir = vDir;
if (pl->nOffset) {
	i = pl->nOffset / 2 + (int) (f_rand () * pl->nOffset / 2);
	VmVecScaleInc (&pl->vPos, &vDir, i);
	VmVecScaleInc (&pl->vEnd, &vDir, i);
	}
vPos = pl->vPos;
if (pl->bInPlane) {
	vDelta [0] = pl->vDelta;
	VmVecZero (vDelta + 1);
	}
else {
	do {
		vDelta->p.x = F1_0 / 4 - rand ();
		vDelta->p.y = F1_0 / 4 - rand ();
		vDelta->p.z = F1_0 / 4 - rand ();
		VmVecNormalize (vDelta);
		} while (abs (VmVecDot (&vDir, vDelta)) > 9 * F1_0 / 10);
	VmVecNormal (vDelta + 1, &vPos, &pl->vEnd, vDelta);
	VmVecAdd (&v, &vPos, vDelta + 1);
	VmVecNormal (vDelta, &vPos, &pl->vEnd, &v);
	}
VmVecScaleFrac (&vDir, pl->nLength, (pl->nNodes - 1) * F1_0);
pl->nNodes = abs (pl->nNodes);
pl->iStep = 0;
if (pp = pl->pParent) {
	i = pp->nChildren + 1;
	l = pp->nLength / i;
	pl->nLength = ComputeChildEnd (&pl->vPos, &pl->vEnd, &pl->vDir, &pp->vDir, l + 3 * l / (pl->nNode + 1)); 
	VmVecCopyScale (&vDir, &pl->vDir, pl->nLength / (pl->nNodes - 1));
	}
for (i = pl->nNodes, pln = pl->pNodes; i; i--, pln++) {
	pln->vBase =
	pln->vPos = vPos;	
	VmVecZero (&pln->vOffs);
	memcpy (pln->vDelta, vDelta, sizeof (vDelta));
	VmVecInc (&vPos, &vDir);
	if (bInit)
		pln->pChild = NULL;
	else if (pln->pChild)
		SetupLightning (pln->pChild, 0);
	}
pl->nSteps = -abs (pl->nSteps);
//(pln - 1)->vPos = pl->vEnd;
}

//------------------------------------------------------------------------------

tLightning *AllocLightning (int nLightnings, vmsVector *vPos, vmsVector *vEnd, vmsVector *vDelta,
									 short nObject, int nLife, int nDelay, int nLength, int nAmplitude, 
									 char nAngle, int nOffset, short nNodes, short nChildren, char nDepth, short nSteps, 
									 short nSmoothe, char bClamp, char bPlasma, char nStyle, tRgbaColorf *colorP, 
									 tLightning *pParent, short nNode)
{
	tLightning		*pfRoot, *pl;
	tLightningNode	*pln;
	int				h, i, bRandom = (vEnd == NULL) || (nAngle > 0);

if (!(nLife && nLength && (nNodes > 4)))
	return NULL;
#if 0
if (!bRandom)
	nLightnings = 1;
#endif
if (!(pfRoot = (tLightning *) D2_ALLOC (nLightnings * sizeof (tLightning))))
	return NULL;
for (i = nLightnings, pl = pfRoot; i; i--, pl++) {
	if (!(pl->pNodes = (tLightningNode *) D2_ALLOC (nNodes * sizeof (tLightningNode)))) {
		while (++i < nLightnings)
			D2_FREE (pl->pNodes);
		D2_FREE (pfRoot);
		return NULL;
		}
	pl->nIndex = gameData.lightnings.iFree;
	if (nObject < 0) {
		pl->nObject = -1;
		pl->nSegment = -nObject - 1;
		}
	else {
		pl->nObject = nObject;
		pl->nSegment = OBJECTS [nObject].nSegment;
		}
	pl->pParent = pParent;
	pl->nNode = nNode;
	pl->nNodes = nNodes;
	pl->nChildren = gameOpts->render.lightnings.nQuality ? (nChildren < 0) ? nNodes / 10 : nChildren : 0;
	pl->nLife = nLife;
	h = abs (nLife);
	pl->nTTL = (bRandom) ? 3 * h / 4 + (int) (f_rand () * h / 2) : h;
	pl->nDelay = abs (nDelay);
	pl->nLength = (bRandom) ? 3 * nLength / 4 + (int) (f_rand () * nLength / 2) : nLength;
	pl->nAmplitude = (nAmplitude < 0) ? nLength / 6 : nAmplitude;
	pl->nAngle = vEnd ? nAngle : 0;
	pl->nOffset = nOffset;
	pl->nSteps = -nSteps;
	pl->nSmoothe = nSmoothe;
	pl->bClamp = bClamp;
	pl->bPlasma = bPlasma;
	pl->iStep = 0;
	pl->color = *colorP;
	pl->vBase = *vPos;
	pl->nStyle = nStyle;
	pl->nNext = -1;
	if (vEnd)
		pl->vRefEnd = *vEnd;
	if (!(pl->bRandom = bRandom))
		pl->vEnd = *vEnd;
	if (pl->bInPlane = (vDelta != NULL))
		pl->vDelta = *vDelta;
	SetupLightning (pl, 1);
	if (gameOpts->render.lightnings.nQuality && nDepth && nChildren) {
		tLightningNode *plh;
		vmsVector		vEnd;
		int				l, n, nNode;
		double			nStep, j;

		n = nChildren + 1 + (nChildren < 2);
		nStep = (double) pl->nNodes / (double) nChildren;
		l = nLength / n;
		plh = pl->pNodes;
		for (h = pl->nNodes - (int) nStep, j = nStep / 2; j < h; j += nStep) {
			nNode = (int) j + 2 - rand () % 5;
			if (nNode < 1)
				nNode = (int) j;
			pln = plh + nNode;
			pln->pChild = AllocLightning (1, &pln->vPos, &vEnd, vDelta, -1, pl->nLife, 0, l, nAmplitude / n * 2, nAngle, 0,
													2 * pl->nNodes / n, nChildren / 5, nDepth - 1, nSteps / 2, nSmoothe, bClamp, bPlasma, nStyle, colorP, pl, nNode);
			}
		}
	}
return pfRoot;
}

//------------------------------------------------------------------------------

int CreateLightning (int nLightnings, vmsVector *vPos, vmsVector *vEnd, vmsVector *vDelta,
							short nObject, int nLife, int nDelay, int nLength, int nAmplitude, 
							char nAngle, int nOffset, short nNodes, short nChildren, char nDepth, short nSteps, 
							short nSmoothe, char bClamp, char bPlasma, char bSound, char nStyle, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNINGS) {
		tLightningBundle	*plb;
		tLightning			*pl;
		int					n;

	if (!nLightnings)
		return -1;
	if (gameData.lightnings.iFree < 0)
		return -1;
	srand (gameStates.app.nSDLTicks);
	if (!(pl = AllocLightning (nLightnings, vPos, vEnd, vDelta, nObject, nLife, nDelay, nLength, nAmplitude, 
										nAngle, nOffset, nNodes, nChildren, nDepth, nSteps, nSmoothe, bClamp, bPlasma, nStyle, colorP, NULL, -1)))
		return -1;
	n = gameData.lightnings.iFree;
	plb = gameData.lightnings.buffer + n;
	gameData.lightnings.iFree = plb->nNext;
	plb->nNext = gameData.lightnings.iUsed;
	gameData.lightnings.iUsed = n;
	plb->pl = pl;
	plb->nLightnings = nLightnings;
	plb->nObject = nObject;
	if (plb->bSound = bSound) {
		DigiSetObjectSound (nObject, -1, "lightng.wav");
		if (vEnd || (nAngle <= 0)) {
			if (0 <= (plb->nSound = DigiGetSoundByName ("ff_amb_1")))
				DigiSetObjectSound (nObject, plb->nSound, NULL);
			}
		}
	else
		plb->nSound = -1;
	plb->tUpdate = -1;
	plb->nKey [0] =
	plb->nKey [1] = 0;
	plb->bDestroy = 0;
	}
return gameData.lightnings.iUsed;
}

//------------------------------------------------------------------------------

int IsUsedLightning (int iLightning)
{
	int	i;

if (iLightning < 0)
	return 0;
for (i = gameData.lightnings.iUsed; i >= 0; i = gameData.lightnings.buffer [i].nNext)
	if (iLightning == i)
		return 1;
return 0;
}

//------------------------------------------------------------------------------

tLightningBundle *PrevLightning (int iLightning)
{
	int	i, j;

if (iLightning < 0)
	return NULL;
for (i = gameData.lightnings.iUsed; i >= 0; i = j)
	if ((j = gameData.lightnings.buffer [i].nNext) == iLightning)
		return gameData.lightnings.buffer + i;
return NULL;
}

//------------------------------------------------------------------------------

void DestroyLightningNodes (tLightning *pl)
{
if (pl && pl->pNodes) {
		tLightningNode	*pln;
		int				i;

	for (i = abs (pl->nNodes), pln = pl->pNodes; i; i--, pln++) {
		if (pln->pChild) {
			DestroyLightningNodes (pln->pChild);
			D2_FREE (pln->pChild);
			}
		}
	D2_FREE (pl->pNodes);
	pl->nNodes = 0;
	}
}

//------------------------------------------------------------------------------

void DestroyLightnings (int iLightning, tLightning *pl, int bDestroy)
{
	tLightningBundle	*plh, *plb = NULL;
	int					i;

if (pl) 
	i = 1;
else {
	if (!IsUsedLightning (iLightning))
		return;
	plb = gameData.lightnings.buffer + iLightning;
	if (!bDestroy) {
		plb->bDestroy = 1;
		return;
		}
	pl = plb->pl;
	i = plb->nNext;
	if (gameData.lightnings.iUsed == iLightning)
		gameData.lightnings.iUsed = i;
	plb->nNext = gameData.lightnings.iFree;
	if ((plh = PrevLightning (iLightning)))
		plh->nNext = i;
	gameData.lightnings.iFree = iLightning;
	gameData.lightnings.objects [iLightning] = -1;
	if (plb->bSound && (plb->nObject >= 0))
		DigiKillSoundLinkedToObject (plb->nObject);
	i = plb->nLightnings;
	}
for (; i; i--, pl++)
	DestroyLightningNodes (pl);
if (plb) {
	plb->nLightnings = 0;
	D2_FREE (plb->pl);
	}
}

//------------------------------------------------------------------------------

int DestroyAllLightnings (int bForce)
{
	int	i, j;

if (!bForce && (gameData.lightnings.bDestroy >= 0))
	gameData.lightnings.bDestroy = 1;
else {
	for (i = gameData.lightnings.iUsed; i >= 0; i = j) {
		j = gameData.lightnings.buffer [i].nNext;
		DestroyLightnings (i, NULL, 1);
		}
	InitLightnings ();
	}
return 1;
}

//------------------------------------------------------------------------------

int ClampLightningPathPoint (vmsVector *vPos, vmsVector *vBase, int nAmplitude)
{
	vmsVector	vRoot;
	int			nDist = VmPointLineIntersection (&vRoot, vBase, vBase + 1, vPos, NULL, 0);

if (nDist < nAmplitude)
	return nDist;
VmVecDec (vPos, &vRoot);	//create vector from intersection to current path point
VmVecScaleFrac (vPos, nAmplitude, nDist);	//scale down to length nAmplitude
VmVecInc (vPos, &vRoot);	//recalculate path point
return nAmplitude;
}

//------------------------------------------------------------------------------

int ComputeAttractor (vmsVector *vAttract, vmsVector *vDest, vmsVector *vPos, int nMinDist, int i)
{
	int nDist;

VmVecSub (vAttract, vDest, vPos);
nDist = VmVecMag (vAttract) / i;
if (!nMinDist)
	VmVecScale (vAttract, F1_0 / i * 2);	// scale attractor with inverse of remaining distance
else {
	if (nDist < nMinDist)
		nDist = nMinDist;
	VmVecScale (vAttract, F1_0 / i / 2);	// scale attractor with inverse of remaining distance
	}
return nDist;
}

//------------------------------------------------------------------------------

vmsVector *CreateLightningPathPoint (vmsVector *vOffs, vmsVector *vAttract, int nDist)
{
	vmsVector	va;
	int			nDot;

if (nDist < F1_0 / 16)
	return VmRandomVector (vOffs);
VmVecCopyNormalize (&va, vAttract);
do {
	VmRandomVector (vOffs);
	nDot = VmVecDot (&va, vOffs);
	} while (abs (nDot) < F1_0 / 32);
if (nDot < 0)
	VmVecNegate (vOffs);
return vOffs;
}

//------------------------------------------------------------------------------

vmsVector *SmootheLightningOffset (vmsVector *vOffs, vmsVector *vPrevOffs, int nDist, int nSmoothe)
{
if (nSmoothe) {
		int nMag = VmVecMag (vOffs);

	if (nSmoothe > 0)
		VmVecScaleFrac (vOffs, nSmoothe * nDist, nMag);	//scale offset vector with distance to attractor (the closer, the smaller)
	else 
		VmVecScaleFrac (vOffs, nDist, nSmoothe * nMag);	//scale offset vector with distance to attractor (the closer, the smaller)
	nMag = VmVecMag (vPrevOffs);
	VmVecInc (vOffs, vPrevOffs);
	nMag = VmVecMag (vOffs);
	VmVecScaleFrac (vOffs, nDist, nMag);
	nMag = VmVecMag (vOffs);
	}
return vOffs;
}

//------------------------------------------------------------------------------

vmsVector *AttractLightningPathPoint (vmsVector *vOffs, vmsVector *vAttract, vmsVector *vPos, int nDist, int i, int bJoinPaths)
{
	int nMag = VmVecMag (vOffs);
// attract offset vector by scaling it with distance from attracting node
VmVecScaleFrac (vOffs, i * nDist / 2, nMag);	//scale offset vector with distance to attractor (the closer, the smaller)
VmVecInc (vOffs, vAttract);	//add offset and attractor vectors (attractor is the bigger the closer)
nMag = VmVecMag (vOffs);
VmVecScaleFrac (vOffs, bJoinPaths ? nDist / 2 : nDist, nMag);	//rescale to desired path length
VmVecInc (vPos, vOffs);
return vPos;
}

//------------------------------------------------------------------------------

vmsVector ComputeJaggyNode (tLightningNode *pln, vmsVector *vPos, vmsVector *vDest, vmsVector *vBase, vmsVector *vPrevOffs,
										  int nSteps, int nAmplitude, int nMinDist, int i, int nSmoothe, int bClamp)
{
	vmsVector	vAttract, vOffs;
	int			nDist = ComputeAttractor (&vAttract, vDest, vPos, nMinDist, i);

CreateLightningPathPoint (&vOffs, &vAttract, nDist);
if (vPrevOffs)
	SmootheLightningOffset (&vOffs, vPrevOffs, nDist, nSmoothe);
else if (pln->vOffs.p.x || pln->vOffs.p.z || pln->vOffs.p.z) {
	VmVecScaleInc (&vOffs, &pln->vOffs, 2 * F1_0);
	VmVecScaleFrac (&vOffs, 1, 3);
	}
if (nDist > F1_0 / 16)
	AttractLightningPathPoint (&vOffs, &vAttract, vPos, nDist, i, 0);
if (bClamp)
	ClampLightningPathPoint (vPos, vBase, nAmplitude);
pln->vNewPos = *vPos;
VmVecSub (&pln->vOffs, &pln->vNewPos, &pln->vPos);
VmVecScale (&pln->vOffs, F1_0 / nSteps);
return vOffs;
}

//------------------------------------------------------------------------------

vmsVector ComputeErraticNode (tLightningNode *pln, vmsVector *vPos, vmsVector *vBase, int nSteps, int nAmplitude, 
										int bInPlane, int bFromEnd, int bRandom, int i, int nNodes, int nSmoothe, int bClamp)
{
	int	h, j, nDelta;

pln->vNewPos = pln->vBase;
for (j = 0; j < 2 - bInPlane; j++) {
	nDelta = nAmplitude / 2 - (int) (f_rand () * nAmplitude);
	if (!bRandom) {
		i -= bFromEnd;
		nDelta *= 3;
#if 0
		nDelta /= (nNodes - i);
		nDelta *= 4;
#endif
		}
	if (bFromEnd) {
		for (h = 1; h <= 2; h++)
			if (i - h > 0)
				nDelta += (h + 1) * (pln + h)->nDelta [j];
		}
	else {
		for (h = 1; h <= 2; h++)
			if (i - h > 0)
				nDelta += (h + 1) * (pln - h)->nDelta [j];
		}
	nDelta /= 6;
	pln->nDelta [j] = nDelta;
	VmVecScaleInc (&pln->vNewPos, pln->vDelta + j, nDelta);
	}
VmVecSub (&pln->vOffs, &pln->vNewPos, &pln->vPos);
VmVecScale (&pln->vOffs, F1_0 / nSteps);
if (bClamp)
	ClampLightningPathPoint (vPos, vBase, nAmplitude);
return pln->vOffs;
}

//------------------------------------------------------------------------------

vmsVector ComputePerlinNode (tLightningNode *pln, int nSteps, int nAmplitude, int *nSeed, double phi, double i)
{
double dx = PerlinNoise1D (i, 0.25, 6, nSeed [0]);
double dy = PerlinNoise1D (i, 0.25, 6, nSeed [1]);
phi = sin (phi * Pi);
phi = sqrt (phi);
dx *= nAmplitude * phi;
dy *= nAmplitude * phi;
VmVecScaleAdd (&pln->vNewPos, &pln->vBase, pln->vDelta, (int) dx);
VmVecScaleInc (&pln->vNewPos, pln->vDelta + 1, (int) dy);
VmVecSub (&pln->vOffs, &pln->vNewPos, &pln->vPos);
VmVecScale (&pln->vOffs, F1_0 / nSteps);
return pln->vOffs;
}

//------------------------------------------------------------------------------

void SmootheLightningPath (tLightning *pl)
{
	tLightningNode	*plh, *pfi, *pfj;
	int			i, j;

for (i = pl->nNodes - 1, j = 0, pfi = pl->pNodes, plh = NULL; j < i; j++) {
	pfj = plh;
	plh = pfi++;
	if (j) {
		plh->vNewPos.p.x = pfj->vNewPos.p.x / 4 + plh->vNewPos.p.x / 2 + pfi->vNewPos.p.x / 4;
		plh->vNewPos.p.y = pfj->vNewPos.p.y / 4 + plh->vNewPos.p.y / 2 + pfi->vNewPos.p.y / 4;
		plh->vNewPos.p.z = pfj->vNewPos.p.z / 4 + plh->vNewPos.p.z / 2 + pfi->vNewPos.p.z / 4;
		}
	}
}

//------------------------------------------------------------------------------

void ComputeNodeOffsets (tLightning *pl)
{
	tLightningNode	*pln;
	int			i, nSteps = pl->nSteps;

for (i = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; i; i--, pln++) {
	VmVecSub (&pln->vOffs, &pln->vNewPos, &pln->vPos);
	VmVecScale (&pln->vOffs, F1_0 / nSteps);
	}
}

//------------------------------------------------------------------------------
// Make sure max. amplitude is reached every once in a while

void BumpLightningPath (tLightning *pl)
{
	tLightningNode	*pln;
	int			h, i, nSteps, nDist, nAmplitude, nMaxDist = 0;
	vmsVector	vBase [2];

nSteps = pl->nSteps;
nAmplitude = pl->nAmplitude;
vBase [0] = pl->vPos;
vBase [1] = pl->pNodes [pl->nNodes - 1].vPos;
for (i = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; i; i--, pln++) {
	nDist = VmVecDist (&pln->vNewPos, &pln->vPos);
	if (nMaxDist < nDist) {
		nMaxDist = nDist;
		h = i;
		}
	}
if (h = nAmplitude - nMaxDist) {
	nMaxDist += (rand () % 5) * h / 4;
	for (i = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; i; i--, pln++)
		VmVecScaleFrac (&pln->vOffs, nAmplitude, nMaxDist);
	}
}

//------------------------------------------------------------------------------

void CreateLightningPath (tLightning *pl, int bSeed, int nDepth)
{
	tLightningNode	*plh, *pln [2];
	int			h, i, j, nSteps, nStyle, nSmoothe, bClamp, bInPlane, nMinDist, nAmplitude, bPrevOffs [2] = {0,0};
	vmsVector	vPos [2], vBase [2], vPrevOffs [2];
	double		phi;

	static int	nSeed [2];

vBase [0] = vPos [0] = pl->vPos;
vBase [1] = vPos [1] = pl->vEnd;
if (bSeed) {
	nSeed [0] = rand ();
	nSeed [1] = rand ();
	nSeed [0] *= rand ();
	nSeed [1] *= rand ();
	}
#ifdef _DEBUG
else
	bSeed = 0;
#endif
nStyle = STYLE (pl);
nSteps = pl->nSteps;
nSmoothe = pl->nSmoothe;
bClamp = pl->bClamp;
bInPlane = pl->bInPlane && ((nDepth == 1) || (nStyle == 2));
nAmplitude = pl->nAmplitude;
plh = pl->pNodes;
plh->vNewPos = plh->vPos;
VmVecZero (&plh->vOffs);
if ((nDepth > 1) || pl->bRandom) {
	if (nStyle == 2) {
		if (nDepth > 1)
			nAmplitude *= 4;
		for (h = pl->nNodes, i = 0, plh = pl->pNodes; i < h; i++, plh++) {
			phi = bClamp ?(double) i / (double) (h - 1) : 1;
			ComputePerlinNode (plh, nSteps, nAmplitude, nSeed, 2 * phi / 3, phi * 7.5);
			}
		}
	else {
		if (pl->bInPlane)
			nStyle = 0;
		nMinDist = pl->nLength / (pl->nNodes - 1);
		if (!nStyle) {
			nAmplitude *= (nDepth == 1) ? 4 : 16;
			for (h = pl->nNodes - 1, i = 0, plh = pl->pNodes + 1; i < h; i++, plh++, bPrevOffs [0] = 1) 
				ComputeErraticNode (plh, vPos, vBase, nSteps, nAmplitude, 0, bInPlane, 1, i, h + 1, nSmoothe, bClamp);
			}
		else {
			for (i = pl->nNodes - 1, plh = pl->pNodes + 1; i; i--, plh++, bPrevOffs [0] = 1) 
				*vPrevOffs = ComputeJaggyNode (plh, vPos, vPos + 1, vBase, bPrevOffs [0] ? vPrevOffs : NULL, nSteps, nAmplitude, nMinDist, i, nSmoothe, bClamp);
			}
		}
	}
else {
	plh = pl->pNodes + pl->nNodes - 1;
	plh->vNewPos = plh->vPos;
	VmVecZero (&plh->vOffs);
	if (nStyle == 2) {
		nAmplitude *= 4;
		for (h = pl->nNodes, i = 0, plh = pl->pNodes; i < h; i++, plh++) {
			phi = bClamp ? (double) i / (double) (h - 1) : 1;
			ComputePerlinNode (plh, nSteps, nAmplitude, nSeed, phi, phi * 10);
			}
		}
	else {
		if (pl->bInPlane)
			nStyle = 0;
		if (!nStyle) {
			nAmplitude *= 4;
			for (h = pl->nNodes - 1, i = j = 0, pln [0] = pl->pNodes + 1, pln [1] = pl->pNodes + h - 1; i < h; i++, j = !j) {
				plh = pln [j];
				ComputeErraticNode (plh, vPos + j, vBase, nSteps, nAmplitude, bInPlane, j, 0, i, h, nSmoothe, bClamp);
				if (pln [1] <= pln [0])
					break;
				if (j)
					pln [1]--;
				else
					pln [0]++;
				}
			}
		else {
			for (i = pl->nNodes - 1, j = 0, pln [0] = pl->pNodes + 1, pln [1] = pl->pNodes + i - 1; i; i--, j = !j) {
				plh = pln [j];
				vPrevOffs [j] = ComputeJaggyNode (plh, vPos + j, vPos + !j, vBase, bPrevOffs [j] ? vPrevOffs + j : NULL, nSteps, nAmplitude, 0, i, nSmoothe, bClamp);
				bPrevOffs [j] = 1;
				if (pln [1] <= pln [0])
					break;
				if (j)
					pln [1]--;
				else
					pln [0]++;
				}
			}
		}
	}
if (nStyle < 2) {
	SmootheLightningPath (pl);
	ComputeNodeOffsets (pl);
	BumpLightningPath (pl);
	}
}

//------------------------------------------------------------------------------

int UpdateLightning (tLightning *pl, int nLightnings, int nDepth)
{
	tLightningNode	*pln;
	int				h, i, j, bSeed, bInit;
	tLightning		*plRoot = pl;

if (!(pl && nLightnings))
	return 0;
#if 0
if (!gameStates.app.tick40fps.bTick)
	return -1;
#endif
nDepth++;
for (i = 0; i < nLightnings; i++, pl++) {
#if UPDATE_LIGHTINGS
	pl->nTTL -= gameStates.app.tick40fps.nTime;
#endif
	if (pl->nNodes > 0) {
		if (bInit = (pl->nSteps < 0))
			pl->nSteps = -pl->nSteps;
		if (!pl->iStep) {
#if 1
			bSeed = 1;
			do {
				CreateLightningPath (pl, bSeed, nDepth);
#if 1
				break;
#else
				if (!bSeed)
					break;
				for (j = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; j; j--, pln++)
					if (VmVecDot (&pln [0].vOffs, &pln [1].vOffs) < 65500)
						break;
				bSeed = !bSeed;
#endif
				} while (!j);
#else
			for (j = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; j; j--, pln++)
				VmVecNegate (&pln->vOffs);
#endif
			pl->iStep = pl->nSteps;
			}
		for (j = pl->nNodes - 1 - !pl->bRandom, pln = pl->pNodes + 1; j; j--, pln++) {
			if (bInit)
				pln->vPos = pln->vNewPos;
#if UPDATE_LIGHTINGS
			else
				VmVecInc (&pln->vPos, &pln->vOffs);
#endif
			if (pln->pChild) {
				MoveLightnings (1, pln->pChild, &pln->vPos, pl->nSegment, 0, 0);
				UpdateLightning (pln->pChild, 1, nDepth + 1);
				}
			}
#if UPDATE_LIGHTINGS
		(pl->iStep)--;
#endif
		}
#if 1
	if (pl->nTTL <= 0) {
		if (pl->nLife < 0) {
			if (pl->bRandom) {
				if (0 > (pl->nNodes = -pl->nNodes)) {
					h = pl->nDelay / 2;
					pl->nTTL = h + (int) (f_rand () * h);
					}
				else {
					h = -pl->nLife;
					pl->nTTL = 3 * h / 4 + (int) (f_rand () * h / 2);
					SetupLightning (pl, 0);
					}
				}
			else {
				pl->nTTL = -pl->nLife;
				pl->nNodes = abs (pl->nNodes);
				SetupLightning (pl, 0);
				}
			}
		else {
			DestroyLightningNodes (pl);
			if (!--nLightnings)
				return 0;
			if (i < nLightnings) {
				*pl = plRoot [nLightnings - 1];
				pl--;
				i--;
				}
			}
		}
#endif
	}
return nLightnings;
}

//------------------------------------------------------------------------------

#define LIMIT_FLASH_FPS	1
#define FLASH_SLOWMO 1

void UpdateLightnings (void)
{
if (SHOW_LIGHTNINGS) {
		tLightningBundle	*plb;
		int					i, n;

#if LIMIT_LIGHTNING_FPS
#	if LIGHTNING_SLOWMO
		static int	t0 = 0;
		int t = gameStates.app.nSDLTicks - t0;

	if (t / gameStates.gameplay.slowmo [0].fSpeed < 25)
		return;
	t0 = gameStates.app.nSDLTicks + 25 - (int) (gameStates.gameplay.slowmo [0].fSpeed * 25);
#	else
	if (!gameStates.app.tick40fps.bTick)
		return 0;
#	endif
#endif
	for (i = gameData.lightnings.iUsed; i >= 0; i = n) {
		plb = gameData.lightnings.buffer + i;
		n = plb->nNext;
		if (gameStates.app.nSDLTicks - plb->tUpdate < 25)
			continue;
		plb->tUpdate = gameStates.app.nSDLTicks; 
		if (plb->bDestroy)
			DestroyLightnings (i, NULL, 1);
		else {
			if (!(plb->nLightnings = UpdateLightning (plb->pl, plb->nLightnings, 0)))
				DestroyLightnings (i, NULL, 1);
			else if (!(plb->nKey [0] || plb->nKey [1]) && (plb->nObject >= 0))
				MoveObjectLightnings (OBJECTS + plb->nObject);
			}
		}
	}
}

//------------------------------------------------------------------------------

void MoveLightnings (int i, tLightning *pl, vmsVector *vNewPos, short nSegment, int bStretch, int bFromEnd)
{
if (SHOW_LIGHTNINGS) {
		tLightningNode	*pln;
		vmsVector		vDelta, vOffs;
		int				h, j; 
		double			fStep;

	if (pl) 
		i = 1;
	else if (IsUsedLightning (i)) {
		tLightningBundle	*plb = gameData.lightnings.buffer + i;
		pl = plb->pl;
		i = plb->nLightnings;
		}
	else
		return;
	for (; i; i--, pl++) {
		pl->nSegment = nSegment;
		if (bFromEnd)
			VmVecSub (&vDelta, vNewPos, &pl->vEnd);
		else
			VmVecSub (&vDelta, vNewPos, &pl->vPos);
		if (vDelta.p.x || vDelta.p.y || vDelta.p.z) {
			if (bStretch) {
				vOffs = vDelta;
				VmVecInc (&pl->vPos, &vDelta);
				}
			else if (bFromEnd) {
				VmVecInc (&pl->vPos, &vDelta);
				pl->vEnd = *vNewPos;
				}
			else {
				VmVecInc (&pl->vEnd, &vDelta);
				pl->vPos = *vNewPos;
				}
			if (bStretch) {
				VmVecSub (&pl->vDir, &pl->vEnd, &pl->vPos);
				pl->nLength = VmVecMag (&pl->vDir);
				}
			if (0 < (h = pl->nNodes)) {
				for (j = h, pln = pl->pNodes; j; j--, pln++) {
					if (bStretch) {
						vDelta = vOffs;
						if (bFromEnd)
							fStep = (double) (h - j + 1) / (double) h;
						else
							fStep = (double) j / (double) h;
						vDelta.p.x = (int) (vOffs.p.x * fStep + 0.5);
						vDelta.p.y = (int) (vOffs.p.y * fStep + 0.5);
						vDelta.p.z = (int) (vOffs.p.z * fStep + 0.5);
						}
					VmVecInc (&pln->vNewPos, &vDelta);
					VmVecInc (&pln->vBase, &vDelta);
					VmVecInc (&pln->vPos, &vDelta);
					if (pln->pChild)
						MoveLightnings (-1, pln->pChild, &pln->vPos, nSegment, 0, bFromEnd);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

void MoveObjectLightnings (tObject *objP)
{
#if 0
if (!SPECTATOR (objP) &&
	 ((objP->vLastPos.p.x != objP->position.vPos.p.x) ||
	  (objP->vLastPos.p.y != objP->position.vPos.p.y) ||
	 (objP->vLastPos.p.z != objP->position.vPos.p.z)))
#endif
	MoveLightnings (gameData.lightnings.objects [OBJ_IDX (objP)], NULL, &OBJPOS (objP)->vPos, objP->nSegment, 0, 0);
}

//------------------------------------------------------------------------------

void DestroyObjectLightnings (tObject *objP)
{
	int i = OBJ_IDX (objP);

if (gameData.lightnings.objects [i] >= 0) {
	DestroyLightnings (gameData.lightnings.objects [i], NULL, 0);
	gameData.lightnings.objects [i] = -1;
	}
}

//------------------------------------------------------------------------------

void DestroyAllObjectLightnings (int nType, int nId)
{
	tObject	*objP;
	int		i;

for (i = 0, objP = OBJECTS; i <= gameData.objs.nLastObject; i++, objP++)
	if ((objP->nType == nType) && ((nId < 0) || (objP->id == nId)))
		DestroyObjectLightnings (objP);
}

//------------------------------------------------------------------------------

void DestroyPlayerLightnings (void)
{
DestroyAllObjectLightnings (OBJ_PLAYER, -1);
}

//------------------------------------------------------------------------------

void DestroyRobotLightnings (void)
{
DestroyAllObjectLightnings (OBJ_ROBOT, -1);
}

//------------------------------------------------------------------------------

void DestroyStaticLightnings (void)
{
DestroyAllObjectLightnings (OBJ_EFFECT, LIGHTNING_ID);
}

//------------------------------------------------------------------------------

#define LIGHTNING_VERT_ARRAYS 1

static tUVLf	uvlPlasma [3][4] = {
	{{{0,0.45f,1}},{{1,0.45f,1}},{{1,0.55f,1}},{{0,0.55f,1}}},
	{{{0,0,1}},{{1,0,1}},{{1,0.5f,1}},{{0,0.5f,1}}},
	{{{0,0.5f,1}},{{1,0.5f,1}},{{1,1,1}},{{0,1,1}}}
	};

void RenderLightningSegment (fVector *vLine, fVector *vPlasma, tRgbaColorf *colorP, int bPlasma, int bStart, int bEnd, short nDepth)
{
	fVector		vDelta;
	int			i, j, h = bStart + 2 * bEnd, bDrawArrays;
	tRgbaColorf	color = *colorP;

if (!bPlasma)
	color.alpha *= 1.5f;
if (nDepth)
	color.alpha /= 2;
if (bPlasma) {
#if RENDER_LIGHTNING_OUTLINE //render lightning segment outline
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	if (bStart)
		glColor3f (1,0,0);
	else
		glColor3f (0,0,1);
	glLineWidth (2);
	glBegin (GL_LINE_LOOP);
	for (i = 0; i < 4; i++) {
		if (i < 2)
			glColor3d (1,1,1);
		else if (i < 3)
			glColor3d (1,0.8,0);
		else
			glColor3d (1,0.5,0);
		glVertex3fv ((GLfloat *) (vPlasma + i));
		}
	glEnd ();
	glLineWidth (1);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
#endif
	bDrawArrays = OglEnableClientStates (1, 0);
	glEnable (GL_TEXTURE_2D);
	if (LoadCorona () && !OglBindBmTex (bmpCorona, 1, -1)) {
		OglTexWrap (bmpCorona->glTexture, GL_CLAMP);
		for (i = 0; i < 2; i++) {
			if (!i) {
				//OglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f (color.red / 2, color.green / 2, color.blue / 2, color.alpha);
				}
			else {
				//OglBlendFunc (GL_SRC_ALPHA, GL_ONE);
				glColor4f (1, 1, 1, color.alpha / 2);
				}
			if (bDrawArrays) {
				glTexCoordPointer (2, GL_FLOAT, sizeof (tUVLf), uvlPlasma + h);
				glVertexPointer (3, GL_FLOAT, sizeof (fVector), vPlasma);
				glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
				}
			else {
				glBegin (GL_QUADS);
				for (j = 0; j < 4; j++) {
					glTexCoord2fv ((GLfloat *) (uvlPlasma [h] + j));
					glVertex3fv ((GLfloat *) (vPlasma + j));
					}
				glEnd ();
				}
			if (!i) {	//resize plasma quad for inner, white plasma path
				VmVecScalef (&vDelta, VmVecSubf (&vDelta, vPlasma, vPlasma + 1), 0.25f);
				VmVecDecf (vPlasma, &vDelta);
				VmVecIncf (vPlasma + 1, &vDelta);
				VmVecScalef (&vDelta, VmVecSubf (&vDelta, vPlasma + 2, vPlasma + 3), 0.25f);
				VmVecDecf (vPlasma + 2, &vDelta);
				VmVecIncf (vPlasma + 3, &vDelta);
				}
			}
		}
	if (bDrawArrays)
		OglDisableClientStates (1, 0);
	}
OglBlendFunc (GL_SRC_ALPHA, GL_ONE);
glColor4fv ((GLfloat *) &color);
glLineWidth ((GLfloat) (nDepth ? 2 : 3));
glDisable (GL_TEXTURE_2D);
glEnable (GL_SMOOTH);
glBegin (GL_LINES);
glVertex3fv ((GLfloat *) vLine);
glVertex3fv ((GLfloat *) (vLine + 1));
glEnd ();
glLineWidth (1);
glDisable (GL_SMOOTH);
OglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//------------------------------------------------------------------------------

void RenderLightningPlasma (fVector *vPosf, tRgbaColorf *color, int nScale, int bDrawArrays, 
									 char bStart, char bEnd, char bPlasma, short nDepth, int bDepthSort)
{
	static fVector	vEye = {{0,0,0}};
	static fVector	vPlasma [6] = {{{0,0,0}},{{0,0,0}},{{0,0,0}},{{0,0,0}},{{0,0,0}},{{0,0,0}}};
	static fVector vNormal [3] = {{{0,0,0}},{{0,0,0}},{{0,0,0}}};

	fVector	vn [2], vd;
	int		i, j = bStart + 2 * bEnd;

memcpy (vNormal, vNormal + 1, 2 * sizeof (fVector));
if (bStart) {
	VmVecNormalf (vNormal + 1, vPosf, vPosf + 1, &vEye);
	vn [0] = vNormal [1];
	}
else
	VmVecScalef (vn, VmVecAddf (vn, vNormal, vNormal + 1), 0.5f);
if (bEnd)
	vn [1] = vNormal [1];
else {
	VmVecNormalf (vNormal + 2, vPosf + 1, vPosf + 2, &vEye);
	VmVecScalef (vn + 1, VmVecAddf (vn + 1, vNormal + 1, vNormal + 2), 0.5f);
	}
if (!(nDepth || nScale)) {
	VmVecScalef (vn, vn, 2);
	VmVecScalef (vn + 1, vn + 1, 2);
	}
if (!nScale && nDepth) {
	VmVecScalef (vn, vn, 0.5f);
	VmVecScalef (vn + 1, vn + 1, 0.5f);
	}
if (bStart) {
	VmVecAddf (vPlasma, vPosf, vn);
	VmVecSubf (vPlasma + 1, vPosf, vn);
	VmVecSubf (&vd, vPosf, vPosf + 1);
	VmVecNormalizef (&vd, &vd);
	if (nScale)
		VmVecScalef (&vd, &vd, 0.5f);
	VmVecIncf (vPlasma, &vd);
	VmVecIncf (vPlasma + 1, &vd);
	}
else {
	vPlasma [0] = vPlasma [3];
	vPlasma [1] = vPlasma [2];
	}
VmVecAddf (vPlasma + 3, vPosf + 1, vn + 1);
VmVecSubf (vPlasma + 2, vPosf + 1, vn + 1);
if (bEnd) {
	VmVecSubf (&vd, vPosf + 1, vPosf);
	VmVecNormalizef (&vd, &vd);
	if (nScale)
		VmVecScalef (&vd, &vd, 0.5f);
	VmVecIncf (vPlasma + 2, &vd);
	VmVecIncf (vPlasma + 3, &vd);
	}
if (bDepthSort) {
	RIAddLightningSegment (vPosf, vPlasma, color, bPlasma, bStart, bEnd, nDepth);
	}
else {
#if 1
	if (bDrawArrays) {
		glTexCoordPointer (2, GL_FLOAT, sizeof (tUVLf), uvlPlasma + j);
		glVertexPointer (3, GL_FLOAT, sizeof (fVector), vPlasma);
		glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
		}
	else {
#if 0
		float		fDot;
		VmVecNormalf (vn, vPlasma, vPlasma + 1, vPlasma + 2);
		VmVecNormalf (vn + 1, vPlasma, vPlasma + 2, vPlasma + 3);
		fDot = VmVecDotf (vn, vn + 1);
		if (fDot >= 0) {
			glBegin (GL_TRIANGLES);
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j]));
			glVertex3fv ((GLfloat *) (vPlasma));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 1));
			glVertex3fv ((GLfloat *) (vPlasma + 1));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 2));
			glVertex3fv ((GLfloat *) (vPlasma + 2));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j]));
			glVertex3fv ((GLfloat *) (vPlasma));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 2));
			glVertex3fv ((GLfloat *) (vPlasma + 2));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 3));
			glVertex3fv ((GLfloat *) (vPlasma + 3));
			glEnd ();
			}	
		else if (fDot < 0) {
			glBegin (GL_TRIANGLES);
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j]));
			glVertex3fv ((GLfloat *) (vPlasma));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 1));
			glVertex3fv ((GLfloat *) (vPlasma + 1));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 3));
			glVertex3fv ((GLfloat *) (vPlasma + 3));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j]));
			glVertex3fv ((GLfloat *) (vPlasma));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 2));
			glVertex3fv ((GLfloat *) (vPlasma + 2));
			glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + 3));
			glVertex3fv ((GLfloat *) (vPlasma + 3));
			glEnd ();
			}
		else 
#endif
			{
			glBegin (GL_QUADS);
			for (i = 0; i < 4; i++) {
				glTexCoord2fv ((GLfloat *) (uvlPlasma [j] + i));
				glVertex3fv ((GLfloat *) (vPlasma + i));
				}	
			}
		glEnd ();
		}
#endif
#if RENDER_LIGHTNING_OUTLINE //render lightning segment outline
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	glLineWidth (1);
	if (bStart)
		glColor3f (1,0,0);
	else
		glColor3f (0,0,1);
	glBegin (GL_LINE_LOOP);
	for (i = 0; i < 4; i++) {
		glVertex3fv ((GLfloat *) (vPlasma + i));
		}
	glEnd ();
	glColor4fv ((GLfloat *) color);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
#endif
	}
}

//------------------------------------------------------------------------------

void RenderLightning (tLightning *pl, int nLightnings, short nDepth, int bDepthSort)
{
	tLightningNode	*pln;
	fVector		vPosf [3] = {{{0,0,0}},{{0,0,0}}};
	int			i;
#if RENDER_LIGHTNING_PLASMA
	int			h, j, bDrawArrays, bPlasma;
#else
	int			bPlasma = 0;
#endif
	tRgbaColorf	color;
	tObject		*objP = NULL;

if (!pl)
	return;
if (bDepthSort > 0) {
	bPlasma = gameOpts->render.lightnings.bPlasma && pl->bPlasma;
	color = pl->color;
	if (pl->nLife > 0) {
		if ((i = pl->nLife - pl->nTTL) < 250)
			color.alpha *= (float) i / 250.0f;
		else if (pl->nTTL < pl->nLife / 3)
			color.alpha *= (float) pl->nTTL / (float) (pl->nLife / 3);
		}
	color.red *= (float) (0.9 + f_rand () / 5);
	color.green *= (float) (0.9 + f_rand () / 5);
	color.blue *= (float) (0.9 + f_rand () / 5);
	for (; nLightnings; nLightnings--, pl++) {
		if ((pl->nNodes < 0) || (pl->nSteps < 0))
			continue;
#if RENDER_LIGHTING_SEGMENTS
		bPlasma = gameOpts->render.lightnings.bPlasma && pl->bPlasma;
		for (i = pl->nNodes - 1, j = 0, pln = pl->pNodes; j <= i; j++) {
			if (j < i)
				memcpy (vPosf, vPosf + 1, 2 * sizeof (fVector));
			if (!j) {
				VmsVecToFloat (vPosf + 1, &(pln++)->vPos);
				G3TransformPointf (vPosf + 1, vPosf + 1, 0);
				}
			if (j < i) {
				VmsVecToFloat (vPosf + 2, &(++pln)->vPos);
				G3TransformPointf (vPosf + 2, vPosf + 2, 0);
				}
			if (j)
				RenderLightningPlasma (vPosf, &color, 0, 0, j == 1, j == i, 1, nDepth, 1);
			}
#else
		RIAddLightnings (pl, 1, nDepth);
#endif
		if (gameOpts->render.lightnings.nQuality)
			for (i = pl->nNodes, pln = pl->pNodes; i; i--, pln++)
				if (pln->pChild)
					RenderLightning (pln->pChild, 1, nDepth + 1, 1);
		}
	}
else {
	if (!nDepth) {
		glEnable (GL_BLEND);
		if ((bDepthSort < 1) || (gameOpts->render.bDepthSort < 1)) {
			glDepthMask (0);
			glDisable (GL_CULL_FACE);
			}
		}
	for (; nLightnings; nLightnings--, pl++) {
		if ((pl->nNodes < 0) || (pl->nSteps < 0))
			continue;
		bPlasma = gameOpts->render.lightnings.bPlasma && pl->bPlasma;
		color = pl->color;
		if (pl->nLife > 0) {
			if ((i = pl->nLife - pl->nTTL) < 250)
				color.alpha *= (float) i / 250.0f;
			else if (pl->nTTL < pl->nLife / 3)
				color.alpha *= (float) pl->nTTL / (float) (pl->nLife / 3);
			}
		color.red *= (float) (0.9 + f_rand () / 5);
		color.green *= (float) (0.9 + f_rand () / 5);
		color.blue *= (float) (0.9 + f_rand () / 5);
		if (!bPlasma)
			color.alpha *= 1.5f;
		if (nDepth)
			color.alpha /= 2;
#if RENDER_LIGHTNING_PLASMA
		if (bPlasma) {
			bDrawArrays = 0; //OglEnableClientStates (1, 0);
			glEnable (GL_TEXTURE_2D);
			if (LoadCorona () && !OglBindBmTex (bmpCorona, 1, -1)) {
				OglTexWrap (bmpCorona->glTexture, GL_CLAMP);
				for (h = 0; h < 2; h++) {
					if (!h) {
						OglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						glColor4f (color.red / 2, color.green / 2, color.blue / 2, color.alpha);
						}
					else {
#if 0
						OglBlendFunc (GL_SRC_ALPHA, GL_ONE);
#endif
						glColor4f (1, 1, 1, color.alpha / 2);
						}
					for (i = pl->nNodes - 1, j = 0, pln = pl->pNodes; j <= i; j++) {
						if (j < i)
							memcpy (vPosf, vPosf + 1, 2 * sizeof (fVector));
						if (!j) {
							VmsVecToFloat (vPosf + 1, &(pln++)->vPos);
							G3TransformPointf (vPosf + 1, vPosf + 1, 0);
							}
						if (j < i) {
							VmsVecToFloat (vPosf + 2, &(++pln)->vPos);
							G3TransformPointf (vPosf + 2, vPosf + 2, 0);
							}
						if (j)
							RenderLightningPlasma (vPosf, &color, h, bDrawArrays, j == 1, j == i, 1, nDepth, 0);
						}
					}
				}
			if (bDrawArrays)
				OglDisableClientStates (1, 0);
			}
#endif
#if 1
#	if 0
		if (nDepth)
			color.alpha /= 2;
#	endif
		OglBlendFunc (GL_SRC_ALPHA, GL_ONE);
		glColor4fv ((GLfloat *) &color);
		glLineWidth ((GLfloat) (nDepth ? 2 : 4));
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_SMOOTH);
		glBegin (GL_LINE_STRIP);
		for (i = pl->nNodes, pln = pl->pNodes; i; i--, pln++) {
			VmsVecToFloat (vPosf, &pln->vPos);
			G3TransformPointf (vPosf, vPosf, 0);
			glVertex3fv ((GLfloat *) vPosf);
			}
		glEnd ();
#	if 0
		if (nDepth)
			color.alpha *= 2;
#	endif
#endif
#if defined (_DEBUG) && RENDER_TARGET_LIGHTNING
		glColor3f (1,0,0,1);
		glLineWidth (1);
		glBegin (GL_LINE_STRIP);
		for (i = pl->nNodes, pln = pl->pNodes; i; i--, pln++) {
			VmsVecToFloat (vPosf, &pln->vNewPos);
			G3TransformPointf (vPosf, vPosf, 0);
			glVertex3fv ((GLfloat *) vPosf);
			}
		glEnd ();
#endif
		if (gameOpts->render.lightnings.nQuality)
			for (i = pl->nNodes, pln = pl->pNodes; i; i--, pln++)
				if (pln->pChild)
					RenderLightning (pln->pChild, 1, nDepth + 1, bDepthSort);
		}
	if (!nDepth) {
		if ((bDepthSort < 1) || (gameOpts->render.bDepthSort < 1)) {
			glEnable (GL_CULL_FACE);
			glDepthMask (1);
			}
		}
	glLineWidth (1);
	glDisable (GL_SMOOTH);
	OglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

//------------------------------------------------------------------------------

void RenderLightnings (void)
{
if (SHOW_LIGHTNINGS) {
		tLightningBundle	*plb;
		int			i, n, bStencil = StencilOff ();

	for (i = gameData.lightnings.iUsed; i >= 0; i = n) {
		plb = gameData.lightnings.buffer + i;
		n = plb->nNext;
		if (!(plb->nKey [0] | plb->nKey [1]))
			RenderLightning (plb->pl, plb->nLightnings, 0, gameOpts->render.bDepthSort > 0);
		}
	StencilOn (bStencil);
	}
}

//------------------------------------------------------------------------------

vmsVector *FindLightningTargetPos (tObject *emitterP, short nTarget)
{
	int				i;
	tObject			*objP;

if (!nTarget)
	return 0;
for (i = 0, objP = gameData.objs.objects; i <= gameData.objs.nLastObject; i++, objP++)
	if ((objP != emitterP) && (objP->nType == OBJ_EFFECT) && (objP->id == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget))
		return &objP->position.vPos;
return NULL;
}

//------------------------------------------------------------------------------

void StaticLightningFrame (void)
{
	int				i;
	tObject			*objP;
	vmsVector		*vEnd, *vDelta, v;
	tLightningInfo	*pli;
	tRgbaColorf		color;

if (!SHOW_LIGHTNINGS)
	return;
if (!gameOpts->render.lightnings.bStatic)
	return;
for (i = 0, objP = gameData.objs.objects; i <= gameData.objs.nLastObject; i++, objP++) {
	if ((objP->nType != OBJ_EFFECT) || (objP->id != LIGHTNING_ID))
		continue;
	if (gameData.lightnings.objects [i] >= 0)
		continue;
	pli = &objP->rType.lightningInfo;
	if (pli->nLightnings <= 0)
		continue;
	if (pli->bRandom && !pli->nAngle)
		vEnd = NULL;
	else if (vEnd = FindLightningTargetPos (objP, pli->nTarget))
		pli->nLength = VmVecDist (&objP->position.vPos, vEnd) / F1_0;
	else {
		VmVecScaleAdd (&v, &objP->position.vPos, &objP->position.mOrient.fVec, F1_0 * pli->nLength);
		vEnd = &v;
		}
	color.red = (float) pli->color.red / 255.0f;
	color.green = (float) pli->color.green / 255.0f;
	color.blue = (float) pli->color.blue / 255.0f;
	color.alpha = (float) pli->color.alpha / 255.0f;
	vDelta = pli->bInPlane ? &objP->position.mOrient.rVec : NULL;
	gameData.lightnings.objects [i] =
		CreateLightning (pli->nLightnings, &objP->position.vPos, vEnd, vDelta, i, -abs (pli->nLife), pli->nDelay, pli->nLength * F1_0,
							  pli->nAmplitude * F1_0, pli->nAngle, pli->nOffset * F1_0, pli->nNodes, pli->nChildren, pli->nChildren > 0, pli->nSteps,
							  pli->nSmoothe, pli->bClamp, pli->bPlasma, pli->bSound, pli->nStyle, &color);
	}
}

//------------------------------------------------------------------------------

void DoLightningFrame (void)
{
if (gameData.lightnings.bDestroy) {
	gameData.lightnings.bDestroy = -1;
	DestroyAllLightnings (0);
	}
else {
	UpdateLightnings ();
	UpdateOmegaLightnings (NULL, NULL);
	StaticLightningFrame ();
	}
}

//------------------------------------------------------------------------------

void SetLightningSegLight (short nSegment, vmsVector *vPos, tRgbaColorf *colorP)
{
	tLightningLight	*pll = gameData.lightnings.lights + nSegment;

if (pll->nFrameFlipFlop != gameStates.render.nFrameFlipFlop) {
	memset (pll, 0, sizeof (*pll));
	pll->nFrameFlipFlop = gameStates.render.nFrameFlipFlop;
	pll->nNext = gameData.lightnings.nFirstLight;
	gameData.lightnings.nFirstLight = nSegment;
	}
pll->nLights++;
VmVecInc (&pll->vPos, vPos);
pll->color.red += colorP->red;
pll->color.green += colorP->green;
pll->color.blue += colorP->blue;
pll->color.alpha += colorP->alpha;
}

//------------------------------------------------------------------------------

int SetLightningLight (tLightning *pl, int i)
{
	tLightningNode	*pln;
	vmsVector		*vPos;
	short				nSegment;
	int				j, nLights;
	double			h, nStep;

for (nLights = 0; i; i--, pl++) {
	if (0 < (j = pl->nNodes)) {
		if (!(nStep = (double) (j - 1) / ((double) pl->nLength / F1_0)))
			nStep = j - 1;
		nSegment = pl->nSegment;
		pln = pl->pNodes;
		SetLightningSegLight (nSegment, &pln->vPos, &pl->color);
		nLights++;
		for (h = 0; h < j; h += nStep) {
			vPos = &pln [(int) h].vPos;
			if (0 > (nSegment = FindSegByPoint (vPos, nSegment, 0)))
				break;
			SetLightningSegLight (nSegment, vPos, &pl->color);
			nLights++;
			}
		}
	}
return nLights;
}

//------------------------------------------------------------------------------

void ResetLightningLights (void)
{
if (SHOW_LIGHTNINGS) {
		tLightningLight	*pll;
		int					i, nLights = 0;

	for (i = gameData.lightnings.nFirstLight; i >= 0; ) {
		pll = gameData.lightnings.lights + i;
		i = pll->nNext;
		pll->nNext = -1;
		pll->color.red =
		pll->color.green =
		pll->color.blue = 0;
		pll->nBrightness = 0;
		}
	gameData.lightnings.nFirstLight = -1;
	}
}

//------------------------------------------------------------------------------

void SetLightningLights (void)
{
if (SHOW_LIGHTNINGS) {
		tLightningBundle	*plb;
		tLightningLight	*pll;
		int					i, n, nLights = 0, bDynLighting = gameOpts->render.bDynLighting;

	gameData.lightnings.nFirstLight = -1;
	for (i = gameData.lightnings.iUsed; i >= 0; i = n) {
		plb = gameData.lightnings.buffer + i;
		n = plb->nNext;
		nLights += SetLightningLight (plb->pl, plb->nLightnings);
		}
	if (nLights) {
		if (bDynLighting)
			RemoveDynLightningLights ();
		for (i = gameData.lightnings.nFirstLight; i >= 0; i = pll->nNext) {
			pll = gameData.lightnings.lights + i;
			n = pll->nLights;
			VmVecScale (&pll->vPos, F1_0 / n);
			pll->color.red /= n;
			pll->color.green /= n;
			pll->color.blue /= n;
			pll->nBrightness = fl2f (sqrt ((pll->color.red * 3 + pll->color.green * 5 + pll->color.blue * 2) * pll->color.alpha));
			if (bDynLighting)
				AddDynLight (&pll->color, pll->nBrightness, i, -1, -1);
			}
		}
	}
}

//------------------------------------------------------------------------------

void CreateExplosionLightnings (tObject *objP, tRgbaColorf *colorP, int nRods, int nRad)
{
if (SHOW_LIGHTNINGS && gameOpts->render.lightnings.bExplosions) {
	//gameData.lightnings.objects [OBJ_IDX (objP)] = 
		CreateLightning (
			nRods, &objP->position.vPos, NULL, NULL, OBJ_IDX (objP), 750, 0, 
			nRad, F1_0 * 4, 0, 2 * F1_0, 50, 5, 1, 3, 1, 1, 0, 0, -1, colorP);
	}
}

//------------------------------------------------------------------------------

void CreateShakerLightnings (tObject *objP)
{
static tRgbaColorf color = {0.1f, 0.1f, 0.8f, 0.2f};

CreateExplosionLightnings (objP, &color, 30, 20 * F1_0);
}

//------------------------------------------------------------------------------

void CreateShakerMegaLightnings (tObject *objP)
{
static tRgbaColorf color = {0.1f, 0.1f, 0.6f, 0.2f};

CreateExplosionLightnings (objP, &color, 20, 15 * F1_0);
}

//------------------------------------------------------------------------------

void CreateMegaLightnings (tObject *objP)
{
static tRgbaColorf color = {0.8f, 0.1f, 0.1f, 0.2f};

CreateExplosionLightnings (objP, &color, 30, 15 * F1_0);
}

//------------------------------------------------------------------------------

void CreateRobotLightnings (tObject *objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNINGS && gameOpts->render.lightnings.bRobots && OBJECT_EXISTS (objP)) {
		int i = OBJ_IDX (objP);

	if (0 <= gameData.lightnings.objects [i]) 
		MoveObjectLightnings (objP);
	else
		gameData.lightnings.objects [i] = CreateLightning (
			2 * objP->size / F1_0, &objP->position.vPos, NULL, NULL, OBJ_IDX (objP), -5000, 1000, 
			objP->size, objP->size / 8, 0, 0, 25, 3, 1, 5, 1, 1, 0, 1, 0, colorP);
	}
}

//------------------------------------------------------------------------------

void CreatePlayerLightnings (tObject *objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNINGS && gameOpts->render.lightnings.bPlayers && OBJECT_EXISTS (objP)) {
		int i = OBJ_IDX (objP);

	if (0 <= gameData.lightnings.objects [i]) 
		MoveObjectLightnings (objP);
	else
		gameData.lightnings.objects [i] = CreateLightning (
			4 * objP->size / F1_0, &objP->position.vPos, NULL, NULL, OBJ_IDX (objP), -5000, 1000, 
			4 * objP->size, objP->size, 0, 2 * objP->size, 50, 5, 1, 5, 1, 1, 0, 1, 1, colorP);
	}
}

//------------------------------------------------------------------------------

void CreateDamageLightnings (tObject *objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNINGS && gameOpts->render.lightnings.bDamage && OBJECT_EXISTS (objP)) {
		int h, n, i = OBJ_IDX (objP);
		tLightningBundle	*plb;

	n = f2ir (RobotDefaultShields (objP));
	h = f2ir (objP->shields) * 100 / n;
	if ((h < 0) || (h >= 50))
		return;	
	n = (5 - h / 10) * 2;
	if (0 <= (h = gameData.lightnings.objects [i])) {
		plb = gameData.lightnings.buffer + h;
		if (plb->nLightnings == n) {
			MoveObjectLightnings (objP);
			return;
			}
		DestroyLightnings (h, NULL, 0);
		}
	gameData.lightnings.objects [i] = CreateLightning (
		n, &objP->position.vPos, NULL, NULL, OBJ_IDX (objP), -1000, 4000, 
		objP->size, objP->size / 8, 0, 0, 20, 0, 1, 10, 1, 1, 0, 1, -1, colorP);
	}
}

//------------------------------------------------------------------------------

int FindDamageLightning (short nObject, int *pKey)
{
		tLightningBundle	*plb;
		int					i;

for (i = gameData.lightnings.iUsed; i >= 0; i = plb->nNext) {
	plb = gameData.lightnings.buffer + i;
	if ((plb->nObject == nObject) && (plb->nKey [0] == pKey [0]) && (plb->nKey [1] == pKey [1]))
		return i;
	}
return -1;
}

//------------------------------------------------------------------------------

typedef union tPolyKey {
	int	i [2];
	short	s [4];
} tPolyKey;

void RenderDamageLightnings (tObject *objP, g3sPoint **pointList, int nVertices)
{
	tLightningBundle	*plb;
	vmsVector			vPos, vEnd;
	vmsVector			vDelta, vNorm;
	int					h, i, j, bUpdate = 0;
	short					nObject;
	tPolyKey				key;

	static short	nLastObject = -1;
	static float	fDamage;
	static int		nFrameFlipFlop = -1;

	static tRgbaColorf color = {0.2f, 0.2f, 1.0f, 1.0f};

if (!(SHOW_LIGHTNINGS && gameOpts->render.lightnings.bDamage))
	return;
if ((objP->nType != OBJ_ROBOT) && (objP->nType != OBJ_PLAYER))
	return;
if (nVertices < 3)
	return;
j = (nVertices > 4) ? 4 : nVertices;
h = (nVertices + 1) / 2;
for (i = 0; i < j; i++)
	key.s [i] = pointList [i]->p3_key;
for (; i < 4; i++)
	key.s [i] = 0;
i = FindDamageLightning (nObject = OBJ_IDX (objP), key.i);
if (i < 0) {
	if ((nLastObject != nObject) || (nFrameFlipFlop != gameStates.render.nFrameFlipFlop)) {
		nLastObject = nObject;
		nFrameFlipFlop = gameStates.render.nFrameFlipFlop;
		fDamage = (0.5f - ObjectDamage (objP)) / 250.0f;
		}
#if 1
	if (f_rand () > fDamage)
		return;
#endif
	vPos = pointList [0]->p3_src;
	vEnd = pointList [1 + rand () % (nVertices - 1)]->p3_src;
	VmVecNormal (&vNorm, &vPos, &pointList [1]->p3_src, &vEnd);
	VmVecScaleInc (&vPos, &vNorm, F1_0 / 64);
	VmVecScaleInc (&vEnd, &vNorm, F1_0 / 64);
	VmVecNormal (&vDelta, &vNorm, &vPos, &vEnd);
	h = VmVecDist (&vPos, &vEnd);
	i = CreateLightning (1, &vPos, &vEnd, NULL /*&vDelta*/, nObject, 1000 + rand () % 2000, 0, 
								h, h / 4 + rand () % 2, 0, 0, 20, 2, 1, 5, 0, 1, 0, 0, 1, &color);
	bUpdate = 1;
	}
if (i >= 0) {
	plb = gameData.lightnings.buffer + i;
	glDisable (GL_CULL_FACE);
	if (bUpdate) {
		plb->nKey [0] = key.i [0];
		plb->nKey [1] = key.i [1];
		}
	plb->nLightnings = UpdateLightning (plb->pl, plb->nLightnings, 0);
	RenderLightning (plb->pl, plb->nLightnings, 0, -1);
	}
}

//------------------------------------------------------------------------------
//eof
