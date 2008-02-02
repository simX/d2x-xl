/* $Id: interp.c, v 1.14 2003/03/19 19:21:34 btb Exp $ */
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

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid [] = "$Id: interp.c, v 1.14 2003/03/19 19:21:34 btb Exp $";
#endif

#include <math.h>
#include <stdlib.h>
#include "error.h"

#include "inferno.h"
#include "interp.h"
#include "hitbox.h"
#include "gr.h"
#include "ogl_color.h"
#include "hiresmodels.h"
#include "buildmodel.h"

//------------------------------------------------------------------------------

bool G3CountPOFModelItems (void *modelP, short *pnSubModels, short *pnVerts, short *pnFaces, short *pnFaceVerts)
{
	ubyte *p = modelP;

G3CheckAndSwap (modelP);
for (;;)
	switch (WORDVAL (p)) {
		case OP_EOF:
			return 1;

		case OP_DEFPOINTS: {
			int n = WORDVAL (p+2);
			(*pnVerts) += n;
			p += n * sizeof (vmsVector) + 4;
			break;
			}

		case OP_DEFP_START: {
			int n = WORDVAL (p+2);
			p += n * sizeof (vmsVector) + 8;
			(*pnVerts) += n;
			break;
			}

		case OP_FLATPOLY: {
			int nVerts = WORDVAL (p+2);
			(*pnFaces)++;
			(*pnFaceVerts) += nVerts;
			p += 30 + ((nVerts & ~ 1) + 1) * 2;
			break;
			}

		case OP_TMAPPOLY: {
			int nVerts = WORDVAL (p + 2);
			(*pnFaces)++;
			(*pnFaceVerts) += nVerts;
			p += 30 + ((nVerts & ~1) + 1) * 2 + nVerts * 12;
			break;
			}

		case OP_SORTNORM:
			G3CountPOFModelItems (p + WORDVAL (p+28), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			G3CountPOFModelItems (p + WORDVAL (p+30), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			p += 32;
			break;


		case OP_RODBM: {
			p += 36;
			break;
			}

		case OP_SUBCALL: {
			(*pnSubModels)++;
			G3CountPOFModelItems (p + WORDVAL (p+16), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			p += 20;
			break;
			}

		case OP_GLOW:
			p += 4;
			break;

		default:
			Error ("invalid polygon model\n");
	}
return 1;
}

//------------------------------------------------------------------------------

tG3ModelFace *G3AddModelFace (tG3Model *pm, tG3SubModel *psm, tG3ModelFace *pmf, vmsVector *pn, ubyte *p, 
										grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	short				nVerts = WORDVAL (p+2);
	tG3ModelVertex	*pmv;
	short				*pfv;
	tUVL				*uvl;
	grsBitmap		*bmP;
	tRgbaColorf		baseColor;
	fVector3			n, *pvn;
	short				i, j;
	ushort			c;
	char				bTextured;

Assert (pmf - pm->pFaces < pm->nFaces);
if (modelBitmaps && *modelBitmaps) {
	bTextured = 1;
	pmf->nBitmap = WORDVAL (p+28);
	bmP = modelBitmaps [pmf->nBitmap];
	if (pObjColor) {
		ubyte c = bmP->bmAvgColor;
		pObjColor->red = CPAL2Tr (gamePalette, c);
		pObjColor->green = CPAL2Tg (gamePalette, c);
		pObjColor->blue = CPAL2Tb (gamePalette, c);
		}
	baseColor.red = baseColor.green = baseColor.blue = baseColor.alpha = 1;
	i = (int) (bmP - gameData.pig.tex.bitmaps [0]);
	pmf->bThruster = (i == 24) || ((i >= 1741) && (i <= 1745));
	}
else {
	bTextured = 0;
	pmf->nBitmap = -1;
	c = WORDVAL (p + 28);
	baseColor.red = (float) PAL2RGBA (((c >> 10) & 31) << 1) / 255.0f;
	baseColor.green = (float) PAL2RGBA (((c >> 5) & 31) << 1) / 255.0f;
	baseColor.blue = (float) PAL2RGBA ((c & 31) << 1) / 255.0f;
	baseColor.alpha = GrAlpha ();
	if (pObjColor)
		*pObjColor = baseColor;
	}
pmf->vNormal = *pn;
pmf->nIndex = pm->iFaceVert;
pmv = pm->pFaceVerts + pm->iFaceVert;
pvn = pm->pVertNorms + pm->iFaceVert;
if (psm->nIndex < 0)
	psm->nIndex = pm->iFaceVert;
pmf->nVerts = nVerts;
if ((pmf->bGlow = (nGlow >= 0)))
	nGlow = -1;
uvl = (tUVL *) (p + 30 + (nVerts | 1) * 2);
VmVecFixToFloat3 (&n, pn);
for (i = nVerts, pfv = WORDPTR (p+30); i; i--, pfv++, uvl++, pmv++, pvn++) {
	j = *pfv;
	pmv->vertex = pm->pVerts [j];
	pmv->texCoord.v.u = f2fl (uvl->u);
	pmv->texCoord.v.v = f2fl (uvl->v);
	pmv->renderColor =
	pmv->baseColor = baseColor;
	pmv->bTextured = bTextured;
	pmv->nIndex = j;
	pmv->normal = *pvn = n;
	G3SetSubModelMinMax (psm, &pmv->vertex);
	}
pm->iFaceVert += nVerts;
pmf++;
pm->iFace++;
psm->nFaces++;
return pmf;
}

//------------------------------------------------------------------------------

bool G3GetPOFModelItems (void *modelP, vmsAngVec *pAnimAngles, tG3Model *pm, int nThis, int nParent, 
								 grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	ubyte				*p = modelP;
	tG3SubModel		*psm = pm->pSubModels + nThis;
	tG3ModelFace	*pmf = pm->pFaces + pm->iFace;
	int				nChild;

G3CheckAndSwap (modelP);
nGlow = -1;
if (!psm->pFaces) {
	psm->pFaces = pmf;
	psm->nIndex = -1;
	psm->nParent = nParent;
	psm->nGunPoint = -1;
	psm->bThruster = 0;
	psm->bGlow = 0;
	}
for (;;)
	switch (WORDVAL (p)) {
		case OP_EOF:
			return 1;

		case OP_DEFPOINTS: {
			int i, n = WORDVAL (p+2);
			fVector3 *pfv = pm->pVerts;
			vmsVector *pv = VECPTR(p+4);
			for (i = n; i; i--)
				VmVecFixToFloat3 (pfv++, pv++);
			p += n * sizeof (vmsVector) + 4;
			break;
			}

		case OP_DEFP_START: {
			int i, n = WORDVAL (p+2);
			int s = WORDVAL (p+4);
			fVector3 *pfv = pm->pVerts + s;
			vmsVector *pv = VECPTR(p+8);
			for (i = n; i; i--)
				VmVecFixToFloat3 (pfv++, pv++);
			p += n * sizeof (vmsVector) + 8;
			break;
			}

		case OP_FLATPOLY: {
			int nVerts = WORDVAL (p+2);
			pmf = G3AddModelFace (pm, psm, pmf, VECPTR (p+16), p, NULL, pObjColor);
			p += 30 + (nVerts | 1) * 2;
			break;
			}

		case OP_TMAPPOLY: {
			int nVerts = WORDVAL (p + 2);
			pmf = G3AddModelFace (pm, psm, pmf, VECPTR (p+16), p, modelBitmaps, pObjColor);
			p += 30 + (nVerts | 1) * 2 + nVerts * 12;
			break;
			}

		case OP_SORTNORM:
			G3GetPOFModelItems (p + WORDVAL (p+28), pAnimAngles, pm, nThis, nParent, modelBitmaps, pObjColor);
			pmf = pm->pFaces + pm->iFace;
			G3GetPOFModelItems (p + WORDVAL (p+30), pAnimAngles, pm, nThis, nParent, modelBitmaps, pObjColor);
			pmf = pm->pFaces + pm->iFace;
			p += 32;
			break;

		case OP_RODBM:
			p+=36;
			break;

		case OP_SUBCALL: {
			nChild = ++pm->iSubModel;
			pm->pSubModels [nChild].vOffset = *VECPTR (p+4);
			pm->pSubModels [nChild].nAngles = WORDVAL (p+2);
			G3InitSubModelMinMax (pm->pSubModels + nChild);
			G3GetPOFModelItems (p + WORDVAL (p+16), pAnimAngles, pm, nChild, nThis, modelBitmaps, pObjColor);
			pmf = pm->pFaces + pm->iFace;
			p += 20;
			break;
			}

		case OP_GLOW:
			nGlow = WORDVAL (p+2);
			p += 4;
			break;

		default:
			Error ("invalid polygon model\n");
		}
return 1;
}

//------------------------------------------------------------------------------

int G3BuildModelFromPOF (tObject *objP, int nModel, tPolyModel *pp, grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	tG3Model	*pm = gameData.models.g3Models [0] + nModel;

if (!pp->modelData)
	return 0;
pm->nSubModels = 1;
#ifdef _DEBUG
HUDMessage (0, "optimizing model");
#endif
G3CountPOFModelItems (pp->modelData, &pm->nSubModels, &pm->nVerts, &pm->nFaces, &pm->nFaceVerts);
if (!G3AllocModel (pm))
	return 0;
G3InitSubModelMinMax (pm->pSubModels);
G3GetPOFModelItems (pp->modelData, NULL, pm, 0, -1, modelBitmaps, pObjColor);
gameData.models.polyModels [nModel].rad = G3ModelSize (objP, pm, nModel, 0);
G3SetupModel (pm, 0, 1);
pm->iSubModel = 0;
return -1;
}

//------------------------------------------------------------------------------
//eof
