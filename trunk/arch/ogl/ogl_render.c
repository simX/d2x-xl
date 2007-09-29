/* $Id: ogl.c, v 1.14 2004/05/11 23:15:55 btb Exp $ */
/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <malloc.h>
# include <SDL.h>
#endif

#include "3d.h"
#include "u_mem.h"
#include "piggy.h"
#include "../../3d/globvars.h"
#include "error.h"
#include "texmap.h"
#include "palette.h"
#include "rle.h"
#include "mono.h"

#include "inferno.h"
#include "textures.h"
#include "texmerge.h"
#include "effects.h"
#include "weapon.h"
#include "powerup.h"
#include "polyobj.h"
#include "gamefont.h"
#include "byteswap.h"
#include "cameras.h"
#include "render.h"
#include "grdef.h"
#include "ogl_init.h"
#include "network.h"
#include "lightmap.h"
#include "mouse.h"
#include "gameseg.h"
#include "lighting.h"
#include "input.h"
#include "endlevel.h"

#ifndef M_PI
#	define M_PI 3.141592653589793240
#endif

#define OGL_CLEANUP		1
#define USE_VERTNORMS	1
#define G3_DRAW_ARRAYS	0

#if DBG_SHADOWS
int bShadowTest = 0;
#endif

extern int bZPass;
extern grsBitmap *bmpDeadzone;
int bSingleStencil = 0;

tRgbaColorf shadowColor [2] = {{1.0f, 0.0f, 0.0f, 0.25f}, {0.0f, 0.0f, 1.0f, 0.25f}};
tRgbaColorf modelColor [2] = {{0.0f, 0.5f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f, 0.5f}};

#define COORTRANS 0

#if defined (_WIN32) || defined (__sun__)
#	define cosf(a) cos (a)
#	define sinf(a) sin (a)
#endif

extern int gr_renderstats;
extern int gr_badtexture;

extern tRenderQuality renderQualities [];
	
GLuint hBigSphere=0;
GLuint hSmallSphere=0;
GLuint circleh5=0;
GLuint circleh10=0;
GLuint mouseIndList = 0;
GLuint cross_lh [2]={0, 0};
GLuint primary_lh [3]={0, 0, 0};
GLuint secondary_lh [5]={0, 0, 0, 0, 0};
GLuint glInitTMU [2][4] = {{0, 0, 0, 0},{0, 0, 0, 0}};
GLuint glExitTMU = 0;

int OglBindBmTex (grsBitmap *bm, int szLevelName, int nTransp);
void ogl_clean_texture_cache (void);
/*inline*/ void SetTMapColor (tUVL *uvlList, int i, grsBitmap *bm, int bResetColor, tFaceColor *vertColor);

//------------------------------------------------------------------------------

void OglPalColor (ubyte *palette, int c)
{
	GLfloat	fc [4];

if (c < 0)
	glColor3f (1.0, 1.0, 1.0);
else {
	if (!palette)
		palette = gamePalette;
	if (!palette)
		palette = defaultPalette;
	c *= 3;
	fc [0] = (float) (palette [c]) / 63.0f;
	fc [1] = (float) (palette [c]) / 63.0f;
	fc [2] = (float) (palette [c]) / 63.0f;
	if (gameStates.render.grAlpha >= GR_ACTUAL_FADE_LEVELS)
		fc [3] = 1.0f;
	else {
		fc [3] = (float) gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS; //1.0f - (float) gameStates.render.grAlpha / ((float) GR_ACTUAL_FADE_LEVELS - 1.0f);
		glEnable (GL_BLEND);
		}
	glColor4fv (fc);
	}
}

//------------------------------------------------------------------------------

void OglBlendFunc (GLenum nSrcBlend, GLenum nDestBlend)
{
gameData.render.ogl.nSrcBlend = nSrcBlend;
gameData.render.ogl.nDestBlend = nDestBlend;
glBlendFunc (nSrcBlend, nDestBlend);
}

//------------------------------------------------------------------------------

void OglGrsColor (grsColor *pc)
{
	GLfloat	fc [4];

if (!pc)
	glColor3f (1.0, 1.0, 1.0);
else if (pc->rgb) {
	fc [0] = (float) (pc->color.red) / 255.0f;
	fc [1] = (float) (pc->color.green) / 255.0f;
	fc [2] = (float) (pc->color.blue) / 255.0f;
	fc [3] = (float) (pc->color.alpha) / 255.0f;
	if (fc [3] < 1.0f) {
		glEnable (GL_BLEND);
		glBlendFunc (gameData.render.ogl.nSrcBlend, gameData.render.ogl.nDestBlend);
		}
	glColor4fv (fc);
	}
else
	OglPalColor (gamePalette, pc->index);
}

//------------------------------------------------------------------------------

int r_polyc, r_tpolyc, r_bitmapc, r_ubitmapc, r_ubitbltc, r_upixelc;

bool G3DrawLine (g3sPoint *p0, g3sPoint *p1)
{
glDisable (GL_TEXTURE_2D);
OglGrsColor (&grdCurCanv->cvColor);
glBegin (GL_LINES);
OglVertex3x (p0->p3_vec.p.x, p0->p3_vec.p.y, p0->p3_vec.p.z);
OglVertex3x (p1->p3_vec.p.x, p1->p3_vec.p.y, p1->p3_vec.p.z);
if (grdCurCanv->cvColor.rgb)
	glDisable (GL_BLEND);
glEnd ();
return 1;
}

//------------------------------------------------------------------------------

void OglComputeSinCos (int nSides, tSinCosd *sinCosP)
{
	int 		i;
	double	ang;

for (i = 0; i < nSides; i++, sinCosP++) {
	ang = 2.0 * M_PI * i / nSides;
	sinCosP->dSin = sin (ang);
	sinCosP->dCos = cos (ang);
	}
}

//------------------------------------------------------------------------------

void OglDrawEllipse (int nSides, int nType, double xsc, double xo, double ysc, double yo, tSinCosd *sinCosP)
{
	int i;
	double ang;

glBegin (nType);
if (sinCosP) {
	for (i = 0; i < nSides; i++, sinCosP++)
		glVertex2d (sinCosP->dCos * xsc + xo, sinCosP->dSin * ysc + yo);
	}
else {
	for (i = 0; i < nSides; i++) {
		ang = 2.0 * M_PI * i / nSides;
		glVertex2d (cos (ang) * xsc + xo, sin (ang) * ysc + yo);
		}
	}
glEnd ();
}

//------------------------------------------------------------------------------

void OglDrawCircle (int nSides, int nType)
{
	int i;
	double ang;

glBegin (nType);
for (i = 0; i < nSides; i++) {
	ang = 2.0 * M_PI * i / nSides;
	glVertex2d (cos (ang), sin (ang));
	}
glEnd ();
}

//------------------------------------------------------------------------------

int CircleListInit (int nSides, int nType, int mode) 
{
	int h = glGenLists (1);

glNewList (h, mode);
/* draw a unit radius circle in xy plane centered on origin */
OglDrawCircle (nSides, nType);
glEndList ();
return h;
}

//------------------------------------------------------------------------------

grsBitmap *bmpDeadzone = NULL;
int bHaveDeadzone = 0;

int LoadDeadzone (void)
{
if (!bHaveDeadzone) {
	bmpDeadzone = CreateAndReadTGA ("deadzone.tga");
	bHaveDeadzone = bmpDeadzone ? 1 : -1;
	}
return bHaveDeadzone > 0;
}

//------------------------------------------------------------------------------

void FreeDeadzone (void)
{
if (bmpDeadzone) {
	GrFreeBitmap (bmpDeadzone);
	bmpDeadzone = NULL;
	bHaveDeadzone = 0;
	}
}

//------------------------------------------------------------------------------

void OglDrawMouseIndicator (void)
{
	double 	scale = (double) grdCurScreen->scWidth / (double) grdCurScreen->scHeight;
	
	static tSinCosd sinCos30 [30];
	static tSinCosd sinCos12 [12];
	static int bInitSinCos = 1;

	double	r, w, h;

if (!gameOpts->input.mouse.nDeadzone)
	return;
if (bInitSinCos) {
	OglComputeSinCos (sizeofa (sinCos30), sinCos30);
	OglComputeSinCos (sizeofa (sinCos12), sinCos12);
	bInitSinCos = 0;
	}
glDisable (GL_TEXTURE_2D);
#if 0
if (mouseIndList)
glCallList (mouseIndList);
else {
	glNewList (mouseIndList, GL_COMPILE_AND_EXECUTE);
#endif
	glEnable (GL_SMOOTH);
	glColor4d (1.0, 0.8, 0.0, 0.9);
	glPushMatrix ();
	glTranslated ((double) (mouseData.x) / (double) SWIDTH, 1.0 - (double) (mouseData.y) / (double) SHEIGHT, 0);
	glScaled (scale / 320.0, scale / 200.0, scale);//the positions are based upon the standard reticle at 320x200 res.
	glLineWidth (3);
	OglDrawEllipse (12, GL_LINE_LOOP, 1.5, 0, 1.5 * (double) grdCurScreen->scHeight / (double) grdCurScreen->scWidth, 0, sinCos12);
	glPopMatrix ();
	glPushMatrix ();
	glTranslated (0.5, 0.5, 0);
	if (LoadDeadzone ()) {
		grsColor c = {-1, 1, {255, 255, 255, 128}};
		OglUBitMapMC (0, 0, 16, 16, bmpDeadzone, &c, 1, 0);
		r = CalcDeadzone (0, gameOpts->input.mouse.nDeadzone);
		w = r / (double) grdCurScreen->scWidth;
		h = r / (double) grdCurScreen->scHeight;
		glEnable (GL_TEXTURE_2D);
		if (OglBindBmTex (bmpDeadzone, 1, -1)) 
			return;
		OglTexWrap (bmpDeadzone->glTexture, GL_CLAMP);
		glColor4d (1.0, 1.0, 1.0, 0.8 / gameOpts->input.mouse.nDeadzone);
		glBegin (GL_QUADS);
		glTexCoord2d (0, 0);
		glVertex2d (-w, -h);
		glTexCoord2d (1, 0);
		glVertex2d (w, -h);
		glTexCoord2d (1, 1);
		glVertex2d (w, h);
		glTexCoord2d (0, 1);
		glVertex2d (-w, h);
		glEnd ();
		OGL_BINDTEX (0);
		glDisable (GL_TEXTURE_2D);
		}
	else {
		glScaled (scale / 320.0, scale / 200.0, scale);//the positions are based upon the standard reticle at 320x200 res.
		glColor4d (1.0, 0.8, 0.0, 1.0 / (3.0 + 0.5 * gameOpts->input.mouse.nDeadzone));
		glLineWidth ((GLfloat) (4 + 2 * gameOpts->input.mouse.nDeadzone));
		r = CalcDeadzone (0, gameOpts->input.mouse.nDeadzone) / 4;
		OglDrawEllipse (30, GL_LINE_LOOP, r, 0, r * (double) grdCurScreen->scHeight / (double) grdCurScreen->scWidth, 0, sinCos30);
		}
	glPopMatrix ();
	glDisable (GL_SMOOTH);
	glLineWidth (1);
#if 0
	glEndList ();
   }
#endif
glPopMatrix ();
}

//------------------------------------------------------------------------------

float bright_g [4]={32.0/256, 252.0/256, 32.0/256, 1.0};
float dark_g  [4]={32.0/256, 148.0/256, 32.0/256, 1.0};
float darker_g [4]={32.0/256, 128.0/256, 32.0/256, 1.0};

void OglDrawReticle (int cross, int primary, int secondary)
{
	double scale = (double)nCanvasHeight/ (double)grdCurScreen->scHeight;

	static tSinCosd sinCos8 [8];
	static tSinCosd sinCos12 [12];
	static tSinCosd sinCos16 [16];
	static int bInitSinCos = 1;
	
if (bInitSinCos) {
	OglComputeSinCos (sizeofa (sinCos8), sinCos8);
	OglComputeSinCos (sizeofa (sinCos12), sinCos12);
	OglComputeSinCos (sizeofa (sinCos16), sinCos16);
	bInitSinCos = 0;
	}
glPushMatrix ();
//	glTranslated (0.5, 0.5, 0);
glTranslated (
	(grdCurCanv->cvBitmap.bmProps.w/2+grdCurCanv->cvBitmap.bmProps.x) / (double) gameStates.ogl.nLastW, 
	1.0 - (grdCurCanv->cvBitmap.bmProps.h/ ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 : 2)+grdCurCanv->cvBitmap.bmProps.y)/ (double)gameStates.ogl.nLastH, 
	0);
glScaled (scale/320.0, scale/200.0, scale);//the positions are based upon the standard reticle at 320x200 res.
glDisable (GL_TEXTURE_2D);

glLineWidth (5);
glEnable (GL_SMOOTH);
if (cross_lh [cross])
	glCallList (cross_lh [cross]);
else {
	cross_lh [cross] = glGenLists (1);
	glNewList (cross_lh [cross], GL_COMPILE_AND_EXECUTE);
	glBegin (GL_LINES);
	//cross top left
	glColor3fv (darker_g);
	glVertex2d (-4.0, 4.0);
	glColor3fv (cross ? bright_g : dark_g);
	glVertex2d (-2.0, 2.0);
	//cross bottom left
	glColor3fv (dark_g);
	glVertex2d (-3.0, -2.0);
	if (cross)
		glColor3fv (bright_g);
	glVertex2d (-2.0, -1.0);
	//cross top right
	glColor3fv (darker_g);
	glVertex2d (4.0, 4.0);
	glColor3fv (cross ? bright_g : dark_g);
	glVertex2d (2.0, 2.0);
	//cross bottom right
	glColor3fv (dark_g);
	glVertex2d (3.0, -2.0);
	if (cross)
		glColor3fv (bright_g);
	glVertex2d (2.0, -1.0);
	glEnd ();
	glEndList ();
	}

//	if (nCanvasHeight>200)
//		glLineWidth (nCanvasHeight/ (double)200);
if (primary_lh [primary])
	glCallList (primary_lh [primary]);
else {
	primary_lh [primary] = glGenLists (1);
	glNewList (primary_lh [primary], GL_COMPILE_AND_EXECUTE);
	glColor3fv (dark_g);
	glBegin (GL_LINES);
	//left primary bar
	glVertex2d (-14.0, -8.0);
	glVertex2d (-8.0, -5.0);
	//right primary bar
	glVertex2d (14.0, -8.0);
	glVertex2d (8.0, -5.0);
	glEnd ();
	glColor3fv (primary ? bright_g : dark_g);
	//left upper
	OglDrawEllipse (12, GL_POLYGON, 1.5, -7.0, 1.5, -5.0, sinCos12);
	//right upper
	OglDrawEllipse (12, GL_POLYGON, 1.5, 7.0, 1.5, -5.0, sinCos12);
	glColor3fv ((primary == 2) ? bright_g : dark_g);
	//left lower
	OglDrawEllipse (8, GL_POLYGON, 1.0, -14.0, 1.0, -8.0, sinCos8);
	//right lower
	OglDrawEllipse (8, GL_POLYGON, 1.0, 14.0, 1.0, -8.0, sinCos8);
	glEndList ();
	}
//	if (nCanvasHeight>200)
//		glLineWidth (1);

if (!secondary_lh [secondary])
	glCallList (secondary_lh [secondary]);
else {
	secondary_lh [secondary] = glGenLists (1);
	glNewList (secondary_lh [secondary], GL_COMPILE_AND_EXECUTE);
	if (secondary <= 2) {
		//left secondary
		glColor3fv ((secondary == 1) ? bright_g : darker_g);
		OglDrawEllipse (16, GL_LINE_LOOP, 2.0, -10.0, 2.0, -1.0, sinCos16);
		//right secondary
		glColor3fv ((secondary == 2) ? bright_g : darker_g);
		OglDrawEllipse (16, GL_LINE_LOOP, 2.0, 10.0, 2.0, -1.0, sinCos16);
		}
	else {
		//bottom/middle secondary
		glColor3fv ((secondary == 4) ? bright_g : darker_g);
		OglDrawEllipse (16, GL_LINE_LOOP, 2.0, 0.0, 2.0, -7.0, sinCos16);
		}
	glEndList ();
	}
glDisable (GL_SMOOTH);
glLineWidth (1);
glPopMatrix ();
}

//------------------------------------------------------------------------------

int G3DrawSphere (g3sPoint *pnt, fix rad, int bBigSphere)
{
	double r;

glDisable (GL_TEXTURE_2D);
OglGrsColor (&grdCurCanv->cvColor);
glPushMatrix ();
glTranslatef (f2fl (pnt->p3_vec.p.x), f2fl (pnt->p3_vec.p.y), f2fl (pnt->p3_vec.p.z));
r = f2fl (rad);
glScaled (r, r, r);
if (bBigSphere)
	if (hBigSphere)
		glCallList (hBigSphere);
	else
		hBigSphere = CircleListInit (20, GL_POLYGON, GL_COMPILE_AND_EXECUTE);
else
	if (hSmallSphere)
		glCallList (hSmallSphere);
	else
		hSmallSphere = CircleListInit (12, GL_POLYGON, GL_COMPILE_AND_EXECUTE);
glPopMatrix ();
if (grdCurCanv->cvColor.rgb)
	glDisable (GL_BLEND);
return 0;
}

//------------------------------------------------------------------------------

int GrUCircle (fix xc1, fix yc1, fix r1)
{//dunno if this really works, radar doesn't seem to.. hm..
glDisable (GL_TEXTURE_2D);
//	glPointSize (f2fl (rad);
OglGrsColor (&grdCurCanv->cvColor);
glPushMatrix ();
glTranslatef (
			(f2fl (xc1) + grdCurCanv->cvBitmap.bmProps.x) / (float) gameStates.ogl.nLastW, 
		1.0f - (f2fl (yc1) + grdCurCanv->cvBitmap.bmProps.y) / (float) gameStates.ogl.nLastH, 0);
glScalef (f2fl (r1), f2fl (r1), f2fl (r1));
if (r1<=i2f (5)){
	if (!circleh5) 
		circleh5=CircleListInit (5, GL_LINE_LOOP, GL_COMPILE_AND_EXECUTE);
	else 
		glCallList (circleh5);
	}
else{
	if (!circleh10) 
		circleh10=CircleListInit (10, GL_LINE_LOOP, GL_COMPILE_AND_EXECUTE);
	else 
		glCallList (circleh10);
}
glPopMatrix ();
if (grdCurCanv->cvColor.rgb)
	glDisable (GL_BLEND);
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawWhitePoly (int nv, g3sPoint **pointList)
{
#if 1
	int i;

r_polyc++;
glDisable (GL_TEXTURE_2D);
glDisable (GL_BLEND);
glColor4d (1.0, 1.0, 1.0, 1.0);
glBegin (GL_TRIANGLE_FAN);
for (i = 0; i < nv; i++, pointList++)
	OglVertex3f (*pointList);
glEnd ();
#endif
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawPoly (int nv, g3sPoint **pointList)
{
	int i;

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nv, pointList);
	return 0;
	}
r_polyc++;
glDisable (GL_TEXTURE_2D);
OglGrsColor (&grdCurCanv->cvColor);
glBegin (GL_TRIANGLE_FAN);
for (i = 0; i < nv; i++, pointList++) {
//	glVertex3f (f2fl (pointList [c]->p3_vec.p.x), f2fl (pointList [c]->p3_vec.p.y), f2fl (pointList [c]->p3_vec.p.z);
	OglVertex3f (*pointList);
	}
if (grdCurCanv->cvColor.rgb || (gameStates.render.grAlpha < GR_ACTUAL_FADE_LEVELS))
	glDisable (GL_BLEND);
glEnd ();
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawPolyAlpha (int nv, g3sPoint **pointList, tRgbaColorf *color, char bDepthMask)
{
	int		i;
	GLint		depthFunc; 

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nv, pointList);
	return 0;
	}
if (color->alpha < 0)
	color->alpha = (float) gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS;
#if 1
if (gameOpts->render.bDepthSort > 0) {
	fVector		vertices [8];

	for (i = 0; i < nv; i++)
		vertices [i] = gameData.render.pVerts [pointList [i]->p3_index];
	RIAddPoly (NULL, vertices, nv, NULL, color, NULL, 1, bDepthMask, GL_TRIANGLE_FAN, GL_REPEAT);
	}
else 
#endif
	{
	r_polyc++;
	glGetIntegerv (GL_DEPTH_FUNC, &depthFunc);
#if OGL_QUERY
	if (gameStates.render.bQueryOcclusion)
		glDepthFunc (GL_LESS);
	else
#endif
	if (!bDepthMask)
		glDepthMask (0);
	glDepthFunc (GL_LEQUAL);
	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glColor4fv ((GLfloat *) color);
	glBegin (GL_TRIANGLE_FAN);
	for (i = 0; i < nv; i++)
		OglVertex3f (*pointList++);
	glEnd ();
	glDepthFunc (depthFunc);
	if (!bDepthMask)
		glDepthMask (1);
	}
return 0;
}

//------------------------------------------------------------------------------

void gr_upoly_tmap (int nverts, int *vert )
{
#if TRACE	
con_printf (CONDBG, "gr_upoly_tmap: unhandled\n");//should never get called
#endif
}

//------------------------------------------------------------------------------

void DrawTexPolyFlat (grsBitmap *bm, int nv, g3sPoint **vertlist)
{
#if TRACE	
con_printf (CONDBG, "DrawTexPolyFlat: unhandled\n");//should never get called
#endif
}

//------------------------------------------------------------------------------

tFaceColor lightColor = {{1.0f, 1.0f, 1.0f, 1.0f}, 0};
tFaceColor tMapColor = {{1.0f, 1.0f, 1.0f, 1.0f}, 0};
tFaceColor vertColors [8] = {
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}, 
	{{1.0f, 1.0f, 1.0f, 1.0f}, 0}
	};
// cap tMapColor scales the color values in tMapColor so that none of them exceeds
// 1.0 if multiplied with any of the current face's corners' brightness values.
// To do that, first the highest corner brightness is determined.
// In the next step, color values are increased to match the max. brightness.
// Then it is determined whether any color value multiplied with the max. brightness would
// exceed 1.0. If so, all three color values are scaled so that their maximum multiplied
// with the max. brightness does not exceed 1.0.

inline void CapTMapColor (tUVL *uvlList, int nv, grsBitmap *bm)
{
#if 0
	tFaceColor *color = tMapColor.index ? &tMapColor : lightColor.index ? &lightColor : NULL;

if (! (bm->bmProps.flags & BM_FLAG_NO_LIGHTING) && color) {
		double	a, m = tMapColor.color.red;
		double	h, l = 0;
		int		i;

	for (i = 0; i < nv; i++, uvlList++) {
		h = (bm->bmProps.flags & BM_FLAG_NO_LIGHTING) ? 1.0 : f2fl (uvlList->l);
		if (l < h)
			l = h;
		}

	// scale brightness with the average color to avoid darkening bright areas with the color
	a = (color->color.red + color->color.green + color->color.blue) / 3;
	if (m < color->color.green)
		m = color->color.green;
	if (m < color->color.blue)
		m = color->color.blue;
	l = l / a;
	// prevent any color component getting over 1.0
	if (l * m > 1.0)
		l = 1.0 / m;
	color->color.red *= l;
	color->color.green *= l;
	color->color.blue *= l;
	}
#endif
}

//------------------------------------------------------------------------------

static inline void ScaleColor (tFaceColor *color, float l)
{
if (l >= 0) {
#if 1
#	if 0
	color->color.red *= l;
	color->color.green *= l;
	color->color.blue *= l;
#	else
		float m = color->color.red;

	if (m < color->color.green)
		m = color->color.green;
	if (m < color->color.blue)
		m = color->color.blue;
	m = l / m;
	color->color.red *= m;
	color->color.green *= m;
	color->color.blue *= m;
#	endif
#endif
	}
}

//------------------------------------------------------------------------------

inline float BC (float c, float b)	//biased contrast
{
return (float) ((c < 0.5) ? pow (c, 1.0f / b) : pow (c, b));
}

void OglColor4sf (float r, float g, float b, float s)
{
if (gameStates.ogl.bStandardContrast)
	glColor4f (r * s, g * s, b * s, gameStates.ogl.fAlpha);
else {
	float c = (float) gameStates.ogl.nContrast - 8.0f;

	if (c > 0.0f)
		c = 1.0f / (1.0f + c * (3.0f / 8.0f));
	else
		c = 1.0f - c * (3.0f / 8.0f);
	glColor4f (BC (r, c) * s, BC (g, c) * s, BC (b, c) * s, gameStates.ogl.fAlpha);
	}
}

//------------------------------------------------------------------------------

/*inline*/ void SetTMapColor (tUVL *uvlList, int i, grsBitmap *bm, int bResetColor, tFaceColor *vertColor)
{
	float l = (bm->bmProps.flags & BM_FLAG_NO_LIGHTING) ? 1.0f : f2fl (uvlList->l);
	float s = 1.0f;

#if SHADOWS
if (gameStates.ogl.bScaleLight)
	s *= gameStates.render.bHeadlightOn ? 0.4f : 0.3f;
#endif
if (gameStates.app.bEndLevelSequence >= EL_OUTSIDE)
	OglColor4sf (l, l, l, s);
else if (vertColor) {
	if (gameStates.ogl.bDynObjLight) {
		vertColor->color.red = 
		vertColor->color.green = 
		vertColor->color.blue = 1.0f;
		}
	else if (tMapColor.index) {
		ScaleColor (&tMapColor, l);
		vertColor->color = tMapColor.color;
		if (l >= 0)
			tMapColor.color.red =
			tMapColor.color.green =
			tMapColor.color.blue = 1.0;
		}	
	else if (i >= sizeof (vertColors) / sizeof (tFaceColor))
		return;
	else if (vertColors [i].index) {
			tFaceColor *pvc = vertColors + i;

		vertColor->color = vertColors [i].color;
		if (bResetColor) {
			pvc->color.red =
			pvc->color.green =
			pvc->color.blue = 1.0;
			pvc->index = 0;
			}
		}	
	else {
		vertColor->color.red = 
		vertColor->color.green = 
		vertColor->color.blue = l;
		}
	vertColor->color.alpha = s;
	}
else {
	if (gameStates.ogl.bDynObjLight)
		OglColor4sf (1.0f, 1.0f, 1.0f, s);
	else if (tMapColor.index) {
		ScaleColor (&tMapColor, l);
		OglColor4sf (tMapColor.color.red, tMapColor.color.green, tMapColor.color.blue, s);
		if (l >= 0)
			tMapColor.color.red =
			tMapColor.color.green =
			tMapColor.color.blue = 1.0;
		}	
	else if (i >= sizeof (vertColors) / sizeof (tFaceColor))
		return;
	else if (vertColors [i].index) {
			tFaceColor *pvc = vertColors + i;

		OglColor4sf (pvc->color.red, pvc->color.green, pvc->color.blue, s);
		if (bResetColor) {
			pvc->color.red =
			pvc->color.green =
			pvc->color.blue = 1.0;
			pvc->index = 0;
			}
		}	
	else {
		OglColor4sf (l, l, l, s);
		}
	}
}

//------------------------------------------------------------------------------

inline void SetTexCoord (tUVL *uvlList, int nOrient, int bMulti, tUVLf *vertUVL)
{
	float u1, v1;

if (nOrient == 1) {
	u1 = 1.0f - f2fl (uvlList->v);
	v1 = f2fl (uvlList->u);
	}
else if (nOrient == 2) {
	u1 = 1.0f - f2fl (uvlList->u);
	v1 = 1.0f - f2fl (uvlList->v);
	}
else if (nOrient == 3) {
	u1 = f2fl (uvlList->v);
	v1 = 1.0f - f2fl (uvlList->u);
	}
else {
	u1 = f2fl (uvlList->u);
	v1 = f2fl (uvlList->v);
	}
if (vertUVL) {
	vertUVL->v.u = u1;
	vertUVL->v.v = v1;
	}
else {
#if OGL_MULTI_TEXTURING
	if (bMulti)
		glMultiTexCoord2f (GL_TEXTURE1_ARB, u1, v1);
else
#endif
		glTexCoord2f (u1, v1);
	}
}

//------------------------------------------------------------------------------
//glTexture MUST be bound first
void OglTexWrap (tOglTexture *tex, int state)
{
#if 0
if (tex->handle < 0)
	state = GL_CLAMP;
#endif
if ((tex->wrapstate != state) || (tex->numrend < 1)) {
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, state);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, state);
	tex->wrapstate = state;
	}
}

//------------------------------------------------------------------------------

inline void InitTMU (int i, int bClientState)
{
	static GLuint tmuIds [] = {GL_TEXTURE0_ARB, GL_TEXTURE1_ARB, GL_TEXTURE2_ARB};

if (glIsList (glInitTMU [bClientState][i]))
	glCallList (glInitTMU [bClientState][i]);
else {
	glInitTMU [bClientState][i] = glGenLists (1);
	if (glInitTMU [bClientState][i])
		glNewList (glInitTMU [bClientState][i], GL_COMPILE);
	OglActiveTexture (tmuIds [i], bClientState);
	glEnable (GL_TEXTURE_2D);
	if (i == 0)
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	else if (i == 1)
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	else
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if (glInitTMU [bClientState][i]) {
		glEndList ();
		InitTMU (i, bClientState);
		}
	}
}

//------------------------------------------------------------------------------

inline void InitTMU0 (int bClientState)
{
if (glIsList (glInitTMU [bClientState][0]))
	glCallList (glInitTMU [bClientState][0]);
else 
	{
	glInitTMU [bClientState][0] = glGenLists (1);
	if (glInitTMU [bClientState][0])
		glNewList (glInitTMU [bClientState][0], GL_COMPILE);
	OglActiveTexture (GL_TEXTURE0_ARB, bClientState);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if (glInitTMU [bClientState][0]) {
		glEndList ();
		InitTMU0 (bClientState);
		}
	}
}

//------------------------------------------------------------------------------

inline void InitTMU1 (bClientState)
{
if (glIsList (glInitTMU [bClientState][1]))
	glCallList (glInitTMU [bClientState][1]);
else 
	{
	glInitTMU [bClientState][1] = glGenLists (1);
	if (glInitTMU [bClientState][1])
		glNewList (glInitTMU [bClientState][1], GL_COMPILE);
	OglActiveTexture (GL_TEXTURE1_ARB, bClientState);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	if (glInitTMU [bClientState][1]) {
		glEndList ();
		InitTMU1 (bClientState);
		}
	}
}

//------------------------------------------------------------------------------

inline void InitTMU2 (bClientState)
{
if (glIsList (glInitTMU [bClientState][2]))
	glCallList (glInitTMU [bClientState][2]);
else 
	{
	glInitTMU [bClientState][2] = glGenLists (1);
	if (glInitTMU [bClientState][2])
		glNewList (glInitTMU [bClientState][2], GL_COMPILE);
	OglActiveTexture (GL_TEXTURE2_ARB, bClientState);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if (glInitTMU [bClientState][2]) {
		glEndList ();
		InitTMU2 (bClientState);
		}
	}
}

//------------------------------------------------------------------------------

inline void InitTMU3 (bClientState)
{
if (glIsList (glInitTMU [bClientState][3]))
	glCallList (glInitTMU [bClientState][3]);
else 
	{
	glInitTMU [bClientState][3] = glGenLists (1);
	if (glInitTMU [bClientState][3])
		glNewList (glInitTMU [bClientState][3], GL_COMPILE);
	OglActiveTexture (GL_TEXTURE2_ARB, bClientState);
	glEnable (GL_TEXTURE_2D);
	if (glInitTMU [bClientState][3]) {
		glEndList ();
		InitTMU3 (bClientState);
		}
	}
}

//------------------------------------------------------------------------------

inline void ExitTMU (int bClientState)
{
if (glIsList (glExitTMU))
	glCallList (glExitTMU);
else 
	{
	glExitTMU = glGenLists (1);
	if (glExitTMU)
		glNewList (glExitTMU, GL_COMPILE);
	OglActiveTexture (GL_TEXTURE2_ARB, bClientState);
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D);
	OglActiveTexture (GL_TEXTURE1_ARB, bClientState);
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D);
	OglActiveTexture (GL_TEXTURE0_ARB, bClientState);
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D);
	if (glExitTMU) {
		glEndList ();
		ExitTMU (bClientState);
		}
	}
}

//------------------------------------------------------------------------------

typedef void tInitTMU (int);
typedef tInitTMU *pInitTMU;

inline int G3BindTex (grsBitmap *bmP, GLint nTexId, GLhandleARB lmProg, char *pszTexId, 
						    pInitTMU initTMU, int bShaderMerge, int bClientState)
{
if (bmP || (nTexId >= 0)) {
	initTMU (bClientState);
	if (nTexId >= 0)
		OGL_BINDTEX (nTexId);
	else {
		if (OglBindBmTex (bmP, 1, 3))
			return 1;
		OglTexWrap (bmP->glTexture, GL_REPEAT);
		}
	if (bShaderMerge)
		glUniform1i (glGetUniformLocation (lmProg, pszTexId), 0);
	}
return 0;
}

//------------------------------------------------------------------------------

void G3Normal (g3sPoint **pointList, vmsVector *pvNormal)
{
vmsVector	vNormal;

#if 1
if (pvNormal) {
	if (gameStates.ogl.bUseTransform)
		glNormal3f ((GLfloat) f2fl (pvNormal->p.x), 
						(GLfloat) f2fl (pvNormal->p.y), 
						(GLfloat) f2fl (pvNormal->p.z));
		//VmVecAdd (&vNormal, pvNormal, &pointList [0]->p3_vec);
	else {
		G3RotatePoint (&vNormal, pvNormal, 0);
		glNormal3f ((GLfloat) f2fl (vNormal.p.x), 
						(GLfloat) f2fl (vNormal.p.y), 
						(GLfloat) f2fl (vNormal.p.z));
		//VmVecInc (&vNormal, &pointList [0]->p3_vec);
		}
//	glNormal3f ((GLfloat) f2fl (vNormal.x), (GLfloat) f2fl (vNormal.y), (GLfloat) f2fl (vNormal.z));
	}
else 
#endif
	{
	int	v [4];

	v [0] = pointList [0]->p3_index;
	v [1] = pointList [1]->p3_index;
	v [2] = pointList [2]->p3_index;
	if ((v [0] < 0) || (v [1] < 0) || (v [2] < 0)) {
		VmVecNormal (&vNormal, 
						 &pointList [0]->p3_vec,
						 &pointList [1]->p3_vec,
						 &pointList [2]->p3_vec);
		glNormal3f ((GLfloat) f2fl (vNormal.p.x), (GLfloat) f2fl (vNormal.p.y), (GLfloat) f2fl (vNormal.p.z));
		}
	else {
		int bFlip = GetVertsForNormal (v [0], v [1], v [2], 32767, v, v + 1, v + 2, v + 3);
		VmVecNormal (&vNormal, 
							gameData.segs.vertices + v [0], 
							gameData.segs.vertices + v [1], 
							gameData.segs.vertices + v [2]);
		if (bFlip)
			VmVecNegate (&vNormal);
		if (!gameStates.ogl.bUseTransform)
			G3RotatePoint (&vNormal, &vNormal, 0);
		//VmVecInc (&vNormal, &pointList [0]->p3_vec);
		glNormal3f ((GLfloat) f2fl (vNormal.p.x), 
						(GLfloat) f2fl (vNormal.p.y), 
						(GLfloat) f2fl (vNormal.p.z));
		}
	}
}

//------------------------------------------------------------------------------

void G3CalcNormal (g3sPoint **pointList, fVector *pvNormal)
{
	vmsVector	vNormal;
	int	v [4];

v [0] = pointList [0]->p3_index;
v [1] = pointList [1]->p3_index;
v [2] = pointList [2]->p3_index;
if ((v [0] < 0) || (v [1] < 0) || (v [2] < 0)) {
	VmVecNormal (&vNormal, 
					 &pointList [0]->p3_vec,
					 &pointList [1]->p3_vec,
					 &pointList [2]->p3_vec);
	}
else {
	int bFlip = GetVertsForNormal (v [0], v [1], v [2], 32767, v, v + 1, v + 2, v + 3);
	VmVecNormal (&vNormal, 
					 gameData.segs.vertices + v [0], 
					 gameData.segs.vertices + v [1], 
					 gameData.segs.vertices + v [2]);
	if (bFlip)
		VmVecNegate (&vNormal);
	}
VmsVecToFloat (pvNormal, &vNormal);
}

//------------------------------------------------------------------------------

inline fVector *G3GetNormal (g3sPoint *pPoint, fVector *pvNormal)
{
return pPoint->p3_normal.nFaces ? &pPoint->p3_normal.vNormal : pvNormal;
}

//------------------------------------------------------------------------------

fVector *G3Reflect (fVector *vReflect, fVector *vLight, fVector *vNormal)
{
//2 * n * (l dot n) - l
	float		LdotN = 2 * VmVecDotf (vLight, vNormal);

#if 0
VmVecScalef (vReflect, vNormal, LdotN);
VmVecDecf (vReflect, vLight);
#else
vReflect->p.x = vNormal->p.x * LdotN - vLight->p.x;
vReflect->p.y = vNormal->p.y * LdotN - vLight->p.y;
vReflect->p.z = vNormal->p.z * LdotN - vLight->p.z;
#endif
return vReflect;
}

//------------------------------------------------------------------------------

#define G3_DOTF(_v0,_v1)	((_v0).p.x * (_v1).p.x + (_v0).p.y * (_v1).p.y + (_v0).p.z * (_v1).p.z)

#define G3_REFLECT(_vr,_vl,_vn) \
	{ \
	float	LdotN = 2 * G3_DOTF(_vl, _vn); \
	(_vr).p.x = (_vn).p.x * LdotN - (_vl).p.x; \
	(_vr).p.y = (_vn).p.y * LdotN - (_vl).p.y; \
	(_vr).p.z = (_vn).p.z * LdotN - (_vl).p.z; \
	} 

//------------------------------------------------------------------------------

inline int sqri (int i)
{
return i * i;
}

//------------------------------------------------------------------------------

#define VECMAT_CALLS 0

float fLightRanges [5] = {5, 7.071f, 10, 14.142f, 20};

int G3AccumVertColor (fVector *pColorSum)
{
	int				h, i, j, nType, bInRad, nSaturation = gameOpts->render.color.nSaturation;
	int				nBrightness, nMaxBrightness = 0;
	float				fLightRange = fLightRanges [IsMultiGame ? 1 : extraGameInfo [IsMultiGame].nLightRange];
	float				fLightDist, fAttenuation, spotEffect, fMag, NdotL, RdotE;
	fVector			spotDir, lightDir, lightColor, lightPos, vReflect, colorSum, 
						vertColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
	tShaderLight	*psl;

colorSum = *pColorSum;
h = (gameData.render.lights.dynamic.shader.nActiveLights < MAX_NEAREST_LIGHTS) ? gameData.render.lights.dynamic.shader.nActiveLights : MAX_NEAREST_LIGHTS;
for (i = j = 0; i < h; i++) {
	psl = gameData.render.lights.dynamic.shader.activeLights [i];
#if 0
	if (!psl->bState)
		continue;
#endif
	if (i == gameData.threads.vertColor.data.nMatLight)
		continue;
#if 0
	if (gameData.threads.vertColor.data.bExclusive) {
		if (gameData.threads.vertColor.data.bExclusive < 0)
			break;
		if (!psl->bExclusive)
			continue;
		gameData.threads.vertColor.data.bExclusive = -1;
		nType = psl->nType;
		}
	else 
#endif
		{
		nType = psl->nType;
#if 0
		if (!nType)
			continue;
#endif
		}
	if (psl->bVariable && gameData.threads.vertColor.data.bDarkness)
		continue;
	lightColor = *((fVector *) &psl->color);
#if STATIC_LIGHT_TRANSFORM
	lightPos = psl->pos [1];
#else
	lightPos = psl->pos [gameStates.render.nState];
#endif
#if VECMAT_CALLS
	VmVecSubf (&lightDir, &lightPos, gameData.threads.vertColor.data.pVertPos);
#else
	lightDir.p.x = lightPos.p.x - gameData.threads.vertColor.data.pVertPos->p.x;
	lightDir.p.y = lightPos.p.y - gameData.threads.vertColor.data.pVertPos->p.y;
	lightDir.p.z = lightPos.p.z - gameData.threads.vertColor.data.pVertPos->p.z;
#endif
	//scaled quadratic attenuation depending on brightness
	bInRad = 0;
	if (psl->brightness < 0)
		fAttenuation = 0.01f;
	else {
		fLightDist = VmVecMagf (&lightDir) / fLightRange;
		if (nType == 1) {
			fLightDist -= psl->rad;	//make light brighter close to light source
			if (fLightDist < 1.0f) {
				bInRad = 1;
				fLightDist = 1.4142f;
				}
			else {	//make it decay faster
				fLightDist *= fLightDist;
				fLightDist *= 2.0f;
				}
			}
		else
			fLightDist *= fLightDist;
		fAttenuation = fLightDist / psl->brightness;
		}
#if VECMAT_CALLS
	VmVecNormalizef (&lightDir, &lightDir);
#else
	fMag = VmVecMagf (&lightDir);
	lightDir.p.x /= fMag;
	lightDir.p.y /= fMag;
	lightDir.p.z /= fMag;
#endif
	NdotL = bInRad ? 1 : G3_DOTF (gameData.threads.vertColor.data.vertNorm, lightDir);
	if (psl->bSpot) {
		if (NdotL <= 0)
			continue;
#if VECMAT_CALLS
		VmVecNormalizef (&spotDir, &psl->dir);
#else
		fMag = VmVecMagf (&psl->dir);
		spotDir.p.x = psl->dir.p.x / fMag;
		spotDir.p.y = psl->dir.p.y / fMag;
		spotDir.p.z = psl->dir.p.z / fMag;
#endif
		lightDir.p.x = -lightDir.p.x;
		lightDir.p.y = -lightDir.p.y;
		lightDir.p.z = -lightDir.p.z;
		spotEffect = G3_DOTF (spotDir, lightDir);
		if (spotEffect <= psl->spotAngle)
			continue;
		spotEffect = (float) pow (spotEffect, psl->spotExponent);
		fAttenuation /= spotEffect * 10;
#if VECMAT_CALLS
		VmVecScaleAddf (&vertColor, &gameData.threads.vertColor.data.matAmbient, &gameData.threads.vertColor.data.matDiffuse, NdotL);
#else
		vertColor.p.x = gameData.threads.vertColor.data.matAmbient.p.x + gameData.threads.vertColor.data.matDiffuse.p.x * NdotL;
		vertColor.p.y = gameData.threads.vertColor.data.matAmbient.p.y + gameData.threads.vertColor.data.matDiffuse.p.y * NdotL;
		vertColor.p.z = gameData.threads.vertColor.data.matAmbient.p.z + gameData.threads.vertColor.data.matDiffuse.p.z * NdotL;
#endif
		}
	else if (NdotL < 0) {
		NdotL = 0;
#if VECMAT_CALLS
		VmVecIncf (&vertColor, &gameData.threads.vertColor.data.matAmbient);
#else
		vertColor.p.x += gameData.threads.vertColor.data.matAmbient.p.x;
		vertColor.p.y += gameData.threads.vertColor.data.matAmbient.p.y;
		vertColor.p.z += gameData.threads.vertColor.data.matAmbient.p.z;
#endif
		}
	else {
		//vertColor = lightColor * (gl_FrontMaterial.diffuse * NdotL + matAmbient);
#if VECMAT_CALLS
		VmVecScaleAddf (&vertColor, &gameData.threads.vertColor.data.matAmbient, &gameData.threads.vertColor.data.matDiffuse, NdotL);
#else
		vertColor.p.x = gameData.threads.vertColor.data.matAmbient.p.x + gameData.threads.vertColor.data.matDiffuse.p.x * NdotL;
		vertColor.p.y = gameData.threads.vertColor.data.matAmbient.p.y + gameData.threads.vertColor.data.matDiffuse.p.y * NdotL;
		vertColor.p.z = gameData.threads.vertColor.data.matAmbient.p.z + gameData.threads.vertColor.data.matDiffuse.p.z * NdotL;
#endif
		}
	//if (fAttenuation > 50.0)
	//	continue;	//too far away
	vertColor.p.x *= lightColor.p.x;
	vertColor.p.y *= lightColor.p.y;
	vertColor.p.z *= lightColor.p.z;
	if ((NdotL > 0.0) && gameData.threads.vertColor.data.bMatSpecular) {
		//spec = pow (reflect dot lightToEye, matShininess) * matSpecular * lightSpecular
		//RdotV = max (dot (reflect (-normalize (lightDir), normal), normalize (-vertPos)), 0.0);
#if VECMAT_CALLS
		VmVecNegatef (&lightPos);	//create vector from light to eye
		VmVecNormalizef (&lightPos, &lightPos);
#else
		fMag = -VmVecMagf (&lightPos);
		lightPos.p.x /= fMag;
		lightPos.p.y /= fMag;
		lightPos.p.z /= fMag;
#endif
		if (!psl->bSpot) {	//need direction from light to vertex now
			lightDir.p.x = -lightDir.p.x;
			lightDir.p.y = -lightDir.p.y;
			lightDir.p.z = -lightDir.p.z;
			}
		G3_REFLECT (vReflect, lightDir, gameData.threads.vertColor.data.vertNorm);
#if VECMAT_CALLS
		VmVecNormalizef (&vReflect, &vReflect);
#else
		fMag = VmVecMagf (&vReflect);
		vReflect.p.x /= fMag;
		vReflect.p.y /= fMag;
		vReflect.p.z /= fMag;
#endif
		RdotE = G3_DOTF (vReflect, lightPos);
		if (RdotE < 0.0)
			RdotE = 0.0;
		//vertColor += matSpecular * lightColor * pow (RdotE, fMatShininess);
		if (RdotE && gameData.threads.vertColor.data.fMatShininess) {
#if VECMAT_CALLS
			VmVecScalef (&lightColor, &lightColor, (float) pow (RdotE, gameData.threads.vertColor.data.fMatShininess));
#else
			fMag = (float) pow (RdotE, gameData.threads.vertColor.data.fMatShininess);
			lightColor.p.x *= fMag;
			lightColor.p.y *= fMag;
			lightColor.p.z *= fMag;
#endif
			}
#if VECMAT_CALLS
		VmVecMulf (&lightColor, &lightColor, &gameData.threads.vertColor.data.matSpecular);
		VmVecIncf (&vertColor, &lightColor);
#else
		vertColor.p.x += lightColor.p.x * gameData.threads.vertColor.data.matSpecular.p.x;
		vertColor.p.y += lightColor.p.y * gameData.threads.vertColor.data.matSpecular.p.y;
		vertColor.p.z += lightColor.p.z * gameData.threads.vertColor.data.matSpecular.p.z;
#endif
		}
	if (nSaturation < 2)	{//sum up color components
#if VECMAT_CALLS
		VmVecScaleAddf (&colorSum, &colorSum, &vertColor, 1.0f / fAttenuation);
#else
		colorSum.p.x += vertColor.p.x / fAttenuation;
		colorSum.p.y += vertColor.p.y / fAttenuation;
		colorSum.p.z += vertColor.p.z / fAttenuation;
#endif
		}
	else {	//use max. color components
		vertColor.p.x /= fAttenuation;
		vertColor.p.y /= fAttenuation;
		vertColor.p.z /= fAttenuation;
		nBrightness = sqri ((int) (vertColor.c.r * 1000)) + sqri ((int) (vertColor.c.g * 1000)) + sqri ((int) (vertColor.c.b * 1000));
		if (nMaxBrightness < nBrightness) {
			nMaxBrightness = nBrightness;
			colorSum = vertColor;
			}
		else if (nMaxBrightness == nBrightness) {
			if (colorSum.c.r < vertColor.c.r)
				colorSum.c.r = vertColor.c.r;
			if (colorSum.c.g < vertColor.c.g)
				colorSum.c.g = vertColor.c.g;
			if (colorSum.c.b < vertColor.c.b)
				colorSum.c.b = vertColor.c.b;
			}
		}
	j++;
	}
if (j) {
	if (nSaturation == 1) {	//if a color component is > 1, cap color components using highest component value
		float	cMax = colorSum.c.r;
		if (cMax < colorSum.c.g)
			cMax = colorSum.c.g;
		if (cMax < colorSum.c.b)
			cMax = colorSum.c.b;
		if (cMax > 1) {
			colorSum.c.r /= cMax;
			colorSum.c.g /= cMax;
			colorSum.c.b /= cMax;
			}
		}
	*pColorSum = colorSum;
	}
return j;
}

//------------------------------------------------------------------------------

#define STATIC_LIGHT_TRANSFORM	0

void G3VertexColor (fVector *pvVertNorm, fVector *pVertPos, int nVertex, tFaceColor *pVertColor, float fScale, int bSetColor)
{
	fVector			matSpecular = {{0.0f, 0.0f, 0.0f, 1.0f}},
						colorSum = {{0.0f, 0.0f, 0.0f, 1.0f}};
#if !STATIC_LIGHT_TRANSFORM
	fVector			vertPos;
#endif
	//float				fScale;
	tFaceColor		*pc = NULL;

gameData.threads.vertColor.data.bExclusive = !FAST_SHADOWS && (gameStates.render.nShadowPass == 3),
gameData.threads.vertColor.data.fMatShininess = 0.0f;
gameData.threads.vertColor.data.bMatSpecular = 0;
gameData.threads.vertColor.data.bMatEmissive = 0; 
gameData.threads.vertColor.data.nMatLight = -1;
if (!FAST_SHADOWS && (gameStates.render.nShadowPass == 3))
	; //fScale = 1.0f;
else if (FAST_SHADOWS || (gameStates.render.nShadowPass != 1))
	; //fScale = 1.0f;
else
	fScale *= gameStates.render.bHeadlightOn ? 0.4f : 0.3f;
if (fScale > 1)
	fScale = 1;
if (gameData.render.lights.dynamic.material.bValid) {
#if 0
	if (gameData.render.lights.dynamic.material.emissive.c.r ||
		 gameData.render.lights.dynamic.material.emissive.c.g ||
		 gameData.render.lights.dynamic.material.emissive.c.b) {
		gameData.threads.vertColor.data.bMatEmissive = 1;
		nMatLight = gameData.render.lights.dynamic.material.nLight;
		colorSum = gameData.render.lights.dynamic.material.emissive;
		}
#endif
	gameData.threads.vertColor.data.bMatSpecular = 
		gameData.render.lights.dynamic.material.specular.c.r ||
		gameData.render.lights.dynamic.material.specular.c.g ||
		gameData.render.lights.dynamic.material.specular.c.b;
	if (gameData.threads.vertColor.data.bMatSpecular) {
		gameData.threads.vertColor.data.matSpecular = gameData.render.lights.dynamic.material.specular;
		gameData.threads.vertColor.data.fMatShininess = (float) gameData.render.lights.dynamic.material.shininess;
		}
	else
		gameData.threads.vertColor.data.matSpecular = matSpecular;
	}
#if 1//ndef _DEBUG //cache light values per frame
if (!(gameData.threads.vertColor.data.bExclusive || gameData.threads.vertColor.data.bMatEmissive) && (nVertex >= 0)) {
	pc = gameData.render.color.vertices + nVertex;
	if (pc->index == gameStates.render.nFrameFlipFlop + 1) {
		if (pVertColor) {
			pVertColor->index = gameStates.render.nFrameFlipFlop + 1;
			pVertColor->color.red = pc->color.red * fScale;
			pVertColor->color.green = pc->color.green * fScale;
			pVertColor->color.blue = pc->color.blue * fScale;
			pVertColor->color.alpha = 1;
			}
		if (bSetColor)
			OglColor4sf (pc->color.red * fScale, pc->color.green * fScale, pc->color.blue * fScale, 1.0);
		return;
		}
	}
#endif
if (gameStates.ogl.bUseTransform) 
	VmVecNormalizef (&gameData.threads.vertColor.data.vertNorm, pvVertNorm);
else {
#if !STATIC_LIGHT_TRANSFORM
	if (!gameStates.render.nState)
		VmVecNormalizef (&gameData.threads.vertColor.data.vertNorm, pvVertNorm);
	else 
#endif
		G3RotatePointf (&gameData.threads.vertColor.data.vertNorm, pvVertNorm, 0);
	}
if (!(gameStates.render.nState || pVertColor)) {
#if !STATIC_LIGHT_TRANSFORM
	VmsVecToFloat (&vertPos, gameData.segs.vertices + nVertex);
	pVertPos = &vertPos;
#endif
	SetNearestVertexLights (nVertex, 1, 0, 1);
	}
gameData.threads.vertColor.data.pVertPos = pVertPos;
//VmVecNegatef (&vertNorm);
//if (nLights)
#if MULTI_THREADED_LIGHTS
if (gameStates.app.bMultiThreaded) {
	SDL_SemPost (gameData.threads.vertColor.info [0].exec);
	SDL_SemPost (gameData.threads.vertColor.info [1].exec);
	SDL_SemWait (gameData.threads.vertColor.info [0].done);
	SDL_SemWait (gameData.threads.vertColor.info [1].done);
	VmVecAddf (&colorSum, gameData.threads.vertColor.data.colorSum, gameData.threads.vertColor.data.colorSum + 1);
	}
else
#endif
	G3AccumVertColor (&colorSum);
if ((nVertex >= 0) && !(gameStates.render.nState && gameData.threads.vertColor.data.bDarkness)) {
	tRgbaColorf	ambient = gameData.render.color.ambient [nVertex].color;
	colorSum.c.r += ambient.red;
	colorSum.c.g += ambient.green;
	colorSum.c.b += ambient.blue;
	}
#if 0
if (!pVertColor && gameData.render.nPaletteGamma) {
	float fBright = 1.0f + (float) gameData.render.nPaletteGamma;

	colorSum.c.r *= fBright;
	colorSum.c.g *= fBright;
	colorSum.c.b *= fBright;
	}
#endif
if (colorSum.c.r > 1.0)
	colorSum.c.r = 1.0;
if (colorSum.c.g > 1.0)
	colorSum.c.g = 1.0;
if (colorSum.c.b > 1.0)
	colorSum.c.b = 1.0;
if (bSetColor)
	OglColor4sf (colorSum.c.r * fScale, colorSum.c.g * fScale, colorSum.c.b * fScale, 1.0);
#if 1
if (!gameData.threads.vertColor.data.bMatEmissive && pc) {
	pc->index = gameStates.render.nFrameFlipFlop + 1;
	pc->color.red = colorSum.c.r;
	pc->color.green = colorSum.c.g;
	pc->color.blue = colorSum.c.b;
	}
if (pVertColor) {
	pVertColor->index = gameStates.render.nFrameFlipFlop + 1;
	pVertColor->color.red = colorSum.c.r * fScale;
	pVertColor->color.green = colorSum.c.g * fScale;
	pVertColor->color.blue = colorSum.c.b * fScale;
	pVertColor->color.alpha = colorSum.c.a;
	}
#endif
} 

//------------------------------------------------------------------------------

int OglEnableClientState (GLuint nState)
{
glEnableClientState (nState);
if (!glGetError ())
	return 1;
glDisable (nState);
glEnableClientState (nState);
return glGetError () == 0;
}

//------------------------------------------------------------------------------

void OglDisableClientStates (int bTexCoord, int bColor)
{
glDisableClientState (GL_VERTEX_ARRAY);
if (bTexCoord)
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
if (bColor)
	glDisableClientState (GL_COLOR_ARRAY);
}

//------------------------------------------------------------------------------

int OglEnableClientStates (int bTexCoord, int bColor)
{
if (!OglEnableClientState (GL_VERTEX_ARRAY))
	return 0;
if (bTexCoord) {
	if (!OglEnableClientState (GL_TEXTURE_COORD_ARRAY)) {
		OglDisableClientStates (0, 0);
		return 0;
		}
	}
else
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
if (bColor) {
	if (!OglEnableClientState (GL_COLOR_ARRAY)) {
		OglDisableClientStates (bTexCoord, 0);
		return 0;
		}
	}
else
	glDisableClientState (GL_COLOR_ARRAY);
glClientActiveTexture (GL_TEXTURE0_ARB);
return 1;
}

//------------------------------------------------------------------------------

int G3EnableClientState (GLuint nState)
{
if (OglEnableClientState (nState))
	return 1;
gameData.render.pVerts = NULL;
return 0;
}

//------------------------------------------------------------------------------

void G3DisableClientStates (GLuint nTMU)
{
glClientActiveTexture (nTMU);
glDisableClientState (GL_VERTEX_ARRAY);
glDisableClientState (GL_COLOR_ARRAY);
//glDisableClientState (GL_INDEX_ARRAY);
glDisableClientState (GL_TEXTURE_COORD_ARRAY);
}

//------------------------------------------------------------------------------

int G3EnableClientStates (GLuint nTMU)
{
if	(G3EnableClientState (GL_COLOR_ARRAY) &&
//	 G3EnableClientState (GL_INDEX_ARRAY) &&
	 G3EnableClientState (GL_TEXTURE_COORD_ARRAY) &&
	 G3EnableClientState (GL_VERTEX_ARRAY)) {
	glClientActiveTexture (nTMU);
	return 1;
	}
G3DisableClientStates (nTMU);
return 0;
}

//------------------------------------------------------------------------------

#define	INIT_TMU(_initTMU,_bmP,_bClientState) \
			_initTMU (_bClientState); \
			if (OglBindBmTex (_bmP, 1, 3)) \
				return 1; \
			(_bmP) = BmCurFrame (_bmP, -1); \
			OglTexWrap ((_bmP)->glTexture, GL_REPEAT);


extern void (*tmap_drawer_ptr) (grsBitmap *bmP, int nv, g3sPoint **vertlist);

static GLhandleARB	lmProg = (GLhandleARB) 0;
static GLhandleARB	tmProg = (GLhandleARB) 0;

//------------------------------------------------------------------------------

bool G3DrawTexPolyFlat (
	int			nVerts, 
	g3sPoint		**pointList, 
	tUVL			*uvlList, 
	tUVL			*uvlLMap, 
	grsBitmap	*bmBot, 
	grsBitmap	*bmTop, 
	tOglTexture	*lightMap, 
	vmsVector	*pvNormal,
	int			orient, 
	int			bBlend)
{
	int			i;
	grsBitmap	*bmP = NULL;
	g3sPoint		**ppl;

if (FAST_SHADOWS) {
	if (bBlend)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glDisable (GL_BLEND);
	}
else {
	if (gameStates.render.nShadowPass == 3) {
		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE, GL_ONE);
		}
	}
OglActiveTexture (GL_TEXTURE0_ARB, 0);
glDisable (GL_TEXTURE_2D);
glColor4d (0, 0, 0, GrAlpha ());
glBegin (GL_TRIANGLE_FAN);
for (i = 0, ppl = pointList; i < nVerts; i++, ppl++)
	OglVertex3f (*ppl);
glEnd ();
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawTexPolyMulti (
	int			nVerts, 
	g3sPoint		**pointList, 
	tUVL			*uvlList, 
	tUVL			*uvlLMap, 
	grsBitmap	*bmBot, 
	grsBitmap	*bmTop, 
	tOglTexture	*lightMap, 
	vmsVector	*pvNormal,
	int			orient, 
	int			bBlend)
{
	int			i, tmType, nFrame;
	int			bShaderMerge = 0,
					bSuperTransp = 0;
	int			bLight = 1, 
					bDynLight = gameStates.render.bApplyDynLight && !gameStates.app.bEndLevelSequence, 
					bDepthSort,
					bResetColor = 0,
					bDrawOverlay = 0;
	grsBitmap	*bmP = NULL, *bmMask;
	g3sPoint		*pl, **ppl;
#if USE_VERTNORMS
	fVector		vNormal, vVertex;
#endif
#if G3_DRAW_ARRAYS
	int			bDrawArrays = gameData.render.pVerts != NULL;
#else
	int			bDrawArrays = 0;
#endif

if (bDynLight)
	bDynLight = 1;
if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVerts, pointList);
	return 0;
	}
if (!bmBot)
	return 1;
if (FAST_SHADOWS) {
	if (!bBlend)
		glDisable (GL_BLEND);
#if 0
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	}
else {
	if (gameStates.render.nShadowPass == 1)
		bLight = !bDynLight;
	else if (gameStates.render.nShadowPass == 3) {
		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE, GL_ONE);
		}
	}
glDepthFunc (GL_LEQUAL);
bmBot = BmOverride (bmBot, -1);
bDepthSort = (!bmTop && (gameOpts->render.bDepthSort > 0) && 
				  ((gameStates.render.grAlpha < GR_ACTUAL_FADE_LEVELS) || 
				   (bmBot->bmProps.flags & (BM_FLAG_TRANSPARENT | BM_FLAG_SEE_THRU | BM_FLAG_TGA)) == (BM_FLAG_TRANSPARENT | BM_FLAG_TGA)));
if ((bmTop = BmOverride (bmTop, -1)) && BM_FRAMES (bmTop)) {
	nFrame = (int) (BM_CURFRAME (bmTop) - BM_FRAMES (bmTop));
	bmP = bmTop;
	bmTop = BM_CURFRAME (bmTop);
	}
else
	nFrame = -1;
if (bmTop) {
	if (nFrame < 0)
      bSuperTransp = (bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
	else
		bSuperTransp = (bmP->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0;
	bShaderMerge = bSuperTransp && gameStates.ogl.bGlTexMerge;
	bDrawOverlay = !bShaderMerge;
	}
else
	bDrawOverlay = -1;
#if G3_DRAW_ARRAYS
retry:
#endif
if (bShaderMerge) {	
	GLint loc;
	bmMask = BM_MASK (bmTop);
	tmType = bSuperTransp ? bmMask ? 2 : 1 : 0;
	glUseProgramObject (tmProg = tmShaderProgs [tmType]);
	INIT_TMU (InitTMU0, bmBot, bDrawArrays);
	glUniform1i (loc = glGetUniformLocation (tmProg, "btmTex"), 0);
	INIT_TMU (InitTMU1, bmTop, bDrawArrays);
	glUniform1i (glGetUniformLocation (tmProg, "topTex"), 1);
	if (bmMask) {
		INIT_TMU (InitTMU2, bmMask, bDrawArrays);
		glUniform1i (glGetUniformLocation (tmProg, "maskTex"), 2);
		}
	glUniform1f (loc = glGetUniformLocation (tmProg, "grAlpha"), 
					 gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS);
	}
else if (!bDepthSort) {
	if (bmBot == gameData.endLevel.satellite.bmP) {
		glActiveTexture (GL_TEXTURE0_ARB);
		glEnable (GL_TEXTURE_2D);
		}
	else
		InitTMU0 (bDrawArrays);
	if (OglBindBmTex (bmBot, 1, 3))
		return 1;
	bmBot = BmCurFrame (bmBot, -1);
	if (bmBot == bmpDeadzone)
		OglTexWrap (bmBot->glTexture, GL_CLAMP);
	else
		OglTexWrap (bmBot->glTexture, GL_REPEAT);
	}

if (!bDepthSort) {
	if (SHOW_DYN_LIGHT) {
#if USE_VERTNORMS
		if (pvNormal)
			VmsVecToFloat (&vNormal, pvNormal);
	else
			G3CalcNormal (pointList, &vNormal);
#else
			G3Normal (pointList, pvNormal);
#endif
		}
	if (gameStates.render.bFullBright) {
		glColor3d (1,1,1);
		bLight = 0;
		}
	else if (!gameStates.render.nRenderPass)
		bLight = 0;
	else if (!bLight)
		glColor3i (0,0,0);
	if (!bLight)
		bDynLight = 0;
	gameStates.ogl.bDynObjLight = bDynLight;
	}

gameStates.ogl.fAlpha = gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS;
if (bDrawArrays || bDepthSort) {
		fVector		vertices [8];
		tFaceColor	vertColors [8];
		tUVLf			vertUVL [2][8];
		int			vertIndex [8];
		//int			colorIndex [8];

	for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
		pl = *ppl;
		vertIndex [i] = pl->p3_index;
		//colorIndex [i] = i;
		if (pl->p3_index < 0)
			VmsVecToFloat (vertices + i, &pl->p3_vec);
		else
			vertices [i] = gameData.render.pVerts [pl->p3_index];
		vertUVL [0][i].v.u = f2fl (uvlList [i].u);
		vertUVL [0][i].v.v = f2fl (uvlList [i].v);
		SetTexCoord (uvlList + i, orient, 1, vertUVL [1] + i);
		if (bDynLight)
			G3VertexColor (G3GetNormal (pl, &vNormal), VmsVecToFloat (&vVertex, &(pl->p3_vec)), vertIndex [i], vertColors + i, 
								gameStates.render.nState ? f2fl (uvlList [i].l) : 1, 0);
		else if (bLight)
			SetTMapColor (uvlList + i, i, bmBot, !bDrawOverlay, vertColors + i);
		}
#if 1
	if (gameOpts->render.bDepthSort > 0) {
		OglLoadBmTexture (bmBot, 1, 3, 0);
		RIAddPoly (bmBot, vertices, nVerts, vertUVL [0], NULL, vertColors, nVerts, 1, GL_TRIANGLE_FAN, GL_REPEAT);
		return 0;
		}
#endif
	}
#if G3_DRAW_ARRAYS
if (bDrawArrays) {
	if (!G3EnableClientStates (GL_TEXTURE0_ARB)) {
		bDrawArrays = 0;
		goto retry;
		}
	glVertexPointer (3, GL_FLOAT, sizeof (fVector), vertices);
//	glIndexPointer (GL_INT, 0, colorIndex);
	glTexCoordPointer (2, GL_FLOAT, sizeof (tUVLf), vertUVL [0]);
	if (bLight)
		glColorPointer (4, GL_FLOAT, sizeof (tFaceColor), vertColors);
	if (bmTop && !bDrawOverlay) {
		if (!G3EnableClientStates (GL_TEXTURE1_ARB)) {
			G3DisableClientStates (GL_TEXTURE0_ARB);
			bDrawArrays = 0;
			goto retry;
			}
		glVertexPointer (3, GL_FLOAT, sizeof (fVector), vertices);
		if (bLight)
			glColorPointer (4, GL_FLOAT, sizeof (tFaceColor), vertColors);
//		glIndexPointer (GL_INT, 0, colorIndex);
		glTexCoordPointer (2, GL_FLOAT, sizeof (tUVLf), vertUVL [1]);
		}
	glDrawArrays (GL_TRIANGLE_FAN, 0, nVerts);
	G3DisableClientStates (GL_TEXTURE0_ARB);
	if (bmTop && !bDrawOverlay)
		G3DisableClientStates (GL_TEXTURE1_ARB);
	}
else 
#endif
	{
	glBegin (GL_TRIANGLE_FAN);
	if (bDynLight) {
		if (bDrawOverlay) {
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				pl = *ppl;
				G3VertexColor (G3GetNormal (pl, &vNormal), VmsVecToFloat (&vVertex, &(pl->p3_vec)), pl->p3_index, NULL, 
									gameStates.render.nState ? f2fl (uvlList [i].l) : 1, 1);
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				OglVertex3f (pl);
				}
			}
		else {
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				pl = *ppl;
				G3VertexColor (G3GetNormal (pl, &vNormal), VmsVecToFloat (&vVertex, &(pl->p3_vec)), pl->p3_index, NULL, 
									gameStates.render.nState ? f2fl (uvlList [i].l) : 1, 1);
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				SetTexCoord (uvlList + i, orient, 1, NULL);
				OglVertex3f (pl);
				}
			}
		}
	else if (bLight) {
		if (bDrawOverlay) {
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				SetTMapColor (uvlList + i, i, bmBot, 1, NULL);
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				OglVertex3f (*ppl);
				}
			}
		else {
			bResetColor = (bDrawOverlay != 1);
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				SetTMapColor (uvlList + i, i, bmBot, bResetColor, NULL);
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				SetTexCoord (uvlList + i, orient, 1, NULL);
				OglVertex3f (*ppl);
				}
			}
		}
	else {
		if (bDrawOverlay) {
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				OglVertex3f (*ppl);
				}
			}
		else {
			for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
				glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
				SetTexCoord (uvlList + i, orient, 1, NULL);
				OglVertex3f (*ppl);
				}
			}
		}
	glEnd ();
	}
if (bDrawOverlay > 0) {
	r_tpolyc++;
	OglActiveTexture (GL_TEXTURE0_ARB, 0);
	glEnable (GL_TEXTURE_2D);
	if (OglBindBmTex (bmTop, 1, 3))
		return 1;
	bmTop = BmCurFrame (bmTop, -1);
	OglTexWrap (bmTop->glTexture, GL_REPEAT);
	glBegin (GL_TRIANGLE_FAN);
	if (bDynLight) {
		for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
			G3VertexColor (G3GetNormal (*ppl, &vNormal), VmsVecToFloat (&vVertex, &((*ppl)->p3_vec)), (*ppl)->p3_index, NULL, 1, 1);
			SetTexCoord (uvlList + i, orient, 0, NULL);
			OglVertex3f (*ppl);
			}
		}
	else if (bLight) {
		for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
			SetTMapColor (uvlList + i, i, bmTop, 1, NULL);
			SetTexCoord (uvlList + i, orient, 0, NULL);
			OglVertex3f (*ppl);
			}
		}
	else {
		for (i = 0, ppl = pointList; i < nVerts; i++, ppl++) {
			SetTexCoord (uvlList + i, orient, 0, NULL);
			OglVertex3f (*ppl);
			}
		}
	glEnd ();
	glDepthFunc (GL_LESS);
#if OGL_CLEANUP
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D);
#endif
	}
else if (bShaderMerge) {
#if OGL_CLEANUP
	OglActiveTexture (GL_TEXTURE1_ARB, bDrawArrays);
	OGL_BINDTEX (0);
	glDisable (GL_TEXTURE_2D); // Disable the 2nd texture
#endif
	glUseProgramObject (tmProg = 0);
	}
OglActiveTexture (GL_TEXTURE0_ARB, bDrawArrays);
OGL_BINDTEX (0);
glDisable (GL_TEXTURE_2D);
tMapColor.index =
lightColor.index = 0;
if (!bBlend)
	glEnable (GL_BLEND);
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawTexPolyLightmap (
	int			nVerts, 
	g3sPoint		**pointList, 
	tUVL			*uvlList, 
	tUVL			*uvlLMap, 
	grsBitmap	*bmBot, 
	grsBitmap	*bmTop, 
	tOglTexture	*lightMap, 
	vmsVector	*pvNormal,
	int			orient, 
	int			bBlend)
{
	int			i, nFrame, bShaderMerge;
	int			bLight = 1;
	grsBitmap	*bmP = NULL;
	g3sPoint		**ppl;

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVerts, pointList);
	return 0;
	}
if (!bmBot)
	return 1;
//if (gameStates.render.nShadowPass != 3)
	glDepthFunc (GL_LEQUAL);
if (FAST_SHADOWS) {
	if (bBlend)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glDisable (GL_BLEND);
	}
else {
	if (gameStates.render.nShadowPass == 3) {
		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE, GL_ONE);
		}
	}
glDepthFunc (GL_LEQUAL);
bmBot = BmOverride (bmBot, -1);
if ((bmTop = BmOverride (bmTop, -1)) && BM_FRAMES (bmTop)) {
	nFrame = (int) (BM_CURFRAME (bmTop) - BM_FRAMES (bmTop));
	bmP = bmTop;
	bmTop = BM_CURFRAME (bmTop);
	}
else
	nFrame = -1;
if (!lightMap) //lightMapping enabled
	return fpDrawTexPolyMulti (nVerts, pointList, uvlList, uvlLMap, bmBot, bmTop, lightMap, pvNormal, orient, bBlend);
// chose shaders depending on whether overlay bitmap present or not
if (bShaderMerge = bmTop && gameOpts->ogl.bGlTexMerge) {
	lmProg = lmShaderProgs [(bmTop->bmProps.flags & BM_FLAG_SUPER_TRANSPARENT) != 0];
	glUseProgramObject (lmProg);
	}
InitTMU0 (0);	// use render pipeline 0 for bottom texture
if (OglBindBmTex (bmBot, 1, 3))
	return 1;
bmBot = BmCurFrame (bmBot, -1);
OglTexWrap (bmBot->glTexture, GL_REPEAT);
if (bShaderMerge)
	glUniform1i (glGetUniformLocation (lmProg, "btmTex"), 0);
if (bmTop) { // use render pipeline 0 for overlay texture
	InitTMU1 (0);
	if (OglBindBmTex (bmTop, 1, 3))
		return 1;
	bmTop = BmCurFrame (bmTop, -1);
	OglTexWrap (bmTop->glTexture, GL_REPEAT);
	glUniform1i (glGetUniformLocation (lmProg, "topTex"), 1);
	}
// use render pipeline 2 for lightmap texture
InitTMU2 (0);
OGL_BINDTEX (lightMap->handle);
if (bShaderMerge)
	glUniform1i (glGetUniformLocation (lmProg, "lMapTex"), 2);
glBegin (GL_TRIANGLE_FAN);
ppl = pointList;
if (gameStates.render.bFullBright)
	glColor3d (1,1,1);
for (i = 0; i < nVerts; i++, ppl++) {
	if (!gameStates.render.bFullBright)
		SetTMapColor (uvlList + i, i, bmBot, 1, NULL);
	glMultiTexCoord2f (GL_TEXTURE0_ARB, f2fl (uvlList [i].u), f2fl (uvlList [i].v));
	if (bmTop)
		SetTexCoord (uvlList + i, orient, 1, NULL);
	glMultiTexCoord2f (GL_TEXTURE2_ARB, f2fl (uvlLMap [i].u), f2fl (uvlLMap [i].v));
	OglVertex3f (*ppl);
	}
glEnd ();
ExitTMU (0);
if (bShaderMerge)
	glUseProgramObject (lmProg = 0);
return 0;
}

//------------------------------------------------------------------------------

bool G3DrawSprite (
	vmsVector	*vPos, 
	fix			xWidth, 
	fix			xHeight, 
	grsBitmap	*bmP, 
	tRgbaColorf	*colorP,
	float			alpha)
{
	vmsVector	pv, v1;
	GLdouble		h, w, u, v, x, y, z;

if ((gameOpts->render.bDepthSort > 0) && ((colorP && (colorP->alpha < 0)) || (alpha < 0))) {
	tRgbaColorf color;
	if (!colorP) {
		color.red =
		color.green =
		color.blue = 1;
		color.alpha = alpha;
		colorP = &color;
		}
	RIAddSprite (bmP, vPos, colorP, xWidth, xHeight, 0);
	}
else {
	OglActiveTexture (GL_TEXTURE0_ARB, 0);
	VmVecSub (&v1, vPos, &viewInfo.pos);
	VmVecRotate (&pv, &v1, &viewInfo.view [0]);
	x = (double) f2fl (pv.p.x);
	y = (double) f2fl (pv.p.y);
	z = (double) f2fl (pv.p.z);
	w = (double) f2fl (xWidth); 
	h = (double) f2fl (xHeight); 
	if (gameStates.render.nShadowBlurPass == 1) {
		glDisable (GL_TEXTURE_2D);
		glColor4d (1,1,1,1);
		glBegin (GL_QUADS);
		glVertex3d (x - w, y + h, z);
		glVertex3d (x + w, y + h, z);
		glVertex3d (x + w, y - h, z);
		glVertex3d (x - w, y - h, z);
		glEnd ();
		}
	else {
		glDepthMask (0);
		glEnable (GL_TEXTURE_2D);
		if (OglBindBmTex (bmP, 1, 1)) 
			return 1;
		bmP = BmOverride (bmP, -1);
		OglTexWrap (bmP->glTexture, GL_CLAMP);
		if (colorP)
			glColor4f (colorP->red, colorP->green, colorP->blue, colorP->alpha);
		else
			glColor4d (1, 1, 1, (double) alpha);
		glBegin (GL_QUADS);
		u = bmP->glTexture->u;
		v = bmP->glTexture->v;
		glTexCoord2d (0, 0);
		glVertex3d (x - w, y + h, z);
		glTexCoord2d (u, 0);
		glVertex3d (x + w, y + h, z);
		glTexCoord2d (u, v);
		glVertex3d (x + w, y - h, z);
		glTexCoord2d (0, v);
		glVertex3d (x - w, y - h, z);
		glEnd ();
		glDepthMask (0);
		}
	}
return 0;
} 

//------------------------------------------------------------------------------

bool G3DrawBitmap (
	vmsVector	*vPos, 
	fix			width, 
	fix			height, 
	grsBitmap	*bmP, 
	tRgbaColorf	*color,
	float			alpha, 
	int			transp)
{
	fVector		fPos;
	GLfloat		h, w, u, v;

r_bitmapc++;
OglActiveTexture (GL_TEXTURE0_ARB, 0);
glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if 1
VmsVecToFloat (&fPos, vPos);
G3TransformPointf (&fPos, &fPos, 0);
#else
VmVecSub (&v1, vPos, &viewInfo.vPos);
VmVecRotate (&pv, &v1, &viewInfo.view [0]);
#endif
w = (GLfloat) f2fl (width); //FixMul (width, viewInfo.scale.x));
h = (GLfloat) f2fl (height); //FixMul (height, viewInfo.scale.y));
if (gameStates.render.nShadowBlurPass == 1) {
	glDisable (GL_TEXTURE_2D);
	glColor4d (1,1,1,1);
	glBegin (GL_QUADS);
	fPos.p.x -= w;
	fPos.p.y += h;
	glVertex3fv ((GLfloat *) &fPos);
	fPos.p.x += 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	fPos.p.y -= 2 * h;
	glVertex3fv ((GLfloat *) &fPos);
	fPos.p.x -= 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	glEnd ();
	}
else {
	glEnable (GL_TEXTURE_2D);
	if (OglBindBmTex (bmP, 1, transp)) 
		return 1;
	bmP = BmOverride (bmP, -1);
	OglTexWrap (bmP->glTexture, GL_CLAMP);
	if (color)
		glColor4f (color->red, color->green, color->blue, alpha);
	else
		glColor4d (1, 1, 1, (double) alpha);
	u = bmP->glTexture->u;
	v = bmP->glTexture->v;
	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	fPos.p.x -= w;
	fPos.p.y += h;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (u, 0);
	fPos.p.x += 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (u, v);
	fPos.p.y -= 2 * h;
	glVertex3fv ((GLfloat *) &fPos);
	glTexCoord2f (0, v);
	fPos.p.x -= 2 * w;
	glVertex3fv ((GLfloat *) &fPos);
	glEnd ();
	}
return 0;
} 

//------------------------------------------------------------------------------

inline void BmSetTexCoord (GLfloat u, GLfloat v, GLfloat a, int orient)
{
switch (orient) {
	case 1:
		glMultiTexCoord2d (GL_TEXTURE0_ARB, (1.0f - v) * a, u);
		break;
	case 2:
		glMultiTexCoord2d (GL_TEXTURE0_ARB, 1.0f - u, 1.0 - v);
		break;
	case 3:
		glMultiTexCoord2d (GL_TEXTURE0_ARB, v * a, (1.0f - u));
		break;
	default:
		glMultiTexCoord2d (GL_TEXTURE0_ARB, u, v);
		break;
	}
}

//------------------------------------------------------------------------------

bool OglUBitMapMC (int x, int y, int dw, int dh, grsBitmap *bmP, grsColor *c, int scale, int orient)
{
	GLint depthFunc, bBlend;
	GLfloat xo, yo, xf, yf;
	GLfloat u1, u2, v1, v2;
	GLfloat	h, a;
	GLfloat	dx, dy;

bmP = BmOverride (bmP, -1);
if (dw < 0)
	dw = grdCurCanv->cvBitmap.bmProps.w;
else if (dw == 0)
	dw = bmP->bmProps.w;
if (dh < 0)
	dh = grdCurCanv->cvBitmap.bmProps.h;
else if (dh == 0)
	dh = bmP->bmProps.h;
r_ubitmapc++;
if (orient & 1) {
	int h = dw;
	dw = dh;
	dh = h;
	dx = (float) grdCurCanv->cvBitmap.bmProps.y / (float) gameStates.ogl.nLastH;
	dy = (float) grdCurCanv->cvBitmap.bmProps.x / (float) gameStates.ogl.nLastW;
	}
else {
	dx = (float) grdCurCanv->cvBitmap.bmProps.x / (float) gameStates.ogl.nLastW;
	dy = (float) grdCurCanv->cvBitmap.bmProps.y / (float) gameStates.ogl.nLastH;
	}
#if 0
a = 1.0;
h = 1.0;
orient = 0;
#else
a = (float) grdCurScreen->scWidth / (float) grdCurScreen->scHeight;
h = (float) scale / (float) F1_0;
#endif
xo = dx + x / ((float) gameStates.ogl.nLastW * h);
xf = dx + (dw + x) / ((float) gameStates.ogl.nLastW * h);
yo = 1.0f - dy - y / ((float) gameStates.ogl.nLastH * h);
yf = 1.0f - dy - (dh + y) / ((float) gameStates.ogl.nLastH * h);

OglActiveTexture (GL_TEXTURE0_ARB, 0);
if (OglBindBmTex (bmP, 0, 3))
	return 1;
OglTexWrap (bmP->glTexture, GL_CLAMP);

glEnable (GL_TEXTURE_2D);
glGetIntegerv (GL_DEPTH_FUNC, &depthFunc);
glDepthFunc (GL_ALWAYS);
//glEnable (GL_ALPHA_TEST);
if (!(bBlend = glIsEnabled (GL_BLEND)))
	glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

if (bmP->bmProps.x == 0) {
	u1 = 0;
	if (bmP->bmProps.w == bmP->glTexture->w)
		u2 = bmP->glTexture->u;
	else
		u2 = (bmP->bmProps.w + bmP->bmProps.x) / (float) bmP->glTexture->tw;
	}
else {
	u1 = bmP->bmProps.x / (float) bmP->glTexture->tw;
	u2 = (bmP->bmProps.w + bmP->bmProps.x) / (float) bmP->glTexture->tw;
	}
if (bmP->bmProps.y == 0) {
	v1 = 0;
	if (bmP->bmProps.h == bmP->glTexture->h)
		v2 = bmP->glTexture->v;
	else
		v2 = (bmP->bmProps.h + bmP->bmProps.y) / (float) bmP->glTexture->th;
	}
else{
	v1 = bmP->bmProps.y / (float) bmP->glTexture->th;
	v2 = (bmP->bmProps.h + bmP->bmProps.y) / (float) bmP->glTexture->th;
	}

glBegin (GL_QUADS);
OglGrsColor (c);
BmSetTexCoord (u1, v1, a, orient);
glVertex2f (xo, yo);
BmSetTexCoord (u2, v1, a, orient);
glVertex2f (xf, yo);
BmSetTexCoord (u2, v2, a, orient);
glVertex2f (xf, yf);
BmSetTexCoord (u1, v2, a, orient);
glVertex2f (xo, yf);
glEnd ();
glDepthFunc (depthFunc);
//glDisable (GL_ALPHA_TEST);
if (!bBlend)
	glDisable (GL_BLEND);
OglActiveTexture (GL_TEXTURE0_ARB, 0);
OGL_BINDTEX (0);
glDisable (GL_TEXTURE_2D);
return 0;
}

//------------------------------------------------------------------------------

bool OglUBitBltI (
	int dw, int dh, int dx, int dy, 
	int sw, int sh, int sx, int sy, 
	grsBitmap *src, grsBitmap *dest, 
	int bMipMaps, int bTransp)
{
	GLdouble xo, yo, xs, ys;
	GLdouble u1, v1;//, u2, v2;
	tOglTexture tex, *texP;
	GLint curFunc; 
	int nTransp = (src->bmProps.flags & BM_FLAG_TGA) ? -1 : GrBitmapHasTransparency (src) ? 2 : 0;

//	unsigned char *oldpal;
r_ubitbltc++;

u1 = v1 = 0;
dx += dest->bmProps.x;
dy += dest->bmProps.y;
xo = dx / (double) gameStates.ogl.nLastW;
xs = dw / (double) gameStates.ogl.nLastW;
yo = 1.0 - dy / (double) gameStates.ogl.nLastH;
ys = dh / (double) gameStates.ogl.nLastH;

glEnable (GL_TEXTURE_2D);
if (!(texP = src->glTexture)) {
	texP = &tex;
	OglInitTexture (texP, 0);
	texP->w = sw;
	texP->h = sh;
	texP->prio = 0.0;
	texP->bMipMaps = bMipMaps;
	texP->lw = src->bmProps.rowSize;
	OglLoadTexture (src, sx, sy, texP, nTransp, 0);
	}
OGL_BINDTEX (texP->handle);
OglTexWrap (texP, GL_CLAMP);
glGetIntegerv (GL_DEPTH_FUNC, &curFunc);
glDepthFunc (GL_ALWAYS); 
if (bTransp && nTransp) {
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1.0, 1.0, 1.0, 1.0);
	}
else {
	glDisable (GL_BLEND);
	glColor3f (1.0, 1.0, 1.0);
	}
glBegin (GL_QUADS);
glTexCoord2d (u1, v1); 
glVertex2d (xo, yo);
glTexCoord2d (texP->u, v1); 
glVertex2d (xo + xs, yo);
glTexCoord2d (texP->u, texP->v); 
glVertex2d (xo + xs, yo-ys);
glTexCoord2d (u1, texP->v); 
glVertex2d (xo, yo - ys);
glEnd ();
if (bTransp && nTransp)
	glDisable (GL_BLEND);
glDisable (GL_TEXTURE_2D);
glDepthFunc (curFunc);
if (!src->glTexture)
	OglFreeTexture (texP);
return 0;
}

//------------------------------------------------------------------------------

bool OglUBitBltToLinear (int w, int h, int dx, int dy, int sx, int sy, 
								 grsBitmap * src, grsBitmap * dest)
{
	unsigned char *d, *s;
	int i, j;
	int w1, h1;
	int bTGA = (dest->bmProps.flags & BM_FLAG_TGA) != 0;
//	w1=w;h1=h;
w1=grdCurScreen->scWidth;
h1=grdCurScreen->scHeight;

if (gameStates.ogl.bReadPixels > 0) {
	glDisable (GL_TEXTURE_2D);
	glReadBuffer (GL_FRONT);
	if (bTGA)
		glReadPixels (0, 0, w1, h1, GL_RGBA, GL_UNSIGNED_BYTE, gameData.render.ogl.texBuf);
//			glReadPixels (sx, grdCurScreen->scHeight - (sy + h), w, h, GL_RGBA, GL_UNSIGNED_BYTE, dest->bmTexBuf);
	else {
		if (w1*h1*3>OGLTEXBUFSIZE)
			Error ("OglUBitBltToLinear: screen res larger than OGLTEXBUFSIZE\n");
		glReadPixels (0, 0, w1, h1, GL_RGB, GL_UNSIGNED_BYTE, gameData.render.ogl.texBuf);
		}
//		glReadPixels (sx, grdCurScreen->scHeight- (sy+h), w, h, GL_RGB, GL_UNSIGNED_BYTE, gameData.render.ogl.texBuf);
//		glReadPixels (sx, sy, w+sx, h+sy, GL_RGB, GL_UNSIGNED_BYTE, gameData.render.ogl.texBuf);
	}
else {
	if (dest->bmProps.flags & BM_FLAG_TGA)
		memset (dest->bmTexBuf, 0, w1*h1*4);
	else
		memset (gameData.render.ogl.texBuf, 0, w1*h1*3);
	}
if (bTGA) {
	sx += src->bmProps.x;
	sy += src->bmProps.y;
	for (i = 0; i < h; i++) {
		d = dest->bmTexBuf + dx + (dy + i) * dest->bmProps.rowSize;
		s = gameData.render.ogl.texBuf + ((h1 - (i + sy + 1)) * w1 + sx) * 4;
		memcpy (d, s, w * 4);
		}
/*
	for (i = 0; i < h; i++)
		memcpy (dest->bmTexBuf + (h - i - 1) * dest->bmProps.rowSize, 
				  gameData.render.ogl.texBuf + ((sy + i) * h1 + sx) * 4, 
				  dest->bmProps.rowSize);
*/
	}
else {
	sx += src->bmProps.x;
	sy += src->bmProps.y;
	for (i = 0; i < h; i++) {
		d = dest->bmTexBuf + dx + (dy + i) * dest->bmProps.rowSize;
		s = gameData.render.ogl.texBuf + ((h1 - (i + sy + 1)) * w1 + sx) * 3;
		for (j = 0; j < w; j++) {
			*d++ = GrFindClosestColor (dest->bmPalette, s [0] / 4, s [1] / 4, s [2] / 4);
			s += 3;
			}
		}
	}
return 0;
}

//------------------------------------------------------------------------------

bool OglUBitBltCopy (int w, int h, int dx, int dy, int sx, int sy, grsBitmap * src, grsBitmap * dest)
{
#if 0 //just seems to cause a mess.
	GLdouble xo, yo;//, xs, ys;
	
	dx+=dest->bmProps.x;
	dy+=dest->bmProps.y;
	
//	xo=dx/ (double)gameStates.ogl.nLastW;
	xo=dx/ (double)grdCurScreen->scWidth;
//	yo=1.0- (dy+h)/ (double)gameStates.ogl.nLastH;
	yo=1.0- (dy+h)/ (double)grdCurScreen->scHeight;
	sx+=src->bmProps.x;
	sy+=src->bmProps.y;
	glDisable (GL_TEXTURE_2D);
	glReadBuffer (GL_FRONT);
	glRasterPos2f (xo, yo);
//	glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, gameData.render.ogl.texBuf);
	glCopyPixels (sx, grdCurScreen->scHeight- (sy+h), w, h, GL_COLOR);
	glRasterPos2f (0, 0);
#endif
	return 0;
}

//------------------------------------------------------------------------------

void OglSetFOV (double fov)
{
gameStates.render.glFOV = fov;
#if 0
gameStates.render.glAspect = 90.0 / gameStates.render.glFOV;
#else
if (gameStates.ogl.bUseTransform)
	gameStates.render.glAspect = (double) grdCurScreen->scWidth / (double) grdCurScreen->scHeight;
else
	gameStates.render.glAspect = 90.0 / gameStates.render.glFOV;
#endif
gluPerspective (gameStates.render.glFOV, gameStates.render.glAspect, 0.01, 
					 (gameStates.ogl.nColorBits == 16) ? 10000.0 : 6553.5);
glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

//------------------------------------------------------------------------------

#define GL_INFINITY	0

void OglStartFrame (int bFlat, int bResetColorBuf)
{
#if SHADOWS
if (gameStates.render.nShadowPass) {
#if GL_INFINITY
	float	infProj [4][4];	//projection to infinity
#endif
	
	if (gameStates.render.nShadowPass == 1) {	//render unlit/final scene
		if (!gameStates.render.bShadowMaps) {
#if GL_INFINITY
			glMatrixMode (GL_PROJECTION);
			memset (infProj, 0, sizeof (infProj));
			infProj [1][1] = 1.0f / (float) tan (gameStates.render.glFOV);
			infProj [0][0] = infProj [1][1] / (float) gameStates.render.glAspect;
			infProj [3][2] = -0.02f;	// -2 * near
			infProj [2][2] =
			infProj [2][3] = -1.0f;
			glLoadMatrixf ((float *) infProj);
#endif
#if 0
			glMatrixMode (GL_MODELVIEW);
			glLoadIdentity ();
#endif
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_STENCIL_TEST);
			glDepthFunc (GL_LESS);
			glEnable (GL_CULL_FACE);		
			glCullFace (GL_BACK);
			if (!FAST_SHADOWS)
				glColorMask (0,0,0,0);
			}
		}
	else if (gameStates.render.nShadowPass == 2) {	//render occluders / shadow maps
		if (gameStates.render.bShadowMaps) {
			glColorMask (0,0,0,0);
			glEnable (GL_POLYGON_OFFSET_FILL);
			glPolygonOffset (1.0f, 2.0f);
			}
		else {
#	if DBG_SHADOWS
			if (bShadowTest) {
				glColorMask (1,1,1,1);
				glDepthMask (0);
				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable (GL_STENCIL_TEST);
				}
			else 
#	endif
				{
				glColorMask (0,0,0,0);
				glDepthMask (0);
				glEnable (GL_STENCIL_TEST);
				if (!glIsEnabled (GL_STENCIL_TEST))
					extraGameInfo [0].bShadows = 
					extraGameInfo [1].bShadows = 0;
				glClearStencil (0);
				glClear (GL_STENCIL_BUFFER_BIT);
#if 0
				if (!glActiveStencilFaceEXT)
#endif
					bSingleStencil = 1;
#	if DBG_SHADOWS
				if (bSingleStencil || bShadowTest) {
#	else
				if (bSingleStencil) {
#	endif
					glStencilMask (~0);
					glStencilFunc (GL_ALWAYS, 0, ~0);
					}
#if 0
				else {
					glEnable (GL_STENCIL_TEST_TWO_SIDE_EXT);
					glActiveStencilFaceEXT (GL_BACK);
					if (bZPass)
						glStencilOp (GL_KEEP, GL_KEEP, GL_DECR_WRAP);
					else
						glStencilOp (GL_KEEP, GL_DECR_WRAP, GL_KEEP);
					glStencilOp (GL_KEEP, GL_DECR_WRAP, GL_KEEP);
					glStencilMask (~0);
					glStencilFunc (GL_ALWAYS, 0, ~0);
					glActiveStencilFaceEXT (GL_FRONT);
					if (bZPass)
						glStencilOp (GL_KEEP, GL_KEEP, GL_INCR_WRAP);
					else
						glStencilOp (GL_KEEP, GL_INCR_WRAP, GL_KEEP);
					glStencilMask (~0);
					glStencilFunc (GL_ALWAYS, 0, ~0);
					}
#endif
#if 0
				glEnable (GL_POLYGON_OFFSET_FILL);
				glPolygonOffset (1.0f, 1.0f);
#endif
				}
			}
		}
	else if (gameStates.render.nShadowPass == 3) { //render final lit scene
		if (gameStates.render.bShadowMaps) {
			glDisable (GL_POLYGON_OFFSET_FILL);
			glDepthFunc (GL_LESS);
			}
		else {
#if 0
			glDisable (GL_POLYGON_OFFSET_FILL);
#endif
			if (gameStates.render.nShadowBlurPass == 2)
				glDisable (GL_STENCIL_TEST);
         else if (FAST_SHADOWS) {
				glStencilFunc (GL_NOTEQUAL, 0, ~0);
				glStencilOp (GL_REPLACE, GL_KEEP, GL_KEEP);		
				}
			else 
				{
				glStencilFunc (GL_EQUAL, 0, ~0);
#if 0
				glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);	//problem: layered texturing fails
#else
				glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
#endif
				}
			glCullFace (GL_BACK);
			glDepthFunc (GL_LESS);
			glColorMask (1,1,1,1);
			}
		}
	else if (gameStates.render.nShadowPass == 4) {	//render unlit/final scene
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LESS);
		glEnable (GL_CULL_FACE);		
		glCullFace (GL_BACK);
		}
#if GL_INFINITY
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
#endif
	}
else 
#endif //SHADOWS
	{
	r_polyc =  
	r_tpolyc = 
	r_bitmapc = 
	r_ubitmapc = 
	r_ubitbltc = 
	r_upixelc = 0;

	//if (gameStates.render.nShadowBlurPass < 2) 
		{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();//clear matrix
		OglSetFOV (gameStates.render.glFOV);
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		OGL_VIEWPORT (grdCurCanv->cvBitmap.bmProps.x, grdCurCanv->cvBitmap.bmProps.y, 
						  nCanvasWidth, nCanvasHeight);
		}
	if (gameStates.render.nRenderPass < 0) {
		glDepthMask (1);
		glColorMask (1,1,1,1);
		glClearColor (0,0,0,0);
		if (bResetColorBuf)
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			glClear (GL_DEPTH_BUFFER_BIT);
		}
	else if (gameStates.render.nRenderPass) {
		glDepthMask (0);
		glColorMask (1,1,1,1);
		glClearColor (0,0,0,0);
		if (bResetColorBuf)
			glClear (GL_COLOR_BUFFER_BIT);
		}
	else { //make a depth-only render pass first to decrease bandwidth waste due to overdraw
		glDepthMask (1);
		glColorMask (0,0,0,0);
		glClear (GL_DEPTH_BUFFER_BIT);
		}
	if (gameStates.ogl.bEnableScissor) {
		glScissor (
			grdCurCanv->cvBitmap.bmProps.x, 
			grdCurScreen->scCanvas.cvBitmap.bmProps.h - grdCurCanv->cvBitmap.bmProps.y - nCanvasHeight, 
			nCanvasWidth, 
			nCanvasHeight);
		glEnable (GL_SCISSOR_TEST);
		}
	else
		glDisable (GL_SCISSOR_TEST);
	if (gameStates.ogl.bAntiAliasingOk && gameStates.ogl.bAntiAliasing)
		glEnable (GL_MULTISAMPLE_ARB);
	if (bFlat) {
		glDisable (GL_DEPTH_TEST);
		glDisable (GL_ALPHA_TEST);
		glDisable (GL_CULL_FACE);
		}
	else {
		glEnable (GL_CULL_FACE);		
		glFrontFace (GL_CW);	//Weird, huh? Well, D2 renders everything reverse ...
		glCullFace (GL_BACK);
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LESS);
		glEnable (GL_ALPHA_TEST);
		glAlphaFunc (GL_GEQUAL, (float) 0.01);	
		}
#if 0
	if (SHOW_DYN_LIGHT)	{//for optional hardware lighting
		GLfloat fAmbient [4] = {0.0f, 0.0f, 0.0f, 1.0f};
		glEnable (GL_LIGHTING);
		glLightModelfv (GL_LIGHT_MODEL_AMBIENT, fAmbient);
		glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, 0);
		glShadeModel (GL_SMOOTH);
		glEnable (GL_COLOR_MATERIAL);
		glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		}
#endif
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_STENCIL_TEST);
	}
}

//------------------------------------------------------------------------------

#ifndef NMONO
void merge_textures_stats (void);
#endif

//------------------------------------------------------------------------------

void OglEndFrame (void)
{
//	OGL_VIEWPORT (grdCurCanv->cvBitmap.bmProps.x, grdCurCanv->cvBitmap.bmProps.y, );
//	glViewport (0, 0, grdCurScreen->scWidth, grdCurScreen->scHeight);
OGL_VIEWPORT (0, 0, grdCurScreen->scWidth, grdCurScreen->scHeight);
#ifndef NMONO
//	merge_textures_stats ();
//	ogl_texture_stats ();
#endif
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();//clear matrix
glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
glMatrixMode (GL_MODELVIEW);
glLoadIdentity ();//clear matrix
glDisable (GL_SCISSOR_TEST);
glDisable (GL_ALPHA_TEST);
glDisable (GL_DEPTH_TEST);
glDisable (GL_CULL_FACE);
glDisable (GL_STENCIL_TEST);
if (SHOW_DYN_LIGHT) {
	glDisable (GL_LIGHTING);
	glDisable (GL_COLOR_MATERIAL);
	}
glDepthMask (1);
glColorMask (1,1,1,1);
if (gameStates.ogl.bAntiAliasingOk && gameStates.ogl.bAntiAliasing)
	glDisable (GL_MULTISAMPLE_ARB);
}

//------------------------------------------------------------------------------

void OglSwapBuffers (int bForce, int bClear)
{
if (!gameStates.menus.nInMenu || bForce) {
	ogl_clean_texture_cache ();
	if (gr_renderstats)
		GrPrintF (5, GAME_FONT->ftHeight*13+3*13, "%i flat %i tex %i sprites %i bitmaps", r_polyc, r_tpolyc, r_bitmapc, r_ubitmapc);
	//if (gameStates.app.bGameRunning && !gameStates.menus.nInMenu)
		OglDoPalFx ();
	OglSwapBuffersInternal ();
	if (gameStates.menus.nInMenu || bClear)
		glClear (GL_COLOR_BUFFER_BIT);
	}
}

// -----------------------------------------------------------------------------------

void OglSetupTransform (void)
{
if (gameStates.ogl.bUseTransform) {
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	//glScalef (f2fl (viewInfo.scale.x), f2fl (viewInfo.scale.y), -f2fl (viewInfo.scale.z));
	glMultMatrixf (viewInfo.glViewf);
	glTranslatef (-viewInfo.glPosf [0], -viewInfo.glPosf [1], -viewInfo.glPosf [2]);
	}
}

// -----------------------------------------------------------------------------------

void OglResetTransform (void)
{
if (gameStates.ogl.bUseTransform)
	glPopMatrix ();
}

//------------------------------------------------------------------------------

int OglRenderArrays (grsBitmap *bmP, int nFrame, fVector *vertexP, int nVertices, tUVLf *texCoordP, 
							tRgbaColorf *colorP, int nColors, int nPrimitive, int nWrap)
{
	int	bDrawArrays = OglEnableClientState (GL_VERTEX_ARRAY) &&
							  (!bmP || !texCoordP || OglEnableClientState (GL_TEXTURE_COORD_ARRAY)) &&
							  (!colorP || (nColors < nVertices) || OglEnableClientState (GL_COLOR_ARRAY));

if (bDrawArrays)
	glClientActiveTexture (GL_TEXTURE0_ARB);
if (bmP)
	glEnable (GL_TEXTURE_2D);
else
	glDisable (GL_TEXTURE_2D);
if (bmP) {
	if (OglBindBmTex (bmP, 1, 1))
		return 0;
	bmP = BmOverride (bmP, -1);
	if (BM_FRAMES (bmP))
		bmP = BM_FRAMES (bmP) + nFrame;
	OglTexWrap (bmP->glTexture, nWrap);
	}
if (bDrawArrays) {
	glVertexPointer (3, GL_FLOAT, sizeof (fVector), vertexP);
	if (texCoordP)
		glTexCoordPointer (2, GL_FLOAT, sizeof (tUVLf), texCoordP);
	if (colorP) {
		if (nColors == nVertices)
			glColorPointer (4, GL_FLOAT, sizeof (tRgbaColorf), colorP);
		else
			glColor4fv ((GLfloat *) colorP);
		}
	glDrawArrays (nPrimitive, 0, nVertices);
	glDisableClientState (GL_VERTEX_ARRAY);
	if (texCoordP)
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	if (colorP)
		glDisableClientState (GL_COLOR_ARRAY);
	}
else {
	int i = nVertices;
	glBegin (nPrimitive);
	if (colorP && (nColors == nVertices)) {
		if (bmP) {
			for (i = 0; i < nVertices; i++) {
				glColor4fv ((GLfloat *) (colorP + i));
				glVertex3fv ((GLfloat *) (vertexP + i));
				glTexCoord2fv ((GLfloat *) (texCoordP + i));
				}
			}
		else {
			for (i = 0; i < nVertices; i++) {
				glColor4fv ((GLfloat *) (colorP + i));
				glVertex3fv ((GLfloat *) (vertexP + i));
				}
			}
		}
	else {
		if (colorP)
			glColor4fv ((GLfloat *) colorP);
		else
			glColor3d (1, 1, 1);
		if (bmP) {
			for (i = 0; i < nVertices; i++) {
				glVertex3fv ((GLfloat *) (vertexP + i));
				glTexCoord2fv ((GLfloat *) (texCoordP + i));
				}
			}
		else {
			for (i = 0; i < nVertices; i++) {
				glVertex3fv ((GLfloat *) (vertexP + i));
				}
			}
		}
	glEnd ();
	}
return 1;
}

//------------------------------------------------------------------------------
