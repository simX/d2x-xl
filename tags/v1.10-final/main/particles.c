//particle.h
//simple particle system handler

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#ifdef __macosx__
# include <SDL/SDL.h>
#else
# include <SDL.h>
#endif

#include "pstypes.h"
#include "inferno.h"
#include "error.h"
#include "u_mem.h"
#include "vecmat.h"
#include "hudmsg.h"
#include "ogl_init.h"
#include "piggy.h"
#include "globvars.h"
#include "gameseg.h"
#include "network.h"
#include "lighting.h"
#include "render.h"
#include "objsmoke.h"
#include "particles.h"

#ifdef __macosx__
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#	include <OpenGL/glext.h>
#else
#	include <GL/gl.h>
#	include <GL/glu.h>
#	ifdef _WIN32
#		include <glext.h>
#		include <wglext.h>
#	else
#		include <GL/glext.h>
#		include <GL/glx.h>
#		include <GL/glxext.h>
#	endif
#endif

//------------------------------------------------------------------------------

#define MAKE_SMOKE_IMAGE 0

#define BLOWUP_PARTICLES 0		//blow particles up at "birth"

#define SMOKE_SLOWMO 0

#define SORT_CLOUDS 1

#define SORT_CLOUD_PARTS 0

#define OGL_VERTEX_ARRAYS	1

#define PARTICLE_TYPES	4

#define PARTICLE_FPS	30

#define PARTICLE_RAD	(F1_0)

#define PART_DEPTHBUFFER_SIZE 100000
#define PARTLIST_SIZE 1000000

static int bHavePartImg [2][PARTICLE_TYPES] = {{0, 0, 0, 0},{0, 0, 0, 0}};

static grsBitmap *bmpParticle [2][PARTICLE_TYPES] = {{NULL, NULL, NULL, NULL},{NULL, NULL, NULL, NULL}};
#if 0
static grsBitmap *bmpBumpMaps [2] = {NULL, NULL};
#endif

static char *szParticleImg [2][PARTICLE_TYPES] = {
	{"smoke.tga", "corona.tga"},
	{"smoke.tga", "corona.tga"}
	};

static int nParticleFrames [2][PARTICLE_TYPES] = {{1,1,1,1},{1,1,1,1}};
static int iParticleFrames [2][PARTICLE_TYPES] = {{0,0,0,0},{0,0,0,0}};
static int iPartFrameIncr  [2][PARTICLE_TYPES] = {{1,1,1,1},{1,1,1,1}};

static double alphaScale [5] = {6.0 / 6.0, 5.0 / 6.0, 4.0 / 6.0, 3.0 / 6.0, 2.0 / 6.0};

#if EXTRA_VERTEX_ARRAYS

#define VERT_BUF_SIZE	512 //8192

static GLdouble vertexBuffer [VERT_BUF_SIZE][3];
static GLdouble texCoordBuffer [VERT_BUF_SIZE][2];
static GLdouble colorBuffer [VERT_BUF_SIZE][4];
static int iBuffer = 0;
#else
static int iBuffer = 0;
static int nBuffer = 0;
#endif

#define SMOKE_START_ALPHA		(gameOpts->render.smoke.bDisperse ? 32 : 64)

//	-----------------------------------------------------------------------------

void InitSmoke (void)
{
	int i, j;
#if OGL_VERTEX_BUFFERS
	GLdouble	pf = colorBuffer;

for (i = 0; i < VERT_BUFFER_SIZE; i++, pf++) {
	*pf++ = 1.0;	
	*pf++ = 1.0;	
	*pf++ = 1.0;	
	}
#endif
for (i = 0, j = 1; j < MAX_SMOKE; i++, j++) 
	gameData.smoke.buffer [i].nNext = j;
gameData.smoke.buffer [i].nNext = -1;
gameData.smoke.iFree = 0;
gameData.smoke.iUsed = -1;
}

//------------------------------------------------------------------------------

void ResetDepthBuf (void)
{
memset (gameData.smoke.depthBuf.pDepthBuffer, 0, PART_DEPTHBUFFER_SIZE * sizeof (struct tPartList **));
memset (gameData.smoke.depthBuf.pPartList, 0, (PARTLIST_SIZE - gameData.smoke.depthBuf.nFreeParts) * sizeof (struct tPartList));
gameData.smoke.depthBuf.nFreeParts = PARTLIST_SIZE;
}

//	-----------------------------------------------------------------------------

int AllocPartList (void)
{
if (gameData.smoke.depthBuf.pDepthBuffer)
	return 1;
if (!(gameData.smoke.depthBuf.pDepthBuffer = (struct tPartList **) D2_ALLOC (PART_DEPTHBUFFER_SIZE * sizeof (struct tPartList *))))
	return 0;
if (!(gameData.smoke.depthBuf.pPartList = (struct tPartList *) D2_ALLOC (PARTLIST_SIZE * sizeof (struct tPartList)))) {
	D2_FREE (gameData.smoke.depthBuf.pDepthBuffer);
	return 0;
	}
gameData.smoke.depthBuf.nFreeParts = 0;
ResetDepthBuf ();
return 1;
}

//	-----------------------------------------------------------------------------

void FreePartList (void)
{
D2_FREE (gameData.smoke.depthBuf.pPartList);
D2_FREE (gameData.smoke.depthBuf.pDepthBuffer);
}

//	-----------------------------------------------------------------------------

void AnimateParticle (int nType)
{
	int	bPointSprites = gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort,
			nFrames = nParticleFrames [bPointSprites][nType];

if (nFrames > 1) {
	static time_t t0 [PARTICLE_TYPES] = {0, 0, 0, 0};

	time_t		t = gameStates.app.nSDLTicks;
	int			iFrame = iParticleFrames [bPointSprites][nType];
	int			iFrameIncr = iPartFrameIncr [bPointSprites][nType];
	int			bPointSprites = gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort;
	grsBitmap	*bmP = bmpParticle [gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort][nType == PARTICLE_TYPES - 1];

	if (!BM_FRAMES (bmP))
		return;
	BM_CURFRAME (bmP) = BM_FRAMES (bmP) + iFrame;
#if 1
	if (t - t0 [nType] > 150) 
#endif
		{

		t0 [nType] = t;
#if 1
		iFrame = (iFrame + 1) % nFrames;
#else
		iFrame += iFrameIncr;
		if ((iFrame < 0) || (iFrame >= nFrames)) {
			iPartFrameIncr [bPointSprites][nType] = -iFrameIncr;
			iFrame += -2 * iFrameIncr;
			}
#endif
		iParticleFrames [bPointSprites][nType] = iFrame;
		}
	}
}

//	-----------------------------------------------------------------------------

void AdjustParticleBrightness (grsBitmap *bmP)
{
	grsBitmap	*bmfP;
	int			i, j = bmP->bmData.alt.bmFrameCount;
	double		*dFrameBright, dAvgBright = 0, dMaxBright = 0;

if (j < 2)
	return;
if (!(dFrameBright = (double *) D2_ALLOC (j * sizeof (double))))
	return;
for (i = 0, bmfP = BM_FRAMES (bmP); i < j; i++, bmfP++) {
	dAvgBright += (dFrameBright [i] = TGABrightness (bmfP));
	if (dMaxBright < dFrameBright [i])
		dMaxBright = dFrameBright [i];
	}
dAvgBright /= j;
for (i = 0, bmfP = BM_FRAMES (bmP); i < j; i++, bmfP++) {
	TGAChangeBrightness (bmfP, 0, 1, 2 * (int) (255 * dFrameBright [i] * (dAvgBright - dFrameBright [i])), 0);
	}
D2_FREE (dFrameBright);
}

//	-----------------------------------------------------------------------------

int LoadParticleImage (int nType)
{
	int			h, 
					bPointSprites = gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort,
					*flagP;
	grsBitmap	*bmP = NULL;

nType = (nType % PARTICLE_TYPES == PARTICLE_TYPES - 1);
flagP = bHavePartImg [bPointSprites] + nType;
if (*flagP < 0)
	return 0;
if (*flagP > 0)
	return 1;
bmP = CreateAndReadTGA (szParticleImg [bPointSprites][nType]);
*flagP = bmP ? 1 : -1;
if (*flagP < 0)
	return 0;
bmpParticle [bPointSprites][nType] = bmP;
#if MAKE_SMOKE_IMAGE
{
	tTgaHeader h;

TGAInterpolate (bmP, 2);
if (TGAMakeSquare (bmP)) {
	memset (&h, 0, sizeof (h));
	SaveTGA (szParticleImg [bPointSprites][nType], gameFolders.szDataDir, &h, bmP);
	}
}
#endif
BM_FRAMECOUNT (bmP) = bmP->bmProps.h / bmP->bmProps.w;
#if 0
if (OglSetupBmFrames (BmOverride (bmP), 0, 0, 0)) {
	AdjustParticleBrightness (bmP);
	D2_FREE (BM_FRAMES (bmP));	// make sure frames get loaded to OpenGL in OglLoadBmTexture ()
	BM_CURFRAME (bmP) = NULL;
	}
#endif
OglLoadBmTexture (bmP, 0, 3, 1);
if (nType == PARTICLE_TYPES - 1)
	nParticleFrames [bPointSprites][nType] = BM_FRAMECOUNT (bmP);
else {
	h = bmP->bmProps.w / 64;
	nParticleFrames [bPointSprites][nType] = h;
	}
return *flagP > 0;
}

//	-----------------------------------------------------------------------------

int LoadParticleImages (void)
{
	int	i;

for (i = 0; i < 2; i++) {
	if (!LoadParticleImage (i))
		return 0;
	AnimateParticle (i);
	}
return 1;
}

//	-----------------------------------------------------------------------------

void FreeParticleImages (void)
{
	int	i, j;

for (i = 0; i < 2; i++)
	for (j = 0; j < 2; j++)
		if (bmpParticle [i][j]) {
			GrFreeBitmap (bmpParticle [i][j]);
			bmpParticle [i][j] = NULL;
			bHavePartImg [i][j] = 0;
			}
}

//------------------------------------------------------------------------------

static inline int randN (int n)
{
if (!n)
	return 0;
return (int) ((double) rand () * (double) n / (double) RAND_MAX);
}

//------------------------------------------------------------------------------

inline double sqr (double n)
{
return n * n;
}

//------------------------------------------------------------------------------

inline double ParticleBrightness (tRgbaColord *pColor)
{
#if 0
return (pColor->red + pColor->green + pColor->blue) / 3.0;
#else
return (pColor->red * 3 + pColor->green * 5 + pColor->blue * 2) / 10.0;
#endif
}

//------------------------------------------------------------------------------

vmsVector *RandomPointOnQuad (vmsVector *quad, vmsVector *vPos)
{
	vmsVector	vOffs;
	int			i;

i = rand () % 2;
VmVecSub (&vOffs, quad + i + 1, quad + i);
VmVecScale (&vOffs, 2 * d_rand ());
VmVecInc (&vOffs, quad + i);
i += 2;
VmVecSub (vPos, quad + (i + 1) % 4, quad + i);
VmVecScale (vPos, 2 * d_rand ());
VmVecInc (vPos, quad + i);
VmVecDec (vPos, &vOffs);
VmVecScale (vPos, 2 * d_rand ());
VmVecInc (vPos, &vOffs);
return vPos;
}

//------------------------------------------------------------------------------

#define RANDOM_FADE	(0.95 + (double) rand () / (double) RAND_MAX / 20.0)

int CreateParticle (tParticle *pParticle, vmsVector *pPos, vmsVector *pDir,
						  short nSegment, int nLife, int nSpeed, char nSmokeType, char nClass,
						  float nScale, tRgbaColord *pColor, int nCurTime, int bBlowUp,
						  double dBrightness, vmsVector *vEmittingFace)
{
	vmsVector	vDrift, vPos;
	int			nRad, nFrames, nType = (nSmokeType == PARTICLE_TYPES - 1);

if (vEmittingFace)
	pPos = RandomPointOnQuad (vEmittingFace, &vPos);
if (nScale < 0)
	nRad = (int) -nScale;
else if (gameOpts->render.smoke.bSyncSizes)
	nRad = (int) PARTICLE_SIZE (gameOpts->render.smoke.nSize [0], nScale);
else
	nRad = (int) nScale;
if (!nRad)
	nRad = F1_0;
pParticle->nType = nType;
pParticle->nClass = nClass;
pParticle->nSegment = nSegment;
pParticle->nBounce = 0;
pParticle->bBright = (rand () % 50) == 0;
if (pColor) {
	pParticle->color.red = pColor->red * RANDOM_FADE;
	pParticle->color.green = pColor->green * RANDOM_FADE;
	pParticle->color.blue = pColor->blue * RANDOM_FADE;
	pParticle->nFade = 0;
	}
else {
	pParticle->color.red = 1.0;
	pParticle->color.green = 0.5;
	pParticle->color.blue = 0.0;//(double) (64 + randN (64)) / 255.0;
	pParticle->nFade = 2;
	}
pParticle->color.alpha = (double) (SMOKE_START_ALPHA + randN (64)) / 255.0;
#if 1
if (gameOpts->render.smoke.bDisperse && !pParticle->bBright) {
	dBrightness = 1.0 - dBrightness;
	pParticle->color.alpha += dBrightness * dBrightness / 8.0;
	}
#endif
//nSpeed = (int) (sqrt (nSpeed) * (double) F1_0);
nSpeed *= F1_0;
if (pDir) {
	vmsAngVec	a;
	vmsMatrix	m;
	double		d;
	a.p = randN (F1_0 / 4) - F1_0 / 8;
	a.b = randN (F1_0 / 4) - F1_0 / 8;
	a.h = randN (F1_0 / 4) - F1_0 / 8;
	VmAngles2Matrix (&m, &a);
	VmVecNormalize (VmVecRotate (&vDrift, pDir, &m));
	d = (double) VmVecDeltaAng (&vDrift, pDir, NULL);
	if (d) {
		d = exp ((F1_0 / 8) / d);
		nSpeed = (fix) ((double) nSpeed / d);
		}
	pParticle->color.green =
	pParticle->color.blue = 1.0;//(double) (64 + randN (64)) / 255.0;
	VmVecScale (&vDrift, nSpeed);
	pParticle->dir = *pDir;
	pParticle->bHaveDir = 1;
	}
else {
	vmsVector	vOffs;
	vDrift.p.x = nSpeed - randN (2 * nSpeed);
	vDrift.p.y = nSpeed - randN (2 * nSpeed);
	vDrift.p.z = nSpeed - randN (2 * nSpeed);
	vOffs = vDrift;
	VmVecZero (&pParticle->dir);
	pParticle->bHaveDir = 1;
	}
if (vEmittingFace)
	pParticle->pos = *pPos;
else
	VmVecScaleAdd (&pParticle->pos, pPos, &vDrift, F1_0 / 64);
pParticle->drift = vDrift;
if (nLife < 0)
	nLife = -nLife;
if (nType < 3) {
	if (gameOpts->render.smoke.bDisperse)
		nLife = (nLife * 2) / 3;
	nLife = nLife / 2 + randN (nLife / 2);
	}
pParticle->nLife = 
pParticle->nTTL = nLife;
pParticle->nMoved = nCurTime;
pParticle->nDelay = 0; //bStart ? randN (nLife) : 0;
nRad += nType ? nRad : randN (nRad);
if (pParticle->bBlowUp = bBlowUp) {
	pParticle->nRad = nRad;
	pParticle->nWidth =
	pParticle->nHeight = nRad / 2;
	}
else {
	pParticle->nWidth =
	pParticle->nHeight = nRad;
	pParticle->nRad = nRad / 2;
	}
nFrames = nParticleFrames [gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort && (gameOpts->render.bDepthSort <= 0)][nType];
pParticle->nFrame = rand () % (nFrames * nFrames);
pParticle->nOrient = rand () % 4;
#if 1
pParticle->color.alpha /= nSmokeType + 2;
if (nType)
	pParticle->color.alpha *= ParticleBrightness (pColor);
#endif
return 1;
}

//------------------------------------------------------------------------------

int DestroyParticle (tParticle *pParticle)
{
#ifdef _DEBUG
memset (pParticle, 0, sizeof (tParticle));
#endif
return 1;
}

//------------------------------------------------------------------------------

inline int ChangeDir (int d)
{
	int	h = d;

if (h)
	h = h / 2 - randN (h);
return (d * 10 + h) / 10;
}

//------------------------------------------------------------------------------

static int nPartSeg = -1;
static int nFaces [6];
static int vertexList [6][6];
static int bSidePokesOut [6];
//static int nVert [6];
static vmsVector	*wallNorm;

int CollideParticleAndWall (tParticle *pParticle)
{
	tSegment		*segP;
	tSide			*sideP;
	int			bInit, nSide, nVert, nChild, nFace, nInFront;
	fix			nDist;
	int			*vlP;
	vmsVector	pos = pParticle->pos;

//redo:

segP = gameData.segs.segments + pParticle->nSegment;
if ((bInit = (pParticle->nSegment != nPartSeg))) 
	nPartSeg = pParticle->nSegment;
for (nSide = 0, sideP = segP->sides; nSide < 6; nSide++, sideP++) {
	vlP = vertexList [nSide];
	if (bInit) {
		nFaces [nSide] = CreateAbsVertexLists (vlP, nPartSeg, nSide);
		if (nFaces [nSide] == 1) {
			bSidePokesOut [nSide] = 0;
			nVert = vlP [0];
			}
		else {
			nVert = min (vlP [0], vlP [2]);
			if (vlP [4] < vlP [1])
				nDist = VmDistToPlane (gameData.segs.vertices + vlP [4], sideP->normals, gameData.segs.vertices + nVert);
			else
				nDist = VmDistToPlane (gameData.segs.vertices + vlP [1], sideP->normals + 1, gameData.segs.vertices + nVert);
			bSidePokesOut [nSide] = (nDist > PLANE_DIST_TOLERANCE);
			}
		}
	else
		nVert = (nFaces [nSide] == 1) ? vlP [0] : min (vlP [0], vlP [2]);
	for (nFace = nInFront = 0; nFace < nFaces [nSide]; nFace++) {
		nDist = VmDistToPlane (&pos, sideP->normals + nFace, gameData.segs.vertices + nVert);
		if (nDist > -PLANE_DIST_TOLERANCE)
			nInFront++;
		else
			wallNorm = sideP->normals + nFace;
		}
	if (!nInFront || (bSidePokesOut [nSide] && (nFaces [nSide] == 2) && (nInFront < 2))) {
		if (0 > (nChild = segP->children [nSide])) 
			return 1;
		pParticle->nSegment = nChild;
		break;
#if 0
		if (bRedo)
			break;
		bRedo = 1;
		goto redo;
#endif
		}
	}
return 0;
}

//------------------------------------------------------------------------------

int UpdateParticle (tParticle *pParticle, int nCurTime)
{
	int			j, nRad;
	fix			t, dot;
	vmsVector	pos, drift;
	fix			drag = fl2f ((float) pParticle->nLife / (float) pParticle->nTTL);


if ((pParticle->nLife <= 0) /*|| (pParticle->color.alpha < 0.01)*/)
	return 0;
t = nCurTime - pParticle->nMoved;
if (pParticle->nDelay > 0)
	pParticle->nDelay -= t;
else {
	pos = pParticle->pos;
	drift = pParticle->drift;
	if (pParticle->nType != 3) {
		drift.p.x = ChangeDir (drift.p.x);
		drift.p.y = ChangeDir (drift.p.y);
		drift.p.z = ChangeDir (drift.p.z);
		}
	for (j = 0; j < 2; j++) {
		if (t < 0)
			t = -t;
		VmVecScaleAdd (&pParticle->pos, &pos, &drift, t);
		if (pParticle->bHaveDir) {
			vmsVector vi = drift, vj = pParticle->dir;
			VmVecNormalize (&vi);
			VmVecNormalize (&vj);
//				if (VmVecDot (&drift, &pParticle->dir) < 0)
			if (VmVecDot (&vi, &vj) < 0)
				drag = -drag;
//				VmVecScaleInc (&drift, &pParticle->dir, drag);
			VmVecScaleInc (&pParticle->pos, &pParticle->dir, drag);
			}
		if (gameOpts->render.smoke.bCollisions && CollideParticleAndWall (pParticle)) {	//reflect the particle
			if (j)
				return 0;
			else if (!(dot = VmVecDot (&drift, wallNorm)))
				return 0;
			else {
				VmVecScaleAdd (&drift, &pParticle->drift, wallNorm, -2 * dot);
				//VmVecScaleAdd (&pParticle->pos, &pos, &drift, 2 * t);
				pParticle->nBounce = 3;
				continue;
				}
			}
		else if (pParticle->nBounce) 
			pParticle->nBounce--;
		else {
			break;
			}
		}
	pParticle->drift = drift;
	if (pParticle->nTTL >= 0) {
#if SMOKE_SLOWMO
		pParticle->nLife -= (int) (t / gameStates.gameplay.slowmo [0].fSpeed);
#else
		pParticle->nLife -= t;
#endif
		if (nRad = pParticle->nRad) {
			if (pParticle->bBlowUp) {
				if (pParticle->nWidth >= nRad)
					pParticle->nRad = 0;
				else {
					pParticle->nWidth += nRad / 10 / pParticle->bBlowUp;
					pParticle->nHeight += nRad / 10 / pParticle->bBlowUp;
					if (pParticle->nWidth > nRad)
						pParticle->nWidth = nRad;
					if (pParticle->nHeight > nRad)
						pParticle->nHeight = nRad;
					pParticle->color.alpha *= (1.0 + 0.0725 / pParticle->bBlowUp);
					if (pParticle->color.alpha > 1)
						pParticle->color.alpha = 1;
					}
				}
			else {
				if (pParticle->nWidth <= nRad)
					pParticle->nRad = 0;
				else {
					pParticle->nRad += nRad / 5;
					pParticle->color.alpha *= 1.0725;
					if (pParticle->color.alpha > 1)
						pParticle->color.alpha = 1;
					}
				}
			}
		}
	}	
pParticle->nMoved = nCurTime;
return 1;
}

//------------------------------------------------------------------------------

#if EXTRA_VERTEX_ARRAYS
static void FlushVertexArrays (void)
#else
static void FlushVertexArrays (tParticle *pParticles, int iBuffer)
#endif
{
if (iBuffer) {
	GLenum	i;

#if !EXTRA_VERTEX_ARRAYS
	glColorPointer (4, GL_DOUBLE, sizeof (tParticle) - sizeof (tRgbaColord), &pParticles->color);
	glVertexPointer (3, GL_DOUBLE, sizeof (tParticle) - sizeof (tPartPos), &pParticles->glPos);
	if (i = glGetError ()) {
#	ifdef _DEBUG
		HUDMessage (0, "glVertexPointer %d failed (%d)", iBuffer, i);
#	endif
		glVertexPointer (3, GL_DOUBLE, sizeof (tParticle) - sizeof (tPartPos), &pParticles->glPos);
		if (i = glGetError ())
			glVertexPointer (3, GL_DOUBLE, sizeof (tParticle) - sizeof (tPartPos), &pParticles->glPos);
		}
#else
	glVertexPointer (3, GL_DOUBLE, 0, vertexBuffer);
#	ifdef _DEBUG
	if ((i = glGetError ()))
		HUDMessage (0, "glVertexPointer %d failed (%d)", iBuffer, i);
#	endif
	if (!gameStates.render.bPointSprites) {
		glTexCoordPointer (2, GL_DOUBLE, 0, texCoordBuffer);
#	ifdef _DEBUG
		if ((i = glGetError ()))
			HUDMessage (0, "glTexCoordPointer %d failed (%d)", iBuffer, i);
#	endif
		}
	glColorPointer (4, GL_DOUBLE, 0, colorBuffer);
#	ifdef _DEBUG
	if ((i = glGetError ()))
		HUDMessage (0, "glColorPointer %d failed (%d)", iBuffer, i);
#	endif
#endif
	i = glGetError ();
	glDrawArrays ((gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort) ? GL_POINTS : GL_QUADS, 0, iBuffer);
	i = glGetError ();
	iBuffer = 0;
	}
}

//------------------------------------------------------------------------------

inline void SetParticleTexCoord (GLdouble u, GLdouble v)
{
#if OGL_VERTEX_ARRAYS
if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
	texCoordBuffer [iBuffer][0] = u;
	texCoordBuffer [iBuffer][1] = v;
	}
else 
#endif
	glTexCoord2d (u, v);
}

//------------------------------------------------------------------------------

int RenderParticle (tParticle *pParticle, double brightness)
{
	vmsVector			hp;
	GLdouble				d, u, v, x, y, z, h, w;
	grsBitmap			*bmP;
	tRgbaColord			pc;
	tUVLd					texCoord [4];
#if OGL_VERTEX_ARRAYS
	double				*pf;
#endif
	double				decay = (double) pParticle->nLife / (double) pParticle->nTTL;
	int					i, nType = pParticle->nType,
							bPointSprites = gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort && (gameOpts->render.bDepthSort <= 0);

	static int			nFrames = 1;
	static double		deltaUV = 1.0;

if (pParticle->nDelay > 0)
	return 0;
if (!(bmP = bmpParticle [bPointSprites][nType]))
	return 0;
if (BM_CURFRAME (bmP))
	bmP = BM_CURFRAME (bmP);
#if 1
if (gameOpts->render.bDepthSort > 0) {
	hp = pParticle->transPos;
	if (gameData.smoke.nLastType != nType) {
		gameData.smoke.nLastType = nType;
		if (OglBindBmTex (bmP, 0, 1))
			return 0;
		nFrames = nParticleFrames [bPointSprites][nType];
		deltaUV = 1.0 / (double) nFrames;
		}
	glBegin (GL_QUADS);
	}
else if (gameOpts->render.smoke.bSort) {
	hp = pParticle->transPos;
	if (gameData.smoke.nLastType != nType) {
		gameData.smoke.nLastType = nType;
		glEnd ();
		if (OglBindBmTex (bmP, 0, 1))
			return 0;
		nFrames = nParticleFrames [bPointSprites][nType];
		deltaUV = 1.0 / (double) nFrames;
		glBegin (GL_QUADS);
		}
	}
else
#endif
	G3TransformPoint (&hp, &pParticle->pos, 0);
if (pParticle->bBright)
	brightness = sqrt (brightness);
if (nType) {
	//pParticle->color.green *= 0.99;
	//pParticle->color.blue *= 0.99;
	}
else if (pParticle->nFade > 0) {
	if (pParticle->color.green < 1.0) {
#if SMOKE_SLOWMO
		pParticle->color.green += 1.0 / 20.0 / (double) gameStates.gameplay.slowmo [0].fSpeed;
#else
		pParticle->color.green += 1.0 / 20.0; 
#endif
		if (pParticle->color.green > 1.0) {
			pParticle->color.green = 1.0;
			pParticle->nFade--;
			}
		}
	if (pParticle->color.blue < 1.0) {
#if SMOKE_SLOWMO
		pParticle->color.blue += 1.0 / 10.0 / (double) gameStates.gameplay.slowmo [0].fSpeed;
#else
		pParticle->color.blue += 1.0 / 10.0;
#endif
		if (pParticle->color.blue > 1.0) {
			pParticle->color.blue = 1.0;
			pParticle->nFade--;
			}
		}
	}
else if (pParticle->nFade == 0) {
	pParticle->color.red = pParticle->color.red * RANDOM_FADE;
	pParticle->color.green = pParticle->color.green * RANDOM_FADE;
	pParticle->color.blue = pParticle->color.blue * RANDOM_FADE;
	pParticle->nFade = -1;
	}
pc = pParticle->color;
//pc.alpha *= /*gameOpts->render.smoke.bDisperse ? decay2 :*/ decay;
if (nType)
	;//pc.alpha /= 2;
else
	pc.alpha *= alphaScale [gameOpts->render.smoke.nAlpha [gameOpts->render.smoke.bSyncSizes ? 0 : pParticle->nClass]];
pc.alpha = (pc.alpha - 0.005) * decay + 0.005;
if (nType) {
	u = v = 0.0;
	d = 1.0;
	}
else {
	u = (double) (pParticle->nFrame % nFrames) * deltaUV;
	v = (double) (pParticle->nFrame / nFrames) * deltaUV;
	d = deltaUV;
	}
if (!nType) {
	if (SHOW_DYN_LIGHT) {
		tFaceColor *psc = AvgSgmColor (pParticle->nSegment, NULL);
		if (psc->index == gameStates.render.nFrameFlipFlop + 1) {
			pc.red *= (double) psc->color.red;
			pc.green *= (double) psc->color.green;
			pc.blue *= (double) psc->color.blue;
			}
		}
	brightness *= 0.9 + (double) (rand () % 1000) / 5000.0;
	pc.red *= brightness;
	pc.green *= brightness;
	pc.blue *= brightness;
	}
#if OGL_POINT_SPRITES
if (bPointSprites) {
#	if OGL_VERTEX_ARRAYS
	if (gameStates.render.bVertexArrays) {
#		if !EXTRA_VERTEX_ARRAYS
		pParticle->glPos.p.x = f2fl (hp.x);
		pParticle->glPos.p.y = f2fl (hp.y);
		pParticle->glPos.p.z = f2fl (hp.z);
		nBuffer++;
#		else
		memcpy (colorBuffer [iBuffer], &pc, sizeof (pc));
		pf = vertexBuffer [iBuffer];
		pf [0] = f2fl (hp.p.x);
		pf [1] = f2fl (hp.p.y);
		pf [2] = f2fl (hp.p.z);
		memcpy (colorBuffer [iBuffer++], &pc, sizeof (pc));
		if (iBuffer >= VERT_BUF_SIZE)
			FlushVertexArrays ();
#	endif
		}
	else
#endif
	{
		double	r = 0.9 + (double) (rand () % 1000) / 5000.0;

	pc.red *= r;
	pc.green *= r;
	pc.blue *= r;
	glColor4dv ((GLdouble *) &pc);
	glVertex3d (f2fl (hp.p.x), f2fl (hp.p.y), f2fl (hp.p.z));
	}
	}
else
#endif
	{
	if (gameOpts->render.smoke.bDisperse && !nType) {
		decay = sqrt (decay);
		w = f2fl (pParticle->nWidth) / decay;
		h = f2fl (pParticle->nHeight) / decay;
		}
	else {
		w = f2fl (pParticle->nWidth) * decay;
		h = f2fl (pParticle->nHeight) * decay;
		}
	x = f2fl (hp.p.x);
	y = f2fl (hp.p.y);
	z = f2fl (hp.p.z);
#if OGL_VERTEX_ARRAYS
	if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort && (gameOpts->render.bDepthSort <= 0)) {
		pf = vertexBuffer [iBuffer];
		pf [0] =
		pf [9] = x - w; 
		pf [3] =
		pf [6] = x + w; 
		pf [1] = 
		pf [4] = y + h; 
		pf [7] =
		pf [10] = y - h; 
		pf [2] =
		pf [5] =
		pf [8] =
		pf [11] = z;
		i = pParticle->nOrient; 
		texCoord [0].v.u =
		texCoord [3].v.u = u;
		texCoord [0].v.v = 
		texCoord [1].v.v = v;
		texCoord [1].v.u =
		texCoord [2].v.u = u + d;
		texCoord [2].v.v =
		texCoord [3].v.v = v + d;
		SetParticleTexCoord (texCoord [i].v.u, texCoord [i].v.v);
		memcpy (colorBuffer [iBuffer++], &pc, sizeof (pc));
		i = ++i % 4;
		SetParticleTexCoord (texCoord [i].v.u, texCoord [i].v.v);
		memcpy (colorBuffer [iBuffer++], &pc, sizeof (pc));
		i = ++i % 4;
		SetParticleTexCoord (texCoord [i].v.u, texCoord [i].v.v);
		memcpy (colorBuffer [iBuffer++], &pc, sizeof (pc));
		i = ++i % 4;
		SetParticleTexCoord (texCoord [i].v.u, texCoord [i].v.v);
		memcpy (colorBuffer [iBuffer++], &pc, sizeof (pc));
		if (iBuffer >= VERT_BUF_SIZE)
			FlushVertexArrays ();
		}
	else {
#else
	{
#endif
	i = pParticle->nOrient; 
	texCoord [0].v.u =
	texCoord [3].v.u = u;
	texCoord [0].v.v = 
	texCoord [1].v.v = v;
	texCoord [1].v.u =
	texCoord [2].v.u = u + d;
	texCoord [2].v.v =
	texCoord [3].v.v = v + d;
	glColor4dv ((GLdouble *) &pc);
	glTexCoord2dv ((GLdouble *) (texCoord + i));
	glVertex3d (x - w, y + h, z);
	glTexCoord2dv ((GLdouble *) (texCoord + (i + 1) % 4));
	glVertex3d (x + w, y + h, z);
	glTexCoord2dv ((GLdouble *) (texCoord + (i + 2) % 4));
	glVertex3d (x + w, y - h, z);
	glTexCoord2dv ((GLdouble *) (texCoord + (i + 3) % 4));
	glVertex3d (x - w, y - h, z);
	if (gameData.smoke.bAnimate)
		pParticle->nFrame = (pParticle->nFrame + 1) % (nFrames * nFrames);
	}
	}
if (gameOpts->render.bDepthSort > 0)
	glEnd ();
return 1;
}

//------------------------------------------------------------------------------

int BeginRenderSmoke (int nType, float nScale)
{
	grsBitmap	*bmP;
	static time_t	t0 = 0;

if (gameOpts->render.bDepthSort <= 0) {
	nType = (nType % PARTICLE_TYPES == PARTICLE_TYPES - 1);
	if ((nType >= 0) && !gameOpts->render.smoke.bSort)
		AnimateParticle (nType);
	bmP = bmpParticle [gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort][nType];
	OglActiveTexture (GL_TEXTURE0_ARB, 0);
	glEnable (GL_TEXTURE_2D);
	gameData.smoke.bStencil = StencilOff ();
	glDepthFunc (GL_LESS);
	glDepthMask (0);
#if 0
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
#if OGL_POINT_SPRITES
	if (gameStates.render.bPointSprites && !gameOpts->render.smoke.bSort) {
		float quadratic [] = {1.0f, 0.0f, 0.01f};
		float partSize = f2fl ((gameOpts->render.smoke.bSyncSizes ? 
										PARTICLE_SIZE (gameOpts->render.smoke.nSize [0], nScale) : nScale)) * 256; 
		static float maxSize = -1.0f;
		glPointParameterfvARB (GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
		glPointParameterfARB (GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f);
		glTexEnvf (GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable (GL_POINT_SPRITE_ARB);
		if (maxSize < 0)
			glGetFloatv (GL_POINT_SIZE_MAX_ARB, &maxSize);
		if (maxSize != partSize)
			maxSize = partSize;
		glPointSize (maxSize);
		glPointParameterfARB (GL_POINT_SIZE_MIN_ARB, 1.0f);
		glPointParameterfARB (GL_POINT_SIZE_MAX_ARB, 100 * maxSize);
		if ((nType >= 0) && OglBindBmTex (bmP, 0, 1))
			return 0;
#	if OGL_VERTEX_ARRAYS
		if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
#	ifdef _DEBUG
			GLenum i;
#	endif
			glEnableClientState (GL_VERTEX_ARRAY);
#	ifdef _DEBUG
			if ((i = glGetError ())) 
				HUDMessage (0, "glEnableClientState (GL_VERTEX_ARRAY) %d failed (%d)", iBuffer, i);
#	endif
			glEnableClientState (GL_COLOR_ARRAY);
#	ifdef _DEBUG
			if ((i = glGetError ())) 
				HUDMessage (0, "glEnableClientState (GL_COLOR_ARRAY) %d failed (%d)", iBuffer, i);
#	endif
			}
		else
#	endif
			glBegin (GL_POINTS);
		}
	else
#endif
		{
		if ((nType >= 0) && OglBindBmTex (bmP, 0, 1))
			return 0;
#if OGL_VERTEX_ARRAYS
		if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
#ifdef _DEBUG
			GLenum i;
#endif
			glDisableClientState (GL_VERTEX_ARRAY);
			glEnableClientState (GL_VERTEX_ARRAY);
#ifdef _DEBUG
			if ((i = glGetError ())) 
				HUDMessage (0, "glEnableClientState (GL_VERTEX_ARRAY) failed (%d)", i);
#endif
			glEnableClientState (GL_TEXTURE_COORD_ARRAY);
#ifdef _DEBUG
			if ((i = glGetError ())) 
				HUDMessage (0, "glEnableClientState (GL_TEX_COORD_ARRAY) failed (%d)", i);
#endif
			glEnableClientState (GL_COLOR_ARRAY);
#ifdef _DEBUG
			if ((i = glGetError ())) 
				HUDMessage (0, "glEnableClientState (GL_COLOR_ARRAY) failed (%d)", i);
#endif
			}
		else
#endif
			if (nType >= 0)
				glBegin (GL_QUADS);
		}
	}
gameData.smoke.nLastType = -1;
if (gameStates.app.nSDLTicks - t0 < 33) 
	gameData.smoke.bAnimate = 0;
else {
	t0 = gameStates.app.nSDLTicks;
	gameData.smoke.bAnimate = 1;
	}
return 1;
}

//------------------------------------------------------------------------------

int EndRenderSmoke (tCloud *pCloud)
{
if (gameOpts->render.bDepthSort <= 0) {
#if OGL_VERTEX_ARRAYS
	if (pCloud && gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
#	if EXTRA_VERTEX_ARRAYS
		FlushVertexArrays ();
#	else
		FlushVertexArrays (pCloud->pParticles + iBuffer, nBuffer - iBuffer);
#	endif
		glDisableClientState (GL_COLOR_ARRAY);
		glDisableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		}
	else
		glEnd ();
#else
	glEnd ();
#endif
#if OGL_POINT_SPRITES
	if (gameStates.render.bPointSprites) {
		glDisable (GL_POINT_SPRITE_ARB);
		}
#endif
	OGL_BINDTEX (0);
	glDepthMask (1);
	StencilOn (gameData.smoke.bStencil);
	}
return 1;
}

//------------------------------------------------------------------------------

char SmokeObjClass (int nObject)
{
if ((nObject >= 0) && (nObject != 0x7fffffff)) {
	tObject	*objP = OBJECTS + nObject;
	if (objP->nType == OBJ_PLAYER)
		return 1;
	if (objP->nType == OBJ_ROBOT)
		return 2;
	if (objP->nType == OBJ_WEAPON)
		return 3;
	if (objP->nType == OBJ_DEBRIS)
		return 4;
	}
return 0;
}

//------------------------------------------------------------------------------

double CloudBrightness (tCloud *pCloud)
{
	tObject	*objP;

if (pCloud->nObject == 0x7fffffff)
	return 0.5f;
if (pCloud->nType == PARTICLE_TYPES - 1)
	return 1.0f;
if (pCloud->nObject < 0)
	return pCloud->brightness;
if (pCloud->nObjType == OBJ_EFFECT)
	return (double) pCloud->nDefBrightness / 100.0;
if (pCloud->nObjType == OBJ_DEBRIS)
	return 0.5;
if ((pCloud->nObjType == OBJ_WEAPON) && (pCloud->nObjId == PROXMINE_ID))
	return 0.2;
objP = OBJECTS + pCloud->nObject;
if ((objP->nType != pCloud->nObjType) || (objP->flags & (OF_EXPLODING | OF_SHOULD_BE_DEAD | OF_DESTROYED | OF_ARMAGEDDON)))
	return pCloud->brightness;
return pCloud->brightness = (double) ObjectDamage (objP) * 0.5 + 0.1;
}

//------------------------------------------------------------------------------

int CreateCloud (tCloud *pCloud, vmsVector *pPos, vmsVector *pDir,
					  short nSegment, short nObject, int nMaxParts, float nPartScale, 
					  int nDensity, int nPartsPerPos, int nLife, int nSpeed, char nType, 
					  tRgbaColord *pColor, int nCurTime, int bBlowUpParts, vmsVector *vEmittingFace)
{
if (!(pCloud->pParticles = (tParticle *) D2_ALLOC (nMaxParts * sizeof (tParticle))))
	return 0;
#if SORT_CLOUD_PARTS
if (gameOpts->render.smoke.bSort) {
	if (!(pCloud->pPartIdx = (tPartIdx *) D2_ALLOC (nMaxParts * sizeof (tPartIdx))))
		gameOpts->render.smoke.bSort = 0;
	}
else
#endif
	pCloud->pPartIdx = NULL;
pCloud->nLife = nLife;
pCloud->nBirth = nCurTime;
pCloud->nSpeed = nSpeed;
pCloud->nType = nType;
if (pCloud->bHaveColor = (pColor != NULL))
	pCloud->color = *pColor;
if ((pCloud->bHaveDir = (pDir != NULL)))
	pCloud->dir = *pDir;
pCloud->prevPos =
pCloud->pos = *pPos;
pCloud->bHavePrevPos = 1;
pCloud->bBlowUpParts = bBlowUpParts;
pCloud->nParts = 0;
pCloud->nMoved = nCurTime;
pCloud->nPartLimit =
pCloud->nMaxParts = nMaxParts;
pCloud->nFirstPart = 0;
pCloud->nPartScale = nPartScale;
pCloud->nDensity = nDensity;
pCloud->nPartsPerPos = nPartsPerPos;
pCloud->nSegment = nSegment;
pCloud->nObject = nObject;
if ((nObject >= 0) && (nObject != 0x7fffffff)) {
	pCloud->nObjType = gameData.objs.objects [nObject].nType;
	pCloud->nObjId = gameData.objs.objects [nObject].id;
	}
pCloud->nClass = SmokeObjClass (nObject);
pCloud->fPartsPerTick = (float) nMaxParts / (float) abs (nLife);
pCloud->nTicks = 0;
pCloud->nDefBrightness = 0;
if (pCloud->bEmittingFace = (vEmittingFace != NULL))
	memcpy (pCloud->vEmittingFace, vEmittingFace, sizeof (pCloud->vEmittingFace));
pCloud->brightness = (nObject < 0) ? 0.5 : CloudBrightness (pCloud);
return 1;
}

//------------------------------------------------------------------------------

int DestroyCloud (tCloud *pCloud)
{
if (pCloud->pParticles) {
	D2_FREE (pCloud->pParticles);
	pCloud->pParticles = NULL;
	D2_FREE (pCloud->pPartIdx);
	pCloud->pPartIdx = NULL;
	}
pCloud->nParts =
pCloud->nMaxParts = 0;
return 1;
}

//------------------------------------------------------------------------------

inline int CloudLives (tCloud *pCloud, int nCurTime)
{
return (pCloud->nLife < 0) || (pCloud->nBirth + pCloud->nLife > nCurTime);
}

//------------------------------------------------------------------------------

inline int CloudIsDead (tCloud *pCloud, int nCurTime)
{
return !(CloudLives (pCloud, nCurTime) || pCloud->nParts);
}

//------------------------------------------------------------------------------

#if 0

void CheckCloud (tCloud *pCloud)
{
	int	i, j;

for (i = pCloud->nParts, j = pCloud->nFirstPart; i; i--, j = (j + 1) % pCloud->nPartLimit)
	if (pCloud->pParticles [j].nType < 0)
		j = j;
}

#endif

//------------------------------------------------------------------------------

int UpdateCloud (tCloud *pCloud, int nCurTime)
{
	tCloud		c = *pCloud;
	int			t, h, i, j;
	float			fDist;
	double		dBrightness = CloudBrightness (pCloud);
	vmsVector	vDelta, vPos, *pDir = (pCloud->bHaveDir ? &pCloud->dir : NULL),
					*vEmittingFace = c.bEmittingFace ? c.vEmittingFace : NULL;
	fVector		vDeltaf, vPosf;

#if SMOKE_SLOWMO
t = (int) ((nCurTime - c.nMoved) / gameStates.gameplay.slowmo [0].fSpeed);
#else
t = nCurTime - c.nMoved;
#endif
nPartSeg = -1;
for (i = c.nParts, j = c.nFirstPart; i; i--, j = (j + 1) % c.nPartLimit)
	if (!UpdateParticle (c.pParticles + j, nCurTime)) {
		if (j != c.nFirstPart)
			c.pParticles [j] = c.pParticles [c.nFirstPart];
		c.nFirstPart = (c.nFirstPart + 1) % c.nPartLimit;
		c.nParts--;
		}
c.nTicks += t;
if ((c.nPartsPerPos = (int) (c.fPartsPerTick * c.nTicks)) >= 1) {
	c.nTicks = 0;
	if (CloudLives (pCloud, nCurTime)) {
		fDist = f2fl (VmVecMag (VmVecSub (&vDelta, &c.pos, &c.prevPos)));
		h = c.nPartsPerPos;
		if (h > c.nMaxParts - i)
			h = c.nMaxParts - i;
		if (h <= 0)
			goto funcExit;
		if (c.bHavePrevPos && (fDist > 0)) {
			VmsVecToFloat (&vPosf, &c.prevPos);
			VmsVecToFloat (&vDeltaf, &vDelta);
			vDeltaf.p.x /= (float) h;
			vDeltaf.p.y /= (float) h;
			vDeltaf.p.z /= (float) h;
			}
		else {
			VmsVecToFloat (&vPosf, &c.pos);
			vDeltaf.p.x =
			vDeltaf.p.y =
			vDeltaf.p.z = 0.0f;
			}
		c.nParts += h;
		for (; h; h--, j = (j + 1) % c.nPartLimit) {
			VmVecIncf (&vPosf, &vDeltaf);
			vPos.p.x = (fix) (vPosf.p.x * 65536.0f);
			vPos.p.y = (fix) (vPosf.p.y * 65536.0f);
			vPos.p.z = (fix) (vPosf.p.z * 65536.0f);
			CreateParticle (c.pParticles + j, &vPos, pDir, c.nSegment, c.nLife, 
								 c.nSpeed, c.nType, c.nClass, c.nPartScale, c.bHaveColor ? &c.color : NULL,
								 nCurTime, c.bBlowUpParts, dBrightness, vEmittingFace);
			}
		}
	}

funcExit:

pCloud->bHavePrevPos = 1;
pCloud->nMoved = nCurTime;
pCloud->prevPos = pCloud->pos;
pCloud->nTicks = c.nTicks;
pCloud->nFirstPart = c.nFirstPart;
return pCloud->nParts = c.nParts;
}

//------------------------------------------------------------------------------

#if SORT_CLOUD_PARTS

void QSortParticles (tPartIdx *pPartIdx, int left, int right)
{
	int	l = left, 
			r = right, 
			m = pPartIdx [(l + r) / 2].z;

do {
	while (pPartIdx [l].z > m)
		l++;
	while (pPartIdx [r].z < m)
		r--;
	if (l <= r) {
		if (l < r) {
			tPartIdx h = pPartIdx [l];
			pPartIdx [l] = pPartIdx [r];
			pPartIdx [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortParticles (pPartIdx, l, right);
if (left < r)
	QSortParticles (pPartIdx, left, r);
}

//------------------------------------------------------------------------------

int TransformParticles (tParticle *pParticles, tPartIdx *pPartIdx, int nParts, int nFirstPart, int nPartLimit)
{
	tParticle	*pp;
	tPartIdx		*pi;

for (pp = pParticles + nFirstPart, pi = pPartIdx; nParts; nParts--, pParticles++, nFirstPart++, pp++) {
	if (nFirstPart == nPartLimit) {
		pp = pParticles;
		nFirstPart = 0;
		}
#if 1
	G3TransformPoint (&pp->transPos, &pp->pos, 0);
	if ((pi->z = pp->transPos.p.z) > 0) {
		pi->i = nFirstPart;
		pi++;
		}
#else
	pi->z = VmVecDist (&pp->pos, &viewInfo.pos);
	pi->i = nFirstPart;
	pi++;
#endif
	}
return (int) (pi - pPartIdx);
}

//------------------------------------------------------------------------------

int SortCloudParticles (tParticle *pParticles, tPartIdx *pPartIdx, int nParts, int nFirstPart, int nPartLimit)
{
if (nParts > 1) {
#if 0
	int	h, i, z, nSortedUp = 1, nSortedDown = 1;
#endif
	nParts = TransformParticles (pParticles, pPartIdx, nParts, nFirstPart, nPartLimit);
#if 0
	for (h = nParts, i = nFirstPart; h; h--, i = (i + 1) % nPartLimit) {
		pPartIdx [i].i = i;
		pPartIdx [i].z = pParticles [i].transPos.p.z;
		if (i) {
			if (z > pPartIdx [i].z)
				nSortedUp++;
			else if (z < pPartIdx [i].z)
				nSortedDown++;
			else {
				nSortedUp++;
				nSortedDown++;
				}
			}
		z = pPartIdx [i].z;
		}
	if (nSortedDown >= 9 * nParts / 10)
		return 0;
	if (nSortedUp >= 9 * nParts / 10)
		return 1;
#endif
	if (nParts > 1)
		QSortParticles (pPartIdx, 0, nParts - 1);
	}
return nParts;
}

#endif

//------------------------------------------------------------------------------

int RenderCloud (tCloud *pCloud)
{
	double		brightness = CloudBrightness (pCloud);
	int			nParts = pCloud->nParts, i, j, 
					nFirstPart = pCloud->nFirstPart,
					nPartLimit = pCloud->nPartLimit;
#if SORT_CLOUD_PARTS
	int			bSorted = gameOpts->render.smoke.bSort && (nParts > 1);
	tPartIdx		*pPartIdx;
#endif
	vmsVector	v;

if (gameOpts->render.bDepthSort) {
	for (i = nParts, j = nFirstPart; i; i--, j = (j + 1) % nPartLimit)
		RIAddParticle (pCloud->pParticles + j, brightness);
	return 1;
	}
else {
	if (!BeginRenderSmoke (pCloud->nType, pCloud->nPartScale))
		return 0;
#if SORT_CLOUD_PARTS
	if (bSorted) {
		pPartIdx = pCloud->pPartIdx;
		nParts = SortCloudParticles (pCloud->pParticles, pPartIdx, nParts, nFirstPart, nPartLimit);
		for (i = 0; i < nParts; i++)
			RenderParticle (pCloud->pParticles + pPartIdx [i].i, brightness);
		}
	else
#endif //SORT_CLOUD_PARTS
	if (pCloud->bHavePrevPos && 
		(VmVecDist (&pCloud->pos, &viewInfo.pos) >= VmVecMag (VmVecSub (&v, &pCloud->prevPos, &viewInfo.pos))) &&
		(VmVecDot (&v, &gameData.objs.viewer->position.mOrient.fVec) >= 0)) {	//emitter moving away and facing towards emitter
		for (i = nParts, j = (nFirstPart + nParts) % nPartLimit; i; i--) {
			if (!j)
				j = nPartLimit;
#if OGL_VERTEX_ARRAYS && !EXTRA_VERTEX_ARRAYS
			if (!RenderParticle (pCloud->pParticles + --j))
				if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
					FlushVertexArrays (pCloud->pParticles + iBuffer, nBuffer - iBuffer);
					nBuffer = iBuffer;
					}
#else
			RenderParticle (pCloud->pParticles + --j, brightness);
#endif
			}
		}
	else {
		for (i = nParts, j = nFirstPart; i; i--, j = (j + 1) % nPartLimit) {
#if OGL_VERTEX_ARRAYS && !EXTRA_VERTEX_ARRAYS
			if (!RenderParticle (pCloud->pParticles + j, brightness))
				if (gameStates.render.bVertexArrays && !gameOpts->render.smoke.bSort) {
					FlushVertexArrays (pCloud->pParticles + iBuffer, nBuffer - iBuffer);
					nBuffer = iBuffer;
					}
#else
			RenderParticle (pCloud->pParticles + j, brightness);
#endif
			}
		}
	return EndRenderSmoke (pCloud);
	}
}

//------------------------------------------------------------------------------

void SetCloudPos (tCloud *pCloud, vmsVector *pos)
{
int nNewSeg = FindSegByPoint (pos, pCloud->nSegment, 1);
pCloud->pos = *pos;
if (nNewSeg >= 0)
	pCloud->nSegment = nNewSeg;
}

//------------------------------------------------------------------------------

void SetCloudDir (tCloud *pCloud, vmsVector *pDir)
{
if ((pCloud->bHaveDir = (pDir != NULL)))
	pCloud->dir = *pDir;
}

//------------------------------------------------------------------------------

void SetCloudLife (tCloud *pCloud, int nLife)
{
pCloud->nLife = nLife;
pCloud->fPartsPerTick = (float) pCloud->nMaxParts / (float) abs (nLife);
pCloud->nTicks = 0;
}

//------------------------------------------------------------------------------

void SetCloudBrightness (tCloud *pCloud, int nBrightness)
{
pCloud->nDefBrightness = nBrightness;
}

//------------------------------------------------------------------------------

void SetCloudSpeed (tCloud *pCloud, int nSpeed)
{
pCloud->nSpeed = nSpeed;
}

//------------------------------------------------------------------------------

void SetCloudType (tCloud *pCloud, int nType)
{
pCloud->nType = nType;
}

//------------------------------------------------------------------------------

int SetCloudDensity (tCloud *pCloud, int nMaxParts, int nDensity)
{
	tParticle	*p;
	int			h;

if (pCloud->nMaxParts == nMaxParts)
	return 1;
if (nMaxParts > pCloud->nPartLimit) {
	if (!(p = D2_ALLOC (nMaxParts * sizeof (tParticle))))
		return 0;
	if (pCloud->pParticles) {
		if (pCloud->nParts > nMaxParts)
			pCloud->nParts = nMaxParts;
		h = pCloud->nPartLimit - pCloud->nFirstPart;
		if (h > pCloud->nParts)
			h = pCloud->nParts;
		memcpy (p, pCloud->pParticles + pCloud->nFirstPart, h * sizeof (tParticle));
		if (h < pCloud->nParts)
			memcpy (p + h, pCloud->pParticles, (pCloud->nParts - h) * sizeof (tParticle));
		pCloud->nFirstPart = 0;
		pCloud->nPartLimit = nMaxParts;
		D2_FREE (pCloud->pParticles);
		}
	pCloud->pParticles = p;
#if SORT_CLOUD_PARTS
	if (gameOpts->render.smoke.bSort) {
		D2_FREE (pCloud->pPartIdx);
		if (!(pCloud->pPartIdx = (tPartIdx *) D2_ALLOC (nMaxParts * sizeof (tPartIdx))))
			gameOpts->render.smoke.bSort = 0;
		}
#endif
	}
pCloud->nDensity = nDensity;
pCloud->nMaxParts = nMaxParts;
#if 0
if (pCloud->nParts > nMaxParts)
	pCloud->nParts = nMaxParts;
#endif
pCloud->fPartsPerTick = (float) pCloud->nMaxParts / (float) abs (pCloud->nLife);
return 1;
}

//------------------------------------------------------------------------------

void SetCloudPartScale (tCloud *pCloud, float nPartScale)
{
pCloud->nPartScale = nPartScale;
}

//------------------------------------------------------------------------------

int IsUsedSmoke (int iSmoke)
{
	int	i;

for (i = gameData.smoke.iUsed; i >= 0; i = gameData.smoke.buffer [i].nNext)
	if (iSmoke == i)
		return 1;
return 0;
}

//------------------------------------------------------------------------------

int RemoveCloud (int iSmoke, int iCloud)
{
	tSmoke	*pSmoke;

if (!IsUsedSmoke (iSmoke))
	return -1;
pSmoke = gameData.smoke.buffer + iSmoke;
if ((pSmoke->pClouds) && (iCloud < pSmoke->nClouds)) {
	DestroyCloud (pSmoke->pClouds + iCloud);
	if (iCloud < --(pSmoke->nClouds))
		pSmoke->pClouds [iCloud] = pSmoke->pClouds [pSmoke->nClouds];
	}
return pSmoke->nClouds;
}

//------------------------------------------------------------------------------

tSmoke *PrevSmoke (int iSmoke)
{
	int	i, j;

for (i = gameData.smoke.iUsed; i >= 0; i = j)
	if ((j = gameData.smoke.buffer [i].nNext) == iSmoke)
		return gameData.smoke.buffer + i;
return NULL;
}

//------------------------------------------------------------------------------

int DestroySmoke (int iSmoke)
{
	int		i;
	tSmoke	*pSmoke;

if (iSmoke < 0)
	iSmoke = -iSmoke - 1;
else if (!IsUsedSmoke (iSmoke))
	return -1;
pSmoke = gameData.smoke.buffer + iSmoke;
if (gameData.smoke.objects && (pSmoke->nObject >= 0))
	SetSmokeObject (pSmoke->nObject, -1);
if (pSmoke->pClouds) {
	for (i = pSmoke->nClouds; i; )
		DestroyCloud (pSmoke->pClouds + --i);
	D2_FREE (pSmoke->pClouds);
	i = pSmoke->nNext;
	if (gameData.smoke.iUsed == iSmoke)
		gameData.smoke.iUsed = i;
	pSmoke->nNext = gameData.smoke.iFree;
	if ((pSmoke = PrevSmoke (iSmoke)))
		pSmoke->nNext = i;
	gameData.smoke.iFree = iSmoke;
	}
return iSmoke;
}

//------------------------------------------------------------------------------

int DestroyAllSmoke (void)
{
	int	i, j;

for (i = gameData.smoke.iUsed; i >= 0; i = j) {
	j = gameData.smoke.buffer [i].nNext;
	DestroySmoke (-i - 1);
	}
FreeParticleImages ();
FreePartList ();
InitSmoke ();
return 1;
}

//------------------------------------------------------------------------------

int CreateSmoke (vmsVector *pPos, vmsVector *pDir, short nSegment, int nMaxClouds, int nMaxParts, 
					  float nPartScale, int nDensity, int nPartsPerPos, int nLife, int nSpeed, char nType, 
					  int nObject, tRgbaColord *pColor, int bBlowUpParts, char nSide)
{
#if 0
if (!(EGI_FLAG (bUseSmoke, 0, 1, 0)))
	return 0;
else 
#endif
if (gameData.smoke.iFree < 0)
	return -1;
else if (!LoadParticleImage (nType)) {
	//LogErr ("cannot create gameData.smoke.buffer\n");
	return -1;
	}
else {
		int			i, t = gameStates.app.nSDLTicks;
		tSmoke		*pSmoke;
		vmsVector	vEmittingFace [4];	

	if (nSide >= 0)
		GetSideVerts (vEmittingFace, nSegment, nSide);
	nMaxParts = MAX_PARTICLES (nMaxParts, gameOpts->render.smoke.nDens [0]);
	if (gameStates.render.bPointSprites)
		nMaxParts *= 2;
	srand (SDL_GetTicks ());
	pSmoke = gameData.smoke.buffer + gameData.smoke.iFree;
	if (!(pSmoke->pClouds = (tCloud *) D2_ALLOC (nMaxClouds * sizeof (tCloud)))) {
		//LogErr ("cannot create gameData.smoke.buffer\n");
		return 0;
		}
	if ((pSmoke->nObject = nObject) != 0x7fffffff) {
		pSmoke->nSignature = gameData.objs.objects [nObject].nSignature;
		pSmoke->nObjType = gameData.objs.objects [nObject].nType;
		pSmoke->nObjId = gameData.objs.objects [nObject].id;
		}
	pSmoke->nClouds = 0;
	pSmoke->nBirth = t;
	pSmoke->nMaxClouds = nMaxClouds;
	for (i = 0; i < nMaxClouds; i++)
		if (CreateCloud (pSmoke->pClouds + i, pPos, pDir, nSegment, nObject, nMaxParts, nPartScale, nDensity, 
							  nPartsPerPos, nLife, nSpeed, nType, pColor, t, bBlowUpParts, (nSide < 0) ? NULL : vEmittingFace))
			pSmoke->nClouds++;
		else {
			DestroySmoke (gameData.smoke.iFree);
			//LogErr ("cannot create gameData.smoke.buffer\n");
			return -1;
			}
	pSmoke->nType = nType;
	i = gameData.smoke.iFree;
	gameData.smoke.iFree = pSmoke->nNext;
	pSmoke->nNext = gameData.smoke.iUsed;
	gameData.smoke.iUsed = i;
	//LogErr ("CreateSmoke (%d) = %d,%d (%d)\n", nObject, i, nMaxClouds, nType);
	return gameData.smoke.iUsed;
	}
}

//------------------------------------------------------------------------------

int UpdateSmoke (void)
{
#if SMOKE_SLOWMO
	static int	t0 = 0;
	int t = gameStates.app.nSDLTicks - t0;

if (t / gameStates.gameplay.slowmo [0].fSpeed < 25)
	return 0;
t0 += (int) (gameStates.gameplay.slowmo [0].fSpeed * 25);
#else
if (!gameStates.app.tick40fps.bTick)
	return 0;
#endif
#if 0
if (!EGI_FLAG (bUseSmoke, 0, 1, 0))
	return 0;
else 
#endif
{
		int		h, i, n, j = 0, t = gameStates.app.nSDLTicks;
		tSmoke	*pSmoke;
		tCloud	*pCloud;

	for (i = gameData.smoke.iUsed; i >= 0; i = n) {
		pSmoke = gameData.smoke.buffer + i;
		n = pSmoke->nNext;
#if 0
		if ((pSmoke->nObject != 0x7fffffff) && (pSmoke->nSignature != gameData.objs.objects [pSmoke->nObject].nSignature)) {
			SetSmokeLife (i, 0);
			//continue;
			}
#endif
		if ((pSmoke->nObject == 0x7fffffff) && (gameStates.app.nSDLTicks - pSmoke->nBirth > (MAX_SHRAPNEL_LIFE / F1_0) * 1000))
			SetSmokeLife (i, 0);
#ifdef _DEBUG
		if ((pSmoke->nObject != 0x7fffffff) && (gameData.objs.objects [pSmoke->nObject].nType == 255))
			i = i;
#endif
		if ((pCloud = pSmoke->pClouds))
			for (h = j = 0; j < pSmoke->nClouds; ) {
				if (!pSmoke->pClouds)
					return 0;
				if (CloudIsDead (pCloud, t)) {
					if (!RemoveCloud (i, j)) {
						//LogErr ("killing gameData.smoke.buffer %d (%d)\n", i, pSmoke->nObject);
						DestroySmoke (i);
						break;
						}
					}
				else {
					//LogErr ("moving %d (%d)\n", i, pSmoke->nObject);
					if ((pSmoke->nObject < 0) || ((pSmoke->nObject != 0x7fffffff) && (gameData.objs.objects [pSmoke->nObject].nType == 255)))
						SetCloudLife (pCloud, 0);
					if (UpdateCloud (pCloud, t))
						h++;
					pCloud++, j++;
					}
				}
			}
	return j;
	}
}

//------------------------------------------------------------------------------

typedef struct tCloudList {
	tCloud		*pCloud;
	fix			xDist;
} tCloudList;

tCloudList *pCloudList = NULL;

//------------------------------------------------------------------------------

void QSortClouds (int left, int right)
{
	int	l = left, 
			r = right; 
	fix	m = pCloudList [(l + r) / 2].xDist;

do {
	while (pCloudList [l].xDist > m)
		l++;
	while (pCloudList [r].xDist < m)
		r--;
	if (l <= r) {
		if (l < r) {
			tCloudList h = pCloudList [l];
			pCloudList [l] = pCloudList [r];
			pCloudList [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortClouds (l, right);
if (left < r)
	QSortClouds (left, r);
}

//------------------------------------------------------------------------------

int CloudCount (void)
{
	int		i, j;
	tSmoke	*pSmoke = gameData.smoke.buffer;

for (i = gameData.smoke.iUsed, j = 0; i >= 0; i = pSmoke->nNext) {
	pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds) {
		j += pSmoke->nClouds;
		if ((pSmoke->nObject != 0x7fffffff) && (gameData.smoke.objects [pSmoke->nObject] < 0))
			SetSmokeLife (i, 0);
		}
	}
return j;
}

//------------------------------------------------------------------------------

int CreateCloudList (void)
{
	int			h, i, j, nClouds;
	tSmoke		*pSmoke = gameData.smoke.buffer;
	tCloud		*pCloud;
	double		brightness;

h = CloudCount ();
if (!h)
	return 0;
if (!(pCloudList = D2_ALLOC (h * sizeof (tCloudList))))
	return -1;
for (i = gameData.smoke.iUsed, nClouds = 0; i >= 0; i = pSmoke->nNext) {
	pSmoke = gameData.smoke.buffer + i;
	if (!LoadParticleImage (pSmoke->nType)) {
		D2_FREE (pCloudList);
		return 0;
		}
	if (pSmoke->pClouds) {
		for (j = pSmoke->nClouds, pCloud = pSmoke->pClouds; j; j--, pCloud++) {
			if (pCloud->nParts > 0) {
				brightness = CloudBrightness (pCloud);
				pCloud->brightness = pCloud->nDefBrightness ? (double) pCloud->nDefBrightness / 100.0 : brightness;
				pCloudList [nClouds].pCloud = pCloud;
#if SORT_CLOUDS
#	if 1	// use the closest point on a line from first to last particle to the viewer
				pCloudList [nClouds++].xDist = VmLinePointDist (&pCloud->pParticles [pCloud->nFirstPart].pos, 
																				&pCloud->pParticles [(pCloud->nFirstPart + pCloud->nParts - 1) % pCloud->nPartLimit].pos,
																				&viewInfo.pos);
#	else	// use distance of the current emitter position to the viewer
				pCloudList [nClouds++].xDist = VmVecDist (&pCloud->pos, &viewInfo.pos);
#	endif
#endif
				}
			}
		}
	}
#if SORT_CLOUDS
if (nClouds > 1)
	QSortClouds (0, nClouds - 1);
#endif
return nClouds;
}

//------------------------------------------------------------------------------

int ParticleCount (void)
{
	int			i, j, nParts, nFirstPart, nPartLimit, z;
	tSmoke		*pSmoke = gameData.smoke.buffer;
	tCloud		*pCloud;
	tParticle	*pParticle;

#if 1
gameData.smoke.depthBuf.zMin = gameData.render.zMin;
gameData.smoke.depthBuf.zMax = gameData.render.zMax;
#else
gameData.smoke.depthBuf.zMin = 0x7fffffff;
gameData.smoke.depthBuf.zMax = -0x7fffffff;
#endif
gameData.smoke.depthBuf.nParts = 0;
for (i = gameData.smoke.iUsed; i >= 0; i = pSmoke->nNext) {
	pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds && (j = pSmoke->nClouds)) {
		for (pCloud = pSmoke->pClouds; j; j--, pCloud++) {
			if (nParts = pCloud->nParts) {
				nFirstPart = pCloud->nFirstPart;
				nPartLimit = pCloud->nPartLimit;
				for (pParticle = pCloud->pParticles + nFirstPart; nParts; nParts--, nFirstPart++, pParticle++) {
					if (nFirstPart == nPartLimit) {
						nFirstPart = 0;
						pParticle = pCloud->pParticles;
						}
					G3TransformPoint (&pParticle->transPos, &pParticle->pos, 0);
					z = pParticle->transPos.p.z;
#if 0
					if ((z < gameData.render.zMin) || (z > gameData.render.zMax))
						continue;
#else
					if (z < 0)
						continue;
					gameData.smoke.depthBuf.nParts++;
					if (gameData.smoke.depthBuf.zMin > z)
						gameData.smoke.depthBuf.zMin = z;
					if (gameData.smoke.depthBuf.zMax < z)
						gameData.smoke.depthBuf.zMax = z;
#endif
					}
				}
			}
		if ((pSmoke->nObject != 0x7fffffff) && (gameData.smoke.objects [pSmoke->nObject] < 0))
			SetSmokeLife (i, 0);
		}
	}
return gameData.smoke.depthBuf.nParts;
}

//------------------------------------------------------------------------------

void DepthSortParticles (void)
{
	int			i, j, z, nParts, nFirstPart, nPartLimit, bSort;
	tSmoke		*pSmoke = gameData.smoke.buffer;
	tCloud		*pCloud;
	tParticle	*pParticle;
	tPartList	*ph, *pi, *pj, **pd;
	float			fBrightness;
	double		zScale;

bSort = (gameOpts->render.smoke.bSort > 1);
zScale = (double) (PART_DEPTHBUFFER_SIZE - 1) / (double) (gameData.smoke.depthBuf.zMax - gameData.smoke.depthBuf.zMin);
if (zScale > 1)
	zScale = 1;
//ResetDepthBuf ();
for (i = gameData.smoke.iUsed; i >= 0; i = pSmoke->nNext) {
	pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds && (j = pSmoke->nClouds)) {
		for (pCloud = pSmoke->pClouds; j; j--, pCloud++) {
			if (nParts = pCloud->nParts) {
				fBrightness = (float) CloudBrightness (pCloud);
				nFirstPart = pCloud->nFirstPart;
				nPartLimit = pCloud->nPartLimit;
				for (pParticle = pCloud->pParticles + nFirstPart; nParts; nParts--, nFirstPart++, pParticle++) {
					if (nFirstPart == nPartLimit) {
						nFirstPart = 0;
						pParticle = pCloud->pParticles;
						}
					z = pParticle->transPos.p.z;
#if 1
					if ((z < F1_0) || (z > gameData.render.zMax))
						continue;
#else
					if (z < 0)
						continue;
#endif
					pd = gameData.smoke.depthBuf.pDepthBuffer + (int) ((double) (z - gameData.smoke.depthBuf.zMin) * zScale);
					// find the first particle to insert the new one *before* and place in pj; pi will be it's predecessor (NULL if to insert at list start)
					ph = gameData.smoke.depthBuf.pPartList + --gameData.smoke.depthBuf.nFreeParts;
					ph->pParticle = pParticle;
					ph->fBrightness = fBrightness;
					if (bSort) {
						for (pi = NULL, pj = *pd; pj && (pj->pParticle->transPos.p.z > z); pj = pj->pNextPart)
							pi = pj;
						if (pi) {
							ph->pNextPart = pi->pNextPart;
							pi->pNextPart = ph;
							}
						else {
							ph->pNextPart = *pd;
							*pd = ph;
							}
						}
					else {
						ph->pNextPart = *pd;
						*pd = ph;
						}
					if (!gameData.smoke.depthBuf.nFreeParts)
						return;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

int RenderParticles (void)
{
	int	h;
	struct tPartList	**pd, *pl, *pn;

if (!AllocPartList ())
	return 0;
pl = gameData.smoke.depthBuf.pPartList + 999999;
pl->pParticle = NULL;
if (!(h = ParticleCount ()))
	return 0;
pl = gameData.smoke.depthBuf.pPartList + 999999;
pl->pParticle = NULL;
DepthSortParticles ();
if (!LoadParticleImages ())
	return 0;
BeginRenderSmoke (-1, 1);
for (pd = gameData.smoke.depthBuf.pDepthBuffer + PART_DEPTHBUFFER_SIZE - 1; 
	  pd >= gameData.smoke.depthBuf.pDepthBuffer; 
	  pd--) {
		if (pl = *pd) {
		do {
			RenderParticle (pl->pParticle, pl->fBrightness);
			pn = pl->pNextPart;
			pl->pNextPart = NULL;
			pl = pn;
			} while (pl);
		*pd = NULL;
		}
	}
gameData.smoke.depthBuf.nFreeParts = PARTLIST_SIZE;
EndRenderSmoke (NULL);
return 1;
}

//------------------------------------------------------------------------------

int RenderClouds (void)
{
	int		h, i, j;

#if !EXTRA_VERTEX_ARRAYS
nBuffer = 0;
#endif
h = (gameOpts->render.bDepthSort > 0) ? -1 : CreateCloudList ();
if (!h)
	return 1;
if (h > 0) {
	do {
		RenderCloud (pCloudList [--h].pCloud);
		} while (h);
	D2_FREE (pCloudList);
	pCloudList = NULL;
	}
else 
	{
	tSmoke *pSmoke = gameData.smoke.buffer;
	tCloud *pCloud;
	for (i = gameData.smoke.iUsed; i >= 0; i = pSmoke->nNext) {
		pSmoke = gameData.smoke.buffer + i;
		if (pSmoke->pClouds) {
			if (!LoadParticleImage (pSmoke->nType))
				return 0;
			if ((pSmoke->nObject >= 0) && (pSmoke->nObject != 0x7fffffff) && (gameData.smoke.objects [pSmoke->nObject] < 0))
				SetSmokeLife (i, 0);
			for (j = pSmoke->nClouds, pCloud = pSmoke->pClouds; j; j--, pCloud++) {
				pCloud->brightness = CloudBrightness (pCloud);
				RenderCloud (pCloud);
				}
			}
		}
	}
return 1;
}

//------------------------------------------------------------------------------

int RenderSmoke (void)
{
return (gameOpts->render.smoke.bSort && (gameOpts->render.bDepthSort <= 0)) ? RenderParticles () : RenderClouds ();
}

//------------------------------------------------------------------------------

void SetSmokePos (int i, vmsVector *pos)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds)
		for (i = 0; i < pSmoke->nClouds; i++)
			SetCloudPos (pSmoke->pClouds, pos);
#ifdef _DEBUG
	else if (pSmoke->nObject >= 0) {
		HUDMessage (0, "no smoke in SetSmokePos (%d,%d)\n", i, pSmoke->nObject);
		//LogErr ("no gameData.smoke.buffer in SetSmokePos (%d,%d)\n", i, pSmoke->nObject);
		pSmoke->nObject = -1;
		}
#endif
	}
}

//------------------------------------------------------------------------------

void SetSmokeDensity (int i, int nMaxParts, int nDensity)
{
nMaxParts = MAX_PARTICLES (nMaxParts, gameOpts->render.smoke.nDens [0]);
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds)
		for (i = 0; i < pSmoke->nClouds; i++)
			SetCloudDensity (pSmoke->pClouds + i, nMaxParts, nDensity);
	}
}

//------------------------------------------------------------------------------

void SetSmokePartScale (int i, float nPartScale)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds)
		for (i = 0; i < pSmoke->nClouds; i++)
			SetCloudPartScale (pSmoke->pClouds + i, nPartScale);
	}
}

//------------------------------------------------------------------------------

void SetSmokeLife (int i, int nLife)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds && (pSmoke->pClouds->nLife != nLife)) {
		//LogErr ("SetSmokeLife (%d,%d) = %d\n", i, pSmoke->nObject, nLife);
		int j;
		for (j = 0; j < pSmoke->nClouds; j++)
			SetCloudLife (pSmoke->pClouds, nLife);
		}
	}
}

//------------------------------------------------------------------------------

void SetSmokeBrightness (int i, int nBrightness)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	if (pSmoke->pClouds && (pSmoke->pClouds->nDefBrightness != nBrightness)) {
		//LogErr ("SetSmokeLife (%d,%d) = %d\n", i, pSmoke->nObject, nLife);
		int j;
		for (j = 0; j < pSmoke->nClouds; j++)
			SetCloudBrightness (pSmoke->pClouds, nBrightness);
		}
	}
}

//------------------------------------------------------------------------------

void SetSmokeType (int i, int nType)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	pSmoke->nType = nType;
	for (i = 0; i < pSmoke->nClouds; i++)
		SetCloudType (pSmoke->pClouds + i, nType);
	}
}

//------------------------------------------------------------------------------

void SetSmokeSpeed (int i, int nSpeed)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	pSmoke->nSpeed = nSpeed;
	for (i = 0; i < pSmoke->nClouds; i++)
		SetCloudSpeed (pSmoke->pClouds + i, nSpeed);
	}
}

//------------------------------------------------------------------------------

void SetSmokeDir (int i, vmsVector *pDir)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	for (i = 0; i < pSmoke->nClouds; i++)
		SetCloudDir (pSmoke->pClouds + i, pDir);
	}
}

//------------------------------------------------------------------------------

int GetSmokeType (int i)
{
return (IsUsedSmoke (i)) ? gameData.smoke.buffer [i].nType : -1;
}

//------------------------------------------------------------------------------

tCloud *GetCloud (int i, int j)
{
if (IsUsedSmoke (i)) {
	tSmoke *pSmoke = gameData.smoke.buffer + i;
	return (pSmoke->pClouds && (j < pSmoke->nClouds)) ? pSmoke->pClouds + j : NULL;
	}
else
	return NULL;
}

//------------------------------------------------------------------------------

int MaxParticles (int nParts, int nDens)
{
return ((nParts < 0) ? -nParts : nParts * (nDens + 1)); //(int) (nParts * pow (1.2, nDens));
}

//------------------------------------------------------------------------------

float ParticleSize (int nSize, float nScale)
{
if (gameOpts->render.smoke.bDisperse)
	return (float) (PARTICLE_RAD * (nSize + 1)) / nScale + 0.5f;
else
	return ((float) (PARTICLE_RAD << nSize)) / nScale + 0.5f;
}

//------------------------------------------------------------------------------

int SetSmokeObject (int nObject, int nSmoke)
{
if ((nObject < 0) || (nObject >= MAX_OBJECTS))
	return -1;
return gameData.smoke.objects [nObject] = nSmoke;
}

//------------------------------------------------------------------------------
//eof
