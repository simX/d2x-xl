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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "descent.h"
#include "rendermine.h"
#include "cockpit.h"
#include "automap.h"
#include "ogl_render.h"

// -----------------------------------------------------------------------------------

int radarRanges [] = {100, 150, 200};

#define RADAR_RANGE	radarRanges [gameOpts->render.automap.nRange]
#define RADAR_SLICES	40
#define BLIP_SLICES	40

static CAngleVector	aRadar = CAngleVector::Create(I2X (1) / 4, 0, 0);
static CFixMatrix		mRadar;
static float			yOffs = 18.0f;
static float			yRadar, fRadius = 10.0f;
static float			fLineWidth = 1.0f;

void RenderRadarBlip (CObject *objP, float r, float g, float b, float a)
{
	CFixVector	n, v [2];
	fix			m;
	float			h, s;

	static tSinCosf sinCosRadar [RADAR_SLICES];
	static tSinCosf sinCosBlip [BLIP_SLICES];
	static int bInitSinCos = 1;

if (bInitSinCos) {
	OglComputeSinCos (sizeofa (sinCosRadar), sinCosRadar);
	OglComputeSinCos (sizeofa (sinCosBlip), sinCosBlip);
	bInitSinCos = 0;
	}
n = objP->info.position.vPos;
transformation.Transform (n, n, 0);
if ((m = n.Mag ()) > I2X (RADAR_RANGE))
	return;
if (m) {
	//HUDMessage (0, "%1.2f", X2F (m));
	v [0][X] = FixDiv (n [X], m) * 15; // /= RADAR_RANGE;
	v [0][Y] = FixDiv (n [Y], m) * 20; // /= RADAR_RANGE;
	v [0][Z] = n[X] / RADAR_RANGE;
	//VmVecNormalize (&n);
	}
else {
	glPushMatrix ();
	glColor4f (r, g, b, a);
	glTranslatef (0, yRadar, 50);
#if 0
	glColor4f (r, g, b, a / 2);
 	OglDrawEllipse (RADAR_SLICES, GL_POLYGON, 10, 0, 7.5f, 0, sinCosRadar);
#endif
	glColor4f (r, g, b, a);
 	OglDrawEllipse (RADAR_SLICES, GL_POLYGON, fRadius, 0, fRadius / 3.0f, 0, NULL /*sinCosRadar*/);
	glColor4f (0.5f, 0.5f, 0.5f, 0.8f);
	glLineWidth (fLineWidth);
	//glEnable (GL_LINE_SMOOTH);
 	OglDrawEllipse (RADAR_SLICES, GL_LINE_LOOP, fRadius, 0, fRadius / 3.0f, 0, NULL /*sinCosRadar*/);
	//glDisable (GL_LINE_SMOOTH);
	glColor4f (0.6f, 0.6f, 0.6f, 0.75f);
	glLineWidth (1.5f * fLineWidth);
	//glEnable (GL_LINE_SMOOTH);
 	OglDrawEllipse (RADAR_SLICES, GL_LINE_LOOP, 2 * fRadius / 3.0f, 0, 2 * fRadius / 9.0f, 0, NULL /*sinCosRadar*/);
	//glDisable (GL_LINE_SMOOTH);
	glColor4f (0.7f, 0.7f, 0.7f, 0.7f);
	glLineWidth (2 * fLineWidth);
	//glEnable (GL_LINE_SMOOTH);
 	OglDrawEllipse (RADAR_SLICES, GL_LINE_LOOP, fRadius / 3.0f, 0, fRadius / 9.0f, 0, NULL /*sinCosRadar*/);
	//glDisable (GL_LINE_SMOOTH);
	glLineWidth (fLineWidth);
	glBegin (GL_LINES);
	float x = fRadius * 0.707f + 0.333f;
	float y = fRadius / 3.0f;
	glVertex2f (0, y + 1);
	glVertex2f (0, -y - 1);
	//y += 0.333f;
	glVertex2f (-x, -y);
	glVertex2f (x, y);
	glVertex2f (-x, y);
	glVertex2f (x, -y);
	glEnd ();
	//glDisable (GL_LINE_SMOOTH);
	glLineWidth (2);
	glPopMatrix ();
	return;
	}
v[0] *= FixDiv(1, 3);
h = X2F (n[Z]) / RADAR_RANGE;
glPushMatrix ();
glTranslatef (0, yRadar + h * fRadius / 3.0f, 50);
glPushMatrix ();
s = 1.0f - (float) fabs (X2F (m) / RADAR_RANGE);
h = 3 * s;
a += a * h;
glColor4f (r + r * h, g + g * h, b + b * h, (float) sqrt (a));
glTranslatef (X2F (v [0][X]), X2F (v [0][Y]), X2F (v [0][Z]));
OglDrawEllipse (BLIP_SLICES, GL_POLYGON, 0.33f + 0.33f * s, 0, 0.33f + 0.33f * s, 0, sinCosBlip);
glPopMatrix ();
#if 1
v [1] = v [0];
v [1][Y] = 0;
glBegin (GL_LINES);
OglVertex3x (v [0][X], v [0][Y], v [0][Z]);
OglVertex3x (v [1][X], v [1][Y], v [1][Z]);
glEnd ();
#endif
glPopMatrix ();
}

// -----------------------------------------------------------------------------------

static tRgbColorf shipColors [8];
static tRgbColorf guidebotColor = {0, 0.75f / 4, 0.25f};
static tRgbColorf robotColor = {0.75f / 4, 0, 0.25f};
static tRgbColorf powerupColor = {0.25f, 0.5f / 4, 0};
static tRgbColorf radarColor [2] = {{1, 1, 1}, {0, 0, 0}};
static int bHaveShipColors = 0;

void InitShipColors (void)
{
if (!bHaveShipColors) {
	int	i;

	for (i = 0; i < 8; i++) {
		shipColors [i].red = 2 * playerColors [i].red / 255.0f;
		shipColors [i].green = 2 * playerColors [i].green / 255.0f;
		shipColors [i].blue = 2 * playerColors [i].blue / 255.0f;
		}
	bHaveShipColors = 1;
	}
}

// -----------------------------------------------------------------------------------

void RenderRadar (void)
{
	int			i, bStencil;
	CObject		*objP;
	GLint			depthFunc;
	tRgbColorf	*pc;

if (gameStates.app.bNostalgia)
	return;
if (cockpit->Hide ())
	return;
if (automap.m_bDisplay)
	return;
if (!(i = EGI_FLAG (nRadar, 0, 1, 0)))
	return;
bStencil = StencilOff ();
InitShipColors ();
yRadar = ((i == 1) || (gameStates.render.cockpit.nType == CM_FULL_COCKPIT) || (gameStates.render.cockpit.nType == CM_STATUS_BAR)) ? yOffs : -yOffs;
fRadius = 5.0f / transformation.m_info.scalef [X];
fLineWidth = (nCanvasWidth >= 1200) ? 2.0f : 1.0f;
mRadar = CFixMatrix::Create (aRadar);
glDisable (GL_CULL_FACE);
glGetIntegerv (GL_DEPTH_FUNC, &depthFunc);
glDepthFunc (GL_ALWAYS);
glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnable (GL_LINE_SMOOTH);
glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
glActiveTexture (GL_TEXTURE0);
glDisable (GL_TEXTURE_2D);
glLineWidth (fLineWidth);
pc = radarColor + gameOpts->render.automap.nColor;
RenderRadarBlip (gameData.objs.consoleP, pc->red, pc->green, pc->blue, 2.0f / 3.0f); //0.5, 0.75, 0.5, 2.0f / 3.0f);
glLineWidth (3);
FORALL_OBJS (objP, i) {
	if ((objP->info.nType == OBJ_PLAYER) && (objP != gameData.objs.consoleP)) {
		if (AM_SHOW_PLAYERS && AM_SHOW_PLAYER (objP->info.nId)) {
			pc = shipColors + (IsTeamGame ? GetTeam (objP->info.nId) : objP->info.nId);
			RenderRadarBlip (objP, pc->red, pc->green, pc->blue, 0.9f / 4);
			}
		}
	else if (objP->info.nType == OBJ_ROBOT) {
		if (AM_SHOW_ROBOTS) {
			if (ROBOTINFO (objP->info.nId).companion)
				RenderRadarBlip (objP, guidebotColor.red, guidebotColor.green, guidebotColor.blue, 0.9f / 4);
			else
				RenderRadarBlip (objP, robotColor.red, robotColor.green, robotColor.blue, 0.9f / 4);
			}
		}
	else if (objP->info.nType == OBJ_POWERUP) {
		if (AM_SHOW_POWERUPS (2))
			RenderRadarBlip (objP, powerupColor.red, powerupColor.green, powerupColor.blue, 0.9f / 4);
		}
	}
glDisable (GL_LINE_SMOOTH);
glLineWidth (1);
glDepthFunc (depthFunc);
glEnable (GL_CULL_FACE);
StencilOn (bStencil);
}

//------------------------------------------------------------------------------
// eof
