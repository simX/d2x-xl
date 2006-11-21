#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif
//#include  "oof.h
#include  "inferno.h"
#include  "cfile.h"
#include  "args.h"
#include  "u_mem.h"
#include  "gr.h"
#include  "error.h"
#include  "globvars.h"
#include  "3d.h"
#include  "lighting.h"
#include  "ogl_init.h"
#include  "network.h"
#include  "vecmat.h"
#include  "render.h"
#include  "strutil.h"
#include  "hudmsg.h"
#include  "ogl_init.h"
#include  "oof.h"

//------------------------------------------------------------------------------

#define OOF_MEM_OPT	1
#define GL_INFINITY	0
#define SHADOW_TEST	0
#define NORM_INF		1
#define INFINITY		10000.0f

#ifdef __unix
#	define	stricmp	strcasecmp
#	define	strnicmp	strncasecmp
#endif

#define oof_free(_p)	if (_p) {d_free (_p); (_p) = NULL;}

extern int bShadowTest;

static tOOF_vector vPos;
static tOOF_vector vLightPos;
static tOOF_vector vrLightPos;
static tOOF_matrix mView;
static int bZPass = 0;
static int bContourEdges = 1;
static int bSWCulling = 0;
static int bFrontCap = 1;
static int bRearCap = 1;
static int bShadowVolume = 1;

//------------------------------------------------------------------------------

static int nIndent = 0;
static int bLogOOF = 0;
extern  FILE *fErr;

void _CDECL_ OOF_PrintLog (char *fmt, ...)
{
if (bLogOOF) {
	va_list arglist;
	static char szLog [1024];

	memset (szLog, ' ', nIndent);
	va_start (arglist, fmt);
	vsprintf (szLog + nIndent, fmt, arglist);
	va_end (arglist);
	fprintf (fErr, szLog);
	}
}

//------------------------------------------------------------------------------

sbyte OOF_ReadByte (CFILE *fp, char *pszIdent)
{
sbyte b = CFReadByte (fp);
OOF_PrintLog ("      %s = %d\n", pszIdent, b);
return b;
}

//------------------------------------------------------------------------------

int OOF_ReadInt (CFILE *fp, char *pszIdent)
{
int i = CFReadInt (fp);
OOF_PrintLog ("      %s = %d\n", pszIdent, i);
return i;
}

//------------------------------------------------------------------------------

float OOF_ReadFloat (CFILE *fp, char *pszIdent)
{
float f = CFReadFloat (fp);
OOF_PrintLog ("      %s = %1.4f\n", pszIdent, f);
return f;
}

//------------------------------------------------------------------------------

void OOF_ReadVector (CFILE *fp, tOOF_vector *pv, char *pszIdent)
{
pv->x = CFReadFloat (fp);
pv->y = CFReadFloat (fp);
pv->z = CFReadFloat (fp);
OOF_PrintLog ("      %s = %1.4f,%1.4f,%1.4f\n", pszIdent, pv->x, pv->y, pv->z);
}

//------------------------------------------------------------------------------

char *OOF_ReadString (CFILE *fp, char *pszIdent)
{
	char	*psz;
	int	l;

l = OOF_ReadInt (fp, "string length");
if (!(psz = d_malloc (l + 1)))
	return NULL;
if (CFRead (psz, l, 1, fp)) {
	psz [l] = '\0';
	OOF_PrintLog ("      %s = '%s'\n", pszIdent, psz);
	return psz;
	}
oof_free (psz);
return NULL;
}

//------------------------------------------------------------------------------

int ListType (char *pListId)
{
if (!strncmp (pListId, "TXTR", 4))
	return 0;
if (!strncmp (pListId, "OHDR", 4))
	return 1;
if (!strncmp (pListId, "SOBJ", 4))
	return 2;
if (!strncmp (pListId, "GPNT", 4))
	return 3;
if (!strncmp (pListId, "SPCL", 4))
	return 4;
if (!strncmp (pListId, "ATCH", 4))
	return 5;
if (!strncmp (pListId, "PANI", 4))
	return 6;
if (!strncmp (pListId, "RANI", 4))
	return 7;
if (!strncmp (pListId, "ANIM", 4))
	return 8;
if (!strncmp (pListId, "WBAT", 4))
	return 9;
if (!strncmp (pListId, "NATH", 4))
	return 10;
return -1;
}

//------------------------------------------------------------------------------

inline float OOF_VecDot3 (float x, float y, float z, tOOF_vector *pv)
{
return (x * pv->x) + (y * pv->y) + (z * pv->z);
}

//------------------------------------------------------------------------------

inline float OOF_VecDot (tOOF_vector *pv0, tOOF_vector *pv1)
{
return (pv0->x * pv1->x) + (pv0->y * pv1->y) + (pv0->z * pv1->z);
}

//------------------------------------------------------------------------------

inline float fsqr (float x)
{
return x * x;
}

float OOF_VecMag (tOOF_vector *pv)
{
return (float) sqrt (fsqr (pv->x) + fsqr (pv->y) + fsqr (pv->z));
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecScale (tOOF_vector *pv, float fScale)
{
pv->x *= fScale;
pv->y *= fScale;
pv->z *= fScale;
return pv;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecSub (tOOF_vector *pvDest, tOOF_vector *pvMin, tOOF_vector *pvSub)
{
pvDest->x = pvMin->x - pvSub->x;
pvDest->y = pvMin->y - pvSub->y;
pvDest->z = pvMin->z - pvSub->z;
return pvDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecAdd (tOOF_vector *pvDest, tOOF_vector *pvSrc, tOOF_vector *pvAdd)
{
pvDest->x = pvSrc->x + pvAdd->x;
pvDest->y = pvSrc->y + pvAdd->y;
pvDest->z = pvSrc->z + pvAdd->z;
return pvDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecNormalize (tOOF_vector *pv)
{
return OOF_VecScale (pv, 1.0f / OOF_VecMag (pv));
}

//------------------------------------------------------------------------------

float OOF_VecMul (tOOF_vector *pvSrc, tOOF_vector *pvMul)
{
return pvSrc->x * pvMul->x + pvSrc->y * pvMul->y + pvSrc->z * pvMul->z;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecInc (tOOF_vector *pvDest, tOOF_vector *pvSrc)
{
pvDest->x += pvSrc->x;
pvDest->y += pvSrc->y;
pvDest->z += pvSrc->z;
return pvDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecDec (tOOF_vector *pvDest, tOOF_vector *pvSrc)
{
pvDest->x -= pvSrc->x;
pvDest->y -= pvSrc->y;
pvDest->z -= pvSrc->z;
return pvDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_CrossProd (tOOF_vector *pvDest, tOOF_vector *pu, tOOF_vector *pv)
{
pvDest->x = (pu->y * pv->z) - (pu->z * pv->y);
pvDest->y = (pu->z * pv->x) - (pu->x * pv->z);
pvDest->z = (pu->x * pv->y) - (pu->y * pv->x);
return pvDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecPerp (tOOF_vector *pvPerp, tOOF_vector *pv0, tOOF_vector *pv1, tOOF_vector *pv2)
{
	tOOF_vector	u, v;

return OOF_CrossProd (pvPerp, OOF_VecSub (&u, pv1, pv0), OOF_VecSub (&v, pv2, pv1));
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecRot (tOOF_vector *pDest, tOOF_vector *pSrc, tOOF_matrix *pRot)
{
pDest->x = OOF_VecDot (pSrc, &pRot->r);
pDest->y = OOF_VecDot (pSrc, &pRot->u);
pDest->z = OOF_VecDot (pSrc, &pRot->f);
return pDest;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_Normalize (tOOF_vector *pv)
{
float fMag = OOF_VecMag (pv);
if (fMag > 0)
	OOF_VecScale (pv, 1.0f / fMag);
else {
	pv->x = 1.0f;
	pv->y =
	pv->z = 0.0f;
	}
return pv;
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_VecNormal (tOOF_vector *pvNormal, tOOF_vector *pv0, tOOF_vector *pv1, tOOF_vector *pv2)
{
return OOF_Normalize (OOF_VecPerp (pvNormal, pv0, pv1, pv2));
}

//------------------------------------------------------------------------------

float OOF_Centroid (tOOF_vector *pvCentroid, tOOF_vector *pvSrc, int nv)
{
	tOOF_vector	vNormal, vCenter;
	float			fArea, fTotalArea;
	int			i;

pvCentroid->x =
pvCentroid->y =
pvCentroid->z = 0.0f;
	
// First figure out the total area of this polygon
fTotalArea = OOF_VecMag (OOF_VecPerp (&vNormal, pvSrc, pvSrc + 1, pvSrc + 2)) / 2;
for (i = 2; i < nv - 1; i++) {
	fArea = OOF_VecMag (OOF_VecPerp (&vNormal, pvSrc, pvSrc + i, pvSrc + i + 1)) / 2;
	fTotalArea += fArea;
	}
// Now figure out how much weight each triangle represents to the overall polygon
fArea = OOF_VecMag (OOF_VecPerp (&vNormal, pvSrc, pvSrc + 1, pvSrc + 2)) / 2;
// Get the center of the first polygon
vCenter = *pvSrc;
OOF_VecInc (&vCenter, pvSrc + 1);
OOF_VecInc (&vCenter, pvSrc + 2);
OOF_VecInc (pvCentroid, OOF_VecScale (&vCenter, 1.0f / (3.0f * (fTotalArea / fArea))));
// Now do the same for the rest	
for (i = 2; i < nv - 1; i++) {
	fArea = OOF_VecMag (OOF_VecPerp (&vNormal, pvSrc, pvSrc + i, pvSrc + i + 1)) / 2;
	vCenter = *pvSrc;
	OOF_VecInc (&vCenter, pvSrc + i);
	OOF_VecInc (&vCenter, pvSrc + 1);
	OOF_VecInc (pvCentroid, OOF_VecScale (&vCenter, 1.0f / (3.0f * (fTotalArea / fArea))));
	}
return fTotalArea;
}

//------------------------------------------------------------------------------

void OOF_MatIdentity (tOOF_matrix *pm)
{
memset (pm, 0, sizeof (*pm));
pm->r.x =
pm->u.y =
pm->f.z = 1.0f;
}	

//------------------------------------------------------------------------------

void OOF_MatMul (tOOF_matrix *pd, tOOF_matrix *ps0, tOOF_matrix *ps1)
{
pd->r.x = OOF_VecDot3 (ps0->r.x, ps0->u.x, ps0->f.x, &ps1->r);
pd->u.x = OOF_VecDot3 (ps0->r.x, ps0->u.x, ps0->f.x, &ps1->u);
pd->f.x = OOF_VecDot3 (ps0->r.x, ps0->u.x, ps0->f.x, &ps1->f);
pd->r.y = OOF_VecDot3 (ps0->r.y, ps0->u.y, ps0->f.y, &ps1->r);
pd->u.y = OOF_VecDot3 (ps0->r.y, ps0->u.y, ps0->f.y, &ps1->u);
pd->f.y = OOF_VecDot3 (ps0->r.y, ps0->u.y, ps0->f.y, &ps1->f);
pd->r.z = OOF_VecDot3 (ps0->r.z, ps0->u.z, ps0->f.z, &ps1->r);
pd->u.z = OOF_VecDot3 (ps0->r.z, ps0->u.z, ps0->f.z, &ps1->u);
pd->f.z = OOF_VecDot3 (ps0->r.z, ps0->u.z, ps0->f.z, &ps1->f);
}

//------------------------------------------------------------------------------

void OOF_InitMinMax (tOOF_vector *pvMin, tOOF_vector *pvMax)
{
if (pvMin && pvMax) {
	pvMin->x =
	pvMin->y =
	pvMin->z = 1000000;
	pvMax->x =
	pvMax->y =
	pvMax->z = -1000000;
	}
}

//------------------------------------------------------------------------------

void OOF_GetMinMax (tOOF_vector *pv, tOOF_vector *pvMin, tOOF_vector *pvMax)
{
if (pvMin && pvMax) {
	if (pvMin->x > pv->x)
		pvMin->x = pv->x;
	if (pvMax->x < pv->x)
		pvMax->x = pv->x;
	if (pvMin->y > pv->y)
		pvMin->y = pv->y;
	if (pvMax->y < pv->y)
		pvMax->y = pv->y;
	if (pvMin->z > pv->z)
		pvMin->z = pv->z;
	if (pvMax->z < pv->z)
		pvMax->z = pv->z;
	}
}

//------------------------------------------------------------------------------

void OOF_SetModelProps (tOOF_subObject *pso,char *pszProps)
{
	// first, extract the command

	int l;
	char command [200], data [200], *psz;

if (3 > (l = (int) strlen (pszProps)))
	return;
if (psz = strchr (pszProps, '='))
	l = (int) (psz - pszProps + 1);
memcpy (command, pszProps, l);
command [l] = '\0';
if (psz)
	strcpy (data, psz + 1);
else
	*data = '\0';

// now act on the command/data pair

if (!stricmp (command,"$rotate=")) { // constant rotation for a subobject
	float spinrate = (float) atof( data);
	if ((spinrate <= 0) || (spinrate > 20))
		return;		// bad data
	pso->nFlags |= OOF_SOF_ROTATE;
	pso->fRPS = 1.0f / spinrate;
	return;
	}

if (!strnicmp (command,"$jitter",7)) {	// this subobject is a jittery tObject
	pso->nFlags |= OOF_SOF_JITTER;
	return;
	}

if (!strnicmp (command,"$shell",6)) { // this subobject is a door shell
	pso->nFlags |= OOF_SOF_SHELL;
	return;
	}

if (!strnicmp (command,"$facing",7)) { // this subobject always faces you
	pso->nFlags |= OOF_SOF_FACING;
	return;
	}

if (!strnicmp (command,"$frontface",10)) { // this subobject is a door front
	pso->nFlags |= OOF_SOF_FRONTFACE;
	return;
	}

if (!stricmp (command,"$glow=")) {
	float r,g,b;
	float size;
	int nValues;

	Assert (!(pso->nFlags & (OOF_SOF_GLOW | OOF_SOF_THRUSTER)));
	nValues = sscanf (data, " %f, %f, %f, %f", &r,&g,&b,&size);
	Assert (nValues == 4);
	pso->nFlags |= OOF_SOF_GLOW;
	//pso->glowInfo = (tOOF_glowInfo *) d_malloc (sizeof(tOOF_glowInfo));
	pso->glowInfo.color.r = r;
	pso->glowInfo.color.g = g;
	pso->glowInfo.color.b = b;
	pso->glowInfo.fSize = size;
	return;
	}

if (!stricmp (command,"$thruster=")) {
	float r,g,b;
	float size;
	int nValues;

	Assert (!(pso->nFlags & (OOF_SOF_GLOW | OOF_SOF_THRUSTER)));
	nValues = sscanf(data, " %f, %f, %f, %f", &r,&g,&b,&size);
	Assert(nValues == 4);
	pso->nFlags |= OOF_SOF_THRUSTER;
	//pso->glowInfo = (tOOF_glowInfo *) d_malloc (sizeof (tOOF_glowInfo));
	pso->glowInfo.color.r = r;
	pso->glowInfo.color.g = g;
	pso->glowInfo.color.b = b;
	pso->glowInfo.fSize = size;
	return;
	}

if (!stricmp (command,"$fov=")) {
	float fov_angle;
	float turret_spr;
	float reactionTime;
	int nValues;

	nValues = sscanf(data, " %f, %f, %f", &fov_angle, &turret_spr, &reactionTime);
	Assert(nValues == 3);
	if (fov_angle < 0.0f || fov_angle > 360.0f) { // Bad data
		Assert(0);
		fov_angle = 1.0;
		}
	// .4 is really fast and really arbitrary
	if (turret_spr < 0.0f || turret_spr > 60.0f) { // Bad data
		Assert(0);
		turret_spr = 1.0f;
		}
	// 10 seconds is really slow and really arbitrary
	if (reactionTime < 0.0f || reactionTime > 10.0f) { // Bad data
		Assert(0);
		reactionTime = 10.0;
		}
	pso->nFlags |= OOF_SOF_TURRET;
	pso->fFOV = fov_angle/720.0f; // 720 = 360 * 2 and we want to make fov the amount we can move in either direction
	                             // it has a minimum value of (0.0) to [0.5]
	pso->fRPS = 1.0f / turret_spr;  // convert spr to rps (rotations per second)
	pso->fUpdate = reactionTime;
	return;
	}

if (!stricmp (command,"$monitor01")) { // this subobject is a monitor
	pso->nFlags |= OOF_SOF_MONITOR1;
	return;
	}

if (!stricmp (command,"$monitor02")) { // this subobject is a 2nd monitor
	pso->nFlags |= OOF_SOF_MONITOR2;
	return;
	}

if (!stricmp (command,"$monitor03")) { // this subobject is a 3rd monitor
	pso->nFlags |= OOF_SOF_MONITOR3;
	return;
	}

if (!stricmp (command,"$monitor04")) { // this subobject is a 4th monitor
	pso->nFlags |= OOF_SOF_MONITOR4;
	return;
	}

if (!stricmp (command,"$monitor05")) { // this subobject is a 4th monitor
	pso->nFlags |= OOF_SOF_MONITOR5;
	return;
	}

if (!stricmp (command,"$monitor06")) { // this subobject is a 4th monitor
	pso->nFlags |= OOF_SOF_MONITOR6;
	return;
	}

if (!stricmp (command,"$monitor07")) { // this subobject is a 4th monitor
	pso->nFlags |= OOF_SOF_MONITOR7;
	return;
	}

if (!stricmp (command,"$monitor08")) { // this subobject is a 4th monitor
	pso->nFlags |= OOF_SOF_MONITOR8;
	return;
	}

if (!stricmp (command,"$viewer")) { // this subobject is a viewer
	pso->nFlags |= OOF_SOF_VIEWER;
	return;
	}

if (!stricmp (command,"$layer")) { // this subobject is a layer to be drawn after original tObject.
	pso->nFlags |= OOF_SOF_LAYER;
	return;
	}

if (!stricmp (command,"$custom")) { // this subobject has custom textures/colors
	pso->nFlags |= OOF_SOF_CUSTOM;
	return;
	}
}

//------------------------------------------------------------------------------

static tOOF_vector *OOF_ReadVertList (CFILE *fp, int nVerts, tOOF_vector *pvMin, tOOF_vector *pvMax)
{
	tOOF_vector	*pv;
	char			szId [20] = "";

OOF_InitMinMax (pvMin, pvMax);
if (pv = (tOOF_vector *) d_malloc (nVerts * sizeof (tOOF_vector))) {
	int	i;

	for (i = 0; i < nVerts; i++) {
		if (bLogOOF)
			sprintf (szId, "pv [%d]", i);
		OOF_ReadVector (fp, pv + i, szId);
		OOF_GetMinMax (pv + i, pvMin, pvMax);
		}
	}
return pv;
}

//------------------------------------------------------------------------------

int OOF_ReadFrameInfo (CFILE *fp, tOOFObject *po, tOOF_frameInfo *pfi, int bTimed)
{
nIndent += 2;
OOF_PrintLog ("reading frame info\n");
if (bTimed) {
	pfi->nFrames = OOF_ReadInt (fp, "nFrames");
	pfi->nFirstFrame = OOF_ReadInt (fp, "nFirstFrame");
	pfi->nLastFrame = OOF_ReadInt (fp, "nLastFrame");
	if (po->frameInfo.nFirstFrame > pfi->nFirstFrame)
		po->frameInfo.nFirstFrame = pfi->nFirstFrame;
	if (po->frameInfo.nLastFrame < pfi->nLastFrame)
		po->frameInfo.nLastFrame = pfi->nLastFrame;
	}
else
	pfi->nFrames = po->frameInfo.nFrames;
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReadRotFrame (CFILE *fp, tOOF_rotFrame *pf, int bTimed)
{
	float	fMag;

nIndent += 2;
OOF_PrintLog ("reading rot frame\n");
if (bTimed)
	pf->nStartTime = OOF_ReadInt (fp, "nStartTime");
OOF_ReadVector (fp, &pf->vAxis, "vAxis");
if (0 < (fMag = OOF_VecMag (&pf->vAxis)))
	OOF_VecScale (&pf->vAxis, 1.0f / fMag);
pf->nAngle = OOF_ReadInt (fp, "nAngle");
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeRotAnim (tOOF_rotAnim *pa)
{
oof_free (pa->pFrames);
oof_free (pa->pRemapTicks);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadRotAnim (CFILE *fp, tOOFObject *po, tOOF_rotAnim *pa, int bTimed)
{
	tOOF_rotAnim a;
	int	i;

memset (&a, 0, sizeof (a));
OOF_ReadFrameInfo (fp, po, &a.frameInfo, bTimed);
if (!(a.pFrames = (tOOF_rotFrame *) d_malloc (a.frameInfo.nFrames * sizeof (tOOF_rotFrame))))
	return 0;
if (bTimed &&
	 (a.nTicks = a.frameInfo.nLastFrame - a.frameInfo.nFirstFrame) &&
	 !(a.pRemapTicks = (ubyte *) d_malloc (a.nTicks * sizeof (ubyte))))
	return OOF_FreeRotAnim (&a);
if (a.nTicks)
for (i = 0; i < a.frameInfo.nFrames; i++)
	if (!OOF_ReadRotFrame (fp, a.pFrames + i, bTimed))
		return OOF_FreeRotAnim (&a);
*pa = a;
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReadPosFrame (CFILE *fp, tOOF_posFrame *pf, int bTimed)
{
nIndent += 2;
OOF_PrintLog ("reading pos frame\n");
if (bTimed)
	pf->nStartTime = OOF_ReadInt (fp, "nStartTime");
OOF_ReadVector (fp, &pf->vPos, "vPos");
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreePosAnim (tOOF_posAnim *pa)
{
oof_free (pa->pFrames);
oof_free (pa->pRemapTicks);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadPosAnim (CFILE *fp, tOOFObject *po, tOOF_posAnim *pa, int bTimed)
{
	tOOF_posAnim a;
	int	i;

memset (&a, 0, sizeof (a));
OOF_ReadFrameInfo (fp, po, &a.frameInfo, bTimed);
if (bTimed &&
	 (a.nTicks = a.frameInfo.nLastFrame - a.frameInfo.nFirstFrame) &&
	 !(a.pRemapTicks = (ubyte *) d_malloc (a.nTicks * sizeof (ubyte))))
	return OOF_FreePosAnim (&a);
if (!(a.pFrames = (tOOF_posFrame *) d_malloc (a.frameInfo.nFrames * sizeof (tOOF_posFrame))))
	return OOF_FreePosAnim (pa);
for (i = 0; i < a.frameInfo.nFrames; i++)
	if (!OOF_ReadPosFrame (fp, a.pFrames + i, bTimed))
		return OOF_FreePosAnim (pa);
*pa = a;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeSpecialPoint (tOOF_specialPoint *pVert)
{
oof_free (pVert->pszName);
oof_free (pVert->pszProps);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadSpecialPoint (CFILE *fp, tOOF_specialPoint *pVert)
{
	memset (pVert, 0, sizeof (tOOF_specialPoint));

nIndent += 2;
OOF_PrintLog ("reading special point\n");
if (!(pVert->pszName = OOF_ReadString (fp, "pszName"))) {
	nIndent -= 2;
	return 0;
	}
if (!(pVert->pszProps = OOF_ReadString (fp, "pszProps"))) {
	nIndent -= 2;
	return 0;
	}
OOF_ReadVector (fp, &pVert->vPos, "vPos");
pVert->fRadius = OOF_ReadFloat (fp, "fRadius");
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeSpecialList (tOOF_specialList *pList)
{
	int	i;

if (pList->pVerts) {
	for (i = 0; i < pList->nVerts; i++)
		OOF_FreeSpecialPoint (pList->pVerts + i);
	oof_free (pList->pVerts);
	}
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadSpecialList (CFILE *fp, tOOF_specialList *pList)
{
	int	i;

pList->nVerts = OOF_ReadInt (fp, "nVerts");
if (!pList->nVerts)
	return 1;
if (!(pList->pVerts = (tOOF_specialPoint *) d_malloc (pList->nVerts * sizeof (tOOF_specialPoint))))
	return 0;
for (i = 0; i < pList->nVerts; i++)
	OOF_ReadSpecialPoint (fp, pList->pVerts + i);
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReadPoint (CFILE *fp, tOOF_point *pPoint, int bParent)
{
nIndent += 2;
OOF_PrintLog ("reading point\n");
pPoint->nParent = bParent ? OOF_ReadInt (fp, "nParent") : 0;
OOF_ReadVector (fp, &pPoint->vPos, "vPos");
OOF_ReadVector (fp, &pPoint->vDir, "vDir");
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreePointList (tOOF_pointList *pList)
{
oof_free (pList->pPoints);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadPointList (CFILE *fp, tOOF_pointList *pList, int bParent)
{
	int	i;

nIndent += 2;
OOF_PrintLog ("reading point list\n");
pList->nPoints = OOF_ReadInt (fp, "nPoints");
if (!(pList->pPoints = (tOOF_point *) d_malloc (pList->nPoints * sizeof (tOOF_point)))) {
	nIndent -= 2;
	return OOF_FreePointList (pList);
	}
for (i = 0; i < pList->nPoints; i++)
	if (!OOF_ReadPoint (fp, pList->pPoints + i, bParent)) {
		nIndent -= 2;
		return 0;
		}
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeAttachList (tOOF_attachList *pList)
{
oof_free (pList->pPoints);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadAttachList (CFILE *fp, tOOF_attachList *pList)
{
	int	i;

nIndent += 2;
OOF_PrintLog ("reading attach list\n");
pList->nPoints = OOF_ReadInt (fp, "nPoints");
if (!(pList->pPoints = (tOOF_attachPoint *) d_malloc (pList->nPoints * sizeof (tOOF_attachPoint)))) {
	nIndent -= 2;
	return OOF_FreeAttachList (pList);
	}
for (i = 0; i < pList->nPoints; i++)
	if (!OOF_ReadPoint (fp, &pList->pPoints [i].point, 1)) {
		nIndent -= 2;
		return OOF_FreeAttachList (pList);
		}
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReadAttachNormals (CFILE *fp, tOOF_attachList *pList)
{
	int	i;
	tOOF_attachPoint *pp = pList->pPoints;

nIndent += 2;
OOF_PrintLog ("reading attach normals\n");
i = OOF_ReadInt (fp, "nPoints");
if (i != pList->nPoints) {
	nIndent -= 2;
	return 0;
	}
for (i = 0; i < pList->nPoints; i++) {
	OOF_ReadVector (fp, &pp->vu, "vu");	//actually ignored
	OOF_ReadVector (fp, &pp->vu, "vu");
	pList->pPoints->bu = 1;
	}
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

static int OOF_ReadIntList (CFILE *fp, int **ppList)
{
	int	*pList, nList, i;
	char	szId [20] = "";

if (!(nList = OOF_ReadInt (fp, "nList"))) {
	*ppList = NULL;
	return 0;
	}
if (!(pList = (int *) d_malloc (nList * sizeof (int))))
	return -1;
for (i = 0; i < nList; i++) {
	if (bLogOOF)
		sprintf (szId, "pList [%d]", i);
	pList [i] = OOF_ReadInt (fp, szId);
	}
*ppList = pList;
return nList;
}

//------------------------------------------------------------------------------

int OOF_FreeBattery (tOOF_battery *pBatt)
{
oof_free (pBatt->pVertIndex);
oof_free (pBatt->pTurretIndex);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadBattery (CFILE *fp, tOOF_battery *pBatt)
{
nIndent += 2;
OOF_PrintLog ("reading battery\n");
if (0 > (pBatt->nVerts = OOF_ReadIntList (fp, &pBatt->pVertIndex))) {
	nIndent -= 2;
	return OOF_FreeBattery (pBatt);
	}
if (0 > (pBatt->nTurrets = OOF_ReadIntList (fp, &pBatt->pTurretIndex))) {
	nIndent -= 2;
	return OOF_FreeBattery (pBatt);
	}
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeArmament (tOOF_armament *pa)
{
if (pa->pBatts) {
	int	i;

	for (i = 0; i < pa->nBatts; i++)
		OOF_FreeBattery (pa->pBatts + i);
	oof_free (pa->pBatts);
	}
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadArmament (CFILE *fp, tOOF_armament *pa)
{
	int	i;

nIndent += 2;
OOF_PrintLog ("reading armament\n");
if (!(pa->nBatts = OOF_ReadInt (fp, "nBatts"))) {
	nIndent -= 2;
	return 1;
	}
if (!(pa->pBatts = (tOOF_battery *) d_malloc (pa->nBatts * sizeof (tOOF_battery)))) {
	nIndent -= 2;
	return OOF_FreeArmament (pa);
	}
for (i = 0; i < pa->nBatts; i++)
	if (!OOF_ReadBattery (fp, pa->pBatts + i)) {
		nIndent -= 2;
		return OOF_FreeArmament (pa);
		}
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReadFaceVert (CFILE *fp, tOOF_faceVert *pfv)
{
nIndent += 2;
OOF_PrintLog ("reading face vertex\n");
pfv->nIndex = OOF_ReadInt (fp, "nIndex");
pfv->fu = OOF_ReadFloat (fp, "fu");
pfv->fv = -OOF_ReadFloat (fp, "fv");
#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/if (pfv->fu == 0.5) pfv->fu = 1.0;
/*!!!*/if (pfv->fv == 0.5) pfv->fv = 1.0;
/*!!!*/if (pfv->fu == -0.5) pfv->fu = 1.0;
/*!!!*/if (pfv->fv == -0.5) pfv->fv = 1.0;
#endif
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeFace (tOOF_face *pf)
{
#if !OOF_MEM_OPT
oof_free (pf->pVerts);
#endif
return 0;
}

//------------------------------------------------------------------------------

int OOF_FindVertex (tOOF_subObject *pso, int i)
{
	tOOF_vector	v, *pv;
	int			j;

pv = pso->pvVerts;
v = pv [i];

for (j = 0; i < i; j++, pv++)
	if ((v.x == pv->x) && (v.y == pv->y) && (v.z == pv->z))
		return j;
return i;
}

//------------------------------------------------------------------------------

#define MAXGAP	1.0f;

int OOF_FindEdge (tOOF_subObject *pso, int v0, int v1)
{
	int			i;
	tOOF_edge	h;

#ifdef _DEBUG
v0 = OOF_FindVertex (pso, v0);
v1 = OOF_FindVertex (pso, v1);
#endif
for (i = 0; i < pso->edges.nEdges; i++) {
	h = pso->edges.pEdges [i];
#if 1
	if (((h.v0 == v0) && (h.v1 == v1)) || ((h.v0 == v1) && (h.v1 == v0)))
#else
	if (((fabs (h.v0 - v0) < MAXGAP) && (fabs (h.v1 - v1) < MAXGAP)) || 
		 ((fabs (h.v0 - v1) < MAXGAP) && (fabs (h.v1 - v0) < MAXGAP)))
#endif
		return i;
	}
return -1;
}

//------------------------------------------------------------------------------

int OOF_AddEdge (tOOF_subObject *pso, tOOF_face *pf, int v0, int v1)
{
	int			i = OOF_FindEdge (pso, v0, v1);
	tOOF_edge	*pe;

if (pso->nFlags & (OOF_SOF_GLOW | OOF_SOF_THRUSTER))
	return -1;
if (i < 0)
	i = pso->edges.nEdges++;
pe = pso->edges.pEdges + i;
if (i < 0) {
	}
if (pe->pf0)
	pe->pf1 = pf;
else {
	pe->pf0 = pf;
	pe->v0 = v0;
	pe->v1 = v1;
	}
return i;
}

//------------------------------------------------------------------------------

inline tOOF_vector *OOF_CalcFaceCenter (tOOF_subObject *pso, tOOF_face *pf)
{
	tOOF_faceVert	*pfv = pf->pVerts;
	tOOF_vector		vc, *pv = pso->pvRotVerts;
	int				i;

vc.x = vc.y = vc.z = 1.0f;
for (i = pf->nVerts; i; i--, pfv++)
	OOF_VecInc (&vc, pv + pfv->nIndex);
OOF_VecScale (&vc, 1.0f / (float) pf->nVerts);
pf->vCenter = vc;
return &pf->vCenter;
}

//------------------------------------------------------------------------------

inline tOOF_vector *OOF_CalcFaceNormal (tOOF_subObject *pso, tOOF_face *pf)
{
	tOOF_vector		*pv = pso->pvRotVerts;
	tOOF_faceVert	*pfv = pf->pVerts;

return OOF_VecNormal (&pf->vRotNormal, pv + pfv [0].nIndex, pv + pfv [1].nIndex, pv + pfv [2].nIndex);
}

//------------------------------------------------------------------------------

#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/static int nTexId = 0;
#endif

int OOF_ReadFace (CFILE *fp, tOOF_subObject *pso, tOOF_face *pf, tOOF_faceVert *pfv)
{
	tOOF_face	f;
	int			i, v0;
	tOOF_edge	e;

nIndent += 2;
OOF_PrintLog ("reading face\n");
memset (&f, 0, sizeof (f));
OOF_ReadVector (fp, &f.vNormal, "vNormal");
f.nVerts = OOF_ReadInt (fp, "nVerts");
f.bTextured = OOF_ReadInt (fp, "bTextured");
if (f.bTextured) {
	f.texProps.nTexId = OOF_ReadInt (fp, "texProps.nTexId");
#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/	f.texProps.nTexId = nTexId % 6;
/*!!!*/	nTexId++;
#endif
	}
else {
	f.texProps.color.r = OOF_ReadByte (fp, "texProps.color.r");
	f.texProps.color.g = OOF_ReadByte (fp, "texProps.color.g");
	f.texProps.color.b = OOF_ReadByte (fp, "texProps.color.b");
	}
#if OOF_MEM_OPT
if (pfv) {
	f.pVerts = pfv;
#else
	if (!(f.pVerts = (tOOF_faceVert *) d_malloc (f.nVerts * sizeof (tOOF_faceVert)))) {
		nIndent -= 2;
		return OOF_FreeFace (&f);
		}	
#endif
	OOF_InitMinMax (&f.vMin, &f.vMax);
	e.v1 = -1;
	for (i = 0; i < f.nVerts; i++)
		if (!OOF_ReadFaceVert (fp, f.pVerts + i)) {
			nIndent -= 2;
			return OOF_FreeFace (&f);
			}
		else {
			e.v0 = e.v1;
			e.v1 = f.pVerts [i].nIndex;
			OOF_GetMinMax (pso->pvVerts + e.v1, &f.vMin, &f.vMax);
			if (i)
				OOF_AddEdge (pso, pf, e.v0, e.v1);
			else
				v0 = e.v1;
			}
	OOF_AddEdge (pso, pf, e.v1, v0);
	//OOF_CalcFaceNormal (pso, &f);
	OOF_CalcFaceCenter (pso, &f);
#if OOF_MEM_OPT
	}
else
	CFSeek (fp, f.nVerts * sizeof (tOOF_faceVert), SEEK_CUR);
#endif
f.fBoundingLength = OOF_ReadFloat (fp, "fBoundingLength");
f.fBoundingWidth = OOF_ReadFloat (fp, "fBoundingWidth");
*pf = f;
nIndent -= 2;
return f.nVerts;
}

//------------------------------------------------------------------------------

int OOF_FreeSubObject (tOOF_subObject *pso)
{
#if !OOF_MEM_OPT
	int	i;
#endif

oof_free (pso->pszName);
oof_free (pso->pszProps);
oof_free (pso->pvVerts);
oof_free (pso->pvRotVerts);
oof_free (pso->pVertColors);
oof_free (pso->pvNormals);
oof_free (pso->pfAlpha);
OOF_FreePosAnim (&pso->posAnim);
OOF_FreeRotAnim (&pso->rotAnim);
if (pso->faces.pFaces) {
#if OOF_MEM_OPT
	oof_free (pso->faces.pFaceVerts);
#else
	for (i = 0; i < pso->faces.nFaces; i++)
		OOF_FreeFace (pso->faces.pFaces + i);
#endif
	oof_free (pso->faces.pFaces);
	}
oof_free (pso->edges.pEdges);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadSubObject (CFILE *fp, tOOFObject *po)
{
	tOOF_subObject	so;
	tOOF_faceVert	*pfv = NULL;
	int				h, i;
#if OOF_MEM_OPT
	int				bReadData, nPos, nFaceVerts;
#endif
	char				szId [20] = "";

nIndent += 2;
OOF_PrintLog ("reading sub tObject\n");
memset (&so, 0, sizeof (so));
so.nIndex = OOF_ReadInt (fp, "nIndex");
if (so.nIndex >= OOF_MAX_SUBOBJECTS) {
	nIndent -= 2;
	return 0;
	}
so.nParent = OOF_ReadInt (fp, "nParent");
OOF_ReadVector (fp, &so.vNormal, "vNormal");
so.fd = OOF_ReadFloat (fp, "fd");
OOF_ReadVector (fp, &so.vPlaneVert, "vPlaneVert");
OOF_ReadVector (fp, &so.vOffset, "vOffset");
so.fRadius = OOF_ReadFloat (fp, "fRadius");
so.nTreeOffset = OOF_ReadInt (fp, "nTreeOffset");
so.nDataOffset = OOF_ReadInt (fp, "nDataOffset");
if (po->nVersion > 1805)
	OOF_ReadVector (fp, &so.vCenter, "vCenter");
if (!(so.pszName = OOF_ReadString (fp, "pszName")))
	return OOF_FreeSubObject (&so);
if (!(so.pszProps = OOF_ReadString (fp, "pszProps")))
	return OOF_FreeSubObject (&so);
OOF_SetModelProps (&so, so.pszProps);
so.nMovementType = OOF_ReadInt (fp, "nMovementType");
so.nMovementAxis = OOF_ReadInt (fp, "nMovementAxis");
so.pFSList = NULL;
if (so.nFSLists = OOF_ReadInt (fp, "nFSLists"))
	CFSeek (fp, so.nFSLists * sizeof (int), SEEK_CUR);
so.nVerts = OOF_ReadInt (fp, "nVerts");
if (so.nVerts) {
	if (!(so.pvVerts = OOF_ReadVertList (fp, so.nVerts, &so.vMin, &so.vMax))) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		OOF_VecAdd (&so.vCenter, &so.vMin, &so.vMax);
		OOF_VecScale (&so.vCenter, 0.5f);
		}
	if (!(so.pvRotVerts = (tOOF_vector *) d_malloc (so.nVerts * sizeof (tOOF_vector)))) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		}
	if (!(so.pVertColors = (tFaceColor *) d_malloc (so.nVerts * sizeof (tFaceColor)))) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		}
	memset (so.pVertColors, 0, so.nVerts * sizeof (tFaceColor));
	if (!(so.pvNormals = OOF_ReadVertList (fp, so.nVerts, NULL, NULL))) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		}
	if (!(so.pfAlpha = (float *) d_malloc (so.nVerts * sizeof (float)))) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		}
	for (i = 0; i < so.nVerts; i++)
		if (po->nVersion < 2300) 
			so.pfAlpha [i] = 1.0f;
		else {
			if (bLogOOF)
				sprintf (szId, "pfAlpha [%d]", i);
			so.pfAlpha [i] = OOF_ReadFloat (fp, szId);
			if	(so.pfAlpha [i] < 0.99)
				po->nFlags |= OOF_PMF_ALPHA;
			}
	}
so.faces.nFaces = OOF_ReadInt (fp, "nFaces");
if (!(so.faces.pFaces = (tOOF_face *) d_malloc (so.faces.nFaces * sizeof (tOOF_face)))) {
	nIndent -= 2;
	return OOF_FreeSubObject (&so);
	}
#if OOF_MEM_OPT
nPos = CFTell (fp);
so.edges.nEdges = 0;
for (bReadData = 0; bReadData < 2; bReadData++) {
	CFSeek (fp, nPos, SEEK_SET);
	if (bReadData) {
		if (!(so.faces.pFaceVerts = d_malloc (nFaceVerts * sizeof (tOOF_faceVert)))) {
			nIndent -= 2;
			return OOF_FreeSubObject (&so);
			}
		if (!(so.edges.pEdges = (tOOF_edge *) d_malloc (nFaceVerts * sizeof (tOOF_edge)))) {
			nIndent -= 2;
			return OOF_FreeSubObject (&so);
			}
		memset (so.edges.pEdges, 0, nFaceVerts * sizeof (tOOF_edge));
		so.edges.nEdges = 0;
		}
	for (i = 0, nFaceVerts = 0; i < so.faces.nFaces; i++) {
		if (!(h = OOF_ReadFace (fp, &so, so.faces.pFaces + i, bReadData ? so.faces.pFaceVerts + nFaceVerts : NULL))) {
			nIndent -= 2;
			return OOF_FreeSubObject (&so);
			}
		nFaceVerts += h;
		}
	}
#else
for (i = 0; i < so.faces.nFaces; i++)
	if (!OOF_ReadFace (fp, &so, so.faces.pFaces + i, NULL)) {
		nIndent -= 2;
		return OOF_FreeSubObject (&so);
		}
#endif
po->pSubObjects [so.nIndex] = so;
nIndent -= 2;
return 1;
}

//------------------------------------------------------------------------------

int OOF_ReleaseTextures (void)
{
	tOOFObject *po;
	int			i, j;

for (i = gameData.models.nHiresModels, po = gameData.models.hiresModels; i; i--, po++)
	if (po->textures.pBitmaps)
		for (j = 0; j < po->textures.nTextures; j++)
			GrFreeBitmapData (po->textures.pBitmaps + j);
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReloadTextures (void)
{
	tOOFObject *po;
	int			i, j;

for (i = gameData.models.nHiresModels, po = gameData.models.hiresModels; i; i--, po++)
	if (po->textures.pszNames && po->textures.pBitmaps)
		for (j = 0; j < po->textures.nTextures; j++)
			if (!ReadTGA (po->textures.pszNames [j], gameFolders.szModelDir, po->textures.pBitmaps + j, -1, 1.0, 0, 0))
				OOF_FreeObject (po);
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeTextures (tOOFObject *po)
{
	int	i;

if (po->textures.pszNames) {
	for (i = 0; i < po->textures.nTextures; i++) {
		oof_free (po->textures.pszNames [i]);
		if (po->textures.pBitmaps)
			GrFreeBitmapData (po->textures.pBitmaps + i);
		}
	oof_free (po->textures.pszNames);
	oof_free (po->textures.pBitmaps);
	}
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadTextures (CFILE *fp, tOOFObject *po)
{
	tOOFObject	o = *po;
	int			i;
	char			szId [30];

nIndent += 2;
OOF_PrintLog ("reading textures\n");
o.textures.nTextures = OOF_ReadInt (fp, "nTextures");
#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/o.textures.nTextures = 6;
#endif
if (!(o.textures.pszNames = (char **) d_malloc (o.textures.nTextures * sizeof (char **)))) {
	nIndent -= 2;
	return OOF_FreeTextures (&o);
	}
memset (o.textures.pszNames, 0, o.textures.nTextures * sizeof (char **));
i = o.textures.nTextures * sizeof (grsBitmap);
if (!(o.textures.pBitmaps = (grsBitmap *) d_malloc (i))) {
	nIndent -= 2;
	return OOF_FreeTextures (&o);
	}
memset (o.textures.pBitmaps, 0, i);
for (i = 0; i < o.textures.nTextures; i++) {
	if (bLogOOF)
		sprintf (szId, "textures.pszId [%d]", i);
#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/if (!i) {
#endif
	if (!(o.textures.pszNames [i] = OOF_ReadString (fp, szId))) {
		nIndent -= 2;
		return OOF_FreeTextures (&o);
		}
#if 0 && defined(_DEBUG) && SHADOWS
/*!!!*/}else o.textures.pszNames [i] = d_malloc (20);
/*!!!*/sprintf (o.textures.pszNames [i], "%d.tga", i + 1);
#endif
	if (!ReadTGA (o.textures.pszNames [i], gameFolders.szModelDir, o.textures.pBitmaps + i, -1, 1.0, 0, 0)) {
		nIndent -= 2;
		return OOF_FreeTextures (&o);
		}
	}
*po = o;
return 1;
}

//------------------------------------------------------------------------------

int OOF_FreeObject (tOOFObject *po)
{
	int	i;

OOF_FreeTextures (po);
if (po->pSubObjects) {
	for (i = 0; i < po->nSubObjects; i++)
		OOF_FreeSubObject (po->pSubObjects + i);
	oof_free (po->pSubObjects);
	}
OOF_FreePointList (&po->gunPoints);
OOF_FreeAttachList (&po->attachPoints);
OOF_FreeSpecialList (&po->specialPoints);
if (po->armament.pBatts) {
	for (i = 0; i < po->armament.nBatts; i++)
		OOF_FreeBattery (po->armament.pBatts + i);
	oof_free (po->armament.pBatts);
	}
return 0;
}

//------------------------------------------------------------------------------

int OOF_ReadObject (CFILE *fp, tOOFObject *po)
{
	tOOFObject	o = *po;

nIndent += 2;
OOF_PrintLog ("reading tObject\n");
o.nVersion = po->nVersion;
o.nSubObjects = OOF_ReadInt (fp, "nSubObjects");
if (o.nSubObjects >= OOF_MAX_SUBOBJECTS) {
	nIndent -= 2;
	return 0;
	}
o.fMaxRadius = OOF_ReadFloat (fp, "fMaxRadius");
OOF_ReadVector (fp, &o.vMin, "vMin");
OOF_ReadVector (fp, &o.vMax, "vMax");
o.nDetailLevels = OOF_ReadInt (fp, "nDetailLevels");
nIndent -= 2;
CFSeek (fp, o.nDetailLevels * sizeof (int), SEEK_CUR);
if (!(o.pSubObjects = (tOOF_subObject *) d_malloc (o.nSubObjects * sizeof (tOOF_subObject))))
	return 0;
*po = o;
return 1;
}

//------------------------------------------------------------------------------

void BuildModelAngleMatrix (tOOF_matrix *pm, int a, tOOF_vector *pAxis)
{
float x = pAxis->x;
float y = pAxis->y;
float z = pAxis->z;
float s = (float) sin (a);
float c = (float) cos (a);
float t = 1.0f - c;
float i = t * x;
float j = s * z;
//pm->r.x = t * x * x + c;
pm->r.x = i * x + c;
i *= y;
//pm->r.y = t * x * y + s * z;
//pm->u.x = t * x * y - s * z;
pm->r.y = i + j;
pm->u.x = i - j;
i = t * z;
//pm->f.z = t * z * z + c;
pm->f.z = i * z + c;
i *= x;
j = s * y;
//pm->r.z = t * x * z - s * y;
//pm->f.x = t * x * z + s * y;
pm->r.z = i - j;
pm->f.x = i + j;
i = t * y;
//pm->u.y = t * y * y + c;
pm->u.y = i * y + c;
i *= z;
j = s * x;
//pm->u.z = t * y * z + s * x;		
//pm->f.y = t * y * z - s * x;
pm->u.z = i + j;
pm->f.y = i - j;
}

//------------------------------------------------------------------------------

void BuildAnimMatrices (tOOFObject *po)
{
	tOOF_subObject *pso;
	tOOF_matrix		mBase, mDest, mTemp;
	tOOF_rotFrame	*pf;
	int				i, j, a;

for (i = po->nSubObjects, pso = po->pSubObjects; i; i--, pso++) {
	OOF_MatIdentity (&mBase);
	for (j = pso->rotAnim.frameInfo.nFrames, pf = pso->rotAnim.pFrames; j; j--, pf++) {
		a = pf->nAngle;
		BuildModelAngleMatrix (&mTemp, a, &pf->vAxis);
		OOF_MatIdentity (&mDest);
		OOF_MatMul (&mDest, &mTemp, &mBase);
		mBase = mDest;
		pf->mMat = mBase;
		}
	}
}

//------------------------------------------------------------------------------

void AssignChildren (tOOFObject *po)
{
	tOOF_subObject *pso, *pParent;
	int				i;

for (i = 0, pso = po->pSubObjects; i < po->nSubObjects; i++, pso++) {
	int nParent = pso->nParent;
	if (nParent == i)
		pso->nParent = -1;
	else if (nParent != -1) {
		pParent = po->pSubObjects + nParent;
		pParent->children [pParent->nChildren++] = i;
		}
	}
}

//------------------------------------------------------------------------------

inline void RecursiveAssignBatt (tOOFObject *po, int iObject, int iBatt)
{
	tOOF_subObject	*pso = po->pSubObjects + iObject;
	int				i, nFlags = iBatt << OOF_WB_INDEX_SHIFT;
	
pso->nFlags |= nFlags | OOF_SOF_WB;		
for (i = 0; i < pso->nChildren; i++)
	RecursiveAssignBatt (po, pso->children [i], iBatt);
}

//------------------------------------------------------------------------------

void AssignBatteries (tOOFObject *po)
{
	tOOF_subObject	*pso;
	tOOF_battery	*pb;
	int				*pti;
	int				i, j, k;

for (i = 0, pso = po->pSubObjects; i < po->nSubObjects; i++, pso++) {
	if (!(pso->nFlags & OOF_SOF_TURRET))
		continue;
	for (j = 0, pb = po->armament.pBatts; j < po->armament.nBatts; j++, pb++)
		for (k = pb->nTurrets, pti = pb->pTurretIndex; k; k--, pti++)
			if (*pti == i) {
				RecursiveAssignBatt (po, i, j);
				j = po->armament.nBatts;
				break;
				}
	}
}

//------------------------------------------------------------------------------

void BuildPosTickRemapList (tOOFObject *po)
{
	int				i, j, k, t, nTicks;
	tOOF_subObject	*pso;

for (i = po->nSubObjects, pso = po->pSubObjects; i; i--, pso++) {
	nTicks = pso->posAnim.frameInfo.nLastFrame - pso->posAnim.frameInfo.nFirstFrame;
	for (j = 0, t = pso->posAnim.frameInfo.nFirstFrame; j < nTicks; j++, t++)
		if (k = pso->posAnim.frameInfo.nFrames - 1)
			for (; k >= 0; k--)
				if (t >= pso->posAnim.pFrames [k].nStartTime) {
					pso->posAnim.pRemapTicks [j] = k;
					break;
					}
	}
}

//------------------------------------------------------------------------------

void BuildRotTickRemapList (tOOFObject *po)
{
	int				i, j, k, t, nTicks;
	tOOF_subObject	*pso;

for (i = po->nSubObjects, pso = po->pSubObjects; i; i--, pso++) {
	nTicks = pso->rotAnim.frameInfo.nLastFrame - pso->rotAnim.frameInfo.nFirstFrame;
	for (j = 0, t = pso->rotAnim.frameInfo.nFirstFrame; j < nTicks; j++, t++)
		if (k = pso->rotAnim.frameInfo.nFrames - 1)
			for (; k >= 0; k--)
				if (t >= pso->rotAnim.pFrames [k].nStartTime) {
					pso->rotAnim.pRemapTicks [j] = k;
					break;
					}
	}
}

//------------------------------------------------------------------------------

void ConfigureSubObjects (tOOFObject *po)
{
	int				i, j;
	tOOF_subObject	*pso;

for (i = po->nSubObjects, pso = po->pSubObjects; i; i--, pso++) {
	if (!pso->rotAnim.frameInfo.nFrames)
		pso->nFlags &= ~(OOF_SOF_ROTATE | OOF_SOF_TURRET);

	if (pso->nFlags & OOF_SOF_FACING) {
		tOOF_vector v [30], avg;
		tOOF_face	*pf = pso->faces.pFaces;

		for (j = 0; j < pf->nVerts; j++)
			v [j] = pso->pvVerts [pf->pVerts [j].nIndex];
		
		pso->fRadius = (float) (sqrt (OOF_Centroid (&avg, v, pf->nVerts)) / 2);
		po->nFlags |= OOF_PMF_FACING;

		}

	if (pso->nFlags & (OOF_SOF_GLOW | OOF_SOF_THRUSTER)) {
		tOOF_vector v [30];
		tOOF_face	*pf = pso->faces.pFaces;

		for (j = 0; j < pf->nVerts; j++)
			v [j] = pso->pvVerts [pf->pVerts [j].nIndex];
		OOF_VecNormal (&pso->glowInfo.vNormal, v, v + 1, v + 2);
		po->nFlags |= OOF_PMF_FACING;	// Set this so we know when to draw
		}
	}
}

//------------------------------------------------------------------------------

void GetSubObjectBounds (tOOFObject *po, tOOF_subObject *pso, tOOF_vector vo)
{
	int	i;

vo.x += pso->vOffset.x;
vo.y += pso->vOffset.y;
vo.z += pso->vOffset.z;
if ((po->vMax.x + vo.x) > pso->vMax.x)
	 pso->vMax.x = po->vMax.x + vo.x;
if ((po->vMax.y + vo.y) > pso->vMax.y)
	 pso->vMax.y = po->vMax.y + vo.y;
if ((po->vMax.z + vo.z) > pso->vMax.z)
	 pso->vMax.z = po->vMax.z + vo.z;
if ((po->vMin.x + vo.x) < pso->vMin.x)
	 pso->vMin.x = po->vMin.x + vo.x;
if ((po->vMin.y + vo.y) < pso->vMin.y)
	 pso->vMin.y = po->vMin.y + vo.y;
if ((po->vMin.z + vo.z) < pso->vMin.z)
	 pso->vMin.z = po->vMin.z + vo.z;
for (i = 0; i < pso->nChildren; i++)
	GetSubObjectBounds (po, po->pSubObjects + pso->children [i], vo);
}

//------------------------------------------------------------------------------

void GetObjectBounds (tOOFObject *po)
{
	tOOF_subObject	*pso;
	tOOF_vector		vo;
	int				i;

vo.x = vo.y = vo.z = 0.0f;
OOF_InitMinMax (&po->vMin, &po->vMax);
for (i = 0, pso = po->pSubObjects; i < po->nSubObjects; i++, pso++)
	if (pso->nParent == -1)
		GetSubObjectBounds (po, pso, vo);
}

//------------------------------------------------------------------------------

int OOF_ReadFile (char *pszFile, tOOFObject *po)
{
	CFILE				*fp;
	char				fileId [4];
	tOOFObject		o;
	int				i, nLength, nFrames, bTimed = 0;

bLogOOF = (fErr != NULL) && FindArg ("-printoof");
nIndent = 0;
OOF_PrintLog ("\nreading %s/%s\n", gameFolders.szModelDir, pszFile);
if (!(fp = CFOpen (pszFile, gameFolders.szModelDir, "rb", 0))) {
	OOF_PrintLog ("  file not found");
	return 0;
	}

if (!CFRead (fileId, sizeof (fileId), 1, fp)) {
	OOF_PrintLog ("  invalid file id\n");
	CFClose (fp);
	return 0;
	}
if (strncmp (fileId, "PSPO", 4)) {
	OOF_PrintLog ("  invalid file id\n");
	CFClose (fp);
	return 0;
	}
memset (&o, 0, sizeof (o));
o.nVersion = OOF_ReadInt (fp, "nVersion");
if (o.nVersion >= 2100)
	o.nFlags |= OOF_PMF_LIGHTMAP_RES;
if (o.nVersion >= 22) {
	bTimed = 1;
	o.nFlags |= OOF_PMF_TIMED;
	o.frameInfo.nFirstFrame = 0;
	o.frameInfo.nLastFrame = 0;
	}


while (!CFEoF (fp)) {
	unsigned char chunkId [4];
	
	if (!CFRead (chunkId, sizeof (chunkId), 1, fp)) {
		CFClose (fp);
		return 0;
		}
	OOF_PrintLog ("  chunkId = '%c%c%c%c'\n", chunkId [0], chunkId [1], chunkId [2], chunkId [3]);
	nLength = OOF_ReadInt (fp, "nLength");
	switch (ListType (chunkId)) {
		case 0:
			if (!OOF_ReadTextures (fp, &o))
				return OOF_FreeObject (&o);
			break;

		case 1:
			if (!OOF_ReadObject (fp, &o))
				return OOF_FreeObject (&o);
			break;

		case 2:
			if (!OOF_ReadSubObject (fp, &o))
				return OOF_FreeObject (&o);
			break;

		case 3:
			if (!OOF_ReadPointList (fp, &o.gunPoints, o.nVersion >= 1908))
				return OOF_FreeObject (&o);
			break;

		case 4:
			if (!OOF_ReadSpecialList (fp, &o.specialPoints))
				return OOF_FreeObject (&o);
			break;

		case 5:
			if (!OOF_ReadAttachList (fp, &o.attachPoints))
				return OOF_FreeObject (&o);
			break;

		case 6:
			nFrames = o.frameInfo.nFrames;
			if (!bTimed)
				o.frameInfo.nFrames = OOF_ReadInt (fp, "nFrames");
			for (i = 0; i < o.nSubObjects; i++)
				if (!OOF_ReadPosAnim (fp, &o, &o.pSubObjects [i].posAnim, bTimed))
				return OOF_FreeObject (&o);
			if (o.frameInfo.nFrames < nFrames)
				o.frameInfo.nFrames = nFrames;
			break;

		case 7:
		case 8:
			nFrames = o.frameInfo.nFrames;
			if (!bTimed)
				o.frameInfo.nFrames = OOF_ReadInt (fp, "nFrames");
			for (i = 0; i < o.nSubObjects; i++)
				if (!OOF_ReadRotAnim (fp, &o, &o.pSubObjects [i].rotAnim, bTimed))
				return OOF_FreeObject (&o);
			if (o.frameInfo.nFrames < nFrames)
				o.frameInfo.nFrames = nFrames;
			break;

		case 9:
			if (!OOF_ReadArmament (fp, &o.armament))
				return OOF_FreeObject (&o);
			break;

		case 10:
			if (!OOF_ReadAttachNormals (fp, &o.attachPoints))
				return OOF_FreeObject (&o);
			break;

		default:
			CFSeek (fp, nLength, SEEK_CUR);
			break;
		}
	}
CFClose (fp);
ConfigureSubObjects (&o);
BuildAnimMatrices (&o);
AssignChildren (&o);
AssignBatteries (&o);
BuildPosTickRemapList (po);
BuildRotTickRemapList (po);
*po = o;
gameData.models.bHaveHiresModel [po - gameData.models.hiresModels] = 1;
return 1;
}

//------------------------------------------------------------------------------

float *OOF_GlIdent (float *pm)
{
memset (pm, 0, sizeof (glMatrixf));
pm [0] = 1.0f;
pm [5] = 1.0f;
pm [10] = 1.0f;
pm [15] = 1.0f;
return pm;
}

//------------------------------------------------------------------------------

float *OOF_VecVms2Gl (float *pDest, vmsVector *pSrc)
{
pDest [0] = (float) pSrc->x / 65536.0f;
pDest [1] = (float) pSrc->y / 65536.0f;
pDest [2] = (float) pSrc->z / 65536.0f;
pDest [3] = 1.0f;
return pDest;
}

//------------------------------------------------------------------------------

float *OOF_MatVms2Gl (float *pDest, vmsMatrix *pSrc)
{
OOF_GlIdent (pDest);
pDest [0] = ((float) pSrc->rVec.x) / 65536.0f;
pDest [4] = ((float) pSrc->rVec.y) / 65536.0f;
pDest [8] = ((float) pSrc->rVec.z) / 65536.0f;
pDest [1] = ((float) pSrc->uVec.x) / 65536.0f;
pDest [5] = ((float) pSrc->uVec.y) / 65536.0f;
pDest [9] = ((float) pSrc->uVec.z) / 65536.0f;
pDest [2] = ((float) pSrc->fVec.x) / 65536.0f;
pDest [6] = ((float) pSrc->fVec.y) / 65536.0f;
pDest [10] = ((float) pSrc->fVec.z) / 65536.0f;
return pDest;
}

//------------------------------------------------------------------------------

float *OOF_VecVms2Oof (tOOF_vector *pDest, vmsVector *pSrc)
{
pDest->x = (float) pSrc->x / 65536.0f;
pDest->y = (float) pSrc->y / 65536.0f;
pDest->z = (float) pSrc->z / 65536.0f;
return (float *) pDest;
}

//------------------------------------------------------------------------------

float *OOF_MatVms2Oof (tOOF_matrix *pDest, vmsMatrix *pSrc)
{
OOF_VecVms2Oof (&pDest->f, &pSrc->fVec);
OOF_VecVms2Oof (&pDest->u, &pSrc->uVec);
OOF_VecVms2Oof (&pDest->r, &pSrc->rVec);
return (float *) pDest;
}

//------------------------------------------------------------------------------

float *OOF_GlInverse (float *pDest, float *pSrc)
{
	glMatrixf	m;
	int			x, y;

if (!pDest)
	pDest = m;
for (y = 0; y < 4; y++)
	for (x = 0; x < 4; x++)
		pDest [x * 4 + y] = pSrc [y * 4 + x];
if (pDest == m) {
	pDest = pSrc;
	memcpy (pDest, &m, sizeof (glMatrixf));
	}
return pDest;
}

//------------------------------------------------------------------------------

int OOF_FacingPoint (tOOF_vector *pv, tOOF_vector *pn, tOOF_vector *pp)
{
	tOOF_vector	v;

return OOF_VecMul (OOF_VecSub (&v, pp, pv), pn) > 0;
}

//------------------------------------------------------------------------------

int OOF_FacingViewer (tOOF_vector *pv, tOOF_vector *pn)
{
return OOF_FacingPoint (pv, pn, &vPos);
}

//------------------------------------------------------------------------------

int OOF_FacingLight (tOOF_vector *pv, tOOF_vector *pn)
{
return OOF_FacingPoint (pv, pn, &vrLightPos);
}

//------------------------------------------------------------------------------

tOOF_vector *OOF_CalcFacePerp (tOOF_subObject *pso, tOOF_face *pf)
{
	tOOF_vector		*pv = pso->pvRotVerts;
	tOOF_faceVert	*pfv = pf->pVerts;

#if 0
OOF_CalcFaceNormal (pso, pf);
#endif
return OOF_VecPerp (&pf->vNormal, pv + pfv [0].nIndex, pv + pfv [1].nIndex, pv + pfv [2].nIndex);
}

//------------------------------------------------------------------------------

int OOF_LitFace (tOOF_subObject *pso, tOOF_face *pf)
{
return pf->bFacingLight = 
#if 0
	OOF_FacingLight (&pf->vRotCenter, &pf->vRotNormal); 
#else
	OOF_FacingLight (pso->pvRotVerts + pf->pVerts->nIndex, &pf->vRotNormal); //OOF_CalcFacePerp (pso, pf)); 
#endif
}

//------------------------------------------------------------------------------

int OOF_FrontFace (tOOF_subObject *pso, tOOF_face *pf)
{
#if 0
return OOF_FacingViewer (&pf->vRotCenter, &pf->vRotNormal);
#else
return OOF_FacingViewer (pso->pvRotVerts + pf->pVerts->nIndex, &pf->vRotNormal);	//OOF_CalcFacePerp (pso, pf));
#endif
}

//------------------------------------------------------------------------------

int OOF_GetLitFaces (tOOF_subObject *pso)
{
	tOOF_face		*pf;
	int				i;

for (i = pso->faces.nFaces, pf = pso->faces.pFaces; i; i--, pf++) {
	pf->bFacingLight = OOF_LitFace (pso, pf);
	if (bSWCulling)
		pf->bFacingViewer = OOF_FrontFace (pso, pf);
	}
return pso->faces.nFaces;
}

//------------------------------------------------------------------------------

int OOF_GetSilhouette (tOOF_subObject *pso)
{
	tOOF_edge		*pe;
	tOOF_vector		*pv;
	int				h, i, j;

OOF_GetLitFaces (pso);
pv = pso->pvRotVerts;
for (h = j = 0, i = pso->edges.nEdges, pe = pso->edges.pEdges; i; i--, pe++) {
	if (pe->pf0 && pe->pf1) {
		if (pe->bContour = (pe->pf0->bFacingLight != pe->pf1->bFacingLight))
			h++;
		}
	else {
#if 0
		pe->bContour = 0;
#else
		pe->bContour = 1;
		h++;
#endif
		j++;
		}
	}
return pso->edges.nContourEdges = h;
}

//------------------------------------------------------------------------------

extern tRgbaColorf shadowColor, modelColor;

//------------------------------------------------------------------------------

void OOF_SetCullAndStencil (int bCullFront, int bSWCulling)
{
if (bSWCulling)
	glDisable (GL_CULL_FACE);
else
	glEnable (GL_CULL_FACE);
if (bCullFront) {
	glCullFace (GL_BACK);
	if (bZPass)
		glStencilOp (GL_KEEP, GL_KEEP, GL_DECR_WRAP);
	else
		glStencilOp (GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	}
else {
	glCullFace (GL_FRONT);
	if (bZPass)
		glStencilOp (GL_KEEP, GL_KEEP, GL_INCR_WRAP);
	else
		glStencilOp (GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	}
}

//------------------------------------------------------------------------------

int OOF_DrawShadowVolume (tOOFObject *po, tOOF_subObject *pso, int bCullFront)
{
	tOOF_edge		*pe;
	tOOF_vector		*pv, v0, v1;
	int				i;

if (!bCullFront)
	OOF_GetSilhouette (pso);
if (!bShadowVolume)
	return 1;
if (bShadowTest > 3)
	return 1;
if (bShadowTest < 2)
	glColor4f (shadowColor.red, shadowColor.green, shadowColor.blue, shadowColor.alpha);
OOF_SetCullAndStencil (bCullFront, 0);
pv = pso->pvRotVerts;
if (bShadowTest < 2)
	glBegin (GL_QUADS);
i = bContourEdges ? pso->edges.nContourEdges : pso->edges.nEdges;
for (pe = pso->edges.pEdges; i; pe++)
	if (!bContourEdges || pe->bContour) {
		i--;
		if (bShadowTest < 2) {
			if (bShadowTest) {
				glColor4f (shadowColor.red, shadowColor.green, shadowColor.blue, shadowColor.alpha);
				glBegin (GL_QUADS);
				}
			v1 = pv [pe->v1];
			glVertex3f (v1.x, v1.y, -v1.z);
			v0 = pv [pe->v0];
			glVertex3f (v0.x, v0.y, -v0.z);
#if NORM_INF
			OOF_VecScale (OOF_VecDec (&v0, &vrLightPos), INFINITY / OOF_VecMag (&v0));
			glVertex3f (v0.x, v0.y, -v0.z);
			OOF_VecScale (OOF_VecDec (&v1, &vrLightPos), INFINITY / OOF_VecMag (&v1));
			glVertex3f (v1.x, v1.y, -v1.z);
#else
			glVertex3f ((v0.x - vrLightPos.x) * INFINITY, 
							(v0.y - vrLightPos.y) * INFINITY, 
						  -(v0.z - vrLightPos.z) * INFINITY);
			glVertex3f ((v1.x - vrLightPos.x) * INFINITY, 
							(v1.y - vrLightPos.y) * INFINITY, 
						  -(v1.z - vrLightPos.z) * INFINITY);
#endif
			if (bShadowTest)
				glEnd ();
			}
		else {
			glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
			glBegin (GL_LINES);
			v1 = pv [pe->v1];
			glVertex3f (v1.x, v1.y, -v1.z);
			v0 = pv [pe->v0];
			glVertex3f (v0.x, v0.y, -v0.z);
			glEnd ();
			}
		}
//if (bShadowTest < 2)
//	glEnd ();
return 1;
}

//------------------------------------------------------------------------------

int OOF_DrawShadowCaps (tOOFObject *po, tOOF_subObject *pso, int bCullFront)
{
	tOOF_face		*pf;
	tOOF_faceVert	*pfv;
	tOOF_vector		*pv, *phv, v0;
	int				i, j;

if (bZPass)
	return 1;
if (bShadowTest > 2)
	return 1;
glColor4f (modelColor.red, modelColor.green, modelColor.blue, modelColor.alpha);
pv = pso->pvRotVerts;
OOF_SetCullAndStencil (bCullFront, bSWCulling);
for (i = pso->faces.nFaces, pf = pso->faces.pFaces; i; i--, pf++) {
	if (bSWCulling && (pf->bFacingViewer == bCullFront))
		continue;
	pfv = pf->pVerts;
	if (pf->bFacingLight) {	//render front cap
		//if (!bFrontCap)
			continue;
		glBegin (GL_TRIANGLE_FAN);
		for (j = pf->nVerts, pfv = pf->pVerts; j; j--, pfv++) {
			phv = pv + pfv->nIndex;
			glVertex3f (phv->x, phv->y, -phv->z);
			}
		glEnd ();
		}
	else {	//render back cap
		if (!bRearCap)
			continue;
		glBegin (GL_TRIANGLE_FAN);
		for (j = pf->nVerts, pfv = pf->pVerts + j; j; j--) {
			--pfv;
#if NORM_INF
			OOF_VecScale (OOF_VecSub (&v0, pv + pfv->nIndex, &vrLightPos), INFINITY / OOF_VecMag (&v0));
			glVertex3f (v0.x, v0.y, -v0.z);
#else
			phv = pv + pfv->nIndex;
			glVertex3f ((phv->x - vrLightPos.x) * INFINITY, 
							(phv->y - vrLightPos.y) * INFINITY, 
						  -(phv->z - vrLightPos.z) * INFINITY);
#endif
			}	
		glEnd ();
		}
	}
return 1;
}

//------------------------------------------------------------------------------

int OOF_DrawShadow (tOOFObject *po, tOOF_subObject *pso, int bCullFront)
{
return OOF_DrawShadowVolume (po, pso, bCullFront) && OOF_DrawShadowCaps (po, pso, bCullFront); 
}

//------------------------------------------------------------------------------

int OOF_DrawSubObject (tObject *objP, tOOFObject *po, tOOF_subObject *pso, int bFacing, float *fLight)
{
	tOOF_face		*pf;
	tOOF_faceVert	*pfv;
	tOOF_vector		*pv, *pvn, *phv;
	tFaceColor		*pvc;
	grsBitmap		*bmP;
	int				h, i, j;
	int				bOglLighting = gameOpts->ogl.bUseLighting && gameOpts->ogl.bLightObjects;
	float				fl, r, g, b;

if (bShadowTest)
	return 1;
pv = pso->pvRotVerts;
pvn = pso->pvNormals;
pvc = pso->pVertColors;
//memset (pvc, 0, pso->nVerts * sizeof (tFaceColor));
for (i = pso->faces.nFaces, pf = pso->faces.pFaces; i; i--, pf++) {
	pfv = pf->pVerts;
#if 0
	if (!(gameStates.ogl.bUseTransform || OOF_FrontFace (pso, pf)))
		continue;
#endif
	if (pf->bTextured) {
#ifdef _DEBUG
		fl = -OOF_VecDot (&pf->vNormal, &mView.f);
		fl = 0.25f + 0.75f * fl;
		fl = fl * *fLight;
#else
		fl = *fLight * (0.25f - 0.75f * OOF_VecDot (&pf->vNormal, &mView.f));
#endif
//		fl = 1.0f;
		bmP = po->textures.pBitmaps + pf->texProps.nTexId;
		if (bmP->glTexture && (bmP->glTexture->handle < 0))
			bmP->glTexture->handle = 0;
		if (OglBindBmTex (bmP, 0))
			return 0;
		OglTexWrap (bmP->glTexture, GL_REPEAT);
		if (pso->nFlags & (bOglLighting ? OOF_SOF_THRUSTER : (OOF_SOF_GLOW | OOF_SOF_THRUSTER))) {
			glColor4f (fl * pso->glowInfo.color.r, 
						  fl * pso->glowInfo.color.g, 
						  fl * pso->glowInfo.color.b, 
						  pso->pfAlpha [pfv->nIndex] * po->fAlpha);
			}
		else if (!bOglLighting) {
			tFaceColor *psc = AvgSgmColor (objP->nSegment, &objP->pos);
			if (psc->index != gameStates.render.nFrameFlipFlop + 1)
				glColor4f (fl, fl, fl, pso->pfAlpha [pfv->nIndex] * po->fAlpha);
			else
				glColor4f (psc->color.red * fl, psc->color.green * fl, psc->color.blue * fl,
							  pso->pfAlpha [pfv->nIndex] * po->fAlpha);
			}
		glBegin (GL_TRIANGLE_FAN);
		for (j = pf->nVerts; j; j--, pfv++) {
			phv = pv + (h = pfv->nIndex);
			if (bOglLighting) {
				if (pvc [h].index == gameStates.render.nFrameFlipFlop + 1)
					OglColor4sf (pvc [h].color.red, pvc [h].color.green, pvc [h].color.blue, 1.0);
				else
					G3VertexColor ((fVector *) (pvn + h), (fVector *) phv, -1, pvc + h);
				}
			glMultiTexCoord2f (GL_TEXTURE0_ARB, pfv->fu, pfv->fv);
			glVertex3f (phv->x, phv->y, -phv->z);
			}	
		glEnd ();
		}
	else {
		fl = fLight [1];
		r = fl * (float) pf->texProps.color.r / 255.0f;
		g = fl * (float) pf->texProps.color.g / 255.0f;
		b = fl * (float) pf->texProps.color.b / 255.0f;
		glColor4f (r, g, b, pso->pfAlpha [pfv->nIndex] * po->fAlpha);
		glBegin (GL_TRIANGLE_FAN);
		for (j = pf->nVerts, pfv = pf->pVerts; j; j--, pfv++) {
			phv = pv + pfv->nIndex;
			glVertex3f (phv->x, phv->y, -phv->z);
			}	
		glEnd ();
		}
	}
return 1;
}

//------------------------------------------------------------------------------

inline void OOF_RotVert (tOOF_vector *prv, tOOF_vector *pv, tOOF_vector *vo)
{
	tOOF_vector	v;

if (gameStates.ogl.bUseTransform)
	OOF_VecAdd (prv, pv, vo);
else {
	OOF_VecSub (&v, pv, &vPos);
	OOF_VecInc (&v, vo);
	OOF_VecRot (prv, &v, &mView);
	}
}

//------------------------------------------------------------------------------

void OOF_RotModelVerts (tOOF_subObject *pso, tOOF_vector vo)
{
	tOOF_vector	*pv, *prv;
	tOOF_face	*pf;
	int			i;

for (i = pso->nVerts, pv = pso->pvVerts, prv = pso->pvRotVerts; i; i--, pv++, prv++)
	OOF_RotVert (prv, pv, &vo);
for (i = pso->faces.nFaces, pf = pso->faces.pFaces; i; i--, pf++) {
	OOF_RotVert (&pf->vRotNormal, &pf->vNormal, &vo);
	OOF_VecNormalize (&pf->vRotNormal);
	OOF_RotVert (&pf->vRotCenter, &pf->vCenter, &vo);
	}
}

//------------------------------------------------------------------------------

int OOF_RenderSubObject (tObject *objP, tOOFObject *po, tOOF_subObject *pso, tOOF_vector vo, 
								 int nIndex, int bFacing, float *fLight)
{
	tOOF_subObject	*psc;
	int				i, j;

OOF_VecInc (&vo, &pso->vOffset);
if (!bFacing)
	OOF_RotModelVerts (pso, vo);
if ((gameStates.render.nShadowPass != 2) && (bFacing != ((pso->nFlags & OOF_SOF_FACING) != 0)))
	return 1;
for (i = 0; i < pso->nChildren; i++) {
	psc = po->pSubObjects + (j = pso->children [i]);
	Assert (j >= 0 && j < po->nSubObjects);
	if (psc->nParent == nIndex)
		if (!OOF_RenderSubObject (objP, po, psc, vo, j, bFacing, fLight))
			return 0;
	}
#if SHADOWS
if (gameStates.render.nShadowPass == 2)
	OOF_DrawShadow (po, pso, bFacing);
else 
#endif
	OOF_DrawSubObject (objP, po, pso, bFacing, fLight);
return 1;
}

//------------------------------------------------------------------------------

int OOF_RenderModel (tObject *objP, tOOFObject *po, float *fLight)
{
	tOOF_subObject	*pso;
	int				r = 1, i, bFacing;
	tOOF_vector		vo = {0.0f,0.0f,0.0f};

G3StartInstanceMatrix (&objP->pos, &objP->orient);
if (!gameStates.ogl.bUseTransform)
	OOF_MatVms2Oof (&mView, &viewInfo.view);
OOF_VecVms2Oof (&vPos, &viewInfo.position);
if (IsMultiGame && netGame.BrightPlayers)
	*fLight = 1.0f;
OglActiveTexture (GL_TEXTURE0_ARB);
glEnable (GL_TEXTURE_2D);
for (bFacing = 0; bFacing < 2; bFacing++)
	for (i = 0, pso = po->pSubObjects; i < po->nSubObjects; i++, pso++)
		if (pso->nParent == -1)
			if (!OOF_RenderSubObject (objP, po, pso, vo, i, bFacing, fLight)) {
				r = 0;
				break;
				}
G3DoneInstance ();
glDisable (GL_TEXTURE_2D);
return r;
}

//------------------------------------------------------------------------------

int OOF_RenderShadow (tObject *objP, tOOFObject *po, float *fLight)
{
	short			*pnl = gameData.render.lights.ogl.nNearestSegLights [gameData.objs.console->nSegment];
	vmsVector	h, vl;

for (gameData.render.shadows.nLight = 0; 
	  (gameData.render.shadows.nLight < gameOpts->render.nMaxLights) && (*pnl >= 0); 
	  gameData.render.shadows.nLight++, pnl++) {
	gameData.render.shadows.pLight = gameData.render.lights.ogl.lights + *pnl;
#if 0
	OOF_VecVms2Oof (&vrLightPos, &gameData.render.lights.ogl.lights [*pnl].vPos);
#else
	G3TransformPoint (&vl, &gameData.render.lights.ogl.lights [*pnl].vPos);
	OOF_VecVms2Oof (&vrLightPos, &vl);
#endif
	if (!OOF_RenderModel (objP, po, fLight))
		return 0;
	if (gameStates.render.bAltShadows)
		RenderShadow (f2fl (VmVecMag (VmVecSub (&h, &gameData.render.shadows.pLight->vPos, &objP->pos))));
	}
return 1;
}

//------------------------------------------------------------------------------

int OOF_Render (tObject *objP, tOOFObject *po, float *fLight, int bCloaked)
{
	float	dt;

if (po->bCloaked != bCloaked) {
	po->bCloaked = bCloaked;
	po->nCloakPulse = 0;
	po->nCloakChangedTime = gameStates.app.nSDLTicks;
	}
dt = (float) (gameStates.app.nSDLTicks - po->nCloakChangedTime) / 1000.0f;
if (bCloaked) {
	if (po->nCloakPulse) {
		//dt = 0.001f;
		po->nCloakChangedTime = gameStates.app.nSDLTicks;
		po->fAlpha += dt * po->nCloakPulse / 10.0f;
		if (po->nCloakPulse < 0) {
			if (po->fAlpha <= 0.01f)
				po->nCloakPulse = -po->nCloakPulse;
			}
		else if (po->fAlpha >= 0.1f)
			po->nCloakPulse = -po->nCloakPulse;
		}
	else {
		po->fAlpha = 1.0f - dt;
		if (po->fAlpha <= 0.01f) {
			po->nCloakPulse = 1;
			po->nCloakChangedTime = gameStates.app.nSDLTicks;
			}
		}
	}
else {
	po->fAlpha += dt;
	if (po->fAlpha > 1.0f)
		po->fAlpha = 1.0f;
	}
if (po->fAlpha < 0.01f)
	po->fAlpha = 0.01f;
#if SHADOWS
return (!gameStates.render.bShadowMaps && (gameStates.render.nShadowPass == 2)) ? 
	OOF_RenderShadow (objP, po, fLight) :
	OOF_RenderModel (objP, po, fLight);
#else
return OOF_RenderModel (objP, po, fLight);
#endif
}

//------------------------------------------------------------------------------
//eof

